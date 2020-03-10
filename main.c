#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include "wlhangul.h"
#include "input-method-unstable-v2-client-protocol.h"

static void handle_activate(void *data,
		struct zwp_input_method_v2 *input_method) {
}

static void handle_deactivate(void *data,
		struct zwp_input_method_v2 *input_method) {
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
	// TODO
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

static struct wlhangul_seat *create_seat(struct wlhangul_state *state,
		struct wl_seat *wl_seat) {
	struct wlhangul_seat *seat = calloc(1, sizeof(*seat));
	seat->wl_seat = wl_seat;
	wl_list_insert(&state->seats, &seat->link);
	return seat;
}

static void registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version) {
	struct wlhangul_state *state = data;
	if (strcmp(interface, wl_seat_interface.name) == 0) {
		struct wl_seat *seat =
			wl_registry_bind(registry, name, &wl_seat_interface, 1);
		create_seat(state, seat);
	} else if (strcmp(interface, zwp_input_method_manager_v2_interface.name) == 0) {
		state->input_method_manager = wl_registry_bind(registry, name,
			&zwp_input_method_manager_v2_interface, 1);
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

int main(int argc, char *argv[]) {
	struct wlhangul_state state = {0};
	wl_list_init(&state.seats);

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

	struct wlhangul_seat *seat;
	wl_list_for_each(seat, &state.seats, link) {
		seat->input_context = hangul_ic_new("2");
		seat->input_method = zwp_input_method_manager_v2_get_input_method(
			state.input_method_manager, seat->wl_seat);
		zwp_input_method_v2_add_listener(seat->input_method,
			&input_method_listener, seat);
	}

	state.running = true;
	while (state.running && wl_display_dispatch(state.display) != -1) {
		// This space is intentionally left blank
	}

	return 0;
}
