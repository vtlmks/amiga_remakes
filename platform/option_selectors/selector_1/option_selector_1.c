// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

#define OPTIONS_BUFFER_WIDTH  320
#define OPTIONS_BUFFER_HEIGHT 240
#define OPTIONS_WINDOW_SCALE  2

#include "../option_selector.c"

//-----------------------------------------------------------------------------------------------------------------------

INCBIN_UGG(awesome_font, "option_selectors/selector_1/data/awesomefont8x8.ugg");

#define OPTION_NSTARS  256
#define OPTION_FOCAL   128.0f
#define OPTION_SPEED   0.7f

struct star_ {
	float x, y, z;
};

static struct star_ option_stars[OPTION_NSTARS];

// [=]===^=[ option_init_stars ]================================================================^===[=]
static void option_init_stars(struct option_state *state) {
	for(int i = 0; i < OPTION_NSTARS; ++i) {
		option_stars[i].x = ((float)xor_generate_random(&state->rand) / (float)0xffffffff - 0.5f) * state->buffer_width;
		option_stars[i].y = ((float)xor_generate_random(&state->rand) / (float)0xffffffff - 0.5f) * state->buffer_height;
		option_stars[i].z = 1.0f + ((float)xor_generate_random(&state->rand) / (float)0xffffffff) * 255.0f;
	}
}

// [=]===^=[ render_starfield ]================================================================^===[=]
static void render_starfield(struct option_state *state) {
	for(int i = 0; i < OPTION_NSTARS; ++i) {
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

//-----------------------------------------------------------------------------------------------------------------------

static uint32_t line_colors[] = {
	0xd0d8e8ff, 0xe0e4ecff, 0xf0f2f8ff, 0xf8faffff, 0xf0f2f8ff, 0xe0e4ecff, 0xd0d8e8ff, 0xc0c8d8ff,
};

static uint32_t line_release_colors[] = {
	0xe8d8d0ff, 0xece4e0ff, 0xf8f2f0ff, 0xfffaf8ff, 0xf8f2f0ff, 0xece4e0ff, 0xe8d8d0ff, 0xd8c8c0ff,
};

// [=]===^=[ render_text ]================================================================^===[=]
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

// [=]===^=[ chrlen ]================================================================^===[=]
static uint32_t chrlen(char *s) {
	uint32_t result = 0;
	while(*s++) {
		result++;
	}
	return result;
}

//-----------------------------------------------------------------------------------------------------------------------

INCBIN_BYTES(billy_the_kid_music, "option_selectors/selector_1/data/billy the kid.fc");
static struct fc14_state option_song;

// [=]===^=[ option_audio ]================================================================^===[=]
static void option_audio(int16_t *audio_buffer, size_t frames) {
	fc14_get_audio(&option_song, audio_buffer, frames);
}

//-----------------------------------------------------------------------------------------------------------------------

static uint32_t release_group_center_x;
static uint32_t release_title_center_x;

// [=]===^=[ option_setup ]================================================================^===[=]
static void option_setup(struct option_state *state, struct platform_state *pstate) {
	fc14_initialize(&option_song, (const uint8_t*)billy_the_kid_music, billy_the_kid_music_end - billy_the_kid_music_data, 48000);
	mkfw_set_audio_callback(option_audio);
}

// [=]===^=[ option_init ]================================================================^===[=]
static void option_init(struct option_state *state, struct platform_state *pstate) {
	option_init_stars(state);
	release_group_center_x = (state->buffer_width - (chrlen(pstate->release_group) * 8)) >> 1;
	release_title_center_x = (state->buffer_width - (chrlen(pstate->release_title) * 8)) >> 1;
}

// [=]===^=[ option_frame ]================================================================^===[=]
static void option_frame(struct option_state *state, struct platform_state *pstate) {
	if(mkfw_is_key_pressed(state->window, MKS_KEY_F1)) {
		pstate->fullscreen = !pstate->fullscreen;
	}
	if(mkfw_is_key_pressed(state->window, MKS_KEY_F2)) {
		pstate->toggle_crt_emulation = !pstate->toggle_crt_emulation;
	}

	render_starfield(state);
	render_text(state,  68,  16, "MINDKILLER SYSTEMS 2025", line_colors);
	render_text(state, 128,  32, "PRESENTS", line_colors);
	render_text(state, release_group_center_x,  64, pstate->release_group, line_release_colors);
	render_text(state, release_title_center_x,  80, pstate->release_title, line_release_colors);

	char *fullscreen_string	= (pstate->fullscreen)				? "F1 - Unlimited Screen Size ON" : "F1 - Unlimited Screen Size OFF";
	char *crt_string			= (pstate->toggle_crt_emulation)	? "F2 - CRT Shader DLC ...... ON" : "F2 - CRT Shader DLC ...... OFF";
	render_text(state,  40, 120, fullscreen_string, line_colors);
	render_text(state,  40, 136, crt_string, line_colors);

	render_text(state,  80, 162, "PRESS SPACE TO START", line_colors);
	render_text(state,  48, 178, "(F11/F12 TOGGLE DURING DEMO)", line_colors);

	render_text(state,  24, 216, "REMAKE BY VITAL/MINDKILLER SYSTEMS", line_colors);
}

// [=]===^=[ option_shutdown ]================================================================^===[=]
static void option_shutdown(struct option_state *state, struct platform_state *pstate) {
	mkfw_set_audio_callback(0);
	fc14_shutdown(&option_song);
}
