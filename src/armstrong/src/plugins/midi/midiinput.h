#pragma once

namespace midiinput {

#pragma pack(1)										// Place to retrieve parameters	

struct gvals {
};

struct tvals {
};

#pragma pack()

struct midiinput : public zzub::plugin {
	gvals gval;
	
	midiinput();
	virtual ~midiinput() { }
	void midi_out(int time, unsigned int data);
	virtual int get_interval_size();
	virtual void process_midi_events(zzub::midi_message* pin, int nummessages);
};

struct midiinput_info : zzub::info {
	midiinput_info() {
		this->flags = zzub_plugin_flag_has_midi_output | zzub_plugin_flag_is_interval;
		this->name = "MIDI Input";
		this->short_name = "MidiIn";
		this->author = "calvin";
		this->uri = "@zzub.org/midiinput;1";
		this->min_tracks = 0;
		this->max_tracks = 0;
		this->inputs = 0;
		this->outputs = 0;
	}

	virtual zzub::plugin* create_plugin() { return new midiinput(); }
	virtual bool store_info(zzub::archive *data) const { return false; }
};

}
