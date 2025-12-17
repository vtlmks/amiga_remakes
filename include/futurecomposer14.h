/*
** fc14play v1.29 - 17th of February 2019 - https://16-bits.org
** ============================================================
** - NOT BIG ENDIAN SAFE! -
**
** Very accurate C port of Future Composer 1.4's replayer,
** by Olav "8bitbubsy" Sørensen, using a FC1.4 disassembly (its supplied replayer code was buggy).
** Works correctly with v1.0..v1.3 modules as well.
**
** The BLEP (Band-Limited Step) and filter routines were coded by aciddose.
** This makes the replayer sound much similar to a real Amiga.
**
** Converted to state-based approach (similar to micromod.h) for multiple song support.
** All functions prefixed with fc14_ to avoid conflicts with other players.
*/

// NOTE(peter): This is NOT the original, it's modified to use a state and some things has been changed over time!

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _MSC_VER
#define inline __forceinline
#endif

// == USER ADJUSTABLE SETTINGS ==
#define FC14_STEREO_SEP (20)		/* --> Stereo separation in percent - 0 = mono, 100 = hard pan (like Amiga) */
// #define FC14_USE_HIGHPASS		/* --> ~5Hz HP filter present in all Amigas - comment out for a tiny speed-up */
// #define FC14_USE_LOWPASS		/* --> ~5kHz LP filter present in all Amigas except A1200 - comment out for sharper sound (and tiny speed-up) */
// #define FC14_USE_BLEP			/* --> Reduces some aliasing in the sound (closer to real Amiga) - comment out for a speed-up */
#define FC14_MIX_BUF_SAMPLES 4096

// main crystal oscillator
#define FC14_AMIGA_PAL_XTAL_HZ 28375160

#define FC14_PAULA_PAL_CLK (FC14_AMIGA_PAL_XTAL_HZ / 8)
#define FC14_CIA_PAL_CLK (FC14_AMIGA_PAL_XTAL_HZ / 40)
#define FC14_AMIGA_PAL_VBLANK_HZ 50

#define FC14_AMIGA_VOICES 4

#define FC14_INITIAL_DITHER_SEED 0x12345000
#define FC14_DENORMAL_OFFSET 1e-10f

#define FC14_SEQ_SIZE 13
#define FC14_PAT_END_MARKER 0x49
#define FC14_NUM_SAMPLES 10
#define FC14_NUM_WAVEFORMS 80
#define FC14_NUM_WAVEFORMS_SMOD 47

#define FC14_BLEP_ZC 8
#define FC14_BLEP_OS 5
#define FC14_BLEP_SP 5
#define FC14_BLEP_NS (FC14_BLEP_ZC * FC14_BLEP_OS / FC14_BLEP_SP)
#define FC14_BLEP_RNS 7 // RNS = (2^ > NS) - 1

#ifdef FC14_USE_BLEP
struct fc14_blep_t {
	int32_t index;
	int32_t samplesLeft;
	float dBuffer[FC14_BLEP_RNS + 1];
	float dLastValue;
};
#endif

struct fc14_paulaVoice_t {
	volatile uint8_t active;
	const int8_t *data;
	const int8_t *newData;
	int32_t length;
	int32_t newLength;
	int32_t pos;
	float dVolume;
	float dDelta;
	float dPhase;
	float dPanL;
	float dPanR;
#ifdef FC14_USE_BLEP
	float dDeltaMul;
	float dLastDelta;
	float dLastPhase;
	float dLastDeltaMul;
#endif
};

#if defined(FC14_USE_HIGHPASS) || defined(FC14_USE_LOWPASS)
struct fc14_lossyIntegrator_t {
	float dBuffer[2];
	float b0;
	float b1;
};
#endif

struct fc14_soundInfo_t {
	int8_t *data;
	int8_t *repeat;
	uint16_t length;
	uint16_t replen;
};

struct fc14_fcChannel_t {
	uint8_t vibratoUp;
	uint8_t portaDelay;
	uint8_t pitchBendDelay;
	uint8_t volSlideDelay;
	int8_t pitchBendValue;
	int8_t pitchBendCounter;
	int8_t note;
	int8_t noteTranspose;
	int8_t soundTranspose;
	int8_t *loopStart;
	int8_t volume;
	int8_t periodTranspose;
	const uint8_t *freqTabPtr;
	const uint8_t *volTabPtr;
	uint8_t voiceIndex;
	uint8_t *seqStartPtr;
	uint8_t *patPtr;
	uint8_t freqSusCounter;
	uint8_t volSusCounter;
	uint8_t vibratoSpeed;
	uint8_t vibratoDepth;
	uint8_t vibratoCounter;
	uint8_t vibratoDelay;
	uint8_t volSlideSpeed;
	uint8_t volSlideCounter;
	uint8_t portaParam;
	uint8_t volDelayCounter;
	uint8_t volDelayLength;
	int16_t portaValue;
	uint16_t loopLength;
	uint16_t freqTabPos;
	uint16_t volTabPos;
	uint16_t patPos;
	uint32_t seqPos;
};

struct fc14_state {
	volatile uint8_t musicPaused;
	uint8_t fc14;
	int8_t *ptr8s_1;
	uint8_t *songData;
	uint8_t	*ptr8u_1;
	uint8_t *ptr8u_2;
	uint8_t spdtemp;
	uint8_t spdtemp2;
	uint8_t respcnt;
	uint8_t repspd;
	uint8_t *SEQpoint;
	uint8_t	*PATpoint;
	uint8_t *FRQpoint;
	uint8_t *VOLpoint;
	uint8_t stereoSep;
	uint16_t oldPeriod;
	int32_t soundBufferSize;
	int32_t samplesPerFrameLeft;
	int32_t samplesPerFrame;
	int32_t randSeed;
	int32_t masterVol;
	uint32_t audioRate;
	uint32_t numSequences;
	uint32_t sampleCounter;
	float oldVoiceDelta;
	float *dMixerBufferL;
	float *dMixerBufferR;
	float dAudioRate;
	float dPeriodToDeltaDiv;
	float dPrngStateL;
	float dPrngStateR;
	struct fc14_paulaVoice_t paula[FC14_AMIGA_VOICES];
	struct fc14_fcChannel_t Channel[FC14_AMIGA_VOICES];
	struct fc14_soundInfo_t samples[FC14_NUM_SAMPLES + FC14_NUM_WAVEFORMS];
#ifdef FC14_USE_BLEP
	float dOldVoiceDeltaMul;
	struct fc14_blep_t blep[FC14_AMIGA_VOICES];
	struct fc14_blep_t blepVol[FC14_AMIGA_VOICES];
#endif
#ifdef FC14_USE_HIGHPASS
	struct fc14_lossyIntegrator_t filterHi;
#endif
#ifdef FC14_USE_LOWPASS
	struct fc14_lossyIntegrator_t filterLo;
#endif
};

#define FC14_LERP(x, y, z) ((x) + ((y) - (x)) * (z))
#define FC14_CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define FC14_CLAMP16(i) if ((int16_t)(i) != i) i = 0x7FFF ^ (i >> 31);
#define FC14_PTR2LONG(x) ((uint32_t *)(x))
#define FC14_PTR2WORD(x) ((uint16_t *)(x))
#define FC14_SSMP_MAGIC 0x504D5353u

static const uint8_t fc14_silentTable[8] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE1 };

static const uint16_t fc14_periods[128] = {
	// 1.0..1.3 periods
	0x06b0, 0x0650, 0x05f4, 0x05a0, 0x054c, 0x0500, 0x04b8, 0x0474, 0x0434, 0x03f8, 0x03c0, 0x038a,
	0x0358, 0x0328, 0x02fa, 0x02d0, 0x02a6, 0x0280, 0x025c, 0x023a, 0x021a, 0x01fc, 0x01e0, 0x01c5,
	0x01ac, 0x0194, 0x017d, 0x0168, 0x0153, 0x0140, 0x012e, 0x011d, 0x010d, 0x00fe, 0x00f0, 0x00e2,
	0x00d6, 0x00ca, 0x00be, 0x00b4, 0x00aa, 0x00a0, 0x0097, 0x008f, 0x0087, 0x007f, 0x0078, 0x0071,
	0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071,

	// 1.4 periods (one extra octave)
	0x0d60, 0x0ca0, 0x0be8, 0x0b40, 0x0a98, 0x0a00, 0x0970, 0x08e8, 0x0868, 0x07f0, 0x0780, 0x0714,
	0x06b0, 0x0650, 0x05f4, 0x05a0, 0x054c, 0x0500, 0x04b8, 0x0474, 0x0434, 0x03f8, 0x03c0, 0x038a,
	0x0358, 0x0328, 0x02fa, 0x02d0, 0x02a6, 0x0280, 0x025c, 0x023a, 0x021a, 0x01fc, 0x01e0, 0x01c5,
	0x01ac, 0x0194, 0x017d, 0x0168, 0x0153, 0x0140, 0x012e, 0x011d, 0x010d, 0x00fe, 0x00f0, 0x00e2,
	0x00d6, 0x00ca, 0x00be, 0x00b4, 0x00aa, 0x00a0, 0x0097, 0x008f, 0x0087, 0x007f, 0x0078, 0x0071,
	0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071, 0x0071
};

static const int8_t fc14_waveformDatas[1344] = {
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0x3f, 0x37, 0x2f, 0x27, 0x1f, 0x17, 0x0f, 0x07, 0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0x37, 0x2f, 0x27, 0x1f, 0x17, 0x0f, 0x07, 0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0x2f, 0x27, 0x1f, 0x17, 0x0f, 0x07, 0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0x27, 0x1f, 0x17, 0x0f, 0x07, 0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0x1f, 0x17, 0x0f, 0x07, 0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x17, 0x0f, 0x07, 0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x0f, 0x07, 0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x07, 0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0x80, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0xa0, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0x37,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8, 0xc0, 0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x7f,
	0x80, 0x80, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
	0x45, 0x45, 0x79, 0x7d, 0x7a, 0x77, 0x70, 0x66, 0x61, 0x58, 0x53, 0x4d, 0x2c, 0x20, 0x18, 0x12,
	0x04, 0xdb, 0xd3, 0xcd, 0xc6, 0xbc, 0xb5, 0xae, 0xa8, 0xa3, 0x9d, 0x99, 0x93, 0x8e, 0x8b, 0x8a,
	0x45, 0x45, 0x79, 0x7d, 0x7a, 0x77, 0x70, 0x66, 0x5b, 0x4b, 0x43, 0x37, 0x2c, 0x20, 0x18, 0x12,
	0x04, 0xf8, 0xe8, 0xdb, 0xcf, 0xc6, 0xbe, 0xb0, 0xa8, 0xa4, 0x9e, 0x9a, 0x95, 0x94, 0x8d, 0x83,
	0x00, 0x00, 0x40, 0x60, 0x7f, 0x60, 0x40, 0x20, 0x00, 0xe0, 0xc0, 0xa0, 0x80, 0xa0, 0xc0, 0xe0,
	0x00, 0x00, 0x40, 0x60, 0x7f, 0x60, 0x40, 0x20, 0x00, 0xe0, 0xc0, 0xa0, 0x80, 0xa0, 0xc0, 0xe0,
	0x80, 0x80, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8, 0xc0, 0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x7f,
	0x80, 0x80, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
};

#ifdef FC14_USE_BLEP

/* Why this table is not represented as readable float (double) numbers:
** Accurate float (double) representation in string format requires at least 14 digits and normalized
** (scientific) notation, notwithstanding compiler issues with precision or rounding error.
** Also, don't touch this table ever, just keep it exactly identical! */

static const uint32_t fc14_dBlepData[48] = {
	0x3F7FFC3E, 0x3F7FFAA9, 0x3F7FFAD5, 0x3F7FFA9C,
	0x3F7FB5B1, 0x3F7F842A, 0x3F7FB7F6, 0x3F7F599C,
	0x3F7EA5E4, 0x3F7D6E71, 0x3F7B7F79, 0x3F78AB9E,
	0x3F74DCA5, 0x3F702519, 0x3F7598FB, 0x3F753D0E,
	0x3F7383A5, 0xBF3C977D, 0xBF3755C1, 0xBF391BDB,
	0xBF3455AF, 0xBF364613, 0xBF3056C4, 0x3F310282,
	0x3F3B5B7E, 0x3F3C5904, 0x3F355404, 0x3F33CED3,
	0xBF3822DB, 0xBF32805D, 0xBF37140D, 0xBF318A77,
	0xBF37FF72, 0x3F38CBFA, 0x3F3D4AEC, 0x3F314A3B,
	0x3F3D5C5B, 0x3F32558B, 0x3F3C997F, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};
#endif

static const uint8_t fc14_waveformLengths[47] = {
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x10, 0x08, 0x10, 0x10, 0x08, 0x08, 0x18
};

// CODE START

static void fc14_paulaStopDMA(struct fc14_state *fc, uint8_t i) {
	fc->paula[i].active = 0;
}

static void fc14_paulaStartDMA(struct fc14_state *fc, uint8_t i) {
	struct fc14_paulaVoice_t *v = &fc->paula[i];

	v->dPhase = 0.0f;
	v->pos = 0;
	v->data = v->newData;
	v->length = v->newLength;
	v->active = 1;
}

static void fc14_paulaSetPeriod(struct fc14_state *fc, uint8_t i, uint16_t period) {
	struct fc14_paulaVoice_t *v = &fc->paula[i];

	if(period == 0) {
		v->dDelta = 0.0f;
#ifdef FC14_USE_BLEP
		v->dDeltaMul = 1.0f;
#endif
		return;
	}

	if(period < 113)
		period = 113;

	if(period == fc->oldPeriod) {
		v->dDelta = fc->oldVoiceDelta;
#ifdef FC14_USE_BLEP
		v->dDeltaMul = fc->dOldVoiceDeltaMul;
#endif
	} else {
		fc->oldPeriod = period;
		v->dDelta = fc->dPeriodToDeltaDiv / (float)period;
		fc->oldVoiceDelta = v->dDelta;

#ifdef FC14_USE_BLEP
		v->dDeltaMul = 1.0f / v->dDelta;
		fc->dOldVoiceDeltaMul = v->dDeltaMul;
#endif
	}

#ifdef FC14_USE_BLEP
	if(v->dLastDelta == 0.0f)
		v->dLastDelta = v->dDelta;

	if(v->dLastDeltaMul == 0.0f)
		v->dLastDeltaMul = v->dDeltaMul;
#endif
}

static void fc14_paulaSetVolume(struct fc14_state *fc, uint8_t i, uint16_t vol) {
	vol &= 127;
	if(vol > 64)
		vol = 64;

	fc->paula[i].dVolume = (float)vol * (1.0f / 64.0f);
}

static void fc14_paulaSetLength(struct fc14_state *fc, uint8_t i, uint16_t len) {
	fc->paula[i].newLength = len * 2;
}

static void fc14_paulaSetData(struct fc14_state *fc, uint8_t i, const int8_t *src) {
	fc->paula[i].newData = src;
}

#if defined(FC14_USE_HIGHPASS) || defined(FC14_USE_LOWPASS)
static void fc14_calcCoeffLossyIntegrator(float dSr, float dHz, struct fc14_lossyIntegrator_t *filter) {
	float dOmega = ((2.0f * M_PI) * dHz) / dSr;
	filter->b0 = 1.0f / (1.0f + (1.0f / dOmega));
	filter->b1 = 1.0f - filter->b0;
}

static void fc14_clearLossyIntegrator(struct fc14_lossyIntegrator_t *filter) {
	filter->dBuffer[0] = 0.0f;
	filter->dBuffer[1] = 0.0f;
}

static inline void fc14_lossyIntegrator(struct fc14_lossyIntegrator_t *filter, float *dIn, float *dOut) {
	filter->dBuffer[0] = (filter->b0 * dIn[0]) + (filter->b1 * filter->dBuffer[0]) + FC14_DENORMAL_OFFSET;
	dOut[0] = filter->dBuffer[0];

	filter->dBuffer[1] = (filter->b0 * dIn[1]) + (filter->b1 * filter->dBuffer[1]) + FC14_DENORMAL_OFFSET;
	dOut[1] = filter->dBuffer[1];
}

static inline void fc14_lossyIntegratorHighPass(struct fc14_lossyIntegrator_t *filter, float *dIn, float *dOut) {
	float dLow[2];

	fc14_lossyIntegrator(filter, dIn, dLow);

	dOut[0] = dIn[0] - dLow[0];
	dOut[1] = dIn[1] - dLow[1];
}
#endif

#ifdef FC14_USE_BLEP
static float fc14_union_u64_f32(uint32_t u) {
	union {
		uint32_t u;
		float f;
	} temp;
	temp.u = u;
	return temp.f;
}

void fc14_blepAdd(struct fc14_blep_t *b, float dOffset, float dAmplitude) {
	int8_t n;
	int32_t i;
	const uint32_t *dBlepSrc;
	float f;

	f = dOffset * (float)FC14_BLEP_SP;

	i = (int32_t)f;
	dBlepSrc = fc14_dBlepData + i + FC14_BLEP_OS;
	f -= (float)i;

	i = b->index;

	n = FC14_BLEP_NS;
	while(n--) {
		b->dBuffer[i] += (dAmplitude * FC14_LERP(fc14_union_u64_f32(dBlepSrc[0]), fc14_union_u64_f32(dBlepSrc[1]), f));
		i = (i + 1) & FC14_BLEP_RNS;
		dBlepSrc += FC14_BLEP_SP;
	}

	b->samplesLeft = FC14_BLEP_NS;
}

void fc14_blepVolAdd(struct fc14_blep_t *b, float dAmplitude) {
	int8_t n;
	int32_t i;
	const uint32_t *dBlepSrc;

	dBlepSrc = fc14_dBlepData + FC14_BLEP_OS;

	i = b->index;

	n = FC14_BLEP_NS;
	while(n--) {
		b->dBuffer[i] += dAmplitude * fc14_union_u64_f32(*dBlepSrc);
		i = (i + 1) & FC14_BLEP_RNS;
		dBlepSrc += FC14_BLEP_SP;
	}

	b->samplesLeft = FC14_BLEP_NS;
}

float fc14_blepRun(struct fc14_blep_t *b) {
	float fBlepOutput;

	fBlepOutput = b->dBuffer[b->index];
	b->dBuffer[b->index] = 0.0f;

	b->index = (b->index + 1) & FC14_BLEP_RNS;

	b->samplesLeft--;
	return fBlepOutput;
}
#endif

static uint8_t fc14_init_music(struct fc14_state *fc, const uint8_t *moduleData) {
	uint8_t i;
	struct fc14_soundInfo_t *s;

	fc->fc14 = (*FC14_PTR2LONG(&moduleData[0]) == 0x34314346);

	if(*FC14_PTR2LONG(&moduleData[0]) != 0x444F4D53 && !fc->fc14)
		return 0;

	fc->SEQpoint = (uint8_t *)&moduleData[fc->fc14 ? 180 : 100];
	fc->PATpoint = (uint8_t *)&moduleData[__builtin_bswap32(*FC14_PTR2LONG(&moduleData[8]))];
	fc->FRQpoint = (uint8_t *)&moduleData[__builtin_bswap32(*FC14_PTR2LONG(&moduleData[16]))];
	fc->VOLpoint = (uint8_t *)&moduleData[__builtin_bswap32(*FC14_PTR2LONG(&moduleData[24]))];

	fc->ptr8s_1 = (int8_t *)&moduleData[__builtin_bswap32(*FC14_PTR2LONG(&moduleData[32]))];
	fc->ptr8u_2 = (uint8_t *)&moduleData[40];

	for(i = 0; i < FC14_NUM_SAMPLES; i++) {
		fc->samples[i].data = fc->ptr8s_1;
		fc->samples[i].length = __builtin_bswap16(*FC14_PTR2WORD(fc->ptr8u_2)); fc->ptr8u_2 += 2;
		fc->samples[i].repeat = &fc->samples[i].data[__builtin_bswap16(*FC14_PTR2WORD(fc->ptr8u_2))]; fc->ptr8u_2 += 2;
		fc->samples[i].replen = __builtin_bswap16(*FC14_PTR2WORD(fc->ptr8u_2)); fc->ptr8u_2 += 2;

		if(fc->samples[i].replen <= 1) {
			fc->samples[i].replen = 1;
			if(fc->samples[i].length >= 1) {
				if(*FC14_PTR2LONG((uint8_t*)fc->samples[i].data) != FC14_SSMP_MAGIC)
					*FC14_PTR2WORD(fc->samples[i].data) = 0;
			}
		}

		fc->ptr8s_1 += (fc->samples[i].length * 2) + 2;
	}

	if(fc->fc14) {
		fc->ptr8s_1 = (int8_t *)&moduleData[__builtin_bswap32(*FC14_PTR2LONG(&moduleData[36]))];
		fc->ptr8u_2 = (uint8_t *)&moduleData[100];

		for(i = 0; i < FC14_NUM_WAVEFORMS; i++) {
			s = &fc->samples[FC14_NUM_SAMPLES + i];

			s->data = fc->ptr8s_1;
			s->length = *fc->ptr8u_2++;
			s->repeat = fc->ptr8s_1;
			s->replen = s->length;

			fc->ptr8s_1 += s->length * 2;
		}
	} else {
		fc->ptr8s_1 = (int8_t *)fc14_waveformDatas;
		for(i = 0; i < FC14_NUM_WAVEFORMS; i++) {
			s = &fc->samples[FC14_NUM_SAMPLES + i];

			if(i < FC14_NUM_WAVEFORMS_SMOD) {
				s->data = fc->ptr8s_1;
				s->length = fc14_waveformLengths[i];
				s->repeat = s->data;
				s->replen = s->length;

				fc->ptr8s_1 += s->length * 2;
			} else {
				s->data = 0;
				s->length = 0;
				s->repeat = 0;
				s->replen = 1;
			}
		}
	}

	fc->numSequences = (uint32_t)(__builtin_bswap32(*FC14_PTR2LONG(&moduleData[4])) / FC14_SEQ_SIZE) * FC14_SEQ_SIZE;

	return 1;
}

static void fc14_restart_song(struct fc14_state *fc) {
	struct fc14_fcChannel_t *ch;

	memset(fc->Channel, 0, sizeof(fc->Channel));
	for(uint8_t i = 0; i < FC14_AMIGA_VOICES; i++) {
		ch = &fc->Channel[i];

		ch->voiceIndex = i;
		ch->volTabPtr = fc14_silentTable;
		ch->freqTabPtr = fc14_silentTable;
		ch->volDelayCounter = 1;
		ch->volDelayLength = 1;
		ch->pitchBendDelay = 1;
		ch->seqPos = FC14_SEQ_SIZE;
		ch->seqStartPtr = &fc->SEQpoint[3 * i];
		ch->patPtr = &fc->PATpoint[ch->seqStartPtr[0] << 6];
		ch->noteTranspose = (int8_t)ch->seqStartPtr[1];
		ch->soundTranspose = (int8_t)ch->seqStartPtr[2];
	}

	fc->repspd = (fc->SEQpoint[12] > 0) ? fc->SEQpoint[12] : 3;
	fc->respcnt = fc->repspd;
	fc->spdtemp = 0;
	fc->spdtemp2 = 0;
}

static void fc14_new_note(struct fc14_state *fc, struct fc14_fcChannel_t *ch) {
	uint8_t *tmpSeqPtr, *tmpPatPtr, note, info;

	tmpPatPtr = &ch->patPtr[ch->patPos];

	if((fc->fc14 && (*tmpPatPtr & 0x7F) == FC14_PAT_END_MARKER) || ch->patPos == 64) {
		ch->patPos = 0;
		if(ch->seqPos >= fc->numSequences)
			ch->seqPos = 0;

		tmpSeqPtr = &ch->seqStartPtr[ch->seqPos];
		ch->patPtr = &fc->PATpoint[tmpSeqPtr[0] << 6];

		ch->noteTranspose = (int8_t)tmpSeqPtr[1];
		ch->soundTranspose = (int8_t)tmpSeqPtr[2];

		if(++fc->spdtemp == 4) {
			fc->spdtemp = 0;

			if(++fc->spdtemp2 == fc->numSequences / FC14_SEQ_SIZE)
				fc->spdtemp2 = 0;
		}

		if(fc->SEQpoint[(fc->spdtemp2 * 13) + 12] != 0) {
			fc->repspd = fc->SEQpoint[(fc->spdtemp2 * 13) + 12];
			fc->respcnt = fc->repspd;
		}

		ch->seqPos += FC14_SEQ_SIZE;
		tmpPatPtr = ch->patPtr;
	}

	note = tmpPatPtr[0];
	info = tmpPatPtr[1];

	if(note == 0) {
		info &= 0xC0;
		if(info != 0) {
			ch->portaParam = 0;
			if(info & (1 << 7))
				ch->portaParam = tmpPatPtr[3];
		}
	} else {
		ch->portaValue = 0;
		ch->portaParam = 0;

		if(info & (1 << 7))
			ch->portaParam = tmpPatPtr[3];
	}

	note &= 0x7F;
	if(note != 0) {
		fc->ptr8u_1 = &fc->VOLpoint[((tmpPatPtr[1] & 0x3F) + ch->soundTranspose) << 6];

		ch->note = note;
		ch->volDelayLength = fc->ptr8u_1[0];
		ch->volDelayCounter = ch->volDelayLength;
		ch->freqTabPtr = &fc->FRQpoint[fc->ptr8u_1[1] << 6];
		ch->freqTabPos = 0;
		ch->freqSusCounter = 0;
		ch->vibratoSpeed = fc->ptr8u_1[2];
		ch->vibratoDepth = fc->ptr8u_1[3];
		ch->vibratoDelay = fc->ptr8u_1[4];
		ch->vibratoCounter = ch->vibratoDepth;
		ch->vibratoUp = 1;
		ch->volTabPtr = &fc->ptr8u_1[5];
		ch->volTabPos = 0;
		ch->volSusCounter = 0;

		fc14_paulaStopDMA(fc, ch->voiceIndex);
	}

	ch->patPos += 2;
}

static void fc14_doFreqModulation(struct fc14_state *fc, struct fc14_fcChannel_t *ch) {
	uint8_t doTranspose;
	uint8_t *tmpPtr;
	const uint8_t *tabPtr;
	struct fc14_soundInfo_t *sample;

testsustain:
	if(ch->freqSusCounter > 0) {
		ch->freqSusCounter--;
	} else {
		tabPtr = &ch->freqTabPtr[ch->freqTabPos];

testeffects:
		if(tabPtr[0] != 0xE1) {
			doTranspose = 1;

			if(tabPtr[0] == 0xE0) {
				ch->freqTabPos = tabPtr[1] & 0x3F;
				tabPtr = &ch->freqTabPtr[ch->freqTabPos];
			}

			if(tabPtr[0] == 0xE2) {
				if(tabPtr[1] < FC14_NUM_SAMPLES + FC14_NUM_WAVEFORMS) {
					sample = &fc->samples[tabPtr[1]];

					ch->loopStart = sample->repeat;
					ch->loopLength = sample->replen;

					fc14_paulaSetData(fc, ch->voiceIndex, sample->data);
					fc14_paulaSetLength(fc, ch->voiceIndex, sample->length);
					fc14_paulaStartDMA(fc, ch->voiceIndex);

					ch->volTabPos = 0;
					ch->volDelayCounter = 1;
				}

				ch->freqTabPos += 2;
			} else if(tabPtr[0] == 0xE4) {
				if(tabPtr[1] < FC14_NUM_SAMPLES + FC14_NUM_WAVEFORMS) {
					sample = &fc->samples[tabPtr[1]];

					ch->loopStart = sample->repeat;
					ch->loopLength = sample->replen;

					fc14_paulaSetData(fc, ch->voiceIndex, sample->data);
					fc14_paulaSetLength(fc, ch->voiceIndex, sample->length);
				}

				ch->freqTabPos += 2;
			} else if(tabPtr[0] == 0xE9) {
				if(tabPtr[1] < FC14_NUM_SAMPLES + FC14_NUM_WAVEFORMS) {
					sample = &fc->samples[tabPtr[1]];
					if(*FC14_PTR2LONG(sample->data) == 0x504D5353) {
						tmpPtr = (uint8_t *)&sample->data[4 + (tabPtr[2] * 16)];

						ch->loopStart = &sample->data[(4 + 320 + *FC14_PTR2LONG(&tmpPtr[0])) + *FC14_PTR2WORD(&tmpPtr[6])];
						ch->loopLength = *FC14_PTR2WORD(&tmpPtr[8]);

						if(ch->loopLength <= 1) {
							ch->loopLength = 1;
							if(*FC14_PTR2WORD(&tmpPtr[4]) >= 1)
								*FC14_PTR2WORD(&sample->data[4 + 320 + *FC14_PTR2LONG(&tmpPtr[0])]) = 0;
						}

						fc14_paulaSetData(fc, ch->voiceIndex, &sample->data[4 + 320 + *FC14_PTR2LONG(&tmpPtr[0])]);
						fc14_paulaSetLength(fc, ch->voiceIndex, *FC14_PTR2WORD(&tmpPtr[4]));
						fc14_paulaStartDMA(fc, ch->voiceIndex);

						ch->volTabPos = 0;
						ch->volDelayCounter = 1;
					}
				}

				ch->freqTabPos += 3;
			} else if(tabPtr[0] == 0xE7) {
				tabPtr = &fc->FRQpoint[(tabPtr[1] & 0x3F) << 6];

				ch->freqTabPtr = tabPtr;
				ch->freqTabPos = 0;

				goto testeffects;
			} else if(tabPtr[0] == 0xE8) {
				ch->freqSusCounter = tabPtr[1];

				ch->freqTabPos += 2;
				goto testsustain;
			} else if(tabPtr[0] == 0xE3) {
				ch->freqTabPos += 3;

				ch->vibratoSpeed = tabPtr[1];
				ch->vibratoDepth = tabPtr[2];

				doTranspose = 0;
			} else if(tabPtr[0] == 0xEA) {
				ch->pitchBendValue = (int8_t)tabPtr[1];
				ch->pitchBendCounter = (int8_t)tabPtr[2];

				ch->freqTabPos += 3;
			}

			if(doTranspose)
				ch->periodTranspose = (int8_t)ch->freqTabPtr[ch->freqTabPos++];
		}
	}
}

static void fc14_do_VOLbend(struct fc14_fcChannel_t *ch) {
	ch->volSlideDelay = !ch->volSlideDelay;
	if(ch->volSlideDelay) {
		ch->volSlideCounter--;

		ch->volume += ch->volSlideSpeed;
		if(ch->volume < 0) {
			ch->volSlideCounter = 0;
			ch->volume = 0;
		}
	}
}

static void fc14_doVolModulation(struct fc14_state *fc, struct fc14_fcChannel_t *ch) {
	const uint8_t *tabPtr;

VOLUfx:
	if(ch->volSusCounter > 0) {
		ch->volSusCounter--;
	} else {
		if(ch->volSlideCounter > 0) {
			fc14_do_VOLbend(ch);
		} else {
			if(--ch->volDelayCounter == 0) {
				ch->volDelayCounter = ch->volDelayLength;
volu_cmd:
				tabPtr = &ch->volTabPtr[ch->volTabPos];

				if(tabPtr[0] != 0xE1) {
					if(tabPtr[0] == 0xE8) {
						ch->volSusCounter = tabPtr[1];
						ch->volTabPos += 2;

						goto VOLUfx;
					} else if(tabPtr[0] == 0xEA) {
						ch->volSlideSpeed = tabPtr[1];
						ch->volSlideCounter = tabPtr[2];

						ch->volTabPos += 3;
						fc14_do_VOLbend(ch);
					} else if(tabPtr[0] == 0xE0) {
						if((int8_t)(tabPtr[1] & 0x3F) - 5 < 0)
							ch->volTabPos = 0;
						else
							ch->volTabPos = (tabPtr[1] & 0x3F) - 5;

						goto volu_cmd;
					} else {
						ch->volume = (int8_t)ch->volTabPtr[ch->volTabPos++];
					}
				}
			}
		}
	}
}

static void fc14_effects(struct fc14_state *fc, struct fc14_fcChannel_t *ch) {
	int8_t tmpNote;
	int16_t tmpVibPeriod, tmpPeriod;
	uint16_t tmpVibNote;

	fc14_doFreqModulation(fc, ch);
	fc14_doVolModulation(fc, ch);

	tmpNote = ch->periodTranspose;
	if(tmpNote >= 0) {
		tmpNote += ch->note;
		tmpNote += ch->noteTranspose;
	}
	tmpNote &= 0x7F;

	tmpPeriod = fc14_periods[tmpNote];

	if(ch->vibratoDelay > 0) {
		ch->vibratoDelay--;
	} else {
		tmpVibPeriod = ch->vibratoCounter;
		if(!ch->vibratoUp) {
			tmpVibPeriod -= ch->vibratoSpeed;
			if(tmpVibPeriod < 0) {
				tmpVibPeriod = 0;
				ch->vibratoUp = 1;
			}
		} else {
			tmpVibPeriod += ch->vibratoSpeed;
			if(tmpVibPeriod > ch->vibratoDepth * 2) {
				tmpVibPeriod = ch->vibratoDepth * 2;
				ch->vibratoUp = 0;
			}
		}
		ch->vibratoCounter = tmpVibPeriod & 0xFF;

		tmpVibPeriod -= ch->vibratoDepth;

		tmpVibNote = tmpNote * 2;
		while(tmpVibNote < 12 * 8) {
			tmpVibPeriod *= 2;
			tmpVibNote += 12 * 2;
		}

		tmpPeriod += tmpVibPeriod;
	}

	ch->portaDelay = !ch->portaDelay;
	if(!fc->fc14 || ch->portaDelay) {
		if(ch->portaParam > 0) {
			if(ch->portaParam > 0x1F)
				ch->portaValue += ch->portaParam & 0x1F;
			else
				ch->portaValue -= ch->portaParam;
		}
	}

	ch->pitchBendDelay = !ch->pitchBendDelay;
	if(ch->pitchBendDelay) {
		if(ch->pitchBendCounter > 0) {
			ch->pitchBendCounter--;
			ch->portaValue -= ch->pitchBendValue;
		}
	}

	tmpPeriod += ch->portaValue;
	tmpPeriod = FC14_CLAMP(tmpPeriod, 0x0071, fc->fc14 ? 0x0D60 : 0x06B0);

	fc14_paulaSetPeriod(fc, ch->voiceIndex, tmpPeriod);
	fc14_paulaSetVolume(fc, ch->voiceIndex, ch->volume);
}

static void fc14_play_music(struct fc14_state *fc) {
	uint8_t i;

	if(--fc->respcnt == 0) {
		fc->respcnt = fc->repspd;
		for(i = 0; i < FC14_AMIGA_VOICES; i++)
			fc14_new_note(fc, &fc->Channel[i]);
	}

	for(i = 0; i < FC14_AMIGA_VOICES; i++) {
		fc14_effects(fc, &fc->Channel[i]);

		fc14_paulaSetData(fc, i, fc->Channel[i].loopStart);
		fc14_paulaSetLength(fc, i, fc->Channel[i].loopLength);
	}
}

static void fc14_resetAudioDithering(struct fc14_state *fc) {
	fc->randSeed = FC14_INITIAL_DITHER_SEED;
	fc->dPrngStateL = 0.0f;
	fc->dPrngStateR = 0.0f;
}

static inline int32_t fc14_random32(struct fc14_state *fc) {
	fc->randSeed = fc->randSeed * 134775813 + 1;
	return fc->randSeed;
}

static void fc14_mixAudio(struct fc14_state *fc, int16_t *stream, int32_t sampleBlockLength) {
	int32_t i, j, smp32;
	float dPrng, dSample, dVolume, dOut[2];
	struct fc14_paulaVoice_t *v;
#ifdef FC14_USE_BLEP
	struct fc14_blep_t *bSmp, *bVol;
#endif

	memset(fc->dMixerBufferL, 0, sizeof(float) * sampleBlockLength);
	memset(fc->dMixerBufferR, 0, sizeof(float) * sampleBlockLength);

	if(fc->musicPaused) {
		memset(stream, 0, sampleBlockLength * (sizeof(int16_t) * 2));
		return;
	}

	v = fc->paula;
	for(i = 0; i < FC14_AMIGA_VOICES; i++, v++) {
		if(!v->active || v->length < 2 || v->data == 0)
			continue;

#ifdef FC14_USE_BLEP
		bSmp = &fc->blep[i];
		bVol = &fc->blepVol[i];
#endif
		for(j = 0; j < sampleBlockLength; j++) {
			dSample = (float)v->data[v->pos] * (1.0f / 128.0f);
			dVolume = v->dVolume;

#ifdef FC14_USE_BLEP
			if(dSample != bSmp->dLastValue) {
				if(v->dLastDelta > v->dLastPhase) {
					fc14_blepAdd(bSmp, v->dLastPhase * v->dLastDeltaMul, bSmp->dLastValue - dSample);
				}

				bSmp->dLastValue = dSample;
			}

			if(dVolume != bVol->dLastValue) {
				fc14_blepVolAdd(bVol, bVol->dLastValue - dVolume);
				bVol->dLastValue = dVolume;
			}

			if(bSmp->samplesLeft > 0) dSample += fc14_blepRun(bSmp);
			if(bVol->samplesLeft > 0) dVolume += fc14_blepRun(bVol);
#endif
			dSample *= dVolume;

			fc->dMixerBufferL[j] += dSample * v->dPanL;
			fc->dMixerBufferR[j] += dSample * v->dPanR;

			v->dPhase += v->dDelta;
			if(v->dPhase >= 1.0f) {
				v->dPhase -= 1.0f;
#ifdef FC14_USE_BLEP
				v->dLastPhase = v->dPhase;
				v->dLastDelta = v->dDelta;
				v->dLastDeltaMul = v->dDeltaMul;
#endif
				if(++v->pos >= v->length) {
					v->pos = 0;

					v->length = v->newLength;
					v->data = v->newData;
				}
			}
		}
	}

	for(i = 0; i < sampleBlockLength; i++) {
		dOut[0] = fc->dMixerBufferL[i];
		dOut[1] = fc->dMixerBufferR[i];

#ifdef FC14_USE_LOWPASS
		fc14_lossyIntegrator(&fc->filterLo, dOut, dOut);
#endif

#ifdef FC14_USE_HIGHPASS
		fc14_lossyIntegratorHighPass(&fc->filterHi, dOut, dOut);
#endif

		dOut[0] *= -(float)INT16_MAX / (float)FC14_AMIGA_VOICES;
		dOut[1] *= -(float)INT16_MAX / (float)FC14_AMIGA_VOICES;

		dPrng = (float)fc14_random32(fc) * (0.5f / (float)INT32_MAX);
		dOut[0] = (dOut[0] + dPrng) - fc->dPrngStateL;
		fc->dPrngStateL = dPrng;
		smp32 = (int32_t)dOut[0];
		smp32 = (smp32 * fc->masterVol) >> 8;
		FC14_CLAMP16(smp32);
		*stream++ = (int16_t)smp32;

		dPrng = (float)fc14_random32(fc) * (0.5f / (float)INT32_MAX);
		dOut[1] = (dOut[1] + dPrng) - fc->dPrngStateR;
		fc->dPrngStateR = dPrng;
		smp32 = (int32_t)dOut[1];
		smp32 = (smp32 * fc->masterVol) >> 8;
		FC14_CLAMP16(smp32);
		*stream++ = (int16_t)smp32;
	}
}

static float fc14_sinApx(float fX) {
	fX = fX * (2.0f - fX);
	return fX * 1.09742972F + fX * fX * 0.31678383F;
}

static float fc14_cosApx(float fX) {
	fX = (1.0f - fX) * (1.0f + fX);
	return fX * 1.09742972F + fX * fX * 0.31678383F;
}

static void fc14_calculatePans(struct fc14_state *fc, uint8_t stereoSeparation) {
	uint8_t scaledPanPos;
	float p;

	if(stereoSeparation > 100)
		stereoSeparation = 100;

	scaledPanPos = (stereoSeparation * 128) / 100;

	p = (float)(128 - scaledPanPos) * (1.0f / 256.0f);
	fc->paula[0].dPanL = fc14_cosApx(p);
	fc->paula[0].dPanR = fc14_sinApx(p);
	fc->paula[3].dPanL = fc14_cosApx(p);
	fc->paula[3].dPanR = fc14_sinApx(p);

	p = (float)(128 + scaledPanPos) * (1.0f / 256.0f);
	fc->paula[1].dPanL = fc14_cosApx(p);
	fc->paula[1].dPanR = fc14_sinApx(p);
	fc->paula[2].dPanL = fc14_cosApx(p);
	fc->paula[2].dPanR = fc14_sinApx(p);
}

static void fc14_PauseSong(struct fc14_state *fc, uint8_t flag) {
	fc->musicPaused = flag;
}

static void fc14_TogglePause(struct fc14_state *fc) {
	fc->musicPaused ^= 1;
}

static void fc14_shutdown(struct fc14_state *fc) {
	if(fc->dMixerBufferL != 0) {
		free(fc->dMixerBufferL);
		fc->dMixerBufferL = 0;
	}

	if(fc->dMixerBufferR != 0) {
		free(fc->dMixerBufferR);
		fc->dMixerBufferR = 0;
	}

	if(fc->songData != 0) {
		free(fc->songData);
		fc->songData = 0;
	}
}

static void fc14_SetStereoSep(struct fc14_state *fc, uint8_t percentage) {
	fc->stereoSep = percentage;
	if(fc->stereoSep > 100)
		fc->stereoSep = 100;

	fc14_calculatePans(fc, fc->stereoSep);
}

static void fc14_SetMasterVol(struct fc14_state *fc, uint16_t vol) {
	fc->masterVol = (vol > 256) ? 256 : vol;
}

static uint16_t fc14_GetMasterVol(struct fc14_state *fc) {
	return (uint16_t)fc->masterVol;
}

static uint32_t fc14_GetMixerTicks(struct fc14_state *fc) {
	if(fc->audioRate < 1000)
		return 0;

	return fc->sampleCounter / (fc->audioRate / 1000);
}

static uint8_t fc14_initialize(struct fc14_state *fc, const uint8_t *moduleData, uint32_t dataLength, uint32_t audioFreq) {
	fc->stereoSep = FC14_STEREO_SEP;
	fc->masterVol = 256;

	if(audioFreq == 0)
		audioFreq = 44100;

	fc->musicPaused = 1;

	fc14_shutdown(fc);

	fc->oldPeriod = 0;
	fc->oldVoiceDelta = 0.0f;
	fc->sampleCounter = 0;

	fc->songData = (uint8_t *)malloc(dataLength);
	if(fc->songData == 0)
		return 0;

	memcpy(fc->songData, moduleData, dataLength);
	if(!fc14_init_music(fc, fc->songData)) {
		fc14_shutdown(fc);
		return 0;
	}

	fc->dMixerBufferL = (float *)malloc(FC14_MIX_BUF_SAMPLES * sizeof(float));
	fc->dMixerBufferR = (float *)malloc(FC14_MIX_BUF_SAMPLES * sizeof(float));

	if(fc->dMixerBufferL == 0 || fc->dMixerBufferR == 0) {
		fc14_shutdown(fc);
		return 0;
	}

	fc14_restart_song(fc);

	audioFreq = FC14_CLAMP(audioFreq, 32000, 96000);

	fc->audioRate = audioFreq;
	fc->dAudioRate = (float)audioFreq;
	fc->dPeriodToDeltaDiv = (float)FC14_PAULA_PAL_CLK / fc->dAudioRate;
	fc->soundBufferSize = FC14_MIX_BUF_SAMPLES;
	fc->samplesPerFrame = (uint32_t)roundf(fc->dAudioRate / (float)FC14_AMIGA_PAL_VBLANK_HZ);

#ifdef FC14_USE_LOWPASS
	fc14_calcCoeffLossyIntegrator(fc->dAudioRate, 4420.97F + 580.0f, &fc->filterLo);
#endif

#ifdef FC14_USE_HIGHPASS
	fc14_calcCoeffLossyIntegrator(fc->dAudioRate, 5.20f + 1.5f, &fc->filterHi);
#endif

	memset(fc->paula, 0, sizeof(fc->paula));
	fc14_calculatePans(fc, fc->stereoSep);

#ifdef FC14_USE_BLEP
	memset(fc->blep, 0, sizeof(fc->blep));
	memset(fc->blepVol, 0, sizeof(fc->blepVol));
#endif

#ifdef FC14_USE_LOWPASS
	fc14_clearLossyIntegrator(&fc->filterLo);
#endif

#ifdef FC14_USE_HIGHPASS
	fc14_clearLossyIntegrator(&fc->filterHi);
#endif

	fc14_resetAudioDithering(fc);

	fc->musicPaused = 0;
	return 1;
}

static void fc14_get_audio(struct fc14_state *fc, int16_t *buffer, int32_t numSamples) {
	int32_t a, b;

	a = numSamples;
	while(a > 0) {
		if(fc->samplesPerFrameLeft == 0) {
			if(!fc->musicPaused) {
				fc14_play_music(fc);
			}

			fc->samplesPerFrameLeft = fc->samplesPerFrame;
		}

		b = a;
		if(b > fc->samplesPerFrameLeft) {
			b = fc->samplesPerFrameLeft;
		}

		fc14_mixAudio(fc, buffer, b);
		buffer += (uint32_t)b * 2;

		a -= b;
		fc->samplesPerFrameLeft -= b;
	}

	fc->sampleCounter += numSamples;
}
