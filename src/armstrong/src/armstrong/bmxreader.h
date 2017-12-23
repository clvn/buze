/*
Copyright (C) 2003-2007 Anders Ervik <calvin@countzero.no>

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

#include <boost/shared_ptr.hpp>
#include "dummy.h"

namespace zzub {

const unsigned int MAGIC_Buzz = 0x7a7a7542;
const unsigned int MAGIC_MACH = 0x4843414d;
const unsigned int MAGIC_CONN = 0x4E4E4F43;
const unsigned int MAGIC_PATT = 0x54544150;
const unsigned int MAGIC_SEQU = 0x55514553;
const unsigned int MAGIC_WAVT = 0x54564157;
const unsigned int MAGIC_CWAV = 0x56415743;
const unsigned int MAGIC_WAVE = 0x45564157;
const unsigned int MAGIC_BLAH = 0x48414c42;
const unsigned int MAGIC_PARA = 0x41524150;
const unsigned int MAGIC_MIDI = 0x4944494D;
const unsigned int MAGIC_CON2 = 0x324E4F43;


struct Section {
	unsigned int magic;
	unsigned int offset;
	unsigned int size;
};

struct bmxmachine {
	zzub_pluginloader_t* loader;
	zzub_plugin_t* plugin;
	armstrong::frontend::dummyinfo* parainfo;
	std::vector<zzub_pattern_t*> patterns;
	std::vector<bmxmachine> from_plugins;
};

class BuzzReader {

	zzub_input_t* f;
	unsigned int sectionCount;
	Section* sections;
	zzub_player_t* player;
	zzub_plugin_t* patplug;
	zzub_pattern_format_t* seqformat;
	zzub_pattern_t* seqpat;
	zzub_pluginloader_t* seqloader;
	zzub_plugin_t* seqplug;
	zzub_plugin_t* masterplug;
	zzub_pluginloader_t* masterloader;
	std::vector<boost::shared_ptr<armstrong::frontend::dummyinfo> > paraplugins;
	std::vector<armstrong::frontend::validationerror> load_errors;

	Section* getSection(unsigned int magic);

	bool loadMachines();
	bool loadPatterns();
	bool loadConnections();
	bool loadConnections2();
	void initSequences();
	bool loadSequences();
	bool loadWaveTable();
	bool loadWaves();
	bool loadPara();
	bool loadMidi();
	bool loadInfoText();

	void clear();

	bool open(zzub_input_t* inf);
	//zzub_pluginloader_t* create_dummy_info(int flags, std::string uri, int tracks_min, int tracks_max, int attributecount, armstrong::frontend::validatorplugin* parainfo);
	zzub_plugin_t* create_and_test_plugin(zzub_input_t* f, int type, int dataSize, std::string machineName, std::string origMachineName, zzub_pluginloader_t* loader);
	zzub_plugin_t* create_dummy_plugin(zzub_input_t* f, int type, int dataSize, std::string machineName, std::string origMachineName, std::string fullName, std::string pluginUri);
	
	// zzub wrapper helpers that remaps master/sequence columns
	zzub_pattern_format_t* create_pattern_format_from_plugin(zzub_player_t* player, zzub_plugin_t* plugin);
	zzub_parameter_t* get_parameter(zzub_pluginloader_t* loader, int group, int column);
	int get_parameter_count(zzub_pluginloader_t* loader, int group);
	zzub_parameter_t* get_parameter(zzub_plugin_t* plugin, int group, int track, int column);
	int get_parameter_count(zzub_plugin_t* plugin, int group, int track);
	void set_parameter_value(zzub_plugin_t* plugin, int group, int track, int column, int value);
	void insert_pattern_value(zzub_pattern_t* pattern, zzub_plugin_t* plugin, int group, int track, int column, int row, int value);
	void add_format_column(zzub_pattern_format_t* format, zzub_plugin_t* plugin, int group, int track, int column, int index);
	armstrong::frontend::dummyinfo* find_dummy(std::string name, std::string typeName);
public:

	std::vector<bmxmachine> machines;
	std::string lastError;
	std::string lastWarning;
	
	bool ignoreWaves;
	bool ignorePatterns;
	bool ignoreSequences;
	float offsetX, offsetY;

	BuzzReader(zzub_input_t* inf);
	~BuzzReader();

	bool read_track(bmxmachine& plugin, zzub_pattern_t* result, int group, int track, int rows);

	bool readPlayer(zzub_player_t* pl);
};

}
