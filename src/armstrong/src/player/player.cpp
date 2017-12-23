#include <algorithm>
#include <list>
#include <stack>
#include <iostream>
#include <vector>
#include <cstring>
#include <boost/make_shared.hpp>
#include "player.h"
#include "mixing/mixer.h"
#include "mixing/connections.h"
#include "archive.h"

#include "plugins/core/plugins.h"
#include "plugins/stream/plugins.h"
#include "plugins/buzz2zzub/plugins.h"
#include "plugins/lunar/lunar.h"
#include "plugins/psy2zzub/plugins.h"
#include "plugins/lad2zzub/lad2zzub.h"
#include "plugins/vst2zzub/vst2zzub.h"
#include "plugins/hw2zzub/hw2zzub.h"
#include "plugins/midi/midi.h"
#include "plugins/modplug/plugins.h"
#include "plugins/ld_fungus/fungus.h"
#include "plugins/vamp2zzub/vamp2zzub.h"
#include "plugins/mfx2zzub/plugins.h"
#include "plugins/sem2zzub/sem2zzub.h"

#if !defined(WIN32)
#define strcmpi strcasecmp
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;

namespace armstrong {
namespace frontend {

inline void normalize(float& v) {
	if (v < SIGNAL_TRESHOLD && v > -SIGNAL_TRESHOLD) v = 0;
}

// find peaks and clip
inline void scan_peak_channels_clip(float** pin, int channels, int numSamples, float* maxes, float falloff) {
	bool zero = true;
	float v;
	for (int i = 0; i < channels; i++) {
		if (pin[i]) { 
			for (int j = 0; j < numSamples; j++) {
				normalize(maxes[i]);
				maxes[i] *= falloff;
				v = pin[i][j];
				maxes[i] = std::max(std::abs(v), maxes[i]);
				pin[i][j] = std::max(std::min(1.0f, v), -1.0f);
			}
		} else {
			for (int j = 0; j < numSamples; j++) {
				normalize(maxes[i]);
				maxes[i] *= falloff;
			}
		}
	}
}

player::player() {
	midiDriver = new mididriver();
	automated_pattern = false;
	event_userdata = 0;
	order_queue_index = -1;
	midi_lock = false;
	memset(last_max_peak, 0, sizeof(last_max_peak));

	plugmgr.add_collection(core_get_plugincollection());
	plugmgr.add_collection(connections_get_plugincollection());
	plugmgr.add_collection(midi_get_plugincollection());
#if defined(_WIN32)
	plugmgr.add_collection(new streamplugincollection());
#endif
#if defined(_WIN32) && !defined(_WIN64)
#pragma comment (lib, "lunar.lib")
	plugmgr.add_collection(lunar_get_plugincollection());
#endif

	plugmgr.add_collection(fungus_get_plugincollection());

#if defined(_WIN32)
#pragma comment (lib, "buzz2zzub.lib")
#pragma comment (lib, "vst2zzub.lib")
	plugmgr.add_collection(buzz2zzub_get_plugincollection());
	plugmgr.add_collection(vst2zzub_get_plugincollection());
#endif

#if defined(_WIN32) && !defined(_WIN64)
#pragma comment (lib, "psy2zzub.lib")
#pragma comment (lib, "lad2zzub.lib")
#pragma comment (lib, "hw2zzub.lib")
#pragma comment (lib, "modplug.lib")
#pragma comment (lib, "vamp2zzub.lib")
#pragma comment (lib, "mfx2zzub.lib")
#pragma comment (lib, "sem2zzub.lib")
	plugmgr.add_collection(psy2zzub_get_plugincollection());
	plugmgr.add_collection(lad2zzub_get_plugincollection());
	plugmgr.add_collection(hw2zzub_get_plugincollection());
	plugmgr.add_collection(modplug_get_plugincollection());
	plugmgr.add_collection(vamp2zzub_get_plugincollection());
	plugmgr.add_collection(mfx2zzub_get_plugincollection());
	plugmgr.add_collection(sem2zzub_get_plugincollection());
#endif

}

player::~player() {

	if (doc) {
		doc->clear();
		doc->barrier(0, 0, "Cleared song");
		doc->close();
		doc->unregister_listener(this);
		doc.reset();
	}
	mix->remove_user_listener(this);
	mix.reset();

	for (vector<zzub::plugincollection*>::iterator i = plugmgr.builtinplugins.begin(); i != plugmgr.builtinplugins.end(); ++i) {
		zzub::plugincollection* coll = *i;
		coll->destroy();
	}
	plugmgr.dummy_plugins.destroy();
	plugmgr.builtinplugins.clear();
	plugmgr.uri_infos.clear();
	plugmgr.plugin_infos.clear();
	delete midiDriver;
}


void player::initialize() {
	assert(!hostpath.empty());
	assert(!userpath.empty());
	assert(!temppath.empty());

	for (vector<zzub::plugincollection*>::iterator i = plugmgr.builtinplugins.begin(); i != plugmgr.builtinplugins.end(); ++i) {
		zzub::plugincollection* coll = *i;
		
		// send event
		zzub_event_data_t data;
		data.type = zzub_event_type_user_alert;
		data.alert.type = zzub_alert_type_enumerating_plugins;
		data.alert.collection = coll;
		invoke_player_event(data);

		coll->configure("hostpath", hostpath.c_str());
		coll->configure("userpath", userpath.c_str());
		coll->configure("temppath", temppath.c_str());
		coll->initialize(&plugmgr);
	}

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_user_alert;
	data.alert.type = zzub_alert_type_enumerating_plugins_done;
	invoke_player_event(data);

	mix = boost::make_shared<zzub::mixer>();

	mix->add_user_listener(this);

	midiDriver->initialize();
	//mix->midi = midiDriver;

	create_document();

	mix->set_state(zzub::player_state_stopped);
}

bool player::create_document() {
	doc = boost::make_shared<armstrong::storage::document>();
	doc->temppath = temppath;
	doc->register_listener(&armclient);
	doc->register_listener(this);

	if (!doc->open(":memory:")) return false;

	songdata = boost::make_shared<armstrong::storage::song>(doc.get());

	initialize_document();

	armclient.document = doc.get();
	return true;
}

bool player::initialize_document() {
	assert(songdata.get() != 0);
	if (!doc->get_song(*songdata)) {
		songdata->loopbegin = 0;
		songdata->loopend = 0;
		songdata->loopenabled = 1;
		songdata->samplerate = 44100;
		songdata->bpm = 126;
		songdata->tpb = 4;
		songdata->swing = 0.5;
		songdata->swingticks = 4;
		doc->create_song(*songdata);

		zzub::info* masterinfo = plugmgr.plugin_get_info("@zzub.org/master");
		assert(masterinfo != 0);
		armstrong::storage::plugin masterdata(doc.get());
		create_plugin(masterinfo, 0, 0, "Master", 0, masterdata);

		for (int i = 0; i < 200; i++) {
			armstrong::storage::wavedata wave;
			songdata->create_wave(0, wave);
		}

		doc->barrier(0, 0, "Created song"); // create undo step
		doc->clear_history();
	} else {
		doc->barrier(0, 0, "Started");
		doc->clear_history();
	}
	return true;
}

void player::reset() {

	// the lock around set_state() ensures the audio thread does not generate 
	// any new user events before the user event queues are emptied
	mix->audiomutex.lock();
	mix->set_state(zzub::player_state_muted);
	mix->audiomutex.unlock();

	mix->process_user_event_queue();
	mix->lock_mode = true; // block on barrier
	doc->undoredo_enabled = false; // disable undo while clearing

	doc->clear();
	doc->barrier(0, 0, "Cleared song");

	plugmgr.clear_dummy_plugins();
	load_errors.clear();

	mix->lock_mode = true; // block on barrier in initialize_document()
	initialize_document();

	doc->clear_unused_plugininfos();
	doc->undoredo_enabled = true;
	mix->set_state(zzub::player_state_stopped);
}

bool player::create_plugin(zzub::info* loader, zzub::instream* data, int datasize, std::string name, int layer_id, armstrong::storage::plugin& result) {
	vector<char> bytes(datasize);
	if (datasize) {
		assert(data != 0);
		data->read(&bytes.front(), datasize);
	}

	armstrong::storage::plugininfodata infodata;
	if (!songdata->get_plugininfo_by_uri(loader->uri, infodata)) {

		// loader's uri isnt in the db, re-create it
		songdata->create_plugininfo(loader->flags, loader->uri, loader->name, loader->short_name, loader->author, loader->min_tracks, loader->max_tracks, loader->inputs, loader->outputs, infodata);

		// create internal parameterinfos
		for (vector<const zzub::parameter*>::const_iterator i = loader->internal_parameters->begin(); i != loader->internal_parameters->end(); ++i) {
			armstrong::storage::parameterinfodata paramdata;
			int column = (int)(i - loader->internal_parameters->begin());
			songdata->create_parameterinfo(infodata.id, 0, 0, column, (*i)->name, (*i)->description, (*i)->flags, (*i)->type, (*i)->value_min, (*i)->value_max, (*i)->value_none, (*i)->value_default, paramdata);
		}

		// create global parameterinfos
		for (vector<const zzub::parameter*>::const_iterator i = loader->global_parameters.begin(); i != loader->global_parameters.end(); ++i) {
			armstrong::storage::parameterinfodata paramdata;
			int column = (int)(i - loader->global_parameters.begin());
			songdata->create_parameterinfo(infodata.id, 1, 0, column, (*i)->name, (*i)->description, (*i)->flags, (*i)->type, (*i)->value_min, (*i)->value_max, (*i)->value_none, (*i)->value_default, paramdata);
		}

		// create track parameterinfos
		for (vector<const zzub::parameter*>::const_iterator i = loader->track_parameters.begin(); i != loader->track_parameters.end(); ++i) {
			armstrong::storage::parameterinfodata paramdata;
			int column = (int)(i - loader->track_parameters.begin());
			songdata->create_parameterinfo(infodata.id, 2, 0, column, (*i)->name, (*i)->description, (*i)->flags, (*i)->type, (*i)->value_min, (*i)->value_max, (*i)->value_none, (*i)->value_default, paramdata);
		}

		// create controller parameterinfos
		for (vector<const zzub::parameter*>::const_iterator i = loader->controller_parameters.begin(); i != loader->controller_parameters.end(); ++i) {
			armstrong::storage::parameterinfodata paramdata;
			int column = (int)(i - loader->controller_parameters.begin());
			songdata->create_parameterinfo(infodata.id, 3, 0, column, (*i)->name, (*i)->description, (*i)->flags, (*i)->type, (*i)->value_min, (*i)->value_max, (*i)->value_none, (*i)->value_default, paramdata);
		}

		// create viurtual parameterinfos
		for (vector<const zzub::parameter*>::const_iterator i = loader->virtual_parameters.begin(); i != loader->virtual_parameters.end(); ++i) {
			armstrong::storage::parameterinfodata paramdata;
			int column = (int)(i - loader->virtual_parameters.begin());
			songdata->create_parameterinfo(infodata.id, 4, 0, column, (*i)->name, (*i)->description, (*i)->flags, (*i)->type, (*i)->value_min, (*i)->value_max, (*i)->value_none, (*i)->value_default, paramdata);
		}

		// create attributes
		for (vector<const zzub::attribute*>::const_iterator i = loader->attributes.begin(); i != loader->attributes.end(); ++i) {
			armstrong::storage::attributeinfodata attrdata;
			int index = (int)(i - loader->attributes.begin());
			songdata->create_attributeinfo(infodata.id, index, (*i)->name, (*i)->value_min, (*i)->value_max, (*i)->value_default, attrdata);
		}
	}

	// insert a record in the "plugin" db table:
	bool success = songdata->create_plugin(infodata.id, bytes, name, loader->min_tracks, 0, layer_id, result);
	if (!success) return false;

	// insert records in the "pluginparameter" db table with default parameter values:
	ensure_pluginparameters_exist("", loader, result.id, result.trackcount);
	return success;
}

void player::ensure_pluginparameters_exist(int pluginid) {
	ensure_pluginparameters_exist("", plugins[pluginid]->loader, pluginid, plugins[pluginid]->dataplugin->trackcount);
}

void player::ensure_pluginparameters_exist(const std::string& prefix, const zzub::info* loader, int pluginid, int numtracks) {

	// set default values in new tracks; these are updated in player::user_event(),
	// which in turn provides correct values to plugin_get_parameter_value():

	// super-hack: dont create internal parameters while upgrading from version9->10 in "select ensure_plugin_parameters()"
	if (prefix != "loaddb.") {

		for (std::vector<const zzub::parameter*>::const_iterator j = loader->internal_parameters->begin(); j != loader->internal_parameters->end(); ++j) {
			int column = (int)(j - loader->internal_parameters->begin());
			if (((*j)->flags & zzub_parameter_flag_state) != 0)
				doc->ensure_parameter_exists(prefix, pluginid, 0, 0, column, (*j)->value_default);
			else
				doc->ensure_parameter_exists(prefix, pluginid, 0, 0, column, (*j)->value_none);
		}

		for (std::vector<const zzub::parameter*>::const_iterator j = loader->virtual_parameters.begin(); j != loader->virtual_parameters.end(); ++j) {
			int column = (int)(j - loader->virtual_parameters.begin());
			if (((*j)->flags & zzub_parameter_flag_state) != 0)
				doc->ensure_parameter_exists(prefix, pluginid, 4, 0, column, (*j)->value_default);
			else
				doc->ensure_parameter_exists(prefix, pluginid, 4, 0, column, (*j)->value_none);
		}
	}

	for (std::vector<const zzub::parameter*>::const_iterator j = loader->global_parameters.begin(); j != loader->global_parameters.end(); ++j) {
		int column = (int)(j - loader->global_parameters.begin());
		if (((*j)->flags & zzub_parameter_flag_state) != 0)
			doc->ensure_parameter_exists(prefix, pluginid, 1, 0, column, (*j)->value_default);
		else
			doc->ensure_parameter_exists(prefix, pluginid, 1, 0, column, (*j)->value_none);
	}
	
	for (int i = 0; i < numtracks; i++) {
		for (std::vector<const zzub::parameter*>::const_iterator j = loader->track_parameters.begin(); j != loader->track_parameters.end(); ++j) {
			int column = (int)(j - loader->track_parameters.begin());
			if (((*j)->flags & zzub_parameter_flag_state) != 0)
				doc->ensure_parameter_exists(prefix, pluginid, 2, i, column, (*j)->value_default); 
			else
				doc->ensure_parameter_exists(prefix, pluginid, 2, i, column, (*j)->value_none);
		}
	}
}

plugin* player::get_plugin_by_name(string name) {
	armstrong::storage::plugindata plugdata;
	if (!songdata->get_plugin_by_name(name.c_str(), plugdata)) return 0;
	return plugins[plugdata.id].get();
}

//
// audioworker
//

void player::audio_enabled() {
	// TODO: notify mididriver and plugins
	midiDriver->set_jack_handle(jackhandle);
	midiDriver->set_latency(work_latency, work_rate);
}

void player::audio_disabled() {
	// TODO: notify mididriver and plugins
	// for use by jack midi and machines
	midiDriver->set_jack_handle(0);
}

void player::samplerate_changed() {

	// this can be called from the user or audio threads

	if (mix->user_thread_id == thread_id::get()) {
		cout << "player: samplerate_changed to " << work_rate << endl;
		mix->set_samplerate(work_rate);
	} else {
		mix->on_set_samplerate(work_rate);
	}

	// abusing the fact that samplerate_changed() == audiodriver_changed()
	mix->output_master_channel = work_master_channel;
	mix->output_channel_count = work_out_channel_count;
	mix->input_channel_count = work_in_channel_count;
}

void player::latency_changed() {
	// notification from the audio thread there was a reset, deviceinfos should be re-enumerated
	zzub::user_event_data ev;
	ev.type = zzub::user_event_type_latency_changed;
	mix->invoke_user_event(ev);
	midiDriver->set_latency(work_latency, work_rate);
}

void player::device_reset() {
	// notification from the audio thread there was a reset, deviceinfos should be re-enumerated
	zzub::user_event_data ev;
	ev.type = zzub::user_event_type_device_reset;
	mix->invoke_user_event(ev);
}


void player::work_stereo(float** pin, float** pout, int sample_count) {
	const int max_midi_count = 1024;
	zzub::midi_message inmidi[max_midi_count] = {0};
	zzub::midi_message outmidi[max_midi_count] = {0};

	int outmidi_count = 0;
	int inmidi_count = 0;
	midiDriver->poll(inmidi, &inmidi_count, max_midi_count, sample_count, work_rate);
	work_stereo_with_midi(pin, pout, sample_count, inmidi, inmidi_count, outmidi, &outmidi_count, max_midi_count);
	midiDriver->schedule_send(outmidi, outmidi_count, sample_count, work_rate);
}

void player::work_stereo_with_midi(float** pin, float** pout, int sample_count, zzub::midi_message* pmidiin, int inmidi_count, zzub::midi_message* pmidiout, int* outmidi_count, int outmidi_maxcount) {
	using namespace std;

	work_buffer_position = 0;
	//mix->latency_samples = work_latency;

	int remaining_samples = sample_count;
	float silent_samples[zzub::mixer::max_channels] = {0};
	int outmidi_remaining = outmidi_maxcount;

	// examine midi input for midi stop/play commands
	process_midi_events(pmidiin, inmidi_count);

	while (remaining_samples > 0) {

		float* plout[zzub::mixer::max_channels] = {0};
		float* plin[zzub::mixer::max_channels] = {0};
		if (pout != 0) {
			for (int i = 0; i < work_out_channel_count; i++) {
				plout[i] = &pout[i][work_buffer_position];
			}
		}

		if (pin != 0) {
			for (int i = 0; i < work_in_channel_count; i++) {
				plin[i] = &pin[i][work_buffer_position];
			}
		}

		mix->buffer_size = work_buffersize;
		mix->buffer_position = work_buffer_position;

		int outmidi_written = 0;
		int outmidi_offset = outmidi_maxcount - outmidi_remaining;

		int chunk_size = mix->generate_audio(plin, plout, remaining_samples, pmidiin, inmidi_count, &pmidiout[outmidi_offset], &outmidi_written, outmidi_maxcount);

		outmidi_remaining -= outmidi_written;
		inmidi_count = 0;

		float samplerate = float(mix->rootplayer->_master_info.samples_per_second);
		float falloff = std::pow(10.0f, (-48.0f / (samplerate * 20.0f))); // vu meter falloff (-48dB/s)
		scan_peak_channels_clip(plout, work_out_channel_count, chunk_size, last_max_peak, falloff);

		// clear work_out_buffer for null plouts .. (unless the buffers were already fully cleared)
		for (int i = 0; i < work_out_channel_count; i++) {
			if (plout[i] == 0) {
				if (silent_samples[i] == work_buffer_position) {
					// if everything in this channel was silent until now, continue counting silent samples
					silent_samples[i] += chunk_size;
				} else if (pout != 0 && pout[i] != 0) {
					memset(&pout[i][work_buffer_position], 0, chunk_size * sizeof(float));
				}
			} else {
				if (silent_samples[i] == work_buffer_position && pout != 0 && pout[i] != 0) {
					// it was silent, up until this chunk, clear what was before
					memset(pout[i], 0, work_buffer_position * sizeof(float));
				}
			}
		}

		work_buffer_position += chunk_size;
		remaining_samples -= chunk_size;

		//mix.swap_lock.unlock();
	}

	for (int i = 0; i < work_out_channel_count; i++) {
		if (silent_samples[i] == work_buffer_position && pout != 0)
			pout[i] = 0;
	}


	// update cpu_load per plugin
	for (size_t i = 0; i < mix->plugins.top().size(); i++) {
		if (mix->plugins.top()[i] == 0) continue;
		zzub::metaplugin& m = *mix->plugins.top()[i];
		double load;
		
		if (m.audiodata->cpu_load_buffersize > 0)
			load = (m.audiodata->cpu_load_time * double(work_rate)) / double(m.audiodata->cpu_load_buffersize); else
			load = 0;
		m.audiodata->cpu_load += 0.1 * (load - m.audiodata->cpu_load);

		m.audiodata->cpu_load_time = 0;
		m.audiodata->cpu_load_buffersize = 0;
	}

	*outmidi_count = outmidi_maxcount - outmidi_remaining;
}

void player::process_midi_events(zzub::midi_message* pmidi, int midi_count) {

	for (int i = 0; i < midi_count; i++) {
		unsigned long message = pmidi[i].message;
		unsigned short status = message & 0xff;
		unsigned char data1 = (message >> 8) & 0xff;
		unsigned char data2 = (message >> 16) & 0xff;

		// midi sync
		if (mix->is_syncing_midi_transport) {
			if (status == 0xf2) {
				// set song position pointer
				int spp = data1 | (data2 << 7);
				mix->on_set_position(mix->orderlist_position, spp);
			} else
			if (status == 0xfa) {
				// midi start
				// get pattern in the orderlist, get pattern loop start
				int spp = 0;
				if (mix->orderlist_position < (int)mix->songinfo.top_item(0).orderlist.size())  {
					int patid = mix->songinfo.top_item(0).orderlist[mix->orderlist_position];
					if (patid > 0 && patid < (int)mix->patterns.top().size()) {
						zzub::pattern* pat = mix->patterns.top()[patid].get();
						if (pat)
							spp = pat->beginloop;
					}
				}
				mix->on_set_position(mix->orderlist_position, spp);
				mix->on_set_state(zzub::player_state_playing, -1);
			} else
			if (status == 0xfb) {
				// midi continue
				mix->on_set_state(zzub::player_state_playing, -1);
			} else
			if (status == 0xfc) {
				// midi stop
				mix->on_set_state(zzub::player_state_stopped, -1);
			}
		}
	}
}

void player::midi_device_changed() {
	mix->output_midi_device_names.clear();
	if (!midiDriver) return ;

	for (int i = 0; i < (int)midiDriver->getDevices(); i++) {
		if (!midiDriver->isOpen(i)) continue;

		if (midiDriver->isOutput(i)) {
			const char* name = midiDriver->getDeviceName(i);
			mix->output_midi_device_names.push_back(std::pair<int, std::string>(i, name));
		}
	}

}

void player::update_document(armstrong::storage::document_event_data* e) {
	using namespace armstrong::storage;

	switch (e->type) {
		case event_type_insert_song:
			on_insert_song(e);
			break;
		case event_type_delete_song:
			on_delete_song(e);
			break;
		case event_type_update_song:
			on_update_song(e);
			break;
		case event_type_insert_plugin:
			on_insert_plugin(e);
			break;
		case event_type_before_delete_plugin:
			on_before_delete_plugin(e);
			break;
		case event_type_delete_plugin:
			on_delete_plugin(e);
			break;
		case event_type_update_plugin:
			on_update_plugin(e);
			break;
		case event_type_insert_parameterinfo:
			on_insert_parameterinfo(e);
			break;
		case event_type_delete_parameterinfo:
			on_delete_parameterinfo(e);
			break;
		case event_type_update_parameterinfo:
			on_update_parameterinfo(e);
			break;
		case event_type_insert_pluginparameter:
			on_insert_pluginparameter(e);
			break;
		case event_type_delete_pluginparameter:
			on_delete_pluginparameter(e);
			break;
		case event_type_update_pluginparameter:
			on_update_pluginparameter(e);
			break;
		case event_type_insert_attribute:
			on_insert_attribute(e);
			break;
		case event_type_delete_attribute:
			on_delete_attribute(e);
			break;
		case event_type_update_attribute:
			on_update_attribute(e);
			break;
		case event_type_insert_pattern:
			on_insert_pattern(e);
			break;
		case event_type_delete_pattern:
			on_delete_pattern(e);
			break;
		case event_type_update_pattern:
			on_update_pattern(e);
			break;
		case event_type_insert_patternformat:
			on_insert_patternformat(e);
			break;
		case event_type_delete_patternformat:
			on_delete_patternformat(e);
			break;
		case event_type_update_patternformat:
			on_update_patternformat(e);
			break;
		case event_type_insert_patternformatcolumn:
			on_insert_patternformatcolumn(e);
			break;
		case event_type_delete_patternformatcolumn:
			on_delete_patternformatcolumn(e);
			break;
		case event_type_update_patternformatcolumn:
			on_update_patternformatcolumn(e);
			break;
		case event_type_insert_patternformattrack:
			on_insert_patternformattrack(e);
			break;
		case event_type_update_patternformattrack:
			on_update_patternformattrack(e);
			break;
		case event_type_delete_patternformattrack:
			on_delete_patternformattrack(e);
			break;
/*		case event_type_insert_patternformatcolumnfilter:
			on_insert_patternformatcolumnfilter(e);
			break;
		case event_type_update_patternformatcolumnfilter:
			on_update_patternformatcolumnfilter(e);
			break;
		case event_type_delete_patternformatcolumnfilter:
			on_delete_patternformatcolumnfilter(e);
			break;*/
		case event_type_insert_patternevent:
			on_insert_patternevent(e);
			break;
		case event_type_delete_patternevent:
			on_delete_patternevent(e);
			break;
		case event_type_update_patternevent:
			on_update_patternevent(e);
			break;
		case event_type_insert_connection:
			on_insert_connection(e);
			break;
		case event_type_delete_connection:
			on_delete_connection(e);
			break;
		case event_type_update_connection:
			on_update_connection(e);
			break;
		case event_type_insert_eventconnectionbinding:
			on_insert_eventconnectionbinding(e);
			break;
		case event_type_update_eventconnectionbinding:
			on_update_eventconnectionbinding(e);
			break;
		case event_type_delete_eventconnectionbinding:
			on_delete_eventconnectionbinding(e);
			break;
		case event_type_insert_patternorder:
			on_insert_patternorder(e);
			break;
		case event_type_delete_patternorder:
			on_delete_patternorder(e);
			break;
		case event_type_update_patternorder:
			on_update_patternorder(e);
			break;
		case event_type_insert_midimapping:
			on_insert_midimapping(e);
			break;
		case event_type_delete_midimapping:
			on_delete_midimapping(e);
			break;
		case event_type_update_midimapping:
			on_update_midimapping(e);
			break;
		case event_type_insert_wave:
			on_insert_wave(e);
			break;
		case event_type_delete_wave:
			on_delete_wave(e);
			break;
		case event_type_update_wave:
			on_update_wave(e);
			break;
		case event_type_insert_wavelevel:
			on_insert_wavelevel(e);
			break;
		case event_type_delete_wavelevel:
			on_delete_wavelevel(e);
			break;
		case event_type_update_wavelevel:
			on_update_wavelevel(e);
			break;
		case event_type_insert_slice:
			on_insert_slice(e);
			break;
		case event_type_delete_slice:
			on_delete_slice(e);
			break;
		case event_type_update_slice:
			on_update_slice(e);
			break;
		case event_type_insert_envelope:
			on_insert_envelope(e);
			break;
		case event_type_delete_envelope:
			on_delete_envelope(e);
			break;
		case event_type_update_envelope:
			on_update_envelope(e);
			break;
		case event_type_insert_plugingroup:
			on_insert_plugingroup(e);
			break;
		case event_type_delete_plugingroup:
			on_delete_plugingroup(e);
			break;
		case event_type_update_plugingroup:
			on_update_plugingroup(e);
			break;

		case event_type_insert_samples:
			on_insert_samples(e);
			break;
		case event_type_delete_samples:
			on_delete_samples(e);
			break;
		case event_type_barrier:
			on_barrier(e);
			break;
//		case event_type_open_document:
//			on_open_document(e);
//			break;
		case event_type_orderlist_timeshift:
			on_orderlist_timeshift(e);
			break;
		case event_type_ensure_plugin_parameters:
			on_ensure_pluginparameters(e);
			break;
	}
}

void player::on_barrier(armstrong::storage::document_event_data* e) {
	cout << "player: barrier" << endl;

	mix->commit();

	zzub_event_data_t data;
	data.type = zzub_event_type_barrier;
	invoke_player_event(data);

	automated_pattern = false;
	event_userdata = 0;
}

void get_order_patterns(armstrong::storage::song* song, std::vector<int>& orderlist) {
	armstrong::storage::tableiterator patterns;
	song->get_order_patterns(&patterns);
	while (!patterns.eof()) {
		armstrong::storage::patterndata pat;
		orderlist.push_back(patterns.id());
		patterns.next();
	}
	patterns.destroy();
}

void player::on_insert_song(armstrong::storage::document_event_data* e) {
	cout << "player: insert song " << e->id << endl;

	doc->get_song(*songdata.get());

	mix->set_tpb(songdata->tpb);
	mix->set_bpm(songdata->bpm);
	mix->set_swing(songdata->swing);
	mix->set_swing_ticks(songdata->swingticks);

	std::vector<int> orderlist;
	get_order_patterns(songdata.get(), orderlist);
	mix->update_song(songdata->loopbegin, songdata->loopend, songdata->loopenabled ? 1 : 0, orderlist);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_song;
	invoke_player_event(data);
}

void player::on_delete_song(armstrong::storage::document_event_data* e) {
	cout << "player: delete song " << e->id << " (no-op)" << endl;
}

void player::on_update_song(armstrong::storage::document_event_data* e) {
	cout << "player: update song " << e->id << endl;

	doc->get_song(*songdata.get());

	mix->set_tpb(songdata->tpb);
	mix->set_bpm(songdata->bpm);
	mix->set_swing(songdata->swing);
	mix->set_swing_ticks(songdata->swingticks);

	std::vector<int> orderlist;
	get_order_patterns(songdata.get(), orderlist);
	mix->update_song(songdata->loopbegin, songdata->loopend, songdata->loopenabled ? 1 : 0, orderlist);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_song;
	invoke_player_event(data);
}

void player::on_insert_plugin(armstrong::storage::document_event_data* e) {
	cout << "player: insert plugin " << e->id << endl;
	
	armstrong::storage::plugin* dataplugin = new armstrong::storage::plugin(doc.get());
	songdata->get_plugin_by_id(e->id, *dataplugin);

	armstrong::storage::plugininfodata datainfo;
	songdata->get_plugininfo_by_plugin_id(dataplugin->id, datainfo);

	cout << "    plugin id: " << dataplugin->id << endl;
	cout << "    plugin name: " << dataplugin->name << endl;
	cout << "    plugin uri: " << datainfo.uri << endl;
	cout << "    tracks: " << dataplugin->trackcount << " (min " << datainfo.mintracks << ", max " << datainfo.maxtracks << ")" << endl;
	cout << "    flags: " << dataplugin->flags << endl;
	cout << "    streamsource: " << dataplugin->streamsource << endl;

	zzub::plugin* userplugin = 0;
	zzub::info* loader = plugmgr.plugin_get_info(datainfo.uri);

	// load parameters from the db into a dummyplugininfo for comparing/testing
	dummyinfo testinfo;
	testinfo.load_from_db(this, datainfo.id);

	if (loader != 0) {
		bool validated = testinfo.test_plugin(loader, load_errors);

		if (!validated) {
			loader = plugmgr.dummy_plugins.clone(this, &testinfo);
			userplugin = loader->create_plugin();

			validationerror err;
			err.type = zzub_validation_error_type_plugin_validation_failed_using_dummy;
			err.description = "Error: Using dummy plugin.";
			err.info = loader;
			err.original_plugin_name = dataplugin->name;
			load_errors.push_back(err);
		} else {
			userplugin = loader->create_plugin();

			if (userplugin == 0) {
				loader = plugmgr.dummy_plugins.clone(this, &testinfo);
				userplugin = loader->create_plugin();

				validationerror err;
				err.type = zzub_validation_error_type_plugin_not_found_using_dummy;
				err.description = "Error: Using dummy plugin.";
				err.info = loader;
				err.original_plugin_name = dataplugin->name;
				load_errors.push_back(err);
			}
		}
	} else {
		loader = plugmgr.dummy_plugins.clone(this, &testinfo);
		userplugin = loader->create_plugin();

		validationerror err;
		err.type = zzub_validation_error_type_plugin_not_found_using_dummy;
		err.description = "Error: Using dummy plugin.";
		err.info = loader;
		err.original_plugin_name = dataplugin->name;
		load_errors.push_back(err);
	}

	assert(userplugin != 0);

	armstrong::frontend::plugin* frontendplugin = new armstrong::frontend::plugin();
	frontendplugin->owner = this;
	frontendplugin->loader = loader;
	frontendplugin->dataplugin = boost::shared_ptr<armstrong::storage::plugin>(dataplugin);
	frontendplugin->userplugin = userplugin;

	if ((int)plugins.size() <= e->id)
		plugins.resize(e->id + 1);
	plugins[e->id] = boost::shared_ptr<plugin>(frontendplugin);

	userplugin->_player = this;
	userplugin->_plugin = frontendplugin;

	// insert into the mixer
	mix->insert_plugin(e->id, userplugin, loader, dataplugin->data, dataplugin->name, dataplugin->trackcount, dataplugin->streamsource, dataplugin->is_muted != 0, dataplugin->is_bypassed != 0, dataplugin->timesource_plugin_id, dataplugin->timesource_group, dataplugin->timesource_track);

	// send document event as mixer event
	zzub_event_data_t data;
	data.type = zzub_event_type_insert_plugin;
	data.insert_plugin.plugin = plugins[dataplugin->id].get();
	invoke_player_event(data);
}

void player::on_delete_plugin(armstrong::storage::document_event_data* e) {
	cout << "player: delete plugin " << e->id << endl;

	if (mix->midi_plugin == e->id && midi_lock)
		midi_lock = false;

	mix->delete_plugin(e->id);

	// send document event as mixer event
	zzub_event_data_t data;
	data.type = zzub_event_type_delete_plugin;
	data.delete_plugin.plugin = plugins[e->id].get();
	invoke_player_event(data);

	plugins[e->id].reset();

}

void player::on_before_delete_plugin(armstrong::storage::document_event_data* e) {
	// send document event as mixer event
	zzub_event_data_t data;
	data.type = zzub_event_type_before_delete_plugin;
	data.delete_plugin.plugin = plugins[e->id].get();
	invoke_player_event(data);
}

void player::on_update_plugin(armstrong::storage::document_event_data* e) {
	cout << "player: update plugin " << e->id << endl;

	armstrong::storage::plugin& plugdata = *plugins[e->id]->dataplugin;
	songdata->get_plugin_by_id(e->id, plugdata);

	mix->update_plugin(e->id, plugdata.name, plugdata.trackcount, plugdata.streamsource, plugdata.is_muted != 0, plugdata.is_bypassed != 0, plugdata.timesource_plugin_id, plugdata.timesource_group, plugdata.timesource_track);

	mix->set_plugin_latency(e->id, plugdata.latency);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_update_plugin;
	data.update_plugin.plugin = plugins[e->id].get();
	invoke_player_event(data);
}

void player::on_insert_parameterinfo(armstrong::storage::document_event_data* e) {

	armstrong::storage::parameterinfodata* dataparaminfo = new armstrong::storage::parameterinfodata(*e->newdata.parameterinfo);

	armstrong::frontend::parameterinfo* frontendparaminfo = new armstrong::frontend::parameterinfo();
	frontendparaminfo->owner = this;
	frontendparaminfo->dataparameterinfo = boost::shared_ptr<armstrong::storage::parameterinfodata>(dataparaminfo);

	if ((int)parameterinfos.size() <= e->id)
		parameterinfos.resize(e->id + 1);
	parameterinfos[e->id] = boost::shared_ptr<parameterinfo>(frontendparaminfo);
}

void player::on_delete_parameterinfo(armstrong::storage::document_event_data* e) {
	parameterinfos[e->id].reset();
}

void player::on_update_parameterinfo(armstrong::storage::document_event_data* e) {
	// shouldnt happen - theres nothing to update here at runtime
}

void player::on_insert_pluginparameter(armstrong::storage::document_event_data* e) {
	//cout << "player: insert pluginparameter " << e->id << endl;

	armstrong::storage::pluginparameter* dataparam = new armstrong::storage::pluginparameter(doc.get());
	songdata->get_pluginparameter_by_id(e->id, *dataparam);

	armstrong::frontend::pluginparameter* frontendparam = new armstrong::frontend::pluginparameter(); 
	frontendparam->owner = this;
	frontendparam->datapluginparameter = boost::shared_ptr<armstrong::storage::pluginparameter>(dataparam);

	if ((int)pluginparameters.size() <= e->id)
		pluginparameters.resize(e->id + 1);
	pluginparameters[e->id] = boost::shared_ptr<pluginparameter>(frontendparam);

	on_update_pluginparameter(e);
}

void player::on_delete_pluginparameter(armstrong::storage::document_event_data* e) {
	cout << "player: delete pluginparameter " << e->id << endl;

	pluginparameters[e->id].reset();
}

void player::on_update_pluginparameter(armstrong::storage::document_event_data* e) {
	cout << "player: insert/update pluginparameter " << e->id << endl;
	armstrong::storage::pluginparameterdata& ppdata = *pluginparameters[e->id]->datapluginparameter;
	//songdata->get_pluginparameter_by_id(e->id, ppdata);
	ppdata = *e->newdata.pluginparameter;

	armstrong::storage::parameterinfodata& ppinfo = *parameterinfos[ppdata.parameterinfo_id]->dataparameterinfo;

	mix->set_parameter_mode(ppdata.plugin_id, ppinfo.paramgroup, ppdata.paramtrack, ppinfo.paramcolumn, ppdata.interpolator);
	mix->set_parameter(ppdata.plugin_id, ppinfo.paramgroup, ppdata.paramtrack, ppinfo.paramcolumn, ppdata.value, true);

	zzub_event_data_t zzubdata;
	zzubdata.type = zzub_event_type_update_pluginparameter;
	zzubdata.update_pluginparameter.plugin = plugins[ppdata.plugin_id].get();
	zzubdata.update_pluginparameter.group = ppinfo.paramgroup;
	zzubdata.update_pluginparameter.track = ppdata.paramtrack;
	zzubdata.update_pluginparameter.param = ppinfo.paramcolumn;
	zzubdata.update_pluginparameter.value = ppdata.value;
	invoke_player_event(zzubdata);
}

void player::on_insert_attribute(armstrong::storage::document_event_data* e) {
	
	armstrong::storage::attributedata* dataattr = new armstrong::storage::attributedata();
	//songdata->get_attribute_by_id(e->id, *dataattr);

	armstrong::frontend::attribute* frontendattr = new armstrong::frontend::attribute();
	frontendattr->owner = this;
	frontendattr->dataattribute = boost::shared_ptr<armstrong::storage::attributedata>(dataattr);

	if ((int)attributes.size() <= e->id)
		attributes.resize(e->id + 1);
	attributes[e->id] = boost::shared_ptr<attribute>(frontendattr);

	on_update_attribute(e);
}

void player::on_update_attribute(armstrong::storage::document_event_data* e) {
	cout << "player: insert/update attribute " << e->id << endl;
	armstrong::frontend::attribute* attr = attributes[e->id].get();
	assert(attr != 0);
	songdata->get_attribute_by_id(e->id, *attr->dataattribute);

	armstrong::frontend::plugin* plugin = plugins[attr->dataattribute->plugin_id].get();
	assert(plugin != 0);
	if (attr->dataattribute->attrindex < (int)plugin->loader->attributes.size()) {
		mix->set_attribute(attr->dataattribute->plugin_id, attr->dataattribute->attrindex, attr->dataattribute->value);

		// send event - TODO: send specific attribute event?
		zzub_event_data_t data;
		data.type = zzub_event_type_update_plugin;
		data.update_plugin.plugin = plugin;
		invoke_player_event(data);
	}
}

void player::on_delete_attribute(armstrong::storage::document_event_data* e) {
	cout << "player: delete attribute " << e->id << endl;

	armstrong::frontend::attribute* attr = attributes[e->id].get();
	assert(attr != 0);

	armstrong::frontend::plugin* plugin = plugins[attr->dataattribute->plugin_id].get();
	assert(plugin != 0);
	if (attr->dataattribute->attrindex < (int)plugin->loader->attributes.size()) {

		mix->set_attribute(attr->dataattribute->plugin_id, attr->dataattribute->attrindex, plugin->loader->attributes[attr->dataattribute->attrindex]->value_default);

		// send event - TODO: send specific attribute event?
		zzub_event_data_t data;
		data.type = zzub_event_type_update_plugin;
		data.update_plugin.plugin = plugin;
		invoke_player_event(data);
	}

	attributes[e->id].reset();
}

void player::on_insert_pattern(armstrong::storage::document_event_data* e) {

	armstrong::storage::pattern* datapatt = new armstrong::storage::pattern(doc.get());
	songdata->get_pattern_by_id(e->id, *datapatt);

	armstrong::frontend::pattern* frontendpattern = new armstrong::frontend::pattern();
	frontendpattern->owner = this;
	frontendpattern->datapattern = boost::shared_ptr<armstrong::storage::pattern>(datapatt);
	cout << "player: insert pattern " << e->id << endl;

	if ((int)patterns.size() <= e->id)
		patterns.resize(e->id + 1);
	patterns[e->id] = boost::shared_ptr<armstrong::frontend::pattern>(frontendpattern);

	mix->insert_pattern(e->id, datapatt->patternformat_id, datapatt->name, datapatt->length, datapatt->resolution, datapatt->beginloop, datapatt->endloop, datapatt->loopenabled != 0);

	zzub_event_data_t data;
	data.type = zzub_event_type_insert_pattern;
	data.insert_pattern.pattern = frontendpattern;
	invoke_player_event(data);
}

void player::on_delete_pattern(armstrong::storage::document_event_data* e) {
	cout << "player: delete pattern " << e->id << endl;

	armstrong::storage::pattern datapatt(doc.get());
	songdata->get_pattern_by_id(e->id, datapatt);

	mix->delete_pattern(e->id);

	zzub_event_data_t data;
	data.type = zzub_event_type_delete_pattern;
	data.delete_pattern.pattern = patterns[e->id].get();
	invoke_player_event(data);

	patterns[e->id].reset();
}

void player::on_update_pattern(armstrong::storage::document_event_data* e) {
	cout << "player: update pattern " << e->id << endl;

	pattern* frontendpatt = patterns[e->id].get();
	songdata->get_pattern_by_id(e->id, *frontendpatt->datapattern);
	mix->update_pattern(e->id, frontendpatt->datapattern->patternformat_id, frontendpatt->datapattern->name, frontendpatt->datapattern->length, frontendpatt->datapattern->resolution, frontendpatt->datapattern->beginloop, frontendpatt->datapattern->endloop, frontendpatt->datapattern->loopenabled != 0);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_pattern;
	data.update_pattern.pattern = frontendpatt;
	invoke_player_event(data);
}

void player::on_insert_patternevent(armstrong::storage::document_event_data* e) {
	//cout << "player: insert patternevent " << e->id << endl;

	armstrong::storage::patternevent* dataevent = new armstrong::storage::patternevent(doc.get());
	*dataevent = *e->newdata.patternevent;

	armstrong::frontend::patternevent* frontendevent = new armstrong::frontend::patternevent();
	frontendevent->owner = this;
	frontendevent->datapatternevent = boost::shared_ptr<armstrong::storage::patternevent>(dataevent);

	if ((int)patternevents.size() <= e->id)
		patternevents.resize(e->id + 1);
	patternevents[e->id] = boost::shared_ptr<armstrong::frontend::patternevent>(frontendevent);

	armstrong::frontend::pluginparameter* param = pluginparameters[dataevent->pluginparameter_id].get();
	assert(param != 0);
	armstrong::frontend::parameterinfo* paraminfo = parameterinfos[param->datapluginparameter->parameterinfo_id].get();
	assert(param != 0);
    
	mix->insert_pattern_value(e->id, dataevent->pattern_id, param->datapluginparameter->plugin_id, paraminfo->dataparameterinfo->paramgroup, param->datapluginparameter->paramtrack, paraminfo->dataparameterinfo->paramcolumn, dataevent->time, dataevent->value);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_insert_patternevent;
	data.insert_patternevent.patternevent = patternevents[e->id].get();
	invoke_player_event(data);
}

void player::on_delete_patternevent(armstrong::storage::document_event_data* e) {
	//cout << "player: delete patternevent " << e->id << endl;

	armstrong::frontend::patternevent* frontendevent = patternevents[e->id].get();
	mix->remove_pattern_value(e->id, frontendevent->datapatternevent->pattern_id);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_delete_patternevent;
	data.delete_patternevent.patternevent = patternevents[e->id].get();
	invoke_player_event(data);

	// clear event
	patternevents[e->id].reset();
}

void player::on_update_patternevent(armstrong::storage::document_event_data* e) {
	//cout << "player: insert/delete/update patternevent " << e->id << endl;

	patternevent* frontendevent = patternevents[e->id].get();
	*frontendevent->datapatternevent = *e->newdata.patternevent;
	armstrong::storage::patternevent* dataevent = frontendevent->datapatternevent.get();

	pluginparameter* frontendparam = pluginparameters[dataevent->pluginparameter_id].get();
	parameterinfo* frontendinfo = parameterinfos[frontendparam->datapluginparameter->parameterinfo_id].get();

	mix->update_pattern_value(e->id, dataevent->pattern_id, 
		frontendparam->datapluginparameter->plugin_id, 
		frontendinfo->dataparameterinfo->paramgroup, frontendparam->datapluginparameter->paramtrack, 
		frontendinfo->dataparameterinfo->paramcolumn, dataevent->time, dataevent->value);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_update_patternevent;
	data.update_patternevent.patternevent = patternevents[e->id].get();
	invoke_player_event(data);
}

void player::on_insert_patternformat(armstrong::storage::document_event_data* e) {
	cout << "player: insert patternformat " << e->id << endl;
	
	armstrong::storage::patternformat* dataformat = new armstrong::storage::patternformat(doc.get());
	songdata->get_patternformat_by_id(e->id, *dataformat);

	armstrong::frontend::patternformat* frontendformat = new armstrong::frontend::patternformat();
	frontendformat->owner = this;
	frontendformat->dataformat = boost::shared_ptr<armstrong::storage::patternformat>(dataformat);

	if ((int)patternformats.size() <= e->id) patternformats.resize(e->id + 1);
	patternformats[e->id] = boost::shared_ptr<armstrong::frontend::patternformat>(frontendformat);

	mix->insert_patternformat(e->id);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_insert_patternformat;
	data.insert_pattern_format.patternformat = patternformats[e->id].get();
	invoke_player_event(data);
}

void player::on_update_patternformat(armstrong::storage::document_event_data* e) {
	cout << "player: update patternformat " << e->id << endl;

	patternformat* frontendformat = patternformats[e->id].get();
	songdata->get_patternformat_by_id(e->id, *frontendformat->dataformat);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_patternformat;
	data.update_pattern_format.patternformat = patternformats[e->id].get();
	invoke_player_event(data);
}

void player::on_delete_patternformat(armstrong::storage::document_event_data* e) {
	cout << "player: delete patternformat " << e->id << endl;

	mix->delete_patternformat(e->id);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_delete_patternformat;
	data.delete_pattern_format.patternformat = patternformats[e->id].get();
	invoke_player_event(data);

	patternformats[e->id].reset();

}

void player::on_insert_patternformatcolumn(armstrong::storage::document_event_data* e) {
	cout << "player: insert patternformatcolumn " << e->id << endl;
	
	armstrong::storage::patternformatcolumn* datacolumn = new armstrong::storage::patternformatcolumn(doc.get());
	//TODO: *datacolumn = *e->newdata.patternformatcolumn;
	songdata->get_patternformatcolumn_by_id(e->id, *datacolumn);

	armstrong::frontend::patternformatcolumn* frontendcolumn = new armstrong::frontend::patternformatcolumn();
	frontendcolumn->owner = this;
	frontendcolumn->datacolumn = boost::shared_ptr<armstrong::storage::patternformatcolumn>(datacolumn);

	if ((int)patternformatcolumns.size() <= e->id) patternformatcolumns.resize(e->id + 1);
	patternformatcolumns[e->id] = boost::shared_ptr<armstrong::frontend::patternformatcolumn>(frontendcolumn);

	armstrong::frontend::pluginparameter* param = pluginparameters[datacolumn->pluginparameter_id].get();
	assert(param != 0);
	armstrong::frontend::parameterinfo* paraminfo = parameterinfos[param->datapluginparameter->parameterinfo_id].get();
	assert(paraminfo != 0);

	mix->insert_patternformatcolumn(e->id, datacolumn->patternformat_id, param->datapluginparameter->plugin_id, paraminfo->dataparameterinfo->paramgroup, param->datapluginparameter->paramtrack, paraminfo->dataparameterinfo->paramcolumn);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_insert_patternformatcolumn;
	data.insert_pattern_format_column.patternformatcolumn = patternformatcolumns[e->id].get();
	invoke_player_event(data);

}

void player::on_update_patternformatcolumn(armstrong::storage::document_event_data* e) {
	cout << "player: update patternformatcolumn " << e->id << endl;

	patternformatcolumn* frontendcolumn = patternformatcolumns[e->id].get();
	songdata->get_patternformatcolumn_by_id(e->id, *frontendcolumn->datacolumn);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_patternformatcolumn;
	data.update_pattern_format_column.patternformatcolumn = patternformatcolumns[e->id].get();
	invoke_player_event(data);
}

void player::on_delete_patternformatcolumn(armstrong::storage::document_event_data* e) {
	cout << "player: delete patternformatcolumn " << e->id << endl;

	mix->delete_patternformatcolumn(e->id);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_delete_patternformatcolumn;
	data.delete_pattern_format_column.patternformatcolumn = patternformatcolumns[e->id].get();
	invoke_player_event(data);

	patternformatcolumns[e->id].reset();
}

void player::on_insert_patternformatcolumnfilter(armstrong::storage::document_event_data* e) {
/*	armstrong::storage::patternformatcolumnfilterdata* datafilter = new armstrong::storage::patternformatcolumnfilterdata();
	songdata->get_patternformatcolumnfilter_by_id(e->id, *datafilter);

	armstrong::frontend::patternformatcolumnfilter* frontendfilter = new armstrong::frontend::patternformatcolumnfilter();
	frontendfilter->owner = this;
	frontendfilter->datafilter = boost::shared_ptr<armstrong::storage::patternformatcolumnfilterdata>(datafilter);

	if ((int)patternformatcolumnfilters.size() <= e->id) patternformatcolumnfilters.resize(e->id + 1);
	patternformatcolumnfilters[e->id] = boost::shared_ptr<armstrong::frontend::patternformatcolumnfilter>(frontendfilter);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_patternformatcolumn;
	data.update_pattern_format_column.patternformatcolumn = patternformatcolumns[frontendfilter->datafilter->patternformatcolumn_id].get();;
	invoke_player_event(data);*/
	assert(false);
}

void player::on_update_patternformatcolumnfilter(armstrong::storage::document_event_data* e) {
/*	armstrong::frontend::patternformatcolumnfilter* frontendfilter = patternformatcolumnfilters[e->id].get();

	zzub_event_data_t data;
	data.type = zzub_event_type_update_patternformatcolumn;
	data.update_pattern_format_column.patternformatcolumn = patternformatcolumns[frontendfilter->datafilter->patternformatcolumn_id].get();;
	invoke_player_event(data);*/
	assert(false);
}

void player::on_delete_patternformatcolumnfilter(armstrong::storage::document_event_data* e) {
/*	armstrong::frontend::patternformatcolumnfilter* frontendfilter = patternformatcolumnfilters[e->id].get();

	zzub_event_data_t data;
	data.type = zzub_event_type_update_patternformatcolumn;
	data.update_pattern_format_column.patternformatcolumn = patternformatcolumns[frontendfilter->datafilter->patternformatcolumn_id].get();;
	invoke_player_event(data);

	patternformatcolumnfilters[e->id].reset();*/
	assert(false);
}

void player::on_insert_patternformattrack(armstrong::storage::document_event_data* e) {
	cout << "player: insert patternformattrack " << e->id << endl;
	armstrong::storage::patternformattrack* datatrack = new armstrong::storage::patternformattrack(doc.get());
	songdata->get_patternformattrack_by_id(e->id, *datatrack);

	armstrong::frontend::patternformattrack* frontendtrack = new armstrong::frontend::patternformattrack();
	frontendtrack->owner = this;
	frontendtrack->datatrack = boost::shared_ptr<armstrong::storage::patternformattrack>(datatrack);

	if ((int)patternformattracks.size() <= e->id) patternformattracks.resize(e->id + 1);
	patternformattracks[e->id] = boost::shared_ptr<armstrong::frontend::patternformattrack>(frontendtrack);

	mix->insert_patternformattrack(e->id, datatrack->patternformat_id, datatrack->plugin_id, datatrack->paramgroup, datatrack->paramtrack, datatrack->is_muted);

	zzub_event_data_t data;
	data.type = zzub_event_type_insert_patternformattrack;
	invoke_player_event(data);
}

void player::on_update_patternformattrack(armstrong::storage::document_event_data* e) {
	cout << "player: update patternformattrack " << e->id << endl;
	patternformattrack* frontendtrack = patternformattracks[e->id].get();
	songdata->get_patternformattrack_by_id(e->id, *frontendtrack->datatrack);

	armstrong::storage::patternformattrack* datatrack = frontendtrack->datatrack.get();

	mix->update_patternformattrack(e->id, datatrack->patternformat_id, datatrack->plugin_id, datatrack->paramgroup, datatrack->paramtrack, datatrack->is_muted);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_patternformattrack;
	invoke_player_event(data);
}

void player::on_delete_patternformattrack(armstrong::storage::document_event_data* e) {
	cout << "player: delete patternformattrack " << e->id << endl;
	patternformattrack* frontendtrack = patternformattracks[e->id].get();

	mix->delete_patternformattrack(e->id);

	zzub_event_data_t data;
	data.type = zzub_event_type_delete_patternformattrack;
	invoke_player_event(data);

	patternformattracks[e->id].reset();
}

void player::on_insert_connection(armstrong::storage::document_event_data* e) {
	cout << "player: insert connection " << e->id << endl;

	armstrong::storage::connection* dataconn = new armstrong::storage::connection(doc.get());
	songdata->get_connection_by_id(e->id, *dataconn);

	armstrong::frontend::connection* frontendconn = new armstrong::frontend::connection();
	frontendconn->owner = this;
	frontendconn->dataconn = boost::shared_ptr<armstrong::storage::connection>(dataconn);

	if ((int)connections.size() <= e->id) connections.resize(e->id + 1);
	connections[e->id] = boost::shared_ptr<armstrong::frontend::connection>(frontendconn);

	mix->insert_connection(e->id, dataconn->plugin_id, dataconn->from_plugin_id, dataconn->to_plugin_id, (zzub_connection_type)dataconn->type, dataconn->first_input, dataconn->first_output, dataconn->input_count, dataconn->output_count, dataconn->mididevice);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_insert_connection;
	data.insert_connection.connection_plugin = plugins[dataconn->plugin_id].get();
	data.insert_connection.from_plugin = plugins[dataconn->from_plugin_id].get();
	data.insert_connection.to_plugin = plugins[dataconn->to_plugin_id].get();
	invoke_player_event(data);
}

void player::on_delete_connection(armstrong::storage::document_event_data* e) {
	cout << "player: delete connection " << e->id << endl;
	mix->delete_connection(e->id);

	armstrong::frontend::connection* conn = connections[e->id].get();

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_delete_connection;
	data.delete_connection.connection_plugin = plugins[conn->dataconn->plugin_id].get();
	data.delete_connection.from_plugin = plugins[conn->dataconn->from_plugin_id].get();
	data.delete_connection.to_plugin = plugins[conn->dataconn->to_plugin_id].get();
	invoke_player_event(data);

	connections[e->id].reset();
}

void player::on_update_connection(armstrong::storage::document_event_data* e) {
	cout << "player: update connection " << e->id << endl;

	armstrong::frontend::connection* conn = connections[e->id].get();

	switch (conn->dataconn->type) {
		case zzub_connection_type_audio:
			mix->update_audioconnection(conn->dataconn->id, conn->dataconn->first_input, conn->dataconn->first_output, conn->dataconn->input_count, conn->dataconn->output_count, 0);
			break;
		case zzub_connection_type_midi:
			mix->update_midiconnection(conn->dataconn->id, conn->dataconn->mididevice);
			break;
		case zzub_connection_type_event:
			break;
	}

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_update_connection;
	data.update_connection.connection_plugin = plugins[conn->dataconn->plugin_id].get();
	data.update_connection.from_plugin = plugins[conn->dataconn->from_plugin_id].get();
	data.update_connection.to_plugin = plugins[conn->dataconn->to_plugin_id].get();
	invoke_player_event(data);

}

void player::on_insert_eventconnectionbinding(armstrong::storage::document_event_data* e) {
	armstrong::storage::eventconnectionbindingdata* databinding = new armstrong::storage::eventconnectionbindingdata();
	songdata->get_eventconnectionbinding_by_id(e->id, *databinding);

	eventconnectionbinding* frontendbinding = new eventconnectionbinding();
	frontendbinding->owner = this;
	frontendbinding->databinding = boost::shared_ptr<armstrong::storage::eventconnectionbindingdata>(databinding);

	if ((int)eventconnectionbindings.size() <= e->id) eventconnectionbindings.resize(e->id + 1);
	eventconnectionbindings[e->id] = boost::shared_ptr<armstrong::frontend::eventconnectionbinding>(frontendbinding);

	mix->add_event_connection_binding(databinding->connection_id, 
		databinding->sourceindex, databinding->targetparamgroup, 
		databinding->targetparamtrack, databinding->targetparamcolumn);

}

void player::on_delete_eventconnectionbinding(armstrong::storage::document_event_data* e) {

	eventconnectionbinding* frontendbinding = eventconnectionbindings[e->id].get();
	armstrong::storage::eventconnectionbindingdata* databinding = frontendbinding->databinding.get();

	mix->delete_event_connection_binding(databinding->connection_id, 
		databinding->sourceindex, databinding->targetparamgroup, 
		databinding->targetparamtrack, databinding->targetparamcolumn);

	eventconnectionbindings[e->id].reset();
}

void player::on_update_eventconnectionbinding(armstrong::storage::document_event_data* e) {
	assert(false);
}

void player::on_insert_patternorder(armstrong::storage::document_event_data* e) {

	armstrong::storage::patternorderdata* dataorder = new armstrong::storage::patternorderdata();
	songdata->get_patternorder_by_id(e->id, *dataorder);

	armstrong::frontend::patternorder* frontendorder = new armstrong::frontend::patternorder();
	frontendorder->owner = this;
	frontendorder->dataorder = boost::shared_ptr<armstrong::storage::patternorderdata>(dataorder);

	if ((int)patternorders.size() <= e->id) patternorders.resize(e->id + 1);
	patternorders[e->id] = boost::shared_ptr<armstrong::frontend::patternorder>(frontendorder);

	on_update_song(e); // wrong id!
}

void player::on_delete_patternorder(armstrong::storage::document_event_data* e) {
	armstrong::frontend::patternorder* frontendorder = patternorders[e->id].get();

	on_update_song(e); // wrong id!
	patternorders[e->id].reset();
}

void player::on_update_patternorder(armstrong::storage::document_event_data* e) {
	armstrong::frontend::patternorder* frontendorder = patternorders[e->id].get();
	songdata->get_patternorder_by_id(e->id, *frontendorder->dataorder.get());

	on_update_song(e); // wrong id!
}

void player::on_insert_midimapping(armstrong::storage::document_event_data* e) {
	cout << "player: insert midimapping " << e->id << endl;

	armstrong::storage::midimapping* datamidimapping = new armstrong::storage::midimapping(doc.get());
	songdata->get_midimapping_by_id(e->id, *datamidimapping);

	armstrong::frontend::midimapping* frontendmidimapping = new armstrong::frontend::midimapping();
	frontendmidimapping->owner = this;
	frontendmidimapping->datamidimapping = boost::shared_ptr<armstrong::storage::midimapping>(datamidimapping);

	if ((int)midimappings.size() <= e->id) midimappings.resize(e->id + 1);

	midimappings[e->id] = boost::shared_ptr<armstrong::frontend::midimapping>(frontendmidimapping);

	mix->insert_midimapping(e->id, datamidimapping->plugin_id, datamidimapping->paramgroup, datamidimapping->paramtrack, datamidimapping->paramcolumn, datamidimapping->midichannel, datamidimapping->midicontroller);

	/*zzub_event_data_t data;
	data.type = zzub_event_type_insert_midimapping;
	data.insert_midimapping.midimapping = midimappings[e->id].get();
	invoke_player_event(data);*/

}

void player::on_delete_midimapping(armstrong::storage::document_event_data* e) {
	cout << "player: delete midimapping " << e->id << endl;

	mix->delete_midimapping(e->id);

	/*zzub_event_data_t data;
	data.type = zzub_event_type_delete_midimapping;
	data.delete_midimapping.midimapping = midimappings[e->id].get();
	invoke_player_event(data);*/

	midimappings[e->id].reset();

}

void player::on_update_midimapping(armstrong::storage::document_event_data* e) {
	cout << "player: update midimapping " << e->id << endl;
	armstrong::frontend::midimapping* frontendmidimapping = midimappings[e->id].get();

	songdata->get_midimapping_by_id(e->id, *frontendmidimapping->datamidimapping);

	//mix->update_midimapping();

	// send event
/*	zzub_event_data_t data;
	data.type = zzub_event_type_update_midimapping;
	data.update_midimapping.midimapping = midimappings[e->id].get();
	invoke_player_event(data);*/
}

void player::on_insert_wave(armstrong::storage::document_event_data* e) {
	cout << "player: insert wave " << e->id << endl;

	armstrong::storage::wave* datawave = new armstrong::storage::wave(doc.get());
	songdata->get_wave_by_id(e->id, *datawave);

	armstrong::frontend::wave* frontendwave = new armstrong::frontend::wave();
	frontendwave->owner = this;
	frontendwave->datawave = boost::shared_ptr<armstrong::storage::wave>(datawave);

	if ((int)waves.size() <= e->id) waves.resize(e->id + 1);

	waves[e->id] = boost::shared_ptr<armstrong::frontend::wave>(frontendwave);

	mix->insert_wave(e->id, datawave->flags, datawave->volume);

	zzub_event_data_t data;
	data.type = zzub_event_type_insert_wave;
	data.insert_wave.wave = waves[e->id].get();
	invoke_player_event(data);

}

void player::on_delete_wave(armstrong::storage::document_event_data* e) {
	cout << "player: delete wave " << e->id << endl;

	zzub_event_data_t data;
	data.type = zzub_event_type_delete_wave;
	data.delete_wave.wave = waves[e->id].get();
	invoke_player_event(data);

	waves[e->id].reset();

}

void player::on_update_wave(armstrong::storage::document_event_data* e) {
	cout << "player: update wave " << e->id << endl;
	armstrong::frontend::wave* frontendwave = waves[e->id].get();

	songdata->get_wave_by_id(e->id, *frontendwave->datawave);

	mix->update_wave(e->id, frontendwave->datawave->flags, frontendwave->datawave->volume);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_update_wave;
	data.update_wave.wave = waves[e->id].get();
	invoke_player_event(data);
}

void player::on_insert_wavelevel(armstrong::storage::document_event_data* e) {
	cout << "player: insert wavelevel " << e->id << endl;
	armstrong::storage::wavelevel* datawavelevel = new armstrong::storage::wavelevel(doc.get());
	songdata->get_wavelevel_by_id(e->id, *datawavelevel);

	armstrong::frontend::wavelevel* frontendwavelevel = new armstrong::frontend::wavelevel();
	frontendwavelevel->owner = this;
	frontendwavelevel->datawavelevel = boost::shared_ptr<armstrong::storage::wavelevel>(datawavelevel);

	if ((int)wavelevels.size() <= e->id) wavelevels.resize(e->id + 1);

	wavelevels[e->id] = boost::shared_ptr<armstrong::frontend::wavelevel>(frontendwavelevel);

	std::vector<int> slices;
	datawavelevel->get_slices(slices);
	mix->insert_wavelevel(e->id, datawavelevel->wave_id, datawavelevel->beginloop, datawavelevel->endloop, datawavelevel->samplecount, datawavelevel->samplerate, (zzub_wave_buffer_type)datawavelevel->format, datawavelevel->basenote, slices);

	zzub_event_data_t data;
	data.type = zzub_event_type_insert_wavelevel;
	data.insert_wavelevel.wavelevel = wavelevels[e->id].get();
	invoke_player_event(data);
}

void player::on_delete_wavelevel(armstrong::storage::document_event_data* e) {
	cout << "player: delete wavelevel " << e->id << endl;
	mix->delete_wavelevel(e->id);

	zzub_event_data_t data;
	data.type = zzub_event_type_delete_wavelevel;
	data.delete_wavelevel.wavelevel = wavelevels[e->id].get();
	invoke_player_event(data);

	wavelevels[e->id].reset();
}

void player::on_update_wavelevel(armstrong::storage::document_event_data* e) {
	cout << "player: update wavelevel " << e->id << endl;
	armstrong::frontend::wavelevel* frontendlevel = wavelevels[e->id].get();

	songdata->get_wavelevel_by_id(e->id, *frontendlevel->datawavelevel);

	armstrong::storage::wavelevel& waveleveldata = *frontendlevel->datawavelevel;

	std::vector<int> slices;
	waveleveldata.get_slices(slices);
	mix->update_wavelevel(e->id, waveleveldata.wave_id, waveleveldata.beginloop, waveleveldata.endloop, waveleveldata.samplecount, waveleveldata.samplerate, (zzub_wave_buffer_type)waveleveldata.format, waveleveldata.basenote, slices);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_wavelevel;
	data.update_wavelevel.wavelevel = wavelevels[e->id].get();
	invoke_player_event(data);
}

void player::on_insert_slice(armstrong::storage::document_event_data* e) {
	on_update_slice(e);
}

void player::on_delete_slice(armstrong::storage::document_event_data* e) {
	on_update_slice(e);
}

void player::on_update_slice(armstrong::storage::document_event_data* e) {
	int wavelevel_id = e->newdata.slice->wavelevel_id;

	// update the mixer wavelevel
	armstrong::frontend::wavelevel* frontendlevel = wavelevels[wavelevel_id].get();
	armstrong::storage::wavelevel& waveleveldata = *frontendlevel->datawavelevel;

	std::vector<int> slices;
	waveleveldata.get_slices(slices);
	mix->update_wavelevel(wavelevel_id, waveleveldata.wave_id, waveleveldata.beginloop, waveleveldata.endloop, waveleveldata.samplecount, waveleveldata.samplerate, (zzub_wave_buffer_type)waveleveldata.format, waveleveldata.basenote, slices);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_wavelevel;
	data.update_wavelevel.wavelevel = wavelevels[wavelevel_id].get();
	invoke_player_event(data);
}

void player::on_insert_envelope(armstrong::storage::document_event_data* e) {
	cout << "player: insert envelope " << e->id << endl;
	armstrong::storage::envelope* dataenvelope = new armstrong::storage::envelope(doc.get());
	songdata->get_envelope_by_id(e->id, *dataenvelope);

	armstrong::frontend::envelope* frontendenvelope = new armstrong::frontend::envelope();
	frontendenvelope->owner = this;
	frontendenvelope->dataenvelope = boost::shared_ptr<armstrong::storage::envelope>(dataenvelope);

	if ((int)envelopes.size() <= e->id) envelopes.resize(e->id + 1);

	envelopes[e->id] = boost::shared_ptr<armstrong::frontend::envelope>(frontendenvelope);
}

void player::on_update_envelope(armstrong::storage::document_event_data* e) {
	cout << "player: update envelope " << e->id << endl;
}

void player::on_delete_envelope(armstrong::storage::document_event_data* e) {
	cout << "player: delete envelope " << e->id << endl;

	envelopes[e->id].reset();
}

void player::on_insert_plugingroup(armstrong::storage::document_event_data* e) {
	cout << "player: insert plugingroup " << e->id << endl;
	armstrong::storage::plugingroup* dataplugingroup = new armstrong::storage::plugingroup(doc.get());
	songdata->get_plugingroup_by_id(e->id, *dataplugingroup);

	armstrong::frontend::plugingroup* frontendplugingroup = new armstrong::frontend::plugingroup();
	frontendplugingroup->owner = this;
	frontendplugingroup->dataplugingroup = boost::shared_ptr<armstrong::storage::plugingroup>(dataplugingroup);

	if ((int)plugingroups.size() <= e->id) plugingroups.resize(e->id + 1);

	plugingroups[e->id] = boost::shared_ptr<armstrong::frontend::plugingroup>(frontendplugingroup);

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_insert_plugin_group;
	data.insert_plugin_group.group = plugingroups[e->id].get();
	invoke_player_event(data);
}

void player::on_update_plugingroup(armstrong::storage::document_event_data* e) {
	cout << "player: update plugingroup " << e->id << endl;
	plugingroup* frontendplugingroup = plugingroups[e->id].get();
	*frontendplugingroup->dataplugingroup = *e->newdata.plugingroup;

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_update_plugin_group;
	data.update_plugin_group.group = plugingroups[e->id].get();
	invoke_player_event(data);
}

void player::on_delete_plugingroup(armstrong::storage::document_event_data* e) {
	cout << "player: delete plugingroup " << e->id << endl;

	// send event
	zzub_event_data_t data;
	data.type = zzub_event_type_delete_plugin_group;
	data.delete_plugin_group.group = plugingroups[e->id].get();
	invoke_player_event(data);

	plugingroups[e->id].reset();
}

void player::set_wavelevel(wavelevel* _wavelevel, bool legacyheader) {
	int length;
	unsigned char* samples;
	doc->wavelevel_get_samples(_wavelevel->datawavelevel->id, 0, &length, &samples, legacyheader);

	int bytes_per_sample = _wavelevel->datawavelevel->get_bytes_per_sample();
	int samplecount = length / bytes_per_sample;

	mix->set_wavelevel_samples(_wavelevel->datawavelevel->id, samples, samplecount, legacyheader);

	zzub_event_data_t data;
	data.type = zzub_event_type_update_wavelevel_samples;
	data.update_wavelevel_samples.wavelevel = _wavelevel;
	invoke_player_event(data);
}

void player::reset_wavelevel_samples(int wave, int wavelevel, int format, int channels) {
	armstrong::storage::wave datawave(doc.get());
	songdata->get_wave_by_index(wave, datawave);

	armstrong::storage::waveleveldata tempwavelevel;
	while (datawave.get_wavelevel_count() <= wavelevel)
		songdata->create_wavelevel(datawave.id, format, work_rate, tempwavelevel);

	armstrong::storage::wavelevel datawavelevel(doc.get());
	datawave.get_wavelevel_by_index(wavelevel, datawavelevel);

	if (datawavelevel.samplecount > 0)
		datawavelevel.delete_sample_range(0, datawavelevel.samplecount);

	datawave.set_stereo(channels > 1);
	datawavelevel.format = format;
	datawavelevel.update();

	std::vector<int> slices;
	datawavelevel.set_slices(slices);
}

void player::append_wavelevel_samples(int wave, int wavelevel, const std::vector<zzub::user_event_data::user_event_data_append_samples::chunk>& chunks, const std::vector<int>& slices) {
	armstrong::storage::wave datawave(doc.get());
	songdata->get_wave_by_index(wave, datawave);

	armstrong::storage::wavelevel datawavelevel(doc.get());
	datawave.get_wavelevel_by_index(wavelevel, datawavelevel);

	std::vector<zzub::user_event_data::user_event_data_append_samples::chunk>::const_iterator i;
	std::vector<armstrong::storage::wavelevel::chunk> datachunks(chunks.size());
	for (i = chunks.begin(); i != chunks.end(); ++i) {
		int index = (int)(i - chunks.begin());
		datachunks[index].buffer = i->buffer;
		datachunks[index].numsamples = i->numsamples;
	}

	int start = datawavelevel.samplecount;
	datawavelevel.insert_chunks(datachunks, start);

	if (slices.size() > 0) {
		std::vector<int> dataslices;
		datawavelevel.get_slices(dataslices);
		dataslices.insert(dataslices.end(), slices.begin(), slices.end());
		datawavelevel.set_slices(dataslices);
	}
}

void player::extend_wave(wave* _wave) {
	_wave->datawave->flags = _wave->datawave->flags | zzub_wave_flag_extended;
	int levelcount = _wave->datawave->get_wavelevel_count();
	for (int i = 0; i < levelcount; i++) {
		armstrong::storage::wavelevel datawavelevel(doc.get());
		_wave->datawave->get_wavelevel_by_index(i, datawavelevel);
		set_wavelevel(wavelevels[datawavelevel.id].get(), true);
	}
	_wave->datawave->update();
}

void player::on_insert_samples(armstrong::storage::document_event_data* e) {
	cout << "player: insert samples " << e->id << endl;
	wavelevel* _wavelevel = wavelevels[e->id].get();
	wave* _wave = waves[_wavelevel->datawavelevel->wave_id].get();

	bool wants_extended = _wavelevel->datawavelevel->format > 0;
	bool is_extended = ((_wave->datawave->flags & zzub_wave_flag_extended) != 0);

	if (wants_extended && !is_extended)
		extend_wave(_wave);

	bool legacyheader = wants_extended || is_extended; // legacyheader = 8 shorts in the beginning of the wavedata reserved for waveformat. used by buzz plugins
	set_wavelevel(_wavelevel, legacyheader);
}

void player::on_delete_samples(armstrong::storage::document_event_data* e) {
	cout << "player: delete samples " << e->id << endl;
	wavelevel* _wavelevel = wavelevels[e->id].get();
	wave* _wave = waves[_wavelevel->datawavelevel->wave_id].get();

	bool wants_extended = _wavelevel->datawavelevel->format > 0;
	bool is_extended = ((_wave->datawave->flags & zzub_wave_flag_extended) != 0);
	set_wavelevel(_wavelevel, wants_extended || is_extended);
}

void player::on_orderlist_timeshift(armstrong::storage::document_event_data* e) {
	mix->sync_orderlist_timeshift(e->newdata.orderlist_timeshift->index, e->newdata.orderlist_timeshift->timeshift);
}

// on_ensure_pluginparameters is invoked via an sql function "ensure_plugin_parameters" as part of the upgrade script from version 9->10
void player::on_ensure_pluginparameters(armstrong::storage::document_event_data* e) {
	armstrong::storage::tableiterator tabit(doc->db, "select p.id, i.uri, p.trackcount from loaddb.plugin p inner join loaddb.plugininfo i on i.id = p.plugininfo_id");

	while (!tabit.eof()) {

		int pluginid = tabit.id();
		const unsigned char* uri = sqlite3_column_text(tabit.stmt, 1);
		int trackcount = sqlite3_column_int(tabit.stmt, 2);
		const zzub::info* pluginfo = plugmgr.plugin_get_info((const char*)uri);

		if (pluginfo) {
			ensure_pluginparameters_exist("loaddb.", pluginfo, pluginid, trackcount);
		} else {
			cerr << "warning: skipping ensure_pluginparameters for " << uri << endl;
		}
		tabit.next();
	}
	tabit.destroy();
}


parameterinfo* player::get_parameterinfo(int plugininfo_id, int group, int column) {
	for (std::vector<boost::shared_ptr<armstrong::frontend::parameterinfo> >::iterator i = parameterinfos.begin(); i != parameterinfos.end(); ++i) {
		if ((*i) == 0) continue;
		if ((*i)->dataparameterinfo->plugininfo_id == plugininfo_id &&
			(*i)->dataparameterinfo->paramgroup == group &&
			(*i)->dataparameterinfo->paramcolumn == column) {

			return i->get();
		}
	}
	return 0;
}

pluginparameter* player::get_pluginparameter(int plugin_id, int group, int track, int column) {
	// this check is here for a shutdown race with the visualizer plugin, when handling 
	// user_event_type_parameter_change after its deleted+created+deleted during zzub_player_clear():
	if (plugin_id >= (int)plugins.size() || plugins[plugin_id] == 0) return 0;

	parameterinfo* paraminfo = get_parameterinfo(plugins[plugin_id]->dataplugin->plugininfo_id, group, column);
	if (paraminfo == 0) return 0;

	for (std::vector<boost::shared_ptr<armstrong::frontend::pluginparameter> >::iterator i = pluginparameters.begin(); i != pluginparameters.end(); ++i) {
		if ((*i) == 0) continue;
		if ((*i)->datapluginparameter->parameterinfo_id == paraminfo->dataparameterinfo->id &&
			(*i)->datapluginparameter->paramtrack == track &&
			(*i)->datapluginparameter->plugin_id == plugin_id) {

			return i->get();
		}
	}
	return 0;
}

bool player::user_event(zzub::user_event_data& data) {
	// forward mixer event
	zzub_event_data_t zzubdata;
	pluginparameter* param;
	zzub_plugin_t* plug;
	switch (data.type) {
		case zzub::user_event_type_midi_control:
			zzubdata.type = zzub_event_type_midi_control;
			zzubdata.midi_message.status = data.midi_control.status;
			zzubdata.midi_message.data1 = data.midi_control.data1;
			zzubdata.midi_message.data2 = data.midi_control.data2;
			return invoke_player_event(zzubdata);

		case zzub::user_event_type_state_change:
			zzubdata.type = zzub_event_type_player_state_changed;
			zzubdata.player_state_changed.player_state = data.state_change.state;
			return invoke_player_event(zzubdata);

		case zzub::user_event_type_parameter_change:
			plug = plugins[data.parameter_change.id].get();
			if (plug != 0) {
				param = get_pluginparameter(data.parameter_change.id, data.parameter_change.group, data.parameter_change.track, data.parameter_change.column);
				if (param != 0)  // TODO: is this is necessary now with the plugin-test in the parent scope?
					param->datapluginparameter->value = data.parameter_change.value;
				zzubdata.type = zzub_event_type_update_pluginparameter;
				zzubdata.update_pluginparameter.plugin = plug;
				zzubdata.update_pluginparameter.group = data.parameter_change.group;
				zzubdata.update_pluginparameter.track = data.parameter_change.track;
				zzubdata.update_pluginparameter.param = data.parameter_change.column;
				zzubdata.update_pluginparameter.value = data.parameter_change.value;

				// write recorded parameter to pattern - this must be commited later
				if (data.parameter_change.automation_pattern != -1) {
					patterns[data.parameter_change.automation_pattern]->datapattern->set_event_at(
						data.parameter_change.id,
						data.parameter_change.group,
						data.parameter_change.track,
						data.parameter_change.column,
						data.parameter_change.automation_timestamp,
						data.parameter_change.value,
						0 // meta
					);
					automated_pattern = true;
				}
				return invoke_player_event(zzubdata);
			}
			break;

		case zzub::user_event_type_committed:
			// unhandled - this event exists so the mixer can clean up (alternate to gc)
			break;

		case zzub::user_event_type_order_change:
			zzubdata.type = zzub_event_type_player_order_changed;
			zzubdata.player_order_changed.orderindex = data.order_change.orderindex;
			return invoke_player_event(zzubdata);

		case zzub::user_event_type_order_queue_change:
			zzubdata.type = zzub_event_type_player_order_queue_changed;
			order_queue_index = mix->queue_index;
			return invoke_player_event(zzubdata);

		case zzub::user_event_type_tempo_change:
			songdata->bpm = data.tempo_change.bpm;
			songdata->tpb = data.tempo_change.tpb;
			songdata->swing = data.tempo_change.swing;
			songdata->swingticks = data.tempo_change.swing_ticks;
			break;

		case zzub::user_event_type_reset_samples:
			cout << "player: user_event_type_reset_samples" << endl;
			reset_wavelevel_samples(data.reset_samples.wave, data.reset_samples.wavelevel, data.reset_samples.format, data.reset_samples.channels);
			automated_pattern = true; // abuse this signal to notify the host to commit
			break;

		case zzub::user_event_type_append_samples:
			cout << "player: user_event_type_append_samples" << endl;
			append_wavelevel_samples(data.append_samples.wave, data.append_samples.wavelevel, *data.append_samples.buffer, *data.append_samples.slices);
			automated_pattern = true; // abuse this signal to notify the host to commit
			break;
		case zzub::user_event_type_infinite_pattern_recursion:
			zzubdata.type = zzub_event_type_user_alert;
			zzubdata.alert.type = zzub_alert_type_pattern_recursion;
			return invoke_player_event(zzubdata);
		case zzub::user_event_type_samplerate_changed:
			zzubdata.type = zzub_event_type_samplerate_changed;
			return invoke_player_event(zzubdata);
		case zzub::user_event_type_latency_changed:
			zzubdata.type = zzub_event_type_latency_changed;
			return invoke_player_event(zzubdata);
		case zzub::user_event_type_device_reset:
			zzubdata.type = zzub_event_type_device_reset;
			return invoke_player_event(zzubdata);
	}

	return false;
}

bool player::invoke_player_event(zzub_event_data_t& data) {
	bool handled = false;
	data.userdata = event_userdata;
	for (vector<callbackpair>::iterator i = callbacks.begin(); i != callbacks.end(); ++i) {
		bool result = i->first(0, 0, &data, i->second) != -1 ? true : false;
		handled = result || handled;
		//handled = handlers[i]->invoke(data)||handled;
	}
	return handled;
}

}
}
