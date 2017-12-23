#pragma once

const int pattern_plugin_max_tracks = 128;

struct pattern_plugin_info : zzub::info {

	pattern_plugin_info() {
		flags = zzub_plugin_flag_is_sequence;
		this->name = "Pattern Player";
		this->short_name = "Pattern";
		this->author = "n/a";
		this->uri = "@zzub.org/sequence/pattern";
		this->min_tracks = 1;
		this->max_tracks = pattern_plugin_max_tracks;
		this->outputs = 0;
		this->inputs = 0;

		add_global_parameter()
			.set_word()
			.set_value_min(0)
			.set_value_max(512)
			.set_value_none(65535)
			.set_value_default(0)
			.set_name("BPM")
			.set_description("Beats Per Minute (1-512, 0=sync to timesource)")
			.set_state_flag()
		;
		add_global_parameter()
			.set_byte()
			.set_value_min(0)
			.set_value_max(254)
			.set_value_none(255)
			.set_value_default(0)
			.set_name("TPB")
			.set_description("Ticks Per Beat (1-254, 0=sync to timesource)")
			.set_state_flag()
		;

		add_global_parameter()
			.set_byte()
			.set_value_min(0)
			.set_value_max(99)
			.set_value_none(255)
			.set_value_default(0)
			.set_name("Swing")
			.set_description("Swing Offset (1-99%, 0=sync to timesource, 50=no swing)")
			.set_state_flag()
		;

		add_global_parameter()
			.set_byte()
			.set_value_min(0)
			.set_value_max(32)
			.set_value_none(255)
			.set_value_default(0)
			.set_name("Swing Ticks")
			.set_description("Ticks Per Swing (1-32, 0=sync to timesource)")
			.set_state_flag()
		;

		add_track_parameter()
			.set_word()
			.set_value_min(0)
			.set_value_max(65534)
			.set_value_none(65535)
			.set_value_default(65535)
			.set_name("Pattern Trigger")
			.set_description("Pattern Trigger")
			.set_flags(zzub_parameter_flag_pattern_index)
		;

		add_track_parameter()
			.set_note()
			.set_name("Transpose")
			.set_description("Transpose pattern relative to C-4")
		;

		add_track_parameter()
			.set_word()
			.set_name("Offset")
			.set_description("Row to play from")
		;

		add_track_parameter()
			.set_word()
			.set_name("Duration")
			.set_description("Number of rows to play, 0 = entire pattern")
			.set_value_default(0)
			.set_state_flag()
		;
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

