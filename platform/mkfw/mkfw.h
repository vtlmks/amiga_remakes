
#pragma once

#include <stdint.h>
#include <string.h>

#include "mkfw_keys.h"

/* Forward declaration */
struct mkfw_state;

/* Callback function pointers */
typedef void (*key_callback_t)(struct mkfw_state *state, uint32_t key, uint32_t action, uint32_t modifier_bits);
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
	uint8_t is_fullscreen;

	// Callbacks
	key_callback_t key_callback;
	mouse_move_delta_callback_t mouse_move_delta_callback;
	mouse_button_callback_t mouse_button_callback;
	framebuffer_callback_t framebuffer_callback;

	// Platform-specific state
	void *platform;
};

/* Platform-specific implementation includes */
#ifdef _WIN32
#include "mkfw_win32.c"
#elif __linux__
#include "mkfw_linux.c"
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
static inline void mkfw_update_modifier_state(struct mkfw_state *state) { memcpy(state->prev_modifier_state, state->modifier_state, sizeof(state->modifier_state)); }
static inline void mkfw_update_keyboard_state(struct mkfw_state *state) { memcpy(state->prev_keyboard_state, state->keyboard_state, sizeof(state->keyboard_state)); }
static inline void mkfw_update_mouse_state(struct mkfw_state *state) { memcpy(state->previous_mouse_buttons, state->mouse_buttons, sizeof(state->mouse_buttons)); }

static inline void mkfw_set_key_callback(struct mkfw_state *state, key_callback_t callback) { state->key_callback = callback; }
static inline void mkfw_set_mouse_move_delta_callback(struct mkfw_state *state, mouse_move_delta_callback_t callback) { state->mouse_move_delta_callback = callback; }
static inline void mkfw_set_mouse_button_callback(struct mkfw_state *state, mouse_button_callback_t callback) { state->mouse_button_callback = callback; }
static inline void mkfw_set_framebuffer_size_callback(struct mkfw_state *state, framebuffer_callback_t callback) { state->framebuffer_callback = callback; }
static inline int mkfw_is_key_pressed(struct mkfw_state *state, uint8_t key) { return state->keyboard_state[key] && !state->prev_keyboard_state[key]; }
static inline int mkfw_was_key_released(struct mkfw_state *state, uint8_t key) { return !state->keyboard_state[key] && state->prev_keyboard_state[key]; }
static inline uint8_t mkfw_is_button_pressed(struct mkfw_state *state, uint8_t button) { return (state->mouse_buttons[button] & !state->previous_mouse_buttons[button]); }
