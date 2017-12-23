#pragma once

namespace midioutput {

#pragma pack(1)										// Place to retrieve parameters	

struct gvals {
};

struct tvals {
};

#pragma pack()

struct midioutput : public zzub::plugin {

	gvals gval;
	int current_device;
	
	midioutput();
	virtual ~midioutput() { }
	virtual void init(zzub::archive*);
	virtual void save(zzub::archive*);
	virtual void get_midi_output_names(zzub::outstream *pout);
	virtual void process_midi_events(zzub::midi_message* pin, int nummessages);
	virtual void get_sub_menu(int i, zzub::outstream* os);
	virtual void command(int index);
	void set_midi_device(std::string name);
};

struct midioutput_info : zzub::info {
	midioutput_info() {
		this->flags = zzub_plugin_flag_has_midi_input;
		this->name = "MIDI Output";
		this->short_name = "MidiOut";
		this->author = "calvin";
		this->uri = "@zzub.org/midioutput;1";
		this->commands = "/MIDI Device";
		this->min_tracks = 0;
		this->max_tracks = 0;
		this->inputs = 0;
		this->outputs = 0;
	}

	virtual zzub::plugin* create_plugin() { return new midioutput(); }
	virtual bool store_info(zzub::archive *data) const { return false; }
};

}
