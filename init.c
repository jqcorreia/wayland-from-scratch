#include "init.h"
#include "wayland-client-core.h"

void registry_handle_global(void* data, struct wl_registry* registry,
    uint32_t name, const char* interface,
    uint32_t version)
{
    printf("interface: '%s', version: %d, name: %d\n", interface, version, name);
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        app_state* state = data;
        state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 6);
    }
    if (strcmp(interface, wl_shm_interface.name) == 0) {
        app_state* state = data;
        state->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        app_state* state = data;
        state->xdg_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
    if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        app_state* state = data;
        state->layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
    }
}

void registry_handle_global_remove(void* data, struct wl_registry* registry,
    uint32_t name)
{
    /* printf("Removed, name: %d\n", name); */
}

struct wl_display* init(app_state* state)
{
    struct wl_display* display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display.\n");
        return NULL;
    }

    struct wl_registry_listener listener = {
        .global = registry_handle_global,
        .global_remove = registry_handle_global_remove,
    };

    fprintf(stderr, "Connection established!\n");
    struct wl_registry* registry = wl_display_get_registry(display);

    wl_registry_add_listener(registry, &listener, state);
    wl_display_roundtrip(display);

    return display;
}
