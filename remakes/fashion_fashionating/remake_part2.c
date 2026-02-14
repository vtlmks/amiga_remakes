// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


INCBIN_UGG(p2_logo, "data/p2_logo.ugg");
INCBIN_UGG(p2_large_scroll_font, "data/p2_large_scroll_font.ugg");
INCBIN_UGG(p2_bouncing_balls, "data/p2_bouncing_balls.ugg");
INCBIN_UGG(p2_bouncing_ball_springs, "data/p2_bouncing_ball_springs.ugg");
INCBIN_UGG(p2_stalaktites, "data/p2_stalaktites.ugg");

static struct scroller_state p2_scroller;

#define springsCopperStart	85
#define scrollerCopperStart 223

__attribute__((aligned(64)))
static uint8_t scroll_text[] =
"               yooh!! let's go!  after 3 months work, 100 kb sourcecode and lots of debugging, we (fashion) are proud to present"
" you our newest demo called 'fashionating'!  as you can see, we didn't just sit on our asses and waited for something wonderfull"
" to happen, no. we wanted to create something to entertain  y o u  behind the screen, not some technical crap. just some cool"
" music and graphics that could set an example for some softwarecompanies. the last few months we saw some games with the worst"
" graphics you can imagine, so that is why we are planning to create a game with astonishing graphics, music and gameplay! in the"
" near future you will hear more about it.          to keep you reading this scroll i will let etonic and d.j. type in some text,"
" so hang on!                   hi ha who? the d.j. and etonic that's who!!! yep, now some real information about everything what"
" life offers. d.j.: well, it's real hard to type something when you don't know what to type. etonic: simple, if you have no"
" brains!! d.j.: hey smartguy, i have brains enough to type, so that will do!! etonic: yes, but i think you have a lot of empty"
" cells!! d.j.: only the idea that you are able to think makes me sick. etonic: why? am i so handsome? d.j.: i'm no longer"
" listening to that shit anymore!! etonic: all the guys who spread software for free are crazy! d.j.: yep, that means that you are"
" double-crazy! (this word is not copyright!) etonic: okay!, let's quit and tell something that interests our readers. d.j.: hey,"
" etonic! stop shaking your hand full (1200 baud). it's now almost a year that he has this obsession, but hey! everyone isn't"
" perfect. (except for me of course) readers!! this concerns you behind the demo. if you find more than 20 errors in this text you"
" will win a weekend in a 3 star hotel and will meet the whole fashion team. i hear you think: if these guys are typing such crap"
" then they look even worse! keep reading guys!    etonic: i wanna greet all the guys in holland and the rest in the world who swap"
" and or trade with us!   special hi's to: some old palls who trade since a year with me and the rest of fashion! ladies send a"
" photo to us and win a weekend emmen with etonic! all for free! big bedroom and big walls for that fucking noise!    okay, now"
" the d.j. is back on the keyboard. did you know that etonic's keyboard is alive, because there's hair growing between the keys!"
" you guessed it right! etonic has problems with keeping his hair on his head. now something completely different: the d.j.,"
" that's me in the place to be is looking for other fellows who are also making music on this super-synthesizer. also guys who"
" need music for a game or demo can contact me. okay, so far this text from the both of us, the d.j. and etonic.         well, this"
" is also the end of this scroll. i guess you will ask yourself : where the hell are the greetings?    just press the mousebutton"
" to enter the next part! (there you can read the greetings)  bye! see you in a few moments!!                                      "
"                                                 @";

struct ballSpring {
	uint8_t ball_anim_offset;
	uint8_t offset;
};

static struct ballSpring	ball_springs[7];
static uint8_t dup_line_index;
static int8_t ttcol_id1;
static uint8_t ttcol_id2;
static uint8_t ttsineid;

static int32_t p2_initialized = 0;

__attribute__((aligned(64)))
static uint32_t scroller_copper[] = {
	0x333333ff, 0x666666ff, 0x999999ff, 0xccccccff, 0xffffffff, 0xccccccff, 0x999999ff, 0x666666ff,
	0x333333ff, 0x000000ff, 0x000000ff, 0x110000ff, 0x220000ff, 0x330000ff, 0x440000ff, 0x550000ff,

	0x660000ff, 0x770000ff, 0x880000ff, 0x990000ff, 0xaa3333ff, 0xbb6666ff, 0xcc9999ff, 0xddccccff,
	0xeeeeeeff, 0xffffffff, 0xeeeeeeff, 0xccccddff, 0x9999ccff, 0x6666bbff, 0x3333aaff, 0x000099ff,

	0x000088ff, 0x000077ff, 0x000066ff, 0x000055ff, 0x000044ff, 0x000033ff, 0x000022ff, 0x000011ff,
	0x000000ff, 0x000000ff, 0x333333ff,	0x666666ff, 0x999999ff, 0xccccccff, 0xffffffff, 0xccccccff,
	0x999999ff, 0x666666ff, 0x333333ff,
};

__attribute__((aligned(64)))
static uint8_t scroll_char_conversion[] = {
	0x1a, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1b, 0x1c, 0x1a, 0x1e, 0x1f, 0x20,
	0x1a, 0x22, 0x23, 0x1a, 0x25, 0x26, 0x1a, 0x28, 0x1c, 0x1e, 0x1a, 0x1a, 0x34, 0x21, 0x24, 0x1a,
	0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x27, 0x1a, 0x1a, 0x29, 0x1a, 0x1d,
};

static uint8_t p2_process_char(struct scroller_state *scr_state, uint8_t c) {
	if(c == '@') {
		scr_state->text_offset = 0;
		c = scr_state->text[scr_state->text_offset++];
	}
	if(c > 63) {
		c -= 96;
	}
	return scroll_char_conversion[c] + 31;
}

__attribute__((aligned(64)))
static uint32_t tech_tech_colors[] = {
	0x000000ff, 0x000022ff, 0x000033ff, 0x111155ff, 0x222266ff, 0x333377ff, 0x444488ff, 0x5566aaff,
	0x7777bbff, 0x9999ccff, 0xbbbbeeff, 0xeeeeffff, 0xffeeeeff, 0xeeccccff, 0xddbbbbff, 0xccaa99ff,
	0xbbaa88ff, 0xbb9977ff, 0xaa9955ff, 0x999944ff, 0x888833ff, 0x667733ff, 0x556622ff, 0x445511ff,
	0x225511ff, 0x114400ff, 0x113300ff, 0x002200ff,
};

__attribute__((aligned(64)))
static uint8_t dup_line_tab[] = {
	1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 2,
	2, 2, 1, 2, 2, 2, 1, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1,
};

__attribute__((aligned(64)))
static uint8_t tech_tech_sine[] = {
	0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8,
	8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0,
};

__attribute__((aligned(64)))
static uint32_t springs_copper[] = {
	0x111111ff, 0x222222ff, 0x333333ff, 0x444444ff, 0x555555ff, 0x666666ff, 0x777777ff, 0x888888ff,
	0x999999ff, 0xaaaaaaff, 0xbbbbbbff, 0xccccccff, 0xddddddff, 0xeeeeeeff, 0xeeffeeff, 0xffffffff,
	0x333333ff, 0x333333ff, 0x333333ff, 0x333333ff, 0x333333ff, 0x333333ff, 0x333333ff, 0x333333ff,
};

__attribute__((aligned(64)))
static uint8_t spring_anim_offset[] = {
	0x0, 0x3, 0x6, 0x9, 0xc, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xc, 0x9, 0x6, 0x3, 0x0,
};

__attribute__((aligned(64)))
static uint8_t half_sine_tab[] = {
	0x3c, 0x39, 0x36, 0x33, 0x30, 0x2d, 0x2a, 0x27, 0x24, 0x21, 0x1e, 0x1c, 0x19, 0x17, 0x14, 0x12,
	0x10, 0x0e, 0x0c, 0x0a, 0x09, 0x07, 0x06, 0x04, 0x03, 0x03, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00,
	0x01, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04, 0x06, 0x07, 0x09, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14,
	0x17, 0x19, 0x1c, 0x1e, 0x21, 0x24, 0x27, 0x2a, 0x2d, 0x30, 0x33, 0x36, 0x39, 0x3c
};

// [=]===^=====================================================================================^===[=]
static void render_copper(struct platform_state *state, uint32_t start_line, uint32_t *copper_data, uint32_t rows) {
	if(start_line >= state->buffer_height) return;

	uint32_t max_rows = (start_line + rows > state->buffer_height) ? (state->buffer_height - start_line) : rows;
	uint32_t *row = BUFFER_PTR(state, 0, start_line);

	for(uint32_t copper_line = 0; copper_line < max_rows; ++copper_line) {
		for(uint32_t i = 0; i < state->buffer_width; ++i) {
			*row++ = copper_data[copper_line];
		}
	}
}

// [=]===^=[ render_tech_tech ]=====================================================^===[=]
static void render_tech_tech(struct platform_state *state) {
	uint8_t logo_line = 0;
	uint8_t tmp_dup_line_index = dup_line_index;
	uint8_t dup_lines = dup_line_tab[tmp_dup_line_index];

	int8_t temp_ttcol_id1 = ttcol_id1;
	uint8_t temp_ttcol_id2 = ttcol_id2;
	uint8_t temp_ttsine_id = ttsineid;

	uint32_t *restrict row = BUFFER_PTR(state, ((state->buffer_width - p2_logo->width) / 2) - 4, 163);

	for (uint8_t line_count = 0; line_count < 56; ++line_count) {
		if (dup_lines == 0) {
			++logo_line;
			tmp_dup_line_index = (tmp_dup_line_index + 1) % ARRAYSIZE(dup_line_tab);
			dup_lines = dup_line_tab[tmp_dup_line_index];
		}

		if (logo_line == p2_logo->height) break;

		uint32_t *restrict pixel = row + tech_tech_sine[temp_ttsine_id];
		const uint8_t *restrict logo_data = p2_logo->data + (logo_line * p2_logo->width);
		for (uint16_t xx = 0; xx < p2_logo->width; ++xx) {
			uint8_t color_id = logo_data[xx];
			if (color_id) {
				*pixel = (color_id == 1) ? tech_tech_colors[temp_ttcol_id2] : tech_tech_colors[temp_ttcol_id1];
			}
			++pixel;
		}

		if (dup_lines) {
			--dup_lines;
		}

		row += state->buffer_width;
		temp_ttcol_id1 = (temp_ttcol_id1 + 1) % ARRAYSIZE(tech_tech_colors);
		temp_ttcol_id2 = (temp_ttcol_id2 + 1) % ARRAYSIZE(tech_tech_colors);
		temp_ttsine_id = (temp_ttsine_id + 1) % ARRAYSIZE(tech_tech_sine);
	}
}

static void render_spring_and_ball(struct platform_state *state, struct ballSpring *spring, uint32_t index) {
#define first_ball_spring_x 68
#define distance_to_next 32
#define spring_width 18
#define spring_offset_y 75
#define ball_width 20
#define ball_height 20
#define ball_offset_y 16

	// Render ball
	uint32_t *row = BUFFER_PTR(state, first_ball_spring_x - 1 + distance_to_next * index, half_sine_tab[spring->offset] + ball_offset_y);

	uint8_t *colId = p2_bouncing_balls->data;
	colId += (spring->ball_anim_offset / 2) * ball_width;
	colId += ball_height * p2_bouncing_balls->width * index;

	for(uint32_t y = 0; y < ball_height; ++y) {
		uint32_t *dst = row;
		const uint8_t *src = colId;
		for(uint32_t x = 0; x < ball_width; ++x) {
			if(*src) {
				*dst = p2_bouncing_balls->palette[*src];
			}
			++dst;
			++src;
		}
		row += state->buffer_width;
		colId += p2_bouncing_balls->width;
	}

	// Render spring
	row = BUFFER_PTR(state, first_ball_spring_x + distance_to_next * index, spring_offset_y);

	colId = p2_bouncing_ball_springs->data;
	colId += spring_anim_offset[spring->offset] * spring_width;

	for(uint32_t y = 0; y < p2_bouncing_ball_springs->height; ++y) {
		uint32_t *dst = row;
		const uint8_t *src = colId;
		for(uint32_t x = 0; x < spring_width; ++x) {
			if(*src) {
				*dst = p2_bouncing_ball_springs->palette[*src];
			}
			++dst;
			++src;
		}
		row += state->buffer_width;
		colId += p2_bouncing_ball_springs->width;
	}
}

static void update_springs() {
	for(uint32_t i = 0; i < ARRAYSIZE(ball_springs); ++i) {
		ball_springs[i].ball_anim_offset += 1;
		if(ball_springs[i].ball_anim_offset >= 30) {
			ball_springs[i].ball_anim_offset = 0;
		}
	}
	for(uint32_t i = 0; i < ARRAYSIZE(ball_springs); ++i) {
		ball_springs[i].offset += 1;
		if(ball_springs[i].offset == ARRAYSIZE(spring_anim_offset)) {
			ball_springs[i].offset = 0;
		}
	}
}

static uint32_t part_2_render(struct platform_state *state) {
	if(!p2_initialized) {
		for(uint8_t i = 0; i < ARRAYSIZE(ball_springs); ++i) {
			ball_springs[i].ball_anim_offset = 2 * i;
			ball_springs[i].offset = 3 * i;
		}
		p2_scroller = (struct scroller_state) {
			.char_width = 32,
			.char_height = 32,
			.dest_offset_y = 233,
			.speed = 2,
			.text = scroll_text,
			.font = p2_large_scroll_font,
			.palette = p2_large_scroll_font->palette,
			.process_char = p2_process_char
		};
		scroller_new(&p2_scroller);
		p2_initialized = 1;
	}

	// Update logic every other frame
	if(state->frame_number & 0x1) {
		dup_line_index = (dup_line_index + 2) % ARRAYSIZE(dup_line_tab);
		ttcol_id1 = (ttcol_id1 - 1 + ARRAYSIZE(tech_tech_colors)) % ARRAYSIZE(tech_tech_colors);
		ttcol_id2 = (ttcol_id2 + 1) % ARRAYSIZE(tech_tech_colors);
	}
	ttsineid = (ttsineid + 1) % ARRAYSIZE(tech_tech_sine);

	// Render copper backgrounds
	render_copper(state, springsCopperStart, springs_copper, ARRAYSIZE(springs_copper));
	render_copper(state, scrollerCopperStart, scroller_copper, ARRAYSIZE(scroller_copper));

	update_springs();
	// Render springs
	for(uint32_t i = 0; i < ARRAYSIZE(ball_springs); ++i) {
		render_spring_and_ball(state, &ball_springs[i], i);
	}


	// Render static elements and tech-tech effect
	blit_full(state, p2_stalaktites, CENTER_X(state, p2_stalaktites->width), 103, 0);

	render_tech_tech(state);

	// Palette rotation for scroller color cycling
	uint32_t temp = p2_large_scroll_font->palette[31];
	for(uint32_t i = 0; i < 28; ++i) {
		p2_large_scroll_font->palette[(31 - i)] = p2_large_scroll_font->palette[(30 - i)];
	}
	p2_large_scroll_font->palette[4] = temp;
	scroller_update(state, &p2_scroller);

	return mkfw_is_button_pressed(state->window, MOUSE_BUTTON_LEFT);
}




