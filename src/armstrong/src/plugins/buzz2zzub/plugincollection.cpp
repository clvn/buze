#define NOMINMAX
#define _USE_MATH_DEFINES
#include <windows.h>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include "inifile.h"
#include "buzz2zzub.h"
#include "plugincollection.h"
#include "unhack.h"
#include "MemoryModule.h"

using std::cout;
using std::cerr;
using std::endl;

void convertParameter(int group, zzub::parameter& param, const CMachineParameter* p) {
	param.type = (zzub_parameter_type)p->Type;
	if (p->Name)
		param.name = p->Name;
	if (p->Description)
		param.description = p->Description;
	param.value_min = p->MinValue;
	param.value_max = p->MaxValue;
	param.value_none = p->NoValue;
	param.value_default = p->DefValue;
	param.flags = p->Flags;

	if (group == 2 && p->Description)
		if ((strstr(p->Description, "elocity") || strstr(p->Description, "olume")))
			param.set_velocity_index_flag();
}

void convertAttribute(zzub::attribute& attr, const CMachineAttribute* a) {
	attr.name = a->Name;
	attr.value_min = a->MinValue;
	attr.value_max = a->MaxValue;
	attr.value_default = a->DefValue;
}

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

	if (param.type != zzub_parameter_type_note) {
		param.value_default = std::max(std::min(param.value_default, param.value_max), param.value_min);
	}
}

int scan_bytes(int offset, unsigned char* data, int datasize, int* pattern, int patternsize, int maxscan) {

	for (int i = offset; i < datasize - patternsize; i++) {

		if (maxscan != -1 && (i - offset) > maxscan)
			break;

		unsigned char* testdata = data + i;
		bool found = true;
		for (int j = 0; j < patternsize; j++) {
			if (pattern[j] != -1 && pattern[j] != testdata[j]) {
				found = false;
				break;
			}
		}
		if (found) 
			return i;
	}
	return -1;
}

namespace buzz2zzub {

short oscTables[8][OSCTABSIZE];

double square(double v) {
		double sqmod=fmod(v, 2.0f*M_PI);
		return sqmod<M_PI?-1:1;
	}

double sawtooth(double v) {
	return (fmod(v, 2.0f*M_PI) / M_PI)-1;
}

double triangle(double v) {
	double sqmod=fmod(v, 2.0f*M_PI);

	if (sqmod<M_PI) {
		return sqmod/M_PI;
	} else
		return (M_PI-(sqmod-M_PI)) / M_PI;
}

void generate_oscillator_tables() {
	int tabSize = 2048;
	srand(static_cast<unsigned int>(time(0)));
	for (int tabLevel = 0; tabLevel < 11; tabLevel++) {
		int tabOfs = GetOscTblOffset(tabLevel);
		for (int i = 0; i < tabSize; i++) {
			double dx = (double)i/tabSize;
			oscTables[OWF_SINE][tabOfs+i] = (short)(sin(dx*2.0f*M_PI)*32000);
			oscTables[OWF_SAWTOOTH][tabOfs+i] = (short)(sawtooth(dx*2.0f*M_PI)*32000);
			oscTables[OWF_PULSE][tabOfs+i] = (short)(square(dx*2.0f*M_PI)*32000);
			oscTables[OWF_TRIANGLE][tabOfs+i] = (short)(triangle(dx*2.0f*M_PI)*32000);
			oscTables[OWF_NOISE][tabOfs+i] = (short) (((float)rand()/(float)RAND_MAX)*64000.f - 32000);
			oscTables[OWF_303_SAWTOOTH][tabOfs+i] = (short)(sawtooth(dx*2.0f*M_PI)*32000);
			oscTables[6][tabOfs+i] = (short)(sin(dx*2.0f*M_PI)*32000);
		}
		tabSize/=2;
	}
}

buzzplugincollection::buzzplugincollection() 
	: effects("buzz2zzub", "8")
	, generators("buzz2zzub", "8")
{
	DSP_Init(44100);

#if defined(_WIN32)
	if (!CMachine::checkBuzzCompatibility()) {
		cerr << "WARNING: The CMachine structure defined in buzz2zzub.dll is not binary compatible with Jeskola Buzz." << endl;
	}
#endif

	buzz2zzub::generate_oscillator_tables();

}

buzzplugincollection::~buzzplugincollection() {
	std::vector<buzzplugininfo *>::iterator i;
	for (i = buzzplugins.begin(); i != buzzplugins.end(); ++i)
	{
		(*i)->unload_buzz_dll();
		delete (*i)->plugin_lib;	// buzzplugininfo has no destructor so we free this here
		delete *i;
	}
	buzzplugins.clear();
}

void buzzplugincollection::configure(const char *key, const char *value) {
	std::string ck = key;
	// TODO: support semicolon delimited buzzpaths key
	if (ck == "hostpath" && value)
		hostpath = value;
	else if (ck == "userpath" && value)
		userpath = value;
}

void buzzplugincollection::initialize(zzub::pluginfactory *factory) {
	load_config();

	boost::filesystem::path fxpath = boost::filesystem::path(hostpath) / boost::filesystem::path("Gear\\Effects\\");
	boost::filesystem::path genpath = boost::filesystem::path(hostpath) / boost::filesystem::path("Gear\\Generators\\");
	boost::filesystem::path cachepath = boost::filesystem::path(userpath) / boost::filesystem::path("PluginCache\\");
	boost::filesystem::create_directories(cachepath);

	effects.enumerate_directory_with_cache(this, factory, fxpath.string(), cachepath.string(), false);
	generators.enumerate_directory_with_cache(this, factory, genpath.string(), cachepath.string(), false);
	load_exe_plugins(factory);
}

void buzzplugincollection::load_config() {
	boost::filesystem::path configpath = boost::filesystem::path(hostpath) / boost::filesystem::path("buzz2zzub.ini");
	ini::file inifile(configpath.string());
	int patchCount = inifile.section("Patches").get("Count", 0);
	for (int i = 0; i < patchCount; i++) {
		std::stringstream patchName;
		patchName << "Patch" << i;
		std::string patchString = inifile.section("Patches").get<std::string>(patchName.str(), ""); 
		if (patchString.empty()) continue;
		size_t fe = patchString.find_first_of(':');
		if (fe == std::string::npos) continue;
		std::string dllName = patchString.substr(0, fe);
		std::string patchCommand = patchString.substr(fe+1);
		unhack::enablePatch(dllName, patchCommand);
		cout << "Read patch from ini: " << dllName << " -> '" << patchCommand << "'" << endl;
	}
}

//
// buzz.exe plugins
//

bool buzzplugincollection::prepatch_buzzexe(unsigned char* data, int size) {
	// scan for a few instructions ahead of the place we're supposed to patch
	int winmain_unique_bytes[] = {
		0x3C, 0x20,            // cmp         al,20h 
		0x77, 0x49,            // ja          004B21D5 
		0x3A, 0xC3,            // cmp         al,bl 
		0x74, 0x05,            // je          004B2195 
		0x39, 0x5D, 0xE4,      // cmp         dword ptr [ebp-1Ch],ebx 
		0x75, 0x40,            // jne         004B21D5 
		0x8A, 0x06,            // mov         al,byte ptr [esi] 
		0x3A, 0xC3,            // cmp         al,bl 
		0x74, 0x0A,            // je          004B21A5 
		0x3C, 0x20,            // cmp         al,20h 
		0x77, 0x06,            // ja          004B21A5 
		0x46,                  // inc         esi  
		0x89, 0x75, 0xE0,      // mov         dword ptr [ebp-20h],esi 
		0xEB, 0xF0,            // jmp         004B2195
	};
	int winmain_unique_bytes_size = sizeof(winmain_unique_bytes) / sizeof(int);

	int off = scan_bytes(0, data, size, winmain_unique_bytes, winmain_unique_bytes_size, -1);
	if (off == -1) return false;

	off += winmain_unique_bytes_size; // off was 0xb15a5 on buzz1385

	// change __tmainCRTStartup to behave more like dllmain - need this to run the global/static constructors
	// insert 0x93 nops to see the range of instructions we're working on and which bytes are patched by the loader in the debugger
	memset(data + off, 0x90, 0x93);

	// insert two consecutive jumps, skip a total of 0x93 bytes which does the winmain/exit stuff
	const int midjump = 0x3c;
	data[off + 0] = 0xeb; // jump +n
	data[off + 1] = midjump; // 0x93 -2 bytes

	data[off + 2 + midjump] = 0xeb;
	data[off + 3 + midjump] = 0x93 - midjump - 2 - 2; // 0x93 -2 bytes
	return true;
}

int buzzplugincollection::scan_buzzexe_plugins_start(unsigned char* codeBase, int size) {

	int enumplugin_unique_bytes[] = {
		0x55,					// push ebp
		0x8b, 0xec,				// mov ebp, esp
		0x83, 0xec, 0x10,		// sub     esp,0x10
		0xa1, -1, -1, -1, -1,	// mov     eax,[image00400000+0xc158c (004c158c)]
        0x56,					// push    esi
		0x57,					// push    edi
		0x50,					// push    eax
		0x68, -1, -1, -1, -1,	// push    0x4ca618
		0xe8, -1, -1, -1, -1,	// call    image00400000+0x5e60 (00405e60)
		0x8b, 0x0d, -1, -1, -1, -1,		// mov     ecx,[image00400000+0xc158c (004c158c)]
		0xbe, 0x01, 0x00, 0x00, 0x00,	// mov     esi,0x1
		0x83, 0xc4, 0x08,		// add     esp,0x8
		0x8d, 0x45, 0xf0,		// lea     eax,[ebp-0x10]
		0x89, 0x4d, 0xf0,		// mov     [ebp-0x10],ecx
		0x89, 0x75, 0xf4,		// mov     [ebp-0xc],esi
		0xc7, 0x45, 0xf8		// this matches the first tree bytes of the next instruction, movs the getinfo/createmachine offsets
		//0043c4af c745f830d44300   mov     dword ptr [ebp-0x8],0x43d430
		//0043c4b6 c745fc40d44300   mov     dword ptr [ebp-0x4],0x43d440
	};

	int enumplugin_unique_bytes_size = sizeof(enumplugin_unique_bytes) / sizeof(int);

	int off = scan_bytes(0, codeBase, size, enumplugin_unique_bytes, enumplugin_unique_bytes_size, -1);
	if (off == -1) return -1;
	return off + enumplugin_unique_bytes_size;
}

int buzzplugincollection::scan_buzzexe_plugins_next(unsigned char* codeBase, int size, int codeofs) {

	int nextplugin_unique_bytes[] = {
		0x89, -1, 0xf4,		// mov     [ebp-0xc],esi(edi)
		0xc7, 0x45, 0xf8		// this matches the first tree bytes of the next instruction, movs the getinfo/createmachine offsets
	};
	int nextplugin_unique_bytes_size = sizeof(nextplugin_unique_bytes) / sizeof(int);
	
	// scan for next plugin - theres a variable number of instructions between each plugin
	int off = scan_bytes(codeofs, codeBase, size, nextplugin_unique_bytes, nextplugin_unique_bytes_size, 100);
	if (off == -1) return -1;
	return off + nextplugin_unique_bytes_size;
}


void buzzplugincollection::load_exe_plugins(zzub::pluginfactory* factory) {
	FILE* fp = fopen("buzz.exe", "rb");
	if (fp == 0) return ;

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	unsigned char* data = new unsigned char[size];
	if (!data) return ;

	fseek(fp, 0, SEEK_SET);
	fread(data, 1, size, fp);
	fclose(fp);

	// two kinds of processing: 
	//  1. pre-patching: turn winmain into dllmain - fixed from entrypoint?
	//  2. post-scanning: search for and parse the plugin enumeration function and extract getinfo/createmachine offsets

	// note that at least jeskola qsamo uses declspec(thread), which, when compiled as an exe, 
	// uses data from the first TLS index of fs:[0x2c]. this has to be initialized by the host by 
	// declaring a single variable with declspec(thread) reserving a few bytes - specifics unknown.
	// if the host itself uses declspec(thread), this will probably not work.

	if (!prepatch_buzzexe(data, size)) {
		delete[] data;
		return ;
	}

	cout << "buzz2zzub: buzz.exe found - scanning for builtin plugins" << endl;
	HMEMORYMODULE hModule = MemoryLoadLibrary(data, "buzz.exe", 0, 0, 1);
	delete[] data;

	if (!hModule) return ;

	int codeofs = scan_buzzexe_plugins_start(hModule->codeBase, size);

	while (codeofs != -1) {

		GET_INFO GetInfo = *(GET_INFO*)(hModule->codeBase + codeofs);
		const CMachineInfo* mi = GetInfo();

		std::string name = mi->Name;

		buzzplugininfo *i = new buzzplugininfo(this, "");
		i->machines = &machines;
		i->basefilename = name;
		
		i->uri = "@zzub.org/buzz2zzub/" + name;
		replace(i->uri.begin(), i->uri.end(), ' ', '+');

		cout << "buzz2zzub: adding plugin from buzz.exe " << name << endl;
		i->fill_machineinfo(mi);
		i->fill_machineinfo_parameters(mi);

		i->hDllInstance = 0;
		i->exePlugin = true;
		i->GetInfo = GetInfo;
		i->CreateMachine = *(CREATE_MACHINE*)(hModule->codeBase + codeofs + 7);
		buzzplugins.push_back(i);
		factory->register_info(i);

		codeofs = scan_buzzexe_plugins_next(hModule->codeBase, size, codeofs + 8 + 4);
	}
}

// plugincache callbacks

void buzzplugincollection::create_plugin_infos_from_file(const std::string& fullpath, std::vector<buzzplugininfo*>* infos) {
	buzzplugininfo* pluginfo = new buzzplugininfo(this, fullpath);
	infos->push_back(pluginfo);
}

void buzzplugincollection::init_plugin_infos(std::vector<buzzplugininfo*>& infos, bool from_cache) {

	assert(infos.size() == 1);
	buzzplugininfo* info = infos[0];

	std::string name = info->pluginfile;
	std::string::size_type ls = name.find_last_of("\\/");
	if (ls != std::string::npos)
		name = name.substr(ls + 1);
	std::string::size_type ld = name.find_last_of('.');
	if (ld != std::string::npos)
		name = name.substr(0, ld);

	info->machines = &machines;
	info->basefilename = name;
	info->uri = "@zzub.org/buzz2zzub/" + name;
	replace(info->uri.begin(), info->uri.end(), ' ', '+');
	
	info->has_filled_parameters = from_cache;

	buzzplugins.push_back(info);
}

void buzzplugincollection::unregister_plugin_infos(std::vector<buzzplugininfo*>& infos) {
	assert(infos.size() == 1);
	buzzplugininfo* info = infos[0];

	std::vector<buzzplugininfo*>::iterator i = std::find(buzzplugins.begin(), buzzplugins.end(), info);
	if (i == buzzplugins.end()) return ;
	buzzplugins.erase(i);
}

bool buzzplugincollection::fill_plugin_infos(std::vector<buzzplugininfo*>& infos) {
	assert(infos.size() == 1);
	buzzplugininfo* info = infos[0];

	if (!info->load_buzz_dll()) return false;

	info->unload_buzz_dll();
	return true;
}

/***

	buzzplugininfo

***/

buzzplugininfo::buzzplugininfo(zzub::plugincollection* coll, std::string dllpath) {
	collection = coll;
	pluginfile = dllpath;
	hDllInstance = 0;
	GetInfo = 0;
	CreateMachine = 0;
	lockAddInput = lockSetTracks = useSequencerHack = false;
	plugin_lib = 0;
	has_filled = false;
	has_filled_parameters = false;
	exePlugin = false;
	create_count = 0;
}

bool buzzplugininfo::load_buzz_dll() {
	if (exePlugin) return true;

	assert(hDllInstance == 0);

	hDllInstance = unhack::loadLibrary(pluginfile.c_str());

	if (!hDllInstance) {
		cout << pluginfile << ": LoadLibrary failed." << endl;
		return false;
	}
	GetInfo = (GET_INFO)unhack::getProcAddress(hDllInstance, "GetInfo");
	if (!GetInfo) {
		cout << pluginfile << ": missing GetInfo." << endl;
		unhack::freeLibrary(hDllInstance);
		hDllInstance = 0;
		return false;
	}
	
	CreateMachine = (CREATE_MACHINE)unhack::getProcAddress(hDllInstance, "CreateMachine");
	if (!CreateMachine) {
		cout << pluginfile << ": missing CreateMachine." << endl;
		unhack::freeLibrary(hDllInstance);
		hDllInstance = 0;
		GetInfo = 0;
		return false;
	}

	if (!has_filled) {

		const CMachineInfo *buzzinfo = GetInfo();

		// small sanity check (this is legacy)
		if (!buzzinfo->Name || !buzzinfo->ShortName) {
			printf("%s: info name or short_name is empty. Skipping.\n", pluginfile.c_str()); 
			unhack::freeLibrary(hDllInstance);
			hDllInstance = 0;
			GetInfo = 0;
			CreateMachine = 0;
			return false;
		}

		fill_machineinfo(buzzinfo);

		if (!has_filled_parameters) {
			fill_machineinfo_parameters(buzzinfo);
			has_filled_parameters = true;
		}

		has_filled = true;
	}

	return true;
}

void buzzplugininfo::fill_machineinfo(const CMachineInfo *buzzinfo) {
	version = buzzinfo->Version;
	origFlags = buzzinfo->Flags;
	flags = zzub_plugin_flag_has_midi_input;
	
	if (origFlags & MIF_PLAYS_WAVES) flags |= zzub_plugin_flag_plays_waves;
	if (origFlags & MIF_USES_LIB_INTERFACE) flags |= zzub_plugin_flag_uses_lib_interface;
//	if (origFlags & MIF_USES_INSTRUMENTS) flags |= zzub_plugin_flag_uses_instruments;
	if (origFlags & MIF_DOES_INPUT_MIXING) flags |= zzub_plugin_flag_does_input_mixing;
//	if (origFlags & MIF_NO_OUTPUT) flags |= zzub_plugin_flag_no_output;
//	if (origFlags & MIF_MONO_TO_STEREO) flags |= zzub_plugin_flag_mono_to_stereo;

	if (flags != origFlags) {
		//cerr << "buzz2zzub: Buzz flags: " << buzzinfo->Flags << ", known flags: " << flags << endl;
	}
	
	// NOTE: A Buzz generator marked MIF_NO_OUTPUT is flagged with
	// neither input nor output flags. An effect with NO_OUTPUT is marked 
	// as input only. The MIF_NO_OUTPUT-flag is cleared no matter.
	// NOTE: this has been changed back, the no_output flag is kept in order
	// to put this special kind of plugin ahead in the work_order
	// therefore...

	// do not apply audio_output/audio_input-flags according to buzz type on
	// machines marked MIF_NO_OUTPUT
	switch (buzzinfo->Type) {
		case MT_MASTER: 
			flags |= BUZZ_ROOT_PLUGIN_FLAGS; 
			break;
		case MT_GENERATOR: 
			if ((buzzinfo->Flags & MIF_NO_OUTPUT) == 0)
				flags |= BUZZ_GENERATOR_PLUGIN_FLAGS; 
			break;
		default: 
			// TODO: we could look at the directory this plugin is contained in to determine bogus types
			cerr << "buzz2zzub: " << buzzinfo->Name << "(" << pluginfile << ") claims to be of type " << buzzinfo->Type << ", assuming effect" << endl;
			assert(false);
		case MT_EFFECT:
			if ((buzzinfo->Flags & MIF_NO_OUTPUT) == 0)
				flags |= BUZZ_EFFECT_PLUGIN_FLAGS; 
			else
				flags |= zzub_plugin_flag_has_audio_input;
			break;
	}
	min_tracks = buzzinfo->minTracks;
	max_tracks = buzzinfo->maxTracks;

	if (buzzinfo->Flags & MIF_MULTI_IO) {
		if (buzzinfo->Type == MT_GENERATOR) {
			outputs = MAX_BUZZ_IO_CHANNELS;
			inputs = 0;
		}
		if (buzzinfo->Type == MT_EFFECT) {
			outputs = MAX_BUZZ_IO_CHANNELS;
			inputs = MAX_BUZZ_IO_CHANNELS;
		}
	} else
	if ((buzzinfo->Flags & MIF_MONO_TO_STEREO) != 0 && (buzzinfo->Flags & MIF_DOES_INPUT_MIXING) == 0) {
		outputs = 2;
		if (buzzinfo->Type == MT_GENERATOR)
			inputs = 0; else
			inputs = 1;
	} else
	if ((buzzinfo->Flags & MIF_MONO_TO_STEREO) || (buzzinfo->Flags & MIF_DOES_INPUT_MIXING) || (buzzinfo->Flags & MIF_STEREO_EFFECT) ) {
		outputs = 2;
		if (buzzinfo->Type == MT_GENERATOR)
			inputs = 0; else
			inputs = 2;
	} else {
		if ((buzzinfo->Flags & MIF_NO_OUTPUT) == 0)
			outputs = 1;
		else
			outputs = 0;
		if (buzzinfo->Type == MT_GENERATOR)
			inputs = 0; else
			inputs = 1;
	}

	name = buzzinfo->Name;
	short_name = buzzinfo->ShortName;
	author = buzzinfo->Author != 0 ? buzzinfo->Author : "";
	commands = buzzinfo->Commands != 0 ? buzzinfo->Commands : "";

	// on re-attachment, we re-use plugin_lib, and simply update the blib member
	if (buzzinfo->pLI && !plugin_lib)
		plugin_lib = new libwrap(buzzinfo->pLI, this); 
	else if (buzzinfo->pLI && plugin_lib)
		((libwrap*)plugin_lib)->blib = buzzinfo->pLI;

	// set flags from buzz2zzub.ini
	std::map<std::string, std::vector<std::string> >::iterator it = unhack::patches.find(basefilename);
	if (it != unhack::patches.end()) {
		for (size_t i = 0; i<it->second.size(); i++) {
			if (it->second[i] == "lock-add-input") {
				lockAddInput = true;
			} else
			if (it->second[i] == "lock-set-tracks") {
				lockSetTracks = true;
			} else
			if (it->second[i] == "patch-seq") {
				useSequencerHack = true;
			}
		}
	}
}

void buzzplugininfo::fill_machineinfo_parameters(const CMachineInfo *buzzinfo) {

	// set parameters only the first time
	for (int i = 0; i < buzzinfo->numGlobalParameters; ++i) {
		zzub::parameter& param = add_global_parameter();
		convertParameter(1, param, buzzinfo->Parameters[i]);
		conformParameter(param);
	}
	if ((buzzinfo->Flags & MIF_PATTERN_EDITOR) != 0) {
		zzub::parameter& param = add_global_parameter()
			.set_byte()
			.set_name("Trigger")
			.set_description("Trigger Internal Pattern #")
			.set_value_min(1)
			.set_value_max(255)
			.set_value_default(0)
			.set_value_none(0)
			.set_state_flag();
	}
	for (int i = 0; i < buzzinfo->numTrackParameters; ++i) {
		zzub::parameter& param=add_track_parameter();
		convertParameter(2, param, buzzinfo->Parameters[buzzinfo->numGlobalParameters+i]);
		conformParameter(param);
	}
	for (int i = 0; i < buzzinfo->numAttributes; ++i) {
		zzub::attribute& attr = add_attribute();
		convertAttribute(attr, buzzinfo->Attributes[i]);
	}
}

void buzzplugininfo::unload_buzz_dll()
{
	if (hDllInstance)
	{
		unhack::freeLibrary(hDllInstance);
		hDllInstance = 0;
		GetInfo = 0;
		CreateMachine = 0;
		if (plugin_lib) {
			// we keep our plugin_lib, to allow re-attaching on calls to libwrap later
			((libwrap*)plugin_lib)->blib = 0;
		}
	}		
}
	
zzub::plugin* buzzplugininfo::create_plugin() {
	if (create_count == 0)
		if (!load_buzz_dll()) return 0;
	create_count++; // reference counting matched in the plugin destructor

	CMachineInterface* machine = CreateMachine();
	if (!machine) return 0;

	return new buzz2zzub::plugin(machine, this);
}
	
bool buzzplugininfo::store_info(zzub::archive *arc) const {
	return false;
}

}

zzub::plugincollection* buzz2zzub_get_plugincollection() {
	return new buzz2zzub::buzzplugincollection();
}
