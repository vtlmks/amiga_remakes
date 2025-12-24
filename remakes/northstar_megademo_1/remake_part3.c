// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

/*
 *
 */

INCBIN_UGG(part3_large_bg_data, "data/p3_large_background.ugg");
INCBIN_UGG(part3_logo_bounce_data, "data/p3_ns_logo_bounce.ugg");

static uint32_t background_palette[] = {
	0xff0000ff, 0xff1100ff, 0xff2200ff, 0xff3300ff, 0xff4400ff, 0xff5500ff, 0xff6600ff, 0xff7700ff, 0xff8800ff, 0xff9900ff, 0xffaa00ff, 0xffbb00ff, 0xffcc00ff, 0xffdd00ff, 0xffee00ff, 0xffff00ff,
	0xeeff00ff, 0xddff00ff, 0xccff00ff, 0xbbff00ff, 0xaaff00ff, 0x99ff00ff, 0x88ff00ff, 0x77ff00ff, 0x66ff00ff, 0x55ff00ff, 0x44ff00ff, 0x33ff00ff, 0x22ff00ff, 0x11ff00ff, 0x00ff00ff, 0x00ff11ff,
	0x00ff22ff, 0x00ff33ff, 0x00ff44ff, 0x00ff55ff, 0x00ff66ff, 0x00ff77ff, 0x00ff88ff, 0x00ff99ff, 0x00ffaaff, 0x00ffbbff, 0x00ffccff, 0x00ffddff, 0x00ffeeff, 0x00ffffff, 0x00eeffff, 0x00ddffff,
	0x00ccffff, 0x00bbffff, 0x00aaffff, 0x0099ffff, 0x0088ffff, 0x0077ffff, 0x0066ffff, 0x0055ffff, 0x0044ffff, 0x0033ffff, 0x0022ffff, 0x0011ffff, 0x0000ffff, 0x1100ffff, 0x2200ffff, 0x3300ffff,
	0x4400ffff, 0x5500ffff, 0x6600ffff, 0x7700ffff, 0x8800ffff, 0x9900ffff, 0xaa00ffff, 0xbb00ffff, 0xcc00ffff, 0xdd00ffff, 0xee00ffff, 0xff00ffff, 0xff00eeff, 0xff00ddff, 0xff00ccff, 0xff00bbff,
	0xff00aaff, 0xff0099ff, 0xff0088ff, 0xff0077ff, 0xff0066ff, 0xff0055ff, 0xff0044ff, 0xff0033ff, 0xff0022ff, 0xff0011ff, 0xff0000ff, 0xff1100ff, 0xff2200ff, 0xff3300ff, 0xff4400ff, 0xff5500ff,
	0xff6600ff, 0xff7700ff, 0xff8800ff, 0xff9900ff, 0xffaa00ff, 0xffbb00ff, 0xffcc00ff, 0xffdd00ff, 0xffee00ff, 0xffff00ff, 0xeeff00ff, 0xddff00ff, 0xccff00ff, 0xbbff00ff, 0xaaff00ff, 0x99ff00ff,
	0x88ff00ff, 0x77ff00ff, 0x66ff00ff, 0x55ff00ff, 0x44ff00ff, 0x33ff00ff, 0x22ff00ff, 0x11ff00ff, 0x00ff00ff, 0x00ff11ff, 0x00ff22ff, 0x00ff33ff, 0x00ff44ff, 0x00ff55ff, 0x00ff66ff, 0x00ff77ff,
	0x00ff88ff, 0x00ff99ff, 0x00ffaaff, 0x00ffbbff, 0x00ffccff, 0x00ffddff, 0x00ffeeff, 0x00ffffff, 0x00eeffff, 0x00ddffff, 0x00ccffff, 0x00bbffff, 0x00aaffff, 0x0099ffff, 0x0088ffff, 0x0077ffff,
	0x0066ffff, 0x0055ffff, 0x0044ffff, 0x0033ffff, 0x0022ffff, 0x0011ffff, 0x0000ffff, 0x1100ffff, 0x2200ffff, 0x3300ffff, 0x4400ffff, 0x5500ffff, 0x6600ffff, 0x7700ffff, 0x8800ffff, 0x9900ffff,
	0xaa00ffff, 0xbb00ffff, 0xcc00ffff, 0xdd00ffff, 0xee00ffff, 0xff00ffff, 0xff00eeff, 0xff00ddff, 0xff00ccff, 0xff00bbff, 0xff00aaff, 0xff0099ff, 0xff0088ff, 0xff0077ff, 0xff0066ff, 0xff0055ff,
	0xff0044ff, 0xff0033ff, 0xff0022ff, 0xff0011ff, 0xff0000ff, 0xff1100ff, 0xff2200ff, 0xff3300ff, 0xff4400ff, 0xff5500ff, 0xff6600ff, 0xff7700ff, 0xff8800ff, 0xff9900ff, 0xffaa00ff, 0xffbb00ff,
	0xffcc00ff, 0xffdd00ff, 0xffee00ff, 0xffff00ff, 0xeeff00ff, 0xddff00ff, 0xccff00ff, 0xbbff00ff, 0xaaff00ff, 0x99ff00ff, 0x88ff00ff, 0x77ff00ff, 0x66ff00ff, 0x55ff00ff, 0x44ff00ff, 0x33ff00ff,
	0x22ff00ff, 0x11ff00ff, 0x00ff00ff, 0x00ff11ff, 0x00ff22ff, 0x00ff33ff, 0x00ff44ff, 0x00ff55ff, 0x00ff66ff, 0x00ff77ff, 0x00ff88ff, 0x00ff99ff, 0x00ffaaff, 0x00ffbbff, 0x00ffccff, 0x00ffddff,
	0x00ffeeff, 0x00ffffff, 0x00eeffff, 0x00ddffff, 0x00ccffff, 0x00bbffff, 0x00aaffff, 0x0099ffff, 0x0088ffff, 0x0077ffff, 0x0066ffff, 0x0055ffff, 0x0044ffff, 0x0033ffff, 0x0022ffff, 0x0011ffff,
	0x0000ffff, 0x1100ffff, 0x2200ffff, 0x3300ffff, 0x4400ffff, 0x5500ffff, 0x6600ffff, 0x7700ffff, 0x8800ffff, 0x9900ffff, 0xaa00ffff, 0xbb00ffff, 0xcc00ffff, 0xdd00ffff, 0xee00ffff, 0xff00ffff,
	0xff00eeff, 0xff00ddff, 0xff00ccff, 0xff00bbff, 0xff00aaff, 0xff0099ff, 0xff0088ff, 0xff0077ff, 0xff0066ff, 0xff0055ff, 0xff0044ff, 0xff0033ff, 0xff0022ff, 0xff0011ff, 0xff0000ff, 0xff1100ff,
	0xff2200ff, 0xff3300ff, 0xff4400ff, 0xff5500ff, 0xff6600ff, 0xff7700ff, 0xff8800ff, 0xff9900ff, 0xffaa00ff, 0xffbb00ff, 0xffcc00ff, 0xffdd00ff, 0xffee00ff, 0xffff00ff, 0xeeff00ff, 0xddff00ff,
	0xccff00ff, 0xbbff00ff, 0xaaff00ff, 0x99ff00ff, 0x88ff00ff, 0x77ff00ff, 0x66ff00ff, 0x55ff00ff, 0x44ff00ff, 0x33ff00ff, 0x22ff00ff, 0x11ff00ff, 0x00ff00ff, 0x00ff11ff,
};

static uint32_t p3_src_offset_y;
static int32_t p3_direction = 1;



static uint32_t xx = 0;
uint32_t p3_bounce_sine[] = {
	 0x6,  0xc, 0x11, 0x16, 0x1b, 0x1f, 0x23, 0x27, 0x2a, 0x2d, 0x30, 0x32, 0x35, 0x37, 0x39, 0x3b, 0x3c, 0x3d,
	0x3d, 0x3c, 0x3b, 0x39, 0x37, 0x35, 0x32, 0x30, 0x2d, 0x2a, 0x27, 0x23, 0x1f, 0x1b, 0x16, 0x11,  0xc,  0x6,
};

static void p3_render_bouncing_logo(struct platform_state *state, uint32_t center_x, uint32_t center_y, uint32_t bounce_offset, uint32_t color) {
	// PROFILE_FUNCTION();
	uint8_t * restrict src = part3_logo_bounce_data->data;
	uint32_t * restrict dst = BUFFER_PTR(state, center_x, center_y - bounce_offset);

	uint32_t tmp_colors[2] = { 0x0, color };

	for(uint32_t y = 0; y < part3_logo_bounce_data->height; ++y) {
		for(uint32_t x = 0; x < part3_logo_bounce_data->width; ++x) {
			uint8_t pixel_color = *src++;
			tmp_colors[0] = dst[x];
			dst[x] = tmp_colors[pixel_color];
		}
		dst += state->buffer_width;
	}
}

static uint32_t palette_dark[]	= { 0x00000000, 0x00000000, 0x555555ff, 0x001155ff, 0x0022bbff, 0x332222ff, 0xaaaaaaff, 0x009999ff, };
static uint32_t palette_bright[]	= { 0x222222ff, 0xff0000ff, 0x887777ff, 0x002299ff, 0x0033ffff, 0x554444ff, 0xddddddff, 0x00ccccff, };

struct stripe {
	uint32_t count;
	int32_t displacement;
	uint32_t *palette;
};

static struct stripe stripes[] = {
	{ 84, -1, palette_bright },
	{  1, -1, palette_dark   },
	{  8,  0, palette_dark   },
	{  6, -1, palette_dark   },
	{  6, -2, palette_dark   },
	{  6, -3, palette_dark   },
	{  6, -4, palette_dark   },
	{  6, -5, palette_dark   },
	{  6, -6, palette_dark   },
	{  6, -7, palette_dark   },
	{  6, -8, palette_dark   },
	{  6, -7, palette_dark   },
	{  6, -6, palette_dark   },
	{  6, -5, palette_dark   },
	{  6, -4, palette_dark   },
	{  6, -3, palette_dark   },
	{  6, -2, palette_dark   },
	{ 11, -1, palette_dark   },
	{ 84, -1, palette_bright },
};

static void p3_render_big_bouncer(struct platform_state *state) {
	// PROFILE_FUNCTION();

	p3_src_offset_y += p3_direction;
	if(p3_src_offset_y == 0 || p3_src_offset_y == (part3_large_bg_data->height - state->buffer_height)) {
		p3_direction = -p3_direction; // Reverse direction
	}

	uint32_t center_x = (state->buffer_width - part3_large_bg_data->width) >> 1;
	uint32_t index_y = 15;

	uint8_t * restrict src = part3_large_bg_data->data + (p3_src_offset_y * part3_large_bg_data->width);
	uint32_t * restrict dst = BUFFER_PTR(state, center_x, 0);

	for(uint32_t stripe_index = 0; stripe_index < ARRAYSIZE(stripes); ++stripe_index) {
		uint32_t count = stripes[stripe_index].count;
		int32_t displacement = stripes[stripe_index].displacement;
		uint32_t * restrict palette = stripes[stripe_index].palette;

		for(uint32_t y = 0; y < count; ++y) {
			palette[1] = background_palette[index_y++];
			for(int32_t x = 0; x < part3_large_bg_data->width; ++x) {
				dst[x + displacement] = palette[src[x]];
			}
			dst += state->buffer_width;
			src += part3_large_bg_data->width;
		}
	}
}

static uint32_t p3_update(struct platform_state *state)  {
	// PROFILE_NAMED("part3 all");

	p3_render_big_bouncer(state);

	uint32_t bounce_offset1 = p3_bounce_sine[(xx + 0) % ARRAYSIZE(p3_bounce_sine)];
	uint32_t bounce_offset2 = p3_bounce_sine[(xx + 3) % ARRAYSIZE(p3_bounce_sine)];
	uint32_t bounce_offset3 = p3_bounce_sine[(xx + 6) % ARRAYSIZE(p3_bounce_sine)];
	xx++;

	uint32_t center_bounce_x = (state->buffer_width - part3_logo_bounce_data->width) >> 1;
	uint32_t center_bounce_y = 104 + 83 - part3_logo_bounce_data->height;	// the two first values are the upper part of the logo, plus the displaced middle area height.

	p3_render_bouncing_logo(state, center_bounce_x + 1, center_bounce_y, bounce_offset1, 0x555555ff);
	p3_render_bouncing_logo(state, center_bounce_x,     center_bounce_y, bounce_offset2, 0xaaaaaaff);
	p3_render_bouncing_logo(state, center_bounce_x - 1, center_bounce_y, bounce_offset3, 0xffffffff);

	return mkfw_is_button_pressed(state->window, MOUSE_BUTTON_LEFT);
}
