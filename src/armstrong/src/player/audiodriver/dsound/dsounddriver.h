#pragma once

struct ds_device {
	LPDIRECTSOUND outdevice;
	ds_output_buffer outbuffer;
	LPDIRECTSOUNDCAPTURE indevice;
	ds_input_buffer inbuffer;

	int deviceformat;
	audiodriver_callback_t usercallback;
	void* userdata;
	volatile bool quit;
	HANDLE audiothread;
	HANDLE startevent;
};

void ds_get_devices(std::vector<deviceinfo>* devices);
bool ds_create_device(const std::string& outdevice, const std::string& indevice, int buffers, int chunksize, int samplerate, audiodriver_callback_t callback, void* userdata, ds_device* device);
void ds_destroy_device(ds_device* device);
void ds_device_start(ds_device* device);
void ds_device_stop(ds_device* device);
