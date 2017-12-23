/*
Copyright (C) 2010 Anders Ervik <calvin@countzero.no>

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

struct modplug_plugin_info : zzub::info {

	enum {
		modplug_max_tracks = 64
	};

	modplug_plugin_info() {
		this->flags = zzub_plugin_flag_has_audio_output;
		this->name = "Modplug";
		this->short_name = "Modplug";
		this->author = "Olivier Lapicque, OpenMPT devs. Ported by Andy Werk";
		this->uri = "@zzub.org/modplug;1";
		this->outputs = 2;
		this->inputs = 0;
		this->min_tracks = 1;
		this->max_tracks = modplug_max_tracks;

		add_attribute()
			.set_name("Mode: 0=All, 1=IT, 2=FT2, 3=ST, 4=PT")
			.set_value_min(0)
			.set_value_max(4)
			.set_value_default(0);

		add_attribute()
			.set_name("Resample: 0=Nearest, 1=Linear, 2=Spline")
			.set_value_min(0)
			.set_value_max(2)
			.set_value_default(2);

		add_track_parameter()
			.set_note()
			.set_name("Note")
			.set_description("Note");

		add_track_parameter()
			.set_byte()
			.set_name("Wave")
			.set_description("Wave")
			.set_value_min(1)
			.set_value_max(200)
			.set_value_none(0)
			.set_value_default(0)
			.set_wavetable_index_flag();
		
		add_track_parameter()
			.set_byte()
			.set_name("Volfx")
			.set_description("Volume Effect")
			.set_value_min(0)
			.set_value_max(254)
			.set_value_none(255);

		add_track_parameter()
			.set_byte()
			.set_name("Volume")
			.set_description("Volume")
			.set_value_min(0)
			.set_value_max(254)
			.set_value_none(255);

		add_track_parameter()
			.set_byte()
			.set_name("Effect")
			.set_description("Effect")
			.set_value_min(0)
			.set_value_max(254)
			.set_value_none(255);

		add_track_parameter()
			.set_byte()
			.set_name("Value")
			.set_description("Effect Value")
			.set_value_min(0)
			.set_value_max(254)
			.set_value_none(255);

	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

