// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

/*
 *
 */

INCBIN_UGG(p5_faces_data, "data/p5_faces.ugg");
INCBIN_UGG(p5_large_font_data, "data/p5_large_font.ugg");
INCBIN_UGG(p5_rogue_40k_logo_data, "data/p5_rogue_40k_logo.ugg");
INCBIN_UGG(p5_warhammer_logo_data, "data/p5_warhammer_logo.ugg");
INCBIN_UGG(p5_amiga_logo_data, "data/p5_amiga_logo.ugg");
INCBIN_UGG(p5_atom_demo_ii_logo_data, "data/p5_atom_demo_ii_logo.ugg");
INCBIN_UGG(p5_atom_logo_data, "data/p5_atom_logo.ugg");
INCBIN_UGG(p5_jazze_logo_data, "data/p5_jazze_logo.ugg");
INCBIN_UGG(p5_members_logo_data, "data/p5_members_logo.ugg");
INCBIN_UGG(p5_northstar_logo_data, "data/p5_northstar_logo.ugg");
INCBIN_UGG(p5_starfire_logo_data, "data/p5_starfire_logo.ugg");
INCBIN_UGG(p5_swap_logo_data, "data/p5_swap_logo.ugg");
INCBIN_UGG(p5_ball_blue_data, "data/p5_ball_blue.ugg");
INCBIN_UGG(p5_ball_grey_data, "data/p5_ball_grey.ugg");
INCBIN_BYTES(p5_scrolltext1_data, "data/p5_scrolltext1.dat");
INCBIN_BYTES(p5_scrolltext2_data, "data/p5_scrolltext2.dat");

// static struct ugg *p5_faces_data					= (struct ugg*)_p5_faces_data;
// static struct ugg *p5_large_font_data			= (struct ugg*)_p5_large_font_data;
// static struct ugg *p5_rogue_40k_logo_data		= (struct ugg*)_p5_rogue_40k_logo_data;
// static struct ugg *p5_warhammer_logo_data		= (struct ugg*)_p5_warhammer_logo_data;
// static struct ugg *p5_amiga_logo_data			= (struct ugg*)_p5_amiga_logo_data;
// static struct ugg *p5_atom_demo_ii_logo_data	= (struct ugg*)_p5_atom_demo_ii_logo_data;
// static struct ugg *p5_atom_logo_data			= (struct ugg*)_p5_atom_logo_data;
// static struct ugg *p5_jazze_logo_data			= (struct ugg*)_p5_jazze_logo_data;
// static struct ugg *p5_members_logo_data		= (struct ugg*)_p5_members_logo_data;
// static struct ugg *p5_northstar_logo_data		= (struct ugg*)_p5_northstar_logo_data;
// static struct ugg *p5_starfire_logo_data		= (struct ugg*)_p5_starfire_logo_data;
// static struct ugg *p5_swap_logo_data			= (struct ugg*)_p5_swap_logo_data;
// static struct ugg *p5_ball_blue_data			= (struct ugg*)_p5_ball_blue_data;
// static struct ugg *p5_ball_grey_data			= (struct ugg*)_p5_ball_grey_data;


static const uint8_t atom_logo_sin_data[] = {
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x2, 0x2, 0x3, 0x3, 0x4, 0x4, 0x5, 0x5, 0x6, 0x6, 0x7, 0x7, 0x8, 0x8, 0x9, 0x9, 0xa, 0xa, 0xb, 0xb, 0xc, 0xc, 0xd,
	0xd, 0xe, 0xe, 0xf, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x8, 0x7, 0x8, 0x6, 0x9, 0x5, 0xa, 0x4, 0xb, 0x3, 0xc, 0x2, 0xd, 0x1, 0xe, 0x0, 0xf, 0x0, 0xf, 0x0,	// 32
	0xf, 0x0, 0xf, 0x0, 0xf, 0x0, 0xf, 0x0, 0xf, 0x1, 0xe, 0x1, 0xe, 0x2, 0xd, 0x2, 0xd, 0x3, 0xc, 0x3, 0xc, 0x4, 0xb, 0x4, 0xb, 0x5, 0xa, 0x5, 0xa, 0x6, 0x9, 0x6,	// 64
	0x9, 0x7, 0x8, 0x7, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x7, 0x7, 0x7, 0x6, 0x6, 0x5, 0x5, 0x4, 0x4, 0x3, 0x2, 0x3, 0x4, 0x4, 0x5,	// 96
	0x5, 0x6, 0x6, 0x7, 0x7, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0, 0x1, 0x1, 0x1,	// 128
	0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x2, 0x3, 0x3, 0x4, 0x4, 0x4, 0x5, 0x5, 0x5, 0x5, 0x6, 0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x7, 0x7, 0x6, 0x6, 0x6, 0x6, 0x5, 0x5,	// 160
	0x5, 0x5, 0x4, 0x4, 0x4, 0x3, 0x3, 0x2, 0x1, 0x0, 0x2, 0x4, 0x6, 0x8, 0xa, 0xa, 0xc, 0xc, 0xe, 0xe, 0xe, 0xc, 0xc, 0xa, 0xa, 0x8, 0x8, 0x7, 0x8, 0x5, 0xa, 0x3,	// 192
	0xc, 0x1, 0xe, 0x0, 0xf, 0x1, 0xe, 0x3, 0xc, 0x5, 0xa, 0x7, 0x8, 0x8, 0x8, 0x7, 0x8, 0x5, 0xa, 0x3, 0xc, 0x1, 0xe, 0x0, 0xf, 0x1, 0xe, 0x3, 0xc, 0x5, 0xa, 0x7,	// 224
	0x8, 0x8, 0x8, 0x7, 0x8, 0x5, 0xa, 0x3, 0xc, 0x1, 0xe, 0x0, 0xf, 0x0, 0xe, 0x0, 0xe, 0x0, 0xd, 0x0, 0xd, 0x0, 0xc, 0x0, 0xc, 0x0, 0xb, 0x0, 0xb, 0x0, 0xa, 0x0,	// 256
	0xa, 0x0, 0x9, 0x0, 0x9, 0x0, 0x8, 0x0, 0x8, 0x0, 0x7, 0x0, 0x7, 0x0, 0x6, 0x0, 0x6, 0x0, 0x5, 0x0, 0x5, 0x0, 0x4, 0x0, 0x4, 0x0, 0x3, 0x0, 0x3, 0x0, 0x2, 0x0,	// 288
	0x2, 0x0, 0x1, 0x0, 0x1, 0x0, 0x0																																											// 320	- 326
};

static uint32_t md1_p5_large_copper_bars[] = {	// start at line 111
	0x001111ff, 0x002222ff, 0x003333ff, 0x004444ff, 0x005555ff, 0x006666ff, 0x007777ff, 0x008888ff,
	0x009999ff, 0x00aaaaff, 0x00bbbbff, 0x00ccccff, 0x00ddddff, 0x00eeeeff, 0x00ffffff, 0x00ffffff,
	0x00eeeeff, 0x00ddddff, 0x00ccccff, 0x00bbbbff, 0x00aaaaff, 0x009999ff, 0x008888ff, 0x007777ff,
	0x006666ff, 0x005555ff, 0x004444ff, 0x003333ff, 0x002222ff, 0x001111ff, 0x000000ff, 0x001111ff,
	0x002222ff, 0x003333ff, 0x004444ff, 0x005555ff, 0x006666ff, 0x007777ff, 0x008888ff, 0x009999ff,
	0x00aaaaff, 0x00bbbbff, 0x00ccccff, 0x00ddddff, 0x00eeeeff, 0x00ffffff, 0x00ffffff, 0x00eeeeff,
	0x00ddddff, 0x00ccccff, 0x00bbbbff, 0x00aaaaff, 0x009999ff, 0x008888ff, 0x007777ff, 0x006666ff,
	0x005555ff, 0x004444ff, 0x003333ff,
	0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
	0x001111ff, 0x002222ff, 0x003333ff, 0x004444ff, 0x005555ff, 0x006666ff, 0x007777ff, 0x008888ff,
	0x009999ff, 0x00aaaaff, 0x00bbbbff, 0x00ccccff, 0x00ddddff, 0x00eeeeff, 0x00ffffff, 0x00ffffff,
	0x00eeeeff, 0x00ddddff, 0x00ccccff, 0x00bbbbff, 0x00aaaaff, 0x009999ff, 0x008888ff, 0x007777ff,
	0x006666ff, 0x005555ff, 0x004444ff, 0x003333ff, 0x002222ff, 0x001111ff, 0x000000ff, 0x001111ff,
};

static const uint32_t md1_p5_large_scroller_outer_colors[] = {
	0x66ffaaff, 0x66eeaaff, 0x66ddaaff, 0x66ccaaff, 0x66bbaaff, 0x66aaaaff, 0x6699aaff, 0x6688aaff,
	0x6677aaff, 0x6666aaff, 0x6655aaff, 0x6644aaff, 0x6633aaff, 0x6622aaff, 0x6611aaff, 0x6600aaff,
	0x6611aaff, 0x6622aaff, 0x6633aaff, 0x6644aaff, 0x6655aaff, 0x6666aaff, 0x6677aaff, 0x6688aaff,
	0x6699aaff, 0x66aaaaff, 0x66bbaaff, 0x66ccaaff, 0x66ddaaff, 0x66eeaaff, 0x66ffaaff, 0x66eeaaff,
};

static const uint32_t md1_p5_large_scroller_inner_color1[] = {
	0x000000ff, 0x000000ff, 0xff2200ff, 0xff3300ff, 0xff4400ff, 0xff5500ff, 0xff6600ff, 0xff7700ff,
	0xff8800ff, 0xff9900ff, 0xffaa00ff, 0xffbb00ff, 0xffcc00ff, 0xffdd00ff, 0xffee00ff, 0xffff00ff,
	0xffee00ff, 0xffdd00ff, 0xffcc00ff, 0xffbb00ff, 0xffaa00ff, 0xff9900ff, 0xff8800ff, 0xff7700ff,
	0xff6600ff, 0xff5500ff, 0xff4400ff, 0xff3300ff, 0xff2200ff, 0xff1100ff, 0x000000ff, 0x000000ff,
};

static const uint32_t md1_p5_large_scroller_inner_color2[] = {
	0x000000ff, 0x000000ff, 0xff2288ff, 0xff3388ff, 0xff4488ff, 0xff5588ff, 0xff6688ff, 0xff7788ff,
	0xff8888ff, 0xff9988ff, 0xffaa88ff, 0xffbb88ff, 0xffcc88ff, 0xffdd88ff, 0xffee88ff, 0xffff88ff,
	0xffee88ff, 0xffdd88ff, 0xffcc88ff, 0xffbb88ff, 0xffaa88ff, 0xff9988ff, 0xff8888ff, 0xff7788ff,
	0xff6688ff, 0xff5588ff, 0xff4488ff, 0xff3388ff, 0xff2288ff, 0xff1188ff, 0x000000ff, 0x000000ff,
};

static const uint32_t md1_p5_small_scroller_inner_color[] = {
	0xff0000ff, 0xff1100ff, 0xff2200ff, 0xff3300ff, 0xff4400ff, 0xff5500ff, 0xff6600ff, 0xff7700ff,
	0xff8800ff, 0xff9900ff, 0xffaa00ff, 0xffbb00ff, 0xffcc00ff, 0xffdd00ff, 0xffee00ff, 0xffff00ff,
};

// grey 30  rogue trader
static uint32_t p5_rogue_palette[30] = {
	0xffffffff, 0xeeeeeeff, 0xddddddff, 0xccccccff, 0xbbbbbbff, 0xaaaaaaff, 0x999999ff, 0x888888ff, 0x777777ff, 0x666666ff,
	0x555555ff, 0x444444ff, 0x333333ff, 0x222222ff, 0x111111ff, 0x000000ff, 0x111111ff, 0x222222ff, 0x333333ff, 0x444444ff,
	0x555555ff, 0x666666ff, 0x777777ff, 0x888888ff, 0x999999ff, 0xaaaaaaff, 0xbbbbbbff, 0xccccccff, 0xddddddff, 0xeeeeeeff,
};

// blue 30  40,000
static uint32_t p5_40k_palette[30] = {
	0xffffffff, 0xeeeeffff, 0xddddffff, 0xccccffff, 0xbbbbffff, 0xaaaaffff, 0x9999ffff, 0x8888ffff, 0x7777ffff, 0x6666ffff,
	0x5555ffff, 0x4444ffff, 0x3333ffff, 0x2222ffff, 0x1111ffff, 0x0000ffff, 0x1111ffff, 0x2222ffff, 0x3333ffff, 0x4444ffff,
	0x5555ffff, 0x6666ffff, 0x7777ffff, 0x8888ffff, 0x9999ffff, 0xaaaaffff, 0xbbbbffff, 0xccccffff, 0xddddffff, 0xeeeeffff,
};

// RED 30  warhammer
static uint32_t p5_warhammer_palette[30] = {
	0xffffffff, 0xffeeeeff, 0xffddddff, 0xffccccff, 0xffbbbbff, 0xffaaaaff, 0xff9999ff, 0xff8888ff, 0xff7777ff, 0xff6666ff,
	0xff5555ff, 0xff4444ff, 0xff3333ff, 0xff2222ff, 0xff1111ff, 0xff0000ff, 0xff1111ff, 0xff2222ff, 0xff3333ff, 0xff4444ff,
	0xff5555ff, 0xff6666ff, 0xff7777ff, 0xff8888ff, 0xff9999ff, 0xffaaaaff, 0xffbbbbff, 0xffccccff, 0xffddddff, 0xffeeeeff,
};

static uint32_t md1_p5_atom_logo_offset;

//  this is to be used to calculate the Y offset for each logo,
//   First column: y offset, -- divide this by 132 to get Y offset.
//  Second column: logo data offset from a poiner,
//   Third column: modulo,
//  Fourth column: blit-size
//
// RAM:0002C0A4 word_2C0A4:     dc.w  0x840,    0,  0x6A, 0x70D	- atom demo ii
// RAM:0002C0A4                 dc.w  0xA50, 0x2D8,  0x6C, 0x50C	- jazze
// RAM:0002C0A4                 dc.w  0x840, 0x4B8,  0x6A, 0x90D	- starfire
// RAM:0002C0A4                 dc.w   0x84, 0x860,  0x74, 0xE48	- atom
// RAM:0002C0A4                 dc.w  0x528, 0xBF0,  0x66, 0xA0F	- northstar
// RAM:0002C0A4                 dc.w  0x294,0x10A0,  0x6A, 0xC4D	- amiga
// RAM:0002C0A4                 dc.w     0,0x159A,  0x5E, 0xF13	- members
// RAM:0002C0A4                 dc.w  0x18C,0x1E82,  0x66, 0xD8F	- swap

// 0x840 (2112) / 132 = 16
// 0xA50 (2640) / 132 = 20
// 0x840 (2112) / 132 = 16
//   0x84 (132) / 132 = 1
// 0x528 (1320) / 132 = 10
//  0x294 (660) / 132 = 5
//        0 (0) / 132 = 0
//  0x18C (396) / 132 = 3

// struct p5_logo {
// 	uint32_t y_offset;
// 	struct ugg *image;
// };

// static const struct p5_logo p5_logos[] = {
// 	{16, p5_atom_demo_ii_logo_data},
// 	{20, p5_jazze_logo_data},
// 	{16, p5_starfire_logo_data},
// 	{1,  p5_atom_logo_data},
// 	{10, p5_northstar_logo_data},
// 	{5,  p5_amiga_logo_data},
// 	{0,  p5_members_logo_data},
// 	{3,  p5_swap_logo_data},
// };

// 2-Word Animation Start:
// 	Start Side (0 = Right, 1 = Left)
// 	Logo Index
// 3-Word Animation Step:   repeat until 0xffff
// 	Direction
// 	Speed
// 	Duration
// 0xffff - end of sequence, restart from 2-word animation start
// 0xaaaa - end of all sequences, restart from beginning.

static const uint32_t animation_data[] = {
	0x0, 0x4,
	0x1, 0x1, 0x1a6,
	0x1, 0x0, 0x190,
	0x0, 0x5, 0x28,
	0x1, 0xa, 0x32,
	0xffff,
	0x1, 0x6,
	0x0, 0xa, 0x20,
	0x0, 0x0, 0xc8,
	0x1, 0x1, 0x12c,
	0x0, 0xa, 0x42,
	0xffff,
	0x0, 0x0,
	0x1, 0x3, 0xeb,
	0xffff,
	0x1, 0x5,
	0x0, 0x5, 0x71,
	0xffff,
	0x1, 0x3,
	0x0, 0x1,  0xc8,
	0x1, 0xa,  0x15,
	0xffff,
	0x0, 0x2,
	0x1, 0x8, 0x58,
	0xffff,
	0x1, 0x1,
	0x0, 0x8, 0x48,
	0xffff,
	0x1, 0x7,
	0x0, 0x2, 0x94,
	0x1, 0x0, 0x1f4,
	0x1, 0x2, 0x96,
	0xffff,
	0x1, 0x0,
	0x0, 0x5, 0x38,
	0x1, 0x0, 0x32,
	0x0, 0x5, 0x14,
	0x1, 0x2, 0xbe,
	0xffff,
	0x0, 0x2,
	0x1, 0x1, 0x1a8,
	0x0, 0x0, 0x50,
	0x0, 0x5, 0x14,
	0x1, 0x5, 0x28,
	0x0, 0x5, 0x28,
	0x1, 0x5, 0x28,
	0x0, 0x5, 0x14,
	0x0, 0x1, 0x180,
	0xffff,
	0x0, 0x4,
	0x1, 0x8, 0x58,
	0xffff,
	0x1, 0x1,
	0x0, 0x8, 0x48,
	0xffff,
	0x0, 0x6,
	0x1, 0x6, 0x3f,
	0x0, 0x0, 0x5a,
	0x0, 0x5, 0xa,
	0x1, 0x5, 0x14,
	0x0, 0x5, 0xa,
	0x1, 0x6, 0x36,
	0xffff,
	0x1, 0x3,
	0x0, 0x8, 0x26,
	0x1, 0x8, 0x28,
	0x0, 0x8, 0x28,
	0x1, 0x8, 0x23,
	0x0, 0x8, 0x23,
	0x1, 0x8, 0x1e,
	0x0, 0x8, 0x1e,
	0x1, 0x8, 0x19,
	0x0, 0x8, 0x19,
	0x1, 0x8, 0x14,
	0x0, 0x8, 0x14,
	0x1, 0x8, 0xf,
	0x0, 0x8, 0xf,
	0x1, 0x8, 0xa,
	0x0, 0x8, 0xa,
	0x1, 0x8, 0x5,
	0x0, 0x0, 0xfa,
	0x0, 0x2, 0x78,
	0xffff,
	0x1, 0x5,
	0x0, 0x4, 0x46,
	0x0, 0x0, 0x64,
	0x1, 0x4, 0x4b,
	0xffff,
	0x1, 0x1,
	0x0, 0x4, 0x44,
	0x0, 0x0, 0x32,
	0x0, 0x4, 0x1e,
	0x1, 0x4, 0x1e,
	0x0, 0x0, 0x32,
	0x1, 0x4, 0x50,
	0xaaaa
};

// SO SO MUCH FUN
static uint8_t p5_sprite_sine[] = {
	0x00, 0x05, 0x0a, 0x0e, 0x12, 0x15, 0x18, 0x1b, 0x1e, 0x20, 0x22, 0x24, 0x25, 0x26, 0x27, 0x28, 0x28, 0x29,
	0x29, 0x28, 0x28, 0x27, 0x26, 0x25, 0x24, 0x22, 0x20, 0x1e, 0x1b, 0x18, 0x15, 0x12, 0x0e, 0x0e, 0x0a, 0x05,
	0xff,
};

static struct scroller_state p5_scroller1;
static struct scroller_state p5_scroller2;

static uint8_t p5_process_char(struct scroller_state *scr_state, uint8_t scroll_character) {
	if(scroll_character == '^') {		// Scrolltext end
		scr_state->text_offset = 0;
		return scr_state->text[scr_state->text_offset++];
	}
	return scroll_character;
}

static void p5_init() {
	p5_scroller1 = (struct scroller_state){ .char_width = 16, .char_height = 16, .dest_offset_y =  78, .speed = 1, .text = (uint8_t*)p5_scrolltext1_data, .font = part1_small_font_data, .process_char = p5_process_char };
	p5_scroller2 = (struct scroller_state){ .char_width = 32, .char_height = 32, .dest_offset_y = 170, .speed = 2, .text = (uint8_t*)p5_scrolltext2_data, .font = p5_large_font_data, .process_char = p5_process_char };
	scroller_new(&p5_scroller1);
	scroller_new(&p5_scroller2);
}

static void p5_shutdown() {
	scroller_remove(&p5_scroller1);
	scroller_remove(&p5_scroller2);
}

static uint32_t p5_logo_background_colors[59];
static void p5_recalculate_background(uint32_t color) {
	int32_t direction = -1;
	uint32_t counter = 0xf;

	for(uint32_t i = 0; i < 59; ++i) {
		p5_logo_background_colors[i] = color;
		color += 0x110000 * direction;

		counter = (counter + direction) & 0xf;
		direction = (counter == 0 || counter == 0xf) ? -direction : direction;
	}
}

struct sprite {
	struct ugg *sprite;
	uint32_t sine_index;
	int32_t xpos;
	int32_t direction;
};

static struct sprite sprites[4] = {
	{ 0,  0,  -2, 1 },
	{ 0,  8,  38, 1 },
	{ 0, 16,  78, 1 },
	{ 0, 24, 118, 1 },
};


static void p5_sprite_bounce(struct platform_state *state) {

	// NOTE(peter): forced nonsense fix because ugg data is no longer const (C is garbage)...
	sprites[0].sprite = p5_ball_blue_data;
	sprites[1].sprite = p5_ball_grey_data;
	sprites[2].sprite = p5_ball_blue_data;
	sprites[3].sprite = p5_ball_grey_data;

	for(uint32_t i = 0; i < 4; ++i) {
		sprites[i].xpos += sprites[i].direction * 2;

		if(sprites[i].xpos > 322 || sprites[i].xpos < -2) {
			sprites[i].direction = -sprites[i].direction;
		}

		uint32_t offset_y = p5_sprite_sine[sprites[i].sine_index++];
		if(offset_y == 0xff) {
			sprites[i].sine_index = 0;
			offset_y = p5_sprite_sine[sprites[i].sine_index++];
		}

		blit_full(state, sprites[i].sprite, sprites[i].xpos, 104 +0x29 - offset_y, 0);

	}

}

static void p5_render_logo(struct platform_state *state, struct ugg *image, int32_t offset_x, uint32_t offset_y) {
	// Calculate center offset for 352-pixel logo area
	int32_t center_offset = ((int32_t)state->buffer_width - 352) / 2;
	if(center_offset < 0) {
		center_offset = 0;
	}

	struct rect clip_rect = { 0, 0, state->buffer_width - 2, state->buffer_height };

	struct clip_info clip = calculate_clip_rect(state, image, center_offset + offset_x, 103 + offset_y, clip_rect);

	if(!clip.visible) {
		return;
	}

	// Set up pointers and palette
	uint8_t * restrict src = clip.src;
	uint32_t * restrict dst = clip.dst;
	uint32_t * restrict palette = &p5_logo_background_colors[offset_y];

	uint32_t color_table[2];

	for(uint32_t y = 0; y < clip.height; ++y) {
		color_table[1] = palette[y];

		for(uint32_t x = 0; x < clip.width; ++x) {
			color_table[0] = dst[x];  // Transparent: keep destination color
			dst[x] = color_table[src[x]];
		}

		dst += clip.dst_stride;
		src += clip.src_stride;
	}
}

struct animation_state {
	uint32_t index;
	uint32_t state;
	int32_t pos;
	uint32_t direction;
	uint32_t speed;
	uint32_t duration;
	uint32_t logoid;
	uint32_t color;
};

static struct animation_state ani = { .duration = 1, .color = 0xffffffff };

static void p5_mid_screen_logos(struct platform_state *state) {
	if(--ani.duration == 0) {
		uint32_t exit;
		do {
			exit = 0;
			if(ani.state == 0) {
				ani.pos = animation_data[ani.index++] ? 352 : -352 - 16;

				ani.logoid = animation_data[ani.index++];
				ani.direction = animation_data[ani.index++];
				ani.speed = animation_data[ani.index++];
				ani.duration = animation_data[ani.index++];

				ani.color -= 0x1100;
				if(ani.color == 0xffff00ff) {
					ani.color = 0xffffffff;
				}
				p5_recalculate_background(ani.color);

				ani.state = 1;

			} else if(ani.state == 1) {
				uint32_t control = animation_data[ani.index++];
				if(control == 0xffff) {		// NOTE: Next sequence
					ani.state = 0;
					exit = 1;
					continue;

				} else if(control == 0xaaaa) {	// NOTE: Restart
					ani.index = 0;
					ani.state = 0;
					exit = 1;
					continue;
				}
				ani.direction = control;
				ani.speed = animation_data[ani.index++];
				ani.duration = animation_data[ani.index++];
			}
		} while(exit);
	}


	struct p5_logo {
		uint32_t y_offset;
		struct ugg *image;
	};

	struct p5_logo p5_logos[] = {
		{16, p5_atom_demo_ii_logo_data},
		{20, p5_jazze_logo_data},
		{16, p5_starfire_logo_data},
		{1,  p5_atom_logo_data},
		{10, p5_northstar_logo_data},
		{5,  p5_amiga_logo_data},
		{0,  p5_members_logo_data},
		{3,  p5_swap_logo_data},
	};

	ani.pos += (ani.direction) ? ani.speed : -ani.speed;
	p5_render_logo(state, p5_logos[ani.logoid].image, ani.pos, p5_logos[ani.logoid].y_offset);
}

static void p5_render_heads(struct platform_state *state) {
	int32_t half_head_w = p5_faces_data->width >> 1;
	blit_full(state, p5_faces_data, -half_head_w, 206, 0);
	blit_full(state, p5_faces_data, state->buffer_width-half_head_w, 206, 0);
}

#define LOGO_Y_OFFSET 224
static void p5_render_atom_logo(struct platform_state *state) {
	uint32_t temp_offset = md1_p5_atom_logo_offset;
	uint8_t * restrict logo_ptr = p5_atom_demo_ii_logo_data->data;
	uint32_t * restrict base_dest = BUFFER_PTR(state, ((state->buffer_width - p5_atom_demo_ii_logo_data->width) >> 1) - 8, LOGO_Y_OFFSET);
	const size_t sin_table_size = sizeof(atom_logo_sin_data);

	uint32_t tmp_palette[2] = { 0 };

	for(uint32_t y = 0; y < p5_atom_demo_ii_logo_data->height; ++y) {
		uint32_t val = atom_logo_sin_data[temp_offset];
		temp_offset = (temp_offset == sin_table_size - 1) ? 0 : temp_offset + 1;

		uint32_t *dest = base_dest + val;

		uint32_t color = (val | (val << 4));
		tmp_palette[1] = (color << 24) | (color << 16) | (color << 8) | 0xff;

		for(uint32_t x = 0; x < p5_atom_demo_ii_logo_data->width; ++x) {
			dest[x] = tmp_palette[*logo_ptr++];	//(value) ? color : 0;
		}
		base_dest += state->buffer_width;
	}

	md1_p5_atom_logo_offset = (md1_p5_atom_logo_offset == sin_table_size - 1) ? 0 : md1_p5_atom_logo_offset + 1;
}

static void p5_render_copperbars(struct platform_state *state) {

	uint32_t * restrict copper_bars_dest = BUFFER_PTR(state, 0, 103);
	uint32_t * restrict copper_bars_src = md1_p5_large_copper_bars;
	uint32_t num_copper_bars = ARRAYSIZE(md1_p5_large_copper_bars);

	for(uint32_t i = 0; i < num_copper_bars; ++i) {
		uint32_t color = copper_bars_src[i];
		for(uint32_t x = 0; x < state->buffer_width; ++x) {
			copper_bars_dest[x] = color;
		}
		copper_bars_dest += state->buffer_width;
	}
}

static void p5_render_small_scroller(struct platform_state *state) {
	const uint8_t * restrict src = p5_scroller1.buffer;
	uint32_t * restrict dst = BUFFER_PTR(state, (state->buffer_width - 336) / 2, 86);

	uint32_t color_lookup[4];
	color_lookup[2] = 0xffffffff;
	color_lookup[3] = 0x555555ff;

	uint32_t scroll_render_offset = p5_scroller1.char_render_offset - 360;

	for(uint32_t y = 0; y < p5_scroller1.char_height; ++y) {
		color_lookup[1] = md1_p5_small_scroller_inner_color[y];

		for(uint32_t x = 0; x < 336; ++x) {
			size_t src_index = (scroll_render_offset + x) & SCROLL_BUFFER_MASK;
			uint8_t color_index = src[src_index];
			color_lookup[0] = dst[x];
			dst[x] = color_lookup[color_index];
		}

		dst += state->buffer_width;
		src += SCROLL_BUFFER_WIDTH;
	}
}

static void p5_render_large_scroller(struct platform_state *state) {
	uint8_t * restrict src = p5_scroller2.buffer;
	uint32_t * restrict dst = BUFFER_PTR(state, (state->buffer_width - 336) / 2, 170);

	uint32_t color_lookup[4];

	for(uint32_t y = 0; y < p5_scroller2.char_height; ++y) {
		color_lookup[1] = md1_p5_large_scroller_outer_colors[y];
		color_lookup[2] = md1_p5_large_scroller_inner_color1[y];
		color_lookup[3] = md1_p5_large_scroller_inner_color2[y];

		for(uint32_t x = 0; x < 336; ++x) {
			size_t src_index = (p5_scroller2.char_render_offset - 360 + x) & SCROLL_BUFFER_MASK;
			color_lookup[0] = dst[x];
			dst[x] = color_lookup[src[src_index]];
		}

		dst += state->buffer_width;
		src += SCROLL_BUFFER_WIDTH;
	}
}

static uint32_t p5_warhammer_index = 0;

__attribute__((hot, always_inline))
static inline void p5_render_image_with_palette(struct platform_state *state, struct ugg *image, uint32_t *palette1, uint32_t *palette2, uint32_t start_row) {
	uint8_t *src = image->data;
	uint32_t *dst = BUFFER_PTR(state, (state->buffer_width - image->width) >> 1, start_row);
	uint32_t color_index = p5_warhammer_index;
	uint32_t fallback_color2 = 0x777777ff;

	uint32_t color_lookup[4];
	color_lookup[1] = 0xffffffff;

	for(uint32_t i = 0; i < image->height; ++i) {
		color_lookup[2] = palette2 ? palette2[color_index % 30] : fallback_color2;
		color_lookup[3] = palette1[color_index % 30];
		color_index++;

		#pragma GCC ivdep
		for(uint32_t j = 0; j < image->width; ++j) {
			uint32_t index = src[j];
			color_lookup[0] = dst[j];
			dst[j] = color_lookup[index];
		}

		dst += state->buffer_width;
		src += image->width;
	}
}

static void p5_render_warhammer_40k(struct platform_state *state) {
	p5_render_image_with_palette(state, p5_warhammer_logo_data, p5_warhammer_palette, 0, 14);
	p5_render_image_with_palette(state, p5_rogue_40k_logo_data, p5_40k_palette, p5_rogue_palette, 41);
	p5_warhammer_index = (p5_warhammer_index == 0) ? 29 : p5_warhammer_index - 1;
}

static uint32_t p5_update(struct platform_state *state)  {
	// PROFILE_NAMED("part5 all");

	if(mkfw_is_button_pressed(state->window, MOUSE_BUTTON_RIGHT)) {
		p5_scroller1.speed += 1;
		p5_scroller2.speed += 2;
		if(p5_scroller1.speed > 4) {
			p5_scroller1.speed = 1;
			p5_scroller2.speed = 2;
		}
	}
	scroller_update(state, &p5_scroller1);
	scroller_update(state, &p5_scroller2);
	p5_render_warhammer_40k(state);
	p5_render_copperbars(state);
	p5_render_heads(state);
	p5_render_small_scroller(state);
	p5_render_large_scroller(state);
	p5_sprite_bounce(state);
	p5_mid_screen_logos(state);
	p5_render_atom_logo(state);

	return mkfw_is_button_pressed(state->window, MOUSE_BUTTON_LEFT);

}

