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

#pragma once

typedef void PortMidiStream;
typedef void PmQueue;

struct mididriver_portmidi : midiapi {
	std::list<zzub::midi_message> outMessages;
	zzub::timer timer;
	double lastTime;
	unsigned long latency_usec;
	PmQueue* sendQueue;
	PmQueue* readQueue;

	~mididriver_portmidi();

	std::vector<PortMidiStream*> devices;

	midiapitype get_api() { return midiapitype_portmidi; }
	virtual bool initialize();
	virtual bool open_device(int index);
	virtual bool close_all_devices();
	virtual void close();
	virtual int get_device_count();
	virtual bool is_input(int index);
	virtual bool is_output(int index);
	virtual bool is_open(int index);
	virtual const char* get_device_name(int index);
	virtual bool poll(zzub::midi_message* poutmidi, int* midi_count, int maxcount, int sample_count, int samplespersec);
	virtual void schedule_send(zzub::midi_message* pmidi, int midi_count, int sample_count, int samplespersec);
	virtual bool send(size_t index, unsigned int data);
	void set_jack_handle(void* handle) {}
	void set_latency(int sample_count, int samplespersec);
};
