#define NOMINMAX
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>
#include <limits>
#include <algorithm>
#include "../audiodriver.h"
#include "dsound.h"
#include "dsoundutil.h"
#include "dsoundbuffer.h"
#include "../convertsample.h"

//
// ds_output_buffer
//

ds_output_buffer::ds_output_buffer() {
	buffer = 0;
	bufferevent = 0;
	playposition = 0;
	maxlatency = 2;
	memset(diff, 0, sizeof(diff));
	memset(&format, 0, sizeof(WAVEFORMATEX));
}

bool ds_output_buffer::create_output_buffer(LPDIRECTSOUND device, const WAVEFORMATEX& _format, int _chunksize) {
	using std::cerr;
	using std::endl;

	format = _format;
	chunksize = _chunksize;
	chunksizebytes = chunksize * format.nBlockAlign;

	// find a buffersize:
	// - must fit at last the number of samples given by the "dsound_min_buffersize" constant
	// - must fit at least the number of chunks given by the "dsound_min_chunks" constant
	// - must be a multiple of the chunksize 

	int bufferchunks = std::max(dsound_min_buffersize / chunksize, dsound_min_chunks);
	buffersize = bufferchunks * chunksize;
	buffersizebytes = buffersize * format.nBlockAlign;

	if (!ds_set_output_device_format(device, format)) {
		// OK if outdevice->SetCooperativeLevel(hWnd, DSSCL_NORMAL);
		// should succeed if DSSCL_PRIORITY which was the original value
		// cerr << "dsdriver: cannot set format" << endl;
		//return false;
	}

	buffer = ds_create_output_device_buffer(device, format, buffersize);
	if (buffer == 0) {
		cerr << "dsdriver: cant create hw not sw secondary buffer" << endl;
		return false;
	}

	DSBCAPS outCaps;
	outCaps.dwSize = sizeof(DSBCAPS);
	if SUCCEEDED(buffer->GetCaps(&outCaps)) {
		assert(outCaps.dwBufferBytes == buffersizebytes);
	}

	// set up notifications when the play position hits multiples of chunksizes
	LPDIRECTSOUNDNOTIFY notify;
	if FAILED(buffer->QueryInterface(IID_IDirectSoundNotify, (void**)&notify)) {
		cerr << "dsdriver: cant query the output buffer notify interface" << endl;
		buffer->Release();
		buffer = 0;
		return false;
	}

	int buffers = buffersize / chunksize;
	std::vector<DSBPOSITIONNOTIFY> positions;
	bufferevent = CreateEvent(0, FALSE, FALSE, 0);
	ds_create_buffer_notifications(buffers, chunksizebytes, bufferevent, &positions);

	notify->SetNotificationPositions((DWORD)positions.size(), &positions.front());
	notify->Release();

	userbuffers.resize(format.nChannels);
	for (int i = 0; i < (int)userbuffers.size(); i++) {
		userbuffers[i] = new float[chunksize];
	}

	return true;
}

void ds_output_buffer::destroy() {
	CloseHandle(bufferevent);

	for (int i = 0; i < (int)userbuffers.size(); i++) {
		userbuffers[i] = new float[chunksize];
	}
	userbuffers.clear();

	assert(buffer != 0);
	buffer->Release();
	buffer = 0;
}

// returns false if an overrun or underrun occured
bool ds_output_buffer::update_play_position() {
	int currentposition;
	int writeposition;
	buffer->GetCurrentPosition((DWORD*)&currentposition, (DWORD*)&writeposition);

	// shift the previous differences while summing the total
	int totaldiff = 0;
	for (int i = 0; i < num_diffs - 1; i++) {
		diff[num_diffs - 1 - i] = diff[num_diffs - 2 - i];
		totaldiff += diff[num_diffs - 1 - i];
	}

	// get signed difference between signaled play position and buffer play position
	diff[0] = currentposition - playposition;
	if (diff[0] > buffersizebytes / 2) diff[0] -= buffersizebytes;
	if (diff[0] < -buffersizebytes / 2) diff[0] += buffersizebytes;

	totaldiff += diff[0]; // total is now a sum of num_diffs + 1 diffs
	averagediff = totaldiff / (num_diffs + 1); // get the average difference

	// NOTE: keeping a record of the last last "dsound_min_chunks" latencies.
	// adjust the write pointer to the max seen latency, so we have a stabler 
	// latency even if it fluctuates locally (seen once where latency was rounded 
	// to 1 and 2 chunks but eventually settled on 1).
	// TODO: also notify the host about latency change!
	for (int i = 0; i < dsound_min_chunks - 1; i++) {
		playlatency[dsound_min_chunks - 1 - i] = playlatency[dsound_min_chunks - 2 - i];
	}
	playlatency[0] = writeposition - currentposition;
	if (playlatency[0] > buffersizebytes / 2) playlatency[0] -= buffersizebytes;
	if (playlatency[0] < -buffersizebytes / 2) playlatency[0] += buffersizebytes;

	maxlatency = 0;
	for (int i = 0; i < dsound_min_chunks - 1; i++) {
		maxlatency = std::max(playlatency[i], maxlatency);
	}
	maxlatency = std::max(playlatency[0], maxlatency) / chunksizebytes;
	maxlatency = maxlatency + 2; // +2 is the range of the overrun/underrun clamps which the latency is calculated from

	if (is_overrun()) {
		// play position is lagging, adjust the writepos one chunk ahead
		memset(diff, 0, sizeof(diff));
		skip_chunk();
		return false;
	} else if (is_underrun()) {
		// buffer play position is too far ahead, leave the write position
		std::cerr << "playback underrun" << std::endl;
		memset(diff, 0, sizeof(diff));
		return false;
	}
	
	//std::cerr << diff[0] << " and " << maxlatency << std::endl; // << "(" << (maxlatency / chunksizebytes) << ")" << std::endl;
	return true;
}

int ds_output_buffer::write_float(int deviceformat) {
	LPVOID buffer1 = NULL;
	LPVOID buffer2 = NULL;
	DWORD buffersize1 = 0;
	DWORD buffersize2 = 0;
	int outwritepos = get_write_position();
	//std::cerr << outwritepos << " and pos " << playposition;
	if (ds_buffer_lock(buffer, outwritepos, chunksizebytes, &buffer1, &buffersize1, &buffer2, &buffersize2)) {
		for (int i = 0; i < (int)format.nChannels; i++) {
			copy_samples(userbuffers[i], buffer1, chunksize, 1, deviceformat,  1, 2, 0, i, false);
		}
		buffer->Unlock(buffer1, buffersize1, buffer2, buffersize2);
	} else {
		std::cerr << "dsdriver: output locking error (serious)" << std::endl;
	}
	skip_chunk();

	return chunksize;
}


//
// ds_input_buffer
//

ds_input_buffer::ds_input_buffer() {
	buffer = 0;
	bufferevent = 0;
	captureposition = 0;
	readposition = 0;
	memset(&format, 0, sizeof(WAVEFORMATEX));
}

bool ds_input_buffer::create_input_buffer(LPDIRECTSOUNDCAPTURE device, WAVEFORMATEX _format, int _buffersize, int _chunksize) {

	using std::cerr;
	using std::endl;

	format = _format;

	int buffersize = _buffersize;
	buffersizebytes = buffersize * format.nBlockAlign;
	chunksize = _chunksize;
	chunksizebytes = chunksize * format.nBlockAlign;

	buffer = ds_create_input_device_buffer(device, format, buffersize);
	if (buffer == 0) {
		cerr << "dsdriver: cannot create capture buffer" << endl;
		return false;
	}

	DSCBCAPS inCaps;
	inCaps.dwSize = sizeof(DSCBCAPS);
	if SUCCEEDED(buffer->GetCaps(&inCaps)) {
		assert(inCaps.dwBufferBytes == buffersize * format.nBlockAlign);
	}

	// set up notifications when the record position hits multiples of chunksizes
	LPDIRECTSOUNDNOTIFY notify;
	if FAILED(buffer->QueryInterface(IID_IDirectSoundNotify, (void**)&notify)) {
		cerr << "dsdriver: cant query the output buffer notify interface" << endl;
		buffer->Release();
		buffer = 0;
		return false;
	}

	int buffers = buffersize / chunksize;
	std::vector<DSBPOSITIONNOTIFY> positions;
	bufferevent = CreateEvent(0, FALSE, FALSE, 0);
	ds_create_buffer_notifications(buffers, chunksize * format.nBlockAlign, bufferevent, &positions);

	notify->SetNotificationPositions((DWORD)positions.size(), &positions.front());
	notify->Release();

	userbuffers.resize(format.nChannels);
	for (int i = 0; i < (int)userbuffers.size(); i++) {
		userbuffers[i] = new float[chunksize];
	}

	return true;
}

void ds_input_buffer::destroy() {
	if (bufferevent != 0) {
		CloseHandle(bufferevent);
		bufferevent = 0;
	}

	for (int i = 0; i < (int)userbuffers.size(); i++) {
		userbuffers[i] = new float[chunksize];
	}
	userbuffers.clear();

	if (buffer != 0) {
		buffer->Release();
		buffer = 0;
	}
}

// returns number bytes available for capture
int ds_input_buffer::update_capture_position() {
	int capturedbytes = get_captured_bytes();
	int capturedchunks = capturedbytes / chunksizebytes;
	int capturedchunkbytes = capturedchunks * chunksizebytes;

	captureposition = (captureposition + capturedchunkbytes) % buffersizebytes;
	return capturedchunkbytes;
}

int ds_input_buffer::get_captured_bytes() {
	int currentposition;
	buffer->GetCurrentPosition(0, (DWORD*)&currentposition);

	if (captureposition <= currentposition)
		return currentposition - captureposition;
	else {
		return currentposition + buffersizebytes - captureposition;
	}
}

int ds_input_buffer::read_float(int deviceformat) {
	LPVOID buffer1 = NULL;
	LPVOID buffer2 = NULL;
	DWORD buffersize1 = 0;
	DWORD buffersize2 = 0;

	HRESULT result = buffer->Lock(readposition, chunksizebytes, &buffer1, &buffersize1, &buffer2, &buffersize2, 0);
	if SUCCEEDED(result) {
		for (int i = 0; i < (int)format.nChannels; i++) {
			copy_samples(buffer1, userbuffers[i], chunksize, deviceformat, 1,  2, 1, i, 0, false);
		}
		buffer->Unlock(buffer1, buffersize1, buffer2, buffersize2);
	} else {
		std::cerr << "dsdriver: input locking error" << std::endl;
	}

	skip_read_chunk();
	return chunksize;
	//cout << "inreadpos=" << inreadpos << ", inpos=" << captureposition << endl;
}
