#ifndef WLANTHY_H
#define WLANTHY_H

#include <anthy/anthy.h>
#include <anthy/input.h>
#include <iconv.h>
#include <stdbool.h>
#include <wayland-client-core.h>
#include <xkbcommon/xkbcommon.h>

struct wlanthy_state {
	struct wl_display *display;
	struct zwp_input_method_manager_v2 *input_method_manager;
	struct zwp_virtual_keyboard_manager_v1 *virtual_keyboard_manager;

	bool running;

	struct wl_list seats;

	bool enabled_by_default;
	xkb_keysym_t toggle_key;
};

struct wlanthy_seat {
	struct wl_list link;
	struct wl_seat *wl_seat;
	struct wlanthy_state *state;

	iconv_t conv_desc;
	struct anthy_input_config *input_config;
	struct anthy_input_context *input_context;
	struct zwp_input_method_v2 *input_method;
	struct zwp_virtual_keyboard_v1 *virtual_keyboard;

	struct xkb_context *xkb_context;
	struct xkb_keymap *xkb_keymap;
	struct xkb_state *xkb_state;
	char *xkb_keymap_string;

	bool active;
	bool enabled;
	uint32_t serial;
	bool pending_activate, pending_deactivate;
	struct zwp_input_method_keyboard_grab_v2 *keyboard_grab;
	xkb_keycode_t pressed[64];
};

char *
iconv_code_conv(iconv_t cd, const char *instr);
#endif
