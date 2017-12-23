/*
Copyright (C) 2003-2012 Anders Ervik <calvin@countzero.no>
Copyright (C) 2006-2007 Leonard Ritter

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
#include <string>
#include <cstring>
#include <complex>
#include "mixing/timer.h"
#include <cassert>
#include "driver.h"
#include "driver_silent.h"
#include "driver_native.h"

namespace armstrong {

namespace frontend {

//
// factories
//

audiodriver* audiodriver::create_native() {
	return new audiodriver_native();
}

audiodriver* audiodriver::create_silent(const char* name, int out_channels, int in_channels, std::vector<unsigned int> rates) {
	audiodriver_silent* driver = new audiodriver_silent();
	audiodevice device;
	device.name = name;
	device.out_channels = out_channels;
	device.in_channels = in_channels;
	device.samplerates.insert(device.samplerates.end(), rates.begin(), rates.end());
	driver->devices.push_back(device);
	return driver;
}

//
// implementation
//

audiodriver::audiodriver() {
	master_channel = 0;
	worker = 0;
}

void audiodriver::add_device(const audiodevice& device) {
	devices.push_back(device);
}

void audiodriver::clear_devices() {
	devices.clear();
}

int audiodriver::get_device_count() {
	return (int)devices.size();
}

audiodevice* audiodriver::get_device(int index) {
	return &devices[index];
}

audiodevice* audiodriver::get_device_by_name(const std::string& name) {
	for (std::vector<audiodevice>::iterator i =	devices.begin(); i != devices.end(); ++i) 
		if (i->name == name) return &*i;
	return 0;
}

int audiodriver::get_buffersize() {
	return worker->work_buffersize;
}

int audiodriver::get_samplerate() {
	return worker->work_rate;
}

} // namespace frontend
} // namespace armstrong
