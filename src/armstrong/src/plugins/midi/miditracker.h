#pragma once

namespace miditracker {

#pragma pack(1)										// Place to retrieve parameters	

struct gvals {
	unsigned short wet;
	unsigned char pan;
	unsigned char global;
	unsigned short globalValue;
	unsigned short program;
};

struct tvals {
	unsigned char note;
	unsigned char velocity;
	unsigned char delay;
	unsigned char cut;
	unsigned char command;
	unsigned short commandValue;
	unsigned short parameter;
	unsigned short parameterValue;
	unsigned char midiChannel;
};

#pragma pack()

struct miditracker;

struct miditrack {
	tvals* values;
	miditracker* tracker;
	//int note;
	int last_note;
	int last_channel;
	int velocity;
	//int note_delay;
	//int note_cut;
	//int command;
	//int commandValue;
	//int parameter;
	//int parameterValue;
	int midi_channel;

	miditrack();
	void tick();
	//void process_stereo(int numsamples);
};


struct miditracker : public zzub::plugin {
	enum {
		max_tracks = 64,
	};

	gvals gval;
	tvals tval[max_tracks];
	miditrack tracks[max_tracks];
	int num_tracks;
	int samples_per_tick;
	//int samples_in_tick;
	//int open_device;
	std::vector<std::string> devices;

	miditracker();
	virtual ~miditracker() { }

	virtual void init(zzub::archive* pi);
	//virtual void load(zzub::archive*);
	//virtual void save(zzub::archive*);
	virtual void process_events();
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode);
	virtual bool process_offline(float **pin, float **pout, int *numsamples, int *channels, int *samplerate) { return false; }
	virtual const char * describe_value(int param, int value); 
	virtual void set_track_count(int i);
	virtual void command(int);
	virtual void stop();

	void midi_out(int time, unsigned int data);
};


struct miditracker_info : zzub::info {

	const zzub::parameter *paraWet;
	const zzub::parameter *paraPanning;
	const zzub::parameter *paraGlobalCommand;
	const zzub::parameter *paraGlobalCommandValue ;
	const zzub::parameter *paraProgram;

	const zzub::parameter *paraNote;
	const zzub::parameter *paraVelocity;
	const zzub::parameter *paraDelay;
	const zzub::parameter *paraCut;
	const zzub::parameter *paraCommand;
	const zzub::parameter *paraCommandValue;
	const zzub::parameter *paraParameter;
	const zzub::parameter *paraParameterValue;
	const zzub::parameter *paraMidiChannel;

	miditracker_info() {
		this->flags = zzub_plugin_flag_has_midi_output;
		this->name = "zzub miditracker";
		this->short_name = "miditracker";
		this->author = "Calvin, Zoner";
		this->uri = "@zzub.org/miditracker;1";
		this->commands = "/MIDI Device";
		this->min_tracks = 1;
		this->max_tracks = miditracker::max_tracks;
		this->inputs = 0;
		this->outputs = 0;
		
		paraWet = &add_global_parameter()
			.set_word()
			.set_name("Wet Out")
			.set_description("Wet Out")
			.set_value_min(0x00)
			.set_value_max(0x190)
			.set_value_none(0xffff)
			.set_state_flag()
			.set_value_default(0x64);
		
		paraPanning = &add_global_parameter()
			.set_byte()
			.set_name("Panning")
			.set_description("Panning")
			.set_value_min(0x00)
			.set_value_max(0x80)
			.set_value_none(0xff)
			.set_state_flag()
			.set_value_default(0x40);

		paraGlobalCommand = &add_global_parameter()
			.set_byte()
			.set_name("Global Command")
			.set_description("Global Command")
			.set_value_min(0)
			.set_value_max(0xfe)
			.set_value_none(0xff)
			.set_state_flag()
			.set_value_default(0xff);

		paraGlobalCommandValue = &add_global_parameter()
			.set_word()
			.set_name("Command Value")
			.set_description("Global Command Value")
			.set_value_min(0)
			.set_value_max(0xfffe)
			.set_value_none(0xffff)
			.set_state_flag()
			.set_value_default(0xffff);

		paraProgram = &add_global_parameter()
			.set_word()
			.set_name("Program")
			.set_description("Program")
			.set_value_min(1)
			.set_value_max(0x80)
			.set_value_none(0)
			.set_state_flag()
			.set_value_default(0x0);

		paraNote = &add_track_parameter()
			.set_note()
			.set_name("Note")
			.set_description("Note")
			.set_value_min(zzub_note_value_min)
			.set_value_max(zzub_note_value_max)
			.set_value_none(zzub_note_value_none)
			.set_flags(0)
			.set_value_default(0);

		paraVelocity = &add_track_parameter()
			.set_byte()
			.set_name("Velocity")
			.set_description("Velocity")
			.set_value_min(0x01)
			.set_value_max(0x7f)
			.set_value_none(0xff)
			.set_value_default(0x7f)
			.set_velocity_index_flag();

		paraDelay = &add_track_parameter()
			.set_byte()
			.set_name("Note Delay")
			.set_description("Note Delay")
			.set_value_min(0x00)
			.set_value_max(0xff)
			.set_value_none(0x00)
			.set_value_default(0);

		paraCut = &add_track_parameter()
			.set_byte()
			.set_name("Note Cut")
			.set_description("Note Cut")
			.set_value_min(0)
			.set_value_max(0xff)
			.set_value_none(0)
			.set_value_default(0);

		paraCommand = &add_track_parameter()
			.set_byte()
			.set_name("Command")
			.set_description("Track Command")
			.set_value_min(0)
			.set_value_max(0xfe)
			.set_value_none(0xff)
			.set_value_default(0xff);

		paraCommandValue = &add_track_parameter()
			.set_word()
			.set_name("Command Value")
			.set_description("Track Command Value")
			.set_value_min(0)
			.set_value_max(0xfffe)
			.set_value_none(0xffff)
			.set_value_default(0xffff);

		paraParameter = &add_track_parameter()
			.set_word()
			.set_name("Parameter")
			.set_description("Track Parameter")
			.set_value_min(0)
			.set_value_max(0x30fe)
			.set_value_none(0x30ff)
			.set_state_flag()
			.set_value_default(0x30ff);

		paraParameterValue = &add_track_parameter()
			.set_word()
			.set_name("Parameter Value")
			.set_description("Track Parameter Value")
			.set_value_min(0)
			.set_value_max(127)
			.set_value_none(128)
			.set_state_flag()
			.set_value_default(128);

		paraMidiChannel = &add_track_parameter()
			.set_byte()
			.set_name("MIDI Channel")
			.set_description("MIDI Channel")
			.set_value_min(1)
			.set_value_max(0x10)
			.set_value_none(0xff)
			.set_state_flag()
			.set_value_default(1);

	}
	virtual zzub::plugin* create_plugin() { return new miditracker(); }
	virtual bool store_info(zzub::archive *data) const { return false; }
};

}
