#pragma once

#include <vector>
#include <string>
#include <istream>

#pragma pack (push, 1)
struct MIDIFILEHEADER {
	char magic[4];
	unsigned int size;
	unsigned short format;
	unsigned short track_count;
	unsigned short delta;
};

struct MIDITRACKHEADER {
	char magic[4];
	unsigned int size;
};
#pragma pack (pop)

struct midievent {

	struct notevelo {
		int note;
		int velocity;
	};

	struct meta {
		int metacommand;
	};

	struct timesignature {
		int num, denom, met, q32;
	};

	struct tempo {
		int musec;
	};

	int timestamp;
	int command;
	int channel;
	int metacommand;
	union {
		notevelo nv;
		timesignature ts;
		tempo t;
		int program;
	};

};

struct miditrack {
	MIDITRACKHEADER header;
	std::string name;
	std::string instrument;
	std::vector<midievent> events;

	int get_time_multiplier(int global_delta);
	void get_unique_deltas(std::vector<int>& deltas);
	void get_tempo_changes(std::vector<midievent*>& tempoevents);
	int get_length();
	int get_channel_polyphony(int channel);
};

struct midifile {
	MIDIFILEHEADER header;
	std::vector<miditrack> tracks;

	bool parse(std::istream& strm);
	bool parse_track(std::istream& strm, miditrack& track);
	bool parse_command(std::istream& strm, miditrack& track, midievent& midev);
	bool parse_metacommand(std::istream& strm, miditrack& track, midievent& midev);
	bool parse_sysex(std::istream& strm, miditrack& track, midievent& midev);
};


struct channelmanager {
	struct trackstate {
		int note;
		int released;
	};
	enum { maxvoices = 64 };
	trackstate trackmap[maxvoices];

	channelmanager() {
		memset(trackmap, 0, sizeof(trackmap));
	}

	int allocate_channel(int note) {
		for (int i = 0; i < maxvoices; i++) {
			if (trackmap[i].note == 0) {
				trackmap[i].note = note;
				trackmap[i].released = 1;
				return i;
			}
		}
		assert(false);
	}

	int release_channel(int note) {
		for (int i = 0; i < maxvoices; i++) {
			if (trackmap[i].note == note) {
				trackmap[i].released = 0;
				return i;
			}
		}
		assert(false);
	}

	void clear_released() {
		for (int i = 0; i < maxvoices; i++) {
			if (trackmap[i].released == 0) {
				trackmap[i].note = 0;
			}
		}
	}

	int get_allocated_channel_count() {
		int counter = 0;
		for (int i = 0; i < maxvoices; i++) {
			if (trackmap[i].released == 1) {
				counter++;
			}
		}
		return counter;
	}
};
