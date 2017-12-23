#pragma once

/*
creates a steady stream of pre-allocated chunks on a separate thread

if there are no available chunks, alloc() retries every 1ms until there is available memory.
otherwise there is no locking or waiting.

pool exhaustion could happen happen on rapid reallocation, the pool_prealloc_chunks
specify the number of preallocated chunks.

the caller is responsible for freeing the returned memory.
*/

struct allocpool {
	enum {
		pool_prealloc_chunks = 32,
		pool_chunk_size = sizeof(float) * 4096
	};
	volatile bool running;
	boost::lockfree::spsc_queue<char*, boost::lockfree::capacity<pool_prealloc_chunks + 1> > buffers;
	boost::atomic<int> availbuffercount;

#if defined(_WIN32)
	HANDLE hThread;
#endif
#if defined(POSIX)
	pthread_t thread;
#endif

	allocpool();
	~allocpool();
	void prealloc();
	char* alloc(int* size);
	bool start();
	void stop();
	void run();
	inline void sleep(int ms);
};
