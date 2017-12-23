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
#include <deque>
#include <functional>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include <map>
#include <cstring>
#include "song.h"
#include "graph.h"
#include "connections.h"

#if defined(__powerpc__) || defined(__POWERPC__) || defined(_M_PPC) || defined(__BIG_ENDIAN__)
#define ZZUB_BIG_ENDIAN
#endif
 
using std::cerr;
using std::endl;

namespace zzub {

void create_state_pattern(const zzub::info* info, int tracks, zzub::pluginstate& result, bool usedefault) {
	// add internal tracks
	if (result.groups[0].size() == 0)
		result.add_track(0, *info->internal_parameters, usedefault);

	// add global tracks
	if (result.groups[1].size() == 0)
		result.add_track(1, info->global_parameters, usedefault);

	for (int i = (int)result.groups[2].size(); i < tracks; i++) {
		result.add_track(2, info->track_parameters, usedefault);
	}
	for (int i = tracks; i < (int)result.groups[2].size(); i++) {
		result.remove_track(2, tracks);
	}

	// add states for controller columns
	if (info->flags & zzub_plugin_flag_has_event_output) {
		while (result.get_track_count(3) > 0) result.remove_track(3, 0);
		result.add_track(3, info->controller_parameters, usedefault);
	}

	// add virtual tracks
	if (result.groups[4].size() == 0)
		result.add_track(4, info->virtual_parameters, usedefault);

}

bool is_note_playing(int plugin_id, const std::vector<zzub::keyjazz_note>& keyjazz, int note) {
	for (size_t i = 0; i < keyjazz.size(); i++)
		if (keyjazz[i].plugin_id == plugin_id && keyjazz[i].note == note) return true;
	return false;
}

/***

	pattern

***/

interpolator::interpolator() {
	param = 0;
	numsteps = 4;
	reset(absolute);
}

interpolator::interpolator(const zzub::parameter* _param, int _mode) {
	param = _param;
	numsteps = 4;
	reset(_mode);
}

void interpolator::reset(int _mode) {
	mode = _mode;
	time = -1;
	memset(events, 0, sizeof(events));
	value_position = -1;
	owner = 0;
	switch (mode) {
		case absolute:
			needs_before = 0;
			needs_after = 0;
			break;
		case inertial:
			needs_before = 1;
			needs_after = 0;
			break;
		case linear:
			needs_before = 0;
			needs_after = 1;
			break;
		default:
			assert(false);
	}
}

void interpolator::set_value(int index, int time, int value) {
	int eventindex = needs_before + index;
	events[eventindex].time = time;
	events[eventindex].value = value;
}

void interpolator::shift_set_value(patternplayer* _owner, int time, int value) {
	owner = _owner;
	int eventcount = needs_before + 1 + needs_after;
	for (int i = eventcount - 1; i > 0; i--) {
		events[i - 1] = events[i];
	}
	events[eventcount - 1].time = time;
	events[eventcount - 1].value = value;
}

int interpolator::get_value() {
	switch (mode) {
		case 0:
			return get_value_absolute();
		case 1:
			return get_value_inertial();
		case 2:
			return get_value_linear();
		default:
			assert(false);
			return 0;
	}
}

int interpolator::get_value_absolute() {
	if (time == value_position && tick_position == 0) 
		return param->value_none;

	value_position = time;
	if (time == events[0].time && tick_position == 0)
		return events[0].value;
	else
		return param->value_none;
}

int interpolator::get_value_inertial() {
	if (time == -1 || tick_position != 0) return param->value_none;

	int timediff = time - events[1].time;
	if (timediff > numsteps) return param->value_none;

	assert(timediff >= 0);

	float valuedelta = (events[1].value - events[0].value) / (float)numsteps;

	int value = events[0].value + valuedelta * timediff;

	return value;
}

int interpolator::get_value_linear() {
	// returns novalue for either:
	//   time == -1  ->  the interpolator is inactive
	//   tick_position != 0   ->  the interpolator is between ticks
	//   events[0].value == param->value_none  ->  the interpolator has not reached the first event on this pattern
	//   time >= events[1].time  ->  the interpolator has reached the end of events on this pattern
	//   value_position == time -> the interpolator already returned values for this position

	if (time == -1 || tick_position != 0 || events[0].value == param->value_none || time > events[1].time || (value_position == time && tick_position == 0)) return param->value_none;

	value_position = time;
	int timediff = events[1].time - events[0].time;
	if (timediff == 0) return events[0].value;

	int valuetime = time - events[0].time;
	float valuedelta = (events[1].value - events[0].value) / (float)timediff;
	int value = events[0].value + valuedelta * valuetime;
	return value;

}

pluginstate::pluginstate() {
}

int pluginstate::add_track(int group, std::vector<const zzub::parameter*> parameters, bool usedefault) {
	groupinfo[group].push_back(parameters);
	groups[group].push_back(pluginstate::track(parameters.size()));
	interpolators[group].push_back(pluginstate::track_interpolate(parameters.size()));
	for (int i = 0; i < (int)parameters.size(); i++) {
		interpolators[group].back()[i].param = parameters[i];
	}

	for (pluginstate::track::iterator i = groups[group].back().begin(); i != groups[group].back().end(); ++i) {
		int index = (int)(i - groups[group].back().begin());
		if (usedefault && (groupinfo[group].back()[index]->flags & zzub_parameter_flag_state) != 0)
			*i = groupinfo[group].back()[index]->value_default; else
			*i = groupinfo[group].back()[index]->value_none;
	}
	return (int)groupinfo[group].size() - 1;
}

void pluginstate::remove_track(int group, int track) {
	groupinfo[group].erase(groupinfo[group].begin() + track);
	groups[group].erase(groups[group].begin() + track);
	interpolators[group].erase(interpolators[group].begin() + track);
}

int pluginstate::get_track_count(int group) const {
	return (int)groupinfo[group].size();
}

int pluginstate::get_column_count(int group, int track) const {
	return (int)groupinfo[group][track].size();
}

int pluginstate::get_value(int group, int track, int column) const {
	return groups[group][track][column];
}

void pluginstate::set_value(int group, int track, int column, int value) {
	assert(track >= 0 && track < (int)groups[group].size());
	assert(column >= 0 && column < (int)groups[group][track].size());

	groups[group][track][column] = value;
}

void pluginstate::clear_contents() {
	for (int i = 0; i < max_group_count; i++) {
		clear_group(i);
	}
}

void pluginstate::clear_group(int group) {
	for (pluginstate::group::iterator j = groups[group].begin(); j != groups[group].end(); ++j) {
		int track = (int)(j - groups[group].begin());
		for (pluginstate::track::iterator k = j->begin(); k != j->end(); ++k) {
			int column = (int)(k - j->begin());
			*k = groupinfo[group][track][column]->value_none;
		}
	}
}

void pluginstate::swap(pluginstate& p) {
	for (int i = 0; i < max_group_count; i++) {
		groupinfo[i].swap(p.groupinfo[i]);
		groups[i].swap(p.groups[i]);
	}
}

/***

	keyjazz

***/

bool keyjazz::set_keyjazz_note(metaplugin& m, int note, int prev_note, int velocity, int& note_group, int& note_track, int& note_column, int& velocity_column) {

	int plugin_id = m.id;

	note_group = -1;

	// ignore note-off when prevNote wasnt already playing, except if prevNote is -1, where we stop all playing notes
	if ((note == zzub_note_value_off || note == zzub_note_value_cut) && prev_note != -1 && !is_note_playing(plugin_id, notes, prev_note)) return false;

	if (m.info->note_group == -1) return false;
	note_group = m.info->note_group;
	note_column = m.info->note_column;
	velocity_column = m.info->velocity_column;

	if (note_group == 2) {
		// play note on track
		if (note == zzub_note_value_off || note == zzub_note_value_cut) {
			// find which track this note was played in and play a note-stop
			// if note-off is on the same tick as the note it stops, we wait 
			// until next tick before "comitting" it, so that we dont overwrite
			// notes when recording
			// if timestamp is >= lastTickPos, set keyjazz->delay_off to true 
			// and return. a poller checks the keyjazz-vector each tick and
			// records/plays noteoffs then.
			for (size_t i = 0; i < notes.size(); i++) {
				if (notes[i].plugin_id != plugin_id) continue;
				if (notes[i].note == prev_note || prev_note == -1) {
					
					note_group = notes[i].group;
					note_track = notes[i].track;

					/*if (notes[i].timestamp >= last_tick_work_position) {
						//cerr << "note off on the same tick as note was played!" << endl;
						notes[i].delay_off = true;
						return true;
					}*/

					notes.erase(notes.begin() + i);
					i--;
					if (prev_note != -1) return true;
				}
			}
		} else {
			int lowest_time = std::numeric_limits<int>::max();
			int lowest_track = -1;
			int found_track = -1;
			size_t lowest_index;

			std::vector<bool> found_tracks(m.tracks);
			for (size_t i = 0; i < found_tracks.size(); i++)
				found_tracks[i] = false;

			for (size_t j = 0; j < notes.size(); j++) {
				if (notes[j].plugin_id != plugin_id) continue;
				size_t track = notes[j].track;
				if (track >= found_tracks.size()) continue;

				found_tracks[track] = true;
				if (notes[j].timestamp < lowest_time) {
					lowest_time = notes[j].timestamp;
					lowest_track = notes[j].track;
					lowest_index = j;
				}
			}

			for (size_t i = 0; i < found_tracks.size(); i++) {
				if (found_tracks[i] == false) {
					found_track = (int)i;
					break;
				}
			}
			if (found_track == -1) {
				found_track = lowest_track;
				notes.erase(notes.begin() + lowest_index);
			}

			note_track = found_track;

			// find an available track or the one with lowest timestamp
			keyjazz_note ki = { plugin_id, 0/*work_position*/, note_group, found_track, note, false };
			notes.push_back(ki);
			return true;
		}
	} else {
		// play global note - no need for track counting
		if (note == zzub_note_value_off || note == zzub_note_value_cut) {
			for (size_t i = 0; i < notes.size(); i++) {
				if (notes[i].plugin_id != plugin_id) continue;
				if (notes[i].note == prev_note || prev_note == -1) {
					note_track = 0;

					/*if (notes[i].timestamp >= last_tick_work_position) {
						//cerr << "detected a note off on the same tick as note was played!" << endl;
						notes[i].delay_off = true;
						return true;
					}*/

					notes.clear();
					return true;
				}
			}
		} else {
			keyjazz_note ki = { plugin_id, 0/*work_position*/, 1, 0, note, false };
			notes.clear();
			notes.push_back(ki);

			note_track = 0;
			return true;
		}
	}
	return false;
}

/***

	pattern

***/

// ---

/***

	metaplugin

***/

metaplugin::metaplugin() {
	id = -1;
	midi_input_channel = 17;
	timesource_plugin_id = -1;
	global_timesource = 0;
	initialized = false;
}

const parameter* metaplugin::get_parameter_info(int group, int track, int column) {
	switch (group) {
		case 0:
			return (*info->internal_parameters)[column];
		case 1: // globals
			return info->global_parameters[column];
		case 2: // track params
			return info->track_parameters[column];
		case 3: // controller params
			return info->controller_parameters[column];
		case 4:
			return info->virtual_parameters[column];
		default:
			return 0;
	}
}

int metaplugin::get_parameter_count(int group, int track) {
	switch (group) {
		case 0: // internals
			return (int)info->internal_parameters->size();
		case 1: // globals
			return (int)info->global_parameters.size();
		case 2: // track params
			return (int)info->track_parameters.size();
		case 3: // controller params
			return (int)info->controller_parameters.size();
		default:
			assert(false);
			return 0;
	}
}

int metaplugin::get_parameter(int group, int track, int column) {
	// use state_write or state_last depending on what is freshest
	int v = state_write.get_value(group, track, column);
	const zzub::parameter* param = get_parameter_info(group, track, column);
	if (v != param->value_none) return v;

	return state_last.get_value(group, track, column);
}


int metaplugin::get_parameter_direct(int group, int track, int column) {
	return state_write.get_value(group, track, column);
}

void metaplugin::set_parameter(int group, int track, int column, int value, bool record) {
	// writing to controller state should not cause a re-process
	if (group != 3) audiodata->is_dirty = true;

	state_write.set_value(group, track, column, value);
	if (record)
		state_automation.set_value(group, track, column, value);
}

void metaplugin::default_parameter_track(zzub::pluginstate& p, int group, int track, const std::vector<const zzub::parameter*>& parameters) {
	for (int j = 0; j < p.get_column_count(group, track); j++) {
		int value;
		if (parameters[j]->flags & zzub_parameter_flag_state)
			value = parameters[j]->value_default; else
			value = parameters[j]->value_none;
		p.groups[group][track][j] = value;
	}
}

int metaplugin::get_parameter_track_row_bytesize(int g, int t) {
	int size = 0;
	for (int i = 0; i < get_parameter_count(g, t); i++) {
		const zzub::parameter* param = get_parameter_info(g, t, i);
		size += param->get_bytesize();
	}
	return size;
}

void metaplugin::transfer_parameter_track_row(int g, int t, const zzub::pluginstate& from_pattern, void* target, bool copy_all) {
	char* param_ptr = (char*)target;
	int param_ofs = 0;
	for (int i = 0; i < (int)from_pattern.get_column_count(g, t); i++) {
		assert(target != 0); // plugin should have a parameter block (if there are parameters)
		int v = from_pattern.get_value(g, t, i);
		const zzub::parameter* param = get_parameter_info(g, t, i);
		assert(v == param->value_none || (v >= param->value_min && v <= param->value_max) || (param->type == zzub_parameter_type_note && (v == zzub_note_value_off || v == zzub_note_value_cut)));
		int size = param->get_bytesize();
		if (copy_all || v != param->value_none) {
			char* value_ptr = (char*)&v;
#if defined(ZZUB_BIG_ENDIAN)
			value_ptr += sizeof(int) - size;
#endif
			memcpy(param_ptr + param_ofs, value_ptr, size);
		}
		param_ofs += size;
	}
}

void metaplugin::transfer_parameter_track_row(int g, int t, const void* source, zzub::pluginstate& to_pattern, bool copy_all) {
	char* param_ptr = (char*)source;
	int param_ofs = 0;
	for (int i = 0; i < (int)to_pattern.get_column_count(g, t); i++) {
		int v = 0;
		const zzub::parameter* param = get_parameter_info(g, t, i);
		int size = param->get_bytesize();
		char* value_ptr = (char*)&v;
#if defined(ZZUB_BIG_ENDIAN)
		value_ptr += sizeof(int) - size;
#endif
		memcpy(value_ptr, param_ptr + param_ofs, size);
		assert(v == param->value_none || (v >= param->value_min && v <= param->value_max) || (param->type == zzub_parameter_type_note && (v == zzub_note_value_off || v == zzub_note_value_cut)));
		if (copy_all || v != param->value_none) {
			to_pattern.set_value(g, t, i, v);
		}
		param_ofs += size;
	}
}

void metaplugin::transfer_parameter_row(int g, const zzub::pluginstate& from_pattern, zzub::pluginstate& target_pattern, bool copy_all) {
	int source_tracks = (int)from_pattern.groupinfo[g].size();
	int target_tracks = (int)target_pattern.groupinfo[g].size();

	// make sure we dont write outside the buffer
	int transfer_track_count = (int)std::min(target_tracks, source_tracks);
	for (int j = 0; j < transfer_track_count; j++) {
		int source_columns = (int)from_pattern.groupinfo[g][j].size();
		int target_columns = (int)target_pattern.groupinfo[g][j].size();
		int transfer_column_count = std::min(target_columns, source_columns);
		for (int i = 0; i < transfer_column_count; i++) {
			const zzub::parameter* param = get_parameter_info(g, j, i);
			int v = from_pattern.get_value(g, j, i);
			if (copy_all || v != param->value_none)
				target_pattern.set_value(g, j, i, v);
		}
	}
}

void metaplugin::pre_initialize() {
	// setting default attributes before init() makes some stereo wrapped machines work - instead of crashing in first attributesChanged 
	if (plugin->attributes) 
		for (size_t i = 0; i < info->attributes.size(); i++) 
			plugin->attributes[i] = info->attributes[i]->value_default;

	create_state_pattern(info, tracks, state_write, true);
	create_state_pattern(info, tracks, state_last, true);
	create_state_pattern(info, tracks, state_automation, false);
}

void metaplugin::initialize(zzub::archive* arc) {
	plugin->init(arc);

	plugin->set_track_count(tracks);
	plugin->attributes_changed();
	plugin->set_stream_source(stream_source.c_str());

	audiodata->is_dirty = true;
	//initialized = true; // must be set after first process_events(), which happens after this

}

int metaplugin::get_latency() {
	return audiodata->latency != -1 ? std::min(audiodata->latency, zzub_latency_buffer_size - zzub_buffer_size) : std::min(plugin->get_latency(), zzub_latency_buffer_size - zzub_buffer_size);
}

int metaplugin::get_input_channel_count() {
	int channelcount = plugin->get_input_channel_count();
	if (channelcount == -1) return info->inputs;
	return channelcount;
}

int metaplugin::get_output_channel_count() {
	int channelcount = plugin->get_output_channel_count();
	if (channelcount == -1) return info->outputs;
	return channelcount;
}

void metaplugin::set_patternplayer(int group, int track, zzub::patternplayer* pp) {
	switch (group) {
		case 1:
			global_timesource = pp;
		case 2:
			if ((int)track_timesources.size() <= track) track_timesources.resize(track + 1);
			track_timesources[track] = pp;
			break;
		default:
			assert(false);
	}
}

patternplayer* metaplugin::get_patternplayer(int group, int track) {
	switch (group) {
		case 1:
			return global_timesource;
		case 2:
			return track_timesources[track];
		default:
			assert(false);
			return 0;
	}
}


};
