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
#include "../../mixing/mixer.h"
#include "midioutput.h"

using namespace std;

namespace midioutput {

#define MAKEMIDI(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))

midioutput::midioutput() {
	global_values = &gval;
	attributes = NULL;
	current_device = -1;
}

void midioutput::init(zzub::archive* arc) {
	if (arc != 0) {
		zzub::instream* ins = arc->get_instream("");
		char version;
		ins->read(version);
		if (version == 1) {
			std::string name;
			ins->read(name);
			set_midi_device(name);
		}
	}
}

void midioutput::save(zzub::archive* arc) {
	zzub::outstream* outs = arc->get_outstream("");
	char version = 1;
	outs->write(version);

	if (current_device == -1) {
		outs->write("\0");
		return ;
	}

	outs->write(_mixer->output_midi_device_names[current_device].second);
}

void midioutput::set_midi_device(std::string name) {
	for (std::vector<std::pair<int, std::string> >::iterator i = _mixer->output_midi_device_names.begin(); i != _mixer->output_midi_device_names.end(); ++i) {
		if (name == i->second) {
			current_device = (int)(i - _mixer->output_midi_device_names.begin());
			break;
		}
	}
}

void midioutput::process_midi_events(zzub::midi_message* pin, int nummessages) {
	zzub::midi_message msg;
	for (int i = 0; i < nummessages; i++) {
		int device = _mixer->output_midi_device_names[current_device].first;
		msg.device = device;
		msg.timestamp = _mixer->buffer_position;
		msg.message = pin[i].message;
		_mixer->write_midi(&msg, 1);
	}
}

void midioutput::get_midi_output_names(zzub::outstream *pout) {
	pout->write("Device");
}

void midioutput::get_sub_menu(int i, zzub::outstream* os) { 
	for (size_t i = 0; i < _mixer->output_midi_device_names.size(); i++) {
		string name = _mixer->output_midi_device_names[i].second;
		if (i == current_device) name = "*" + name;
		os->write((void*)name.c_str(), (unsigned int)(name.length()) + 1);
	}
	os->write("\0");
}

void midioutput::command(int index) {
	if (index >= 0x100 && index < 0x200) {
		current_device = index - 0x100;
	}
}

}
