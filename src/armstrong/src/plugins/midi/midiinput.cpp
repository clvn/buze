#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
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
#include "midiinput.h"

using namespace std;

namespace midiinput {

midiinput::midiinput() {
	global_values = &gval;
	attributes = NULL;
}

int midiinput::get_interval_size() {
	// return number of samples until next midi event
	for (int i = 0; i < _mixer->input_midi_message_count; i++) {
		zzub::midi_message& msg = _mixer->input_midi_messages[i];
		if (msg.timestamp > (unsigned long)_mixer->buffer_position)
			return msg.timestamp - _mixer->buffer_position;
	}
	return _mixer->buffer_size - _mixer->buffer_position;
}

void midiinput::process_midi_events(zzub::midi_message* pin, int nummessages) {
	// send global midi events at this sample position through the midi wires
	for (int i = 0; i < _mixer->input_midi_message_count; i++) {
		zzub::midi_message& msg = _mixer->input_midi_messages[i];
		if (msg.timestamp == _mixer->buffer_position)
			midi_out(0, msg.message);
	}
}

void midiinput::midi_out(int time, unsigned int data) {
	zzub::midi_message msg = { -1, data, (unsigned long)time };
	_mixer->midi_out(_id, msg);
}

}
