/*
Copyright (C) 2011 Anders Ervik <calvin@countzero.no>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
	struct metaplugin;
};
typedef struct zzub::mixer zzub_mixer_t;

#include "mixing/mixer.h"
#include <sstream>
#include <cassert>
#include <iostream>
#include <cstring>
#include "note.h"

#pragma pack (push, 1)
struct gvals {
	unsigned char octave;
	unsigned char note;
	unsigned char quantize;
	unsigned char notes[12];
	unsigned char allowmin;
	unsigned char allowmax;
};

struct tvals {
	unsigned char note;
	unsigned char wave;
	unsigned char amp;
};
#pragma pack (pop)
	
#define MAKEMIDI(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))

namespace {	// duplicate from ccm.h and pattern.cpp

int midi_to_buzz_note(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}

int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}
}

int quantizes[3][12] = {
//    C C#  D D#  E  F F#  G G#  A A#  B
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // none
	{ 0,-1, 0, 1, 0, 0,-1, 0,-1, 0,-1, 0}, // major
	{ 0,-1, 0, 0,-1, 0,-1, 0, 0,-1, 0,-1}, // minor
}; 

struct note_plugin : zzub::plugin {

	gvals gval;
	tvals tval[128];
	int tnotes[128];

	int track_count;
	int otrans;
	int ntrans;
	int *quantize;
	int custom_quantize[12]; 
	int allowmin;
	int allowmax;

	note_plugin() {
		global_values = &gval;
		track_values = &tval;
		memset(tnotes, 0, sizeof(tnotes));
		otrans = 0;
		ntrans = 0;
		quantize = quantizes[0];
	}

	void set_track_count(int count) {
		track_count = count;
	}

	int quantize_note(int note) {
		int octave = note / 12;
		return (((note % 12) + quantize[note % 12]) % 12) + octave * 12;
	}

	virtual void process_events() {
		if (gval.octave != 255) {
			otrans = gval.octave - 5;
		}
		if (gval.note != 255) {
			ntrans = gval.note - 6;
		}
		if (gval.quantize != 255) {
			if (gval.quantize == 0)
				quantize = custom_quantize;
			else
				quantize = quantizes[gval.quantize];
		}
		for (int i = 0; i < 12; i++) {
			if (gval.notes[i] != 255)
				custom_quantize[i] = (gval.notes[i] + 12 - i) % 12;
		}
		if (gval.allowmin != 255)
			allowmin = gval.allowmin;
		if (gval.allowmax != 255)
			allowmax = gval.allowmax;

		int midi_channel = 0;

		for (int i = 0; i < track_count; i++) {
			if (tval[i].note != zzub_note_value_none) {
				int note, midi_note;
				if (tval[i].note == zzub_note_value_off || tval[i].note == zzub_note_value_cut) {
					note = midi_note = tval[i].note;
				} else {
					midi_note = buzz_to_midi_note(tval[i].note);
					if (midi_note < allowmin || midi_note > allowmax)
						continue;
					midi_note = quantize_note(midi_note) + otrans*12 + ntrans;
					midi_note = std::max(std::min(midi_note, 119), 0);
					note = midi_to_buzz_note(midi_note);
				}
				int velocity = 0x7f;
				if (tval[i].amp != 255) velocity = tval[i].amp;

				// handle zzub_plugin_flag_has_note_output:
				zzub::note_message msg = { i, note, tval[i].wave, tval[i].amp };
				_mixer->note_out(_id, msg);

				// handle zzub_plugin_flag_has_midi_output:
				if (tnotes[i] != zzub_note_value_none) {
					// send note off for one or more of: stop previous note, note_off, note_cut
					int status = 0x80 | (midi_channel & 0x0f);
					unsigned int data = MAKEMIDI(status, tnotes[i], velocity);
					zzub::midi_message message = { -1, data, 0 };
					_mixer->midi_out(_id, message);
				}

				if (note != zzub_note_value_off && note != zzub_note_value_cut) {
					int status = 0x90 | (midi_channel & 0x0f);
					unsigned int data = MAKEMIDI(status, midi_note, velocity);
					zzub::midi_message message = { -1, data, 0 };
					_mixer->midi_out(_id, message);
					tnotes[i] = midi_note;
				} else {
					tnotes[i] = zzub_note_value_none;
				}
			}
		}
	}

	void get_midi_output_names(zzub::outstream* pout) {
		pout->write("MIDI Notes");
	}

	int find_voice(int note, bool mustexist) {
		int minfree = -1;
		for (int i = 0; i < 128; i++) {
			if (tnotes[i] == note) return i;
			if (minfree == -1 && tnotes[i] == zzub_note_value_none)
				minfree = i;
		}
		if (mustexist) return -1;
		return minfree;
	}

	bool process_midi_command(int channel, int command, int data1, int data2) {
		// using tnotes for voice mapping on the unquantized basenote
		if (command == 8 || (command == 9 && data2 == 0)) {
			// note off
			int track = find_voice(data1, true); // determine voice# from basenote
			if (track == -1) return false;
			zzub::note_message msg = { track, zzub_note_value_off, 0, 0 };
			_mixer->note_out(_id, msg);
			tnotes[track] = zzub_note_value_none;
			return true;
		} else if (command == 9) {
			// note on
			if (data1 < allowmin || data1 > allowmax)
				return false;
			int note = quantize_note(data1);
			note = std::max(std::min(note, 119), 0);
			int track = find_voice(data1, false); // determine voice# from basenote
			if (track == -1) return false;
			zzub::note_message msg = { track, midi_to_buzz_note(note), 0, data2 };
			_mixer->note_out(_id, msg);
			tnotes[track] = note;
			return true;
		}
		return false;
	}

	void process_midi_events(zzub::midi_message* pin, int nummessages) {
		int polycount = 0;
		for (int i = 0; i < nummessages; i++) {
			unsigned short status = pin[i].message & 0xff;
			int channel = status&0xf;
			int command = (status & 0xf0) >> 4;
			unsigned char data1 = (pin[i].message >> 8) & 0xff;
			unsigned char data2 = (pin[i].message >> 16) & 0xff;
			if (process_midi_command(channel, command, data1, data2))
				_mixer->midi_out(_id, pin[i]);
		}
	}

	void stop() {
		int midi_channel = 0;

		for (int i = 0; i < track_count; i++) {
			if (tnotes[i] != zzub_note_value_none) {
				int status = 0x80 | (midi_channel & 0x0f);
				unsigned int data = MAKEMIDI(status, tnotes[i], 0);
				zzub::midi_message message = { -1, data, 0 };
				_mixer->midi_out(_id, message);
				tnotes[i] = zzub_note_value_none;
			}
		}
	}

	const char* describe_value(int param, int value) {
		static char desc[32];
		const char* notestr[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		std::stringstream strm;
		switch (param) {
			case 0: // global param 0 - global octave
				value -= 5;
				if (value < 0) 
					strm << value;
				else if (value > 0) 
					strm << "+" << value;
				else 
					strm << "None";
				strcpy(desc, strm.str().c_str());
				return desc;
			case 1: // global param 1 - global note
				value -= 6;
				if (value < 0) 
					strm << value;
				else if (value > 0) 
					strm << "+" << value;
				else 
					strm << "None";
				strcpy(desc, strm.str().c_str());
				return desc;
			case 2: // global param 2 - quantize
				switch (value) {
					case 0: 
						return "Custom";
					case 1:
						return "Major";
					case 2:
						return "Minor";
				}
				break;
			case 3: // transpose single notes
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
				return notestr[value];
			case 15:
			case 16:
				strm << notestr[value%12];
				strm << (value/12);
				// midi note, 1..119?
				strcpy(desc, strm.str().c_str());
				return desc;
		}
		return 0;
	}
};

zzub::plugin* note_plugin_info::create_plugin() { 
	return new note_plugin();
}

