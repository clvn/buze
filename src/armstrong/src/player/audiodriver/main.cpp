#define NOMINMAX
#include <atlbase.h>
#include <atlcom.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>
#include <limits>
#include "audiodriver.h"
#include "asio/asio.h"
#include "asio/asiodriver.h"
#include "dsound/dsound.h"
#include "dsound/dsounddriver.h"
#include <jack/jack.h>
#include "jack/jackdriver.h"
#include "audioapi.h"

using std::cerr;
using std::cout;
using std::cin;
using std::endl;

int ccc = 0;

void audio_callback(callbacktype type, float** pout, float** pin, int numsamples, void* userdata) {
	if (type == callbacktype_buffer) {
		//cout << "its a callback: " << (int)userdata << endl;
		for (int i = 0; i < numsamples; i++) {
			float v = sin(float(ccc) / 100.0f);
			pout[0][i] = pout[1][i] = v;
			ccc++;
		}

	} else if (type == callbacktype_samplerate) {
		cout << "changed samplerate" << endl;
	} else if (type == callbacktype_latency) {
		cout << "changed latencies" << endl;
	} else if (type == callbacktype_reset) {
		cout << "reset eyeye" << endl;
	}
}

void print_devices(const std::vector<deviceinfo>& devices, bool verbose = true) {

	for (std::vector<deviceinfo>::const_iterator i = devices.begin(); i != devices.end(); ++i) {
		int index = (int)std::distance(devices.begin(), i);
		cout << index << ") " << i->name << ", " << i->numoutputs << ", " << i->numinputs << endl;
		if (verbose) {
			cout << "    samplerates:" << endl;
			for (std::vector<int>::const_iterator j = i->samplerates.begin(); j != i->samplerates.end(); ++j) {
				cout << "        " << *j << endl;
			}
			cout << "    buffer sizes:" << endl;
			for (std::vector<int>::const_iterator j = i->buffersizes.begin(); j != i->buffersizes.end(); ++j) {
				cout << "        " << *j << endl;
			}
		}
	}

}


int main() {
	CoInitialize(0);

	audioapi aa;
	aa.initialize();
	print_devices(aa.devices, false);

	int outindex = 0;
	int inindex = 0;
	cout << "Output index> ";
	cin >> outindex;
	cout << "Input index> ";
	cin >> inindex;

	deviceinfo* outdevice = &aa.devices[outindex];
	deviceinfo* indevice = (inindex > 0) ? &aa.devices[inindex] : 0;

	cout << "Using output device: " << aa.devices[outindex].name << endl;

	if (indevice != 0) {
		cout << "Using input device: " << aa.devices[inindex].name << endl;
	}

	int buffersize = 128;
	int samplerate = 44100;

	if (!aa.create_device(outdevice->name, indevice?indevice->name:"", buffersize, samplerate, &audio_callback, 0)) {
		cerr << "cant create device";
		return 2;
	}

	cout << "samplerate: " << aa.get_samplerate() << endl;
	cout << "buffersize: " << aa.get_buffersize() << endl;

	aa.start();

	cout << "q = quit, r = asio stop/start, c = asio config" << endl;
	for (;;) {
		std::string line;
		cout << "[q|r|c] ";
		cin >> line;
		if (line == "q") break;
		if (line == "r") {
			aa.stop();
			aa.start();
		}
		if (line == "c") {
			aa.configure();
		}
	}

	aa.destroy_device();
	
	CoUninitialize();

	return 0;
}


int jmain() {
	jack_device device;
	jack_create_device("anders", &audio_callback, 0, &device);

	jack_device_start(&device);

	cout << "q = quit, r = asio stop/start, c = asio config" << endl;
	for (;;) {
		std::string line;
		cout << "[q|r|c] ";
		cin >> line;
		if (line == "q") break;
		if (line == "r") {
			jack_device_stop(&device);
			jack_device_start(&device);
		}
	}

	jack_client_close (device.client);
	return 0;
}


int dsmain() {

	CoInitialize(0);

	std::vector<deviceinfo> devices;
	ds_get_devices(&devices);

	print_devices(devices, false);

	int deviceindex = 0;
	cout << "Device index> ";
	cin >> deviceindex;

	cout << "Using device: " << devices[deviceindex].name << endl;

	int buffersize = 2048;
	int samplerate = 48000;

	ds_device device;
	if (!ds_create_device(devices[deviceindex].name, "Primary Sound Capture Driver", 4, buffersize, samplerate, &audio_callback, 0, &device)) {
		cerr << "cant create device";
		return 2;
	}

	ds_device_start(&device);

	cout << "q = quit, r = asio stop/start, c = asio config" << endl;
	for (;;) {
		std::string line;
		cout << "[q|r|c] ";
		cin >> line;
		if (line == "q") break;
		if (line == "r") {
			ds_device_stop(&device);
			ds_device_start(&device);
		}
	}

	ds_destroy_device(&device);


}


int amain() {

	CoInitialize(0);

	std::vector<deviceinfo> devices;

	asio_get_deviceinfos(&devices);


	print_devices(devices);

	int deviceindex = 0;
	cout << "Device index> ";
	cin >> deviceindex;

	cout << "Using device: " << devices[deviceindex].name << endl;

	deviceinfo info;
	if (!asio_get_deviceinfo(devices[deviceindex].name, &info)) {
		cerr << "cant find device" << endl;
		return 3;
	}

	int buffersize = 128;
	int samplerate = 48000;
	info.get_default_deviceinfo(&buffersize, &samplerate);

	asio_device device;
	if (!asio_create_device(info.name, 0, 2, 0, 0, buffersize, samplerate, &audio_callback, (void*)666, &device)) {
	//if (!asio_create_device("kX ASIO Driver", 0, 2, 0, 0, 1024, 48000, &device)) {
		cout << "cant create device" << endl;
		return 2;
	}

	device.driver->start();

	cout << "q = quit, r = asio stop/start, c = asio config" << endl;
	for (;;) {
		std::string line;
		cout << "[q|r|c] ";
		cin >> line;
		if (line == "q") break;
		if (line == "r") {
			device.driver->stop();
			device.driver->start();
			// *some* devices request a reset here after stop/start 
			// (f.ex calvins steinberg ci2+, qu0ll's saffire usb, gimmeapills soundboard)
		}
		if (line == "c") {
			device.driver->controlPanel();
			// the control panel is modeless and requests a reset when the user presses ok. 
			// the driver must be 100% destroyed and recreated to pick up the new values
		}
	}

	device.driver->stop();

	asio_destroy_device(&device);

}
