#ifndef __TYPES__H
#define __TYPES__H

#include <schrift.h>

typedef struct app_state {
    struct wl_compositor* compositor;
    struct wl_shm* shm;
    struct xdg_wm_base* xdg_base;
    struct wl_surface* surface;
    struct zwlr_layer_shell_v1* layer_shell;

    SFT sft;
} app_state;

typedef struct pixel {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} pixel;

#endif
/* void draw_text(uint8_t* data, char* text, SFT_Font* font, uint32_t x, uint32_t y) */
/* { */
/*     int text_x = x; */
/*     int text_y = y; */
/*     int stride = 4; // Hardcoded for now */

/*     for (int i = 0; i < strlen(text); i++) { */
/*         SFT_Glyph glyph; */
/*         SFT_GMetrics metrics; */
/*         sft_lookup(font, text[i], &glyph); */
/*         /1* printf("Glyph ID: %ld\n", glyph); *1/ */

/*         sft_gmetrics(font, glyph, &metrics); */
/*         text_x += metrics.advanceWidth; */

/*         SFT_Image img = { */
/*             .width = metrics.minWidth, */
/*             .height = metrics.minHeight, */
/*         }; */

/*         char gp[img.width * img.height]; */
/*         img.pixels = gp; */

/*         sft_render(font, glyph, img); */

/*         for (int y = 0; y < img.height; y++) { */
/*             for (int x = 0; x < img.width; x++) { */
/*                 unsigned char val = gp[y * img.width + x]; */
/*                 /1* printf("%d\n", val); *1/ */
/*                 if (val != 0) { */
/*                     pixel* px = (pixel*)(data + (text_y + y + metrics.yOffset) * stride + (text_x + x) * 4); */

/*                     unsigned char alpha = (val + 128) / 255; */

/*                     /1* px->red = (0 * val + px->red * px->alpha * (1 - alpha)) / px->alpha; *1/ */
/*                     /1* px->green = (0 * val + px->green * px->alpha * (1 - alpha)) / px->alpha; *1/ */
/*                     /1* px->blue = (255 * val + px->blue * px->alpha * (1 - alpha)) / px->alpha; *1/ */
/*                     /1* px->alpha = alpha + px->alpha * (1 - alpha); *1/ */

/*                     px->red = 0; */
/*                     px->green = 0; */
/*                     px->blue = 255; */
/*                     px->alpha = 255; */
/*                 } */
/*             } */
/*         } */
/*     } */
/* } */
