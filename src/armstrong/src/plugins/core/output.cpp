/*
Copyright (C) 2003-2007 Anders Ervik <calvin@countzero.no>
Copyright (C) 2006-2007 Leonard Ritter

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

#include "mixing/mixer.h"
#include <cassert>
#include <sstream>
#include <iostream>
#include <cstring>
#include "output.h"

struct output_plugin : zzub::plugin {
	int channel;
	bool cleared[2];
	
	output_plugin() {
		channel = 0;
		cleared[0] = cleared[1] = false;
	}

	virtual void init(zzub::archive* arc) {
		if (arc) load(arc);
	}

	virtual void load(zzub::archive* arc) {
		channel = 0;
		zzub::instream* ins = arc->get_instream("");
		if (!ins) return ;
		int version;
		if (!ins->read(version)) return ;
		if (version != 1) return ;
		ins->read(channel);
	}

	virtual void save(zzub::archive* arc) {
		zzub::outstream* outs = arc->get_outstream("");
		outs->write((int)1); // version 1
		outs->write(channel);
	}

	virtual void destroy() { 
		delete this; 
	}

	virtual void process_events() {
		if (channel < 0 || channel >= _mixer->output_channel_count / 2) {
			channel = 0;
		}
	}

	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode) {
		int first_channel = channel * 2;
		zzub::metaplugin* mpl = _mixer->plugins.top()[_id].get();
		int writeoffset = mpl->audiodata->output_buffer_write;

		bool has_signals = false;
		for (int i = 0; i < 2; i++) {
			if (pin[i]) {
				bool has_channel_signals = _mixer->write_output(first_channel + i, writeoffset, pin[i], numsamples, 1.0f);
				has_signals &= has_channel_signals;
				cleared[i] = !has_channel_signals;
			} else if (!cleared[i]) {
				_mixer->write_output(first_channel + i, writeoffset, 0, numsamples, 0.0f);
				cleared[i] = true;
			}
		}

		return false; // didnt write anything to pout
	}

	virtual void command(int index) {
		if (index >= 0x100 && index < 0x200) {
			channel = index - 0x100;
		}
	}

	virtual void get_sub_menu(int index, zzub::outstream *os) { 
		if (index != 0) return ;
		for (int i = 0; i < _mixer->output_channel_count / 2; i++) {
			std::stringstream strm;
			strm << (i == channel ? "*" : "") << "Stereo Channel " << (i*2) << " / " << i*2+1;
			os->write((const char*)strm.str().c_str());
		}
		os->write("\0");
	}
};

zzub::plugin* output_plugin_info::create_plugin() { 
	return new output_plugin();
}

struct output16_plugin : zzub::plugin {
	bool cleared[16];
 
	output16_plugin() { 
		for (int i = 0; i < 16; i++) { cleared[i] = false; } 
	}

	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode) {
		zzub::metaplugin* mpl = _mixer->plugins.top()[_id].get();
		int writeoffset = mpl->audiodata->output_buffer_write;

		for (int i = 0; i < 16; i++) {
			if (pin[i] != 0) {
				_mixer->write_output(i, writeoffset, pin[i], numsamples, 1.0f);
				cleared[i] = false;
			} else if (!cleared[i]) {
				_mixer->write_output(i, writeoffset, 0, numsamples, 0.0f);
				cleared[i] = true;
			}
		}
		return false;
	}
};

zzub::plugin* output16_plugin_info::create_plugin() { 
	return new output16_plugin();
}


struct output32_plugin : zzub::plugin {
	bool cleared[32];
 
	output32_plugin() { 
		for (int i = 0; i < 32; i++) { cleared[i] = false; } 
	}

	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode) {
		zzub::metaplugin* mpl = _mixer->plugins.top()[_id].get();
		int writeoffset = mpl->audiodata->output_buffer_write;

		for (int i = 0; i < 32; i++) {
			if (pin[i] != 0) {
				_mixer->write_output(i, writeoffset, pin[i], numsamples, 1.0f);
				cleared[i] = false;
			} else if (!cleared[i]) {
				_mixer->write_output(i, writeoffset, 0, numsamples, 0.0f);
				cleared[i] = true;
			}
		}
		return false;
	}
};

zzub::plugin* output32_plugin_info::create_plugin() { 
	return new output32_plugin();
}
