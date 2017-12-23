#define NOMINMAX
#include <atlbase.h>
#include <atlcom.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include "../audiodriver.h"
#include "asio.h"
#include "asiodriver.h"
#include "asiocallbacks.h"
#include "../convertsample.h"

using std::cerr;
using std::cin;
using std::endl;

const unsigned int SAMPLE_RATES[] = {
  4000, 5512, 8000, 9600, 11025, 16000, 22050,
  32000, 44100, 48000, 88200, 96000, 176400, 192000
};

const unsigned int MAX_SAMPLE_RATES = sizeof(SAMPLE_RATES) / sizeof(unsigned int);

struct noasio : IASIO {
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) {
		*ppvObject = this;
		return S_OK;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef(void) {
		return 2;
	}
	virtual ULONG STDMETHODCALLTYPE Release(void) {
		return 1;
	}
	virtual ASIOBool init(HWND sysHandle) {
		return ASIOTrue;
	}
	virtual void getDriverName(char *name) {
		strcpy(name, "error");
	}
	virtual long getDriverVersion() {
		return 1;
	}
	virtual void getErrorMessage(char *string) {
		strcpy(string, "");
	}
	virtual ASIOError start() {
		return ASE_OK;
	}
	virtual ASIOError stop() {
		return ASE_OK;
	}
	virtual ASIOError getChannels(long *numInputChannels, long *numOutputChannels) {
		*numInputChannels = 0;
		*numOutputChannels = 0;
		return ASE_OK;
	}
	virtual ASIOError getLatencies(long *inputLatency, long *outputLatency) {
		*inputLatency = 0;
		*outputLatency = 0;
		return ASE_OK;
	}
	virtual ASIOError getBufferSize(long *minSize, long *maxSize, long *preferredSize, long *granularity) {
		*minSize = 0;
		*maxSize = 0;
		*preferredSize = 0;
		return ASE_OK;
	}
	virtual ASIOError canSampleRate(ASIOSampleRate sampleRate) {
		return ASE_OK;
	}
	virtual ASIOError getSampleRate(ASIOSampleRate *sampleRate) {
		*sampleRate = 0;
		return ASE_OK;
	}
	virtual ASIOError setSampleRate(ASIOSampleRate sampleRate) {
		return ASE_OK;
	}
	virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources) {
		*numSources = 0;
		return ASE_OK;
	}
	virtual ASIOError setClockSource(long reference) {
		return ASE_OK;
	}
	virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) {
		*sPos = 0;
		*tStamp = 0;
		return ASE_OK;
	}
	virtual ASIOError getChannelInfo(ASIOChannelInfo *info) {
		return ASE_OK;
	}
	virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels, long bufferSize, ASIOCallbacks *callbacks) {
		return ASE_OK;
	}
	virtual ASIOError disposeBuffers() {
		return ASE_OK;
	}
	virtual ASIOError controlPanel() {
		return ASE_OK;
	}
	virtual ASIOError future(long selector,void *opt) {
		return ASE_OK;
	}
	virtual ASIOError outputReady() {
		return ASE_OK;
	}
};

noasio noasioimpl;

//
// sample type utils
//

// converts from asio sample types to internal "convertsample.h" types
bool asio_get_convert_format(ASIOSampleType type, int* deviceformat, bool* byteswap) {
	switch (type) {
		case ASIOSTInt16LSB:
			*deviceformat = 0;
			*byteswap = false;
			return true;
		case ASIOSTInt24LSB:
			*deviceformat = 3;
			*byteswap = false;
			return true;
		case ASIOSTInt32LSB:
			*deviceformat = 2;
			*byteswap = false;
			return true;
		case ASIOSTFloat32LSB:
			*deviceformat = 1;
			*byteswap = false;
			return true;
		/*case ASIOSTFloat64LSB:
			*deviceformat = 1;
			*byteswap = false;
			return true;*/

		case ASIOSTInt16MSB:
			*deviceformat = 0;
			*byteswap = true;
			return true;
		case ASIOSTInt24MSB:
			*deviceformat = 3;
			*byteswap = true;
			return true;
		case ASIOSTInt32MSB:
			*deviceformat = 2;
			*byteswap = true;
			return true;
		case ASIOSTFloat32MSB:
			*deviceformat = 1;
			*byteswap = true;
			return true;
		/*case ASIOSTFloat64MSB:
			*deviceformat = 1;
			*byteswap = true;
			return true;*/

		default:
			return false;
	}
}

std::string asio_get_sample_name(ASIOSampleType type) {
	switch (type) {
		case ASIOSTInt16MSB: return "ASIOSTInt16MSB";
		case ASIOSTInt24MSB: return "ASIOSTInt24MSB";
		case ASIOSTInt32MSB: return "ASIOSTInt32MSB";
		case ASIOSTFloat32MSB: return "ASIOSTFloat32MSB";
		case ASIOSTFloat64MSB: return "ASIOSTFloat64MSB";
		case ASIOSTInt32MSB16: return "ASIOSTInt32MSB16";
		case ASIOSTInt32MSB18: return "ASIOSTInt32MSB18";
		case ASIOSTInt32MSB20: return "ASIOSTInt32MSB20";
		case ASIOSTInt32MSB24: return "ASIOSTInt32MSB24";

		case ASIOSTInt16LSB: return "ASIOSTInt16LSB";
		case ASIOSTInt24LSB: return "ASIOSTInt24LSB";
		case ASIOSTInt32LSB: return "ASIOSTInt32LSB";
		case ASIOSTFloat32LSB: return "ASIOSTFloat32LSB";
		case ASIOSTFloat64LSB: return "ASIOSTFloat64LSB";
		case ASIOSTInt32LSB16: return "ASIOSTInt32LSB16";
		case ASIOSTInt32LSB18: return "ASIOSTInt32LSB18";
		case ASIOSTInt32LSB20: return "ASIOSTInt32LSB20";
		case ASIOSTInt32LSB24: return "ASIOSTInt32LSB24";
		default: return "";
	}
}

//
// control thread
//

void asio_reset_device(asio_device* device);

DWORD WINAPI asio_control_thread_proc(LPVOID lpParam) {

	// TODO: another possible problem, when using launching the asio control panel 
	// externally, f.ex when launching from the start menu or system tray, 
	// the audio thread will stop without any notification (kx asio driver)
	// this could be monitored here

	CoInitialize(0);

	asio_device* device = (asio_device*)lpParam;

	HANDLE handles[] = { device->quitevent, device->resetevent };
	int numhandles = sizeof(handles) / sizeof(HANDLE);
	for (;;) {
		DWORD wait = WaitForMultipleObjects(numhandles, handles, FALSE, INFINITE);
		if (wait == WAIT_OBJECT_0 + 0) {
			break;
		} else if (wait == WAIT_OBJECT_0 + 1) {
			// TODO: critical section
			asio_reset_device(device);
		}
	}

	CoUninitialize();
	return 0;
}

// 
// create / initialize device instance
// 

bool asio_get_device_registry(const std::string& name, TCHAR* friendlyName, CLSID* clsid) {
	std::stringstream keyStrm;
	keyStrm << "SOFTWARE\\ASIO\\" << name;

	CRegKey pluginClassKey;
	if (ERROR_SUCCESS != pluginClassKey.Open(HKEY_LOCAL_MACHINE, keyStrm.str().c_str(), KEY_READ)) {
		cerr << "asiodriver: cant open HKLM\\SOFTWARE\\ASIO\\" << keyStrm.str() << endl;
		return false;
	}

	DWORD keySize = 1024;
	if (ERROR_SUCCESS != pluginClassKey.QueryStringValue("CLSID", friendlyName, &keySize)) {
		cerr << "asiodriver: cant query driver classid" << endl;
		pluginClassKey.Close();
		return false;
	}
	pluginClassKey.Close();

	if FAILED(CLSIDFromString(CComBSTR(friendlyName), clsid)) {
		cerr << "asiodriver: bad driver clsid" << endl;
		return false;
	}

	return true;
}

IASIO* asio_create_driver(const std::string& name) {
	TCHAR friendlyName[1024];
	CLSID clsid;
	if (!asio_get_device_registry(name, friendlyName, &clsid))
		return 0;

	IASIO* driver = 0;
	if FAILED(CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, clsid, (void**)&driver)) {
		cerr << "asiodriver: cant cocreate driver instance" << endl;
		return 0;
	}

	HWND hWnd = GetDesktopWindow();

	if (!driver->init(hWnd)) {
		char errormessage[1024];
		driver->getErrorMessage(errormessage);
		cerr << "asiodriver: cant init driver: " << errormessage << endl;
		driver->Release();
		return 0;
	}

	char drivername[256];
	driver->getDriverName(drivername);
	long version = driver->getDriverVersion();

	return driver;
}

bool asio_create_device_buffers(asio_device* device, int first_output, int output_count, int first_input, int input_count, int buffersize) {

	device->buffers = new ASIOBufferInfo[output_count + input_count];
	if (device->buffers == 0) return false;

	device->buffersize = buffersize;
	device->userbuffers.resize(output_count + input_count);

	for (int i = 0; i < output_count; i++) {
		device->buffers[i].isInput = ASIOFalse;
		device->buffers[i].channelNum = first_output + i;
		device->buffers[i].buffers[0] = 0;
		device->buffers[i].buffers[1] = 0;

		device->userbuffers[i] = new float[buffersize];
	}

	for (int i = 0; i < input_count; i++) {
		device->buffers[output_count + i].isInput = ASIOTrue;
		device->buffers[output_count + i].channelNum = first_input + i;
		device->buffers[output_count + i].buffers[0] = 0;
		device->buffers[output_count + i].buffers[1] = 0;

		device->userbuffers[output_count + i] = new float[buffersize];
	}

	ASIOError err = device->driver->createBuffers(device->buffers, output_count + input_count, buffersize, device->callbacks);
	if (err != ASE_OK) {
		cerr << "asiodriver: cant create buffers" << endl;
		delete[] device->buffers;
		device->buffers = 0;
		return false;
	}

	return true;
}

void asio_destroy_device_buffers(asio_device* device) {
	device->driver->disposeBuffers();
	delete[] device->buffers;
	device->buffers = 0;

	for (int i = 0; i < device->output_count; i++)
		delete[] device->userbuffers[i];

	device->userbuffers.clear();
}

bool asio_create_device_callbacks(asio_device* device, audiodriver_callback_t callback, void* userdata) {

	// asio does not support user data parameters in its callbacks directly.
	// this is handled by the asio_callbacks class

	for (int i = 0; i < asio_callbacks::max_sessions; i++) {
		if (asio_callbacks::devices[i] == 0) {
			asio_callbacks::devices[i] = device;
			device->session = i;

			device->usercallback = callback;
			device->userdata = userdata;
			device->callbacks = new ASIOCallbacks();
			device->callbacks->bufferSwitch = asio_callbacks::bufferswitch_callbacks[device->session];
			device->callbacks->sampleRateDidChange = asio_callbacks::samplerate_callbacks[device->session];
			device->callbacks->asioMessage = asio_callbacks::message_callbacks[device->session];
			device->callbacks->bufferSwitchTimeInfo = asio_callbacks::bufferswitch_timeinfo_callbacks[device->session];
			return true;
		}
	}
	return false;
}

void asio_destroy_device_callbacks(asio_device* device) {
	// TODO: device->session
	for (int i = 0; i < asio_callbacks::max_sessions; i++) {
		if (asio_callbacks::devices[i] == device) {
			delete device->callbacks;
			device->callbacks = 0;
			device->session = -1;
			asio_callbacks::devices[i] = 0;
		}
	}
}

// asio_create_device() fails if any of the parameters are invalid. 
// use asio_get_default_deviceinfo() to sanitize parameters before.
bool asio_create_device(const std::string& name, int first_output, int output_count, int first_input, int input_count, int buffersize, int samplerate, audiodriver_callback_t callback, void* userdata, asio_device* device) {

	device->name = name;
	device->driver = asio_create_driver(name);
	if (!device->driver) return false;

	device->first_output = first_output;
	device->output_count = output_count;
	device->first_input = first_input;
	device->input_count = input_count;

	// obtain sample format - assume always the same input/output format in all channels
	ASIOChannelInfo channelInfo;
	channelInfo.channel = 0;
	channelInfo.isInput = ASIOFalse;
	ASIOError err = device->driver->getChannelInfo(&channelInfo);
	if (err != ASE_OK) {
		cerr << "asiodriver: unable get channel info" << endl;
		device->driver->Release();
		return false;
	}
	if (!asio_get_convert_format(channelInfo.type, &device->deviceformat, &device->byteswap)) {
		cerr << "asiodriver: Sample format not supported: " << asio_get_sample_name(channelInfo.type) << endl;
		device->driver->Release();
		return false;
	} 
	
	{ // TODO: if verbose...
		cerr << "asiodriver: using format: " << asio_get_sample_name(channelInfo.type) << endl;
	}

	// set sample rate
	ASIOSampleRate currentrate;
	err = device->driver->getSampleRate(&currentrate);
	if (err != ASE_OK) {
		cerr << "asiodriver: unable to get samplerate" << endl;
		device->driver->Release();
		return false;
	}

	if (currentrate != samplerate) {
		err = device->driver->setSampleRate(samplerate);
		if (err != ASE_OK) {
			cerr << "asiodriver: unable to set samplerate" << endl;
			device->driver->Release();
			return false;
		}
	}
	device->samplerate = samplerate;

	{ // TODO: if verbose...
		cerr << "asiodriver: using samplerate: " << samplerate << endl;
		cerr << "asiodriver: using buffersize: " << buffersize << endl;
		cerr << "asiodriver: using output_count: " << output_count << endl;
		cerr << "asiodriver: using input_count: " << input_count << endl;
	}

	if (!asio_create_device_callbacks(device, callback, userdata)) {
		cerr << "asiodriver: unable to create callbacks" << endl;
		device->driver->Release();
		return false;
	}

	if (!asio_create_device_buffers(device, first_output, output_count, first_input, input_count, buffersize)) {
		cerr << "asiodriver: unable to create buffers" << endl;
		// TODO: revert samplerate?
		asio_destroy_device_callbacks(device);
		device->driver->Release();
		return false;
	}

	// create control thread and handles to communicate with it
	device->quitevent = CreateEvent(0, FALSE, FALSE, 0);
	device->resetevent = CreateEvent(0, FALSE, FALSE, 0);
	device->resetflag = false;
	device->samplerateflag = false;
	device->latencyflag = false;
	DWORD dw;
	device->controlthread = CreateThread(0, 0, asio_control_thread_proc, device, 0, &dw);

	return true;
}

void asio_destroy_device(asio_device* device) {

	SetEvent(device->quitevent);
	WaitForSingleObject(device->controlthread, INFINITE);

	CloseHandle(device->quitevent);
	CloseHandle(device->resetevent);
	CloseHandle(device->controlthread);
	device->quitevent = 0;
	device->resetevent = 0;
	device->controlthread = 0;

	asio_destroy_device_callbacks(device);
	asio_destroy_device_buffers(device);

	device->driver->Release();
	device->driver = 0;
}

void asio_reset_device(asio_device* device) {

	device->driver->stop();

	asio_destroy_device_buffers(device);

	device->driver->Release();
	device->driver = asio_create_driver(device->name);

	if (device->driver == 0) {
		// happens f.ex when unplugging an usb sound card
		MessageBox(GetForegroundWindow(), _T("Audio driver was reset, please check the audio settings."), _T("Notification"), MB_OK);

		// assign a dummy asio implementation as the driver, to avoid littering audioapi.cpp with "if(device->driver)"
		device->driver = &noasioimpl;
		device->first_output = 0;
		device->output_count = 0;
		device->first_input = 0;
		device->input_count = 0;

		// it could create a dummy audio thread to send the reset event, but since there is no audio thread
		// just call the reset callback directly
		device->usercallback(callbacktype_reset, 0, 0, 0, device->userdata);
		return;
	}

	ASIOBool ret = device->driver->init(0);
	assert(ret != 0);

	ASIOSampleRate samplerate;
	device->driver->getSampleRate(&samplerate);
	if (device->samplerate != (int)samplerate) {
		device->samplerate = (int)samplerate;
		device->samplerateflag = true;
	}

	long minsize;
	long maxsize;
	long prefersize;
	long granularity;
	ASIOError err = device->driver->getBufferSize(&minsize, &maxsize, &prefersize, &granularity);
	assert(err == ASE_OK);

	if (device->buffersize != prefersize) {
		device->buffersize = prefersize;
		device->latencyflag = true;
	}

	cerr << "asiodriver: reset, buffer size " << device->buffersize << ", samplerate " << device->samplerate << endl;

	ret = asio_create_device_buffers(device, device->first_output, device->output_count, device->first_input, device->input_count, device->buffersize);
	assert(ret);

	// the resetflag is checked in the audiothread
	device->resetflag = true;

	device->driver->start();
}

//
// enumerate/probe deviceinfo
//

bool asio_get_deviceinfo(IASIO* driver, deviceinfo* info) {

	info->api = api_asio;

	long numinputs;
	long numoutputs;
	ASIOError err = driver->getChannels(&numinputs, &numoutputs);
	if (err != ASE_OK) {
		cerr << "asiodriver: unable to query channels" << endl;
		return false;
	}

	info->numinputs = numinputs;
	info->numoutputs = numoutputs;		

	long minsize;
	long maxsize;
	long prefersize;
	long granularity;
	err = driver->getBufferSize(&minsize, &maxsize, &prefersize, &granularity);
	if (err != ASE_OK) {
		cerr << "asiodriver: unable to query buffersize" << endl;
		return false;
	}

	info->defaultbuffersize = prefersize;
	info->buffersizes.clear();
	if (minsize == maxsize) {
		info->buffersizes.push_back(minsize);
	} else if (minsize < maxsize && granularity == -1) {
		// sizes are powers of 2
		int rate = minsize;
		while (rate <= maxsize) {
			info->buffersizes.push_back(rate);
			rate *= 2;
		}
	} else if (minsize < maxsize && granularity > 0) {
		// suzes are increments of granularity
		int rate = minsize;
		while (rate <= maxsize) {
			info->buffersizes.push_back(rate);
			rate += granularity;
		}
	} else {
		cerr << "asiodriver: driver reports invalid buffer size" << endl;
		return false;
	}

	info->samplerates.clear();
	for (int i = 0; i < MAX_SAMPLE_RATES; i++) {
		err = driver->canSampleRate((ASIOSampleRate)SAMPLE_RATES[i]);
		if (err == ASE_OK)
			info->samplerates.push_back(SAMPLE_RATES[i]);
	}
	
	if (info->samplerates.empty()) {
		cerr << "asiodriver: unable to confirm any samplerates" << endl;
		return false;
	}

	ASIOSampleRate samplerate;
	err = driver->getSampleRate(&samplerate);
	if (err != ASE_OK) {
		cerr << "asiodriver: unable to query default samplerate" << endl;
		return false;
	}

	info->defaultsamplerate = (int)samplerate;
	return true;
}

bool asio_get_deviceinfo(const std::string& name, deviceinfo* info) {

	// NOTE: cannot create deviceinfo for running devices (steinberg ci2+), but 
	// we can check for known instances in asiocallbacks, but this couples unrelated things
	// NOTE: assume asio_get_deviceinfo() is called before any devices are created
/*
	// first try if the driver is already running
	for (int i = 0; i < asio_callbacks::max_sessions; i++) {
		if (asio_callbacks::devices[i] != 0) {
			if (asio_callbacks::devices[i]->name == name) {
				// it runs! fill info and return
				IASIO* driver = asio_callbacks::devices[i]->driver;
				info->name = name;
				return asio_get_deviceinfo(driver, info);
			}
		}
	}
*/
	// try load the driver, fill info, and release before returning
	IASIO* driver = asio_create_driver(name);
	if (!driver) return false;
	info->name = name;

	bool ret = asio_get_deviceinfo(driver, info);

	driver->Release();
	return ret;
}

void asio_get_deviceinfos(std::vector<deviceinfo>* devices) {

	CRegKey pluginRootKey;

	if (ERROR_SUCCESS != pluginRootKey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\ASIO", KEY_READ)) {
		cerr << "asiodriver: cannot open HKLM\\SOFTWARE\\ASIO" << endl;
		return ;
	}

	for (int i = 0;; ++i) {
		DWORD keySize = 1024;
		TCHAR keyName[1024];
		FILETIME keyTime;
		if (ERROR_SUCCESS != pluginRootKey.EnumKey(i, keyName, &keySize, &keyTime))
			break;

		cerr << "asiodriver: probing " << keyName << endl;
		deviceinfo asiodevice;
		if (asio_get_deviceinfo(keyName, &asiodevice))
			devices->push_back(asiodevice);
	}

	pluginRootKey.Close();
}
