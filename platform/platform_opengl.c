// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

/*
 * This is a 2-pass shader
 *
 * 1. Phosphor persistence at source resolution (346x270)
 * 2. CRT shader with phosphor masks and scanlines at viewport resolution
 *
 */

#include "shader.c"
#include "shader.h"

INCBIN_SHADER_NOHEADER(vertex_shader,						"#version 330", "shaders/gl_vertex.glsl");
INCBIN_SHADER(fragment_shader,								"#version 330", "shader.h", "shaders/gl_crt_fragment.glsl");
INCBIN_SHADER_NOHEADER(phosphor_persistence_fragment,	"#version 330", "shaders/gl_phosphor_persistence_fragment.glsl");
INCBIN_SHADER_NOHEADER(passthrough_fragment,				"#version 330", "shaders/gl_passthrough_fragment.glsl");

// [=]===^=[ setup_render_target ]================================================================^===[=]
static void setup_render_targets(struct remake_state *state) {

	// Ensure source texture exists (might not if framebuffer_callback called before change_resolution)
	if(state->texture == 0 && state->buffer_width > 0 && state->buffer_height > 0) {
		GLfloat border_color[] = {0.0f, 0.0f, 0.0f, 1.0f};
		glGenTextures(1, &state->texture);
		glBindTexture(GL_TEXTURE_2D, state->texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, state->buffer_width, state->buffer_height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// Ensure persistence textures exist (created in change_resolution, not recreated on viewport change)
	if(state->persistence_texture == 0 && state->buffer_width > 0 && state->buffer_height > 0) {
		glGenTextures(1, &state->persistence_texture);
		glBindTexture(GL_TEXTURE_2D, state->persistence_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, state->buffer_width, state->buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glGenTextures(1, &state->persistence_output_texture);
		glBindTexture(GL_TEXTURE_2D, state->persistence_output_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, state->buffer_width, state->buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glGenFramebuffers(1, &state->persistence_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, state->persistence_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->persistence_output_texture, 0);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			DEBUG_PRINT("Persistence FBO not complete!\n");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

// [=]===^=[ change_resolution ]=================================================================^===[=]
static void change_resolution(struct remake_state *state, uint32_t new_width, uint32_t new_height) {
	state->buffer_width = new_width;
	state->buffer_height = new_height;

	// Recreate source texture
	GLfloat border_color[] = {0.0f, 0.0f, 0.0f, 1.0f};
	glDeleteTextures(1, &state->texture);
	glGenTextures(1, &state->texture);
	glBindTexture(GL_TEXTURE_2D, state->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, state->buffer_width, state->buffer_height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Recreate persistence textures (tied to source resolution)
	glDeleteTextures(1, &state->persistence_texture);
	glDeleteTextures(1, &state->persistence_output_texture);
	glDeleteFramebuffers(1, &state->persistence_fbo);

	glGenTextures(1, &state->persistence_texture);
	glBindTexture(GL_TEXTURE_2D, state->persistence_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, state->buffer_width, state->buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &state->persistence_output_texture);
	glBindTexture(GL_TEXTURE_2D, state->persistence_output_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, state->buffer_width, state->buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &state->persistence_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, state->persistence_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->persistence_output_texture, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		DEBUG_PRINT("Persistence FBO not complete!\n");
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// [=]===^=[ compile_shader ]==============================================================^===[=]
static GLuint compile_shader(GLenum shader_type, const char *shader_source) {
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_source, 0);
	glCompileShader(shader);

	GLint success;
	GLchar info_log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(shader, sizeof(info_log), 0, info_log);
		DEBUG_PRINT("%s shader compilation failed:\n%s\n", (shader_type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment", info_log);
	}
	return shader;
}

// [=]===^=[ opengl_setup ]================================================================^===[=]
static void opengl_setup(struct remake_state *state) {
	gl_loader();
	glEnable(GL_FRAMEBUFFER_SRGB);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	// --- CRT Shader Setup ---
	GLuint main_vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_data);
	GLuint main_fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_data);
	state->shader_program = glCreateProgram();
	glAttachShader(state->shader_program, main_vertex_shader);
	glAttachShader(state->shader_program, main_fragment_shader);
	glBindAttribLocation(state->shader_program, 0, "position");
	glBindAttribLocation(state->shader_program, 1, "texture_coord");
	glLinkProgram(state->shader_program);
	glDeleteShader(main_vertex_shader);
	glDeleteShader(main_fragment_shader);


	// Initialize phosphor persistence parameter
	state->persistence_decay = 0.15f;		// 0.5 = subtle, 0.7 = more noticeable

	// Initialize CRTS shader parameters
	state->contrast = 1.0f;
	state->saturation = 0.0f;
	state->brightness = 1.3f;
	CrtsTone(state->tone_data, state->contrast, state->saturation, INPUT_THIN, INPUT_MASK);

	glUseProgram(state->shader_program);
	state->uniform_resolution = glGetUniformLocation(state->shader_program, "resolution");
	state->uniform_src_image_size = glGetUniformLocation(state->shader_program, "src_image_size");
	state->uniform_brightness = glGetUniformLocation(state->shader_program, "brightness");
	state->uniform_tone = glGetUniformLocation(state->shader_program, "tone_data");
	state->uniform_sampler_location = glGetUniformLocation(state->shader_program, "iChannel0");

	// --- Phosphor Persistence Shader Setup ---
	GLuint persistence_vertex = compile_shader(GL_VERTEX_SHADER, vertex_shader_data);
	GLuint persistence_fragment = compile_shader(GL_FRAGMENT_SHADER, phosphor_persistence_fragment_data);
	state->persistence_program = glCreateProgram();
	glAttachShader(state->persistence_program, persistence_vertex);
	glAttachShader(state->persistence_program, persistence_fragment);
	glBindAttribLocation(state->persistence_program, 0, "position");
	glBindAttribLocation(state->persistence_program, 1, "texture_coord");
	glLinkProgram(state->persistence_program);
	glDeleteShader(persistence_vertex);
	glDeleteShader(persistence_fragment);

	// --- Passthrough Shader Setup ---
	GLuint passthrough_vertex = compile_shader(GL_VERTEX_SHADER, vertex_shader_data);
	GLuint passthrough_fragment = compile_shader(GL_FRAGMENT_SHADER, passthrough_fragment_data);
	state->passthrough_program = glCreateProgram();
	glAttachShader(state->passthrough_program, passthrough_vertex);
	glAttachShader(state->passthrough_program, passthrough_fragment);
	glBindAttribLocation(state->passthrough_program, 0, "position");
	glBindAttribLocation(state->passthrough_program, 1, "texture_coord");
	glLinkProgram(state->passthrough_program);
	glDeleteShader(passthrough_vertex);
	glDeleteShader(passthrough_fragment);

	glUseProgram(state->passthrough_program);
	state->passthrough_uniform_source = glGetUniformLocation(state->passthrough_program, "source");

	glGenVertexArrays(1, &state->vao);
	glGenBuffers(1, &state->vbo);
	glGenBuffers(1, &state->ebo);
	glBindVertexArray(state->vao);
	const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f
	};
	static const unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
	glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

// [=]===^=[ render_frame ]=================================================================^===[=]
__attribute__((always_inline))
static inline void render_frame(struct remake_state *state) {
	// Check if viewport changed and we need to recreate render targets
	if(state->viewport_changed) {
		setup_render_targets(state);
		state->viewport_changed = 0;
	}

	// Upload new frame to texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state->texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, state->buffer_width, state->buffer_height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, state->buffer);

	// Apply phosphor persistence (subtle trails on bright objects)
	// Render: state.texture (current upload) + persistence_texture (previous) -> persistence_output_texture
	glBindFramebuffer(GL_FRAMEBUFFER, state->persistence_fbo);
	glViewport(0, 0, state->buffer_width, state->buffer_height);

	glUseProgram(state->persistence_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state->texture);  // Current frame just uploaded
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, state->persistence_texture);  // Previous frame
	glUniform1i(glGetUniformLocation(state->persistence_program, "current_frame"), 0);
	glUniform1i(glGetUniformLocation(state->persistence_program, "previous_frame"), 1);
	glUniform1f(glGetUniformLocation(state->persistence_program, "decay"), state->persistence_decay);

	glBindVertexArray(state->vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// Copy output back to persistence_texture for next frame
	glBindTexture(GL_TEXTURE_2D, state->persistence_texture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, state->buffer_width, state->buffer_height);

	// Use persistence output as source for rest of rendering pipeline
	GLuint source_texture = state->persistence_output_texture;

	if(state->toggle_crt_emulation) {
		// ========== PASS 2: CRT Shader directly to screen ==========
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(state->shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, source_texture);
		glUniform2f(state->uniform_src_image_size, (float)state->buffer_width, (float)state->buffer_height);
		glUniform2f(state->uniform_resolution, (float)state->viewport.w, (float)state->viewport.h);
		glUniform1f(state->uniform_brightness, state->brightness);
		glUniform4f(state->uniform_tone, state->tone_data[0], state->tone_data[1], state->tone_data[2], state->tone_data[3]);
		glUniform1i(state->uniform_sampler_location, 0);

		glBindVertexArray(state->vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glUseProgram(0);

	} else {
		// ========== Simple passthrough: CRT emulation is OFF ==========
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Simple passthrough - just display the source texture with nearest neighbor (sharp pixels)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, source_texture);

		glUseProgram(state->passthrough_program);
		glUniform1i(state->passthrough_uniform_source, 0);

		glBindVertexArray(state->vao);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glUseProgram(0);
	}
}
