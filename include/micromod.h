// NOTE(peter): This is NOT the original, it's modified to use a state and some things has been changed over time!

#pragma once

#define FP_SHIFT 14
#define FP_ONE   16384
#define FP_MASK  16383

#define MICROMOD_MAX_CHANNELS 4

struct micromod_note {
	uint16_t key;
	uint8_t instrument, effect, param;
};

struct micromod_instrument {
	uint8_t volume, fine_tune;
	uint32_t loop_start, loop_length;
	int8_t *sample_data;
};

struct micromod_channel {
	struct micromod_note note;
	uint16_t period, porta_period;
	uint32_t sample_offset, sample_idx, step;
	uint8_t volume, panning, fine_tune, ampl, mute;
	uint8_t id, instrument, assigned, porta_speed, pl_row, fx_count;
	uint8_t vibrato_type, vibrato_phase, vibrato_speed, vibrato_depth;
	uint8_t tremolo_type, tremolo_phase, tremolo_speed, tremolo_depth;
	int8_t tremolo_add, vibrato_add, arpeggio_add;
};

struct micromod_state {
	int8_t *module_data;
	uint8_t *pattern_data, *sequence;
	int32_t song_length, restart, num_patterns, num_channels;
	struct micromod_instrument instruments[32];

	int32_t sample_rate, gain, c2_rate, tick_len, tick_offset;
	int32_t pattern, break_pattern, row, next_row, tick;
	int32_t speed, pl_count, pl_channel, random_seed;

	struct micromod_channel channels[MICROMOD_MAX_CHANNELS];

	/* Note trigger tracking */
	uint8_t note_triggered[4];     /* Per-channel: 1 if note was triggered this tick, 0 otherwise */
	uint8_t triggered_sample[4];   /* Per-channel: Sample/instrument number that was triggered (1-31) */
	uint8_t sample_triggered[32];  /* Per-sample: 1 if this sample was triggered on any channel this tick */
};

static const char *MICROMOD_VERSION = "Micromod Protracker replay 20180625 (c)mumart@gmail.com";

static const uint16_t fine_tuning[] = {
	4340, 4308, 4277, 4247, 4216, 4186, 4156, 4126, 4096, 4067, 4037, 4008, 3979, 3951, 3922, 3894
};

static const uint16_t arp_tuning[] = {
	4096, 3866, 3649, 3444, 3251, 3069, 2896, 2734, 2580, 2435, 2299, 2170, 2048, 1933, 1825, 1722
};

static const uint8_t sine_table[] = {
	0,  24,  49,  74,  97, 120, 141, 161, 180, 197, 212, 224, 235, 244, 250, 253, 255, 253, 250, 244, 235, 224, 212, 197, 180, 161, 141, 120,  97,  74,  49,  24
};

static int32_t micromod_calculate_num_patterns(int8_t *module_header) {
	int32_t num_patterns, order_entry, pattern;
	num_patterns = 0;
	for(pattern = 0; pattern < 128; pattern++) {
		order_entry = module_header[ 952 + pattern ] & 0x7F;
		if(order_entry >= num_patterns) num_patterns = order_entry + 1;
	}
	return num_patterns;
}

static int32_t micromod_calculate_num_channels(int8_t *module_header) {
	int32_t numchan;
	switch((module_header[ 1082 ] << 8) | module_header[ 1083 ]) {
		case 0x4b2e: /* M.K. */
		case 0x4b21: /* M!K! */
		case 0x542e: /* N.T. */
		case 0x5434: /* FLT4 */
			numchan = 4;
			break;
		case 0x484e: /* xCHN */
			numchan = module_header[ 1080 ] - 48;
			break;
		case 0x4348: /* xxCH */
			numchan = ((module_header[ 1080 ] - 48) * 10) + (module_header[ 1081 ] - 48);
			break;
		default: /* Not recognised. */
			numchan = 0;
			break;
	}
	if(numchan > MICROMOD_MAX_CHANNELS) numchan = 0;
	return numchan;
}


static void micromod_set_tempo(struct micromod_state *m, int32_t tempo) {
	m->tick_len = ((m->sample_rate << 1) + (m->sample_rate >> 1)) / tempo;
}

static void micromod_update_frequency(struct micromod_state *m, struct micromod_channel *chan) {
	int32_t period, volume;
	uint32_t freq;
	period = chan->period + chan->vibrato_add;
	period = period * arp_tuning[ chan->arpeggio_add ] >> 11;
	period = (period >> 1) + (period & 1);
	if(period < 14) period = 6848;
	freq = m->c2_rate * 428 / period;
	chan->step = (freq << FP_SHIFT) / m->sample_rate;
	volume = chan->volume + chan->tremolo_add;
	if(volume > 64) volume = 64;
	if(volume < 0) volume = 0;
	chan->ampl = (volume * m->gain) >> 5;
}

static void micromod_tone_portamento(struct micromod_channel *chan) {
	int32_t source, dest;
	source = chan->period;
	dest = chan->porta_period;
	if(source < dest) {
		source += chan->porta_speed;
		if(source > dest) source = dest;
	} else if(source > dest) {
		source -= chan->porta_speed;
		if(source < dest) source = dest;
	}
	chan->period = source;
}

static void micromod_volume_slide(struct micromod_channel *chan, int32_t param) {
	int32_t volume;
	volume = chan->volume + (param >> 4) - (param & 0xF);
	if(volume < 0) volume = 0;
	if(volume > 64) volume = 64;
	chan->volume = volume;
}

static int32_t micromod_waveform(struct micromod_state *m, int32_t phase, int32_t type) {
	int32_t amplitude = 0;
	switch(type & 0x3) {
		case 0: /* Sine. */
			amplitude = sine_table[ phase & 0x1F ];
			if((phase & 0x20) > 0) amplitude = -amplitude;
			break;
		case 1: /* Saw Down. */
			amplitude = 255 - (((phase + 0x20) & 0x3F) << 3);
			break;
		case 2: /* Square. */
			amplitude = 255 - ((phase & 0x20) << 4);
			break;
		case 3: /* Random. */
			amplitude = (m->random_seed >> 20) - 255;
			m->random_seed = (m->random_seed * 65 + 17) & 0x1FFFFFFF;
			break;
	}
	return amplitude;
}

static void micromod_vibrato(struct micromod_state *m, struct micromod_channel *chan) {
	chan->vibrato_add = micromod_waveform(m, chan->vibrato_phase, chan->vibrato_type) * chan->vibrato_depth >> 7;
}

static void micromod_tremolo(struct micromod_state *m, struct micromod_channel *chan) {
	chan->tremolo_add = micromod_waveform(m, chan->tremolo_phase, chan->tremolo_type) * chan->tremolo_depth >> 6;
}

static void micromod_trigger(struct micromod_state *m, struct micromod_channel *channel) {
	int32_t period, ins;
	ins = channel->note.instrument;
	if(ins > 0 && ins < 32) {
		channel->assigned = ins;
		channel->sample_offset = 0;
		channel->fine_tune = m->instruments[ ins ].fine_tune;
		channel->volume = m->instruments[ ins ].volume;
		if(m->instruments[ ins ].loop_length > 0 && channel->instrument > 0)
			channel->instrument = ins;
	}
	if(channel->note.effect == 0x09) {
		channel->sample_offset = (channel->note.param & 0xFF) << 8;
	} else if(channel->note.effect == 0x15) {
		channel->fine_tune = channel->note.param;
	}
	if(channel->note.key > 0) {
		period = (channel->note.key * fine_tuning[ channel->fine_tune & 0xF ]) >> 11;
		channel->porta_period = (period >> 1) + (period & 1);
		if(channel->note.effect != 0x3 && channel->note.effect != 0x5) {
			channel->instrument = channel->assigned;
			channel->period = channel->porta_period;
			channel->sample_idx = (channel->sample_offset << FP_SHIFT);
			if(channel->vibrato_type < 4) channel->vibrato_phase = 0;
			if(channel->tremolo_type < 4) channel->tremolo_phase = 0;

			/* Mark note as triggered */
			m->note_triggered[ channel->id ] = 1;
			m->triggered_sample[ channel->id ] = channel->assigned;
			if(channel->assigned > 0 && channel->assigned < 32) {
				m->sample_triggered[ channel->assigned ] = 1;
			}
		}
	}
}

static void micromod_channel_row(struct micromod_state *m, struct micromod_channel *chan) {
	int32_t effect, param, volume, period;
	effect = chan->note.effect;
	param = chan->note.param;
	chan->vibrato_add = chan->tremolo_add = chan->arpeggio_add = chan->fx_count = 0;
	if(!(effect == 0x1D && param > 0)) {
		/* Not note delay. */
		micromod_trigger(m, chan);
	}
	switch(effect) {
		case 0x3: /* Tone Portamento.*/
			if(param > 0) chan->porta_speed = param;
			break;
		case 0x4: /* Vibrato.*/
			if((param & 0xF0) > 0) chan->vibrato_speed = param >> 4;
			if((param & 0x0F) > 0) chan->vibrato_depth = param & 0xF;
			micromod_vibrato(m, chan);
			break;
		case 0x6: /* Vibrato + Volume Slide.*/
			micromod_vibrato(m, chan);
			break;
		case 0x7: /* Tremolo.*/
			if((param & 0xF0) > 0) chan->tremolo_speed = param >> 4;
			if((param & 0x0F) > 0) chan->tremolo_depth = param & 0xF;
			micromod_tremolo(m, chan);
			break;
		case 0x8: /* Set Panning (0-127). Not for 4-channel Protracker. */
			if(m->num_channels != 4) {
				chan->panning = (param < 128) ? param : 127;
			}
			break;
		case 0xB: /* Pattern Jump.*/
			if(m->pl_count < 0) {
				m->break_pattern = param;
				m->next_row = 0;
			}
			break;
		case 0xC: /* Set Volume.*/
			chan->volume = param > 64 ? 64 : param;
			break;
		case 0xD: /* Pattern Break.*/
			if(m->pl_count < 0) {
				if(m->break_pattern < 0) m->break_pattern = m->pattern + 1;
				m->next_row = (param >> 4) * 10 + (param & 0xF);
				if(m->next_row >= 64) m->next_row = 0;
			}
			break;
		case 0xF: /* Set Speed.*/
			if(param > 0) {
				if(param < 32) m->tick = m->speed = param;
				else micromod_set_tempo(m, param);
			}
			break;
		case 0x11: /* Fine Portamento Up.*/
			period = chan->period - param;
			chan->period = period < 0 ? 0 : period;
			break;
		case 0x12: /* Fine Portamento Down.*/
			period = chan->period + param;
			chan->period = period > 65535 ? 65535 : period;
			break;
		case 0x14: /* Set Vibrato Waveform.*/
			if(param < 8) chan->vibrato_type = param;
			break;
		case 0x16: /* Pattern Loop.*/
			if(param == 0) /* Set loop marker on this channel. */
				chan->pl_row = m->row;
			if(chan->pl_row < m->row && m->break_pattern < 0) { /* Marker valid. */
				if(m->pl_count < 0) { /* Not already looping, begin. */
					m->pl_count = param;
					m->pl_channel = chan->id;
				}
				if(m->pl_channel == chan->id) { /* Next Loop.*/
					if(m->pl_count == 0) { /* Loop finished. */
						/* Invalidate current marker. */
						chan->pl_row = m->row + 1;
					} else { /* Loop. */
						m->next_row = chan->pl_row;
					}
					--(m->pl_count);
				}
			}
			break;
		case 0x17: /* Set Tremolo Waveform.*/
			if(param < 8) chan->tremolo_type = param;
			break;
		case 0x1A: /* Fine Volume Up.*/
			volume = chan->volume + param;
			chan->volume = volume > 64 ? 64 : volume;
			break;
		case 0x1B: /* Fine Volume Down.*/
			volume = chan->volume - param;
			chan->volume = volume < 0 ? 0 : volume;
			break;
		case 0x1C: /* Note Cut.*/
			if(param <= 0) chan->volume = 0;
			break;
		case 0x1E: /* Pattern Delay.*/
			m->tick = m->speed * (param + 1L);
			break;
	}
	micromod_update_frequency(m, chan);
}

static void micromod_channel_tick(struct micromod_state *m, struct micromod_channel *chan) {
	int32_t effect, param, period;
	effect = chan->note.effect;
	param = chan->note.param;
	chan->fx_count++;
	switch(effect) {
		case 0x1: /* Portamento Up.*/
			period = chan->period - param;
			chan->period = period < 0 ? 0 : period;
			break;
		case 0x2: /* Portamento Down.*/
			period = chan->period + param;
			chan->period = period > 65535 ? 65535 : period;
			break;
		case 0x3: /* Tone Portamento.*/
			micromod_tone_portamento(chan);
			break;
		case 0x4: /* Vibrato.*/
			chan->vibrato_phase += chan->vibrato_speed;
			micromod_vibrato(m, chan);
			break;
		case 0x5: /* Tone Porta + Volume Slide.*/
			micromod_tone_portamento(chan);
			micromod_volume_slide(chan, param);
			break;
		case 0x6: /* Vibrato + Volume Slide.*/
			chan->vibrato_phase += chan->vibrato_speed;
			micromod_vibrato(m, chan);
			micromod_volume_slide(chan, param);
			break;
		case 0x7: /* Tremolo.*/
			chan->tremolo_phase += chan->tremolo_speed;
			micromod_tremolo(m, chan);
			break;
		case 0xA: /* Volume Slide.*/
			micromod_volume_slide(chan, param);
			break;
		case 0xE: /* Arpeggio.*/
			if(chan->fx_count > 2) chan->fx_count = 0;
			if(chan->fx_count == 0) chan->arpeggio_add = 0;
			if(chan->fx_count == 1) chan->arpeggio_add = param >> 4;
			if(chan->fx_count == 2) chan->arpeggio_add = param & 0xF;
			break;
		case 0x19: /* Retrig.*/
			if(chan->fx_count >= param) {
				chan->fx_count = 0;
				chan->sample_idx = 0;
			}
			break;
		case 0x1C: /* Note Cut.*/
			if(param == chan->fx_count) chan->volume = 0;
			break;
		case 0x1D: /* Note Delay.*/
			if(param == chan->fx_count) micromod_trigger(m, chan);
			break;
	}
	if(effect > 0) micromod_update_frequency(m, chan);
}

static int32_t micromod_sequence_row(struct micromod_state *m) {
	int32_t song_end, chan_idx, pat_offset;
	int32_t effect, param;
	struct micromod_note *note;
	song_end = 0;
	if(m->next_row < 0) {
		m->break_pattern = m->pattern + 1;
		m->next_row = 0;
	}
	if(m->break_pattern >= 0) {
		if(m->break_pattern >= m->song_length) m->break_pattern = m->next_row = 0;
		if(m->break_pattern <= m->pattern) song_end = 1;
		m->pattern = m->break_pattern;
		for(chan_idx = 0; chan_idx < m->num_channels; chan_idx++) m->channels[ chan_idx ].pl_row = 0;
		m->break_pattern = -1;
	}
	m->row = m->next_row;
	m->next_row = m->row + 1;
	if(m->next_row >= 64) m->next_row = -1;
	pat_offset = (m->sequence[ m->pattern ] * 64 + m->row) * m->num_channels * 4;
	for(chan_idx = 0; chan_idx < m->num_channels; chan_idx++) {
		note = &m->channels[ chan_idx ].note;
		note->key  = (m->pattern_data[ pat_offset ] & 0xF) << 8;
		note->key |=   m->pattern_data[ pat_offset + 1 ];
		note->instrument  = m->pattern_data[ pat_offset + 2 ] >> 4;
		note->instrument |= m->pattern_data[ pat_offset ] & 0x10;
		effect = m->pattern_data[ pat_offset + 2 ] & 0xF;
		param = m->pattern_data[ pat_offset + 3 ];
		pat_offset += 4;
		if(effect == 0xE) {
			effect = 0x10 | (param >> 4);
			param &= 0xF;
		}
		if(effect == 0 && param > 0) effect = 0xE;
		note->effect = effect;
		note->param = param;
		micromod_channel_row(m, &m->channels[ chan_idx ]);
	}
	return song_end;
}

static int32_t micromod_sequence_tick(struct micromod_state *m) {
	int32_t song_end, chan_idx;
	song_end = 0;
	if(--(m->tick) <= 0) {
		m->tick = m->speed;
		song_end = micromod_sequence_row(m);
	} else {
		for(chan_idx = 0; chan_idx < m->num_channels; chan_idx++)
			micromod_channel_tick(m, &m->channels[ chan_idx ]);
	}
	return song_end;
}

static void micromod_resample(struct micromod_state *m, struct micromod_channel *chan, int16_t *buf, int32_t offset, int32_t count) {
	uint32_t epos;
	uint32_t buf_idx = offset << 1;
	uint32_t buf_end = (offset + count) << 1;
	uint32_t sidx = chan->sample_idx;
	uint32_t step = chan->step;
	uint32_t llen = m->instruments[ chan->instrument ].loop_length;
	uint32_t lep1 = m->instruments[ chan->instrument ].loop_start + llen;
	int8_t *sdat = m->instruments[ chan->instrument ].sample_data;
	int16_t ampl = buf && !chan->mute ? chan->ampl : 0;
	int16_t lamp = ampl * (127 - chan->panning) >> 5;
	int16_t ramp = ampl * chan->panning >> 5;
	while(buf_idx < buf_end) {
		if(sidx >= lep1) {
			/* Handle loop. */
			if(llen <= FP_ONE) {
				/* One-shot sample. */
				sidx = lep1;
				break;
			}
			/* Subtract loop-length until within loop points. */
			while(sidx >= lep1) sidx -= llen;
		}
		/* Calculate sample position at end. */
		epos = sidx + ((buf_end - buf_idx) >> 1) * step;
		/* Most of the cpu time is spent here. */
		if(lamp || ramp) {
			/* Only mix to end of current loop. */
			if(epos > lep1) epos = lep1;
			if(lamp && ramp) {
				/* Mix both channels. */
				while(sidx < epos) {
					ampl = sdat[ sidx >> FP_SHIFT ];
					buf[ buf_idx++ ] += ampl * lamp >> 2;
					buf[ buf_idx++ ] += ampl * ramp >> 2;
					sidx += step;
				}
			} else {
				/* Only mix one channel. */
				if(ramp) buf_idx++;
				while(sidx < epos) {
					buf[ buf_idx ] += sdat[ sidx >> FP_SHIFT ] * ampl;
					buf_idx += 2;
					sidx += step;
				}
				buf_idx &= -2;
			}
		} else {
			/* No need to mix.*/
			buf_idx = buf_end;
			sidx = epos;
		}
	}
	chan->sample_idx = sidx;
}

/*
	Returns a string containing version information.
*/
static const char* micromod_get_version(void) {
	return MICROMOD_VERSION;
}

/*
	Calculate the length in bytes of a module file given the 1084-byte header.
	Returns -1 if the data is not recognised as a module.
*/
static int32_t micromod_calculate_mod_file_len(int8_t *module_header) {
	int32_t length, numchan, inst_idx;
	numchan = micromod_calculate_num_channels(module_header);
	if(numchan <= 0) return -1;
	length = 1084 + 4 * numchan * 64 * micromod_calculate_num_patterns(module_header);
	for(inst_idx = 1; inst_idx < 32; inst_idx++)
		length += __builtin_bswap16(*(uint16_t*)&module_header[inst_idx * 30 + 12]) * 2;
	return length;
}

/*
	Jump directly to a specific pattern in the sequence.
*/
static void micromod_set_position(struct micromod_state *m, int32_t pos) {
	int32_t chan_idx;
	struct micromod_channel *chan;
	if(m->num_channels <= 0) return;
	if(pos >= m->song_length) pos = 0;
	m->break_pattern = pos;
	m->next_row = 0;
	m->tick = 1;
	m->speed = 6;
	micromod_set_tempo(m, 125);
	m->pl_count = m->pl_channel = -1;
	m->random_seed = 0xABCDEF;
	for(chan_idx = 0; chan_idx < m->num_channels; chan_idx++) {
		chan = &m->channels[ chan_idx ];
		chan->id = chan_idx;
		chan->instrument = chan->assigned = 0;
		chan->volume = 0;
		switch(chan_idx & 0x3) {
			case 0: case 3: chan->panning = 0; break;
			case 1: case 2: chan->panning = 127; break;
		}
	}
	micromod_sequence_tick(m);
	m->tick_offset = 0;
}

/*
	Mute the specified channel.
	If channel is negative, un-mute all channels.
	Returns the number of channels.
*/
static int32_t micromod_mute_channel(struct micromod_state *m, int32_t channel) {
	int32_t chan_idx;
	if(channel < 0) {
		for(chan_idx = 0; chan_idx < m->num_channels; chan_idx++) {
			m->channels[ chan_idx ].mute = 0;
		}
	} else if(channel < m->num_channels) {
		m->channels[ channel ].mute = 1;
	}
	return m->num_channels;
}

/*
	Set the player to play the specified module data.
	Returns -1 if the data is not recognised as a module.
	Returns -2 if the sampling rate is less than 8000hz.
*/
static int32_t micromod_initialize(struct micromod_state *m, int8_t *data, int32_t sampling_rate) {
	struct micromod_instrument *inst;
	int32_t sample_data_offset, inst_idx;
	int32_t sample_length, volume, fine_tune, loop_start, loop_length;
	m->num_channels = micromod_calculate_num_channels(data);
	if(m->num_channels <= 0) {
		m->num_channels = 0;
		return -1;
	}
	if(sampling_rate < 8000) return -2;
	m->module_data = data;
	m->sample_rate = sampling_rate;
	m->song_length = m->module_data[ 950 ] & 0x7F;
	m->restart = m->module_data[ 951 ] & 0x7F;
	if(m->restart >= m->song_length) m->restart = 0;
	m->sequence = (uint8_t *) m->module_data + 952;
	m->pattern_data = (uint8_t *) m->module_data + 1084;
	m->num_patterns = micromod_calculate_num_patterns(m->module_data);
	sample_data_offset = 1084 + m->num_patterns * 64 * m->num_channels * 4;
	for(inst_idx = 1; inst_idx < 32; inst_idx++) {
		inst = &m->instruments[ inst_idx ];
		sample_length = __builtin_bswap16(*(uint16_t*)&m->module_data[inst_idx * 30 + 12]) * 2;
		fine_tune = m->module_data[ inst_idx * 30 + 14 ] & 0xF;
		inst->fine_tune = (fine_tune & 0x7) - (fine_tune & 0x8) + 8;
		volume = m->module_data[ inst_idx * 30 + 15 ] & 0x7F;
		inst->volume = volume > 64 ? 64 : volume;
		loop_start = __builtin_bswap16(*(uint16_t*)&m->module_data[inst_idx * 30 + 16]) * 2;
		loop_length = __builtin_bswap16(*(uint16_t*)&m->module_data[inst_idx * 30 + 18]) * 2;
		if(loop_start + loop_length > sample_length) {
			if(loop_start / 2 + loop_length <= sample_length) {
				/* Some old modules have loop start in bytes. */
				loop_start = loop_start / 2;
			} else {
				loop_length = sample_length - loop_start;
			}
		}
		if(loop_length < 4) {
			loop_start = sample_length;
			loop_length = 0;
		}
		inst->loop_start = loop_start << FP_SHIFT;
		inst->loop_length = loop_length << FP_SHIFT;
		inst->sample_data = m->module_data + sample_data_offset;
		sample_data_offset += sample_length;
	}
	m->c2_rate = (m->num_channels > 4) ? 8363 : 8287;
	m->gain = (m->num_channels > 4) ? 32 : 64;

	/* Initialize trigger tracking arrays */
	for(inst_idx = 0; inst_idx < 4; inst_idx++) {
		m->note_triggered[inst_idx] = 0;
		m->triggered_sample[inst_idx] = 0;
	}
	for(inst_idx = 0; inst_idx < 32; inst_idx++) {
		m->sample_triggered[inst_idx] = 0;
	}

	micromod_mute_channel(m, -1);
	micromod_set_position(m, 0);
	return 0;
}

/*
	Obtains song and instrument names from the module.
	The song name is returned as instrument 0.
	The name is copied into the location pointed to by string,
	and is at most 23 characters long, including the trailing null.
*/
static void micromod_get_string(struct micromod_state *m, int32_t instrument, char *string) {
	int32_t index, offset, length, character;
	if(m->num_channels <= 0) {
		string[ 0 ] = 0;
		return;
	}
	offset = 0;
	length = 20;
	if(instrument > 0 && instrument < 32) {
		offset = (instrument - 1) * 30 + 20;
		length = 22;
	}
	for(index = 0; index < length; index++) {
		character = m->module_data[ offset + index ];
		if(character < 32 || character > 126) character = ' ';
		string[ index ] = character;
	}
	string[ length ] = 0;
}

/*
	Returns the total song duration in samples at the current sampling rate.
*/
static int32_t micromod_calculate_song_duration(struct micromod_state *m) {
	int32_t duration, song_end;
	duration = 0;
	if(m->num_channels > 0) {
		micromod_set_position(m, 0);
		song_end = 0;
		while(!song_end) {
			duration += m->tick_len;
			song_end = micromod_sequence_tick(m);
		}
		micromod_set_position(m, 0);
	}
	return duration;
}

/*
	Clear all note trigger flags.
	Call this after reading the trigger state to reset for the next audio chunk.
*/
static void micromod_clear_triggers(struct micromod_state *m) {
	int32_t i;
	for(i = 0; i < 4; i++) {
		m->note_triggered[i] = 0;
		m->triggered_sample[i] = 0;
	}
	for(i = 0; i < 32; i++) {
		m->sample_triggered[i] = 0;
	}
}

/*
	Set the playback gain.
	For 4-channel modules, a value of 64 can be used without distortion.
	For 8-channel modules, a value of 32 or less is recommended.
*/
static void micromod_set_gain(struct micromod_state *m, int32_t value) {
	m->gain = value;
}

/*
	Calculate the specified number of samples of audio.
	If output pointer is zero, the replay will quickly skip count samples.
	The output buffer should be cleared with zeroes.
*/
static void micromod_get_audio(struct micromod_state *m, int16_t *output_buffer, int32_t count) {
	int32_t offset, remain, chan_idx;
	if(m->num_channels <= 0) return;
	offset = 0;
	while(count > 0) {
		remain = m->tick_len - m->tick_offset;
		if(remain > count) remain = count;
		for(chan_idx = 0; chan_idx < m->num_channels; chan_idx++) {
			micromod_resample(m, &m->channels[ chan_idx ], output_buffer, offset, remain);
		}
		m->tick_offset += remain;
		if(m->tick_offset == m->tick_len) {
			micromod_sequence_tick(m);
			m->tick_offset = 0;
		}
		offset += remain;
		count -= remain;
	}
}
