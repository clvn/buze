#pragma once

// abstracts all audio drivers into a single interface

struct deviceapi;

struct audioapi {
	deviceapi* device;
	deviceinfo* currentinfo;
	audiodriver_callback_t usercallback;
	void* userdata;

	std::vector<deviceinfo> devices;
	audioapi();
	virtual ~audioapi() {}
	void initialize();
	deviceinfo* get_device(const std::string& name);
	bool create_device(const std::string& outdevice, const std::string& indevice, int buffersize, int samplerate, audiodriver_callback_t callback, void* userdata);
	void destroy_device();
	void start();
	void stop();
	void configure();
	int get_samplerate();
	int get_buffersize();
	int get_latency();
	int get_outputs();
	int get_inputs();
	void* get_jack_handle();
};
