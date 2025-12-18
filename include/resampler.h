// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT

#pragma once

#include <stdlib.h>
#include <stdint.h>

static int16_t *resample_audio(int8_t *input_audio, size_t input_size, uint32_t period, uint32_t *outsize);

#ifdef MKS_RESAMPLER_IMPLEMENTATION

/*
 * NOTE(peter): To get the sample rate on the Amiga, use this equation:
 *              3546895 / PERiod  which you can find if you resource the
 *              demo you are remaking where they set the
 *              AUDxPER : DFF0A6 : DFF0B6 : DFF0C6 : DFF0D6
 */
/*
 * NOTE(peter): Memory returned should be freed by the caller!
 */
static int16_t *resample_audio(int8_t *input_audio, size_t input_size, uint32_t period, uint32_t *outsize) {
	float input_sample_rate = 3546895.0f / (float)period;
	float ratio = 48000.0f / input_sample_rate;
	size_t output_size = (size_t)(input_size * ratio + 0.5f);
	*outsize = output_size;

	int16_t *output_audio = malloc(output_size * sizeof(int16_t));

	for(size_t i = 0; i < output_size; ++i) {
		float index = (float)i / ratio;
		uint32_t idx_floor = (uint32_t)index;
		uint32_t idx_ceil = (idx_floor + 1) % input_size;

		float frac = index - idx_floor;
		float sample1 = (float)input_audio[idx_floor] / 128.0f;
		float sample2 = (float)input_audio[idx_ceil] / 128.0f;
		float interpolated_sample = (1.0f - frac) * sample1 + frac * sample2;

		output_audio[i] = (int16_t)(interpolated_sample * 32767.0f);
	}

	return output_audio;
}


#endif /* MKS_RESAMPLER_IMPLEMENTATION */

