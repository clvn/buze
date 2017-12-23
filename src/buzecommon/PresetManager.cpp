/*
	This file is part of the Buzé base Buzz-library. 
	
	Please refer to LICENSE.TXT for details regarding usage.
*/

#include <windows.h>
#include <string>
#include <vector>
#include <cassert>
#include "presetmanager.h"
#include "FileReader.h"

using namespace std;

PresetManager::PresetManager() {
}

size_t PresetManager::getPresetCount() {
	return presets.size();
}

PresetInfo& PresetManager::getPreset(size_t index) {
	assert(index < presets.size());
	return presets[index];
}

int PresetManager::findPreset(std::string name) {
	for (int i = 0; i < presets.size(); i++)
		if (presets[i].name == name)
			return i;
	return -1;
}

bool PresetManager::save(std::string fileName) {
	FileWriter writer;
	if (!writer.create(fileName.c_str())) return false;

	// NOTE: version 2 presets are buze-only. 
	// buzz only supports version 1 and will refuse to load presets saved in buze.
	unsigned int version = 2;
	writer.write(version);

	writer.write((unsigned int)machineName.length());
	writer.writeString(machineName);    

	writer.write((unsigned int)presets.size());

	for (size_t i=0; i<presets.size(); i++) {
		PresetInfo& preset = presets[i];
		writer.write((unsigned int)preset.name.size());
		writer.writeString(preset.name);

		writer.write(preset.tracks);
		writer.write(preset.parameters);
		for (int j = 0; j<preset.parameters; j++) {
			writer.write(preset.values[j]);
		}

		writer.write((unsigned int)preset.comment.size());
		writer.writeString(preset.comment);

		if (version > 1) {
			unsigned int size = preset.savedata.size();
			writer.write(size);
			if (size > 0) writer.writeBytes(&preset.savedata[0], size);
		}
	}

	writer.close();

	presetFile = fileName;
	return true;
}

bool PresetManager::load(std::string fileName) {
	presets.clear();
	FileReader reader;
	if (!reader.open(fileName.c_str())) return false;

	unsigned int version, len;
	reader.read(&version);
	if (!(version == 1 || version == 2)) {
		reader.close();
		return false;
	}

	reader.read(&len);
	char* machineName=new char[len+1];
	reader.readBytes(machineName, len);
	machineName[len]=0;
	this->machineName = machineName;

	delete[] machineName;

	unsigned int presetCount;
	reader.read(&presetCount);	// parameters*4	?? antall presets?

	for (size_t j=0; j<presetCount; j++) {
		reader.read(&len);

		char* presetName=new char[len+1];
		reader.readBytes(presetName, len);
		presetName[len]=0;

		PresetInfo preset;
		memset(&preset, 0, sizeof(PresetInfo));
		preset.name=presetName;
		delete[] presetName;

		reader.read(&preset.tracks);
		reader.read(&preset.parameters);
		if (preset.parameters>=1024) {
#if defined(_WIN32)
			MessageBox(0, "Unexpected many preset parameters. Will now begin trashing memory", "Alert alert!", MB_OK);
#else
			printf("Unexpected many preset parameters. Will now begin trashing memory\n");
#endif
		}

		int value;
		for (size_t i=0; i<preset.parameters; i++) {
			if (!reader.read(&value)) {
				reader.close();
				return false;
			}
			if (i<1024)
				preset.values[i]=value;
		}

		// read preset comment
		reader.read(&len);
		
		presetName=new char[len+1];
		reader.readBytes(presetName, len);
		presetName[len]=0;

		preset.comment = presetName;
		delete[] presetName;

		if (version > 1) {
			// read machine data
			unsigned int size;
			reader.read(&size);

			preset.savedata.resize(size);
			if (size > 0) reader.readBytes(&preset.savedata[0], size);
		}

		presets.push_back(preset);

	}

	reader.close();

	presetFile = fileName;

	return true;
}

void PresetManager::add(PresetInfo pi) {
	presets.push_back(pi);
}

void PresetManager::remove(size_t index) {
	presets.erase(presets.begin() + index);
}



