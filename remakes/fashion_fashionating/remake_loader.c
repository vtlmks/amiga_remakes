// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


INCBIN_UGG(loader_logo, "data/p0_loader_logo.ugg");

static uint32_t loader_logo_red_palette[] = {
	0x00000000,0xffeeeeff,0xffddddff,0xffccccff,0xffbbbbff,0xffaaaaff,0xff9999ff,0xff8888ff,
	0xff7777ff,0xff6666ff,0xee4444ff,0xcc2222ff,0xbb0000ff,0x990000ff,0x770000ff,0x440000ff,
	0x771111ff,0x993333ff,0xaa4444ff,0xcc6666ff,0xdd8888ff,0xffbbbbff,0xeeaaaaff,0xdd8888ff,
	0xcc6666ff,0xaa4444ff,0x993333ff,0x771111ff,0x661111ff,0x440000ff,0x330000ff,0x661111ff,
};

static uint32_t loader(struct platform_state *state) {
	static uint32_t loader_timer = 200;
	static uint32_t counter = 0;
	static uint32_t roller = 0;

	++counter;

	// Simulate Amiga disk interrupt "bug" - palette rolls faster at irregular intervals
	if((counter % 21) == 0) roller += 2;
	if((counter % 3) == 0) roller += 1;

	// Roll palette while roller counter is active
	while(roller > 0) {
		uint32_t temp = loader_logo_red_palette[16];
		memmove(&loader_logo_red_palette[16], &loader_logo_red_palette[17], 15 * sizeof(uint32_t));
		loader_logo_red_palette[31] = temp;
		--roller;
	}

	uint32_t y = (state->buffer_height - loader_logo->height) >> 1;
	blit_full(state, loader_logo, CENTER_X(state, loader_logo->width), y, loader_logo_red_palette);

	if(loader_timer) {
		--loader_timer;
	} else {
		loader_timer = 75;
		return 1;
	}

	return mkfw_is_button_pressed(window, MOUSE_BUTTON_LEFT);
}
