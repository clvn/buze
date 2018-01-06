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

#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include <stack>
#include <cmath>
#include <limits>
#include <map>
#include <cstring>
#include <boost/make_shared.hpp>

#define NO_ZZUB_MIXER_TYPE

namespace zzub {
	struct mixer;
};

typedef struct zzub::mixer zzub_mixer_t;

#include "graph.h"
#include "mixer.h"
#include "connections.h"
#include "../player/archive.h"
#include "patternplayer.h"
#include "convertsample.h"

using std::cout;
using std::cerr;
using std::endl;

#define DBG(x) { std::cerr << #x << "=" << (x) << "  "; }
#define END() { std::cerr << std::endl; }

int convert_sample_count(int samples, int from_rate, int to_rate) {
	return (int)(((float)samples / from_rate) * to_rate);
}

namespace {	// duplicate from ccm.h and pattern.cpp

int midi_to_buzz_note(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}

int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}

inline void normalize(float& v) {
	if (std::abs(v) < SIGNAL_TRESHOLD) v = 0;
}

inline bool scan_peak_channel(float* pin, int numSamples, float* maxpeak, float falloff) {
	bool zero = true;
	for (int j = 0; j < numSamples; j++) {
		normalize(*maxpeak);
		*maxpeak *= falloff;
		float v = std::abs(pin[j]);
		*maxpeak = std::max(v, *maxpeak);

		if (zero && v > SIGNAL_TRESHOLD) zero = false;
	}
	return zero;
}

inline bool scan_peak_channels(float** pin, int channels, int numSamples, float* maxes, float falloff) {
	bool zero = true;
	for (int i = 0; i < channels; i++) {
		for (int j = 0; j < numSamples; j++) {
			normalize(maxes[i]);
			maxes[i] *= falloff;
			if (pin[i]) { 
				float v = std::abs(pin[i][j]);
				maxes[i] = std::max(v, maxes[i]);

				if (zero && v > SIGNAL_TRESHOLD) zero = false;
			}
		}
	}
	return zero;
}

}

namespace zzub {

/***

	recursive functions for nested pattern players

***/

void patternplayer_play(const std::vector<boost::shared_ptr<patternplayer> >& patternplayers, patternplayer* parentplayer, int stoprow = -1) {
	parentplayer->play(stoprow);

	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.begin(); i != patternplayers.end(); ++i) {
		patternplayer* pp = i->get();

		if (pp && pp->parent == parentplayer)
			patternplayer_play(patternplayers, pp);
	}
}

void patternplayer_stop(const std::vector<boost::shared_ptr<patternplayer> >& patternplayers, patternplayer* parentplayer) {
	parentplayer->stop();

	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.begin(); i != patternplayers.end(); ++i) {
		patternplayer* pp = i->get();

		if (pp && pp->parent == parentplayer)
			patternplayer_stop(patternplayers, pp);
	}
}

void patternplayer_seek_end(const std::vector<boost::shared_ptr<patternplayer> >& patternplayers, patternplayer* parentplayer) {
	parentplayer->reset_next_position();
	parentplayer->reset_interpolators();

	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.begin(); i != patternplayers.end(); ++i) {
		patternplayer* pp = i->get();

		if (pp && pp->parent == parentplayer)
			patternplayer_seek_end(patternplayers, pp);
	}
}

void patternplayer_advance(const std::vector<boost::shared_ptr<patternplayer> >& patternplayers, patternplayer* parentplayer, int numsamples) {
	// originally only advance if playing // if (parentplayer->playing)
	parentplayer->advance(numsamples);

	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.begin(); i != patternplayers.end(); ++i) {
		patternplayer* pp = i->get();

		if (pp && pp->parent == parentplayer)
			patternplayer_advance(patternplayers, pp, numsamples);
	}
}

void patternplayer_apply_sequence_size(const std::vector<boost::shared_ptr<patternplayer> >& patternplayers, patternplayer* parentplayer, int& min_sequence_count) {
	parentplayer->apply_sequence_size(min_sequence_count);

	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.begin(); i != patternplayers.end(); ++i) {
		patternplayer* pp = i->get();

		if (pp && pp->parent == parentplayer)
			patternplayer_apply_sequence_size(patternplayers, pp, min_sequence_count);
	}
}

void patternplayer_process_sequence(const std::vector<boost::shared_ptr<patternplayer> >& patternplayers, patternplayer* parentplayer, bool offline) {
	parentplayer->process_sequence(offline);

	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.begin(); i != patternplayers.end(); ++i) {
		patternplayer* pp = i->get();

		if (pp && pp->parent == parentplayer)
			patternplayer_process_sequence(patternplayers, pp, offline);
	}
}

void patternplayer_clear_children(const std::vector<boost::shared_ptr<patternplayer> >& patternplayers, patternplayer* parentplayer, bool is_root) {

	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.begin(); i != patternplayers.end(); ++i) {
		patternplayer* pp = i->get();

		if (pp && pp->parent == parentplayer)
			patternplayer_clear_children(patternplayers, pp, false);
	}

	parentplayer->clear_children(is_root);
}


/***

	mixer

***/

mixer::mixer() {
	song_position = work_position = 0;
	latency_buffer_position = 0;
	is_recording_parameters = false;
	is_syncing_midi_transport = false;

	user_thread_id = thread_id::get();

	state = player_state_muted;

	midi_plugin = -1;
	thread_count = 1;
	threadworkers.commit();
	threadworkers.pop();

	for (int i = 0; i < max_channels; i++) {
		input_buffer[i] = 0;
	}
	encodermgr.start(); // start encoder thread
	encoderbuffers.start(); // start encoder allocator thread

	lock_mode = false;
	dirty_graph = false;
	dirty_mutestate = false;
	dirty_latencies = false;

	rootplayer = create_patternplayer(48000, 126, 4, 0.5, 4);

	memset(&hostinfo, 0, sizeof(host_info));

	insert_song();

	// add empty garbage containers for the first commit
	commit_garbage();

	commit_events = new std::vector<commit_event_data>();
}

mixer::~mixer() {
	process_audio_event_queue();
	process_user_event_queue();
	finalize_garbage();
	delete commit_events;
}

void mixer::commit_garbage() {
	buffer_garbage.push_back(std::vector<boost::shared_array<short> >());
	userplugin_garbage.push_back(std::vector<zzub::plugin*>());
}

// called by mixer::notify_user_event_listeners
void mixer::finalize_garbage() {
	// dispose of released objects
	plugins.gc();
	patterns.gc();
	patternevents.gc();
	patternformats.gc();
	patterncolumns.gc();
	patternformattracks.gc();
	connections.gc();
	midimappings.gc();
	waves.gc();
	wavelevels.gc();
	patternplayers.gc();
	buffer_garbage.pop_front();

	for (std::vector<zzub::plugin*>::iterator i = userplugin_garbage.front().begin(); i != userplugin_garbage.front().end(); ++i) {
		(*i)->destroy();
	}
	userplugin_garbage.pop_front();
}

// returns true if an insert_connection-event for this connection was removed from the queue
bool mixer::reset_pending_connection_commit_events(int connid) {
	bool deleted_insert_event = false;
	for (std::vector<commit_event_data>::iterator i = commit_events->begin(); i != commit_events->end(); ) {
		bool deletable;
		switch (i->type) {
			case commit_event_insert_connection:
				deletable = i->insert_connection.conn->id == connid;
				if (!deleted_insert_event && deletable) deleted_insert_event = true;
				break;
			case commit_event_set_audio_connection:
				deletable = i->set_audio_connection.connid == connid;
				break;
			default:
				deletable = false;
				break;
		}
		if (deletable) {
			finalize_commit_event(*i);
			i = commit_events->erase(i);
		} else
			++i;
	}
	return deleted_insert_event;
}

void mixer::reset_pending_plugin_commit_events(int pluginid) {
	for (std::vector<commit_event_data>::iterator i = commit_events->begin(); i != commit_events->end(); ) {
		bool deletable;
		switch (i->type) {
			case commit_event_set_attribute:
				deletable = i->set_attribute.plugin->id == pluginid;
				break;
			case commit_event_set_state_format:
				deletable = i->set_state_format.newplugin->id == pluginid;
				break;
			case commit_event_set_parameter:
				deletable = i->set_parameter.pluginid == pluginid;
				break;
			case commit_event_process_events:
				deletable = i->process_events.plugin->id == pluginid;
				break;
			case commit_event_insert_plugin:
				deletable = i->insert_plugin.plugin->id == pluginid;
				break;
			case commit_event_delete_plugin:
				deletable = i->delete_plugin.plugin->id == pluginid;
				break;
			case commit_event_type_set_plugin_latency:
				deletable = i->set_plugin_latency.plugin_id == pluginid;
				break;
			case commit_event_type_set_plugin_stream_source:
				deletable = i->set_plugin_stream_source.plugin->id == pluginid;
				break;
			default:
				deletable = false;
				break;
		}
		if (deletable) {
			finalize_commit_event(*i);
			i = commit_events->erase(i);
		} else
			++i;
	}
}

void mixer::reset_pending_patternevent_commit_events(int patternid) {
	for (std::vector<commit_event_data>::iterator i = commit_events->begin(); i != commit_events->end(); ) {
		bool deletable;
		switch (i->type) {
			case commit_event_insert_patternevent:
				deletable = i->insert_patternevent.patternid == patternid;
				break;
			case commit_event_remove_patternevent:
				deletable = i->remove_patternevent.patternid == patternid;
				break;
			case commit_event_update_patternevent:
				deletable = i->update_patternevent.patternid == patternid;
				break;
			default:
				deletable = false;
				break;
		}
		if (deletable) {
			finalize_commit_event(*i);
			i = commit_events->erase(i);
		} else
			++i;
	}
}

void mixer::insert_song() {
	song* s = new song();
	s->loop_begin = 0;
	s->loop_end = 16;
	s->loop_enabled = true;
	s->orderlist;
	
	orderlist_position = 0;
	queue_index = -1;

	songinfo.assign(0, s);
}

void mixer::update_song(int loop_begin, int loop_end, bool loop_enabled, const std::vector<int>& order_patterns) {
	song* s = &songinfo.next_for_write(0);
	s->loop_begin = loop_begin;
	s->loop_end = loop_end;
	s->loop_enabled = loop_enabled;
	s->orderlist = order_patterns;
}

void mixer::insert_plugin(int id, zzub::plugin* userplugin, zzub::info* loader, std::vector<unsigned char>& data, std::string name, int num_tracks, std::string stream_source, bool is_muted, bool is_bypassed, int timesourceid, int timesourcegroup, int timesourcetrack) {
	// NOTE: some plugins (e.g utrk) query the wavetable during init() so we need the wavetable

	assert(num_tracks >= (int)loader->min_tracks && num_tracks <= (int)loader->max_tracks);

	metaplugin* k = new metaplugin();
	k->id = id;
	k->name = name;
	k->plugin = userplugin;
	k->info = loader;
	k->tracks = num_tracks;
	k->plugin->_master_info = &rootplayer->_master_info;
	k->plugin->_mixer = this;
	k->plugin->_id = id;
	k->stream_source = stream_source;
	k->timesource_plugin_id = timesourceid;
	k->timesource_group = timesourcegroup;
	k->timesource_track = timesourcetrack;

	int plugin_channels = std::max(loader->inputs, loader->outputs);
	if (k->info->flags & zzub_plugin_flag_has_audio_input || k->info->flags & zzub_plugin_flag_has_audio_output) {
		k->work_buffer.resize(plugin_channels);
		k->feedback_buffer.resize(plugin_channels);
		for (int i = 0; i < plugin_channels; i++) {
			k->work_buffer[i] = boost::shared_array<float>(new float[zzub_latency_buffer_size + zzub_buffer_size]);
			memset(k->work_buffer[i].get(), 0, zzub_latency_buffer_size * sizeof(float));
			
			k->feedback_buffer[i] = boost::shared_array<float>(new float[zzub_latency_buffer_size + zzub_buffer_size]);
			memset(k->feedback_buffer[i].get(), 0, zzub_latency_buffer_size * sizeof(float));
		}
	} else {
		assert(plugin_channels == 0);
	}

	k->audiodata = boost::make_shared<pluginaudiodata>();
	k->audiodata->is_muted = is_muted;
	k->audiodata->is_bypassed = is_bypassed;
	k->audiodata->is_softmuted = false;
	k->audiodata->is_softbypassed = false;
	k->audiodata->next_interval_position = 0;
	k->audiodata->latency_write = 0;
	k->audiodata->last_latency = 0;
	k->audiodata->latency = -1;
	k->audiodata->last_work_max.resize(loader->outputs);
	k->audiodata->last_work_audio_result = false;
	k->audiodata->last_work_midi_result = false;
	k->audiodata->last_work_buffersize = 0;
	k->audiodata->last_work_time = 0.0f;
	k->audiodata->last_work_frame = 0;
	k->audiodata->cpu_load = 0.0f;
	k->audiodata->cpu_load_buffersize = 0;
	k->audiodata->cpu_load_time = 0.0f;
	k->audiodata->writemode_errors = 0;
	k->audiodata->is_dirty = false;
	k->audiodata->in_sequence_chain = false;

	if (k->info->flags & zzub_plugin_flag_is_encoder) {
		k->audiodata->encoder_buffer.resize(k->info->inputs);
		k->audiodata->encoder_frame_buffer.resize(k->info->inputs);
		for (int i = 0; i < k->info->inputs; i++) {
			char* buffer = encoderbuffers.alloc(&k->audiodata->encoder_size);
			k->audiodata->encoder_buffer[i] = (float*)buffer;
			k->audiodata->encoder_size = k->audiodata->encoder_size / sizeof(float);
			k->audiodata->encoder_frame_buffer[i] = boost::shared_array<float>(new float[zzub_buffer_size]);
			memset(k->audiodata->encoder_frame_buffer[i].get(), 0, zzub_buffer_size * sizeof(float));
		}
		k->audiodata->encoder_position = 0;
		k->audiodata->connection_process_events_time = 0;
	}

	plugins.assign(id, k);

	// if this is a time source, make sure its initialized before any consumers
	std::vector<newplugindata>::iterator timeplugin = std::find_if(new_plugins.begin(), new_plugins.end(), find_new_timeplugin(id));

	newplugindata new_plugin;
	new_plugin.id = id;
	new_plugin.timesourceid = timesourceid;
	new_plugin.data = data;
	new_plugin.initialized = false;

	if (timeplugin == new_plugins.end()) {
		new_plugins.push_back(new_plugin);
	} else {
		// insert time source before its first time source user:
		new_plugins.insert(timeplugin, new_plugin);
	}

	k->pre_initialize();

	commit_event_data ev;
	ev.type = commit_event_insert_plugin;
	ev.insert_plugin.plugin = k;
	invoke_commit_event(ev);
}


void mixer::on_insert_plugin(metaplugin* plugin) {
	if (plugin->info->flags & zzub_plugin_flag_is_encoder)
		encodermgr.register_encoder(plugin->plugin);

	// Is not fully initialized until swapped into the audio thread:
	plugin->initialized = true;
}

void mixer::initialize_plugin(int id, std::vector<unsigned char>& data) {
	metaplugin* k = plugins.next()[id].get();
	cerr << "initializing plugin " << id << " " << k->name << endl;
	on_update_plugin_timesource(k, false);
	if (data.size()) {
		armstrong::frontend::mem_archive arc;
		zzub::outstream* outs = arc.get_outstream("");
		outs->write(&data.front(), (int)data.size());
		k->initialize(&arc); 
	} else
		k->initialize(0);

	on_process_events(k);

	dirty_graph = true;
}

void mixer::initialize_new_plugins() {
	for (std::vector<newplugindata>::iterator i = new_plugins.begin(); i != new_plugins.end(); ++i) {
		if (!i->initialized) {
			initialize_plugin(i->id, i->data);
			i->initialized = true;
		}
	}
}

void mixer::update_plugin(int id, std::string name, int num_tracks, std::string stream_source, bool is_muted, bool is_bypassed, int timesourceid, int timesourcegroup, int timesourcetrack) {
	metaplugin* k = &plugins.next_for_write(id);
	k->name = name;
	k->tracks = num_tracks;
	//k->stream_source = stream_source;
	k->timesource_plugin_id = timesourceid;
	k->timesource_group = timesourcegroup;
	k->timesource_track = timesourcetrack;

	invalidate_state_format(id);
	dirty_graph = true;

	if (stream_source != k->stream_source) {
		commit_event_data ev;
		ev.type = commit_event_type_set_plugin_stream_source;
		ev.set_plugin_stream_source.plugin = k;
		ev.set_plugin_stream_source.source = new std::string(stream_source);
		invoke_commit_event(ev);

		k->stream_source = stream_source;
	}
}

void mixer::delete_plugin(int id) {

	// delete pending events - to prevent sending events to the wrong plugin in the 
	// case a new plugin is created immediately afterwards that gets the same id
	reset_pending_plugin_commit_events(id);

	if (id == midi_plugin)
		set_midi_plugin(-1);

	metaplugin* k = plugins.next()[id].get();

	// remove associated timesources/patternplayers
	if (k->global_timesource) destroy_patternplayer(k->global_timesource);
	for (std::vector<patternplayer*>::iterator i = k->track_timesources.begin(); i != k->track_timesources.end(); ++i)
		destroy_patternplayer(*i);
	
	// reset timesources on all plugins who have us as their time source
	for (std::vector<boost::shared_ptr<metaplugin> >::iterator i = plugins.next().begin(); i != plugins.next().end(); ++i) {
		if (i->get() != 0 && i->get()->timesource_plugin_id == id) {
			metaplugin* tp = &plugins.next_for_write(i->get()->id);
			tp->timesource_plugin_id = -1;
			tp->timesource_group = 0;
			tp->timesource_track = 0;
		}
	}

	std::vector<newplugindata>::iterator newplugin = std::find_if(new_plugins.begin(), new_plugins.end(), find_new_plugin(id));
	if (newplugin != new_plugins.end()) {
		// if a newly created plugin in the newplugins-array was deleted before
		// it was initialized (which usually happens later, either after connecting
		// something to it or at a barrier), then remove it from the list as the 
		// plugin id used for lookup will no longer be valid.

		new_plugins.erase(newplugin);
	} else {
		// if the deleted plugin was created before being committed, there is nothing 
		// to do in the audio thread, so dont generate a commit-event.
		// in other words, if on_insert_plugin() isnt called, then we shouldnt call on_delete_plugin() either

		commit_event_data ev;
		ev.type = commit_event_delete_plugin;
		ev.delete_plugin.plugin = k;
		invoke_commit_event(ev);
	}

	userplugin_garbage.back().push_back(k->plugin);
	plugins.remove(id);
	dirty_graph = true;
}

void mixer::on_delete_plugin(metaplugin* plugin) {
	if (plugin->info->flags & zzub_plugin_flag_is_encoder) {
		process_encoder_buffer(plugin); // finalize buffers
		encodermgr.unregister_encoder(plugin->plugin);
	}
}

void mixer::insert_connection(int id, int plugin_id, int from_plugin_id, int to_plugin_id, zzub_connection_type type, int first_input, int first_output, int inputs, int outputs, std::string midi_device) {
	initialize_new_plugins();

	metaconnection* conn = new metaconnection();
	conn->id = id;
	conn->plugin_id = plugin_id;
	conn->from_plugin_id = from_plugin_id;
	conn->to_plugin_id = to_plugin_id;
	conn->type = type;
	conn->first_input = first_input;
	conn->first_output = first_output;
	conn->output_count = outputs;
	conn->input_count = inputs;
	conn->flags = 0;
	conn->midi_device_name = midi_device;
	conn->is_back_edge = false;

	conn->audiodata = boost::shared_ptr<connectionaudiodata>(new connectionaudiodata());
	conn->audiodata->latency_read = 0;

	connections.assign(id, conn);

	invalidate_state_format(to_plugin_id);
	invalidate_state_format(from_plugin_id);

	// generate a commit event for calling plugin->add_input() on the audio thread
	commit_event_data ev;
	ev.type = commit_event_insert_connection;
	ev.insert_connection.conn = conn;
	ev.insert_connection.from_plugin = plugins.next()[from_plugin_id].get();
	ev.insert_connection.to_plugin = plugins.next()[to_plugin_id].get();
	invoke_commit_event(ev);

	plugins.next()[plugin_id].get()->plugin->set_connection(id);

	dirty_graph = true;
}

void mixer::on_insert_connection(metaconnection* conn, metaplugin* to_plugin, metaplugin* from_plugin) {
	to_plugin->plugin->add_input(conn->id);
	if (conn->type == zzub_connection_type_audio)
		on_update_audioconnection(conn->id);
}

void mixer::delete_connection(int id) {
	metaconnection* userconn = connections.next()[id].get();

	// generate a commit event for calling plugin->delete_input() on the audio thread.
	// and; do not call plugin->delete_input() if add_input() wasnt called yet:
	bool deleted_insert_event = reset_pending_connection_commit_events(id);
	if (!deleted_insert_event) {
		commit_event_data ev;
		ev.type = commit_event_delete_connection;
		ev.delete_connection.conn = userconn;
		ev.delete_connection.from_plugin = plugins.next()[userconn->from_plugin_id].get();
		ev.delete_connection.to_plugin = plugins.next()[userconn->to_plugin_id].get();
		invoke_commit_event(ev);
	}
	connections.remove(id);

	dirty_graph = true;
	invalidate_state_format(userconn->to_plugin_id);
	invalidate_state_format(userconn->from_plugin_id);
}

void mixer::on_delete_connection(metaconnection* conn, metaplugin* to_plugin, metaplugin* from_plugin) {
	to_plugin->plugin->delete_input(conn->id);
}

void mixer::update_audioconnection(int id, int first_input, int first_output, int inputs, int outputs, int flags) {

	assert(connections.next()[id] != 0);

	metaconnection* p = &connections.next_for_write(id);
	assert(p->type == zzub_connection_type_audio);
	p->first_input = first_input;
	p->first_output = first_output;
	p->input_count = inputs;
	p->output_count = outputs;

	invalidate_state_format(p->to_plugin_id);
	invalidate_state_format(p->from_plugin_id);

	// create a commit event so we can call set_input_channels on the audio thread
	commit_event_data ev;
	ev.type = commit_event_set_audio_connection;
	ev.set_audio_connection.connid = id;
	invoke_commit_event(ev);
}

void mixer::update_midiconnection(int id, std::string device) {
	assert(connections.next()[id] != 0);

	metaconnection* p = &connections.next_for_write(id);
	assert(p->type == zzub_connection_type_midi);
	p->midi_device_name = device;

	invalidate_state_format(p->to_plugin_id);
	invalidate_state_format(p->from_plugin_id);
}

void mixer::on_update_audioconnection(int id) {
	metaconnection* userconn = connections.top()[id].get();
	assert(userconn != 0);
	assert(userconn->type == zzub_connection_type_audio);

	// if does-input-mixing:
	metaplugin* to_plugin = plugins.top()[userconn->to_plugin_id].get();
	to_plugin->plugin->set_input_channels(userconn->from_plugin_id, userconn->first_input, userconn->first_output, userconn->input_count, userconn->output_count, 0);
}

void mixer::add_event_connection_binding(int connid, int sourceparam, int group, int track, int column) {
	metaconnection* userconn = &connections.next_for_write(connid);
	assert(userconn != 0);
	assert(userconn->type == zzub_connection_type_event);

	event_connection_binding ecb;
	ecb.source_param_index = sourceparam;
	ecb.target_group_index = group;
	ecb.target_track_index = track;
	ecb.target_param_index = column;
	userconn->bindings.push_back(ecb);

	invalidate_state_format(userconn->to_plugin_id);
	invalidate_state_format(userconn->from_plugin_id);
}

void mixer::delete_event_connection_binding(int connid, int sourceparam, int group, int track, int column) {
	metaconnection* userconn = &connections.next_for_write(connid);
	assert(userconn != 0);
	assert(userconn->type == zzub_connection_type_event);

	for (std::vector<event_connection_binding>::iterator i = userconn->bindings.begin(); i != userconn->bindings.end(); ++i) {
		if (i->source_param_index == sourceparam &&
			i->target_group_index == group &&
			i->target_track_index == track &&
			i->target_param_index == column) {
				userconn->bindings.erase(i);
				break;
		}
	}
	invalidate_state_format(userconn->to_plugin_id);
	invalidate_state_format(userconn->from_plugin_id);
}

void mixer::insert_pattern(int id, int formatid, std::string name, int length, int resolution, int beginloop, int endloop, bool loopenabled) {
	pattern* p = new pattern();
	p->name = name;
	p->rows = length;
	p->resolution = resolution;
	p->formatid = formatid;
	p->beginloop = beginloop;
	p->endloop = endloop;
	p->loopenabled = loopenabled;

	patterns.assign(id, p);
	patternevents.assign(id, new pattern_events());
}

void mixer::update_pattern(int id, int formatid, std::string name, int length, int resolution, int beginloop, int endloop, bool loopenabled) {
	assert(patterns.next()[id] != 0);

	
	pattern* p = &patterns.next_for_write(id);

	int oldresolution = p->resolution;

	p->name = name;
	p->rows = length;
	p->resolution = resolution;
	p->formatid = formatid;
	p->beginloop = beginloop;
	p->endloop = endloop;
	p->loopenabled = loopenabled;

	// a pattern resolution change is like a tempo change pr pattern, needs patternplayer::reset_next_position() on
	// TODO: also relocate the pattern_row
	if (oldresolution != resolution) {
		for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.next().begin(); i != patternplayers.next().end(); i++) {
			patternplayer* pp = i->get();
			if (pp && pp->pattern_id == id)
				pp->reset_next_position();
		}

	}
	// TODO: remove events in pattern where time > length??
}

void mixer::delete_pattern(int id) {
	assert(patterns.next()[id] != 0);
	patterns.remove(id);

	reset_pending_patternevent_commit_events(id);
	patternevents.remove(id);
}

void mixer::insert_pattern_value(int id, int patternid, int pluginid, int group, int track, int column, int time, int value) {
	commit_event_data event_data;
	event_data.type = commit_event_insert_patternevent;
	event_data.insert_patternevent.patternid = patternid;
	event_data.insert_patternevent.ev =
		new pattern_event(id, pluginid, group, track, column, time, value);
	invoke_commit_event(event_data);
}

void mixer::remove_pattern_value(int id, int patternid) {
	commit_event_data event_data;
	event_data.type = commit_event_remove_patternevent;
	event_data.remove_patternevent.patternid = patternid;
	event_data.remove_patternevent.id = id;
	event_data.remove_patternevent.ev = 0;
	invoke_commit_event(event_data);
}

void mixer::update_pattern_value(int id, int patternid, int pluginid, int group, int track, int column, int time, int value) {
	commit_event_data event_data;
	event_data.type = commit_event_update_patternevent;
	event_data.update_patternevent.id = id;
	event_data.update_patternevent.patternid = patternid;
	event_data.update_patternevent.time = time;
	event_data.update_patternevent.value = value;
	event_data.update_patternevent.pluginid = pluginid;
	event_data.update_patternevent.group = group;
	event_data.update_patternevent.track = track;
	event_data.update_patternevent.column = column;
	invoke_commit_event(event_data);
}

void mixer::insert_patternformat(int id) {
	patternformat* format = new patternformat();
	format->id = id;
	patternformats.assign(id, format);
}

void mixer::delete_patternformat(int id) {
	assert(patternformats.next()[id] != 0);
	patternformats.remove(id);
}

void mixer::insert_patternformatcolumn(int id, int formatid, int pluginid, int group, int track, int column) {

	patternformatcolumn* p = new patternformatcolumn();
	p->id = id;
	p->formatid = formatid;
	p->pluginid = pluginid;
	p->group = group;
	p->track = track;
	p->column = column;
	patterncolumns.assign(id, p);

	patternformat* format = &patternformats.next_for_write(formatid);
	format->columns.push_back(*p);
}

void mixer::delete_patternformatcolumn(int id) {

	patternformatcolumn* col = patterncolumns.next()[id].get();
	assert(col != 0);

	patternformat* format = &patternformats.next_for_write(col->formatid);
	for (std::vector<patternformatcolumn>::iterator i = format->columns.begin(); i != format->columns.end(); ++i) {
		if (i->id == id) {
			format->columns.erase(i);
			return ;
		}
	}

	patterncolumns.remove(id);

}

void mixer::insert_patternformattrack(int id, int formatid, int pluginid, int group, int track, int is_muted) {
	patternformattrack* p = new patternformattrack();
	p->id = id;
	p->formatid = formatid;
	p->pluginid = pluginid;
	p->group = group;
	p->track = track;
	p->is_muted = is_muted;

	patternformattracks.assign(id, p);
	dirty_mutestate = true;
}

void mixer::update_patternformattrack(int id, int formatid, int pluginid, int group, int track, int is_muted) {
	assert(patternformattracks.next()[id] != 0);
	patternformattrack* p = &patternformattracks.next_for_write(id);
	p->formatid = formatid;
	p->pluginid = pluginid;
	p->group = group;
	p->track = track;
	p->is_muted = is_muted;
	dirty_mutestate = true;
}

void mixer::delete_patternformattrack(int id) {
	assert(patternformattracks.next()[id] != 0);
	patternformattracks.remove(id);
	dirty_mutestate = true;
}

void mixer::insert_wave(int id, int flags, float volume) {
	wave_info* wave = new wave_info();
	wave->id = id;
	wave->flags = flags;
	wave->volume = volume;
	wave->wavelevel_count = 0;
	waves.assign(id, wave);
}

void mixer::update_wave(int id, int flags, float volume) {
	assert(waves.next()[id] != 0);
	
	wave_info* wave = &waves.next_for_write(id);
	wave->flags = flags;
	wave->volume = volume;
}

void mixer::insert_wavelevel(int id, int waveid, int beginloop, int endloop, int numsamples, int samplerate, zzub_wave_buffer_type format, int basenote, const std::vector<int>& slices) {
	wave_level* wavelevel = new wave_level();
	wavelevel->id = id;
	wavelevel->root_note = basenote;
	wavelevel->wave_id = waveid;
	wavelevel->loop_start = beginloop;
	wavelevel->loop_end = endloop;
	wavelevel->sample_count = numsamples;
	wavelevel->samples_per_second = samplerate;
	wavelevel->format = format;
	wavelevel->samples = wavelevel->legacy_sample_ptr = 0;
	wavelevel->slices = slices;

	wavelevels.assign(id, wavelevel);

	// allocate a silent buffer during initialization
	wave_info* wave = &waves.next_for_write(wavelevel->wave_id);
	wave->wavelevel_count++;
	int channels = (wave->flags & zzub_wave_flag_stereo) != 0 ? 2 : 1;
	bool is_extended = (wave->flags & zzub_wave_flag_extended) != 0;
	int extended_bytes = is_extended ? 16 : 0;

	int bytes_per_sample = sizeFromWaveFormat(wavelevel->format) * channels;
	unsigned char* samplebuffer = new unsigned char[numsamples * bytes_per_sample + extended_bytes];
	memset(samplebuffer, 0, numsamples * bytes_per_sample + extended_bytes);
	set_wavelevel_samples(id, samplebuffer, numsamples, is_extended);
}

int get_legacy_samples(int samples, int bytes_per_sample, int channels) {
	return (int)ceil((samples * bytes_per_sample) / 2.0f) + (4 / channels);
}

void mixer::update_legacy_wavelevel(wave_level* wavelevel) {
	int bytes_per_sample = sizeFromWaveFormat(wavelevel->format);

	assert(waves.next()[wavelevel->wave_id] != 0);
	wave_info* wave = waves.next()[wavelevel->wave_id].get();
	int channels = (wave->flags & zzub_wave_flag_stereo) != 0 ? 2 : 1;
	bool is_extended = (wave->flags & zzub_wave_flag_extended) != 0;
	
	if (is_extended) {
		wavelevel->legacy_loop_start = get_legacy_samples(wavelevel->loop_start, bytes_per_sample, channels);
		wavelevel->legacy_loop_end = get_legacy_samples(wavelevel->loop_end, bytes_per_sample, channels);
		wavelevel->legacy_sample_count = get_legacy_samples(wavelevel->sample_count, bytes_per_sample, channels);
	} else {
		wavelevel->legacy_loop_start = wavelevel->loop_start;
		wavelevel->legacy_loop_end = wavelevel->loop_end;
		wavelevel->legacy_sample_count = wavelevel->sample_count;
	}
}

void mixer::update_wavelevel(int id, int waveid, int beginloop, int endloop, int numsamples, int samplerate, zzub_wave_buffer_type format, int basenote, const std::vector<int>& slices) {
	assert(wavelevels.next()[id] != 0);

	wave_level* wavelevel = &wavelevels.next_for_write(id);

	wavelevel->wave_id = waveid;
	wavelevel->loop_start = beginloop;
	wavelevel->loop_end = endloop;
	wavelevel->sample_count = numsamples;
	wavelevel->samples_per_second = samplerate;
	wavelevel->format = format;
	wavelevel->root_note = basenote;
	wavelevel->slices = slices;

	update_legacy_wavelevel(wavelevel);
}

void mixer::delete_wavelevel(int id) {
	assert(wavelevels.next()[id] != 0);

	wave_level* wavelevel = wavelevels.next()[id].get();
	wave_info* wave = &waves.next_for_write(wavelevel->wave_id);
	wave->wavelevel_count--;

	// flag sample pointer for gc collecting later (plugins may still use the wave)
	buffer_garbage.back().push_back(boost::shared_array<short>(wavelevel->legacy_sample_ptr));

	wavelevels.remove(id);
}

void mixer::set_wavelevel_samples(int id, unsigned char* ptr, int samplecount, bool legacyheader) {
	assert(wavelevels.next()[id] != 0);

	wave_level* wavelevel = &wavelevels.next_for_write(id);

	if (wavelevel->legacy_sample_ptr)
		buffer_garbage.back().push_back(boost::shared_array<short>(wavelevel->legacy_sample_ptr));

	int legacysize = legacyheader ? 8 : 0;
	wavelevel->legacy_sample_ptr = (short*)ptr;
	wavelevel->samples = (short*)(ptr + legacysize);
	wavelevel->sample_count = samplecount;
	if (legacyheader) {
		wavelevel->legacy_sample_ptr[0] = wavelevel->format;
	}

	// TODO: adjust loop points

	update_legacy_wavelevel(wavelevel);
}

void mixer::insert_midimapping(int id, int pluginid, int group, int track, int column, int midichannel, int midicc) {
	zzub::midimapping* mm = new zzub::midimapping();
	mm->id = id;
	mm->plugin_id = pluginid;
	mm->group = group;
	mm->track = track;
	mm->column = column;
	mm->channel = midichannel;
	mm->controller = midicc;

	midimappings.assign(id, mm);
}

void mixer::delete_midimapping(int id) {
	assert(midimappings.next()[id] != 0);
	midimappings.remove(id);
}

void mixer::commit() {
	if (lock_mode) {
		audiomutex.lock();
	}

	initialize_new_plugins();
	new_plugins.clear();

	// handle user thread pre-commit events
	for (std::vector<commit_event_data>::iterator i = commit_events->begin(); i != commit_events->end(); ++i) {
		switch (i->type) {
			case commit_event_insert_plugin:
				// NOTE: this matches the same plugins as in new_plugins processed separately above
				i->insert_plugin.plugin->plugin->set_stream_source(i->insert_plugin.plugin->stream_source.c_str());
				break;
			case commit_event_type_set_plugin_stream_source:
				i->set_plugin_stream_source.plugin->plugin->set_stream_source(i->set_plugin_stream_source.source->c_str());
				break;
		}
	}

	if (dirty_graph) {
		invalidate_graph();
		dirty_graph = false;
	}

	if (dirty_mutestate) {
		invalidate_mutestates();
		dirty_mutestate = false;
	}

	plugins.commit();
	connections.commit();
	midimappings.commit();
	patterns.commit();
	patternevents.commit();
	patternformats.commit();
	patterncolumns.commit();
	patternformattracks.commit();
	waves.commit();
	wavelevels.commit();
	songinfo.commit();
	patternplayers.commit();

	commit_garbage();

	audio_event_data ev;
	ev.type = audio_event_type_barrier;
	ev.barrier.commit_events = commit_events;
	invoke_audio_event(ev);

	commit_events = new std::vector<commit_event_data>(); // delete old commit_events when handling post_barrier

	if (lock_mode) { // handle serialized editing in the user thread
		process_audio_event_queue();
		audiomutex.unlock();
		lock_mode = false;
	}
}

void mixer::set_state(player_state newstate, int stoprow) {
	// pass state change to audio thread
	audio_event_data ev;
	ev.type = audio_event_type_set_state;
	ev.state_change.state = newstate;
	ev.state_change.stop_row = stoprow;
	invoke_audio_event(ev);
}

void mixer::set_play_position(int orderindex, int position) {
	audio_event_data ev;
	ev.type = audio_event_type_set_position;
	ev.set_position.position = position;
	ev.set_position.orderindex = orderindex;
	invoke_audio_event(ev);
}

void mixer::play_plugin_note(int plugin_id, int note, int prevNote, int velocity) {
	audio_event_data ev;
	ev.type = audio_event_type_play_note;
	ev.play_note.plugin = plugins.next()[plugin_id].get();
	ev.play_note.note = note;
	ev.play_note.prevnote = prevNote;
	ev.play_note.velocity = velocity;
	invoke_audio_event(ev);
}

void mixer::reset_keyjazz() {
	if (playnotes.notes.size() == 0) return ;

	audio_event_data ev;
	ev.type = audio_event_type_play_note;

	// send note off for all currently playing notes

	std::vector<keyjazz_note> keycopy = playnotes.notes;
	for (std::vector<keyjazz_note>::iterator i = keycopy.begin(); i != keycopy.end(); ++i) {
		if (i->delay_off) continue;

		ev.play_note.plugin = plugins.next()[i->plugin_id].get();
		ev.play_note.note = zzub_note_value_off;
		ev.play_note.prevnote = i->note;
		ev.play_note.velocity = 0;
		invoke_audio_event(ev);
	}
}

void mixer::set_midi_plugin(int plugin_id) {
	audio_event_data ev;
	ev.type = audio_event_type_set_midi_plugin;
	ev.set_midi_plugin.pluginid = plugin_id;
	invoke_audio_event(ev);
}

void mixer::invalidate_graph() {
	commit_event_data ev;
	ev.type = commit_event_set_graph;
	ev.set_graph.plugins = new std::vector<metaplugin*>();
	ev.set_graph.backedges = new std::vector<metaconnection*>();

	plugingraph pg(*ev.set_graph.plugins, *ev.set_graph.backedges);
	pg.make(plugins.next(), connections.next());

	invoke_commit_event(ev);
}

void mixer::invalidate_state_format(int pluginid) {
	commit_event_data ev;
	std::vector<metaconnection*> newinconnections;
	std::vector<metaconnection*> newoutconnections;
	metaplugin* k = &plugins.next_for_write(pluginid);
	dirty_graph = true;

	for (std::vector<boost::shared_ptr<metaconnection> >::iterator i = connections.next().begin(); i != connections.next().end(); ++i) {
		if (*i != 0 && (*i)->to_plugin_id == pluginid)
			newinconnections.push_back((*i).get());
		if (*i != 0 && (*i)->from_plugin_id == pluginid)
			newoutconnections.push_back((*i).get());
	}
	k->connections = newinconnections;
	k->out_connections = newoutconnections;

	create_state_pattern(k->info, k->tracks, k->state_automation, false);
	create_state_pattern(k->info, k->tracks, k->state_write, true);
	create_state_pattern(k->info, k->tracks, k->state_last, true);

	ev.type = commit_event_set_state_format;
	ev.set_state_format.newplugin = plugins.next()[pluginid].get();
	if (plugins._top && pluginid < (int)plugins.top().size())
		ev.set_state_format.oldplugin = plugins.top()[pluginid].get();
	else
		ev.set_state_format.oldplugin = 0;
	invoke_commit_event(ev);
}

void mixer::set_parameter_mode(int pluginid, int group, int track, int column, int mode) {
	metaplugin* k = &plugins.next_for_write(pluginid);
	assert(k != 0);

	dirty_graph = true;

	const zzub::parameter* param = k->get_parameter_info(group, track, column);
	assert(param != 0);

	k->state_write.interpolators[group][track][column].reset(mode);
}


void mixer::invalidate_mutestates() {
	std::map<mutetrackkeytype, int> mutestates;

	// for each patternformattrack, if is_muted, add to mutestates

	for (std::vector<boost::shared_ptr<patternformattrack> >::iterator i = patternformattracks.next().begin(); i != patternformattracks.next().end(); ++i) {
		if (*i != 0 && (*i)->is_muted) {
			mutetrackkeytype mtkt = { (*i)->formatid, (*i)->pluginid, (*i)->group, (*i)->track };
			mutestates[mtkt] = (*i)->id;
		}
	}

	commit_event_data ev;
	ev.type = commit_event_set_mutestate;
	ev.set_mutestate.mutestate = new std::map<mutetrackkeytype, int>(mutestates);
	invoke_commit_event(ev);
}

void mixer::add_user_listener(user_event_listener* listener) {
	user_listeners.push_back(listener);
}

void mixer::remove_user_listener(user_event_listener* listener) {
	std::vector<user_event_listener*>::iterator i = std::find(user_listeners.begin(), user_listeners.end(), listener);
	if (i != user_listeners.end())
		user_listeners.erase(i);
}

bool mixer::notify_user_event_listeners(user_event_data& data) {
	bool handled = false;
	// notify listeners
	for (std::vector<user_event_listener*>::iterator i = user_listeners.begin(); i != user_listeners.end(); ++i) {
		handled |= (*i)->user_event(data);
	}

	// whenever something is committed, we need to release event-specific memory
	if (data.type == user_event_type_threads_change) {
		// signal and wait for thread stoppage
		finalize_thread_count(data.threads_change);
	} else
	if (data.type == user_event_type_append_samples) {
		// buffer goes allocatorthread->audiothread->encoderthread->userthread->here:
		delete data.append_samples.buffer;
		delete data.append_samples.slices;
	} else
	if (data.type == user_event_type_post_barrier) {
		for (std::vector<commit_event_data>::iterator i = data.post_barrier.commit_events->begin(); i != data.post_barrier.commit_events->end(); ++i)
			finalize_commit_event(*i);
		delete data.post_barrier.commit_events; // no need to stick with these any more
		finalize_garbage();
	}
	return handled;
}

void mixer::process_user_event_queue() {
	while (!user_event_queue.empty()) {
		user_event_data data;
		user_event_queue.pop(data);
		notify_user_event_listeners(data);
	}

	while (!encoder_user_event_queue.empty()) {
		user_event_data data;
		encoder_user_event_queue.pop(data);
		notify_user_event_listeners(data);
	}
}

int mixer::get_currently_playing_row(int patternid) {
	assert(in_user_thread());

	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.next().begin(); i != patternplayers.next().end(); i++) {
		patternplayer* pp = i->get();
		if (pp->playing && pp->pattern_id == patternid)
			return pp->pattern_row;
	}
/*
	for (std::vector<boost::shared_ptr<metaplugin> >::iterator i = plugins.next().begin(); i != plugins.next().end(); ++i) {
		if (*i == 0 || ((*i)->info->flags & zzub_plugin_flag_is_sequence) == 0) continue;

		int row = (*i)->plugin->get_currently_playing_row(patternid);
		if (row != -1) return row;
	}*/
	return -1;
}

//
// internals
//

void mixer::finalize_commit_event(commit_event_data& data) {
	switch (data.type) {
		case commit_event_set_graph:
			delete data.set_graph.plugins;
			delete data.set_graph.backedges;
			break;
		case commit_event_set_state_format:
			break;
		case commit_event_set_mutestate:
			delete data.set_mutestate.mutestate;
			break;
		case commit_event_insert_patternevent:
			if (data.insert_patternevent.ev)
				delete data.insert_patternevent.ev;
			break;
		case commit_event_remove_patternevent:
			delete data.remove_patternevent.ev;
			break;
		case commit_event_type_set_plugin_stream_source:
			data.set_plugin_stream_source.plugin->plugin->flush_stream_source();
			delete data.set_plugin_stream_source.source;
			break;
	}
}

bool mixer::invoke_user_event(user_event_data& data) {
	bool handled = false;
	if (in_encoder_thread())
		encoder_user_event_queue.push(data); // queue in encoder->user queue
	else if (in_user_thread())
		handled = notify_user_event_listeners(data); // invoke directly in user thread
	else
		user_event_queue.push(data); // assume audio thread, queue in audio->user queue
	return handled;
}

bool mixer::invoke_audio_event(audio_event_data& data) {
	return audio_event_queue.push(data);
}

bool mixer::invoke_commit_event(commit_event_data& data) {
	commit_events->push_back(data);
	return true;
}

void mixer::on_set_position(int orderindex, int position) {
	// reset plugins and sequences to song position 0

	song_position = work_position = 0;
	rootplayer->_master_info.tick_position = 0;
	rootplayer->_master_info.tick_position_frac = 0.0f;
	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.top().begin(); i != patternplayers.top().end(); i++) {
		patternplayer* pp = i->get();
		pp->pattern_id = -1;
		pp->subtick_frac_remainder = 0.0f;
	}
	set_orderlist_index(orderindex);
	process_seek(position);
	song_position = rootplayer->sample_position;
}

void mixer::on_set_state(player_state newstate, int stoprow) {
	state = newstate;

	// send current encoder buffers before setting encoder state
	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		metaplugin* k = *i;
		if (k->info->flags & zzub_plugin_flag_is_encoder) process_encoder_buffer(k);
	}

	if (state == player_state_stopped) {
		patternplayer_stop(patternplayers.top(), rootplayer);
		rootplayer->_master_info.tick_position = 0;
		for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
			(*i)->plugin->stop();

			// stops hung notes for plugins which don't stop notes when plugin->stop()'d
			// TODO: please remind me why this must be handled globally and not in the plugin loaders:
			/*
			{	int note_group = (*i)->info->note_group;
				int note_col = (*i)->info->note_column;
				if (note_group != -1) {
					int track_count = (note_group == 2) ? (*i)->tracks : 1;
					for (int j = 0; j < track_count; ++j) {
						(*i)->set_parameter(note_group, j, note_col, 255, false);
					}
				}
			}*/
		}
		encodermgr.set_state(zzub_encoder_state_stopped);
	} else
	if (state == zzub_player_state_playing) {
		if (orderlist_position < (int)songinfo.top_item(0).orderlist.size())
			rootplayer->set_pattern(songinfo.top_item(0).orderlist[orderlist_position], 0, false);
		else
			rootplayer->set_pattern(-1, 0, false);

		patternplayer_play(patternplayers.top(), rootplayer, stoprow);
		reset_interval_plugins();
		work_position = song_position; // interval can use work_position to feed their lfos. work_position will keep running when the song is stopped, but reset to the song position when playing again
		encodermgr.set_state(zzub_encoder_state_playing);
	}

	user_event_data data = { user_event_type_state_change };
	data.state_change.state = state;
	invoke_user_event(data);
}

void mixer::on_play_note(metaplugin* k, int note, int prevnote, int velocity) {
	// find note_group, track, column and velocity_group, track and column based on keyjazz-struct
	int note_group = -1, note_track = -1, note_column = -1;
	int velocity_column = -1;
	playnotes.set_keyjazz_note(*k, note, prevnote, velocity, note_group, note_track, note_column, velocity_column);

	if (note_group != -1) {
		//k->sequencer_state = sequencer_event_type_none;
		k->set_parameter(note_group, note_track, note_column, note, true);
		if (velocity_column != -1 && velocity != 0)
			k->set_parameter(note_group, note_track, velocity_column, velocity, true);

		//on_process_events(k);
	}
}

void mixer::on_set_midi_plugin(int pluginid) {
	midi_plugin = pluginid;
}

void mixer::on_set_parameter(int pluginid, int group, int track, int column, int value, bool record) {
	metaplugin* k = plugins.top()[pluginid].get();
	assert(k != 0); // was plugin deleted?
	
	// NOTE: could these checks be removed now? they had a purpose before adding 
	// reset_pending_plugin_commit_events(), but there is still the audio_event-path 
	// which could be initiated by (broken?) plugins (??? - needs testing)
	if (k == 0) return ; 

	// set_parameter on a track that was deleted?
	if (group == 2 && track >= k->tracks) return ;

	k->set_parameter(group, track, column, value, record);
}

void mixer::on_set_state_format(metaplugin* oldplugin, metaplugin* newplugin) {
	metaplugin* k = newplugin;
	assert(k != 0);

	// newplugin is the new plugin being swapped in as part of copy-on-write.
	// oldplugin is the plugin being replaced, containing the previous state.
	// the engine might have altered the state since it was copied in the user thread, 
	// so it is desirable to copy the latest state values into the new state:
	if (oldplugin) {
		k->transfer_parameter_row(1, oldplugin->state_write, k->state_write, false);
		k->transfer_parameter_row(2, oldplugin->state_write, k->state_write, false);
		k->transfer_parameter_row(1, oldplugin->state_last, k->state_last, false);
		k->transfer_parameter_row(2, oldplugin->state_last, k->state_last, false);
		k->transfer_parameter_row(1, oldplugin->state_automation, k->state_automation, false);
		k->transfer_parameter_row(2, oldplugin->state_automation, k->state_automation, false);
	}

	assert(k->tracks >= (int)k->info->min_tracks && k->tracks <= (int)k->info->max_tracks);

	k->plugin->set_track_count(k->tracks);
	//k->plugin->set_stream_source(k->stream_source.c_str());

	on_update_plugin_timesource(k, true);
}

void mixer::on_update_plugin_timesource(metaplugin* k, bool notifyplugin) {
	patternplayer* timesource = 0;
	if (k->timesource_plugin_id != -1) {
		metaplugin* tp;
		if (in_user_thread())
			tp = plugins.next()[k->timesource_plugin_id].get(); else
			tp = &plugins.top_item(k->timesource_plugin_id);
		timesource = tp->get_patternplayer(k->timesource_group, k->timesource_track);
	}

	if (timesource == 0) timesource = rootplayer;
	k->plugin->_master_info = &timesource->_master_info;
	if (notifyplugin) k->plugin->update_timesource();
}

bool is_sequence_plugin(zzub::metaplugin* k) {
	return ((k->info->flags & zzub_plugin_flag_is_sequence) != 0 || 
		(k->info->flags & zzub_plugin_flag_stream) != 0 || 
		(k->info->flags & zzub_plugin_flag_has_event_output) != 0);
}

void mixer::set_plugin_sequence_chain(zzub::metaplugin* k, bool in_sequence_chain) {
	if (in_sequence_chain || is_sequence_plugin(k)) {
		in_sequence_chain = true;
	} else
		in_sequence_chain = false;

	k->audiodata->in_sequence_chain = in_sequence_chain;
	
	// cannot rely on k->connections unless its maintained somewhere else

	const std::vector<boost::shared_ptr<metaconnection> >& conns = (in_user_thread() ? connections.next() : connections.top());

	for (std::vector<boost::shared_ptr<metaconnection> >::const_iterator i = conns.begin(); i != conns.end(); ++i) {
		//for (std::vector<metaconnection*>::iterator i = k->connections.begin(); i != k->connections.end(); ++i) {
		metaconnection* conn = i->get();
		if (conn == 0) continue;
		if (conn->to_plugin_id != k->id) continue;
		if (conn->is_back_edge) continue;
		metaplugin* fromplugin;
		if (in_user_thread())
			fromplugin = plugins.next()[conn->from_plugin_id].get(); else
			fromplugin = &plugins.top_item(conn->from_plugin_id);
		assert(fromplugin != 0); // TODO: asserts during cleanup (connection to deleted plugin)
		//metaplugin* fromplugin = plugins.top()[conn->from_plugin_id].get();
		set_plugin_sequence_chain(fromplugin, in_sequence_chain);
	}
}

void mixer::on_barrier(std::vector<commit_event_data>* events) {

	plugins.pop();
	connections.pop();
	midimappings.pop();
	patterns.pop();
	patternevents.pop();
	patternformats.pop();
	patterncolumns.pop();
	patternformattracks.pop();
	waves.pop();
	wavelevels.pop();
	songinfo.pop();
	patternplayers.pop();

	user_event_data userevent;
	userevent.type = user_event_type_committed;

	for (std::vector<commit_event_data>::iterator i = events->begin(); i != events->end(); ++i) {
		switch (i->type) {
			case commit_event_set_parameter:
				on_set_parameter(i->set_parameter.pluginid, i->set_parameter.group, i->set_parameter.track, i->set_parameter.column, i->set_parameter.value, i->set_parameter.record);
				break;
			case commit_event_set_state_format:
				on_set_state_format(i->set_state_format.oldplugin, i->set_state_format.newplugin);
				break;
			case commit_event_set_graph:
				workorder.swap(*i->set_graph.plugins);
				for (std::vector<boost::shared_ptr<metaconnection> >::const_iterator j = connections.top().begin(); j != connections.top().end(); ++j) {
					if (!j->get()) continue;
					j->get()->is_back_edge = false;
				}
				for (std::vector<metaconnection*>::iterator j = i->set_graph.backedges->begin(); j != i->set_graph.backedges->end(); ++j) {
					(*j)->is_back_edge = true;
				}
				for (std::vector<metaplugin*>::iterator j = workorder.begin(); j != workorder.end(); ++j) {
					if ((*j)->out_connections.empty())
						set_plugin_sequence_chain(*j, false);
				}
				dirty_latencies = true; // update latency compensation later
				break;
			case commit_event_insert_connection:
				on_insert_connection(i->insert_connection.conn, i->insert_connection.to_plugin, i->insert_connection.from_plugin);
				break;
			case commit_event_delete_connection:
				on_delete_connection(i->delete_connection.conn, i->delete_connection.to_plugin, i->delete_connection.from_plugin);
				break;
			case commit_event_process_events:
				//assert(false); // no-op, events are processed later
				break;
			case commit_event_set_audio_connection:
				on_update_audioconnection(i->set_audio_connection.connid);
				break;
			case commit_event_set_attribute:
				on_set_attribute(i->set_attribute.plugin, i->set_attribute.index, i->set_attribute.value);
				break;
			case commit_event_set_mutestate:
				mutestate.swap(*i->set_mutestate.mutestate);
				break;
			case commit_event_insert_patternevent:
				on_insert_patternevent(i);
				break;
			case commit_event_remove_patternevent:
				on_remove_patternevent(i);
				break;
			case commit_event_update_patternevent:
				on_update_patternevent(i);
				break;
			case commit_event_insert_plugin:
				on_insert_plugin(i->insert_plugin.plugin);
				break;
			case commit_event_delete_plugin:
				on_delete_plugin(i->delete_plugin.plugin);
				break;
			case commit_event_set_tpb:
				// set_speed -> on_tempo_change -> update sliders etc
				rootplayer->set_speed(rootplayer->_master_info.beats_per_minute, i->set_tpb.tpb, rootplayer->_master_info.swing_amount, rootplayer->_master_info.swing_ticks);
				break;
			case commit_event_set_bpm:
				rootplayer->set_speed(i->set_bpm.bpm, rootplayer->_master_info.ticks_per_beat, rootplayer->_master_info.swing_amount, rootplayer->_master_info.swing_ticks);
				break;
			case commit_event_set_swing:
				rootplayer->set_speed(rootplayer->_master_info.beats_per_minute, rootplayer->_master_info.ticks_per_beat, i->set_swing.swing_amount, rootplayer->_master_info.swing_ticks);
				break;
			case commit_event_set_swing_ticks:
				rootplayer->set_speed(rootplayer->_master_info.beats_per_minute, rootplayer->_master_info.ticks_per_beat, rootplayer->_master_info.swing_amount, i->set_swing_ticks.swing_ticks);
				break;
			case commit_event_orderlist_timeshift:
				on_sync_orderlist_timeshift(i->orderlist_timeshift.index, i->orderlist_timeshift.timeshift);
				break;
			case commit_event_type_set_plugin_latency:
				on_set_plugin_latency(i->set_plugin_latency.plugin_id, i->set_plugin_latency.latency);
				break;
			case commit_event_type_set_plugin_stream_source:
				i->set_plugin_stream_source.plugin->plugin->enable_stream_source();
				break;

		}
	}

	// since we own the commit events, pass its pointer to the user thread for cleanup.
	// see the handling of user_event_type_post_barrier in notify_user_event_listeners()
	userevent.type = user_event_type_post_barrier;
	userevent.post_barrier.commit_events = events;

	invoke_user_event(userevent);
}

void mixer::on_insert_patternevent(std::vector<commit_event_data>::iterator i) {
	pattern_events* events = patternevents.top()[i->insert_patternevent.patternid].get();
	events->by_id.insert(*(i->insert_patternevent.ev));
	events->by_time.insert(*(i->insert_patternevent.ev));
	i->insert_patternevent.ev = 0;
}

void mixer::on_remove_patternevent(std::vector<commit_event_data>::iterator i) {
	pattern_events* events = patternevents.top()[i->remove_patternevent.patternid].get();
	pattern_events_by_id::iterator j = events->by_id.find(i->remove_patternevent.id, hash_by_id(), equal_by_id());
	if (j != events->by_id.end()) {
		pattern_events_by_time::iterator k = pattern_events_by_time::s_iterator_to(*j);
		assert(k->id == j->id);
		events->by_id.erase(j);
		events->by_time.erase(k);
		i->remove_patternevent.ev = &(*j);
	}
}

void mixer::on_update_patternevent(std::vector<commit_event_data>::iterator i) {
	pattern_events* events = patternevents.top()[i->update_patternevent.patternid].get();
	pattern_events_by_id::iterator j = events->by_id.find(i->update_patternevent.id, hash_by_id(), equal_by_id());
	if (j != events->by_id.end()) {
		j->value = i->update_patternevent.value;
		j->pluginid = i->update_patternevent.pluginid;
		j->group = i->update_patternevent.group;
		j->track = i->update_patternevent.track;
		j->column = i->update_patternevent.column;
		if (j->time != i->update_patternevent.time) {
			j->time = i->update_patternevent.time;
			pattern_events_by_time::iterator k = pattern_events_by_time::s_iterator_to(*j);
			assert(k->id == j->id);
			events->by_time.erase(k);
			events->by_time.insert(*j);
		}
	}
}

void mixer::process_audio_event_queue() {
	while (!audio_event_queue.empty()) {
		audio_event_data data;
		audio_event_queue.pop(data);

		switch (data.type) {
			case audio_event_type_play_note:
				on_play_note(data.play_note.plugin, data.play_note.note, 
					data.play_note.prevnote, data.play_note.velocity);
				break;
			case audio_event_type_set_state:
				on_set_state(data.state_change.state, data.state_change.stop_row);
				break;
			case audio_event_type_set_position:
				on_set_position(data.set_position.orderindex, data.set_position.position);
				break;
			case audio_event_type_set_midi_plugin:
				on_set_midi_plugin(data.set_midi_plugin.pluginid);
				break;
			case audio_event_type_alter_parameter:
				on_set_parameter(data.alter_parameter.pluginid, data.alter_parameter.group, data.alter_parameter.track, data.alter_parameter.column, data.alter_parameter.value, data.alter_parameter.record);
				break;
			case audio_event_type_process_events:
				//on_process_events(data.process_events.plugin);
				process_graph_events();
				break;
			case audio_event_type_barrier:
				on_barrier(data.barrier.commit_events);
				break;
			case audio_event_type_samplerate_changed:
				on_set_samplerate(data.samplerate_changed.samplerate);
				break;
			case audio_event_type_play_pattern:
				on_play_pattern(data.play_pattern.pattern_id, data.play_pattern.row, data.play_pattern.stoprow);
				break;
			case audio_event_type_queue_index: {
				queue_index = data.queue_index.index;
				user_event_data data = { user_event_type_order_queue_change };
				invoke_user_event(data);
				// TODO: notify user thread the index was updated?
				break;
			}
			case audio_event_type_order_position:
				set_orderlist_index(data.order_position.index);
				rootplayer->reset_interpolators();
				break;
			case audio_event_type_midi_event:
				on_midi_out(data.midi_event.plugin_id, data.midi_event.message);
				break;
			case audio_event_type_note_event:
				on_note_out(data.note_event.plugin_id, data.note_event.message);
				break;
			case audio_event_type_set_thread_count:
				on_set_thread_count(data.set_thread_count.thread_count);
				break;
		}
	}
}

bool mixer::format_has_column(int formatid, int pluginid, int group, int track, int column) {
	for (std::vector<boost::shared_ptr<patternformatcolumn> >::const_iterator i = patterncolumns.top().begin(); i != patterncolumns.top().end(); ++i) {
		patternformatcolumn* col = i->get();
		if (col != 0 && col->formatid == formatid && col->pluginid == pluginid && col->group == group && col->track == track && col->column == column) return true;
	}
	return false;
}

bool mixer::get_currently_playing_pattern_row_for_column(int pluginid, int group, int track, int column, int* pattern_id, int* pattern_row) {
	// when recording parameters
	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.top().begin(); i != patternplayers.top().end(); i++) {
		patternplayer* pp = i->get();
		if (!pp->playing || pp->pattern_id == -1) continue;

		pattern* pat = patterns.top()[pp->pattern_id].get();
		if (!pat) continue;

		if (format_has_column(pat->formatid, pluginid, group, track, column)) {
			*pattern_id = pp->pattern_id;
			*pattern_row = pp->pattern_row;
			return true;
		}
	}
	return false;
}

void mixer::on_parameter_change(int pluginid, int group, int track, int column, int value, bool record) {
	int pattern_id = -1, pattern_row = -1;
	if (record && is_recording_parameters) {
		get_currently_playing_pattern_row_for_column(pluginid, group, track, column, &pattern_id, &pattern_row);
	}

	user_event_data event_data;
	event_data.type = user_event_type_parameter_change;
	event_data.parameter_change.id = pluginid;
	event_data.parameter_change.group = group;
	event_data.parameter_change.track = track;
	event_data.parameter_change.column = column;
	event_data.parameter_change.value = value;
	event_data.parameter_change.automation_pattern = pattern_id;
	event_data.parameter_change.automation_timestamp = pattern_row;
	invoke_user_event(event_data);
}

// changes a parameter value via the undo buffer
void mixer::set_parameter(int id, int group, int track, int column, int value, bool record) {
	metaplugin* k = &plugins.next_for_write(id);

	k->set_parameter(group, track, column, value, false);

	commit_event_data event_data;
	event_data.type = commit_event_set_parameter;
	event_data.set_parameter.pluginid = id;
	event_data.set_parameter.group = group;
	event_data.set_parameter.track = track;
	event_data.set_parameter.column = column;
	event_data.set_parameter.value = value;
	event_data.set_parameter.record = record;
	invoke_commit_event(event_data);

	dirty_graph = true;
}

int mixer::get_parameter(int id, int group, int track, int column) {
	bool immediate = user_thread_id == thread_id::get();
	if (immediate)
		return plugins.next()[id]->get_parameter(group, track, column);
	else
		return plugins.top()[id]->get_parameter(group, track, column);
}

// alters the parameter value on a committed plugin, bypasses the undo buffer
void mixer::alter_parameter(int id, int group, int track, int column, int value, bool record) {
	audio_event_data event_data;
	event_data.type = audio_event_type_alter_parameter;
	event_data.alter_parameter.pluginid = id;
	event_data.alter_parameter.group = group;
	event_data.alter_parameter.track = track;
	event_data.alter_parameter.column = column;
	event_data.alter_parameter.value = value;
	event_data.alter_parameter.record = record;
	invoke_audio_event(event_data);
}

// set_parameter_check() is thread safe and bypasses the undo buffer
void mixer::set_parameter_check(int id, int group, int track, int column, int value, bool record) {
	if (in_user_thread()) {
		metaplugin* k = plugins.next()[id].get();
		if (k->initialized) {
			alter_parameter(id, group, track, column, value, record);
		} else {
			set_parameter(id, group, track, column, value, record);
		}
	} else {
		on_set_parameter(id, group, track, column, value, record);
	}
}

void mixer::process_events(int plugin_id, bool immediate_mode) {
	if (immediate_mode) {
		audio_event_data ev;
		ev.type = audio_event_type_process_events;
		ev.process_events.plugin = plugins.next()[plugin_id].get();
		invoke_audio_event(ev);
	} else {
		commit_event_data ev;
		ev.type = commit_event_process_events;
		ev.process_events.plugin = plugins.next()[plugin_id].get();
		invoke_commit_event(ev);
	}
}

void mixer::on_set_samplerate(int newsamplerate) {
	int samplerate = rootplayer->_master_info.samples_per_second;
	song_position = convert_sample_count(song_position, samplerate, newsamplerate);
	rootplayer->_master_info.samples_per_second = newsamplerate;
	for (std::vector<boost::shared_ptr<patternplayer> >::const_iterator i = patternplayers.top().begin(); i != patternplayers.top().end(); ++i) {
		(*i)->_master_info.samples_per_second = newsamplerate;
	}

	user_event_data ev;
	ev.type = user_event_type_samplerate_changed;
	ev.samplerate_changed.samplerate = newsamplerate;
	invoke_user_event(ev);

	falloff = std::pow(10.0f, (-48.0f / (samplerate * 20.0f))); // vu meter falloff (-48dB/s)
}

void mixer::set_samplerate(int samplerate) {
	audio_event_data ev;
	ev.type = audio_event_type_samplerate_changed;
	ev.samplerate_changed.samplerate = samplerate;
	invoke_audio_event(ev);
}

void mixer::on_set_attribute(metaplugin* plugin, int index, int value) {
	plugin->plugin->attributes[index] = value;
	plugin->plugin->attributes_changed();
}

void mixer::set_attribute(int plugin_id, int index, int value) {
	commit_event_data ev;
	ev.type = commit_event_set_attribute;
	ev.set_attribute.plugin = plugins.next()[plugin_id].get();
	ev.set_attribute.index = index;
	ev.set_attribute.value = value;
	invoke_commit_event(ev);
}

void mixer::on_play_pattern(int pattern_id, int row, int stoprow) {
	rootplayer->set_pattern(pattern_id, 0, true);
	process_seek(row);
	rootplayer->play(stoprow);
}

void mixer::play_pattern(int pattern_id, int row, int stoprow) {
	audio_event_data ev;
	ev.type = audio_event_type_play_pattern;
	ev.play_pattern.pattern_id = pattern_id;
	ev.play_pattern.row = row;
	ev.play_pattern.stoprow = stoprow;
	invoke_audio_event(ev);
}

// 
// processing
//

void mixer::process_graph_singlethreaded(int sample_count) {
	// process plugins
	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		// process audio
		process_stereo(*i, sample_count);
	}
}

void mixer::process_graph_multithreaded_worker(multithreadworker* worker) {

	bool expected;
	//SetThreadPriority(worker->hThread, THREAD_PRIORITY_HIGHEST );
	thread_id::set_priority(worker->thread, threadpriority_high);

	while (!worker->quitting) {
		worker->start_event->wait();

		if (worker->quitting) break;

		metaplugin* m;
		while (thread_workqueue.pop(m)) {
			process_stereo(m, worker->sample_count);

			// decrease dependency counter in each input plugin
			for (std::vector<metaconnection*>::iterator i = m->out_connections.begin(); i != m->out_connections.end(); ++i) {
				metaplugin* outplugin = plugins.top()[(*i)->to_plugin_id].get();
				int deps = outplugin->audiodata->dependencies.fetch_sub(1) - 1; // fetch_sub returns the old value, subtract 1 again
				if (deps == 0) {
					thread_workqueue.push(outplugin);

					// restart sleeping threads atomicly:
					for (std::vector<boost::shared_ptr<multithreadworker> >::const_iterator j = threadworkers.top().begin(); j != threadworkers.top().end(); ++j) {
						multithreadworker* threadworker = j->get();
						if (threadworker == worker) continue;
						
						expected = true;
						if (threadworker->sleeping.compare_exchange_strong(expected, false)) {
							thread_sleep_count--;
							threadworker->start_event->signal(); 
						}
					}
				}
			}
		}

		expected = false;
		if (!worker->sleeping.compare_exchange_strong(expected, true)) {
			assert(false); // already sleeping - shouldnt happen
		}

		if (thread_sleep_count.fetch_add(1) == threadworkers.top().size() - 1) {
			// last thread going to sleep
			thread_done_event.signal();
		}

	}
}

void mixer::process_graph_multithreaded(int sample_count) {
	// add all leaves - ie generators etc - to the work queue
	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		(*i)->audiodata->dependencies.store((int)(*i)->connections.size());
		if ((*i)->connections.size() == 0) {
			thread_workqueue.push(*i);
		}
	}

	thread_sleep_count.store(0);

	// tell worker threads to process the queue
	for (std::vector<boost::shared_ptr<multithreadworker> >::const_iterator i = threadworkers.top().begin(); i != threadworkers.top().end(); ++i) {
		(*i)->sample_count = sample_count;
		(*i)->sleeping = false;
	}
	for (std::vector<boost::shared_ptr<multithreadworker> >::const_iterator i = threadworkers.top().begin(); i != threadworkers.top().end(); ++i) {
		(*i)->start_event->signal();
	}

	// wait for all worker threads to finish
	thread_done_event.wait();
}

extern "C" void* mixer_multithread_worker(void* param) {

	multithreadworker* worker = (multithreadworker*)param;

	worker->mixer->process_graph_multithreaded_worker(worker);
	return 0;
}

void mixer::finalize_thread_count(user_event_data::user_event_data_threads_change& ev) {
	// if the number of threads decrease, stop the worker threads now that we're back in the user thread
	if (ev.old_threads != 0) {
		int new_thread_count = ev.thread_count;
		if (new_thread_count == 1) {
			// 1 worker thread = use audio thread so free the last one as well
			new_thread_count = 0;
		}
		for (int i = new_thread_count; i < (int)ev.old_threads->size(); i++) {
			boost::shared_ptr<multithreadworker> worker = (*ev.old_threads)[i];
			worker->quitting = true;
			worker->start_event->signal();
			thread_id::join(worker->thread);
			delete worker->start_event;
		}
		ev.old_threads->clear();
	}
	threadworkers.gc();
}

void mixer::on_set_thread_count(int threads) {
	// if the number of threads decrease, queue the decreased threads for stopping in the user thread!
	user_event_data ev;
	ev.type = user_event_type_threads_change;
	ev.threads_change.thread_count = threads;

	// keep a pointer to the array at threadworksers.top() and pass it back to the user thread - it is valid until then
	ev.threads_change.old_threads = threadworkers._top;

	threadworkers.pop();

	assert(threads == 1 || threadworkers.top().size() == threads); // check new count

	thread_count = threads;

	invoke_user_event(ev);
}

void mixer::set_thread_count(int threads) {

	assert(threads >= 1);

	if (threads == 1) {
		// use the main thread only
		threadworkers.next().clear();
	} else {
		// create new worker threads - if the number of threads is reduced, it'll be handled in on_set_thread_count and finalize_set_thread_count
		int curthreadcount = (int)threadworkers.next().size();
		threadworkers.next().resize(threads);
		for (int i = curthreadcount; i < threads; i++) {
			//DWORD threadID;
			multithreadworker* worker = new multithreadworker();
			worker->quitting = false;
			worker->mixer = this;
			worker->start_event = new synchronization::event();
			//worker->hThread = CreateThread(0, 0, mixer_multithread_worker, worker, 0, &threadID);
			worker->thread = thread_id::create_thread(mixer_multithread_worker, worker);
			threadworkers.assign(i, worker);
		}
	}

	// stop threads = tell audio thread about new thread count, audio thread tells user thread about stopped thread handles, user thread stops thread

	threadworkers.commit();

	audio_event_data ev;
	ev.type = audio_event_type_set_thread_count;
	ev.set_thread_count.thread_count = threads;
	invoke_audio_event(ev);

}

void mixer::process_graph(int sample_count) {
	if (sample_count == 0) return ;

	assert(thread_count > 0);
	if (thread_count == 1) {
		process_graph_singlethreaded(sample_count);
	} else {
		process_graph_multithreaded(sample_count);
	}

	// process encoder, pdc and midi
	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		metaplugin* k = *i;

		process_encoder(k, sample_count);
		k->state_write.clear_group(3); // quick insertion, probably wrong. this could/should be at the end of the sequence processing instead of end of audio processing
	}

	// update internal stuff
	work_position += sample_count;
	latency_buffer_position = (latency_buffer_position + sample_count) % zzub_latency_buffer_size;

	if (state == zzub_player_state_playing) 
		song_position += sample_count;
}

void mixer::process_stereo_chunk(metaplugin* k, int writepos, float** pin, float** pout, int sample_count, int flags) {

	float *plout[mixer::max_channels];
	float *plin[mixer::max_channels];

	for (int i = 0; i < k->info->outputs; i++) {
		if (pout[i] != 0)
			plout[i] = pout[i] + k->audiodata->latency_write;
		else
			plout[i] = 0;
	}
	for (int i = 0; i < k->info->inputs; i++) {
		if (pin[i] != 0)
			plin[i] = pin[i] + writepos;
		else
			plin[i] = 0;
	}

	k->audiodata->output_buffer_write = writepos; // thread local variable must be accessible from plugins accessing mixer::output_buffer
	if (k->audiodata->is_muted || k->audiodata->is_softmuted) {
		for (int i = 0; i < k->info->outputs; i++) 
			plout[i] = 0;
	} else
	if (k->audiodata->is_bypassed || k->audiodata->is_softbypassed) {
		// bypass: copy from input to output, clear the remaining outputs
		int minchannels = std::min(k->info->outputs, k->info->inputs);

		for (int i = 0; i < minchannels; i++) {
			if (plout[i] != 0)
				if (plin[i] != 0)
					memcpy(plout[i], plin[i], sample_count * sizeof(float));
				else 
					plout[i] = 0;
		}
		for (int i = minchannels; i < k->info->outputs; i++) {
			plout[i] = 0;
		}
	} else {
		k->plugin->process_stereo(plin, plout, sample_count, flags);
		// assume plugins change the buffer pointers in plout (in addition to clearing silent channels)
		// reset the plout pointers for later
		for (int i = 0; i < k->info->outputs; i++) {
			if (plout[i] != 0) plout[i] = pout[i] + k->audiodata->latency_write;
		}
	}

	// find max peak and doublecheck nulled silent buffers
	float samplerate = float(rootplayer->_master_info.samples_per_second);
	for (int i = 0; i< k->info->outputs; i++) {
		if (plout[i] == 0) {
			pout[i] = 0;
			k->audiodata->last_work_max[i] = 0;
		} else {
			if (scan_peak_channel(plout[i], sample_count, &k->audiodata->last_work_max[i], falloff)) {
				pout[i] = 0;
				plout[i] = 0;
			}
		}
	}

	// copy plugin output to the feedback buffer (TODO: if has-back-edge)
	for (int i = 0; i < k->info->outputs; i++) {
		if (plout[i] != 0)	{
			std::copy(k->feedback_buffer[i].get() + sample_count, k->feedback_buffer[i].get() + zzub_buffer_size, k->feedback_buffer[i].get());
			std::copy(plout[i], plout[i] + sample_count, k->feedback_buffer[i].get() + zzub_buffer_size - sample_count);
		} else {
			// TODO: clear feedback buffer?
		}
	}

	// copy plugin -input- to the encoder framebuffer - encoders usually have no plugin_flag_has_audio_output and thus no output buffers
	if (k->info->flags & zzub_plugin_flag_is_encoder) {
		// plugin has input = copy to encoder frame
		for (int i = 0; i < k->info->inputs; i++) {
			if (pin[i] != 0)
				memcpy(k->audiodata->encoder_frame_buffer[i].get() + writepos, pin[i] + writepos, sample_count * sizeof(float));
			else
				memset(k->audiodata->encoder_frame_buffer[i].get() + writepos, 0, sample_count * sizeof(float));
		}
	}
}

void mixer::process_stereo(metaplugin* k, int sample_count) {

	float *plin[max_channels] = {0};   // pointers to mix_buffer channels in use by input connections
	float *plout[max_channels] = {0};  // pointers to the plugins work_buffer channels
	float *plmix[max_channels] = {0};  // copy of channels in plin with non-silent input
	float *plconn[max_channels] = {0}; // copy of plin

	double start_time = timer.frame();

	// process connections
	float mix_buffer[max_channels][zzub_buffer_size * 2];

	for (std::vector<metaconnection*>::iterator i = k->connections.begin(); i != k->connections.end(); ++i) {
		metaconnection* conn = *i;
		metaplugin* plugin_self = plugins.top()[conn->plugin_id].get();
		for (int j = 0; j < conn->input_count; j++) {
			plin[conn->first_input + j] = mix_buffer[conn->first_input + j];
		}
	}
	memcpy(plconn, plin, sizeof(plin));

	for (std::vector<metaconnection*>::iterator i = k->out_connections.begin(); i != k->out_connections.end(); ++i) {
		metaconnection* conn = *i;
		for (int j = 0; j < conn->output_count; j++) {
			plout[conn->first_output + j] = k->work_buffer[conn->first_output + j].get();
		}
	}

	// ringbuffer offsets
	int beforecycle = zzub_latency_buffer_size - k->audiodata->latency_write;
	int aftercycle = k->audiodata->latency_write + sample_count - zzub_latency_buffer_size;

	// clear used temporary and output channels buffer before mixing
	for (int i = 0; i < std::max(k->info->outputs, k->info->inputs); i++) {
		if (plin[i] != 0)
			memset(plin[i], 0, sample_count * sizeof(float));
		if (plout[i] != 0) {
			// NOTE: can eliminate this memset if first connection does a set_samples() instead of add_samples()?
			if (k->audiodata->latency_write + sample_count > zzub_latency_buffer_size ) {
				if (beforecycle > 0)
					memset(plout[i] + k->audiodata->latency_write, 0, beforecycle * sizeof(float));
				if (aftercycle > 0)
					memset(plout[i], 0, aftercycle * sizeof(float));
			} else {
				memset(plout[i] + k->audiodata->latency_write, 0, sample_count * sizeof(float));
			}
		}
	}

	bool has_input = false;

	for (std::vector<metaconnection*>::iterator i = k->connections.begin(); i != k->connections.end(); ++i) {
		metaconnection* conn = *i;
		metaplugin* plugin_self = plugins.top()[conn->plugin_id].get();

		plugin_self->plugin->process_stereo(0, plin, sample_count, 0);

		for (int j = 0; j < max_channels; j++) {
			// process_stereo() nulls silent channels, keep only used channels in plmix and restore the previous plin for the next connection
			if (plin[j] != 0) {
				plmix[j] = plin[j];
				has_input = true;
			}
			plin[j] = plconn[j];
		}
	}

	// process audio:
	int flags;
	if (((k->info->flags & zzub_plugin_flag_has_audio_output) != 0) &&
		((k->info->flags & zzub_plugin_flag_has_audio_input) == 0)) {
		flags = zzub_process_mode_write;
	} else {
		flags = has_input ? zzub_process_mode_read_write : zzub_process_mode_write;
	}

	// handle ringbuffer overflow
	if (k->audiodata->latency_write + sample_count > zzub_latency_buffer_size ) {

		float *ploutfirst[max_channels];
		if (beforecycle > 0) {
			memcpy(ploutfirst, plout, sizeof(plout));
			process_stereo_chunk(k, 0, plmix, ploutfirst, beforecycle, flags);
		}

		k->audiodata->latency_write = 0;

		process_stereo_chunk(k, beforecycle, plmix, plout, aftercycle, flags);
		
		// merge active channels for both chunks
		if (beforecycle > 0)
			for (int i = 0; i < max_channels; i++)
				if (ploutfirst[i] != 0 && plout[i] == 0)
					plout[i] = ploutfirst[i];

		k->audiodata->latency_write += aftercycle;
		
	} else {
		process_stereo_chunk(k, 0, plmix, plout, sample_count, flags);
		k->audiodata->latency_write += sample_count;
	}

	// update statistics
	k->audiodata->last_work_audio_result = false;
	for (int i = 0; i < k->info->outputs; i++) {
		if (plout[i] != 0) {
			k->audiodata->last_work_audio_result = true;
			break;
		}
	}

	k->audiodata->last_work_time = timer.frame() - start_time;
	k->audiodata->last_work_buffersize = sample_count;
	k->audiodata->last_work_frame = work_position;

	// these are used to calculating cpu_load-per-plugin-per-buffer in op_player_get_plugins_load_snapshot::operate()
	k->audiodata->cpu_load_time += k->audiodata->last_work_time;
	k->audiodata->cpu_load_buffersize += sample_count;
}
/*
void mixer::process_midi(metaplugin* k, int sample_count) {
	int result = 0;
	if (k->info->flags & zzub_plugin_flag_has_midi_input) {

		for (std::vector<metaconnection*>::iterator i = k->connections.begin(); i != k->connections.end(); ++i) {
			metaconnection* conn = *i;
			metaplugin* plugin_self = plugins.top()[conn->plugin_id].get();
			metaplugin* plugin_from = plugins.top()[conn->from_plugin_id].get();
			if (plugin_from->info->flags & zzub_plugin_flag_has_midi_output)
				plugin_self->plugin->process_midi_events(0, 0);
		}

		zzub::midi_message* messages;
		if (k->in_midi_messages.size() > 0)
			messages = &k->in_midi_messages.front();
		else
			messages = 0;
		k->plugin->process_midi_events(messages, (int)k->in_midi_messages.size());
	}
}*/

bool mixer::write_output(int channel, int write_offset, float* samples, int numsamples, float amp) {
	if ((samples != 0 && amp > 0.0f) && zzub::buffer_has_signals(samples, numsamples)) {
		if (output_buffer[channel] != 0) {
			dspcopyamp(output_buffer[channel] + write_offset, samples, numsamples, amp);
			output_buffer_count[channel]++;
		}
		return true;
	} else {
		//memset(output_buffer[channel] + write_offset, 0, numsamples * sizeof(float));
		return false;
	}
}


void mixer::process_encoder(metaplugin* k, int sample_count) {

	if (k->info->flags & zzub_plugin_flag_is_encoder) {
		// check for encoder buffer overflow, send off encoder events on overflow etc
		// remember sample_count can extend several etc

		int framepos = 0;
		int encoder_totalsamples = sample_count;
		while (encoder_totalsamples > 0) {
			int encoder_samples;
			if (k->audiodata->encoder_position + encoder_totalsamples > k->audiodata->encoder_size) {
				encoder_samples = k->audiodata->encoder_size - k->audiodata->encoder_position;
			} else
				encoder_samples = encoder_totalsamples;

			for (int i = 0; i< k->info->inputs; i++) {
				memcpy(k->audiodata->encoder_buffer[i] + k->audiodata->encoder_position, k->audiodata->encoder_frame_buffer[i].get() + framepos, encoder_samples * sizeof(float));
			}

			k->audiodata->encoder_position += encoder_samples;
			encoder_totalsamples -= encoder_samples;
			framepos += encoder_samples;

			// send chunks to encoder, get new buffers from threaded pool
			if (k->audiodata->encoder_position == k->audiodata->encoder_size) {
				process_encoder_buffer(k);
			}
		}
	}
}

void mixer::process_encoder_buffer(metaplugin* k) {
	int encoder_buffer_size = k->audiodata->encoder_position;
	float* buffers[max_channels];
	for (int i = 0; i< k->info->inputs; i++) {
		buffers[i] = k->audiodata->encoder_buffer[i];
		k->audiodata->encoder_buffer[i] = (float*)encoderbuffers.alloc(&k->audiodata->encoder_size);
		k->audiodata->encoder_size = k->audiodata->encoder_size / sizeof(float);
	}
	encodermgr.set_chunk(k->plugin, buffers, k->info->inputs, encoder_buffer_size);
	k->audiodata->encoder_position = 0;
}

void mixer::process_parameter_changes(metaplugin* k, int g) {
	const zzub::pluginstate& p = k->state_write;
	for (int j = 0; j < p.get_track_count(g); j++) {
		for (int i = 0; i < p.get_column_count(g, j); i++) {
			const zzub::parameter* param = k->get_parameter_info(g, j, i);
			int v = p.get_value(g, j, i);

			if (is_recording_parameters) {
				int a = k->state_automation.get_value(g, j, i);
				if (a != param->value_none)
					on_parameter_change(k->id, g, j, i, a, true);
				else if (v != param->value_none)
					on_parameter_change(k->id, g, j, i, v, false);
			} else {
				if (v != param->value_none)
					on_parameter_change(k->id, g, j, i, v, false);
			}
		}
	}
}

void mixer::on_process_events(metaplugin* k) {

	k->transfer_parameter_track_row(1, 0, k->state_write, k->plugin->global_values, true);
	char* track_ptr = (char*)k->plugin->track_values;
	int track_size = k->get_parameter_track_row_bytesize(2, 0);
	for (int i = 0; i < k->tracks; i++) {
		assert(track_ptr != 0);
		k->transfer_parameter_track_row(2, i, k->state_write, track_ptr, true);
		track_ptr += track_size;
	}

	// send parameter change notifications
	process_parameter_changes(k, 0);
	process_parameter_changes(k, 1);
	process_parameter_changes(k, 2);

	// process plugin
	k->plugin->process_events();

	// transfer state_write to state_last
	k->transfer_parameter_row(1, k->state_write, k->state_last, false);
	k->transfer_parameter_row(2, k->state_write, k->state_last, false);

	k->state_write.clear_group(0);
	k->state_write.clear_group(1);
	k->state_write.clear_group(2);

	k->state_automation.clear_contents();
	
	// check for latency change
	int latency = k->get_latency();
	if (k->audiodata->last_latency != latency) {
		dirty_latencies = true;
		k->audiodata->last_latency = latency;
	}
	k->audiodata->is_dirty = false;
}

 // returns true if the plugins parameters changed, e.g via event connections or pattern/manual parameter changes
bool mixer::on_process_internal_events(zzub::metaplugin* k) {

	// process internal parameters
	on_process_interpolators(0, k);

	bool mutechanged = false;
	int mutevalue = k->state_write.get_value(0, 0, 0);
	if (mutevalue != zzub_switch_value_none) {
		k->audiodata->is_muted = mutevalue == zzub_switch_value_on;
		mutechanged = true;
	}

	int bypassvalue = k->state_write.get_value(0, 0, 1);
	if (bypassvalue != zzub_switch_value_none) {
		k->audiodata->is_bypassed = bypassvalue == zzub_switch_value_on;
		mutechanged = true;
	}

	int softmutevalue = k->state_write.get_value(0, 0, 2);
	if (softmutevalue != zzub_switch_value_none) {
		k->audiodata->is_softmuted = softmutevalue == zzub_switch_value_on;
		mutechanged = true;
	}

	int softbypassvalue = k->state_write.get_value(0, 0, 3);
	if (softbypassvalue != zzub_switch_value_none) {
		k->audiodata->is_softbypassed = softbypassvalue == zzub_switch_value_on;
		mutechanged = true;
	}

	if (!mutechanged && k->audiodata->is_dirty) {
		if (k->audiodata->is_softmuted)
			k->audiodata->is_softmuted = false;
		if (k->audiodata->is_softbypassed)
			k->audiodata->is_softbypassed = false;
	}

	// process parameter changes on connections
	for (std::vector<metaconnection*>::iterator j = k->connections.begin(); j != k->connections.end(); ++j) {
		metaplugin* cp = plugins.top()[(*j)->plugin_id].get();

		if (cp->audiodata->is_dirty)
			on_process_events(cp);

		if (cp->audiodata->connection_process_events_time != work_position) {
			cp->plugin->process_connection_events();	// step 2: this forwards anything written in group 3 to k
			cp->audiodata->connection_process_events_time = work_position;
		}

		if (k->info->flags & zzub_plugin_flag_has_midi_input) {
			metaplugin* plugin_from = plugins.top()[(*j)->from_plugin_id].get();
			if (plugin_from->info->flags & zzub_plugin_flag_has_midi_output)
				cp->plugin->process_midi_events(0, 0);
		}
	}

	// abort if muted/bypassed after connection processing - f.ex allow changing volume on incoming connections even if the plugin is bypassed
	if (!mutechanged && (k->audiodata->is_muted || k->audiodata->is_bypassed || k->audiodata->is_softmuted || k->audiodata->is_softbypassed)) return false;

	if ((k->info->flags & zzub_plugin_flag_has_midi_input) != 0 || (k->info->flags & zzub_plugin_flag_has_midi_output) != 0) {
		zzub::midi_message* messages;
		if (k->in_midi_messages.size() > 0)
			messages = &k->in_midi_messages.front();
		else
			messages = 0;
		k->plugin->process_midi_events(messages, (int)k->in_midi_messages.size());
	}

	if ((k->info->flags & zzub_plugin_flag_has_event_output) != 0) {
		// transfer anything written in group 3 so far into the plugin. process_events could write more
		k->transfer_parameter_track_row(3, 0, k->state_write, k->plugin->controller_values, true);
	}

	// process interpolators -> write state_final
	on_process_interpolators(1, k);
	on_process_interpolators(2, k);

	bool changed = k->audiodata->is_dirty;

	if (k->audiodata->is_dirty)
		on_process_events(k);

	// TODO: only need to call process_controller_events() if the plugin was interrupted by either 
	// interval (next_interval_position == 0) or patternplayer (next_sequence_position == 0, or just is_dirty)
	if ((k->info->flags & zzub_plugin_flag_has_event_output) != 0) {
		k->plugin->process_controller_events(); // step 1: the plugin could write to group 3
		k->transfer_parameter_track_row(3, 0, k->plugin->controller_values, k->state_write, true);
	}
	return changed;
}

bool mixer::on_process_interpolators(int group, zzub::metaplugin* k) {
	bool dirty = false;
	for (int i = 0; i < k->state_write.get_track_count(group); ++i) {
		for (int j = 0; j < k->state_write.get_column_count(group, i); ++j) {
			int value = k->state_write.interpolators[group][i][j].get_value();
			int novalue = k->state_write.groupinfo[group][i][j]->value_none;
			if (value != novalue) {
				k->set_parameter(group, i, j, value, false);
			}
		}
	}
	return dirty;
}

void mixer::process_graph_events() {

	for (size_t i = 0; i < workorder.size(); i++) {
		metaplugin* k = workorder[i];
		on_process_internal_events(k);
	}
}

void mixer::process_sequence_events(bool& reprocess) {

	for (size_t i = 0; i < workorder.size(); i++) {
		metaplugin* k = workorder[i];

		if (k->audiodata->in_sequence_chain) {
		//if ((k->info->flags & zzub_plugin_flag_is_sequence) != 0 || 
		//	(k->info->flags & zzub_plugin_flag_stream) != 0 || 
		//	(k->info->flags & zzub_plugin_flag_has_event_output) != 0) 
		//{
			if (on_process_internal_events(k))
				reprocess = true;

		}
	}

}

// NOTE: the interval functions enumerate _all_ plugins, and not in the workorder. because connection plugins can be intervals.

void mixer::reset_interval_plugins() {
	for (threadqueuearray<metaplugin, max_threadqueue>::items_type::const_iterator i = plugins.top().begin(); i != plugins.top().end(); ++i) {
		metaplugin* k = i->get();
		if (!k || (k->info->flags & zzub_plugin_flag_is_interval) == 0) continue;
		k->audiodata->next_interval_position = 0;
	}
}

void mixer::process_interval_plugins() {
	for (threadqueuearray<metaplugin, max_threadqueue>::items_type::const_iterator i = plugins.top().begin(); i != plugins.top().end(); ++i) {
		metaplugin* k = i->get();
		if (!k || (k->info->flags & zzub_plugin_flag_is_interval) == 0) continue;

		if (k->audiodata->next_interval_position == 0) {
			k->plugin->process_interval();
		}
	}
}

void mixer::update_interval_plugins() {
	for (threadqueuearray<metaplugin, max_threadqueue>::items_type::const_iterator i = plugins.top().begin(); i != plugins.top().end(); ++i) {
		metaplugin* k = i->get();
		if (!k || (k->info->flags & zzub_plugin_flag_is_interval) == 0) continue;

		if (k->audiodata->next_interval_position == 0) {
			k->audiodata->next_interval_position = k->plugin->get_interval_size();
			assert(k->audiodata->next_interval_position > 0);
		}
	}
}

void mixer::apply_interval_size(int& min_sequence_chunk) {
	for (threadqueuearray<metaplugin, max_threadqueue>::items_type::const_iterator i = plugins.top().begin(); i != plugins.top().end(); ++i) {
		metaplugin* k = i->get();
		if (!k || (k->info->flags & zzub_plugin_flag_is_interval) == 0) continue;

		if (k->audiodata->next_interval_position > 0 && k->audiodata->next_interval_position < min_sequence_chunk) {
			min_sequence_chunk = k->audiodata->next_interval_position;
		}
	}
}

void mixer::apply_interval_chunk_size(int chunksize) {
	for (threadqueuearray<metaplugin, max_threadqueue>::items_type::const_iterator i = plugins.top().begin(); i != plugins.top().end(); ++i) {
		metaplugin* k = i->get();
		if (!k || (k->info->flags & zzub_plugin_flag_is_interval) == 0) continue;

		k->audiodata->next_interval_position -= chunksize;
		assert(k->audiodata->next_interval_position >= 0);
	}
}

void mixer::process_seek(int row) {
	patternplayer_clear_children(patternplayers.top(), rootplayer, true);

	zzub::pattern* playpattern = rootplayer->get_pattern();
	if (!playpattern) return ;

	if (playpattern->loopenabled)
		rootplayer->seek_row = std::min(row, std::max(playpattern->endloop, 0)); else
		rootplayer->seek_row = row;

	bool startedplay;
	if (!rootplayer->playing) {
		rootplayer->play();
		startedplay = true;
	} else
		startedplay = false;

	while (rootplayer->pattern_row < rootplayer->seek_row) {
		int chunksize = 30 * rootplayer->_master_info.samples_per_second;
		if (!process_sequencer(chunksize, true))
			break; // sequencer recursion error
		//cout << "seeking to " << seek_row << " @ " << pattern_row << " w/" << chunksize << ", state=" << playing << endl;
		song_position += chunksize; // not calling process_graph() during seek, so we need to maintain the sample based song position ourself
	}

	if (startedplay && rootplayer->pattern_row == rootplayer->seek_row) {
		patternplayer_stop(patternplayers.top(), rootplayer);
	}

	rootplayer->seek_row = -1;
	patternplayer_seek_end(patternplayers.top(), rootplayer);
}

bool mixer::process_sequencer(int& min_sequence_chunk, bool offline) {
	bool reprocess;
	int counter = 0;
	const int max_nested_patterns = 64;

	do {
		reprocess = false;
		patternplayer_process_sequence(patternplayers.top(), rootplayer, offline);
		process_interval_plugins(); // signal intervals in interval plugins

		// call process_events() on sequence plugins with changed parameters.
		// reprocess is set to true if any parameters were processed, e.g on automated
		// tempo changes, new pattern triggers, or automated interval changes.
		reprocess = false;
		process_sequence_events(reprocess);
		counter++;
	} while (reprocess && counter < max_nested_patterns);	// stop after max 64 nested levels or infinite pattern recursion

	if (counter == max_nested_patterns) {
		// error condition - just mute the player and return
		on_set_state(player_state_muted);

		// send user max-recursion error notification so guis can popup a message
		user_event_data data = { user_event_type_infinite_pattern_recursion };
		invoke_user_event(data);

		return false;
	}

	// do not process graph events during seeks, the interpolators dont like that:
	if (!offline)
		process_graph_events(); // call process_events() on remaining gens/fx etc

	update_interval_plugins(); // update intervals from interval plugins

	patternplayer_apply_sequence_size(patternplayers.top(), rootplayer, min_sequence_chunk); // determine smallest chunk size based on pattern players
	apply_interval_size(min_sequence_chunk); // determine smallest chunk size based on interval plugins

	apply_interval_chunk_size(min_sequence_chunk); // apply the smalles chunk size to interval plugins

	patternplayer_advance(patternplayers.top(), rootplayer, min_sequence_chunk); // advance row numbers, tick counters etc	}

	// done with midi/note connections, clear buffers here to allow adding notes/midi for next frame in process_stereo
	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		metaplugin* k = *i;
		//if (k->info->flags & zzub_plugin_flag_has_note_output)
		k->note_messages.clear();
		k->out_midi_messages.clear();
		k->in_midi_messages.clear();

	}
	return true;
}

bool mixer::in_user_thread() {
	return user_thread_id == thread_id::get();
}

bool mixer::in_encoder_thread() {
	return encodermgr.encoder_thread_id == thread_id::get();
}

void mixer::note_out(int from_plugin_id, const zzub::note_message& message) {
	if (in_user_thread()) {
		if (from_plugin_id > 0 && from_plugin_id < (int)plugins.next().size()) {
			zzub::metaplugin* m = plugins.next()[from_plugin_id].get();
			if (m->initialized) {
				audio_event_data ev;
				ev.type = audio_event_type_note_event;
				ev.note_event.plugin_id = from_plugin_id;
				ev.note_event.message = message;
				invoke_audio_event(ev);
			} else {
				// send notes directly to the plugin before and during plugin initialization. no need for next_for_write() since it was newly created
				m->note_messages.push_back(message);
			}
		}
	} else {
		on_note_out(from_plugin_id, message);
	}
}

void mixer::on_note_out(int from_plugin_id, const zzub::note_message& message) {
	zzub::metaplugin* m = &plugins.top_item(from_plugin_id);
	m->note_messages.push_back(message);
}

void mixer::midi_out(int from_plugin_id, const zzub::midi_message& message) {
	if (in_user_thread()) {
		if (from_plugin_id > 0 && from_plugin_id < (int)plugins.next().size()) {
			zzub::metaplugin* m = plugins.next()[from_plugin_id].get();
			if (m->initialized) {
				audio_event_data ev;
				ev.type = audio_event_type_midi_event;
				ev.midi_event.plugin_id = from_plugin_id;
				ev.midi_event.message = message;
				invoke_audio_event(ev);
			} else {
				// send midi messages directly to the plugin before and during plugin initialization. no need for next_for_write() since it was newly created
				m->out_midi_messages.push_back(message);
			}
		}
	} else {
		on_midi_out(from_plugin_id, message);
	}
}

void mixer::on_midi_out(int from_plugin_id, const zzub::midi_message& message) {
	zzub::metaplugin* m = &plugins.top_item(from_plugin_id);
	m->out_midi_messages.push_back(message);
}

bool mixer::is_input_channel_connected(int to_plugin_id, int index) {
	zzub::metaplugin* k;
	if (in_user_thread()) {
		k = plugins.next()[to_plugin_id].get();
	} else {
		k = &plugins.top_item(to_plugin_id);
	}

	// NOTE: this loop could be moved to invalidate_state() and instead just index into an array of channel states here
	for (std::vector<zzub::metaconnection*>::iterator i = k->connections.begin(); i != k->connections.end(); ++i) {
		zzub::metaconnection* conn = *i;
		if (conn->type != zzub_connection_type_audio) continue;

		// return true if the index is in the audio connections mapping range
		if (index >= conn->first_input && index < conn->first_input + conn->input_count) 
			return true;
	}
	return false;
}

/*void mixer::process_keyjazz_noteoff_events() {
	// check for delayed note offs
	// set_keyjazz_note may modify keyjazz so we use a copy
	std::vector<keyjazz_note> keycopy = playnotes.notes;//keyjazz;
	for (size_t i = 0; i < keycopy.size(); i++) {
		if (keycopy[i].delay_off == true) {
			//cerr << "playing delayed off" << endl;
			int plugin_id = keycopy[i].plugin_id;
			metaplugin& m = *plugins[plugin_id];
			assert(&m);
			int note_group = -1, note_track = -1, note_column = -1;
			int velocity_column = -1;
			playnotes.set_keyjazz_note(m, zzub_note_value_off, keycopy[i].note, 0, note_group, note_track, note_column, velocity_column);
			if (note_group != -1) {
				m.set_parameter_direct(note_group, note_track, note_column, zzub_note_value_off, true);
			}
		}
	}
}*/

int mixer::generate_audio(float** pin, float** pout, int sample_count, zzub::midi_message* pmidiin, int inmidi_count, zzub::midi_message* pmidiout, int* outmidi_count, int outmidi_maxcount) {
	audiomutex.lock();

	// handle serialized editing
	process_audio_event_queue();

	// is state is muted, we abort so the user thread can modify song data freely
	if (state == player_state_muted) {
		size_t mute_buffer_size = sample_count > zzub_buffer_size ? zzub_buffer_size : sample_count;
		for (int i = 0; i < max_channels; ++i)
			pout[i] = 0;
		*outmidi_count = 0;
		audiomutex.unlock();
		return (int)mute_buffer_size;
	}

	for (int i = 0; i < max_channels; ++i) {
		output_buffer_count[i] = 0;
		output_buffer[i] = pout[i];
		input_buffer[i] = pin[i];
	}

	// NOTE/TODO: deprecate midievent on the plugins, and instead in plugin that needs the raw midi input has access to them for a frame
	input_midi_messages = pmidiin;
	input_midi_message_count = inmidi_count;
	process_midi_bindings(pmidiin, inmidi_count);

	output_midi_messages = pmidiout;
	output_midi_message_count = 0;
	output_midi_message_maxcount = outmidi_maxcount;

	int work_chunk_size = std::min((int)zzub_buffer_size, sample_count);

	process_sequencer(work_chunk_size, false);
	assert(work_chunk_size >= 0);

	if (dirty_latencies) {
		update_latency_compensation();
		dirty_latencies = false;
	}

	process_graph(work_chunk_size);

	// set pout indices to null where output_buffer_count is zero / silent
	for (int i = 0; i < max_channels; ++i)
		if (output_buffer_count[i] == 0)
			pout[i] = 0;

	*outmidi_count = output_midi_message_count;
	audiomutex.unlock();
	return work_chunk_size;
}

// returns the number of messages written, less than midi_count if the midioutputbuffer was exhausted
int mixer::write_midi(midi_message* messages, int midi_count) {
	for (int i = 0; i < midi_count; i++) {
		if (output_midi_message_count >= output_midi_message_maxcount) return i;
		output_midi_messages[output_midi_message_count] = messages[i];
		output_midi_message_count++;
	}
	return midi_count;
}

void mixer::process_midi_bindings(zzub::midi_message* pmidi, int midi_count) {
	//midi_event(unsigned short status, unsigned char data1, unsigned char data2) {
	
	for (int i = 0; i < midi_count; i++) {
		unsigned long message = pmidi[i].message;
		unsigned short status = message & 0xff;
		unsigned char data1 = (message >> 8) & 0xff;
		unsigned char data2 = (message >> 16) & 0xff;
		unsigned char channel = status&0xF;
		unsigned char command = (status & 0xf0) >> 4;

		// look up mapping(s) and send value to plugin
		if (command == 0xb) {
			float midi_value = data2;
			const float midi_max_value = 127.0f;

			for (size_t i = 0; i < midimappings.top().size(); i++) {
				midimapping* mm = midimappings.top()[i].get();
				if (mm == 0) continue;
				if (mm->channel == channel && mm->controller == data1) {
					metaplugin* k = plugins.top()[mm->plugin_id].get();
					assert(k != 0);
					const parameter* param = k->get_parameter_info(mm->group, mm->track, mm->column);
					float minValue = (float)param->value_min;
					float maxValue = (float)param->value_max;
					float delta = (maxValue - minValue) / midi_max_value;

					k->set_parameter(mm->group, mm->track, mm->column, (int)ceil(minValue + midi_value * delta), true);
				}
			}
		}

		// also send note events to plugins directly
		if ((command == 8) || (command == 9)) {
			int velocity = (int)data2;
			if (command == 8)
				velocity = 0;
			for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
				metaplugin* k = *i;
				if (!k) continue;

				//k->plugin->midi_note(channel, (int)data1, velocity);

				if (k->midi_input_channel == channel || k->midi_input_channel == 16 || (k->midi_input_channel == 17 && k->id == midi_plugin)) {

					// TODO: if the plugin has midi focus, but no note columns, but has midi input, we can just forward

					// play a recordable note/off, w/optional velocity, delay and cut
					int note, prevNote;
					if (command == 9 && velocity != 0) {
						note = midi_to_buzz_note(data1);
						prevNote = -1;
					} else {
						note = zzub_note_value_off;
						prevNote = midi_to_buzz_note(data1);
					}

					// find note_group, track, column and velocity_group, track and column based on keyjazz-struct
					int note_group = -1, note_track = -1, note_column = -1;
					int velocity_column = -1;
					playnotes.set_keyjazz_note(*k, note, prevNote, velocity, note_group, note_track, note_column, velocity_column);

					if (note_group != -1) {
						k->set_parameter(note_group, note_track, note_column, note, true);
						if (velocity_column != -1 && velocity != 0) {
							const parameter* param = k->get_parameter_info(note_group, note_track, velocity_column);
							velocity = (velocity * param->value_max) / 127; // Set appropriate velocity
							k->set_parameter(note_group, note_track, velocity_column, velocity, true);
						}
					}
				}
			}
		}

		for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
			(*i)->plugin->midi_event(status, data1, data2);
		}

		// plus all midi messages should be sent as master-events, so ui's can pick these up
		user_event_data data = { user_event_type_midi_control };
		data.midi_control.status = (unsigned char)status;
		data.midi_control.data1 = data1;
		data.midi_control.data2 = data2;

		invoke_user_event(data);
	}
}

void mixer::set_plugin_latency(int plugin_id, int samplecount) {
	commit_event_data data;
	data.type = commit_event_type_set_plugin_latency;
	data.set_plugin_latency.plugin_id = plugin_id;
	data.set_plugin_latency.latency = samplecount;
	invoke_commit_event(data);
}

void mixer::on_set_plugin_latency(int plugin_id, int samplecount) {
	metaplugin* k = plugins.top()[plugin_id].get();
	if (samplecount != k->audiodata->last_latency) {
		k->audiodata->latency = samplecount;
		k->audiodata->last_latency = samplecount;
		dirty_latencies = true;
	}
}

void mixer::update_latency_compensation() {

	int max_latency = 0;
	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		metaplugin* k = *i;
		if (k->out_connections.size() == 0) {
			int latency = get_max_latency(k);
			max_latency = std::max(max_latency, latency);
		}
	}

	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		metaplugin* k = *i;
		if (k->out_connections.size() == 0) {
			set_latencies(k, max_latency);
		}
	}

	// position new latencies relative to current latency buffer position
	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		metaplugin* k = *i;
		for (std::vector<metaconnection*>::iterator j = k->connections.begin(); j != k->connections.end(); ++j) {
			metaconnection* conn = *j;
			metaplugin* fromplugin = plugins.top()[conn->from_plugin_id].get();
			conn->audiodata->latency_read = (conn->audiodata->latency_read + latency_buffer_position) % zzub_latency_buffer_size;
		}
		k->audiodata->latency_write = (k->audiodata->latency_write + latency_buffer_position) % zzub_latency_buffer_size;
	}

/*
	// print info:
	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		metaplugin* k = *i;
		cout << k->name << ": latency_write: " << k->audiodata->latency_write << " from " << endl;
		for (std::vector<metaconnection*>::iterator j = k->connections.begin(); j != k->connections.end(); ++j) {
			metaplugin* t = plugins.top()[(*j)->from_plugin_id].get();
			cout << "    " << t->name << ": latency_write: " << t->audiodata->latency_write << ", latency_read=" << (*j)->audiodata->latency_read << endl;
		}
	}*/
}

int mixer::get_max_latency(metaplugin* k) {
	int max_latency = 0;
	k->audiodata->latency_write = 0; // reset
	for (std::vector<metaconnection*>::iterator i = k->connections.begin(); i != k->connections.end(); ++i) {
		metaconnection* conn = *i;
		conn->audiodata->latency_read = 0; // reset
		if (conn->is_back_edge) continue;
		metaplugin* fromplugin = plugins.top()[conn->from_plugin_id].get();
		int latency = get_max_latency(fromplugin);
		max_latency = std::max(max_latency, latency);
	}
	return max_latency + k->get_latency();
}

int mixer::set_latencies(metaplugin* k, int max_latency) {
	int maxinputlatency = 0;

	for (std::vector<metaconnection*>::iterator i = k->connections.begin(); i != k->connections.end(); ++i) {
		metaconnection* conn = *i;
		if (conn->is_back_edge) continue;
		metaplugin* fromplugin = plugins.top()[conn->from_plugin_id].get();
		int latency = set_latencies(fromplugin, max_latency);
		maxinputlatency = std::max(maxinputlatency, latency);
	}

	for (std::vector<metaconnection*>::iterator i = k->connections.begin(); i != k->connections.end(); ++i) {
		metaconnection* conn = *i;
		if (conn->is_back_edge) continue;
		metaplugin* fromplugin = plugins.top()[conn->from_plugin_id].get();
		conn->audiodata->latency_read = std::max(conn->audiodata->latency_read, max_latency - (maxinputlatency - fromplugin->get_latency()));
	}

	k->audiodata->latency_write = std::max(k->audiodata->latency_write, max_latency - maxinputlatency);

	return maxinputlatency + k->get_latency();

}

void mixer::set_plugin_timesource(patternplayer* pp, int pluginid, int group, int track) {
	metaplugin* k = &plugins.next_for_write(pluginid);
	dirty_graph = true;

	pp->_plugin = k->plugin;
	pp->plugin_group = group;
	pp->plugin_track = track;

	k->set_patternplayer(group, track, pp);
}

patternplayer* mixer::create_patternplayer(int samplerate, int bpm, int tpb, float swing, int swingticks) {
	patternplayer* pp = new patternplayer();
	pp->_mixer = this;
	pp->_master_info.samples_per_second = samplerate;
	pp->set_speed(bpm, tpb, swing, swingticks);
	patternplayers.next().push_back(boost::shared_ptr<patternplayer>(pp));
	return pp;
}

bool ppcompare(boost::shared_ptr<patternplayer>& a, boost::shared_ptr<patternplayer>& b) {
	return a.get() == b.get();
}

template <typename T> 
struct shared_equals_raw : public std::unary_function<boost::shared_ptr<T>, bool> {
	shared_equals_raw(T* raw) :_raw(raw) {}
	bool operator()(const boost::shared_ptr<T>& ptr) const {
		return (ptr.get()==_raw);
	}
private:
	T* const _raw;
};

void mixer::destroy_patternplayer(patternplayer* pp) {

	if (pp != rootplayer) {
		pp->parent = 0;
		
		patternplayers.next().erase(
			std::remove_if(patternplayers.next().begin(), patternplayers.next().end(), shared_equals_raw<patternplayer>(pp)),
			patternplayers.next().end());
	} else {
		rootplayer->_plugin = 0;
	}
}

void mixer::on_tempo_changed(patternplayer* player, int timepluginid, int timegroup, int timetrack) {
	for (std::vector<metaplugin*>::iterator i = workorder.begin(); i != workorder.end(); ++i) {
		metaplugin* k = *i;
		if (!k) continue;

		int plugtimepluginid = k->timesource_plugin_id;
		int plugtimegroup = k->timesource_group;
		int plugtimetrack = k->timesource_track;

		// plugin has no timesource -> use the root player
		/*if (plugtimepluginid == -1 && rootplayer->_plugin != 0) {
			plugtimepluginid = rootplayer->_plugin->_id;
			plugtimegroup = rootplayer->plugin_group;
			plugtimetrack = rootplayer->plugin_track;
		}*/
		if (plugtimepluginid == -1 && player == rootplayer) {
			k->plugin->update_timesource();
		} else if (k->id != timepluginid && plugtimepluginid == timepluginid && plugtimegroup == timegroup && plugtimetrack == timetrack) {
			k->plugin->update_timesource();
		}
	}

	if (player == rootplayer) {
		if (!in_user_thread()) {
			// rootplayer tempo change -> update sliders
			// TODO: the plugin should do this itself in case the sequence plugin's parameters change
			if (rootplayer->_plugin != 0 && rootplayer->_plugin->_id < (int)plugins.top().size()) {
				plugins.top()[rootplayer->_plugin->_id].get()->set_parameter(1, 0, 0, player->_master_info.beats_per_minute, false);
				plugins.top()[rootplayer->_plugin->_id].get()->set_parameter(1, 0, 1, player->_master_info.ticks_per_beat, false);
				plugins.top()[rootplayer->_plugin->_id].get()->set_parameter(1, 0, 2, int(player->_master_info.swing_amount * 100.0f), false);
				plugins.top()[rootplayer->_plugin->_id].get()->set_parameter(1, 0, 3, player->_master_info.swing_ticks, false);
			}
		}

		// notify user thread about tempo change
		user_event_data data = { user_event_type_tempo_change };
		data.tempo_change.bpm = player->_master_info.beats_per_minute;
		data.tempo_change.tpb = player->_master_info.ticks_per_beat;
		data.tempo_change.swing = player->_master_info.swing_amount;
		data.tempo_change.swing_ticks = player->_master_info.swing_ticks;
		invoke_user_event(data);
	}

}

void mixer::end_of_pattern(patternplayer* player, bool offline) {
	// clear all interpolators
	{
		pattern* p = player->get_pattern();
		patternformat* format = patternformats.top()[p->formatid].get();
		for (std::vector<patternformatcolumn>::iterator i = format->columns.begin(); i != format->columns.end(); ++i) {
			zzub::metaplugin* mpl = plugins.top()[i->pluginid].get();
			interpolator* interpol = &mpl->state_write.interpolators[i->group][i->track][i->column];
			if (interpol->owner == player) 
				interpol->owner = 0;
		}
		
	}


	if (player->parent) {
		patternplayer_stop(patternplayers.top(), player);
	} else {
		// play next order, or stop

		if (player->is_preview_pattern) {
			patternplayer_stop(patternplayers.top(), player);
		} else {
			int next_index = orderlist_position;

			bool has_queue = false;

			if (queue_index != -1) {
				next_index = queue_index;
				queue_index = -1;
				has_queue = true;
				user_event_data data = { user_event_type_order_queue_change };
				invoke_user_event(data);
			} else {
				next_index++;
			}

			// skip nulls in the patternlist
			while (next_index < (int)songinfo.top_item(0).orderlist.size() && songinfo.top_item(0).orderlist[next_index] == 0) {
				next_index++;
			}

			if (has_queue) {
				//
			} else
			if (songinfo.top_item(0).loop_enabled) {
				if ((songinfo.top_item(0).loop_end < songinfo.top_item(0).loop_begin) && (next_index >= (int)songinfo.top_item(0).orderlist.size())) {
					// allows using inverted loop range to create a "skip section"
					next_index = 0;
				} else
				if (orderlist_position == songinfo.top_item(0).loop_end) {
					// loop when hitting loop end
					next_index = songinfo.top_item(0).loop_begin;
				} else
				if (next_index >= (int)songinfo.top_item(0).orderlist.size()) {
					// loop back to beginning of song if user explicitly moved past the loop end
					next_index = 0;
				}
			} else {
				if (next_index >= (int)songinfo.top_item(0).orderlist.size()) {
					on_set_state(player_state_stopped);
					next_index = songinfo.top_item(0).loop_begin;
				}
			}

			set_orderlist_index(next_index);
			player->reset_interpolators();

			player->update(offline); // does advance_pattern
		}
	}
}

void mixer::end_of_loop(patternplayer* player, bool offline, int beginloop) {
	assert(false);
/*	if (player == rootplayer)
		encodermgr.set_state(zzub_encoder_state_seeking);

	player->seek(beginloop);
	player->update(offline);

	if (player->parent)
		player->update_fracs(player->parent->subtick_frac_remainder);

	if (player == rootplayer)
		encodermgr.set_state(player->playing ? zzub_encoder_state_playing : zzub_encoder_state_stopped);*/
}

void mixer::set_orderlist_index(int index) {
	orderlist_position = index;

	if (index < (int)songinfo.top_item(0).orderlist.size())
		rootplayer->set_pattern(songinfo.top_item(0).orderlist[index], 0, false);
	else
		rootplayer->set_pattern(-1, 0, false);
	rootplayer->reset_position();

	// send user end-of-pattern notification
	user_event_data data = { user_event_type_order_change };
	data.order_change.orderindex = index;
	invoke_user_event(data);
}

void mixer::set_tpb(int tpb) {
	commit_event_data ev;
	ev.type = commit_event_set_tpb;
	ev.set_tpb.tpb = tpb;
	invoke_commit_event(ev);
}

void mixer::set_bpm(int bpm) {
	commit_event_data ev;
	ev.type = commit_event_set_bpm;
	ev.set_bpm.bpm = bpm;
	invoke_commit_event(ev);
}

void mixer::set_swing(float swing) {
	commit_event_data ev;
	ev.type = commit_event_set_swing;
	ev.set_swing.swing_amount = swing;
	invoke_commit_event(ev);
}

void mixer::set_swing_ticks(int swing_ticks) {
	commit_event_data ev;
	ev.type = commit_event_set_swing_ticks;
	ev.set_swing_ticks.swing_ticks = swing_ticks;
	invoke_commit_event(ev);
}

void mixer::set_queue_index(int index) {
	audio_event_data ev;
	ev.type = audio_event_type_queue_index;
	ev.queue_index.index = index;
	invoke_audio_event(ev);
}

void mixer::set_order_position(int index) {
	audio_event_data ev;
	ev.type = audio_event_type_order_position;
	ev.order_position.index = index;
	invoke_audio_event(ev);
}

void mixer::sync_orderlist_timeshift(int index, int timeshift) {
	commit_event_data ev;
	ev.type = commit_event_orderlist_timeshift;
	ev.orderlist_timeshift.index = index;
	ev.orderlist_timeshift.timeshift = timeshift;
	invoke_commit_event(ev);
}

void mixer::on_sync_orderlist_timeshift(int index, int timeshift) {
	if (timeshift == 0) return ;

	if (orderlist_position >= index) {
		if (orderlist_position + timeshift < index)
			orderlist_position = index; 
		else
			orderlist_position += timeshift;

		user_event_data data = { user_event_type_order_change };
		data.order_change.orderindex = orderlist_position;
		invoke_user_event(data);
	}

	if (queue_index > 0 && queue_index >= index) {
		if (queue_index + timeshift < index)
			queue_index = index;
		else
			queue_index += timeshift;

		user_event_data data = { user_event_type_order_queue_change };
		invoke_user_event(data);
	}

	assert(orderlist_position >= 0);
	assert(queue_index >= -1);
}

}
