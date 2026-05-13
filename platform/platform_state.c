// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

#define CRT_ASPECT_NUM 4
#define CRT_ASPECT_DEN 3
#define CRT_ASPECT     ((float)CRT_ASPECT_NUM / (float)CRT_ASPECT_DEN)

#define FRAME_RATE_HZ 50.0
#define FRAME_TIME_NS ((uint64_t)(1000000000.0 / FRAME_RATE_HZ))

// 4 mips give a halo up to ~32 viewport pixels wide - tight CRT-style glow
// around bright sources. Going deeper (5-6 mips) produces a screen-wide veil
// that raises black levels and crushes contrast.
#define BLOOM_MIP_COUNT 4

struct platform_state {
	struct mkfw_state *window;

	// Demo identity (set by remake_options)
	char *release_group;	// MAX 40 chars
	char *release_title;	// MAX 40 chars
	char *window_title;

	// Framebuffer (fixed 1024x1024, only buffer_width x buffer_height is used)
	uint32_t buffer[1024 * 1024] __attribute__((aligned(4096)));
	uint32_t buffer_width;	// Current logical buffer width
	uint32_t buffer_height;	// Current logical buffer height

	struct { int32_t x, y, w, h; } viewport;
	int32_t mouse_dx;
	int32_t mouse_dy;

	float contrast;
	float saturation;
	float brightness;
	float tone_data[4];

	// OpenGL Objects
	GLuint shader_program;
	GLuint persistence_program;
	GLuint passthrough_program;
	GLuint upscale_warp_program;
	GLuint bloom_downsample_program;
	GLuint bloom_upsample_program;
	GLuint bloom_composite_program;

	GLuint texture;
	GLuint persistence_texture;
	GLuint persistence_output_texture;
	GLuint crt_output_texture;
	GLuint upscaled_source_texture;
	GLuint bloom_mip_texture[BLOOM_MIP_COUNT];

	GLuint persistence_fbo;
	GLuint crt_fbo;
	GLuint upscaled_source_fbo;
	GLuint bloom_mip_fbo[BLOOM_MIP_COUNT];

	uint32_t bloom_mip_w[BLOOM_MIP_COUNT];
	uint32_t bloom_mip_h[BLOOM_MIP_COUNT];

	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	// CRT Shader Uniforms
	GLuint uniform_resolution;
	GLuint uniform_src_image_size;
	GLuint uniform_brightness;
	GLuint uniform_tone;
	GLuint uniform_sampler_location;
	GLuint uniform_use_mask_texture;
	GLuint uniform_apply_mask;
	GLuint uniform_mask_sampler;

	// Passthrough Shader Uniforms
	GLuint passthrough_uniform_source;

	// Bloom downsample uniforms
	GLuint downsample_uniform_source;
	GLuint downsample_uniform_texel_size;
	GLuint downsample_uniform_first_pass;
	GLuint downsample_uniform_threshold;
	GLuint downsample_uniform_knee;
	GLuint downsample_uniform_halation_power;

	// Bloom upsample uniforms
	GLuint upsample_uniform_source;
	GLuint upsample_uniform_texel_size;
	GLuint upsample_uniform_intensity;

	// Bloom composite uniforms
	GLuint composite_uniform_crt_sampler;
	GLuint composite_uniform_bloom_sampler;
	GLuint composite_uniform_bloom_strength;
	GLuint composite_uniform_viewport_size;

	// Phosphor mask texture
	GLuint mask_texture;
	uint8_t crt_mask_type;	// 0 = computed (original), 1 = texture

	// Bloom settings
	float bloom_threshold;
	float bloom_knee;
	float bloom_strength;
	float bloom_upsample_intensity;
	float bloom_halation_power;

	// Phosphor persistence
	float persistence_decay;

	// Rendering & Frame State
	uint32_t frame_number;
	uint8_t running;
	uint8_t toggle_crt_emulation;
	uint8_t toggle_bloom;
	uint8_t fullscreen;
	uint8_t viewport_changed;

};

static struct platform_state platform_state;
