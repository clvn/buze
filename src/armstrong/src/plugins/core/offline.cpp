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
#include "mixing/mixer.h"
#include "offline.h"


struct fadeplugin : zzub::plugin {

#pragma pack (push, 1)
	struct gval {
		unsigned short minamp;
		unsigned short maxamp;
	};
#pragma pack (pop)

	gval gvals;
	gval curval;

	fadeplugin() {
		global_values = &gvals;
		track_values = 0;
	}

	virtual void process_events() {
		if (gvals.minamp != 65535) {
			curval.minamp = gvals.minamp;
		}

		if (gvals.maxamp != 65535) {
			curval.maxamp = gvals.maxamp;
		}
	}

	virtual bool process_offline(float **pin, float **pout, std::vector<int>& slices, int *numsamples, int *channels, int *samplerate, int beginloop, int endloop) { 
		if (!pin && !pout) {
			// numsamples = numsamples
			return true;
		}

		float startamp = curval.minamp / 1000.0f;
		float endamp = curval.maxamp / 1000.0f;
		float ampdelta = (endamp - startamp) / *numsamples;

		for (int i = 0; i < *numsamples; i++) {
			for (int j = 0; j < *channels; j++) {
				pout[j][i] = pin[j][i] * (startamp + i * ampdelta);
			}
		}

		return true; 
	}


};

zzub::plugin* fade_plugin_info::create_plugin() {
	return new fadeplugin();
}




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
#include "mixing/mixer.h"
#include "offline.h"

	// the end of the loop is crossfaded with the section leading up to beginloop. the output can be looped seamlessly
struct loopxfadeplugin : zzub::plugin {

	loopxfadeplugin() {
		global_values = 0;
		track_values = 0;
	}

	virtual bool process_offline(float **pin, float **pout, std::vector<int>& slices, int *numsamples, int *channels, int *samplerate, int beginloop, int endloop) { 
		if (!pin && !pout) {
			return true;
		}

		if (beginloop == -1 || endloop == -1) {
			return false;
		}

		int fadelen = beginloop;
		int looplen = endloop - beginloop;

		if (fadelen > looplen) {
			// xfading section must be shorter than the looping section
			return false;
		}

		for (int i = 0; i < *numsamples; i++) {

			if (i >= endloop - fadelen && i <= endloop) {
				// fade out pin[i], fade in pin[fadeindex]
				int fadeindex = i - (endloop - fadelen);
				float fadeunit = (float)fadeindex / (float)fadelen;

				for (int j = 0; j < *channels; j++) {
					pout[j][i] = 
						pin[j][i] * (1 - fadeunit) + 
						pin[j][fadeindex] * fadeunit;
				}
			} else {
				// copy channel
				for (int j = 0; j < *channels; j++) {
					pout[j][i] = pin[j][i];
				}
			}
		}

		return true; 
	}


};

zzub::plugin* loopxfade_plugin_info::create_plugin() {
	return new loopxfadeplugin();
}




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
#include "mixing/mixer.h"
#include "offline.h"


struct reverseplugin : zzub::plugin {

#pragma pack (push, 1)
	struct gval {
	};
#pragma pack (pop)

	gval gvals;
	gval curval;

	reverseplugin() {
		global_values = &gvals;
		track_values = 0;
	}

	virtual void process_events() { }

	virtual bool process_offline(float **pin, float **pout, std::vector<int>& slices, int *numsamples, int *channels, int *samplerate, int beginloop, int endloop) { 
		if (!pin && !pout) {
			// numsamples = numsamples
			return true;
		}

		for (int i = 0; i < *numsamples; i++) {
			for (int j = 0; j < *channels; j++) {
				pout[j][i] = pin[j][*numsamples - i - 1];
			}
		}

		return true; 
	}


};

zzub::plugin* reverse_plugin_info::create_plugin() {
	return new reverseplugin();
}

#if defined(_WIN32)

// TODO: consider moving into separate pluginlib with other plugins based on soundtouch, e.g a realtime pitchshifter
#include <SoundTouchDLL.h>

// Available setting IDs for the 'setSetting' & 'get_setting' functions:

/// Enable/disable anti-alias filter in pitch transposer (0 = disable)
#define SETTING_USE_AA_FILTER       0

/// Pitch transposer anti-alias filter length (8 .. 128 taps, default = 32)
#define SETTING_AA_FILTER_LENGTH    1

/// Enable/disable quick seeking algorithm in tempo changer routine
/// (enabling quick seeking lowers CPU utilization but causes a minor sound
///  quality compromising)
#define SETTING_USE_QUICKSEEK       2

/// Time-stretch algorithm single processing sequence length in milliseconds. This determines 
/// to how long sequences the original sound is chopped in the time-stretch algorithm. 
/// See "STTypes.h" or README for more information.
#define SETTING_SEQUENCE_MS         3

/// Time-stretch algorithm seeking window length in milliseconds for algorithm that finds the 
/// best possible overlapping location. This determines from how wide window the algorithm 
/// may look for an optimal joining location when mixing the sound sequences back together. 
/// See "STTypes.h" or README for more information.
#define SETTING_SEEKWINDOW_MS       4

/// Time-stretch algorithm overlap length in milliseconds. When the chopped sound sequences 
/// are mixed back together, to form a continuous sound stream, this parameter defines over 
/// how long period the two consecutive sequences are let to overlap each other. 
/// See "STTypes.h" or README for more information.
#define SETTING_OVERLAP_MS          5

namespace {
void s2i(float *i, float **s, int numsamples) {
	float *p[] = {s[0],s[1]};
	if (p[0] && !p[1]) {
		while (numsamples--)
		{
			*i++ = *p[0]++;
			*i++ = 0;
		}
	} else
	if (!p[0] && p[1]) {
		while (numsamples--)
		{
			*i++ = 0;
			*i++ = *p[1]++;
		}
	} else
	if (p[0] && p[1]) {
		while (numsamples--)
		{
			*i++ = *p[0]++;
			*i++ = *p[1]++;
		}
	}
}

void i2s(float **s, float *i, int numsamples) {
	if (!numsamples)
		return;
	float *p[] = {s[0],s[1]};
	if (p[0] && !p[1]) {
		while (numsamples--) {
			*p[0]++ = *i++;
			i++;
		}
	} else
	if (!p[0] && p[1]) {
		while (numsamples--) {
			i++;
			*p[1]++ = *i++;
		}
	} else
	if (p[0] && p[1]) {
		while (numsamples--) {
			*p[0]++ = *i++;
			*p[1]++ = *i++;
		}
	}
}

}
struct soundtouchplugin : zzub::plugin {

#pragma pack (push, 1)
	struct gval {
		unsigned short stretch;
		unsigned char pitch;
		unsigned short pitchfine;
		unsigned char sequence;
		unsigned char seek;
		unsigned char overlap;
		unsigned char quickmode;
	};
#pragma pack (pop)

	gval gvals;
	gval curval;

	soundtouchplugin() {
		global_values = &gvals;
		track_values = 0;
	}

	virtual void process_events() {
		if (gvals.stretch != 65535) {
			curval.stretch = gvals.stretch;
		}
		if (gvals.pitch != 255) {
			curval.pitch = gvals.pitch;
		}
		if (gvals.pitchfine != 65535) {
			curval.pitchfine = gvals.pitchfine;
		}
		if (gvals.sequence != 255) {
			curval.sequence = gvals.sequence;
		}
		if (gvals.seek != 255) {
			curval.seek = gvals.seek;
		}
		if (gvals.overlap != 255) {
			curval.overlap = gvals.overlap;
		}
		if (gvals.quickmode != 255) {
			curval.quickmode = gvals.quickmode;
		}
	}

	virtual bool process_offline(float **pin, float **pout, std::vector<int>& slices, int *numsamples, int *channels, int *samplerate, int beginloop, int endloop) { 
		float tempodelta = ((float)curval.stretch - 950.0f) / 10.0f;
		float stretchfactor = 1.0f / ((tempodelta + 100.0f) / 100.0f);
		int expected_samples = (int)((float)*numsamples * stretchfactor);

		if (!pin && !pout) {
			*numsamples = expected_samples;
			return true;
		}

		float pitchfine = ((float)curval.pitchfine - 950.0f) / 1000.0f;
		float pitch = (int)curval.pitch - 60 + pitchfine;

		HANDLE hTouch = soundtouch_createInstance();
		soundtouch_setSampleRate(hTouch, *samplerate);
		soundtouch_setChannels(hTouch, *channels);
		soundtouch_setTempoChange(hTouch, tempodelta);
		soundtouch_setPitchSemiTones(hTouch, pitch);
		soundtouch_setRateChange(hTouch, 0);

		if (curval.sequence != 0) 
			soundtouch_setSetting(hTouch, SETTING_SEQUENCE_MS, curval.sequence);

		if (curval.seek != 0) 
			soundtouch_setSetting(hTouch, SETTING_SEEKWINDOW_MS, curval.seek);

		if (curval.overlap != 0) 
			soundtouch_setSetting(hTouch, SETTING_OVERLAP_MS, curval.overlap);

		if (curval.quickmode != 0)
			soundtouch_setSetting(hTouch, SETTING_USE_QUICKSEEK, 1);

	    soundtouch_setSetting(hTouch, SETTING_USE_AA_FILTER, 1);

		const int chunksize = 2048;
		float ibuf[chunksize * 2];
		int inoffset = 0;
		int outoffset = 0;
		int touchsamples;

		while (inoffset <  *numsamples) {
			int chunksamples = read_samples(ibuf, inoffset, pin, chunksize, *channels, *numsamples);
			inoffset += chunksamples;
			soundtouch_putSamples(hTouch, ibuf, chunksamples);

			do {
				touchsamples = soundtouch_receiveSamples(hTouch, ibuf, chunksize);
				int copysamples = write_samples(pout, outoffset, ibuf, touchsamples, *channels, expected_samples);
				outoffset += copysamples;
			} while (touchsamples != 0);
		}

		soundtouch_flush(hTouch);

		do {
			touchsamples = soundtouch_receiveSamples(hTouch, ibuf, chunksize);
			int copysamples = write_samples(pout, outoffset, ibuf, touchsamples, *channels, expected_samples);
			outoffset += copysamples;
		} while (touchsamples != 0);

		// zero remainder
		memset(pout[0] + outoffset, 0, (expected_samples - outoffset) * sizeof(float));
		if (*channels == 2)
			memset(pout[1] + outoffset, 0, (expected_samples - outoffset) * sizeof(float));

		soundtouch_clear(hTouch);
		soundtouch_destroyInstance(hTouch);
		return true; 
	}

	int read_samples(float* outbuffer, int offset, float** insamples, int chunksize, int channels, int maxsamples) {
		int chunksamples = std::min(chunksize, maxsamples - offset);
		if (channels == 2) {
			float* pbuf[] = {insamples[0] + offset, insamples[1] + offset };
			s2i(outbuffer, pbuf, chunksamples);
		} else
			memcpy(outbuffer, insamples[0] + offset, chunksamples * sizeof(float));
		return chunksamples;
	}

	int write_samples(float** outbuffer, int offset, float* insamples, int numsamples, int channels, int maxsamples) {
		int copysamples = std::min(numsamples, maxsamples - offset);
		if (channels == 2) {
			float* pbuf[] = {outbuffer[0] + offset, outbuffer[1] + offset };
			i2s(pbuf, insamples, copysamples);
		} else {
			memcpy(outbuffer[0] + offset, insamples, copysamples * sizeof(float));
		}
		return copysamples;
	}

	const char* describe_value(int param, int value) {
		static char desc[32];
		std::stringstream strm;
		switch (param) {
			case 0: {// global param 0 - tempo
				float tempodelta = ((float)value - 950.0f) / 10.0f;
				std::stringstream strm;
				strm << std::fixed << std::setprecision(1) << tempodelta << "%";
				strcpy(desc, strm.str().c_str());
				return desc;
			}
			case 1: { // global param 1 - pitch
				int pitch = value - 60;
				std::stringstream strm;
				strm << pitch << " seminotes";
				strcpy(desc, strm.str().c_str());
				return desc;
			}
			case 2: { // global param 2 - finetune pitch
				float ratedelta = ((float)value - 950.0f) / 10.0f;
				std::stringstream strm;
				strm << ratedelta << " percents";
				strcpy(desc, strm.str().c_str());
				return desc;
			}
			case 3: // sequence
			case 4: // seek window
			case 5: { // overlap
				if (value == 0) return "Autodetect";
				std::stringstream strm;
				strm << value << " ms";
				strcpy(desc, strm.str().c_str());
				return desc;
			}
		}
		return 0;
	}

};

zzub::plugin* soundtouch_plugin_info::create_plugin() {
	return new soundtouchplugin();
}

#endif
