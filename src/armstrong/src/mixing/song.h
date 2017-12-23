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

#pragma once
#define BOOST_ATOMIC_NO_LIB
#include <boost/shared_ptr.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/shared_array.hpp>
#include <boost/atomic.hpp>
#include <zzub/plugin.h>

const int zzub_latency_buffer_size = 65536 + zzub_buffer_size;

namespace zzub {

struct connection;
struct info;
struct parameter;
struct plugin;
struct mixer;
struct metaplugin;
struct patternplayer;

// internal player states
enum player_state {
	player_state_playing,
	player_state_stopped,
	player_state_muted,
	player_state_released,
};

struct song {
	int loop_begin;
	int loop_end;
	bool loop_enabled;
	std::vector<int> orderlist;
};

struct interpolator {

	struct interpolatorevent {
		int time, value;
	};

	enum {
		max_lookahead_events = 2
	};
	enum {
		absolute,
		inertial,
		linear
	};

	const zzub::parameter* param;
	int mode;
	int time;
	int tick_position;
	int needs_before;
	int needs_after;
	int value_position;
	patternplayer* owner;
	interpolatorevent events[max_lookahead_events];

	// inertial
	int numsteps;

	interpolator();
	interpolator(const zzub::parameter* _param, int mode);
	void reset(int mode);
	int get_value();
	int get_value_absolute();
	int get_value_inertial();
	int get_value_linear();
	void set_value(int index, int time, int value);
	void shift_set_value(patternplayer* _owner, int time, int value);
};


struct pluginstate {
	enum {
		max_group_count = 5
	};

	//typedef std::vector<pattern_event> column;
	typedef std::vector<int> track;
	typedef std::vector<track> group;
	typedef std::vector<const parameter*> track_info;
	typedef std::vector<track_info> group_info;

	typedef std::vector<interpolator> track_interpolate;
	typedef std::vector<track_interpolate> group_interpolate;

	int id;
	group groups[max_group_count];
	group_info groupinfo[max_group_count];
	group_interpolate interpolators[max_group_count];

	pluginstate();
	int add_track(int group, std::vector<const zzub::parameter*> parameters, bool usedefault);
	void remove_track(int group, int track);
	int get_track_count(int group) const;
	int get_column_count(int group, int track) const;
	int get_value(int group, int track, int column) const;
	void set_value(int group, int track, int column, int value);
	void clear_contents();
	void clear_group(int group);
	void swap(pluginstate& p);

};

// --- pattern events ---

struct by_id {};
struct by_time {};

typedef boost::intrusive::unordered_set_base_hook<
	boost::intrusive::tag<by_id>
> by_id_hook;

typedef boost::intrusive::set_base_hook<
	boost::intrusive::tag<by_time>
> by_time_hook;

struct pattern_event : by_id_hook, by_time_hook
{
	pattern_event() {}

	pattern_event(int id, int pluginid, int group, int track, int column, int time, int value) :	
		id(id), pluginid(pluginid), group(group), track(track), column(column), time(time), value(value)
	{}

	int id;
	int pluginid, group, track, column;
	int time;
	int value;
};

struct hash_by_id {
	size_t operator()(pattern_event const& ev) const { return (size_t)ev.id; }
	size_t operator()(int id) const { return (size_t)id; }
};

struct equal_by_id {
	bool operator()(pattern_event const& lhs, pattern_event const& rhs) const { return lhs.id == rhs.id; }
	bool operator()(pattern_event const& lhs, int id) const { return lhs.id == id; }
	bool operator()(int id, pattern_event const& rhs) const { return id == rhs.id; }
};

struct less_by_time {
	bool operator()(pattern_event const& lhs, pattern_event const& rhs) const { return lhs.time < rhs.time; }
	bool operator()(pattern_event const& lhs, int time) const { return lhs.time < time; }
	bool operator()(int time, pattern_event const& rhs) const { return time < rhs.time; }
};

struct delete_disposer {
	void operator()(pattern_event* ev) const {
		delete ev;
	}
};

typedef boost::intrusive::make_unordered_set<
	pattern_event,
	boost::intrusive::base_hook<by_id_hook>,
	boost::intrusive::hash<hash_by_id>,
	boost::intrusive::equal<equal_by_id>
>::type pattern_events_by_id;

typedef boost::intrusive::make_multiset<
	pattern_event,
	boost::intrusive::base_hook<by_time_hook>,
	boost::intrusive::compare<less_by_time>
>::type pattern_events_by_time;

struct pattern_events
{
	pattern_events_by_id::bucket_type buckets[100];
	pattern_events_by_id by_id;
	pattern_events_by_time by_time;

	pattern_events() :
		by_id(pattern_events_by_id::bucket_traits(buckets, 100))
	{}

	~pattern_events() {
		by_time.clear();
		by_id.clear_and_dispose(delete_disposer());
	}
};

// ----------------------

struct pattern {
	int rows;
	int resolution;
	std::string name;
	int formatid;
	int beginloop;
	int endloop;
	bool loopenabled;
};

struct patternformatcolumn {
	int id;
	int formatid, pluginid, group, track, column;
};

struct patternformat {
	int id;
	std::vector<patternformatcolumn> columns;
};

struct patternformattrack {
	int id;
	int formatid, pluginid, group, track;
	int is_muted;
};

struct mutetrackkeytype {
	int format_id, plugin_id, group, track;
};

inline bool operator < (const mutetrackkeytype& a, const mutetrackkeytype& b) {
	if (a.format_id != b.format_id)
		return a.format_id < b.format_id;
	if (a.plugin_id != b.plugin_id)
		return a.plugin_id < b.plugin_id;
	if (a.group != b.group)
		return a.group < b.group;
	return a.track < b.track;
}

struct event_connection_binding {
	int source_param_index;
	int target_group_index;
	int target_track_index;
	int target_param_index;
};

// stuff that is owned by the audio thread across the lifetime of a connection
struct connectionaudiodata {
	int latency_read;
};

struct metaconnection {
	int id;
	int plugin_id;
	int from_plugin_id, to_plugin_id;
	zzub_connection_type type;
	// midi connections:
	int midi_device;
	std::string midi_device_name;
	// event connections:
	std::vector<event_connection_binding> bindings;
	// audio connections:
	int first_output, first_input, output_count, input_count, flags;
	bool is_back_edge;
	boost::shared_ptr<connectionaudiodata> audiodata;
};

// stuff that is owned by the audio thread across the lifetime of a plugin
struct pluginaudiodata {
	std::vector<float*> encoder_buffer;
	std::vector<boost::shared_array<float> > encoder_frame_buffer;
	int encoder_position;
	int encoder_size;
	boost::atomic<int> dependencies; // multi threading dep counter
	int connection_process_events_time;
	bool is_muted, is_bypassed, is_softmuted, is_softbypassed;
	int next_interval_position;
	int latency; // user latency, or -1 = ask plugin for latency
	int latency_write;
	int last_latency;
	int output_buffer_write; // used by master output plugins to determine ringbuffer chunk positions
	std::vector<float> last_work_max;
	int last_work_buffersize, last_work_frame; // used with feedback buffer
	bool last_work_audio_result;
	bool last_work_midi_result;
	double last_work_time;
	double cpu_load_time;
	int cpu_load_buffersize;
	double cpu_load;
	int writemode_errors;
	bool is_dirty; // true if plugin has pending parameter changes
	bool in_sequence_chain; // true for sequence/stream/event plugins and plugins above them in the chain

};

struct note_message {
	int track;
	int note;
	int wave;
	int amp;
};

struct metaplugin {
	int id;
	std::string name;
	zzub::plugin* plugin;
	zzub::info* info;
	bool initialized;
	pluginstate state_write;
	pluginstate state_last;
	pluginstate state_automation;
	int tracks;
	int midi_input_channel;
	std::vector<boost::shared_array<float> > work_buffer;
	std::vector<boost::shared_array<float> > feedback_buffer;
	std::vector<midi_message> out_midi_messages;
	std::vector<midi_message> in_midi_messages;
	std::vector<note_message> note_messages;
	std::vector<metaconnection*> connections;
	std::vector<metaconnection*> out_connections;
	std::string stream_source;
	int timesource_plugin_id;
	int timesource_group;
	int timesource_track;
	patternplayer* global_timesource; // this plugins global timesource
	std::vector<patternplayer*> track_timesources; // this plugins track timesources
	boost::shared_ptr<pluginaudiodata> audiodata;

	metaplugin();
	void pre_initialize();
	void initialize(zzub::archive* arc);
	int get_parameter_count(int group, int track);
	const parameter* get_parameter_info(int group, int track, int column);
	int get_parameter_track_row_bytesize(int g, int t);
	void transfer_parameter_track_row(int g, int t, const pluginstate& from_pattern, void* param_ptr, bool copy_all);
	void transfer_parameter_row(int g, const pluginstate& from_pattern, pluginstate& target_pattern, bool copy_all);
	void transfer_parameter_track_row(int g, int t, const void* source, zzub::pluginstate& to_pattern, bool copy_all);
	void default_parameter_track(pluginstate& p, int group, int track, const std::vector<const parameter*>& parameters);
	int get_parameter(int group, int track, int column);
	int get_parameter_direct(int group, int track, int column);
	void set_parameter(int group, int track, int column, int value, bool record);
	int get_latency();
	int get_input_channel_count();
	int get_output_channel_count();
	void set_patternplayer(int group, int track, zzub::patternplayer* player);
	patternplayer* get_patternplayer(int group, int track);
};

struct midimapping {
	int id;
	int plugin_id;
	int group, track, column;
	int channel, controller;

	bool operator == (const midimapping& mm) {
		return this == &mm;
	}
};

struct keyjazz_note {
	int plugin_id;
	int timestamp;	// tick when note was played
	int group, track;
	int note;
	bool delay_off;		// set to true if a noteoff was sent on the same timestamp (tick)
};

struct keyjazz {
	std::vector<keyjazz_note> notes;

	bool set_keyjazz_note(metaplugin& mp, int note, int prev_note, int velocity, int& note_group, int& note_track, int& note_column, int& velocity_column);

};

struct plugingraph {
	std::vector<metaplugin*>& workorder;
	std::vector<metaconnection*>& backedges;

	plugingraph(std::vector<metaplugin*>& wo, std::vector<metaconnection*>& be):workorder(wo), backedges(be) { }
	void make(std::vector<boost::shared_ptr<metaplugin> >& plugins, std::vector<boost::shared_ptr<metaconnection> >& connections);
};

void create_state_pattern(const zzub::info* info, int tracks, zzub::pluginstate& result, bool usedefault);

};
