// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

/* ===^=====================================================================================^===[=]
 *
 */

const char remake_name[] = "Alpha Flight - Dr.Mabuse first intro";

#include "graphics/background.h"
#include "graphics/testfont14.h"
#include "audio/music.h"
#include "resample.c"

struct remake_state {
	uint32_t *frame_buffer;			// The buffer is RGBA formatted!
	uint32_t frame_buffer_size;	// In bytes, for clearsize or whatnot
	uint32_t frame_buffer_width;
	uint32_t frame_buffer_height;
	uint32_t frame_number;			// Increased by one each frame.
	uint32_t ms_since_start;		// Enough for just below 50 days before wrapping
	int32_t mouse_x;					// Absolute position for mouse, restricted
	int32_t mouse_y;					//   inside 0,0 -> BUFFER_WIDTH, BUFFER_HEIGHT
	uint32_t mouse_buttons;			// Mouse buttons, left, middle and right. MACROS will be available.
};

#define _USE_MATH_DEFINES
#include <math.h>

uint8_t aud_channel_trigger[4];
uint8_t aud_note_trigger[37];
uint8_t aud_sample_trigger[32];

#define DEVICE_SAMPLE_RATE 48000
#define DEVICE_FORMAT ma_format_s16
#define DEVICE_CHANNELS 2
#include "miniaudio.h"

static uint32_t md1_p1_sample_count = 0;

static ma_context ma_ctx;
ma_device_config device_config;
ma_device device;

static bool led_filter_toggle = false;



uint32_t audio_sample_count;

// [=]===^=====================================================================================^===[=]
void audio_data_callback(ma_device *device, void *output, const void *input, ma_uint32 frame_count) {
	(void)input;
	(void)device;

	int16_t *dst = output;
	for(uint32_t i = 0; i < frame_count; ++i) {
		int16_t sample = (int16_t)(music_data[audio_sample_count]);
		*dst++ = sample;
		*dst++ = sample;
		audio_sample_count++;
		audio_sample_count %=  ARRAY_SIZE(music_data);
	}
}

/*
 * Here you should load/initialize images, music, convert data, whatever you need to do
 */
static void remake_init(struct remake_state *state) {
	// NOTE(peter): Audio setup, after ma_device_start(..) our audio_data_callback(..) will be active, make sure you have set it up!
	device_config = ma_device_config_init(ma_device_type_playback);
	device_config.playback.format = DEVICE_FORMAT;
	device_config.playback.channels = DEVICE_CHANNELS;
	device_config.sampleRate = DEVICE_SAMPLE_RATE;
	device_config.dataCallback = audio_data_callback;
	device_config.pUserData = state;					// NOTE(peter): If we need it from the callback, do: struct remake_state *state = (struct remake_state *)device->pUserData;
	ma_device_init(0, &device_config, &device);

	{
		uint32_t line = 5;		// first bar start at line 5 (it seem)
		uint32_t *dst = bars_on_top_of_eachother;
		for(uint32_t i = 0; i < 7; ++i) {
			for(uint32_t y = 0; y < 5; ++y) {
				dst[line++] = small_copper_bars[i][y];
			}
			line += 6;
		}
		// we will access 5 below last copperbar (I think)..
	}
	ma_device_start(&device);
}

/*
 * return 'true' if we are to quit
 */


static void remake_shutdown(struct remake_state *state) {
	ma_device_uninit(&device);			// PLEASE MAKE IT STOP!!111
}



static void blit(uint8_t *src, int32_t src_x, int32_t src_y, int32_t src_width, int32_t src_height,
					  uint8_t *dst, int32_t dst_x, int32_t dst_y, int32_t dst_width, int32_t dst_height) {

// TODO: copy from src to dst with clipping against all dst borders.
// TODO: If the clipping result is that there is no data to be copied, do an early return

}











