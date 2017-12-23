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
#include "mixing/mixer.h"
#include "it_enum_list.h"
//#include "mp_api.h"
#include "SEModule_base.h"
#include "SEMod_struct.h"
#include "plugincollection.h"
#include "semplugin.h"

using std::cout;
using std::endl;

std::map<SEMod_struct_base2*, semplugin*> semplugin::pluginmap;

extern "C" long SE_CALLING_CONVENTION audioMasterCallback(SEMod_struct_base2 *effect, long opcode, long index, long value, void *ptr, float opt) {

	std::map<SEMod_struct_base2*, semplugin*>::iterator pluginit = semplugin::pluginmap.find(effect);
	assert(pluginit != semplugin::pluginmap.end());

	semplugin* plugin = pluginit->second;

	switch (opcode) {
		case seaudioMasterSetPinStatus:
			// dont print dont flood
			//cout << "seaudioMasterSetPinStatus" << endl;
			break;
		case seaudioMasterIsPinConnected:
			cout << "seaudioMasterIsPinConnected" << endl;
			break;
		case seaudioMasterGetPinInputText:
			cout << "seaudioMasterGetPinInputText" << endl;
			break;
		case seaudioMasterGetSampleClock:
			//cout << "seaudioMasterGetSampleClock" << endl;
			return plugin->_mixer->work_position;
		case seaudioMasterSendMIDI:
			cout << "seaudioMasterSendMIDI" << endl;
			break;
		case seaudioMasterGetInputPinCount:
			cout << "seaudioMasterGetInputPinCount" << endl;
			return plugin->_info->inputs;
		case seaudioMasterGetOutputPinCount:
			cout << "seaudioMasterGetOutputPinCount" << endl;
			return plugin->_info->outputs;
		case seaudioMasterGetPinVarAddress:
			cout << "seaudioMasterGetPinVarAddress" << endl;
			break;
		case seaudioMasterGetBlockStartClock:
			//cout << "seaudioMasterGetBlockStartClock" << endl;
			return plugin->_mixer->work_position;
		case seaudioMasterGetTime:
			cout << "seaudioMasterGetTime" << endl;
			break;
		case seaudioMasterSleepMode:
			// dont print dont flood
			//cout << "seaudioMasterSleepMode" << endl;
			break;
		case seaudioMasterGetRegisteredName:
			cout << "seaudioMasterGetRegisteredName" << endl;
			break;
		case seaudioMasterGetFirstClone:
			cout << "seaudioMasterGetFirstClone" << endl;
			break;
		case seaudioMasterGetNextClone:
			cout << "seaudioMasterGetNextClone" << endl;
			break;
		case seaudioMasterGetTotalPinCount:
			cout << "seaudioMasterGetTotalPinCount" << endl;
			return plugin->_info->inputs + plugin->_info->outputs + plugin->midiinputs.size() + plugin->midioutputs.size() + plugin->_info->global_parameters.size();
			break;
		case seaudioMasterCallVstHost:
			cout << "seaudioMasterCallVstHost" << endl;
			break;
		case seaudioMasterResolveFilename:
			cout << "seaudioMasterResolveFilename" << endl;
			break;
		case seaudioMasterSendStringToGui:
			cout << "seaudioMasterSendStringToGui" << endl;
			break;
		case seaudioMasterGetModuleHandle:
			cout << "seaudioMasterGetModuleHandle" << endl;
			break;
		case seaudioMasterAddEvent:
			cout << "seaudioMasterAddEvent" << endl;
			break;
		case seaudioMasterCreateSharedLookup:
			cout << "seaudioMasterCreateSharedLookup" << endl;
			break;
		case seaudioMasterSetPinOutputText:
			cout << "seaudioMasterSetPinOutputText" << endl;
			break;
		case seaudioMasterSetProcessFunction:
			cout << "seaudioMasterSetProcessFunction" << endl;
			// plugin->eff->sub_process_ptr; <- was changed
			break;
		case seaudioMasterResolveFilename2:
			cout << "seaudioMasterResolveFilename2" << endl;
			break;
		case seaudioMasterGetSeVersion:
			cout << "seaudioMasterGetSeVersion" << endl;
			break;
		default:
			cout << "unknown audioMasterCallback opcode " << opcode << endl;
			break;

	}
	// see se_sdk2/SEMod_struct_base.h for possible Host opCodes
	return 0;
}

semplugin::semplugin(semplugininfo* info) {
	_info = info;
	global_values = parameterbuffer;
	
	semparams = info->semparams;
	audioinputs = info->audioinputs;
	audiooutputs = info->audiooutputs;

	eff = 0;
	hPlugin = 0;
}

bool semplugin::load_dll() {
	hPlugin = LoadLibrary(_info->pluginFile.c_str());
	if (!hPlugin) return false;

	SE2_MAKEMODULE SE2_makeModule = (SE2_MAKEMODULE)GetProcAddress(hPlugin, "makeModule");

	if (!SE2_makeModule) {
		FreeLibrary(hPlugin);
		return false;
	}

	eff = (SEMod_struct_base2*)SE2_makeModule(_info->index, 1, audioMasterCallback, 0);
	if (!eff) {
		FreeLibrary(hPlugin);
		return false;
	}

	semplugin::pluginmap[eff] = this;

	// update variable addresses in pins:
	update_pins(semparams, true);
	update_pins(audioinputs, false);
	update_pins(audiooutputs, false);
	update_pins(midiinputs, false);
	update_pins(midioutputs, false);

	return true;
}

void semplugin::update_pins(std::vector<semparameter>& semparams, bool is_param ) {
	SEPinProperties pinprop;
	for (std::vector<semparameter>::iterator i = semparams.begin(); i != semparams.end(); ++i) {
		memset(&pinprop, 0, sizeof(SEPinProperties));
		if (eff->dispatcher(eff, seffGetPinProperties, i->pinindex, 0, &pinprop, 0)) {
			i->prop = pinprop; // retreive fresh variable_address for current instance
			if (!is_param && i->prop.datatype == DT_FSAMPLE) {
				// assign something for the defaults
				i->floatvalue = i->defaultvalue;
				*(float**)i->prop.variable_address = &i->floatvalue;
			}
		}
	}
}

void semplugin::init(zzub::archive* arc) {
	eff->dispatcher(eff, seffSetSampleRate, 0, 0, 0, (float)_master_info->samples_per_second);
	eff->dispatcher(eff, seffSetBlockSize, 0, 256, 0, 0);

	eff->dispatcher(eff, seffOpen, 0, 0, 0, 0);
}

void semplugin::destroy() {
	if (eff) {
		eff->dispatcher(eff, seffClose, 0, 0, 0, 0);
		//semplugin::pluginmap.erase(eff);
	}

	if (hPlugin)
		FreeLibrary(hPlugin);

}

void semplugin::process_events() {
	for (int i = 0; i < (int)_info->global_parameters.size(); i++) {
		const zzub::parameter* param = _info->global_parameters[i];
		semparameter& semparam = semparams[i];
		unsigned short* svalue;
		bool changed = false;
		switch (semparam.prop.datatype) {
			case DT_ENUM:
				svalue = (unsigned short*)&parameterbuffer[semparam.offset];
				if (*svalue != param->value_none  && semparam.range->FindIndex(*svalue)) {
					*((short*)semparam.prop.variable_address) = semparam.range->CurrentItem()->value;
					changed = true;
				}
				break;
			case DT_INT:
				svalue = (unsigned short*)&parameterbuffer[semparam.offset];
				if (*svalue != param->value_none) {
					*((int*)semparam.prop.variable_address) = _info->internal_to_sem_int(*svalue);
					changed = true;
				}
				break;
			case DT_FSAMPLE:
				svalue = (unsigned short*)&parameterbuffer[semparam.offset];
				if (*svalue != param->value_none) {
					semparameter* ioparam;
					if (semparam.prop.direction == DR_IN || semparam.prop.direction == DR_PARAMETER)
						ioparam = &audioinputs[semparam.channel];
					else if (semparam.prop.direction == DR_OUT)
						ioparam = &audiooutputs[semparam.channel];
					ioparam->floatvalue = semparam.floatvalue = _info->internal_to_sem_float(*svalue, &semparam);
					*((float**)semparam.prop.variable_address) = &ioparam->floatvalue;
					changed = true;
				}
				break;
				
		}

		if (changed) {
			SEModule_base* effobject = (SEModule_base *)eff->object;

			void* handleventptr = *(void**)(&eff->event_handler_ptr);
			my_delegate<event_function_ptr> myevent(handleventptr);

			SeEvent e(_mixer->work_position, UET_STAT_CHANGE, ST_STATIC, 0, (void*)semparam.pinindex);
			(effobject->*myevent.pointer.native)(&e);
		}
	}
	
	// MIDI events?
	//eff->event_handler_ptr(SeEvent* e);
}

bool semplugin::process_stereo(float** pin, float** pout, int numsamples, int mode) {
	// map inputs&outputs to pins
	float emptysamples[32][256];
	for (int i = 0; i < audioinputs.size(); i++) {
		float** ins = (float**)audioinputs[i].prop.variable_address;
		if (pin[i] != 0)
			*ins = pin[i];
		else {
			*ins = emptysamples[i];
			std::fill(*ins, *ins + numsamples, audioinputs[i].floatvalue);
		}
	}
	for (int i = 0; i < audiooutputs.size(); i++) {
		float** outs = (float**)audiooutputs[i].prop.variable_address;
		if (pout[i] != 0)
			*outs = pout[i];
		else {
			*outs = emptysamples[16+i];
			std::fill(*outs, *outs + numsamples, audiooutputs[i].floatvalue);
		}
	}

	SEModule_base* effobject = (SEModule_base *)eff->object;

	void* subprocessptr = *(void**)(&eff->sub_process_ptr);
	my_delegate<process_function_ptr2> myprocess(subprocessptr);

	(effobject->*myprocess.pointer.native)(0, numsamples);
	return true;
}

const char* semplugin::describe_value(int param, int value) {
	semparameter& semparam = semparams[param];
	SEPinProperties& pinprop = semparam.prop;

	std::stringstream strm;
	static std::string name;

	switch (pinprop.datatype) {
		case DT_ENUM:
			if (semparam.range->FindValue(value)) {
				enum_entry* entry = semparam.range->CurrentItem();
				static std::string t;
				t = std::string(entry->text.begin(), entry->text.end());
				return t.c_str();
			} else
				return 0;
		case DT_BOOL:
			if (value)
				return "true";
			else
				return "false";
		case DT_INT:
			strm << _info->internal_to_sem_int(value);
			name = strm.str();
			return name.c_str();
		case DT_FLOAT:
		case DT_FSAMPLE:
			strm << std::fixed << _info->internal_to_sem_float(value, &semparam);
			name = strm.str();
			return name.c_str();
		default:
			return 0;
	}
}

const char* semplugin::get_output_channel_name(int i) {
	return audiooutputs[i].prop.name;
}

const char* semplugin::get_input_channel_name(int i) {
	return audioinputs[i].prop.name;
}
