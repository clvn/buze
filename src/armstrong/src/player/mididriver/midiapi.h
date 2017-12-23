#pragma once

enum midiapitype {
	midiapitype_portmidi = 0,
	midiapitype_jack = 1
};

struct midiapi {
	virtual ~midiapi() { }
	virtual midiapitype get_api() = 0;
	virtual bool initialize() = 0;
	virtual bool open_device(int index) = 0;
	virtual bool close_all_devices() = 0;
	virtual void close() = 0;
	virtual int get_device_count() = 0;
	virtual bool is_input(int index) = 0;
	virtual bool is_output(int index) = 0;
	virtual bool is_open(int index) = 0;
	virtual const char* get_device_name(int index) = 0;
	virtual void set_jack_handle(void* handle) = 0;
	virtual void set_latency(int sample_count, int samplespersec) = 0;

	virtual bool poll(zzub::midi_message* pmidi, int* midi_count, int maxcount, int sample_count, int samplespersec) = 0;
	virtual void schedule_send(zzub::midi_message* pmidi, int midi_count, int sample_count, int samplespersec) = 0;
};
