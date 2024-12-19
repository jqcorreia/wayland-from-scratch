#include "gen/xdg-shell.h"
#include "shm.h"
#include "types.h"
#include <schrift.h>

// Mess of a code for documentation purposes
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
