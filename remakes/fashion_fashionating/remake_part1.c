// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

INCBIN_UGG(p1_c64_screen, "data/p1_c64_screen.ugg");
INCBIN_UGG(p1_c64_loading_run, "data/p1_c64_loading_run.ugg");
INCBIN_UGG(p1_presents_fashionating, "data/p1_presents_fashionating.ugg");
INCBIN_UGG(p1_rotating_logo, "data/p1_rotating_logo.ugg");
INCBIN_UGG(p1_scroll_font, "data/p1_scroll_font.ugg");
INCBIN_UGG(p3_stars, "data/p3_stars.ugg");  // Part 1 uses part 3's star sprite

// Amiga-style 4-bit per channel color blending (12-bit color)
static uint8_t blend_4bit(uint8_t from, uint8_t to, uint32_t step, uint32_t total_steps) {
	int32_t from_nibble = from >> 4;
	int32_t to_nibble = to >> 4;
	int32_t blended_nibble = from_nibble + ((to_nibble - from_nibble) * (int32_t)step) / (int32_t)total_steps;
	return (blended_nibble << 4) | blended_nibble;
}

static uint32_t blend_color_4bit(uint32_t from, uint32_t to, uint32_t step, uint32_t total_steps) {
	uint8_t r = blend_4bit((from >> 24) & 0xff, (to >> 24) & 0xff, step, total_steps);
	uint8_t g = blend_4bit((from >> 16) & 0xff, (to >> 16) & 0xff, step, total_steps);
	uint8_t b = blend_4bit((from >>  8) & 0xff, (to >>  8) & 0xff, step, total_steps);
	uint8_t a = blend_4bit((from >>  0) & 0xff, (to >>  0) & 0xff, step, total_steps);

	return (r << 24) | (g << 16) | (b << 8) | a;
}

#define p1_rotating_logo_steps 30


#define p1_c64CursorWidth 7
#define p1_c64CursorHeight 8


#define p1_c64ScreenStartX	57
#define p1_c64ScreenWidth	280
#define p1_c64ScreenStartY	39
#define p1_c64ScreenHeight	200

// NOTE(peter): The following 5 all start at X = 0
#define p1_c64TopTextPosY			47
#define p1_c64LoadFashionatingPosY	87
#define p1_c64SearchingLoadingPosY	103
#define p1_c64ReadyPosY				119
#define p1_c64RunPosY				127

#define P1_ROTATING_LOGO_Y_START	46
#define P1_PRESENTS_Y_START		157
#define P1_FASHIONATING_Y_START	197
#define P1_SCROLLER_Y_START		239


static uint32_t p1_rotating_logo_fade_colors[64] = {
};

static uint32_t c64_colors[] = {
	0x00000000, 0xffffffff, 0xcc0000ff, 0x00ffddff, 0xbb00ffff, 0x00cc00ff, 0x0000ccff, 0xffff00ff,
	0xcc7700ff, 0x773300ff, 0xff6688ff, 0x666666ff, 0x999999ff, 0x99ff77ff, 0x8899ffff, 0xccccccff,
};

static uint32_t p1_rotating_logo_final_colors[64] = {
	0xffddddff, 0xffddddff, 0xffbbbbff, 0xffbbbbff, 0xee9999ff, 0xee9999ff, 0xee8888ff, 0xee8888ff,
	0xdd7777ff, 0xdd7777ff, 0xdd6666ff, 0xdd6666ff, 0xdd5555ff, 0xdd5555ff, 0xcc4444ff, 0xcc4444ff,
	0xcc3333ff, 0xcc3333ff, 0xcc3322ff, 0xcc3322ff, 0xbb2222ff, 0xbb2222ff, 0xaa1111ff, 0xaa1111ff,
	0x880000ff, 0x880000ff, 0x660000ff, 0x660000ff, 0x440000ff, 0x440000ff, 0x220000ff, 0x220000ff,
	0x111144ff, 0x111144ff, 0x111199ff, 0x111199ff, 0x2222ccff, 0x2222ccff, 0x2222ffff, 0x2222ffff,
	0x3333ffff, 0x3333ffff, 0x4444ffff, 0x4444ffff, 0x4455ffff, 0x4455ffff, 0x5555ffff, 0x5555ffff,
	0x6666ffff, 0x6666ffff, 0x7777ffff, 0x7777ffff, 0x8888ffff, 0x8888ffff, 0x9999ffff, 0x9999ffff,
	0xaaaaffff, 0xaaaaffff, 0xbbbbffff, 0xbbbbffff, 0xccbbffff, 0xccbbffff, 0xccccffff, 0xccccffff,
};

static uint8_t scroll_speed[96] = {		// these scroll-values totals 352 pixels of scrolling
	1, 1, 1, 1, 2, 2, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static uint32_t scroll_colors[] = {
	0x550000ff, 0x770000ff, 0x990000ff, 0xaa3333ff, 0xbb6666ff, 0xcc9999ff, 0xddccccff, 0xeeeeeeff, 0xffffffff,
	0xffffffff, 0xeeeeeeff, 0xccccddff, 0x9999ccff, 0x6666bbff, 0x3333aaff, 0x000099ff, 0x000077ff, 0x000055ff,
};

static uint8_t p1_scroll_text[] = {			// each line is 352 pixels wide, or 22 characters; not strictly neccesary, but it was done this way in the original code.
	"   fashion presents   "
	"  their newest demo!  "
	"                      "
	"  designed & created  "
	"          by          "
	"  shark (programming) "
	"    scum (artworx)    "
	"  the dj. (all music) "
	"                      "
	"   this demo is just  "
	"   knock em dead!!!   "
	"   so fashion says:   "
	"    spread this!!!    "
	"                      "
	"  all routines, music "
	"   and graphics are   "
	"      homemade!!!     "
	"                      "
	"     release date:    "
	"       may 1988       "
	"                      "
	"  press mousebutton   "
	"  to start the show!  "
	"                      "
	"                      "
	"                      "
	"                      "
	"@"
};

static uint32_t c64_colors_load_run[] = {
	0x8899ffff, 0x0000ccff,
};

static uint32_t p1_presents_colors[] = {
	0xffddddff, 0xffbbbbff, 0xee9999ff, 0xee8888ff, 0xdd7777ff, 0xdd6666ff, 0xdd5555ff,
	0xcc4444ff, 0xcc3333ff, 0xcc3232ff, 0xbb2222ff, 0xaa1111ff, 0x990000ff, 0x880000ff,
};

static uint32_t p1_fashionating_colors[] = {
	0x2222bbff, 0x2222ccff, 0x2222ffff, 0x3333ffff, 0x4444ffff, 0x4545ffff, 0x5555ffff,
	0x6666ffff, 0x7777ffff, 0x8888ffff, 0x9999ffff, 0xaaaaffff, 0xbbbbffff, 0xcbcbffff,
};

#define p1_scroll_buffer_width BUFFER_WIDTH
static uint8_t p1_scroll_buffer[p1_scroll_buffer_width * 2 * 18];
static uint8_t *p1_temp_buffer; // Allocated in p1_init() based on p1_presents_fashionating->width

static int32_t p1_stars[270];
static int32_t p1_initialized = 0;

static uint32_t p1_star_colors[] = { 0x444444ff, 0x888888ff, 0xbbbbbbff, 0xffffffff };

static uint32_t p1_presents_counter = 0;
static uint32_t pixel_counter = 0;
static uint32_t color_index = 0;
static uint32_t p1_frame = 0;

static uint32_t p1_bling_star_phase_index = 0;
static uint32_t p1_bling_pos_index = 0;

static uint32_t p1_bling_star_phases[] = {
	0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
	11, 11, 11, 11, 11, 11, 11,
	10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0,
};

// Removed unused 'mo' macro
static struct point p1_bling_sprite_locations[] = {
	{ 134,  88 },
	{ 109,  64 },
	{ 186,  64 },
	{ 194,  40 },
	{  70,  58 },
	{ 238,  99 },
	{ 259,  40 },
	{ 238,  88 },
	{ 167,  40 },
	{  70,  98 },
};

struct animationstep {
	uint32_t startFrame;
	uint32_t endFrame;	// Inclusive
	void (*renderFunction)(uint32_t);
};

static void part_1_init(void) {
	// Allocate temp buffer based on actual UGG dimensions
	p1_temp_buffer = (uint8_t*)malloc((352 + p1_presents_fashionating->width) * 15);
}

static void render_blinking_cursor(uint32_t frame);
static void render_repositioned_blinking_cursor(uint32_t frame);
static void render_type_load_command(uint32_t frame);
static void render_search_and_load_text(uint32_t frame);
static void render_type_run_command(uint32_t frame);
static void finalize_animation_sequence(uint32_t frame);

// Array of animation steps (sentinel value at the end)
static const struct  animationstep animationSteps[] = {

	{   0, 261, render_blinking_cursor },							// Displays a blinking cursor
	{ 262, 421, render_type_load_command },						// Types out the command `load "fashionating",8,1`
	{ 422, 599, render_search_and_load_text },					// Displays `Searching for fashionating\nloading`
	{ 600, 859, render_repositioned_blinking_cursor },			// Repositions and displays a blinking cursor
	{ 860, 905, render_type_run_command },							// Types out the word `run`
	{ 600, 905, finalize_animation_sequence },					// Finalizes the sequence, overlaps with previous steps
	{  -1,  -1, 0 }														// Sentinel value to mark the end of the array
};

static void c64_effect(void) {
	// Fill background with the first color in the palette
	uint32_t *dst = buffer;
	uint32_t color = c64_colors_load_run[0];
	for(uint32_t i = 0; i < BUFFER_WIDTH * BUFFER_HEIGHT; ++i) {
		*dst++ = color;
	}

	// Calculate offset for centering C64 screen data
	uint32_t skip = (BUFFER_WIDTH - p1_c64_screen->width);
	dst = buffer + 28 * BUFFER_WIDTH + skip / 2;

	// Copy C64 screen data with color lookups
	uint8_t *src = p1_c64_screen->data;
	for(uint32_t y = 0; y < p1_c64_screen->height; ++y) {
		for(uint32_t x = 0; x < p1_c64_screen->width; ++x) {
			*dst++ = c64_colors_load_run[*src++];  // Lookup color from LUT
		}
		dst += skip;
	}

	// Iterate through animation steps
	int i = 0;
	do {
		if(p1_frame >= animationSteps[i].startFrame && p1_frame <= animationSteps[i].endFrame) {
			animationSteps[i].renderFunction(p1_frame);
		}
		i++;
	} while (animationSteps[i].startFrame != -1);  // Check sentinel value
}

static void render_blinking_square(uint32_t onoff, uint32_t *dst) {
	uint32_t color = c64_colors_load_run[onoff];
	for(uint32_t y = 0; y < 8; ++y) {
		uint32_t *row = dst;
		for(uint32_t x = 0; x < 8; ++x) {
			*row++ = color;
		}
		dst += BUFFER_WIDTH;
	}
}

static void render_blinking_cursor(uint32_t frame) {
	uint32_t skip = (BUFFER_WIDTH - p1_c64_screen->width);
	uint32_t *dst = (buffer + (28 + 48) * BUFFER_WIDTH + 20 + (skip / 2));
	render_blinking_square((frame >> 4) & 1, dst);
}

static void render_repositioned_blinking_cursor(uint32_t frame) {
	uint32_t skip = (BUFFER_WIDTH - p1_c64_screen->width);
	uint32_t *dst = (buffer + (92 + 24) * BUFFER_WIDTH + 20 + (skip / 2));
	render_blinking_square((frame >> 4) & 1, dst);
}

static void render_type_load_command(uint32_t frame) {
	uint32_t skip = (BUFFER_WIDTH - p1_c64_screen->width);
	uint32_t *dst = (buffer + (28 + 48) * BUFFER_WIDTH + 20 + (skip / 2));
	uint8_t *src = p1_c64_loading_run->data;

	for(uint32_t y = 0; y < 8; ++y) {
		uint32_t *row = dst;
		uint8_t *source = src;
		for(uint32_t x = 0; x < 7 * ((p1_frame - 256) >> 3); ++x) {
			*row++ = c64_colors_load_run[*source++];
		}
		dst += BUFFER_WIDTH;
		src += p1_c64_loading_run->width;
	}
}

static void render_search_and_load_text(uint32_t frame) {
	uint32_t skip = (BUFFER_WIDTH - p1_c64_screen->width);
	uint32_t *dst = (buffer + 76 * BUFFER_WIDTH + 20 + (skip / 2));
	uint8_t *src = p1_c64_loading_run->data;

	for(uint32_t y = 0; y < 8; ++y) {
		uint32_t *row = dst;
		uint8_t *source = src;
		for(uint32_t x = 0; x < p1_c64_loading_run->width; ++x) {
			*row++ = c64_colors_load_run[*source++];
		}
		dst += BUFFER_WIDTH;
		src += p1_c64_loading_run->width;
	}

	dst = (buffer + 92 * BUFFER_WIDTH + 20 + (skip / 2));
	for(uint32_t y = 0; y < 16; ++y) {
		uint32_t *row = dst;
		uint8_t *source = src;
		for(uint32_t x = 0; x < p1_c64_loading_run->width; ++x) {
			*row++ = c64_colors_load_run[*source++];
		}
		dst += BUFFER_WIDTH;
		src += p1_c64_loading_run->width;
	}
}

static void render_type_run_command(uint32_t frame) {
	uint32_t skip = (BUFFER_WIDTH - p1_c64_screen->width);
	uint32_t *dst = (buffer + (92 + 24) * BUFFER_WIDTH + 20 + (skip / 2));
	uint8_t *src = p1_c64_loading_run->data + 32 * p1_c64_loading_run->width;

	for(uint32_t y = 0; y < 8; ++y) {
		uint32_t *row = dst;
		uint8_t *source = src;
		for(uint32_t x = 0; x < 7 * ((frame - 860) >> 3); ++x) {
			*row++ = c64_colors_load_run[*source++];
		}
		dst += BUFFER_WIDTH;
		src += p1_c64_loading_run->width;
	}
}

static void finalize_animation_sequence(uint32_t frame) {
	uint32_t skip = (BUFFER_WIDTH - p1_c64_screen->width);
	uint32_t *dst = (buffer + 76 * BUFFER_WIDTH + 20 + (skip / 2));
	uint8_t *src = p1_c64_loading_run->data;

	for(uint32_t y = 0; y < 8; ++y) {
		uint32_t *row = dst;
		uint8_t *source = src;
		for(uint32_t x = 0; x < p1_c64_loading_run->width; ++x) {
			*row++ = c64_colors_load_run[*source++];
		}
		dst += BUFFER_WIDTH;
		src += p1_c64_loading_run->width;
	}

	dst = (buffer + 92 * BUFFER_WIDTH + 20 + (skip / 2));
	for(uint32_t y = 0; y < 24; ++y) {
		uint32_t *row = dst;
		uint8_t *source = src;
		for(uint32_t x = 0; x < p1_c64_loading_run->width; ++x) {
			*row++ = c64_colors_load_run[*source++];
		}
		dst += BUFFER_WIDTH;
		src += p1_c64_loading_run->width;
	}
}

// [=]===^=====================================================================================^===[=]
static void decrunchEffect(void) {
	uint32_t *dest = buffer;
	uint32_t temp_color_index = color_index++;
	uint32_t temp_color = c64_colors[temp_color_index & 0xf];
	uint32_t total_pixels = BUFFER_WIDTH * BUFFER_HEIGHT;
	uint32_t set_size = 9 * BUFFER_WIDTH + ((temp_color_index % 3) * BUFFER_WIDTH);
	uint32_t i = 0;

	for(i = 0; i < (set_size - (pixel_counter % set_size)); ++i) {
		*dest++ = temp_color;
	}

	pixel_counter += set_size;
	total_pixels -= set_size - (pixel_counter % set_size);

	for(;;) {
		temp_color = c64_colors[++temp_color_index % 0xf];
		set_size = 5 * BUFFER_WIDTH + ((temp_color_index % 10) * BUFFER_WIDTH);

		if(total_pixels < set_size) {
			for(i = 0; i < total_pixels; ++i) {
				*dest++ = temp_color;
			}
			pixel_counter += total_pixels;
			break;
		} else {
			for(i = 0; i < set_size; ++i) {
				*dest++ = temp_color;
			}
			pixel_counter += set_size;
			total_pixels -= set_size;
		}
	}
}

// [=]===^=====================================================================================^===[=]
static void p1_scroller(void) {
	static uint8_t *scroll_text_ptr = p1_scroll_text;
	static uint32_t count = 96;

	if(count == 96) {
		count = 0;

		if(*scroll_text_ptr == '@') {
			scroll_text_ptr = p1_scroll_text;
		}

		/* clear right side of buffer */
		uint8_t *dest = p1_scroll_buffer + p1_scroll_buffer_width;
		for(uint32_t y = 0; y < 18; ++y) {
			memset(dest, 0, p1_scroll_buffer_width);
			dest += p1_scroll_buffer_width*2;
		}

		/* write 22 characters to buffer */
		for(uint32_t i = 0; i < 22; ++i) {
			uint8_t character = *scroll_text_ptr++;
			if(character >= 'a') {
				character -= 'a';
			} else {
				character -= 1;
			}
			uint8_t *font = p1_scroll_font->data + character * 24 * 16;
			uint8_t *dest = p1_scroll_buffer + p1_scroll_buffer_width + 16 * i;
			for(uint32_t y = 0; y < 18; ++y) {
				for(uint32_t x = 0; x < 16; ++x) {
					*dest++ = *font++;
				}
				dest += p1_scroll_buffer_width * 2 - 16;
			}
		}
	}

	uint8_t *scr_src = p1_scroll_buffer + scroll_speed[count];
	uint8_t *scr_dst = p1_scroll_buffer;

	for(int32_t x = scroll_speed[count]; x < p1_scroll_buffer_width * 2 * 18; ++x) {
		*scr_dst++ = *scr_src++;
	}

	uint32_t *data = buffer + P1_SCROLLER_Y_START * BUFFER_WIDTH;

	uint8_t *source = p1_scroll_buffer + CENTER_X(p1_scroll_buffer_width);
	uint32_t *dest = data;

	uint32_t max_lines = 18;
	if(P1_SCROLLER_Y_START + max_lines > BUFFER_HEIGHT) {
		max_lines = BUFFER_HEIGHT - P1_SCROLLER_Y_START;
	}

	for(uint32_t y = 0; y < max_lines; ++y) {
		uint32_t *row = dest;
		uint32_t color = scroll_colors[y];

		for(uint32_t x = 0; x < BUFFER_WIDTH; ++x) {
			if(source[x]) {
				row[x] = color;
			}
		}
		source += p1_scroll_buffer_width * 2;
		dest += BUFFER_WIDTH;
	}

	count++;
}

static uint32_t part_1_render(void) {
	static uint32_t decrunchTime = 256;
	static float accumulator = 0.f;
	static uint32_t cursorCounter = 0;
	static int32_t cursorVisible = 0;

	static uint32_t rotatingLogoStep = 30;		// test
	static uint32_t writtenLength = 0;

	// accumulator += dt;

	if(p1_frame < 906) {
		c64_effect();
	}

	if(p1_frame >= 906 && p1_frame < 1092) {
		decrunchEffect();
	}

	if(p1_frame == 1100) {
		p1_audio_state = 1;
	}

	if(p1_frame == 1950) {
		p1_audio_state = 2;
	}

	/*
	 * Start rotating the logo
	*/
	if(p1_frame < 1453 - 12 + 30) {
		++rotatingLogoStep;
	}

	if (p1_frame >= 1188 && p1_frame < 1444) { // Fade from black to white in 256 frames
		uint8_t grayValue = (uint8_t)(p1_frame - 1188);
		uint32_t color = (grayValue << 24) | (grayValue << 16) | (grayValue << 8) | grayValue;

		for (uint32_t i = 0; i < ARRAYSIZE(p1_rotating_logo_fade_colors); ++i) {
			p1_rotating_logo_fade_colors[i] = color; // All pixels get the same grayscale
		}
	}

	if (p1_frame >= 1444 && p1_frame < 1494) { // Fade from white to final colors
		uint32_t step = p1_frame - 1444; // 0 to 49

		for(uint32_t i = 0; i < ARRAYSIZE(p1_rotating_logo_fade_colors); ++i) {
			p1_rotating_logo_fade_colors[i] = blend_color_4bit(0xffffffff, p1_rotating_logo_final_colors[i], step, 50);
		}
	}

	if(p1_frame >= 1154) {
		/*
		 *  Stars
		 */
		if(!p1_initialized) {
			uint32_t offset = BUFFER_WIDTH + 30;
			for(uint32_t i = 0; i < BUFFER_HEIGHT; ++i) {
				p1_stars[i] = offset + xor_generate_random(&rand_state) % (BUFFER_WIDTH + 30);
			}
			p1_initialized = 1;
		}

		uint32_t *row = buffer;
		for(uint32_t i = 0; i < BUFFER_HEIGHT; ++i) {
			uint32_t offs = p1_stars[i];

			if((offs > 0) & (offs < BUFFER_WIDTH)) {
				*(row + offs) = p1_star_colors[(i % 4)];
			}

			p1_stars[i] -= (1 + (i % 4));
			if(p1_stars[i] <= 0) {
				p1_stars[i] += BUFFER_WIDTH + 30;
			}

			row += BUFFER_WIDTH;
		}

		/*
		 *  Logo
		 */
		uint32_t p1_rotating_logo_frame_height = p1_rotating_logo->height / p1_rotating_logo_steps;
		uint32_t *data = buffer + P1_ROTATING_LOGO_Y_START * BUFFER_WIDTH + ((BUFFER_WIDTH - p1_rotating_logo->width) / 2);
		uint8_t *src = p1_rotating_logo->data + 2 * p1_rotating_logo->width + (((rotatingLogoStep / 2) % p1_rotating_logo_steps) * p1_rotating_logo->width * p1_rotating_logo_frame_height);
		for(uint32_t y = 0; y < p1_rotating_logo_frame_height - 6; ++y) {
			row = data;
			for(uint32_t x = 0; x < p1_rotating_logo->width; ++x) {
				if(*src++) {
					*row = p1_rotating_logo_fade_colors[y];
				}
				++row;
			}
			data += BUFFER_WIDTH;
		}
	}

	/*
	 * Show Presents + Fashionating
	 * Presents from right, fashionating from the left.
	 */
	if(p1_frame >= 1550) {
		uint8_t *source;
		uint8_t *dest;
		uint32_t *row;


		if(p1_presents_counter != 17) {
			++p1_presents_counter;
		}

		memset(p1_temp_buffer, 0, (352 + p1_presents_fashionating->width) * 15);
		source = p1_presents_fashionating->data;
		dest = p1_temp_buffer + BUFFER_WIDTH - (p1_presents_counter * 15);
		for(uint32_t y = 0; y < 14; ++y) {
			for(uint32_t x = 0; x < p1_presents_fashionating->width; ++x) {
				*dest++ = *source++;
			}
			dest += BUFFER_WIDTH;
		}

		source = p1_temp_buffer;
		row = buffer + P1_PRESENTS_Y_START * BUFFER_WIDTH;
		for(uint32_t y = 0; y < 14; ++y) {
			for(uint32_t x = 0; x < BUFFER_WIDTH; ++x) {
				if(*source++) {
					*row = p1_presents_colors[y];
				}
				++row;
			}
			source += p1_presents_fashionating->width;
		}

		memset(p1_temp_buffer, 0, (352 + p1_presents_fashionating->width) * 15);
		source = p1_presents_fashionating->data + p1_presents_fashionating->width * 14;
		dest = p1_temp_buffer + (p1_presents_counter * 15);
		for(uint32_t y = 0; y < 14; ++y) {
			for(uint32_t x = 0; x < p1_presents_fashionating->width; ++x) {
				*dest++ = *source++;
			}
			dest += BUFFER_WIDTH;
		}

		source = p1_temp_buffer + p1_presents_fashionating->width;
		row = buffer + P1_FASHIONATING_Y_START * BUFFER_WIDTH;
		for(uint32_t y = 0; y < 14; ++y) {
			for(uint32_t x = 0; x < BUFFER_WIDTH; ++x) {
				if(*source++) {
					*row = p1_fashionating_colors[y];
				}
				++row;
			}
			source += p1_presents_fashionating->width;
		}

	}

	/*
	 * scroller
	 */
	if(p1_frame >= 1940) {
		p1_scroller();

		/*
		 * Render Bling stars
		 */
		if((p1_frame % 81) == 0) {
			p1_bling_star_phase_index = ARRAYSIZE(p1_bling_star_phases);
			if(++p1_bling_pos_index == ARRAYSIZE(p1_bling_sprite_locations)) {
				p1_bling_pos_index = 0;
			}
		}

		if(p1_bling_star_phase_index) {

			uint8_t *src = p3_stars->data + p1_bling_star_phases[p1_bling_star_phase_index] * 15;
			uint32_t *dest = buffer + (BUFFER_WIDTH * p1_bling_sprite_locations[p1_bling_pos_index].y + p1_bling_sprite_locations[p1_bling_pos_index].x);
			for(uint32_t y = 0; y < p3_stars->height; ++y) {
				uint32_t *row = dest;
				for(uint32_t x = 0; x < p3_stars->height; ++x) {
					if(*src) {
						*row = p3_stars->palette[*src];
					}
					++src;
					++row;
				}
				dest += BUFFER_WIDTH;
				src += p3_stars->width - p3_stars->height;

			}
			--p1_bling_star_phase_index;
		}
	}

	++p1_frame;
	return mkfw_is_button_pressed(window, MOUSE_BUTTON_LEFT);
}

