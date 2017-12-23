#if !defined(LUNAR_PLUGIN_LOCALS_H) && !defined(LUNAR_STD_BUILD)
#error You must first include the header generated for the Lunar plugin before including fx.hpp.
#endif

#if !defined(LUNAR_FX_HPP)
#define LUNAR_FX_HPP

#include "fx.h"

// if your platform expects a different type
// for the size argument, extend the 
// lunar_size_t define in fx.h
inline void *operator new (size_t size) {
	return malloc(size);
}

inline void operator delete (void *ptr) {
	free(ptr);
}

inline void *operator new[] (size_t size) {
	return malloc(size);
}
inline void operator delete[] (void *ptr) {
	free(ptr);
}

namespace lunar {

typedef ::lunar_transport_t transport;
typedef ::lunar_host_t host;
	
template<typename inheritantT>
class fx : public ::lunar_fx {
private:
	static void _init(::lunar_fx *h) {
		static_cast<inheritantT*>(h)->init();
	}
	
	static void _exit(::lunar_fx *h) {
		delete static_cast<inheritantT*>(h);
	}
	
	static void _process_events(::lunar_fx *h) {
		static_cast<inheritantT*>(h)->process_events();
	}

	static void _process_controller_events(::lunar_fx *h) {
		static_cast<inheritantT*>(h)->process_controller_events();
	}
	
	static void _process_stereo(::lunar_fx *h, float *a, float *b,float *c, float *d, int n) {
		static_cast<inheritantT*>(h)->process_stereo(a,b,c,d,n);
	}
	
	static void _process_timer(::lunar_fx *h, int* numsamples, int* processevents) {
		static_cast<inheritantT*>(h)->process_timer(numsamples, processevents);
	}
	
	static void _transport_changed(::lunar_fx *h) {
		static_cast<inheritantT*>(h)->transport_changed();
	}

	static void _attributes_changed(::lunar_fx *h) {
		static_cast<inheritantT*>(h)->attributes_changed();
	}
	
	static int _get_latency(::lunar_fx* h) {
		return static_cast<inheritantT*>(h)->get_latency();
	}

	static void _redraw_gui(::lunar_fx* h) {
		static_cast<inheritantT*>(h)->redraw_gui();
	}

	static void _resize_gui(::lunar_fx* h, int width, int height) {
		static_cast<inheritantT*>(h)->resize_gui(width,  height);
	}

	static bool _process_audio(::lunar_fx* h, float** ins, float** outs, int n) {
		return static_cast<inheritantT*>(h)->process_audio(ins, outs, n);
	}

	static void _process_midi(::lunar_fx* h, lunar_midi_message_t* inm, int n) {
		static_cast<inheritantT*>(h)->process_midi(inm, n);
	}

	static const char* _describe_value(::lunar_fx* h, lunar_parameter_names_t param, float value) {
		return static_cast<inheritantT*>(h)->describe_value(param, value);
	}

public:

	fx() {
		::lunar_init_fx(this);
		::lunar_fx *_fx = this;
		_fx->init = _init;
		_fx->exit = _exit;
		_fx->process_events = _process_events;
		_fx->process_controller_events = _process_controller_events;
		_fx->process_stereo = _process_stereo;
		_fx->process_timer = _process_timer;
		_fx->transport_changed = _transport_changed;
		_fx->attributes_changed = _attributes_changed;
		_fx->get_latency = _get_latency;
		_fx->redraw_gui = _redraw_gui;
		_fx->resize_gui = _resize_gui;
		_fx->process_audio = _process_audio;
		_fx->process_midi = _process_midi;
		_fx->describe_value = _describe_value;
	}

	void request_gui_redraw() { lunar_request_gui_redraw(host); }
	void midi_out(lunar_midi_message_t* outm, int n) { lunar_midi_out(host, outm, n); }
	void note_out(lunar_note_message_t* outn, int n) { lunar_note_out(host, outn, n); }
	bool is_channel_connected(int index) { return lunar_is_channel_connected(host, index); }

	void init() {}
	void process_events() {}
	void process_controller_events() {}
	void process_stereo(float *inL, float *inR, float *outL, float *outR, int n) {}
	void process_timer(int* numsamples, int* processevents) { }
	void transport_changed() {}
	void attributes_changed() {}
	int get_latency() { return 0; }
	void redraw_gui() {}
	void resize_gui(int width, int heigth) {}
	bool process_audio(float** ins, float** outs, int n) { return false; }
	void process_midi(lunar_midi_message_t* inm, int n) {}
	const char* describe_value(lunar_parameter_names_t param, float value) { return 0; }
};
	
} // namespace lunar

#endif // LUNAR_FX_HPP
