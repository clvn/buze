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

#include <sstream>

struct value_plugin_info : zzub::info {
	value_plugin_info() {
		this->flags = zzub_plugin_flag_has_event_output;
		this->name = "Value Generator";
		this->short_name = "Value";
		this->author = "n/a";
		this->uri = "@zzub.org/peer/wordvalue";
		this->inputs = 0;
		this->outputs = 0;
		this->min_tracks = 0;
		this->max_tracks = 8;

		add_global_parameter()
			.set_word()
			.set_name("Value")
			.set_description("Word Value")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65534 / 2)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Seed")
			.set_description("Random Generator Seed (used with rnd)")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65534 / 2)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Threshold")
			.set_description("Skips n values for every outputted value")
			.set_value_min(0)
			.set_value_max(254)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Allow Min")
			.set_description("Min value in allowed output range")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Allow Max")
			.set_description("Max value in allowed output range")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65534)
			.set_state_flag();

		add_track_parameter()
			.set_byte()
			.set_name("Operator")
			.set_description("Operator (0=add, 1=sub, 2=mul, 3=div, 4=mod, 5=neg, 6=rnd, 7=scale, 8=min, 9=max)")
			.set_value_min(0)
			.set_value_max(9)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_track_parameter()
			.set_word()
			.set_name("Op Value")
			.set_description("Operator Value (ignored on neg and rnd)")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(0)
			.set_state_flag();

		add_controller_parameter()
			.set_word()
			.set_name("ValueOut")
			.set_description("Value Output")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535);
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

struct value_mapper_plugin_info : zzub::info {
	value_mapper_plugin_info() {
		this->flags = zzub_plugin_flag_has_event_output;
		this->name = "Value Mapper";
		this->short_name = "ValueMapper";
		this->author = "n/a";
		this->uri = "@zzub.org/peer/valuemapper";
		this->inputs = 0;
		this->outputs = 0;
		this->min_tracks = 0;
		this->max_tracks = 128;

		add_global_parameter()
			.set_word()
			.set_name("Value #1")
			.set_description("Word Input Value #1")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65534 / 2)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Value #2")
			.set_description("Word Input Value #2")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65534 / 2)
			.set_state_flag();

		add_track_parameter()
			.set_word()
			.set_name("Input 1 Start")
			.set_description("Input Value 1 Range Start")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(0)
			.set_state_flag();

		add_track_parameter()
			.set_word()
			.set_name("Input 1 End")
			.set_description("Input Value 1 Range End")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535)
			.set_state_flag();

		add_track_parameter()
			.set_word()
			.set_name("Output 1 Start")
			.set_description("Output Value 1 Range Start")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(0)
			.set_state_flag();

		add_track_parameter()
			.set_word()
			.set_name("Output 1 End")
			.set_description("Output Value 1 Range End")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535)
			.set_state_flag();

		add_track_parameter()
			.set_word()
			.set_name("Input 2 Start")
			.set_description("Input Value 2 Range Start")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(0)
			.set_state_flag();

		add_track_parameter()
			.set_word()
			.set_name("Input 2 End")
			.set_description("Input Value 2 Range End")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535)
			.set_state_flag();

		add_track_parameter()
			.set_word()
			.set_name("Output 2 Start")
			.set_description("Output Value 2 Range Start")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(0)
			.set_state_flag();

		add_track_parameter()
			.set_word()
			.set_name("Output 2 End")
			.set_description("Output Value 2 Range End")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535)
			.set_state_flag();

		add_controller_parameter()
			.set_word()
			.set_name("ValueOut1")
			.set_description("Mapped Value #1")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535);

		add_controller_parameter()
			.set_word()
			.set_name("ValueOut2")
			.set_description("Mapped Value #2")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535);
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};


struct lfo_plugin_info : zzub::info {
	lfo_plugin_info() {
		this->flags = zzub_plugin_flag_has_event_output | zzub_plugin_flag_is_interval ;
		this->name = "LFO Value Generator";
		this->short_name = "LFO";
		this->author = "n/a";
		this->uri = "@zzub.org/peer/lfo";
		this->inputs = 0;
		this->outputs = 0;
		this->min_tracks = 0;
		this->max_tracks = 0;

		add_global_parameter()
			.set_byte()
			.set_name("Interval")
			.set_description("Interval Type (0=Ticks, 1=Ticks/256, 2=Sec/16)")
			.set_value_min(0)
			.set_value_max(2)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Length")
			.set_description("Interval Length")
			.set_value_min(1)
			.set_value_max(254)
			.set_value_none(255)
			.set_value_default(1)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Frequency")
			.set_description("Frequency (1=0.03hz, 20=1hz, 80=4hz, FF=7.96hz)")
			.set_value_min(1)
			.set_value_max(255)
			.set_value_none(0)
			.set_value_default(32)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Amplitude")
			.set_description("Amplitude")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65534)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Minimum")
			.set_description("Minimum")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Type")
			.set_description("LFO Type (0=sine, 1=square, 2=triangle, 3=saw, 4=random")
			.set_value_min(0)
			.set_value_max(4)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Seed")
			.set_description("Random Number Generator Seed (used with type = random)")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65534 / 2)
			.set_state_flag();

		add_controller_parameter()
			.set_word()
			.set_name("LfoOut")
			.set_description("LFO Output")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535);

	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};


struct signal_value_plugin_info : zzub::info {
	signal_value_plugin_info() {
		this->flags = zzub_plugin_flag_has_audio_input | zzub_plugin_flag_has_event_output | zzub_plugin_flag_is_interval ;
		this->name = "Signal Value Generator";
		this->short_name = "Signal";
		this->author = "n/a";
		this->uri = "@zzub.org/peer/signal";
		this->inputs = 1;
		this->outputs = 0;
		this->min_tracks = 0;
		this->max_tracks = 0;

		add_global_parameter()
			.set_byte()
			.set_name("Interval")
			.set_description("Interval Type (0=Ticks, 1=Ticks/256, 2=Sec/16)")
			.set_value_min(0)
			.set_value_max(2)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Length")
			.set_description("Interval Length")
			.set_value_min(1)
			.set_value_max(254)
			.set_value_none(255)
			.set_value_default(1)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Mode")
			.set_description("Mode (0=envelope, 1=immediate, 2=absolute)")
			.set_value_min(0)
			.set_value_max(2)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_controller_parameter()
			.set_word()
			.set_name("Out")
			.set_description("Output")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535);

	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

struct adsr_value_plugin_info : zzub::info {
	adsr_value_plugin_info() {
		this->flags = zzub_plugin_flag_has_event_output | zzub_plugin_flag_is_interval ;
		this->name = "ADSR Value Generator";
		this->short_name = "ADSR";
		this->author = "n/a";
		this->uri = "@zzub.org/peer/adsr";
		this->inputs = 0;
		this->outputs = 0;
		this->min_tracks = 0;
		this->max_tracks = 0;

		add_global_parameter()
			.set_byte()
			.set_name("Interval")
			.set_description("Interval Type (0=Ticks, 1=Ticks/256, 2=Sec/16)")
			.set_value_min(0)
			.set_value_max(2)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Length")
			.set_description("Interval Length")
			.set_value_min(1)
			.set_value_max(254)
			.set_value_none(255)
			.set_value_default(1)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Attack")
			.set_description("Attack")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(10000)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Decay")
			.set_description("Decay")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(20000)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Sustain")
			.set_description("Sustain")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(13000)
			.set_state_flag();

		add_global_parameter()
			.set_word()
			.set_name("Release")
			.set_description("Relase")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(5000)
			.set_state_flag();

		add_global_parameter()
			.set_switch()
			.set_name("Trigger")
			.set_description("Trigger")
			.set_value_default(0)
			.set_state_flag();

		add_controller_parameter()
			.set_word()
			.set_name("AdsrOut")
			.set_description("ADSR Output")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535);

	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

#if defined(_WIN32)

struct joystickinfo {
	GUID id;
	std::string name;
};

struct joystick_value_plugin_info : zzub::info {
	LPDIRECTINPUT8 di;
	std::vector<joystickinfo> joystickinfos;

	joystick_value_plugin_info() {
		di = 0;

		this->flags = zzub_plugin_flag_has_event_output | zzub_plugin_flag_is_interval ;
		this->name = "Joystick Value Generator";
		this->short_name = "Joystick";
		this->author = "n/a";
		this->uri = "@zzub.org/peer/joystick";
		this->inputs = 0;
		this->outputs = 0;
		this->min_tracks = 0;
		this->max_tracks = 0;

		add_global_parameter()
			.set_byte()
			.set_name("Interval")
			.set_description("Interval Type (0=Ticks, 1=Ticks/256, 2=Sec/16)")
			.set_value_min(0)
			.set_value_max(2)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Length")
			.set_description("Interval Length")
			.set_value_min(1)
			.set_value_max(254)
			.set_value_none(255)
			.set_value_default(1)
			.set_state_flag();

		add_controller_parameter()
			.set_word()
			.set_name("JoystickX")
			.set_description("Joystick X position")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535);

		add_controller_parameter()
			.set_word()
			.set_name("JoystickY")
			.set_description("Joystick Y position")
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535);

		add_controller_parameter()
			.set_switch()
			.set_name("Button0")
			.set_description("Joystick Button 0");

		add_controller_parameter()
			.set_switch()
			.set_name("Button1")
			.set_description("Joystick Button 1");

	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }

	bool init_joysticks();
	LPDIRECTINPUTDEVICE8 create_joystick(GUID id);
	HRESULT poll(LPDIRECTINPUTDEVICE8 joystick, DIJOYSTATE2 *js);
};
#endif

struct midinotevel_value_plugin_info : zzub::info {
	midinotevel_value_plugin_info() {
		this->flags = zzub_plugin_flag_has_event_output | zzub_plugin_flag_has_midi_input/* | zzub_plugin_flag_has_midi_output*/ | zzub_plugin_flag_is_interval;
		this->name = "MIDI Note Velocity Value Generator";
		this->short_name = "NoteVelVal";
		this->author = "n/a";
		this->uri = "@zzub.org/peer/midinotevelvalue";
		this->inputs = 0;
		this->outputs = 0;
		this->min_tracks = 0;
		this->max_tracks = 0;

		add_global_parameter()
			.set_byte()
			.set_name("Note")
			.set_description("Midi note")
			.set_value_min(0)
			.set_value_max(127)
			.set_value_none(255)
			.set_value_default(48)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("Channel")
			.set_description("Midi channel")
			.set_value_min(1)
			.set_value_max(16)
			.set_value_none(255)
			.set_value_default(1)
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_name("NoteOffToZero")
			.set_description("Map note off to zero volume")
			.set_value_min(0)
			.set_value_max(1)
			.set_value_none(255)
			.set_value_default(0)
			.set_state_flag();

		add_controller_parameter()
			.set_byte()
			.set_name("VelocityOut")
			.set_description("Velocity Output")
			.set_value_min(0)
			.set_value_max(127)
			.set_value_none(255)
			.set_value_default(255);
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
}; 

struct midictrl_value_plugin_info : zzub::info {
	midictrl_value_plugin_info() {
		this->flags = zzub_plugin_flag_has_event_output | zzub_plugin_flag_has_midi_input; // | zzub_plugin_flag_is_interval;
		this->name = "MIDI Controller Value Generator";
		this->short_name = "MidiCtrlValue";
		this->author = "n/a";
		this->uri = "@zzub.org/peer/midictrlvalue";
		this->inputs = 0;
		this->outputs = 0;
		this->min_tracks = 0;
		this->max_tracks = 0;

		add_global_parameter()
			.set_byte()
			.set_name("Channel")
			.set_description("MIDI channel")
			.set_value_min(1)
			.set_value_max(16)
			.set_value_none(255)
			.set_value_default(1)
			.set_state_flag();

		add_controller_parameter()
			.set_word()
			.set_name("Chn Aftertouch")
			.set_description("MIDI Channel Aftertouch")
			.set_value_min(0)
			.set_value_max(16383)
			.set_value_none(65535)
			.set_value_default(65535);

		add_controller_parameter()
			.set_word()
			.set_name("Pitchwheel")
			.set_description("MIDI Pitchwheel")
			.set_value_min(0)
			.set_value_max(16383)
			.set_value_none(65535)
			.set_value_default(65535);

		for (int i = 0; i < 32; i++) {
			std::stringstream namestrm, descstrm;
			namestrm << "CC#" << i << " (14bit)";
			descstrm << "MIDI CC# " << i << " (14bit, range 0-16383)";
			add_controller_parameter()
				.set_word()
				.set_name(namestrm.str().c_str())
				.set_description(descstrm.str().c_str())
				.set_value_min(0)
				.set_value_max(16383)
				.set_value_none(65535)
				.set_value_default(65535);
		}

		for (int i = 0; i < 128; i++) {
			std::stringstream namestrm, descstrm;
			namestrm << "CC#" << i << " (7bit)";
			descstrm << "MIDI CC# " << i << " (7bit, range 0-127)";
			add_controller_parameter()
				.set_byte()
				.set_name(namestrm.str().c_str())
				.set_description(descstrm.str().c_str())
				.set_value_min(0)
				.set_value_max(127)
				.set_value_none(255)
				.set_value_default(255);
		}
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
}; 
