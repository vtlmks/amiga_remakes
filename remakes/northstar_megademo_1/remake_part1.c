// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

/*
 *
 */

struct Firework {
	uint8_t quadrant;  // Determines movement direction
	uint8_t x;         // X-coordinate
	uint8_t y;         // Y-coordinate
	uint8_t dx;        // X displacement per step
	uint8_t dy;        // Y displacement per step
	uint8_t age;       // Current age of the firework
	uint8_t lifespan;  // Maximum lifespan before reset
	uint8_t padding;   // Reserved (unused)
};

#include "firework_data.h"

static struct Firework firework_data_copy[216];

static const uint32_t p1_copper_background[207] = {	// cyan -> blue -> purple -> red -> yellow -> green -> cyan...
	0x0088ffff, 0x0077ffff, 0x0066ffff, 0x0055ffff, 0x0044ffff, 0x0033ffff, 0x0022ffff, 0x0011ffff, 0x0000ffff,
	0x1100ffff, 0x2200ffff, 0x3300ffff, 0x4400ffff, 0x5500ffff, 0x6600ffff, 0x7700ffff, 0x8800ffff, 0x9900ffff,
	0xaa00ffff, 0xbb00ffff, 0xcc00ffff, 0xdd00ffff, 0xee00ffff, 0xff00ffff, 0xff00eeff, 0xff00ddff, 0xff00ccff,
	0xff00bbff, 0xff00aaff, 0xff0099ff, 0xff0088ff, 0xff0077ff, 0xff0066ff, 0xff0055ff, 0xff0044ff, 0xff0033ff,
	0xff0022ff, 0xff0011ff, 0xff0000ff, 0xff1100ff, 0xff2200ff, 0xff3300ff, 0xff4400ff, 0xff5500ff, 0xff6600ff,
	0xff7700ff, 0xff8800ff, 0xff9900ff, 0xffaa00ff, 0xffbb00ff, 0xffcc00ff, 0xffdd00ff, 0xffee00ff, 0xffff00ff,
	0xeeff00ff, 0xddff00ff, 0xccff00ff, 0xbbff00ff, 0xaaff00ff, 0x99ff00ff, 0x88ff00ff, 0x77ff00ff, 0x66ff00ff,
	0x55ff00ff, 0x44ff00ff, 0x33ff00ff, 0x22ff00ff, 0x11ff00ff, 0x00ff00ff, 0x00ff11ff, 0x00ff22ff, 0x00ff33ff,
	0x00ff44ff, 0x00ff55ff, 0x00ff66ff, 0x00ff77ff, 0x00ff88ff, 0x00ff99ff, 0x00ffaaff, 0x00ffbbff, 0x00ffccff,
	0x00ffddff, 0x00ffeeff, 0x00ffffff, 0x00eeffff, 0x00ddffff, 0x00ccffff, 0x00bbffff, 0x00aaffff, 0x0099ffff,
	0x0088ffff, 0x0077ffff, 0x0066ffff, 0x0055ffff, 0x0044ffff, 0x0033ffff, 0x0022ffff, 0x0011ffff, 0x0000ffff,
	0x1100ffff, 0x2200ffff, 0x3300ffff, 0x4400ffff, 0x5500ffff, 0x6600ffff, 0x7700ffff, 0x8800ffff, 0x9900ffff,
	0xaa00ffff, 0xbb00ffff, 0xcc00ffff, 0xdd00ffff, 0xee00ffff, 0xff00ffff, 0xff00eeff, 0xff00ddff, 0xff00ccff,
	0xff00bbff, 0xff00aaff, 0xff0099ff, 0xff0088ff, 0xff0077ff, 0xff0066ff, 0xff0055ff, 0xff0044ff, 0xff0033ff,
	0xff0022ff, 0xff0011ff, 0xff0000ff, 0xff1100ff, 0xff2200ff, 0xff3300ff, 0xff4400ff, 0xff5500ff, 0xff6600ff,
	0xff7700ff, 0xff8800ff, 0xff9900ff, 0xffaa00ff, 0xffbb00ff, 0xffcc00ff, 0xffdd00ff, 0xffee00ff, 0xffff00ff,
	0xeeff00ff, 0xddff00ff, 0xccff00ff, 0xbbff00ff, 0xaaff00ff, 0x99ff00ff, 0x88ff00ff, 0x77ff00ff, 0x66ff00ff,
	0x55ff00ff, 0x44ff00ff, 0x33ff00ff, 0x22ff00ff, 0x11ff00ff, 0x00ff00ff, 0x00ff11ff, 0x00ff22ff, 0x00ff33ff,
	0x00ff44ff, 0x00ff55ff, 0x00ff66ff, 0x00ff77ff, 0x00ff88ff, 0x00ff99ff, 0x00ffaaff, 0x00ffbbff, 0x00ffccff,
	0x00ffddff, 0x00ffeeff, 0x00ffffff, 0x00eeffff, 0x00ddffff, 0x00ccffff, 0x00bbffff, 0x00aaffff, 0x0099ffff,
	0x0088ffff, 0x0077ffff, 0x0066ffff, 0x0055ffff, 0x0044ffff, 0x0033ffff, 0x0022ffff, 0x0011ffff, 0x0000ffff,
	0x1100ffff, 0x2200ffff, 0x3300ffff, 0x4400ffff, 0x5500ffff, 0x6600ffff, 0x7700ffff, 0x8800ffff, 0x9900ffff,
	0xaa00ffff, 0xbb00ffff, 0xcc00ffff, 0xdd00ffff, 0xee00ffff, 0xff00ffff, 0xff00eeff, 0xff00ddff, 0xff00ccff,
};

#define FIREWORK_BUFFER_WIDTH 201
#define FIREWORK_BUFFER_HEIGHT 206
static uint32_t firework_buffer[FIREWORK_BUFFER_WIDTH * FIREWORK_BUFFER_HEIGHT];
static uint32_t firework_noclear = 0;

static void p1_init() {
	memcpy(firework_data_copy, firework_data, sizeof(firework_data));
}

static void p1_firework() {
	// PROFILE_FUNCTION();
	if((state.frame_number % 3) == 0) {
		for(uint32_t particle_index = 0; particle_index < 216; ++particle_index) {
			struct Firework *data = &firework_data_copy[particle_index];

			// Clear old position
			if(!firework_noclear) {
				firework_buffer[data->x + (data->y * FIREWORK_BUFFER_WIDTH)] = 0x0;
			}

			// Apply movement based on quadrant
			int dx = (data->quadrant & 0x2) ? data->dx : -data->dx;
			int dy = (data->quadrant & 0x1) ? data->dy : -data->dy;

			data->x += dx;
			data->y += dy;

			// Age and reset logic
			data->age++;
			if(data->age >= data->lifespan) {
				// Increment quadrant and wrap at 4
				uint8_t next_quadrant = (data->quadrant + 1) & 0x3;

				// Reset to original data
				*data = firework_data[particle_index];

				// Apply new quadrant
				data->quadrant = next_quadrant;
			}

			// Draw new position
			firework_buffer[data->x + (data->y * FIREWORK_BUFFER_WIDTH)] = p1_copper_background[data->y];
		}
	}

	uint32_t *src = firework_buffer;
	uint32_t *dest = buffer + ((BUFFER_WIDTH - FIREWORK_BUFFER_WIDTH) >> 1) + (((BUFFER_HEIGHT - FIREWORK_BUFFER_HEIGHT) >> 1) * BUFFER_WIDTH);
	for(uint32_t y = 0; y < FIREWORK_BUFFER_HEIGHT; ++y) {
		for(uint32_t x = 0; x < FIREWORK_BUFFER_WIDTH; ++x) {
			dest[x] = src[x];
		}
		src += FIREWORK_BUFFER_WIDTH;
		dest += BUFFER_WIDTH;
	}
}

struct p1_string_data {
	uint32_t y_offset;
	uint32_t x_offset;
	char *string;
	uint8_t dat;
};

#define TEXT_LINE1_Y (((BUFFER_HEIGHT >> 1) - 23 ) * BUFFER_WIDTH)
#define TEXT_LINE2_Y (((BUFFER_HEIGHT >> 1) - 3 ) * BUFFER_WIDTH)
#define TEXT_OFFSET_X (((BUFFER_WIDTH - (17*16)) >> 1) + 8)
#define END_OF_TEXT  0xff

struct p1_string_data p1_text_data[][2] = {
	{ { TEXT_LINE1_Y, TEXT_OFFSET_X, "  AFTER A LONG   ", 0 }, { TEXT_LINE2_Y, TEXT_OFFSET_X, "PERIOD OF SILENCE", 0 } },
	{ { TEXT_LINE1_Y, TEXT_OFFSET_X, "  ARE WE BACK    ", 0 }, { TEXT_LINE2_Y, TEXT_OFFSET_X, "     WITH A      ", 0 } },
	{ { TEXT_LINE1_Y, TEXT_OFFSET_X, "                 ", 0 }, { TEXT_LINE2_Y, TEXT_OFFSET_X, " MEGA-DEMO-DISK  ", 0 } },
	{ { TEXT_LINE1_Y, TEXT_OFFSET_X, " COPYRIGHT 1988  ", 0 }, { TEXT_LINE2_Y, TEXT_OFFSET_X, " NORTH STAR LTD  ", 0 } },
	{ { TEXT_LINE1_Y, TEXT_OFFSET_X, "                 ", 0 }, { TEXT_LINE2_Y, TEXT_OFFSET_X, "                 ", END_OF_TEXT } },
};

uint32_t p1_font_colors[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

uint32_t p1_text_writer_state = 1;
uint32_t p1_text_writer_delay = 0;
uint32_t p1_text_writer_line = 0;

#define COLOR_INCREMENT 0x11111100
#define FRONT_FACE_MAX  0x55555500
#define BORDER_MAX      0xcccccc00
#define TEXT_DELAY      20

#define TEXT_STATE_WRITE     0
#define TEXT_STATE_FADE_UP   1
#define TEXT_STATE_DELAY     2
#define TEXT_STATE_FADE_DOWN 3

__attribute__((always_inline))
static inline uint32_t fade_up(uint32_t color, uint32_t max) {
	return (color < max) ? (color + COLOR_INCREMENT) : color;
}

__attribute__((always_inline))
static inline uint32_t fade_down(uint32_t color) {
	return (color > 0) ? (color - COLOR_INCREMENT) : color;
}

static void p1_render_text() {
	for(uint32_t i = 0; i < 2; ++i) {
		uint32_t y_offset = p1_text_data[p1_text_writer_line][i].y_offset;
		uint32_t x_offset = p1_text_data[p1_text_writer_line][i].x_offset;
		char *c = p1_text_data[p1_text_writer_line][i].string;

		uint8_t *small_font_data = part1_small_font_data->data;

		uint32_t *dest = buffer + y_offset + x_offset;
		while(*c) {
			uint8_t *src = small_font_data + (*c++ - 0x20) * 16 * 16;
			uint32_t *dst = dest;

			for(uint32_t y = 0; y < 16; ++y) {
				for(uint32_t x = 0; x < 16; ++x) {
					p1_font_colors[0] = dst[x];
					dst[x] = p1_font_colors[*src++];
				}
				dst += BUFFER_WIDTH;
			}
			dest += 16;
		}
	}

	if((state.frame_number % 3) == 0) {
		switch(p1_text_writer_state) {
			case TEXT_STATE_WRITE: {
				if(p1_text_data[p1_text_writer_line][1].dat == 0xff) {
					p1_text_writer_line = 0;
				} else {
					p1_text_writer_line++;
				}
				p1_text_writer_state = TEXT_STATE_FADE_UP;
			} break;

			case TEXT_STATE_FADE_UP: {
				p1_font_colors[1] = fade_up(p1_font_colors[1], FRONT_FACE_MAX);
				p1_font_colors[2] = fade_up(p1_font_colors[2], BORDER_MAX);
				p1_font_colors[3] = fade_up(p1_font_colors[3], BORDER_MAX);

				if(p1_font_colors[1] == FRONT_FACE_MAX && p1_font_colors[2] == BORDER_MAX) {
					p1_text_writer_state = TEXT_STATE_DELAY;
				}
			} break;

			case TEXT_STATE_DELAY: {
				if(++p1_text_writer_delay == TEXT_DELAY) {
					p1_text_writer_delay = 0;
					p1_text_writer_state = TEXT_STATE_FADE_DOWN;
				}
			} break;

			case TEXT_STATE_FADE_DOWN: {
				p1_font_colors[1] = fade_down(p1_font_colors[1]);
				p1_font_colors[2] = fade_down(p1_font_colors[2]);
				p1_font_colors[3] = fade_down(p1_font_colors[3]);

				if(p1_font_colors[1] == 0 && p1_font_colors[2] == 0) {
					p1_text_writer_state = TEXT_STATE_WRITE;
				}
			} break;
		}
	}
}

static uint32_t p1_update()  {
	// PROFILE_NAMED("part1 all");
	if(mkfw_is_button_pressed(window, MOUSE_BUTTON_RIGHT)) {
		firework_noclear = !firework_noclear;
	}

	p1_firework();
	p1_render_text();

	return mkfw_is_button_pressed(window, MOUSE_BUTTON_LEFT);
}
