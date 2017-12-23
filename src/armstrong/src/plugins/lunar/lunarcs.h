#pragma once

#if !defined(_WIN32)
#include <pthread.h>
#else
#include <windows.h>
#endif
#include <string>
#include <assert.h>
#if !defined(PTHREAD_MUTEX_RECURSIVE_NP)
#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#endif

namespace lunar {

class mutex  {
public:
	mutex() {
#if defined(_WIN32)
		InitializeCriticalSection(&m_sect); 
#else
		pthread_mutexattr_t mutexattr;   // Mutex attribute variable
		pthread_mutexattr_init(&mutexattr);
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m, &mutexattr);
		pthread_mutexattr_destroy(&mutexattr);
#endif
	}
	~mutex() {
#if defined(_WIN32)
		DeleteCriticalSection(&m_sect);
#else
		pthread_mutex_destroy (&m);
#endif
	}


	bool unlock() {
#if defined(_WIN32)
		LeaveCriticalSection(&m_sect); 
#else
		pthread_mutex_unlock (&m);
#endif
		return true;
	}

	bool lock() {
#if defined(_WIN32)
		EnterCriticalSection(&m_sect); 
#else
		pthread_mutex_lock (&m);
#endif
		return true; 
	}
protected:

#if defined(_WIN32)
	CRITICAL_SECTION m_sect;
#else
	pthread_mutex_t m;           // Mutex
#endif

	
};

class mutex_locker
{
protected:
	mutex* cs;

public:
	mutex_locker(mutex& cs)
	{
		this->cs = &cs;
		this->cs->lock();
	}
	
	~mutex_locker()
	{
		this->cs->unlock();
	}
};

} // namespace lunar
