#pragma once

// minimal steinberg asio host routines (using ATL)

//  - handles kAsioResetRequest and changes done in the ASIO control panel transparently 
//  - notifies the host on external changes, buffersize, samplerate or latency changes
//  - supports multiple asio devices in the same process
//  - callbacks have user data parameter
//  - callbacks get only non-interleaved float buffers

// todo:
//  - optimizations for silent buffers
//  - audio devices using 64bit float formats
//  - test byteswap (need asio msb device/driver)
//  - does not handle external asio control panel (with kX drivers)

struct asio_device {
	int session;
	std::string name;
	IASIO* driver;
	int first_output, output_count;
	int first_input, input_count;
	int buffersize;
	int samplerate;
	ASIOBufferInfo* buffers;
	ASIOCallbacks* callbacks;
	audiodriver_callback_t usercallback;
	void* userdata;
	HANDLE quitevent;
	HANDLE resetevent;
	HANDLE controlthread;
	std::vector<float*> userbuffers;
	bool byteswap;
	int deviceformat;
	volatile bool resetflag;
	volatile bool samplerateflag;
	volatile bool latencyflag;
};

void asio_get_deviceinfos(std::vector<deviceinfo>* devices);
bool asio_get_deviceinfo(const std::string& name, deviceinfo* info);
bool asio_get_deviceinfo(IASIO* driver, deviceinfo* info);
bool asio_create_device(const std::string& name, int first_output, int output_count, int first_input, int input_count, int buffersize, int samplerate, audiodriver_callback_t callback, void* userdata, asio_device* device);
void asio_destroy_device(asio_device* device);
