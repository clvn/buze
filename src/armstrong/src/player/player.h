#pragma once

#include "library.h"

#include <iostream>
#include <list>
#include <zzub/zzub.h>
#include <zzub/plugin.h>
#include "storage/document.h"
#include "dummy.h"
#include "pluginmanager.h"
#include "driver.h"
#include "mixing/mixer.h"
#include "midi.h"
#include "armserveclient.h"

namespace armstrong {

namespace frontend {

struct player;

struct connection {
	player* owner;
	boost::shared_ptr<armstrong::storage::connection> dataconn;
};

struct plugin {
	player* owner;
	boost::shared_ptr<armstrong::storage::plugin> dataplugin;
	zzub::plugin* userplugin;
	zzub::info* loader;
};

struct pluginiterator {
	player* owner;
	boost::shared_ptr<armstrong::storage::tableiterator> recordset;
};

struct pluginparameter {
	player* owner;
	boost::shared_ptr<armstrong::storage::pluginparameter> datapluginparameter;
};

struct parameterinfo {
	player* owner;
	boost::shared_ptr<armstrong::storage::parameterinfodata> dataparameterinfo;
};

struct attribute {
	player* owner;
	boost::shared_ptr<armstrong::storage::attributedata> dataattribute;
};

struct pattern {
	player* owner;
	boost::shared_ptr<armstrong::storage::pattern> datapattern;
};

struct patterniterator {
	player* owner;
	boost::shared_ptr<armstrong::storage::tableiterator> recordset;
};

struct patternevent {
	player* owner;
	boost::shared_ptr<armstrong::storage::patternevent> datapatternevent;
};

struct patterneventiterator {
	player* owner;
	boost::shared_ptr<armstrong::storage::tableiterator> recordset;
};

struct patternorder {
	player* owner;
	boost::shared_ptr<armstrong::storage::patternorderdata> dataorder;
};

struct patternformat {
	player* owner;
	boost::shared_ptr<armstrong::storage::patternformat> dataformat;
};

struct patternformatiterator {
	player* owner;
	boost::shared_ptr<armstrong::storage::tableiterator> recordset;
};

struct patternformatcolumn {
	player* owner;
	boost::shared_ptr<armstrong::storage::patternformatcolumn> datacolumn;
};

struct patternformatcolumniterator {
	player* owner;
	boost::shared_ptr<armstrong::storage::tableiterator> recordset;
};

struct patternformattrack {
	player* owner;
	boost::shared_ptr<armstrong::storage::patternformattrack> datatrack;
};

struct midimapping {
	player* owner;
	boost::shared_ptr<armstrong::storage::midimapping> datamidimapping;
};

struct midimappingiterator {
	player* owner;
	boost::shared_ptr<armstrong::storage::tableiterator> recordset;
};

struct wave {
	player* owner;
	boost::shared_ptr<armstrong::storage::wave> datawave;
};

struct wavelevel {
	player* owner;
	boost::shared_ptr<armstrong::storage::wavelevel> datawavelevel;
};

struct envelope {
	player* owner;
	boost::shared_ptr<armstrong::storage::envelope> dataenvelope;
};

struct eventconnectionbinding {
	player* owner;
	boost::shared_ptr<armstrong::storage::eventconnectionbindingdata> databinding;
};

struct eventconnectionbindingiterator {
	player* owner;
	boost::shared_ptr<armstrong::storage::tableiterator> recordset;
};

struct validationerroriterator {
	player* owner;
	std::vector<validationerror>::iterator i;
};

struct plugingroup {
	player* owner;
	boost::shared_ptr<armstrong::storage::plugingroup> dataplugingroup;
};

struct plugingroupiterator {
	player* owner;
	boost::shared_ptr<armstrong::storage::tableiterator> recordset;
};

struct audiodeviceiterator {
	audiodriver* owner;
	std::vector<audiodevice*> items;
	std::vector<audiodevice*>::iterator i;
};

struct player : armstrong::storage::documentlistener, audioworker, zzub::user_event_listener {
	std::string hostpath;
	std::string userpath;
	std::string temppath;
	pluginmanager plugmgr;
	mididriver* midiDriver;
	boost::shared_ptr<zzub::mixer> mix;
	boost::shared_ptr<armstrong::storage::document> doc;
	boost::shared_ptr<armstrong::storage::song> songdata;
	armserve_client armclient;

	std::vector<boost::shared_ptr<parameterinfo> > parameterinfos;
	std::vector<boost::shared_ptr<plugin> > plugins;
	std::vector<boost::shared_ptr<connection> > connections;
	std::vector<boost::shared_ptr<pattern> > patterns;
	std::vector<boost::shared_ptr<patternevent> > patternevents;
	std::vector<boost::shared_ptr<patternorder> > patternorders;
	std::vector<boost::shared_ptr<patternformat> > patternformats;
	std::vector<boost::shared_ptr<patternformatcolumn> > patternformatcolumns;
	std::vector<boost::shared_ptr<patternformattrack> > patternformattracks;
	std::vector<boost::shared_ptr<midimapping> > midimappings;
	std::vector<boost::shared_ptr<wave> > waves;
	std::vector<boost::shared_ptr<wavelevel> > wavelevels;
	std::vector<boost::shared_ptr<envelope> > envelopes;
	std::vector<boost::shared_ptr<attribute> > attributes;
	std::vector<boost::shared_ptr<pluginparameter> > pluginparameters;
	std::vector<boost::shared_ptr<eventconnectionbinding> > eventconnectionbindings;
	std::vector<boost::shared_ptr<plugingroup> > plugingroups;
	int work_buffer_position;						// sample position in current buffer

	typedef std::pair<zzub_callback_t, void*> callbackpair;
	std::vector<callbackpair> callbacks; // callbacks for user_event_queue
	bool automated_pattern;
	void* event_userdata;
	int order_queue_index;  // song state, need a copy of this for user thread get/set
	std::vector<armstrong::frontend::validationerror> load_errors; // on_insert_plugin adds plugin validation results here
	float last_max_peak[64];
	bool midi_lock;

	player();
	~player();
	void initialize();
	void reset();

	bool create_document();
	bool initialize_document();
	bool create_plugin(zzub::info* _info, zzub::instream* data, int datasize, std::string name, int layer_id, armstrong::storage::plugin& result);

	plugin* get_plugin_by_name(std::string name);
	pluginparameter* get_pluginparameter(int plugin_id, int group, int track, int column);
	parameterinfo* get_parameterinfo(int plugininfo_id, int group, int column);
	void ensure_pluginparameters_exist(int pluginid);
	void ensure_pluginparameters_exist(const std::string& prefix, const zzub::info* loader, int pluginid, int tracks);

	bool invoke_player_event(zzub_event_data_t& data);

	void extend_wave(wave* _wave);
	void set_wavelevel(wavelevel* _wavelevel, bool legacyheader);
	void reset_wavelevel_samples(int wave, int wavelevel, int format, int channels);
	void append_wavelevel_samples(int wave, int wavelevel, const std::vector<zzub::user_event_data::user_event_data_append_samples::chunk>& chunks, const std::vector<int>& slices);

	bool validate_plugin(int plugininfo_id, zzub::info* loader, const std::string& name, std::vector<armstrong::frontend::validationerror>& result);

	// documentlistener
	virtual void update_document(armstrong::storage::document_event_data* e);

	void on_insert_song(armstrong::storage::document_event_data* e);
	void on_delete_song(armstrong::storage::document_event_data* e);
	void on_update_song(armstrong::storage::document_event_data* e);
	void on_insert_plugin(armstrong::storage::document_event_data* e);
	void on_before_delete_plugin(armstrong::storage::document_event_data* e);
	void on_delete_plugin(armstrong::storage::document_event_data* e);
	void on_update_plugin(armstrong::storage::document_event_data* e);
	void on_insert_parameterinfo(armstrong::storage::document_event_data* e);
	void on_delete_parameterinfo(armstrong::storage::document_event_data* e);
	void on_update_parameterinfo(armstrong::storage::document_event_data* e);
	void on_insert_pluginparameter(armstrong::storage::document_event_data* e);
	void on_delete_pluginparameter(armstrong::storage::document_event_data* e);
	void on_update_pluginparameter(armstrong::storage::document_event_data* e);
	void on_insert_attribute(armstrong::storage::document_event_data* e);
	void on_update_attribute(armstrong::storage::document_event_data* e);
	void on_delete_attribute(armstrong::storage::document_event_data* e);
	void on_insert_pattern(armstrong::storage::document_event_data* e);
	void on_delete_pattern(armstrong::storage::document_event_data* e);
	void on_update_pattern(armstrong::storage::document_event_data* e);
	void on_insert_patternevent(armstrong::storage::document_event_data* e);
	void on_delete_patternevent(armstrong::storage::document_event_data* e);
	void on_update_patternevent(armstrong::storage::document_event_data* e);
	void on_insert_patternformat(armstrong::storage::document_event_data* e);
	void on_delete_patternformat(armstrong::storage::document_event_data* e);
	void on_update_patternformat(armstrong::storage::document_event_data* e);
	void on_insert_patternformatcolumn(armstrong::storage::document_event_data* e);
	void on_delete_patternformatcolumn(armstrong::storage::document_event_data* e);
	void on_update_patternformatcolumn(armstrong::storage::document_event_data* e);
	void on_insert_patternformatcolumnfilter(armstrong::storage::document_event_data* e);
	void on_update_patternformatcolumnfilter(armstrong::storage::document_event_data* e);
	void on_delete_patternformatcolumnfilter(armstrong::storage::document_event_data* e);
	void on_insert_patternformattrack(armstrong::storage::document_event_data* e);
	void on_update_patternformattrack(armstrong::storage::document_event_data* e);
	void on_delete_patternformattrack(armstrong::storage::document_event_data* e);
	void on_insert_connection(armstrong::storage::document_event_data* e);
	void on_delete_connection(armstrong::storage::document_event_data* e);
	void on_update_connection(armstrong::storage::document_event_data* e);
	void on_insert_eventconnectionbinding(armstrong::storage::document_event_data* e);
	void on_delete_eventconnectionbinding(armstrong::storage::document_event_data* e);
	void on_update_eventconnectionbinding(armstrong::storage::document_event_data* e);
	void on_insert_patternorder(armstrong::storage::document_event_data* e);
	void on_delete_patternorder(armstrong::storage::document_event_data* e);
	void on_update_patternorder(armstrong::storage::document_event_data* e);
	void on_insert_midimapping(armstrong::storage::document_event_data* e);
	void on_delete_midimapping(armstrong::storage::document_event_data* e);
	void on_update_midimapping(armstrong::storage::document_event_data* e);
	void on_insert_wave(armstrong::storage::document_event_data* e);
	void on_delete_wave(armstrong::storage::document_event_data* e);
	void on_update_wave(armstrong::storage::document_event_data* e);
	void on_insert_wavelevel(armstrong::storage::document_event_data* e);
	void on_delete_wavelevel(armstrong::storage::document_event_data* e);
	void on_update_wavelevel(armstrong::storage::document_event_data* e);
	void on_insert_slice(armstrong::storage::document_event_data* e);
	void on_delete_slice(armstrong::storage::document_event_data* e);
	void on_update_slice(armstrong::storage::document_event_data* e);
	void on_insert_samples(armstrong::storage::document_event_data* e);
	void on_delete_samples(armstrong::storage::document_event_data* e);
	void on_insert_envelope(armstrong::storage::document_event_data* e);
	void on_delete_envelope(armstrong::storage::document_event_data* e);
	void on_update_envelope(armstrong::storage::document_event_data* e);
	void on_insert_plugingroup(armstrong::storage::document_event_data* e);
	void on_delete_plugingroup(armstrong::storage::document_event_data* e);
	void on_update_plugingroup(armstrong::storage::document_event_data* e);
	void on_barrier(armstrong::storage::document_event_data* e);
	void on_open_document(armstrong::storage::document_event_data* e);
	void on_orderlist_timeshift(armstrong::storage::document_event_data* e);
	void on_ensure_pluginparameters(armstrong::storage::document_event_data* e);

	void work_stereo_with_midi(float** pin, float** pout, int sample_count, zzub::midi_message* pmidiin, int inmidi_count, zzub::midi_message* pmidiout, int* outmidi_count, int outmidi_maxcount);

	// audioworker
	virtual void work_stereo(float** pin, float** pout, int num);
	virtual void audio_enabled();
	virtual void audio_disabled();
	virtual void samplerate_changed();
	virtual void latency_changed();
	virtual void device_reset();

	// user_event_listener
	virtual bool user_event(zzub::user_event_data& data);

	void process_midi_events(zzub::midi_message* pmidi, int midi_count);

	void midi_device_changed();

};

}
}
