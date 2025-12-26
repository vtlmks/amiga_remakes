// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// [=]===^=[ base setup ]============================================================^===[=]

#include "platform.c"

#define BUFFER_WIDTH  (346 << 0)
#define BUFFER_HEIGHT (270 << 0)

// [=]===^=[ remake stuff below ]============================================================^===[=]
#include "demo.c"

INCBIN_BYTES(music, "data/2d6.mod");

static struct micromod_state module;


#define FADE_IN_TIME 16
#define FADE_OUT_TIME 16
#define VECTOR_BOB_TIME 750
#define LOGO_TIME 172

enum wf_state {
	STATE_SHOW_LOGO,					// 172 frames
	STATE_PRECALC,						// from the precalc_delays[] array
	STATE_FADE_VECTORBOBS,			// 16 frames
	STATE_VECTORBOBS,					// 750 frames
	STATE_FADE_AWAY_VECTORBOBS,	// 16 frames
};

static enum wf_state run_state = STATE_SHOW_LOGO;

// [=]===^=[ audio_callback ]============================================================^===[=]
static void remake_audio_callback(int16_t *data, size_t frames) {
	micromod_get_audio(&module, (short*)data, frames);

	for(size_t i = 0; i < frames; i++) {
		int32_t old_left = (int32_t)data[i * 2];
		int32_t old_right = (int32_t)data[i * 2 + 1];

		int32_t mixed_left = old_left + (old_right * 3) / 4;
		int32_t mixed_right = old_right + (old_left * 3) / 4;

		// Shift right by 1 to prevent clipping and scale down
		data[i * 2] = (int16_t)(mixed_left >> 1);
		data[i * 2 + 1] = (int16_t)(mixed_right >> 1);
	}
}

struct star {
	int16_t x;
	int16_t y;
	uint16_t speed;
};

struct star stars[79];
struct rng_state base_rand;
static uint32_t star_colors[] = {
	0x00000000, 0x333355ff, 0x444466ff, 0x555577ff, 0x666688ff, 0x777799ff, 0x8888aaff, 0x9999bbff, 0xaaaaccff, 0xff00ffff
};

// [=]===^=[ remake_init ]============================================================^===[=]
static void remake_init(struct platform_state *state) {
	change_resolution(state, BUFFER_WIDTH, BUFFER_HEIGHT);
	text_writer_init();

	xor_init_rng(&base_rand, 187481201);

	for(size_t i = 0; i < 79; ++i) {
		stars[i].speed = ((79 - i) & 0x7) + 1;
		stars[i].y = 25 + 34 + (i << 1);
		stars[i].x = xor_generate_random(&base_rand) % state->buffer_width;
	}

	struct ugg *initializers[] = {
		bob01, bob02, bob03, bob04, bob05, bob06, bob07, bob08, bob09, bob0a, bob0b, bob0c, bob0d,
		bob11, bob12, bob13, bob14, bob15, bob16, bob17, bob18, bob19, bob1a, bob1b, bob1c, bob1d,
		bob21, bob22, bob23, bob24, bob25, bob26, bob27, bob28, bob29, bob2a, bob2b, bob2c, bob2d,
		bob31, bob32, bob33, bob34, bob35, bob36, bob37, bob38, bob39, bob3a, bob3b, bob3c, bob3d
	};
	memcpy(bobs, initializers, sizeof(bobs));

	micromod_initialize(&module, (signed char*)music, 48000);
	mkfw_audio_callback =remake_audio_callback;
}

static void remake_options(struct options *opt) {
	opt->release_group = "Warfalcons";
	opt->release_title = "Wings of Victory";
	opt->window_title = "Warfalcons - Wings of Victory 1990-02\0";
}

// [=]===^=[ remake_frame ]============================================================^===[=]
static void remake_frame(struct platform_state *state) {
	uint32_t *dst1 = BUFFER_PTR(state, 0, 33);
	uint32_t *dst2 = BUFFER_PTR(state, 0, 238);
	for(size_t i = 0; i < state->buffer_width; ++i) {
		*dst1++ = 0x000099ff;
		*dst2++ = 0x000099ff;
	}

	for(size_t i = 0; i < 79; ++i) {
		if(stars[i].x >= 0 && stars[i].x < (int32_t)state->buffer_width) {
			uint32_t *dst = BUFFER_PTR(state, stars[i].x, stars[i].y);
			*dst = star_colors[stars[i].speed];
		}
		stars[i].x += stars[i].speed;
		if(stars[i].x > 480) {
			stars[i].x -= 512;
		}
	}
/*
 * NOTE(peter):
 *   Demo-states
 *   display logo for 172 frames, due to faulty vblank delay they will hit vblank multiple times at a time, making the timing be very off, it varies between 170-174, so I chose 172 frames
 *   then there is a delay of several hundred frames between the logo disapearing and the vector-bobs, they precalculate the vector bob motion, which is why it's random, I've counted frames so you get the accurate delays, enjoy!
 *   vector-bobs
 *   fade-up 16 frames
 *   display 750 frames
 *   fade down 16 frames
 */

	switch(run_state) {
		case STATE_SHOW_LOGO: {
			blit_full(state, logo, CENTER_X(state, logo->width), 79, 0);

			if(frame_count++ == LOGO_TIME) {
				frame_count = 0;
				run_state = STATE_PRECALC;
			}
		} break;

		case STATE_PRECALC: {

			if(frame_count++ == precalc_delays[object_index & 0x7]) {
				frame_count = 0;
				run_state = STATE_FADE_VECTORBOBS;
			}
		} break;

		case STATE_FADE_VECTORBOBS: {
			fade_colors(bob_palette, bobs[0]->palette, 32);
			render_vector_bobs(state, object_index & 0x7);

			if(frame_count++ == FADE_IN_TIME) {
				frame_count = 0;
				run_state = STATE_VECTORBOBS;
			}
		} break;
		case STATE_VECTORBOBS: {
			render_vector_bobs(state, object_index & 0x7);

			if(frame_count++ == VECTOR_BOB_TIME) {
				frame_count = 0;
				run_state = STATE_FADE_AWAY_VECTORBOBS;
			}
		} break;
		case STATE_FADE_AWAY_VECTORBOBS: {
			fade_colors(bob_palette, black_palette, 32);
			render_vector_bobs(state, object_index & 0x7);

			if(frame_count++ == FADE_OUT_TIME) {
				object_index++;
				angle_x = 0;		// reset rotation
				angle_y = 0;
				angle_z = 0;
				frame_count = 0;
				run_state = STATE_SHOW_LOGO;
			}
		} break;
	}

	text_writer_update();
	text_writer_render(state);

}

// [=]===^=[ remake_shutdown ]============================================================^===[=]
static void remake_shutdown(struct platform_state *state) {
	mkfw_audio_callback = 0;
}

