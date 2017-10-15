#ifndef WAVE_FILE_H
#define WAVE_FILE_H

#define STR_LEN 4

#define RIFF_STR "RIFF"
#define WAVE_STR "WAVE"
#define FMT_STR  "fmt "
#define DATA_STR "data"

typedef struct 
{
	char riff_str[STR_LEN];
	long filesize;
	char wave_str[STR_LEN];
} waveHdr;

typedef struct
{
	char  fmt_str[STR_LEN];
	long  chunk_size;
	short fmt_code;
	short channels;
	long  sampling_rate;
	long  byte_per_sec;
	short block_size;
	short bit_depth;
} waveFmtChunk;

typedef struct
{
	char data_str[STR_LEN];
	long chunk_size;
} waveDataChunk;

void
generate_wave_file(const long sampling_rate,
				   const short bit_depth,
				   const PCM_Data *out_pcm,
				   const unsigned long total_sample);

#endif
