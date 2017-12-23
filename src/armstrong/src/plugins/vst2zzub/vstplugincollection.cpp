#define NOMINMAX
#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
};

typedef struct zzub::mixer zzub_mixer_t;

#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <map>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <zzub/plugin.h>
#include "vst2zzub.h"
#include "pluginterfaces/vst2.x/aeffectx.h"
#include "editor.h"
#include "mixing/mixer.h"
#include "../plugincache.h"
#include "vstplugincollection.h"
#include "vstplugin.h"
#include "../stringutil.h"

using std::cerr;
using std::endl;
using std::cout;
using std::string;

//plugincache<vstplugincollection, vstplugininfo> vstplugincollection::cachedplugins("vst2zzub", "8");
std::map<std::string, vstplugincache*> vstplugincollection::plugincaches;

typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);
static VstIntPtr VSTCALLBACK HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

std::string opcode_to_string(VstInt32 opcode) {
	switch(opcode) {
		case audioMasterVersion :
			return "audioMasterVersion";
		case audioMasterAutomate:
			return "audioMasterAutomate" ;
		case audioMasterCurrentId:
			return "audioMasterCurrentId";
		case audioMasterIdle:
			return "audioMasterIdle";
		case audioMasterPinConnected:
			return "audioMasterPinConnected" ;
		case audioMasterWantMidi:
			return "audioMasterWantMidi" ;
		case audioMasterGetTime:
			return "audioMasterGetTime";
		case audioMasterProcessEvents:
			return "audioMasterProcessEvents" ;
		case audioMasterTempoAt:
			return "audioMasterTempoAt" ;
		case audioMasterGetNumAutomatableParameters:
			return "audioMasterGetNumAutomatableParameters" ;
		case audioMasterGetParameterQuantization:
			return "audioMasterGetParameterQuantization" ;
		case audioMasterIOChanged:
			return "audioMasterIOChanged" ;
		case audioMasterNeedIdle:
			return "audioMasterNeedIdle" ;
		case audioMasterSizeWindow:
			return "audioMasterSizeWindow" ;
		case audioMasterGetSampleRate:
			return "audioMasterGetSampleRate" ;
		case audioMasterGetBlockSize:
			return "audioMasterGetBlockSize" ;
		case audioMasterGetInputLatency:
			return "audioMasterGetInputLatency" ;
		case audioMasterGetOutputLatency:
			return "audioMasterGetOutputLatency" ;
		case audioMasterGetPreviousPlug:
			return "audioMasterGetPreviousPlug" ;
		case audioMasterGetNextPlug:
			return "audioMasterGetNextPlug" ;
		case audioMasterWillReplaceOrAccumulate:
			return "audioMasterWillReplaceOrAccumulate" ;
		case audioMasterGetCurrentProcessLevel:
			return "audioMasterGetCurrentProcessLevel" ;
		case audioMasterGetAutomationState:
			return "audioMasterGetAutomationState" ;
		case audioMasterGetVendorString:
			return "audioMasterGetVendorString" ;
		case audioMasterGetProductString:
			return "audioMasterGetProductString" ;
		case audioMasterGetVendorVersion:
			return "audioMasterGetVendorVersion" ;
		case audioMasterVendorSpecific:
			return "audioMasterVendorSpecific" ;
		case audioMasterSetIcon:
			return "audioMasterSetIcon" ;
		case audioMasterCanDo:
			return "audioMasterCanDo" ;
		case audioMasterGetLanguage:
			return "audioMasterGetLanguage" ;
		case audioMasterGetDirectory:
			return "audioMasterGetDirectory" ;
		case audioMasterUpdateDisplay:
			return "audioMasterUpdateDisplay" ;
	}
	return "";
}


VstIntPtr VSTCALLBACK HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
/*
	ONLY the following audioMaster callbacks are allowed before the initial DLL-call "main()" or "VstPluginMain()" returns to host:
	{01} audioMasterVersion
	{06} audioMasterWantMidi ... deprecated in VST2.4
	{32} audioMasterGetVendorString
	{33} audioMasterGetProductString
	{34} audioMasterGetVendorVersion
	{38} audioMasterGetLanguage
	{41} audioMasterGetDirectory
*/
	VstIntPtr result = 0;
	switch(opcode) {
		case audioMasterVersion:
			return kVstVersion;
		case audioMasterNeedIdle:
			cout << "ignoring audioMasterNeedIdle" << endl;
			return 0;
	}

	std::map<AEffect*, vstplugin*>::iterator i = vstplugin::plugins.find(effect);
	if (i == vstplugin::plugins.end()) {
		cout << "Ignoring HostCallback " << opcode << ": " << opcode_to_string(opcode) << endl;
		return 0;
	}

	return i->second->host_callback(opcode, index, value, ptr, opt);
}

//
// vstplugininfo
//

vstplugininfo::vstplugininfo(zzub::plugincollection* coll, std::string _path) {
	collection = coll;
	pluginfile = _path;
}

void vstplugininfo::load_program_names(AEffect* effect) {
	programNames.clear();
	char name[1024]; // kVstMaxProgNameLen = stack corruption
	for (int i = 0; i < effect->numPrograms; i++) {
		memset(name, 0, sizeof(name));
		effect->dispatcher(effect, effGetProgramNameIndexed, i, 0, name, 0);//setProgram(i);
		programNames.push_back(name);
	}
}

void vstplugininfo::load_parameter_properties(AEffect* effect) {
	// VstParameterProperties can be used to determine more userfriendly min-max
	for (int i = 0; i < effect->numParams; i++) {
		VstParameterProperties vstParam;
		memset(&vstParam, 0, sizeof(vstParam));
		bool hasVstParam = effect->dispatcher(effect, effGetParameterProperties, i, 0, &vstParam, 0) == 1;

		if (!hasVstParam) vstParam.flags = 0;
		parameterProperties.push_back(vstParam);
	}
}

AEffect* vstplugininfo::create_vst_effect(HMODULE* hPluginResult) {

	HMODULE hPlugin = LoadLibrary(pluginfile.c_str());
	if (!hPlugin) {
		int error = GetLastError();
		cerr << "vst2zzub: cannot open " << pluginfile << " (" << error << ")" << endl;
		return 0;
	}

	PluginEntryProc mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)hPlugin, "VSTPluginMain");
	if (!mainProc) 
		mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)hPlugin, "main");

	if (!mainProc) {
		cerr << "vst2zzub: cannot find entry point in " << pluginfile << endl;
		FreeLibrary(hPlugin);
		return 0;
	}

	cout << "vst2zzub: found " << pluginfile << endl;
	AEffect* effect = mainProc(HostCallback);
	if(!effect) {
		cout << "vst2zzub: cannot create vst instance" << endl;
		// TODO: unregister from cache
		FreeLibrary(hPlugin);
		return 0;
	}

	if (programNames.empty())
		load_program_names(effect);
	if (parameterProperties.empty())
		load_parameter_properties(effect);

	if (hPluginResult)
		*hPluginResult = hPlugin;
	return effect;
}

bool vstplugininfo::check_can_do(int canDo, bool defaultValue) {
	// effCanDo x[return]: 1='cando', 0='don't know' (default), -1='No'
	if (canDo == 1) return true;
	if (canDo == 0 && defaultValue) return true;
	return false;
}

// fills in plugin info based on a vst plugin instance
void vstplugininfo::fill_plugin_info(AEffect* effect) {

	size_t ls = pluginfile.find_last_of("/\\");
	std::string filename = pluginfile.substr(ls + 1);
	ls = filename.find_last_of(".");
	filename = filename.substr(0, ls);

	flags |= zzub_plugin_flag_has_midi_input;

	if (effect->numOutputs > 0)
		flags |= zzub_plugin_flag_has_audio_output;

	if (effect->numInputs > 0)
		flags |= zzub_plugin_flag_has_audio_input;

	bool can_vst_midi_events = check_can_do(effect->dispatcher(effect, effCanDo, 0, 0, "sendVstMidiEvents", 0));
	bool can_vst_events = check_can_do(effect->dispatcher(effect, effCanDo, 0, 0, "sendVstEvents", 0));

	if (can_vst_midi_events || can_vst_events)
		flags |= zzub_plugin_flag_has_midi_output;

	char effectName[256] = {0};
	char vendorString[256] = {0};
	char productString[256] = {0};

	effect->dispatcher (effect, effGetEffectName, 0, 0, effectName, 0);
	effect->dispatcher (effect, effGetVendorString, 0, 0, vendorString, 0);
	effect->dispatcher (effect, effGetProductString, 0, 0, productString, 0);
	if (strlen(productString) > 0)
		name = productString; else
		name = filename;

	author = vendorString;
	short_name = filename;
	std::stringstream uristrm;
	uristrm << "@zzub.org/vst/" << std::hex << std::setw(8) << std::setfill('0') << effect->uniqueID;
	uri = uristrm.str();

	outputs = effect->numOutputs;
	inputs = effect->numInputs;

	for(VstInt32 paramIndex = 0; paramIndex < effect->numParams; paramIndex++)
	{
		// some synthmaker synths crash in effClose unless the param name buffer is valid throughout the plugins lifetime
		static char paramName[256] = {0};
		strcpy(paramName, "");

		effect->dispatcher (effect, effGetParamName, paramIndex, 0, paramName, 0);
		//effect->dispatcher (effect, effGetParamDisplay, paramIndex, 0, paramDisplay, 0);

		float value = effect->getParameter (effect, paramIndex);
		value = std::max(std::min(value, 1.0f), 0.0f); // clamping to 0-1 cause am freehand returns 32 for a bunch of params

		add_global_parameter()
			.set_byte()
			.set_name(paramName)
			.set_description(paramName)
			.set_value_min(0)
			.set_value_max(127)
			.set_value_default((int)(value * 127.0f))
			.set_value_none(255)
			.set_state_flag()
		;

		//printf ("Param %03d: %s [%s %s] (normalized = %f)\n", paramIndex, paramName, paramDisplay, paramLabel, value);
	}

}

// creates a vst plugin instance, fills in plugin info and destroys the vst plugin instance
bool vstplugininfo::fill_plugin_info() {
	HMODULE hPlugin;
	AEffect* effect = create_vst_effect(&hPlugin);
	if (!effect) {
		return false;
	}
	
	// zynaddsubfx needs effOpen before probing parameters
	effect->dispatcher(effect, effOpen, 0, 0, 0, 0);

	fill_plugin_info(effect);

 	effect->dispatcher (effect, effClose, 0, 0, 0, 0);
	FreeLibrary(hPlugin);
	return true;
}

zzub::plugin* vstplugininfo::create_plugin() {
	return new vstplugin(this);
}

bool vstplugininfo::store_info(zzub::archive *) const {
	return false;
}

//
// vstplugincollection
//

vstplugincache* vstplugincollection::get_plugincache(const std::string& path) {
	std::map<std::string, vstplugincache*>::iterator i = plugincaches.find(path);
	if (i != plugincaches.end()) return i->second;

	vstplugincache* newcache = new vstplugincache("vst2zzub", "9");
	plugincaches[path] = newcache;
	return newcache;
}

void vstplugincollection::initialize(zzub::pluginfactory * factory) {
	if (!factory) return;

	set_vst_paths();

	boost::filesystem::path cachepath = boost::filesystem::path(userpath) / boost::filesystem::path("PluginCache\\");
	boost::filesystem::create_directories(cachepath);

	for (std::vector<std::string>::iterator i = vstpaths.begin(); i != vstpaths.end(); ++i) {
		const std::string& vstpath = *i;
		std::cout << "vst2zzub: enumerating vst plugins in: " << vstpath << "\n";

		vstplugincache* cache = get_plugincache(vstpath);
		cache->cached_info.clear();
		cache->registered_infos.clear();
		cache->enumerate_directory_with_cache(this, factory, vstpath, cachepath.string(), true);
	}
}

const char* vstplugincollection::get_uri() {
	return "@zzub.org/vst";
}

const char* vstplugincollection::get_name() {
	return "VST";
}

void vstplugincollection::configure(const char *key, const char *value) {
	std::string ck = key;
	if (ck == "vstpaths") {
		std::string path = value;
		// split semicolonseparated string into separate paths
		split(value, vstpaths, ";");
	} else if (ck == "hostpath" && value)
		hostpath = value;
	else if (ck == "userpath" && value)
		userpath = value;
}

void vstplugincollection::destroy() {
	delete this;
}

void vstplugincollection::set_vst_paths() {
	if (vstpaths.empty()) {
		boost::filesystem::path path = boost::filesystem::path(hostpath) / "Gear\\Vst\\";
		vstpaths.push_back(path.string());
	}

	for (std::vector<std::string>::iterator i = vstpaths.begin(); i != vstpaths.end(); ++i) {
		std::string vstpath = trim(*i);
		if (vstpath.find_last_of("/\\") != vstpath.length() - 1) vstpath += "\\";
		*i = vstpath;
	}
}

void vstplugincollection::create_plugin_infos_from_file(const std::string& fullpath, std::vector<vstplugininfo*>* infos) {
	vstplugininfo* info = new vstplugininfo(this, fullpath);
	infos->push_back(info);
}

void vstplugincollection::init_plugin_infos(std::vector<vstplugininfo*>& infos, bool from_cache) {
	assert(infos.size() == 1);

	vstplugininfo* info = infos[0];
	info->commands = "/Programs";
}

bool vstplugincollection::fill_plugin_infos(std::vector<vstplugininfo*>& infos) {
	vstplugininfo* info = infos[0];

	if (!info->fill_plugin_info()) {
		cerr << "vst2zzub: cannot create vst instance" << endl;
		return false;
	}

	return true;
}

void vstplugincollection::unregister_plugin_infos(std::vector<vstplugininfo*>& info) {
	// no-op, vst2zzub does not track its own plugins (does it??)
}
