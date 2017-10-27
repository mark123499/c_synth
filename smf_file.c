#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smf_file.h"

static SongData *smf_init_song_data(void);
static Tick_t smf_calc_song_dulation(SongData *song);
static unsigned char smf_extract_delta_time(FILE *fp,
											Tick_t *delta_time);
static unsigned char smf_extract_midi_event(FILE *fp, SongData *song,
											Tick_t offset);

/* Temporary Functions */
static void mem_dump(void* ptr, int counts);

/* Wrapper Functions */
#define FREAD(a, b, c, d, e) fread(a,b,c,d);e++;

SongData *
load_smf_file(const char *smf_path)
{
	FILE         *fpr       = NULL;
	SongData     *song      = NULL;
	SmfHdr        smf_hdr;
	unsigned int  track_idx = 0;

	song = smf_init_song_data();
	memset(&smf_hdr, 0x00, sizeof(SmfHdr));

	fpr = fopen(smf_path, "rb");
	if (fpr == NULL) {
		printf("fopen error!");
		goto fail;
	}

	fread(&smf_hdr, sizeof(SmfHdr), 1, fpr);

	if (strncmp(smf_hdr.mthd_str, MTHD_STR, SMF_STR_LEN) != 0) {
		printf("Invalid SMF file input.\n");
		fclose(fpr);
		goto fail;
	}

	smf_hdr.hdr_size = BYTE_SWAP_32(smf_hdr.hdr_size);
	smf_hdr.format = BYTE_SWAP_16(smf_hdr.format);
	smf_hdr.track_cnt = BYTE_SWAP_16(smf_hdr.track_cnt);
	smf_hdr.time_type = BYTE_SWAP_16(smf_hdr.time_type);

	for (track_idx = 0; track_idx < smf_hdr.track_cnt; track_idx++) {
		TrackHdr      track_hdr;
		Tick_t        offset         = 0;
		Tick_t        delta_time     = 0;
		unsigned char load_byte      = 0;
		unsigned long processed_byte = 0;

		memset(&track_hdr, 0x00, sizeof(TrackHdr));

		fread(&track_hdr, sizeof(TrackHdr), 1, fpr);


		if (strncmp(track_hdr.mtrk_str, MTRK_STR, SMF_STR_LEN) != 0) {
			printf("Invalid track.\n");
			fclose(fpr);
			goto fail;
		}
		track_hdr.track_size = BYTE_SWAP_32(track_hdr.track_size);

		do {
			load_byte = smf_extract_delta_time(fpr, &delta_time);
			processed_byte += load_byte;
			offset += delta_time;
			load_byte = smf_extract_midi_event(fpr, song, offset);
			processed_byte += load_byte;
			if (load_byte == 0) {
				printf("Invalid track. idx:%u\n", track_idx);
				fclose(fpr);
				goto fail;
			}
		} while (processed_byte < track_hdr.track_size);
	}

	song->tick_per_beat = smf_hdr.time_type;
	song->total_tick = smf_calc_song_dulation(song);

	if (0) {
		unsigned int ch_idx = 0;

		printf("hdr_size:%lu format:%u track_cnt:%u time_type:%u\n",
			   smf_hdr.hdr_size, smf_hdr.format,
			   smf_hdr.track_cnt, smf_hdr.time_type);
		printf("usec_per_beat=%u\n", song->usec_per_beat);

		for (ch_idx = 0; ch_idx < SMF_MAX_CHANNEL_NUM; ch_idx++) {
			NoteData *head = song->notes[ch_idx];

			printf("\nDump notes ch:%u\n", ch_idx);
			while (head != NULL) {
				printf("offset %5lu, dulation %5lu, note %2x, velocity %3u\n",
					   head->offset, head->dulation, head->note, head->velocity);
				head = head->next;
			}
		}
	}

	fclose(fpr);

	return song;

fail:
	smf_free_song_data(song);

	return NULL;
}

void
smf_free_song_data(SongData *song)
{
	unsigned int  ch_idx = 0;

	for (ch_idx = 0; ch_idx < SMF_MAX_CHANNEL_NUM; ch_idx++) {
		NoteData *head = song->notes[ch_idx];
		NoteData *next;

		while (head) {
			next = head->next;
			free(head);
			head = next;
		}
	}

	free(song);
}


/* Private Functions */
static SongData *
smf_init_song_data(void)
{
	SongData     *song   = NULL;
	unsigned int  ch_idx = 0;

	song = malloc(sizeof(SongData));
	memset(song, 0x00, sizeof(SongData));

	return song;
}

static Tick_t
smf_calc_song_dulation(SongData *song)
{
	unsigned int ch_idx         = 0;
	Tick_t       max_end_offset = 0;

	for (ch_idx = 0; ch_idx < SMF_MAX_CHANNEL_NUM; ch_idx++) {
		NoteData *head = song->notes[ch_idx];

		while (head != NULL) {
			Tick_t end_offset = head->offset + head->dulation;

			if (end_offset > max_end_offset) {
				max_end_offset = end_offset;
			}
			head = head->next;
		}
	}

	return max_end_offset;
}

static unsigned char
smf_extract_delta_time(FILE *fp, Tick_t *delta_time)
{
	unsigned char delta_idx                  = 0;
	unsigned char delta[MAX_DELTA_TIME_BYTE] = {0};
	unsigned char delta_time_buf             = 0;

	for (delta_idx = 0; delta_idx < MAX_DELTA_TIME_BYTE; delta_idx++) {
		fread(&delta_time_buf, sizeof(unsigned char), 1, fp);
		delta[delta_idx] = delta_time_buf;
		if (delta_time_buf & DELTA_TIME_CARRY) {
			continue;
		}
		break;
	}
	delta_idx++;

	if (delta_idx == 1) {
		delta[3] = delta[0];
		delta[0] = 0;
	} else if (delta_idx == 2) {
		delta[3] = delta[1];
		delta[2] = delta[0];
		delta[1] = 0;
		delta[0] = 0;
	} else if (delta_idx == 3) {
		delta[3] = delta[2];
		delta[2] = delta[1];
		delta[1] = delta[0];
		delta[0] = 0;
	}

	*delta_time = DELTA_TIME(delta);

	return delta_idx;
}

static unsigned char
smf_extract_midi_event(FILE *fp, SongData *song, Tick_t offset)
{
	unsigned char event_buf = 0;
	unsigned char load_byte = 0;

	FREAD(&event_buf, sizeof(unsigned char), 1, fp, load_byte);

	if (event_buf == SMF_STATUS_META) {
		FREAD(&event_buf, sizeof(unsigned char), 1, fp, load_byte);
		switch (event_buf) {
			case SMF_META_TEMPO:
				FREAD(&event_buf, sizeof(unsigned char), 1, fp, load_byte);
				if (event_buf != 3) {
					printf("Invalid MIDI event format: Tempo\n");
					return 0;
				}
				FREAD(&event_buf, sizeof(unsigned char), 1, fp, load_byte);
				song->usec_per_beat |= event_buf << 16;
				FREAD(&event_buf, sizeof(unsigned char), 1, fp, load_byte);
				song->usec_per_beat |= event_buf << 8;
				FREAD(&event_buf, sizeof(unsigned char), 1, fp, load_byte);
				song->usec_per_beat |= event_buf;
				break;

			case SMF_META_END:
				FREAD(&event_buf, sizeof(unsigned char), 1, fp, load_byte);
				if (event_buf != 0) {
					printf("Invalid MIDI event format: Track END\n");
					return 0;
				}
				break;

			default:
				printf("Unknown MIDI event\n");
				return 0;
		}
	} else {
		switch(event_buf & SMF_STATUS_CH_MASK) {
			case SMF_STATUS_NOTE_OFF: {
				unsigned char  channel  = event_buf & ~SMF_STATUS_CH_MASK;
				unsigned char  note     = 0;
				unsigned char  velocity = 0;

				FREAD(&note, sizeof(unsigned char), 1, fp, load_byte);
				FREAD(&velocity, sizeof(unsigned char), 1, fp, load_byte);

				if (song->notes[channel]) {
					NoteData *head = song->notes[channel];

					while (head != NULL) {
						if (head->note == note && head->dulation == 0) {
							break;
						}
						head = head->next;
					}
					if (head) {
						head->dulation = offset - head->offset;
					} else {
						printf("No target note exist\n");
						return 0;
					}
				} else {
					printf("No target note exist\n");
					return 0;
				}

				break;
			}

			case SMF_STATUS_NOTE_ON: {
				unsigned char  channel  = event_buf & ~SMF_STATUS_CH_MASK;
				unsigned char  note     = 0;
				unsigned char  velocity = 0;
				NoteData      *new_note = malloc(sizeof(NoteData));

				memset(new_note, 0, sizeof(NoteData));

				FREAD(&note, sizeof(unsigned char), 1, fp, load_byte);
				FREAD(&velocity, sizeof(unsigned char), 1, fp, load_byte);

				new_note->offset = offset;
				new_note->note = note;
				new_note->velocity = velocity;

				if (song->notes[channel]) {
					NoteData *head = song->notes[channel];

					new_note->next = head;
					song->notes[channel] = new_note;
				} else {
					song->notes[channel] = new_note;
				}

				break;
			}

			case SMF_STATUS_PROG_CHNG: {
				unsigned char channel = event_buf & ~SMF_STATUS_CH_MASK;
				unsigned char sound   = 0;

				FREAD(&sound, sizeof(unsigned char), 1, fp, load_byte);
				song->sound[channel] = sound;

				break;
			}

			default:
				printf("Unknown MIDI event\n");
				return 0;
		}
	}

	return load_byte;
}

/* Temporary Functions */
static void
mem_dump(void* ptr, int counts)
{
	int i;
	unsigned __int64 *ulong_ptr = (unsigned __int64 *)ptr;
	unsigned char uchar_buf[8];
	printf("--------------------------------------------------------\n");
	printf(" Address | HEX data | +0 +1 +2 +3 +4 +5 +6 +7 | 01234567\n");
	printf("--------------------------------------------------------\n");
	for(i=0; i<counts; i++) {
		printf(" %08x| %08x", &ulong_ptr[i], ulong_ptr[i]);
		memcpy(uchar_buf, &ulong_ptr[i], sizeof(uchar_buf));
		printf(" | %02x %02x %02x %02x %02x %02x %02x %02x",
		uchar_buf[0], uchar_buf[1], uchar_buf[2], uchar_buf[3],
		uchar_buf[4], uchar_buf[5], uchar_buf[6], uchar_buf[7]);
		if(uchar_buf[0]<32 || uchar_buf[0]>126) uchar_buf[0] = '.';
		if(uchar_buf[1]<32 || uchar_buf[1]>126) uchar_buf[1] = '.';
		if(uchar_buf[2]<32 || uchar_buf[2]>126) uchar_buf[2] = '.';
		if(uchar_buf[3]<32 || uchar_buf[3]>126) uchar_buf[3] = '.';
		if(uchar_buf[4]<32 || uchar_buf[4]>126) uchar_buf[4] = '.';
		if(uchar_buf[5]<32 || uchar_buf[5]>126) uchar_buf[5] = '.';
		if(uchar_buf[6]<32 || uchar_buf[6]>126) uchar_buf[6] = '.';
		if(uchar_buf[7]<32 || uchar_buf[7]>126) uchar_buf[7] = '.';

		printf(" | %c%c%c%c%c%c%c%c\n",
		uchar_buf[0], uchar_buf[1], uchar_buf[2], uchar_buf[3],
		uchar_buf[4], uchar_buf[5], uchar_buf[6], uchar_buf[7]);
	}
	printf("--------------------------------------------------------\n");
}
