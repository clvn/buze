#pragma once

struct fade_plugin_info : zzub::info {
	fade_plugin_info() {
		flags = zzub_plugin_flag_is_offline;
		this->name = "Fade/Gain";
		this->short_name = "FadeGain";
		this->author = "andyw";
		this->uri = "@zzub.org/offline/fade";
		this->min_tracks = 0;
		this->max_tracks = 0;
		this->outputs = 0;
		this->inputs = 0;

		add_global_parameter()
			.set_word()
			.set_value_min(0)
			.set_value_max(2000)
			.set_value_none(65535)
			.set_value_default(1000)
			.set_name("Start Amp")
			.set_description("Start Amp (0=0%, 100=100%, 2000=200%)")
			.set_state_flag()
		;
		add_global_parameter()
			.set_word()
			.set_value_min(0)
			.set_value_max(2000)
			.set_value_none(65535)
			.set_value_default(1000)
			.set_name("End Amp")
			.set_description("End Amp (0=0%, 100=100%, 2000=200%)")
			.set_state_flag()
		;
	}

	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

struct reverse_plugin_info : zzub::info {
	reverse_plugin_info() {
		flags = zzub_plugin_flag_is_offline;
		this->name = "Reverse";
		this->short_name = "Reverse";
		this->author = "andyw";
		this->uri = "@zzub.org/offline/reverse";
		this->min_tracks = 0;
		this->max_tracks = 0;
		this->outputs = 0;
		this->inputs = 0;
	}

	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};


struct loopxfade_plugin_info : zzub::info {
	loopxfade_plugin_info() {
		flags = zzub_plugin_flag_is_offline;
		this->name = "Loop Crossfade";
		this->short_name = "Crossfade";
		this->author = "andyw";
		this->uri = "@zzub.org/offline/loopxfade";
		this->min_tracks = 0;
		this->max_tracks = 0;
		this->outputs = 0;
		this->inputs = 0;
	}

	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};

struct soundtouch_plugin_info : zzub::info {
	soundtouch_plugin_info() {
		flags = zzub_plugin_flag_is_offline;
		this->name = "Timestretch/Pitchshift (SoundTouch)";
		this->short_name = "Timestretch/Pitch";
		this->author = "SoundTouch library Copyright Olli Parviainen 2001-2011. Wrapped by andyw.";
		this->uri = "@zzub.org/offline/soundtouch";
		this->min_tracks = 0;
		this->max_tracks = 0;
		this->outputs = 0;
		this->inputs = 0;

		add_global_parameter()
			.set_word()
			.set_value_min(0)
			.set_value_max(1900) // set to 1900 to make slider more accurate/usable
			.set_value_none(65535)
			.set_value_default(950)
			.set_name("Tempo Change")
			.set_description("Tempo Change (0=-95%, 950=0%/no change, 1900=+95%")
			.set_state_flag()
		;

		add_global_parameter()
			.set_byte()
			.set_value_min(0)
			.set_value_max(120)
			.set_value_none(255)
			.set_value_default(60)
			.set_name("Pitch")
			.set_description("Pitch Semitones (0=-60 semitones, 60=0 semitones, 120=60 semitones)")
			.set_state_flag()
		;

 		add_global_parameter()
			.set_word()
			.set_value_min(0)
			.set_value_max(1900) // set to 1900 to make slider more accurate/usable
			.set_value_none(65535)
			.set_value_default(950)
			.set_name("Pitch Finetune")
			.set_description("Pitch Finetune (0=-95%, 950=0%/no change, 1900=+95%")
			.set_state_flag()
		;

		add_global_parameter()
			.set_byte()
			.set_value_min(0)
			.set_value_max(100)
			.set_value_none(255)
			.set_value_default(1)
			.set_name("Sequence")
			.set_description("Sequence ms (0=auto, 1-100=ms)")
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_value_min(0)
			.set_value_max(100)
			.set_value_none(255)
			.set_value_default(35)
			.set_name("Seek Window")
			.set_description("Seek Window (0=auto, 1-100=ms)")
			.set_state_flag();

		add_global_parameter()
			.set_byte()
			.set_value_min(1)
			.set_value_max(100)
			.set_value_none(255)
			.set_value_default(10)
			.set_name("Overlap")
			.set_description("Overlap ms (1-100ms)")
			.set_state_flag();
		;

		add_global_parameter()
			.set_switch()
			.set_value_default(0)
			.set_name("QuickMode")
			.set_description("Quick Mode: 0=Off, 1=Faster but reduced quality")
			.set_state_flag()
		;


	}

	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};