#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "c_synth.h"

typedef double Freq_t;

typedef struct 
{
	Freq_t       osc_freq;
	unsigned int osc_gain;
	PCMSample_t  offset;
	PCMSample_t  dulation;
} OSC_Params;

void
generate_sine_wave(OSC_Params *osc_param,
				   PCM_Data *out_pcm);

void
generate_square_wave(OSC_Params *osc_param,
					 PCM_Data *out_pcm);

void
generate_saw_wave(OSC_Params *osc_param,
				  PCM_Data *out_pcm);

#endif
