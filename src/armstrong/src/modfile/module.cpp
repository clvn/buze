#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "module.h"
#include "xm.h"
#include "s3m.h"
#include "mod.h"
#include "it.h"
#include "drumkitimport.h"
#include "module_xm.h"
#include "module_s3m.h"
#include "module_mod.h"
#include "module_it.h"
#include "module_sndfile.h"
#include "module_mad.h"
#include "module_drumkit.h"

using namespace std;

namespace modimport {

char transform_tolower(char c) {
	return tolower(c);
}

module* module::create(std::string fileName) {
	unsigned ld = fileName.find_last_of('.');
	if (ld == -1) return 0;

	string ext = fileName.substr(ld + 1);
	transform(ext.begin(), ext.end(), ext.begin(), transform_tolower);

	module* archive = 0;
#if !defined(_WIN64)
	if (ext == "xm") archive = new xm::module_xm(); else
#endif
	if (ext == "s3m") archive = new s3m::module_s3m(); else
	if (ext == "mod") archive = new mod::module_mod(); else
	if (ext == "it") archive = new it::module_it(); else
	if (ext == "mp3") archive = new mad::module_mad(); else
	if (ext == "wav" || ext == "aif" || ext == "aifc" || ext == "aiff") archive = new sndfile::module_sndfile(); else
	if (ext == "drumkit") archive = new drumkit::module_drumkit(); else
		return 0;

	if (!archive->open(fileName)) {
		delete archive;
		return 0;
	}

	return archive;
}

unsigned long module::sample_bytes(int instrument, int sample) {
	int channels = sample_stereo(instrument, sample) ? 2 : 1;
	return sample_samples(instrument, sample) * sample_bytes_per_sample(instrument, sample) * channels;
}

int module::sample_bytes_per_sample(int instrument, int sample) {
	return sample_bits_per_sample(instrument, sample) / 8;
}

}
