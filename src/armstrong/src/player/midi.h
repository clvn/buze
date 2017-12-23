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

struct midiapi;

namespace armstrong {
	namespace frontend {

struct mididevice {
	int api_index;
	int device_index;
};

struct mididriver {
	std::vector<midiapi*> apis;
	std::vector<mididevice> devices;

	mididriver();
	virtual ~mididriver() { }
	virtual bool initialize();
	virtual bool openDevice(size_t index);
	virtual bool closeAllDevices();
	virtual void close();
	virtual size_t getDevices();
	virtual bool isInput(size_t index);
	virtual bool isOutput(size_t index);
	virtual bool isOpen(size_t index);
	virtual const char* getDeviceName(size_t index);

	virtual bool poll(zzub::midi_message* pmidi, int* midi_count, int maxcount, int sample_count, int samplespersec);
	virtual void schedule_send(zzub::midi_message* pmidi, int midi_count, int sample_count, int samplespersec);
	void set_jack_handle(void* handle);
	void set_latency(int sample_count, int samplespersec);
};

	};
};
