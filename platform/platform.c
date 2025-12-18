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

// #ifdef DEBUGPRINT
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
// #else
// #define DEBUG_PRINT(fmt, ...)
// #endif

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)) + 0 * !__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])))

#define CENTER_X(width) (((int32_t)BUFFER_WIDTH - (int32_t)(width)) / 2)


// GL typedefs must come before mkfw.h for Linux (mkfw's GLX loader needs them)
#ifdef _WIN32
typedef __int64 GLintptr;
#else
typedef intptr_t GLintptr;
#endif
typedef void GLvoid;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef char GLchar;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef unsigned int GLbitfield;
typedef float GLfloat;
typedef double GLdouble;
typedef unsigned long long GLsizeiptr;

// Forward declare glXGetProcAddress for mkfw's GLX loader (defined in platform_gl_loader.c)
#ifdef __linux__
static void *glXGetProcAddress(const GLubyte *procName);
#endif

#include "incbin.h"

#define MKFW_TIMER
#define MKFW_AUDIO
#include "mkfw.h"
#include "platform_gl_loader.c"
#include "platform_state.c"

struct mkfw_state *window;

#include "platform_opengl.c"

#include "ugg.h"
#include "clip_blit.h"
#include "futurecomposer14.h"
#include "generic_scroller.h"
#include "micromod.h"
#define RAND_IMPLEMENTATION
#include "rand.h"
#define MKS_RESAMPLER_IMPLEMENTATION
#include "resampler.h"

#include "option_selector_1/option_window.c"

static void remake_options(struct options *opt);
static void remake_init(struct mkfw_state *window);
static void remake_frame(struct mkfw_state *window);
static void remake_shutdown(struct mkfw_state *window);

static void framebuffer_callback(struct mkfw_state *window, int32_t width, int32_t height, float aspect_ratio) {
	// state.screen_width = width;
	// state.screen_height = height;

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

	state.viewport.x = viewport_x;
	state.viewport.y = viewport_y;
	state.viewport.w = viewport_width;
	state.viewport.h = viewport_height;

	state.viewport_changed = 1;
}

// [=]===^=[ key_callback ]=================================================================^===[=]
static void key_callback(struct mkfw_state *window, uint32_t key, uint32_t action, uint32_t mods) {

	// remake_key(key, action, mods);

	// if(key == MKS_KEY_ESCAPE) {
	// 	if(action == MKS_PRESSED) {
	// 		mkfw_set_should_close(1);
	// 	}
	// }

	// if(action == MKS_RELEASED) {
	// 	switch(key) {
	// 		// Handle shader CRT emulation toggle

	// 		default: break;
	// 	}
	// }
}

// [=]===^=[ mouse_move_callback ]=================================================================^===[=]
static void mouse_move_callback(int32_t x, int32_t y) {
	state.mouse_dx += x;
	state.mouse_dy += y;
}

// [=]===^=[ mouse_button_callback ]=================================================================^===[=]
static void mouse_button_callback(uint8_t button, int action) {
	// DEBUG_PRINT("mouse_button\n");
}

// [=]===^=[ error_callback ]=================================================================^===[=]
static void error_callback(int error, const char* description) {
	DEBUG_PRINT("Error: %s\n", description);
}

// [=]===^=[ render_thread_func ]=================================================================^===[=]
#ifdef _WIN32
static DWORD WINAPI render_thread_func(LPVOID arg) {
#else
static void *render_thread_func(void *arg) {
#endif
	mkfw_attach_context(window);

	opengl_setup();
	setup_render_targets();
	change_resolution(BUFFER_WIDTH, BUFFER_HEIGHT);

	// Get actual window size (handles both windowed and fullscreen mode)
	int32_t init_w, init_h;
	mkfw_get_framebuffer_size(window, &init_w, &init_h);
	framebuffer_callback(window, init_w, init_h, CRT_ASPECT);

	// profiler_init();
	struct mkfw_timer_handle *timer = mkfw_timer_new(FRAME_TIME_NS);
	uint64_t last_frame_time_ns = mkfw_gettime(window);

	while(state.running) {
		if(mkfw_is_key_pressed(window, MKS_KEY_F11)) {
			state.fullscreen = !state.fullscreen;
			mkfw_fullscreen(window, state.fullscreen);
		}

		if(mkfw_is_key_pressed(window, MKS_KEY_F12)) {
			state.toggle_crt_emulation = !state.toggle_crt_emulation;
		}

		if(mkfw_is_key_pressed(window, MKS_KEY_ESCAPE)) {
			state.running = 0;
		}


		static uint8_t paused = 0;
		if(mkfw_is_key_pressed(window, MKS_KEY_P)) {
			paused = !paused;
		}

		if(!paused) {
			memset(buffer, 0, sizeof(buffer));
			remake_frame(window);
		}

{
		// PROFILE_NAMED("Frame misc");
		mkfw_update_keyboard_state(window);
		mkfw_update_modifier_state(window);
		mkfw_update_mouse_state(window);
		state.frame_number++;
}

		render_frame();
		// profiler_upload();
		mkfw_swap_buffers(window);
		mkfw_timer_wait(timer);

		// uint64_t current_frame_time_ns = mkfw_gettime(window);
		// uint64_t frame_delta_ns = current_frame_time_ns - last_frame_time_ns;
		// last_frame_time_ns = current_frame_time_ns;
		// PROFILE_VALUE("frametime_ns", frame_delta_ns);
	} // mainloop

	mkfw_timer_destroy(timer);
	// profiler_shutdown();

	return 0;
}

// [=]===^=[ main ]=================================================================^===[=]
int main(int argc, char **argv) {
	state.toggle_crt_emulation = 1;

	mkfw_audio_initialize();
	mkfw_timer_init();

	remake_options(&opt);
	opt.fullscreen = state.fullscreen;
	opt.crtshader = state.toggle_crt_emulation;
	if(option_selector(&opt)) {
		mkfw_timer_shutdown();
		return -1;
	}

	state.toggle_crt_emulation = !!(opt.crtshader);
	state.fullscreen = !!(opt.fullscreen);

	int init_w = (int)(WINDOW_WIDTH * SCALE + 0.5f);
	int init_h = (init_w * CRT_ASPECT_DEN) / CRT_ASPECT_NUM;
	window = mkfw_init(init_w, init_h);
	gl_loader();

	mkfw_set_swapinterval(window, 0);
	mkfw_set_key_callback(window, key_callback);
	// mkfw_set_mouse_move_delta_callback(mouse_move_callback);
	// mkfw_set_mouse_button_callback(mouse_button_callback);
	mkfw_set_framebuffer_size_callback(window, framebuffer_callback);

	int min_w = WINDOW_WIDTH * 2;
	int min_h = (min_w * CRT_ASPECT_DEN) / CRT_ASPECT_NUM;
	mkfw_set_window_min_size_and_aspect(window, min_w, min_h, CRT_ASPECT_NUM, CRT_ASPECT_DEN);
	mkfw_set_window_title(window, opt.window_title);
	mkfw_show_window(window);

	mkfw_fullscreen(window, state.fullscreen);

	remake_init(window);

	state.running = 1;

	mkfw_detach_context(window);

#ifdef _WIN32
	HANDLE render_thread = CreateThread(0, 0, render_thread_func, 0, 0, 0);
	if(render_thread) {
#else
	pthread_t render_thread;
	if(!pthread_create(&render_thread, 0, render_thread_func, 0)) {
#endif
		while(state.running && !mkfw_should_close(window)) {
			mkfw_pump_messages(window);
			mkfw_sleep(5000000);
		}
		state.running = 0;
#ifdef _WIN32
		WaitForSingleObject(render_thread, INFINITE);
		CloseHandle(render_thread);
#else
		pthread_join(render_thread, 0);
#endif
	}


	remake_shutdown(window);
	mkfw_cleanup(window);
	mkfw_audio_shutdown();
	mkfw_timer_shutdown();
	return 0;
}


