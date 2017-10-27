#ifndef WAVE_FILE_H
#define WAVE_FILE_H

#pragma pack(1)

#define WAVE_STR_LEN 4

#define RIFF_STR "RIFF"
#define WAVE_STR "WAVE"
#define FMT_STR  "fmt "
#define DATA_STR "data"

typedef struct 
{
	char          riff_str[WAVE_STR_LEN];
	unsigned long filesize;
	char          wave_str[WAVE_STR_LEN];
} WaveHdr;

typedef struct
{
	char           fmt_str[WAVE_STR_LEN];
	unsigned long  chunk_size;
	unsigned short fmt_code;
	unsigned short channels;
	unsigned long  sampling_rate;
	unsigned long  byte_per_sec;
	unsigned short block_size;
	unsigned short bit_depth;
} WaveFmtChunk;

typedef struct
{
	char          data_str[WAVE_STR_LEN];
	unsigned long chunk_size;
} WaveDataChunk;

void
generate_wave_file(PCM_Data *out_pcm);

#endif
