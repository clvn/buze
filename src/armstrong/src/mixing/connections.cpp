/*
Copyright (C) 2003-2008 Anders Ervik <calvin@countzero.no>

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
};
typedef struct zzub::mixer zzub_mixer_t;

#include <functional>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include "mixer.h"
#include "connections.h"
#include "convertsample.h"
#include "../player/driver.h"
#include "../player/midinames.h"	// DOH!

using namespace std;

namespace {

inline int get_track_count(int group, int trackcount) {
	if (group == 0 || group == 1 || group == 3 || group == 4) return 1;
	if (group == 2) return trackcount;
	assert(false);
	return 0;
}

}

namespace zzub {


struct mixer;

struct audio_connection_track_values {
	unsigned short amp;
};

struct audio_connection_info : zzub::info {
	enum {
		max_channels = 64
	};
	audio_connection_info() {
		flags = zzub_plugin_flag_is_connection;
		name = "Audio Connection";
		short_name = "Audio";
		uri = "@zzub.org/connection/audio";
		min_tracks = 0;
		max_tracks = max_channels;
		outputs = 0;
		inputs = 0;

		add_track_parameter()
			.set_word()
			.set_name("Volume")
			.set_description("Volume (0=0%, 4000=100%, 8000=200%)")
			.set_value_min(0).set_value_max(0x8000)
			.set_value_none(0xffff)
			.set_state_flag()
			.set_value_default(0x4000);

	}
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *arc) const { return false; }
};

struct midi_connection_info : zzub::info {
	midi_connection_info() {
		flags = zzub_plugin_flag_is_connection|zzub_plugin_flag_is_interval;
		name = "Midi Connection";
		short_name = "Midi";
		uri = "@zzub.org/connection/midi";
		min_tracks = 0;
		max_tracks = 0;
		outputs = 0;
		inputs = 0;
	}
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *arc) const { return false; }
};

struct event_connection_info : zzub::info {
	event_connection_info() {
		flags = zzub_plugin_flag_is_connection;
		name = "Event Connection";
		short_name = "Event";
		uri = "@zzub.org/connection/event";
		min_tracks = 0;
		max_tracks = 0;
		outputs = 0;
		inputs = 0;
	}
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *arc) const { return false; }
};

struct note_connection_info : zzub::info {
	note_connection_info() {
		flags = zzub_plugin_flag_is_connection;
		name = "Note Connection";
		short_name = "Note";
		uri = "@zzub.org/connection/note";
		min_tracks = 0;
		max_tracks = 0;
		outputs = 0;
		inputs = 0;
	}
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *arc) const { return false; }
};

struct audio_connection : zzub::plugin {
	int connid;
	audio_connection_track_values tstate[audio_connection_info::max_channels];
	audio_connection_track_values tvalues[audio_connection_info::max_channels];
	
	audio_connection();

	virtual void process_events();
	virtual bool process_stereo(float** pin, float** pout, int numsamples, int mode);
	virtual void set_connection(int _connid);

	void process_stereo_chunk(int offset, float** pout, int sample_count);
};

struct event_connection : zzub::plugin {
	
	int connid;
	event_connection();
	int convert(int value, const zzub::parameter *oldparam, const zzub::parameter *newparam);
	virtual void set_connection(int _connid);
	void process_connection_events();
};

struct note_connection : zzub::plugin {
	int connid;

	note_connection();
	virtual void set_connection(int _connid);
	virtual void process_events();
	virtual void process_connection_events();
};

struct midi_connection : zzub::plugin {

	int connid;
	std::vector<zzub::midi_message> midiqueue;

	midi_connection();
	int get_midi_device(int plugin, std::string name);
	virtual void process_events();
	virtual bool process_stereo(float** pin, float** pout, int numsamples, int mode);
	virtual void process_midi_events(zzub::midi_message* pin, int nummessages);
	virtual void set_connection(int _connid);
	virtual int get_interval_size();
};

audio_connection::audio_connection() {
	for (int i = 0; i < audio_connection_info::max_channels; i++) {
		tvalues[i].amp = 0x4000;
		tstate[i] = tvalues[i];
	}
	track_values = &tstate;
}

void audio_connection::process_events() {
	for (int i = 0; i < audio_connection_info::max_channels; i++) {
		if (tstate[i].amp != 0xffff)
			tvalues[i].amp = tstate[i].amp;
	}
}

void audio_connection::set_connection(int _connid) { 
	connid = _connid;
}

void audio_connection::process_stereo_chunk(int offset, float** pout, int sample_count) {
	metaconnection& plugin_self = *_mixer->connections.top()[connid].get();
	metaplugin& plugin_from = *_mixer->plugins.top()[plugin_self.from_plugin_id].get();
	metaplugin& plugin_to = *_mixer->plugins.top()[plugin_self.to_plugin_id].get();

	float *plout[mixer::max_channels] = {0};
	float *plin[mixer::max_channels] = {0};

	for (int i = 0; i < plugin_to.info->inputs; i++) {
		if (pout[i] != 0) plout[i] = pout[i] + offset;
	}

	if (plugin_self.is_back_edge) {
		for (int i = 0; i < std::max(plugin_self.input_count, plugin_self.output_count); i++) {
			int in_channel = plugin_self.first_output + (i % plugin_self.output_count);
			plin[in_channel] = plugin_from.feedback_buffer[in_channel].get();
		}
	} else {
		for (int i = 0; i < std::max(plugin_self.input_count, plugin_self.output_count); i++) {
			int in_channel = plugin_self.first_output + (i % plugin_self.output_count);
			float* workbuffer = plugin_from.work_buffer[in_channel].get();
			plin[in_channel] = workbuffer + plugin_self.audiodata->latency_read;
		}
	}

	bool plugin_to_does_input_mixing = (plugin_to.info->flags & zzub_plugin_flag_does_input_mixing) != 0;
	bool plugin_to_is_bypassed = plugin_to.audiodata->is_bypassed || plugin_to.audiodata->is_softbypassed;
	bool plugin_to_is_muted = plugin_to.audiodata->is_muted || plugin_to.audiodata->is_softmuted;
	bool plugin_from_is_muted = plugin_from.audiodata->is_muted || plugin_from.audiodata->is_softmuted;

	bool max_result = false;
	int totamp = 0;

	if (!plugin_from_is_muted) {
		for (int i = 0; i < std::max(plugin_self.input_count, plugin_self.output_count); i++) {
			int in_channel = plugin_self.first_output + (i % plugin_self.output_count);
			int value_channel = (i) % plugin_from.get_output_channel_count();

			if (plin[in_channel] != 0) {
				//NOTE: we can check last_work_max like this instead of testing the entire buffer if writepos = 0
				//max_result |= plugin_from.last_work_max[i] > SIGNAL_TRESHOLD;
				bool buffer_result = buffer_has_signals(plin[in_channel], sample_count);
				if (buffer_result) {
					max_result = true;
					totamp += tvalues[value_channel].amp;
				} else {
					plin[in_channel] = 0;
				}
			}
		}
	} else {
		for (int i = 0; i < std::max(plugin_self.input_count, plugin_self.output_count); i++) {
			int in_channel = plugin_self.first_output + (i % plugin_self.output_count);
			plin[in_channel] = 0;
		}
	}

	bool result = totamp > 0 && max_result;

	if (plugin_to_does_input_mixing && !plugin_to_is_bypassed && !plugin_to_is_muted) {
		if (result) {
			// assume plugin_to_does_input_mixing is only implemented for mono/stereo buzz machines
			// if this were really internal, the amps for each channel shoould be sent to input() instead of the average:
			plugin_to.plugin->input(plugin_self.id, plugin_self.first_input, plugin_self.first_output, plugin_self.input_count, plugin_self.output_count, plugin_self.flags, plin, sample_count, totamp / (float)plugin_self.output_count / (float)0x4000); 
		} else {
			plugin_to.plugin->input(plugin_self.id, 0, 0, 0, 0, 0, 0, sample_count, 0);
			for (int i = 0; i < plugin_self.input_count; i++) {
				pout[plugin_self.first_input + i] = 0;
			}
		}
	} else {
		int outcounts[mixer::max_channels] = {0};
		// mono to stereo / wrap output channels when input>output by default
		int min_count = std::min(plugin_self.input_count, plugin_self.output_count);
		if (min_count > 0) {
			for (int i = 0; i < std::max(plugin_self.input_count, plugin_self.output_count); i++) {
				int out_channel = plugin_self.first_input + (i % plugin_self.input_count);
				int in_channel = plugin_self.first_output + (i % plugin_self.output_count);
				int value_channel = (i) % plugin_from.get_output_channel_count();
				if (plout[out_channel] != 0 && plin[in_channel] != 0) {
					add_samples(plout[out_channel], plin[in_channel], sample_count, tvalues[value_channel].amp / (float)0x4000);
					outcounts[out_channel]++;
				}
			}
		}

		for (int i = 0; i < mixer::max_channels; i++) {
			if (pout[i] && outcounts[i] == 0)
				pout[i] = 0;
		}
	}

}

bool audio_connection::process_stereo(float** pin, float** pout, int sample_count, int mode) {

	metaconnection& plugin_self = *_mixer->connections.top()[connid].get();
	metaplugin& plugin_from = *_mixer->plugins.top()[plugin_self.from_plugin_id].get();
	metaplugin& plugin_to = *_mixer->plugins.top()[plugin_self.to_plugin_id].get();

	// we are already aligned to the end of the write-to-ringbuffer, but the read-ringbuffer could still overlap
	// => if plugin_self.latency_read + samplecount > maxlatency+buffersize => input/add chunked

	if (plugin_self.audiodata->latency_read + sample_count > zzub_latency_buffer_size) {
		int beforecycle = zzub_latency_buffer_size - plugin_self.audiodata->latency_read;
		int aftercycle = plugin_self.audiodata->latency_read + sample_count - zzub_latency_buffer_size;

		float *ploutfirst[mixer::max_channels];

		if (beforecycle > 0) {
			memcpy(ploutfirst, pout, sizeof(ploutfirst));
			process_stereo_chunk(0, ploutfirst, beforecycle);
		}

		plugin_self.audiodata->latency_read = 0;

		process_stereo_chunk(beforecycle, pout, aftercycle);

		// merge active channels for both chunks
		if (beforecycle > 0)
			for (int i = 0; i < mixer::max_channels; i++)
				if (ploutfirst[i] != 0 && pout[i] == 0)
					pout[i] = ploutfirst[i];

		plugin_self.audiodata->latency_read += aftercycle;
	} else {
		process_stereo_chunk(0, pout, sample_count);
		plugin_self.audiodata->latency_read += sample_count;
	}

	return true;
}

event_connection::event_connection() {
}

void event_connection::set_connection(int _connid) { 
	connid = _connid;
}

int event_connection::convert(int value, const zzub::parameter *oldparam, const zzub::parameter *newparam) {
	if (value != oldparam->value_none) {
		if ((oldparam->type == zzub_parameter_type_note) && (newparam->type == zzub_parameter_type_note)) {
			return value;
		} else {
			if ((oldparam->type == zzub_parameter_type_note) && (value == zzub_note_value_off || value == zzub_note_value_cut)) {
				return newparam->value_none; // ignore noteoff/notecut commands - cant be scaled
			}
			float v = oldparam->normalize(value);
			return newparam->scale(v);
		}
	}
	return newparam->value_none;
}

void event_connection::process_connection_events() {
	const zzub::parameter *param_in;
	const zzub::parameter *param_out;

	metaconnection& plugin_self = *_mixer->connections.top()[connid].get();
	metaplugin& from_m = *_mixer->plugins.top()[plugin_self.from_plugin_id].get();
	metaplugin& to_m = *_mixer->plugins.top()[plugin_self.to_plugin_id].get();

	bool modified = false;
	std::vector<event_connection_binding>::iterator b;
	for (b = plugin_self.bindings.begin(); b != plugin_self.bindings.end(); ++b) {
		if (b->source_param_index >= from_m.get_parameter_count(3, 0))
			continue; // out of range source param (could happen with dummy plugins pre rev 13)

		if (b->target_track_index >= get_track_count(b->target_group_index, to_m.tracks))
			continue; // out of range target track param (could happen with dummy plugins pre rev 13)

		if (b->target_param_index >= to_m.get_parameter_count(b->target_group_index, 0))
			continue; // out of range target column param (could happen with dummy plugins pre rev 13)

		param_in = from_m.get_parameter_info(3, 0, b->source_param_index);
		param_out = to_m.get_parameter_info(b->target_group_index, b->target_track_index, b->target_param_index);

		int sv = from_m.get_parameter_direct(3, 0, b->source_param_index);
		int cv = convert(sv, param_in, param_out);
		if (cv != param_out->value_none) {
			to_m.set_parameter(b->target_group_index, b->target_track_index, b->target_param_index, cv, false);
		}
	}
}


note_connection::note_connection() {
	connid = -1;
}

void note_connection::set_connection(int _connid) {
	connid = _connid;
}

void note_connection::process_events() {
}

void note_connection::process_connection_events() { 

	if (connid == -1) return ;

	metaconnection& plugin_self = *_mixer->connections.top()[connid].get();
	metaplugin& plugin_from = *_mixer->plugins.top()[plugin_self.from_plugin_id].get();
	metaplugin& plugin_to = *_mixer->plugins.top()[plugin_self.to_plugin_id].get();

	assert(plugin_to.info->note_group != -1); // the note connection should never have been made

	int to_tracks = get_track_count(plugin_to.info->note_group, plugin_to.tracks);

	for (std::vector<note_message>::iterator i = plugin_from.note_messages.begin(); i != plugin_from.note_messages.end(); ++i) {
		if (i->track < to_tracks) {
			if (i->note != zzub_note_value_none && plugin_to.info->note_column != -1)
				plugin_to.set_parameter(plugin_to.info->note_group, i->track, plugin_to.info->note_column, i->note, false);

			if (i->amp != 255 && plugin_to.info->velocity_column != -1)
				plugin_to.set_parameter(plugin_to.info->note_group, i->track, plugin_to.info->velocity_column, i->amp, false);

			if (i->wave != 0 && plugin_to.info->wave_column != -1)
				plugin_to.set_parameter(plugin_to.info->note_group, i->track, plugin_to.info->wave_column, i->wave, false);
		}
	}
}

midi_connection::midi_connection() {
}

void midi_connection::set_connection(int _connid) { 
	connid = _connid;
}

void midi_connection::process_events() {
}

bool midi_connection::process_stereo(float** pin, float** pout, int sample_count, int mode) {
	// advance queued midi timestamps - the has_interval flag ensures sample_count stays below the next timestamp
	for (std::vector<zzub::midi_message>::iterator i = midiqueue.begin(); i != midiqueue.end(); ++i) {
		zzub::midi_message& msg = *i;
		assert((int)msg.timestamp >= sample_count); // NOTE: assert could be too aggressive...
		msg.timestamp -= sample_count ;
	}

	return false;
}

int midi_connection::get_interval_size() {
	// return number of samples until next midi event
	int mintimestamp = std::numeric_limits<int>::max();
	for (std::vector<zzub::midi_message>::iterator i = midiqueue.begin(); i != midiqueue.end(); ++i) {
		zzub::midi_message& msg = *i;
		if ((int)msg.timestamp < mintimestamp)
			mintimestamp = (int)msg.timestamp;
	}
	return mintimestamp;
}

void midi_connection::process_midi_events(zzub::midi_message* pint, int nummessages) {

	metaconnection& plugin_self = *_mixer->connections.top()[connid].get();

	metaplugin& fromplugin = *_mixer->plugins.top()[plugin_self.from_plugin_id].get();
	metaplugin& toplugin = *_mixer->plugins.top()[plugin_self.to_plugin_id].get();
	metaplugin& connplugin = *_mixer->plugins.top()[plugin_self.plugin_id].get();
	if (fromplugin.out_midi_messages.size() != 0) {

		int device = get_midi_device(plugin_self.to_plugin_id, plugin_self.midi_device_name);

		// send midi messages directly to the connected midi plugin if timestamp == 0, otherwise queue
		for (size_t i = 0; i < fromplugin.out_midi_messages.size(); i++) {
			zzub::midi_message& msg = fromplugin.out_midi_messages[i];
			msg.device = device;
			if (msg.timestamp == 0) {
				toplugin.in_midi_messages.push_back(msg);
			} else {
				// reset the midi connection interval when the midi queue changes
				connplugin.audiodata->next_interval_position = 0;
				midiqueue.push_back(msg);
			}
		}
	}

	// send/remove previously queued midi messages
	for (std::vector<zzub::midi_message>::iterator i = midiqueue.begin(); i != midiqueue.end(); ) {
		if (i->timestamp == 0) {
			toplugin.in_midi_messages.push_back(*i);
			i = midiqueue.erase(i);
		} else {
			++i;
		}
	}
	
}

int midi_connection::get_midi_device(int plugin_id, std::string name) {
	_midiouts midiouts;
	_mixer->plugins.top()[plugin_id].get()->plugin->get_midi_output_names(&midiouts);
	std::vector<string>::iterator i = find(midiouts.names.begin(), midiouts.names.end(), name);
	if (i == midiouts.names.end()) return -1;
	// we really need to find the _global_ device index, not the target machine one.... ? ?? ???
	return (int)(i - midiouts.names.begin());
}

zzub::plugin* audio_connection_info::create_plugin() {
	return new audio_connection();
}

zzub::plugin* event_connection_info::create_plugin() {
	return new event_connection();
}

zzub::plugin* note_connection_info::create_plugin() {
	return new note_connection();
}

zzub::plugin* midi_connection_info::create_plugin() {
	return new midi_connection();
}

struct connectioncollection : zzub::plugincollection {
	audio_connection_info audioconnection;
	midi_connection_info midiconnection;
	event_connection_info eventconnection;
	note_connection_info noteconnection;

	virtual void initialize(zzub::pluginfactory *factory);
};

void connectioncollection::initialize(zzub::pluginfactory *factory) {
	audioconnection.collection = this;
	midiconnection.collection = this;
	eventconnection.collection = this;
	noteconnection.collection = this;

	factory->register_info(&audioconnection);
	factory->register_info(&midiconnection);
	factory->register_info(&eventconnection);
	factory->register_info(&noteconnection);
}

} // namespace zzub


zzub::plugincollection* connections_get_plugincollection() {
	return new zzub::connectioncollection();
}
