#if defined(_WIN32)
#include <windows.h>
#endif
#if defined(POSIX)
#include <pthread.h>
#include <unistd.h>
#endif
#include <vector>
#include <cassert>
#include <zzub/plugin.h>
#include "thread_id.h"
#include "ringbuffer.h"
#include "encoder.h"

encoder::encoder() {
	running = false;
#if defined(_WIN32)
	hThread = 0;
#endif
#if defined(POSIX)
	thread = 0;
#endif
	state = zzub_encoder_state_stopped;
}

encoder::~encoder() {
	stop();
}

#if defined(_WIN32)

DWORD WINAPI EncoderProc(LPVOID lpParam) {
	encoder* enc = (encoder*)lpParam;
	enc->run();
	return 0;
}

bool encoder::start() {
	hThread = CreateThread(0, 0, EncoderProc, this, 0, &encoder_thread_id);
	Sleep(100); // TODO: use a signal
	return hThread != 0;
}

void encoder::stop() {
	if (!hThread) return ;
	running = false;
	WaitForSingleObject(hThread, INFINITE);
	hThread = 0;
}

void encoder::sleep(int ms) {
	Sleep(ms);
}

#elif defined(POSIX)

void* encoder_proc(void* arg) {
	encoder* enc = (encoder*)arg;
	enc->run();
	return 0;
}

bool encoder::start() {
	pthread_create(&thread, 0, encoder_proc, this);
	return true;
}


void encoder::stop() {
	if (!thread) return ;
	running = false;
	pthread_join(thread, 0);
	thread = 0;
}

void encoder::sleep(int ms) {
	::usleep(ms * 1000);
}

#else

#error Needs an implementation of encoder::start(), stop() and sleep()

#endif

void encoder::run() {
	running = true;
	while (running) {
		while (!encoder_event_queue.empty()) {
			encoder_event_data ev;
			encoder_event_queue.pop(ev);
			on_encoder_event(ev);
		}
		sleep(1);
	}
}

void encoder::on_encoder_event(encoder_event_data& ev) {
	switch (ev.type) {
		case encoder_event_type_set_state:
			on_set_state(ev.set_state.newstate);
			break;
		case encoder_event_type_set_chunk:
			on_set_chunk(ev.set_chunk.plugin, ev.set_chunk.buffer, ev.set_chunk.numchannels, ev.set_chunk.numsamples);
			break;
		case encoder_event_type_add_plugin:
			on_register_encoder(ev.add_plugin.plugin);
			break;
		case encoder_event_type_remove_plugin:
			on_unregister_encoder(ev.remove_plugin.plugin);
			break;
	}
}

void encoder::set_chunk(zzub::plugin* plugin, float** buffers, int numchannels, int numsamples) {
	encoder_event_data encev;
	encev.type = encoder_event_type_set_chunk;
	encev.set_chunk.plugin = plugin;
	encev.set_chunk.numchannels = numchannels;
	encev.set_chunk.numsamples = numsamples;
	for (int i = 0; i< numchannels; i++) {
		encev.set_chunk.buffer[i] = buffers[i];
	}
	encoder_event_queue.push(encev);

}

void encoder::on_set_chunk(zzub::plugin* plugin, float** buffers, int numchannels, int numsamples) {
	// free buffer memory if plugin::process_encoder returns false
	if (!plugin->process_encoder(state, buffers, numsamples)) {
		for (int i = 0; i < numchannels; i++) {
			delete[] buffers[i];
			buffers[i] = 0;
		}
	}
}

void encoder::set_state(zzub_encoder_state newstate) {
	encoder_event_data encev;
	encev.type = encoder_event_type_set_state;
	encev.set_state.newstate = newstate;
	encoder_event_queue.push(encev);
}

void encoder::on_set_state(zzub_encoder_state newstate) {
	state = newstate;
	for (std::vector<zzub::plugin*>::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		zzub::plugin* p = *i;
		p->process_encoder(state, 0, 0);
	}
}


void encoder::register_encoder(zzub::plugin* p) {
	encoder_event_data encev;
	encev.type = encoder_event_type_add_plugin;
	encev.add_plugin.plugin = p;
	encoder_event_queue.push(encev);
}

void encoder::on_register_encoder(zzub::plugin* p) {
	plugins.push_back(p);
	p->process_encoder(zzub_encoder_state_created, 0, 0);
}

void encoder::unregister_encoder(zzub::plugin* p) {
	encoder_event_data encev;
	encev.type = encoder_event_type_remove_plugin;
	encev.remove_plugin.plugin = p;
	encoder_event_queue.push(encev);
}

void encoder::on_unregister_encoder(zzub::plugin* p) {
	std::vector<zzub::plugin*>::iterator i = std::find(plugins.begin(), plugins.end(), p);
	assert(i != plugins.end());

	// notify plugin abot termination so it can optionally synchronize destruction across encoder/user threads
	p->process_encoder(zzub_encoder_state_deleted, 0, 0);
	plugins.erase(i);
}


