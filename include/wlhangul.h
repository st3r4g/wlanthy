#ifndef WLHANGUL_H
#define WLHANGUL_H

#include <hangul.h>
#include <stdbool.h>
#include <wayland-client-core.h>

struct wlhangul_state {
	struct wl_display *display;
	struct zwp_input_method_manager_v2 *input_method_manager;

	bool running;

	struct wl_list seats;
};

struct wlhangul_seat {
	struct wl_list link;
	struct wl_seat *wl_seat;
	struct wlhangul_state *state;

	HangulInputContext *input_context;
	struct zwp_input_method_v2 *input_method;
};

#endif
