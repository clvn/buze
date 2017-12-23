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

struct output_plugin_info : zzub::info {
	output_plugin_info() {
		this->flags = zzub_plugin_flag_has_audio_input;
		this->name = "Audio Output, Stereo";
		this->short_name = "Output";
		this->author = "Armstrong Development Team";
		this->uri = "@zzub.org/output";
		this->commands = "/Output Channel";
		this->inputs = 2;
		this->outputs = 0;
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};


struct output16_plugin_info : zzub::info {
	output16_plugin_info() {
		this->flags = zzub_plugin_flag_has_audio_input | zzub_plugin_flag_is_singleton;
		this->name = "Audio Output, 16 Channels";
		this->short_name = "Output16";
		this->author = "Armstrong Development Team";
		this->uri = "@zzub.org/output16";
		this->inputs = 16;
		this->outputs = 0;
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};


struct output32_plugin_info : zzub::info {
	output32_plugin_info() {
		this->flags = zzub_plugin_flag_has_audio_input | zzub_plugin_flag_is_singleton;
		this->name = "Audio Output, 32 Channels";
		this->short_name = "Output32";
		this->author = "Armstrong Development Team";
		this->uri = "@zzub.org/output32";
		this->inputs = 32;
		this->outputs = 0;
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};
