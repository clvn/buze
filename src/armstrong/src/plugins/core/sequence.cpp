#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
};
typedef struct zzub::mixer zzub_mixer_t;

#include <zzub/plugin.h>
#include <sstream>
#include <cassert>
#include <iostream>
#include <cmath>
#include <limits>
#include "mixing/mixer.h"
#include "mixing/patternplayer.h"
#include "sequence.h"

using namespace std;
using zzub::pattern;

struct sequenceplugin : zzub::plugin {

#pragma pack (push, 1)
	struct gval {
		unsigned short bpm;
		unsigned char tpb;
		unsigned char swing;
		unsigned char swingticks;
	};
#pragma pack (pop)

	gval gvals;
	zzub::patternplayer* seqplayer;

	sequenceplugin() {
		global_values = &gvals;
		track_values = 0;
		seqplayer = 0;
	}

	void init(zzub::archive* arc) {
		// associate with the root player
		seqplayer = _mixer->rootplayer;
		_mixer->set_plugin_timesource(seqplayer, _id, 1, 0);
	}

	void destroy() {

		zzub::plugin::destroy();
	}

	void process_events() {
		bool changed = false;
		int bpm = seqplayer->_master_info.beats_per_minute;
		int tpb = seqplayer->_master_info.ticks_per_beat;
		float swingamount = seqplayer->_master_info.swing_amount;
		int swingticks = seqplayer->_master_info.swing_ticks;

		if (gvals.bpm != 65535) {
			bpm = gvals.bpm;
			changed = true;
		}
		if (gvals.tpb != 255) {
			tpb = gvals.tpb;
			changed = true;
		}
		if (gvals.swing != 255) {
			swingamount = gvals.swing * 0.01f;
			changed = true;
		}
		if (gvals.swingticks != 255) {
			swingticks = gvals.swingticks;
			changed = true;
		}

		if (changed) {
			seqplayer->set_speed(bpm, tpb, swingamount, swingticks);
		}
	}

	void update_timesource() {
		// ... nop? update parameters to reflect tempo change?
	}

	int get_currently_playing_patterns(int** result) { 
		static int patterns[1];

		int count = 0;
		if (seqplayer && seqplayer->pattern_id != -1)
			patterns[count++] = seqplayer->pattern_id;

		*result = patterns;
		return count; 
	}

	int get_currently_playing_row(int patternid) { 
		if (seqplayer && patternid == seqplayer->pattern_id) {
			return seqplayer->pattern_row;
		}
		return -1; 
	}
};

//
// pattern_plugin_info
//

zzub::plugin* sequence_plugin_info::create_plugin() { 
	return new sequenceplugin();
}
