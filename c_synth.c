#include <stdio.h>
#include <stdlib.h>
#include "c_synth.h"
#include "wave_file.h"
#include "smf_file.h"
#include "sequencer.h"

int main(int argc, char *argv[]) {
	SongData *song = NULL;
	PCM_Data  out_pcm;

	out_pcm.sampling_rate = DEF_SAMPLING_RATE;

	song = load_smf_file("./dokuhaku.mid");
	if (!song) {
		printf("Failed to load SMF file.\n");
		return 0;
	}
	sequencer_playback(song, &out_pcm);
	generate_wave_file(&out_pcm);

	if (song) {
		smf_free_song_data(song);
	}
	free(out_pcm.pcm);
}
