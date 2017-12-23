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
#include <zzub/plugin.h>
#include "vst2zzub.h"
#include "pluginterfaces/vst2.x/aeffectx.h"
#include "editor.h"
#include "mixing/mixer.h"
#include "../plugincache.h"
#include "vstplugincollection.h"
#include "vstplugin.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);
static VstIntPtr VSTCALLBACK HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

std::string opcode_to_string(VstInt32 opcode);

#define MAKEMIDI(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))

/***

	vstplugin

***/

std::map<AEffect*, vstplugin*> vstplugin::plugins;

vstplugin::vstplugin(vstplugininfo* info) {
	_mixer = 0; // will be set by engine afterwards, set to 0 so we can test its validity in case the vst host callback tries to use it
	_info = info;
	timeInfoLastState = zzub::player_state_stopped;
	resizable = false;
	send_notesoff = false;

	cerr << "vst2zzub: creating plugin " << info->pluginfile << endl;
	program = new unsigned char[info->global_parameters.size()];
	currentprogram = new unsigned char[info->global_parameters.size()];
	vsteventbuffer = (VstEvents*)new char[sizeof(VstEvents) + sizeof(VstEvent*) * max_midievents - 1];
	for (int i = 0; i < max_midievents; i++)
		vsteventbuffer->events[i] = (VstEvent*)&midieventbuffer[i];

	global_values = program;
	set_new_program = -1;
	for (int i = 0; i < (int)info->global_parameters.size(); i++) 
		currentprogram[i] = info->global_parameters[i]->value_default;

	effect = info->create_vst_effect(&hPlugin);
	if (effect) {
		plugins[effect] = this;
	} else {
		hPlugin = 0;
	}
}

LRESULT CALLBACK ContainerProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	vstplugin* self;
	CREATESTRUCT* cs;

	switch(msg) {
		case WM_CREATE:
			cs = (CREATESTRUCT*)lParam;
			self = (vstplugin*)cs->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)self);
			return 0;
		case WM_DESTROY:
			self = (vstplugin*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			self->on_destroy_embedded_gui();
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void vstplugin::init(zzub::archive* arc) {
	int blocksize = zzub_buffer_size;
	timeInfoLastState = _mixer->state;

	audioinputs.resize(_info->inputs);
	audiooutputs.resize(_info->outputs);

	InitializeCriticalSection(&labelcs);

	effect->dispatcher(effect, effOpen, 0, 0, 0, 0);
	effect->dispatcher(effect, effSetSampleRate, 0, 0, NULL, (int)_master_info->samples_per_second);
	effect->dispatcher(effect, effSetBlockSize, 0, blocksize, NULL, 0.0f); 
	effect->dispatcher(effect, effMainsChanged, 0, 1, 0, 0);

	if (effect->dispatcher(effect, effCanDo, 0, 0, "sizeWindow", 0))
		resizable = true;

	// create hidden gui -  some plugins need a gui to handle setParameter/setProgram (EX-096) in ways that 

	if ((effect->flags & effFlagsHasEditor) != 0) {
		zzub::host_info* hi = &_mixer->hostinfo;
		HWND hHostWnd = (HWND)hi->host_ptr;

		editor.create(effect, _info->name, hHostWnd, true);
	}

	// load from chunk
	if (arc) load(arc);
}

void vstplugin::load(zzub::archive* arc) {
	// http://www.asseca.com/vst-24-specs/efSetChunk.html
	// check for programsAreChunks
	// VstInt32 vst::setChunk(void * data, VstInt32 byteSize, bool isPreset)
	// Alternatively, when not using chunk, the Host will simply save all parameter values <- this is done automatically
	assert(arc);
	zzub::instream* is = arc->get_instream("");

	int version, size;
	is->read(version);
	if (version == 0) {
		// version 0 didnt save these, use the defaults
		VstPatchChunkInfo chunkinfo;
		chunkinfo.version = 1;
		chunkinfo.pluginUniqueID = effect->uniqueID;
		chunkinfo.pluginVersion = effect->version;
		chunkinfo.numElements = effect->numParams;

		// read data (v0)
		is->read(size);
		char* data = new char[size];
		is->read(data, size);

		if (effect->dispatcher(effect, effBeginLoadProgram, 0, 0, &chunkinfo, 0.0f) >= 0) {
			effect->dispatcher(effect, effSetChunk, 1, size, data, 0.0f);
		}
		
		delete[] data;
	} else if (version == 1) {
		 // read chunk info (v1)
		VstPatchChunkInfo chunkinfo;
		chunkinfo.version = 1;
		is->read(chunkinfo.pluginUniqueID); // effect->uniqueID;
		is->read(chunkinfo.pluginVersion); // effect->version;
		is->read(chunkinfo.numElements); // = effect->numParams;
		
		// read current program (v1)
		int currentprogram = 0;
		is->read(currentprogram);

		// read data (v0)
		is->read(size);
		char* data = new char[size];
		is->read(data, size);

		// save/load program no
		effect->dispatcher(effect, effBeginSetProgram, 0, 0, 0, 0.0f);
		effect->dispatcher(effect, effSetProgram, 0, currentprogram, 0, 0.0f);
		effect->dispatcher(effect, effEndSetProgram, 0, 0, 0, 0.0f);

		// let plugin validate chunk before its set
		if (effect->dispatcher(effect, effBeginLoadProgram, 0, 0, &chunkinfo, 0.0f) >= 0) {
			effect->dispatcher(effect, effSetChunk, 1, size, data, 0.0f);
		}
		delete[] data;
	} else {
		cerr << "vst2zzub: attempt to load data for a newer version!" << endl;
	}
}

void vstplugin::save(zzub::archive* arc) {
	// http://www.asseca.com/vst-24-specs/efGetChunk.html
	// VstInt32 vst::getChunk(void ** data, bool isPreset);
	// Alternatively, when not using chunk, the Host will simply save all parameter values <- this is done automatically
	if((effect->flags & effFlagsProgramChunks) == 0 ) return; // prevent crash when dealing with nonchunky plugs
	void* data;
	int bytesize = effect->dispatcher(effect, effGetChunk, 1, 0, &data, 0.0f);
	if (bytesize > 0 && data) {
		zzub::outstream* os = arc->get_outstream("");

		int version = 1;
		os->write(version);
		os->write(effect->uniqueID);
		os->write(effect->version);
		os->write(effect->numParams);

		int currentprogram = effect->dispatcher(effect, effGetProgram, 0, 0, 0, 0.0f);
		os->write(currentprogram);

		os->write(bytesize);
		os->write(data, bytesize);
	}
}


void vstplugin::process_events() {
	EnterCriticalSection(&labelcs);

	// NOTE: effCanBeAutomated - hosts asks if parameter can be automated, this should be called each time before attempting to call SetParameter() 

	for (int i = 0; i < (int)_info->global_parameters.size(); i++) {
		if (program[i] != 255 && program[i] != currentprogram[i]) {
			currentprogram[i] = program[i];
			float value = (float)program[i] / 127.0f;
			effect->setParameter(effect, i, value);
		}
	}

	LeaveCriticalSection(&labelcs);
}

bool vstplugin::process_stereo(float **pin, float **pout, int numsamples, int mode) {
	EnterCriticalSection(&labelcs);

	if (set_new_program != -1) {
		/*std::vector<zzub::midi_message> messages;
		for (int i = 0; i < 16; i++) {
			zzub::midi_message msg;
			msg.message = MAKEMIDI(0xC0 | i, set_new_program, 0);
			messages.push_back(msg);
		}
		process_midi_events(&messages.front(), messages.size());*/


		// in many plugins, this is a no-op, unless the gui is open
		// in those cases, the midi-approach might be better. but its still not always saved correctly
		effect->dispatcher(effect, effBeginSetProgram, 0, 0, 0, 0.0f);
		effect->dispatcher(effect, effSetProgram, 0, set_new_program, 0, 0.0f);
		effect->dispatcher(effect, effEndSetProgram, 0, 0, 0, 0.0f);


		set_new_program = -1;
	}

	// use empty buffers instead of null-buffers for unconnected audio channels
	float* plin[64];
	float* plout[64];
	__declspec(align(16)) float zerobuffer[zzub_buffer_size] = {0}; // 64 == mixer::max_channels
	__declspec(align(16)) float tempbuffer[64][zzub_buffer_size] = {0}; // 64 == mixer::max_channels
	for (int i = 0; i < _info->inputs; i++) {
		if (pin[i] != 0)
			plin[i] = pin[i];
		else
			plin[i] = zerobuffer;
	}
	for (int i = 0; i < _info->outputs; i++) {
		if (pout[i] != 0)
			plout[i] = pout[i];
		else
			plout[i] = tempbuffer[i];
	}

	// TODO: check flags & effCanReplacing
	effect->processReplacing(effect, plin, plout, numsamples);

	LeaveCriticalSection(&labelcs);

	bool has_signals = false;
	for (int i = 0; i < _info->outputs && !has_signals; i++) {
		if (pout[i] != 0) {
			bool buffer_result = zzub::buffer_has_signals(pout[i], numsamples);
			if (buffer_result) {
				has_signals = true;
			} else
				pout[i] = 0;
		}
	}
	
	return has_signals;
}

void vstplugin::stop() {
	// there can be only one effProcessEvents per frame 
	// => postpone sending the notesoff message until process_midi_events()
	send_notesoff = true; 
}

void vstplugin::get_sub_menu(int index, zzub::outstream* os) {
	//char name[kVstMaxProgNameLen];
	for (std::vector<std::string>::const_iterator i = _info->programNames.begin(); i != _info->programNames.end(); ++i) {
		os->write(i->c_str());
	}
	os->write("\0", 1);
}

void vstplugin::command(int index) {
	if (index >= 0x100 && index < 0x200) {
		set_new_program = index - 0x100;
	} else
		return ;
}

int vstplugin::get_latency() {
	return effect->initialDelay;
}

const char* vstplugin::describe_value(int param, int value) {
	// http://www.reaper.fm/sdk/vst/vst_ext.php:
	// check for hasCockosExtensions -> effect->dispatcher(effect,effVendorSpecific,effGetParamDisplay,parm_idx,buf,val)>=0xbeef && *buf)

	static char description[256];
	static char label[256];
	description[0] = 0;
	label[0] = 0;

	// protect the user thread setParameters with a critical section to prevent deadlocks
	EnterCriticalSection(&labelcs);

	float oldvalue = effect->getParameter(effect, param);
	effect->setParameter(effect, param, (float)value / 127.0f);
	effect->dispatcher(effect, effGetParamDisplay, param, 0, &description, 0.0);
	effect->dispatcher(effect, effGetParamLabel, param, 0, &label, 0.0);

	// clamping to 0-1 cause am freehand returns 32 for a bunch of params
	effect->setParameter(effect, param, std::max(std::min(oldvalue, 1.0f), 0.0f));
	LeaveCriticalSection(&labelcs);

	strcat(description, " ");
	strcat(description, label);
	return description;
}


const char* vstplugin::get_output_channel_name(int i) {
	VstPinProperties& pinprop = audiooutputs[i];
	memset(&pinprop, 0, sizeof(VstPinProperties));

	if (effect->dispatcher(effect, effGetOutputProperties, i, 0, &pinprop, 0.0)) {
		return pinprop.label;
	}
	return 0;
}

const char* vstplugin::get_input_channel_name(int i) {
	VstPinProperties& pinprop = audioinputs[i];
	memset(&pinprop, 0, sizeof(VstPinProperties));

	if (effect->dispatcher(effect, effGetInputProperties, i, 0, &pinprop, 0.0)) {
		return pinprop.label;
	}
	return 0;
}

VstMidiEvent* vstplugin::get_vst_midievent(unsigned int message, VstMidiEvent* midiev) {
	//VstMidiEvent* midiev = new VstMidiEvent();
	midiev->type = kVstMidiType;
	midiev->byteSize = 24;
	midiev->deltaFrames = 0;
	midiev->flags = 0;
	midiev->noteLength = 0;
	midiev->noteOffset = 0;
	memcpy(midiev->midiData, &message, 4);
	midiev->detune = 0;
	midiev->noteOffVelocity = 0;
	return midiev;
}

void vstplugin::process_midi_events(zzub::midi_message* pin, int nummessages) {
	// checking for plugin.canDo(receiveMidiEvents) is not reliable
	// => assume all plugins support effProcessEvents (or at least do not crash) 

	int totalmessages = nummessages;
	if (send_notesoff)
		totalmessages++;

	if (totalmessages == 0) return ;

	assert(totalmessages < max_midievents);

	vsteventbuffer->numEvents = totalmessages;

	int index = 0;
	if (send_notesoff) { // send queued messages first
		get_vst_midievent(MAKEMIDI(0xB0, 123, 0), &midieventbuffer[index]);// = (VstEvent*);
		index++;
		send_notesoff = false;
	}

	for (int i = 0; i < nummessages; i++, index++) {
		get_vst_midievent(pin[i].message, &midieventbuffer[index]);
	}

	effect->dispatcher(effect, effProcessEvents, 0, 0, vsteventbuffer, 0);
}

void vstplugin::process_plugin_midi_events(VstEvents* events) {
	// midievents from plugin -> host
	for (int i = 0; i < events->numEvents; i++) {
		VstEvent* ev = events->events[i];
		if (ev->type == kVstMidiType) {
			VstMidiEvent* midiev = (VstMidiEvent*)ev;
			unsigned int message;
			memcpy(&message, midiev->midiData, 4);
			midi_out(midiev->deltaFrames, message); // TODO: check deltaFrames!
		}
	}
}

void vstplugin::midi_out(int time, unsigned int data) {
	zzub::midi_message msg = { -1, data, time };
	_mixer->midi_out(_id, msg);
}

void vstplugin::destroy() { 
	if (editor.hWnd)
		DestroyWindow(editor.hWnd);
	plugins.erase(effect);

	effect->dispatcher (effect, effMainsChanged, 0, 0, 0, 0);
	effect->dispatcher (effect, effClose, 0, 0, 0, 0);
	effect = 0;
	FreeLibrary(hPlugin);
	hPlugin = 0;
	DeleteCriticalSection(&labelcs);
	delete program;
	delete currentprogram;
	delete vsteventbuffer;
	delete this; 
}

void vstplugin::get_midi_output_names(zzub::outstream *pout) {
	string name = "VST";
	pout->write((void*)name.c_str(), (unsigned int)(name.length()) + 1);
}

bool vstplugin::has_embedded_gui() { 
	return (effect->flags & effFlagsHasEditor) != 0;
}

bool vstplugin::create_embedded_gui(void* hwnd) {

	static ATOM atomContainerClass = 0;
	if (atomContainerClass == 0) {
		WNDCLASS wc;
		memset(&wc, 0, sizeof(WNDCLASS));
		wc.lpfnWndProc = ContainerProc;
		wc.lpszClassName = "vst2zzubcontainerwnd";
		wc.style = CS_HREDRAW|CS_VREDRAW;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		atomContainerClass = RegisterClass(&wc);
	}
	
	hContainerWnd = CreateWindow("vst2zzubcontainerwnd", "", WS_CHILD|WS_VISIBLE, 0, 0, 0, 0, (HWND)hwnd, 0, 0, this);
	SetParent(editor.hWnd, (HWND)hContainerWnd);
	editor.show(true);
	return true;
}

void vstplugin::on_destroy_embedded_gui() {
	zzub::host_info* hi = &_mixer->hostinfo;
	HWND hHostWnd = (HWND)hi->host_ptr;

	editor.show(false);
	SetParent(editor.hWnd, hHostWnd);

	hContainerWnd = 0;
}


void vstplugin::resize_embedded_gui(void* hwnd, int* outwidth, int* outheight) {
	ERect* eRect = 0;
	effect->dispatcher(effect, effEditGetRect, 0, 0, &eRect, 0);
	if (eRect) {
		MoveWindow(hContainerWnd, 0, 0, eRect->right - eRect->left, eRect->bottom - eRect->top, TRUE);
		*outwidth = eRect->right - eRect->left;
		*outheight = eRect->bottom - eRect->top;
	} else {
		*outwidth = 0;
		*outheight = 0;
	}
}

void vstplugin::set_parameter(int column, int value) {
	if (column < 0 || column >= effect->numParams) {
		cerr << "vst2zzub: set_parameter() out of range, column = " << column << ", max = " << effect->numParams << endl;
		return ;
	}

	if (value < 0 || value > 127) {
		cerr << "vst2zzub: set_parameter() out of range, column = " << column << ", value = " << value << endl;
		return ;
	}

	// can get here via audioMasterAutomate, audioMasterDisplayChanged, effSetChunk, etc which could happen during init or from different threads.
	// so use set_parameter_check() to make sure the parameter is set correctly:
	_mixer->set_parameter_check(_id, 1, 0, column, value, true);
}

VstIntPtr vstplugin::host_callback(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {

	VstIntPtr result = 0;
	std::string canDo;

	if (opcode != audioMasterGetTime) {
		cout << "host_callback: " << opcode << ": " << opcode_to_string(opcode) << endl;
	}

	float tickpos;

	switch(opcode) {
		case audioMasterVersion :
			result = kVstVersion;
			break;
		case audioMasterAutomate:
			// generate an event that a parameter changed
			//<index> parameter that has changed
			//<opt> new value 
			set_parameter(index, opt*127);
			break;
		case audioMasterCurrentId:
			break;
		case audioMasterIdle:
			//NB - idle routine should also call effEditIdle for all open editors
			Sleep(1);
			break;
		case audioMasterPinConnected:
			break;
		case audioMasterWantMidi:
			return true;
			break;
		case audioMasterGetTime:
			memset(&timeInfo, 0, sizeof(VstTimeInfo));
			timeInfo.sampleRate = _master_info->samples_per_second;
			timeInfo.tempo = _master_info->beats_per_minute;
			timeInfo.flags = kVstPpqPosValid | kVstTempoValid | kVstBarsValid;

			if (_mixer) {
				timeInfo.samplePos = _mixer->song_position;
				if (_mixer->state == zzub_player_state_playing) {
					timeInfo.flags |= kVstTransportPlaying;
					tickpos = (float)_master_info->tick_position / ((float)_master_info->samples_per_tick + _master_info->samples_per_tick_frac);
					timeInfo.ppqPos = (float)(_master_info->row_position + tickpos) / (float)_master_info->ticks_per_beat;
				}

				if (timeInfoLastState != _mixer->state) {
					timeInfo.flags |= kVstTransportChanged;
				}
				timeInfoLastState = _mixer->state;
			}
			return (VstIntPtr)&timeInfo;
		case audioMasterProcessEvents:
			// plugin informs host that the plugin has MIDI events ready for the host to process, plugin should call this just before returning from processReplacing()
			// e[ptr]: pointer to VstEvents*
			// x[return]: 1 = supported and processed OK
			process_plugin_midi_events((VstEvents*)ptr);
			return 1;
		case audioMasterTempoAt:
			break;
		case audioMasterGetNumAutomatableParameters:
			break;
		case audioMasterGetParameterQuantization:
			break;
		case audioMasterIOChanged:
			break;
		case audioMasterNeedIdle:
			break;
		case audioMasterSizeWindow:
			editor.update_size();
			return 1;
		case audioMasterGetSampleRate:
			return _master_info->samples_per_second;
		case audioMasterGetBlockSize:
			return 0;
		case audioMasterGetInputLatency:
			break;
		case audioMasterGetOutputLatency:
			break;
		case audioMasterGetPreviousPlug:
			break;
		case audioMasterGetNextPlug:
			break;
		case audioMasterWillReplaceOrAccumulate:
			//0: not supported
			//1: replace
			//2: accumulate 
			break;
		case audioMasterGetCurrentProcessLevel:
			//0: not supported,
			//1: currently in user thread (gui)
			//2: currently in audio thread (where process is called)
			//3: currently in 'sequencer' thread (midi, timer etc)
			//4: currently offline processing and thus in user thread
			//other: not defined, but probably pre-empting user thread. 
			if (_mixer->in_user_thread())
				return 1;
			if (_mixer->in_encoder_thread())
				return 4;
			return 2;
		case audioMasterGetAutomationState:
			//0: not supported
			//1: off
			//2:read
			//3:write
			//4:read/write 
			if (_mixer->is_recording_parameters)
				return 4;
			else
				return 1;
		case audioMasterGetVendorString:
			break;
		case audioMasterGetProductString:
			strcpy((char*) ptr, "vst2zzub");
			break;
		case audioMasterGetVendorVersion:
			strcpy((char*) ptr, "vst2zzub devs");
			break;
		case audioMasterVendorSpecific:
			break;
		case audioMasterSetIcon:
			break;
		case audioMasterCanDo:
			canDo = (const char*)ptr;
			if (
				canDo == "sendVstEvents" ||
				canDo == "sendVstMidiEvent" ||
				canDo == "sendVstTimeInfo" || 
				canDo == "receiveVstEvents" ||
				canDo == "receiveVstMidiEvents" ||
				canDo == "receiveVstTimeInfo" ||
				canDo == "sizeWindow"
			)
				return 1;
			cout << "host cannot canDo " << canDo << endl;
			//"sendVstEvents",
			//"sendVstMidiEvent",
			//"sendVstTimeInfo",
			//"receiveVstEvents",
			//"receiveVstMidiEvent",
			//"receiveVstTimeInfo",
			//"reportConnectionChanges",
			//"acceptIOChanges",
			//"sizeWindow",
			//"asyncProcessing",
			//"offline",
			//"supplyIdle",
			//"supportShell" 
			break;
		case audioMasterGetLanguage:
			break;
		case audioMasterGetDirectory: {
			static char plugindir[MAX_PATH];
			std::string::size_type ls = _info->pluginfile.find_last_of("\\/");
			std::string path = _info->pluginfile.substr(0, ls);
			strcpy(plugindir, path.c_str());
			return (VstIntPtr)plugindir;
		}
		case audioMasterUpdateDisplay:
			// program change? send parameter update notifications
			// NOTE: synth1's getParameter() some times returns out of range values, why? thread-stuff?
			for (int i = 0; i < effect->numParams; i++) {
				set_parameter(i, effect->getParameter(effect, i) * 127);
			}
			break;
	}

	return result;
}

// http://www.cakewalk.com/devxchange/sidechainvst.asp
void vstplugin::set_speaker_arrangement() {
	VstInt32 typeIn, typeOut;
	VstInt32 numChannelsIn, numChannelsOut;

	// If it's a multi-input VST utilize all available inputs.

	//if (effect->numInputs > 2) {
		typeIn = kSpeakerArrUserDefined;
		numChannelsIn = effect->numInputs;
	//}
	typeOut = kSpeakerArrUserDefined;
	numChannelsOut = effect->numOutputs;

	VstSpeakerArrangement* pVstSpeakerArrangementIn;
	VstSpeakerArrangement* pVstSpeakerArrangementOut;
	int nExtraSpeakers = 0; 

	// Allocate variable size VstSpeakerArrangement structs
	nExtraSpeakers = numChannelsIn > 8 ? numChannelsIn - 8 : 0;
	pVstSpeakerArrangementIn
			= (VstSpeakerArrangement*) new BYTE[ sizeof(VstSpeakerArrangement) +
					(nExtraSpeakers * sizeof(VstSpeakerProperties)) ];

	nExtraSpeakers = numChannelsOut > 8 ? numChannelsOut - 8 : 0;
	pVstSpeakerArrangementOut
			= (VstSpeakerArrangement*) new BYTE[ sizeof(VstSpeakerArrangement) +
					(nExtraSpeakers * sizeof(VstSpeakerProperties)) ];

	// Configure the speaker arrangement for the VST to let it know
	// which inputs and outputs are in use
	// Input speaker arrangement

	memset(pVstSpeakerArrangementIn, 0, sizeof VstSpeakerArrangement);

	VstSpeakerArrangement& vstSpeakerArrangementIn = *pVstSpeakerArrangementIn;
	VstSpeakerArrangement& vstSpeakerArrangementOut = *pVstSpeakerArrangementOut;
	vstSpeakerArrangementIn.type = typeIn;
	vstSpeakerArrangementIn.numChannels = numChannelsIn;

	for (int ix = 0; ix < numChannelsIn; ++ix)
	{
			vstSpeakerArrangementIn.speakers[ix].azimuth = 0;
			vstSpeakerArrangementIn.speakers[ix].elevation = 0;
			vstSpeakerArrangementIn.speakers[ix].radius = 0;
			vstSpeakerArrangementIn.speakers[ix].radius = 0;
			vstSpeakerArrangementIn.speakers[ix].type = kSpeakerUndefined;
			vstSpeakerArrangementIn.speakers[ix].name[0] = '\0';
	}

	// Output speaker arrangement
	vstSpeakerArrangementOut.type = typeOut;
	vstSpeakerArrangementOut.numChannels = numChannelsOut;

	for (int ix = 0; ix < numChannelsOut; ++ix)
	{
			vstSpeakerArrangementOut.speakers[ix].azimuth = 0;
			vstSpeakerArrangementOut.speakers[ix].elevation = 0;
			vstSpeakerArrangementOut.speakers[ix].radius = 0;
			vstSpeakerArrangementOut.speakers[ix].radius = 0;
			vstSpeakerArrangementOut.speakers[ix].type = kSpeakerUndefined;
			vstSpeakerArrangementOut.speakers[ix].name[0] = '\0';
	}

	// Set the input and output speaker arrangement for the plug-in
	effect->dispatcher(effect, effSetSpeakerArrangement, 0,
                (LONG_PTR)pVstSpeakerArrangementIn,
                pVstSpeakerArrangementOut, 0.f); 

	//call_dispatcher(VSTEffect, effSetSpeakerArrangement, 0,
      //          (LONG_PTR)pVstSpeakerArrangementIn,
        //        pVstSpeakerArrangementOut, 0.f); 
}

zzub::plugincollection* vst2zzub_get_plugincollection() {
	return new vstplugincollection();
}
