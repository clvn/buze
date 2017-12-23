#pragma once

// common header for audio driver abstractions
// all audio drivers share device info and callback definition

enum {
	api_asio,
	api_ds,
	api_jack,
};

struct deviceinfo {
	int api;
	std::string name;
	int numinputs;
	int numoutputs;
	int defaultsamplerate;
	int defaultbuffersize;
	std::vector<int> buffersizes;
	std::vector<int> samplerates;

	void get_default_deviceinfo(int* buffersize, int *samplerate);
};

enum callbacktype {
	callbacktype_buffer,
	callbacktype_samplerate,
	callbacktype_latency,
	callbacktype_overrun,
	callbacktype_reset,
};

typedef void (*audiodriver_callback_t)(callbacktype type, float** pout, float** pin, int numsamples, void* userdata);
