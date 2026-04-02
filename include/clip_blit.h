// Copyright (c) 2026 Peter Fors
// SPDX-License-Identifier: MIT


struct rect {
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
};

struct blit_op {
	struct ugg *src;
	struct rect src_rect;
	int32_t dst_x;
	int32_t dst_y;
	struct rect clip_rect;
	uint32_t *palette;
};

struct clip_info {
	uint8_t *src;			// Pointer to first source pixel to read
	uint32_t *dst;			// Pointer to first destination pixel to write
	uint32_t width;		// Width to blit (in pixels)
	uint32_t height;		// Height to blit (in pixels)
	uint32_t src_stride;	// Bytes to add to src to move to next row
	uint32_t dst_stride;	// Pixels to add to dst to move to next row
	uint8_t visible;		// Non-zero if visible (has pixels to render)
};

__attribute__((always_inline))
static inline struct clip_info calculate_clip(struct platform_state *state, struct blit_op *op) {
	struct clip_info info = { 0 };

	struct rect src_rect = op->src_rect;
	if(src_rect.w == 0 || src_rect.h == 0) {
		src_rect = (struct rect){ 0, 0, op->src->width, op->src->height };
	}

	struct rect clip_rect = op->clip_rect;
	if(clip_rect.w == 0 || clip_rect.h == 0) {
		clip_rect = (struct rect){ 0, 0, state->buffer_width, state->buffer_height };
	}

	// Where the source image origin maps in destination space
	int32_t img_dx = op->dst_x - src_rect.x;
	int32_t img_dy = op->dst_y - src_rect.y;

	// 3-way intersection: source rect, image bounds, and dest clip (all in dest space)
	int32_t x1 = max(max(op->dst_x, img_dx), clip_rect.x);
	int32_t y1 = max(max(op->dst_y, img_dy), clip_rect.y);
	int32_t x2 = min(min(op->dst_x + src_rect.w, img_dx + (int32_t)op->src->width), clip_rect.x + clip_rect.w);
	int32_t y2 = min(min(op->dst_y + src_rect.h, img_dy + (int32_t)op->src->height), clip_rect.y + clip_rect.h);

	if(x1 >= x2 || y1 >= y2) {
		return info;
	}

	info.width = x2 - x1;
	info.height = y2 - y1;
	info.src = op->src->data + (y1 - img_dy) * (int32_t)op->src->width + (x1 - img_dx);
	info.dst = state->buffer + y1 * state->buffer_width + x1;
	info.src_stride = op->src->width;
	info.dst_stride = state->buffer_width;
	info.visible = 1;

	return info;
}

__attribute__((always_inline))
static inline void blit(struct platform_state *state, struct blit_op *op) {
	struct clip_info info = calculate_clip(state, op);
	if(!info.visible) {
		return;
	}

	uint32_t *palette = op->palette ? op->palette : op->src->palette;
	uint8_t * restrict data = info.src;
	uint32_t * restrict dst = info.dst;

	for(uint32_t y = 0; y < info.height; ++y, data += info.src_stride, dst += info.dst_stride) {
		for(uint32_t x = 0; x < info.width; ++x) {
			uint8_t val = data[x];
			palette[0] = dst[x];
			dst[x] = palette[val];
		}
	}
}
