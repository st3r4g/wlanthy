#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include "wlanthy.h"
#include "log.h"
#include "input-method-unstable-v2-client-protocol.h"
#include "virtual-keyboard-unstable-v1-client-protocol.h"

#define PREEDIT_BUFSIZE 4000

/*
 * Returns false if the key needs to be passed through
 */
static bool handle_key_anthy(struct wlanthy_seat *seat,
							 xkb_keycode_t xkb_key) {
	int state = anthy_input_get_state(seat->input_context);
	int map = anthy_input_get_selected_map(seat->input_context);
	xkb_keysym_t sym = xkb_state_key_get_one_sym(seat->xkb_state, xkb_key);

	if (sym == seat->state->toggle_key) {
		seat->enabled = !seat->enabled;
		if (!seat->enabled) {
			// reset state
			anthy_input_free_context(seat->input_context);
			seat->input_context = anthy_input_create_context(seat->input_config);
			zwp_input_method_v2_set_preedit_string(seat->input_method, "", 0, 0);
			zwp_input_method_v2_commit(seat->input_method, seat->serial);
		}
		return true;
	} else if (!seat->enabled) {
		return false;
	} else if (state == ANTHY_INPUT_ST_NONE && (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_ALT,
XKB_STATE_MODS_EFFECTIVE) > 0 || sym == XKB_KEY_Alt_L)) {
		return false;
	} else {
		switch (sym) {
		case XKB_KEY_exclam ... XKB_KEY_asciitilde:;
			uint32_t ch = xkb_state_key_get_utf32(seat->xkb_state, xkb_key);
			anthy_input_key(seat->input_context, ch);
			break;
		case XKB_KEY_space:
			anthy_input_space(seat->input_context);
			break;
		case XKB_KEY_BackSpace:
			if (state != ANTHY_INPUT_ST_NONE) {
				// TODO: send this repeatedly until key release
				anthy_input_erase_prev(seat->input_context);
			} else
				return false;
			break;
		case XKB_KEY_Tab:
			if (state != ANTHY_INPUT_ST_NONE) {
				if (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_ALT,
												 XKB_STATE_MODS_EFFECTIVE) > 0)
					anthy_input_resize(seat->input_context, 1);
				else
				anthy_input_move(seat->input_context, 1);
			} else
				return false;
			break;
		case XKB_KEY_ISO_Left_Tab:
			if (state != ANTHY_INPUT_ST_NONE) {
				if (xkb_state_mod_name_is_active(seat->xkb_state, XKB_MOD_NAME_ALT,
												 XKB_STATE_MODS_EFFECTIVE) > 0)
					anthy_input_resize(seat->input_context, -1);
				else
				anthy_input_move(seat->input_context, -1);
			} else
				return false;
			break;
		case XKB_KEY_Return:
			if (state != ANTHY_INPUT_ST_NONE) {
				anthy_input_commit(seat->input_context);
			} else
				return false;
			break;
		case XKB_KEY_Alt_L:
			return true;
		case XKB_KEY_F5:
			anthy_input_map_select(seat->input_context, ANTHY_INPUT_MAP_HIRAGANA);
			break;
		case XKB_KEY_F6:
			anthy_input_map_select(seat->input_context, ANTHY_INPUT_MAP_KATAKANA);
			break;
		case XKB_KEY_F7:
			anthy_input_map_select(seat->input_context, ANTHY_INPUT_MAP_ALPHABET);
			break;
		case XKB_KEY_F8:
			anthy_input_map_select(seat->input_context, ANTHY_INPUT_MAP_WALPHABET);
			break;
		case XKB_KEY_Up:
			if (state != ANTHY_INPUT_ST_NONE)
				anthy_input_prev_candidate(seat->input_context);
			else
				return false;
			break;
		case XKB_KEY_Down:
			if (state != ANTHY_INPUT_ST_NONE)
				anthy_input_next_candidate(seat->input_context);
			else
				return false;
			break;
		case XKB_KEY_Left:
			if (state != ANTHY_INPUT_ST_NONE)
				anthy_input_move(seat->input_context, -1);
			else
				return false;
			break;
		case XKB_KEY_Right:
			if (state != ANTHY_INPUT_ST_NONE)
				anthy_input_move(seat->input_context, +1);
			else
				return false;
			break;
		default:
			return false;
		}
	}
	/*
	 * At this point the key has been handled by anthy
	 */
	char name[64];
	xkb_keysym_get_name(sym, name, 64);
	log_line(LV_DEBUG, "state: %d, map: %d", state, map);
	log_line(LV_DEBUG, "pressed %s", name);
/*	anthy_context_t ac;
	if ((ac = anthy_input_get_anthy_context(seat->input_context)))
		anthy_print_context(ac);*/

	struct anthy_input_preedit *pe = anthy_input_get_preedit(seat->input_context);

	if (pe->commit) {
#ifndef HAVE_UTF8
		char *commit_str = iconv_code_conv(seat->conv_desc, pe->commit);
#else
		char *commit_str = pe->commit;
#endif
		log_line(LV_DEBUG, "%s", commit_str);
		zwp_input_method_v2_commit_string(seat->input_method, commit_str);
#ifndef HAVE_UTF8
		free(commit_str);
#endif
	zwp_input_method_v2_commit(seat->input_method, seat->serial);
	return true;
	}

	char preedit_str[PREEDIT_BUFSIZE]; preedit_str[0] = '\0';
	int totlen = 0;
	int begin = 0;
	int end = 0;
	log_head(LV_DEBUG);
	log_body(LV_DEBUG, "|");
	for (struct anthy_input_segment* cur = pe->segment; cur != NULL; cur = cur->next) {
		if (cur->str == NULL) { // the cursor, maybe should also check for ANTHY_INPUT_SF_CURSOR flag
			begin = end = totlen;
			continue;
		}
#ifndef HAVE_UTF8
		char *utf8_str = iconv_code_conv(seat->conv_desc, cur->str);
#else
		char *utf8_str = cur->str;
#endif
		size_t len = strlen(utf8_str);
		if (cur == pe->cur_segment) {
			begin = totlen;
			end = totlen+len;
			log_body(LV_DEBUG, "*");
		}
		totlen += len;
		log_body(LV_DEBUG, "%s|", utf8_str);
		if (PREEDIT_BUFSIZE-totlen-1 > 0)
			strcat(preedit_str, utf8_str);
#ifndef HAVE_UTF8
		free(utf8_str);
#endif
	}
	log_tail(LV_DEBUG);
	zwp_input_method_v2_set_preedit_string(seat->input_method,
	preedit_str, begin, end);
	zwp_input_method_v2_commit(seat->input_method, seat->serial);

	anthy_input_free_preedit(pe);
	return true;
}

uint32_t last = 0; // HACK?

static void handle_key(void *data,
		struct zwp_input_method_keyboard_grab_v2 *keyboard_grab,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	struct wlanthy_seat *seat = data;
	xkb_keycode_t xkb_key = key + 8;
	last = key; // HACK?

	if (seat->xkb_state == NULL) {
		return;
	}

	bool handled = false;
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		if (xkb_state_mod_names_are_active(seat->xkb_state,
XKB_STATE_MODS_EFFECTIVE, XKB_STATE_MATCH_ANY, XKB_MOD_NAME_CTRL,
XKB_MOD_NAME_LOGO, XKB_MOD_NAME_CAPS,
// TODO: investigate EFFECTIVE vs others
// TODO: investigate XKB_LED_NAME_CAPS, XKB_LED_NAME_NUM, XKB_LED_NAME_SCROLL
NULL) > 0) {
		/*
		 * Passthrough key if any modifier is active
		 */
			handled = false;
		} else
			handled = handle_key_anthy(seat, xkb_key);
	}

	// we are sending too many release here... bring back wlhangul stuff
	if (!handled) {
		zwp_virtual_keyboard_v1_key(seat->virtual_keyboard, time, key, state);
	}
}

static void handle_modifiers(void *data,
		struct zwp_input_method_keyboard_grab_v2 *keyboard_grab,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
	struct wlanthy_seat *seat = data;

	if (seat->xkb_state == NULL) {
		return;
	}

	xkb_state_update_mask(seat->xkb_state, mods_depressed,
		mods_latched, mods_locked, 0, 0, group);
	zwp_virtual_keyboard_v1_modifiers(seat->virtual_keyboard,
		mods_depressed, mods_latched, mods_locked, group);
}

static void handle_keymap(void *data,
		struct zwp_input_method_keyboard_grab_v2 *keyboard_grab,
		uint32_t format, int32_t fd, uint32_t size) {
	struct wlanthy_seat *seat = data;

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
		close(fd);
		return;
	}

	char *str = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (str == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return;
	}

	/*
	 * This is currently needed to avoid the keymap loop bug in sway
	 */
	static bool first_call = true;
	if (!first_call)
    		return;
	first_call = false;

	zwp_virtual_keyboard_v1_keymap(seat->virtual_keyboard, format, fd,
								   size);
	seat->xkb_keymap_string = strdup(str);

	if (seat->xkb_keymap != NULL) {
		xkb_keymap_unref(seat->xkb_keymap);
	}
	seat->xkb_keymap = xkb_keymap_new_from_string(seat->xkb_context, str,
		XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap(str, size);
	close(fd);

	if (seat->xkb_keymap == NULL) {
		fprintf(stderr, "Failed to compile keymap\n");
		return;
	}

	if (seat->xkb_state != NULL) {
		xkb_state_unref(seat->xkb_state);
	}
	seat->xkb_state = xkb_state_new(seat->xkb_keymap);
	if (seat->xkb_state == NULL) {
		fprintf(stderr, "Failed to create XKB state\n");
		return;
	}
}

static void handle_repeat_info(void *data,
		struct zwp_input_method_keyboard_grab_v2 *keyboard_grab, int32_t rate,
		int32_t delay) {
	// TODO
}

static const struct zwp_input_method_keyboard_grab_v2_listener
		keyboard_grab_listener = {
	.key = handle_key,
	.modifiers = handle_modifiers,
	.keymap = handle_keymap,
	.repeat_info = handle_repeat_info,
};

static void handle_activate(void *data,
		struct zwp_input_method_v2 *input_method) {
	struct wlanthy_seat *seat = data;
	seat->pending_activate = true;
}

static void handle_deactivate(void *data,
		struct zwp_input_method_v2 *input_method) {
	struct wlanthy_seat *seat = data;
	seat->pending_deactivate = true;
}

static void handle_surrounding_text(void *data,
		struct zwp_input_method_v2 *input_method, const char *text,
		uint32_t cursor, uint32_t anchor) {
}

static void handle_text_change_cause(void *data,
		struct zwp_input_method_v2 *input_method, uint32_t cause) {
}

static void handle_content_type(void *data,
		struct zwp_input_method_v2 *input_method, uint32_t hint,
		uint32_t purpose) {
}

static void handle_done(void *data, struct zwp_input_method_v2 *input_method) {
	struct wlanthy_seat *seat = data;
	seat->serial++;

	if (seat->pending_activate && !seat->active) {
		seat->keyboard_grab = zwp_input_method_v2_grab_keyboard(input_method);
		zwp_input_method_keyboard_grab_v2_add_listener(seat->keyboard_grab,
			&keyboard_grab_listener, seat);
		seat->active = true;
	} else if (seat->pending_deactivate && seat->active) {
		zwp_input_method_keyboard_grab_v2_release(seat->keyboard_grab);

		// reset state
		anthy_input_free_context(seat->input_context);
		seat->input_context = anthy_input_create_context(seat->input_config);

		memset(seat->pressed, 0, sizeof(seat->pressed));

		// wlhangul doesn't need this... why?
		if (seat->xkb_keymap != NULL)
			zwp_virtual_keyboard_v1_key(seat->virtual_keyboard, 0, last,
										WL_KEYBOARD_KEY_STATE_RELEASED);
		seat->keyboard_grab = NULL;
		seat->active = false;
	}

	seat->pending_activate = false;
	seat->pending_deactivate = false;
}

static void handle_unavailable(void *data,
		struct zwp_input_method_v2 *input_method) {
}

static const struct zwp_input_method_v2_listener input_method_listener = {
	.activate = handle_activate,
	.deactivate = handle_deactivate,
	.surrounding_text = handle_surrounding_text,
	.text_change_cause = handle_text_change_cause,
	.content_type = handle_content_type,
	.done = handle_done,
	.unavailable = handle_unavailable,
};

static struct wlanthy_seat *create_seat(struct wlanthy_state *state,
		struct wl_seat *wl_seat) {
	struct wlanthy_seat *seat = calloc(1, sizeof(*seat));
	seat->wl_seat = wl_seat;
	seat->state = state;
	seat->xkb_keymap_string = "";
	wl_list_insert(&state->seats, &seat->link);
	return seat;
}

static void registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	struct wlanthy_state *state = data;
	if (strcmp(interface, wl_seat_interface.name) == 0) {
		struct wl_seat *seat =
			wl_registry_bind(registry, name, &wl_seat_interface, 1);
		create_seat(state, seat);
	} else if (strcmp(interface, zwp_input_method_manager_v2_interface.name) == 0) {
		state->input_method_manager = wl_registry_bind(registry, name,
			&zwp_input_method_manager_v2_interface, 1);
	} else if (strcmp(interface, zwp_virtual_keyboard_manager_v1_interface.name) == 0) {
		state->virtual_keyboard_manager = wl_registry_bind(registry, name,
			&zwp_virtual_keyboard_manager_v1_interface, 1);
	}
}

static void registry_handle_global_remove(void *data,
		struct wl_registry *registry, uint32_t name) {
	// TODO
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

static const char usage[] = "usage: wlanthy [options...]\n"
	"\n"
	"    -d                 Show debug messages\n"
	"    -i anthy|pass      Initial input mode (default: anthy)\n"
	"    -k <key>           Key to toggle mode (default: F12)\n";

int main(int argc, char *argv[]) {
	struct wlanthy_state state = {0};
	state.enabled_by_default = true;
	state.toggle_key = XKB_KEY_F12;
	wl_list_init(&state.seats);

	int opt;
	while ((opt = getopt(argc, argv, "hdi:k:")) != -1) {
		switch (opt) {
		case 'd':
			log_set_loglevel(LV_DEBUG);
			break;
		case 'i':
			if (strcmp(optarg, "anthy") == 0) {
				state.enabled_by_default = true;
			} else if (strcmp(optarg, "pass") == 0) {
				state.enabled_by_default = false;
			} else {
				fprintf(stderr, "Invalid value for -i\n");
				return 1;
			}
			break;
		case 'k':;
			state.toggle_key =
				xkb_keysym_from_name(optarg, XKB_KEYSYM_NO_FLAGS);
			if (state.toggle_key == XKB_KEY_NoSymbol) {
				fprintf(stderr, "Invalid key for -k\n");
				return 1;
			}
			break;
		default:
			fprintf(stderr, "%s", usage);
			return 1;
		}
	}

	state.display = wl_display_connect(NULL);
	if (state.display == NULL) {
		fprintf(stderr, "failed to connect to Wayland display\n");
		return 1;
	}

	struct wl_registry *registry = wl_display_get_registry(state.display);
	wl_registry_add_listener(registry, &registry_listener, &state);
	wl_display_roundtrip(state.display);

	if (state.input_method_manager == NULL) {
		fprintf(stderr, "missing wl_seat or zwp_input_method_manager_v2\n");
		return 1;
	}

	anthy_input_init();

	struct wlanthy_seat *seat;
	wl_list_for_each(seat, &state.seats, link) {
		seat->conv_desc = iconv_open("UTF-8", "EUC-JP"); // should be unique...
		seat->input_config = anthy_input_create_config();
		seat->input_context = anthy_input_create_context(seat->input_config);
		seat->input_method = zwp_input_method_manager_v2_get_input_method(
			state.input_method_manager, seat->wl_seat);
		zwp_input_method_v2_add_listener(seat->input_method,
			&input_method_listener, seat);
		seat->virtual_keyboard =
			zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(
			state.virtual_keyboard_manager, seat->wl_seat);
		seat->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		seat->enabled = state.enabled_by_default;
	}

	state.running = true;
	while (state.running && wl_display_dispatch(state.display) != -1) {
		// This space is intentionally left blank
	}

//	finalize (seat->conv_desc);
	return 0;
}
