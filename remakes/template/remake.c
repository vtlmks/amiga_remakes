// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// [=]===^=[ base setup ]============================================================^===[=]

#define BUFFER_WIDTH  (346 << 0)
#define BUFFER_HEIGHT (270 << 0)

#include "platform.c"

// [=]===^=[ remake stuff below ]============================================================^===[=]


// [=]===^=[ audio_callback ]============================================================^===[=]
static void remake_audio_callback(int16_t *data, size_t frames) {
	// PROFILE_FUNCTION();
	// memset(data, 0, 2*2*frames);
	// micromod_get_audio(&ctx, (short*)data, frames);
	// fc14play_FillAudioBuffer(data, frames);

	// NOTE(peter): Enable for 75% mix if the player doesn't have that functionality!
#if 0
	for(size_t i = 0; i < frames; i++) {
			int32_t old_left = (int32_t)data[i * 2];
			int32_t old_right = (int32_t)data[i * 2 + 1];

			int32_t mixed_left = old_left + (old_right * 3) / 4;
			int32_t mixed_right = old_right + (old_left * 3) / 4;

			// Shift right by 1 to prevent clipping and scale down
			data[i * 2] = (int16_t)(mixed_left >> 1);
			data[i * 2 + 1] = (int16_t)(mixed_right >> 1);
	}
#endif
}


// [=]===^=[ remake_init ]============================================================^===[=]
static void remake_init(struct remake_state *state) {
	change_resolution(state, BUFFER_WIDTH, BUFFER_HEIGHT);

	// mkfw_set_window_title(remake_title);
	// int mod_size = _2d6_end - _2d6_data;
	// micromod_initialise(&ctx, (signed char*)_2d6_data, 48000);
	// micromod_set_gain(&ctx, 64);
	// micromod_set_position(&ctx, 0);
	// fc14play_PlaySong((const uint8_t*)chambers_music_data, chambers_music_end - chambers_music_data, 48000);

	// mkfw_audio_callback = remake_audio_callback;
}

static void remake_options(struct options *opt) {
	opt->release_group = "REMAKE";
	opt->release_title = "TEMPLATE";
	opt->window_title = "remake - remake 1985-01\0";
}

// [=]===^=[ remake_frame ]============================================================^===[=]
static void remake_frame(struct remake_state *state) {
	// PROFILE_FUNCTION();
}

// [=]===^=[ remake_shutdown ]============================================================^===[=]
static void remake_shutdown(struct remake_state *state) {
	mkfw_audio_callback = 0;
	// fc14play_Close();
}
