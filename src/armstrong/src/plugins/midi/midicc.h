#pragma once

namespace midicc {

#pragma pack(1)										// Place to retrieve parameters	

struct gvals {
	unsigned char smooth;
	unsigned char auto_learn;
};

struct tvals {
	unsigned char channel;
	unsigned char cc;
	unsigned char value;
};

#pragma pack()

struct midicc : public zzub::plugin {
	enum {
		max_tracks = 16,
	};

	gvals gval;
	tvals tval[max_tracks];
	int num_tracks;

	int updatecounter;
	
	int auto_learn;
	int smooth;
	
	struct {
		int channel;
		int cc;
		int prevvalue;
		int updated;
	} track[max_tracks];

	midicc();
	virtual ~midicc() { }

	virtual void process_events();
	virtual void set_track_count(int i);
	void midi_event(unsigned short status, unsigned char data1, unsigned char data2);
	void midi_out(int time, unsigned int data);
};

struct midicc_info : zzub::info {
	midicc_info() {
		this->flags = zzub_plugin_flag_has_midi_output;
		this->name = "zzub midicc";
		this->short_name = "midicc";
		this->author = "Lauri Koponen <ld0d@iki.fi>";
		this->uri = "@zzub.org/midicc;1";
		//this->commands = "/MIDI Device";
		this->min_tracks = 8;
		this->max_tracks = 16;
		this->inputs = 0;
		this->outputs = 0;

		add_global_parameter()
			.set_switch()
			.set_value_default(zzub_switch_value_off)
			.set_name("Smooth")
			.set_description("Smooth changes")
			.set_state_flag();

		add_global_parameter()
			.set_switch()
			.set_value_default(zzub_switch_value_off)
			.set_name("Auto learn")
			.set_description("Auto learn controllers")
			.set_state_flag();

		add_track_parameter()
			.set_byte()
			.set_name("Channel")
			.set_description("Midi channel")
			.set_value_min(1)
			.set_value_max(16)
			.set_value_none(0xff)
			.set_value_default(1)
			.set_state_flag();

		add_track_parameter()
			.set_byte()
			.set_name("CC")
			.set_description("Controller")
			.set_value_min(0)
			.set_value_max(127)
			.set_value_none(0xff)
			.set_value_default(0)
			.set_state_flag();

		add_track_parameter()
			.set_byte()
			.set_name("Value")
			.set_description("Controller value")
			.set_value_min(0)
			.set_value_max(127)
			.set_value_none(0xff)
			.set_value_default(0)
			.set_state_flag();
	}

	virtual zzub::plugin* create_plugin() { return new midicc(); }
	virtual bool store_info(zzub::archive *data) const { return false; }
};

}
