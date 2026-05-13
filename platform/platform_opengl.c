// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

/*
 * 2-pass rendering pipeline:
 *
 * 1. Phosphor persistence at source resolution
 * 2. CRT shader at viewport resolution
 *
 */

#include "opengl_shader.c"
#include "opengl_shader.h"

INCBIN_SHADER_NOHEADER(vertex_shader,						"#version 330", "shaders/gl_vertex.glsl");
INCBIN_SHADER(fragment_shader,								"#version 330", "opengl_shader.h", "shaders/gl_crt_fragment.glsl");
INCBIN_SHADER_NOHEADER(phosphor_persistence_fragment,	"#version 330", "shaders/gl_phosphor_persistence_fragment.glsl");
INCBIN_SHADER_NOHEADER(passthrough_fragment,				"#version 330", "shaders/gl_passthrough_fragment.glsl");
INCBIN_SHADER_NOHEADER(upscale_warp_fragment,			"#version 330", "shaders/gl_upscale_warp_fragment.glsl");
INCBIN_SHADER_NOHEADER(bloom_downsample_fragment,		"#version 330", "shaders/gl_bloom_downsample_fragment.glsl");
INCBIN_SHADER_NOHEADER(bloom_upsample_fragment,			"#version 330", "shaders/gl_bloom_upsample_fragment.glsl");
INCBIN_SHADER(bloom_composite_fragment,					"#version 330", "opengl_shader.h", "shaders/gl_bloom_composite_fragment.glsl");

// [=]===^=[ opengl_compile_shader ]==============================================================^===[=]
static GLuint opengl_compile_shader(GLenum shader_type, const char *shader_source) {
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &shader_source, 0);
	glCompileShader(shader);

	GLint success;
	GLchar info_log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success) {
		glGetShaderInfoLog(shader, sizeof(info_log), 0, info_log);
		mkfw_error("%s shader compilation failed: %s", (shader_type == GL_VERTEX_SHADER) ? "vertex" : "fragment", info_log);
	}
	return shader;
}

// [=]===^=[ opengl_create_texture ]===================================================================^===[=]
static GLuint opengl_create_texture(int w, int h, GLenum pixel_type, GLfloat *border_color) {
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, pixel_type, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if(border_color) {
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	return tex;
}

// [=]===^=[ opengl_link_program ]=====================================================================^===[=]
static GLuint opengl_link_program(const char *vertex_src, const char *fragment_src) {
	GLuint vs = opengl_compile_shader(GL_VERTEX_SHADER, vertex_src);
	GLuint fs = opengl_compile_shader(GL_FRAGMENT_SHADER, fragment_src);
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "position");
	glBindAttribLocation(prog, 1, "texture_coord");
	glLinkProgram(prog);
	glDeleteShader(vs);
	glDeleteShader(fs);
	return prog;
}

// [=]===^=[ opengl_create_fullscreen_quad ]================================================================^===[=]
static void opengl_create_fullscreen_quad(GLuint *vao, GLuint *vbo, GLuint *ebo) {
	float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f
	};
	unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

	glGenVertexArrays(1, vao);
	glGenBuffers(1, vbo);
	glGenBuffers(1, ebo);
	glBindVertexArray(*vao);

	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

// [=]===^=[ opengl_setup_render_targets ]================================================================^===[=]
static void opengl_setup_render_targets(struct platform_state *state) {
	int w = state->buffer_width;
	int h = state->buffer_height;

	if(w == 0 || h == 0) {
		return;
	}

	// Ensure source texture exists (might not if framebuffer_callback called before change_resolution)
	if(state->texture == 0) {
		GLfloat border_color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		state->texture = opengl_create_texture(w, h, GL_UNSIGNED_INT_8_8_8_8, border_color);
	}

	// Ensure persistence textures exist (created in change_resolution, not recreated on viewport change)
	if(state->persistence_texture == 0) {
		state->persistence_texture = opengl_create_texture(w, h, GL_UNSIGNED_BYTE, 0);
		state->persistence_output_texture = opengl_create_texture(w, h, GL_UNSIGNED_BYTE, 0);

		glGenFramebuffers(1, &state->persistence_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, state->persistence_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->persistence_output_texture, 0);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			mkfw_error("persistence FBO not complete");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Viewport-dependent targets - recreated on every viewport change.
	int vw = state->viewport.w;
	int vh = state->viewport.h;
	if(vw <= 0 || vh <= 0) {
		return;
	}

	glDeleteTextures(1, &state->crt_output_texture);
	glDeleteTextures(1, &state->upscaled_source_texture);
	glDeleteTextures(BLOOM_MIP_COUNT, state->bloom_mip_texture);
	glDeleteFramebuffers(1, &state->crt_fbo);
	glDeleteFramebuffers(1, &state->upscaled_source_fbo);
	glDeleteFramebuffers(BLOOM_MIP_COUNT, state->bloom_mip_fbo);

	// CRT output at full viewport (sRGB so writes get gamma-encoded properly)
	glGenTextures(1, &state->crt_output_texture);
	glBindTexture(GL_TEXTURE_2D, state->crt_output_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, vw, vh, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &state->crt_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, state->crt_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->crt_output_texture, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		mkfw_error("CRT FBO not complete");
	}

	// Upscaled+warped source at viewport resolution (feeds the bloom extract)
	glGenTextures(1, &state->upscaled_source_texture);
	glBindTexture(GL_TEXTURE_2D, state->upscaled_source_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, vw, vh, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &state->upscaled_source_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, state->upscaled_source_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->upscaled_source_texture, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		mkfw_error("upscaled source FBO not complete");
	}

	// Bloom mip chain - half-float so HDR bright-pass values survive the chain.
	// Each level is half the size of the previous; mip[0] is half the viewport.
	uint32_t mw = vw / 2;
	uint32_t mh = vh / 2;
	glGenTextures(BLOOM_MIP_COUNT, state->bloom_mip_texture);
	glGenFramebuffers(BLOOM_MIP_COUNT, state->bloom_mip_fbo);
	for(uint32_t i = 0; i < BLOOM_MIP_COUNT; ++i) {
		if(mw < 1) mw = 1;
		if(mh < 1) mh = 1;
		state->bloom_mip_w[i] = mw;
		state->bloom_mip_h[i] = mh;
		glBindTexture(GL_TEXTURE_2D, state->bloom_mip_texture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mw, mh, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, state->bloom_mip_fbo[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->bloom_mip_texture[i], 0);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			mkfw_error("bloom mip FBO not complete");
		}
		mw /= 2;
		mh /= 2;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// [=]===^=[ platform_change_resolution ]=================================================================^===[=]
static void platform_change_resolution(struct platform_state *state, uint32_t new_width, uint32_t new_height) {
	state->buffer_width = new_width;
	state->buffer_height = new_height;
	int w = state->buffer_width;
	int h = state->buffer_height;

	// Recreate source texture
	GLfloat border_color[] = {0.0f, 0.0f, 0.0f, 1.0f};
	glDeleteTextures(1, &state->texture);
	state->texture = opengl_create_texture(w, h, GL_UNSIGNED_INT_8_8_8_8, border_color);

	// Recreate persistence textures (tied to source resolution)
	glDeleteTextures(1, &state->persistence_texture);
	glDeleteTextures(1, &state->persistence_output_texture);
	glDeleteFramebuffers(1, &state->persistence_fbo);

	state->persistence_texture = opengl_create_texture(w, h, GL_UNSIGNED_BYTE, 0);
	state->persistence_output_texture = opengl_create_texture(w, h, GL_UNSIGNED_BYTE, 0);

	glGenFramebuffers(1, &state->persistence_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, state->persistence_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->persistence_output_texture, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		mkfw_error("persistence FBO not complete");
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	state->viewport_changed = 1;
}

// [=]===^=[ opengl_create_phosphor_mask_texture ]=====================================================^===[=]
static GLuint opengl_create_phosphor_mask_texture(float dark) {
	// 6x1 shadow mask pattern: RR GG BB (2 pixels per color channel)
	// Uses linear RGB values (not sRGB) since these are multiplicative factors
	uint8_t d = (uint8_t)(dark * 255.0f);
	uint8_t pixels[6 * 3] = {
		255, d,   d,	// R
		255, d,   d,	// R
		d,   255, d,	// G
		d,   255, d,	// G
		d,   d,   255,	// B
		d,   d,   255,	// B
	};

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 6, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return tex;
}

// [=]===^=[ opengl_setup ]================================================================^===[=]
static void opengl_setup(struct platform_state *state) {
	opengl_function_loader();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	state->shader_program = opengl_link_program(vertex_shader_data, fragment_shader_data);
	state->persistence_program = opengl_link_program(vertex_shader_data, phosphor_persistence_fragment_data);
	state->passthrough_program = opengl_link_program(vertex_shader_data, passthrough_fragment_data);
	state->upscale_warp_program = opengl_link_program(vertex_shader_data, upscale_warp_fragment_data);
	state->bloom_downsample_program = opengl_link_program(vertex_shader_data, bloom_downsample_fragment_data);
	state->bloom_upsample_program = opengl_link_program(vertex_shader_data, bloom_upsample_fragment_data);
	state->bloom_composite_program = opengl_link_program(vertex_shader_data, bloom_composite_fragment_data);

	state->persistence_decay = 0.07f;

	state->contrast = 1.0f;
	state->saturation = 0.0f;
	state->brightness = 1.3f;
	opengl_shader_crts_tone(state->tone_data, state->contrast, state->saturation, INPUT_THIN, INPUT_MASK);

	// Cache CRT uniforms
	glUseProgram(state->shader_program);
	state->uniform_resolution = glGetUniformLocation(state->shader_program, "resolution");
	state->uniform_src_image_size = glGetUniformLocation(state->shader_program, "src_image_size");
	state->uniform_brightness = glGetUniformLocation(state->shader_program, "brightness");
	state->uniform_tone = glGetUniformLocation(state->shader_program, "tone_data");
	state->uniform_sampler_location = glGetUniformLocation(state->shader_program, "iChannel0");
	state->uniform_use_mask_texture = glGetUniformLocation(state->shader_program, "use_mask_texture");
	state->uniform_apply_mask = glGetUniformLocation(state->shader_program, "apply_mask");
	state->uniform_mask_sampler = glGetUniformLocation(state->shader_program, "mask_sampler");

	// Create phosphor mask texture
	state->mask_texture = opengl_create_phosphor_mask_texture(INPUT_MASK);

	glUseProgram(state->passthrough_program);
	state->passthrough_uniform_source = glGetUniformLocation(state->passthrough_program, "source");

	// Cache bloom uniforms
	glUseProgram(state->bloom_downsample_program);
	state->downsample_uniform_source = glGetUniformLocation(state->bloom_downsample_program, "source");
	state->downsample_uniform_texel_size = glGetUniformLocation(state->bloom_downsample_program, "src_texel_size");
	state->downsample_uniform_first_pass = glGetUniformLocation(state->bloom_downsample_program, "first_pass");
	state->downsample_uniform_threshold = glGetUniformLocation(state->bloom_downsample_program, "threshold");
	state->downsample_uniform_knee = glGetUniformLocation(state->bloom_downsample_program, "knee");
	state->downsample_uniform_halation_power = glGetUniformLocation(state->bloom_downsample_program, "halation_power");

	glUseProgram(state->bloom_upsample_program);
	state->upsample_uniform_source = glGetUniformLocation(state->bloom_upsample_program, "source");
	state->upsample_uniform_texel_size = glGetUniformLocation(state->bloom_upsample_program, "src_texel_size");
	state->upsample_uniform_intensity = glGetUniformLocation(state->bloom_upsample_program, "intensity");

	glUseProgram(state->bloom_composite_program);
	state->composite_uniform_crt_sampler = glGetUniformLocation(state->bloom_composite_program, "crt_texture");
	state->composite_uniform_bloom_sampler = glGetUniformLocation(state->bloom_composite_program, "bloom_texture");
	state->composite_uniform_bloom_strength = glGetUniformLocation(state->bloom_composite_program, "bloom_strength");
	state->composite_uniform_viewport_size = glGetUniformLocation(state->bloom_composite_program, "viewport_size");

	// Bloom defaults - see mknes platform_opengl.c for the tuning rationale.
	state->bloom_threshold = 0.75f;
	state->bloom_knee = 0.15f;
	state->bloom_strength = 0.10f;
	state->bloom_upsample_intensity = 0.5f;
	state->bloom_halation_power = 1.0f;

	opengl_create_fullscreen_quad(&state->vao, &state->vbo, &state->ebo);
}

// [=]===^=[ opengl_render_frame ]=================================================================^===[=]
__attribute__((always_inline))
static inline void opengl_render_frame(struct platform_state *state) {
	// Check if viewport changed and we need to recreate render targets
	if(__atomic_load_n(&state->viewport_changed, __ATOMIC_ACQUIRE)) {
		opengl_setup_render_targets(state);
		__atomic_store_n(&state->viewport_changed, 0, __ATOMIC_RELAXED);
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

	glBindVertexArray(state->vao);

	if(state->toggle_crt_emulation && state->toggle_bloom) {
		// ========== PASS: Upscale + warp source (matches CRT geometry) ==========
		glBindFramebuffer(GL_FRAMEBUFFER, state->upscaled_source_fbo);
		glViewport(0, 0, state->viewport.w, state->viewport.h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(state->upscale_warp_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, state->persistence_output_texture);
		// Linear filter while sampling source for the warp; restore nearest after.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glUniform1i(glGetUniformLocation(state->upscale_warp_program, "source"), 0);
		glUniform2f(glGetUniformLocation(state->upscale_warp_program, "resolution"), (float)state->viewport.w, (float)state->viewport.h);
		glUniform2f(glGetUniformLocation(state->upscale_warp_program, "src_image_size"), (float)state->buffer_width, (float)state->buffer_height);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindTexture(GL_TEXTURE_2D, state->persistence_output_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// ========== PASSES: Bloom downsample chain ==========
		// First pass applies the soft-knee bright-pass + Karis anti-firefly
		// weighting; subsequent passes do a plain 13-tap Karis box downsample.
		glUseProgram(state->bloom_downsample_program);
		glUniform1i(state->downsample_uniform_source, 0);
		glActiveTexture(GL_TEXTURE0);

		for(uint32_t i = 0; i < BLOOM_MIP_COUNT; ++i) {
			uint32_t src_w, src_h;
			GLuint src_tex;
			if(i == 0) {
				src_w = state->viewport.w;
				src_h = state->viewport.h;
				src_tex = state->upscaled_source_texture;
			} else {
				src_w = state->bloom_mip_w[i - 1];
				src_h = state->bloom_mip_h[i - 1];
				src_tex = state->bloom_mip_texture[i - 1];
			}

			glBindFramebuffer(GL_FRAMEBUFFER, state->bloom_mip_fbo[i]);
			glViewport(0, 0, state->bloom_mip_w[i], state->bloom_mip_h[i]);
			glBindTexture(GL_TEXTURE_2D, src_tex);
			glUniform2f(state->downsample_uniform_texel_size, 1.0f / (float)src_w, 1.0f / (float)src_h);
			glUniform1i(state->downsample_uniform_first_pass, (i == 0) ? 1 : 0);
			if(i == 0) {
				glUniform1f(state->downsample_uniform_threshold, state->bloom_threshold);
				glUniform1f(state->downsample_uniform_knee, state->bloom_knee);
				glUniform1f(state->downsample_uniform_halation_power, state->bloom_halation_power);
			}
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		// ========== PASSES: Bloom upsample chain (additive blend) ==========
		glBlendFunc(GL_ONE, GL_ONE);
		glUseProgram(state->bloom_upsample_program);
		glUniform1i(state->upsample_uniform_source, 0);
		glUniform1f(state->upsample_uniform_intensity, state->bloom_upsample_intensity);

		for(int32_t i = BLOOM_MIP_COUNT - 1; i > 0; --i) {
			uint32_t src_w = state->bloom_mip_w[i];
			uint32_t src_h = state->bloom_mip_h[i];
			glBindFramebuffer(GL_FRAMEBUFFER, state->bloom_mip_fbo[i - 1]);
			glViewport(0, 0, state->bloom_mip_w[i - 1], state->bloom_mip_h[i - 1]);
			glBindTexture(GL_TEXTURE_2D, state->bloom_mip_texture[i]);
			glUniform2f(state->upsample_uniform_texel_size, 1.0f / (float)src_w, 1.0f / (float)src_h);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// ========== PASS: CRT shader to offscreen FBO ==========
		glBindFramebuffer(GL_FRAMEBUFFER, state->crt_fbo);
		glViewport(0, 0, state->viewport.w, state->viewport.h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(state->shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, state->persistence_output_texture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, state->mask_texture);
		glUniform2f(state->uniform_src_image_size, (float)state->buffer_width, (float)state->buffer_height);
		glUniform2f(state->uniform_resolution, (float)state->viewport.w, (float)state->viewport.h);
		glUniform1f(state->uniform_brightness, state->brightness);
		glUniform4f(state->uniform_tone, state->tone_data[0], state->tone_data[1], state->tone_data[2], state->tone_data[3]);
		glUniform1i(state->uniform_sampler_location, 0);
		glUniform1i(state->uniform_mask_sampler, 2);
		glUniform1i(state->uniform_use_mask_texture, state->crt_mask_type);
		glUniform1i(state->uniform_apply_mask, 0);  // Bezel applied by composite
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// ========== PASS: Composite CRT + bloom to screen ==========
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(state->bloom_composite_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, state->crt_output_texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, state->bloom_mip_texture[0]);
		glUniform1i(state->composite_uniform_crt_sampler, 0);
		glUniform1i(state->composite_uniform_bloom_sampler, 1);
		glUniform1f(state->composite_uniform_bloom_strength, state->bloom_strength);
		glUniform2f(state->composite_uniform_viewport_size, (float)state->viewport.w, (float)state->viewport.h);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	} else if(state->toggle_crt_emulation) {
		// ========== CRT shader directly to screen (no bloom) ==========
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(state->shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, state->persistence_output_texture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, state->mask_texture);
		glUniform2f(state->uniform_src_image_size, (float)state->buffer_width, (float)state->buffer_height);
		glUniform2f(state->uniform_resolution, (float)state->viewport.w, (float)state->viewport.h);
		glUniform1f(state->uniform_brightness, state->brightness);
		glUniform4f(state->uniform_tone, state->tone_data[0], state->tone_data[1], state->tone_data[2], state->tone_data[3]);
		glUniform1i(state->uniform_sampler_location, 0);
		glUniform1i(state->uniform_mask_sampler, 2);
		glUniform1i(state->uniform_use_mask_texture, state->crt_mask_type);
		glUniform1i(state->uniform_apply_mask, 1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	} else {
		// ========== Simple passthrough: CRT emulation is OFF ==========
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(state->viewport.x, state->viewport.y, state->viewport.w, state->viewport.h);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, state->texture);
		glUseProgram(state->passthrough_program);
		glUniform1i(state->passthrough_uniform_source, 0);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}
