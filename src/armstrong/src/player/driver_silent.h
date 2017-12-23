/*
Copyright (C) 2008-2012 Anders Ervik <calvin@countzero.no>

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

/*

	Implements a silent audio driver without a player thread. 
	Unlike the other drivers, clients are expected to poll with work_stereo() manually (??)

*/

#pragma once

namespace armstrong {

namespace frontend {

struct audiodriver_silent : audiodriver {

	virtual void enumerate_devices() {}

	virtual void initialize(audioworker *worker) { 
		this->worker = worker; 
	}

	virtual bool enable(bool e) {
		worker->work_started = e;
		return true;
	}

	virtual bool create_device(const std::string& outputname, const std::string& inputname, int buffersize, int samplerate) {
		worker->work_out_device = outputname; //&device;
		worker->work_in_device = inputname; //inputIndex != -1 ? &device : 0;
		worker->work_rate = samplerate;
		worker->work_buffersize = buffersize;
		worker->work_master_channel = master_channel;
		worker->work_latency = 0;
		worker->samplerate_changed();
		return true; 
	}

	virtual void destroy_device() { 
		enable(false);
	}

	virtual double get_cpu_load() { 
		return 0; 
	}

	virtual void configure() {
	}

};

}
}
