// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <immintrin.h>
#include <unistd.h>
#include <stdio.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)) + 0 * !__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])))
#define CENTER_X(state, width) (((int32_t)(state)->buffer_width - (int32_t)(width)) / 2)
#define BUFFER_PTR(state, x, y) ((state)->buffer + (x) + (y) * (state)->buffer_width)

#include "incbin.h"

#define MKFW_TIMER
#define MKFW_AUDIO
#include "mkfw.h"
#include "platform_gl_loader.c"
#include "platform_state.c"

#include "platform_opengl.c"

#include "ugg.h"
#include "clip_blit.h"
#include "fade.h"
#include "futurecomposer14.h"
#include "generic_scroller.h"
#include "micromod.h"
#define RAND_IMPLEMENTATION
#include "rand.h"
#define MKS_RESAMPLER_IMPLEMENTATION
#include "resampler.h"

#include "option_selectors/selector_1/option_selector_1.c"

// [=]===^=[ platform_clear_buffer ]=================================================================^===[=]
__attribute__((always_inline))
static inline void platform_clear_buffer(struct platform_state *state) {
	memset(state->buffer, 0, state->buffer_width * state->buffer_height * sizeof(uint32_t));
}

static void remake_options(struct platform_state *state);
static void remake_init(struct platform_state *state);
static void remake_frame(struct platform_state *state);
static void remake_shutdown(struct platform_state *state);

static void framebuffer_callback(struct mkfw_state *window, int32_t width, int32_t height, float aspect_ratio) {
	struct platform_state *state = (struct platform_state *)window->user_data;

	int32_t viewport_x = 0;
	int32_t viewport_y = 0;
	int32_t viewport_width = width;
	int32_t viewport_height = height;

	double target_aspect = (aspect_ratio!=0.f) ? (double)aspect_ratio : ((double)width / (double)height);
	double current_aspect = (double)width / (double)height;

	if(current_aspect>target_aspect) {
		int32_t new_width = (int32_t)((double)height * target_aspect + 0.5);
		viewport_x = (width - new_width) / 2;
		viewport_width = new_width;
	} else if(current_aspect<target_aspect) {
		int32_t new_height = (int32_t)((double)width / target_aspect + 0.5);
		viewport_y = (height - new_height) / 2;
		viewport_height = new_height;
	} else {
	}

	viewport_x &= ~1;
	viewport_y &= ~1;

	state->viewport.x = viewport_x;
	state->viewport.y = viewport_y;
	state->viewport.w = viewport_width;
	state->viewport.h = viewport_height;

	state->viewport_changed = 1;
}

// [=]===^=[ key_callback ]=================================================================^===[=]
static void key_callback(struct mkfw_state *window, uint32_t key, uint32_t action, uint32_t mods) {

	// remake_key(key, action, mods);

}

// [=]===^=[ mouse_move_callback ]=================================================================^===[=]
static void mouse_move_callback(struct mkfw_state *window, int32_t x, int32_t y) {
	struct platform_state *state = (struct platform_state *)window->user_data;
	state->mouse_dx += x;
	state->mouse_dy += y;
}

// [=]===^=[ mouse_button_callback ]=================================================================^===[=]
static void mouse_button_callback(struct mkfw_state *window, uint8_t button, int action) {
}

// [=]===^=[ error_callback ]=================================================================^===[=]
static void error_callback(const char *message) {
	fprintf(stderr, "mkfw: %s\n", message);
}

// [=]===^=[ render_thread_func ]=================================================================^===[=]
static MKFW_THREAD_FUNC(render_thread_func, arg) {
	struct platform_state *state = (struct platform_state *)arg;
	struct mkfw_state *window = state->window;

	mkfw_attach_context(window);

	struct mkfw_timer_handle *timer = mkfw_timer_new(FRAME_TIME_NS);
	uint64_t last_frame_time_ns = mkfw_gettime(window);

	while(state->running) {
		if(mkfw_is_key_pressed(window, MKS_KEY_F11)) {
			state->fullscreen = !state->fullscreen;
			mkfw_fullscreen(window, state->fullscreen);
		}

		if(mkfw_is_key_pressed(window, MKS_KEY_F12)) {
			state->toggle_crt_emulation = !state->toggle_crt_emulation;
		}

		if(mkfw_is_key_pressed(window, MKS_KEY_ESCAPE)) {
			state->running = 0;
		}

		remake_frame(state);

		mkfw_update_input_state(window);
		state->frame_number++;
		opengl_render_frame(state);
		mkfw_swap_buffers(window);
		mkfw_timer_wait(timer);

	}

	mkfw_timer_destroy(timer);
	return 0;
}

// [=]===^=[ main ]=================================================================^===[=]
int main(int argc, char **argv) {
	mkfw_set_error_callback(error_callback);
	platform_state.toggle_crt_emulation = 1;
	platform_state.crt_mask_type = 1;

	mkfw_audio_initialize();
	mkfw_timer_init();

	remake_options(&platform_state);
	if(option_selector(&platform_state)) {
		mkfw_timer_shutdown();
		return -1;
	}

	platform_state.window = mkfw_init(1024, 768);
	opengl_function_loader();

	mkfw_set_user_data(platform_state.window, &platform_state);
	mkfw_set_swapinterval(platform_state.window, 0);
	mkfw_set_key_callback(platform_state.window, key_callback);
	// mkfw_set_mouse_move_delta_callback(platform_state.window, mouse_move_callback);
	// mkfw_set_mouse_button_callback(platform_state.window, mouse_button_callback);
	mkfw_set_framebuffer_size_callback(platform_state.window, framebuffer_callback);

	// Minimum window: 640x480 (2x scale for typical content)
	mkfw_set_window_min_size_and_aspect(platform_state.window, 640, 480, CRT_ASPECT_NUM, CRT_ASPECT_DEN);
	mkfw_set_window_title(platform_state.window, platform_state.window_title);
	mkfw_show_window(platform_state.window);

	mkfw_fullscreen(platform_state.window, platform_state.fullscreen);

	opengl_setup(&platform_state);
	opengl_setup_render_targets(&platform_state);
	remake_init(&platform_state);

	int32_t init_w, init_h;
	mkfw_get_framebuffer_size(platform_state.window, &init_w, &init_h);
	framebuffer_callback(platform_state.window, init_w, init_h, CRT_ASPECT);

	platform_state.running = 1;

	mkfw_detach_context(platform_state.window);

	mkfw_thread render_thread = mkfw_thread_create(render_thread_func, &platform_state);
	if(render_thread) {
		while(platform_state.running && !mkfw_should_close(platform_state.window)) {
			mkfw_pump_messages(platform_state.window);
			mkfw_sleep(5000000);
		}
		platform_state.running = 0;
		mkfw_thread_join(render_thread);
	}


	remake_shutdown(&platform_state);
	mkfw_cleanup(platform_state.window);
	mkfw_audio_shutdown();
	mkfw_timer_shutdown();
	return 0;
}

