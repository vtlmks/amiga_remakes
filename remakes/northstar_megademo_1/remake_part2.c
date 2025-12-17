/*
 *
 */


#define NUM_STARS 116
static uint32_t scroll_positions[256];
static int32_t p2_stars[NUM_STARS];

uint8_t part2_c = 0;

static const uint8_t part2_sine_values[42] = {
	0x39, 0x33, 0x2d, 0x28, 0x23, 0x1f, 0x1b, 0x18, 0x15, 0x12, 0x10, 0x0e, 0x0c, 0x0a, 0x08, 0x06,
	0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x08, 0x0a,
	0x0c, 0x0e, 0x10, 0x12, 0x18, 0x1b, 0x1f, 0x23, 0x28, 0x2d, 0x33,
};
static uint32_t md1_p2_sine_background_color[] = {
	0x22ff22ff, 0x11ff11ff, 0x00ff00ff, 0x11ff11ff, 0x22ff22ff, 0x33ff33ff, 0x44ff44ff, 0x55ff55ff,
	0x66ff66ff, 0x77ff77ff, 0x88ff88ff, 0x99ff99ff, 0xaaffaaff, 0xbbffbbff, 0xccffccff, 0xddffddff,
	0xeeffeeff, 0xffffffff, 0xeeffeeff, 0xddffddff, 0xccffccff, 0xbbffbbff, 0xaaffaaff, 0x99ff99ff,
	0x88ff88ff, 0x77ff77ff, 0x66ff66ff, 0x55ff55ff, 0x44ff44ff, 0x33ff33ff, 0x22ff22ff, 0x11ff11ff,
	0x00ff00ff, 0x11ff11ff, 0x22ff22ff, 0x33ff33ff, 0x44ff44ff, 0x55ff55ff, 0x66ff66ff, 0x77ff77ff,
	0x88ff88ff, 0x99ff99ff, 0xaaffaaff, 0xbbffbbff, 0xccffccff, 0xddffddff, 0xeeffeeff, 0xffffffff,
	0xeeffeeff, 0xddffddff, 0xccffccff, 0xbbffbbff, 0xaaffaaff, 0x99ff99ff, 0x88ff88ff, 0x77ff77ff,
	0x66ff66ff, 0x55ff55ff, 0x44ff44ff, 0x33ff33ff, 0x22ff22ff, 0x11ff11ff, 0x00ff00ff, 0x11ff11ff,
	0x22ff22ff, 0x33ff33ff, 0x44ff44ff, 0x55ff55ff, 0x66ff66ff, 0x77ff77ff, 0x88ff88ff, 0x99ff99ff
};

static const uint32_t md1_p2_text_writer_background_colors[] = {
	0x000000ff, 0xaa4400ff, 0xbb5500ff, 0xcc7722ff, 0xdd9933ff, 0xeecc66ff, 0xeeee88ff, 0xffffeeff,
	0xffffeeff, 0xeeee88ff, 0xeecc66ff, 0xdd9933ff, 0xcc7722ff, 0xbb5500ff, 0xaa4400ff,
};

/*

000290EA unk_290EA:      dc.b   2                ; Probable_entry_point+32C↑o
000290EB                 dc.b   8
000290EC aPresents:      dc.b '*** PRESENTS :***'
000290FD                 dc.b $23 ; #
000290FE                 dc.b $16
000290FF                 dc.b   8
00029100 aANewIntro:     dc.b '*  A NEW INTRO  *'
00029111                 dc.b $23 ; #
00029112                 dc.b $2A ; *
00029113                 dc.b   8
00029114 aCodedByAtom:   dc.b '* CODED BY ATOM *'
00029125                 dc.b $23 ; #
00029126                 dc.b $3E ; >
00029127                 dc.b   8
00029128                 dc.b '*****************'
00029139 unk_29139:      dc.b $23 ; #            ; RAM:off_290D4↑o
0002913A                 dc.b   0
*/

static const uint8_t p2_scroll_text[] = {
	"  NORTH STAR& 1988    @"
	"                INTRO BY: ATOM      @"
	" ASSISTED BY: STARFIRE  @"
	"          MUZAX BY: TITAN     @"
	"  CONTACT NORTH STAR FOR SWAPPING THE HOTTEST STUFF, WRITE TO: "
	"NORTH STAR , HOGASTENSG.13 , 252 32 HELSINGBORG    OR CALL : (SWEDEN) 042-260664 (REX)"
	"     SPECIAL GREETINGS TO THE OTHER MEMBERS IN NORTH STAR: HEDDA, RILLE, REX, LIBERATOR,"
	" FOETUS, CRAB, GIZMO, ELECTRO, MC, ZOETROPE AND JAZZE !!!!      NORMAL GREETINGS TO: "
	"SINE * TRIAD * XAKK * SCIENCE 451 * VORTEX 42 * TETRAGON * THE ARNE TEAM * THE SILENTS "
	"* POWER-EXTREME * SOS * CPU * DEFIERS * TRIUMPH * HALLOWEN * WARHEAD * TTW * TEC "
	"* THE BEASTIE BOYS * WACO * AMIGOS * SAS * WOLFMAN * THUNDER CATS * FAIRLIGHT * WARLORDS "
	"* XL CRACKERS * HORROR CRACKINGS * NINJA CRACKING CREW * THE CHAMPS * CRAY * MLC * PMI "
	"* DURAC SYSTEMS * MIRACLE BOYS * TO PIZZERIA VINKELN FOR BRINGING US THE PIZZAZ AT 23:40, "
	"THATS WHAT I CALL SERVICE!! AND THE REST THAT I AM TO LAZY TO MENTION....... "
	"WELL, WHY NOT JUMP ON THE MOUSE ??????               (C) NORTH STAR&               ^"
};

static uint32_t logo_render() {
	// PROFILE_FUNCTION();
	// NOTE(peter): Update and scroll the northstar logo into place
	uint32_t scroll_speed = 2;
	uint32_t target_x = (BUFFER_WIDTH - part2_ns_logo_data->width) >> 1;
	uint32_t logo_target_reached = 1;

	for(uint32_t line = 0; line < part2_ns_logo_data->height; ++line) {
		if(scroll_positions[line] > target_x) {
			scroll_positions[line] = (scroll_positions[line] - scroll_speed > target_x) ? scroll_positions[line] - scroll_speed : target_x;
			logo_target_reached = 0;
		}

		uint32_t current_x = scroll_positions[line];
		if(current_x >= BUFFER_WIDTH) continue; // Skip rendering if the line is completely off-screen

		uint32_t *dest = buffer + ((15 + line) * BUFFER_WIDTH);
		uint8_t *src = &part2_ns_logo_data->data[line * part2_ns_logo_data->width];

		// Render the visible part of the line
		for(uint32_t x = current_x; x < BUFFER_WIDTH; ++x) {
			uint32_t logo_index = x - current_x;
			uint8_t color_index = src[logo_index];
			if(color_index == 0) continue;
			if(logo_index >= part2_ns_logo_data->width) break;
			dest[x] = part2_ns_logo_data->palette[color_index];
		}
	}
	return logo_target_reached;
}

#define MD1_P2_TEXT_WRITER_COLOR_COUNT 71
static uint32_t part2_text_writer_colors[MD1_P2_TEXT_WRITER_COLOR_COUNT];
static uint32_t part2_color_rows = 0;
static uint32_t part2_color_offset = 0;
static uint32_t p2_logo_reveal_done = 0;
static uint32_t p2_colors_done = 0;

static void p2_texteffect() {
	// PROFILE_FUNCTION();
	if(p2_logo_reveal_done & !p2_colors_done) {
		if((state.frame_number & 0x3) == 0) {
			part2_color_rows++;
		}

		if(part2_color_rows == MD1_P2_TEXT_WRITER_COLOR_COUNT) {
			p2_colors_done = 1;
		}
	}

	uint32_t *dst = part2_text_writer_colors;	// palette cycle
	for(uint32_t y = 0; y < part2_color_rows; ++y) {
		*dst++ = md1_p2_text_writer_background_colors[(part2_color_offset + y) % ARRAYSIZE(md1_p2_text_writer_background_colors)];
	}
	part2_color_offset++;

	uint8_t *src = part2_text_data->data;
	dst = buffer + (72 * BUFFER_WIDTH) + ((BUFFER_WIDTH - part2_text_data->width) >> 1);
	for(uint32_t y = 0; y < part2_text_data->height; ++y) {
		for(uint32_t x = 0; x < part2_text_data->width; ++x) {
			uint8_t color = *src++;
			if(color == 0) continue;
			switch(color) {
				case 1:
				case 3: { dst[x] = part2_text_data->palette[color]; } break;
				default: { dst[x] = part2_text_writer_colors[y]; } break;
			}
		}
		dst += BUFFER_WIDTH;
	}
}


static void p2_render_char_clipped(uint8_t *glyph_data, int32_t dest_x, int32_t dest_y) {
	// PROFILE_FUNCTION();

	int glyph_size = 16;

	// Clip region
	int32_t clip_x0 = 0;
	int32_t clip_x1 = 336;

	// Early-out if entirely outside vertically or horizontally
	if(dest_y >= BUFFER_HEIGHT || dest_y + glyph_size <= 0 || dest_x >= clip_x1 || dest_x + glyph_size <= clip_x0) {
		return;
	}

	// Calculate X clipping
	int x_start = 0;
	if(dest_x < clip_x0) {
		x_start = clip_x0 - dest_x;
	}

	int x_end = glyph_size;
	if(dest_x + glyph_size > clip_x1) {
		x_end = clip_x1 - dest_x;
	}

	if(x_start >= x_end) {
		return;
	}

	// Calculate Y clipping
	int y_start = 0;
	int y_end = glyph_size;
	if(dest_y < 0) {
		y_start = -dest_y;
	}
	if(dest_y + glyph_size > BUFFER_HEIGHT) {
		y_end = BUFFER_HEIGHT - dest_y;
	}

	if(y_start >= y_end) {
		return;
	}

	// Render the glyph
	for(int y = y_start; y < y_end; y++) {
		for(int x = x_start; x < x_end; x++) {
			int dest_index = (dest_y + y) * BUFFER_WIDTH + (dest_x + x);
			int src_index = y * glyph_size + x;
			uint32_t color_index = glyph_data[src_index];
			uint32_t line_color = md1_p2_sine_background_color[dest_y + y - 154];

			switch(color_index) {
				case 1:
					buffer[dest_index] = line_color;
					break;
				case 2:
				case 3:
					buffer[dest_index] = part1_small_font_data->palette[color_index];
					break;
			}
		}
	}
}


struct scroller_entity {
	uint8_t character;      // The character to render
	int32_t x_pos;          // Current x-position
	uint8_t sine_phase;     // Phase for sine motion
};

#define NUM_CHARACTERS 23
static struct scroller_entity scroller_entities[NUM_CHARACTERS];
static const uint8_t *current_text_ptr; // Pointer to the current position in the scroll text
static uint32_t p2_scroller_sleep;

static void p2_initialize_scroller(void) {
	current_text_ptr = p2_scroll_text;

	for(uint32_t i = 0; i < NUM_CHARACTERS; ++i) {
		scroller_entities[i].character = ' ';
		scroller_entities[i].x_pos = i * 16;
		scroller_entities[i].sine_phase = (i * 2)  % ARRAYSIZE(part2_sine_values);
	}
}


static void p2_init(void) {
	p2_initialize_scroller();

	for(uint32_t i = 0; i < NUM_STARS; ++i) {
		p2_stars[i] = xor_generate_random(&base_rand) & 0x7ff;
	}
	int32_t middle = part2_ns_logo_data->height / 2;

	for(uint32_t i = 0; i < part2_ns_logo_data->height; i++) {
		uint32_t diff = (i >= middle) ? (i - middle) : (middle - i);
		uint32_t line_offset = diff * 22;
		scroll_positions[i] = BUFFER_WIDTH + part2_ns_logo_data->width + line_offset;
	}
}


void p2_bouncing_scroller(void) {
	// PROFILE_FUNCTION();/

	if(!p2_colors_done) return;

	if(p2_scroller_sleep == 0) {
		for(uint32_t i = 0; i < NUM_CHARACTERS; ++i) {
			struct scroller_entity *entity = &scroller_entities[i];

			entity->x_pos -= 1;

			if(entity->x_pos <= -16) {
				entity->x_pos += NUM_CHARACTERS * 16;
				entity->sine_phase = (scroller_entities[(i + NUM_CHARACTERS - 1) % NUM_CHARACTERS].sine_phase + 2) % ARRAYSIZE(part2_sine_values);
				entity->character = *current_text_ptr++;
				if(*current_text_ptr == '@') {
					current_text_ptr++;
					p2_scroller_sleep = 200;
				}
				if(*current_text_ptr == '^') {
					current_text_ptr = p2_scroll_text;
				}
			}
		}
	} else {
		p2_scroller_sleep--;
	}

	for(uint32_t i = 0; i < NUM_CHARACTERS; ++i) {
		struct scroller_entity *entity = &scroller_entities[i];
		uint8_t sine_offset = part2_sine_values[entity->sine_phase];
		uint8_t *font_src = part1_small_font_data->data + (entity->character - ' ') * 256;

		p2_render_char_clipped(font_src, entity->x_pos, 154 + sine_offset);
		entity->sine_phase = (entity->sine_phase + 1) % ARRAYSIZE(part2_sine_values);
	}
}

static void p2_starfield(void) {
	uint32_t *dst = buffer + 17 * BUFFER_WIDTH;
	uint32_t star_add = (state.frame_number & 0x1);
	for(uint32_t i = 0; i < NUM_STARS; ++i) {
		int32_t x = p2_stars[i] & 0x7ff;
		if((x >= 0) && (x < 336)) {
			dst[x] = 0xffffffff;
		}
		dst += BUFFER_WIDTH * 2;
		p2_stars[i] += (i % 3) + 1;
	}
}

static void p2_render_logo(void) {
	// PROFILE_FUNCTION();
	if(p2_logo_reveal_done) {
		uint32_t logo_x_offset = (BUFFER_WIDTH - part2_ns_logo_data->width) >> 1;
		blit_full(part2_ns_logo_data, logo_x_offset, 15, 0);
	} else {
		p2_logo_reveal_done = logo_render();
	}
}

static uint32_t p2_update(void) {
	// PROFILE_NAMED("part2 all");

	p2_starfield();

	p2_render_logo();

	p2_texteffect();
	p2_bouncing_scroller();

	return mkfw_is_button_pressed(window, MOUSE_BUTTON_LEFT);
}

