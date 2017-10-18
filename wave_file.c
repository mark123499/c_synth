#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_synth.h"
#include "wave_file.h"

static waveHdr*
create_riff_header(long datasize);
static waveFmtChunk*
create_format_chunk(long sampling_rate);
static waveDataChunk*
create_data_chunk(long datasize);

void
generate_wave_file(const long sampling_rate,
				   PCM_Data *out_pcm)
{
	waveHdr       *wave_hdr   = NULL;
	waveFmtChunk  *fmt_chunk  = NULL;
	waveDataChunk *data_chunk = NULL;
	FILE          *fpw        = NULL;
	long           total_byte = 0;
	unsigned int   sample_idx = 0;
	unsigned int   ch_idx     = 0;

	fmt_chunk = create_format_chunk(sampling_rate);

	total_byte = fmt_chunk->block_size * out_pcm->total_sample;

	wave_hdr = create_riff_header(total_byte);
	data_chunk = create_data_chunk(total_byte);

	fpw = fopen("pcmout.wav", "wb");
	if (fpw == NULL) {
		printf("fopen error!");
		goto fail;
	}

	fwrite(wave_hdr, sizeof(waveHdr), 1, fpw);
	fwrite(fmt_chunk, sizeof(waveFmtChunk), 1, fpw);
	fwrite(data_chunk, sizeof(waveDataChunk), 1, fpw);

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
create_format_chunk(long sampling_rate)
{
	waveFmtChunk *fmt = NULL;
	fmt = malloc(sizeof(waveFmtChunk));
	memset(fmt, 0x00, sizeof(waveFmtChunk));

	strncpy(fmt->fmt_str, FMT_STR, STR_LEN);
	fmt->chunk_size += sizeof(waveFmtChunk) - 8;
	fmt->fmt_code = DEF_FORMAT_CODE;
	fmt->channels = DEF_CHANNEL_NUM;
	fmt->bit_depth = DEF_BIT_DEPTH;
	fmt->sampling_rate = sampling_rate;
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
