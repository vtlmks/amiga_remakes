// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


struct rect {
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
};

__attribute__((always_inline))
static inline void blit_clipped_src_dst(struct ugg *src, struct rect src_rect, int32_t dst_x, int32_t dst_y, struct rect dst_clip, uint32_t *override_palette) {
	// Clamp source rect to actual image bounds
	int32_t src_x1 = max(src_rect.x, 0);
	int32_t src_y1 = max(src_rect.y, 0);
	int32_t src_x2 = min(src_rect.x + src_rect.w, src->width);
	int32_t src_y2 = min(src_rect.y + src_rect.h, src->height);

	if(src_x1 >= src_x2 || src_y1 >= src_y2) {
		return;
	}

	// Calculate destination bounds based on source rect
	int32_t dst_x1 = dst_x + (src_x1 - src_rect.x);
	int32_t dst_y1 = dst_y + (src_y1 - src_rect.y);
	int32_t dst_x2 = dst_x1 + (src_x2 - src_x1);
	int32_t dst_y2 = dst_y1 + (src_y2 - src_y1);

	// Clip against destination clip rect
	int32_t x1 = max(dst_x1, dst_clip.x);
	int32_t y1 = max(dst_y1, dst_clip.y);
	int32_t x2 = min(dst_x2, dst_clip.x + dst_clip.w);
	int32_t y2 = min(dst_y2, dst_clip.y + dst_clip.h);

	if(x1 >= x2 || y1 >= y2) {
		return;
	}

	uint32_t blit_width = x2 - x1;
	uint32_t blit_height = y2 - y1;

	// Adjust source data pointer for both source clipping and destination clipping
	int32_t src_offset_x = src_x1 + (x1 - dst_x1);
	int32_t src_offset_y = src_y1 + (y1 - dst_y1);
	uint8_t * restrict data = src->data + src_offset_y * src->width + src_offset_x;
	uint32_t * restrict dst = buffer + y1 * BUFFER_WIDTH + x1;
	uint32_t *palette = override_palette ? override_palette : src->palette;

	for(uint32_t y = 0; y < blit_height; y++, data += src->width, dst += BUFFER_WIDTH) {
		for(uint32_t x = 0; x < blit_width; x++) {
			uint8_t val = data[x];
			palette[0] = dst[x];
			dst[x] = palette[val];
		}
	}
}

__attribute__((always_inline))
static inline void blit_clipped(struct ugg *src, int32_t dst_x, int32_t dst_y, struct rect clip_rect, uint32_t *override_palette) {
	struct rect src_rect = { 0, 0, src->width, src->height };
	blit_clipped_src_dst(src, src_rect, dst_x, dst_y, clip_rect, override_palette);
}

__attribute__((always_inline))
static inline void blit_full(struct ugg *src, int32_t dst_x, int32_t dst_y, uint32_t *override_palette) {
	struct rect src_rect = { 0, 0, src->width, src->height };
	struct rect dst_rect = { 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT };
	blit_clipped_src_dst(src, src_rect, dst_x, dst_y, dst_rect, override_palette);
}

__attribute__((always_inline))
static inline void blit_full_src_dst(struct ugg *src, struct rect src_rect, int32_t dst_x, int32_t dst_y, uint32_t *override_palette) {
	struct rect dst_rect = { 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT };
	blit_clipped_src_dst(src, src_rect, dst_x, dst_y, dst_rect, override_palette);
}

