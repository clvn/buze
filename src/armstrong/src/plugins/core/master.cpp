/*
Copyright (C) 2003-2013 Anders Ervik <calvin@countzero.no>

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
#define _USE_MATH_DEFINES

#define NO_ZZUB_MIXER_TYPE
namespace zzub { struct mixer; }
typedef struct zzub::mixer zzub_mixer_t;

#include <cmath>
#include <cstring>
#include <zzub/plugin.h>

#include "mixing/mixer.h"
#include "mixing/convertsample.h"

#include "master.h"

using namespace std;

/*! \struct master_plugin_info
	\brief Master plugin description
*/
/*! \struct master_plugin
	\brief Master plugin implementation
*/


float dB_to_linear(float val) {
	if (val == 0.0) return(1.0);
	return (float)(pow(10.0f, val / 20.0f));
}

const int NO_MASTER_VOLUME = 0xFFFF;


master_plugin_info::master_plugin_info() {
	this->flags = zzub_plugin_flag_is_root | zzub_plugin_flag_is_singleton | zzub_plugin_flag_has_audio_output | zzub_plugin_flag_has_audio_input | zzub_plugin_flag_has_midi_input;
	this->name = "Master";
	this->short_name = "Master";
	this->author = "n/a";
	this->uri = "@zzub.org/master";
	this->inputs = 2;
	this->outputs = 2;

	add_global_parameter()
		.set_word()
		.set_name("Volume")
		.set_description("Master Volume (0=0 dB, 4000=-80 dB)")
		.set_value_min(0)
		.set_value_max(0x4000)
		.set_value_none(NO_MASTER_VOLUME)
		.set_state_flag()
		.set_value_default(0);
}

zzub::plugin* master_plugin_info::create_plugin() { 
	return new master_plugin(); 
}

bool master_plugin_info::store_info(zzub::archive *) const { 
	return false; 
}


/***

    master_plugin

***/

master_plugin::master_plugin() {
	global_values = gvals = &dummy;
	track_values = 0;
	attributes = 0;

	gvals->volume = 0;
}

void master_plugin::init(zzub::archive*) {
}

void master_plugin::destroy() {
	delete this;
}

void master_plugin::process_events() {
	int volume = gvals->volume;
	if (volume != NO_MASTER_VOLUME) master_volume = volume;
}

bool master_plugin::process_stereo(float **pin, float **pout, int numSamples, int mode) { 
	zzub::metaplugin* mpl = _mixer->plugins.top()[_id].get();
	int writeoffset = mpl->audiodata->output_buffer_write;

	int mchnL = _mixer->output_master_channel * 2 + 0;
	int mchnR = _mixer->output_master_channel * 2 + 1;

	if (mode == zzub_process_mode_write || mode == zzub_process_mode_no_io) {
		if (pout[0]) memset(pout[0], 0, numSamples * sizeof(float));
		if (pout[1]) memset(pout[1], 0, numSamples * sizeof(float));

		_mixer->write_output(mchnL, writeoffset, 0, numSamples, 0.0f);
		_mixer->write_output(mchnR, writeoffset, 0, numSamples, 0.0f);
		return false;
	}

	bool has_signals = false;
	float db = ((float)master_volume / (float)0x4000)*-80.0f;
	float amp = dB_to_linear(db);

	if (pout[0] && pin[0]) {
		if (amp > 0) {
			memcpy(pout[0], pin[0], numSamples * sizeof(float));
			dspamp(pout[0], numSamples, amp);
		} else
			pout[0] = 0;
	}
	has_signals &= _mixer->write_output(mchnL, writeoffset, pin[0], numSamples, amp);

	if (pout[1] && pin[1]) {
		if (amp > 0) {
			memcpy(pout[1], pin[1], numSamples * sizeof(float));
			dspamp(pout[1], numSamples, amp);
		} else
			pout[1] = 0;
	}

	has_signals &= _mixer->write_output(mchnR, writeoffset, pin[1], numSamples, amp);
	return has_signals;
}

void master_plugin::process_midi_events(zzub::midi_message* pin, int nummessages) {
	zzub::midi_message msg;
	for (int i = 0; i < nummessages; i++) {
		int device = _mixer->output_midi_device_names[pin[i].device].first;
		msg.device = device;
		msg.timestamp = _mixer->buffer_position;
		msg.message = pin[i].message;
		_mixer->write_midi(&msg, 1);
	}
}

void master_plugin::get_midi_output_names(zzub::outstream *pout) {
	for (size_t i = 0; i < _mixer->output_midi_device_names.size(); i++) {
		string name = _mixer->output_midi_device_names[i].second;
		pout->write((void*)name.c_str(), (unsigned int)(name.length()) + 1);
	}
}
