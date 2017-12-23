#include <complex>
#include <vector>
#include "audiodriver.h"

	
// sanitize buffersize and samplerate defaults
void deviceinfo::get_default_deviceinfo(int* buffersize, int *samplerate){
	deviceinfo* info = this;

	int nearest_rate = info->defaultsamplerate;
	int nearest_ratediff = std::abs(*samplerate - info->defaultsamplerate);
	for (std::vector<int>::iterator i = info->samplerates.begin(); i != info->samplerates.end(); ++i) {
		int ratediff = std::abs(*samplerate - *i);
		if (ratediff < nearest_ratediff) {
			nearest_rate = *i;
			nearest_ratediff = ratediff;
		}
	}

	*samplerate = nearest_rate;

	int nearest_size = info->defaultbuffersize;
	int nearest_sizediff = std::abs(*buffersize -  info->defaultbuffersize); //std::numeric_limits<int>::max();
	for (std::vector<int>::iterator i = info->buffersizes.begin(); i != info->buffersizes.end(); ++i) {
		int sizediff = std::abs(*buffersize - *i);
		if (sizediff < nearest_sizediff) {
			nearest_size = *i;
			nearest_sizediff = sizediff;
		}
	}

	*buffersize = nearest_size;

}
