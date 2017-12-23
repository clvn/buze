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
#include "pattern.h"

using namespace std;
using zzub::pattern;
///using zzub::pattern_event;

struct patternplugin;
#pragma pack (push, 1)
struct gval {
	unsigned short bpm;
	unsigned char tpb;
	unsigned char swing;
	unsigned char swingticks;
};
struct tval {
	unsigned short pattern;
	unsigned char note;
	unsigned short offset;
	unsigned short length;
};
#pragma pack (pop)


struct sequencetrack {
	patternplugin* _plugin;
	zzub::mixer* _mixer;
	tval* values;
	zzub::patternplayer* patplayer;

	int playpatternid;
	int playlength;

	sequencetrack() {
		patplayer = 0;
		playpatternid = -1;
		playlength = -1;
	}

	void process_events();
	int process_sequence(bool offline);
};

struct patternplugin : zzub::plugin {
	gval gvals;
	tval tvals[pattern_plugin_max_tracks];
	sequencetrack tracks[pattern_plugin_max_tracks];

// mixer specific:
	int shared_beats_per_minute;
	int shared_ticks_per_beat;
	int shared_swing_amount;
	int shared_swing_ticks;
	int num_tracks;

	patternplugin() {
		global_values = &gvals;
		track_values = &tvals;

		num_tracks = 1;

		shared_beats_per_minute = 0;
		shared_ticks_per_beat = 0;
		shared_swing_amount = 50;
		shared_swing_ticks = 4;
	}

	void init(zzub::archive* arc) {
		for (int i = 0; i < pattern_plugin_max_tracks; i++) {
			zzub::master_info& m = _mixer->rootplayer->_master_info;
			tracks[i].patplayer = _mixer->create_patternplayer(m.samples_per_second, m.beats_per_minute, m.ticks_per_beat, m.swing_amount, m.swing_ticks);
			tracks[i]._plugin = this;
			tracks[i]._mixer = _mixer;
			tracks[i].values = &tvals[i];
			_mixer->set_plugin_timesource(tracks[i].patplayer, _id, 2, i);
		}
	}

	void destroy() {
		zzub::plugin::destroy();
	}

	void set_track_count(int count) {
		int bpm = shared_beats_per_minute;
		if (bpm == 0) bpm = _master_info->beats_per_minute;

		int tpb = shared_ticks_per_beat;
		if (tpb == 0) tpb = _master_info->ticks_per_beat;

		float swing;
		if (shared_swing_amount == 0) {
			swing = _master_info->swing_amount;
		} else {
			swing = shared_swing_amount * 0.01;
		}

		int swing_ticks = shared_swing_ticks;
		if (swing_ticks == 0) swing_ticks = _master_info->swing_ticks;

		for (int i = num_tracks; i < count; i++) {
			tracks[i].patplayer->set_speed(bpm, tpb, swing, swing_ticks);
			tracks[i].patplayer->stop();
			tracks[i].patplayer->set_pattern(-1, 0, false);
		}
		num_tracks = count;
	}

	void process_events() {
		bool changed = false;
		int bpm, tpb, swing, swingticks;

		if (gvals.bpm != 65535) {
			// bpm == 0 -> sync to timesource
			bpm = gvals.bpm;
			changed = true;
		} else
			bpm = shared_beats_per_minute;
		if (gvals.tpb != 255) {
			// tpb == 0 -> sync to timesource
			tpb = gvals.tpb;
			changed = true;
		} else
			tpb = shared_ticks_per_beat;
		if (gvals.swing != 255) {
			// swing == 0 -> sync to timesource
			swing = gvals.swing;
			changed = true;
		} else {
			swing = shared_swing_amount;
		}
		if (gvals.swingticks != 255) {
			swingticks = gvals.swingticks;
			changed = true;
		} else {
			swingticks = shared_swing_ticks;
		}

		if (changed) {
			set_speed(bpm, tpb, swing, swingticks);
		}

		for (int i = 0; i < num_tracks; i++) {
			tracks[i].process_events();
		}
	}

	void update_timesource() {
		bool changed = false;
		int bpm, tpb, swing, swing_ticks;
		if (shared_beats_per_minute == 0) {
			bpm = 0;
			changed = true;
		} else
			bpm = shared_beats_per_minute;

		if (shared_ticks_per_beat == 0) {
			tpb = 0;
			changed = true;
		} else
			tpb = shared_ticks_per_beat;

		if (shared_swing_amount == 0) {
			swing = 0;
			changed = true;
		} else {
			swing = shared_swing_amount;
		}

		if (shared_swing_ticks == 0) {
			swing_ticks = 0;
			changed = true;
		} else {
			swing_ticks = shared_swing_ticks;
		}

		if (changed)
			set_speed(bpm, tpb, swing, swing_ticks);
	}

	void set_speed(int bpm, int tpb, int swing, int swing_ticks) {

		shared_beats_per_minute = bpm;
		shared_ticks_per_beat = tpb;
		shared_swing_amount = swing;
		shared_swing_ticks = swing_ticks;

		if (bpm == 0) bpm = _master_info->beats_per_minute;
		if (tpb == 0) tpb = _master_info->ticks_per_beat;
		float fswing;
		if (swing == 0) 
			fswing = _master_info->swing_amount;
		else
			fswing = shared_swing_amount * 0.01;
		
		if (swing_ticks == 0) swing_ticks = _master_info->swing_ticks;

		for (int i = 0; i < num_tracks; i++) {
			zzub::patternplayer* patplayer = tracks[i].patplayer;
			patplayer->set_speed(bpm, tpb, fswing, swing_ticks);
		}

		//cout << "samples per tick: " << std::fixed << tracks[0].patplayer->samples_per_tick << endl;
	}

	int get_currently_playing_patterns(int** result) { 
		static int patterns[pattern_plugin_max_tracks];

		int count = 0;
		for (int i = 0; i < num_tracks; i++) {
			if (tracks[i].patplayer->get_pattern() != 0) {
				patterns[count++] = tracks[i].playpatternid;
			}
		}
		
		*result = patterns;
		return count; 
	}

	int get_currently_playing_row(int patternid) { 
		for (int i = 0; i < num_tracks; i++) {
			if (patternid == tracks[i].playpatternid) {
				return tracks[i].patplayer->pattern_row;
			}
		}
		return -1; 
	}

	virtual const char * describe_value(int param, int value) { 
		pattern* pat = 0;
		switch (param) {
			case 0:
			case 1:
			case 2:
			case 3:
				if (value == 0)
					return "Time Source"; // or use name of time source plugin?
				return 0;
			case 4:
				if (param >= 0 && param < _mixer->patterns.next().size())
					pat = _mixer->patterns.next()[param].get();
				if (pat == 0) return 0;
				return pat->name.c_str();
		}
		return 0; 
	}

};

//
// pattern_plugin_info
//

zzub::plugin* pattern_plugin_info::create_plugin() { 
	return new patternplugin();
}

//
// sequencetrack
//

namespace {	// duplicate from ccm.h and pattern.cpp

int midi_to_buzz_note(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}

int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}
}


void sequencetrack::process_events() {
	int noterel = 0;
	bool hasnote = false;
	if (values->note != zzub_note_value_none && values->note != zzub_note_value_cut && values->note != zzub_note_value_off) {
		int notec4 = buzz_to_midi_note(zzub_note_value_c4);
		int note = buzz_to_midi_note(values->note);
		noterel = note - notec4;
		hasnote = true;
	}
	int startpos = 0;
	if (values->offset != 65535) {
		startpos = values->offset;
	}

	if (values->length != 65535) {
		if (values->length == 0)
			playlength = -1;
		else
			playlength = values->length;
	}

	int endpos;
	if (playlength != -1)
		endpos = startpos + playlength; 
	else
		endpos = -1;

	if (values->pattern != 65535) {
		// associate with the rootplayer in case this player was triggered by an arbitrary parameter change
		if (patplayer->parent == 0) {
			patplayer->parent = _mixer->rootplayer;
		}
		playpatternid = values->pattern;
		patplayer->set_pattern(playpatternid, noterel, false);
		patplayer->reset_position(startpos);
		patplayer->reset_interpolators();
		patplayer->play(endpos);
	} else {
		if (hasnote)
			patplayer->transpose = noterel;

		if (values->offset != 65535) {
			pattern* p = patplayer->get_pattern();
			if (p != 0 && startpos < p->rows) {
				patplayer->set_pattern(playpatternid, noterel, false);
				patplayer->reset_position(startpos);
				patplayer->reset_interpolators();
				patplayer->play(std::min(p->rows, endpos));
			}
		} else if (values->length != 65535) {
			// when length is specified alone, it specifies a length from the current pattern position
			pattern* p = patplayer->get_pattern();
			if (p != 0 && startpos < p->rows) {
				patplayer->set_pattern(playpatternid, noterel, false);
				patplayer->reset_position(startpos);
				patplayer->reset_interpolators();
				patplayer->play(std::min(p->rows, patplayer->pattern_row + endpos));
			}
		}
	}

}
