#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
	struct metaplugin;
};
typedef struct zzub::mixer zzub_mixer_t;
#include <string>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <zzub/signature.h>
#include <zzub/plugin.h>
#include "../../mixing/mixer.h"
#include <stdio.h>
#include "miditime.h"

using namespace std;

namespace miditime {

#define MAKEMIDI(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))

miditimemachine::miditimemachine() {
	global_values = &gval;
	track_values = NULL;
	attributes = NULL;
}

void miditimemachine::init(zzub::archive * const pi) {
	playing = 0;
	last_play_pos = 0;
}

void miditimemachine::process_interval() {
	midi_tick();
}
 
int miditimemachine::get_interval_size() {
	int numsamples;
	float samples_per_tick = (float)_master_info->samples_per_tick + _master_info->samples_per_tick_frac;

	// should only pulse when tick_position == 0, if not, sync to the next tick
	if (_master_info->tick_position != 0) {
		// find how many samples until next tick
		numsamples = (int)ceil(samples_per_tick - ((float)_master_info->tick_position + _master_info->tick_position_frac));
		// if the tick was terminated prematurely, e.g via tempo change, sync to the next sample
		return (numsamples > 0) ? numsamples : 1;
	}

	numsamples = (int)floor(samples_per_tick + _master_info->tick_position_frac);

	assert(numsamples > 0);
	return numsamples;
}

void miditimemachine::midi_tick() {
	int message;
	int sent_playpos = 0;

	if((_mixer->state == zzub::player_state_playing)) {
		if(!playing) {
			int play_pos = _master_info->row_position * 6 / _master_info->ticks_per_beat;
			message = MAKEMIDI(0xF2, play_pos & 0x7F, (play_pos >> 7) & 0x7F); // song position
			midi_out(0, message);

			message = MAKEMIDI(0xFB, 0, 0); // continue
			midi_out(0, message);

			playing = 1;
			sent_playpos = 1;
			last_play_pos = _master_info->row_position;
		}
	}

	if(!(_mixer->state == zzub::player_state_playing)) {
		if(playing) {
			message = MAKEMIDI(0xFC, 0, 0); // stop
			midi_out(0, message);

			playing = 0;
		}
	}

	if(playing && !sent_playpos) {
		if(last_play_pos + 1 != _master_info->row_position) {
			int play_pos = _master_info->row_position * 6 / _master_info->ticks_per_beat;
			message = MAKEMIDI(0xF2, play_pos & 0x7F, (play_pos >> 7) & 0x7F); // song position
			midi_out(0, message);

			last_play_pos = _master_info->row_position;
		} else {
			last_play_pos++;
		}
	}

	if(playing) {

		//printf("tick\n");
		message = MAKEMIDI(0xF8, 0, 0); // tick
		midi_out(0, message);

		//printf("%d\n", ts);

		int tick_count = 24/_master_info->ticks_per_beat;
		for(int i = 1; i < tick_count; i++) {
			message = MAKEMIDI(0xF8, 0, 0); // tick
			int time = (i * _master_info->samples_per_tick) / tick_count;
			midi_out(time, message);
			//printf("%d\n", time);
		}
	
	}
}

void miditimemachine::midi_out(int time, unsigned int data) {
	zzub::midi_message msg = { -1, data, time };
	_mixer->midi_out(_id, msg);
}

}
