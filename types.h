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
