#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "smf_file.h"

void
sequencer_playback(SongData *song, PCM_Data *out_pcm);

#endif
