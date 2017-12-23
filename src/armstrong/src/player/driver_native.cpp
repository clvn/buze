/*
Copyright (C) 2012 Anders Ervik <calvin@countzero.no>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <iostream>
#include <cassert>
#include <cstring>
#include "mixing/timer.h"
#include "driver.h"
#include "driver_native.h"
#include "audiodriver/audiodriver.h"
#include "audiodriver/audioapi.h"

using namespace std;

namespace armstrong {

namespace frontend {

void audio_callback(callbacktype type, float** pout, float** pin, int numsamples, void* userdata) {
	audiodriver_native *self = (audiodriver_native *)userdata;

	if (type == callbacktype_buffer) {
		self->process(pout, pin, numsamples);
	
	} else if (type == callbacktype_latency) {
		self->worker->work_buffersize = self->audio->get_buffersize();
		self->worker->work_latency = self->audio->get_latency();
		self->worker->latency_changed();
	
	} else if (type == callbacktype_samplerate) {
		self->worker->work_rate = self->audio->get_samplerate();
		self->worker->samplerate_changed();
	
	} else if (type == callbacktype_reset) {
		// a device reset means the current device's buffersize, samplerate 
		// settings may have changed. set the dirty_devices flag, such that
		// the next attempt to read the deviceinfos from the user thread will 
		// update the device descriptions from the audioapi
		self->worker->work_rate = self->audio->get_samplerate();
		self->worker->work_buffersize = self->audio->get_buffersize();
		self->worker->work_latency = self->audio->get_latency();
		self->dirty_devices = true;
		self->worker->device_reset();
	}
}

audiodriver_native::audiodriver_native() {
	audio = new audioapi();
	cpu_load = 0;
	last_work_time = 0;
	timer.start();
	dirty_devices = false;
}

audiodriver_native::~audiodriver_native() {
	destroy_device();
	if (audio) {
		delete audio;
		audio = 0;
	}
}

void audiodriver_native::initialize(audioworker* _worker) {
	worker = _worker;
	audio->initialize();
	enumerate_devices();
}

int audiodriver_native::get_device_count() {
	if (dirty_devices) enumerate_devices();
	return audiodriver::get_device_count();
}

audiodevice* audiodriver_native::get_device_by_name(const std::string& name) {
	if (dirty_devices) enumerate_devices();
	return audiodriver::get_device_by_name(name);
}

audiodevice* audiodriver_native::get_device(int index) {
	if (dirty_devices) enumerate_devices();
	return audiodriver::get_device(index);
}

void audiodriver_native::enumerate_devices() {
	clear_devices();
	for (std::vector<deviceinfo>::iterator i = audio->devices.begin(); i != audio->devices.end(); ++i) {
		deviceinfo& info = *i;
		audiodevice ad;
		ad.name = info.name;
		ad.api_id = info.api;
		ad.device_id = (int)std::distance(audio->devices.begin(), i);
		ad.out_channels = info.numoutputs;
		ad.in_channels = info.numinputs;
		ad.default_samplerate = info.defaultsamplerate;
		ad.samplerates.insert(ad.samplerates.end(), info.samplerates.begin(), info.samplerates.end());
		ad.default_buffersize = info.defaultbuffersize;
		ad.buffersizes.insert(ad.buffersizes.end(), info.buffersizes.begin(), info.buffersizes.end());
		add_device(ad);
	}
	dirty_devices = false;
}

bool audiodriver_native::enable(bool e) {
	//if (!audio) return false;

	if (e) {
		if (!worker->work_started) {
			audio->start();
			worker->jackhandle = audio->get_jack_handle();
			worker->work_started = true;
			worker->audio_enabled();
		}
		return true;
	} else {
		if (worker->work_started) {
			worker->audio_disabled();
			audio->stop();
			worker->jackhandle = 0;
			worker->work_started = false;
		}
		return true;
	}
}

bool audiodriver_native::create_device(const std::string& outputname, const std::string& inputname, int buffersize, int samplerate) {
	cout << "creating output device '" << outputname << "' with " << samplerate << "Hz samplerate" << " and " << buffersize << " buffer size" << endl;
	if (!inputname.empty())
		cout << "creating input device '" << inputname << "'" << endl;

	if (!audio->create_device(outputname, inputname, buffersize, samplerate, audio_callback, this)) {
		return false;
	}

	worker->work_out_device = outputname;
	worker->work_in_device = inputname;
	worker->work_rate = audio->get_samplerate();
	worker->work_buffersize = audio->get_buffersize();
	worker->work_master_channel = master_channel;
	worker->work_latency = audio->get_latency();
	worker->work_out_first_channel = 0;
	worker->work_out_channel_count = audio->get_outputs();
	worker->work_in_first_channel = 0;
	worker->work_in_channel_count = audio->get_inputs();
	worker->samplerate_changed();
	return true;
}

void audiodriver_native::destroy_device() {
	enable(false);
	audio->destroy_device();

	worker->work_out_device = "";
	worker->work_in_device = "";
}

void audiodriver_native::process(float** pout, float** pin, int numsamples) {

	double start_time = timer.frame();

	int out_ch = worker->work_out_channel_count;
	int in_ch = worker->work_in_channel_count;

	float* plout[64] = {0};
	float* plin[64] = {0};
	for (int i = 0; i < in_ch; i++)
		if (pin)
			plin[i] = pin[i];

	for (int i = 0; i < out_ch; i++)
		if (pout)
			plout[i] = pout[i];

	worker->work_stereo(plin, plout, numsamples);

	for (int j = 0; j < out_ch; j++)
		if (plout[j] == 0) 
			memset(pout[j], 0, numsamples * sizeof(float));

	// update stats
	last_work_time = timer.frame() - start_time;
	double load = (last_work_time * double(worker->work_rate)) / double(numsamples);
	// slowly approach to new value
	cpu_load += 0.1 * (load - cpu_load);
}

double audiodriver_native::get_cpu_load() {
	return cpu_load;
}

void audiodriver_native::configure() {
	audio->configure();
}

} // namespace frontend
} // namespace armstrong
