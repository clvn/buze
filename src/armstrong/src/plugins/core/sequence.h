#pragma once

struct sequence_plugin_info : zzub::info
{
	sequence_plugin_info() {
		flags = zzub_plugin_flag_is_sequence | zzub_plugin_flag_is_singleton;
		this->name = "Sequence Player";
		this->short_name = "Sequence";
		this->author = "n/a";
		this->uri = "@zzub.org/sequence/sequence";
		this->min_tracks = 0;
		this->max_tracks = 0;
		this->outputs = 0;
		this->inputs = 0;

		add_global_parameter()
			.set_word()
			.set_value_min(1)
			.set_value_max(512)
			.set_value_none(65535)
			.set_value_default(126)
			.set_name("BPM")
			.set_description("Beats Per Minute (1-512)")
			.set_state_flag()
		;
		add_global_parameter()
			.set_byte()
			.set_value_min(1)
			.set_value_max(254)
			.set_value_none(255)
			.set_value_default(4)
			.set_name("TPB")
			.set_description("Ticks Per Beat (1-254)")
			.set_state_flag()
		;

		add_global_parameter()
			.set_byte()
			.set_value_min(1)
			.set_value_max(99)
			.set_value_none(255)
			.set_value_default(50)
			.set_name("Swing")
			.set_description("Swing Offset (1-99%, 50=no swing)")
			.set_state_flag()
		;

		add_global_parameter()
			.set_byte()
			.set_value_min(1)
			.set_value_max(32)
			.set_value_none(255)
			.set_value_default(4)
			.set_name("Swing Ticks")
			.set_description("Ticks Per Swing (1-32)")
			.set_state_flag()
		;
	}

	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};
