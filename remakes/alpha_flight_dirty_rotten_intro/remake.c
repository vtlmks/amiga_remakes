// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// [=]===^=[ base setup ]============================================================^===[=]

#define BUFFER_WIDTH  (346 << 0)
#define BUFFER_HEIGHT (270 << 0)

#define AUDIO_STEREO_MIX
#include "platform.c"

// [=]===^=[ remake stuff below ]============================================================^===[=]

INCBIN_UGG(alpha_flight_logo, "data/alpha_flight_logo.ugg");
INCBIN_UGG(alpha_flight_font, "data/alpha_flight_font.ugg");
INCBIN_BYTES(chipsong_data, "data/chipsong.mod");

static struct micromod_state chipsong_song;

// [=]===^=[ data ]============================================================^===[=]

static uint32_t star_speeds[160] = {
	1, 2, 3, 4, 1, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 1, 2, 2, 3, 4, 1, 2, 3, 4, 5, 6, 1, 2, 3, 1, 1, 2,
	3, 1, 1, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 1, 1, 2, 1, 4, 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 1, 2, 3, 5,
	2, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 2, 1, 1, 3, 4, 1, 2, 3, 4, 1, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 1,
	2, 2, 3, 4, 1, 2, 3, 4, 5, 6, 1, 2, 3, 1, 1, 2, 3, 1, 1, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 1, 1, 2,
	1, 4, 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 1, 2, 3, 5, 2, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 2, 1, 1, 3, 4,
};

#define FONT_ROW_0 0
#define FONT_ROW_1 10240
#define FONT_ROW_2 20480
#define FONT_ROW_3 30720

struct font_char_info font_char_infos[128] = {
	['A'] = {.offset = FONT_ROW_0 + 0,   .width = 32},
	['B'] = {.offset = FONT_ROW_0 + 32,  .width = 32},
	['C'] = {.offset = FONT_ROW_0 + 64,  .width = 32},
	['D'] = {.offset = FONT_ROW_0 + 96,  .width = 32},
	['E'] = {.offset = FONT_ROW_0 + 128, .width = 32},
	['F'] = {.offset = FONT_ROW_0 + 160, .width = 32},
	['G'] = {.offset = FONT_ROW_0 + 192, .width = 32},
	['H'] = {.offset = FONT_ROW_0 + 224, .width = 32},
	['I'] = {.offset = FONT_ROW_0 + 256, .width = 32},
	['J'] = {.offset = FONT_ROW_0 + 288, .width = 32},

	['K'] = {.offset = FONT_ROW_1 + 0,   .width = 32},
	['L'] = {.offset = FONT_ROW_1 + 32,  .width = 32},
	['M'] = {.offset = FONT_ROW_1 + 64,  .width = 32},
	['N'] = {.offset = FONT_ROW_1 + 96,  .width = 32},
	['O'] = {.offset = FONT_ROW_1 + 128, .width = 32},
	['P'] = {.offset = FONT_ROW_1 + 160, .width = 32},
	['Q'] = {.offset = FONT_ROW_1 + 192, .width = 32},
	['R'] = {.offset = FONT_ROW_1 + 224, .width = 32},
	['S'] = {.offset = FONT_ROW_1 + 256, .width = 32},
	['T'] = {.offset = FONT_ROW_1 + 288, .width = 32},

	['U'] = {.offset = FONT_ROW_2 + 0,   .width = 32},
	['V'] = {.offset = FONT_ROW_2 + 32,  .width = 32},
	['W'] = {.offset = FONT_ROW_2 + 64,  .width = 32},
	['X'] = {.offset = FONT_ROW_2 + 96,  .width = 32},
	['Y'] = {.offset = FONT_ROW_2 + 128, .width = 32},
	['Z'] = {.offset = FONT_ROW_2 + 160, .width = 32},
	['0'] = {.offset = FONT_ROW_2 + 192, .width = 32},
	['1'] = {.offset = FONT_ROW_2 + 224, .width = 32},
	['2'] = {.offset = FONT_ROW_2 + 256, .width = 32},
	['3'] = {.offset = FONT_ROW_2 + 288, .width = 32},

	['4'] = {.offset = FONT_ROW_3 + 0,   .width = 32},
	['5'] = {.offset = FONT_ROW_3 + 32,  .width = 32},
	['6'] = {.offset = FONT_ROW_3 + 64,  .width = 32},
	['7'] = {.offset = FONT_ROW_3 + 96,  .width = 32},
	['8'] = {.offset = FONT_ROW_3 + 128, .width = 32},
	['9'] = {.offset = FONT_ROW_3 + 160, .width = 32},
	[' '] = {.offset = FONT_ROW_3 + 224, .width = 32},
	['n'] = {.offset = FONT_ROW_3 + 224, .width = 32},
	['o'] = {.offset = FONT_ROW_3 + 224, .width = 32},
	['p'] = {.offset = FONT_ROW_3 + 224, .width = 32},
	['q'] = {.offset = FONT_ROW_3 + 224, .width = 32},
	['r'] = {.offset = FONT_ROW_3 + 224, .width = 32},
};

static uint8_t scroll_text[] = {
	"n         p  HELLO FREAKS AND FREAKYS   MURFS AND SMURFS   THIS"
	" WILL BE THE NEW CRACKINTRO OF THE GLORIOUS ALPHAFLIGHT     MAD"
	"E IN 5 HOURS BY RAXXION OF ALPHAFLIGHT   THE GFX WERE DRAWN BY "
	" PARALAX  AND THE SOUND WAS DONE BY CRYSTAL     THIS IS ONLY A "
	"SHORT PREVIEW OF THIZ DIRTY ROTTEN INTRO    DONE TO PUT IT BEFO"
	"RE CRACKS    SEE YOU ALL LATER IN OTHER PRODUCTIONS  FROM      "
	"o    ALPHAFLIGHT                    THIS FLIGHT WILL NEVER STOP"
	"            "
};

// nopqr - different speeds
// n = 1
// o = 2
// p = 4
// q = 8
// r = 16

// s = I have zero idea,

// NOTE(peter): 56 colors from copperlist + 1 wait (4 color-clocks) makes last color repeat 3 times
static uint32_t background_copper[] = {
	0x001111ff, 0x002222ff, 0x003333ff, 0x004444ff, 0x005555ff, 0x006666ff, 0x007777ff, 0x008888ff,
	0x009999ff, 0x00aaaaff, 0x00bbbbff, 0x00ccccff, 0x00ddddff, 0x00eeeeff, 0x00ffffff, 0x11ffeeff,
	0x22ffddff, 0x33ffccff, 0x44ffbbff, 0x55ffaaff, 0x66ff99ff, 0x77ff88ff, 0x88ff77ff, 0x99ff66ff,
	0xaaff55ff, 0xbbff44ff, 0xccff33ff, 0xddff22ff, 0xeeff11ff, 0xffff00ff, 0xffee00ff, 0xffdd00ff,
	0xffcc00ff, 0xffbb00ff, 0xffaa00ff, 0xff9900ff, 0xff8800ff, 0xff7700ff, 0xff6600ff, 0xff5500ff,
	0xff4411ff, 0xee3322ff, 0xdd2233ff, 0xcc1144ff, 0xbb0055ff, 0xaa0066ff, 0x990077ff, 0x880088ff,
	0x770099ff, 0x6600aaff, 0x5500bbff, 0x4400ccff, 0x3300ddff, 0x2200eeff, 0x1100ffff, 0x0000ffff, 0x0000ffff, 0x0000ffff,
};

uint32_t color_buffer[452*5];	// copper is 226 clocks wide, each is 2 pixels, so we render the colors here

#define NUM_STARS 65
#define LOGO_Y_POS 56

static uint32_t stars[95];

struct rng_state base_rand;

static struct scroller_state scroll_state;

static struct rect clip_rect = { 0, 44, 346, 130 };

enum af_state {
	STATE_WAIT_LOGO,
	STATE_FADE_STARS,
	STATE_RUN
};

static enum af_state af_remake_state = STATE_WAIT_LOGO;
static uint32_t star_color;
static uint32_t logo_y = LOGO_Y_POS + 120;	// +120 to start outside visible area (clip_rect)

// [=]===^=[ functions ]============================================================^===[=]

static void remake_audio_callback(int16_t *data, size_t frames) {
	micromod_get_audio(&chipsong_song, data, frames);
}

static uint8_t af_process_char(struct scroller_state *scr_state, uint8_t char_index) {
	if(char_index == 0) {		// Scrolltext end
		scr_state->text_offset = 0;
		return scr_state->text[scr_state->text_offset++];
	}

	if(char_index == 'n') {
		scr_state->speed = 1;
		return ' ';
	}
	if(char_index == 'o') {
		scr_state->speed = 2;
		return ' ';
	}
	if(char_index == 'p') {
		scr_state->speed = 4;
		return ' ';
	}
	if(char_index == 'q') {
		scr_state->speed = 8;
		return ' ';
	}
	if(char_index == 'r') {
		scr_state->speed = 16;
		return ' ';
	}

	return char_index;
}

// [=]===^=[ remake_init ]============================================================^===[=]
static void remake_init(struct platform_state *state) {
	platform_change_resolution(state, BUFFER_WIDTH, BUFFER_HEIGHT);

	xor_init_rng(&base_rand, 187481201);

	for(size_t i = 0; i < 95; ++i) {
		stars[i] = xor_generate_random(&base_rand) % state->buffer_width;
	}

	for(size_t pixel = 0; pixel < 452 * 5; ++pixel) {
		color_buffer[pixel] = background_copper[(pixel / 8) % 58];
	}

	scroll_state = (struct scroller_state){ .char_width = 32, .char_height = 32, .dest_offset_y = 152, .speed = 1, .text = scroll_text, .font = alpha_flight_font, .char_info = font_char_infos, .process_char = af_process_char };
	scroller_new(&scroll_state);

	micromod_initialize(&chipsong_song, (int8_t*)chipsong_data, 48000);

	mkfw_audio_callback = remake_audio_callback;
}

static void remake_options(struct platform_state *state) {
	state->release_group = "Alpha Flight";
	state->release_title = "Dirty Rotten Intro";
	state->window_title = "Alpha Flight - Dirty Rotten Intro 1992-10\0";
}

static void af_render_scroll_buffer(struct platform_state *state, struct scroller_state *scr_state) {
	uint32_t *scroll_dest = BUFFER_PTR(state, 0, scr_state->dest_offset_y);
	uint8_t *scroll_src = scr_state->buffer;

	// NOTE(peter): Copper colors start off-screen, 70 is first visible color on screen
	uint32_t *color = color_buffer + 70;

	size_t base_src_index = (scr_state->char_render_offset - 370) & (SCROLL_BUFFER_WIDTH - 1);
	for(size_t i = 0; i < scr_state->char_height; ++i) {
		for(size_t j = 0; j < state->buffer_width; ++j) {
			size_t src_index = (base_src_index + j) & (SCROLL_BUFFER_WIDTH - 1);
			uint8_t color_index = scroll_src[src_index];
			if(!color_index) continue;

			scroll_dest[j] = color[j];
		}
		scroll_dest += state->buffer_width;
		scroll_src += SCROLL_BUFFER_WIDTH;
		color += 452;
	}
}

static void render_background(struct platform_state *state) {
	uint32_t *dst = state->buffer;
	// Top grey area (43 scanlines)
	for(size_t i = 0; i < 43 * state->buffer_width; ++i) {
		*dst++ = 0x555555ff;
	}
	// Blue separator line (scanline 44)
	for(size_t i = 0 ; i < state->buffer_width; ++i) {
		*dst++ = 0x7777ffff;
	}

	// Blue separator line (scanline 174)
	dst = BUFFER_PTR(state, 0, 174);
	for(size_t i = 0 ; i < state->buffer_width; ++i) {
		*dst++ = 0x7777ffff;
	}
	// Bottom grey area (95 scanlines)
	for(size_t i = 0; i < 95 * state->buffer_width; ++i) {
		*dst++ = 0x555555ff;
	}
}

static void fade_star_color(void) {
	uint8_t r = (star_color >> 24) & 0xff;
	uint8_t g = (star_color >> 16) & 0xff;
	uint8_t b = (star_color >> 8) & 0xff;
	uint8_t a = star_color & 0xff;

	if(r < 0x44) r += 0x11;
	if(g < 0x44) g += 0x11;
	if(b < 0x88) b += 0x11;
	if(a < 0xff) a += 0x11;

	star_color = (r << 24) | (g << 16) | (b << 8) | a;

	if(a == 0xff) {
		af_remake_state = STATE_RUN;
	}
}

static void render_stars(struct platform_state *state) {
	uint32_t *dst = BUFFER_PTR(state, 0, 45);	// Start below blue line
	for(size_t i = 0; i < NUM_STARS; ++i) {
		dst[stars[i]] = star_color;
		dst += 2*state->buffer_width;
		stars[i] += star_speeds[i] << 1;
		if(stars[i] > state->buffer_width) stars[i] -= state->buffer_width;
	}
}

// [=]===^=[ remake_frame ]============================================================^===[=]
static void remake_frame(struct platform_state *state) {
	platform_clear_buffer(state);

	scroller_update(state, &scroll_state);
	render_background(state);
	render_stars(state);

	switch(af_remake_state) {
		case STATE_WAIT_LOGO: {
			blit_full_src(state, alpha_flight_logo, CENTER_X(state, alpha_flight_logo->width), logo_y, clip_rect, 0);
			if(state->frame_number & 1) {
				logo_y = (logo_y == LOGO_Y_POS) ? LOGO_Y_POS : logo_y - 1;
			}
			if(logo_y == LOGO_Y_POS) {
				af_remake_state = STATE_FADE_STARS;
			}
		} break;

		case STATE_FADE_STARS: {
			blit_full(state, alpha_flight_logo, CENTER_X(state, alpha_flight_logo->width), LOGO_Y_POS, 0);
			fade_star_color();
		} break;

		case STATE_RUN: {
			blit_full(state, alpha_flight_logo, CENTER_X(state, alpha_flight_logo->width), LOGO_Y_POS, 0);
		} break;
	}

	af_render_scroll_buffer(state, &scroll_state);
}

// [=]===^=[ remake_shutdown ]============================================================^===[=]
static void remake_shutdown(struct platform_state *state) {
	mkfw_audio_callback = 0;
}
