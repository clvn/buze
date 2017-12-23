/*
	This file is part of the Buzé base Buzz-library. 
	
	Please refer to LICENSE.TXT for details regarding usage.
*/

#pragma once

struct PresetInfo {
	std::string name;
	std::string comment;
	unsigned int tracks;
	unsigned int parameters;
	int values[1024];
	std::vector<char> savedata;

	PresetInfo() {
		parameters = 0;
	}
};

class PresetManager {
	std::vector<PresetInfo> presets;
public:
	std::string machineName;
	std::string presetFile;

	PresetManager();
	bool load(std::string fileName);
	bool save(std::string fileName);

	void addPreset(std::string name, int* values);

	size_t getPresetCount();
	PresetInfo& getPreset(size_t index);
	int findPreset(std::string name);
	void add(PresetInfo pi);
	void remove(size_t index);
};


