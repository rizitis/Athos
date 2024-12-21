#ifndef GSR_UTILS_H
#define GSR_UTILS_H

#include "vec2.h"
#include "../include/egl.h"
#include "../include/defs.h"
#include <stdbool.h>
#include <stdint.h>

#define CONNECTOR_TYPE_COUNTS 32

typedef struct AVCodecContext AVCodecContext;
typedef struct AVFrame AVFrame;

typedef struct {
    const char *name;
    int name_len;
    vec2i pos;
    vec2i size;
    uint32_t connector_id; /* Only on x11 and drm */
    gsr_monitor_rotation rotation; /* Only on x11 and wayland */
    uint32_t monitor_identifier; /* On x11 this is the crtc id */
} gsr_monitor;

typedef struct {
    const char *name;
    int name_len;
    gsr_monitor *monitor;
    bool found_monitor;
} get_monitor_by_name_userdata;

typedef struct {
    int type;
    int count;
    int count_active;
} drm_connector_type_count;

double clock_get_monotonic_seconds(void);
bool generate_random_characters(char *buffer, int buffer_size, const char *alphabet, size_t alphabet_size);
bool generate_random_characters_standard_alphabet(char *buffer, int buffer_size);

typedef void (*active_monitor_callback)(const gsr_monitor *monitor, void *userdata);
void for_each_active_monitor_output_x11_not_cached(Display *display, active_monitor_callback callback, void *userdata);
void for_each_active_monitor_output(const gsr_window *window, const char *card_path, gsr_connection_type connection_type, active_monitor_callback callback, void *userdata);
bool get_monitor_by_name(const gsr_egl *egl, gsr_connection_type connection_type, const char *name, gsr_monitor *monitor);
gsr_monitor_rotation drm_monitor_get_display_server_rotation(const gsr_window *window, const gsr_monitor *monitor);

int get_connector_type_by_name(const char *name);
drm_connector_type_count* drm_connector_types_get_index(drm_connector_type_count *type_counts, int *num_type_counts, int connector_type);
uint32_t monitor_identifier_from_type_and_count(int monitor_type_index, int monitor_type_count);

bool gl_get_gpu_info(gsr_egl *egl, gsr_gpu_info *info);
bool gl_driver_version_greater_than(const gsr_egl *egl, int major, int minor, int patch);

bool try_card_has_valid_plane(const char *card_path);
/* |output| should be at least 128 bytes in size */
bool gsr_get_valid_card_path(gsr_egl *egl, char *output, bool is_monitor_capture);
/* |render_path| should be at least 128 bytes in size */
bool gsr_card_path_get_render_path(const char *card_path, char *render_path);

int create_directory_recursive(char *path);

/* |img_attr| needs to be at least 44 in size */
void setup_dma_buf_attrs(intptr_t *img_attr, uint32_t format, uint32_t width, uint32_t height, const int *fds, const uint32_t *offsets, const uint32_t *pitches, const uint64_t *modifiers, int num_planes, bool use_modifier);
bool video_codec_context_is_vaapi(AVCodecContext *video_codec_context);
bool vaapi_copy_drm_planes_to_video_surface(AVCodecContext *video_codec_context, AVFrame *video_frame, vec2i source_pos, vec2i source_size, vec2i dest_pos, vec2i dest_size, uint32_t format, vec2i size, const int *fds, const uint32_t *offsets, const uint32_t *pitches, const uint64_t *modifiers, int num_planes);
bool vaapi_copy_egl_image_to_video_surface(gsr_egl *egl, EGLImage image, vec2i source_pos, vec2i source_size, vec2i dest_pos, vec2i dest_size, AVCodecContext *video_codec_context, AVFrame *video_frame);

vec2i scale_keep_aspect_ratio(vec2i from, vec2i to);

#endif /* GSR_UTILS_H */
