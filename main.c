#include "shm.h"
#include "wayland-client-protocol.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client.h>

typedef struct app_state {
    struct wl_compositor* compositor;
    struct wl_shm* shm;
} app_state;

void registry_handle_global(void* data, struct wl_registry* registry,
    uint32_t name, const char* interface, uint32_t version)
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
}

void registry_handle_global_remove(void* data, struct wl_registry* registry,
    uint32_t name)
{
    /* printf("Removed, name: %d\n", name); */
}

int main(int argc, char* argv[])
{
    app_state state;

    struct wl_display* display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "Failed to connect to Wayland display.\n");
        return 1;
    }

    struct wl_registry_listener listener = {
        .global = registry_handle_global,
        .global_remove = registry_handle_global_remove,
    };

    fprintf(stderr, "Connection established!\n");
    struct wl_registry* registry = wl_display_get_registry(display);

    wl_registry_add_listener(registry, &listener, &state);

    wl_display_roundtrip(display);

    // Only after first round trip state.compositor is set
    struct wl_surface* surface = wl_compositor_create_surface(state.compositor);

    const int width = 1920, height = 1080;
    const int stride = width * 4;
    const int shm_pool_size = height * stride * 2;

    int fd = allocate_shm_file(shm_pool_size);
    uint8_t* pool_data = mmap(NULL, shm_pool_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct wl_shm_pool* pool = wl_shm_create_pool(state.shm, fd, shm_pool_size);
    int index = 0;
    int offset = index * height * stride;

    struct wl_buffer* buffer = wl_shm_pool_create_buffer(pool, offset,
        width, height, stride, WL_SHM_FORMAT_XRGB8888);

    // This should work by referencing uint8 directly i believe.
    uint32_t* pixels = (uint32_t*)&pool_data[offset];
    memset(pixels, 0, width * height * 4);

    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0, UINT32_MAX, UINT32_MAX);
    wl_surface_commit(surface);
    // Loop
    while (wl_display_dispatch(display) != -1) {
    }

    // Disconnect
    wl_display_disconnect(display);
    return 0;
}
