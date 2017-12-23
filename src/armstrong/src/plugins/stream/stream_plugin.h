#pragma once

#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <cstdio>
#include "mixer.h"
#include <zzub/plugin.h>
#include "resample.h"

const char* get_open_filename(const char* fileName, const char* filter);
int buzz_to_midi_note(int note);
void add_samples(float *pout, float *pin, int numsamples, float amp);
float lognote(int freq);


#pragma pack(1)										// Place to retrieve parameters	

struct gvals {
	unsigned char note;
	unsigned int offset;
	unsigned int length;
};

struct avals {
	int offsetfromsong;
};


#pragma pack()

/***

	Stream plugin base class

***/

template <typename SOURCE>
struct stream_plugin : zzub::plugin {
	SOURCE* source;
	boost::lockfree::spsc_queue<SOURCE*, boost::lockfree::capacity<16> > source_queue; 
	boost::lockfree::spsc_queue<SOURCE*, boost::lockfree::capacity<16> > source_garbage; 

	stream_plugin() {
		attributes = (int*)&aval;
		global_values = &gval;
		track_values = 0;
	}

	// ::zzub::plugin methods
	virtual void process_controller_events() {}
	virtual const char * describe_value(int param, int value) { return 0; }
	virtual void destroy() { delete this; }

	unsigned int get_offset() {
		unsigned short low = gval.offset & 0xFFFF;
		unsigned short high = gval.offset >> 16;
		unsigned int offset;
		if (low == 0xFFFF) 
			offset = high << 16; else
		if (high == 0xFFFF)
			offset = low; else
			offset = gval.offset;
		return offset;
	}

	unsigned int get_length() {
		unsigned short low = gval.length & 0xFFFF;
		unsigned short high = gval.length >> 16;
		unsigned int length;
		if (low == 0xFFFF) 
			length = high << 16; else
		if (high == 0xFFFF)
			length = low; else
			length = gval.length;
		return length;
	}

	virtual void enable_stream_source() {
		SOURCE* strm;
		while (source_queue.pop(strm)) {
			if (source != 0) {
				source_garbage.push(source);
			}
			source = strm;
		}
	}

	virtual void flush_stream_source() {
		SOURCE* strm;
		while (source_garbage.pop(strm)) {
			delete strm;
		}
	}

protected:

	gvals gval;
	avals aval;
};
/*
struct stream_machine_info : zzub::info {
	stream_machine_info();
};

#include "stream_wav.h"
#include "stream_mp3.h"
#include "stream_wavetable.h"
#include "resample.h"


**

	Stream plugin collection

**

struct streamplugincollection : zzub::plugincollection {
	virtual void initialize(zzub::pluginfactory *factory) { 
		factory->register_info(&stream_info_wav); 
		factory->register_info(&stream_info_mp3); 
		factory->register_info(&stream_info_wavetable); 
	}
	virtual const zzub::info *get_info(const char *uri, zzub::archive *data) { return 0; }
	virtual void destroy() { delete this; }
	virtual const char *get_uri() { return 0; }
	virtual void configure(const char *key, const char *value) {}
};
*/
