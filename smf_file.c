#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smf_file.h"

static SongData *smf_init_song_data(void);
static void smf_calc_song_statistics(SongData *song);
static unsigned char smf_extract_delta_time(FILE *fp,
											Tick_t *delta_time);
static unsigned int smf_extract_midi_event(FILE *fp, SongData *song,
										   Tick_t offset);

/* Temporary Functions */
static void mem_dump(void* ptr, int counts);

/* Wrapper Functions */
#define SMF_FREAD(a, b, c) fread(a,sizeof(unsigned char),1,b);c++;

/* Variables */
static unsigned char prev_event; /* For running status */

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
				printf("SMF parse error track_idx:%u processed_byte:%lu\n",
					   track_idx, processed_byte);
				fclose(fpr);
				goto fail;
			}
		} while (processed_byte < track_hdr.track_size);
	}

	song->tick_per_beat = smf_hdr.time_type;
	smf_calc_song_statistics(song);
	song->tempos->offset = 0;

	if (0) {
		unsigned int  ch_idx = 0;
		TempoData    *tmp_tempo = song->tempos;

		printf("hdr_size:%lu format:%u track_cnt:%u time_type:%u\n",
			   smf_hdr.hdr_size, smf_hdr.format,
			   smf_hdr.track_cnt, smf_hdr.time_type);

		while(tmp_tempo) {
			printf("offset: %5lu, usec_per_beat: %u\n",
				   tmp_tempo->offset, tmp_tempo->usec_per_beat);
			tmp_tempo = tmp_tempo->next;
		}

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
	unsigned int ch_idx = 0;

	for (ch_idx = 0; ch_idx < SMF_MAX_CHANNEL_NUM; ch_idx++) {
		NoteData *head = song->notes[ch_idx];
		NoteData *next;

		while (head) {
			next = head->next;
			free(head);
			head = next;
		}
	}

	while(song->tempos) {
		TempoData *tmp_tempo = song->tempos;
		song->tempos = tmp_tempo->next;

		free(tmp_tempo);
	}

	free(song);
}


/* Private Functions */
static SongData *
smf_init_song_data(void)
{
	SongData     *song   = NULL;
	unsigned int  ch_idx = 0;

	song = calloc(1, sizeof(SongData));

	return song;
}

static void
smf_calc_song_statistics(SongData *song)
{
	unsigned int  ch_idx         = 0;
	Tick_t        max_end_offset = 0;
	unsigned long note_cnt       = 0;

	for (ch_idx = 0; ch_idx < SMF_MAX_CHANNEL_NUM; ch_idx++) {
		NoteData *head = song->notes[ch_idx];

		while (head != NULL) {
			Tick_t end_offset = head->offset + head->dulation;

			if (end_offset > max_end_offset) {
				max_end_offset = end_offset;
			}
			head = head->next;
			note_cnt++;
		}
	}

	song->total_tick = max_end_offset;
	song->total_note = note_cnt;
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

static unsigned int
smf_extract_midi_event(FILE *fp, SongData *song, Tick_t offset)
{
	unsigned char event_buf = 0;
	unsigned int  load_byte = 0;

	SMF_FREAD(&event_buf, fp, load_byte);

	/* Check running status */
	if (!(event_buf & 0x80)) {
		fseek(fp, -1, SEEK_CUR);
		load_byte--;
		event_buf = prev_event;
	}

	/* Store event for running status */
	prev_event = event_buf;

	switch (event_buf) {
		case SMF_EVENT_SYSEX_F0:
		case SMF_EVENT_SYSEX_F7: {
			unsigned char length = 0;

			SMF_FREAD(&length, fp, load_byte);
			while (length) {
				SMF_FREAD(&event_buf, fp, load_byte);
				length--;
			}

			break;
		}

		case SMF_EVENT_META:
			SMF_FREAD(&event_buf, fp, load_byte);
			switch (event_buf) {
				case SMF_META_TEXT:
				case SMF_META_COPYRIGHT:
				case SMF_META_NAME:
				case SMF_META_PORT:
				case SMF_META_BEAT:
				case SMF_META_KEY: {
					unsigned char length = 0;

					SMF_FREAD(&length, fp, load_byte);
					while (length) {
						SMF_FREAD(&event_buf, fp, load_byte);
						length--;
					}

					break;
				}

				case SMF_META_END:
					SMF_FREAD(&event_buf, fp, load_byte);
					if (event_buf != 0) {
						printf("Invalid MIDI event format: Track END\n");
						return 0;
					}
					break;

				case SMF_META_TEMPO: {
					unsigned long  usec_per_beat = 0;
					TempoData     *new_tempo     = NULL;

					SMF_FREAD(&event_buf, fp, load_byte);
					if (event_buf != 3) {
						printf("Invalid MIDI event format: Tempo\n");
						return 0;
					}
					SMF_FREAD(&event_buf, fp, load_byte);
					usec_per_beat |= event_buf << 16;
					SMF_FREAD(&event_buf, fp, load_byte);
					usec_per_beat |= event_buf << 8;
					SMF_FREAD(&event_buf, fp, load_byte);
					usec_per_beat |= event_buf;

					new_tempo = calloc(1, sizeof(TempoData));
					new_tempo->offset = offset;
					new_tempo->usec_per_beat = usec_per_beat;

					if (!song->tempos) {
						song->tempos = new_tempo;
					} else {
						if (song->tempos->offset > offset) {
							new_tempo->next = song->tempos;
							song->tempos = new_tempo;
						} else {
							TempoData *tmp_tempo = song->tempos;
							while (tmp_tempo) {
								if (!tmp_tempo->next) {
									tmp_tempo->next = new_tempo;
									break;
								}
								if (tmp_tempo->next->offset > offset) {
									new_tempo->next = tmp_tempo->next;
									tmp_tempo->next = new_tempo;
									break;
								}
								tmp_tempo = tmp_tempo->next;
							}
						}
					}
					break;
				}

				default:
					printf("Unknown MIDI meta event %X\n", event_buf);
					return 0;
			}
			break;

		default:
			switch(event_buf & SMF_EVENT_CH_MASK) {
				case SMF_EVENT_NOTE_OFF: {
					unsigned char channel  = event_buf & ~SMF_EVENT_CH_MASK;
					unsigned char note     = 0;
					unsigned char velocity = 0;

					SMF_FREAD(&note, fp, load_byte);
					SMF_FREAD(&velocity, fp, load_byte);

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

				case SMF_EVENT_NOTE_ON: {
					unsigned char  channel  = event_buf & ~SMF_EVENT_CH_MASK;
					unsigned char  note     = 0;
					unsigned char  velocity = 0;
					NoteData      *new_note = NULL;

					SMF_FREAD(&note, fp, load_byte);
					SMF_FREAD(&velocity, fp, load_byte);

					if (!velocity) {
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
								/* Processed note OFF event.
								 * Gonna break from switch statement. */
								break;
							}
						}
					}

					new_note = calloc(1, sizeof(NoteData));

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

				case SMF_EVENT_CTL_CHG: {
					unsigned char channel              = event_buf & ~SMF_EVENT_CH_MASK;
					unsigned char control_number       = 0;
					static unsigned char prev_ch       = 0;
					static unsigned char prev_ctrl_num = 0;

					SMF_FREAD(&control_number, fp, load_byte);
					if (control_number >= SMF_CH_MODE_MSG_BDR) {
						if (channel == prev_ch &&
							prev_ctrl_num == 0x7C &&
							control_number == 0x7E) {
							SMF_FREAD(&event_buf, fp, load_byte);
						}
					}
					SMF_FREAD(&event_buf, fp, load_byte);
					prev_ch = channel;
					prev_ctrl_num = control_number;

					break;
				}

				case SMF_EVENT_PROG_CHNG: {
					unsigned char channel = event_buf & ~SMF_EVENT_CH_MASK;
					unsigned char sound   = 0;

					SMF_FREAD(&sound, fp, load_byte);
					song->sound[channel] = sound;

					break;
				}

				case SMF_EVENT_PITCH_BEND:
					SMF_FREAD(&event_buf, fp, load_byte);
					SMF_FREAD(&event_buf, fp, load_byte);

					break;

				default:
					printf("Unknown MIDI event %X\n", event_buf);
					return 0;
			}
			break;
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
