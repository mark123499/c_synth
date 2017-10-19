#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "c_synth.h"
#include "wave_file.h"
#include "oscillator.h"

int main(int argc, char *argv[]) {
	long           sampling_rate = DEF_SAMPLING_RATE;
	unsigned int   total_sec     = 5;
	OSC_Params     sine_wave_param;
	OSC_Params     square_wave_param;
	OSC_Params     saw_wave_param;
	PCM_Data       out_pcm;

	out_pcm.total_sample = sampling_rate * total_sec;
	out_pcm.pcm = calloc(out_pcm.total_sample, sizeof(PCM_Block));

	sine_wave_param.osc_freq = 440;
	sine_wave_param.osc_gain = 20000;
	sine_wave_param.sampling_rate = sampling_rate;

	square_wave_param.osc_freq = 440;
	square_wave_param.osc_gain = 20000;
	square_wave_param.sampling_rate = sampling_rate;

	saw_wave_param.osc_freq = 440;
	saw_wave_param.osc_gain = 15000;
	saw_wave_param.sampling_rate = sampling_rate;

	//generate_sine_wave(&sine_wave_param, &out_pcm);
	//generate_square_wave(&square_wave_param, &out_pcm);
	generate_saw_wave(&saw_wave_param, &out_pcm);
	generate_wave_file(sampling_rate, &out_pcm);

	free(out_pcm.pcm);
}
