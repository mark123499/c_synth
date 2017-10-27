#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "c_synth.h"
#include "wave_file.h"
#include "oscillator.h"
#include "smf_file.h"

int main(int argc, char *argv[]) {
	SongData *song = NULL;
	PCM_Data  out_pcm;

	out_pcm.sampling_rate = DEF_SAMPLING_RATE;

	song = load_smf_file("./kanon_small.mid");
	smf_free_song_data(song);

	if (song) {
		smf_free_song_data(song);
	}
	free(out_pcm.pcm);
}
