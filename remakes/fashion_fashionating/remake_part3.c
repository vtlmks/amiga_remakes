// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

INCBIN_UGG(p3_bob, "data/p3_bob.ugg");
INCBIN_UGG(p3_eyes, "data/p3_eyes.ugg");
INCBIN_UGG(p3_flirty_eye, "data/p3_flirty_eye.ugg");
INCBIN_UGG(p3_game_over_logo, "data/p3_game_over_logo.ugg");
INCBIN_UGG(p3_small_scroll_font, "data/p3_small_scroll_font.ugg");
// INCBIN_UGG(p3_stars, "data/p3_stars.ugg");


static struct point remake_stars[31];
int32_t p3Init = 0;

#define p3_scroller_width 346
#define p3_scroller_height 16
#define p3_scroller_char_width 16

// Copper Colors

uint32_t p3_copper_colors[] = {
	0x333333ff, 0x555555ff, 0x888888ff, 0xbbbbbbff, 0xddddddff, 0xeeeeeeff, 0xddddddff, 0xbbbbbbff, 0x888888ff, 0x555555ff, 0x333333ff,
};

uint8_t p3_scroll_text[] = {
	"                 welcome to another part of 'fashionating'. first of all we want you to look at that picture! great i"
	"sn't it? ofcourse it was drawn by the master of art : scum !! and the great music was composed and arranged by the re-in"
	"carnation of mozart : the d.j.!! and who is me? i am just the incredible shark, who is just busy beating his keyboard. bythewa"
	"y,ithinkmyspacebarisbroken. ha there it works again.    enough, lets move on to the most important thing of a demo : the greet"
	"ings!  first of all we do not have a ranking list, just because some people could get embarrased when they read their name at "
	"number 234!  so we have a greetingslist in random order. we send our fashionating greetings to : pentagon - dominators - rage "
	"- helloween - the web inc. - bamiga sector 1 - 7 up crew - starlight - e$g/ibb - the kent team - axenon - tedware - general/or"
	"acle - demian - dma - axxess - random access - flashpoint - omicrons - eric/fac - f4cg - tic/supersonics - the weird science -"
	" finnish gold - mc.crack/apt - amiga industries - amiga gang - cbc - tsk crew - the bos team - dbs - the bitboys - new generat"
	"ion crew - gcc - taurus 1 - new frontiers and all the others we forgot to mention here!!       we also want to mention all the"
	" others who wrote us a letter but never got an answer from us. we say sorry to : the pioneers/tbc - rs6502 - xades society - b"
	"ad boys - the new dimension - the future team - softrunner crew - u.s.s. - the honey crew - tff/dragon lights - mr. michael - "
	"tcf 2087 - dexion - errorsoft - catlon crew - excess - powerstation - term of notice - the rest. also sorry to the new aces, t"
	"he l.n.c.c. and tc/shadow software.     the reason why these people didn't got an answer from us was that we were very b"
	"usy with this demo. we also make demos for a videocompany, and we have our normal jobs too. anyway, thank you for writing us. "
	"                    the next people we are going to greet are no contacts of us, but we greet them for their work on the amiga"
	". we greet : hqc, unit a, blizzard, karsten obarski, defjam, knight hawks, antitrax, sodan, magician 42, etc.               if"
	" you want to write us then press the mouse button to enter the last part of this fashionating mega-demo. else, keep on reading"
	"!                      fashion is now officially no longer a member of tls!               fashion members today are : the d.j."
	" - scum - shark - etonic - joshua 5 - solo - the coach and skully.        watch out for solo's game : outrun!         th"
	"is demo was produced in co-operation with tdk, 3m, random access virus killer v1.2 and coca-cola. (i'm trying to bring s"
	"ome humor in this scroll)                 we have a question : why doesn't anyone use the build-in speech-synthesizer? ("
	"maybe because it is rotten?)                       well, i'm out of text, so this is the point where i have to say : goodb"
	"ye!! see you next time!!                                                    all routines, music, graphics and ideas are copyri"
	"ght by fashion (tm) in the year 1988                                                                                          "
	"                                                                                             @ "
};

static uint8_t *p3_scroll_text_ptr = p3_scroll_text;

static uint32_t p3_star_colors[] = {
	0x777777ff, 0xaaaaaaff, 0xffffffff,
};

static uint8_t p3_scroll_buffer[(p3_scroller_width + p3_scroller_char_width) * p3_scroller_height];
// static uint8_t p3_bob_buffer[((2 * p3_bob->width) + 376) * p3_bob->height];		// TODO(peter): Fix this, can't be having fixed width 376........
static uint32_t p3_scroll_count = 8;

static uint32_t p3_frame;
static int32_t p3_flirt_frames;
static uint32_t p3_eyes_frame = 1;  // Start at frame 1 (straight ahead)

static uint32_t p3_bling_star_phase_index;
static uint32_t p3_bling_pos_index;
static uint32_t p3_bling_star_phases[] = {
	0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 11, 11, 11, 11, 11,
	10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0,
};

static struct point p3_bling_sprite_locations[] = {
	{ 281,	103 },				// Bottom left on right N
	{ 253,	139 },				// Right shoulder
	{  21,	163 },				// Left 'thing'
	{ 272,	 70 },				// top left (right N)
	{  33,	113 },				// Left shoulder
	{  20,	  5 },				// Top right (left F)
	{ 218,	 59 },				// Top right, female helmet
	{ 281,	 29 },				// Bottom of right F
	{ 281,	179 },				// 'U' on signature
	{ 296,	 32 },				// Top of right S
	{ 217,	115 },				// Top of right shoulder
	{  87,	  8 },				// Top left, male helmet
	{  20,	 69 },				// top Right (left N)
	{ 174,	 26 },				// right side of male helmet
	{  20,	 32 },				// Top, right (Left S)
	{ 249,	169 },				// S on signature
};

static void initialize_stars(void) {
	for(uint16_t i = 0; i < ARRAYSIZE(remake_stars); ++i) {
		remake_stars[i].x = BUFFER_WIDTH + xor_generate_random(&rand_state) % (BUFFER_WIDTH + 30);

		remake_stars[i].y = i;
	}
	p3Init = 1;
}

static void render_copper_bars(void) {
	uint32_t *row = buffer;
	uint32_t x, y;

	// Render top copper bar - just one line at Y=0
	for(x = 0; x < BUFFER_WIDTH; ++x) {
		*row++ = p3_copper_colors[10];  // Last color in the array
	}

	// Render middle copper bar
	row = buffer + 201 * BUFFER_WIDTH;
	for(y = 0; y < 11; ++y) {
		for(x = 0; x < BUFFER_WIDTH; ++x) {
			*row++ = p3_copper_colors[y];
		}
	}

	// Render bottom copper bar
	row = buffer + 230 * BUFFER_WIDTH;
	for(y = 0; y < 11; ++y) {
		for(x = 0; x < BUFFER_WIDTH; ++x) {
			*row++ = p3_copper_colors[y];
		}
	}
}

static uint32_t render_game_over_logo(uint32_t frame) {
	static uint32_t first_line_of_logo_rendered = 0;
	static uint32_t flood_fill = 1;
	static uint32_t initialized = 0;

	if(!initialized) {
		first_line_of_logo_rendered = p3_game_over_logo->height - 1;
		initialized = 1;
	}

	uint32_t *game_over_logo_start = buffer + 1 * BUFFER_WIDTH +
		((BUFFER_WIDTH - p3_game_over_logo->width) / 2);

	if(flood_fill && frame % 2 == 0) {
		if(--first_line_of_logo_rendered == 0xffffffff) {
			flood_fill = 0;
		}
	}

	if(flood_fill) {
		uint8_t *src;
		uint32_t *row = game_over_logo_start;
		uint32_t floodY = 0;

		while(floodY < first_line_of_logo_rendered) {
			uint32_t *pixel = row;
			src = p3_game_over_logo->data + first_line_of_logo_rendered * p3_game_over_logo->width;
			for(uint32_t x = 0; x < p3_game_over_logo->width; ++x) {
				*pixel++ = p3_game_over_logo->palette[*src++];
			}
			row += BUFFER_WIDTH;
			++floodY;
		}

		row = game_over_logo_start + first_line_of_logo_rendered * BUFFER_WIDTH;
		src = p3_game_over_logo->data + first_line_of_logo_rendered * p3_game_over_logo->width;
		for(uint32_t y = 0; y < (p3_game_over_logo->height - first_line_of_logo_rendered); ++y) {
			uint32_t *pixel = row;
			for(uint32_t x = 0; x < p3_game_over_logo->width; ++x) {
				*pixel++ = p3_game_over_logo->palette[*src++];
			}
			row += BUFFER_WIDTH;
		}
	} else {
		uint32_t *row = game_over_logo_start;
		uint8_t *src = p3_game_over_logo->data;

		for(uint32_t y = 0; y < p3_game_over_logo->height; ++y) {
			uint32_t *pixel = row;
			for(uint32_t x = 0; x < p3_game_over_logo->width; ++x) {
				*pixel++ = p3_game_over_logo->palette[*src++];
			}
			row += BUFFER_WIDTH;
		}
	}

	return !flood_fill;  // Return 1 when logo is fully rendered
}

static void update_and_render_scroller(void) {
	if(--p3_scroll_count == 0) {
		p3_scroll_count = 8;
		if(*p3_scroll_text_ptr == '@') {
			p3_scroll_text_ptr = p3_scroll_text;
		}

		uint8_t character = *p3_scroll_text_ptr++;
		if(character >= 'a') {
			character -= 'a';
		} else {
			character -= 1;
		}

		uint8_t *dest = p3_scroll_buffer + p3_scroller_width;
		uint8_t *src = p3_small_scroll_font->data + character * 16 * 16;
		for(uint32_t y = 0; y < 16; ++y) {
			for(uint32_t x = 0; x < 16; ++x) {
				*dest++ = *src++;
			}
			dest += p3_scroller_width;
		}
	}

	memmove(p3_scroll_buffer, p3_scroll_buffer + 2, sizeof(p3_scroll_buffer) - 2);

	uint8_t *src = p3_scroll_buffer;
	uint32_t *row = buffer + 213 * BUFFER_WIDTH +
		((BUFFER_WIDTH - p3_scroller_width) / 2);

	for(uint32_t y = 0; y < p3_scroller_height; ++y) {
		uint32_t *pixel = row;
		for(uint32_t x = 0; x < p3_scroller_width; ++x) {
			if(*src) {
				*pixel = p3_small_scroll_font->palette[*src];
			}
			++pixel;
			++src;
		}
		row += BUFFER_WIDTH;
		src += p3_scroller_char_width;
	}
}

static void render_stars(void) {
	for(uint32_t i = 0; i < ARRAYSIZE(remake_stars); ++i) {
		remake_stars[i].x -= (i % 3) + 1;
		if(remake_stars[i].x < 0) {
			remake_stars[i].x += BUFFER_WIDTH;
		}
	}

	uint32_t *stars_start = buffer + 242 * BUFFER_WIDTH;
	uint32_t *row = stars_start;
	uint32_t current_y = 242;

	for(uint32_t i = 0; i < ARRAYSIZE(remake_stars); ++i) {
		if(current_y < BUFFER_HEIGHT && (remake_stars[i].x > 0) && (remake_stars[i].x < (int32_t)BUFFER_WIDTH)) {
			uint32_t *pixel = row + remake_stars[i].x;
			*pixel = p3_star_colors[i % 3];
		}
		row += BUFFER_WIDTH;
		++current_y;
	}
}

static void render_flirty_eyes(void) {
	// Flirty eyes animation - woman's eyes blink every 167 frames
	static uint32_t local_frame_counter = 0;

	if((local_frame_counter % 167) == 0) {
		p3_flirt_frames = 7;
	}

	if(p3_flirt_frames) {
		static uint32_t flirty_eyes_offsets[] = {
			0, 16, 32, 48, 32, 16, 0,
		};
		uint8_t *src = p3_flirty_eye->data + flirty_eyes_offsets[7 - p3_flirt_frames];
		uint32_t *dest = buffer + 90 * BUFFER_WIDTH + 187;
		for(uint32_t y = 0; y < 11; ++y) {
			for(uint32_t x = 0; x < 16; ++x) {
				if(*src) {
					*dest = p3_flirty_eye->palette[*src];
				}
				src++;
				dest++;
			}
			src += 48;
			dest += (BUFFER_WIDTH - 16);
		}
		if(local_frame_counter % 2 == 0) {
			--p3_flirt_frames;
		}
	}
	++local_frame_counter;
}

static void render_moving_eyes(void) {
	// Moving eyes animation - man's eyes change position every 80 frames
	static uint32_t local_frame_counter = 0;

	if((local_frame_counter % 80) == 0) {
		++p3_eyes_frame;
	}
	++local_frame_counter;

	uint8_t *src = p3_eyes->data + ((p3_eyes_frame % 3) * 8) * p3_eyes->width;
	uint32_t *dest = buffer + 36 * BUFFER_WIDTH + 110;
	for(uint32_t y = 0; y < 8; ++y) {
		for(uint32_t x = 0; x < p3_eyes->width; ++x) {
			*dest++ = p3_eyes->palette[*src++];
		}
		dest += (BUFFER_WIDTH - p3_eyes->width);
	}
}

static void render_bling_stars(void) {
	// Bling star animation - sparkles on the logo every 81 frames
	static uint32_t local_frame_counter = 0;

	if((local_frame_counter % 81) == 0) {
		p3_bling_star_phase_index = ARRAYSIZE(p3_bling_star_phases);
		if(++p3_bling_pos_index == ARRAYSIZE(p3_bling_sprite_locations)) {
			p3_bling_pos_index = 0;
		}
	}

	if(p3_bling_star_phase_index) {
		int32_t star_x = p3_bling_sprite_locations[p3_bling_pos_index].x + 20;
		int32_t star_y = p3_bling_sprite_locations[p3_bling_pos_index].y;

		// Bounds check - skip if completely off-screen
		if(star_x >= 0 && star_y >= 0 &&
		   star_x < (int32_t)BUFFER_WIDTH && star_y < (int32_t)BUFFER_HEIGHT) {
			uint8_t *src = p3_stars->data + p3_bling_star_phases[p3_bling_star_phase_index] * 15;
			uint32_t *dest = buffer + (BUFFER_WIDTH * star_y + star_x);

			for(uint32_t y = 0; y < p3_stars->height && (star_y + y) < BUFFER_HEIGHT; ++y) {
				uint32_t *row = dest;
				for(uint32_t x = 0; x < p3_stars->height && (star_x + x) < BUFFER_WIDTH; ++x) {
					if(*src) {
						*row = p3_stars->palette[*src];
					}
					++src;
					++row;
				}
				dest += BUFFER_WIDTH;
				src += p3_stars->width - p3_stars->height;
			}
		}
		--p3_bling_star_phase_index;
	}
	++local_frame_counter;
}

static void render_bob_scroller(void) {
	// Bob scrolling text - horizontal scrolling text at bottom
	static uint8_t p3_bob_buffer[((2 * 16) + 376) * 16];  // TODO: Fix hardcoded dimensions
	uint8_t *src;
	uint8_t *dst;
	uint32_t x, y;

	if((p3_frame % 512) == 0) {
		src = p3_bob->data;
		dst = p3_bob_buffer;
		for(y = 0; y < p3_bob->height; ++y) {
			for(x = 0; x < p3_bob->width; ++x) {
				if(*src) {
					*dst = *src;
				}
				++src;
				++dst;
			}
			dst += BUFFER_WIDTH + p3_bob->width;
		}
	}

	dst = p3_bob_buffer + 2 * p3_bob->width + BUFFER_WIDTH - 1;
	for(y = 0; y < p3_bob->height; ++y) {
		*dst = 0;
		dst += 2 * p3_bob->width + BUFFER_WIDTH;
	}

	memmove(p3_bob_buffer + 1, p3_bob_buffer, sizeof(p3_bob_buffer) - 1);

	src = p3_bob_buffer + p3_bob->width;
	uint32_t *dest = buffer + 247 * BUFFER_WIDTH;
	for(y = 0; y < p3_bob->height; ++y) {
		for(x = 0; x < BUFFER_WIDTH; ++x) {
			if(*src) {
				*dest = p3_bob->palette[*src];
			}
			++src;
			++dest;
		}
		src += p3_bob->width * 2;
	}
}

static uint32_t part_3_render(void) {
	if(!p3Init) {
		initialize_stars();
	}

	render_copper_bars();

	uint32_t logo_complete = render_game_over_logo(p3_frame);
	// Only render these elements after the logo flood-fill is complete
	if(logo_complete) {
		render_flirty_eyes();
		render_moving_eyes();
		render_bling_stars();
		update_and_render_scroller();
		render_stars();
		render_bob_scroller();
	}

	// Increment the frame counter
	++p3_frame;
	return mkfw_is_button_pressed(window, MOUSE_BUTTON_LEFT);
}
