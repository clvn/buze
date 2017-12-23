#define BOOST_ATOMIC_NO_LIB
#if defined(_WIN32)
#include <windows.h>
#endif
#if defined(POSIX)
#include <pthread.h>
#include <unistd.h>
#endif
#include <vector>
#include <cassert>
#include <boost/atomic.hpp>
#include "ringbuffer.h"
#include "allocpool.h"

allocpool::allocpool() : availbuffercount(0) {
	running = false;
#if defined(_WIN32)
	hThread = 0;
#endif
#if defined(POSIX)
	thread = 0;
#endif
}

allocpool::~allocpool() {
	stop();
}

void allocpool::prealloc() {
	while (availbuffercount < pool_prealloc_chunks) {
		bool result = buffers.push(new char[pool_chunk_size]);
		assert(result);
		availbuffercount++;
	}
}

char* allocpool::alloc(int* size) {
	while (buffers.empty()) sleep(1);
	char* buffer;
	bool result = buffers.pop(buffer);
	assert(result);
	availbuffercount--;
	*size = pool_chunk_size;
	return buffer;
}

void allocpool::run() {

	running = true;
	while (running) {
		prealloc();
		sleep(1);
	}
}

#if defined(_WIN32)

DWORD WINAPI AllocPoolProc(LPVOID lpParam) {
	allocpool* pool = (allocpool*)lpParam;
	pool->run();
	return 0;
}

bool allocpool::start() {
	DWORD threadid;
	hThread = CreateThread(0, 0, AllocPoolProc, this, 0, &threadid);
	Sleep(100); // TODO: use a signal
	return hThread != 0;
}

void allocpool::stop() {
	if (!hThread) return ;
	running = false;
	WaitForSingleObject(hThread, INFINITE);
}

void allocpool::sleep(int ms) {
	Sleep(ms);
}

#elif defined(POSIX)

void* alloc_pool_proc(void* arg) {
	allocpool* pool = (allocpool*)arg;
	pool->run();
	return 0;
}

bool allocpool::start() {
	if (pthread_create(&thread, 0, alloc_pool_proc, this))
		return false;
	return true;
}

void allocpool::stop() {
	if (!thread) return ;
	running = false;
	pthread_join(thread, 0);
	thread = 0;
}

void allocpool::sleep(int ms) {
	::usleep(ms * 1000);
}

#else

#error Needs an implementation of allocpool::start(), stop() and sleep()

#endif
