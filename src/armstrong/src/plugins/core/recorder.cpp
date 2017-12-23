/*
Copyright (C) 2003-2010 Anders Ervik <calvin@countzero.no>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
};
typedef struct zzub::mixer zzub_mixer_t;

#include <zzub/plugin.h>
#include <cstring>
#include <sndfile.h>
#include "mixing/mixer.h"
#include "recorder.h"

using namespace std;

struct recorder_wavetable_plugin : zzub::plugin {
	
#pragma pack (push, 1)
	struct gvals {
		unsigned char enable;
		unsigned char mode;
		unsigned char format;
		unsigned char wave;
	};

	struct avals {
		int lock;
	};
#pragma pack (pop)

	gvals gval;
	avals aval;
	bool enabled;
	int mode;
	zzub_wave_buffer_type format;
	int wave;
	int handle;
	typedef zzub::user_event_data::user_event_data_append_samples::chunk chunk;
	std::vector<chunk> recordedbuffers;
	std::vector<int> recordedslices;
	int recordedsize;
	volatile bool registered;
	synchronization::event doneevent;
	
	recorder_wavetable_plugin() {
		enabled = false;
		mode = 0;
		format = zzub_wave_buffer_type_si16;
		wave = 0;
		handle = 0;
		recordedsize = 0;
		global_values = &gval;
		attributes = (int*)&aval;
		registered = false;
	}
	
	virtual void destroy() { 
		// wait for encoder thread to finish
		if (registered)
			doneevent.wait();
		delete this; 
	}
	virtual void init(zzub::archive *arc) {}
	
	virtual void process_events() {	
		// TODO: this merely enables recording from the current chunk - can we somehow split the encoding buffer and slip an "enable/disable"-event in there?
		if (gval.enable != zzub_switch_value_none)
			enabled = gval.enable == zzub_switch_value_on;

		if (gval.mode != zzub_switch_value_none)
			mode = gval.mode;

		if (gval.format != 255)
			format = (zzub_wave_buffer_type)gval.format;

		if (gval.wave != zzub_wavetable_index_value_none)
			wave = gval.wave;
	}

	// process_encoder return false = default behaviour = engine frees buffers
	virtual bool process_encoder(int state, float** buffers, int numsamples) { // called from encoder thread!
		if (state == zzub_encoder_state_created) {
			registered = true;
			return false;
		} else
		if (state == zzub_encoder_state_deleted) {
			close();
			// signal we are done in the encoder thread
			assert(registered != 0);
			doneevent.signal();
			return false;
		} else
		if (state == zzub_encoder_state_seeking) {
			recordedslices.push_back(recordedsize);
			return false;
		}

		bool playing = state == zzub_encoder_state_playing;
		if (mode == 0 && enabled && playing && !handle) open();
		if (mode == 1 && enabled && !handle) open();

		if (mode == 0 && handle && (!playing || !enabled)) {
			// TODO: set "enabled" parameter to off in audio or user thread .. we are now in encoder thread .. is it safe to use user thread objects? we can send a user event!
			close();
		}
		if (mode == 1 && handle && !enabled) close();

		if (handle && numsamples && buffers) {
			write(buffers, numsamples);

			return true; // we will free the buffer memory ourself
		}

		return false;
	}

	bool open() {
		if (handle || wave == 0) return false;
		handle = wave;
		zzub::user_event_data userev;
		userev.type = zzub::user_event_type_reset_samples ;
		userev.reset_samples.channels = 2;
		userev.reset_samples.format = format;
		userev.reset_samples.wave = handle - 1;
		userev.reset_samples.wavelevel = 0;
		_mixer->invoke_user_event(userev);

		recordedsize = 0;
		return handle != 0;
	}

	void write(float** samples, int numsamples) {
		if (!numsamples || !handle) return;
		chunk buffers;
		buffers.buffer.resize(2);
		buffers.numsamples = numsamples;
		for (int i = 0; i < 2; i++) {
			buffers.buffer[i] = boost::shared_array<float>(samples[i]);
		}
		recordedbuffers.push_back(buffers);

		if (recordedbuffers.size() > 20) flush();
		recordedsize += numsamples;
	}

	void flush() {
		zzub::user_event_data userev;
		userev.type = zzub::user_event_type_append_samples ;
		userev.append_samples.wave = handle - 1;
		userev.append_samples.wavelevel = 0;
		userev.append_samples.numsamples = 0;
		userev.append_samples.buffer = new std::vector<chunk>(recordedbuffers);
		userev.append_samples.slices = new std::vector<int>(recordedslices);
		_mixer->invoke_user_event(userev);

		recordedbuffers.clear();
		recordedslices.clear();
	}

	void close() {
		if (handle) {
			flush();
			handle = 0;
		}
	}

	virtual const char * describe_value(int param, int value) { 
		switch (param) {
			case 0:
				if (value == 0) return "off";
				return "on";
			case 1:
				if (value == 0) return "automatic"; // or use name of time source plugin?
				else if (value == 1) return "manual";
				else return 0;
			case 2:
				if (value == 0) return "16bit";
				else if (value == 1) return "32bit float";
				else if (value == 2) return "32bit int";
				else if (value == 3) return "24bit";
				else return 0;
		}
		return 0; 
	}
};


zzub::plugin* recorder_wavetable_plugin_info::create_plugin() { 
	return new recorder_wavetable_plugin();
}




struct recorder_file_plugin : zzub::plugin {

#pragma pack (push, 1)
	struct gvals {
		unsigned char enable;
		unsigned char mode;
		unsigned char format;
	};
#pragma pack (pop)

	gvals gval;
	bool enabled;
	bool recording;
	int mode;
	zzub_wave_buffer_type format;
	std::string wavefilepath;
	SNDFILE* handle; // handle of wavefile to write to
	float ilsamples[4096 * 2]; // 4096 = mixing/allocpool::pool_chunk_size, 2 = stereo interleaved
	volatile bool registered;
	synchronization::event doneevent;

	recorder_file_plugin() {
		global_values = &gval;
		recording = false;
		enabled = false;
		mode = 0;
		format = zzub_wave_buffer_type_si16;
		handle = 0;
		registered = false;
	}

	virtual void init(zzub::archive* arc) { }

	virtual void destroy() {
		if (registered) doneevent.wait();
		close();
		delete this; 
	}

	virtual void process_events() {
		if (gval.enable != zzub_switch_value_none)
			enabled = gval.enable == zzub_switch_value_on;

		if (gval.mode != zzub_switch_value_none)
			mode = gval.mode;

		if (gval.format != 255)
			format = (zzub_wave_buffer_type)gval.format;
	}

	virtual bool process_encoder(int state, float** buffers, int numsamples) { // called from encoder thread!
		if (state == zzub_encoder_state_created) {
			registered = true;
			return false;
		} else
		if (state == zzub_encoder_state_deleted) {
			// signal we are done in the encoder thread
			assert(registered != 0);
			doneevent.signal();
			return false;
		}

		bool playing = state == zzub_encoder_state_playing;
		if (mode == 0 && enabled && playing && !handle) open();
		if (mode == 1 && enabled && !handle) open();

		if (mode == 0 && handle && (!playing || !enabled)) {
			// TODO: set "enabled" parameter to off in audio or user thread .. we are now in encoder thread .. is it safe to use user thread objects?
			close();
		}
		if (mode == 1 && handle && !enabled) close();

		if (handle && numsamples && buffers) write(buffers, numsamples);
		return false; // let the engine free the buffers
	}

	int get_sndfile_format(int format) {
		switch (format) {
			case zzub_wave_buffer_type_si16:
				return SF_FORMAT_PCM_16;
			case zzub_wave_buffer_type_si24:
				return SF_FORMAT_PCM_24;
			case zzub_wave_buffer_type_si32:
				return SF_FORMAT_PCM_32;
			case zzub_wave_buffer_type_f32:
				return SF_FORMAT_FLOAT;
			default:
				assert(false);
				return 0;
		}
	}

	bool open() {
		if (handle || wavefilepath.length() == 0) return false;

		SF_INFO sfinfo;
		memset(&sfinfo, 0, sizeof(sfinfo));
		sfinfo.samplerate = _master_info->samples_per_second;
		sfinfo.channels = 2;
		sfinfo.format = SF_FORMAT_WAV | get_sndfile_format(format);
		handle = sf_open(wavefilepath.c_str(), SFM_WRITE, &sfinfo); // open a handle
		if (!handle)
			printf("opening '%s' for writing failed.\n", wavefilepath.c_str());

		return handle != 0;
	}

	void write(float** samples, int numsamples) {
		if (!numsamples || !handle) return;

		float *p = ilsamples;
		for (int i = 0; i < numsamples; ++i) {
			float l = *(samples[0]+i);
			float r = *(samples[1]+i);
			if (l>1.0) l = 1.0; else if (l<-1.0) l = -1.0;
			if (r>1.0) r = 1.0; else if (r<-1.0) r = -1.0;
			*p++ = l;
			*p++ = r;
		}
		sf_writef_float(handle, ilsamples, numsamples);
	}

	void close() {
		if (handle) {
			sf_close(handle); // so close it
			handle = 0;
		}
	}

	virtual void set_stream_source(const char* resource) {
		wavefilepath = resource;
	}

	virtual const char * describe_value(int param, int value) { 
		switch (param) {
			case 0:
				if (value == 0) return "off";
				return "on";
			case 1:
				if (value == 0) return "automatic"; // or use name of time source plugin?
				else if (value == 1) return "manual";
				else return 0;
			case 2:
				if (value == 0) return "16bit";
				else if (value == 1) return "32bit float";
				else if (value == 2) return "32bit int";
				else if (value == 3) return "24bit";
				else return 0;
		}
		return 0; 
	}
};


zzub::plugin* recorder_file_plugin_info::create_plugin() { 
	return new recorder_file_plugin();
}

// make various raw visualizer data available to clients through get_encoder_digest()
struct visualizer_plugin : zzub::plugin {

	struct bufferinfo {
		std::vector<boost::shared_array<float> > buffer;
		int numsamples;
	};

	synchronization::critical_section critsec; 
	int buffersize;
	std::vector<bufferinfo> buffercopies;
	volatile bool registered;
	synchronization::event doneevent;

	visualizer_plugin() {
		registered = false;
	}

	virtual void destroy() {
		if (registered)
			doneevent.wait();
	}

	// process_encoder() -> prechunked, lockable data
	virtual bool process_encoder(int state, float** buffers, int numsamples) { // called from encoder thread!
		if (state == zzub_encoder_state_created) {
			registered = true;
			return false;
		} else
		if (state == zzub_encoder_state_deleted) {
			// signal we are done in the encoder thread
			assert(registered != 0);
			doneevent.signal();
			return false;
		}

		if (buffers == 0 || numsamples == 0) return false;

		synchronization::critical_section_locker cslock(critsec);
		
		// never keep more than the last 5 buffers
		if (buffercopies.size() > 10) buffercopies.erase(buffercopies.begin(), buffercopies.end() - 5);

		bufferinfo buffercopy;
		buffercopy.buffer.resize(2);
		buffercopy.numsamples = numsamples;
		for (int i = 0; i < 2; i++)
			buffercopy.buffer[i] = boost::shared_array<float>(buffers[i]);
		buffercopies.push_back(buffercopy);
		return true; // tell engine we take ownership of the buffers
	}

	// type 0 = raw wave data, type 1 = ? fft?? client should do fft to prevent unnecessary fft-ing?
	// buffers = 0 -> return number of samples of buffer
	// this should be udp-like, in that we need to call this very frequently to not miss any buffers
	virtual int get_encoder_digest(int type, float** buffers, int numsamples) { // called from user thread!
		synchronization::critical_section_locker cslock(critsec);
		if (buffercopies.size() == 0) return 0;

		bufferinfo& buffercopy = buffercopies.front();
		if (buffers == 0) return buffercopy.numsamples;

		numsamples = std::min(numsamples, buffercopy.numsamples);
		for (int i = 0; i < 2; i++)
			memcpy(buffers[i], buffercopy.buffer[i].get(), numsamples * sizeof(float));
		buffercopies.erase(buffercopies.begin(), buffercopies.begin() + 1);
		return numsamples;
	}
};

zzub::plugin* visualizer_plugin_info::create_plugin() { 
	return new visualizer_plugin();
}

