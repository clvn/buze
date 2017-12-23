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
#include <cstdio>
#include <zzub/signature.h>
#include <zzub/plugin.h>
#include "mixing/mixer.h"
#include "miditracker.h"

using namespace std;

namespace miditracker {

#define MAKEMIDI(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))


int midi_to_buzz_note(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}

int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}

std::string note_string(unsigned char i) {
	if (i==zzub_note_value_off) return "off";
	if (i==zzub_note_value_cut) return "cut";
	static const char* notes[]={"..", "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-", "..", "..", "..", ".." };
	char pc[16];
	int note=i&0xF;
	int oct=(i&0xF0) >> 4;

	sprintf(pc, "%x", oct);
	std::string s=notes[note]+std::string(pc);
	return s;
}

struct midioutnames : zzub::outstream {
	miditracker* tracker;
	midioutnames(miditracker* _tracker) {
		tracker = _tracker;
	}

	virtual int write(void* buffer, int size) {
		char* str = (char*)buffer;
		tracker->devices.push_back(str);
		return size;
	}
	long position() {
		return 0;
	}
	void seek(long, int) { 
	}
};

miditrack::miditrack() {
/*	note = zzub_note_value_none;
	note_delay = 0;
	note_cut = 0;
	command = 0xff;
	commandValue = 0xffff;
	parameter = 0x30ff; // novalue
	parameterValue = 128; // novalue
*/
	midi_channel = 0;
	last_note = zzub_note_value_none;
}

void miditrack::tick() {
	int note = zzub_note_value_none;
	int note_delay = 0;
	int note_cut = 0;
	int command = 0xff;
	int commandValue = 0xffff;
	int parameter = 0x30ff;
	int parameterValue = 128;
	//int midi_channel;

	if (values->note != zzub_note_value_none) {
		note = values->note;
		note_delay = 0;
		velocity = 0x7f;
	}

	if (values->velocity != 0xff) {
		velocity = values->velocity;
	}

	// TODO/NOTE: the note-delay uses the whole range - no novalue available ..
	//if (values->delay != _miditracker_info.paraDelay->value_none) {
		float unit = (float)tracker->samples_per_tick / 255;

		note_delay = (int)(unit * values->delay); // convert to samples in tick
	//}

	if (values->command != 0xff) {
		command = values->command;
	}

	if (values->commandValue != 0xffff) {
		commandValue = values->commandValue;
	}

	if (values->parameter != 0x30ff) {
		parameter = values->parameter;
	}

	if (values->parameterValue != 128) {
		parameterValue = values->parameterValue;
	}

	if (values->midiChannel != 0xff)
		midi_channel = values->midiChannel - 1;

	// find out how many samples we should wait before triggering this note

	// Determine desired delay
	// ts is incremented during each midi_out() to curtail simultaneous midi output.

	// note delay is 0..FF = a tick, divide by 16 and it can delay up to 16 ticks ahead
	int ts = note_delay; // / 16; // this division by 16 is arbitrary, what does Polac use?

	// Parameters

	if (parameter != 0x30ff && parameterValue != 128) {

		int msg;
		int status;
		int x;
		int y;
		int message;

		/*

			Polac VST(i):

				30ff:	None
				30fe:	Pitch Bend
				30fd:	PolyAT
				30fc:	MonoAT
				30fb:	Morph Programs
				30fa:	CC 250
					...
				3000:	CC 0
				2fff:	unused (or VST parameter)
					...
				0000:	unused (or VST parameter)

		*/
		if (parameter < 0x3000) {
			// unused
			msg = 0;
		}
		else if (parameter < 0x30fb) {
			msg = 0xb0; // CC
			x = parameter - 0x3000;
			y = (parameterValue > 127) ? 127 : parameterValue;
		}
		else if (parameter == 0x30fe) {
			msg = 0xe0; // Pitch Bend
			x = (parameterValue > 127) ? 127 : parameterValue;
			y = 0;
			printf("Pitch bend: %i\n", x);
		}
		else {
			// TODO: the other parameter values
			msg = 0;
		}

		if (msg) {
			status = msg | (midi_channel & 0x0f);
			message = MAKEMIDI(status, x, y);
			tracker->midi_out(ts, message);
		}

		//parameter = paraParameter->value_none;
		//parameterValue = paraParameterValue->value_none;
	}

	// Commands

	if (command != 0xff && commandValue != 0xffff) {
		switch (command) {

			case 9: // MIDI Message

				int msg;
				/*
					From Polac VST(i):

					09 xxyy

					xx(0-FF): MIDI Message #
					00-7F: CC 0-7F
					80-FD: user-defined MIDI Message
					FE: Pitch Bend Range
					FF: Pitch Bend

					yy(0-FF): Value

					The MIDI Messages can be edited: ->Default Valus->Midi Messages.
				*/

				int x = commandValue >> 8;
				int y = commandValue - (x << 8);

				if (x <= 0x7f) {
					msg = 0xb0;	// Control Change (CC)
					if(y > 0x7f) // limit is 127
						y = 0x7f;
				}
				else if (x <= 0xfd);		// user-defined MIDI message (To Do)
				else msg = 0xe0;			// Pitch Bend (To Fix?)


				int status = msg | (midi_channel & 0x0f);
				int message = MAKEMIDI(status, x, y);
				tracker->midi_out(ts, message);

				//cout << "midiTracker sending MIDI Message=\"" << hex << message << "\" commandValue=\"" << commandValue << "\" x=\"" << x << "\" y=\"" << y << "\"."<<dec<< endl;
				break;
		};
		command = 0xff;
		commandValue = 0xffff;
	}

	if (note == zzub_note_value_none) return;

	if ( last_note != zzub_note_value_none) {
		int status = 0x80 | (last_channel & 0x0f);
		int message = MAKEMIDI(status, last_note, 0);

		tracker->midi_out(ts, message);
		last_note = zzub_note_value_none;

		//cout << "miditracker playing note-off " << note << " with delay " << (int)note_delay << endl;
	}

	if (note != zzub_note_value_off && note != zzub_note_value_cut) {

		int midi_note = buzz_to_midi_note(note);
		int status = 0x90 | (midi_channel & 0x0f);
		int message = MAKEMIDI(status, midi_note, velocity);

		last_note = midi_note;
		last_channel = midi_channel;

		tracker->midi_out(ts, message);

		//cout << "miditracker playing note " << note << " with delay " << (int)note_delay << endl;
	}
	//note = zzub_note_value_none;

}

miditracker::miditracker() {
	global_values = &gval;
	track_values = &tval;
	attributes = NULL;
	//open_device = -1;

	for (int i = 0; i < max_tracks; i++) {
		tracks[i].tracker = this;
		tracks[i].values = &tval[i];
	}
}

void miditracker::init(zzub::archive * const pi) {
	devices.clear();
}

void miditracker::set_track_count(int i) {
	num_tracks = i;
}

void miditracker::process_events() {
	samples_per_tick = _master_info->samples_per_tick;
	//samples_in_tick = 0;

	if (gval.program != 0xff) {
		// here we change program on all midi channels
		for (int i = 0; i < 16; i++) {
			unsigned int data = MAKEMIDI(0xC0 | i, gval.program, 0);
			midi_out(0, data);
		}
	}

	for (int i = 0; i < num_tracks; i++) {
		tracks[i].tick();
	}
}

void miditracker::stop() {

	for (int i = 0; i < num_tracks; i++) {
		if (tracks[i].last_note != zzub_note_value_none) {
			int status = 0x80 | (tracks[i].last_channel & 0x0f);
			int message = MAKEMIDI(status, tracks[i].last_note, 0);

			midi_out(0, message);
//			tracks[i].note = zzub_note_value_none;
			tracks[i].last_note = zzub_note_value_none;
		}
	}
}

bool miditracker::process_stereo(float **pin, float **pout, int numsamples, int mode) {
	return false;
}

const char * miditracker::describe_value(int param, int value) {
	static char temp[1024];

	switch (param) {
		case 11: // track parameter
			/*

				Polac VST(i):

					30ff:	None
					30fe:	Pitch Bend
					30fd:	PolyAT
					30fc:	MonoAT
					30fb:	Morph Programs
					30fa:	CC 250
						...
					3000:	CC 0
					2fff:	unused (or VST parameter)
						...
					0000:	unused (or VST parameter)

			*/
			if (value < 0x3000)
				return "unused";
			else if (value < 0x30fb) {
				sprintf(temp, "CC: %3i   %02Xh", value - 0x3000, value - 0x3000);
				return temp;
			}
			break;
		default:

			break;
	};

	return 0;
}

void miditracker::command(int index) {
	/*if (index >= 0x100 && index < 0x200) {
		open_device = index - 0x100;
	}*/
}

void miditracker::midi_out(int time, unsigned int data) {
	zzub::midi_message msg = { -1, data, time };
	_mixer->midi_out(_id, msg);
}

}
