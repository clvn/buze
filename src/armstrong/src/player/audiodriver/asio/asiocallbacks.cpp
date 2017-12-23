#define NOMINMAX
#include <windows.h>
#include <vector>
#include <cassert>
#include <iostream>
#include "../audiodriver.h"
#include "asio.h"
#include "asiodriver.h"
#include "asiocallbacks.h"
#include "../convertsample.h"

using std::cerr;
using std::cout;
using std::cin;
using std::endl;

asio_callback_bufferswitch_t asio_callbacks::bufferswitch_callbacks[] = {
	&asio_callbacks::asio_callback_bufferswitch<0>, 
	&asio_callbacks::asio_callback_bufferswitch<1>, 
	&asio_callbacks::asio_callback_bufferswitch<2>, 
	&asio_callbacks::asio_callback_bufferswitch<3>, 
};

asio_callback_bufferswitch_timeinfo_t asio_callbacks::bufferswitch_timeinfo_callbacks[] = {
	&asio_callbacks::asio_callback_bufferswitch_timeinfo<0>, 
	&asio_callbacks::asio_callback_bufferswitch_timeinfo<1>, 
	&asio_callbacks::asio_callback_bufferswitch_timeinfo<2>, 
	&asio_callbacks::asio_callback_bufferswitch_timeinfo<3>, 
};

asio_callback_samplerate_t asio_callbacks::samplerate_callbacks[] = {
	&asio_callbacks::asio_callback_samplerate<0>, 
	&asio_callbacks::asio_callback_samplerate<1>, 
	&asio_callbacks::asio_callback_samplerate<2>, 
	&asio_callbacks::asio_callback_samplerate<3>, 
};

asio_callback_message_t asio_callbacks::message_callbacks[] = {
	&asio_callbacks::asio_callback_message<0>, 
	&asio_callbacks::asio_callback_message<1>, 
	&asio_callbacks::asio_callback_message<2>, 
	&asio_callbacks::asio_callback_message<3>, 
};

asio_device* asio_callbacks::devices[] = {0};

void asio_callback_device_bufferswitch(asio_device* device, long index, ASIOBool processNow) {
	assert(device->usercallback != 0);
	assert(!device->userbuffers.empty());

	if (device->resetflag) {
		device->usercallback(callbacktype_reset, 0, 0, 0, device->userdata);
		device->resetflag = false;
	}

	if (device->samplerateflag) {
		device->usercallback(callbacktype_samplerate, 0, 0, 0, device->userdata);
		device->samplerateflag = false;
	}

	if (device->latencyflag) {
		device->usercallback(callbacktype_latency, 0, 0, 0, device->userdata);
		device->latencyflag = false;
	}

	// TODO: skip memset if callback didnt write to buffer last frame
	for (int i = 0; i < device->output_count; i++) {
		memset(device->userbuffers[i], 0, device->buffersize * sizeof(float));
	}

	for (int i = 0; i < device->input_count; i++) {
		copy_samples(device->buffers[device->output_count + i].buffers[index], device->userbuffers[device->output_count + i], device->buffersize, device->deviceformat, 1, 1, 1, 0, 0, device->byteswap);
	}

	float** plout = &device->userbuffers[0];
	float** plin = device->input_count > 0 ? &device->userbuffers[device->output_count] : 0;

	device->usercallback(callbacktype_buffer, plout, plin, device->buffersize, device->userdata);

	// TODO: skip conversion if callback didnt write to buffer
	for (int i = 0; i < device->output_count; i++) {
		copy_samples(device->userbuffers[i], device->buffers[i].buffers[index], device->buffersize, 1, device->deviceformat, 1, 1, 0, 0, device->byteswap);
	}

	device->driver->outputReady();
}

long asio_callback_device_message(asio_device* device, long selector, long value, void* message, double* opt) {
	cerr << selector << " selector" << endl;
	switch (selector) {
		case kAsioSelectorSupported:
			switch (value) {
				case kAsioResetRequest:
				case kAsioEngineVersion:
				case kAsioResyncRequest:
				case kAsioLatenciesChanged:
				case kAsioBufferSizeChange:
					return 1;
			}
			return 0;
		case kAsioBufferSizeChange:
			cerr << "kAsioBufferSizeChange!" << endl;
			device->latencyflag = true;
			return 0;
		case kAsioResetRequest:
			SetEvent(device->resetevent);
			return 1;
		case kAsioResyncRequest:
			cerr << "kAsioResyncRequest!" << endl;
			// TODO: notify overrun
			return 1;
		case kAsioLatenciesChanged:
			cerr << "kAsioLatenciesChanged!" << endl;
			device->latencyflag = true;
			return 1;
		case kAsioEngineVersion:
			return 2;
		default:
			return 0;
	}
}
