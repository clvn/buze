#pragma once

/*
"encoder" is a general label for plugins that perform some kind of buffered,
offline operations outside of the audio thread. 

the encoder class receives buffers from the audio thread in a lock-/wait free manner
and passes the buffers to plugins in a separate thread.
*/

enum encoder_event_type {
	encoder_event_type_set_state,
	encoder_event_type_set_chunk,
	encoder_event_type_add_plugin,
	encoder_event_type_remove_plugin,
};

struct encoder_event_data {
	struct encoder_event_data_set_state {
		zzub_encoder_state newstate;
	};

	struct encoder_event_data_set_chunk {
		zzub::plugin* plugin;
		float* buffer[128];
		int numchannels;
		int numsamples;
	};

	struct encoder_event_data_plugin {
		zzub::plugin* plugin;
	};

	encoder_event_type type;
	union {
		encoder_event_data_set_state set_state;
		encoder_event_data_set_chunk set_chunk;
		encoder_event_data_plugin add_plugin;
		encoder_event_data_plugin remove_plugin;
	};
};

struct encoder {
protected:
	volatile bool running;
	std::vector<zzub::plugin*> plugins;
	zzub_encoder_state state;
	boost::lockfree::spsc_queue<encoder_event_data, boost::lockfree::capacity<256> > encoder_event_queue;

#if defined(_WIN32)
	HANDLE hThread;
	friend DWORD WINAPI EncoderProc(LPVOID lpParam);
#endif
#if defined(POSIX)
	pthread_t thread;
	friend void* encoder_proc(void* arg);
#endif
public:
	thread_id_t encoder_thread_id;						// thread id as returned by GetCurrentThreadId or pthread_self

	encoder();
	~encoder();

	// user thread
	bool start();
	void stop();

	// audio thread
	void register_encoder(zzub::plugin* plugin);
	void unregister_encoder(zzub::plugin* plugin);
	void set_chunk(zzub::plugin* plugin, float** buffers, int numchannels, int numsamples);
	void set_state(zzub_encoder_state newstate);
protected:
	inline void sleep(int ms);
	void run();

	// encoder thread
	void on_encoder_event(encoder_event_data& ev);
	void on_set_state(zzub_encoder_state newstate);
	void on_set_chunk(zzub::plugin* plugin, float** buffers, int numchannels, int numsamples);
	void on_register_encoder(zzub::plugin* plugin);
	void on_unregister_encoder(zzub::plugin* plugin);
};
