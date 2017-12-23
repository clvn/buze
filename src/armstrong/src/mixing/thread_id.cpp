#include "thread_id.h"

thread_id_t thread_id::get() {
#ifdef _WIN32
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

#ifdef _WIN32

struct dataptr {
	thread_proc proc;
	void* data;
};

DWORD WINAPI CreateThreadProc(LPVOID lpParam) {
	dataptr* calldata = (dataptr*)lpParam;
	thread_proc proc = calldata->proc;
	void* data = calldata->data;
	delete calldata;
	return (DWORD)proc(data);
}
#endif

thread_handle_t thread_id::create_thread(thread_proc proc, void* data) {
#ifdef _WIN32
	DWORD id;
	dataptr* calldata = new dataptr();
	calldata->proc = proc;
	calldata->data = data;
	return CreateThread(0, 0, CreateThreadProc, calldata, 0, &id);
#else
	thread_handle_t id;
	pthread_create( &id, NULL, proc, data );
	return id;
#endif
}

void thread_id::set_priority(thread_handle_t handle, threadpriority pri) {
#if defined(_WIN32)
	// TODO: deal with different priorities in a system-neutral way; THREAD_PRIORITY_HIGHEST 
	SetThreadPriority(handle, (int)pri);
#else
	// scheduler?
#endif
}

void thread_id::join(thread_handle_t handle) {
#if defined(_WIN32)
	WaitForSingleObject(handle, INFINITE);
#else
	void* result;
	pthread_join(handle, &result);
#endif
}
