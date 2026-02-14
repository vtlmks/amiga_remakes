// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// [=]===^=[ base setup ]============================================================^===[=]

#define BUFFER_WIDTH  (346 << 0)
#define BUFFER_HEIGHT (270 << 0)

#include "platform.c"

// [=]===^=[ remake stuff below ]============================================================^===[=]

INCBIN_UGG(picture, "data/picture.ugg");
INCBIN_BYTES(sample, "data/sound");
INCBIN_UGG(font, "data/font.ugg");

struct sample_state {
	int16_t *data;
	uint32_t size;
	uint32_t position;
	uint32_t done;				// Used for one-shot playback
};

static struct sample_state audio_sample;
static struct scroller_state triad_scroller;


#define CHAR_SIZE (16 * 8)  // 16 pixels wide * 8 pixels tall = 128 bytes per char

static struct font_char_info font_chars[128] = {
	['A'] = {.offset = CHAR_SIZE * 0,  .width = 16 },
	['B'] = {.offset = CHAR_SIZE * 1,  .width = 16 },
	['C'] = {.offset = CHAR_SIZE * 2,  .width = 16 },
	['D'] = {.offset = CHAR_SIZE * 3,  .width = 16 },
	['E'] = {.offset = CHAR_SIZE * 4,  .width = 16 },
	['F'] = {.offset = CHAR_SIZE * 5,  .width = 16 },
	['G'] = {.offset = CHAR_SIZE * 6,  .width = 16 },
	['H'] = {.offset = CHAR_SIZE * 7,  .width = 16 },
	['I'] = {.offset = CHAR_SIZE * 8,  .width = 16 },
	['J'] = {.offset = CHAR_SIZE * 9,  .width = 16 },
	['K'] = {.offset = CHAR_SIZE * 10, .width = 16 },
	['L'] = {.offset = CHAR_SIZE * 11, .width = 16 },
	['M'] = {.offset = CHAR_SIZE * 12, .width = 16 },
	['N'] = {.offset = CHAR_SIZE * 13, .width = 16 },
	['O'] = {.offset = CHAR_SIZE * 14, .width = 16 },
	['P'] = {.offset = CHAR_SIZE * 15, .width = 16 },
	['Q'] = {.offset = CHAR_SIZE * 16, .width = 16 },
	['R'] = {.offset = CHAR_SIZE * 17, .width = 16 },
	['S'] = {.offset = CHAR_SIZE * 18, .width = 16 },
	['T'] = {.offset = CHAR_SIZE * 19, .width = 16 },
	['U'] = {.offset = CHAR_SIZE * 20, .width = 16 },
	['V'] = {.offset = CHAR_SIZE * 21, .width = 16 },
	['W'] = {.offset = CHAR_SIZE * 22, .width = 16 },
	['X'] = {.offset = CHAR_SIZE * 23, .width = 16 },
	['Y'] = {.offset = CHAR_SIZE * 24, .width = 16 },
	['Z'] = {.offset = CHAR_SIZE * 25, .width = 16 },
	['a'] = {.offset = CHAR_SIZE * 0,  .width = 16 },
	['b'] = {.offset = CHAR_SIZE * 1,  .width = 16 },
	['c'] = {.offset = CHAR_SIZE * 2,  .width = 16 },
	['d'] = {.offset = CHAR_SIZE * 3,  .width = 16 },
	['e'] = {.offset = CHAR_SIZE * 4,  .width = 16 },
	['f'] = {.offset = CHAR_SIZE * 5,  .width = 16 },
	['g'] = {.offset = CHAR_SIZE * 6,  .width = 16 },
	['h'] = {.offset = CHAR_SIZE * 7,  .width = 16 },
	['i'] = {.offset = CHAR_SIZE * 8,  .width = 16 },
	['j'] = {.offset = CHAR_SIZE * 9,  .width = 16 },
	['k'] = {.offset = CHAR_SIZE * 10, .width = 16 },
	['l'] = {.offset = CHAR_SIZE * 11, .width = 16 },
	['m'] = {.offset = CHAR_SIZE * 12, .width = 16 },
	['n'] = {.offset = CHAR_SIZE * 13, .width = 16 },
	['o'] = {.offset = CHAR_SIZE * 14, .width = 16 },
	['p'] = {.offset = CHAR_SIZE * 15, .width = 16 },
	['q'] = {.offset = CHAR_SIZE * 16, .width = 16 },
	['r'] = {.offset = CHAR_SIZE * 17, .width = 16 },
	['s'] = {.offset = CHAR_SIZE * 18, .width = 16 },
	['t'] = {.offset = CHAR_SIZE * 19, .width = 16 },
	['u'] = {.offset = CHAR_SIZE * 20, .width = 16 },
	['v'] = {.offset = CHAR_SIZE * 21, .width = 16 },
	['w'] = {.offset = CHAR_SIZE * 22, .width = 16 },
	['x'] = {.offset = CHAR_SIZE * 23, .width = 16 },
	['y'] = {.offset = CHAR_SIZE * 24, .width = 16 },
	['z'] = {.offset = CHAR_SIZE * 25, .width = 16 },
	[' '] = {.offset = CHAR_SIZE * 26, .width = 16 },
	['('] = {.offset = CHAR_SIZE * 27, .width = 16 },
	[')'] = {.offset = CHAR_SIZE * 28, .width = 16 },
	[','] = {.offset = CHAR_SIZE * 29, .width = 16 },
	[':'] = {.offset = CHAR_SIZE * 30, .width = 16 },
	['?'] = {.offset = CHAR_SIZE * 31, .width = 16 },
	['-'] = {.offset = CHAR_SIZE * 32, .width = 16 },
	['1'] = {.offset = CHAR_SIZE * 33, .width = 16 },
	['2'] = {.offset = CHAR_SIZE * 34, .width = 16 },
	['3'] = {.offset = CHAR_SIZE * 35, .width = 16 },
	['4'] = {.offset = CHAR_SIZE * 36, .width = 16 },
	['5'] = {.offset = CHAR_SIZE * 37, .width = 16 },
	['6'] = {.offset = CHAR_SIZE * 38, .width = 16 },
	['7'] = {.offset = CHAR_SIZE * 39, .width = 16 },
	['8'] = {.offset = CHAR_SIZE * 40, .width = 16 },
	['9'] = {.offset = CHAR_SIZE * 41, .width = 16 },
	['0'] = {.offset = CHAR_SIZE * 42, .width = 16 },
	['.'] = {.offset = CHAR_SIZE * 43, .width = 16 },
	['!'] = {.offset = CHAR_SIZE * 44, .width = 16 },
	['\''] = {.offset = CHAR_SIZE * 45, .width = 16 },
};

// static struct font_char_info font_chars[128] = {
// 	[' '] = {.offset =     0,  .width = 32 },
// 	['!'] = {.offset =    32,  .width = 13 },
// 	['"'] = {.offset =    45,  .width = 15 },
// 	['#'] = {.offset =    60,  .width = 32 },
// 	['$'] = {.offset =    92,  .width = 23 },
// 	['%'] = {.offset =   115,  .width = 21 },
// 	['&'] = {.offset =   136,  .width = 32 },
// 	['\''] = {.offset =   168,  .width =  7 },
// 	['('] = {.offset =   175,  .width = 21 },
// 	[')'] = {.offset =   196,  .width = 21 },
// 	['*'] = {.offset =   217,  .width = 32 },
// 	['+'] = {.offset =   249,  .width = 18 },
// 	[','] = {.offset =   267,  .width =  9 },
// 	['-'] = {.offset =   276,  .width = 17 },
// 	['.'] = {.offset =   293,  .width =  9 },
// 	['/'] = {.offset =   302,  .width = 21 },
// 	['0'] = {.offset =   323,  .width = 32 },
// 	['1'] = {.offset =   355,  .width = 13 },
// 	['2'] = {.offset =   368,  .width = 24 },
// 	['3'] = {.offset =   392,  .width = 25 },
// 	['4'] = {.offset =   417,  .width = 28 },
// 	['5'] = {.offset =   445,  .width = 25 },
// 	['6'] = {.offset =   470,  .width = 24 },
// 	['7'] = {.offset =   494,  .width = 24 },
// 	['8'] = {.offset =   518,  .width = 25 },
// 	['9'] = {.offset =   543,  .width = 24 },
// 	[':'] = {.offset =   567,  .width = 10 },
// 	[';'] = {.offset =   577,  .width = 10 },
// 	['<'] = {.offset =   587,  .width = 16 },
// 	['='] = {.offset =   603,  .width = 26 },
// 	['>'] = {.offset =   629,  .width = 16 },
// 	['?'] = {.offset =   645,  .width = 18 },
// 	['@'] = {.offset =   663,  .width = 28 },
// 	['A'] = {.offset =   691,  .width = 32 },
// 	['B'] = {.offset =   723,  .width = 27 },
// 	['C'] = {.offset =   750,  .width = 27 },
// 	['D'] = {.offset =   777,  .width = 28 },
// 	['E'] = {.offset =   805,  .width = 27 },
// 	['F'] = {.offset =   832,  .width = 27 },
// 	['G'] = {.offset =   859,  .width = 27 },
// 	['H'] = {.offset =   886,  .width = 28 },
// 	['I'] = {.offset =   914,  .width = 13 },
// 	['J'] = {.offset =   927,  .width = 21 },
// 	['K'] = {.offset =   948,  .width = 32 },
// 	['L'] = {.offset =   980,  .width = 27 },
// 	['M'] = {.offset =  1007,  .width = 32 },
// 	['N'] = {.offset =  1039,  .width = 27 },
// 	['O'] = {.offset =  1066,  .width = 24 },
// 	['P'] = {.offset =  1090,  .width = 24 },
// 	['Q'] = {.offset =  1114,  .width = 26 },
// 	['R'] = {.offset =  1140,  .width = 24 },
// 	['S'] = {.offset =  1164,  .width = 27 },
// 	['T'] = {.offset =  1191,  .width = 29 },
// 	['U'] = {.offset =  1220,  .width = 24 },
// 	['V'] = {.offset =  1244,  .width = 30 },
// 	['W'] = {.offset =  1274,  .width = 32 },
// 	['X'] = {.offset =  1306,  .width = 25 },
// 	['Y'] = {.offset =  1331,  .width = 26 },
// 	['Z'] = {.offset =  1357,  .width = 27 },
// 	['['] = {.offset =  1384,  .width = 17 },
// 	['\\'] = {.offset =  1401,  .width = 32 },
// 	[']'] = {.offset =  1433,  .width = 17 },
// 	['^'] = {.offset =  1450,  .width = 32 },
// 	['_'] = {.offset =  1482,  .width = 32 },
// 	['`'] = {.offset =  1514,  .width = 32 },
// 	['a'] = {.offset =  1546,  .width = 25 },
// 	['b'] = {.offset =  1571,  .width = 22 },
// 	['c'] = {.offset =  1593,  .width = 22 },
// 	['d'] = {.offset =  1615,  .width = 22 },
// 	['e'] = {.offset =  1637,  .width = 23 },
// 	['f'] = {.offset =  1660,  .width = 18 },
// 	['g'] = {.offset =  1678,  .width = 25 },
// 	['h'] = {.offset =  1703,  .width = 22 },
// 	['i'] = {.offset =  1725,  .width =  9 },
// 	['j'] = {.offset =  1734,  .width =  9 },
// 	['k'] = {.offset =  1743,  .width = 21 },
// 	['l'] = {.offset =  1764,  .width =  9 },
// 	['m'] = {.offset =  1773,  .width = 32 },
// 	['n'] = {.offset =  1805,  .width = 25 },
// 	['o'] = {.offset =  1830,  .width = 25 },
// 	['p'] = {.offset =  1855,  .width = 25 },
// 	['q'] = {.offset =  1880,  .width = 25 },
// 	['r'] = {.offset =  1905,  .width = 18 },
// 	['s'] = {.offset =  1923,  .width = 23 },
// 	['t'] = {.offset =  1946,  .width = 13 },
// 	['u'] = {.offset =  1959,  .width = 25 },
// 	['v'] = {.offset =  1984,  .width = 26 },
// 	['w'] = {.offset =  2010,  .width = 32 },
// 	['x'] = {.offset =  2042,  .width = 19 },
// 	['y'] = {.offset =  2061,  .width = 26 },
// 	['z'] = {.offset =  2087,  .width = 20 },
// 	['{'] = {.offset =  2107,  .width = 22 },
// 	['|'] = {.offset =  2129,  .width =  9 },
// 	['}'] = {.offset =  2138,  .width = 22 },
// 	['~'] = {.offset =  2160,  .width = 32 },
// 	[127] = {.offset =  2192,  .width = 32 },
// };

static uint8_t scroll_text[] = {
	"                     here is 'zeke wolf' for  t r i a d  with our first demo! i'm "
	"sorry that i can't programming any demo myself, but i will send thanks to "
	"supremacy for this good 'demo-creator'! maybe you all also have seen the old "
	"piccy but i think you all will see my headquarter!! ok the music is also old "
	"but i like the film 'commando' so why not! right now i'm the only member on "
	"amiga in  t r i a d  but very soon a new member will enjoy us! his "
	"computer-name will be mr. midi! i have already got a lot of very good demos "
	"and i'm a little bit envious on all guys which are so good in programming! "
	"i think i'm to old for that but i will try to do my best! however i think "
	"you now are tired of all this shit so over to our amiga-greetings! it's not "
	"in alpha-order or any form of ratings!             so here comes  t r i a d "
	"'s amiga-hi to: the movers, mfc, the light circle, the task force, danish "
	"gold, 1001 crew, irata, red sector, micro mix, the new age, tsk, pct, hqc, "
	"action 2009, afl, crm crew, star frontiers, bst, hotline, kent team, delta "
	"force, ecc, fairlight, vision, gcc, amigo, relax, tdc, krabat, lda, new "
	"edition, eca, supremacy, dusa-team, popeye, wom, yeti, megabyte, ds "
	"compware, guru master, the prophets, the judges, tcs, headbanger, syntax "
	"2001, rdi, digital arts, plutonium crackers, tst, toc, sodan, megabyte, usr, "
	"reflex, jcf, errorsoft, rbb, electro, acf, bks, bamiga sector one, tcb, "
	"major rom, acs, adj, skyline, gss, asc, sss, 007 firma, skar, the hunter, "
	"bfbs, snoopy, e605 international and all other amiga-freaks i have forgot to "
	"mention!   ok i will also send my greetings to all my friends on c-64 in  t "
	"r i a d  and wish them good luck! but i really hope that mr.z - our "
	"'super-cracker' on c-64 - soon will be a hero on  a m i g a  too!! "
	"hope to get in touch with new and good contacts soon!          my address(!) "
	"yeah i think you can get, coz i will not write it here! i think it's to "
	"danger to do that!    oh by the way - can anyone fix me a real version of "
	"'chessmaster 2000' !! not the beta-version from 1986(!) coz it's not "
	"correct! you see i'm a chessplayer since 27(!) years ago and i'm still "
	"wanted the real(!) version!! it would be nice of you if you can help me with "
	"that! i know that many of you don't like to play chess - it's the best game "
	"you ever can play - but i think that most of you always wanted the best and "
	"correct version of every games! in german and us-magazines they have "
	"released the game (chessmaster 2000) so i think that it will not be so hard "
	"for you to fix it! here in sweden i haven't seen the original and i also "
	"know that the game haven't been released in sweden yet!   before i end this "
	"text i will also send a message to  'danish gold' ! thank's for the nice "
	"copy-party you had in last summer! sorry that i (zeke) didn't have so much "
	"of time to spend in your 'copy-place'! as you already know (you met them) i "
	"were in odense 2 days with my family! the second day we were in 'legoland'! "
	"nice place but the weather wasn't so good! and after that - in monday and no "
	"danish money left - we must go to sweden again! but it was nice to meet some "
	"hackers like you (danish gold), 1001 crew, duke (tlc), dusa-team, rdi, "
	"fairlight, relax and some others! i hope to be on another copy-party a "
	"little bit longer another time!                          ok it's now time to "
	"say:                              bye-bye!                              z e "
	"k e  w o l f                             from                              t "
	"r i a d  !!                           (870821)                            the "
	"text will soon start again !  press left mouse button for exit or stay "
	"around for next lap!                                        "
};

static uint32_t scroller_color_cycle[70] = {
	0x220022ff, 0x440044ff, 0x660066ff, 0x880088ff, 0xaa00aaff, 0xcc00ccff, 0xee00eeff, 0xee00eeff,
	0xcc00ccff, 0xaa00aaff, 0x880088ff, 0x660066ff, 0x440044ff, 0x220022ff, 0x002200ff, 0x004400ff,
	0x006600ff, 0x008800ff, 0x00aa00ff, 0x00cc00ff, 0x00ee00ff, 0x00ee00ff, 0x00cc00ff, 0x00aa00ff,
	0x008800ff, 0x006600ff, 0x004400ff, 0x002200ff, 0x220000ff, 0x440000ff, 0x660000ff, 0x880000ff,
	0xaa0000ff, 0xcc0000ff, 0xee0000ff, 0xee0000ff, 0xcc0000ff, 0xaa0000ff, 0x880000ff, 0x660000ff,
	0x440000ff, 0x220000ff, 0x002222ff, 0x004444ff, 0x006666ff, 0x008888ff, 0x00aaaaff, 0x00ccccff,
	0x00eeeeff, 0x00eeeeff, 0x00ccccff, 0x00aaaaff, 0x008888ff, 0x006666ff, 0x004444ff, 0x002222ff,
	0x222200ff, 0x444400ff, 0x666600ff, 0x888800ff, 0xaaaa00ff, 0xcccc00ff, 0xeeee00ff, 0xeeee00ff,
	0xcccc00ff, 0xaaaa00ff, 0x888800ff, 0x666600ff, 0x444400ff, 0x222200ff,
};

static uint32_t color_cycle_index;

static void render_scroll_buffer(struct platform_state *state, struct scroller_state *scr_state) {
	uint32_t *scroll_dest = BUFFER_PTR(state, 0, scr_state->dest_offset_y);
	uint8_t *scroll_src = scr_state->buffer;

	uint32_t temp_color_cycle_index = color_cycle_index;

	size_t base_src_index = (scr_state->char_render_offset - 370) & SCROLL_BUFFER_MASK;
	for(size_t i = 0; i < scr_state->char_height; ++i) {
		scr_state->font->palette[1] = scroller_color_cycle[temp_color_cycle_index++ % 70];
		for(uint32_t j = 0; j < state->buffer_width; ++j) {
			size_t src_index = (base_src_index + j) & SCROLL_BUFFER_MASK;
			uint8_t color_index = scroll_src[src_index];
			scr_state->font->palette[0] = scroll_dest[j];
			scroll_dest[j] = scr_state->font->palette[color_index];
		}
		scroll_dest += state->buffer_width;
		scroll_src += SCROLL_BUFFER_WIDTH;
	}
	if((state->frame_number % 4) == 0) {
		color_cycle_index++;
	}
}

static void process_sampled_audio(int16_t *audio_buffer, size_t frames, struct sample_state *sample) {
	for(size_t i = 0; i < frames; ++i) {
		int16_t processed_sample = (int16_t)(sample->data[sample->position] * 0.707f);
		*audio_buffer++ = processed_sample;
		*audio_buffer++ = processed_sample;
		if(++sample->position == sample->size) {
			sample->position = 0;
		}
	}
}

// [=]===^=[ audio_callback ]============================================================^===[=]
static void remake_audio_callback(int16_t *data, size_t frames) {
	process_sampled_audio(data, frames, &audio_sample);
}

// [=]===^=[ remake_init ]============================================================^===[=]
static void remake_init(struct platform_state *state) {
	platform_change_resolution(state, BUFFER_WIDTH, BUFFER_HEIGHT);

	audio_sample.data = resample_audio((int8_t*)sample, sample_end - sample, 358, &audio_sample.size);

	triad_scroller = (struct scroller_state) {
		.char_width = 16,
		.char_height = 8,
		.dest_offset_y = 231,
		.speed = 2,
		.text = scroll_text,
		.font = font,
		.char_info = font_chars
	};
	scroller_new(&triad_scroller);

	mkfw_audio_callback = remake_audio_callback;
}

static void remake_options(struct platform_state *state) {
	state->release_group = "Triad";
	state->release_title = "First Demo";
	state->window_title = "Triad - First Demo - 1987-08\0";
}

static uint32_t copper_bar_colors[] = {
	0x333333ff, 0x666666ff, 0x999999ff, 0xccccccff, 0xffffffff, 0xccccccff, 0x999999ff, 0x666666ff, 0x333333ff
};

static void render_copper_bar(struct platform_state *state, uint32_t y) {
	uint32_t *dst = BUFFER_PTR(state, 0, y);

	for(size_t y = 0; y < 9; y++, dst += state->buffer_width) {
		uint32_t c = copper_bar_colors[y];
		for(size_t x = 0; x < state->buffer_width; ++x) {
			dst[x] = c;
		}
	}
}

// [=]===^=[ remake_frame ]============================================================^===[=]
static void remake_frame(struct platform_state *state) {
	platform_clear_buffer(state);

	render_copper_bar(state, 219);
	render_copper_bar(state, 241);

	struct rect source_clip = { 0, 9, picture->width, picture->height };
	blit_full_dst(state, picture, source_clip, CENTER_X(state, picture->width), 0, 0);
	scroller_update(state, &triad_scroller);
	render_scroll_buffer(state, &triad_scroller);
}

// [=]===^=[ remake_shutdown ]============================================================^===[=]
static void remake_shutdown(struct platform_state *state) {
	mkfw_audio_callback = 0;
	free(audio_sample.data);
}
