#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "c_synth.h"
#include "oscillator.h"

void
generate_sine_wave(OSC_Params *osc_param,
				   PCM_Data *out_pcm)
{
	double      sine_wave_seed = 0;
	PCMSample_t sample_idx     = 0;

	sine_wave_seed = 2 * M_PI * osc_param->osc_freq /
		out_pcm->sampling_rate;

	for (sample_idx = 0; sample_idx < out_pcm->total_sample; sample_idx++) {
		out_pcm->pcm[sample_idx].pcm_l +=
			osc_param->osc_gain * sin(sine_wave_seed * sample_idx);
		out_pcm->pcm[sample_idx].pcm_r +=
			osc_param->osc_gain * sin(sine_wave_seed * sample_idx);
	}
}

void
generate_square_wave(OSC_Params *osc_param,
					 PCM_Data *out_pcm)
{
	double      square_wave_seed = 0;
	PCMSample_t sample_idx       = 0;

	square_wave_seed = 2 * M_PI * osc_param->osc_freq /
		out_pcm->sampling_rate;

	for (sample_idx = 0; sample_idx < out_pcm->total_sample; sample_idx++) {
		unsigned int harmonic_idx = 1;

		while(1) {
			unsigned int harmonic_factor = 2 * harmonic_idx - 1;

			if (harmonic_factor * osc_param->osc_freq >
				out_pcm->sampling_rate / 2) {
				break;
			}

			out_pcm->pcm[sample_idx].pcm_l += osc_param->osc_gain *
				sin(harmonic_factor * square_wave_seed * sample_idx) /
				harmonic_factor;
			out_pcm->pcm[sample_idx].pcm_r += osc_param->osc_gain *
				sin(harmonic_factor * square_wave_seed * sample_idx) /
				harmonic_factor;

			harmonic_idx++;
		}
	}
}

void
generate_saw_wave(OSC_Params *osc_param,
				  PCM_Data *out_pcm)
{
	double      saw_wave_seed = 0;
	PCMSample_t sample_idx    = 0;

	saw_wave_seed = 2 * M_PI * osc_param->osc_freq /
		out_pcm->sampling_rate;

	for (sample_idx = 0; sample_idx < out_pcm->total_sample; sample_idx++) {
		unsigned int harmonic_idx = 1;

		while(1) {
			if (harmonic_idx * osc_param->osc_freq >
				out_pcm->sampling_rate / 2) {
				break;
			}

			out_pcm->pcm[sample_idx].pcm_l += osc_param->osc_gain *
				sin(harmonic_idx * saw_wave_seed * sample_idx) /
				harmonic_idx;
			out_pcm->pcm[sample_idx].pcm_r += osc_param->osc_gain *
				sin(harmonic_idx * saw_wave_seed * sample_idx) /
				harmonic_idx;

			harmonic_idx++;
		}
	}
}
