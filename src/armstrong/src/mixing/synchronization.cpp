#include <iostream>
#if defined(_WIN32)
#define NOMINMAX
#include <windows.h> 
#else
#include <pthread.h> 
#endif
#include "synchronization.h"

using namespace std;

namespace synchronization {

/***

	event

***/
#if defined(_WIN32)

struct event_win32 : sync_object {
	HANDLE hEvent;

	event_win32() {
		hEvent = 0;
	}
	virtual void initialize() {
		hEvent = CreateEvent(0, FALSE, FALSE, 0);
	}
	virtual void lock() {
		WaitForSingleObject(hEvent, INFINITE); 
	}
	virtual void unlock() {
		SetEvent(hEvent);
		
	}
	virtual void uninitialize() {
		CloseHandle(hEvent);
	}
};

#else

struct event_pthreads : sync_object {
	pthread_cond_t cond;
	pthread_mutex_t mutex;

	event_pthreads() { }

	virtual void initialize() {
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&cond, NULL);
	}

	virtual void lock() {
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond, &mutex);
		pthread_mutex_unlock(&mutex);
	}
	virtual void unlock() {
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
		
	}
	virtual void uninitialize() {
		pthread_cond_destroy(&cond);
		pthread_mutex_destroy(&mutex);
	}
};
#endif

event::event() {
#if defined(_WIN32)
	api = new event_win32();
#else
	api = new event_pthreads();
#endif
	api->initialize();
}

event::~event() {
	api->uninitialize();
	delete api;
}

	
/***

	critical section

***/

#if defined(_WIN32)
struct critical_section_win32 : sync_object {
	CRITICAL_SECTION cs;

	virtual void initialize() {
		InitializeCriticalSection(&cs); 
	}
	virtual void lock() {
		EnterCriticalSection(&cs); 
	}
	virtual void unlock() {
		LeaveCriticalSection(&cs); 
	}
	virtual void uninitialize() {
		DeleteCriticalSection(&cs); 
	}
};

#else

#if !defined(PTHREAD_MUTEX_RECURSIVE_NP)
#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#endif 

struct critical_section_pthreads : sync_object {
	pthread_mutex_t mutex;

	virtual void initialize() {
		pthread_mutexattr_t mutexattr;   // Mutex attribute variable
		pthread_mutexattr_init(&mutexattr);
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&mutex, &mutexattr);
		pthread_mutexattr_destroy(&mutexattr);
	}
	virtual void lock() {
		pthread_mutex_lock (&mutex);
	}
	virtual void unlock() {
		pthread_mutex_unlock (&mutex);
	}
	virtual void uninitialize() {
		pthread_mutex_destroy (&mutex);
	}
};

#endif

critical_section::critical_section() {
#if defined(_WIN32)
	api = new critical_section_win32();
#else
	api = new critical_section_pthreads();
#endif

	api->initialize();
}

critical_section::~critical_section() {
	api->uninitialize();
	delete api;
}

}
