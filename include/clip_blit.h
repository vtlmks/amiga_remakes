// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


struct rect {
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
};

struct clip_info {
	uint8_t *src;           // Pointer to first source pixel to read
	uint32_t *dst;          // Pointer to first destination pixel to write
	uint32_t width;         // Width to blit (in pixels)
	uint32_t height;        // Height to blit (in pixels)
	uint32_t src_stride;    // Bytes to add to src to move to next row
	uint32_t dst_stride;    // Pixels to add to dst to move to next row
	uint8_t visible;        // Non-zero if visible (has pixels to render)
};

__attribute__((always_inline))
static inline struct clip_info calculate_clip(struct platform_state *state, struct ugg *src, struct rect src_rect, int32_t dst_x, int32_t dst_y, struct rect dst_clip) {
	struct clip_info info = {0};

	// Clamp source rect to actual image bounds
	int32_t src_x1 = max(src_rect.x, 0);
	int32_t src_y1 = max(src_rect.y, 0);
	int32_t src_x2 = min(src_rect.x + src_rect.w, (int32_t)src->width);
	int32_t src_y2 = min(src_rect.y + src_rect.h, (int32_t)src->height);

	if(src_x1 >= src_x2 || src_y1 >= src_y2) {
		return info;
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
		return info;
	}

	// Calculate blit dimensions
	info.width = x2 - x1;
	info.height = y2 - y1;

	// Calculate source and destination pointers
	int32_t src_offset_x = src_x1 + (x1 - dst_x1);
	int32_t src_offset_y = src_y1 + (y1 - dst_y1);
	info.src = src->data + src_offset_y * (int32_t)src->width + src_offset_x;
	info.dst = state->buffer + y1 * state->buffer_width + x1;

	// Calculate strides
	info.src_stride = src->width;
	info.dst_stride = state->buffer_width;
	info.visible = 1;

	return info;
}

__attribute__((always_inline))
static inline struct clip_info calculate_clip_full(struct platform_state *state, struct ugg *src, int32_t dst_x, int32_t dst_y) {
	struct rect src_rect = { 0, 0, src->width, src->height };
	struct rect dst_rect = { 0, 0, state->buffer_width, state->buffer_height };
	return calculate_clip(state, src, src_rect, dst_x, dst_y, dst_rect);
}

__attribute__((always_inline))
static inline struct clip_info calculate_clip_rect(struct platform_state *state, struct ugg *src, int32_t dst_x, int32_t dst_y, struct rect clip_rect) {
	struct rect src_rect = { 0, 0, src->width, src->height };
	return calculate_clip(state, src, src_rect, dst_x, dst_y, clip_rect);
}

__attribute__((always_inline))
static inline struct clip_info calculate_clip_src_dst(struct platform_state *state, struct ugg *src, struct rect src_rect, int32_t dst_x, int32_t dst_y, struct rect dst_clip) {
	return calculate_clip(state, src, src_rect, dst_x, dst_y, dst_clip);
}

__attribute__((always_inline))
static inline struct clip_info calculate_clip_full_src_dst(struct platform_state *state, struct ugg *src, struct rect src_rect, int32_t dst_x, int32_t dst_y) {
	struct rect dst_rect = { 0, 0, state->buffer_width, state->buffer_height };
	return calculate_clip(state, src, src_rect, dst_x, dst_y, dst_rect);
}


__attribute__((always_inline))
static inline void blit_clipped_src_dst(struct platform_state *state, struct ugg *src, struct rect src_rect, int32_t dst_x, int32_t dst_y, struct rect dst_clip, uint32_t *override_palette) {
	struct clip_info info = calculate_clip(state, src, src_rect, dst_x, dst_y, dst_clip);
	if(!info.visible) return;

	uint32_t *palette = override_palette ? override_palette : src->palette;
	uint8_t * restrict data = info.src;
	uint32_t * restrict dst = info.dst;

	for(uint32_t y = 0; y < info.height; y++, data += info.src_stride, dst += info.dst_stride) {
		for(uint32_t x = 0; x < info.width; x++) {
			uint8_t val = data[x];
			palette[0] = dst[x];
			dst[x] = palette[val];
		}
	}
}

__attribute__((always_inline))
static inline void blit_full_src(struct platform_state *state, struct ugg *src, int32_t dst_x, int32_t dst_y, struct rect dst_rect, uint32_t *override_palette) {
	struct rect src_rect = { 0, 0, src->width, src->height };
	blit_clipped_src_dst(state, src, src_rect, dst_x, dst_y, dst_rect, override_palette);
}

__attribute__((always_inline))
static inline void blit_full(struct platform_state *state, struct ugg *src, int32_t dst_x, int32_t dst_y, uint32_t *override_palette) {
	struct rect src_rect = { 0, 0, src->width, src->height };
	struct rect dst_rect = { 0, 0, state->buffer_width, state->buffer_height };
	blit_clipped_src_dst(state, src, src_rect, dst_x, dst_y, dst_rect, override_palette);
}

__attribute__((always_inline))
static inline void blit_full_dst(struct platform_state *state, struct ugg *src, struct rect src_rect, int32_t dst_x, int32_t dst_y, uint32_t *override_palette) {
	struct rect dst_rect = { 0, 0, state->buffer_width, state->buffer_height };
	blit_clipped_src_dst(state, src, src_rect, dst_x, dst_y, dst_rect, override_palette);
}
