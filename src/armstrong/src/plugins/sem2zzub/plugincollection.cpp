#define _NOMINMAX
#include <windows.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#define __BUZZ2ZZUB__
#define NO_ZZUB_MIXER_TYPE

namespace zzub {
	struct mixer;
};
typedef struct zzub::mixer zzub_mixer_t;

#include <zzub/plugin.h>
//#include "mp_api.h"
#include "it_enum_list.h"
#include "SEModule_base.h"
#include "SEMod_struct.h"
#include "plugincollection.h"
#include "semplugin.h"

// http://www.synthedit.com/software-development-kit/documentation-old-version/technical-overview/


// DT_FSAMPLE:
//When you connect a slider to a plug, the slider's range is set 0 to 10. 
// You can specify a different range, for example 0 to 5...
//properties->datatype_extra = "5,0,5,0";

// DT_ENUM: 
// properties->datatype_extra = "Semitone=1,Octave=12";
// properties->datatype_extra = "range -12,12"; // To specify a range of values instead...
// 

using std::cout;
using std::cerr;
using std::endl;

zzub::plugincollection* sem2zzub_get_plugincollection() {
	static semplugincollection* coll = new semplugincollection();
	return coll;
}

zzub::plugin* semplugininfo::create_plugin() {
	semplugin* plugin = new semplugin(this);
	if (!plugin->load_dll()) {
		delete plugin;
		return 0;
	}
	return plugin;
}

int semplugininfo::sem_to_internal_int(int value) {
	return (value >> 16) + 32768;
}

int semplugininfo::internal_to_sem_int(int value) {
	return (value - 32768) << 16;
}

int semplugininfo::sem_to_internal_float(float value, semparameter* semparam) {
	// -1..1 -> 0..65534
	// semparameter* semparam
	float minvalue, maxvalue;
	if (semparam->range) {
		if (semparam->range->IsRange()) {
			minvalue = semparam->range->RangeLo();
			maxvalue = semparam->range->RangeHi();
		} else {
			// the range should be an array of 4 numbers: max, min, max, min - still dont know why there are two
			if (semparam->range->FindIndex(1))
				minvalue = _wtof(semparam->range->CurrentItem()->text.c_str());
			else
				minvalue = 0;
			if (semparam->range->FindIndex(0))
				maxvalue = _wtof(semparam->range->CurrentItem()->text.c_str());
			else
				maxvalue = 1.0f;
		}
	} else {
		minvalue = 0;
		maxvalue = 1;
	}

	float delta = 65534.0f * (1.0f / (maxvalue - minvalue));
	return (value - minvalue) * delta;
	//return ((value + 1) * 32767);
}

float semplugininfo::internal_to_sem_float(int value, semparameter* semparam) {
	float minvalue, maxvalue;
	if (semparam->range) {
		if (semparam->range->IsRange()) {
			minvalue = semparam->range->RangeLo();
			maxvalue = semparam->range->RangeHi();
		} else {
			// the range should be an array of 4 numbers: max, min, max, min - still dont know why there are two
			if (semparam->range->FindIndex(1))
				minvalue = _wtof(semparam->range->CurrentItem()->text.c_str());
			else
				minvalue = 0;
			if (semparam->range->FindIndex(0))
				maxvalue = _wtof(semparam->range->CurrentItem()->text.c_str());
			else
				maxvalue = 1.0f;
		}
	} else {
		minvalue = 0;
		maxvalue = 1;
	}

	// 0..65534 -> -1..1
	float delta = 65534.0f * (1.0f / (maxvalue - minvalue));
	return (float)value / delta + minvalue;
	//return (float)value / 32767.0f - 1.0f;
}

long SE_CALLING_CONVENTION audioMasterInitCallback(SEMod_struct_base2 *effect, long opcode, long index, long value, void *ptr, float opt) {
	cout << "audioMasterCallback opcode " << opcode << endl;
	// see se_sdk2/SEMod_struct_base.h for possible Host opCodes
	return 0;
}

semparameter* add_pin(semplugininfo* info, SEPinProperties& pinprop, int pinindex, std::vector<semparameter>* semparams) {

	semparameter semparam;
	if (pinprop.datatype_extra != 0) {
		std::string dtx = pinprop.datatype_extra;
		std::wstring datatype_extra(dtx.begin(), dtx.end());
		it_enum_list* paramrange = new it_enum_list(datatype_extra);
		semparam.range = paramrange;
	} else
		semparam.range = 0;
	
	semparam.pinindex = pinindex;
	semparam.prop = pinprop;
	semparam.offset = 0;
	semparam.channel = 0;
	if (pinprop.default_value != 0)
		semparam.defaultvalue = atof(pinprop.default_value);
	else
		semparam.defaultvalue = 0.0f;
	semparams->push_back(semparam);

	return &semparams->back();
}

void add_parameter(semplugininfo* info, SEPinProperties& pinprop, int pinindex, bool is_state, int channel, int* parameteroffset, std::vector<semparameter>* semparams) {

	if (pinprop.datatype == DT_TEXT) return ; // unsupported

	semparameter* param = add_pin(info, pinprop, pinindex, semparams);
	param->offset = *parameteroffset;
	param->channel = channel;

	zzub::parameter& p = info->add_global_parameter();
	switch (pinprop.datatype) {
		case DT_ENUM:
			p.set_word();
			p.set_value_min(0);
			if (param->range && param->range->size() > 0)
				p.set_value_max(param->range->size() - 1);
			else
				p.set_value_max(0);
			if (param->range && param->range->FindValue(param->defaultvalue))
				p.set_value_default(param->range->CurrentItem()->index);
			else
				p.set_value_default(0);
			*parameteroffset += 2;
			break;
		case DT_BOOL:
			p.set_switch();
			*parameteroffset += 1;
			break;
		case DT_FLOAT:
			p.set_word();
			p.set_value_min(0);
			p.set_value_max(65534);
			p.set_value_none(65535);
			p.set_value_default(info->sem_to_internal_float(param->defaultvalue, param));
			*parameteroffset += 2;
			break;
		case DT_INT:
			p.set_word();
			p.set_value_min(0);
			p.set_value_max(65534);
			p.set_value_none(65535);
			p.set_value_default(info->sem_to_internal_int(param->defaultvalue));
			*parameteroffset += 2;
			break;
		case DT_FSAMPLE:
			p.set_word();
			p.set_value_min(0);
			p.set_value_max(65534);
			p.set_value_none(65535);
			p.set_value_default(info->sem_to_internal_float(param->defaultvalue, param));
			*parameteroffset += 2;
			break;
		default:
			assert(false);
			break;
	}

	p.set_name(pinprop.name); // set name after type
	if (is_state)
		p.set_state_flag();
}

void semplugininfo::enum_parameters(SEMod_struct_base2* eff) {
	int parameteroffset = 0;
	SEPinProperties pinprop;
	for (int j = 0; ;++j) {
		memset(&pinprop, 0, sizeof(SEPinProperties));
		if (!eff->dispatcher(eff, seffGetPinProperties, j, 0, &pinprop, 0))
			break;
		if (pinprop.direction == DR_IN) {
			if (pinprop.datatype == DT_FSAMPLE) {
				add_pin(this, pinprop, j, &audioinputs);
				if (pinprop.default_value != 0)
					add_parameter(this, pinprop, j, true, (int)audioinputs.size() - 1, &parameteroffset, &semparams);
			} else 
			if (pinprop.datatype == DT_MIDI2) {
				add_pin(this, pinprop, j, &midiinputs);
			} else {
				add_parameter(this, pinprop, j, true, 0, &parameteroffset, &semparams);
			}
		} else
		if (pinprop.direction == DR_OUT) {
			if (pinprop.datatype == DT_FSAMPLE) {
				add_pin(this, pinprop, j, &audiooutputs);
			} else
			if (pinprop.datatype == DT_MIDI2) {
				add_pin(this, pinprop, j, &midioutputs);
			} else
				add_parameter(this, pinprop, j, true, 0, &parameteroffset, &semparams);
		} else
		if (pinprop.direction == DR_PARAMETER) {
			if (pinprop.datatype == DT_FSAMPLE) {
				add_pin(this, pinprop, j, &audioinputs);
				if (pinprop.default_value != 0)
					add_parameter(this, pinprop, j, true, (int)audioinputs.size() - 1, &parameteroffset, &semparams);
			} else {
				add_parameter(this, pinprop, j, true, 0, &parameteroffset, &semparams);
			}
		}
	}
}

bool semplugincollection::add_plugin(zzub::pluginfactory *factory, std::string abspath, std::string relpath) {
	//USES_CONVERSION;

	HMODULE hPlugin = LoadLibrary(abspath.c_str());
	if (!hPlugin) return false;

	SE2_GETMODULEPROPERTIES SE2_getModuleProperties = (SE2_GETMODULEPROPERTIES)GetProcAddress(hPlugin, "getModuleProperties");
	SE2_MAKEMODULE SE2_makeModule = (SE2_MAKEMODULE)GetProcAddress(hPlugin, "makeModule");
	/*MP_GETFACTORY MP_GetFactory = (MP_GETFACTORY)GetProcAddress(hPlugin, "MP_GetFactory");

	if (MP_GetFactory) {
		// version 3 yey
		;
	} else*/
	if (SE2_getModuleProperties && SE2_makeModule) {
		// version 2 yey
		SEModuleProperties modprop;
		for (int i = 0; SE2_getModuleProperties(i, &modprop); ++i) {
			cout << modprop.name << endl;
			SEMod_struct_base2* eff = (SEMod_struct_base2*)SE2_makeModule(i, 1, audioMasterInitCallback, this);

			if (eff == 0) continue;
			//void* effgui = SE2_makeModule(i, 2, audioMasterInitCallback, 0);

			if (eff != 0) {
				semplugininfo* info = new semplugininfo();
				info->collection = this;
				info->pluginFile = abspath;
				info->index = i;
				info->name = modprop.id;
				info->short_name = modprop.name;
				info->author = modprop.about;
				info->flags = 0;
				info->min_tracks = 0;
				info->max_tracks = 0;
				info->inputs = 0;
				info->outputs = 0;

				std::stringstream uristrm;
				uristrm << "@zzub.org/sem2zzub/" << modprop.id; // TODO: make spaceless uri

				info->uri = uristrm.str();

				// enumerate parameters and inputs/outputs
				info->enum_parameters(eff);

				info->inputs = (int)info->audioinputs.size();
				if (info->inputs > 0)
					info->flags |= zzub_plugin_flag_has_audio_input;
				
				info->outputs = (int)info->audiooutputs.size();
				if (info->outputs > 0)
					info->flags |= zzub_plugin_flag_has_audio_output;

				plugins.push_back(info);
				factory->register_info(info);

				eff->dispatcher(eff, seffClose, 0, 0, 0, 0);
			}

			// get pins = retreive parameter-channels and io/channels
		}

	} else {
		FreeLibrary(hPlugin);
		return false;
	}

	FreeLibrary(hPlugin);
	return true;
}

void semplugincollection::initialize(zzub::pluginfactory *factory) {
	std::string sempath = get_plugin_path();
	scan_plugins(sempath, "", factory);
}

std::string semplugincollection::get_plugin_path() {
	HMODULE module_handle = ::GetModuleHandle(0);
	if (!module_handle) return "";

	char path[MAX_PATH + 32] = {0};
	::GetModuleFileName(module_handle, path, MAX_PATH);
	std::size_t n(std::strlen(path));
	if(!n) return "";
	while(n--) {
		if (path[n]=='\\') {
			path[n] = 0;
			break;
		}
	}
	std::string s(path);
	return s + "\\Gear\\SynthEdit\\";
}

void semplugincollection::scan_plugins(std::string const & rootpath, std::string path, zzub::pluginfactory* factory) {
	// enumerate synthedit modules
	std::string search_path(rootpath + path);
	search_path += "*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	while(hFind != INVALID_HANDLE_VALUE) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
				std::string relpath(path + '\\' + fd.cFileName);
				scan_plugins(rootpath, relpath + "\\", factory);
			}
		} else {
			char* ext = strrchr(fd.cFileName, '.');
			if (ext != 0 && (stricmp(ext, ".sem") == 0 || stricmp(ext, ".sep") == 0)  ) {
				char absolute_path[MAX_PATH];
				std::string relpath(path + '\\' + (std::string)fd.cFileName);
				::GetFullPathName((rootpath + relpath).c_str(), MAX_PATH, absolute_path, 0);
				add_plugin(factory, absolute_path, relpath);
			}
		}
		if (!::FindNextFile(hFind, &fd)) break;
	}
	::FindClose(hFind);
}

