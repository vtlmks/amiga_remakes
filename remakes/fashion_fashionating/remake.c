// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// [=]===^=[ base setup ]============================================================^===[=]

#define BUFFER_WIDTH  (346 << 0)
#define BUFFER_HEIGHT (270 << 0)

#include "platform.c"

// [=]===^=[ remake stuff below ]============================================================^===[=]

#define MKS_RESAMPLER_IMPLEMENTATION
#include "resampler.h"

static struct rng_state rand_state;
static uint32_t active_demo_part;

static struct sample_state part1_sample;
static uint32_t p1_audio_state;
struct micromod_state part1_song;
struct micromod_state part2_song;
struct micromod_state part3_song;
struct micromod_state part4_song;

struct point {
	int32_t x, y;
};

#include "remake_loader.c"
#include "remake_part1.c"
#include "remake_part2.c"
#include "remake_part3.c"
#include "remake_part4.c"

INCBIN_BYTES(p1_fashion_sample, "data/p1_fashion-sample.raw");
INCBIN_BYTES(p1_fashinating, "data/p1_fashionating.mod");
INCBIN_BYTES(p2_the_world_of_the_dj, "data/p2_the world of the dj.mod");
INCBIN_BYTES(p3_parallax_ii, "data/p3_parallax ii.mod");
INCBIN_BYTES(p4_ivory_towers, "data/p4_ivorytowers.mod");


// [=]===^=[ process_sampled_audio ]============================================================^===[=]
struct sample_state {
	int16_t *data;
	uint32_t size;
	uint32_t position;
	uint32_t done;				// Used for one-shot playback
};

static void process_sampled_audio(int16_t *audio_buffer, size_t frames, struct sample_state *sample) {
	if(sample->done) return;
	for(size_t i = 0; i < frames; ++i) {
		int16_t processed_sample = (int16_t)(sample->data[sample->position] * 0.707f);
		*audio_buffer++ = processed_sample;
		*audio_buffer++ = processed_sample;
		if(++sample->position == sample->size) {
			sample->position = 0;
			sample->done = 1;
		}
	}
}

static void part_1_audio(int16_t *audio_buffer, size_t frames) {
	switch(p1_audio_state) {
		case 1: {
			process_sampled_audio(audio_buffer, frames, &part1_sample);
		} break;
		case 2: {
			micromod_get_audio(&part1_song, audio_buffer, frames);
		} break;
	}
}
static void part_2_audio(int16_t *audio_buffer, size_t frames) { micromod_get_audio(&part2_song, audio_buffer, frames); };
static void part_4_audio(int16_t *audio_buffer, size_t frames) { micromod_get_audio(&part4_song, audio_buffer, frames); };
static void part_3_audio(int16_t *audio_buffer, size_t frames) { micromod_get_audio(&part3_song, audio_buffer, frames); };

typedef uint32_t (*render_function)(struct platform_state *state);
typedef void (*audio_function)(int16_t *data, size_t frames);

struct callback {
	render_function render;
	audio_function audio;
};

struct callback update_callbacks[] = {
	{ loader, 0 },
	{ part_1_render, part_1_audio },
	{ loader, 0 },
	{ part_2_render, part_2_audio },
	{ loader, 0 },
	{ part_3_render, part_3_audio },
	{ loader, 0 },
	{ part_4_render, part_4_audio },
};

// [=]===^=[ audio_callback ]============================================================^===[=]
static void remake_audio_callback(int16_t *data, size_t frames) {
	// PROFILE_FUNCTION();
	memset(data, 0, 2*2*frames);
	if(update_callbacks[active_demo_part].audio) {
		update_callbacks[active_demo_part].audio(data, frames);
	}

	// NOTE(peter): Cross-over mixing ~75% from left in right, and ~75% from right in left
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

static void remake_options(struct options *opt) {
	opt->release_group = "FASHION";
	opt->release_title = "FASHIONATING";
	opt->window_title = "Fashion - Fashionating - 1988-05";
}

// [=]===^=[ remake_init ]============================================================^===[=]
static void remake_init(struct platform_state *state) {
	change_resolution(state, BUFFER_WIDTH, BUFFER_HEIGHT);

	xor_init_rng(&rand_state, 0x47189239);
	part_1_init();
	part_4_init();

	part1_sample.data = resample_audio((int8_t*)p1_fashion_sample, p1_fashion_sample_end - p1_fashion_sample, 267, &part1_sample.size);

	micromod_initialize(&part1_song, (int8_t*)p1_fashinating,			48000);
	micromod_initialize(&part2_song, (int8_t*)p2_the_world_of_the_dj,	48000);
	micromod_initialize(&part3_song, (int8_t*)p3_parallax_ii,			48000);
	micromod_initialize(&part4_song, (int8_t*)p4_ivory_towers,			48000);
	mkfw_audio_callback = remake_audio_callback;
}


// [=]===^=[ remake_frame ]============================================================^===[=]
static void remake_frame(struct platform_state *state) {
	// PROFILE_FUNCTION();

	if(update_callbacks[active_demo_part].render(state)) {
		active_demo_part = (active_demo_part < 7) ? active_demo_part + 1 : 0;
	}
}

// [=]===^=[ remake_shutdown ]============================================================^===[=]
static void remake_shutdown(struct platform_state *state) {
	free(part1_sample.data);
}

