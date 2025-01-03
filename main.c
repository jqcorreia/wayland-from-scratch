#include "gen/wlr-layer-shell-unstable-v1.h"
#include "gen/xdg-shell.h"
#include "init.h"
#include "schrift.h"
#include "shm.h"
#include "types.h"
#include "utils.h"
#include "wayland-client-core.h"
#include "wayland-client-protocol.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wayland-client.h>

void draw_text(uint8_t* data, char* text, const SFT* font, uint32_t x, uint32_t y)
{
    int text_x = x;
    int text_y = y;
    int stride = 320 * 4; // Hardcoded for now

    for (int i = 0; i < strlen(text); i++) {
        SFT_Glyph glyph;
        SFT_GMetrics metrics;
        sft_lookup(font, text[i], &glyph);
        /* printf("Glyph ID: %ld\n", glyph); */

        sft_gmetrics(font, glyph, &metrics);
        text_x += metrics.advanceWidth;

        SFT_Image img = {
            .width = metrics.minWidth,
            .height = metrics.minHeight,
        };

        char gp[img.width * img.height];
        img.pixels = gp;

        sft_render(font, glyph, img);

        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                unsigned char val = gp[y * img.width + x];
                /* printf("%d\n", val); */
                if (val != 0) {
                    pixel* px = (pixel*)(data + (text_y + y + metrics.yOffset) * stride + (text_x + x) * 4);

                    unsigned char alpha = (val + 128) / 255;

                    /* px->red = (0 * val + px->red * px->alpha * (1 - alpha)) / px->alpha; */
                    /* px->green = (0 * val + px->green * px->alpha * (1 - alpha)) / px->alpha; */
                    /* px->blue = (255 * val + px->blue * px->alpha * (1 - alpha)) / px->alpha; */
                    /* px->alpha = alpha + px->alpha * (1 - alpha); */

                    px->red = 0;
                    px->green = 0;
                    px->blue = 255;
                    px->alpha = 255;
                }
            }
        }
    }
}

void draw(app_state* state, uint8_t* data, uint32_t width, uint32_t height, uint32_t stride)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint32_t* color = (uint32_t*)(data + y * stride + x * 4);
            *color = 0x00000000;
            /* pixel* px = (pixel*)(pool_data + y * stride + x * 4); */
            /* px->alpha = 255; */
            /* px->red = 255; */
            /* px->green = 0; */
            /* px->blue = 0; */
        }
    }

    time_t epoch;
    epoch = time(NULL);
    struct tm* info;

    info = localtime(&epoch);
    char buffer[20];

    strftime(buffer, 20, "%H:%M", info);
    draw_text(data, buffer, &state->sft, 10, 50);
    /* int text_x = 10; */
    /* int text_y = 50; */

    /* char* text = "Jose"; */
    /* for (int i = 0; i < strlen(text); i++) { */
    /*     SFT_Glyph glyph; */
    /*     SFT_GMetrics metrics; */
    /*     sft_lookup(&state->sft, text[i], &glyph); */
    /*     /1* printf("Glyph ID: %ld\n", glyph); *1/ */

    /*     sft_gmetrics(&state->sft, glyph, &metrics); */
    /*     text_x += metrics.advanceWidth; */

    /*     SFT_Image img = { */
    /*         .width = metrics.minWidth, */
    /*         .height = metrics.minHeight, */
    /*     }; */

    /*     char gp[img.width * img.height]; */
    /*     img.pixels = gp; */

    /*     sft_render(&state->sft, glyph, img); */

    /*     for (int y = 0; y < img.height; y++) { */
    /*         for (int x = 0; x < img.width; x++) { */
    /*             unsigned char val = gp[y * img.width + x]; */
    /*             /1* printf("%d\n", val); *1/ */
    /*             if (val != 0) { */
    /*                 pixel* px = (pixel*)(data + (text_y + y + metrics.yOffset) * stride + (text_x + x) * 4); */

    /*                 unsigned char alpha = (val + 128) / 255; */

    /*                 /1* px->red = (0 * val + px->red * px->alpha * (1 - alpha)) / px->alpha; *1/ */
    /*                 /1* px->green = (0 * val + px->green * px->alpha * (1 - alpha)) / px->alpha; *1/ */
    /*                 /1* px->blue = (255 * val + px->blue * px->alpha * (1 - alpha)) / px->alpha; *1/ */
    /*                 /1* px->alpha = alpha + px->alpha * (1 - alpha); *1/ */

    /*                 px->red = 0; */
    /*                 px->green = 0; */
    /*                 px->blue = 255; */
    /*                 px->alpha = 0; */
    /*             } */
    /*         } */
    /*     } */
    /* } */
}

static void
wl_buffer_release(void* data, struct wl_buffer* wl_buffer)
{
    /* Sent by the compositor when it's no longer using this buffer */
    wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener wl_buffer_listener = {
    .release = wl_buffer_release,
};

struct wl_buffer* get_buffer(app_state* state, uint32_t width, uint32_t height)
{
    const int stride = width * 4;
    const int shm_pool_size = height * stride;

    int fd = allocate_shm_file(shm_pool_size);
    struct wl_shm_pool* pool = wl_shm_create_pool(state->shm, fd, shm_pool_size);

    uint8_t* pool_data = mmap(NULL, shm_pool_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct wl_buffer* buffer = wl_shm_pool_create_buffer(
        pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);

    wl_shm_pool_destroy(pool);
    close(fd);

    // This munmap yields segfault, but this is in default documentation
    /* munmap(pool_data, shm_pool_size); */

    wl_buffer_add_listener(buffer, &wl_buffer_listener, NULL);
    // Clear the surface (can't use the memset way for the life of me)
    draw(state, pool_data, width, height, stride);

    return buffer;
}

static void layer_configure(void* data, struct zwlr_layer_surface_v1* layer_surface,
    uint32_t serial, uint32_t width, uint32_t height)
{
    printf("Layer configure\n");
    struct app_state* state = data;
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

    struct wl_buffer* buffer = get_buffer(state, width, height);

    wl_surface_attach(state->surface, buffer, 0, 0);
    wl_surface_damage(state->surface, 0, 0, UINT32_MAX, UINT32_MAX);
    wl_surface_commit(state->surface);
}

static const struct wl_callback_listener frame_callback_listener;

void wl_surface_frame_done(void* data,
    struct wl_callback* wl_callback,
    uint32_t callback_data)
{
    app_state* state = (app_state*)data;
    wl_callback_destroy(wl_callback);

    wl_callback = wl_surface_frame(state->surface);
    wl_callback_add_listener(wl_callback, &frame_callback_listener, state);

    struct wl_buffer* buffer = get_buffer(state, 320, 100);
    wl_surface_attach(state->surface, buffer, 0, 0);
    wl_surface_damage_buffer(state->surface, 0, 0, INT32_MAX, INT32_MAX);
    wl_surface_commit(state->surface);
}

static const struct wl_callback_listener frame_callback_listener = {
    .done = wl_surface_frame_done,
};

static const struct zwlr_layer_surface_v1_listener layer_listener = {
    .configure = layer_configure,
};

SFT load_font(char* name)
{
    int s = 2;
    SFT sft = {
        .xScale = 16 * s,
        .yScale = 16 * s,
        .flags = SFT_DOWNWARD_Y,
    };
    sft.font = sft_loadfile(name);
    return sft;
}

int main(int argc, char* argv[])
{
    app_state state;

    struct wl_display* display = init(&state);

    if (display == NULL) {
        return -1;
    }

    state.sft = load_font("/nix/store/z2lkf8q9ii0h46h782qy0i5dp18im047-nerdfonts-3.2.1/share/fonts/truetype/NerdFonts/JetBrainsMonoNerdFontPropo-Regular.ttf");

    // Only after first round trip state.compositor is set
    state.surface = wl_compositor_create_surface(state.compositor);

    // Turn surface into layer
    struct zwlr_layer_surface_v1* layer = zwlr_layer_shell_v1_get_layer_surface(state.layer_shell, state.surface, NULL, 3, "test");
    zwlr_layer_surface_v1_add_listener(layer, &layer_listener, &state);

    // xdg_surface stuff, disregard
    /* struct xdg_surface* xdg_surface = xdg_wm_base_get_xdg_surface(state.xdg_base, state.surface); */
    /* xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, &state); */
    /* struct xdg_toplevel* toplevel = xdg_surface_get_toplevel(xdg_surface); */
    /* xdg_toplevel_set_title(toplevel, "Wayland test"); */

    struct wl_callback* callback = wl_surface_frame(state.surface);
    wl_callback_add_listener(callback, &frame_callback_listener, &state);

    zwlr_layer_surface_v1_set_size(layer, 320, 100);
    zwlr_layer_surface_v1_set_anchor(layer, ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    /* zwlr_layer_surface_v1_set_exclusive_zone(layer, 100); */
    wl_surface_commit(state.surface);

    // Loop
    while (wl_display_dispatch(display) != -1) {
    }

    // Disconnect
    wl_display_disconnect(display);
    return 0;
}
