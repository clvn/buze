#define NOMINMAX
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>
#include <limits>
#include "../audiodriver.h"
#include <dsound.h>
#include "dsoundutil.h"
#include "dsoundbuffer.h"
#include "dsounddriver.h"
#include "../convertsample.h"

#pragma comment(lib, "dsound.lib")

using std::cerr;
using std::cout;
using std::cin;
using std::endl;

// The ds playback buffer looks like this: (capture is conceptually similar)
//
//                   |<- Play position    [-----]<- Chunk size
// |-----|-----|-----|--------------------|-----|-----|-----|-----|
// |     |     |     | Latency            |Chunk|     |     |     |
// |-----|-----|-----|--------------------|-----|-----|-----|-----|
//                        Write position->|
// [--------------------------------------------------------------] <-|
//                                                       Buffer size -|
//
// The playback buffer is separated into chunk sizes aligned by the 
// user-defined chunk size. 
//
// Buffer size:
// - The buffer size is always a multiple of the chunk size
// - The buffer size is always >= 16k samples
// - The buffer size is always >= 4 chunks
//
// Latency:
// - The latency is derived from ds by the distance between the play position and write position
// - DS latency is now known after the stream is running
// - DS may not report a stable latency, ie distance between the play position and write position
// - The latency is the max of last N reported latencies (N = min_chunks)
//
// Notifications:
// - DS sends notifications when the play position hits the start of a chunk
// - The notifications could come too late under heavy load
// - The notifications are not very precise, worse for smaller chunk sizes
// - Notifications for capture buffers may or may not arrive
//
// Playback position:
// - Detect over/underruns when notifications consistently come too late
// - An overrun happens when the last N chunks are too late
// - N should allow recovering from temporary loads and play position fluctuations

template <typename T>
bool push_if_between(std::vector<T>& vec, const T& v, const T& r1, const T& r2) {
	if (v < r1 || v > r2)
		return false;
	
	vec.push_back(v);
	return true;
}

//
// create output device
//

bool ds_create_output_device(const std::string& name, int samplerate, int chunksize, ds_device* device) {
	assert(!name.empty());
	assert(samplerate != 0);
	assert(chunksize != 0);

	device->outdevice = ds_create_output_device(name);
	if (device->outdevice == 0) return false;

	int bitspersample = 0;
	int outputs = 0;
	if (!ds_get_output_caps(device->outdevice, &outputs, &bitspersample, &device->deviceformat)) {
		cerr << "dsdriver: cant get outdevice caps" << endl;
		device->outdevice->Release();
		device->outdevice = 0;
		return false;
	}

	HWND hWnd = GetDesktopWindow();
	device->outdevice->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
	//device->outdevice->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);

	WAVEFORMATEX outformat;
	ds_create_waveformat(outputs, samplerate, bitspersample, &outformat);

	if (!device->outbuffer.create_output_buffer(device->outdevice, outformat, chunksize)) {
		cerr << "dsdriver: cant create output buffer" << endl;
		device->outdevice->Release();
		device->outdevice = 0;
		return false;
	}

	return true;
}

//
// create input device
//

bool ds_create_input_device(const std::string& name, int samplerate, int chunksize, ds_device* device) {
	if (name.empty()) {
		device->indevice = 0;
		device->inbuffer.buffer = 0;
		return true;
	}

	// depend on these already being set
	assert(device->outdevice != 0);
	assert(device->outbuffer.buffer != 0);

	device->indevice = ds_create_input_device(name);
	if (device->indevice == 0) {
		cerr << "dsdriver: cannot create input device" << endl;
		return false;
	}

	int inputs = 0;
	if (!ds_get_input_channels(device->indevice, &inputs)) {
		cerr << "dsdriver: cant determine indevice channels" << endl;
		device->indevice->Release();
		device->indevice = 0;
		return false;
	}

	// create input format based on the output format
	WAVEFORMATEX format;
	ds_create_waveformat(inputs, samplerate, device->outbuffer.format.wBitsPerSample, &format);

	// using same device->buffersize as calculated for the outbuffer
	if (!device->inbuffer.create_input_buffer(device->indevice, format, device->outbuffer.buffersize, device->outbuffer.chunksize)) {
		cerr << "dsdriver: cannot create capture buffer" << endl;
		device->indevice->Release();
		device->indevice = 0;
		return false;
	}

	return true;
}

//
// create device
//

bool ds_create_device(const std::string& outdevice, const std::string& indevice, int buffers, int chunksize, int samplerate, audiodriver_callback_t callback, void* userdata, ds_device* device) {

	if (!ds_create_output_device(outdevice, samplerate, chunksize, device)) {
		cerr << "dsdriver: cannot create outdevice" << endl;
		return false;
	}

	if (!ds_create_input_device(indevice, samplerate, chunksize, device)) {
		cerr << "dsdriver: cannot create indevice" << endl;
		device->outbuffer.buffer->Release();
		device->outbuffer.buffer = 0;
		device->outdevice->Release();
		device->outdevice = 0;
		return false;
	}

	device->quit = false;
	device->usercallback = callback;
	device->userdata = userdata;
	device->audiothread = 0;
	device->startevent = 0;

	return true;
}

void ds_destroy_device(ds_device* device) {

	ds_device_stop(device);

	device->inbuffer.destroy();
	device->outbuffer.destroy();

	if (device->indevice != 0) {
		device->indevice->Release();
		device->indevice = 0;
	}

	assert(device->outdevice != 0);
	device->outdevice->Release();
	device->outdevice = 0;
}

//
// device processing
//

DWORD WINAPI ds_audio_thread_proc(LPVOID lpParam) {

	ds_device* device = (ds_device*)lpParam;
	ds_output_buffer& outbuffer = device->outbuffer;
	ds_input_buffer& inbuffer = device->inbuffer;

	std::vector<HANDLE> bufferevents;
	bufferevents.push_back(outbuffer.bufferevent);
	if (device->inbuffer.bufferevent != 0) {
		bufferevents.push_back(inbuffer.bufferevent);
	}

	if (inbuffer.buffer) inbuffer.buffer->Start(DSCBSTART_LOOPING);
	if (outbuffer.buffer) outbuffer.buffer->Play(0, 0, DSBPLAY_LOOPING);
	SetEvent(device->startevent);

	int inavail = 0;
	int outavail = 0;
	int outlatency = 0;

	for (;!device->quit;) {

		// wait for the play position to hit a multiple of the chunksize in either the playback or capture buffer
		// WAIT_OBJECT_0 + 0 = play position notification
		// WAIT_OBJECT_0 + 1 = capture position notification

		DWORD waitresult;

		while (true) {
			waitresult = WaitForMultipleObjects((DWORD)bufferevents.size(), &bufferevents.front(), FALSE, 500);
			if (waitresult == WAIT_TIMEOUT) {
				// happens when unplugging an usb soundcard. checking the status avoids deadlock
				DWORD status;
				outbuffer.buffer->GetStatus(&status);
				if (inbuffer.buffer)
					inbuffer.buffer->GetStatus(&status);
			} else
				break;
		}

		if (waitresult == WAIT_OBJECT_0 + 0) {
			if (!outbuffer.update_play_position()) {
				if (outbuffer.is_overrun()) {
					if (inavail) inavail -= inbuffer.skip_read_chunk();
					//cerr << "playback overrun, inavail=" << inavail << endl;

				} else if (outbuffer.is_underrun()) {
					continue; // dont do anything more on underruns
				}
			}

			if (outavail > 0) outavail -= outbuffer.write_float(device->deviceformat);

			if (outlatency != outbuffer.get_latency()) {
				outlatency = outbuffer.get_latency();
				device->usercallback(callbacktype_latency, 0, 0, 0, device->userdata);
			}

			// generate sample data for next chunk
			if (outavail == 0) {
				if (inavail > 0) inavail -= inbuffer.read_float(device->deviceformat);
				float** inbuffers = (!inbuffer.userbuffers.empty()) ? &inbuffer.userbuffers.front() : 0;
				device->usercallback(callbacktype_buffer, &outbuffer.userbuffers.front(), inbuffers, outbuffer.chunksize, device->userdata);
				outavail += outbuffer.chunksize;
			}

		} else if (waitresult == WAIT_OBJECT_0 + 1) {
			// assume dsound triggers ALL notifications in the playback buffers
			// assume dsound triggers SOME notifications on the capture buffer
			// ie, we may get more than asked for here
			int capturedbytes = inbuffer.update_capture_position();
			inavail += capturedbytes;
			continue;
		} else {
			// not input buffer, not output buffer
			assert(false);
		}
	}

	if (inbuffer.buffer) inbuffer.buffer->Stop();
	outbuffer.buffer->Stop();
	return 0;
}

void ds_device_start(ds_device* device) {
	if (device->audiothread) return ;

	device->quit = false;
	device->startevent = CreateEvent(0, FALSE, FALSE, 0);
	DWORD dwid;
	device->audiothread = CreateThread(0, 0, &ds_audio_thread_proc, device, 0, &dwid);

	// Boost DS thread priority
	SetThreadPriority(device->audiothread, THREAD_PRIORITY_HIGHEST);

	// wait until buffer is filled and starts playing
	WaitForSingleObject(device->startevent, INFINITE);
}

void ds_device_stop(ds_device* device) {
	if (!device->audiothread) return ;

	device->quit = true;
	WaitForSingleObject(device->audiothread, INFINITE);
	CloseHandle(device->audiothread);
	device->audiothread = 0;

	CloseHandle(device->startevent);
	device->startevent = 0;

}

//
// enumeration probe
//

static BOOL CALLBACK ds_enum_output_callback(LPGUID lpguid, LPCTSTR description, LPCTSTR module, LPVOID lpContext) {

	std::vector<deviceinfo>* devices = (std::vector<deviceinfo>*)lpContext;

	deviceinfo dsdevice;
	dsdevice.name = description;
	dsdevice.api = api_ds;
	dsdevice.numinputs = 0;

	dsdevice.buffersizes.push_back(128);
	dsdevice.buffersizes.push_back(256);
	dsdevice.buffersizes.push_back(512);
	dsdevice.buffersizes.push_back(768);
	dsdevice.buffersizes.push_back(1024);
	dsdevice.buffersizes.push_back(1536);
	dsdevice.buffersizes.push_back(2048);
	dsdevice.buffersizes.push_back(3072);
	dsdevice.buffersizes.push_back(4096);
	dsdevice.defaultbuffersize = 1024;

	LPDIRECTSOUND output = 0;
	if FAILED(DirectSoundCreate(lpguid, &output, 0))
		return TRUE;

	DSCAPS caps;
	caps.dwSize = sizeof(caps);
	if SUCCEEDED(output->GetCaps(&caps)) {
		if (caps.dwFlags & DSCAPS_PRIMARYSTEREO)
			dsdevice.numoutputs = 2;
		else if (caps.dwFlags & DSCAPS_PRIMARYMONO)
			dsdevice.numoutputs = 1;
		else {
			output->Release();
			output = 0;
			return TRUE;
		}

		int minrate = caps.dwMinSecondarySampleRate;
		int maxrate = caps.dwMaxSecondarySampleRate;
		push_if_between(dsdevice.samplerates, 11025, minrate, maxrate);
		push_if_between(dsdevice.samplerates, 22050, minrate, maxrate);
		push_if_between(dsdevice.samplerates, 44100, minrate, maxrate);
		push_if_between(dsdevice.samplerates, 48000, minrate, maxrate);
		push_if_between(dsdevice.samplerates, 96000, minrate, maxrate);

		int defaultrate = 44100;
		if (defaultrate >= minrate && defaultrate <= maxrate)
			dsdevice.defaultsamplerate = defaultrate;
		else if (defaultrate > maxrate)
			dsdevice.defaultsamplerate = maxrate;
		else
			dsdevice.defaultsamplerate = minrate;

		output->Release();
	} else {
		output->Release();
		output = 0;
		return TRUE;
	}

	devices->push_back(dsdevice);
	return TRUE;
}

static BOOL CALLBACK ds_enum_input_callback(LPGUID lpguid, LPCTSTR description, LPCTSTR module, LPVOID lpContext) {

	std::vector<deviceinfo>* devices = (std::vector<deviceinfo>*)lpContext;

	deviceinfo dsdevice;
	dsdevice.name = description;
	dsdevice.api = api_ds;
	dsdevice.numoutputs = 0;
	dsdevice.defaultbuffersize = 0;
	dsdevice.defaultsamplerate = 0;

	LPDIRECTSOUNDCAPTURE input = 0;
	if FAILED(DirectSoundCaptureCreate(lpguid, &input, 0))
		return TRUE;

	DSCCAPS ccaps;
	ccaps.dwSize = sizeof(ccaps);
	if SUCCEEDED(input->GetCaps(&ccaps)) {
		dsdevice.numinputs = ccaps.dwChannels;
		input->Release();
	} else {
		input->Release();
		return TRUE;
	}
	devices->push_back(dsdevice);
	return TRUE;
}


void ds_get_devices(std::vector<deviceinfo>* devices) {
	HRESULT result = DirectSoundEnumerate(&ds_enum_output_callback, devices);
	result = DirectSoundCaptureEnumerate(&ds_enum_input_callback, devices);
}
