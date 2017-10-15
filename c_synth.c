#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "c_synth.h"
#include "wave_file.h"

int main(int argc, char *argv[]) {
	long           sampling_rate = DEF_SAMPLING_RATE;
	unsigned long  total_sample  = 0;
	unsigned int   sample_idx    = 0;
	double         sin_wave_seed = 0;
	PCM_Data      *out_pcm       = NULL;

	unsigned int   sin_wave_freq = 440;
	double         sin_wave_gain = 3000;
	unsigned int   total_sec     = 5;

	total_sample = sampling_rate * total_sec;
	out_pcm = calloc(total_sample, sizeof(PCM_Data));

	sin_wave_seed = 2 * M_PI * sin_wave_freq / sampling_rate;
	for (sample_idx = 0; sample_idx < total_sample; sample_idx++) {
		out_pcm[sample_idx].pcm_l +=
			sin_wave_gain * sin(sin_wave_seed * sample_idx);
		out_pcm[sample_idx].pcm_r +=
			sin_wave_gain * sin(sin_wave_seed * sample_idx);
	}

	generate_wave_file(sampling_rate, DEF_BIT_DEPTH,
					   out_pcm, total_sample);

	free(out_pcm);
}
