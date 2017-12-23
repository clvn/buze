#pragma once

namespace synchronization {

struct sync_object {
	virtual void initialize() = 0;
	virtual void lock() = 0;
	virtual void unlock() = 0;
	virtual void uninitialize() = 0;
};

struct event {
	sync_object* api;

	event();
	~event();
	void wait() { api->lock(); }
	void signal() { api->unlock(); }
};

struct critical_section {
	sync_object* api;

	critical_section();
	~critical_section();

	void lock() { api->lock(); }
	void unlock() { api->unlock(); }
};

class critical_section_locker
{
protected:
	critical_section* cs;

public:
	critical_section_locker(critical_section& cs) {
		this->cs = &cs;
		this->cs->lock();
	}
	
	~critical_section_locker() {
		this->cs->unlock();
	}
};

}
