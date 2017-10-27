#ifndef C_SYNTH_H
#define C_SYNTH_H

#define DEF_FORMAT_CODE 1
#define DEF_CHANNEL_NUM 2
#define DEF_SAMPLING_RATE 44100
#define DEF_BIT_DEPTH 16

typedef unsigned long PCMSample_t;

typedef struct 
{
	signed short pcm_l;
	signed short pcm_r;
} PCM_Block;

typedef struct
{
	PCM_Block   *pcm;
	PCMSample_t  total_sample;
	PCMSample_t  sampling_rate;
} PCM_Data;

#endif
