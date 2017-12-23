#pragma once

typedef void (*asio_callback_bufferswitch_t)(long index, ASIOBool processNow);
typedef ASIOTime* (*asio_callback_bufferswitch_timeinfo_t)(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
typedef void (*asio_callback_samplerate_t)(ASIOSampleRate sRate);
typedef long (*asio_callback_message_t)(long selector, long value, void* message, double* opt);

void asio_callback_device_bufferswitch(asio_device* device, long index, ASIOBool processNow);
//void asio_callback_bufferswitch_timeinfo_t(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
long asio_callback_device_message(asio_device* device, long selector, long value, void* message, double* opt);

// helpers for asio driver callbacks with user data
struct asio_callbacks {
	enum { 
		// if you need more sessions, remember to update the callback arrays in the cpp too
		max_sessions = 4
	};

	static asio_device* devices[max_sessions];
	static asio_callback_bufferswitch_t bufferswitch_callbacks[max_sessions];
	static asio_callback_bufferswitch_timeinfo_t bufferswitch_timeinfo_callbacks[max_sessions];
	static asio_callback_samplerate_t samplerate_callbacks[max_sessions];
	static asio_callback_message_t message_callbacks[max_sessions];

	template <int SESSION>
	static void asio_callback_bufferswitch(long index, ASIOBool processNow) {
		assert(devices[SESSION] != 0);
		asio_callback_device_bufferswitch(devices[SESSION], index, processNow);
	}

	template <int SESSION>
	static ASIOTime* asio_callback_bufferswitch_timeinfo(ASIOTime* params, long index, ASIOBool processNow) {
		assert(devices[SESSION] != 0);
		// some drivers like to call bufferswitch_timeinfo even if its formally denied in kAsioSelectorSupported.
		// just forward to the regular user callback
		asio_callback_device_bufferswitch(devices[SESSION], index, processNow);
		return params;
	}

	template <int SESSION>
	static void asio_callback_samplerate(ASIOSampleRate sRate) {
		assert(devices[SESSION]->usercallback != 0);
		devices[SESSION]->usercallback(callbacktype_samplerate, 0, 0, 0, devices[SESSION]->userdata);
	}

	template <int SESSION>
	static long asio_callback_message(long selector, long value, void* message, double* opt) {
		assert(devices[SESSION] != 0);
		return asio_callback_device_message(devices[SESSION], selector, value, message, opt);
	}
};
