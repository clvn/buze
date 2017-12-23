#pragma once

namespace miditime {

#pragma pack(1)										// Place to retrieve parameters	

struct gvals {
	int dummy;
};

#pragma pack()

struct miditimemachine;


struct miditimemachine : public zzub::plugin {
	enum {
		max_tracks = 16,
	};

	gvals gval;
	int playing;
	int last_play_pos;

	miditimemachine();
	virtual ~miditimemachine() { }
	virtual void init(zzub::archive* pi);
	virtual void process_interval();
	virtual int get_interval_size();
	void midi_out(int time, unsigned int data);
	void midi_tick();
};

struct miditime_info : zzub::info {
	miditime_info() {
		this->flags = zzub_plugin_flag_has_midi_output | zzub_plugin_flag_is_interval;
		this->name = "zzub miditime";
		this->short_name = "miditime";
		this->author = "Lauri Koponen <ld0d@iki.fi>";
		this->uri = "@zzub.org/miditime;1";
		this->commands = "";
		this->min_tracks = 0;
		this->max_tracks = 0;
		this->inputs = 0;
		this->outputs = 0;
	}
	virtual zzub::plugin* create_plugin() { return new miditimemachine(); }
	virtual bool store_info(zzub::archive *data) const { return false; }
};

}
