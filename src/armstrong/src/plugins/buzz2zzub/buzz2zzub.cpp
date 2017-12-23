// buzz2zzub plugin adapter
// Copyright (C) 2006 Leonard Ritter (contact@leonard-ritter.com)
// Copyright (C) 2006-2013 Anders Ervik <calvin@countzero.no>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

// zzub2buzz allows running buzzmachines as zzub plugins
// please note that this library will only build correctly
// with msvc since gcc does not support thiscalls.

// however for debugging reasons, this file is also set up
// so it can be included on linux to fix compiler errors

#define NOMINMAX
#include <windows.h>
#include <cassert>
#include <vector>
#include <iostream>
#include <list>
#include <algorithm>
#include <map>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>
#include "buzz2zzub.h"
#include "dsp.h"
#include "plugincollection.h"
#include "editor.h"
#include "unhack.h"
#include "mdkimpl.h"
#include "mixing/convertsample.h"

using std::cout;
using std::cerr;
using std::endl;
using std::map;
using std::string;
using std::vector;

namespace {

int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}

#if !defined(_M_X64)
bool has_sse() {
	static bool sse_checked = false;
	static bool sse_result;

	if (sse_checked) return sse_result;

	__try {
		__asm {
			//xorps xmm0, xmm0        // executing SSE instruction
			xorpd xmm0, xmm0        // executing SSE2 instruction
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		// assert(_exception_code() == STATUS_ILLEGAL_INSTRUCTION)
		sse_result = false;
		sse_checked = true;
		return sse_result;
	}
	sse_result = true;
	sse_checked = true;
	return sse_result;
}
#else
inline bool has_sse() {
	return true;
}
#endif

#include <xmmintrin.h>
#define _MM_DENORM_ZERO_ON     0x0040 

struct fpustate {
	unsigned int sse_control_store;
	fpustate() {
		if (has_sse()) {
			// get old control word, set desired bits
			sse_control_store = _mm_getcsr();
			//cout << "sse control state " << sse_control_store << endl;
			// bits: 15 = denormals are zero | 14:13 = round to zero | 6 = flush to zero
			//_MM_MASK_UNDERFLOW
			//_mm_setcsr(sse_control_store | _MM_MASK_DENORM | _MM_MASK_UNDERFLOW | _MM_FLUSH_ZERO_MASK | _MM_DENORM_ZERO_ON); 
			_mm_setcsr(sse_control_store | _MM_FLUSH_ZERO_MASK | _MM_DENORM_ZERO_ON); 
		}
	}

	~fpustate() {
		if (has_sse()) {
			// restore old control word
			_mm_setcsr(sse_control_store); 
		}
	}

};

}

using namespace zzub;

namespace buzz2zzub {

struct plugin;
struct buzzplugininfo;

plugin::plugin(CMachineInterface* machine, buzzplugininfo* mi) {
	this->implementation = 0;
	this->machine2 = 0;
	this->machine = machine;
	this->global_values = this->machine->GlobalVals;
	this->track_values = this->machine->TrackVals;
	this->attributes = this->machine->AttrVals;
	this->machineInfo = mi;

	if (mi->origFlags & MIF_PATTERN_EDITOR)
		editor = new buzzeditor();

	track_count = machineInfo->min_tracks;
//	local_wavetable.resize(200);

	aux_buffer.resize(zzub_buffer_size * sizeof(float) * 4);
	oldinfo = 0;
	dirty_inputs = false;
	//dirty_waves.push();
}

plugin::~plugin() {
	// armstrong does not necessarily call init() before the destructor, but 
	// some buzz plugins require it (chorpse sr). 
	// assume init() was not called if oldinfo == 0
	if (oldinfo == 0) {
		init(0);
	}
	if (machineInfo->origFlags & MIF_PATTERN_EDITOR)
		delete editor;

	machineInfo->machines->destroy(_plugin);
	zzub_player_remove_callback(_player, armstrong_callback, this);
	delete this->machine;

	machineInfo->create_count--;
	if (machineInfo->create_count == 0) machineInfo->unload_buzz_dll();
}

// zzub::plugin implementations

int plugin::armstrong_callback(zzub_player_t* player, zzub_plugin_t* machine, zzub_event_data_t* data, void* tag) {
	int waveindex;
	plugin* self = (plugin*)tag;
	switch (data->type) {
		case zzub_event_type_update_wavelevel_samples:
			// wave id is not really index but we can make that assumption as long as there are 200 hardcoded waves
			waveindex = data->update_wavelevel_samples.wavelevel->datawavelevel->wave_id;
			// let the audio thread make the wave change notification
			// self->dirty_waves.next().push_back(waveindex);
			break;
		case zzub_event_type_insert_connection:
			if (self->machineInfo->lockAddInput)
				self->_mixer->lock_mode = true;
			break;
		case zzub_event_type_insert_plugin:
		case zzub_event_type_update_plugin:
			// TODO: should check for changed track count
			if (self->machineInfo->lockSetTracks)
				self->_mixer->lock_mode = true;
			break;
		case zzub_event_type_double_click:
			if (data->double_click.plugin == self->_plugin) {
				if ((self->machineInfo->origFlags & MIF_PATTERN_EDITOR) != 0) {
					self->editor->create(self, self->_plugin->dataplugin->name + " Editor");
					return 0;
				} else
					return self->invoke(DoubleClickMachine, 0) ? 0 : -1;
			}
			break;
		case zzub_event_type_barrier:
			//self->dirty_waves.push();
			//self->dirty_waves.next().clear();
			break;
	}
	return -1;
}

void plugin::destroy() { delete this; }

void plugin::init(archive *arc)
{ 
	unhack::hostwnd = (HWND)GetMainWindow();
	dwThreadID = GetCurrentThreadId();
	machineInfo->machines->create(this);

	machine->pCB = this;
	machine->pMasterInfo = reinterpret_cast<CMasterInfo*>(_master_info);
	oldinfo = _master_info;

	if ((machineInfo->origFlags & MIF_MONO_TO_STEREO) != 0 || (machineInfo->origFlags & MIF_STEREO_EFFECT) != 0 || (machineInfo->origFlags & MIF_MULTI_IO) != 0) {
		input_channels = machineInfo->inputs;
		output_channels = machineInfo->outputs;
	} else if ((machineInfo->origFlags & MIF_NO_OUTPUT) != 0) {
		output_channels = 0;
		if (machineInfo->flags & zzub_plugin_flag_has_audio_input)
			input_channels = 1;
	} else {
		output_channels = 1;
		if (machineInfo->flags & zzub_plugin_flag_has_audio_input)
			input_channels = 1;
	}

	if (arc) {
		zzub::instream* loadstream = arc->get_instream("");
		loadplugin(loadstream);
		machine->Init(&CMachineDataInputWrap(loadstream)); 

		if ((machineInfo->origFlags & MIF_PATTERN_EDITOR) != 0 && machine2 != 0) {
			for (std::vector<CPattern*>::iterator i = patterns.begin(); i != patterns.end(); ++i) {
				machine2->CreatePattern(*i, (*i)->length);
			}
		}
	}
	else
		machine->Init(0);

	// set param pointers again. they are already set in the constructor, but devon wahdul 
	// needs it here, and at least ninja delay needs it in the constructor (?)
	this->global_values = this->machine->GlobalVals;
	this->track_values = this->machine->TrackVals;
	this->attributes = this->machine->AttrVals;

	if ((machineInfo->origFlags & MIF_PATTERN_EDITOR) != 0) {
		editor->editorplugin = this;
		editor->create_framewindow();
		pattern_editor_trigger_offset = 0;
		for (std::vector<const zzub::parameter*>::const_iterator i = machineInfo->global_parameters.begin(); i != machineInfo->global_parameters.end() - 1; ++i) {
			pattern_editor_trigger_offset += (*i)->get_bytesize();
		}
	}

	zzub_player_add_callback(_player, &armstrong_callback, this);
}

void plugin::process_controller_events() {}

void plugin::transfer_hacked_plugin_states() {
	// TODO: transfer current parameter values from state_last to hacked tstate and gstate
	CMachine* m = GetThisMachine();
	
	transfer_track(1, 0, m->_internal_global_state, machineInfo->global_parameters);
	
	char* param_ptr = m->_internal_track_state;
	for (int i = 0; i < track_count; i++) {
		int size = transfer_track(2, i, param_ptr, machineInfo->track_parameters);
		param_ptr += size;

	}
}

void plugin::transfer_no_values() {
	transfer_track(1, 0, (char*)machine->GlobalVals, machineInfo->global_parameters, false, true);
	char* param_ptr = (char*)machine->TrackVals;
	for (int i = 0; i < track_count; i++) {
		int size = transfer_track(2, i, param_ptr, machineInfo->track_parameters, false, true);
		param_ptr += size;
	}
}

void plugin::transfer_last_values() {
	transfer_track(1, 0, (char*)machine->GlobalVals, machineInfo->global_parameters);
	char* param_ptr = (char*)machine->TrackVals;
	for (int i = 0; i < track_count; i++) {
		int size = transfer_track(2, i, param_ptr, machineInfo->track_parameters, true);
		param_ptr += size;
	}
}

int plugin::transfer_track(int group, int track, char* param_ptr, const std::vector<const zzub::parameter*>& params, bool stateonly, bool novalues) {
	int size = 0;
	for (int i = 0; i < (int)params.size(); i++) {
		if (stateonly && (params[i]->flags & zzub_parameter_flag_state) == 0) continue;
		int v;
		if (novalues)
			v = params[i]->value_none;
		else
			v = _mixer->get_parameter(_id, group, track, i);
		int param_size = params[i]->get_bytesize();
		memcpy(param_ptr, &v, param_size);
		param_ptr += param_size;
		size += param_size;
	}
	return size;
}

void plugin::process_events()
{
	if (dirty_inputs) {
		update_inputs();
		dirty_inputs = false;
	}

	if (_master_info != oldinfo) {
		machine->pMasterInfo = reinterpret_cast<CMasterInfo*>(_master_info);
		transfer_last_values();
		oldinfo = _master_info;
	}

	int last_play_position;
	if (machineInfo->useSequencerHack) {
		// support hacked jumping from work()
		unhack::hackTick(_master_info->beats_per_minute, _mixer->songinfo.top_item(0).loop_begin, 
			_mixer->songinfo.top_item(0).loop_end, _mixer->songinfo.top_item(0).loop_end, _master_info->row_position + 1);
		last_play_position = _master_info->row_position;
	}

	transfer_hacked_plugin_states();

	if (machineInfo->origFlags & MIF_PATTERN_EDITOR) {
		char* paramblock = (char*)global_values;
		int patternid = paramblock[pattern_editor_trigger_offset];
		if (patternid != 0) {
			patternid--;
			cout << "lets trigger pattern " << patternid << endl;
			if (patternid >= 0 && patternid < (int)patterns.size()) {
				CPattern* pat = patterns[patternid];
				if (pat != 0)
					machine2->PlayPattern(pat, 0, 0);
			}
		}
	}

	machine->Tick();

	// check for hacked jumps
	if (machineInfo->useSequencerHack) {
		if (last_play_position != unhack::hackseq->songPos)
			_master_info->row_position = unhack::hackseq->songPos;
	}

	last_process_events_tick_position = _master_info->tick_position;
}

bool plugin::process_offline(float **pin, float **pout, int *numsamples, int *channels, int *samplerate) { return false; }

bool plugin::process_stereo_multi(float **pin, float **pout, int numsamples, int mode) {

	bool ret;
	float* pouts[MAX_BUZZ_IO_CHANNELS / 2] = {0};
	float* pins[MAX_BUZZ_IO_CHANNELS / 2] = {0};
	__declspec(align(16)) float pouti[MAX_BUZZ_IO_CHANNELS / 2][256*2*2];
	__declspec(align(16)) float pini[MAX_BUZZ_IO_CHANNELS / 2][256*2*2];

	for (int i = 0; i < input_channels / 2; i++) {
		if (pin[i * 2] != 0 || pin[i * 2 + 1] != 0) {
			if (mode & zzub_process_mode_read) {
				s2i_amp(pini[i], &pin[i * 2], numsamples, 0x8000);
			} else
				memset(pini[i], 0, numsamples * 2 * sizeof(float));
			pins[i] = pini[i];
		} else
			pins[i] = 0;
	}
	int plugin_outputs = output_channels; // in case the machine changes this in its multiwork
	for (int i = 0; i < plugin_outputs / 2; i++) {
		if (pout[i * 2] != 0 || pout[i * 2 + 1] != 0) {
			if (mode & zzub_process_mode_read) {
				s2i_amp(pouti[i], &pout[i * 2], numsamples, 0x8000);
			}
			pouts[i] = pouti[i];
		} else
			pouts[i] = 0;
	}

	machine2->MultiWork(pins, pouts, numsamples);

	ret = false;
	for (int i = 0; i < plugin_outputs / 2; i++) {
		if (pouts[i] == 0) continue;
		if (!buffer_has_signals(pouts[i], numsamples * 2)) {
			pouts[i] = 0;
		} else {
			i2s_amp(&pout[i * 2], pouts[i], numsamples, 1.0f / 0x8000);
			ret = true;
		}
	}
	return ret;
}

bool plugin::process_stereo_mono(float **pin, float **pout, int numsamples, int mode) {

	bool ret;
	// mono out can be to left or right
	float* outbuffer;
	if (machineInfo->outputs == 1) {
		outbuffer = pout[0];
	} else if (machineInfo->outputs == 2) {
		outbuffer = (pout[0] != 0) ? pout[0] : pout[1];
	} else {
		outbuffer = 0;
	}

	if (mode & zzub_process_mode_read) {
		if (pin[0] != 0 && outbuffer != 0) {
			memcpy(outbuffer, pin[0], numsamples * sizeof(float));
			Amp(outbuffer, numsamples, 0x8000);
		} else
		if (outbuffer != 0) {
			memset(outbuffer, 0, numsamples * sizeof(float));
		}
	}

	if (outbuffer != 0) {
		ret = machine->Work(outbuffer, numsamples, mode);
		if (ret) {
			Amp(outbuffer, numsamples, 1.0f / 0x8000);
			// since we expose two channels, but internally only use one, make an extra copy of the resulting buffer
			if (machineInfo->outputs == 2 && pout[1] != 0 && pout[1] != outbuffer)
				memcpy(pout[1], outbuffer, numsamples * sizeof(float));
		}
	} else {
		// the output buffer is null (has no audio-out connections)
		float dummy[256 * 2 * 2] = {0.0f};
		ret = machine->Work(dummy, numsamples, mode);
	}

	if (!ret) {
		if (pout[0]) memset(pout[0], 0, numsamples * sizeof(float));
		if (machineInfo->outputs == 2 && pout[1]) memset(pout[1], 0, numsamples * sizeof(float));
		pout[0] = 0;
		if (machineInfo->outputs == 2) pout[1] = 0;
	}

	return ret;
}

bool plugin::process_stereo_stereo(float **pin, float **pout, int numsamples, int mode) {
	bool ret;
	__declspec(align(16)) float pouti[256*2*2] = {0.0f};
	__declspec(align(16)) float pini[256*2*2];
	if (input_channels == 1) {
		if ((mode & zzub_process_mode_read) != 0) {
			if (pin[0] != 0) {
				memcpy(pini, pin[0], numsamples * sizeof(float));
				Amp(pini, numsamples, 0x8000);
			} else
				memset(pini, 0, numsamples * sizeof(float));
		}
		ret = machine->WorkMonoToStereo(pini, pouti, numsamples, mode);
		if (ret) {
			i2s_amp(pout, pouti, numsamples, 1.0f / 0x8000);
		} else {
			if (pout[0]) memset(pout[0], 0, numsamples * sizeof(float));
			if (pout[1]) memset(pout[1], 0, numsamples * sizeof(float));
			pout[0] = pout[1] = 0;
		}
	} else {
		if (machineInfo->origFlags & MIF_STEREO_EFFECT) {
			if (mode & zzub_process_mode_read) {
				s2i_amp(pouti, pin, numsamples, 0x8000);
			}
			ret = machine->Work(pouti, numsamples, mode);
		} else {
			if (mode & zzub_process_mode_read) {
				s2i_amp(pini, pin, numsamples, 0x8000);
				s2i_amp(pouti, pout, numsamples, 0x8000);
			}
			ret = machine->WorkMonoToStereo(pini, pouti, numsamples, mode);
		}
		if (ret) {
			i2s_amp(pout, pouti, numsamples, 1.0f / 0x8000);
		} else {
			if (pout[0]) memset(pout[0], 0, numsamples * sizeof(float));
			if (pout[1]) memset(pout[1], 0, numsamples * sizeof(float));
			pout[0] = pout[1] = 0;
		}
	}
	return ret;
}

bool plugin::process_stereo(float **pin, float **pout, int numsamples, int mode) {

	fpustate ssezero; // contructor/destructor toggles sse flush to zero flags

	/*while (!dirty_waves.empty()) {
		for (std::vector<int>::iterator i = dirty_waves.top().begin(); i != dirty_waves.top().end(); ++i) {
			invoke(gWaveChanged, (void*)(int)*i);
		}
		dirty_waves.pop();
	}*/

	if (dirty_inputs) {
		update_inputs();
		dirty_inputs = false;
	}

	if (machineInfo->origFlags & MIF_DOES_INPUT_MIXING) {
		for (std::vector<plugininput*>::iterator i = inputs.begin(); i != inputs.end(); ++i) {
			plugininput* pi = *i;
			if (pi->has_buffer) {
				if (pi->output_count == 1 && pi->input_count == 2) {
					// mono mode, machine expands mono input to stereo itself
					machine2->Input(pi->buffer + pi->readpos, numsamples, pi->amp);
				} else {
					machine2->Input(pi->buffer + pi->readpos * 2, numsamples, pi->amp);
				}
			} else
				machine2->Input(0, 0, 0);
			pi->writepos = 0;
			pi->readpos += numsamples;
		}
	}

	int last_play_position;
	if (machineInfo->useSequencerHack) {
		// support hacked jumping from work()
		unhack::hackTick(_master_info->beats_per_minute, _mixer->songinfo.top_item(0).loop_begin, 
			_mixer->songinfo.top_item(0).loop_end, _mixer->songinfo.top_item(0).loop_end, _master_info->row_position + 1);
		last_play_position = _master_info->row_position;
	}

	if (/*((machineInfo->origFlags & MIF_CONTROL_MACHINE) || true) && */_mixer->state == zzub_player_state_playing) {
		// quickfix to ensure we call Tick() regularly on peer plugs even tho no parameters have changed
		if (_master_info->tick_position == 0 && last_process_events_tick_position != _master_info->tick_position) {
			last_process_events_tick_position = _master_info->tick_position;
			transfer_no_values(); // armstrong doesnt clear the parameter values like buzz, so clear here
			machine->Tick();
		}
	}
	last_process_events_tick_position = -1;

	bool ret;
	if (machineInfo->origFlags & MIF_MULTI_IO) {
		ret = process_stereo_multi(pin, pout, numsamples, mode);
	} else
	if (output_channels == 2) {
		ret = process_stereo_stereo(pin, pout, numsamples, mode);
	} else {
		// mono plugins and peers
		ret = process_stereo_mono(pin, pout, numsamples, mode);
	}

	// check for hacked jumps
	if (machineInfo->useSequencerHack) {
		if (last_play_position != unhack::hackseq->songPos)
			_master_info->row_position = unhack::hackseq->songPos;
	}

	return ret;
}

void plugin::stop()
{
	machine->Stop();
}

void plugin::loadplugin(zzub::instream *ins) {
	if (machineInfo->origFlags & MIF_PATTERN_EDITOR) {
		int version;
		size_t numpatterns;
		ins->read(version);
		if (version != 1) return ;
		ins->read(numpatterns);

		patterns.clear();
		for (size_t i = 0; i < numpatterns; i++) {
			CPattern* pat = new CPattern();
			ins->read(pat->name);
			ins->read(pat->length);
			patterns.push_back(pat);
		}
	}
}

void plugin::load(archive *arc) {
	zzub::instream* ins = arc->get_instream("");
	loadplugin(ins);
}

void plugin::save(archive *arc)
{
	zzub::outstream* outs = arc->get_outstream("");
	
	if (machineInfo->origFlags & MIF_PATTERN_EDITOR) {
		int version = 1;
		outs->write(version);
		outs->write(patterns.size());
		for (std::vector<CPattern*>::iterator i = patterns.begin(); i != patterns.end(); ++i) {
			outs->write((*i)->name.c_str());
			outs->write((*i)->length);
		}
	}

	machine->Save(&CMachineDataOutputWrap(outs));
}

void plugin::attributes_changed()
{
	machine->AttributesChanged();
}

void plugin::command(int index)
{
	machine->Command(index);
}

void plugin::set_track_count(int count)
{
	track_count = count;
	machine->SetNumTracks(count);
}

void plugin::mute_track(int index)
{
	machine->MuteTrack(index);
}

bool plugin::is_track_muted(int index) const
{ 
	return machine->IsTrackMuted(index); 
}

void plugin::event(unsigned int data)
{
	machine->Event(data);
}

const char * plugin::describe_value(int param, int value)
{
	return machine->DescribeValue(param, value); 
}

const envelope_info ** plugin::get_envelope_infos()
{
	return reinterpret_cast<const envelope_info**>(machine->GetEnvelopeInfos());
}

bool plugin::play_wave(int wave, int note, float volume, int offset, int length)
{ 
	return machine->PlayWave(wave, note, volume); 
}

void plugin::stop_wave()
{
	machine->StopWave();
}

int plugin::get_wave_envelope_play_position(int env)
{ 
	return machine->GetWaveEnvPlayPos(env);
}

// CMICallbacks implementations

CWaveInfo const *plugin::GetWave(const int i)
{
	zzub::wave_info* wave;
	if (dwThreadID == GetCurrentThreadId())
		wave = _mixer->waves.next()[i].get();
	else 
		wave = _mixer->waves.top()[i].get();

	if (!wave || wave->wavelevel_count == 0) return 0;

	return reinterpret_cast<CWaveInfo const*>(wave);
	/*CWaveInfo const* w = reinterpret_cast<CWaveInfo const*>(_host->get_wave(i));
	if (w) {
	}
	return w;*/
}

zzub::wave_level* plugin::get_wavelevel(zzub::wave_info* wave, int index) {
	int count = 0;
	const vector<boost::shared_ptr<wave_level> >& wavelevels = dwThreadID == GetCurrentThreadId() ? _mixer->wavelevels.next() : _mixer->wavelevels.top();
	for (vector<boost::shared_ptr<wave_level> >::const_iterator i = wavelevels.begin(); i != wavelevels.end(); ++i) {
		if (*i == 0) continue;
		if ((*i)->wave_id == wave->id) {
			if (count == index) return i->get();
			count++;
		}
	}
	return 0;
}

zzub::wave_level* plugin::get_nearest_wavelevel(zzub::wave_info* wave, int note) {
	if (wave->wavelevel_count == 0)
		return 0;

	if (note == zzub_note_value_none || note == zzub_note_value_off || note == zzub_note_value_cut)
		return 0;

	wave_level* nearest_level = 0;
	int nearest_notediff = -1;
	int midinote = buzz_to_midi_note(note);

	const vector<boost::shared_ptr<wave_level> >& wavelevels = dwThreadID == GetCurrentThreadId() ? _mixer->wavelevels.next() : _mixer->wavelevels.top();
	for (vector<boost::shared_ptr<wave_level> >::const_iterator i = wavelevels.begin(); i != wavelevels.end(); ++i) {
		if (*i == 0) continue;

		if ((*i)->wave_id == wave->id) {
			int wavemidinote = buzz_to_midi_note((*i)->root_note);
			int wavenotediff = abs(wavemidinote - note);
			if (nearest_notediff == -1 || wavenotediff < nearest_notediff) {
				nearest_level = i->get();
				nearest_notediff = wavenotediff;
			}
		}
	}
	return nearest_level;
}

// plugin2
const char* plugin::describe_param(int param) { 
	if (!machine2) return 0;
	return machine2->DescribeParam(param); 
}

bool plugin::set_instrument(const char *name) { 
	if (!machine2) return false;
	machine2->SetInstrument(name);
	return false; 
}

void plugin::get_sub_menu(int index, outstream *os) {
	if (!machine2) return ;
	CMachineDataOutputWrap mdow(os);
	machine2->GetSubMenu(index, &mdow);
}

void plugin::add_input(int connection_id) {
	if (!machine2) return;

	// called for does_input_mixing:
	// - if outputs == 1 && inputs == 1, we shall go to stereo mode so we accept input on r or l
	// - if outputs == 1 && inputs == 2, we shall go to mono mode so we accept input on r and l
	// - if outputs == 2 && inputs == 1, we shall go to stereo mode so we accept onput on r or l
	// - if outputs == 2 && inputs == 2, we shall go to stereo mode so we accept stereo input

	zzub::metaconnection* aconn = _mixer->connections.top()[connection_id].get();
	assert(aconn != 0);
	if (aconn->type != zzub_connection_type_audio) return ;
	zzub::metaplugin* mpl = _mixer->plugins.top()[_id].get();
	zzub::metaplugin* from_plugin = _mixer->plugins.top()[aconn->from_plugin_id].get();
	assert(from_plugin != 0);

	bool stereo = false;
	if (mpl->info->flags & zzub_plugin_flag_does_input_mixing) {
		if (aconn->output_count == 1 && aconn->input_count == 2) {
			stereo = false;
		} else
			stereo = true;
	} else {
		if (from_plugin->info->outputs >= 2) 
			stereo = true;
	}

	plugininput* inp = new plugininput();
	inp->name = from_plugin->name;
	inp->stereo = stereo;
	inp->writepos = 0;
	inp->readpos = 0;

	inputbyid[connection_id] = inp;
	inputs.push_back(inp);

	machine2->AddInput(from_plugin->name.c_str(), stereo);
	
	dirty_inputs = true;

	// force stereo input:
	// pvst may in some cases insist on interpreting the input buffer as a mono signal
	// unless we specifically call SetInputChannels(). otherwise we get garbled sound.
	// this could happen when loading a bmx saved in buzz with a mono machine running into pvst.
	
	// because everything is stereo in libzzub, the same problem is (still) true in reverse:
	// a bmx saved in buze where a mono machine runs into pvst will cause garbled sound in buzz.
	// machine2->SetInputChannels(name, true);
}

void plugin::delete_input(int connection_id) {
	if (!machine2) return ;

	std::map<int, plugininput*>::iterator i = inputbyid.find(connection_id);
	//assert(i != inputbyid.end());
	if (i == inputbyid.end()) return ; // e.g an event connection or something

	plugininput* pi = i->second;

	machine2->DeleteInput(pi->name.c_str());
	std::vector<plugininput*>::iterator newend = std::remove(inputs.begin(), inputs.end(), pi);
	inputs.erase(newend, inputs.end());
	delete pi;

	inputbyid.erase(i);

	dirty_inputs = true;
}

void plugin::rename_input(const char *oldname, const char *newname) { 
	if (!machine2) return;
	assert(false);	// TODO: this is not even in use!
	machine2->RenameInput(oldname, newname);
}

void plugin::update_inputs() {
	if (!machine2) return ;

	int maxinputs = 0, maxoutputs = 0;
	zzub::metaplugin* m = _mixer->plugins.top()[_id].get();
	assert(m != 0);
	if ((machineInfo->origFlags & MIF_MULTI_IO) != 0) return ; // does multi-io-does-input-mixing work? can only notify about stereo or no-stereo

	for (std::vector<metaconnection*>::iterator i = m->connections.begin(); i != m->connections.end(); ++i) {
		maxoutputs = std::max(maxoutputs, (*i)->output_count);
		maxinputs = std::max(maxinputs, (*i)->input_count);
	}

	if ((machineInfo->origFlags & MIF_STEREO_EFFECT) == 0 && maxoutputs == 1 && maxinputs == 2) {
		input_channels = 1;
	} else {
		input_channels = 2;
	}
}

/*
void plugin::call_input(int first_input, int first_output, int inputs, int outputs, int flags, float **samples, int size, float amp) {

	// called for does_input_mixing:
	// - if outputs == 1 && inputs == 1, we shall go to stereo mode so we accept input on r or l
	// - if outputs == 1 && inputs == 2, we shall go to mono mode so we accept input on r and l
	// - if outputs == 2 && inputs == 1, we shall go to stereo mode so we accept onput on r or l
	// - if outputs == 2 && inputs == 2, we shall go to stereo mode so we accept stereo input
	if (samples != 0) {
		if (outputs == 1) {
			if (inputs == 1) {
				// stereo mode, mono input on either on l or r channel
				float buffer[256*2*2];
				float zerobuffer[256*2*2];
				memset(zerobuffer, 0, sizeof(zerobuffer));
				float* ssamples[2] = { zerobuffer, zerobuffer };
				ssamples[first_input] = samples[first_output];
				s2i(buffer, ssamples, size);
				Amp(buffer, size * 2, 0x8000);
				machine2->Input(buffer, size, amp);
			} else
			if (inputs == 2) {
				// mono mode, machine expands mono input to stereo itself
				float buffer[256*2*2];
				memcpy(buffer, samples[0], size * sizeof(float));
				Amp(buffer, size, 0x8000);
				machine2->Input(buffer, size, amp);
			} else
				assert(false);  // invalid number of inputs
		} else
		if (outputs == 2) {
			if (inputs == 1) {
				// stereo mode, we shall take l or r from the input and output on both channels
				float buffer[256*2*2];
				float* ssamples[] = { samples[first_output], samples[first_output] };
				s2i(buffer, ssamples, size);
				Amp(buffer, size * 2, 0x8000);
				machine2->Input(buffer, size, amp);
			} else
			if (inputs == 2) {
				// stereo mode, stereo -> stereo
				float buffer[256*2*2];
				s2i(buffer,samples,size);
				Amp(buffer, size*2, 0x8000);
				machine2->Input(buffer, size, amp);
			} else
				assert(false);  // invalid number of inputs
		} else
			assert(false); // invalid number of outputs
	} else
		machine2->Input(0,0,0);
}
*/

void plugin::input(int connection_id, int first_input, int first_output, int inputs, int outputs, int flags, float **samples, int size, float amp) {
	if (!machine2) return ;

	static float zerobuffer[256*2*2] = {0};
	float* plin[MAX_BUZZ_IO_CHANNELS];
	for (int i = 0; i <  MAX_BUZZ_IO_CHANNELS; i++) {
		if (samples != 0 && samples[i] != 0)
			plin[i] = samples[i];
		else
			plin[i] = zerobuffer;
	}

	std::map<int, plugininput*>::iterator i = inputbyid.find(connection_id);
	assert(i != inputbyid.end());
	plugininput* pi = i->second;
	pi->has_buffer = samples != 0;
	if (samples != 0) {
		if (outputs == 1) {
			if (inputs == 1) {
				// stereo mode, mono input on either on l or r channel
				//float zerobuffer[256*2*2];
				//memset(zerobuffer, 0, sizeof(zerobuffer));
				float* ssamples[2] = { zerobuffer, zerobuffer };
				ssamples[first_input] = plin[first_output];
				s2i_amp(pi->buffer + pi->writepos * 2, ssamples, size, 0x8000);
			} else
			if (inputs == 2) {
				// mono mode, machine expands mono input to stereo itself
				memcpy(pi->buffer + pi->writepos, plin[0], size * sizeof(float));
				Amp(pi->buffer + pi->writepos, size, 0x8000);
			} else
				assert(false);  // invalid number of inputs
		} else
		if (outputs == 2) {
			if (inputs == 1) {
				// stereo mode, we shall take l or r from the input and output on both channels
				float* ssamples[] = { plin[first_output], plin[first_output] };
				s2i_amp(pi->buffer + pi->writepos * 2, ssamples, size, 0x8000);
			} else
			if (inputs == 2) {
				// stereo mode, stereo -> stereo
				float* ssamples[] = { plin[first_output], plin[first_output + 1] };
				s2i_amp(pi->buffer + pi->writepos * 2, ssamples, size, 0x8000);
			} else
				assert(false);  // invalid number of inputs
		} else {
			// convert >2 channels to stereo interleaved
			s2i_amp(pi->buffer + pi->writepos * 2, &plin[first_output], outputs, size, 0x8000);
		}

		//memcpy(pi->buffer, samples, sizeof(float*) * std::max(first_input + inputs, first_output + outputs));
	} else {
		memset(pi->buffer + pi->writepos * 2, 0, size * sizeof(float) * 2);
	}
	pi->amp = amp;
	pi->first_input = first_input;
	pi->first_output = first_output;
	pi->output_count = outputs;
	pi->input_count = inputs;
	pi->writepos += size;
	pi->readpos = 0;
}

int plugin::get_output_channel_count() {
	return output_channels;
}

int plugin::get_input_channel_count() {
	return input_channels;
}

const char* plugin::get_output_channel_name(int i) {
	if ((machineInfo->origFlags & MIF_MULTI_IO) == 0)
		return 0;
	const char* name = machine2->GetChannelName(false, i / 2);
	if (name == 0) 
		return 0;
	static char channelname[256];
	strncpy(channelname, name, 256);
	if (i%2 == 0)
		strncat(channelname, " L", 256);
	else
		strncat(channelname, " R", 256);
	return channelname;
}

const char* plugin::get_input_channel_name(int i) {
	if ((machineInfo->origFlags & MIF_MULTI_IO) == 0)
		return 0;
	const char* name = machine2->GetChannelName(true, i / 2);
	if (name == 0) 
		return 0;
	static char channelname[256];
	strncpy(channelname, name, 256);
	if (i%2 == 0)
		strncat(channelname, " L", 256);
	else
		strncat(channelname, " R", 256);
	return channelname;
}

void plugin::set_input_channels(int from_plugin_id, int first_input, int first_output, int inputs, int outputs, int flags) { 
	if (!machine2) return ;

	// called for does_input_mixing:
	// - if outputs == 1 && inputs == 1, we shall go to stereo mode so we accept input on r or l
	// - if outputs == 1 && inputs == 2, we shall go to mono mode so we accept input on r and l
	// - if outputs == 2 && inputs == 1, we shall go to stereo mode so we accept onput on r or l
	// - if outputs == 2 && inputs == 2, we shall go to stereo mode so we accept stereo input
	// this logic must be matched in input() in what buffer is passed to CMachineInterface::Input()
	if ((machineInfo->origFlags & MIF_MULTI_IO) != 0) return ; // does multi-io-does-input-mixing work? can only notify about stereo or no-stereo

	zzub::metaplugin* from_plugin = _mixer->plugins.top()[from_plugin_id].get();
	assert(from_plugin != 0);
	if ((machineInfo->origFlags & MIF_STEREO_EFFECT) == 0 && outputs == 1 && inputs == 2) {
		machine2->SetInputChannels(from_plugin->name.c_str(), false); 
	} else {
		machine2->SetInputChannels(from_plugin->name.c_str(), true);
	}

	dirty_inputs = true;
}

bool plugin::invoke(BEventType buzzevent, void* data) {
	for (size_t i=0; i<events.size(); i++) {
		if (events[i].et == buzzevent) {
			EVENT_HANDLER_PTR evptr = events[i].p;
			//return (machine->*evptr)(events[i].param);
			return (machine->*evptr)(data);
		}
	}
	return false;
}

bool plugin::handle_input(int index, int amp, int pan) { 
	if (!machine2) return false;
	return machine2->HandleInput(index, amp, pan);
}

void plugin::process_midi_events(zzub::midi_message* pin, int nummessages) {
	for (int i = 0; i < nummessages; i++) {
		unsigned short status = pin[i].message & 0xff;
		int channel = status&0xf;
		int command = (status & 0xf0) >> 4;
		unsigned char data1 = (pin[i].message >> 8) & 0xff;
		unsigned char data2 = (pin[i].message >> 16) & 0xff;
		int velocity;
		switch (command) {
			case 0x8:
			case 0x9:
				velocity = data2;
				if (command == 8) velocity = 0;
				machine->MidiNote(channel, data1, velocity);
				break;
			case 0xb:
				if (machine2)
					machine2->MidiControlChange(data1, channel, data2);
				break;
		}
	}
}

void plugin::get_midi_output_names(zzub::outstream *pout) {
	string name = "Buzz";
	pout->write((void*)name.c_str(), (unsigned int)(name.length()) + 1);
}

int plugin::get_latency() { 
	int plugin_version = get_plugin_version();
	if (machine2 && plugin_version >= 24)  // dont know which version introduced GetLatency, 24 might crash some plugins
		return machine2->GetLatency();
	return 0;
}

int plugin::get_plugin_version() {
	// returns the MI_VERSION from MachineInterface.h the plugin was built with
	return LOWORD(machineInfo->version);
}

CWaveLevel const *plugin::GetWaveLevel(const int i, const int level)
{
	wave_info* wave = _mixer->waves.top()[i].get();
	if (wave == 0) return 0;

	return reinterpret_cast<CWaveLevel const*>(get_wavelevel(wave, level));
}

void plugin::MessageBox(char const *txt) {
	cout << "buzz2zzub: " << this->machineInfo->name << ": " << txt << endl;
}

void plugin::Lock() { 
	//assert(false);
	//_host->lock(); 
}

void plugin::Unlock() { 
	//assert(false);
	//_host->unlock(); 
}

int plugin::GetWritePos() { 
	return 0;
	//return _host->get_write_position(); 
}

int plugin::GetPlayPos() { 
	return 0;
	//return _host->get_play_position(); 
}

float *plugin::GetAuxBuffer() { 
	return &aux_buffer.front();
}

void plugin::ClearAuxBuffer() { 
	std::fill(aux_buffer.begin(), aux_buffer.end(), 0.0f);
}

int plugin::GetFreeWave() { 
	return 0;
	//return _host->get_next_free_wave_index(); 
}

bool plugin::AllocateWave(int const i, int const size, char const *name) { 
	// TODO: should we use GetThreadId() to determine whether we are in the user thread 
	// or the audio thread...? (allocate_wave_direct vs allocate_wave)
	// TODO: we need to return a fake wave to the plugin and allocate a proper wave
	// in zzub after the plugin messes with the const datas.
	assert(false);
	return false;
	//return _host->allocate_wave_direct(i,0,size,wave_buffer_type_si16,false,name); 
}

void plugin::ScheduleEvent(int const time, dword const data) { 
	MessageBox("ScheduleEvent not implemented");
	//_host->schedule_event(time,data); 
}

void plugin::MidiOut(int const dev, dword const data) { 
	//_host->midi_out(dev, data); 
}
short const *plugin::GetOscillatorTable(int const waveform) { return oscTables[waveform]; /*return _host->get_oscillator_table(waveform);*/ }

// incredibly odd - raverb and some other jeskola machines require this to run =)
// we do not keep the value though, it may haunt us later. both raverb and the host keep their own static copies of this value
// the value seems to be combined from getlocaltime, getsystemtime, gettimezoneinfo and more.
/*
from buzz.exe disassembly of GetEnvSize implementation:
00425028 69 C0 93 B1 39 3E imul        eax,eax,3E39B193h 
0042502E 05 3B 30 00 00   add         eax,303Bh 
00425033 25 FF FF FF 7F   and         eax,7FFFFFFFh 
00425038 A3 F0 26 4D 00   mov         dword ptr ds:[004D26F0h],eax 
*/

int plugin::GetEnvSize(int const wave, int const env) { 
	if (wave<0) {
		return ((wave*0x3E39B193) + 0x303b ) & 0x7FFFFFFF;
	} else
	if (wave == 0 || wave > 200)	// jeskola tracker asks for GetEnvSize(0)
		return 0;
	//assert(false);
	return 0;
	//return _host->get_envelope_size(wave,env); 
}

bool plugin::GetEnvPoint(int const wave, int const env, int const i, word &x, word &y, int &flags) {
	//assert(false);
	return false;
	//return _host->get_envelope_point(wave, env, i, x, y, flags); 
}

CWaveLevel const *plugin::GetNearestWaveLevel(int const wave, int const note)
{
	if (wave==-1 && note==-1) {
		return (CWaveLevel*)(implementation=new CMDKImplementation());
	} else
	if (wave==-2 && note==-2) {
		return (CWaveLevel*)1;
	} else
	if (wave < 1 || wave > 200)	// jeskola tracker asks for GetNearestWaveLevel(0)
		return 0;

	wave_info* w = _mixer->waves.top()[wave].get();
	if (!w) return 0;
	return reinterpret_cast<CWaveLevel const*>(get_nearest_wavelevel(w, note));
}

void plugin::SetNumberOfTracks(int const n) { 
	cout << "SetNumberOfTracks" << endl;
}

CPattern *plugin::CreatePattern(char const *name, int const length) { 
	cout << "CreatePattern" << endl;
	return 0;
}

CPattern *plugin::GetPattern(int const index) { 
	cout << "GetPattern " << index << endl;
	return patterns[index];
}

char const *plugin::GetPatternName(CPattern *ppat) { 
	return ppat->name.c_str();
}

void plugin::RenamePattern(char const *oldname, char const *newname) { 
	cout << "RenamePattern" << endl;
}

void plugin::DeletePattern(CPattern *ppat) { 
	cout << "DeletePattern" << endl;
}

int plugin::GetPatternData(CPattern *ppat, int const row, int const group, int const track, int const field) { 
	cout << "GetPatternData" << endl;
	return 0;
}

void plugin::SetPatternData(CPattern *ppat, int const row, int const group, int const track, int const field, int const value) { 
	cout << "SetPatternData" << endl;
}

CSequence *plugin::CreateSequence() { 
	return 0;
	//return reinterpret_cast<CSequence*>(_host->create_sequence()); 
}

void plugin::DeleteSequence(CSequence *pseq) { 
	//_host->delete_sequence(reinterpret_cast<zzub_sequence_t*>(pseq)); 
}

CPattern *plugin::GetSequenceData(int const row) { 
	return 0;
	//return reinterpret_cast<CPattern*>(_host->get_sequence_data(row)); 
}

void plugin::SetSequenceData(int const row, CPattern *ppat) { 
	//_host->set_sequence_data(row, reinterpret_cast<int>(ppat)); 
}

void plugin::SetMachineInterfaceEx(CMachineInterfaceEx *pex) { 
	this->machine2 = pex;
}
void plugin::ControlChange__obsolete__(int group, int track, int param, int value) {
	//_host->_legacy_control_change(group, track, param, value); 
}

int plugin::ADGetnumChannels(bool input) {
	MessageBox("ADGetnumChannels not implemented");
	return 0;
	//return _host->audio_driver_get_channel_count(input); 
}

void plugin::ADWrite(int channel, float *psamples, int numsamples) {
	MessageBox("ADWrite not implemented");
	//_host->audio_driver_write(channel, psamples, numsamples); }
}

void plugin::ADRead(int channel, float *psamples, int numsamples) {
	MessageBox("ADRead not implemented");
	//_host->audio_driver_read(channel, psamples, numsamples); 
}

CMachine *plugin::GetThisMachine() { 
	return reinterpret_cast<CMachine*>(machineInfo->machines->get(_player, _plugin));
}

	// set value of parameter (group & 16 == don't record)
void plugin::ControlChange(CMachine *pmac, int group, int track, int param, int value) {
	bool record = true;
	bool immediate = false;
	if ((group & 0x10) == 0x10) {
		record = false;
		group ^= (group & 0x10);
	}

	// BTDSys PeerCtrl doesnt initialize the track parameter on the global track, so we set to 0
	// BTDSys PeerChord sends out-of range tracks, so we discard those
	// const CMachineParameter* mp;
	// we also use the zzub parameter, since pmac->buzzinfo may not be initialized, e.g with live slice
	const zzub::info* info = pmac->plugin->loader;
	const zzub::parameter* mp;
	switch (group) {
		case 1:
			track = 0;
			assert(param >= 0 && param < (int)info->global_parameters.size());
			mp = info->global_parameters[param];//pmac->buzzinfo->Parameters[param];
			break;
		case 2:
			if (track >= pmac->plugin->dataplugin->trackcount) {
				cerr << "buzz2zzub: discarding out-of-range track in ControlChange" << endl;
				return ;
			}
			assert(param >= 0 && param < (int)info->track_parameters.size());
			mp = info->track_parameters[param];// pmac->buzzinfo->Parameters[pmac->buzzinfo->numGlobalParameters + param];
			break;
		default:
			cerr << "buzz2zzub: attempt to ControlChange illegal group" << endl;
			return;
	}

	// BTDSys PeerState sends out-of-range values, so we sanitize
	if (mp->type == zzub_parameter_type_byte || mp->type == zzub_parameter_type_word) {
		if (value < mp->value_min) value = mp->value_min;
		if (value > mp->value_max) value = mp->value_max;
	} else if (mp->type == zzub_parameter_type_note) {
		if (value != zzub_note_value_off && value != zzub_note_value_cut && value != zzub_note_value_none) {
			if (value > zzub_note_value_max)
				value = zzub_note_value_max;
			if (value < 0)
				value = zzub_note_value_none;
		}
	}

	// can get here from many ways; use set_parameter_check() to make sure the parameter is set correctly:
	_mixer->set_parameter_check(pmac->plugin->userplugin->_id, group, track, param, value, record);
}

CSequence *plugin::GetPlayingSequence(CMachine *pmac) { 
	return 0;
	//return reinterpret_cast<CSequence*>(_host->get_playing_sequence(pmac->plugin)); 
}

void *plugin::GetPlayingRow(CSequence *pseq, int group, int track) { 
	return 0;
	//return _host->get_playing_row(reinterpret_cast<zzub_sequence_t*>(pseq), group, track); 
}

int plugin::GetStateFlags() { 
	//zzub_state_flag_playing = 1,
	//zzub_state_flag_recording = 2,
	return _mixer->state == zzub_player_state_playing?1:0;
}

void plugin::SetnumOutputChannels(CMachine *pmac, int n) { 
	cout << "plugin::SetnumOutputChannels(" << n << ")" << endl;
	output_channels = n;
	// at this point, buzz changes input_channels on connected plugins
	// we dont, since the number of input channels is determined by other factors
}

void plugin::SetEventHandler(CMachine *pmac, BEventType et, EVENT_HANDLER_PTR p, void *param) {
	event_wrap ew = {et, p, param};
	events.push_back(ew);
}

char const *plugin::GetWaveName(int const i) { 
	//return _host->get_wave_name(i); 
	return "";
}

void plugin::SetInternalWaveName(CMachine *pmac, int const i, char const *name) {
	//_host->set_internal_wave_name(pmac->plugin, i, name); 
}

void plugin::GetMachineNames(CMachineDataOutput *pout)
{ 
	for (int i = 0; i < (int)_mixer->plugins.next().size(); i++) {
		zzub::metaplugin* m = _mixer->plugins.next()[i].get();
		if (m == 0) continue;

		pout->Write((void*)m->name.c_str(), (int)m->name.length()+1);
	}
}

CMachine *plugin::GetMachine(char const *name) { 
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_name(_player, name);
	if (plugin == 0) return 0;
	return machineInfo->machines->get(_player, plugin);
}


void plugin::convertToBuzzParameter(CMachineParameter* toparam, const zzub::parameter* fromparam) {
	toparam->Name = new char[fromparam->name.length() + 1];
	strcpy((char*)toparam->Name, fromparam->name.c_str());
	toparam->Description = new char[fromparam->description.length() + 1];
	strcpy((char*)toparam->Description, fromparam->description.c_str());
	toparam->Type = (CMPType)fromparam->type;
	toparam->Flags = fromparam->flags;
	toparam->MinValue = fromparam->value_min;
	toparam->MaxValue = fromparam->value_max;
	toparam->NoValue = fromparam->value_none;
	toparam->DefValue = fromparam->value_default;
}

void plugin::convertToBuzzAttribute(CMachineAttribute* toattr, const zzub::attribute* fromattr) {
	toattr->Name = new char[fromattr->name.length() + 1];
	strcpy((char*)toattr->Name, fromattr->name.c_str());
	toattr->MinValue = fromattr->value_min;
	toattr->MaxValue = fromattr->value_max;
	toattr->DefValue = fromattr->value_default;
}

CMachineInfo const *plugin::GetMachineInfo(CMachine *pmac) {	
	if (pmac == 0) return 0;	// could happen after deleting a peer controlled machine
	const zzub::info *_info = pmac->plugin->loader;// _host->get_info(pmac->plugin);
	if (!_info) return 0;
	
	if (pmac->buzzinfo != 0) return pmac->buzzinfo;

	CMachineInfo *buzzinfo = pmac->buzzinfo = new CMachineInfo();

	if ((_info->flags & BUZZ_PLUGIN_FLAGS_MASK) == BUZZ_ROOT_PLUGIN_FLAGS)
		buzzinfo->Type = MT_MASTER;
	else if ((_info->flags & BUZZ_PLUGIN_FLAGS_MASK) == BUZZ_GENERATOR_PLUGIN_FLAGS)
		buzzinfo->Type = MT_GENERATOR;
	else if ((_info->flags & BUZZ_PLUGIN_FLAGS_MASK) == BUZZ_EFFECT_PLUGIN_FLAGS)
		buzzinfo->Type = MT_EFFECT;
	else
		buzzinfo->Type = MT_EFFECT;
	buzzinfo->Version = _info->version;
	buzzinfo->Flags = _info->flags;
	buzzinfo->minTracks = _info->min_tracks;
	buzzinfo->maxTracks = _info->max_tracks;
	buzzinfo->numGlobalParameters = (int)_info->global_parameters.size();
	buzzinfo->numTrackParameters = (int)_info->track_parameters.size();

	const CMachineParameter **param = new const CMachineParameter *[buzzinfo->numGlobalParameters+buzzinfo->numTrackParameters];
	buzzinfo->Parameters = param;
	for (int i=0; i < buzzinfo->numGlobalParameters; ++i) {
		CMachineParameter* newParam = new CMachineParameter();
		convertToBuzzParameter(newParam, _info->global_parameters[i]);
		*param = newParam;
		param++;
	}

	for (int i=0; i < buzzinfo->numTrackParameters; ++i) {
		CMachineParameter* newParam = new CMachineParameter();
		convertToBuzzParameter(newParam, _info->track_parameters[i]);
		*param = newParam;
		param++;
	}

	buzzinfo->numAttributes = (int)_info->attributes.size();
	if (buzzinfo->numAttributes > 0) {
		const CMachineAttribute ** attrs = new const CMachineAttribute *[buzzinfo->numAttributes];
		buzzinfo->Attributes = attrs;

		for (int i = 0; i < buzzinfo->numAttributes; i++) {
			CMachineAttribute* newattr = new CMachineAttribute();
			convertToBuzzAttribute(newattr, _info->attributes[i]);
			*attrs = newattr;
			attrs++;
		}
	} else {
		buzzinfo->Attributes = 0;
	}

	buzzinfo->Name = _info->name.c_str();
	buzzinfo->ShortName = _info->short_name.c_str();
	buzzinfo->Author = _info->author.c_str();
	buzzinfo->Commands = _info->commands.c_str();
	buzzinfo->pLI = (CLibInterface*)_info->plugin_lib;

	return buzzinfo;
}

char const *plugin::GetMachineName(CMachine *pmac) { 
	// PeerLFO may send in an invalid pmac
	if (!machineInfo->machines->exists(pmac)) {
		cerr << "buzz2zzub: Invalid CMachine* passed to GetMachineName()" << endl;
		return "";
	}

	static char name[256];
	strncpy(name, pmac->plugin->dataplugin->name.c_str(), 256);
	return name;
}

bool plugin::GetInput(int index, float *psamples, int numsamples, bool stereo, float *extrabuffer) { 
	return false;
}

// MI_VERSION 16

int plugin::GetHostVersion() {
	// available if GetNearestWaveLevel(-2, -2) returns non-zero, returns MI_VERSION of host
	cout << "GetHostVersion" << endl;
	return MI_VERSION;
}

// if host version >= 2
int plugin::GetSongPosition() {
	cout << "GetSongPosition" << endl;
	return _master_info->row_position + 1;
}

void plugin::SetSongPosition(int pos) {
	cout << "SetSongPosition" << endl;
}

int plugin::GetTempo() {
	cout << "GetTempo" << endl;
	return _master_info->beats_per_minute;
}

void plugin::SetTempo(int bpm) {
	cout << "SetTempo" << endl;
}

int plugin::GetTPB() {
	cout << "GetTPB" << endl;
	return _master_info->ticks_per_beat;
}

void plugin::SetTPB(int tpb) {
	cout << "SetTPB" << endl;
}

int plugin::GetLoopStart() {
	cout << "GetLoopStart" << endl;
	return 0;
	//return _host->get_song_begin_loop();
}

int plugin::GetLoopEnd() {
	cout << "GetLoopEnd" << endl;
	return 0;
	//return _host->get_song_end_loop();
}

int plugin::GetSongEnd() {
	cout << "GetSongEnd" << endl;
	return 0;
	//return _host->get_song_end();
}

void plugin::Play() {
	cout << "Play" << endl;
}

void plugin::Stop() {
	cout << "Stop" << endl;
}

bool plugin::RenameMachine(CMachine *pmac, char const *name) {
	// returns false if name is invalid
	cout << "RenameMachine" << endl;
	return false;
}

void plugin::SetModifiedFlag() {
	cout << "SetModifiedFlag" << endl;
}

int plugin::GetAudioFrame() {
	cout << "GetAudioFrame" << endl;
	return 0;
}

bool plugin::HostMIDIFiltering() { // if true, the machine should always accept midi messages on all channels
	cout << "HostMIDIFiltering" << endl;
	return false;
}
dword plugin::GetThemeColor(char const *name) {
	host_info* hi = &_mixer->hostinfo;
	if (hi->id == 42 && hi->version == 0x0503) {
		return (dword)SendMessage((HWND)hi->host_ptr, WM_GET_THEME, 0, (LPARAM)name);
	} else {
		cout << "GetThemeColor: " << name << endl;
		return 0xFFFFFF;
	}
}
void plugin::WriteProfileInt(char const *entry, int value) {
	cout << "WriteProfileInt " << entry << ": " << value << endl;
}
void plugin::WriteProfileString(char const *entry, char const *value) {
	cout << "WriteProfileString " << entry << ": " << value << endl;
}
void plugin::WriteProfileBinary(char const *entry, byte *data, int nbytes) {
	cout << "WriteProfileBinary " << entry << endl;
}
int plugin::GetProfileInt(char const *entry, int defvalue) {
	cout << "GetProfileInt " << entry << endl;
	return defvalue;
}
void plugin::GetProfileString(char const *entry, char const *value, char const *defvalue) {
	cout << "GetProfileString " << entry << endl;
}
void plugin::GetProfileBinary(char const *entry, byte **data, int *nbytes) {
	cout << "GetProfileBinary " << entry << endl;
}
void plugin::FreeProfileBinary(byte *data) {
	cout << "FreeProfileBinary" << endl;
}
int plugin::GetNumTracks(CMachine *pmac) {
	cout << "GetNumTracks" << endl;
	return 0;
}

void plugin::SetNumTracks(CMachine *pmac, int n) {
	// bonus trivia question: why is calling this SetNumberOfTracks not a good idea?
	cout << "SetNumTracks " << n << endl;
}

void plugin::SetPatternEditorStatusText(int pane, char const *text) {
	cout << "SetPatternEditorStatusText" << endl;
}

char const *plugin::DescribeValue(CMachine *pmac, int const param, int const value) {
	cout << "DescribeValue" << endl;
	return "";
}

int plugin::GetBaseOctave() {
	cout << "GetBaseOctave" << endl;
	return 4;
}

int plugin::GetSelectedWave() {
	cout << "GetSelectedWave" << endl;
	return 0;
}
void plugin::SelectWave(int i) {
	cout << "SelectWave" << endl;
}

void plugin::SetPatternLength(CPattern *p, int length) {
	cout << "SetPatternLength" << endl;
	p->length = length;
	machine2->SetPatternLength(p, length);
}

int plugin::GetParameterState(CMachine *pmac, int group, int track, int param) {
	cout << "GetParameterState" << endl;
	return 0;
}
void plugin::ShowMachineWindow(CMachine *pmac, bool show) {
	cout << "ShowMachineWindow" << endl;
}
void plugin::SetPatternEditorMachine(CMachine *pmac, bool gotoeditor) {
	cout << "SetPatternEditorMachine: " << pmac->buzzinfo->Name << ", gotoeditor=" << gotoeditor << endl;
}
// returns NULL if subtick timing is disabled in buzz options
CSubTickInfo const *plugin::GetSubTickInfo() {
	//cout << "GetSubTickInfo" << endl;
	return 0;
}
// returns zero-based index to the columns in the editor or -1 if s is not a valid sequence pointer
int plugin::GetSequenceColumn(CSequence *s) {
	cout << "GetSequenceColumn" << endl;
	return 0;
}
// call only in CMachineInterface::Tick()
void plugin::SetGroovePattern(float *data, int size) {
	cout << "SetGroovePattern" << endl;
}
void plugin::ControlChangeImmediate(CMachine *pmac, int group, int track, int param, int value) {
	cout << "ControlChangeImmediate" << endl;
}
void plugin::SendControlChanges(CMachine *pmac) {
	cout << "SendControlChanges" << endl;
	_mixer->on_process_events(_mixer->plugins.top()[pmac->plugin->dataplugin->id].get());
}
int plugin::GetAttribute(CMachine *pmac, int index) {
	cout << "GetAttribute" << endl;
	return 0;
}
void plugin::SetAttribute(CMachine *pmac, int index, int value) {
	cout << "SetAttribute" << endl;
}
void plugin::AttributesChanged(CMachine *pmac) {
	cout << "AttributesChanged" << endl;
}
void plugin::GetMachinePosition(CMachine *pmac, float &x, float &y) {
	cout << "GetMachinePosition" << endl;
}
void plugin::SetMachinePosition(CMachine *pmac, float x, float y) {
	cout << "SetMachinePosition" << endl;
}
void plugin::MuteMachine(CMachine *pmac, bool mute) {
	cout << "MuteMachine" << endl;
}
void plugin::SoloMachine(CMachine *pmac) {
	cout << "SoloMachine" << endl;
}
void plugin::UpdateParameterDisplays(CMachine *pmac) {
	cout << "UpdateParameterDisplays" << endl;
}

// write to debug console
void plugin::WriteLine(char const *text) {
	cout << "WriteLine" << endl;
}

// returns the state of a View->Options item
bool plugin::GetOption(char const *name) {
	cout << "GetOption" << endl;
	return false;
}

bool plugin::GetPlayNotesState() {
	cout << "GetPlayNotesState" << endl;
	return false;
}

// enable/disable multithreaded Work calls for the machine. can be called at any time.
void plugin::EnableMultithreading(bool enable) {
	cout << "EnableMultithreading" << endl;
}

CPattern *plugin::GetPatternByName(CMachine *pmac, char const *patname) {
	cout << "GetPatternByName" << endl;
	return editor->get_pattern_by_name(patname);
}

void plugin::SetPatternName(CPattern *p, char const *name) {
	cout << "SetPatternName" << endl;
	p->name = name;
	editor->bind_pattern_combo();
}

int plugin::GetPatternLength(CPattern *p) {
	cout << "GetPatternLength" << endl;
	return p->length;
}

CMachine *plugin::GetPatternOwner(CPattern *p) {
	cout << "GetPatternOwner" << endl;
	return 0;
}

bool plugin::MachineImplementsFunction(CMachine *pmac, int vtblindex, bool miex) {
	cout << "MachineImplementsFunction" << endl;
	return false;
}

void plugin::SendMidiNote(CMachine *pmac, int const channel, int const value, int const velocity) {
	cout << "SendMidiNote" << endl;
}

void plugin::SendMidiControlChange(CMachine *pmac, int const ctrl, int const channel, int const value) {
	cout << "SendMidiControlChange" << endl;
}

int plugin::GetBuildNumber() {
	return 1191;
}

void plugin::SetMidiFocus(CMachine *pmac) {
	cout << "SetMidiFocus" << endl;
}

void plugin::BeginWriteToPlayingPattern(CMachine *pmac, int quantization, CPatternWriteInfo &outpwi) {
	cout << "BeginWriteToPlayingPattern" << endl;
}

void plugin::WriteToPlayingPattern(CMachine *pmac, int group, int track, int param, int value) {
	cout << "WriteToPlayingPattern" << endl;
}

void plugin::EndWriteToPlayingPattern(CMachine *pmac) {
	cout << "EndWriteToPlayingPattern" << endl;
}

void *plugin::GetMainWindow() {
	host_info* hi = &_mixer->hostinfo;
	if (hi && hi->id == 42 && hi->version == 0x0503)
		return hi->host_ptr;
	return 0;
}

void plugin::DebugLock(char const *sourcelocation) {
	cout << "DebugLock" << endl;
}

void plugin::SetInputChannelCount(int count) {
	cout << "SetInputChannelCount" << endl;
	input_channels = count * 2;
}

void plugin::SetOutputChannelCount(int count) {
	cout << "SetOutputChannelCount" << endl;
	output_channels = count * 2;
}

bool plugin::IsSongClosing() {
	return false;
}

void plugin::SetMidiInputMode(MidiInputMode mode) { }

int plugin::RemapLoadedMachineParameterIndex(CMachine *pmac, int i) {
	// returns -1 if no mapping found, see pattern xp for example usage
	return -1;
}

char const *plugin::GetThemePath() {
	return "";
}

void plugin::InvalidateParameterValueDescription(CMachine *pmac, int index) { }
void plugin::RemapLoadedMachineName(char *name, int bufsize) {}			// needed by machines that refer other machines by name when importing (machines may change names)
bool plugin::IsMachineMuted(CMachine *pmac) { return false; }
int plugin::GetInputChannelConnectionCount(CMachine *pmac, int channel) { return 0; }
int plugin::GetOutputChannelConnectionCount(CMachine *pmac, int channel) { return 0; }
void plugin::ToggleRecordMode() {}

/***

	libwrap

***/

libwrap::libwrap(CLibInterface* mlib, buzzplugininfo* _info) {
	blib = mlib;
	info = _info;
}

void libwrap::get_instrument_list(zzub::outstream* os)  {
	if (!blib) {
		cout << "Preloading " << info->name << " via libwrap::get_instrument_list" << endl;
		buzzplugincollection* coll = (buzzplugincollection*)info->collection;
		if (!info->load_buzz_dll())
			return ;

		const CMachineInfo *buzzinfo = info->GetInfo();

		if ((buzzinfo->Flags & MIF_USES_LIB_INTERFACE) == 0 || buzzinfo->pLI == 0)
			return ;

		blib = buzzinfo->pLI;
	}

	CMachineDataOutputWrap mdow(os);
	blib->GetInstrumentList(&mdow);
}


/***

	CMachineManager

***/

CMachine* CMachineManager::get(zzub_player_t* host, zzub_plugin_t* metaplugin) {
	std::map<zzub_plugin_t*, CMachine*>::iterator i = plugin_to_machine.find(metaplugin);
	if (i == plugin_to_machine.end()) {
		return create(metaplugin->userplugin);
	} else
		return i->second;

}

// we need a buzz-compatible wrapper for zzub-plugins, i.e zzub2buzz
class CPluginWrap : public CMachineInterface {
public:
	CPluginWrap(CMachine* machine) {
		// peers try to write to these?
		GlobalVals = machine->_internal_global_state;
		TrackVals = machine->_internal_track_state;
		pMasterInfo = 0;
		pCB = 0;
	}
	virtual void Tick() {
		cerr << "buzz2zzub::CPluginWrap::Tick()" << endl;
	}
	virtual bool Work(float *psamples, int numsamples, int const mode) { 
		cerr << "buzz2zzub::CPluginWrap::Work()" << endl;
		return false; 
	}
	virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode) { 
		cerr << "buzz2zzub::CPluginWrap::WorkMonoToStereo()" << endl;
		return false; 
	}
	virtual void Stop() {
		cerr << "buzz2zzub::CPluginWrap::Stop()" << endl;
	}

};

CMachine* CMachineManager::create(zzub::plugin* plugin) {
	CMachine* machine = new CMachine();
	machine->plugin = plugin->_plugin;//_host->get_metaplugin();
	machine->_internal_machine_ex = 0;
	machine->_internal_seqCommand = 0;
	machine->_internal_name = "";	// Must set to "" or else PVST will crash in SetInstrument
	machine->_internal_track_state = (char*)new char[256*128*2];	// max 128 word parameters in 256 tracks;
	machine->_internal_global_state = (char*)new char[128*2];		// max 128 word parameters
	machine->_internal_machine = new CPluginWrap(machine);
	machine->hardMuted = false;	// at least polac out II uses this

	plugin_to_machine[machine->plugin] = machine;
	machine_to_plugin[machine] = machine->plugin;
	return machine;
}

CMachine* CMachineManager::create(buzz2zzub::plugin* plugin) {
	CMachine* machine = new CMachine();
	machine->plugin = plugin->_plugin;//_host->get_metaplugin();
	machine->_internal_machine = plugin->machine;
	machine->_internal_machine_ex = plugin->machine2;
	machine->_internal_seqCommand = 0;
	machine->_internal_name = "";	// Must set to "" or else PVST will crash in SetInstrument
	machine->_internal_track_state = (char*)new char[256*128*2];	// max 128 word parameters in 256 tracks;
	machine->_internal_global_state = (char*)new char[128*2];		// max 128 word parameters
	machine->hardMuted = false;	// at least polac out II uses this

	plugin_to_machine[machine->plugin] = machine;
	machine_to_plugin[machine] = machine->plugin;
	return machine;
}

void CMachineManager::destroy(zzub_plugin_t* metaplugin) {

	std::map<zzub_plugin_t*, CMachine*>::iterator i = plugin_to_machine.find(metaplugin);
	if (i != plugin_to_machine.end()) {

		std::map<CMachine*, zzub_plugin_t*>::iterator cmacit = machine_to_plugin.find(i->second);
		assert(cmacit != machine_to_plugin.end());

		delete[] i->second->_internal_global_state;
		delete[] i->second->_internal_track_state;
		if (i->second->buzzinfo) {
			delete[] i->second->buzzinfo->Parameters;
			delete i->second->buzzinfo;
		}
		delete i->second;
		plugin_to_machine.erase(i);
		machine_to_plugin.erase(cmacit);
	}
}

bool CMachineManager::exists(CMachine* pmac) {
	std::map<CMachine*, zzub_plugin_t*>::iterator cmacit = machine_to_plugin.find(pmac);
	return cmacit != machine_to_plugin.end();
}


} // namespace buzz2zzub

