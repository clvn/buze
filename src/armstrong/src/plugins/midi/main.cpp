#include <string>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <zzub/signature.h>
#include <zzub/plugin.h>
#include "miditracker.h"
#include "midicc.h"
#include "miditime.h"
#include "midi.h"
#include "midiinput.h"
#include "midioutput.h"

using namespace std;

const char *zzub_get_signature() { return ZZUB_SIGNATURE; }

struct midipluginscollection : zzub::plugincollection {

	miditracker::miditracker_info _miditracker_info;
	midicc::midicc_info _midicc_info;
	miditime::miditime_info _miditime_info;
	midiinput::midiinput_info _midiinput_info;
	midioutput::midioutput_info _midioutput_info;

	virtual void initialize(zzub::pluginfactory *factory) {
		_miditracker_info.collection = this;
		_midicc_info.collection = this;
		_miditime_info.collection = this;
		_midiinput_info.collection = this;
		_midioutput_info.collection = this;

		factory->register_info(&_miditracker_info);
		factory->register_info(&_midicc_info);
		factory->register_info(&_miditime_info);
		factory->register_info(&_midiinput_info);
		factory->register_info(&_midioutput_info);
	}
	
	virtual zzub::info *get_info(const char *uri, zzub::archive *data) { return 0; }
	virtual void destroy() { delete this; }
	// Returns the uri of the collection to be identified,
	// return zero for no uri. Collections without uri can not be 
	// configured.
	virtual const char *get_uri() { return 0; }
	
	// Called by the host to set specific configuration options,
	// usually related to paths.
	virtual void configure(const char *key, const char *value) {}

	virtual const char* get_name() {
		return "MIDI";
	}
};

zzub::plugincollection *midi_get_plugincollection() {
	return new midipluginscollection();
}
