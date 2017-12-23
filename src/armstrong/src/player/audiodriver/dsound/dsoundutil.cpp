#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "dsound.h"
#include "dsoundutil.h"

using std::cerr;
using std::cout;
using std::cin;
using std::endl;

//
// enumeration context used by create_output_device(name) and create_input_device(name)
//

struct ds_create_context {
	std::string name;
	GUID clsid;
	bool found;

	ds_create_context() {
		found = false;
		memset(&clsid, 0, sizeof(GUID));
	}
};

static BOOL CALLBACK ds_enum_create_callback(LPGUID lpguid, LPCTSTR description, LPCTSTR module, LPVOID lpContext) {
	ds_create_context* context = (ds_create_context*)lpContext;
	if (context->name == description) {
		if (lpguid) context->clsid = *lpguid;
		context->found = true;
		return FALSE;
	}
	return TRUE;
}

//
// helpers
//

void ds_create_waveformat(int channels, int samplerate, int bitspersample, WAVEFORMATEX* waveformat) {
	ZeroMemory(waveformat, sizeof(WAVEFORMATEX));
	waveformat->cbSize = sizeof(WAVEFORMATEX);
	waveformat->wFormatTag = WAVE_FORMAT_PCM;
	waveformat->nChannels = channels;
	waveformat->nSamplesPerSec = samplerate;
	waveformat->wBitsPerSample = bitspersample;
    waveformat->nBlockAlign = waveformat->nChannels * waveformat->wBitsPerSample / 8;
    waveformat->nAvgBytesPerSec = waveformat->nSamplesPerSec * waveformat->nBlockAlign;
}

void ds_create_buffer_notifications(int count, int bytesmultiple, HANDLE event, std::vector<DSBPOSITIONNOTIFY>* positions) {

	for (int i = 0; i < count; i++) {
		DSBPOSITIONNOTIFY position;
		position.dwOffset = i * bytesmultiple;
		position.hEventNotify = event;
		positions->push_back(position);
	}

}

//
// output
//

bool ds_get_output_caps(LPDIRECTSOUND device, int* outputs, int* bitspersample, int* convertformat) {

    DSCAPS outCaps;
    outCaps.dwSize = sizeof(outCaps);
    if FAILED(device->GetCaps(&outCaps))
		return false;

	if ((outCaps.dwFlags & DSCAPS_PRIMARYSTEREO) != 0)
		*outputs = 2;
	else
		*outputs = 1;

	if ((outCaps.dwFlags & DSCAPS_PRIMARY16BIT) != 0) {
		*bitspersample = 16;
		*convertformat = 0;
	} else if ((outCaps.dwFlags & DSCAPS_PRIMARY8BIT) != 0) {
		*bitspersample = 8;
		*convertformat = 5;
	} else {
		*bitspersample = 16; // this happens with my ci2+ ds driver
		*convertformat = 0;
	}

	return true;
}

bool ds_set_output_device_format(LPDIRECTSOUND device, WAVEFORMATEX& waveFormat) {

    DSBUFFERDESC bufferDescription;
	memset(&bufferDescription, 0, sizeof(DSBUFFERDESC));
    bufferDescription.dwSize = sizeof(DSBUFFERDESC);
    bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

    LPDIRECTSOUNDBUFFER buffer = 0;
    HRESULT result = device->CreateSoundBuffer(&bufferDescription, &buffer, NULL);
	if FAILED(result) {
		cerr << "dsdriver: cannot get primary buffer" << endl;
		return false;
	}

	result = buffer->SetFormat(&waveFormat);
	if FAILED(result) {
		cerr << "dsdriver: cannot set primary buffer format" << endl;
		return false;
	}
	buffer->Release();
	return true;
}

LPDIRECTSOUNDBUFFER ds_create_output_device_buffer(LPDIRECTSOUND device, WAVEFORMATEX& waveFormat, int buffersize) {
    DSBUFFERDESC bufferDescription;
	ZeroMemory(&bufferDescription, sizeof(DSBUFFERDESC));
	bufferDescription.dwSize = sizeof(DSBUFFERDESC);
	//bufferDescription.dwFlags = DSBCAPS_STICKYFOCUS | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCHARDWARE;
	bufferDescription.dwBufferBytes = buffersize * waveFormat.nBlockAlign;
	bufferDescription.lpwfxFormat = &waveFormat;

	LPDIRECTSOUNDBUFFER buffer;
    //HRESULT result = device->CreateSoundBuffer(&bufferDescription, &buffer, NULL);
    //if FAILED(result) {
		// hw mixing failed, try sw mixing
		bufferDescription.dwFlags = /*DSBCAPS_STICKYFOCUS |*/ DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_LOCSOFTWARE;
		HRESULT result = device->CreateSoundBuffer(&bufferDescription, &buffer, NULL);
		if FAILED(result) {
			cerr << "dsdriver: cant create hw not sw secondary buffer" << endl;
			return 0;
		}
    //}
	return buffer;
}

LPDIRECTSOUND ds_create_output_device(const std::string& name) {
	ds_create_context outcontext;
	outcontext.name = name;

	HRESULT result = DirectSoundEnumerate(&ds_enum_create_callback, &outcontext);
	if (FAILED(result) || !outcontext.found) {
		cerr << "dsdriver: cannot find dsound" << endl;
		return 0;
	}

	LPDIRECTSOUND device = 0;
	if FAILED(DirectSoundCreate(&outcontext.clsid, &device, 0)) {
		cerr << "dsdriver: cannot create dsound" << endl;
		return 0;
	}

	return device;
}

//
// input
//

bool ds_get_input_channels(LPDIRECTSOUNDCAPTURE device, int* inputs) {

	DSCCAPS inCaps;
	inCaps.dwSize = sizeof(DSCCAPS);
	if FAILED(device->GetCaps(&inCaps))
		return false;

	// inCaps.dwFormats is not reliable for anything (0 on Steinberg CI2+)

	*inputs = inCaps.dwChannels;
	return true;
}

LPDIRECTSOUNDCAPTUREBUFFER ds_create_input_device_buffer(LPDIRECTSOUNDCAPTURE device, WAVEFORMATEX& waveFormat, int buffersize) {
    DSCBUFFERDESC bufferDescription;
    ZeroMemory(&bufferDescription, sizeof(DSCBUFFERDESC));
    bufferDescription.dwSize = sizeof(DSCBUFFERDESC);
    bufferDescription.dwFlags = DSCBCAPS_WAVEMAPPED;
    bufferDescription.dwReserved = 0;
    bufferDescription.dwBufferBytes = buffersize * waveFormat.nBlockAlign;
    bufferDescription.lpwfxFormat = &waveFormat;

    LPDIRECTSOUNDCAPTUREBUFFER buffer = 0;
	if FAILED(device->CreateCaptureBuffer(&bufferDescription, &buffer, NULL)) {
		cerr << "dsdriver: cannot create capture buffer" << endl;
		return 0;
	}

	return buffer;
}

LPDIRECTSOUNDCAPTURE ds_create_input_device(const std::string& indevice) {
	if (indevice.empty()) return 0;

	ds_create_context incontext;
	incontext.name = indevice;
	HRESULT result = DirectSoundCaptureEnumerate(&ds_enum_create_callback, &incontext);
	if (FAILED(result) || !incontext.found) {
		cerr << "dsdriver: cannot find dsoundcapture" << endl;
		return 0;
	}

	LPDIRECTSOUNDCAPTURE device = 0;
	if FAILED(DirectSoundCaptureCreate(&incontext.clsid, &device, 0)) {
		cerr << "dsdriver: cannot create dsoundcapture" << endl;
		return 0;
	}
	return device;
}

bool ds_buffer_lock(LPDIRECTSOUNDBUFFER buffer, DWORD offset, DWORD bytes, LPVOID* buffer1, DWORD* buffersize1, LPVOID* buffer2, DWORD* buffersize2) {

	HRESULT result = buffer->Lock(offset, bytes, buffer1, buffersize1, buffer2, buffersize2, 0);
	if (result == DSERR_BUFFERLOST) {
		result = buffer->Restore();
		if SUCCEEDED(result)
			result = buffer->Lock(offset, bytes, buffer1, buffersize1, buffer2, buffersize2, 0);
	}

	return SUCCEEDED(result);
}
