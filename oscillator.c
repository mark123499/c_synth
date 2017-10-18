#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "c_synth.h"
#include "oscillator.h"

void
generate_sine_wave(OSC_Params *osc_param,
				   PCM_Data *out_pcm)
{
	double       sine_wave_seed = 0;
	unsigned int sample_idx     = 0;

	sine_wave_seed = 2 * M_PI * osc_param->osc_freq;
	sine_wave_seed /= osc_param->sampling_rate;

	for (sample_idx = 0; sample_idx < out_pcm->total_sample; sample_idx++) {
		out_pcm->pcm[sample_idx].pcm_l +=
			osc_param->osc_gain * sin(sine_wave_seed * sample_idx);
		out_pcm->pcm[sample_idx].pcm_r +=
			osc_param->osc_gain * sin(sine_wave_seed * sample_idx);
	}
}
