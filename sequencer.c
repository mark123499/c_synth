#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "c_synth.h"
#include "sequencer.h"
#include "oscillator.h"

static double
calculate_offset_usec(SongData *song, Tick_t tick);
static PCMSample_t
convert_usec_to_sample(double offset_usec, PCMSample_t sampling_rate);
static Freq_t
convert_note_to_freq(unsigned char note);

void
sequencer_playback(SongData *song, PCM_Data *out_pcm)
{
	unsigned int  ch_idx   = 0;
	unsigned long note_cnt = 0;
	double total_usec = calculate_offset_usec(song, song->total_tick);

	out_pcm->total_sample = convert_usec_to_sample(total_usec, out_pcm->sampling_rate);
	out_pcm->pcm = calloc(out_pcm->total_sample, sizeof(PCM_Block));

	for (ch_idx = 0; ch_idx < SMF_MAX_CHANNEL_NUM; ch_idx++) {
		NoteData *head = song->notes[ch_idx];

		while (head != NULL) {
			OSC_Params osc_param;
			double     offset_usec = calculate_offset_usec(song, head->offset);
			double     end_usec = calculate_offset_usec(song, head->offset + head->dulation);
			double     dulation = end_usec - offset_usec;

			osc_param.osc_freq = convert_note_to_freq(head->note);
			osc_param.osc_gain = 30 * head->velocity;
			osc_param.offset = convert_usec_to_sample(offset_usec, out_pcm->sampling_rate);
			osc_param.dulation = convert_usec_to_sample(dulation, out_pcm->sampling_rate);
			generate_sine_wave(&osc_param, out_pcm);

			head = head->next;
			note_cnt++;
			if (note_cnt % (song->total_note / 10) == 0) {
				printf("%u0%% ", (unsigned int)note_cnt / (song->total_note / 10));
			}
		}
	}
	printf("\ndone.\n");
}

static double
calculate_offset_usec(SongData *song, Tick_t tick)
{
	double     offset_usec = 0;
	TempoData *tmp_tempo   = song->tempos;

	while (tmp_tempo) {
		double usec_per_tick = tmp_tempo->usec_per_beat / song->tick_per_beat;

		if (!tmp_tempo->next) {
			offset_usec += usec_per_tick * (tick - tmp_tempo->offset);
			break;
		} else {
			if (tmp_tempo->next->offset > tick) {
				offset_usec += usec_per_tick * (tick - tmp_tempo->offset);
				break;
			} else {
				offset_usec += usec_per_tick * (tmp_tempo->next->offset - tmp_tempo->offset);
			}
		}
		tmp_tempo = tmp_tempo->next;
	}

	return offset_usec;
}

static PCMSample_t
convert_usec_to_sample(double offset_usec, PCMSample_t sampling_rate)
{
	return offset_usec * sampling_rate / (1000 * 1000);
}

static Freq_t
convert_note_to_freq(unsigned char note)
{
	return 440 * pow(2, (double)(note - 69) / 12);
}
