#pragma once

struct audioapi;

namespace armstrong {

namespace frontend {

struct audiodriver_native : audiodriver {

	audioapi* audio;
	zzub::timer timer;								// hires timer, for cpu-meter
	double last_work_time;							// length of last WorkStereo
	double cpu_load;
	bool dirty_devices;

	audiodriver_native();
	virtual ~audiodriver_native();	
	virtual void initialize(audioworker *worker);	
	virtual bool enable(bool e);	
	virtual bool create_device(const std::string& outputname, const std::string& inputname, int buffersize, int samplerate);
	virtual void destroy_device();
	virtual double get_cpu_load();
	virtual void configure();
	virtual int get_device_count();
	virtual audiodevice* get_device_by_name(const std::string& name);
	virtual audiodevice* get_device(int index);

	void enumerate_devices();
	void process(float** pout, float** pin, int numsamples);
};

}
}
