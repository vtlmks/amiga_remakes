// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

__attribute__((always_inline))
static inline void fade_colors(uint32_t * restrict colors, uint32_t * restrict color_target, size_t num_colors) {
	if(num_colors == 0) __builtin_unreachable();

	for(size_t i = 0; i < num_colors; i++) {
		uint32_t px   = *colors;
		uint32_t tgt  = *color_target++;

		uint8_t r = (px >> 28);
		uint8_t g = (px >> 20) & 0xf;
		uint8_t b = (px >> 12) & 0xf;

		uint8_t r_tgt = (tgt >> 28);
		uint8_t g_tgt = (tgt >> 20) & 0xf;
		uint8_t b_tgt = (tgt >> 12) & 0xf;

		r += (r < r_tgt) - (r > r_tgt);
		g += (g < g_tgt) - (g > g_tgt);
		b += (b < b_tgt) - (b > b_tgt);

		uint32_t new_color = r << 24 | g << 16 | b << 8 | 0xf;
		*colors++ = new_color << 4 | new_color;
	}
}
