#ifndef __INIT__H
#define __INIT__H

#include "gen/wlr-layer-shell-unstable-v1.h"
#include "gen/xdg-shell.h"
#include "types.h"
#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

struct wl_display* init(app_state* state);

#endif
