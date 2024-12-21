#include "../../include/capture/nvfbc.h"
#include "../../external/NvFBC.h"
#include "../../include/egl.h"
#include "../../include/utils.h"
#include "../../include/color_conversion.h"
#include "../../include/window/window.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <libavcodec/avcodec.h>

typedef struct {
    gsr_capture_nvfbc_params params;
    void *library;

    NVFBC_SESSION_HANDLE nv_fbc_handle;
    PNVFBCCREATEINSTANCE nv_fbc_create_instance;
    NVFBC_API_FUNCTION_LIST nv_fbc_function_list;
    bool fbc_handle_created;
    bool capture_session_created;

    NVFBC_TOGL_SETUP_PARAMS setup_params;

    bool supports_direct_cursor;
    bool capture_region;
    uint32_t x, y, width, height;
    NVFBC_TRACKING_TYPE tracking_type;
    uint32_t output_id;
    uint32_t tracking_width, tracking_height;
    bool nvfbc_needs_recreate;
    double nvfbc_dead_start;
} gsr_capture_nvfbc;

static int max_int(int a, int b) {
    return a > b ? a : b;
}

/* Returns 0 on failure */
static uint32_t get_output_id_from_display_name(NVFBC_RANDR_OUTPUT_INFO *outputs, uint32_t num_outputs, const char *display_name, uint32_t *width, uint32_t *height) {
    if(!outputs)
        return 0;

    for(uint32_t i = 0; i < num_outputs; ++i) {
        if(strcmp(outputs[i].name, display_name) == 0) {
            *width = outputs[i].trackedBox.w;
            *height = outputs[i].trackedBox.h;
            return outputs[i].dwId;
        }
    }

    return 0;
}

/* TODO: Test with optimus and open kernel modules */
static bool get_driver_version(int *major, int *minor) {
    *major = 0;
    *minor = 0;

    FILE *f = fopen("/proc/driver/nvidia/version", "rb");
    if(!f) {
        fprintf(stderr, "gsr warning: failed to get nvidia driver version (failed to read /proc/driver/nvidia/version)\n");
        return false;
    }

    char buffer[2048];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[bytes_read] = '\0';

    bool success = false;
    const char *p = strstr(buffer, "Kernel Module");
    if(p) {
        p += 13;
        int driver_major_version = 0, driver_minor_version = 0;
        if(sscanf(p, "%d.%d", &driver_major_version, &driver_minor_version) == 2) {
            *major = driver_major_version;
            *minor = driver_minor_version;
            success = true;
        }
    }

    if(!success)
        fprintf(stderr, "gsr warning: failed to get nvidia driver version\n");

    fclose(f);
    return success;
}

static bool version_at_least(int major, int minor, int expected_major, int expected_minor) {
    return major > expected_major || (major == expected_major && minor >= expected_minor);
}

static bool version_less_than(int major, int minor, int expected_major, int expected_minor) {
    return major < expected_major || (major == expected_major && minor < expected_minor);
}

static void set_func_ptr(void **dst, void *src) {
    *dst = src;
}

static bool gsr_capture_nvfbc_load_library(gsr_capture *cap) {
    gsr_capture_nvfbc *self = cap->priv;

    dlerror(); /* clear */
    void *lib = dlopen("libnvidia-fbc.so.1", RTLD_LAZY);
    if(!lib) {
        fprintf(stderr, "gsr error: failed to load libnvidia-fbc.so.1, error: %s\n", dlerror());
        return false;
    }

    set_func_ptr((void**)&self->nv_fbc_create_instance, dlsym(lib, "NvFBCCreateInstance"));
    if(!self->nv_fbc_create_instance) {
        fprintf(stderr, "gsr error: unable to resolve symbol 'NvFBCCreateInstance'\n");
        dlclose(lib);
        return false;
    }

    memset(&self->nv_fbc_function_list, 0, sizeof(self->nv_fbc_function_list));
    self->nv_fbc_function_list.dwVersion = NVFBC_VERSION;
    NVFBCSTATUS status = self->nv_fbc_create_instance(&self->nv_fbc_function_list);
    if(status != NVFBC_SUCCESS) {
        fprintf(stderr, "gsr error: failed to create NvFBC instance (status: %d)\n", status);
        dlclose(lib);
        return false;
    }

    self->library = lib;
    return true;
}

/* TODO: check for glx swap control extension string (GLX_EXT_swap_control, etc) */
static void set_vertical_sync_enabled(gsr_egl *egl, int enabled) {
    int result = 0;

    if(egl->glXSwapIntervalEXT) {
        assert(gsr_window_get_display_server(egl->window) == GSR_DISPLAY_SERVER_X11);
        Display *display = gsr_window_get_display(egl->window);
        const Window window = (Window)gsr_window_get_window(egl->window);
        egl->glXSwapIntervalEXT(display, window, enabled ? 1 : 0);
    } else if(egl->glXSwapIntervalMESA) {
        result = egl->glXSwapIntervalMESA(enabled ? 1 : 0);
    } else if(egl->glXSwapIntervalSGI) {
        result = egl->glXSwapIntervalSGI(enabled ? 1 : 0);
    } else {
        static int warned = 0;
        if (!warned) {
            warned = 1;
            fprintf(stderr, "gsr warning: setting vertical sync not supported\n");
        }
    }

    if(result != 0)
        fprintf(stderr, "gsr warning: setting vertical sync failed\n");
}

static void gsr_capture_nvfbc_destroy_session(gsr_capture_nvfbc *self) {
    if(self->fbc_handle_created && self->capture_session_created) {
        NVFBC_DESTROY_CAPTURE_SESSION_PARAMS destroy_capture_params;
        memset(&destroy_capture_params, 0, sizeof(destroy_capture_params));
        destroy_capture_params.dwVersion = NVFBC_DESTROY_CAPTURE_SESSION_PARAMS_VER;
        self->nv_fbc_function_list.nvFBCDestroyCaptureSession(self->nv_fbc_handle, &destroy_capture_params);
        self->capture_session_created = false;
    }
}

static void gsr_capture_nvfbc_destroy_handle(gsr_capture_nvfbc *self) {
    if(self->fbc_handle_created) {
        NVFBC_DESTROY_HANDLE_PARAMS destroy_params;
        memset(&destroy_params, 0, sizeof(destroy_params));
        destroy_params.dwVersion = NVFBC_DESTROY_HANDLE_PARAMS_VER;
        self->nv_fbc_function_list.nvFBCDestroyHandle(self->nv_fbc_handle, &destroy_params);
        self->fbc_handle_created = false;
        self->nv_fbc_handle = 0;
    }
}

static void gsr_capture_nvfbc_destroy_session_and_handle(gsr_capture_nvfbc *self) {
    gsr_capture_nvfbc_destroy_session(self);
    gsr_capture_nvfbc_destroy_handle(self);
}

static int gsr_capture_nvfbc_setup_handle(gsr_capture_nvfbc *self) {
    NVFBCSTATUS status;

    NVFBC_CREATE_HANDLE_PARAMS create_params;
    memset(&create_params, 0, sizeof(create_params));
    create_params.dwVersion = NVFBC_CREATE_HANDLE_PARAMS_VER;
    create_params.bExternallyManagedContext = NVFBC_TRUE;
    create_params.glxCtx = self->params.egl->glx_context;
    create_params.glxFBConfig = self->params.egl->glx_fb_config;

    status = self->nv_fbc_function_list.nvFBCCreateHandle(&self->nv_fbc_handle, &create_params);
    if(status != NVFBC_SUCCESS) {
        // Reverse engineering for interoperability
        const uint8_t enable_key[] = { 0xac, 0x10, 0xc9, 0x2e, 0xa5, 0xe6, 0x87, 0x4f, 0x8f, 0x4b, 0xf4, 0x61, 0xf8, 0x56, 0x27, 0xe9 };
        create_params.privateData = enable_key;
        create_params.privateDataSize = 16;

        status = self->nv_fbc_function_list.nvFBCCreateHandle(&self->nv_fbc_handle, &create_params);
        if(status != NVFBC_SUCCESS) {
            fprintf(stderr, "gsr error: gsr_capture_nvfbc_start failed: %s\n", self->nv_fbc_function_list.nvFBCGetLastErrorStr(self->nv_fbc_handle));
            goto error_cleanup;
        }
    }
    self->fbc_handle_created = true;

    NVFBC_GET_STATUS_PARAMS status_params;
    memset(&status_params, 0, sizeof(status_params));
    status_params.dwVersion = NVFBC_GET_STATUS_PARAMS_VER;

    status = self->nv_fbc_function_list.nvFBCGetStatus(self->nv_fbc_handle, &status_params);
    if(status != NVFBC_SUCCESS) {
        fprintf(stderr, "gsr error: gsr_capture_nvfbc_start failed: %s\n", self->nv_fbc_function_list.nvFBCGetLastErrorStr(self->nv_fbc_handle));
        goto error_cleanup;
    }

    if(status_params.bCanCreateNow == NVFBC_FALSE) {
        fprintf(stderr, "gsr error: gsr_capture_nvfbc_start failed: it's not possible to create a capture session on this system\n");
        goto error_cleanup;
    }

    assert(gsr_window_get_display_server(self->params.egl->window) == GSR_DISPLAY_SERVER_X11);
    Display *display = gsr_window_get_display(self->params.egl->window);

    self->tracking_width = XWidthOfScreen(DefaultScreenOfDisplay(display));
    self->tracking_height = XHeightOfScreen(DefaultScreenOfDisplay(display));
    self->tracking_type = strcmp(self->params.display_to_capture, "screen") == 0 ? NVFBC_TRACKING_SCREEN : NVFBC_TRACKING_OUTPUT;
    if(self->tracking_type == NVFBC_TRACKING_OUTPUT) {
        if(!status_params.bXRandRAvailable) {
            fprintf(stderr, "gsr error: gsr_capture_nvfbc_start failed: the xrandr extension is not available\n");
            goto error_cleanup;
        }

        if(status_params.bInModeset) {
            fprintf(stderr, "gsr error: gsr_capture_nvfbc_start failed: the x server is in modeset, unable to record\n");
            goto error_cleanup;
        }

        self->output_id = get_output_id_from_display_name(status_params.outputs, status_params.dwOutputNum, self->params.display_to_capture, &self->tracking_width, &self->tracking_height);
        if(self->output_id == 0) {
            fprintf(stderr, "gsr error: gsr_capture_nvfbc_start failed: display '%s' not found\n", self->params.display_to_capture);
            goto error_cleanup;
        }
    }

    if(!self->capture_region) {
        self->width = self->tracking_width;
        self->height = self->tracking_height;
    }

    return 0;

    error_cleanup:
    gsr_capture_nvfbc_destroy_session_and_handle(self);
    return -1;
}

static int gsr_capture_nvfbc_setup_session(gsr_capture_nvfbc *self) {
    NVFBC_CREATE_CAPTURE_SESSION_PARAMS create_capture_params;
    memset(&create_capture_params, 0, sizeof(create_capture_params));
    create_capture_params.dwVersion = NVFBC_CREATE_CAPTURE_SESSION_PARAMS_VER;
    create_capture_params.eCaptureType = NVFBC_CAPTURE_TO_GL;
    create_capture_params.bWithCursor = (!self->params.direct_capture || self->supports_direct_cursor) ? NVFBC_TRUE : NVFBC_FALSE;
    if(!self->params.record_cursor)
        create_capture_params.bWithCursor = false;
    if(self->capture_region)
        create_capture_params.captureBox = (NVFBC_BOX){ self->x, self->y, self->width, self->height };
    create_capture_params.eTrackingType = self->tracking_type;
    create_capture_params.dwSamplingRateMs = (uint32_t)ceilf(1000.0f / (float)self->params.fps);
    create_capture_params.bAllowDirectCapture = self->params.direct_capture ? NVFBC_TRUE : NVFBC_FALSE;
    create_capture_params.bPushModel = self->params.direct_capture ? NVFBC_TRUE : NVFBC_FALSE;
    create_capture_params.bDisableAutoModesetRecovery = true;
    if(self->tracking_type == NVFBC_TRACKING_OUTPUT)
        create_capture_params.dwOutputId = self->output_id;

    NVFBCSTATUS status = self->nv_fbc_function_list.nvFBCCreateCaptureSession(self->nv_fbc_handle, &create_capture_params);
    if(status != NVFBC_SUCCESS) {
        fprintf(stderr, "gsr error: gsr_capture_nvfbc_start failed: %s\n", self->nv_fbc_function_list.nvFBCGetLastErrorStr(self->nv_fbc_handle));
        return -1;
    }
    self->capture_session_created = true;

    memset(&self->setup_params, 0, sizeof(self->setup_params));
    self->setup_params.dwVersion = NVFBC_TOGL_SETUP_PARAMS_VER;
    self->setup_params.eBufferFormat = NVFBC_BUFFER_FORMAT_BGRA;

    status = self->nv_fbc_function_list.nvFBCToGLSetUp(self->nv_fbc_handle, &self->setup_params);
    if(status != NVFBC_SUCCESS) {
        fprintf(stderr, "gsr error: gsr_capture_nvfbc_start failed: %s\n", self->nv_fbc_function_list.nvFBCGetLastErrorStr(self->nv_fbc_handle));
        gsr_capture_nvfbc_destroy_session(self);
        return -1;
    }

    return 0;
}

static void gsr_capture_nvfbc_stop(gsr_capture_nvfbc *self) {
    gsr_capture_nvfbc_destroy_session_and_handle(self);
    if(self->library) {
        dlclose(self->library);
        self->library = NULL;
    }
    if(self->params.display_to_capture) {
        free((void*)self->params.display_to_capture);
        self->params.display_to_capture = NULL;
    }
}

static int gsr_capture_nvfbc_start(gsr_capture *cap, AVCodecContext *video_codec_context, AVFrame *frame) {
    gsr_capture_nvfbc *self = cap->priv;

    if(!gsr_capture_nvfbc_load_library(cap))
        return -1;

    self->x = max_int(self->params.pos.x, 0);
    self->y = max_int(self->params.pos.y, 0);
    self->width = max_int(self->params.size.x, 0);
    self->height = max_int(self->params.size.y, 0);

    self->capture_region = (self->x > 0 || self->y > 0 || self->width > 0 || self->height > 0);

    self->supports_direct_cursor = false;
    int driver_major_version = 0;
    int driver_minor_version = 0;
    if(self->params.direct_capture && get_driver_version(&driver_major_version, &driver_minor_version)) {
        fprintf(stderr, "Info: detected nvidia version: %d.%d\n", driver_major_version, driver_minor_version);

        // TODO:
        if(version_at_least(driver_major_version, driver_minor_version, 515, 57) && version_less_than(driver_major_version, driver_minor_version, 520, 56)) {
            self->params.direct_capture = false;
            fprintf(stderr, "Warning: \"screen-direct\" has temporary been disabled as it causes stuttering with driver versions >= 515.57 and < 520.56. Please update your driver if possible. Capturing \"screen\" instead.\n");
        }

        // TODO:
        // Cursor capture disabled because moving the cursor doesn't update capture rate to monitor hz and instead captures at 10-30 hz
        /*
        if(direct_capture) {
            if(version_at_least(driver_major_version, driver_minor_version, 515, 57))
                self->supports_direct_cursor = true;
            else
                fprintf(stderr, "Info: capturing \"screen-direct\" but driver version appears to be less than 515.57. Disabling capture of cursor. Please update your driver if you want to capture your cursor or record \"screen\" instead.\n");
        }
        */
    }

    if(gsr_capture_nvfbc_setup_handle(self) != 0) {
        goto error_cleanup;
    }

    if(gsr_capture_nvfbc_setup_session(self) != 0) {
        goto error_cleanup;
    }

    if(self->capture_region) {
        video_codec_context->width = FFALIGN(self->width, 2);
        video_codec_context->height = FFALIGN(self->height, 2);
    } else {
        video_codec_context->width = FFALIGN(self->tracking_width, 2);
        video_codec_context->height = FFALIGN(self->tracking_height, 2);
    }

    if(self->params.output_resolution.x == 0 && self->params.output_resolution.y == 0) {
        self->params.output_resolution = (vec2i){video_codec_context->width, video_codec_context->height};
    } else {
        self->params.output_resolution = scale_keep_aspect_ratio((vec2i){video_codec_context->width, video_codec_context->height}, self->params.output_resolution);
        video_codec_context->width = FFALIGN(self->params.output_resolution.x, 2);
        video_codec_context->height = FFALIGN(self->params.output_resolution.y, 2);
    }

    frame->width = video_codec_context->width;
    frame->height = video_codec_context->height;

    /* Disable vsync */
    set_vertical_sync_enabled(self->params.egl, 0);

    return 0;

    error_cleanup:
    gsr_capture_nvfbc_stop(self);
    return -1;
}

static int gsr_capture_nvfbc_capture(gsr_capture *cap, AVFrame *frame, gsr_color_conversion *color_conversion) {
    gsr_capture_nvfbc *self = cap->priv;

    const double nvfbc_recreate_retry_time_seconds = 1.0;
    if(self->nvfbc_needs_recreate) {
        const double now = clock_get_monotonic_seconds();
        if(now - self->nvfbc_dead_start >= nvfbc_recreate_retry_time_seconds) {
            self->nvfbc_dead_start = now;
            gsr_capture_nvfbc_destroy_session_and_handle(self);

            if(gsr_capture_nvfbc_setup_handle(self) != 0) {
                fprintf(stderr, "gsr error: gsr_capture_nvfbc_capture failed to recreate nvfbc handle, trying again in %f second(s)\n", nvfbc_recreate_retry_time_seconds);
                return -1;
            }

            if(gsr_capture_nvfbc_setup_session(self) != 0) {
                fprintf(stderr, "gsr error: gsr_capture_nvfbc_capture failed to recreate nvfbc session, trying again in %f second(s)\n", nvfbc_recreate_retry_time_seconds);
                return -1;
            }

            self->nvfbc_needs_recreate = false;
        } else {
            return 0;
        }
    }

    const vec2i frame_size = (vec2i){self->width, self->height};
    const bool is_scaled = self->params.output_resolution.x > 0 && self->params.output_resolution.y > 0;
    vec2i output_size = is_scaled ? self->params.output_resolution : frame_size;
    output_size = scale_keep_aspect_ratio(frame_size, output_size);

    const vec2i target_pos = { max_int(0, frame->width / 2 - output_size.x / 2), max_int(0, frame->height / 2 - output_size.y / 2) };

    NVFBC_FRAME_GRAB_INFO frame_info;
    memset(&frame_info, 0, sizeof(frame_info));

    NVFBC_TOGL_GRAB_FRAME_PARAMS grab_params;
    memset(&grab_params, 0, sizeof(grab_params));
    grab_params.dwVersion = NVFBC_TOGL_GRAB_FRAME_PARAMS_VER;
    grab_params.dwFlags = NVFBC_TOGL_GRAB_FLAGS_NOWAIT | NVFBC_TOGL_GRAB_FLAGS_FORCE_REFRESH; // TODO: Remove NVFBC_TOGL_GRAB_FLAGS_FORCE_REFRESH
    grab_params.pFrameGrabInfo = &frame_info;
    grab_params.dwTimeoutMs = 0;

    NVFBCSTATUS status = self->nv_fbc_function_list.nvFBCToGLGrabFrame(self->nv_fbc_handle, &grab_params);
    if(status != NVFBC_SUCCESS) {
        fprintf(stderr, "gsr error: gsr_capture_nvfbc_capture failed: %s (%d), recreating session after %f second(s)\n", self->nv_fbc_function_list.nvFBCGetLastErrorStr(self->nv_fbc_handle), status, nvfbc_recreate_retry_time_seconds);
        self->nvfbc_needs_recreate = true;
        self->nvfbc_dead_start = clock_get_monotonic_seconds();
        return 0;
    }

    self->params.egl->glFlush();
    self->params.egl->glFinish();

    gsr_color_conversion_draw(color_conversion, self->setup_params.dwTextures[grab_params.dwTextureIndex],
        target_pos, (vec2i){output_size.x, output_size.y},
        (vec2i){0, 0}, frame_size,
        0.0f, false, GSR_SOURCE_COLOR_BGR);

    self->params.egl->glFlush();
    self->params.egl->glFinish();

    return 0;
}

static void gsr_capture_nvfbc_destroy(gsr_capture *cap, AVCodecContext *video_codec_context) {
    (void)video_codec_context;
    gsr_capture_nvfbc *self = cap->priv;
    gsr_capture_nvfbc_stop(self);
    free(cap->priv);
    free(cap);
}

gsr_capture* gsr_capture_nvfbc_create(const gsr_capture_nvfbc_params *params) {
    if(!params) {
        fprintf(stderr, "gsr error: gsr_capture_nvfbc_create params is NULL\n");
        return NULL;
    }

    if(!params->display_to_capture) {
        fprintf(stderr, "gsr error: gsr_capture_nvfbc_create params.display_to_capture is NULL\n");
        return NULL;
    }

    gsr_capture *cap = calloc(1, sizeof(gsr_capture));
    if(!cap)
        return NULL;

    gsr_capture_nvfbc *cap_nvfbc = calloc(1, sizeof(gsr_capture_nvfbc));
    if(!cap_nvfbc) {
        free(cap);
        return NULL;
    }

    const char *display_to_capture = strdup(params->display_to_capture);
    if(!display_to_capture) {
        free(cap);
        free(cap_nvfbc);
        return NULL;
    }

    cap_nvfbc->params = *params;
    cap_nvfbc->params.display_to_capture = display_to_capture;
    cap_nvfbc->params.fps = max_int(cap_nvfbc->params.fps, 1);

    *cap = (gsr_capture) {
        .start = gsr_capture_nvfbc_start,
        .tick = NULL,
        .should_stop = NULL,
        .capture = gsr_capture_nvfbc_capture,
        .uses_external_image = NULL,
        .destroy = gsr_capture_nvfbc_destroy,
        .priv = cap_nvfbc
    };

    return cap;
}
