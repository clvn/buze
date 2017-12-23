#define _USE_MATH_DEFINES
#define NOMINMAX
#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
};
typedef struct zzub::mixer zzub_mixer_t;

#include <zzub/plugin.h>
#include <sstream>
#include <cassert>
#include <iostream>
#include <cmath>
#include <limits>
#include <iomanip>
#if defined(_WIN32)
#include <dinput.h>
#endif
#include "mixing/mixer.h"
#include "peer.h"
#include "MersenneTwister.h"

struct intervalhelper {
	static int schedule_interval(int interval_type, int interval_length, float samples_per_tick, float samples_per_second, float tick_position) {
		int numsamples;
		switch (interval_type) {
			case 0:
				// should only pulse when tick_position == 0, if not, sync to the next tick
				if (tick_position < 1) {
					numsamples = (int)(samples_per_tick * interval_length + tick_position);
				} else {
					// find how many samples until next tick
					numsamples = (int)(samples_per_tick - tick_position);
					// if the tick was terminated prematurely, e.g via tempo change, sync to the next sample
					return (numsamples > 0) ? numsamples : 1;
				}
				break;
			case 1:
				// note: sub-tick intervals arent synced to the "outer tick"
				numsamples = (int)(samples_per_tick * interval_length / 256.0f);
				break;
			case 2:
				numsamples = (int)((float)samples_per_second / 16.0f * interval_length);
				break;
			default:
				assert(false);
				return std::numeric_limits<int>::max();
		}
		assert(numsamples > 0);
		return numsamples;
	}
};

struct value_plugin : zzub::plugin {

#pragma pack (push, 1)
	struct tvals {
		unsigned char op;
		unsigned short opvalue;
	};

	struct gvals {
		unsigned short value;
		unsigned short seed;
		unsigned char threshold;
		unsigned short allowmin;
		unsigned short allowmax;
	};

	struct cvals {
		unsigned short value;
	};
#pragma pack (pop)

	gvals gval;
	gvals glastval;
	tvals tval[128];
	tvals tlastval[128];
	cvals cval;

	int track_count;
	int triggercounter;
	MTRand mtrand;

	value_plugin() {
		global_values = &gval;
		track_values = tval;
		controller_values = &cval;
		track_count = 0;
		triggercounter = 0;
	}

	void set_track_count(int count) {
		track_count = count;
	}

	inline void apply_operator(int& value, const unsigned char& op, const unsigned short& opvalue) {

		switch (op) {
			case 0:
				value = value + opvalue;
				break;
			case 1:
				value = value - opvalue;
				break;
			case 2:
				value = value * opvalue;
				break;
			case 3:
				if (opvalue != 0)
					value = value / opvalue;
				else
					value = 0; // div0
				break;
			case 4:
				if (opvalue != 0)
					value = value % opvalue;
				else
					value = 0; // div0
				break;
			case 5:
				value = -value;
				break;
			case 6:
				value = mtrand.randInt() % 65534;
				break;
			case 7:
				value = (value * opvalue) / 65534;
				break;
			case 8:
				value = std::min(value, (int)opvalue);
				break;
			case 9:
				value = std::max(value, (int)opvalue);
				break;
		}
	}

	void update_value(bool trigger) {
		if (glastval.value < glastval.allowmin || glastval.value > glastval.allowmax)
			return ;

		if (trigger && triggercounter > 0) {
			triggercounter--;
			return ;
		}
		if (trigger)
			triggercounter = glastval.threshold;

		int value = glastval.value;
		for (int i = 0; i < track_count; i++) {
			apply_operator(value, tlastval[i].op, tlastval[i].opvalue);
		}
		cval.value = std::max(0, std::min(value, 65534));
	}

	void process_events() {
		bool changed = false;
		bool trigger = false;

		if (gval.value != 65535) {
			glastval.value = gval.value;
			changed = true;
			trigger = true;
		}

		if (gval.seed != 65535) {
			glastval.seed = gval.seed;
			mtrand.seed(gval.seed);
			changed = true;
		}

		if (gval.threshold != 255) {
			glastval.threshold = gval.threshold;
			triggercounter = glastval.threshold;
			changed = true;
		}

		if (gval.allowmin != 65535)
			glastval.allowmin = gval.allowmin;
		if (gval.allowmax != 65535)
			glastval.allowmax = gval.allowmax;

		for (int i = 0; i < track_count; i++) {
			unsigned short opvalue = 0;
			if (tval[i].op != 255) {
				tlastval[i].op = tval[i].op;
				changed = true;
			}
			if (tval[i].opvalue != 65535) {
				tlastval[i].opvalue = tval[i].opvalue;
				changed = true;
			}
		}

		if (changed)
			update_value(trigger);
	}

	const char* describe_value(int param, int value) {
		switch (param) {
			case 5: // track param 0
				switch (value) {
					case 0:
						return "Add";
					case 1:
						return "Subtract";
					case 2:
						return "Multiply";
					case 3:
						return "Divide";
					case 4:
						return "Modulus";
					case 5:
						return "Negate Source";
					case 6:
						return "Random Number";
					case 7:
						return "Scale";
					case 8:
						return "Min";
					case 9:
						return "Max";
				}
		}
		return 0;
	}
};

zzub::plugin* value_plugin_info::create_plugin() {
	return new value_plugin();
}

struct value_mapper_plugin : zzub::plugin {

#pragma pack (push, 1)

	struct tivals {
		unsigned short input_start;
		unsigned short input_end;
		unsigned short output_start;
		unsigned short output_end;
	};

	struct tvals {
		tivals value[2];
	};

	struct gvals {
		unsigned short value1;
		unsigned short value2;
	};

	struct cvals {
		unsigned short value1;
		unsigned short value2;
	};
#pragma pack (pop)

	gvals gval;
	gvals glastval;
	tvals tval[128];
	tvals tlastval[128];
	cvals cval;

	int track_count;

	value_mapper_plugin() {
		global_values = &gval;
		track_values = tval;
		controller_values = &cval;
		track_count = 0;
	}

	void set_track_count(int count) {
		track_count = count;
	}

	void update_value() {
		for (int i = 0; i < track_count; i++) {
			if ((glastval.value1 >= tlastval[i].value[0].input_start && glastval.value1 <= tlastval[i].value[0].input_end) &&
				(glastval.value2 >= tlastval[i].value[1].input_start && glastval.value2 <= tlastval[i].value[1].input_end)) {

				cval.value1 = tlastval[i].value[0].output_start ; // TODO interpolate!
				cval.value2 = tlastval[i].value[1].output_start ; // TODO interpolate!
				break;
			}
		}
	}

	void process_events() {
		bool changed = false;

		if (gval.value1 != 65535) {
			glastval.value1 = gval.value1;
			changed = true;
		}

		if (gval.value2 != 65535) {
			glastval.value2 = gval.value2;
			changed = true;
		}

		for (int i = 0; i < track_count; i++) {
			// value1:
			if (tval[i].value[0].input_start != 65535) {
				tlastval[i].value[0].input_start = tval[i].value[0].input_start;
				changed = true;
			}
			if (tval[i].value[0].input_end != 65535) {
				tlastval[i].value[0].input_end = tval[i].value[0].input_end;
				changed = true;
			}
			if (tval[i].value[0].output_start != 65535) {
				tlastval[i].value[0].output_start = tval[i].value[0].output_start;
				changed = true;
			}
			if (tval[i].value[0].output_end != 65535) {
				tlastval[i].value[0].output_end = tval[i].value[0].output_end;
				changed = true;
			}

			// value2:
			if (tval[i].value[1].input_start != 65535) {
				tlastval[i].value[1].input_start = tval[i].value[1].input_start;
				changed = true;
			}
			if (tval[i].value[1].input_end != 65535) {
				tlastval[i].value[1].input_end = tval[i].value[1].input_end;
				changed = true;
			}
			if (tval[i].value[1].output_start != 65535) {
				tlastval[i].value[1].output_start = tval[i].value[1].output_start;
				changed = true;
			}
			if (tval[i].value[1].output_end != 65535) {
				tlastval[i].value[1].output_end = tval[i].value[1].output_end;
				changed = true;
			}
		}

		if (changed)
			update_value();
	}

	const char* describe_value(int param, int value) {
		return 0;
	}
};

zzub::plugin* value_mapper_plugin_info::create_plugin() {
	return new value_mapper_plugin();
}

struct lfo_plugin : zzub::plugin {

#pragma pack (push, 1)
	struct gvals {
		unsigned char interval_type;
		unsigned char interval_length;
		unsigned char frequency;
		unsigned short amplitude;
		unsigned short minimum;
		unsigned char type;
		unsigned short seed;
	};

	struct cvals {
		unsigned short value;
	};
#pragma pack (pop)

	gvals gval;
	gvals glastval;
	cvals cval;

	bool pulse;
	int track_count;
	MTRand mtrand;

	lfo_plugin() {
		global_values = &gval;
		controller_values = &cval;
		track_count = 0;
		pulse = false;
	}

	void set_track_count(int count) {
		track_count = count;
	}

	float sgn(float f) {
		if (f < 0) return -1;
		if (f > 0) return 1;
		return 0;
	}

	void process_interval() {
		pulse = true;
	}

	int get_interval_size() {
		float samples_per_tick = (float)_master_info->samples_per_tick + _master_info->samples_per_tick_frac;
		float tick_position = (float)_master_info->tick_position + _master_info->tick_position_frac;
		return intervalhelper::schedule_interval(glastval.interval_type, glastval.interval_length, samples_per_tick, _master_info->samples_per_second, tick_position);
	}

	void update_value() {
		float time = ((float)_mixer->work_position / _master_info->samples_per_second);
		float phase = 2 * M_PI * time;

		float freq = (float)glastval.frequency / 32.0f;
		float amp = (float)glastval.amplitude / 65534.0f;
		float minval = (float)glastval.minimum / 65534.0f;
		float wave; // -1..1
		switch (glastval.type) {
			case 0:
				wave = sin(phase * freq);
				break;
			case 1:
				wave = sgn(sin(phase * freq));
				break;
			case 2:
				wave = (time * freq - floor(time * freq + 0.5f)) * 2.0f;
				break;
			case 3:
				wave = abs(time * freq - floor(time * freq + 0.5f)) * 4.0f - 1;
				break;
			case 4:
				wave = mtrand.randDblExc(2) - 1.0f;
				break;
		}
		float value = ((1.0f+wave)*0.5f) * amp + minval;
		value = std::min(std::max(value, 0.0f), 1.0f);

		cval.value = (unsigned short)(value * 65534.0f);
	}

	void process_events() {
		bool changed = false;

		if (gval.interval_type != 255) {
			glastval.interval_type = gval.interval_type;
			changed = true;
		}

		if (gval.interval_length != 255) {
			glastval.interval_length = gval.interval_length;
			changed = true;
		}

		if (gval.frequency != 0) {
			glastval.frequency = gval.frequency;
			changed = true;
		}

		if (gval.amplitude != 65535) {
			glastval.amplitude = gval.amplitude;
			changed = true;
		}

		if (gval.minimum != 65535) {
			glastval.minimum = gval.minimum;
			changed = true;
		}

		if (gval.type != 255) {
			glastval.type = gval.type;
			changed = true;
		}

		if (gval.seed != 65535) {
			mtrand.seed(gval.seed);
			glastval.seed = gval.seed;
			changed = true;
		}

		if (changed)
			pulse = true;
	}

	void process_controller_events() {
		if (pulse) {
			update_value();
			pulse = false;
		}
	}

	const char* describe_value(int param, int value) {
		static char desc[32];
		std::stringstream strm;
		switch (param) {
			case 0: // global param 0 - interval type
				switch (value) {
					case 0:
						return "Ticks";
					case 1:
						return "Ticks/256";
					case 2:
						return "Sec/16";
				}
			case 2: // global param 2 - frequency
				strm << std::setprecision(2) << std::fixed << (float)(value / 32.0f) << "hz";
				strcpy(desc, strm.str().c_str());
				return desc;
			case 5: // global param 5 - lfo type
				switch (value) {
					case 0:
						return "Sine";
					case 1:
						return "Square";
					case 2:
						return "Saw";
					case 3:
						return "Triangle";
					case 4:
						return "Random";
				}
		}
		return 0;
	}

};

zzub::plugin* lfo_plugin_info::create_plugin() {
	return new lfo_plugin();
}

struct signal_plugin : zzub::plugin {

#pragma pack (push, 1)
	struct gvals {
		unsigned char interval_type;
		unsigned char interval_length;
		unsigned char mode;
	};

	struct cvals {
		unsigned short value;
	};
#pragma pack (pop)

	gvals gval;
	gvals glastval;
	cvals cval;

	float maxsample;
	float immediate;
	bool pulse;

	signal_plugin() {
		global_values = &gval;
		controller_values = &cval;
		pulse = false;
		maxsample = 0;
	}

	void process_interval() {
		pulse = true;
	}

	int get_interval_size() {
		float samples_per_tick = (float)_master_info->samples_per_tick + _master_info->samples_per_tick_frac;
		float tick_position = (float)_master_info->tick_position + _master_info->tick_position_frac;
		return intervalhelper::schedule_interval(glastval.interval_type, glastval.interval_length, samples_per_tick, _master_info->samples_per_second, tick_position);
	}

	void process_events() {
		bool changed = false;

		if (gval.interval_type != 255) {
			glastval.interval_type = gval.interval_type;
			changed = true;
		}

		if (gval.interval_length != 255) {
			glastval.interval_length = gval.interval_length;
			changed = true;
		}

		if (gval.mode != 255) {
			glastval.mode = gval.mode;
			changed = true;
		}

		if (changed)
			pulse = true;
	}

	void process_controller_events() {
		if (pulse) {
			update_value();
			pulse = false;
		}
	}

	void update_value() {
		switch (glastval.mode) {
			case 0:
				cval.value = maxsample * 65534.0f;
				break;
			case 1:
				cval.value = (immediate / 2.0f + 0.5f) * 65534.0f;
				break;
			case 2:
				cval.value = abs(immediate) * 65534.0f;
				break;
		}
	}

	bool process_stereo(float** pin, float** pout, int numsamples, int mode) {
		// if mode == 0, average absolute samples to get the envelope
		// if mode == 1, take the last sample
		// if mode == 2, take absolute value of the last sample
		if (pin[0] == 0 || (mode & zzub_process_mode_read) == 0)
			return false;

		maxsample = 0;
		for (int i = 0; i < numsamples; i++) {
			maxsample = std::max(maxsample, std::abs(pin[0][i]));
		}
		immediate = pin[0][numsamples - 1];
		return false;
	}

	const char* describe_value(int param, int value) {
		static char desc[32];
		std::stringstream strm;
		switch (param) {
			case 0: // global param 0 - interval type
				switch (value) {
					case 0:
						return "Ticks";
					case 1:
						return "Ticks/256";
					case 2:
						return "Sec/16";
				}
			case 2: // global param 2 - mode
				switch (value) {
					case 0:
						return "Envelope";
					case 1:
						return "Immediate";
					case 2:
						return "Absolute";
				}
		}
		return 0;
	}

};

zzub::plugin* signal_value_plugin_info::create_plugin() {
	return new signal_plugin();
}

// adsr generator

struct adsr {
	float sps;
	float attack; // amp per sample
	float decay; // amp per sample
	float sustain; // 0-1
	float release; // amp per sample
	int sustime; // samplecount
	bool hold;
	int state;
	float a;
	int suswait;
	
	enum {
		state_off,
		state_attack,
		state_decay,
		state_sustain,
		state_release
	};
	
	adsr() {
		a = 0.0f;
		state = state_off;
	}
	
	void setup(float sps, float attack, float decay, float sustain, float release, float sustime) {
		float mintime = 0.00001f; // 0.01ms
		this->sps = sps;
		this->attack = 1.0f / (sps * std::max(attack,mintime));
		this->decay = (1.0f - sustain) / (sps * std::max(decay,mintime));
		this->sustain = sustain;
		this->release = sustain / (sps * std::max(release,mintime));
		this->sustime = int(sps * sustime + 0.5f);
	}
	
	void on() {
		a = 0.0f;
		state = state_attack;
	}
	
	void off() {
		suswait = 0;
	}
	
	inline float process() {
		switch(state) {
			case state_attack:
			{
				a += attack;
				if (a >= 1.0f) {
					a = 1.0f;
					state = state_decay;
				}
			} break;
			case state_decay:
			{
				a -= decay;
				if (a <= sustain) {
					a = sustain;
					state = state_sustain;
					suswait = sustime;
				}
			} break;
			case state_sustain:
			{
				a = sustain;
				if (suswait <= 0)
					state = state_release;
				else
					suswait--;
			} break;
			case state_release:
			{
				a -= release;
				if (a <= 0.0f) {
					a = 0.0f;
					state = state_off;
				}
			} break;
			default:
			{
				a = 0.0f;
			} break;
		}
		return a;
	}

}; 


struct adsr_plugin : zzub::plugin {

#pragma pack (push, 1)
	struct gvals {
		unsigned char interval_type;
		unsigned char interval_length;
		unsigned short attack;
		unsigned short decay;
		unsigned short sustain;
		unsigned short release;
		unsigned char trigger;
	};

	struct cvals {
		unsigned short value;
	};
#pragma pack (pop)

	gvals gval;
	gvals glastval;
	cvals cval;

	adsr gen;
	float adsrvalue;
	bool pulse;
	bool resync;

	adsr_plugin() {
		global_values = &gval;
		controller_values = &cval;
		pulse = false;
		resync = false;
		adsrvalue = 0.0f;
	}

	void process_interval() {
		pulse = true;
	}

	int get_interval_size() {
		float samples_per_tick = (float)_master_info->samples_per_tick + _master_info->samples_per_tick_frac;
		float tick_position = (float)_master_info->tick_position + _master_info->tick_position_frac;
		return intervalhelper::schedule_interval(glastval.interval_type, glastval.interval_length, samples_per_tick, _master_info->samples_per_second, tick_position);
	}

	void process_events() {
		bool changed = false;

		if (gval.interval_type != 255) {
			glastval.interval_type = gval.interval_type;
			changed = true;
		}

		if (gval.interval_length != 255) {
			glastval.interval_length = gval.interval_length;
			changed = true;
		}

		if (gval.attack != 65535) {
			glastval.attack = gval.attack;
			changed = true;
		}

		if (gval.decay != 65535) {
			glastval.decay = gval.decay;
			changed = true;
		}

		if (gval.sustain != 65535) {
			glastval.sustain = gval.sustain;
			changed = true;
		}

		if (gval.release != 65535) {
			glastval.release = gval.release;
			changed = true;
		}

		if (changed) {
			float attack = glastval.attack / 65534.0f;
			float decay = glastval.decay / 65534.0f;
			float sustain = glastval.sustain / 65534.0f;
			float release = glastval.release / 65534.0f;
			gen.setup(_master_info->samples_per_second, attack, decay, sustain, release, 9999.0f);
			pulse = true;
		}

		if (gval.trigger != 255) {
			glastval.trigger = gval.trigger;
			pulse = true;
			switch (gval.trigger) {
				case 0:
					gen.off();
					break;
				case 1:
					gen.on();
					break;
			}
		}
	}

	void process_controller_events() {
		if (pulse) {
			update_value();
			pulse = false;
		}
	}

	void update_value() {
		cval.value = adsrvalue * 65534.0f;
	}

	bool process_stereo(float** pin, float** pout, int numsamples, int mode) {
		for (int i = 0; i < numsamples; i++) {
			adsrvalue = gen.process();
		}
		return false;
	}

	const char* describe_value(int param, int value) {
		static char desc[32];
		std::stringstream strm;
		switch (param) {
			case 0: // global param 0 - interval type
				switch (value) {
					case 0:
						return "Ticks";
					case 1:
						return "Ticks/256";
					case 2:
						return "Sec/16";
				}
				break;
			case 2: // global param 2 - attack
			case 3: // global param 3 - decay
			case 5: // global param 5 - release
				strm << std::setprecision(2) << std::fixed << (float)(value / 65534.0f) << "sec";
				strcpy(desc, strm.str().c_str());
				return desc;
				break;
			case 4: // global param 4 - sustain
				strm << std::setprecision(2) << std::fixed << (float)(value / 655.34f) << "%";
				strcpy(desc, strm.str().c_str());
				return desc;
				break;
		}
		return 0;
	}

};

zzub::plugin* adsr_value_plugin_info::create_plugin() {
	return new adsr_plugin();
}

#if defined(_WIN32)

// joystick code was ripped from:
// http://www.cs.cmu.edu/~jparise/directx/joystick/

struct joystick_plugin : zzub::plugin {

#pragma pack (push, 1)
	struct gvals {
		unsigned char interval_type;
		unsigned char interval_length;
	};

	struct cvals {
		unsigned short joy_x;
		unsigned short joy_y;
		unsigned char button0;
		unsigned char button1;
	};
#pragma pack (pop)

	joystick_value_plugin_info* info;
	LPDIRECTINPUTDEVICE8 joystick;

	gvals gval;
	gvals glastval;
	cvals cval;

	bool pulse;

	joystick_plugin(joystick_value_plugin_info* _info) {
		info = _info;
		global_values = &gval;
		controller_values = &cval;
		pulse = false;
		joystick = 0;
	}

	void init(zzub::archive* inf) {
		info->init_joysticks(); // TODO: add ref count

		// TODO: read guid from inf
		if (info->joystickinfos.size() > 0)
			joystick = info->create_joystick(info->joystickinfos[0].id);
		else
			joystick = 0;
	}

	void save(zzub::archive* outf) {
		// TODO: save guid
	}

	void destroy() {
		// info->uninit_joysticks(); // TODO: subtract ref count
		plugin::destroy();
	}

	void process_interval() {
		pulse = true;
	}

	int get_interval_size() {
		float samples_per_tick = (float)_master_info->samples_per_tick + _master_info->samples_per_tick_frac;
		float tick_position = (float)_master_info->tick_position + _master_info->tick_position_frac;
		return intervalhelper::schedule_interval(glastval.interval_type, glastval.interval_length, samples_per_tick, _master_info->samples_per_second, tick_position);
	}

	void process_events() {
		bool changed = false;

		if (gval.interval_type != 255) {
			glastval.interval_type = gval.interval_type;
			changed = true;
		}

		if (gval.interval_length != 255) {
			glastval.interval_length = gval.interval_length;
			changed = true;
		}

		if (changed) {
			pulse = true;
		}
	}

	void process_controller_events() {
		if (pulse) {
			update_value();
			pulse = false;
		}
	}

	void update_value() {

		if (!joystick) return ;

		DIJOYSTATE2 state;
		if SUCCEEDED(info->poll(joystick, &state)) {
			cval.joy_x = std::min(state.lX, 65534L);
			cval.joy_y = std::min(state.lY, 65534L);
			cval.button0 = (state.rgbButtons[0] & 0x80) != 0 ? 1 : 0;
			cval.button1 = (state.rgbButtons[1] & 0x80) != 0 ? 1 : 0;
		}
	}

	bool process_stereo(float** pin, float** pout, int numsamples, int mode) {
		return false;
	}

	const char* describe_value(int param, int value) {
		static char desc[32];
		std::stringstream strm;
		switch (param) {
			case 0: // global param 0 - interval type
				switch (value) {
			case 0:
				return "Ticks";
			case 1:
				return "Ticks/256";
			case 2:
				return "Sec/16";
				}
				break;
		}
		return 0;
	}

};



BOOL CALLBACK enumCallback(const DIDEVICEINSTANCE* instance, VOID* context) {
	joystick_value_plugin_info* joyinfo = (joystick_value_plugin_info*)context;
	joystickinfo info;
	info.id = instance->guidInstance;
	info.name = instance->tszProductName;
	joyinfo->joystickinfos.push_back(info);
	return DIENUM_CONTINUE; //DIENUM_STOP;
}

bool joystick_value_plugin_info::init_joysticks() {

	if (di != 0) return true;

	HRESULT hr;

	// Create a DirectInput device
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&di, NULL))) {
		return false;
	}

	// Look for the first simple joystick we can find.
	if (FAILED(hr = di->EnumDevices(DI8DEVCLASS_GAMECTRL, enumCallback, this, DIEDFL_ATTACHEDONLY))) {
		return false;
	}

	return true;
}

LPDIRECTINPUTDEVICE8 joystick_value_plugin_info::create_joystick(GUID id) {
	LPDIRECTINPUTDEVICE8 joystick;

	HRESULT hr = di->CreateDevice(id, &joystick, 0);
	if FAILED(hr) return 0;


	if (FAILED(hr = joystick->SetDataFormat(&c_dfDIJoystick2))) {
		return 0;
	}
	// Set the cooperative level to let DInput know how this device should
	// interact with the system and with other DInput applications.
	//if (FAILED(hr = joystick->SetCooperativeLevel(NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND))) {
	if (FAILED(hr = joystick->SetCooperativeLevel(NULL, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND))) {
		return 0;
	}


	return joystick;
}

HRESULT joystick_value_plugin_info::poll(LPDIRECTINPUTDEVICE8 joystick, DIJOYSTATE2 *js) {
	HRESULT     hr;

	if (joystick == NULL) {
		return S_OK;
	}

	// Poll the device to read the current state
	hr = joystick->Poll(); 
	if (FAILED(hr)) {
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = joystick->Acquire();
		while (hr == DIERR_INPUTLOST) {
			hr = joystick->Acquire();
		}

		// If we encounter a fatal error, return failure.
		if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED)) {
			return E_FAIL;
		}

		// If another application has control of this device, return successfully.
		// We'll just have to wait our turn to use the joystick.
		if (hr == DIERR_OTHERAPPHASPRIO) {
			return S_OK;
		}
	}

	// Get the input's device state
	if (FAILED(hr = joystick->GetDeviceState(sizeof(DIJOYSTATE2), js))) {
		return hr; // The device should have been acquired during the Poll()
	}

	return S_OK;
}

zzub::plugin* joystick_value_plugin_info::create_plugin() {
	return new joystick_plugin(this);
}

#endif

struct midinotevel_value_plugin : zzub::plugin {

#pragma pack (push, 1)
	struct gvals {
		unsigned char note;
		unsigned char channel;
		unsigned char mapnoteoff;
	};

	struct cvals {
		unsigned char value;
	};
#pragma pack (pop)

	gvals gval;
	gvals glastval;
	//tvals tval[128];
	//tvals tlastval[128];
	cvals cval;

	int queue[1024];
	int queueplaypos;
	int queuepos;
	
	int note;
	int channel;
	bool mapnoteoff;

	midinotevel_value_plugin() {
		global_values = &gval;
		controller_values = &cval;
		note = 48;
		channel = 1;
		mapnoteoff = false;
		
		queueplaypos = 0;
		queuepos = 0;
	}

	int get_interval_size() {
		if (queuepos != queueplaypos) {
			return 8;
		}

		return 10000; // XXX better value?
	}

	/*void get_midi_output_names(zzub::outstream *pout) {
		std::string name = "Note input";
		pout->write((void*)name.c_str(), (unsigned int)(name.length()) + 1);
	}*/

	void midi_event(unsigned short status, unsigned char data1, unsigned char data2) {
		int channel = status&0xf;
		int command = (status & 0xf0) >> 4;
		int velocity;

		if (this->channel != channel + 1) {
			return;
		}

		switch (command) {
			case 0x8:
				if (data1 != note) {
					break;
				}
				if (mapnoteoff) {
					queue[queuepos] = 0;
					queuepos++;
				}
				break;
			case 0x9:
				if (data1 != note) {
					break;
				}
				velocity = data2;
				if (velocity == 0 && !mapnoteoff) {
					break;
				}
				queue[queuepos] = velocity;
				queuepos++;
				break;
			/*case 0xb:
				if (machine2)
					machine2->MidiControlChange(data1, channel, data2);
				break;*/
		}
	}

	/*void process_midi_events(zzub::midi_message* pin, int nummessages) {
		while (nummessages--) {
			unsigned short status = pin->message & 0xff;
			int channel = status&0xf;
			int command = (status & 0xf0) >> 4;
			unsigned char data1 = (pin->message >> 8) & 0xff;
			unsigned char data2 = (pin->message >> 16) & 0xff;
			int velocity;
			switch (command) {
				case 0x8:
					if (mapnoteoff) {
						queue[queuepos] = 0;
						queuepos++;
					}
					break;
				case 0x9:
					velocity = data2;
					queue[queuepos] = velocity;
					queuepos++;
					break;
				//case 0xb:
				//	if (machine2)
				//		machine2->MidiControlChange(data1, channel, data2);
				//	break;
			}

			pin++;
		}
	}*/

	void process_events() {
		if (gval.note != 255) {
			note = gval.note;
		}

		if (gval.channel != 255) {
			channel = gval.channel;
		}

		if (gval.mapnoteoff != 255) {
			mapnoteoff = (gval.mapnoteoff != 0);
		}
	}

	void process_controller_events() {
		if (queuepos != queueplaypos) {
			cval.value = queue[queueplaypos];
			queueplaypos++;

			if (queueplaypos == queuepos) {
				queuepos = queueplaypos = 0;
			}
		}
	}

	const char* describe_value(int param, int value) {
		static char buf[30];
		const char* notes[] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };
		int n, oct;
		switch (param) {
			case 0: // global param 0
				n = value % 12;
				oct = value / 12;
				sprintf(buf, "%s%d", notes[n], oct);
				return buf;

			default:
				break;
		}
		return 0;
	}
};

zzub::plugin* midinotevel_value_plugin_info::create_plugin() {
	return new midinotevel_value_plugin();
}

struct midictrl_value_plugin : zzub::plugin {

	// midiccstream = add single midi cc's, retreive 14-bit cc changes
	struct midiccstream {
		struct ccvalue {
			int time;
			int ctrl;
			int value;
			bool wide;
		};
		ccvalue ccmap[127];
		std::vector<ccvalue> output;
		int curtime;
		
		midiccstream() {
			curtime = 0;
			for (int i = 0; i < 127; i++) {
				ccvalue& ccv = ccmap[i];
				ccv.ctrl = i;
				ccv.time = -1;
			}
		}

		void set_time(int time) {
			curtime = time;
		}

		void add_cc(int ctrl, int value) {
			assert(ctrl >= 0 && ctrl <= 127);
			assert(value >= 0 && value <= 127);
			if (ctrl < 32)
				add_lo_cc(ctrl, value);
			else if (ctrl < 64)
				add_hi_cc(ctrl, value);

			// always add byte cc output
			add_raw_cc(ctrl, value, false);
		}

		void add_lo_cc(int ctrl, int value) {
			ccvalue& ccv0 = ccmap[ctrl];
			ccvalue& ccv1 = ccmap[ctrl + 32];

			ccv0.time = curtime;
			ccv0.value = value;

			// added hi, lo:
			if (ccv1.time == curtime) {
				add_raw_cc(ctrl, (ccv0.value << 7) | ccv1.value, true);
				ccv0.time = -1;
				ccv1.time = -1;
			}

		}

		void add_hi_cc(int ctrl, int value) {
			ccvalue& ccv0 = ccmap[ctrl - 32];
			ccvalue& ccv1 = ccmap[ctrl];

			ccv1.time = curtime;
			ccv1.value = value;
			
			// added lo, hi:
			if (ccv0.time == curtime) {
				add_raw_cc(ctrl, (ccv0.value << 7) | ccv1.value, true);
				ccv0.time = -1;
				ccv1.time = -1;
			}
		}

		void add_raw_cc(int ctrl, int value, bool wide) {
			ccvalue ccv;
			ccv.time = curtime;
			ccv.ctrl = ctrl;
			ccv.value = value;
			ccv.wide = wide;
			output.push_back(ccv);
		}

		void flush() {
			for (int i = 0; i < 127; i++) {
				ccvalue& ccv = ccmap[i];
				if (ccv.time != -1) {
					ccv.wide = false;
					output.push_back(ccv);
					ccv.time = -1;
				}
			}
		}
		
		int get_events() {
			return output.size();
		}

		ccvalue* get_event(int index) {
			return &output[index];
		}
	};

#pragma pack (push, 1)
	struct gvals {
		unsigned char channel;
	};

	struct cvals {
		unsigned short channelaftertouch;
		unsigned short pitchwheel;
		unsigned short ccword[32];
		unsigned char ccbyte[128];
	};
#pragma pack (pop)

	gvals gval;
	cvals cval;
	
	int channel;
	int channelaftertouch;
	int pitchwheel;
	midiccstream midicc;

	midictrl_value_plugin() {
		global_values = &gval;
		controller_values = &cval;
		channel = 1;
		channelaftertouch = -1;
		pitchwheel = -1;
	}

	void midi_event(unsigned short status, unsigned char data1, unsigned char data2) {
		int channel = status&0xf;
		int command = (status & 0xf0) >> 4;

		if (this->channel != channel + 1) {
			return;
		}

		switch (command) {
			case 0xd:
				// channel aftertouch
				channelaftertouch = data1 | (((int)data2) << 7);
				break;
			case 0xe:
				// pitch wheel
				pitchwheel = data1 | (((int)data2) << 7);
				break;
			case 0xb:
				midicc.add_cc(data1, data2);
				break;
		}
	}

	void process_events() {
		if (gval.channel != 255) {
			channel = gval.channel;
		}
	}

	void process_controller_events() {
		if (pitchwheel != -1) {
			cval.pitchwheel = pitchwheel;
			pitchwheel = -1;
		}
		if (channelaftertouch != -1) {
			cval.channelaftertouch = channelaftertouch;
			channelaftertouch = -1;
		}

		midicc.flush();
		for (int i = 0; i < midicc.get_events(); i++) {
			midiccstream::ccvalue* ccv = midicc.get_event(i);
			if (ccv->wide)
				cval.ccword[ccv->ctrl] = ccv->value;
			else 
				cval.ccbyte[ccv->ctrl] = ccv->value;
		}
		midicc.output.clear();
	}

	void get_midi_output_names(zzub::outstream *pout) {
		std::string name = "MIDI Controller Input";
		pout->write((void*)name.c_str(), (unsigned int)(name.length()) + 1);
	}

	const char* describe_value(int param, int value) {
		static char buf[30];
		return 0;
	}
};

zzub::plugin* midictrl_value_plugin_info::create_plugin() {
	return new midictrl_value_plugin();
}
