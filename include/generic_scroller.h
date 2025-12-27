// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


#define SCROLL_BUFFER_WIDTH 512U // Fixed buffer width for wrapping
#define SCROLL_BUFFER_MASK 511U

struct font_char_info {
	uint32_t offset;  // Absolute pixel offset in bitmap
	uint16_t width;   // Character width in pixels
};

struct scroller_state {
	struct font_char_info *char_info;			// Optional character info array (offset and width per char)
	uint8_t (*process_char)(struct scroller_state *scr_state, uint8_t scroll_character); // Callback for custom behavior
	uint8_t *buffer;								// Pointer to the scroll buffer
	struct ugg *font;								// Font pointer
	uint8_t *text;									// Scroll text
	uint32_t dest_offset_y;						// Offset on screen
	uint32_t char_render_offset;				// Offset from start of buffer
	uint32_t char_next_render_offset;		// Next time we render a character to buffer
	uint32_t text_offset;						// Offset in scrolltext
	uint32_t pause_timer;						// If not 0, it will count down to zero
	uint32_t speed;								// The speed of the scroller
	uint32_t char_width;							// Width of each character
	uint32_t char_height;						// Height of each character
};

static struct scroller_state *scroller_new(uint32_t char_width, uint32_t char_height, uint32_t offset_y, uint32_t speed, uint8_t *text, struct ugg *font, struct font_char_info *char_info, uint8_t (*process_char)(struct scroller_state *, uint8_t));
static void scroller(struct scroller_state *scr_state);

// TODO(peter): Add #ifdef GENERIC_SCROLLER_IMPLEMENTATION

static uint8_t default_process_char(struct scroller_state *scr_state, uint8_t scroll_character) {
	if(scroll_character == 0) {		// Scrolltext end
		scr_state->text_offset = 0;
		return scr_state->text[scr_state->text_offset++];
	}
	return scroll_character;
}

static size_t vertical_offset(struct scroller_state *scr, uint8_t c) {
	return (c - ' ') * scr->char_width * scr->char_height;
}

static size_t horizontal_offset(struct scroller_state *scr, uint8_t c) {
	return (c - ' ') * scr->char_width;
}

static struct scroller_state *scroller_new(uint32_t char_width, uint32_t char_height, uint32_t offset_y, uint32_t speed, uint8_t *text, struct ugg *font, struct font_char_info *char_info, uint8_t (*process_char)(struct scroller_state *, uint8_t)) {
	struct scroller_state *scr_state = calloc(1, sizeof(struct scroller_state));
	scr_state->char_width = char_width;
	scr_state->char_height = char_height;
	scr_state->buffer = calloc(1, SCROLL_BUFFER_WIDTH * char_height);
	scr_state->speed = speed;
	scr_state->font = font;
	scr_state->text = text;
	scr_state->dest_offset_y = offset_y;
	scr_state->char_info = char_info;
	scr_state->process_char = process_char ? process_char : default_process_char;
	return scr_state;
}

static void scroller_remove(struct scroller_state *scr_state) {
	free(scr_state->buffer);
	free(scr_state);
}

static void scroller(struct scroller_state * restrict scr_state) {
	if(scr_state->pause_timer) {
		scr_state->pause_timer--;
		return;
	}

	while(scr_state->char_render_offset >= scr_state->char_next_render_offset) {
		uint8_t char_index = scr_state->process_char(scr_state, scr_state->text[scr_state->text_offset++]);

		size_t font_offset = vertical_offset(scr_state, char_index);
		uint32_t char_width = scr_state->char_width;

		if(scr_state->char_info) {
			font_offset = scr_state->char_info[char_index].offset;
			char_width = scr_state->char_info[char_index].width;
		}

		uint8_t *font_src = scr_state->font->data + font_offset;

		uint8_t *dst = scr_state->buffer;
		size_t dst_offset = scr_state->char_next_render_offset;
		for(size_t i = 0; i < scr_state->char_height; ++i) {
			for(size_t j = 0; j < char_width; ++j) {
				dst[(dst_offset + j) & SCROLL_BUFFER_MASK] = font_src[j];
			}
			font_src += scr_state->font->width;
			dst += SCROLL_BUFFER_WIDTH;
		}
		scr_state->char_next_render_offset += char_width;
	}
	scr_state->char_render_offset += scr_state->speed;
}

