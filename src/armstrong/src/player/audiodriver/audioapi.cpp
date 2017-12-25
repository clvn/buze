#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "audiodriver.h"
#if defined(_WIN32)
#include "asio/asio.h"
#include "asio/asiodriver.h"
#include <dsound.h>
#include "dsound/dsoundbuffer.h"
#include "dsound/dsounddriver.h"
#endif
#if !defined(HAVE_JACK) || HAVE_JACK == 1
#include <jack/jack.h>
#include "jack/jackdriver.h"
#endif
#include "audioapi.h"

using std::cerr;
using std::cout;
using std::cin;
using std::endl;

// abstractions for instances of individual api streams

struct deviceapi {
	virtual ~deviceapi() {}
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void destroy() = 0;
	virtual void configure() = 0;
	virtual int get_samplerate() = 0;
	virtual int get_buffersize() = 0;
	virtual int get_latency() = 0;
	virtual int get_outputs() = 0;
	virtual int get_inputs() = 0;
	virtual void* get_jack_handle() = 0;
	virtual void probe(deviceinfo* info) = 0;
};

#if defined(_WIN32)

struct asioapi : deviceapi {
	asio_device device;

	void start() {
		device.driver->start();
	}

	void stop() {
		device.driver->stop();
	}

	void destroy() {
		stop();
		asio_destroy_device(&device);
		delete this;
	}

	void configure() {
		device.driver->controlPanel();
	}

	int get_samplerate() {
		return device.samplerate;
	}

	int get_buffersize() {
		return device.buffersize;
	}

	int get_latency() {
		long inlatency, outlatency;
		ASIOError err = device.driver->getLatencies(&inlatency, &outlatency);
		if (err != ASE_OK) return 0;
		return inlatency + outlatency; // ??
	}

	virtual int get_outputs() {
		return device.output_count;
	}

	virtual int get_inputs() {
		return device.input_count;
	}

	virtual void probe(deviceinfo* info) {
		asio_get_deviceinfo(device.driver, info);
	}

	virtual void* get_jack_handle() { 
		return 0; 
	}
};


struct dsapi : deviceapi {
	ds_device device;

	void start() {
		ds_device_start(&device);
	}

	void stop() {
		ds_device_stop(&device);
	}

	void destroy() {
		stop();
		ds_destroy_device(&device);
		delete this;
	}

	void configure() {
	}

	int get_samplerate() {
		return device.outbuffer.format.nSamplesPerSec;
	}

	int get_buffersize() {
		return device.outbuffer.chunksize;
	}

	int get_latency() {
		return device.outbuffer.get_latency();
	}

	virtual int get_outputs() {
		return device.outbuffer.format.nChannels;
	}

	virtual int get_inputs() {
		return device.inbuffer.format.nChannels;
	}

	virtual void probe(deviceinfo* info) {}

	virtual void* get_jack_handle() { 
		return 0; 
	}
};

#endif

#if !defined(HAVE_JACK) || HAVE_JACK == 1

struct jackapi : deviceapi {
	jack_device device;

	void start() {
		jack_device_start(&device);
	}

	void stop() {
		jack_device_stop(&device);
	}

	void destroy() {
		stop();
		jack_destroy_device(&device);
		delete this;
	}

	void configure() {
		// lanunch qjackctl?
	}

	int get_samplerate() {
		return jack_get_sample_rate(device.client);
	}

	int get_buffersize() {
		return jack_get_buffer_size(device.client);
	}

	int get_latency() {
		if (device.output_ports.empty()) return 0;
		return jack_port_get_total_latency(device.client, device.output_ports[0].port);
	}

	virtual int get_outputs() {
		return (int)device.output_ports.size();
	}

	virtual int get_inputs() {
		return (int)device.input_ports.size();
	}

	virtual void probe(deviceinfo* info) {}

	virtual void* get_jack_handle() { 
		return device.client; 
	}
};
#endif

void audioapi_callback(callbacktype type, float** pout, float** pin, int numsamples, void* userdata) {
	audioapi* audio = (audioapi*)userdata;
	if (type == callbacktype_reset || type == callbacktype_samplerate || type == callbacktype_latency)
		audio->device->probe(audio->currentinfo);

	audio->usercallback(type, pout, pin, numsamples, audio->userdata);
}

audioapi::audioapi() {
	device = 0;
}

void audioapi::initialize() {
	devices.clear();
#if defined(_WIN32)
	ds_get_devices(&devices);
	asio_get_deviceinfos(&devices);
#endif
#if !defined(HAVE_JACK) || HAVE_JACK == 1
	jack_get_deviceinfo(&devices);
#endif
}

deviceinfo* audioapi::get_device(const std::string& name) {
	for (std::vector<deviceinfo>::iterator i = devices.begin(); i != devices.end(); ++i) {
		if (i->name == name) return &*i;
	}
	return 0;
}

bool audioapi::create_device(const std::string& outdevice, const std::string& indevice, int buffersize, int samplerate, audiodriver_callback_t callback, void* callbackdata) {
	deviceinfo* outinfo = get_device(outdevice);
	if (outinfo == 0) {
		cerr << "invalid outdevice" << endl;
		return false;
	}

	deviceinfo* ininfo = get_device(indevice);
	if (!indevice.empty() && !ininfo) {
		cerr << "audioapi: invalid indevice" << endl;
		return false;
	}

	if (ininfo != 0 && outinfo->api != ininfo->api) {
		cerr << "audioapi: input/output api mismatch" << endl;
		return false;
	}

	outinfo->get_default_deviceinfo(&buffersize, &samplerate);
#if defined(_WIN32)

	if (outinfo->api == api_asio) {
		if (ininfo != 0 && outinfo != ininfo) {
			cerr << "audioapi: asio input and output device must be the same" << endl;
			return false;
		}

		int input_count = (ininfo != 0) ? ininfo->numinputs : 0;
		asioapi* api = new asioapi();
		if (!asio_create_device(outdevice, 0, outinfo->numoutputs, 0, input_count, buffersize, samplerate, &audioapi_callback, this, &api->device)) {
			cerr << "audioapi: cannot create asio device" << endl;
			delete api;
			return false;
		}
		device = api;
	} else if (outinfo->api == api_ds) {
		dsapi* api = new dsapi();
		if (!ds_create_device(outdevice, indevice, 4, buffersize, samplerate, &audioapi_callback, this, &api->device)) {
			cerr << "audioapi: cannot create ds device" << endl;
			delete api;
			return false;
		}
		device = api;
	} else 
#endif
#if !defined(HAVE_JACK) || HAVE_JACK == 1
	if (outinfo->api == api_jack) {
		jackapi* api = new jackapi();
		if (!jack_create_device("AudioApiClient", &audioapi_callback, this, &api->device)) {
			cerr << "audioapi: cannot create jack device" << endl;
			delete api;
			return false;
		}
		device = api;
	}
#endif
	currentinfo = outinfo;
	usercallback = callback;
	userdata = callbackdata;
	return true;
}

void audioapi::destroy_device() {
	if (device) {
		device->destroy();
		device = 0;
	}
}

void audioapi::start() {
	if (device) device->start();
}

void audioapi::stop() {
	if (device) device->stop();
}

void audioapi::configure() {
	if (device) device->configure();
}

int audioapi::get_buffersize() {
	if (!device) return 1024;
	return device->get_buffersize();
}

int audioapi::get_samplerate() {
	if (!device) return 44100;
	return device->get_samplerate();
}

int audioapi::get_latency() {
	if (!device) return 0;
	return device->get_latency();
}

int audioapi::get_outputs() {
	if (!device) return 0;
	return device->get_outputs();
}

int audioapi::get_inputs() {
	if (!device) return 0;
	return device->get_inputs();
}

void* audioapi::get_jack_handle() {
	if (!device) return 0;
	return device->get_jack_handle();
}
