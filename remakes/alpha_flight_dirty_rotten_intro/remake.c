// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// [=]===^=[ base setup ]============================================================^===[=]

#define WINDOW_WIDTH 360
#define WINDOW_HEIGHT 270
#define BUFFER_WIDTH  (346 << 0)
#define BUFFER_HEIGHT (270 << 0)

#include "platform.c"

// [=]===^=[ remake stuff below ]============================================================^===[=]

INCBIN_UGG(alpha_flight_logo, "data/alpha_flight_logo.ugg");
INCBIN_UGG(alpha_flight_font, "data/alpha_flight_font.ugg");
INCBIN_BYTES(chipsong_data, "data/chipsong.mod");

static struct micromod_state chipsong_song;

// [=]===^=[ audio_callback ]============================================================^===[=]
static void remake_audio_callback(int16_t *data, size_t frames) {
	micromod_get_audio(&chipsong_song, data, frames);

	// NOTE(peter): Enable for 75% mix if the player doesn't have that functionality!
#if 1
	for(size_t i = 0; i < frames; i++) {
		int32_t old_left = (int32_t)data[i * 2];
		int32_t old_right = (int32_t)data[i * 2 + 1];

		int32_t mixed_left = old_left + (old_right * 3) / 4;
		int32_t mixed_right = old_right + (old_left * 3) / 4;

		// Shift right by 1 to prevent clipping and scale down
		data[i * 2] = (int16_t)(mixed_left >> 1);
		data[i * 2 + 1] = (int16_t)(mixed_right >> 1);
	}
#endif
}

static uint32_t star_speeds[160] = {
	1, 2, 3, 4, 1, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 1, 2, 2, 3, 4, 1, 2, 3, 4, 5, 6, 1, 2, 3, 1, 1, 2,
	3, 1, 1, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 1, 1, 2, 1, 4, 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 1, 2, 3, 5,
	2, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 2, 1, 1, 3, 4, 1, 2, 3, 4, 1, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 1,
	2, 2, 3, 4, 1, 2, 3, 4, 5, 6, 1, 2, 3, 1, 1, 2, 3, 1, 1, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 1, 1, 2,
	1, 4, 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 1, 2, 3, 5, 2, 6, 1, 2, 3, 4, 1, 2, 3, 4, 5, 2, 1, 1, 3, 4,
};

uint32_t font_coords[128] = {
	['A'] = 0,
	['B'] = 32,
	['C'] = 64,
	['D'] = 96,
	['E'] = 128,
	['F'] = 160,
	['G'] = 192,
	['H'] = 224,
	['I'] = 256,
	['J'] = 288,

	['K'] = 10240 + 0,
	['L'] = 10240 + 32,
	['M'] = 10240 + 64,
	['N'] = 10240 + 96,
	['O'] = 10240 + 128,
	['P'] = 10240 + 160,
	['Q'] = 10240 + 192,
	['R'] = 10240 + 224,
	['S'] = 10240 + 256,
	['T'] = 10240 + 288,

	['U'] = 20480 + 0,
	['V'] = 20480 + 32,
	['W'] = 20480 + 64,
	['X'] = 20480 + 96,
	['Y'] = 20480 + 128,
	['Z'] = 20480 + 160,
	['0'] = 20480 + 192,
	['1'] = 20480 + 224,
	['2'] = 20480 + 256,
	['3'] = 20480 + 288,

	['4'] = 30720 + 0,
	['5'] = 30720 + 32,
	['6'] = 30720 + 64,
	['7'] = 30720 + 96,
	['8'] = 30720 + 128,
	['9'] = 30720 + 160,
	[' '] = 30720 + 224,
	['n'] = 30720 + 224,
	['o'] = 30720 + 224,
	['p'] = 30720 + 224,
	['q'] = 30720 + 224,
	['r'] = 30720 + 224,
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

static struct scroller_state *scroll_state;

static size_t get_font_offset(struct scroller_state *scr_state, uint8_t char_index) {
	if(char_index == 'n') { scr_state->speed = 1; }
	if(char_index == 'o') { scr_state->speed = 2; }
	if(char_index == 'p') { scr_state->speed = 4; }
	if(char_index == 'q') { scr_state->speed = 8; }
	if(char_index == 'r') { scr_state->speed = 16; }

	return font_coords[char_index];
}

struct rng_state base_rand;

static uint32_t stars[95];

// [=]===^=[ remake_init ]============================================================^===[=]
static void remake_init(struct mkfw_state *window) {
	xor_init_rng(&base_rand, 187481201);

	for(size_t i = 0; i < 95; ++i) {
		stars[i] = xor_generate_random(&base_rand) % BUFFER_WIDTH;
	}

	for(size_t pixel = 0; pixel < 452 * 5; ++pixel) {
		color_buffer[pixel] = background_copper[(pixel / 8) % 58];
	}

	scroll_state = scroller_new(32, 32, 152, 1, scroll_text, alpha_flight_font, 0, get_font_offset);

	micromod_initialize(&chipsong_song, (int8_t*)chipsong_data, 48000);

	mkfw_audio_callback = remake_audio_callback;
}

static void remake_options(struct options *opt) {
	opt->release_group = "Alpha Flight";
	opt->release_title = "Dirty Rotten Intro";
	opt->window_title = "Alpha Flight - Dirty Rotten Intro 1992-10\0";
}



static void af_render_scroll_buffer(struct scroller_state *scr_state) {
	uint32_t *scroll_dest = buffer + scr_state->dest_offset_y * BUFFER_WIDTH;
	uint8_t *scroll_src = scr_state->buffer;

	uint32_t *color = color_buffer + 70;


	size_t base_src_index = (scr_state->char_render_offset - 370) & (SCROLL_BUFFER_WIDTH - 1);
	for(size_t i = 0; i < scr_state->char_height; ++i) {
		for(size_t j = 0; j < BUFFER_WIDTH; ++j) {
			size_t src_index = (base_src_index + j) & (SCROLL_BUFFER_WIDTH - 1);
			uint8_t color_index = scroll_src[src_index];
			if(!color_index) continue;

			scroll_dest[j] = color[j];
		}
		scroll_dest += BUFFER_WIDTH;
		scroll_src += SCROLL_BUFFER_WIDTH;
		color += 452;
	}
}

static void render_background(void) {
	uint32_t *dst = buffer;
	for(size_t i = 0; i < 43 * BUFFER_WIDTH; ++i) {
		*dst++ = 0x555555ff;
	}
	for(size_t i = 0 ; i < BUFFER_WIDTH; ++i) {
		*dst++ = 0x7777ffff;
	}

	dst = buffer + 174 * BUFFER_WIDTH;
	for(size_t i = 0 ; i < BUFFER_WIDTH; ++i) {
		*dst++ = 0x7777ffff;
	}
	for(size_t i = 0; i < 95 * BUFFER_WIDTH; ++i) {
		*dst++ = 0x555555ff;
	}
}

static struct rect clip_rect = { 0, 44, BUFFER_WIDTH, 130 };

enum af_state {
	STATE_WAIT_LOGO,
	STATE_FADE_STARS,
	STATE_RUN
};

static enum af_state af_remake_state = STATE_WAIT_LOGO;


static uint32_t star_color;
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

static void render_stars(void) {
	uint32_t *dst = buffer + 45 * BUFFER_WIDTH;
	for(size_t i = 0; i < 65; ++i) {
		dst[stars[i]] = star_color;
		dst += 2*BUFFER_WIDTH;
		stars[i] += star_speeds[i] << 1;
		if(stars[i] > BUFFER_WIDTH) stars[i] -= BUFFER_WIDTH;
	}
}

// [=]===^=[ remake_frame ]============================================================^===[=]
static void remake_frame(struct mkfw_state *window) {

	scroller(scroll_state);
	render_background();
	render_stars();

	switch(af_remake_state) {
		case STATE_WAIT_LOGO: {
			static uint32_t logo_y = 56 + 120;
			blit_clipped(alpha_flight_logo, ((BUFFER_WIDTH - alpha_flight_logo->width) >> 1), logo_y, clip_rect, 0);
			if(state.frame_number & 1) {
				logo_y = (logo_y == 56) ? 56 : logo_y - 1;
			}
			if(logo_y == 56) {
				af_remake_state = STATE_FADE_STARS;
			}
		} break;

		case STATE_FADE_STARS: {
			blit_full(alpha_flight_logo, ((BUFFER_WIDTH - alpha_flight_logo->width) >> 1), 56, 0);
			fade_star_color();
		} break;

		case STATE_RUN: {
			blit_full(alpha_flight_logo, ((BUFFER_WIDTH - alpha_flight_logo->width) >> 1), 56, 0);
		} break;
	}

	af_render_scroll_buffer(scroll_state);
}

// [=]===^=[ remake_shutdown ]============================================================^===[=]
static void remake_shutdown(struct mkfw_state *window) {
	mkfw_audio_callback = 0;
	// fc14play_Close();
}



/*
Loaded file 'Saved State 1.bin' (size: 0x80000 bytes)
Parsing copperlist at offset 0x10b18

WAIT 0x4
     REG $120 -> VALUE 0x0002
     REG $122 -> VALUE 0x44cc
     BPL1PTH -> 0x0001 (full: $00010000) at scanline 4
     BPL1PTL -> 0x13b0 (full: $000113b0) at scanline 4
     BPL2PTH -> 0x0001 (full: $00010000) at scanline 4
     BPL2PTL -> 0x4c54 (full: $00014c54) at scanline 4
     BPL3PTH -> 0x0001 (full: $00010000) at scanline 4
     BPL3PTL -> 0x84f8 (full: $000184f8) at scanline 4
     BPL4PTH -> 0x0001 (full: $00010000) at scanline 4
     BPL4PTL -> 0xbd9c (full: $0001bd9c) at scanline 4
     BPL5PTH -> 0x0001 (full: $00010000) at scanline 4
     BPL5PTL -> 0xf640 (full: $0001f640) at scanline 4
     REG $100 -> VALUE 0x4200
     REG $104 -> VALUE 0x0000
     COLOR17 -> RGB 0x000 at scanline 4
     COLOR18 -> RGB 0x000 at scanline 4
     COLOR19 -> RGB 0x000 at scanline 4
     REG $108 -> VALUE 0x0002
     REG $10a -> VALUE 0x0002
WAIT 0x5
     COLOR0 -> RGB 0x555 at scanline 5
     COLOR18 -> RGB 0x555 at scanline 5
WAIT 0x50
     COLOR0 -> RGB 0x77f at scanline 80
     COLOR1 -> RGB 0x77f at scanline 80
     COLOR18 -> RGB 0x77f at scanline 80
WAIT 0x51
     COLOR0 -> RGB 0x000 at scanline 81
     COLOR1 -> RGB 0x000 at scanline 81
     COLOR18 -> RGB 0x000 at scanline 81
WAIT 0x52
     REG $100 -> VALUE 0x0000
WAIT 0x53
     REG $100 -> VALUE 0x4200
     COLOR18 -> RGB 0x448 at scanline 83
     COLOR0 -> RGB 0x000 at scanline 83
     COLOR1 -> RGB 0x000 at scanline 83
     COLOR2 -> RGB 0xfff at scanline 83
     COLOR3 -> RGB 0xfff at scanline 83
     COLOR4 -> RGB 0x99c at scanline 83
     COLOR5 -> RGB 0x88b at scanline 83
     COLOR6 -> RGB 0x77a at scanline 83
     COLOR7 -> RGB 0x669 at scanline 83
     COLOR8 -> RGB 0x558 at scanline 83
     COLOR9 -> RGB 0x447 at scanline 83
     COLOR10 -> RGB 0x336 at scanline 83
     COLOR11 -> RGB 0x225 at scanline 83
     COLOR12 -> RGB 0x114 at scanline 83
     COLOR13 -> RGB 0x003 at scanline 83
     COLOR14 -> RGB 0xaad at scanline 83
     COLOR15 -> RGB 0xbbe at scanline 83
WAIT 0xbc
     COLOR2 -> RGB 0x011 at scanline 188
     COLOR2 -> RGB 0x022 at scanline 188
     COLOR2 -> RGB 0x033 at scanline 188
     COLOR2 -> RGB 0x044 at scanline 188
     COLOR2 -> RGB 0x055 at scanline 188
     COLOR2 -> RGB 0x066 at scanline 188
     COLOR2 -> RGB 0x077 at scanline 188
     COLOR2 -> RGB 0x088 at scanline 188
     COLOR2 -> RGB 0x099 at scanline 188
     COLOR2 -> RGB 0x0aa at scanline 188
     COLOR2 -> RGB 0x0bb at scanline 188
     COLOR2 -> RGB 0x0cc at scanline 188
     COLOR2 -> RGB 0x0dd at scanline 188
     COLOR2 -> RGB 0x0ee at scanline 188
     COLOR2 -> RGB 0x0ff at scanline 188
     COLOR2 -> RGB 0x1fe at scanline 188
     COLOR2 -> RGB 0x2fd at scanline 188
     COLOR2 -> RGB 0x3fc at scanline 188
     COLOR2 -> RGB 0x4fb at scanline 188
     COLOR2 -> RGB 0x5fa at scanline 188
     COLOR2 -> RGB 0x6f9 at scanline 188
     COLOR2 -> RGB 0x7f8 at scanline 188
     COLOR2 -> RGB 0x8f7 at scanline 188
     COLOR2 -> RGB 0x9f6 at scanline 188
     COLOR2 -> RGB 0xaf5 at scanline 188
     COLOR2 -> RGB 0xbf4 at scanline 188
     COLOR2 -> RGB 0xcf3 at scanline 188
     COLOR2 -> RGB 0xdf2 at scanline 188
     COLOR2 -> RGB 0xef1 at scanline 188
     COLOR2 -> RGB 0xff0 at scanline 188
     COLOR2 -> RGB 0xfe0 at scanline 188
     COLOR2 -> RGB 0xfd0 at scanline 188
     COLOR2 -> RGB 0xfc0 at scanline 188
     COLOR2 -> RGB 0xfb0 at scanline 188
     COLOR2 -> RGB 0xfa0 at scanline 188
     COLOR2 -> RGB 0xf90 at scanline 188
     COLOR2 -> RGB 0xf80 at scanline 188
     COLOR2 -> RGB 0xf70 at scanline 188
     COLOR2 -> RGB 0xf60 at scanline 188
     COLOR2 -> RGB 0xf50 at scanline 188
     COLOR2 -> RGB 0xf41 at scanline 188
     COLOR2 -> RGB 0xe32 at scanline 188
     COLOR2 -> RGB 0xd23 at scanline 188
     COLOR2 -> RGB 0xc14 at scanline 188
     COLOR2 -> RGB 0xb05 at scanline 188
     COLOR2 -> RGB 0xa06 at scanline 188
     COLOR2 -> RGB 0x907 at scanline 188
     COLOR2 -> RGB 0x808 at scanline 188
     COLOR2 -> RGB 0x709 at scanline 188
     COLOR2 -> RGB 0x60a at scanline 188
     COLOR2 -> RGB 0x50b at scanline 188
     COLOR2 -> RGB 0x40c at scanline 188
     COLOR2 -> RGB 0x30d at scanline 188
     COLOR2 -> RGB 0x20e at scanline 188
     COLOR2 -> RGB 0x10f at scanline 188
     COLOR2 -> RGB 0x00f at scanline 188
WAIT 0xbd
     COLOR2 -> RGB 0x011 at scanline 189
     COLOR2 -> RGB 0x022 at scanline 189
     COLOR2 -> RGB 0x033 at scanline 189
     COLOR2 -> RGB 0x044 at scanline 189
     COLOR2 -> RGB 0x055 at scanline 189
     COLOR2 -> RGB 0x066 at scanline 189
     COLOR2 -> RGB 0x077 at scanline 189
     COLOR2 -> RGB 0x088 at scanline 189
     COLOR2 -> RGB 0x099 at scanline 189
     COLOR2 -> RGB 0x0aa at scanline 189
     COLOR2 -> RGB 0x0bb at scanline 189
     COLOR2 -> RGB 0x0cc at scanline 189
     COLOR2 -> RGB 0x0dd at scanline 189
     COLOR2 -> RGB 0x0ee at scanline 189
     COLOR2 -> RGB 0x0ff at scanline 189
     COLOR2 -> RGB 0x1fe at scanline 189
     COLOR2 -> RGB 0x2fd at scanline 189
     COLOR2 -> RGB 0x3fc at scanline 189
     COLOR2 -> RGB 0x4fb at scanline 189
     COLOR2 -> RGB 0x5fa at scanline 189
     COLOR2 -> RGB 0x6f9 at scanline 189
     COLOR2 -> RGB 0x7f8 at scanline 189
     COLOR2 -> RGB 0x8f7 at scanline 189
     COLOR2 -> RGB 0x9f6 at scanline 189
     COLOR2 -> RGB 0xaf5 at scanline 189
     COLOR2 -> RGB 0xbf4 at scanline 189
     COLOR2 -> RGB 0xcf3 at scanline 189
     COLOR2 -> RGB 0xdf2 at scanline 189
     COLOR2 -> RGB 0xef1 at scanline 189
     COLOR2 -> RGB 0xff0 at scanline 189
     COLOR2 -> RGB 0xfe0 at scanline 189
     COLOR2 -> RGB 0xfd0 at scanline 189
     COLOR2 -> RGB 0xfc0 at scanline 189
     COLOR2 -> RGB 0xfb0 at scanline 189
     COLOR2 -> RGB 0xfa0 at scanline 189
     COLOR2 -> RGB 0xf90 at scanline 189
     COLOR2 -> RGB 0xf80 at scanline 189
     COLOR2 -> RGB 0xf70 at scanline 189
     COLOR2 -> RGB 0xf60 at scanline 189
     COLOR2 -> RGB 0xf50 at scanline 189
     COLOR2 -> RGB 0xf41 at scanline 189
     COLOR2 -> RGB 0xe32 at scanline 189
     COLOR2 -> RGB 0xd23 at scanline 189
     COLOR2 -> RGB 0xc14 at scanline 189
     COLOR2 -> RGB 0xb05 at scanline 189
     COLOR2 -> RGB 0xa06 at scanline 189
     COLOR2 -> RGB 0x907 at scanline 189
     COLOR2 -> RGB 0x808 at scanline 189
     COLOR2 -> RGB 0x709 at scanline 189
     COLOR2 -> RGB 0x60a at scanline 189
     COLOR2 -> RGB 0x50b at scanline 189
     COLOR2 -> RGB 0x40c at scanline 189
     COLOR2 -> RGB 0x30d at scanline 189
     COLOR2 -> RGB 0x20e at scanline 189
     COLOR2 -> RGB 0x10f at scanline 189
     COLOR2 -> RGB 0x00f at scanline 189
WAIT 0xbe
     COLOR2 -> RGB 0x011 at scanline 190
     COLOR2 -> RGB 0x022 at scanline 190
     COLOR2 -> RGB 0x033 at scanline 190
     COLOR2 -> RGB 0x044 at scanline 190
     COLOR2 -> RGB 0x055 at scanline 190
     COLOR2 -> RGB 0x066 at scanline 190
     COLOR2 -> RGB 0x077 at scanline 190
     COLOR2 -> RGB 0x088 at scanline 190
     COLOR2 -> RGB 0x099 at scanline 190
     COLOR2 -> RGB 0x0aa at scanline 190
     COLOR2 -> RGB 0x0bb at scanline 190
     COLOR2 -> RGB 0x0cc at scanline 190
     COLOR2 -> RGB 0x0dd at scanline 190
     COLOR2 -> RGB 0x0ee at scanline 190
     COLOR2 -> RGB 0x0ff at scanline 190
     COLOR2 -> RGB 0x1fe at scanline 190
     COLOR2 -> RGB 0x2fd at scanline 190
     COLOR2 -> RGB 0x3fc at scanline 190
     COLOR2 -> RGB 0x4fb at scanline 190
     COLOR2 -> RGB 0x5fa at scanline 190
     COLOR2 -> RGB 0x6f9 at scanline 190
     COLOR2 -> RGB 0x7f8 at scanline 190
     COLOR2 -> RGB 0x8f7 at scanline 190
     COLOR2 -> RGB 0x9f6 at scanline 190
     COLOR2 -> RGB 0xaf5 at scanline 190
     COLOR2 -> RGB 0xbf4 at scanline 190
     COLOR2 -> RGB 0xcf3 at scanline 190
     COLOR2 -> RGB 0xdf2 at scanline 190
     COLOR2 -> RGB 0xef1 at scanline 190
     COLOR2 -> RGB 0xff0 at scanline 190
     COLOR2 -> RGB 0xfe0 at scanline 190
     COLOR2 -> RGB 0xfd0 at scanline 190
     COLOR2 -> RGB 0xfc0 at scanline 190
     COLOR2 -> RGB 0xfb0 at scanline 190
     COLOR2 -> RGB 0xfa0 at scanline 190
     COLOR2 -> RGB 0xf90 at scanline 190
     COLOR2 -> RGB 0xf80 at scanline 190
     COLOR2 -> RGB 0xf70 at scanline 190
     COLOR2 -> RGB 0xf60 at scanline 190
     COLOR2 -> RGB 0xf50 at scanline 190
     COLOR2 -> RGB 0xf41 at scanline 190
     COLOR2 -> RGB 0xe32 at scanline 190
     COLOR2 -> RGB 0xd23 at scanline 190
     COLOR2 -> RGB 0xc14 at scanline 190
     COLOR2 -> RGB 0xb05 at scanline 190
     COLOR2 -> RGB 0xa06 at scanline 190
     COLOR2 -> RGB 0x907 at scanline 190
     COLOR2 -> RGB 0x808 at scanline 190
     COLOR2 -> RGB 0x709 at scanline 190
     COLOR2 -> RGB 0x60a at scanline 190
     COLOR2 -> RGB 0x50b at scanline 190
     COLOR2 -> RGB 0x40c at scanline 190
     COLOR2 -> RGB 0x30d at scanline 190
     COLOR2 -> RGB 0x20e at scanline 190
     COLOR2 -> RGB 0x10f at scanline 190
     COLOR2 -> RGB 0x00f at scanline 190
WAIT 0xbf
     COLOR2 -> RGB 0x011 at scanline 191
     COLOR2 -> RGB 0x022 at scanline 191
     COLOR2 -> RGB 0x033 at scanline 191
     COLOR2 -> RGB 0x044 at scanline 191
     COLOR2 -> RGB 0x055 at scanline 191
     COLOR2 -> RGB 0x066 at scanline 191
     COLOR2 -> RGB 0x077 at scanline 191
     COLOR2 -> RGB 0x088 at scanline 191
     COLOR2 -> RGB 0x099 at scanline 191
     COLOR2 -> RGB 0x0aa at scanline 191
     COLOR2 -> RGB 0x0bb at scanline 191
     COLOR2 -> RGB 0x0cc at scanline 191
     COLOR2 -> RGB 0x0dd at scanline 191
     COLOR2 -> RGB 0x0ee at scanline 191
     COLOR2 -> RGB 0x0ff at scanline 191
     COLOR2 -> RGB 0x1fe at scanline 191
     COLOR2 -> RGB 0x2fd at scanline 191
     COLOR2 -> RGB 0x3fc at scanline 191
     COLOR2 -> RGB 0x4fb at scanline 191
     COLOR2 -> RGB 0x5fa at scanline 191
     COLOR2 -> RGB 0x6f9 at scanline 191
     COLOR2 -> RGB 0x7f8 at scanline 191
     COLOR2 -> RGB 0x8f7 at scanline 191
     COLOR2 -> RGB 0x9f6 at scanline 191
     COLOR2 -> RGB 0xaf5 at scanline 191
     COLOR2 -> RGB 0xbf4 at scanline 191
     COLOR2 -> RGB 0xcf3 at scanline 191
     COLOR2 -> RGB 0xdf2 at scanline 191
     COLOR2 -> RGB 0xef1 at scanline 191
     COLOR2 -> RGB 0xff0 at scanline 191
     COLOR2 -> RGB 0xfe0 at scanline 191
     COLOR2 -> RGB 0xfd0 at scanline 191
     COLOR2 -> RGB 0xfc0 at scanline 191
     COLOR2 -> RGB 0xfb0 at scanline 191
     COLOR2 -> RGB 0xfa0 at scanline 191
     COLOR2 -> RGB 0xf90 at scanline 191
     COLOR2 -> RGB 0xf80 at scanline 191
     COLOR2 -> RGB 0xf70 at scanline 191
     COLOR2 -> RGB 0xf60 at scanline 191
     COLOR2 -> RGB 0xf50 at scanline 191
     COLOR2 -> RGB 0xf41 at scanline 191
     COLOR2 -> RGB 0xe32 at scanline 191
     COLOR2 -> RGB 0xd23 at scanline 191
     COLOR2 -> RGB 0xc14 at scanline 191
     COLOR2 -> RGB 0xb05 at scanline 191
     COLOR2 -> RGB 0xa06 at scanline 191
     COLOR2 -> RGB 0x907 at scanline 191
     COLOR2 -> RGB 0x808 at scanline 191
     COLOR2 -> RGB 0x709 at scanline 191
     COLOR2 -> RGB 0x60a at scanline 191
     COLOR2 -> RGB 0x50b at scanline 191
     COLOR2 -> RGB 0x40c at scanline 191
     COLOR2 -> RGB 0x30d at scanline 191
     COLOR2 -> RGB 0x20e at scanline 191
     COLOR2 -> RGB 0x10f at scanline 191
     COLOR2 -> RGB 0x00f at scanline 191
WAIT 0xc0
     COLOR2 -> RGB 0x011 at scanline 192
     COLOR2 -> RGB 0x022 at scanline 192
     COLOR2 -> RGB 0x033 at scanline 192
     COLOR2 -> RGB 0x044 at scanline 192
     COLOR2 -> RGB 0x055 at scanline 192
     COLOR2 -> RGB 0x066 at scanline 192
     COLOR2 -> RGB 0x077 at scanline 192
     COLOR2 -> RGB 0x088 at scanline 192
     COLOR2 -> RGB 0x099 at scanline 192
     COLOR2 -> RGB 0x0aa at scanline 192
     COLOR2 -> RGB 0x0bb at scanline 192
     COLOR2 -> RGB 0x0cc at scanline 192
     COLOR2 -> RGB 0x0dd at scanline 192
     COLOR2 -> RGB 0x0ee at scanline 192
     COLOR2 -> RGB 0x0ff at scanline 192
     COLOR2 -> RGB 0x1fe at scanline 192
     COLOR2 -> RGB 0x2fd at scanline 192
     COLOR2 -> RGB 0x3fc at scanline 192
     COLOR2 -> RGB 0x4fb at scanline 192
     COLOR2 -> RGB 0x5fa at scanline 192
     COLOR2 -> RGB 0x6f9 at scanline 192
     COLOR2 -> RGB 0x7f8 at scanline 192
     COLOR2 -> RGB 0x8f7 at scanline 192
     COLOR2 -> RGB 0x9f6 at scanline 192
     COLOR2 -> RGB 0xaf5 at scanline 192
     COLOR2 -> RGB 0xbf4 at scanline 192
     COLOR2 -> RGB 0xcf3 at scanline 192
     COLOR2 -> RGB 0xdf2 at scanline 192
     COLOR2 -> RGB 0xef1 at scanline 192
     COLOR2 -> RGB 0xff0 at scanline 192
     COLOR2 -> RGB 0xfe0 at scanline 192
     COLOR2 -> RGB 0xfd0 at scanline 192
     COLOR2 -> RGB 0xfc0 at scanline 192
     COLOR2 -> RGB 0xfb0 at scanline 192
     COLOR2 -> RGB 0xfa0 at scanline 192
     COLOR2 -> RGB 0xf90 at scanline 192
     COLOR2 -> RGB 0xf80 at scanline 192
     COLOR2 -> RGB 0xf70 at scanline 192
     COLOR2 -> RGB 0xf60 at scanline 192
     COLOR2 -> RGB 0xf50 at scanline 192
     COLOR2 -> RGB 0xf41 at scanline 192
     COLOR2 -> RGB 0xe32 at scanline 192
     COLOR2 -> RGB 0xd23 at scanline 192
     COLOR2 -> RGB 0xc14 at scanline 192
     COLOR2 -> RGB 0xb05 at scanline 192
     COLOR2 -> RGB 0xa06 at scanline 192
     COLOR2 -> RGB 0x907 at scanline 192
     COLOR2 -> RGB 0x808 at scanline 192
     COLOR2 -> RGB 0x709 at scanline 192
     COLOR2 -> RGB 0x60a at scanline 192
     COLOR2 -> RGB 0x50b at scanline 192
     COLOR2 -> RGB 0x40c at scanline 192
     COLOR2 -> RGB 0x30d at scanline 192
     COLOR2 -> RGB 0x20e at scanline 192
     COLOR2 -> RGB 0x10f at scanline 192
     COLOR2 -> RGB 0x00f at scanline 192
WAIT 0xc1
     COLOR2 -> RGB 0x011 at scanline 193
     COLOR2 -> RGB 0x022 at scanline 193
     COLOR2 -> RGB 0x033 at scanline 193
     COLOR2 -> RGB 0x044 at scanline 193
     COLOR2 -> RGB 0x055 at scanline 193
     COLOR2 -> RGB 0x066 at scanline 193
     COLOR2 -> RGB 0x077 at scanline 193
     COLOR2 -> RGB 0x088 at scanline 193
     COLOR2 -> RGB 0x099 at scanline 193
     COLOR2 -> RGB 0x0aa at scanline 193
     COLOR2 -> RGB 0x0bb at scanline 193
     COLOR2 -> RGB 0x0cc at scanline 193
     COLOR2 -> RGB 0x0dd at scanline 193
     COLOR2 -> RGB 0x0ee at scanline 193
     COLOR2 -> RGB 0x0ff at scanline 193
     COLOR2 -> RGB 0x1fe at scanline 193
     COLOR2 -> RGB 0x2fd at scanline 193
     COLOR2 -> RGB 0x3fc at scanline 193
     COLOR2 -> RGB 0x4fb at scanline 193
     COLOR2 -> RGB 0x5fa at scanline 193
     COLOR2 -> RGB 0x6f9 at scanline 193
     COLOR2 -> RGB 0x7f8 at scanline 193
     COLOR2 -> RGB 0x8f7 at scanline 193
     COLOR2 -> RGB 0x9f6 at scanline 193
     COLOR2 -> RGB 0xaf5 at scanline 193
     COLOR2 -> RGB 0xbf4 at scanline 193
     COLOR2 -> RGB 0xcf3 at scanline 193
     COLOR2 -> RGB 0xdf2 at scanline 193
     COLOR2 -> RGB 0xef1 at scanline 193
     COLOR2 -> RGB 0xff0 at scanline 193
     COLOR2 -> RGB 0xfe0 at scanline 193
     COLOR2 -> RGB 0xfd0 at scanline 193
     COLOR2 -> RGB 0xfc0 at scanline 193
     COLOR2 -> RGB 0xfb0 at scanline 193
     COLOR2 -> RGB 0xfa0 at scanline 193
     COLOR2 -> RGB 0xf90 at scanline 193
     COLOR2 -> RGB 0xf80 at scanline 193
     COLOR2 -> RGB 0xf70 at scanline 193
     COLOR2 -> RGB 0xf60 at scanline 193
     COLOR2 -> RGB 0xf50 at scanline 193
     COLOR2 -> RGB 0xf41 at scanline 193
     COLOR2 -> RGB 0xe32 at scanline 193
     COLOR2 -> RGB 0xd23 at scanline 193
     COLOR2 -> RGB 0xc14 at scanline 193
     COLOR2 -> RGB 0xb05 at scanline 193
     COLOR2 -> RGB 0xa06 at scanline 193
     COLOR2 -> RGB 0x907 at scanline 193
     COLOR2 -> RGB 0x808 at scanline 193
     COLOR2 -> RGB 0x709 at scanline 193
     COLOR2 -> RGB 0x60a at scanline 193
     COLOR2 -> RGB 0x50b at scanline 193
     COLOR2 -> RGB 0x40c at scanline 193
     COLOR2 -> RGB 0x30d at scanline 193
     COLOR2 -> RGB 0x20e at scanline 193
     COLOR2 -> RGB 0x10f at scanline 193
     COLOR2 -> RGB 0x00f at scanline 193
WAIT 0xc2
     COLOR2 -> RGB 0x011 at scanline 194
     COLOR2 -> RGB 0x022 at scanline 194
     COLOR2 -> RGB 0x033 at scanline 194
     COLOR2 -> RGB 0x044 at scanline 194
     COLOR2 -> RGB 0x055 at scanline 194
     COLOR2 -> RGB 0x066 at scanline 194
     COLOR2 -> RGB 0x077 at scanline 194
     COLOR2 -> RGB 0x088 at scanline 194
     COLOR2 -> RGB 0x099 at scanline 194
     COLOR2 -> RGB 0x0aa at scanline 194
     COLOR2 -> RGB 0x0bb at scanline 194
     COLOR2 -> RGB 0x0cc at scanline 194
     COLOR2 -> RGB 0x0dd at scanline 194
     COLOR2 -> RGB 0x0ee at scanline 194
     COLOR2 -> RGB 0x0ff at scanline 194
     COLOR2 -> RGB 0x1fe at scanline 194
     COLOR2 -> RGB 0x2fd at scanline 194
     COLOR2 -> RGB 0x3fc at scanline 194
     COLOR2 -> RGB 0x4fb at scanline 194
     COLOR2 -> RGB 0x5fa at scanline 194
     COLOR2 -> RGB 0x6f9 at scanline 194
     COLOR2 -> RGB 0x7f8 at scanline 194
     COLOR2 -> RGB 0x8f7 at scanline 194
     COLOR2 -> RGB 0x9f6 at scanline 194
     COLOR2 -> RGB 0xaf5 at scanline 194
     COLOR2 -> RGB 0xbf4 at scanline 194
     COLOR2 -> RGB 0xcf3 at scanline 194
     COLOR2 -> RGB 0xdf2 at scanline 194
     COLOR2 -> RGB 0xef1 at scanline 194
     COLOR2 -> RGB 0xff0 at scanline 194
     COLOR2 -> RGB 0xfe0 at scanline 194
     COLOR2 -> RGB 0xfd0 at scanline 194
     COLOR2 -> RGB 0xfc0 at scanline 194
     COLOR2 -> RGB 0xfb0 at scanline 194
     COLOR2 -> RGB 0xfa0 at scanline 194
     COLOR2 -> RGB 0xf90 at scanline 194
     COLOR2 -> RGB 0xf80 at scanline 194
     COLOR2 -> RGB 0xf70 at scanline 194
     COLOR2 -> RGB 0xf60 at scanline 194
     COLOR2 -> RGB 0xf50 at scanline 194
     COLOR2 -> RGB 0xf41 at scanline 194
     COLOR2 -> RGB 0xe32 at scanline 194
     COLOR2 -> RGB 0xd23 at scanline 194
     COLOR2 -> RGB 0xc14 at scanline 194
     COLOR2 -> RGB 0xb05 at scanline 194
     COLOR2 -> RGB 0xa06 at scanline 194
     COLOR2 -> RGB 0x907 at scanline 194
     COLOR2 -> RGB 0x808 at scanline 194
     COLOR2 -> RGB 0x709 at scanline 194
     COLOR2 -> RGB 0x60a at scanline 194
     COLOR2 -> RGB 0x50b at scanline 194
     COLOR2 -> RGB 0x40c at scanline 194
     COLOR2 -> RGB 0x30d at scanline 194
     COLOR2 -> RGB 0x20e at scanline 194
     COLOR2 -> RGB 0x10f at scanline 194
     COLOR2 -> RGB 0x00f at scanline 194
WAIT 0xc3
     COLOR2 -> RGB 0x011 at scanline 195
     COLOR2 -> RGB 0x022 at scanline 195
     COLOR2 -> RGB 0x033 at scanline 195
     COLOR2 -> RGB 0x044 at scanline 195
     COLOR2 -> RGB 0x055 at scanline 195
     COLOR2 -> RGB 0x066 at scanline 195
     COLOR2 -> RGB 0x077 at scanline 195
     COLOR2 -> RGB 0x088 at scanline 195
     COLOR2 -> RGB 0x099 at scanline 195
     COLOR2 -> RGB 0x0aa at scanline 195
     COLOR2 -> RGB 0x0bb at scanline 195
     COLOR2 -> RGB 0x0cc at scanline 195
     COLOR2 -> RGB 0x0dd at scanline 195
     COLOR2 -> RGB 0x0ee at scanline 195
     COLOR2 -> RGB 0x0ff at scanline 195
     COLOR2 -> RGB 0x1fe at scanline 195
     COLOR2 -> RGB 0x2fd at scanline 195
     COLOR2 -> RGB 0x3fc at scanline 195
     COLOR2 -> RGB 0x4fb at scanline 195
     COLOR2 -> RGB 0x5fa at scanline 195
     COLOR2 -> RGB 0x6f9 at scanline 195
     COLOR2 -> RGB 0x7f8 at scanline 195
     COLOR2 -> RGB 0x8f7 at scanline 195
     COLOR2 -> RGB 0x9f6 at scanline 195
     COLOR2 -> RGB 0xaf5 at scanline 195
     COLOR2 -> RGB 0xbf4 at scanline 195
     COLOR2 -> RGB 0xcf3 at scanline 195
     COLOR2 -> RGB 0xdf2 at scanline 195
     COLOR2 -> RGB 0xef1 at scanline 195
     COLOR2 -> RGB 0xff0 at scanline 195
     COLOR2 -> RGB 0xfe0 at scanline 195
     COLOR2 -> RGB 0xfd0 at scanline 195
     COLOR2 -> RGB 0xfc0 at scanline 195
     COLOR2 -> RGB 0xfb0 at scanline 195
     COLOR2 -> RGB 0xfa0 at scanline 195
     COLOR2 -> RGB 0xf90 at scanline 195
     COLOR2 -> RGB 0xf80 at scanline 195
     COLOR2 -> RGB 0xf70 at scanline 195
     COLOR2 -> RGB 0xf60 at scanline 195
     COLOR2 -> RGB 0xf50 at scanline 195
     COLOR2 -> RGB 0xf41 at scanline 195
     COLOR2 -> RGB 0xe32 at scanline 195
     COLOR2 -> RGB 0xd23 at scanline 195
     COLOR2 -> RGB 0xc14 at scanline 195
     COLOR2 -> RGB 0xb05 at scanline 195
     COLOR2 -> RGB 0xa06 at scanline 195
     COLOR2 -> RGB 0x907 at scanline 195
     COLOR2 -> RGB 0x808 at scanline 195
     COLOR2 -> RGB 0x709 at scanline 195
     COLOR2 -> RGB 0x60a at scanline 195
     COLOR2 -> RGB 0x50b at scanline 195
     COLOR2 -> RGB 0x40c at scanline 195
     COLOR2 -> RGB 0x30d at scanline 195
     COLOR2 -> RGB 0x20e at scanline 195
     COLOR2 -> RGB 0x10f at scanline 195
     COLOR2 -> RGB 0x00f at scanline 195
WAIT 0xd3
     COLOR0 -> RGB 0x77f at scanline 211
     COLOR1 -> RGB 0x77f at scanline 211
     COLOR2 -> RGB 0x77f at scanline 211
     COLOR3 -> RGB 0x77f at scanline 211
     COLOR4 -> RGB 0x77f at scanline 211
     COLOR5 -> RGB 0x77f at scanline 211
     COLOR6 -> RGB 0x77f at scanline 211
     COLOR7 -> RGB 0x77f at scanline 211
     COLOR8 -> RGB 0x77f at scanline 211
     COLOR9 -> RGB 0x77f at scanline 211
     COLOR10 -> RGB 0x77f at scanline 211
     COLOR11 -> RGB 0x77f at scanline 211
     COLOR12 -> RGB 0x77f at scanline 211
     COLOR13 -> RGB 0x77f at scanline 211
     COLOR14 -> RGB 0x77f at scanline 211
     COLOR15 -> RGB 0x77f at scanline 211
     COLOR18 -> RGB 0x77f at scanline 211
WAIT 0xd4
     COLOR0 -> RGB 0x555 at scanline 212
     COLOR1 -> RGB 0x555 at scanline 212
     COLOR2 -> RGB 0x555 at scanline 212
     COLOR3 -> RGB 0x555 at scanline 212
     COLOR4 -> RGB 0x555 at scanline 212
     COLOR5 -> RGB 0x555 at scanline 212
     COLOR6 -> RGB 0x555 at scanline 212
     COLOR7 -> RGB 0x555 at scanline 212
     COLOR8 -> RGB 0x555 at scanline 212
     COLOR9 -> RGB 0x555 at scanline 212
     COLOR10 -> RGB 0x555 at scanline 212
     COLOR11 -> RGB 0x555 at scanline 212
     COLOR12 -> RGB 0x555 at scanline 212
     COLOR13 -> RGB 0x555 at scanline 212
     COLOR14 -> RGB 0x555 at scanline 212
     COLOR15 -> RGB 0x555 at scanline 212
     COLOR18 -> RGB 0x555 at scanline 212
WAIT 0xff
WAIT (Skip VP >= 256 check)
     REG $1f0 -> VALUE 0x0000
WAIT 0x135
     REG $09c -> VALUE 0x8004
COPPER_END

*/
