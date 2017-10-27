#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "c_synth.h"
#include "sequencer.h"
#include "oscillator.h"

static PCMSample_t
convert_tick_to_sample(SongData *song, PCMSample_t sampling_rate, Tick_t tick);
static Freq_t
convert_note_to_freq(unsigned char note);

void
sequencer_playback(SongData *song, PCM_Data *out_pcm)
{
	unsigned int ch_idx = 0;

	out_pcm->total_sample = convert_tick_to_sample(song, out_pcm->sampling_rate,
												   song->total_tick);
	out_pcm->pcm = calloc(out_pcm->total_sample, sizeof(PCM_Block));

	for (ch_idx = 0; ch_idx < SMF_MAX_CHANNEL_NUM; ch_idx++) {
		NoteData *head = song->notes[ch_idx];

		while (head != NULL) {
			OSC_Params osc_param;

			osc_param.osc_freq = convert_note_to_freq(head->note);
			osc_param.osc_gain = 30 * head->velocity;
			osc_param.offset = convert_tick_to_sample(song, out_pcm->sampling_rate,
													  head->offset);
			osc_param.dulation = convert_tick_to_sample(song, out_pcm->sampling_rate,
														head->dulation);
			generate_sine_wave(&osc_param, out_pcm);

			head = head->next;
		}
	}
}

static PCMSample_t
convert_tick_to_sample(SongData *song, PCMSample_t sampling_rate, Tick_t tick)
{
	double usec_per_tick = song->usec_per_beat / song->tick_per_beat;
	double offset_usec   = tick * usec_per_tick;

	return offset_usec * sampling_rate / (1000 * 1000);
}

static Freq_t
convert_note_to_freq(unsigned char note)
{
	return 440 * pow(2, (double)(note - 69) / 12);
}
