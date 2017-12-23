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

#pragma once

struct input_plugin_info : zzub::info {

	input_plugin_info() {
		this->flags = zzub_plugin_flag_has_audio_output;
		this->name = "Audio Input";
		this->short_name = "Input";
		this->author = "n/a";
		this->uri = "@zzub.org/input";
		this->commands = "/Input Channel";
		this->outputs = 2;
		this->inputs = 0;
		add_attribute()
			.set_name("Record Channel")
			.set_value_min(0)
			.set_value_max(32)
			.set_value_default(0);
		//~ add_global_parameter()
			//~ .set_byte()
			//~ .set_name("Channel")
			//~ .set_description("Recording channel")
			//~ .set_value_min(1)
			//~ .set_value_max(32)
			//~ .set_value_none(255)
			//~ .set_state_flag()
			//~ .set_value_default(1);

	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

