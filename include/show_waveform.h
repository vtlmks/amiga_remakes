

static void draw_waveform(struct platform_state *state, int channel, int x, int y, int width, int height) {
	struct micromod_channel *chan = &module.channels[channel];
	int ins = chan->instrument;

	if(ins <= 0) return;

	struct micromod_instrument *inst = &module.instruments[ins];
	int8_t *data = inst->sample_data;
	uint32_t pos = chan->sample_idx >> FP_SHIFT;
	uint32_t loop_start = inst->loop_start >> FP_SHIFT;
	uint32_t loop_len = inst->loop_length >> FP_SHIFT;
	uint32_t loop_end = loop_start + loop_len;
	int has_loop = (loop_len > 1);

	for(int i = 0; i < width; ++i) {
		int8_t val;
		uint32_t read_pos = pos + i;

		if(has_loop && read_pos >= loop_end) {
			read_pos = loop_start + ((read_pos - loop_start) % loop_len);
			val = data[read_pos];
		} else if(!has_loop && read_pos >= loop_start) {
			val = 0;
		} else {
			val = data[read_pos];
		}

		int py = y + (height / 2) - (val * height / 256);
		*BUFFER_PTR(state, x + i, py) = 0xffffffff;
	}
}

