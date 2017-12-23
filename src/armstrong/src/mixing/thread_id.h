#pragma once

#ifdef _WIN32

#define NOMINMAX
#include <windows.h>

typedef DWORD thread_id_t;
typedef HANDLE thread_handle_t;

enum threadpriority {
	threadpriority_high = THREAD_PRIORITY_HIGHEST,
	threadpriority_normal = THREAD_PRIORITY_NORMAL
};

#else

#include <pthread.h>

// NOTE/TODO: pthread_t could be an opaque type, probably wont work:

typedef pthread_t thread_id_t;
typedef pthread_t thread_handle_t;

enum threadpriority {
	threadpriority_high,
	threadpriority_normal
};

#endif


extern "C" typedef void* (*thread_proc)(void*);

struct thread_id {
	static thread_id_t get();
	static thread_handle_t create_thread(thread_proc proc, void* data);
	static void set_priority(thread_handle_t handle, threadpriority pri);
	static void join(thread_handle_t handle);
};
