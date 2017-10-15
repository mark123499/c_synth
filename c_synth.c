#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "c_synth.h"

static waveHdr*
create_riff_header(long datasize);
static waveFmtChunk*
create_format_chunk(long sampling_rate, short bit_depth);
static waveDataChunk*
create_data_chunk(long datasize);

int main(int argc,char *argv[]) {
	waveHdr       *wave_hdr      = NULL;
	waveFmtChunk  *fmt_chunk     = NULL;
	waveDataChunk *data_chunk    = NULL;
	FILE          *fpw           = NULL;
	long           sampling_rate = DEF_SAMPLING_RATE;
	short          bit_depth     = DEF_BIT_DEPTH;
	long           total_byte    = 0;
	unsigned long  total_sample  = 0;
	unsigned int   sample_idx    = 0;
	unsigned int   channel_idx   = 0;
	double         sin_wave_seed = 0;
	signed short  *out_pcm[DEF_CHANNEL_NUM];

	unsigned int   sin_wave_freq = 440;
	double         sin_wave_gain = 3000;
	unsigned int   total_sec     = 5;

	total_sample = sampling_rate * total_sec;
	for (channel_idx = 0; channel_idx < DEF_CHANNEL_NUM; channel_idx++) {
		out_pcm[channel_idx] = calloc(total_sample, sizeof(signed short));
	}

	fmt_chunk = create_format_chunk(sampling_rate, bit_depth);

	total_byte = fmt_chunk->byte_per_sec * total_sec;

	wave_hdr = create_riff_header(total_byte);
	data_chunk = create_data_chunk(total_byte);

	fpw = fopen("pcmout.wav", "wb");
	if (fpw == NULL) {
		printf("fopen error!");
		return 0;
	}

	fwrite(wave_hdr, sizeof(waveHdr), 1, fpw);
	fwrite(fmt_chunk, sizeof(waveFmtChunk), 1, fpw);
	fwrite(data_chunk, sizeof(waveDataChunk), 1, fpw);

	sin_wave_seed = 2 * M_PI * sin_wave_freq / sampling_rate;
	for (sample_idx = 0; sample_idx < total_sample; sample_idx++) {
		out_pcm[0][sample_idx] +=
			sin_wave_gain * sin(sin_wave_seed * sample_idx);
		out_pcm[1][sample_idx] +=
			sin_wave_gain * sin(sin_wave_seed * sample_idx);
	}

	for (sample_idx = 0; sample_idx < total_sample; sample_idx++) {
		fwrite(&out_pcm[0][sample_idx], sizeof(signed short), 1, fpw);
		fwrite(&out_pcm[1][sample_idx], sizeof(signed short), 1, fpw);
	}

	fclose(fpw);
	free(wave_hdr);
	free(fmt_chunk);
	free(data_chunk);
	for (channel_idx = 0; channel_idx < DEF_CHANNEL_NUM; channel_idx++) {
		free(out_pcm[channel_idx]);
	}
}

static waveHdr*
create_riff_header(long datasize)
{
	waveHdr *wave_hdr = NULL;

	wave_hdr = malloc(sizeof(waveHdr));
	memset(wave_hdr, 0x00, sizeof(waveHdr));

	strncpy(wave_hdr->riff_str, RIFF_STR, STR_LEN);
	wave_hdr->filesize += sizeof(waveHdr) - 8;
	wave_hdr->filesize += sizeof(waveFmtChunk);
	wave_hdr->filesize += sizeof(waveDataChunk);
	wave_hdr->filesize += datasize;
	strncpy(wave_hdr->wave_str, WAVE_STR, STR_LEN);

	return wave_hdr;
}

static waveFmtChunk*
create_format_chunk(long sampling_rate, short bit_depth)
{
	waveFmtChunk *fmt = NULL;
	fmt = malloc(sizeof(waveFmtChunk));
	memset(fmt, 0x00, sizeof(waveFmtChunk));

	strncpy(fmt->fmt_str, FMT_STR, STR_LEN);
	fmt->chunk_size += sizeof(waveFmtChunk) - 8;
	fmt->fmt_code = DEF_FORMAT_CODE;
	fmt->channels = DEF_CHANNEL_NUM;
	fmt->sampling_rate = sampling_rate;
	fmt->bit_depth = bit_depth;
	fmt->block_size = (fmt->bit_depth / 8) * fmt->channels;
	fmt->byte_per_sec = fmt->sampling_rate * fmt->block_size;

	return fmt;
}

static waveDataChunk*
create_data_chunk(long datasize)
{
	waveDataChunk *data = NULL;

	data = malloc(sizeof(waveDataChunk));
	memset(data, 0x00, sizeof(waveDataChunk));

	strncpy(data->data_str, DATA_STR, STR_LEN);
	data->chunk_size = datasize;

	return data;
}
