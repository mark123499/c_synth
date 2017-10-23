#ifndef SMF_FILE_H
#define SMF_FILE_H

#pragma pack(1)

#define _BYTE1(x) (  x        & 0xFF )
#define _BYTE2(x) ( (x >>  8) & 0xFF )
#define _BYTE3(x) ( (x >> 16) & 0xFF )
#define _BYTE4(x) ( (x >> 24) & 0xFF )
 
#define BYTE_SWAP_16(x) \
	((unsigned short)( \
	_BYTE1(x) << 8 | \
	_BYTE2(x)))

#define BYTE_SWAP_32(x) \
	((unsigned long)( \
	_BYTE1(x) << 24 | \
	_BYTE2(x) << 16 | \
	_BYTE3(x) << 8 |  \
	_BYTE4(x)))

#define DELTA_CALC(x) \
	((unsigned long)( \
	(x[0] & 0x7F) << 21 | \
	(x[1] & 0x7F) << 14 | \
	(x[2] & 0x7F) << 7 | \
	(x[3] & 0x7F)))

#define SMF_STR_LEN 4

#define MTHD_STR "MThd"
#define MTRK_STR "MTrk"

#define SMF_MAX_CHANNEL_NUM 16

#define MAX_DELTA_TIME_BYTE 4
#define DELTA_TIME_SHIFT 7
#define DELTA_TIME_CARRY (1 << DELTA_TIME_SHIFT)

#define SMF_STATUS_CH_MASK   0xF0

#define SMF_STATUS_NOTE_OFF  0x80
#define SMF_STATUS_NOTE_ON   0x90
#define SMF_STATUS_PROG_CHNG 0xC0
#define SMF_STATUS_META      0xFF

#define SMF_META_END         0x2F
#define SMF_META_TEMPO       0x51

typedef struct 
{
	char           mthd_str[SMF_STR_LEN];
	unsigned long  hdr_size;
	unsigned short format;
	unsigned short track_cnt;
	unsigned short time_type;
} SmfHdr;

typedef struct 
{
	char          mtrk_str[SMF_STR_LEN];
	unsigned long track_size;
} TrackHdr;

typedef struct _NoteData
{
	unsigned long     offset;
	unsigned long     dulation;
	unsigned char     note;
	unsigned char     velocity;
	struct _NoteData *next;
} NoteData;

typedef struct
{
	unsigned char  channel_cnt;
	unsigned long  msec_per_beat;
	unsigned long  time_per_beat;
	unsigned char  sound[SMF_MAX_CHANNEL_NUM];
	NoteData      *notes[SMF_MAX_CHANNEL_NUM];
} SongData;

SongData *
load_smf_file(const char *smf_path);
void
smf_free_song_data(SongData *song);

#endif
