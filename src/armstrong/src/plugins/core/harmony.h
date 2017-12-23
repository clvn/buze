#pragma once

static int const harmony_plugin_max_tracks = 16;

struct harmony_plugin_info : zzub::info
{
	harmony_plugin_info() {
		flags = 0;
		this->name = "Harmony";
		this->short_name = "Harmony";
		this->author = "Megzlna";
		this->uri = "@zzub.org/harmony";
		this->min_tracks = 1;
		this->max_tracks = harmony_plugin_max_tracks;
		this->outputs = 0;
		this->inputs = 0;

		add_track_parameter()
			.set_word()
			.set_value_min(0)
			.set_value_max(293)
			.set_value_none(0xFFFF)
			.set_value_default(0)
			.set_name("Symbol")
			.set_description("Harmonic Symbol")
			.set_flags(zzub_parameter_flag_harmony_index)
		;
	}

	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive*) const { return false; }
};
