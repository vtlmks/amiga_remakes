// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


INCBIN_UGG(p4_greetings_text, "data/p4_greetings_text.ugg");

__attribute__((aligned(64)))
static uint32_t background_upscroll_color[] = {
	0x110000ff, 0x110000ff, 0x110000ff, 0x220000ff, 0x220000ff, 0x220000ff, 0x330000ff, 0x330000ff,
	0x330000ff, 0x440011ff, 0x440011ff, 0x440011ff, 0x550011ff, 0x550011ff, 0x550011ff, 0x660011ff,
	0x660011ff, 0x660011ff, 0x770022ff, 0x770022ff, 0x770022ff, 0x880022ff, 0x880022ff, 0x880022ff,
	0x990022ff, 0x990022ff, 0x990022ff, 0xaa0033ff, 0xaa0033ff, 0xaa0033ff, 0xbb0033ff, 0xbb0033ff,
	0xbb0033ff, 0xcc0033ff, 0xcc0033ff, 0xcc0033ff, 0xdd0044ff, 0xdd0044ff, 0xdd0044ff, 0xee0044ff,
	0xee0044ff, 0xee0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff,
	0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff,
	0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff,
	0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff,
	0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff,
	0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff,
	0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff,
	0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff,
	0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff, 0xff0044ff,
	0xff0044ff, 0xff0044ff, 0xff0044ff, 0xee0044ff, 0xee0044ff, 0xee0044ff, 0xdd0044ff, 0xdd0044ff,
	0xdd0044ff, 0xcc0033ff, 0xcc0033ff, 0xcc0033ff, 0xbb0033ff, 0xbb0033ff, 0xbb0033ff, 0xaa0033ff,
	0xaa0033ff, 0xaa0033ff, 0x990022ff, 0x990022ff, 0x990022ff, 0x880022ff, 0x880022ff, 0x880022ff,
	0x770022ff, 0x770022ff, 0x770022ff, 0x660011ff, 0x660011ff, 0x660011ff, 0x550011ff, 0x550011ff,
	0x550011ff, 0x440011ff, 0x440011ff, 0x440011ff, 0x330000ff, 0x330000ff, 0x330000ff, 0x220000ff,
	0x220000ff, 0x220000ff, 0x110000ff, 0x110000ff, 0x110000ff, 0x000000ff,
};

__attribute__((aligned(64)))
static uint32_t fsn_top_logo_colors[32] = {			// Blue
	0x00000000, 0xeeddffff, 0xbbbbffff, 0xaaaaffff, 0xaaaaffff, 0x9999ffff, 0x8888ffff, 0x7777ffff,
	0x7777ffff, 0x6666ffff, 0x4444eeff, 0x2222ccff, 0x0000bbff, 0x000099ff, 0x000077ff, 0x000044ff,
	0x333399ff, 0x4444aaff, 0x6666ccff, 0x8888ddff, 0xbbbbffff, 0xbbbbffff, 0x8888ddff, 0x6666ccff,
	0x4444aaff, 0x333399ff, 0x111177ff, 0x111166ff, 0x000044ff, 0x000044ff, 0x111166ff, 0x111177ff
};

__attribute__((aligned(64)))
static uint32_t fsn_bottom_logo_colors[32] = {	// Green
	0x00000000, 0xeeffddff, 0xbbffbbff, 0xaaffaaff, 0xaaffaaff, 0x99ff99ff, 0x88ff88ff, 0x77ff77ff,
	0x77ff77ff, 0x66ff66ff, 0x44ee44ff, 0x22cc22ff, 0x00bb00ff, 0x009900ff, 0x007700ff, 0x004400ff,
	0x339933ff, 0x44aa44ff, 0x66cc66ff, 0x88dd88ff, 0xbbffbbff, 0xbbffbbff, 0x88dd88ff, 0x66cc66ff,
	0x44aa44ff, 0x339933ff, 0x117711ff, 0x116611ff, 0x004400ff, 0x004400ff, 0x116611ff, 0x117711ff
};

// Macro definitions for dimensions and offsets
#define P4_LOGO_X_OFFSET                            55
#define P4_TOP_LOGO_Y_START                         13
#define P4_BOTTOM_LOGO_Y_START                      222
#define P4_UPSCROLL_Y_START                         60
#define P4_UPSCROLL_WIDTH                           BUFFER_WIDTH
#define P4_UPSCROLL_HEIGHT                          220
#define P4_UPSCROLL_VISIBLE_HEIGHT                  157
#define P4_UPSCROLL_RENDER_CONTENT_Y_OFFSET         158

__attribute__((aligned(64)))
static uint8_t p4_scroll_buffer[P4_UPSCROLL_WIDTH * P4_UPSCROLL_HEIGHT];

__attribute__((aligned(64)))
static uint32_t greetings_heights[] = { 8, 57, 61, 61, 61, 17, 17, 28 };
__attribute__((aligned(64)))
static uint8_t *greetings_offsets[8]; // Will be initialized at runtime

// Function to handle palette cycling
static void p4_color_cycle(uint32_t *palette) {
	uint32_t temp = palette[16];
	for(uint8_t i = 16; i < 31; ++i) {
		palette[i] = palette[i + 1];
	}
	palette[31] = temp;
}

static void part_4_init(void) {
	// Initialize greetings_offsets array using UGG data
	greetings_offsets[0] = p4_greetings_text->data;
	greetings_offsets[1] = p4_greetings_text->data + p4_greetings_text->width * 18;
	greetings_offsets[2] = p4_greetings_text->data + p4_greetings_text->width * 85;
	greetings_offsets[3] = p4_greetings_text->data + p4_greetings_text->width * 157;
	greetings_offsets[4] = p4_greetings_text->data + p4_greetings_text->width * 226;
	greetings_offsets[5] = p4_greetings_text->data + p4_greetings_text->width * 297;
	greetings_offsets[6] = p4_greetings_text->data + p4_greetings_text->width * 320;
	greetings_offsets[7] = p4_greetings_text->data + p4_greetings_text->width * 351;
}

static uint32_t p4_frame = 0;

static uint32_t part_4_render(void) {
	static struct {
		uint32_t counter;
		uint32_t scroll;
		uint32_t text_index;
		uint32_t next_reset;
		uint32_t next_text_update;
	} p4_state = {0};

	// Center logos
	uint32_t x_center = (BUFFER_WIDTH - loader_logo->width) >> 1;

	// Blit logos

	// blit_full(p2_stalaktites, CENTER_X(p2_stalaktites->width), 103, 0);

	blit_full(loader_logo, CENTER_X(loader_logo->width), P4_TOP_LOGO_Y_START, fsn_top_logo_colors);
	blit_full(loader_logo, CENTER_X(loader_logo->width), P4_BOTTOM_LOGO_Y_START, fsn_bottom_logo_colors);

	// fast_blit_with_palette(state->shared, loader_logo->data, loader_logo->width, loader_logo->height, fsn_top_logo_colors, x_center, P4_TOP_LOGO_Y_START);
	// fast_blit_with_palette(state->shared, loader_logo->data, loader_logo->width, loader_logo->height, fsn_bottom_logo_colors, x_center, P4_BOTTOM_LOGO_Y_START);

	// Color cycle
	if(p4_frame % 2 == 0) {
		p4_color_cycle(fsn_top_logo_colors);
		p4_color_cycle(fsn_bottom_logo_colors);
	}

	// Increment counter (up to 8000)
	if (p4_state.counter < 8000) {

		// Handle scroll & text
		if(p4_state.counter == p4_state.next_reset) {
			p4_state.scroll = 256;
			p4_state.next_reset += 1024;
			p4_state.next_text_update = p4_state.counter + 153;
		}

		if(p4_state.counter == p4_state.next_text_update) {
			uint8_t *src = greetings_offsets[p4_state.text_index];
			uint8_t *scroll_dst = p4_scroll_buffer + P4_UPSCROLL_RENDER_CONTENT_Y_OFFSET * P4_UPSCROLL_WIDTH + ((P4_UPSCROLL_WIDTH - p4_greetings_text->width) / 2);
			for(uint32_t i = 0; i < greetings_heights[p4_state.text_index]; ++i) {
				memcpy(scroll_dst, src, p4_greetings_text->width);
				scroll_dst += P4_UPSCROLL_WIDTH;
				src += p4_greetings_text->width;
			}
			p4_state.text_index = (p4_state.text_index + 1) % 8;
		}

		if(p4_state.scroll > 0) {
			memmove(p4_scroll_buffer, p4_scroll_buffer + P4_UPSCROLL_WIDTH, sizeof(p4_scroll_buffer) - P4_UPSCROLL_WIDTH);
			--p4_state.scroll;
		}

		p4_state.counter++;
	}

	// Render scroll buffer (center the 352-pixel scroll in 346-pixel buffer)
	uint32_t *data = buffer + P4_UPSCROLL_Y_START * BUFFER_WIDTH;
	uint8_t *src = p4_scroll_buffer + CENTER_X(P4_UPSCROLL_WIDTH);

	for(uint32_t y = 0; y < P4_UPSCROLL_VISIBLE_HEIGHT; ++y) {
		uint32_t color = background_upscroll_color[y];
		for(uint32_t x = 0; x < BUFFER_WIDTH; ++x) {
			data[x] = (src[x]) ? color : data[x];
		}
		data += BUFFER_WIDTH;
		src += P4_UPSCROLL_WIDTH;
	}

	++p4_frame;

	return mkfw_is_button_pressed(window, MOUSE_BUTTON_LEFT);
}
