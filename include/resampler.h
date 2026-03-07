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

	float lp_cutoff = input_sample_rate / 2.0f;
	float lp_rc = 1.0f / (2.0f * 3.14159265f * lp_cutoff);
	float lp_dt = 1.0f / input_sample_rate;
	float lp_alpha = lp_dt / (lp_rc + lp_dt);

	float *filtered = malloc(input_size * sizeof(float));
	float prev = (float)input_audio[0] / 128.0f;
	filtered[0] = prev;
	for(size_t i = 1; i < input_size; ++i) {
		float s = (float)input_audio[i] / 128.0f;
		prev += lp_alpha * (s - prev);
		filtered[i] = prev;
	}

	int16_t *output_audio = malloc(output_size * sizeof(int16_t));

	for(size_t i = 0; i < output_size; ++i) {
		float index = (float)i / ratio;
		uint32_t idx_floor = (uint32_t)index;
		uint32_t idx_ceil = (idx_floor + 1) % input_size;

		float frac = index - idx_floor;
		float interpolated_sample = (1.0f - frac) * filtered[idx_floor] + frac * filtered[idx_ceil];

		output_audio[i] = (int16_t)(interpolated_sample * 32767.0f);
	}

	free(filtered);
	return output_audio;
}


#endif /* MKS_RESAMPLER_IMPLEMENTATION */

