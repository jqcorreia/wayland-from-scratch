#ifndef __INIT__H
#define __INIT__H

#include "types.h"
#include "wayland-client-protocol.h"
#include "wlr-layer-shell-unstable-v1.h"
#include "xdg-shell.h"
#include <stdio.h>
#include <wayland-client-core.h>

struct wl_display* init(app_state* state);

#endif
