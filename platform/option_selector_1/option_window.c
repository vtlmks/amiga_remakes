// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


#define OPTIONS_WINDOW_WIDTH 320
#define OPTIONS_WINDOW_HEIGHT 240
#define OPTIONS_WINDOW_SCALE 2

#define OPTIONS_BUFFER_WIDTH 320
#define OPTIONS_BUFFER_HEIGHT 240

static uint32_t options_buffer[OPTIONS_BUFFER_WIDTH * OPTIONS_BUFFER_HEIGHT];	// GL_SRGB8_ALPHA8

struct option_ {
	GLuint texture, vao, vbo;
	GLuint program;
};

struct option_ option_state;

// Thread synchronization
static uint8_t option_running;
static uint8_t option_result;
static struct mkfw_state *option_window;
struct rng_state option_rand;

static void option_opengl_initialize(void) {

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

	option_state.program = glCreateProgram();
	glAttachShader(option_state.program, vs);
	glAttachShader(option_state.program, fs);
	glLinkProgram(option_state.program);

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

	glGenVertexArrays(1, &option_state.vao);
	glGenBuffers(1, &option_state.vbo);

	glBindVertexArray(option_state.vao);
	glBindBuffer(GL_ARRAY_BUFFER, option_state.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLint pos_attrib = glGetAttribLocation(option_state.program, "position");
	glEnableVertexAttribArray(pos_attrib);
	glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	GLint tex_attrib = glGetAttribLocation(option_state.program, "texcoord");
	glEnableVertexAttribArray(tex_attrib);
	glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// Create texture
	glGenTextures(1, &option_state.texture);
	glBindTexture(GL_TEXTURE_2D, option_state.texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, OPTIONS_BUFFER_WIDTH, OPTIONS_BUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);

	glBindVertexArray(0);
	glUseProgram(option_state.program);

	// Set viewport to match window size (fixes Windows pixel offset issue)
	glViewport(0, 0, OPTIONS_WINDOW_WIDTH * OPTIONS_WINDOW_SCALE, OPTIONS_WINDOW_HEIGHT * OPTIONS_WINDOW_SCALE);
}

static void option_render(void) {
	glBindTexture(GL_TEXTURE_2D, option_state.texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, OPTIONS_BUFFER_WIDTH, OPTIONS_BUFFER_HEIGHT, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, options_buffer);

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(option_state.vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//-----------------------------------------------------------------------------------------------------------------------

#define OPTION_NSTARS  256
#define OPTION_FOCAL   128.0f
#define OPTION_SPEED   0.7f

struct star_ {
	float x, y, z;
};

static struct star_ option_stars[OPTION_NSTARS];

static void init_stars(void) {
	for (int i = 0; i < OPTION_NSTARS; ++i) {
		option_stars[i].x = ((float)xor_generate_random(&option_rand) / (float)0xffffffff - 0.5f) * OPTIONS_BUFFER_WIDTH;
		option_stars[i].y = ((float)xor_generate_random(&option_rand) / (float)0xffffffff - 0.5f) * OPTIONS_BUFFER_HEIGHT;
		option_stars[i].z = 1.0f + ((float)xor_generate_random(&option_rand) / (float)0xffffffff) * 255.0f;
	}
}

static void render_starfield(void) {
	for (int i = 0; i < OPTION_NSTARS; ++i) {
		struct star_ *s = &option_stars[i];
		s->z -= OPTION_SPEED;
		if(s->z <= 1.0f) {
			s->x = ((float)xor_generate_random(&option_rand) / (float)0xffffffff - 0.5f) * OPTIONS_BUFFER_WIDTH;
			s->y = ((float)xor_generate_random(&option_rand) / (float)0xffffffff - 0.5f) * OPTIONS_BUFFER_HEIGHT;
			s->z = 255.0f;
		}

		float sx = OPTIONS_BUFFER_WIDTH  / 2.0f + (s->x / s->z) * OPTION_FOCAL;
		float sy = 40.0f  + (s->y / s->z) * OPTION_FOCAL;

		int x = (int)sx;
		int y = (int)sy;
		if(x >= 0 && x < OPTIONS_BUFFER_WIDTH && y >= 0 && y < OPTIONS_BUFFER_HEIGHT) {
			float t = (255.0f - s->z) / 255.0f;

			uint8_t r = (uint8_t)(0x10 + t * (0xff - 0x20));
			uint8_t g = (uint8_t)(0x20 + t * (0xff - 0x30));
			uint8_t b = (uint8_t)(0x40 + t * (0xff - 0x50));

			options_buffer[y * OPTIONS_BUFFER_WIDTH + x] = (r << 24) | (g << 16) | (b << 8) | 0xff;
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

static void render_text(uint32_t x, uint32_t y, char *str, uint32_t *palette) {
	uint32_t *org_dst = options_buffer + y * OPTIONS_BUFFER_WIDTH + x;

	while(*str) {
		uint8_t c = *str++ - ' ';

		uint8_t *src = awesome_font->data + ((c/20) * 8 * awesome_font->width) + ((c % 20) * 8);

		uint32_t *dst = org_dst;
		for(size_t y = 0; y < 8; ++y) {
			uint32_t color = palette[y];
			for(size_t x = 0; x < 8; ++x) {
				dst[x] = src[x] ? color : dst[x];
			}
			dst += OPTIONS_BUFFER_WIDTH;
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

// Render thread function
#ifdef _WIN32
static DWORD WINAPI option_render_thread_func(LPVOID arg) {
#else
static void *option_render_thread_func(void *arg) {
#endif
	struct options *opt = (struct options *)arg;

	mkfw_attach_context(option_window);
	option_opengl_initialize();
	init_stars();

	// opt->release_group = "ALPHA FLIGHT";
	// opt->release_title = "DR.MABUSE FIRST INTRO";

	uint32_t release_group_center_x = (OPTIONS_BUFFER_WIDTH - (chrlen(opt->release_group) * 8)) >> 1;
	uint32_t release_title_center_x = (OPTIONS_BUFFER_WIDTH - (chrlen(opt->release_title) * 8)) >> 1;

	struct mkfw_timer_handle *timer = mkfw_timer_new(FRAME_TIME_NS);

	while(option_running) {
		for(size_t i = 0; i < OPTIONS_BUFFER_HEIGHT * OPTIONS_BUFFER_WIDTH; ++i) {
			options_buffer[i] = 0x102040ff;
		}

		if(mkfw_is_key_pressed(option_window, MKS_KEY_ESCAPE)) {
			option_running = 0;
			option_result = 1;
		}

		if(mkfw_should_close(option_window)) {
			option_running = 0;
			option_result = 1;
		}

		// NOTE(peter): only exit 'true' if mouse clicked 'start' or space was hit
		if(mkfw_is_key_pressed(option_window, MKS_KEY_SPACE)) {
			option_running = 0;
		}

		if(mkfw_is_key_pressed(option_window, MKS_KEY_F1)) {
			opt->fullscreen = !opt->fullscreen;
		}
		if(mkfw_is_key_pressed(option_window, MKS_KEY_F2)) {
			opt->crtshader = !opt->crtshader;
		}

		// Render starfield and UI
		render_starfield();
		render_text( 68,  16, "MINDKILLER SYSTEMS 2025", line_colors);
		render_text(128,  32, "PRESENTS", line_colors);
		render_text(release_group_center_x,  64, opt->release_group, line_release_colors);
		render_text(release_title_center_x,  80, opt->release_title, line_release_colors);

		char *fullscreen_string	= (opt->fullscreen)	? "F1 - Unlimited Screen Size ON" : "F1 - Unlimited Screen Size OFF";
		char *crt_string			= (opt->crtshader)	? "F2 - CRT Shader DLC ...... ON" : "F2 - CRT Shader DLC ...... OFF";
		render_text( 40, 120, fullscreen_string, line_colors);
		render_text( 40, 136, crt_string, line_colors);

		render_text( 80, 162, "PRESS SPACE TO START", line_colors);
		render_text( 48, 178, "(F11/F12 TOGGLE DURING DEMO)", line_colors);

		render_text( 24, 216, "REMAKE BY VITAL/MINDKILLER SYSTEMS", line_colors);

		// Update input state
		mkfw_update_keyboard_state(option_window);
		mkfw_update_modifier_state(option_window);
		mkfw_update_mouse_state(option_window);

		// Render and swap buffers
		option_render();
		mkfw_swap_buffers(option_window);

		mkfw_timer_wait(timer);
	}

	mkfw_timer_destroy(timer);
	return 0;
}

// INCBIN_BYTES(antipasti, "option_selector_1/data/antipasti#34.mod");
// struct micromod_state antipasti_song;

INCBIN_BYTES(billy_the_kid_music, "option_selector_1/data/billy the kid.fc");
static struct fc14_state option_song;

static void option_audio(int16_t *audio_buffer, size_t frames) {
	// micromod_get_audio(&antipasti_song, audio_buffer, frames);
	fc14_get_audio(&option_song, audio_buffer, frames);

	// for(size_t i = 0; i < frames; i++) {
	// 	int32_t old_left = (int32_t)audio_buffer[i * 2];
	// 	int32_t old_right = (int32_t)audio_buffer[i * 2 + 1];

	// 	int32_t mixed_left = old_left + (old_right * 3) / 4;
	// 	int32_t mixed_right = old_right + (old_left * 3) / 4;

	// 	// Shift right by 1 to prevent clipping and scale down
	// 	audio_buffer[i * 2] = (int16_t)(mixed_left >> 1);
	// 	audio_buffer[i * 2 + 1] = (int16_t)(mixed_right >> 1);
	// }
};

static uint8_t option_selector(struct options *opt) {
	// micromod_initialize(&antipasti_song, (int8_t*)antipasti_data,		48000);
	fc14_initialize(&option_song, (const uint8_t*)billy_the_kid_music, billy_the_kid_music_end - billy_the_kid_music_data, 48000);

	xor_init_rng(&option_rand, 187481201);


	uint32_t width = OPTIONS_WINDOW_WIDTH * OPTIONS_WINDOW_SCALE;
	uint32_t height = OPTIONS_WINDOW_HEIGHT * OPTIONS_WINDOW_SCALE;
	option_window = mkfw_init(width, height);
	gl_loader();

	mkfw_set_swapinterval(option_window, 0);
	mkfw_set_window_min_size_and_aspect(option_window, width, height, width, height);
	mkfw_set_window_resizable(option_window, 0);
	mkfw_set_window_title(option_window, "remake options");
	mkfw_show_window(option_window);


	option_running = 1;

	mkfw_audio_callback = option_audio;
	// Detach context from main thread before creating render thread
	mkfw_detach_context(option_window);

	// Create render thread
#ifdef _WIN32
	HANDLE render_thread = CreateThread(0, 0, option_render_thread_func, opt, 0, 0);
	if(render_thread) {
#else
	pthread_t render_thread;
	if(!pthread_create(&render_thread, 0, option_render_thread_func, opt)) {
#endif
		// Main thread handles window messages
		while(option_running) {
			mkfw_pump_messages(option_window);
			mkfw_sleep(5000000); // Sleep 5ms to avoid busy-waiting
		}

		// Signal render thread to stop and wait for it
		option_running = 0;
#ifdef _WIN32
		WaitForSingleObject(render_thread, INFINITE);
		CloseHandle(render_thread);
#else
		pthread_join(render_thread, 0);
#endif
	}

	mkfw_audio_callback = 0;
	fc14_shutdown(&option_song);
	mkfw_cleanup(option_window);
	return option_result;
}
