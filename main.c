#include "shm.h"
#include "wayland-client-protocol.h"
#include "wlr-layer-shell-unstable-v1.h"
#include <schrift.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>
#include <wayland-client.h>
#include <xdg-shell.h>

typedef struct app_state {
    struct wl_compositor* compositor;
    struct wl_shm* shm;
    struct xdg_wm_base* xdg_base;
    struct wl_surface* surface;
    struct zwlr_layer_shell_v1* layer_shell;
    SFT sft;
} app_state;

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
typedef struct pixel {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char alpha;
} pixel;

static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface,
    uint32_t serial)
{
    struct app_state* state = data;
    xdg_surface_ack_configure(xdg_surface, serial);

    const int width = 800, height = 600;
    const int stride = width * 4;
    const int shm_pool_size = height * stride * 2;

    int fd = allocate_shm_file(shm_pool_size);
    uint8_t* pool_data = mmap(NULL, shm_pool_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct wl_shm_pool* pool = wl_shm_create_pool(state->shm, fd, shm_pool_size);

    int index = 0;
    int offset = index * height * stride;

    struct wl_buffer* buffer = wl_shm_pool_create_buffer(
        pool, offset, width, height, stride, WL_SHM_FORMAT_ARGB8888);

    // This should work by referencing uint8 directly i believe.
    /* uint32_t* pixels = (uint32_t*)&pool_data[offset]; */

    SFT_Glyph glyph;
    SFT_GMetrics metrics;
    sft_lookup(&state->sft, 'g', &glyph);
    /* printf("Glyph ID: %ld\n", glyph); */

    sft_gmetrics(&state->sft, glyph, &metrics);
    SFT_Image img = {
        .width = (metrics.minWidth + 3) & ~3,
        .height = metrics.minHeight,
    };

    char gp[img.width * img.height];
    img.pixels = gp;

    sft_render(&state->sft, glyph, img);

    int text_x = 100;
    int text_y = 100;

    /*memset(pixels, 0, width * height * sizeof(uint32_t));*/

    // Clear the surface (can't use the memset way for the life of me)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixel* px = (pixel*)(pool_data + y * stride + x * 4);
            px->alpha = 255;
            px->red = 0;
            px->green = 0;
            px->blue = 0;
        }
    }

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            unsigned char val = gp[y * img.width + x];
            /* printf("%d\n", val); */
            if (val != 0) {
                pixel* px = (pixel*)(pool_data + (text_y + y) * stride + (text_x + x) * 4);

                unsigned char alpha = (val + 128) / 255;

                px->red = (255 * val + px->red * px->alpha * (1 - alpha)) / px->alpha;
                px->green = (255 * val + px->green * px->alpha * (1 - alpha)) / px->alpha;
                px->blue = (255 * val + px->blue * px->alpha * (1 - alpha)) / px->alpha;
                px->alpha = val + px->alpha * (1 - alpha);
            }
        }
    }

    text_x += 100;
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            unsigned char val = gp[y * img.width + x];
            if (val != 0) {
                if (y > img.height / 2) {
                    val = 100;

                } else {
                    val = 254;
                }

                pixel* px = (pixel*)(pool_data + (text_y + y) * stride + (text_x + x) * 4);
                px->alpha = val;
                px->red = 100;
                px->green = 100;
                px->blue = 100;
            }
        }
    }

    wl_surface_attach(state->surface, buffer, 0, 0);
    wl_surface_damage(state->surface, 0, 0, UINT32_MAX, UINT32_MAX);
    wl_surface_commit(state->surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

SFT load_font(char* name)
{
    char* fclist[] = { "/run/current-system/sw/bin/fc-list" };

    char* res = qx(fclist);
    printf("len %ld\n", strlen(res));

    printf("%s\n", res);
    int s = 2;
    SFT sft = {
        .xScale = 64 * s,
        .yScale = 64 * s,
        .flags = SFT_DOWNWARD_Y,
    };
    sft.font = sft_loadfile(name);
    return sft;
}

int main(int argc, char* argv[])
{
    app_state state;

    state.sft = load_font("/nix/store/z2lkf8q9ii0h46h782qy0i5dp18im047-nerdfonts-3.2.1/share/fonts/truetype/NerdFonts/JetBrainsMonoNerdFontPropo-Regular.ttf");

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
    state.surface = wl_compositor_create_surface(state.compositor);
    struct xdg_surface* xdg_surface = xdg_wm_base_get_xdg_surface(state.xdg_base, state.surface);
    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, &state);

    struct xdg_toplevel* toplevel = xdg_surface_get_toplevel(xdg_surface);

    xdg_toplevel_set_title(toplevel, "Wayland test");
    wl_surface_commit(state.surface);

    // Loop
    while (wl_display_dispatch(display) != -1) {
    }

    // Disconnect
    wl_display_disconnect(display);
    return 0;
}
