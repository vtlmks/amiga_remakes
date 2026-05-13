// Copyright (c) 2025-2026 Peter Fors
// SPDX-License-Identifier: MIT

#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)) + 0 * !__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])))
#define CENTER_X(state, width) (((int32_t)(state)->buffer_width - (int32_t)(width)) / 2)
#define BUFFER_PTR(state, x, y) ((state)->buffer + (x) + (y) * (state)->buffer_width)

#include "incbin.h"

#include "mkfw.h"
#include "mkfw_audio.h"
#include "mkfw_timer.h"

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

// The remakes produce int16 stereo (micromod, fc14, raw samples).  The new
// mkfw_audio callback is float; the platform layer owns the int16->float
// bridge and the LPF+dither post-process, exposing a small int16 callback API
// for the remakes.
typedef void (*platform_audio_callback_t)(int16_t *data, size_t frames);
static platform_audio_callback_t platform_audio_callback;

#define PLATFORM_AUDIO_BRIDGE_FRAMES 4096
static int16_t platform_audio_bridge_buffer[PLATFORM_AUDIO_BRIDGE_FRAMES * 2];

// Single-pole IIR low-pass filter (RC filter).
// alpha = dt / (rc + dt), where dt = 1/sample_rate, rc = 1/(2*pi*cutoff_hz).
// At 48 kHz with ~8 kHz cutoff: alpha ~= 0.7265.
#define PLATFORM_LP_ALPHA 0.7265f
static float platform_lp_prev_l = 0.0f;
static float platform_lp_prev_r = 0.0f;
static struct rng_state platform_dither_rng = { 0x12345678, 0x9abcdef0, 0xdeadbeef, 0xcafebabe };

// [=]===^=[ platform_audio_post_process ]=============================================================^===[=]
static void platform_audio_post_process(int16_t *audio_buffer, size_t frames) {
#ifdef AUDIO_STEREO_MIX
	for(size_t i = 0; i < frames; ++i) {
		int32_t old_left = (int32_t)audio_buffer[i * 2];
		int32_t old_right = (int32_t)audio_buffer[i * 2 + 1];

		int32_t mixed_left = old_left + (old_right * 3) / 4;
		int32_t mixed_right = old_right + (old_left * 3) / 4;

		audio_buffer[i * 2] = (int16_t)(mixed_left >> 1);
		audio_buffer[i * 2 + 1] = (int16_t)(mixed_right >> 1);
	}
#endif

	for(size_t i = 0; i < frames; ++i) {
		float l = (float)audio_buffer[i * 2 + 0];
		float r = (float)audio_buffer[i * 2 + 1];
		platform_lp_prev_l += PLATFORM_LP_ALPHA * (l - platform_lp_prev_l);
		platform_lp_prev_r += PLATFORM_LP_ALPHA * (r - platform_lp_prev_r);
		float dither_l = (float)(xor_generate_random(&platform_dither_rng) & 1) - (float)(xor_generate_random(&platform_dither_rng) & 1);
		float dither_r = (float)(xor_generate_random(&platform_dither_rng) & 1) - (float)(xor_generate_random(&platform_dither_rng) & 1);
		audio_buffer[i * 2 + 0] = (int16_t)(platform_lp_prev_l + dither_l);
		audio_buffer[i * 2 + 1] = (int16_t)(platform_lp_prev_r + dither_r);
	}
}

// [=]===^=[ platform_audio_float_bridge ]=============================================================^===[=]
// Runs on the mkfw audio thread.  Negotiated stream is always stereo float at
// 48 kHz per mkfw_audio_options in main(); the int16-based audio decoders the
// remakes use produce samples in the platform staging buffer, get the LPF +
// dither pass, then are converted to float for mkfw.
static void platform_audio_float_bridge(void *userdata, float *buffer, uint32_t frames) {
	(void)userdata;
	platform_audio_callback_t cb = __atomic_load_n(&platform_audio_callback, __ATOMIC_ACQUIRE);
	if(!cb) {
		memset(buffer, 0, sizeof(float) * frames * 2);
		return;
	}

	uint32_t remaining = frames;
	float *out = buffer;
	while(remaining) {
		uint32_t chunk = remaining;
		if(chunk > PLATFORM_AUDIO_BRIDGE_FRAMES) {
			chunk = PLATFORM_AUDIO_BRIDGE_FRAMES;
		}

		cb(platform_audio_bridge_buffer, chunk);
		platform_audio_post_process(platform_audio_bridge_buffer, chunk);

		for(uint32_t i = 0; i < chunk * 2; ++i) {
			out[i] = (float)platform_audio_bridge_buffer[i] * (1.0f / 32768.0f);
		}

		out += chunk * 2;
		remaining -= chunk;
	}
}

static inline void platform_set_audio_callback(platform_audio_callback_t cb) {
	__atomic_store_n(&platform_audio_callback, cb, __ATOMIC_RELEASE);
}

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

// [=]===^=[ framebuffer_callback ]=================================================================^===[=]
static void framebuffer_callback(struct mkfw_window *window, int32_t width, int32_t height, float aspect_ratio) {
	struct platform_state *state = (struct platform_state *)mkfw_window_get_user_data(window);

	int32_t viewport_x = 0;
	int32_t viewport_y = 0;
	int32_t viewport_width = width;
	int32_t viewport_height = height;

	double target_aspect = (aspect_ratio != 0.f) ? (double)aspect_ratio : ((double)width / (double)height);
	double current_aspect = (double)width / (double)height;

	if(current_aspect > target_aspect) {
		int32_t new_width = (int32_t)((double)height * target_aspect + 0.5);
		viewport_x = (width - new_width) / 2;
		viewport_width = new_width;
	} else if(current_aspect < target_aspect) {
		int32_t new_height = (int32_t)((double)width / target_aspect + 0.5);
		viewport_y = (height - new_height) / 2;
		viewport_height = new_height;
	}

	viewport_x &= ~1;
	viewport_y &= ~1;

	state->viewport.x = viewport_x;
	state->viewport.y = viewport_y;
	state->viewport.w = viewport_width;
	state->viewport.h = viewport_height;

	__atomic_store_n(&state->viewport_changed, 1, __ATOMIC_RELEASE);
}

// [=]===^=[ key_callback ]=================================================================^===[=]
static void key_callback(struct mkfw_window *window, uint32_t key, uint32_t action, uint32_t mods) {
	(void)window;
	(void)key;
	(void)action;
	(void)mods;
}

// [=]===^=[ mouse_move_callback ]=================================================================^===[=]
static void mouse_move_callback(struct mkfw_window *window, int32_t x, int32_t y) {
	struct platform_state *state = (struct platform_state *)mkfw_window_get_user_data(window);
	state->mouse_dx += x;
	state->mouse_dy += y;
}

// [=]===^=[ mouse_button_callback ]=================================================================^===[=]
static void mouse_button_callback(struct mkfw_window *window, uint8_t button, uint32_t action) {
	(void)window;
	(void)button;
	(void)action;
}

// [=]===^=[ error_callback ]=================================================================^===[=]
static void error_callback(const char *message) {
	fprintf(stderr, "mkfw: %s\n", message);
}

// [=]===^=[ render_thread_func ]=================================================================^===[=]
static MKFW_THREAD_FUNC(render_thread_func, arg) {
	struct platform_state *state = (struct platform_state *)arg;
	struct mkfw_window *window = state->window;

	mkfw_window_attach_context(window);

	struct mkfw_timer_handle *timer = mkfw_timer_create(FRAME_TIME_NS);

	while(__atomic_load_n(&state->running, __ATOMIC_ACQUIRE)) {
		if(mkfw_window_is_key_pressed(window, MKFW_KEY_F11)) {
			state->fullscreen = !state->fullscreen;
			mkfw_window_set_fullscreen(window, state->fullscreen);
		}

		if(mkfw_window_is_key_pressed(window, MKFW_KEY_F12)) {
			state->toggle_crt_emulation = !state->toggle_crt_emulation;
		}

		if(mkfw_window_is_key_pressed(window, MKFW_KEY_F10)) {
			state->toggle_bloom = !state->toggle_bloom;
		}

		if(mkfw_window_is_key_pressed(window, MKFW_KEY_ESCAPE)) {
			__atomic_store_n(&state->running, 0, __ATOMIC_RELEASE);
		}

		remake_frame(state);

		mkfw_window_update_input_state(window);
		state->frame_number++;
		opengl_render_frame(state);
		mkfw_window_swap_buffers(window);
		mkfw_timer_wait(timer);
	}

	mkfw_timer_destroy(timer);
	return 0;
}

// [=]===^=[ main ]=================================================================^===[=]
int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	mkfw_set_error_callback(error_callback);
	platform_state.toggle_crt_emulation = 1;
	platform_state.toggle_bloom = 1;
	platform_state.crt_mask_type = 1;

	platform_state.ctx = mkfw_init(0);
	if(!platform_state.ctx) {
		return -1;
	}

	struct mkfw_audio_options aopts = {
		.preferred_sample_rate = 48000,
		.preferred_channels = 2,
	};
	if(!mkfw_audio_init(&aopts)) {
		mkfw_shutdown(platform_state.ctx);
		return -1;
	}
	mkfw_audio_set_callback(platform_audio_float_bridge, 0);

	mkfw_timer_init();

	remake_options(&platform_state);
	if(option_selector(&platform_state)) {
		mkfw_timer_shutdown();
		mkfw_audio_shutdown();
		mkfw_shutdown(platform_state.ctx);
		return -1;
	}

	struct mkfw_window_options wopts = {
		.width = 1024,
		.height = 768,
		.title = platform_state.window_title,
	};
	platform_state.window = mkfw_window_create(platform_state.ctx, &wopts);
	if(!platform_state.window) {
		mkfw_timer_shutdown();
		mkfw_audio_shutdown();
		mkfw_shutdown(platform_state.ctx);
		return -1;
	}
	opengl_function_loader();

	mkfw_window_set_user_data(platform_state.window, &platform_state);
	mkfw_window_set_swap_interval(platform_state.window, 0);
	mkfw_window_set_key_callback(platform_state.window, key_callback);
	mkfw_window_set_framebuffer_size_callback(platform_state.window, framebuffer_callback);

	// Minimum window: 640x480 (2x scale for typical content)
	mkfw_window_set_size_limits(platform_state.window, 640, 480, 0, 0);
	mkfw_window_set_aspect_ratio(platform_state.window, CRT_ASPECT_NUM, CRT_ASPECT_DEN);

	mkfw_window_set_fullscreen(platform_state.window, platform_state.fullscreen);

	opengl_setup(&platform_state);
	opengl_setup_render_targets(&platform_state);
	remake_init(&platform_state);

	int32_t init_w, init_h;
	mkfw_window_get_framebuffer_size(platform_state.window, &init_w, &init_h);
	framebuffer_callback(platform_state.window, init_w, init_h, CRT_ASPECT);

	__atomic_store_n(&platform_state.running, 1, __ATOMIC_RELEASE);

	mkfw_window_detach_context(platform_state.window);

	mkfw_thread render_thread = mkfw_thread_create(render_thread_func, &platform_state);
	if(render_thread) {
		while(__atomic_load_n(&platform_state.running, __ATOMIC_ACQUIRE) && !mkfw_window_should_close(platform_state.window)) {
			mkfw_poll_events(platform_state.ctx);
			mkfw_sleep(5000000);
		}
		__atomic_store_n(&platform_state.running, 0, __ATOMIC_RELEASE);
		mkfw_thread_join(render_thread);
	}

	remake_shutdown(&platform_state);
	mkfw_window_destroy(platform_state.window);
	mkfw_timer_shutdown();
	mkfw_audio_shutdown();
	mkfw_shutdown(platform_state.ctx);
	return 0;
}
