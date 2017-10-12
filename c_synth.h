#ifndef C_SYNTH_H
#define C_SYNTH_H

typedef struct 
{
	char riff_str[STR_LEN];
	long filesize;
	char wave_str[STR_LEN];
} waveHdr;

typedef struct
{
	char fmt_str[STR_LEN];
	long fmtsize;
	short fmtcode;
	short channels;
	long samplingrate;
	long byte_per_sec;
	short block_size;
	short bits;
} waveFmtChunk;

#define STR_LEN 4
#define RIFF_STR "RIFF"
#define WAVE_STR "WAVE"
#define FMT_STR  "fmt "
#define DATA_STR "data"

#define DEF_FORMAT_CODE 1
#define DEF_CHANNEL_NUM 2
#define DEF_SAMPLING_RATE 44100
#define DEF_BIT_DEPTH 16

#endif
