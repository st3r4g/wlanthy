#ifndef WLHANGUL_H
#define WLHANGUL_H

#include <hangul.h>
#include <stdbool.h>
#include <wayland-client-core.h>
#include <xkbcommon/xkbcommon.h>

struct wlhangul_state {
	struct wl_display *display;
	struct zwp_input_method_manager_v2 *input_method_manager;
	struct zwp_virtual_keyboard_manager_v1 *virtual_keyboard_manager;

	bool running;

	struct wl_list seats;
};

struct wlhangul_seat {
	struct wl_list link;
	struct wl_seat *wl_seat;
	struct wlhangul_state *state;

	HangulInputContext *input_context;
	struct zwp_input_method_v2 *input_method;
	struct zwp_virtual_keyboard_v1 *virtual_keyboard;

	struct xkb_context *xkb_context;
	struct xkb_keymap *xkb_keymap;
	struct xkb_state *xkb_state;

	bool active;
	uint32_t serial;
	bool pending_activate, pending_deactivate;
	struct zwp_input_method_keyboard_grab_v2 *keyboard_grab;
	xkb_keycode_t pressed[64];
};

char *ucsstr_to_str(const ucschar *ucsstr);

#endif
