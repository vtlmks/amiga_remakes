// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

// [=]===^=[ base setup ]============================================================^===[=]

#define BUFFER_WIDTH  (336 << 0)
#define BUFFER_HEIGHT (272 << 0)

#include "platform.c"

// [=]===^=[ remake stuff below ]============================================================^===[=]

#define MKS_RESAMPLER_IMPLEMENTATION
#include "resampler.h"

INCBIN_UGG(part1_small_font_data, "data/p1_small_font.ugg");
INCBIN_UGG(part2_ns_logo_data, "data/p2_ns_logo.ugg");
INCBIN_UGG(part2_text_data, "data/p2_text.ugg");
INCBIN_UGG(part4_atom_logo_bg_data, "data/p4_atom_logo_bg.ugg");

INCBIN_BYTES(part1_audio8_data, "data/p1_audio8.raw");
INCBIN_BYTES(part3_audio8_data, "data/p3_audio8.raw");
INCBIN_BYTES(part7_audio8_data, "data/p7_audio8.raw");
INCBIN_BYTES(part2_n_s_beat_data, "data/p2_n.s_beat.mod");
INCBIN_BYTES(part4_n_s_quiz_data, "data/p4_n.s_quiz.mod");
INCBIN_BYTES(part5_n_s_speedo_data, "data/p5_n.s_speedo.mod");
INCBIN_BYTES(part6_tune13_data, "data/p6_tune13.mod");
INCBIN_BYTES(part8_n_s_dreamer_data, "data/p8_n.s_dreamer.mod");


struct rng_state base_rand;

static uint32_t active_demo_part;

static struct sample_state part1_sample;
static struct sample_state part3_sample;
static struct sample_state part7_sample;

struct micromod_state part2_song;
struct micromod_state part4_song;
struct micromod_state part5_song;
struct micromod_state part6_song;
struct micromod_state part8_song;

#include "remake_part1.c"
#include "remake_part2.c"
#include "remake_part3.c"
#include "remake_part4.c"
#include "remake_part5.c"
#include "remake_part6.c"
#include "remake_part7.c"
#include "remake_part8.c"

// [=]===^=[ process_sampled_audio ]============================================================^===[=]
struct sample_state {
	int16_t *data;
	uint32_t size;
	uint32_t position;
	uint32_t done;				// Used for one-shot playback
};

static void process_sampled_audio(int16_t *audio_buffer, size_t frames, struct sample_state *sample) {
	for(size_t i = 0; i < frames; ++i) {
		int16_t processed_sample = (int16_t)(sample->data[sample->position] * 0.707f);
		*audio_buffer++ = processed_sample;
		*audio_buffer++ = processed_sample;
		if(++sample->position == sample->size) {
			sample->position = 0;
		}
	}
}

static void p1_audio(int16_t *audio_buffer, size_t frames) { process_sampled_audio(audio_buffer, frames, &part1_sample); }
static void p2_audio(int16_t *audio_buffer, size_t frames) { micromod_get_audio(&part2_song, audio_buffer, frames); };
static void p3_audio(int16_t *audio_buffer, size_t frames) { process_sampled_audio(audio_buffer, frames, &part3_sample); }
static void p4_audio(int16_t *audio_buffer, size_t frames) { micromod_get_audio(&part4_song, audio_buffer, frames); };
static void p5_audio(int16_t *audio_buffer, size_t frames) { micromod_get_audio(&part5_song, audio_buffer, frames); };
static void p6_audio(int16_t *audio_buffer, size_t frames) { micromod_get_audio(&part6_song, audio_buffer, frames); };
static void p7_audio(int16_t *audio_buffer, size_t frames) { process_sampled_audio(audio_buffer, frames, &part7_sample); }
static void p8_audio(int16_t *audio_buffer, size_t frames) { micromod_get_audio(&part8_song, audio_buffer, frames); };


typedef void (*audio_function)(int16_t *audio_buffer, size_t frames);
audio_function audio_callbacks[] = {
	p1_audio, p2_audio, p3_audio, p4_audio, p5_audio, p6_audio, p7_audio, p8_audio,
};

// [=]===^=[ audio_callback ]============================================================^===[=]
static void remake_audio_callback(int16_t *data, size_t frames) {
	// PROFILE_FUNCTION();
	memset(data, 0, 2*2*frames);
	audio_callbacks[active_demo_part](data, frames);
	// micromod_get_audio(&part2_song, (short*)data, frames);
	// fc14play_FillAudioBuffer(data, frames);


	// NOTE(peter): Enable for 75% mix if the player doesn't have that functionality!
#if 1
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
static void remake_init(struct platform_state *state) {
	change_resolution(state, BUFFER_WIDTH, BUFFER_HEIGHT);
//	set_window_title(remake_title);
	xor_init_rng(&base_rand, 187481201);

	p1_init(state);
	p2_init(state);
	p4_init();
	p5_init();
	p6_init();
	p7_init();
	p8_init();

	part1_sample.data = resample_audio((int8_t*)part1_audio8_data, part1_audio8_data_end - part1_audio8_data, 416, &part1_sample.size);
	part3_sample.data = resample_audio((int8_t*)part3_audio8_data, part3_audio8_data_end - part3_audio8_data, 428, &part3_sample.size);
	part7_sample.data = resample_audio((int8_t*)part7_audio8_data, part7_audio8_data_end - part7_audio8_data, 428, &part7_sample.size);

	micromod_initialize(&part2_song, (int8_t*)part2_n_s_beat_data,		48000);
	micromod_initialize(&part4_song, (int8_t*)part4_n_s_quiz_data,		48000);
	micromod_initialize(&part5_song, (int8_t*)part5_n_s_speedo_data,	48000);
	micromod_initialize(&part6_song, (int8_t*)part6_tune13_data,		48000);
	micromod_initialize(&part8_song, (int8_t*)part8_n_s_dreamer_data,	48000);

	mkfw_audio_callback = remake_audio_callback;
}

// [=]===^=[ remake_shutdown ]============================================================^===[=]
static void remake_shutdown(struct platform_state *state) {
	// fc14play_Close();
	p4_shutdown();
	p5_shutdown();
	p6_shutdown();
	p7_shutdown();
	p8_shutdown();

	free(part1_sample.data);
	free(part3_sample.data);
	free(part7_sample.data);

	mkfw_audio_callback = 0;

	// printf("\n=== CRT Parameters ===\n");
	// printf("state.bloom_threshold = %.3ff;\n", state.bloom_threshold);
	// printf("state.bloom_intensity = %.3ff;\n", state.bloom_intensity);
	// printf("state.persistence_decay = %.3ff;\n", state.persistence_decay);
	// printf("state.brightness = %.3ff;\n", state.brightness);
	// printf("state.bloom_scale = %.3ff;\n", state.bloom_scale);
	// printf("=====================\n");
}


typedef uint32_t (*update_function)(struct platform_state *state);
update_function update_callbacks[] = { p1_update, p2_update, p3_update, p4_update, p5_update, p6_update, p7_update, p8_update, };

void remake_options(struct options *opt) {
	opt->release_group = "NORTH STAR";
	opt->release_title = "MEGADEMO 1";
	opt->window_title = "North Star - Megademo - 1988-03-31";
}

// [=]===^=[ remake_frame ]============================================================^===[=]
static void remake_frame(struct platform_state *state) {
	// PROFILE_FUNCTION();

	// float step = keyboard_state[MKS_KEY_LSHIFT] ? 0.001f : 0.01f;

	// // Bloom Threshold (Q/A)
	// if(keyboard_state[MKS_KEY_Q]) state.bloom_threshold += step;
	// if(keyboard_state[MKS_KEY_A]) state.bloom_threshold -= step;

	// // Bloom Intensity (W/S)
	// if(keyboard_state[MKS_KEY_W]) state.bloom_intensity += step;
	// if(keyboard_state[MKS_KEY_S]) state.bloom_intensity -= step;

	// // Persistence Decay (E/D)
	// if(keyboard_state[MKS_KEY_E]) state.persistence_decay += step;
	// if(keyboard_state[MKS_KEY_D]) state.persistence_decay -= step;

	// // Brightness (R/F)
	// if(keyboard_state[MKS_KEY_R]) state.brightness += step;
	// if(keyboard_state[MKS_KEY_F]) state.brightness -= step;

	// // Clamp values
	// state.bloom_threshold = fminf(fmaxf(state.bloom_threshold, 0.0f), 1.0f);
	// state.bloom_intensity = fminf(fmaxf(state.bloom_intensity, 0.0f), 1.0f);
	// state.persistence_decay = fminf(fmaxf(state.persistence_decay, 0.0f), 0.99f);
	// state.brightness = fminf(fmaxf(state.brightness, 0.1f), 3.0f);

	for(uint32_t i = '1'; i <= '8'; ++i) {
		if(mkfw_is_key_pressed(state->window, i)) {
			active_demo_part = i - '1';
			break;
		}
	}

	if(mkfw_is_key_pressed(state->window, MKS_KEY_UP) || mkfw_is_key_pressed(state->window, MKS_KEY_LEFT)) {
		active_demo_part = (active_demo_part > 0) ? active_demo_part - 1 : 7;
	}

	if(mkfw_is_key_pressed(state->window, MKS_KEY_RIGHT) || mkfw_is_key_pressed(state->window, MKS_KEY_DOWN)) {
		active_demo_part = (active_demo_part < 7) ? active_demo_part + 1 : 0;
	}

	if(update_callbacks[active_demo_part](state)) {
		active_demo_part = (active_demo_part < 7) ? active_demo_part + 1 : 0;
	}
}
