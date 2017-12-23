/*
Copyright (C) 2011 Anders Ervik <calvin@countzero.no>

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


struct note_plugin_info : zzub::info {
	note_plugin_info() {
		this->flags = zzub_plugin_flag_has_note_output | zzub_plugin_flag_has_midi_output | zzub_plugin_flag_has_midi_input;
		this->name = "Note Generator";
		this->short_name = "Note";
		this->author = "andyw, Leonard Ritter (contact at leonard-ritter+com)";
		this->uri = "@zzub.org/notegen";
		this->inputs = 0;
		this->outputs = 0;
		this->min_tracks = 1;
		this->max_tracks = 128;

		add_global_parameter()
			.set_byte()
			.set_name("Global Octave")
			.set_description("Transpose note value globally (octave-wise)")
			.set_value_min(0)
			.set_value_max(10)
			.set_value_none(255)
			.set_value_default(5)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Global Note")
			.set_description("Transpose note value globally (note-wise)")
			.set_value_min(0)
			.set_value_max(12)
			.set_value_none(255)
			.set_value_default(6)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Harmonic Quantize")
			.set_description("Quantize note harmonically")
			.set_value_min(0)
			.set_value_max(2)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("C")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("C#")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(1)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("D")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(2)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("D#")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(3)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("E")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(4)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("F")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(5)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("F#")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(6)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("G")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(7)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("G#")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(8)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("A")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(9)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("A#")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(10)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("B")
			.set_description("Transpose just a single note")
			.set_value_min(0)
			.set_value_max(11)
			.set_value_none(255)
			.set_value_default(11)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Allow Min")
			.set_description("Min allowed input value")
			.set_value_min(0)
			.set_value_max(119)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Allow Max")
			.set_description("Max allowed input value")
			.set_value_min(0)
			.set_value_max(119)
			.set_value_none(255)
			.set_value_default(119)
			.set_state_flag();

		add_track_parameter()
			.set_note()
			.set_name("Note")
			.set_description("Note")
			.set_value_min(zzub_note_value_min)
			.set_value_max(zzub_note_value_max)
			.set_value_none(zzub_note_value_none)
			.set_flags(0)
			.set_value_default(0);

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
			.set_name("Velocity")
			.set_description("Velocity")
			.set_value_min(0x01)
			.set_value_max(0x7f)
			.set_value_none(0xff)
			.set_value_default(0x7f)
			.set_velocity_index_flag();
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

