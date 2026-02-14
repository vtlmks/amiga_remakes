// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "mkfw_keys.h"

/* Forward declaration */
struct mkfw_state;

/* Callback function pointers */
typedef void (*key_callback_t)(struct mkfw_state *state, uint32_t key, uint32_t action, uint32_t modifier_bits);
typedef void (*char_callback_t)(struct mkfw_state *state, uint32_t codepoint);
typedef void (*scroll_callback_t)(struct mkfw_state *state, double xoffset, double yoffset);
typedef void (*mouse_move_delta_callback_t)(struct mkfw_state *state, int32_t x, int32_t y);
typedef void (*mouse_button_callback_t)(struct mkfw_state *state, uint8_t button, int action);
typedef void (*framebuffer_callback_t)(struct mkfw_state *state, int32_t width, int32_t height, float aspect_ratio);

/* Main state structure */
struct mkfw_state {
	// Shared input state
	uint8_t keyboard_state[MKS_KEY_LAST];
	uint8_t prev_keyboard_state[MKS_KEY_LAST];
	uint8_t modifier_state[MKS_MODIFIER_LAST];
	uint8_t prev_modifier_state[MKS_MODIFIER_LAST];
	uint8_t mouse_buttons[3];
	uint8_t previous_mouse_buttons[3];
	int32_t mouse_x;
	int32_t mouse_y;
	uint8_t is_fullscreen;

	// Callbacks
	key_callback_t key_callback;
	char_callback_t char_callback;
	scroll_callback_t scroll_callback;
	mouse_move_delta_callback_t mouse_move_delta_callback;
	mouse_button_callback_t mouse_button_callback;
	framebuffer_callback_t framebuffer_callback;

	// Platform-specific state
	void *platform;

	// User data (for application use)
	void *user_data;
};

/* Error reporting callback */
typedef void (*mkfw_error_callback_t)(const char *message);
static mkfw_error_callback_t mkfw_error_callback;

__attribute__((format(printf, 1, 2)))
static inline void mkfw_error(const char *fmt, ...) {
	if(mkfw_error_callback) {
		char buf[512];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);
		mkfw_error_callback(buf);
	}
}

/* Platform-specific implementation includes */
#ifdef _WIN32
#include "mkfw_win32.c"
#elif __linux__
#include "mkfw_linux.c"
#endif

/* Thread abstraction */
#ifdef _WIN32
	typedef HANDLE mkfw_thread;
	#define MKFW_THREAD_FUNC(name, arg) DWORD WINAPI name(LPVOID arg)

	static inline mkfw_thread mkfw_thread_create(LPTHREAD_START_ROUTINE func, void *arg) {
		return CreateThread(0, 0, func, arg, 0, 0);
	}

	static inline void mkfw_thread_join(mkfw_thread t) {
		WaitForSingleObject(t, INFINITE);
		CloseHandle(t);
	}
#else
	#include <pthread.h>
	typedef pthread_t mkfw_thread;
	#define MKFW_THREAD_FUNC(name, arg) void *name(void *arg)

	static inline mkfw_thread mkfw_thread_create(void *(*func)(void *), void *arg) {
		pthread_t t;
		if(pthread_create(&t, 0, func, arg)) {
			return 0;
		}
		return t;
	}

	static inline void mkfw_thread_join(mkfw_thread t) {
		pthread_join(t, 0);
	}

	static inline void mkfw_thread_cancel(mkfw_thread t) {
		pthread_cancel(t);
	}
#endif

/* Audio subsystem - optional */
#ifdef MKFW_AUDIO
#ifdef _WIN32
#include "mkfw_win32_audio.c"
#elif __linux__
#include "mkfw_linux_audio.c"
#endif
#endif

/* Timer subsystem - optional */
#ifdef MKFW_TIMER
#ifdef _WIN32
#include "mkfw_win32_timer.c"
#elif __linux__
#include "mkfw_linux_timer.c"
#endif
#endif

/* Inline helper functions - placed after platform includes so struct is defined */
static inline void mkfw_update_input_state(struct mkfw_state *state) {
	memcpy(state->prev_keyboard_state, state->keyboard_state, sizeof(state->keyboard_state));
	memcpy(state->prev_modifier_state, state->modifier_state, sizeof(state->modifier_state));
	memcpy(state->previous_mouse_buttons, state->mouse_buttons, sizeof(state->mouse_buttons));
}

static inline void mkfw_set_error_callback(mkfw_error_callback_t callback) { mkfw_error_callback = callback; }
static inline void mkfw_set_user_data(struct mkfw_state *state, void *user_data) { state->user_data = user_data; }
static inline void *mkfw_get_user_data(struct mkfw_state *state) { return state->user_data; }
static inline void mkfw_set_key_callback(struct mkfw_state *state, key_callback_t callback) { state->key_callback = callback; }
static inline void mkfw_set_char_callback(struct mkfw_state *state, char_callback_t callback) { state->char_callback = callback; }
static inline void mkfw_set_scroll_callback(struct mkfw_state *state, scroll_callback_t callback) { state->scroll_callback = callback; }
static inline void mkfw_set_mouse_move_delta_callback(struct mkfw_state *state, mouse_move_delta_callback_t callback) { state->mouse_move_delta_callback = callback; }
static inline void mkfw_set_mouse_button_callback(struct mkfw_state *state, mouse_button_callback_t callback) { state->mouse_button_callback = callback; }
static inline void mkfw_set_framebuffer_size_callback(struct mkfw_state *state, framebuffer_callback_t callback) { state->framebuffer_callback = callback; }
static inline int mkfw_is_key_pressed(struct mkfw_state *state, uint8_t key) { return state->keyboard_state[key] && !state->prev_keyboard_state[key]; }
static inline int mkfw_was_key_released(struct mkfw_state *state, uint8_t key) { return !state->keyboard_state[key] && state->prev_keyboard_state[key]; }
static inline uint8_t mkfw_is_button_pressed(struct mkfw_state *state, uint8_t button) { return (state->mouse_buttons[button] & !state->previous_mouse_buttons[button]); }

/* UI subsystem - optional */
#ifdef MKFW_UI
#include "mkfw_ui.c"
#else
/* Stub macros when UI is not enabled - allows code to compile without #ifdefs */
#define mkui_init(mkfw) ((void)0)
#define mkui_shutdown() ((void)0)
#define mkui_new_frame(w, h) ((void)0)
#define mkui_render() ((void)0)
#define mkui_begin_window(title, x, y, w, h) ((void)0)
#define mkui_end_window() ((void)0)
#define mkui_text(text) ((void)0)
#define mkui_text_colored(text, color) ((void)0)
#define mkui_button(label) (0)
#define mkui_checkbox(label, checked) (0)
#define mkui_slider_float(label, value, min_val, max_val) (0)
#define mkui_slider_int(label, value, min_val, max_val) (0)
#define mkui_slider_int64(label, value, min_val, max_val) (0)
#define mkui_radio_button(label, selected, value) (0)
#define mkui_collapsing_header(label, open) (0)
#define mkui_text_input(label, buffer, buffer_size) (0)
#define mkui_combo(label, current_item, items, items_count) (0)
#define mkui_listbox(label, current_item, items, items_count, visible_items) (0)
#define mkui_separator() ((void)0)
#define mkui_set_cursor_pos(x, y) ((void)0)
#define mkui_same_line() ((void)0)
#define mkui_image(texture_id, width, height) ((void)0)
#define mkui_image_rgba(rgba_buffer, width, height) ((void)0)
#define mkui_rgb(r, g, b) ((struct mkui_color){0,0,0,0})
#define mkui_rgba(r, g, b, a) ((struct mkui_color){0,0,0,0})
#define mkui_get_style() ((struct mkui_style*)0)
#define mkui_style_set_color(color_id, r, g, b, a) ((void)0)
#endif
