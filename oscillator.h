#ifndef OSCILLATOR_H
#define OSCILLATOR_H

typedef double Freq_t;

typedef struct 
{
	Freq_t        osc_freq;
	unsigned int  osc_gain;
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
