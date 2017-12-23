#include <zzub/plugin.h>

#include "harmony.h"

using namespace std;

struct harmonyplugin;

struct gval {
};

struct tval {
	unsigned short symbol;
};

struct harmonytrack {
	harmonyplugin* _plugin;
	tval* values;

	int playpatternid;

	harmonytrack() : _plugin(0), values(0), playpatternid(-1) {}

	void process_events();
	int process_sequence(bool offline);
};

struct harmonyplugin : zzub::plugin {
	gval gvals;
	tval tvals[harmony_plugin_max_tracks];
	harmonytrack tracks[harmony_plugin_max_tracks];
	int num_tracks;

	harmonyplugin() {
		global_values = &gvals;
		track_values = &tvals;
		num_tracks = 1;
	}

	void init(zzub::archive* arc) {
		for (int i = 0; i < harmony_plugin_max_tracks; ++i) {
			tracks[i]._plugin = this;
			tracks[i].values = &tvals[i];
		}
	}

	void destroy() {
		for (int i = 0; i < harmony_plugin_max_tracks; i++) {
			//if (tracks[i].xxx) {
			//}
		}
		zzub::plugin::destroy();
	}

	void set_track_count(int count) {
		for (int i = num_tracks; i < count; ++i) {
			///tracks[i].foo();
		}
		num_tracks = count;
	}

	void process_events() {
		bool changed = false;

		for (int i = 0; i < num_tracks; ++i) {
			//tracks[i].process_events();
		}
	}

	int get_currently_playing_patterns(int** result) {
		return 0;
	}

	int get_currently_playing_row(int patternid) { 
		return -1; 
	}

	virtual const char* describe_value(int param, int value) { 
		switch (param) {
			case 0:
				return 0;
		}
		return 0; 
	}

};

//
// harmony_plugin_info
//

zzub::plugin* harmony_plugin_info::create_plugin() { 
	return new harmonyplugin();
}

//
// harmonytrack
//

void harmonytrack::process_events() {
// 	if (values->pattern != 65535) {
// 		playpatternid = values->pattern;
// 		patplayer->set_pattern(playpatternid, false);
// 		patplayer->reset_position();
// 		patplayer->play();
// 	}
}
