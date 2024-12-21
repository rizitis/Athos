#include "../include/pipewire_audio.h"

#include <pipewire/pipewire.h>

static void on_core_info_cb(void *user_data, const struct pw_core_info *info) {
    gsr_pipewire_audio *self = user_data;
    //fprintf(stderr, "server name: %s\n", info->name);
}

static void on_core_error_cb(void *user_data, uint32_t id, int seq, int res, const char *message) {
    gsr_pipewire_audio *self = user_data;
    //fprintf(stderr, "gsr error: pipewire: error id:%u seq:%d res:%d: %s\n", id, seq, res, message);
    pw_thread_loop_signal(self->thread_loop, false);
}

static void on_core_done_cb(void *user_data, uint32_t id, int seq) {
    gsr_pipewire_audio *self = user_data;
    if(id == PW_ID_CORE && self->server_version_sync == seq)
        pw_thread_loop_signal(self->thread_loop, false);
}

static const struct pw_core_events core_events = {
    PW_VERSION_CORE_EVENTS,
    .info = on_core_info_cb,
    .done = on_core_done_cb,
    .error = on_core_error_cb,
};

static gsr_pipewire_audio_node* gsr_pipewire_audio_get_node_by_name_case_insensitive(gsr_pipewire_audio *self, const char *node_name, gsr_pipewire_audio_node_type node_type) {
    for(int i = 0; i < self->num_stream_nodes; ++i) {
        const gsr_pipewire_audio_node *node = &self->stream_nodes[i];
        if(node->type == node_type && strcasecmp(node->name, node_name) == 0)
            return &self->stream_nodes[i];
    }
    return NULL;
}

static gsr_pipewire_audio_port* gsr_pipewire_audio_get_node_port_by_name(gsr_pipewire_audio *self, uint32_t node_id, const char *port_name) {
    for(int i = 0; i < self->num_ports; ++i) {
        if(self->ports[i].node_id == node_id && strcmp(self->ports[i].name, port_name) == 0)
            return &self->ports[i];
    }
    return NULL;
}

static bool requested_link_matches_name_case_insensitive(const gsr_pipewire_audio_requested_link *requested_link, const char *name) {
    for(int i = 0; i < requested_link->num_output_names; ++i) {
        if(strcasecmp(requested_link->output_names[i], name) == 0)
            return true;
    }
    return false;
}

static void gsr_pipewire_audio_create_link(gsr_pipewire_audio *self, const gsr_pipewire_audio_requested_link *requested_link) {
    const gsr_pipewire_audio_node_type requested_link_node_type = requested_link->input_type == GSR_PIPEWIRE_AUDIO_LINK_INPUT_TYPE_STREAM ? GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_INPUT : GSR_PIPEWIRE_AUDIO_NODE_TYPE_SINK_OR_SOURCE;
    const gsr_pipewire_audio_node *stream_input_node = gsr_pipewire_audio_get_node_by_name_case_insensitive(self, requested_link->input_name, requested_link_node_type);
    if(!stream_input_node)
        return;

    const gsr_pipewire_audio_port *input_fl_port = NULL;
    const gsr_pipewire_audio_port *input_fr_port = NULL;

    switch(requested_link->input_type) {
        case GSR_PIPEWIRE_AUDIO_LINK_INPUT_TYPE_STREAM: {
            input_fl_port = gsr_pipewire_audio_get_node_port_by_name(self, stream_input_node->id, "input_FL");
            input_fr_port = gsr_pipewire_audio_get_node_port_by_name(self, stream_input_node->id, "input_FR");
            break;
        }
        case GSR_PIPEWIRE_AUDIO_LINK_INPUT_TYPE_SINK: {
            input_fl_port = gsr_pipewire_audio_get_node_port_by_name(self, stream_input_node->id, "playback_FL");
            input_fr_port = gsr_pipewire_audio_get_node_port_by_name(self, stream_input_node->id, "playback_FR");
            break;
        }
    }

    if(!input_fl_port || !input_fr_port)
        return;

    for(int i = 0; i < self->num_stream_nodes; ++i) {
        const gsr_pipewire_audio_node *output_node = &self->stream_nodes[i];
        if(output_node->type != requested_link->output_type)
            continue;

        const bool requested_link_matches_app = requested_link_matches_name_case_insensitive(requested_link, output_node->name);
        if(requested_link->inverted) {
            if(requested_link_matches_app)
                continue;
        } else {
            if(!requested_link_matches_app)
                continue;
        }

        const gsr_pipewire_audio_port *output_fl_port = NULL;
        const gsr_pipewire_audio_port *output_fr_port = NULL;

        switch(requested_link->output_type) {
            case GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_OUTPUT:
                output_fl_port = gsr_pipewire_audio_get_node_port_by_name(self, output_node->id, "output_FL");
                output_fr_port = gsr_pipewire_audio_get_node_port_by_name(self, output_node->id, "output_FR");
                break;
            case GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_INPUT:
                output_fl_port = gsr_pipewire_audio_get_node_port_by_name(self, output_node->id, "monitor_FL");
                output_fr_port = gsr_pipewire_audio_get_node_port_by_name(self, output_node->id, "monitor_FR");
                break;
            case GSR_PIPEWIRE_AUDIO_NODE_TYPE_SINK_OR_SOURCE: {
                output_fl_port = gsr_pipewire_audio_get_node_port_by_name(self, output_node->id, "monitor_FL");
                output_fr_port = gsr_pipewire_audio_get_node_port_by_name(self, output_node->id, "monitor_FR");
                if(!output_fl_port || !output_fr_port) {
                    output_fl_port = gsr_pipewire_audio_get_node_port_by_name(self, output_node->id, "capture_FL");
                    output_fr_port = gsr_pipewire_audio_get_node_port_by_name(self, output_node->id, "capture_FR");
                }
                break;
            }
        }

        if(!output_fl_port || !output_fr_port)
            continue;

        // TODO: Detect if link already exists before so we dont create these proxies when not needed

        //fprintf(stderr, "linking!\n");
        // TODO: error check and cleanup
        {
            struct pw_properties *props = pw_properties_new(NULL, NULL);
            pw_properties_setf(props, PW_KEY_LINK_OUTPUT_PORT, "%u", output_fl_port->id);
            pw_properties_setf(props, PW_KEY_LINK_INPUT_PORT, "%u", input_fl_port->id);
            // TODO: Clean this up when removing node
            struct pw_proxy *proxy = pw_core_create_object(self->core, "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict, 0);
            //self->server_version_sync = pw_core_sync(self->core, PW_ID_CORE, self->server_version_sync);
            pw_properties_free(props);
        }

        {
            struct pw_properties *props = pw_properties_new(NULL, NULL);
            pw_properties_setf(props, PW_KEY_LINK_OUTPUT_PORT, "%u", output_fr_port->id);
            pw_properties_setf(props, PW_KEY_LINK_INPUT_PORT, "%u", input_fr_port->id);
            // TODO: Clean this up when removing node
            struct pw_proxy *proxy = pw_core_create_object(self->core, "link-factory", PW_TYPE_INTERFACE_Link, PW_VERSION_LINK, &props->dict, 0);
            //self->server_version_sync = pw_core_sync(self->core, PW_ID_CORE, self->server_version_sync);
            pw_properties_free(props);
        }
    }
}

static void gsr_pipewire_audio_create_links(gsr_pipewire_audio *self) {
    for(int j = 0; j < self->num_requested_links; ++j) {
        gsr_pipewire_audio_create_link(self, &self->requested_links[j]);
    }
}

static void registry_event_global(void *data, uint32_t id, uint32_t permissions,
                  const char *type, uint32_t version,
                  const struct spa_dict *props)
{
    //fprintf(stderr, "add: id: %d, type: %s\n", (int)id, type);
    if (props == NULL)
        return;

    //pw_properties_new_dict(props);

    gsr_pipewire_audio *self = (gsr_pipewire_audio*)data;
    if(strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
        const char *node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
        const char *media_class = spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
        //fprintf(stderr, "  node name: %s, media class: %s\n", node_name, media_class);
        const bool is_stream_output = media_class && strcmp(media_class, "Stream/Output/Audio") == 0;
        const bool is_stream_input = media_class && strcmp(media_class, "Stream/Input/Audio") == 0;
        const bool is_sink = media_class && strcmp(media_class, "Audio/Sink") == 0;
        const bool is_source = media_class && strcmp(media_class, "Audio/Source") == 0;
        if(self->num_stream_nodes < GSR_PIPEWIRE_AUDIO_MAX_STREAM_NODES && node_name && (is_stream_output || is_stream_input || is_sink || is_source)) {
            //const char *application_binary = spa_dict_lookup(props, PW_KEY_APP_PROCESS_BINARY);
            //const char *application_name = spa_dict_lookup(props, PW_KEY_APP_NAME);
            //fprintf(stderr, "  node name: %s, app binary: %s, app name: %s\n", node_name, application_binary, application_name);

            char *node_name_copy = strdup(node_name);
            if(node_name_copy) {
                self->stream_nodes[self->num_stream_nodes].id = id;
                self->stream_nodes[self->num_stream_nodes].name = node_name_copy;
                if(is_stream_output)
                    self->stream_nodes[self->num_stream_nodes].type = GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_OUTPUT;
                else if(is_stream_input)
                    self->stream_nodes[self->num_stream_nodes].type = GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_INPUT;
                else if(is_sink || is_source)
                    self->stream_nodes[self->num_stream_nodes].type = GSR_PIPEWIRE_AUDIO_NODE_TYPE_SINK_OR_SOURCE;
                ++self->num_stream_nodes;

                gsr_pipewire_audio_create_links(self);
            }
        } else if(self->num_stream_nodes >= GSR_PIPEWIRE_AUDIO_MAX_STREAM_NODES) {
            fprintf(stderr, "gsr error: reached the maximum amount of audio stream nodes\n");
        }
    } else if(strcmp(type, PW_TYPE_INTERFACE_Port) == 0) {
        const char *port_name = spa_dict_lookup(props, PW_KEY_PORT_NAME);

        const char *port_direction = spa_dict_lookup(props, PW_KEY_PORT_DIRECTION);
        gsr_pipewire_audio_port_direction direction = -1;
        if(port_direction && strcmp(port_direction, "in") == 0)
            direction = GSR_PIPEWIRE_AUDIO_PORT_DIRECTION_INPUT;
        else if(port_direction && strcmp(port_direction, "out") == 0)
            direction = GSR_PIPEWIRE_AUDIO_PORT_DIRECTION_OUTPUT;

        const char *node_id = spa_dict_lookup(props, PW_KEY_NODE_ID);
        const int node_id_num = node_id ? atoi(node_id) : 0;

        if(self->num_ports < GSR_PIPEWIRE_AUDIO_MAX_PORTS && port_name && direction >= 0 && node_id_num > 0) {
            //fprintf(stderr, "  port name: %s, node id: %d, direction: %s\n", port_name, node_id_num, port_direction);
            char *port_name_copy = strdup(port_name);
            if(port_name_copy) {
                self->ports[self->num_ports].id = id;
                self->ports[self->num_ports].node_id = node_id_num;
                self->ports[self->num_ports].direction = direction;
                self->ports[self->num_ports].name = port_name_copy;
                ++self->num_ports;

                gsr_pipewire_audio_create_links(self);
            }
        } else if(self->num_ports >= GSR_PIPEWIRE_AUDIO_MAX_PORTS) {
            fprintf(stderr, "gsr error: reached the maximum amount of audio ports\n");
        }
    }
}

static bool gsr_pipewire_audio_remove_node_by_id(gsr_pipewire_audio *self, uint32_t node_id) {
    for(int i = 0; i < self->num_stream_nodes; ++i) {
        if(self->stream_nodes[i].id != node_id)
            continue;

        free(self->stream_nodes[i].name);
        for(int j = i + 1; j < self->num_stream_nodes; ++j) {
            self->stream_nodes[j - 1] = self->stream_nodes[j];
        }
        --self->num_stream_nodes;
        return true;
    }
    return false;
}

static bool gsr_pipewire_audio_remove_port_by_id(gsr_pipewire_audio *self, uint32_t port_id) {
    for(int i = 0; i < self->num_ports; ++i) {
        if(self->ports[i].id != port_id)
            continue;

        free(self->ports[i].name);
        for(int j = i + 1; j < self->num_ports; ++j) {
            self->ports[j - 1] = self->ports[j];
        }
        --self->num_ports;
        return true;
    }
    return false;
}

static void registry_event_global_remove(void *data, uint32_t id) {
    //fprintf(stderr, "remove: %d\n", (int)id);
    gsr_pipewire_audio *self = (gsr_pipewire_audio*)data;
    if(gsr_pipewire_audio_remove_node_by_id(self, id)) {
        //fprintf(stderr, "removed node\n");
        return;
    }

    if(gsr_pipewire_audio_remove_port_by_id(self, id)) {
        //fprintf(stderr, "removed port\n");
        return;
    }
}

static const struct pw_registry_events registry_events = {
    PW_VERSION_REGISTRY_EVENTS,
    .global = registry_event_global,
    .global_remove = registry_event_global_remove,
};

bool gsr_pipewire_audio_init(gsr_pipewire_audio *self) {
    memset(self, 0, sizeof(*self));

    pw_init(NULL, NULL);
    
    self->thread_loop = pw_thread_loop_new("gsr screen capture", NULL);
    if(!self->thread_loop) {
        fprintf(stderr, "gsr error: gsr_pipewire_audio_init: failed to create pipewire thread\n");
        gsr_pipewire_audio_deinit(self);
        return false;
    }

    self->context = pw_context_new(pw_thread_loop_get_loop(self->thread_loop), NULL, 0);
    if(!self->context) {
        fprintf(stderr, "gsr error: gsr_pipewire_audio_init: failed to create pipewire context\n");
        gsr_pipewire_audio_deinit(self);
        return false;
    }

    if(pw_thread_loop_start(self->thread_loop) < 0) {
        fprintf(stderr, "gsr error: gsr_pipewire_audio_init: failed to start thread\n");
        gsr_pipewire_audio_deinit(self);
        return false;
    }

    pw_thread_loop_lock(self->thread_loop);

    self->core = pw_context_connect(self->context, pw_properties_new(PW_KEY_REMOTE_NAME, NULL, NULL), 0);
    if(!self->core) {
        pw_thread_loop_unlock(self->thread_loop);
        gsr_pipewire_audio_deinit(self);
        return false;
    }

    // TODO: Error check
    pw_core_add_listener(self->core, &self->core_listener, &core_events, self);

    self->registry = pw_core_get_registry(self->core, PW_VERSION_REGISTRY, 0);
    pw_registry_add_listener(self->registry, &self->registry_listener, &registry_events, self);

    self->server_version_sync = pw_core_sync(self->core, PW_ID_CORE, 0);
    pw_thread_loop_wait(self->thread_loop);
    pw_thread_loop_unlock(self->thread_loop);
    return true;
}

void gsr_pipewire_audio_deinit(gsr_pipewire_audio *self) {
    if(self->thread_loop) {
        //pw_thread_loop_wait(self->thread_loop);
        pw_thread_loop_stop(self->thread_loop);
    }

    for(int i = 0; i < self->num_virtual_sink_proxies; ++i) {
        if(self->virtual_sink_proxies[i]) {
            pw_proxy_destroy(self->virtual_sink_proxies[i]);
            self->virtual_sink_proxies[i] = NULL;
        }
    }
    self->num_virtual_sink_proxies = 0;

    if(self->core) {
        pw_core_disconnect(self->core);
        self->core = NULL;
    }

    if(self->context) {
        pw_context_destroy(self->context);
        self->context = NULL;
    }

    if(self->thread_loop) {
        pw_thread_loop_destroy(self->thread_loop);
        self->thread_loop = NULL;
    }

    for(int i = 0; i < self->num_stream_nodes; ++i) {
        free(self->stream_nodes[i].name);
    }
    self->num_stream_nodes = 0;

    for(int i = 0; i < self->num_ports; ++i) {
        free(self->ports[i].name);
    }
    self->num_ports = 0;

    for(int i = 0; i < self->num_requested_links; ++i) {
        for(int j = 0; j < self->requested_links[i].num_output_names; ++j) {
            free(self->requested_links[i].output_names[j]);
        }
        free(self->requested_links[i].output_names);
        free(self->requested_links[i].input_name);
    }
    self->num_requested_links = 0;

#if PW_CHECK_VERSION(0, 3, 49)
    pw_deinit();
#endif
}

static struct pw_properties* gsr_pipewire_create_null_audio_sink(const char *name) {
    char props_str[512];
    snprintf(props_str, sizeof(props_str), "{ factory.name=support.null-audio-sink node.name=\"%s\" media.class=Audio/Sink object.linger=false audio.position=[FL FR] monitor.channel-volumes=true monitor.passthrough=true adjust_time=0 slaves=\"\" }", name);
    struct pw_properties *props = pw_properties_new_string(props_str);
    if(!props) {
        fprintf(stderr, "gsr error: gsr_pipewire_create_null_audio_sink: failed to create virtual sink properties\n");
        return NULL;
    }
    return props;
}

bool gsr_pipewire_audio_create_virtual_sink(gsr_pipewire_audio *self, const char *name) {
    if(self->num_virtual_sink_proxies == GSR_PIPEWIRE_AUDIO_MAX_VIRTUAL_SINKS) {
        fprintf(stderr, "gsr error: gsr_pipewire_audio_create_virtual_sink: reached max number of virtual sinks\n");
        return false;
    }

    pw_thread_loop_lock(self->thread_loop);

    struct pw_properties *virtual_sink_props = gsr_pipewire_create_null_audio_sink(name);
    if(!virtual_sink_props) {
        pw_thread_loop_unlock(self->thread_loop);
        return false;
    }

    struct pw_proxy *virtual_sink_proxy = pw_core_create_object(self->core, "adapter", PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, &virtual_sink_props->dict, 0);
    // TODO:
    // If these are done then the above needs sizeof(*self) as the last argument
    //pw_proxy_add_object_listener(virtual_sink_proxy, &pd->object_listener, &node_events, self);
	//pw_proxy_add_listener(virtual_sink_proxy, &pd->proxy_listener, &proxy_events, self);
    // TODO: proxy
    pw_properties_free(virtual_sink_props);
    if(!virtual_sink_proxy) {
        fprintf(stderr, "gsr error: gsr_pipewire_audio_create_virtual_sink: failed to create virtual sink\n");
        pw_thread_loop_unlock(self->thread_loop);
        return false;
    }

    self->server_version_sync = pw_core_sync(self->core, PW_ID_CORE, self->server_version_sync);
    pw_thread_loop_wait(self->thread_loop);
    pw_thread_loop_unlock(self->thread_loop);

    self->virtual_sink_proxies[self->num_virtual_sink_proxies] = virtual_sink_proxy;
    ++self->num_virtual_sink_proxies;

    return true;
}

static bool string_remove_suffix(char *str, const char *suffix) {
    int str_len = strlen(str);
    int suffix_len = strlen(suffix);
    if(str_len >= suffix_len && memcmp(str + str_len - suffix_len, suffix, suffix_len) == 0) {
        str[str_len - suffix_len] = '\0';
        return true;
    } else {
        return false;
    }
}

static bool gsr_pipewire_audio_add_link_from_apps_to_output(gsr_pipewire_audio *self, const char **output_names, int num_output_names, const char *input_name, gsr_pipewire_audio_node_type output_type, gsr_pipewire_audio_link_input_type input_type, bool inverted) {
    if(self->num_requested_links >= GSR_PIPEWIRE_AUDIO_MAX_REQUESTED_LINKS) {
        fprintf(stderr, "gsr error: reached the maximum amount of audio links\n");
        return false;
    }
    
    char **output_names_copy = calloc(num_output_names, sizeof(char*));
    if(!output_names_copy)
        return false;

    char *input_name_copy = strdup(input_name);
    if(!input_name_copy)
        goto error;

    if(input_type == GSR_PIPEWIRE_AUDIO_LINK_INPUT_TYPE_SINK)
        string_remove_suffix(input_name_copy, ".monitor");

    for(int i = 0; i < num_output_names; ++i) {
        output_names_copy[i] = strdup(output_names[i]);
        if(!output_names_copy[i])
            goto error;

        if(output_type == GSR_PIPEWIRE_AUDIO_NODE_TYPE_SINK_OR_SOURCE)
            string_remove_suffix(output_names_copy[i], ".monitor");
    }

    pw_thread_loop_lock(self->thread_loop);
    self->requested_links[self->num_requested_links].output_names = output_names_copy;
    self->requested_links[self->num_requested_links].num_output_names = num_output_names;
    self->requested_links[self->num_requested_links].input_name = input_name_copy;
    self->requested_links[self->num_requested_links].output_type = output_type;
    self->requested_links[self->num_requested_links].input_type = input_type;
    self->requested_links[self->num_requested_links].inverted = inverted;
    ++self->num_requested_links;
    gsr_pipewire_audio_create_link(self, &self->requested_links[self->num_requested_links - 1]);
    pw_thread_loop_unlock(self->thread_loop);

    return true;

    error:
    free(input_name_copy);
    for(int i = 0; i < num_output_names; ++i) {
        free(output_names_copy[i]);
    }
    free(output_names_copy);
    return false;
}

bool gsr_pipewire_audio_add_link_from_apps_to_stream(gsr_pipewire_audio *self, const char **app_names, int num_app_names, const char *stream_name_input) {
    return gsr_pipewire_audio_add_link_from_apps_to_output(self, app_names, num_app_names, stream_name_input, GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_OUTPUT, GSR_PIPEWIRE_AUDIO_LINK_INPUT_TYPE_STREAM, false);
}

bool gsr_pipewire_audio_add_link_from_apps_to_stream_inverted(gsr_pipewire_audio *self, const char **app_names, int num_app_names, const char *stream_name_input) {
    return gsr_pipewire_audio_add_link_from_apps_to_output(self, app_names, num_app_names, stream_name_input, GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_OUTPUT, GSR_PIPEWIRE_AUDIO_LINK_INPUT_TYPE_STREAM, true);
}

bool gsr_pipewire_audio_add_link_from_apps_to_sink(gsr_pipewire_audio *self, const char **app_names, int num_app_names, const char *sink_name_input) {
    return gsr_pipewire_audio_add_link_from_apps_to_output(self, app_names, num_app_names, sink_name_input, GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_OUTPUT, GSR_PIPEWIRE_AUDIO_LINK_INPUT_TYPE_SINK, false);
}

bool gsr_pipewire_audio_add_link_from_apps_to_sink_inverted(gsr_pipewire_audio *self, const char **app_names, int num_app_names, const char *sink_name_input) {
    return gsr_pipewire_audio_add_link_from_apps_to_output(self, app_names, num_app_names, sink_name_input, GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_OUTPUT, GSR_PIPEWIRE_AUDIO_LINK_INPUT_TYPE_SINK, true);
}

bool gsr_pipewire_audio_add_link_from_sources_to_sink(gsr_pipewire_audio *self, const char **source_names, int num_source_names, const char *sink_name_input) {
    return gsr_pipewire_audio_add_link_from_apps_to_output(self, source_names, num_source_names, sink_name_input, GSR_PIPEWIRE_AUDIO_NODE_TYPE_SINK_OR_SOURCE, GSR_PIPEWIRE_AUDIO_LINK_INPUT_TYPE_SINK, false);
}

void gsr_pipewire_audio_for_each_app(gsr_pipewire_audio *self, gsr_pipewire_audio_app_query_callback callback, void *userdata) {
    pw_thread_loop_lock(self->thread_loop);
    for(int i = 0; i < self->num_stream_nodes; ++i) {
        const gsr_pipewire_audio_node *node = &self->stream_nodes[i];
        if(node->type != GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_OUTPUT)
            continue;

        bool duplicate_app = false;
        for(int j = i - 1; j >= 0; --j) {
            const gsr_pipewire_audio_node *prev_node = &self->stream_nodes[j];
            if(prev_node->type != GSR_PIPEWIRE_AUDIO_NODE_TYPE_STREAM_OUTPUT)
                continue;

            if(strcasecmp(node->name, prev_node->name) == 0) {
                duplicate_app = true;
                break;
            }
        }

        if(duplicate_app)
            continue;

        if(!callback(node->name, userdata))
            break;
    }
    pw_thread_loop_unlock(self->thread_loop);
}
