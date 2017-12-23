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

#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <map>
#include <cstring>
#include <limits>
#include "library.h"
#include "player.h"
#include "zzub/zzub.h"
#include "zzub/plugin.h"
#include "bmxreader.h"
#include "decompress.h"

#if defined(POSIX)
#define _strcmpi strcasecmp
#endif

using namespace std;

namespace zzub {

//#define BUZZ_PLUGIN_FLAGS_MASK (zzub_plugin_flag_is_root|zzub_plugin_flag_has_audio_input|zzub_plugin_flag_has_audio_output)
//#define BUZZ_ROOT_PLUGIN_FLAGS (zzub_plugin_flag_is_root|zzub_plugin_flag_has_audio_input)
//#define BUZZ_GENERATOR_PLUGIN_FLAGS (zzub_plugin_flag_has_audio_output)
//#define BUZZ_EFFECT_PLUGIN_FLAGS (zzub_plugin_flag_has_audio_input|zzub_plugin_flag_has_audio_output)
// machine types
#define MT_MASTER		0 
#define MT_GENERATOR	1
#define MT_EFFECT		2


/***

	BuzzReader

***/

BuzzReader::BuzzReader(zzub_input_t* inf) {
	sections = 0;
	if (!open(inf))
		f = 0;

	ignoreWaves = false;
	ignorePatterns = false;
	ignoreSequences = false;
	offsetX = offsetY = 0.0;
}

BuzzReader::~BuzzReader() {
	clear();
}

void BuzzReader::clear() {
	delete[] sections;
	sections = 0;
}

/*! \brief Open and append contents from a .BMX-file into the current player instance. */
bool BuzzReader::readPlayer(zzub_player_t* pl) {
	if (!f) return false;

	bool returnValue = true;

	player = pl;

//	player->set_state(player_state_muted);

	initSequences();
	if (!loadPara()) goto error;
	if (!ignoreWaves && !loadWaveTable()) goto error;
	if (!loadMachines()) goto error;
	if (!loadConnections()) goto error;
	if (!ignorePatterns && !loadPatterns()) goto error;
	if (!ignoreSequences && !loadSequences()) goto error;
	if (!ignoreWaves && !loadWaves()) goto error;
	if (!loadMidi()) goto error;
	if (!loadInfoText()) goto error;

	goto all_ok;
error:
	returnValue = false;
	//player->front.load_error = lastError;
all_ok:
	//player->front.load_warning = lastWarning;

//	player->set_state(player_state_stopped);

	return returnValue;
}

bool BuzzReader::open(zzub_input_t* inf) {

	lastWarning = "";
	lastError = "";

	machines.clear();
	//connections.clear();

	unsigned int magic;

	zzub_input_read(inf, (char*)&magic, sizeof(unsigned int));
	zzub_input_read(inf, (char*)&sectionCount, sizeof(unsigned int));
	//inf->read(sectionCount);

	if (magic != MAGIC_Buzz) {
		lastError = "Not a valid Buzz file";
		return false;
	}
	
	sections = new Section[sectionCount];
	for (size_t i = 0; i<sectionCount; i++) {
		Section& section = sections[i];
		zzub_input_read(inf, (char*)&section.magic, sizeof(unsigned int));
		zzub_input_read(inf, (char*)&section.offset, sizeof(unsigned int));
		zzub_input_read(inf, (char*)&section.size, sizeof(unsigned int));
	}
	f = inf;
	return true;
}

Section* BuzzReader::getSection(unsigned int magic) {
	for (size_t i=0; i<sectionCount; i++) {
		if (sections[i].magic==magic) return &sections[i];
	}
	return 0;
}

std::string rewriteBuzzWrapperUri(std::string fileName) {
	string prefix = "@zzub.org/buzz2zzub/";
	if (fileName.find(prefix) == 0) {
		// i did it again; this trims off extra prefixes that were accidentally added in some songs
		while (fileName.find(prefix, prefix.length()) != std::string::npos) {
			fileName = fileName.substr(prefix.length());
		}
		return fileName;
	}
	fileName = prefix + fileName;
	replace(fileName.begin(), fileName.end(), ' ', '+');
	return fileName;
}

template <typename T>
int zzub_read(zzub_input_t* f, T &d) { return zzub_input_read(f, (char*)&d, sizeof(T)); } 

int zzub_read(zzub_input_t* f, std::string& d) {
	char c = -1;
	d = "";
	int i = 0;
	do {
		if (!zzub_read<char>(f, c)) break;
		if (c) d += c;
		i++;
	} while (c != 0);
	return i;
}

int get_bytesize(zzub_parameter_t* p) {
	int type = zzub_parameter_get_type(p);
	switch(type) {
		case zzub_parameter_type_note:
		case zzub_parameter_type_switch:
		case zzub_parameter_type_byte:
			return 1;
		case zzub_parameter_type_word:
			return 2;
		default:
			return 0;
	}
	return 0;
}

// buzz_name / fullName = dll file name part
zzub_pluginloader_t* get_loader_from_buzz_name(zzub_player_t* player, std::string fullName, std::string* resulturi) {
	zzub_pluginloader_t* loader;
	std::string uri;
	if (fullName == "Master") {
		uri = "@zzub.org/master"; 
		loader = zzub_player_get_pluginloader_by_name(player, uri.c_str());
	} else {
		uri = rewriteBuzzWrapperUri(fullName);
		loader = zzub_player_get_pluginloader_by_name(player, uri.c_str());
	}
	*resulturi = uri;
	return loader;
}

zzub_parameter_t* BuzzReader::get_parameter(zzub_pluginloader_t* loader, int group, int column) {
	if (loader == masterloader && group == 1 && (column == 1 || column == 2))
		return zzub_pluginloader_get_parameter(seqloader, group, column - 1);
	return zzub_pluginloader_get_parameter(loader, group, column);
}

int BuzzReader::get_parameter_count(zzub_pluginloader_t* loader, int group) {
	if (loader == masterloader && group == 1) return 3;
	return zzub_pluginloader_get_parameter_count(loader, group);
}

zzub_parameter_t* BuzzReader::get_parameter(zzub_plugin_t* plugin, int group, int track, int column) {
	if (plugin == masterplug && group == 1 && (column == 1 || column == 2))
		return zzub_plugin_get_parameter(seqplug, group, track, column - 1);
	return zzub_plugin_get_parameter(plugin, group, track, column);
}

int BuzzReader::get_parameter_count(zzub_plugin_t* plugin, int group, int track) {
	if (plugin == masterplug && group == 1) return 3;
	return zzub_plugin_get_parameter_count(plugin, group, track);
}

void BuzzReader::set_parameter_value(zzub_plugin_t* plugin, int group, int track, int column, int value) {
	if (plugin == masterplug && group == 1 && (column == 1 || column == 2))
		return zzub_plugin_set_parameter_value(seqplug, group, track, column - 1, value, 0);
	return zzub_plugin_set_parameter_value(plugin, group, track, column, value, 0);
}

void BuzzReader::insert_pattern_value(zzub_pattern_t* pattern, zzub_plugin_t* plugin, int group, int track, int column, int time, int value) {
	if (plugin == masterplug && group == 1 && (column == 1 || column == 2)) {
		int id = zzub_plugin_get_id(seqplug);
		zzub_pattern_insert_value(pattern, id, group, track, column - 1, time, value, 0);
	} else {
		int id = zzub_plugin_get_id(plugin);
		zzub_pattern_insert_value(pattern, id, group, track, column, time, value, 0);
	}
}

void BuzzReader::add_format_column(zzub_pattern_format_t* format, zzub_plugin_t* plugin, int group, int track, int column, int index) {
	if (plugin == masterplug && group == 1 && (column == 1 || column == 2))
		zzub_pattern_format_add_column(format, seqplug, group, track, column - 1, index);
	else
		zzub_pattern_format_add_column(format, plugin, group, track, column, index);
}


armstrong::frontend::dummyinfo* BuzzReader::find_dummy(std::string name, std::string typeName) {
	for (std::vector<boost::shared_ptr<armstrong::frontend::dummyinfo> >::iterator i = paraplugins.begin(); i != paraplugins.end(); ++i) {
		if (i->get()->instanceShortName == name) return i->get();
	}
	return 0;
}

bool BuzzReader::loadMachines() {

	Section* section = getSection(MAGIC_MACH);
	if (!section) {
		lastError="Error: Cannot find MACH section.\n" + lastError;
		return false;
	}
	zzub_input_seek(f, section->offset, SEEK_SET);

	bool is_importing = false;//num_vertices(player->front.graph) > 1;

	// put wavetable on the backbuffer in case any plugins query for info while loading

	bool returnValue = true;
	unsigned short machineCount;
	zzub_input_read(f, (char*)&machineCount, sizeof(unsigned short));

	for (int j = 0; j < machineCount; j++) {
		string machineName;
		char type;
		float x, y;
		string machineType, pluginUri;
		int dataSize;
		unsigned short attributeCount, tracks;

		zzub_read(f, machineName);
		zzub_read(f, type);
		if (type)
			zzub_read(f, machineType); else
			machineType = "Master";
		zzub_read(f, x);
		zzub_read(f, y);

		// dont offset the master when importing
		if (!(type == 0 && is_importing)) {
			x += offsetX;
			y += offsetY;
		}

		zzub_read(f, dataSize);

		int filepos = zzub_input_position(f);

		bmxmachine mac;
		mac.loader = get_loader_from_buzz_name(player, machineType, &pluginUri);
		mac.plugin = 0;
		mac.parainfo = find_dummy(machineName, machineType);

		string loadedMachineName = machineName;

		if (mac.loader != 0) {
			if (type == MT_MASTER) {
				mac.plugin = zzub_player_get_plugin_by_name(player, "Master");	// NOTE: 0 == master
			} else {
				mac.plugin = zzub_player_create_plugin(player, f, dataSize, machineName.c_str(), mac.loader, 0);
			}
		} else
			mac.loader = mac.parainfo;

		// NOTE: seek to end of data, as create_plugin() could read partially from the input in case of failure
		zzub_input_seek(f, filepos + dataSize, SEEK_SET);

		// NOTE: cannot create dummy machines via the api anymore, so any missing machines need to be skipped
		// if there is no loader for this uri, or validation failed, try to create a dummy loader + machine
		if (mac.plugin == 0 && mac.parainfo == 0) {
			armstrong::frontend::validationerror err;
			err.type = zzub_validation_error_type_plugin_not_found;
			err.description = "Error: Cannot load plugin.";
			err.info = 0;
			err.original_plugin_name = loadedMachineName;
			player->load_errors.push_back(err);

			lastError = machineName + " (" + machineType + ") Error: Cannot load machine.\n" + lastError;
			returnValue = false;
			break;
		} else if (mac.plugin == 0) {
			armstrong::frontend::validationerror err;
			err.type = zzub_validation_error_type_plugin_not_found;
			err.description = "Error: Cannot load plugin.";
			err.info = 0; //mac.loader;
			err.original_plugin_name = loadedMachineName;
			player->load_errors.push_back(err);
		}

		if (type == MT_MASTER) {
			masterplug = mac.plugin;
			masterloader = mac.loader;
		}

		// read attributes, and then test if machine was successfully created. attributes are used to create a dummy machine in case a machine was not found
		zzub_read(f, attributeCount);

		std::vector<int> attributeValues(attributeCount);

		// casting attributeCount to signed short so we catch the situation described in bmformat_hotkey ??
		for (int k = 0; k < (signed short)attributeCount; k++) {
			std::string name;
			zzub_read(f, name);
			zzub_read(f, attributeValues[k]);
		}
		
		// load global default
		int global_param_count = get_parameter_count(mac.loader, 1);
		
		std::vector<int> globals(global_param_count);
		for (int k = 0; k < global_param_count; k++) {
			zzub_parameter_t* param = get_parameter(mac.loader, 1, k);

			int v = 0;
			zzub_input_read(f, (char*)&v, get_bytesize(param));
			int flags = zzub_parameter_get_flags(param);
			if ((flags & zzub_parameter_flag_state) == 0)
				v = zzub_parameter_get_value_none(param);
			globals[k] = v;
		}

		// load track defaults
		zzub_read(f, tracks);

		std::vector<std::vector<int> > trackvalues(tracks);
		for (int l = 0; l < tracks; l++) {
			// Get param count and bytesizes from either pluginloader or PARA section
			int track_param_count = zzub_pluginloader_get_parameter_count(mac.loader, 2);
			trackvalues[l].resize(track_param_count);
			for (int k = 0; k < track_param_count; k++) {
				zzub_parameter_t* param = zzub_pluginloader_get_parameter(mac.loader, 2, k);
				int v = 0;
				int parambytes = get_bytesize(param);
				zzub_input_read(f, (char*)&v, parambytes);

				int flags = zzub_parameter_get_flags(param);
				if ((flags & zzub_parameter_flag_state) == 0)
					v = zzub_parameter_get_value_none(param);
				trackvalues[l][k] = v;
			}
		}

		// done reading machine data from file, initialize the machine
		if (mac.plugin != 0) {

			int plugAttrCount = std::min(zzub_pluginloader_get_attribute_count(mac.loader), (int)attributeValues.size());
			for (int i = 0; i < plugAttrCount; i++)
				zzub_plugin_set_attribute_value(mac.plugin, i, attributeValues[i]);

			zzub_plugin_set_track_count(mac.plugin, tracks);
			zzub_plugin_set_position(mac.plugin, x, y);

			for (int k = 0; k < global_param_count; k++) {
				int v = globals[k]; 
				set_parameter_value(mac.plugin, 1, 0, k, v);
			}

			for (size_t l = 0; l < tracks; l++) {
				for (int k = 0; k < zzub_pluginloader_get_parameter_count(mac.loader, 2); k++) {
					zzub_parameter_t* param = zzub_pluginloader_get_parameter(mac.loader, 2, k);
					int v = trackvalues[l][k];
					zzub_plugin_set_parameter_value(mac.plugin, 2, l, k, v, 0);
				}
			}

			// add a tick to the backbuffer-operations so we dont tick here, but later in the flush.
			// this is specifically to not confuse plugins that reset parameters in set_track_count().

			zzub_plugin_tick(mac.plugin, 0);
		}

		machines.push_back(mac);
	}

	return returnValue;
}

bool BuzzReader::read_track(bmxmachine& plugin, zzub_pattern_t* result, int group, int track, int rows) {

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < get_parameter_count(plugin.loader, group); j++) {
			zzub_parameter_t* param = get_parameter(plugin.loader, group, j);
			assert(param != 0);
			int value = 0;
			zzub_input_read(f, (char*)&value, get_bytesize(param));
			if (result != 0 && value != zzub_parameter_get_value_none(param))
				insert_pattern_value(result, plugin.plugin, group, track, j, i, value);
		}
	}

	return true;
}

zzub_pattern_format_t* BuzzReader::create_pattern_format_from_plugin(zzub_player_t* player, zzub_plugin_t* plugin) {
	const char* pluginname = zzub_plugin_get_name(plugin);

	zzub_pattern_format_t* patfmt = zzub_player_create_pattern_format(player, pluginname);
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(plugin);
	
	int screenidx = 0;

	for (int j = 0; j < zzub_plugin_get_input_connection_count(plugin); j++) {
		zzub_connection_t* conn = zzub_plugin_get_input_connection(plugin, j);
		zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(conn);

		for (int l = 0; l < zzub_plugin_get_track_count(connplug, 2); l++) {
			for (int k = 0; k < zzub_plugin_get_parameter_count(connplug, 2, l); k++) {
				zzub_parameter_t* para = zzub_plugin_get_parameter(connplug, 2, l, k);
				zzub_pattern_format_add_column(patfmt, connplug, 2, l, k, screenidx);
				++screenidx;
			}
		}
	}

	for (int j = 0; j < get_parameter_count(loader, 1); j++) {
		zzub_parameter_t* para = get_parameter(loader, 1, j);
		add_format_column(patfmt, plugin, 1, 0, j, screenidx);
		++screenidx;
	}

	for (int i = 0; i < zzub_plugin_get_track_count(plugin, 2); i++) {
		for (int j = 0; j < zzub_pluginloader_get_parameter_count(loader, 2); j++) {
			zzub_parameter_t* para = zzub_pluginloader_get_parameter(loader, 2, j);
			zzub_pattern_format_add_column(patfmt, plugin, 2, i, j, screenidx);
			++screenidx;
		}
	}

	return patfmt;
}

bool BuzzReader::loadPatterns() {
	Section* section = getSection(MAGIC_PATT);
	zzub_input_seek(f, section->offset, SEEK_SET);

	for (vector<bmxmachine>::iterator i = machines.begin(); i != machines.end(); ++i) {
		unsigned short patterns = 0;
		unsigned short tracks = 0;

		zzub_read(f, patterns);
		zzub_read(f, tracks);

		zzub_pattern_format_t* format;
		if (i->plugin != 0)
			format = create_pattern_format_from_plugin(player, i->plugin);
		else
			format = 0;

		for (int j = 0; j < patterns; j++) {
			unsigned short rows;

			std::string name;
			zzub_read(f, name);
			zzub_read(f, rows);

			zzub_pattern_t* pat;
			if (format != 0) {
				pat = zzub_player_create_pattern(player, format, "", rows);
				zzub_pattern_set_name(pat, name.c_str());
				i->patterns.push_back(pat);
			} else
				pat = 0;

			for (size_t k = 0; k < i->from_plugins.size(); k++) {
				unsigned short machineIndex;
				zzub_read(f, machineIndex);	// NOTE: this byte is not documented in bmformat.txt. in fact the connection pattern section is terribly documented.

				if (machineIndex >= machines.size()) {
					lastError = "Invalid pattern connection machine index on ";// + machine.name;
					return false;
				}

				zzub_plugin_t* from_plugin = i->from_plugins[k].plugin;
				zzub_connection_t* conn;
				if (i->plugin != 0 && from_plugin != 0)
					conn = zzub_plugin_get_input_connection_by_type(i->plugin, from_plugin, zzub_connection_type_audio);
				else
					conn = 0;

				if (conn != 0) {
					zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(conn);
					int connplugid = zzub_plugin_get_id(connplug);
					int conntracks = zzub_plugin_get_track_count(connplug, 2);
					for (int k = 0; k < rows; k++) {
						int ampvalue = 0;
						int panvalue = 0;
						zzub_input_read(f, (char*)&ampvalue, sizeof(short));
						zzub_input_read(f, (char*)&panvalue, sizeof(short));
						if (ampvalue != 65535) {
							for (int l = 0; l < conntracks; l++) {
								zzub_pattern_insert_value(pat, connplugid, 2, l, 0, k, ampvalue, 0);
							}
						}

					}
				} else {
					// if we come here the connection was saved, but not successfully reconnected.
					// just skip it.
					for (int k = 0; k < rows; k++) {
						int tempvalue = 0;
						zzub_input_read(f, (char*)&tempvalue, sizeof(short));
						zzub_input_read(f, (char*)&tempvalue, sizeof(short));
					}
				}
			}

			read_track(*i, pat, 1, 0, rows);

			for (int l = 0; l < tracks; l++) {
				read_track(*i, pat, 2, l, rows);
			}
		}
	}

	return true;
}

bool BuzzReader::loadConnections2() {

/*	Section* section=getSection(MAGIC_CON2);
	f->seek(section->offset, SEEK_SET);

	unsigned short version = 1;
	f->read(version);
	if (version != 1) return false;

	unsigned short conns=0;
	f->read(conns);

	unsigned short type = 0;
	unsigned short index1 = 0, index2 = 0;
	for (int i = 0; i<conns; i++) {
		f->read(type);
		f->read(index1);
		f->read(index2);

		int from_id = machines[index1];
		int to_id = machines[index2];

		unsigned short amp, pan;
		std::string deviceName;
		event_connection_binding binding;
		switch (type) {
			case zzub::connection_type_audio: {
				f->read(amp);
				f->read(pan);

				bool result = player->plugin_add_input(to_id, from_id, connection_type_audio);
				if (result) {
					int track = player->back.plugin_get_input_connection_count(to_id) - 1;
					player->plugin_set_parameter(to_id, 0, track, 0, amp, false, false, true);
					player->plugin_set_parameter(to_id, 0, track, 1, pan, false, false, true);
				}

				connections[to_id].push_back(std::pair<int, zzub::connection_type>(from_id, connection_type_audio) );
				break;
			}
			case zzub::connection_type_midi: {
				f->read(deviceName);

				bool result = player->plugin_add_input(to_id, from_id, connection_type_midi);
				if (result) {
					player->plugin_set_midi_connection_device(to_id, from_id, deviceName);
				}

				connections[to_id].push_back(std::pair<int, connection_type>(from_id, connection_type_midi) );
				break;
			}
			case zzub::connection_type_event: {
				unsigned short bindings = 0;
				f->read(bindings);
				bool result = player->plugin_add_input(to_id, from_id, connection_type_event);
				if (result) {
					for (int i = 0; i < bindings; i++) {
						f->read(binding.source_param_index);
						f->read(binding.target_group_index);
						f->read(binding.target_track_index);
						f->read(binding.target_param_index);
						player->plugin_add_event_connection_binding(to_id, from_id, binding.source_param_index, 
							binding.target_group_index, binding.target_track_index, binding.target_param_index);
					}
				}
				connections[to_id].push_back(std::pair<int, zzub::connection_type>(from_id, connection_type_event) );
				break;
			}
			default:
				assert(false);
				return false;
		}

	}*/
	return true;
}

bool BuzzReader::loadConnections() {
	Section* section = getSection(MAGIC_CON2);
//	if (section && loadConnections2()) return true;

	section=getSection(MAGIC_CONN);
	zzub_input_seek(f, section->offset, SEEK_SET);
	unsigned short conns = 0;
	zzub_read(f, conns);

	unsigned short index1 = 0, index2 = 0;
	for (int i = 0; i < conns; i++) {
		zzub_read(f, index1);
		zzub_read(f, index2);
		unsigned short amp, pan;
		zzub_read(f, amp);
		zzub_read(f, pan);

		bmxmachine& from_id = machines[index1];
		bmxmachine& to_id = machines[index2];

		zzub_plugin_t* from_plugin = from_id.plugin;
		zzub_plugin_t* to_plugin = to_id.plugin;

		if (from_plugin != 0 && to_plugin != 0) {
			zzub_connection_t* conn = zzub_plugin_create_audio_connection(to_plugin, from_plugin, -1, -1, -1, -1);
			if (conn != 0) {
				zzub_plugin_t* connplugin = zzub_connection_get_connection_plugin(conn);
				int conntracks = zzub_plugin_get_track_count(connplugin, 2);
				for (int j = 0; j < conntracks; j++) {
					zzub_plugin_set_parameter_value(connplugin, 2, j, 0, amp, false);
				}
			}
		}
		
		to_id.from_plugins.push_back(from_id);

	}

	return true;
}


enum patternsequence_value {
	patternsequence_type_none = 65535,
	patternsequence_type_mute = 65534,
	patternsequence_type_break = 65533,
	patternsequence_type_thru = 65532,
};


void BuzzReader::initSequences() {

	zzub_pluginloader_t* loader = zzub_player_get_pluginloader_by_name(player, "@zzub.org/sequence/pattern");
	assert(loader);
	const char* newname = zzub_player_get_new_plugin_name(player, "@zzub.org/sequence/pattern");
	patplug = zzub_player_create_plugin(player, 0, 0, newname, loader, 0);
	assert(patplug);

	seqformat = zzub_player_create_pattern_format(player, newname);

	seqpat = zzub_player_create_pattern(player, seqformat, newname, 4096 * 2);
	zzub_pattern_set_display_resolution(seqpat, 16);
	zzub_pattern_set_loop_enabled(seqpat, 1);

	zzub_player_set_order_length(player, 1);
	zzub_player_set_order_pattern(player, 0, seqpat);

	seqloader = zzub_player_get_pluginloader_by_name(player, "@zzub.org/sequence/sequence");
	assert(seqloader);
	newname = zzub_player_get_new_plugin_name(player, "@zzub.org/sequence/sequence");
	seqplug = zzub_player_create_plugin(player, 0, 0, newname, seqloader, 0);
	assert(seqplug);
}

bool BuzzReader::loadSequences() {
	Section* section=getSection(MAGIC_SEQU);
	zzub_input_seek(f, section->offset, SEEK_SET);

	unsigned int endSong, beginLoop, endLoop;
	unsigned short numSequences;

	zzub_read(f, endSong); // TODO / unused
	zzub_read(f, beginLoop);
	zzub_read(f, endLoop);
	zzub_read(f, numSequences);

	zzub_plugin_set_track_count(patplug, numSequences);
	zzub_pattern_set_loop_start(seqpat, beginLoop);
	zzub_pattern_set_loop_end(seqpat, endLoop);

	int screenidx = 0;
	int patplugid = zzub_plugin_get_id(patplug);
	for (int i = 0; i < numSequences; i++) {
		zzub_pattern_format_add_column(seqformat, patplug, 2, i, 0, screenidx);
		++screenidx;

		unsigned short machineIndex;
		zzub_read(f, machineIndex);
		
		bmxmachine plugin_id = machines[machineIndex];
		unsigned int events;
		unsigned char posSize, eventSize;
		zzub_read(f, events);
		if (events > 0) {
			zzub_read(f, posSize);
			zzub_read(f, eventSize);
		}

		if (plugin_id.plugin) {
			const char* plugname = zzub_plugin_get_name(plugin_id.plugin);
			zzub_pattern_format_set_track_name(seqformat, patplug, 2, i, plugname);
			for (size_t j = 0; j < events; j++) {
				unsigned long pos = 0, value = 0;
				zzub_input_read(f, (char*)&pos, posSize);
				zzub_input_read(f, (char*)&value, eventSize);

				if (value == 0) {
					zzub_pattern_insert_value(seqpat, patplugid, 2, i, 0, pos, patternsequence_type_mute, 0);
				} else
				if (value == 1) {
					zzub_pattern_insert_value(seqpat, patplugid, 2, i, 0, pos, patternsequence_type_break, 0);
				} else
				if (value == 2) {
					zzub_pattern_insert_value(seqpat, patplugid, 2, i, 0, pos, patternsequence_type_thru, 0);
				} else
				if (value >= 0x10) {
					// machine+value -> pattern id
					int ptnidx = value - 0x10;
					if (ptnidx >= 0 && ptnidx < (int)plugin_id.patterns.size()) {
						int ptnid = zzub_pattern_get_id(plugin_id.patterns[ptnidx]);
						zzub_pattern_insert_value(seqpat, patplugid, 2, i, 0, pos, ptnid, 0);
					}
				}
			}
		} else {
			// plugin no exists; skip events
			for (size_t j = 0; j < events; j++) {
				unsigned long pos = 0, value = 0;
				zzub_input_read(f, (char*)&pos, posSize);
				zzub_input_read(f, (char*)&value, eventSize);
			}
			
		}
	}

	return true;
}


bool BuzzReader::loadWaveTable() {
	Section* section = getSection(MAGIC_WAVT);
	if (section == 0) return true;	// no wavetable
	zzub_input_seek(f, section->offset, SEEK_SET);

	unsigned short waveCount;
	zzub_read(f, waveCount);

	for (int i = 0; i < waveCount; i++) {
		unsigned short index;

		zzub_read(f, index);
		if (index >= 200) {
			std::stringstream strm;
			strm << "Error: Invalid index " << index << " on wave " << i << "/" << waveCount << " in WAVT" << endl;
			lastError = strm.str();
			break;
		}

		zzub_wave_t* wave = zzub_player_get_wave(player, index);
		zzub_wave_clear(wave);

		vector<zzub::envelope_entry> envelopes;
		std::string fileName, name;
		float volume;
		zzub_read(f, fileName);
		zzub_read(f, name);
		zzub_read(f, volume);
		int flags = 0;
		zzub_read(f, (unsigned char&)flags);
		if ((flags & zzub_wave_flag_envelope) != 0) {
			unsigned short numEnvelopes;
			zzub_read(f, numEnvelopes);
			envelopes.resize(numEnvelopes);

			for (int j = 0; j < numEnvelopes; j++) {
				unsigned short numPoints;
				envelope_entry& env = envelopes[j];//.back();

				zzub_read(f, env.attack);	// Attack time 
				zzub_read(f, env.decay);		// Decay time
				zzub_read(f, env.sustain);	// Sustain level
				zzub_read(f, env.release);	// Release time
				zzub_read(f, env.subDivide);	// ADSR Subdivide
				zzub_read(f, env.flags);		// ADSR Flags
				zzub_read(f, numPoints);		// number of points (can be zero) (bit 15 set = envelope disabled)
				env.disabled = (numPoints&0x8000)!=0;
				numPoints &= 0x7FFF;

				env.points.resize(numPoints);
				for (int k = 0; k < numPoints; k++) {
					envelope_point& pt = env.points[k];
					zzub_read(f, pt.x);	// x
					zzub_read(f, pt.y);	// y
					zzub_read(f, pt.flags);	// flags
				}
			}
		}

		zzub_wave_set_flags(wave, flags);
		zzub_wave_set_volume(wave, volume);
		zzub_wave_set_name(wave, name.c_str());
		zzub_wave_set_path(wave, fileName.c_str());

		unsigned char waveLevels;
		zzub_read(f, waveLevels);

		// NOTE: allocate_level() resets volume to 1.0 on the first allocate_level(),
		// (which in turn, happens because the default volume is 0.0, and when machines 
		// allocate waves we need to update the volume to 1.0 without breaking anything)
		// so we need to make a copy of the loaded volume and set it again afterwards

		for (int j = 0; j < waveLevels; j++) {
			int numSamples, loopStart, loopEnd, samplesPerSec;
			unsigned char rootNote;
			zzub_read(f, numSamples);
			zzub_read(f, loopStart);
			zzub_read(f, loopEnd);
			zzub_read(f, samplesPerSec);
			zzub_read(f, rootNote);
			
			zzub_wavelevel_t* wavelevel = zzub_wave_add_level(wave);
			zzub_wavelevel_set_sample_count(wavelevel, numSamples);
			zzub_wavelevel_set_root_note(wavelevel, rootNote);
			zzub_wavelevel_set_loop_start(wavelevel, loopStart);
			zzub_wavelevel_set_loop_end(wavelevel, loopEnd);
			zzub_wavelevel_set_samples_per_second(wavelevel, samplesPerSec);
		}

		zzub_wave_set_volume(wave, volume);

		//player->wave_set_envelopes(index, wave.envelopes);

		// if this was an extended wave; we need to update the extended fields,
		// but we dont know the target format yet.
		// IF there is an extended sample, we'll get race conditions if:
		// - the song is playing when loading
		// - the machines and wavetable description is flushed
		// - a machine is playing the initialized but unloaded sampledata
		// - and we load the extended buffer directly into the levels 
		//   sampledata
		// so: 1) if we insisit on flushing, the waveloading should work on 
		//        the backbuffer (= require bunch of extra ram)
		// or: 2) dont flush at all when loading bmxes (= harder to debug: some
		//        errors appear only after everything is loaded)

	}

	//player->flush_operations(0, 0, 0);

	return true;
}

bool BuzzReader::loadWaves() {
	Section* section = getSection(MAGIC_CWAV);
	if (section == 0) section=getSection(MAGIC_WAVE);
	if (section == 0) return true;
	zzub_input_seek(f, section->offset, SEEK_SET);

	unsigned short waveCount;
	zzub_read(f, waveCount);

	for (int i = 0; i<waveCount; i++) {
		unsigned short index;
		zzub_read(f, index);
		if (index >= 200) {
			std::stringstream strm;
			strm << "Error: Invalid index " << index << " on wave " << i << "/" << waveCount << " in CWAV/WAVE" << endl;
			lastError = strm.str();
			break;
		}
		unsigned char format;
		zzub_read(f, format);

		zzub_wave_t* wave = zzub_player_get_wave(player, index);
		int waveflags = zzub_wave_get_flags(wave);
		int numchannels = (waveflags & zzub_wave_flag_stereo) != 0 ? 2 : 1;
		int numlevels = zzub_wave_get_level_count(wave);
		if (format == 0) {
			unsigned int totalBytes;
			zzub_read(f, totalBytes);
			for (int j = 0; j < numlevels; j++) {
				zzub_wavelevel_t* wavelevel = zzub_wave_get_level(wave, j);
				int numsamples = zzub_wavelevel_get_sample_count(wavelevel);
				int buffersize = numsamples * 2 * numchannels;
				char* buffer = new char[buffersize];
				zzub_input_read(f, buffer, buffersize);
				zzub_wavelevel_replace_sample_range(wavelevel, 0, buffer, numchannels, format, numsamples);
				delete[] buffer;
			}
		} else 
		if (format == 1) {
			WAVEUNPACK wup;
			InitWaveUnpack(&wup, f, section->size);

			for (int j = 0; j < numlevels; j++) {
				zzub_wavelevel_t* wavelevel = zzub_wave_get_level(wave, j);
				int numsamples = zzub_wavelevel_get_sample_count(wavelevel);
				int buffersize = numsamples * 2 * numchannels;
				char* buffer = new char[buffersize];
				
				DecompressWave(&wup, (LPWORD)buffer, numsamples, numchannels == 2?TRUE:FALSE);
				zzub_wavelevel_replace_sample_range(wavelevel, 0, buffer, numchannels, zzub_wave_buffer_type_si16, numsamples);

				delete[] buffer;
			}

			int iRemain = wup.dwCurIndex - wup.dwBytesInBuffer;
			zzub_input_seek(f, iRemain+1, SEEK_CUR);

		} else {
			std::stringstream strm;
			strm << "Error: Unknown compression format (" << format << ") on wave " << i << "/" << waveCount << " at #" << index << endl;
			lastError = strm.str();
			break;
		}

		// update non-legacy fields using the extended section
		// a potential race condition is described in the wavetable description section
		// this also doesnt work with undo when importing
		/*if (entry.get_extended()) {
			for (int j = 0; j < entry.get_levels(); j++) {		
				zzub::wave_level* level = entry.get_level(j);
				level->sample_count = entry.get_sample_count(j);
				level->loop_start = entry.get_loop_start(j);
				level->loop_end = entry.get_loop_end(j);
				level->samples = (short*)entry.get_sample_ptr(j);
				level->format = entry.get_wave_format(j);
			}
		}*/

	}
	return true;
}


// from buzz2zzub
void conformParameter(zzub::parameter& param) {
	int in = std::min(param.value_min, param.value_max);
	int ax = std::max(param.value_min, param.value_max);
	param.value_min = in;
	param.value_max = ax;

	if (param.type == zzub_parameter_type_switch) {
		param.value_min = zzub_switch_value_off;
		param.value_max = zzub_switch_value_on;
		param.value_none = zzub_switch_value_none;
	} else
	if (param.type == zzub_parameter_type_note) {
		param.value_min = zzub_note_value_min;
		param.value_max = zzub_note_value_max;
		param.value_none = zzub_note_value_none;
	}
}


bool BuzzReader::loadPara() {
	Section* section = getSection(MAGIC_PARA);
	if (!section) return true;
	zzub_input_seek(f, section->offset, SEEK_SET);

	unsigned int numMachines;
	zzub_read(f, numMachines);

	for (size_t i = 0; i < numMachines; i++) {
		unsigned int numGlobals = 0, numTrackParams = 0;
		armstrong::frontend::dummyinfo* dummy = new armstrong::frontend::dummyinfo(); // allocate here to keep out of player.plugmgr.dummy_plugins, clone into the player when needed
		zzub_read(f, dummy->instanceShortName);
		zzub_read(f, dummy->name);
		zzub_read(f, numGlobals);
		zzub_read(f, numTrackParams);

		for (size_t j = 0; j < numGlobals + numTrackParams; j++) {
			zzub::parameter cmp;
			memset(&cmp, 0, sizeof(zzub::parameter));
			zzub_read(f, (char&)cmp.type);	// undocumented
			zzub_read(f, cmp.name);
			zzub_read(f, cmp.value_min);
			zzub_read(f, cmp.value_max);
			zzub_read(f, cmp.value_none);
			zzub_read(f, cmp.flags);
			zzub_read(f, cmp.value_default);
			conformParameter(cmp);

			if (j < numGlobals)
				dummy->add_global_parameter() = cmp;
			else
				dummy->add_track_parameter() = cmp;
		}
		paraplugins.push_back(boost::shared_ptr<armstrong::frontend::dummyinfo>(dummy));
	}

	return true;
}


bool BuzzReader::loadMidi() {
/*	Section* section=getSection(MAGIC_MIDI);
	if (section==0) return true;
	f->seek(section->offset, SEEK_SET);
	
	for (;;) {
		string name;
		f->read(name);
		if (name=="") break;
		plugin_descriptor mmdesc = player->front.get_plugin_descriptor(name);
		char g, t, c, mc, mn;
		f->read(g);
		f->read(t);
		f->read(c);
		f->read(mc);
		f->read(mn);
		if (mmdesc == -1) continue;
		int plugin_id = player->front.get_plugin(mmdesc).descriptor;
		player->add_midimapping(plugin_id, g, t, c, mc, mn);
	}
	player->flush_operations(0, 0, 0);
*/
	return true;
}

bool BuzzReader::loadInfoText() {
/*	Section* section = getSection(MAGIC_BLAH);
	if (!section)
		return true;
	f->seek(section->offset, SEEK_SET);
	
	unsigned int textlength;
	f->read(textlength);
	
	if (!textlength)
		return true;
	char *text = new char[textlength+1];
	text[textlength] = '\0';
	f->read(text, sizeof(char)*textlength);
	player->front.song_comment = text;
	delete[] text;*/
	return true;
}

}
