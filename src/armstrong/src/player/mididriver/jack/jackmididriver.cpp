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
#define NOMINMAX
#include <vector>
#include <list>
#include <algorithm>
#include <porttime.h>
#include <portmidi.h>
#include <pmutil.h>
#if defined(_STDINT) && !defined(_STDINT_H)
#define _STDINT_H
#endif
#include <jack/jack.h>
#include <jack/midiport.h>
#include <zzub/plugin.h>
#include "mixing/timer.h"
#include "../midiapi.h"
#include "jackmididriver.h"

static bool is_jack_installed = false;

const int BUFFER_EVENTS = 256;

mididriver_jack::mididriver_jack() {
	client = 0;
}

mididriver_jack::~mididriver_jack() {
	close();
}

bool mididriver_jack::initialize() {

	// check if jack is installed, use jackweakapi, if it is installed, create midi ports when ready
	// how to determine when ready?
	const char* jackversion = jack_get_version_string();
	if (jackversion == 0) return false ;

	is_jack_installed = true;
	midiports.clear();

	jackmidiport inport;
	inport.is_open = false;
	inport.name = "JACK->Armstrong MIDI Port";
	inport.port = 0;
	midiports.push_back(inport);

	jackmidiport outport;
	outport.is_open = false;
	outport.name = "Armstrong->JACK MIDI Port";
	outport.port = 0;
	midiports.push_back(outport);
	return true;
}

bool mididriver_jack::open_device(int index) {
	if (!is_jack_installed) return false;
	if (index > 1) return false;

	midiports[index].is_open = true;
	create_client_ports();
	return true;
}

void mididriver_jack::create_client_ports() {
	if (!client) return;

	if (midiports[0].is_open && midiports[0].port == 0) {
		midiports[0].port = jack_port_register(client, midiports[0].name.c_str(), JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	}

	if (midiports[1].is_open && midiports[1].port == 0) {
		midiports[1].port = jack_port_register(client, midiports[1].name.c_str(), JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
	}

}

void mididriver_jack::destroy_client_ports() {
	// TODO: disconnect anything?
	for (int i = 0; i <2 ; i++) {
		if (midiports[i].port != 0)
			jack_port_unregister(client, midiports[i].port);
		midiports[i].port = 0;
	}
}

bool mididriver_jack::close_all_devices() {
	if (!is_jack_installed) return false;
	destroy_client_ports();
	for (int i = 0; i <2 ; i++)
		midiports[i].is_open = false;
	return true;
}

void mididriver_jack::close() {
	destroy_client_ports();
}

int mididriver_jack::get_device_count() {
	if (!is_jack_installed) return 0;
	return 2;
}

bool mididriver_jack::is_input(int index) {
	if (!is_jack_installed) return false;
	if (index == 0) return true;
	return false;
}

bool mididriver_jack::is_output(int index) {
	if (!is_jack_installed) return false;
	if (index == 1) return true;
	return false;
}

bool mididriver_jack::is_open(int index) {
	if (!is_jack_installed) return false;
	return midiports[index].is_open;
}

const char* mididriver_jack::get_device_name(int index) {
	if (!is_jack_installed) return 0;
	return this->midiports[index].name.c_str();
}

#define MAKEMIDI(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))

bool mididriver_jack::poll(zzub::midi_message* poutmidi, int* midi_count, int maxcount, int numsamples, int samplespersec) {
	*midi_count = 0;
	if (!is_jack_installed) return true;

	if (midiports[0].is_open && midiports[0].port) {
		void* port_buf = jack_port_get_buffer(midiports[0].port, numsamples);
		jack_nframes_t midieventcount = jack_midi_get_event_count(port_buf);
		int counter = 0;
		for (int i = 0; i < (int)midieventcount; i++) {
			if (counter >= maxcount) break;
			jack_midi_event_t in_event;
			int result = jack_midi_event_get(&in_event, port_buf, i);
			if (result == 0) { // 0 == ok
				zzub::midi_message& message = poutmidi[counter];
				message.device = 0;
				message.timestamp = in_event.time;
				message.message = MAKEMIDI(in_event.buffer[0], in_event.buffer[1], in_event.buffer[2]);
				counter++;
			}
		}
		*midi_count = counter;
	}
	return true;
}

void mididriver_jack::set_latency(int sample_count, int samplespersec) {
	// no op - assume jack does latency itself
}

void mididriver_jack::schedule_send(zzub::midi_message* pmidi, int midi_count, int sample_count, int samplespersec) {
	if (!is_jack_installed) return ;

	if (midiports[1].is_open && midiports[1].port) {
		void* port_buf = jack_port_get_buffer(midiports[1].port, sample_count);
		jack_midi_clear_buffer(port_buf);
		jack_nframes_t maxcount = jack_midi_max_event_size(port_buf);
		int counter = 0;
		for (int i = 0 ; i < std::min((int)maxcount, midi_count); i++) {
			unsigned long message = pmidi[i].message;
			unsigned char status = message & 0xff;
			unsigned char data1 = (message >> 8) & 0xff;
			unsigned char data2 = (message >> 16) & 0xff;
			jack_midi_data_t mididata[4] = { status, data1, data2, 0 };
			jack_midi_event_write(port_buf, pmidi[i].timestamp, mididata, 3);
		}
	}
}

void mididriver_jack::set_jack_handle(void* handle) {
	if (!is_jack_installed) return;

	// if setting the handle to 0 and client exists, let go of the ports
	if (!handle && client)
		destroy_client_ports();

	client = (jack_client_t*)handle;
	if (client)
		create_client_ports();
}
