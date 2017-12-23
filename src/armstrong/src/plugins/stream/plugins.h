#pragma once
#include "stream_info.h"
#include "stream_file.h"
#include "stream_wavetable.h"

struct streamplugincollection : zzub::plugincollection {
	stream_machine_info_file stream_info_file;
	stream_machine_info_wavetable stream_info_wavetable;

	virtual void initialize(zzub::pluginfactory *factory) { 
		stream_info_file.collection = this;
		stream_info_wavetable.collection = this;

		factory->register_info(&stream_info_file); 
		factory->register_info(&stream_info_wavetable); 
	}
	virtual zzub::info *get_info(const char *uri, zzub::archive *data) { return 0; }
	virtual void destroy() { delete this; }
	virtual const char *get_uri() { return 0; }
	virtual void configure(const char *key, const char *value) {}

	virtual const char* get_name() {
		return "Stream";
	}

};
