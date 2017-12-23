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
#include <sstream>
#include <cassert>
#include <iostream>
#include <cstring>
#include "input.h"

struct input_plugin : zzub::plugin {
	int attributeValues[1];
	
	input_plugin() {
		attributes = attributeValues;
		attributeValues[0] = 0;
	}
	
	virtual void destroy() { delete this; }
	virtual void init(zzub::archive *arc) {}
	virtual void process_events() {
		attributes_changed();
	}
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode) {
		if( (mode&zzub_process_mode_write)==0 || attributeValues[0] >= _mixer->input_channel_count)
			return false;
		zzub::metaplugin* mpl = _mixer->plugins.top()[_id].get();
		int writeoffset = mpl->audiodata->output_buffer_write;

		float* l = _mixer->input_buffer[attributeValues[0] * 2 + 0] + writeoffset;
		float* r = _mixer->input_buffer[attributeValues[0] * 2 + 1] + writeoffset;

		if (pout[0]) memcpy(pout[0], l, numsamples*sizeof(float));
		if (pout[1]) memcpy(pout[1], r, numsamples*sizeof(float));

		return zzub::buffer_has_signals(l, numsamples) || zzub::buffer_has_signals(r, numsamples);
	}

	virtual void attributes_changed() {
		if (attributeValues[0]<0 || attributeValues[0] >= _mixer->input_channel_count / 2) {
			attributeValues[0] = 0;
		}
	}
	virtual void command(int index) {
		if (index>=0x100 && index < 0x200) {
			int channel = index - 0x100;
			attributeValues[0] = channel;
			attributes_changed();
		}
	}

	virtual void get_sub_menu(int index, zzub::outstream *os) { 
		if (index != 0) return ;
		for (int i = 0; i < _mixer->input_channel_count / 2; i++) {
			std::stringstream strm;
			strm << (i==attributeValues[0]?"*":"") << "Stereo Channel " << (i*2) << " / " << i*2+1;
			os->write((const char*)strm.str().c_str());
		}
		os->write("\0");
	}
};

zzub::plugin* input_plugin_info::create_plugin() { 
	return new input_plugin();
}

