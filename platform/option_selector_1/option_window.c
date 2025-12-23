// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


#define OPTIONS_WINDOW_WIDTH 320
#define OPTIONS_WINDOW_HEIGHT 240
#define OPTIONS_WINDOW_SCALE 2

#define OPTIONS_BUFFER_WIDTH 320
#define OPTIONS_BUFFER_HEIGHT 240

struct option_state {
	struct mkfw_state *window;

	uint32_t buffer[OPTIONS_BUFFER_WIDTH * OPTIONS_BUFFER_HEIGHT];
	uint32_t buffer_width;
	uint32_t buffer_height;

	GLuint texture, vao, vbo;
	GLuint program;

	struct rng_state rand;

	uint8_t running;
	uint8_t result;
};

// Thread synchronization
static struct option_state option_state;

//-----------------------------------------------------------------------------------------------------------------------

#define OPTION_NSTARS  256
#define OPTION_FOCAL   128.0f
#define OPTION_SPEED   0.7f

struct star_ {
	float x, y, z;
};

static struct star_ option_stars[OPTION_NSTARS];

static void option_init_stars(struct option_state *state) {
	for (int i = 0; i < OPTION_NSTARS; ++i) {
		option_stars[i].x = ((float)xor_generate_random(&state->rand) / (float)0xffffffff - 0.5f) * state->buffer_width;
		option_stars[i].y = ((float)xor_generate_random(&state->rand) / (float)0xffffffff - 0.5f) * state->buffer_height;
		option_stars[i].z = 1.0f + ((float)xor_generate_random(&state->rand) / (float)0xffffffff) * 255.0f;
	}
}

static void render_starfield(struct option_state *state) {
	for (int i = 0; i < OPTION_NSTARS; ++i) {
		struct star_ *s = &option_stars[i];
		s->z -= OPTION_SPEED;
		if(s->z <= 1.0f) {
			s->x = ((float)xor_generate_random(&state->rand) / (float)0xffffffff - 0.5f) * state->buffer_width;
			s->y = ((float)xor_generate_random(&state->rand) / (float)0xffffffff - 0.5f) * state->buffer_height;
			s->z = 255.0f;
		}

		float sx = state->buffer_width  / 2.0f + (s->x / s->z) * OPTION_FOCAL;
		float sy = 40.0f  + (s->y / s->z) * OPTION_FOCAL;

		int x = (int)sx;
		int y = (int)sy;
		if(x >= 0 && x < (int)state->buffer_width && y >= 0 && y < (int)state->buffer_height) {
			float t = (255.0f - s->z) / 255.0f;

			uint8_t r = (uint8_t)(0x10 + t * (0xff - 0x20));
			uint8_t g = (uint8_t)(0x20 + t * (0xff - 0x30));
			uint8_t b = (uint8_t)(0x40 + t * (0xff - 0x50));

			state->buffer[y * state->buffer_width + x] = (r << 24) | (g << 16) | (b << 8) | 0xff;
		}
	}
}

INCBIN_UGG(awesome_font, "option_selector_1/data/awesomefont8x8.ugg");

static uint32_t line_colors[] = {
	0xd0d8e8ff, 0xe0e4ecff, 0xf0f2f8ff, 0xf8faffff, 0xf0f2f8ff, 0xe0e4ecff, 0xd0d8e8ff, 0xc0c8d8ff,
};

static uint32_t line_release_colors[] = {
	0xe8d8d0ff, 0xece4e0ff, 0xf8f2f0ff, 0xfffaf8ff, 0xf8f2f0ff, 0xece4e0ff, 0xe8d8d0ff, 0xd8c8c0ff,
};

static void render_text(struct option_state *state, uint32_t x, uint32_t y, char *str, uint32_t *palette) {
	uint32_t *org_dst = state->buffer + y * state->buffer_width + x;

	while(*str) {
		uint8_t c = *str++ - ' ';

		uint8_t *src = awesome_font->data + ((c/20) * 8 * awesome_font->width) + ((c % 20) * 8);

		uint32_t *dst = org_dst;
		for(size_t y = 0; y < 8; ++y) {
			uint32_t color = palette[y];
			for(size_t x = 0; x < 8; ++x) {
				dst[x] = src[x] ? color : dst[x];
			}
			dst += state->buffer_width;
			src += awesome_font->width;
		}
		org_dst += 8;
	}
}

uint32_t chrlen(char *s) {
	uint32_t result = 0;
	while(*s++) {
		result++;
	}
	return result;
}

static void option_opengl_initialize(struct option_state *state) {

	// Create and compile vertex shader
	const char* vertex_shader_source =
		"#version 140\n"
		"in vec2 position;\n"
		"in vec2 texcoord;\n"
		"out vec2 v_texcoord;\n"
		"void main() {\n"
		"    v_texcoord = texcoord;\n"
		"    gl_Position = vec4(position, 0.0, 1.0);\n"
		"}\n";

	// Create and compile fragment shader
	const char* fragment_shader_source =
		"#version 140\n"
		"in vec2 v_texcoord;\n"
		"out vec4 fragColor;\n"
		"uniform sampler2D tex;\n"
		"void main() {\n"
		"    fragColor = texture(tex, v_texcoord);\n"
		"}\n";

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader_source, 0);
	glCompileShader(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader_source, 0);
	glCompileShader(fs);

	state->program = glCreateProgram();
	glAttachShader(state->program, vs);
	glAttachShader(state->program, fs);
	glLinkProgram(state->program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	// Create fullscreen quad
	float vertices[] = {
		// positions   // texcoords
		-1.0f, -1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 0.0f
	};

	glGenVertexArrays(1, &state->vao);
	glGenBuffers(1, &state->vbo);

	glBindVertexArray(state->vao);
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLint pos_attrib = glGetAttribLocation(state->program, "position");
	glEnableVertexAttribArray(pos_attrib);
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	GLint tex_attrib = glGetAttribLocation(state->program, "texcoord");
	glEnableVertexAttribArray(tex_attrib);
	glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// Create texture
	glGenTextures(1, &state->texture);
	glBindTexture(GL_TEXTURE_2D, state->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, state->buffer_width, state->buffer_height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);

	glBindVertexArray(0);
	glUseProgram(state->program);

	// Set viewport to match window size (fixes Windows pixel offset issue)
	glViewport(0, 0, OPTIONS_WINDOW_WIDTH * OPTIONS_WINDOW_SCALE, OPTIONS_WINDOW_HEIGHT * OPTIONS_WINDOW_SCALE);
}

static void option_render(struct option_state *state) {
	glBindTexture(GL_TEXTURE_2D, state->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, state->buffer_width, state->buffer_height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, state->buffer);

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(state->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Render thread function
#ifdef _WIN32
static DWORD WINAPI option_render_thread_func(LPVOID arg) {
#else
static void *option_render_thread_func(void *arg) {
#endif
	struct options *opt = (struct options *)arg;
	struct option_state *state = &option_state;

	mkfw_attach_context(state->window);
	option_opengl_initialize(state);
	option_init_stars(state);

	uint32_t release_group_center_x = (state->buffer_width - (chrlen(opt->release_group) * 8)) >> 1;
	uint32_t release_title_center_x = (state->buffer_width - (chrlen(opt->release_title) * 8)) >> 1;

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

		if(mkfw_is_key_pressed(state->window, MKS_KEY_F1)) {
			opt->fullscreen = !opt->fullscreen;
		}
		if(mkfw_is_key_pressed(state->window, MKS_KEY_F2)) {
			opt->crtshader = !opt->crtshader;
		}

		// Render starfield and UI
		render_starfield(state);
		render_text(state,  68,  16, "MINDKILLER SYSTEMS 2025", line_colors);
		render_text(state, 128,  32, "PRESENTS", line_colors);
		render_text(state, release_group_center_x,  64, opt->release_group, line_release_colors);
		render_text(state, release_title_center_x,  80, opt->release_title, line_release_colors);

		char *fullscreen_string	= (opt->fullscreen)	? "F1 - Unlimited Screen Size ON" : "F1 - Unlimited Screen Size OFF";
		char *crt_string			= (opt->crtshader)	? "F2 - CRT Shader DLC ...... ON" : "F2 - CRT Shader DLC ...... OFF";
		render_text(state,  40, 120, fullscreen_string, line_colors);
		render_text(state,  40, 136, crt_string, line_colors);

		render_text(state,  80, 162, "PRESS SPACE TO START", line_colors);
		render_text(state,  48, 178, "(F11/F12 TOGGLE DURING DEMO)", line_colors);

		render_text(state,  24, 216, "REMAKE BY VITAL/MINDKILLER SYSTEMS", line_colors);

		// Update input state
		mkfw_update_keyboard_state(state->window);
		mkfw_update_modifier_state(state->window);
		mkfw_update_mouse_state(state->window);

		// Render and swap buffers
		option_render(state);
		mkfw_swap_buffers(state->window);

		mkfw_timer_wait(timer);
	}

	mkfw_timer_destroy(timer);
	return 0;
}

INCBIN_BYTES(billy_the_kid_music, "option_selector_1/data/billy the kid.fc");
static struct fc14_state option_song;

static void option_audio(int16_t *audio_buffer, size_t frames) {
	fc14_get_audio(&option_song, audio_buffer, frames);
};

static uint8_t option_selector(struct options *opt) {
	fc14_initialize(&option_song, (const uint8_t*)billy_the_kid_music, billy_the_kid_music_end - billy_the_kid_music_data, 48000);

	xor_init_rng(&option_state.rand, 187481201);

	option_state.buffer_width = OPTIONS_BUFFER_WIDTH;
	option_state.buffer_height = OPTIONS_BUFFER_HEIGHT;

	uint32_t width = OPTIONS_WINDOW_WIDTH * OPTIONS_WINDOW_SCALE;
	uint32_t height = OPTIONS_WINDOW_HEIGHT * OPTIONS_WINDOW_SCALE;
	option_state.window = mkfw_init(width, height);
	gl_loader();

	mkfw_set_swapinterval(option_state.window, 0);
	mkfw_set_window_min_size_and_aspect(option_state.window, width, height, width, height);
	mkfw_set_window_resizable(option_state.window, 0);
	mkfw_set_window_title(option_state.window, "remake options");
	mkfw_show_window(option_state.window);

	option_state.running = 1;

	mkfw_audio_callback = option_audio;
	mkfw_detach_context(option_state.window);

#ifdef _WIN32
	HANDLE render_thread = CreateThread(0, 0, option_render_thread_func, opt, 0, 0);
	if(render_thread) {
#else
	pthread_t render_thread;
	if(!pthread_create(&render_thread, 0, option_render_thread_func, opt)) {
#endif
		while(option_state.running) {
			mkfw_pump_messages(option_state.window);
			mkfw_sleep(5000000);
		}

		option_state.running = 0;
#ifdef _WIN32
		WaitForSingleObject(render_thread, INFINITE);
		CloseHandle(render_thread);
#else
		pthread_join(render_thread, 0);
#endif
	}

	mkfw_audio_callback = 0;
	fc14_shutdown(&option_song);
	mkfw_cleanup(option_state.window);
	return option_state.result;
}
