// Copyright (c) 2025 Peter Fors
// SPDX-License-Identifier: MIT


#define MKFW_SAMPLE_RATE     48000
#define MKFW_NUM_CHANNELS    2
#define MKFW_BITS_PER_SAMPLE 16
#define MKFW_FRAME_SIZE      (MKFW_NUM_CHANNELS * (MKFW_BITS_PER_SAMPLE / 8))
#define MKFW_PREFERRED_FRAMES_PER_BUFFER 256
#define MKFW_BUFFER_SIZE     (MKFW_PREFERRED_FRAMES_PER_BUFFER * MKFW_FRAME_SIZE)
#define MKFW_BUFFER_COUNT    2

void (*mkfw_audio_callback)(int16_t *audio_buffer, size_t frames);

// Single-pole IIR low-pass filter (RC filter)
// alpha = dt / (rc + dt), where dt = 1/sample_rate, rc = 1/(2*pi*cutoff_hz)
// At 48kHz with ~8kHz cutoff: alpha ≈ 0.7265
#define MKFW_LP_ALPHA 0.7265f

static float mkfw_lp_prev_l = 0.0f;
static float mkfw_lp_prev_r = 0.0f;

static void mkfw_audio_callback_thread(int16_t *audio_buffer, size_t frames) {
	memset(audio_buffer, 0, frames * MKFW_NUM_CHANNELS * 2);
	if(mkfw_audio_callback) {
		mkfw_audio_callback(audio_buffer, frames);
	}

	for(size_t i = 0; i < frames; ++i) {
		float l = (float)audio_buffer[i * 2 + 0];
		float r = (float)audio_buffer[i * 2 + 1];
		mkfw_lp_prev_l += MKFW_LP_ALPHA * (l - mkfw_lp_prev_l);
		mkfw_lp_prev_r += MKFW_LP_ALPHA * (r - mkfw_lp_prev_r);
		audio_buffer[i * 2 + 0] = (int16_t)mkfw_lp_prev_l;
		audio_buffer[i * 2 + 1] = (int16_t)mkfw_lp_prev_r;
	}
}

#include <alsa/asoundlib.h>

static snd_pcm_t *mkfw_pcm;
static mkfw_thread mkfw_audio_thread;
static int16_t *mkfw_audio_buffer;
static snd_pcm_uframes_t mkfw_frames_per_period;

static MKFW_THREAD_FUNC(mkfw_audio_thread_func, arg) {
	while(1) {
		int32_t err = snd_pcm_wait(mkfw_pcm, -1);
		if(err < 0) {
			snd_pcm_recover(mkfw_pcm, err, 0);
			continue;
		}

		mkfw_audio_callback_thread(mkfw_audio_buffer, mkfw_frames_per_period);
		err = snd_pcm_writei(mkfw_pcm, mkfw_audio_buffer, mkfw_frames_per_period);
		if(err < 0) {
			snd_pcm_recover(mkfw_pcm, err, 0);
		}
		pthread_testcancel();
	}
	return 0;
}

static int32_t mkfw_set_hw_params(snd_pcm_t *handle) {
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t period = MKFW_PREFERRED_FRAMES_PER_BUFFER;
	snd_pcm_uframes_t buffer = period * MKFW_BUFFER_COUNT;
	uint32_t rate = MKFW_SAMPLE_RATE;
	int32_t err;
	int32_t dir;

	snd_pcm_hw_params_alloca(&params);
	if((err = snd_pcm_hw_params_any(handle, params)) < 0) {
		return err;
	}
	if((err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		return err;
	}
	if((err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
		return err;
	}
	if((err = snd_pcm_hw_params_set_channels(handle, params, MKFW_NUM_CHANNELS)) < 0) {
		return err;
	}
	if((err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0)) < 0) {
		return err;
	}
	if((err = snd_pcm_hw_params_set_period_size_near(handle, params, &period, 0)) < 0) {
		return err;
	}
	if((err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer)) < 0) {
		return err;
	}
	if((err = snd_pcm_hw_params(handle, params)) < 0) {
		return err;
	}

	snd_pcm_hw_params_get_period_size(params, &mkfw_frames_per_period, &dir);

	return 0;
}

static void mkfw_audio_initialize(void) {
	if(snd_pcm_open(&mkfw_pcm, "plug:pipewire", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		if(snd_pcm_open(&mkfw_pcm, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
			return;
		}
	}
	if(mkfw_set_hw_params(mkfw_pcm) < 0) {
		return;
	}

	mkfw_audio_buffer = calloc(mkfw_frames_per_period * MKFW_NUM_CHANNELS, sizeof(int16_t));

	if(snd_pcm_start(mkfw_pcm) < 0) {
		return;
	}
	mkfw_audio_thread = mkfw_thread_create(mkfw_audio_thread_func, 0);
}

static void mkfw_audio_shutdown(void) {
	mkfw_thread_cancel(mkfw_audio_thread);
	mkfw_thread_join(mkfw_audio_thread);
	snd_pcm_drop(mkfw_pcm);
	snd_pcm_close(mkfw_pcm);
	free(mkfw_audio_buffer);
}

