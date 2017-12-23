/*
Copyright (C) 2003-2007 Anders Ervik <calvin@countzero.no>

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

#pragma once

struct recorder_wavetable_plugin_info : zzub::info {

	recorder_wavetable_plugin_info() {
		this->flags = 
			zzub_plugin_flag_has_audio_input |	// accepts audio input connections
			zzub_plugin_flag_is_encoder;		// receive buffered input

		this->name = "Wavetable Recorder 2.0";
		this->short_name = "Recorder";
		this->author = "n/a";
		this->uri = "@zzub.org/recorder/wavetable;2";
		this->inputs = 2;
		this->outputs = 0;

		add_global_parameter()
			.set_switch()
			.set_name("Record")
			.set_description("Start/Stop Recording")
			.set_value_default(zzub_switch_value_on)
			.set_state_flag();

		add_global_parameter()
			.set_switch()
			.set_name("Record Mode")
			.set_description("Record Mode (0=automatic, 1=manual)")
			.set_value_default(zzub_switch_value_off)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Format")
			.set_description("Format (0=16bit, 1=32bit float, 2=32bit integer, 3=24bit)")
			.set_value_default(0)
			.set_value_min(0)
			.set_value_max(3)
			.set_value_none(255)
			.set_state_flag();

		add_global_parameter()
			.set_wavetable_index()
			.set_value_default(1)
			.set_state_flag();
/*
		add_attribute()
			.set_name("Wavetable Lock")
			.set_value_min(0)
			.set_value_max(1)
			.set_value_default(0);*/
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

struct recorder_file_plugin_info : zzub::info {

	recorder_file_plugin_info() {
		this->flags = 
			zzub_plugin_flag_has_audio_input |	// accepts audio input connections
			zzub_plugin_flag_stream |			// expose "stream source" in plugin properties
			zzub_plugin_flag_is_encoder;		// receive buffered input

		this->name = "File Recorder 2.0";
		this->short_name = "Recorder";
		this->author = "n/a";
		this->uri = "@zzub.org/recorder/file;2";
		this->inputs = 2;
		this->outputs = 0;

		add_global_parameter()
			.set_switch()
			.set_name("Record")
			.set_description("Start/Stop Recording")
			.set_value_default(zzub_switch_value_on)
			.set_state_flag();

		add_global_parameter()
			.set_switch()
			.set_name("Record Mode")
			.set_description("Record Mode (0=automatic, 1=manual)")
			.set_value_default(zzub_switch_value_off)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Format")
			.set_description("Format (0=16bit, 1=32bit float, 2=32bit integer, 3=24bit)")
			.set_value_default(0)
			.set_value_min(0)
			.set_value_max(3)
			.set_value_none(255)
			.set_state_flag();
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

struct visualizer_plugin_info : zzub::info {
	
	visualizer_plugin_info() {
		this->flags = zzub_plugin_flag_has_audio_input | zzub_plugin_flag_is_encoder;
		this->name = "Visualizer";
		this->short_name = "Visualizer";
		this->author = "andyw";
		this->uri = "@zzub.org/recorder/visualizer";
		this->inputs = 2;
		this->outputs = 0;
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

