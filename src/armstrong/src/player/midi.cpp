/*
Copyright (C) 2013 Anders Ervik <calvin@countzero.no>

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

#include <vector>
#include <list>
#if !defined(HAVE_PORTMIDI) || HAVE_PORTMIDI == 1
#include <porttime.h>
#include <portmidi.h>
#include <pmutil.h>
#endif
#include <jack/jack.h>
#include <zzub/plugin.h>
#include "mixing/timer.h"
#include "midi.h"
#include "mididriver/midiapi.h"
#if !defined(HAVE_PORTMIDI) || HAVE_PORTMIDI == 1
#include "mididriver/portmidi/portmididriver.h"
#endif
#if !defined(HAVE_JACK) || HAVE_JACK == 1
#include "mididriver/jack/jackmididriver.h"
#endif

namespace armstrong {
	namespace frontend {

mididriver::mididriver() {
	apis.push_back(new mididriver_portmidi());
	apis.push_back(new mididriver_jack());
}

bool mididriver::initialize() {
	devices.clear();
	for (std::vector<midiapi*>::iterator i = apis.begin(); i != apis.end(); ++i) {
		midiapi* api = *i;
		if (!api->initialize()) continue;
		for (int j = 0; j < api->get_device_count(); j++) {
			mididevice dev;
			dev.device_index = j;
			dev.api_index = (int)std::distance(apis.begin(), i);
			devices.push_back(dev);
		}
	}

	return true;
}

bool mididriver::openDevice(size_t index) {
	mididevice* device = &devices[index];
	midiapi* api = apis[device->api_index];
	return api->open_device(device->device_index);
}

bool mididriver::closeAllDevices() {
	for (std::vector<midiapi*>::iterator i = apis.begin(); i != apis.end(); ++i) {
		midiapi* api = *i;
		api->close_all_devices();
	}
	return true;
}

void mididriver::close() {
	for (std::vector<midiapi*>::iterator i = apis.begin(); i != apis.end(); ++i) {
		midiapi* api = *i;
		api->close();
		delete api;
	}
	apis.clear();
	devices.clear();
}

size_t mididriver::getDevices() {
	return devices.size();
}

bool mididriver::isInput(size_t index) {
	mididevice* device = &devices[index];
	midiapi* api = apis[device->api_index];
	return api->is_input(device->device_index);
}

bool mididriver::isOutput(size_t index) {
	mididevice* device = &devices[index];
	midiapi* api = apis[device->api_index];
	return api->is_output(device->device_index);
}

bool mididriver::isOpen(size_t index) {
	mididevice* device = &devices[index];
	midiapi* api = apis[device->api_index];
	return api->is_open(device->device_index);
}

const char* mididriver::getDeviceName(size_t index) {
	mididevice* device = &devices[index];
	midiapi* api = apis[device->api_index];
	return api->get_device_name(device->device_index);
}

bool mididriver::poll(zzub::midi_message* pmidi, int* midi_count, int maxcount, int sample_count, int samplespersec) {
	zzub::midi_message* pcurrent = pmidi;
	int remaining = maxcount;
	int api_index = 0;
	for (std::vector<midiapi*>::iterator i = apis.begin(); i != apis.end(); ++i) {
		midiapi* api = *i;
		int written = 0;
		int offset = maxcount - remaining;
		if (!api->poll(&pmidi[offset], &written, remaining, sample_count, samplespersec))
			return false;
		// fix the message device by api start index
		for (int j = 0; j < written; j++) {
			pmidi[offset + j].device += api_index;
		}
		remaining -= written;
		api_index += api->get_device_count();
	}
	*midi_count = maxcount - remaining;
	return true;
}

void mididriver::set_latency(int sample_count, int samplespersec) {
	for (std::vector<midiapi*>::iterator i = apis.begin(); i != apis.end(); ++i) {
		midiapi* api = *i;
		api->set_latency(sample_count, samplespersec);
	}
}

void mididriver::schedule_send(zzub::midi_message* pmidi, int midi_count, int sample_count, int samplespersec) {
	// NOTE: schedule_send must be called once per audio driver buffer, passing all events at once
	// TODO: events must be sorted, and remapped to a device!
	assert(midi_count < 4*1024);
	zzub::midi_message messages[4*1024];

	for (std::vector<midiapi*>::iterator i = apis.begin(); i != apis.end(); ++i) {
		midiapi* api = *i;
		int message_counter = 0;
		for (int j = 0; j < midi_count; j++) {
			zzub::midi_message& msg = pmidi[j];
			mididevice* device = &devices[msg.device];
			midiapi* deviceapi = apis[device->api_index];
			if (deviceapi != api) continue;

			zzub::midi_message& devicemsg = messages[message_counter];
			devicemsg.device = device->device_index;
			devicemsg.timestamp = msg.timestamp;
			devicemsg.message = msg.message;
			message_counter++;
		}
		api->schedule_send(messages, message_counter, sample_count, samplespersec);
	}

	/*
	for (int i = 0; i < midi_count; i++) {
		zzub::midi_message& msg = pmidi[i];
		mididevice* device = &devices[msg.device];
		midiapi* api = apis[device->api_index];
		// fix the message device by api start index
		msg.device = device->device_index;
		api->schedule_send(&msg, 1, sample_count);
	}*/
}

void mididriver::set_jack_handle(void* handle) {
	apis[1]->set_jack_handle(handle);
}


	}
}
