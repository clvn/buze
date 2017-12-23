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
#include "group.h"


struct group_input_plugin : zzub::plugin {

	group_input_plugin() { }
	virtual void init(zzub::archive *arc) {}
	virtual void process_events() {}
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode) {
		if (mode == zzub_process_mode_write || mode == zzub_process_mode_no_io) {
			pout[0] = 0;
			pout[1] = 0;
			//if (pout[0]) memset(pout[0], 0, numsamples * sizeof(float));
			//if (pout[1]) memset(pout[1], 0, numsamples * sizeof(float));
			return false;
		}

		if (pout[0] && pin[0]) memcpy(pout[0], pin[0], numsamples * sizeof(float));
		if (pout[1] && pin[1]) memcpy(pout[1], pin[1], numsamples * sizeof(float));
		return pout[0] && pout[1];
	}
};

zzub::plugin* group_input_plugin_info::create_plugin() { 
	return new group_input_plugin();
}




struct group_output_plugin : zzub::plugin {

	group_output_plugin() { }
	virtual void init(zzub::archive *arc) {}
	virtual void process_events() {}
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode) {
		if (mode == zzub_process_mode_write || mode == zzub_process_mode_no_io) {
			pout[0] = 0;
			pout[1] = 0;
			//memset(pout[0], 0, numsamples * sizeof(float));
			//memset(pout[1], 0, numsamples * sizeof(float));
			return false;
		}

		if (pout[0] && pin[0]) memcpy(pout[0], pin[0], numsamples * sizeof(float));
		if (pout[1] && pin[1]) memcpy(pout[1], pin[1], numsamples * sizeof(float));
		return pout[0] && pout[1];
	}
};

zzub::plugin* group_output_plugin_info::create_plugin() { 
	return new group_output_plugin();
}

