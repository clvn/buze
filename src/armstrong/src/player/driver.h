/*
Copyright (C) 2003-2007 Anders Ervik <calvin@countzero.no>
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

#pragma once

#include <string>
#include <vector>

namespace armstrong {

namespace frontend {

struct audiodevice;
struct audioworker;

struct audiodevice {
	int api_id;
	int device_id;
	std::string name;
	int in_channels;
	int out_channels;
	int default_buffersize;
	int default_samplerate;
	std::vector<int> samplerates;
	std::vector<int> buffersizes;
};

struct audiodriver {
public:
	enum {
		// increase this if you get problems
		MAX_FRAMESIZE = 16384,
		MAX_CHANNELS = 64,
	};

	audioworker *worker;
	int master_channel;

	static audiodriver* create_native();
	static audiodriver* create_silent(const char* name, int out_channels, int in_channels, std::vector<unsigned int> rates);

	audiodriver();
	virtual ~audiodriver() {}
	virtual void initialize(audioworker *worker) = 0;
	virtual bool enable(bool e) = 0;
	virtual bool create_device(const std::string& outputname, const std::string& inputname, int buffersize, int samplerate) = 0;
	virtual void destroy_device() = 0;
	virtual double get_cpu_load() = 0;
	virtual void configure() = 0;

	virtual int get_device_count();
	virtual audiodevice* get_device_by_name(const std::string& name);
	virtual audiodevice* get_device(int index);
	int get_buffersize();
	int get_samplerate();

protected:
	void clear_devices();
	void add_device(const audiodevice& device);
private:
	std::vector<audiodevice> devices;
};



struct audioworker {
	int work_master_channel;		// 0..maxChannels/2
	int work_rate;
	int work_buffersize;
	int work_latency;
	bool work_started;

	std::string work_out_device;
	int work_out_first_channel;
	int work_out_channel_count;

	std::string work_in_device;
	int work_in_first_channel;
	int work_in_channel_count;

	void* jackhandle;

	audioworker() { 
		work_master_channel = 0; 
		work_rate = 48000;
		work_buffersize = 512;
		work_in_device = "";
		work_in_first_channel = 0;
		work_in_channel_count = 2;
		work_out_device = "";
		work_out_first_channel = 0;
		work_out_channel_count = 2;
		work_started = false;
		jackhandle = 0;
	}
	virtual ~audioworker() {}
	virtual void work_stereo(float** pin, float** pout, int num) {}
	virtual void audio_enabled() {}
	virtual void audio_disabled() {}
	virtual void samplerate_changed() {}
	virtual void latency_changed() {}
	virtual void device_reset() {}
};

}
}
