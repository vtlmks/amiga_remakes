// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// The including file must define these before #include:
//   OPTIONS_BUFFER_WIDTH
//   OPTIONS_BUFFER_HEIGHT
//   OPTIONS_WINDOW_SCALE

#define OPTIONS_WINDOW_WIDTH  OPTIONS_BUFFER_WIDTH
#define OPTIONS_WINDOW_HEIGHT OPTIONS_BUFFER_HEIGHT

struct option_state {
	struct mkfw_state *window;

	uint32_t buffer[OPTIONS_BUFFER_WIDTH * OPTIONS_BUFFER_HEIGHT];
	uint32_t buffer_width;
	uint32_t buffer_height;

	GLuint texture, vao, vbo, ebo;
	GLuint program;

	struct rng_state rand;

	uint8_t running;
	uint8_t result;
};

static struct option_state option_state;

// Forward declarations - implemented by each concrete selector
static void option_setup(struct option_state *state, struct platform_state *pstate);
static void option_init(struct option_state *state, struct platform_state *pstate);
static void option_frame(struct option_state *state, struct platform_state *pstate);
static void option_shutdown(struct option_state *state, struct platform_state *pstate);

// [=]===^=[ option_opengl_initialize ]================================================================^===[=]
static void option_opengl_initialize(struct option_state *state) {
	const char *vs_src =
		"#version 140\n"
		"in vec2 position;\n"
		"in vec2 texture_coord;\n"
		"out vec2 v_texcoord;\n"
		"void main() {\n"
		"	v_texcoord = texture_coord;\n"
		"	gl_Position = vec4(position, 0.0, 1.0);\n"
		"}\n";

	const char *fs_src =
		"#version 140\n"
		"in vec2 v_texcoord;\n"
		"out vec4 fragColor;\n"
		"uniform sampler2D tex;\n"
		"void main() {\n"
		"	fragColor = texture(tex, vec2(v_texcoord.x, 1.0 - v_texcoord.y));\n"
		"}\n";

	state->program = opengl_link_program(vs_src, fs_src);
	state->texture = opengl_create_texture(state->buffer_width, state->buffer_height, GL_UNSIGNED_INT_8_8_8_8, 0);
	opengl_create_fullscreen_quad(&state->vao, &state->vbo, &state->ebo);

	glUseProgram(state->program);
	glViewport(0, 0, OPTIONS_WINDOW_WIDTH * OPTIONS_WINDOW_SCALE, OPTIONS_WINDOW_HEIGHT * OPTIONS_WINDOW_SCALE);
}

// [=]===^=[ option_render ]================================================================^===[=]
static void option_render(struct option_state *state) {
	glBindTexture(GL_TEXTURE_2D, state->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, state->buffer_width, state->buffer_height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, state->buffer);

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(state->vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// [=]===^=[ option_render_thread_func ]================================================================^===[=]
static MKFW_THREAD_FUNC(option_render_thread_func, arg) {
	struct platform_state *pstate = (struct platform_state *)arg;
	struct option_state *state = &option_state;

	mkfw_attach_context(state->window);
	option_opengl_initialize(state);
	option_init(state, pstate);

	struct mkfw_timer_handle *timer = mkfw_timer_new(FRAME_TIME_NS);

	while(state->running) {
		for(size_t i = 0; i < state->buffer_height * state->buffer_width; ++i) {
			state->buffer[i] = 0x102040ff;
		}

		if(mkfw_is_key_pressed(state->window, MKS_KEY_ESCAPE)) {
			state->running = 0;
			state->result = 1;
		}

		if(mkfw_should_close(state->window)) {
			state->running = 0;
			state->result = 1;
		}

		if(mkfw_is_key_pressed(state->window, MKS_KEY_SPACE)) {
			state->running = 0;
		}

		option_frame(state, pstate);

		mkfw_update_input_state(state->window);

		option_render(state);
		mkfw_swap_buffers(state->window);

		mkfw_timer_wait(timer);
	}

	mkfw_timer_destroy(timer);
	return 0;
}

// [=]===^=[ option_selector ]================================================================^===[=]
static uint8_t option_selector(struct platform_state *pstate) {
	xor_init_rng(&option_state.rand, 187481201);

	option_state.buffer_width = OPTIONS_BUFFER_WIDTH;
	option_state.buffer_height = OPTIONS_BUFFER_HEIGHT;

	uint32_t width = OPTIONS_WINDOW_WIDTH * OPTIONS_WINDOW_SCALE;
	uint32_t height = OPTIONS_WINDOW_HEIGHT * OPTIONS_WINDOW_SCALE;
	option_state.window = mkfw_init(width, height);
	opengl_function_loader();

	mkfw_set_swapinterval(option_state.window, 0);
	mkfw_set_window_min_size_and_aspect(option_state.window, width, height, width, height);
	mkfw_set_window_resizable(option_state.window, 0);
	mkfw_set_window_title(option_state.window, "remake options");
	mkfw_show_window(option_state.window);

	option_state.running = 1;

	option_setup(&option_state, pstate);
	mkfw_detach_context(option_state.window);

	mkfw_thread render_thread = mkfw_thread_create(option_render_thread_func, pstate);
	if(render_thread) {
		while(option_state.running) {
			mkfw_pump_messages(option_state.window);
			mkfw_sleep(5000000);
		}

		option_state.running = 0;
		mkfw_thread_join(render_thread);
	}

	option_shutdown(&option_state, pstate);
	mkfw_cleanup(option_state.window);
	return option_state.result;
}
