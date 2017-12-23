/*
Copyright (C) 2003-2013 Anders Ervik <calvin@countzero.no>

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
#include <porttime.h>
#include <portmidi.h>
#include <pmutil.h>
#include <zzub/plugin.h>
#include "mixing/timer.h"
#include "../midiapi.h"
#include "portmididriver.h"

const int BUFFER_EVENTS = 256;

mididriver_portmidi::~mididriver_portmidi() {
	close();
}

void process_midi(PtTimestamp timestamp, void *userData) {
	mididriver_portmidi* driver = (mididriver_portmidi*)userData;
	PmError result;

	zzub::midi_message msg;

	PmEvent event[BUFFER_EVENTS];
	for (size_t i = 0; i < driver->devices.size(); i++) {
		if (driver->devices[i] == 0) continue;
		if (driver->is_input(i) && (TRUE == Pm_Poll(driver->devices[i]))) { 
			int ret = Pm_Read(driver->devices[i], event, BUFFER_EVENTS);
			if (ret<0) continue;
			for (int j = 0; j < ret; j++) {
				msg.message = event[j].message;
				msg.timestamp = event[j].timestamp;
				Pm_Enqueue(driver->readQueue, &msg);
			}
		}
	}


	while (result = Pm_Dequeue(driver->sendQueue, &msg)) {
		driver->outMessages.push_back(msg);
	}

	double time = driver->timer.frame();
	double diff_ms = (time - driver->lastTime) * 1000 * 1000;	// using microseconds internally

	for (std::list<zzub::midi_message>::iterator i = driver->outMessages.begin(); i != driver->outMessages.end(); ) {
		if (i->timestamp > diff_ms)
			i->timestamp -= (unsigned long)diff_ms; else
			i->timestamp = 0;

		if (i->timestamp == 0) {
			driver->send(i->device, i->message);
			i = driver->outMessages.erase(i);
		} else
			i++;
	}

	driver->lastTime = time;
}

bool mididriver_portmidi::initialize() {
	this->readQueue = Pm_QueueCreate(32, sizeof(zzub::midi_message));
	this->sendQueue = Pm_QueueCreate(32, sizeof(zzub::midi_message));

	timer.start();
	lastTime = timer.frame();
	Pt_Start(1, &process_midi, this);  // start 1ms timer

	if (pmNoError != Pm_Initialize()) return false;

	devices.resize(get_device_count());

	return true;
}

bool mididriver_portmidi::open_device(int index) {
	PortMidiStream* stream;

	const PmDeviceInfo* deviceInfo = Pm_GetDeviceInfo(index);
	if (deviceInfo->input) {
		if (pmNoError != Pm_OpenInput(&stream, index, 0, BUFFER_EVENTS, 0, 0))
			return false;
	} else
	if (deviceInfo->output) {
		if (pmNoError != Pm_OpenOutput(&stream, index, 0, BUFFER_EVENTS, 0, 0, 0))
			return false;
	}

	devices[index] = stream;

	return true;
}

bool mididriver_portmidi::close_all_devices() {
	for (size_t i = 0; i < devices.size(); i++) {
		if (devices[i] != 0) {
			PortMidiStream* stream = devices[i];
			devices[i] = 0;
			Pm_Close(stream);
		}
	}

	return true;
}

void mididriver_portmidi::close() {
	Pt_Stop();
	Pm_Terminate();

	Pm_QueueDestroy(readQueue);
	Pm_QueueDestroy(sendQueue);
	readQueue = 0;
	sendQueue = 0;
}

int mididriver_portmidi::get_device_count() {
	return Pm_CountDevices();
}

bool mididriver_portmidi::is_input(int index) {
	const PmDeviceInfo* deviceInfo = Pm_GetDeviceInfo(index);
	return deviceInfo->input != 0;
}

bool mididriver_portmidi::is_output(int index) {
	const PmDeviceInfo* deviceInfo = Pm_GetDeviceInfo(index);
	return deviceInfo->output != 0;
}

bool mididriver_portmidi::is_open(int index) {
	const PmDeviceInfo* deviceInfo = Pm_GetDeviceInfo(index);
	return deviceInfo->opened != 0;
}

const char* mididriver_portmidi::get_device_name(int index) {
	const PmDeviceInfo* deviceInfo = Pm_GetDeviceInfo(index);
	return deviceInfo->name;
}

bool mididriver_portmidi::poll(zzub::midi_message* poutmidi, int* midi_count, int maxcount, int sample_count, int samplespersec) {

	if (readQueue == 0 || sendQueue == 0) return false;

	int counter = 0;
	PmError result;
	do {
		zzub::midi_message msg;
		result = Pm_Dequeue(readQueue, &msg);
		// convert from msec to sampels, timestamp comes straight from Pm_Poll, assuming msec
		poutmidi[counter].timestamp = msg.timestamp * samplespersec / 1000;
		poutmidi[counter].message = msg.message;
		poutmidi[counter].device = msg.device;
		counter++;
	} while (result);
	*midi_count = counter - 1;
	return true;
}

void mididriver_portmidi::set_latency(int sample_count, int samplespersec) {
	latency_usec = (unsigned long)sample_count * 1000 * 1000 / samplespersec;
}

void mididriver_portmidi::schedule_send(zzub::midi_message* pmidi, int midi_count, int sample_count, int samplespersec) {
	// NOTE: incoming timestamps are in samples - adjust for audio driver latency and convert to us
	for (int i = 0 ; i < midi_count; i++) {
		zzub::midi_message msg;
		msg.device = pmidi[i].device;
		msg.timestamp = latency_usec + pmidi[i].timestamp * 1000 * 1000 / samplespersec;
		msg.message = pmidi[i].message;
		Pm_Enqueue(sendQueue, &msg);
	}
}

bool mididriver_portmidi::send(size_t index, unsigned int data) {
	if (index >= devices.size()) return false;
	if (devices[index] == 0) return false;

	PmEvent event = { data, 0 };
	Pm_Write(devices[index], &event, 1);
	return true;
}

