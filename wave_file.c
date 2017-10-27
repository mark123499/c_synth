#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_synth.h"
#include "wave_file.h"

static WaveHdr*
create_riff_header(unsigned long datasize);
static WaveFmtChunk*
create_format_chunk(unsigned long sampling_rate);
static WaveDataChunk*
create_data_chunk(unsigned long datasize);

void
generate_wave_file(PCM_Data *out_pcm)
{
	WaveHdr       *wave_hdr   = NULL;
	WaveFmtChunk  *fmt_chunk  = NULL;
	WaveDataChunk *data_chunk = NULL;
	FILE          *fpw        = NULL;
	unsigned long  total_byte = 0;
	PCMSample_t    sample_idx = 0;
	unsigned int   ch_idx     = 0;

	fmt_chunk = create_format_chunk(out_pcm->sampling_rate);

	total_byte = fmt_chunk->block_size * out_pcm->total_sample;

	wave_hdr = create_riff_header(total_byte);
	data_chunk = create_data_chunk(total_byte);

	fpw = fopen("pcmout.wav", "wb");
	if (fpw == NULL) {
		printf("fopen error!");
		goto fail;
	}

	fwrite(wave_hdr, sizeof(WaveHdr), 1, fpw);
	fwrite(fmt_chunk, sizeof(WaveFmtChunk), 1, fpw);
	fwrite(data_chunk, sizeof(WaveDataChunk), 1, fpw);

	for (sample_idx = 0; sample_idx < out_pcm->total_sample; sample_idx++) {
		fwrite(&out_pcm->pcm[sample_idx], sizeof(PCM_Block), 1, fpw);
	}

	fclose(fpw);

fail:
	free(wave_hdr);
	free(fmt_chunk);
	free(data_chunk);
}


/* Private Functions */
static WaveHdr*
create_riff_header(unsigned long datasize)
{
	WaveHdr *wave_hdr = NULL;

	wave_hdr = malloc(sizeof(WaveHdr));
	memset(wave_hdr, 0x00, sizeof(WaveHdr));

	strncpy(wave_hdr->riff_str, RIFF_STR, WAVE_STR_LEN);
	wave_hdr->filesize += sizeof(WaveHdr) - 8;
	wave_hdr->filesize += sizeof(WaveFmtChunk);
	wave_hdr->filesize += sizeof(WaveDataChunk);
	wave_hdr->filesize += datasize;
	strncpy(wave_hdr->wave_str, WAVE_STR, WAVE_STR_LEN);

	return wave_hdr;
}

static WaveFmtChunk*
create_format_chunk(unsigned long sampling_rate)
{
	WaveFmtChunk *fmt = NULL;
	fmt = malloc(sizeof(WaveFmtChunk));
	memset(fmt, 0x00, sizeof(WaveFmtChunk));

	strncpy(fmt->fmt_str, FMT_STR, WAVE_STR_LEN);
	fmt->chunk_size += sizeof(WaveFmtChunk) - 8;
	fmt->fmt_code = DEF_FORMAT_CODE;
	fmt->channels = DEF_CHANNEL_NUM;
	fmt->bit_depth = DEF_BIT_DEPTH;
	fmt->sampling_rate = sampling_rate;
	fmt->block_size = (fmt->bit_depth / 8) * fmt->channels;
	fmt->byte_per_sec = fmt->sampling_rate * fmt->block_size;

	return fmt;
}

static WaveDataChunk*
create_data_chunk(unsigned long datasize)
{
	WaveDataChunk *data = NULL;

	data = malloc(sizeof(WaveDataChunk));
	memset(data, 0x00, sizeof(WaveDataChunk));

	strncpy(data->data_str, DATA_STR, WAVE_STR_LEN);
	data->chunk_size = datasize;

	return data;
}
