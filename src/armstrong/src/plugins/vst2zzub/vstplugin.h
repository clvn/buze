#pragma once

struct vstplugininfo;

struct vstplugin : zzub::plugin {
	enum {
		max_midievents = 1024
	};
	static std::map<AEffect*, vstplugin*> plugins;
	const vstplugininfo* _info;
	unsigned char* program;
	// currentprogram contains a copy of the current parameter values, used to prevent calling setParameter() with the same value, which fails e.g with "sanestation"
	unsigned char* currentprogram;
	volatile int set_new_program;
	HMODULE hPlugin;
	AEffect* effect;
	VstTimeInfo timeInfo;
	zzub::player_state timeInfoLastState;
	vsteditor editor;
	HWND hContainerWnd;
	std::vector<VstPinProperties> audioinputs;
	std::vector<VstPinProperties> audiooutputs;
	bool resizable;
	bool send_notesoff;
	CRITICAL_SECTION labelcs;
	VstEvents* vsteventbuffer;
	VstMidiEvent midieventbuffer[max_midievents];

	vstplugin(vstplugininfo* info);
	virtual void init(zzub::archive* arc);
	virtual void load(zzub::archive* arc);
	virtual void save(zzub::archive* arc);
	virtual void destroy();
	virtual void process_events();
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode);
	virtual void process_midi_events(zzub::midi_message* pin, int nummessages);
	virtual int get_latency();
	virtual void stop();
	virtual void get_midi_output_names(zzub::outstream *pout);
	virtual void get_sub_menu(int index, zzub::outstream* os);
	virtual void command(int index);
	virtual const char* describe_value(int param, int value);
	virtual const char* get_output_channel_name(int i);
	virtual const char* get_input_channel_name(int i);
	virtual bool has_embedded_gui();
	virtual bool create_embedded_gui(void* hwnd);
	virtual void resize_embedded_gui(void* hwnd, int* outwidth, int* outheight);
	void on_destroy_embedded_gui();

	void set_speaker_arrangement();
	VstIntPtr host_callback(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
	void set_parameter(int column, int value);
	void process_plugin_midi_events(VstEvents* events);
	VstMidiEvent* get_vst_midievent(unsigned int message, VstMidiEvent* midiev);
	void midi_out(int time, unsigned int data);
};
