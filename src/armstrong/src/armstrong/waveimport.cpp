/*
Copyright (C) 2003-2014 Anders Ervik <calvin@countzero.no>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <functional>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include <vector>
#include <cassert>
#include <cstring>
#include "library.h"
#include "zzub/zzub.h"
#include "waveimport.h"
#include "mixing/convertsample.h"

#if defined(POSIX)
#include <dirent.h>
#include <sys/stat.h>
#endif

#if defined(POSIX)
#define _strcmpi strcasecmp
#define strcmpi strcasecmp
#endif

using namespace std;

namespace zzub {

/***

	waveimporter

***/

import_modfile::import_modfile() {
	modf = 0;
}

bool import_modfile::open(std::string filename) {
	modf = modimport::module::create(filename);
	return modf != 0;
}

int import_modfile::get_wave_count() {
	return modf->instrument_count();
}

std::string import_modfile::get_wave_name(int i) {
	return modf->instrument_name(i);
}

int import_modfile::get_wave_level_count(int i) {
	return modf->sample_count(i);
}

bool import_modfile::get_wave_level_info(int i, int level, importwave_info& info) {
	info.sample_count = modf->sample_samples(i, level);
	info.samples_per_second = modf->sample_samplespersecond(i, level);
	info.channels = modf->sample_stereo(i, level) ? 2 : 1;
	info.name = modf->sample_name(i, level);
	int bits = modf->sample_bits_per_sample(i, level);

	switch (bits) {
		case 8:
		case 16:
			info.format = zzub_wave_buffer_type_si16;
			break;
		case 24:
			info.format = zzub_wave_buffer_type_si24;
			break;
		case 32:
			if (modf->sample_float(i, level))
				info.format = zzub_wave_buffer_type_f32;
			else
				info.format = zzub_wave_buffer_type_si32;
			break;
		default:
			assert(false); // unexpected bits
			return false;
	}
	return true;
}

namespace {

// from http://www.winehq.com/hypermail/wine-patches/2002/12/0179.html
inline short u8tos16(unsigned char b) {	// wavs are unsigned
	return (short)((b+(b << 8))-32768);
}

inline short s8tos16(char b) {	// mods are singed
	return ((short)b) * 256;
}

}

void import_modfile::read_wave_level_samples(int i, int level, void* pbuffer) {
	int samplesize = modf->sample_bytes(i, level);
	int bits = modf->sample_bits_per_sample(i, level);
	int buffersize = bits == 8 ? samplesize * 2 : samplesize; // reserve extra space for in-buffer 8-to-16-bit conversion
	char* buffer = (char*)pbuffer; //new char[buffersize];
	modf->sample_seek(i, level);
	modf->sample_read(buffer, samplesize);

	int numsamples = modf->sample_samples(i, level);
	bool is_signed = modf->sample_signed(i, level);
	bool is_looping = modf->sample_looping(i, level);
	bool is_bidir = modf->sample_bidir_looping(i, level);
	bool is_stereo = modf->sample_stereo(i, level);
	int channels = is_stereo ? 2 : 1;
	int loop_start = modf->sample_loop_start(i, level);
	int loop_end = modf->sample_loop_end(i, level);
	float amp = modf->sample_amp(i, level);

	if (bits == 8) {
		if (is_signed) {
			for (int i = numsamples - 1; i >= 0; i--) {
				((signed short*)buffer)[i * channels] = s8tos16(buffer[i * channels]);
				if (is_stereo)
					((signed short*)buffer)[i * channels + 1] = s8tos16(buffer[i * channels + 1]);
			}
		} else {
			for (int i = numsamples - 1; i>=0; i--) {
				((signed short*)buffer)[i * channels] = u8tos16(buffer[i * channels]);
				if (is_stereo)
					((signed short*)buffer)[i * channels + 1] = u8tos16(buffer[i * channels + 1]);
			}
		}
	}
}

void import_modfile::close() {
	modf->close();
}

struct import_instrument_archive_factory : importfactory {
	import_instrument_archive_factory() {
		extensions.push_back("it");
		extensions.push_back("s3m");
		extensions.push_back("xm");
	}

	importplugin* create_importer() {
		return new import_modfile();
	}

	virtual bool is_container() {
		return true;
	}

	virtual zzub_wave_importer_type get_type() {
		return zzub_wave_importer_type_instrument_archive;
	}
};

struct import_wave_archive_factory : importfactory {
	import_wave_archive_factory() {
		extensions.push_back("mod");
		extensions.push_back("drumkit");
	}

	importplugin* create_importer() {
		return new import_modfile();
	}

	virtual bool is_container() {
		return true;
	}

	virtual zzub_wave_importer_type get_type() {
		return zzub_wave_importer_type_wave_archive;
	}
};

struct import_wave_file_factory : importfactory {
	import_wave_file_factory() {
		static const char* ext[] = { 
			"wav", "aif", "aifc", "aiff", "flac", "xi", "au", "paf", "snd", 
			"voc", "smp", "iff", "8svx", "16svx", "w64", "mat4", "mat5", 
			"pvf", "htk", "caf", "sd2", "raw", "mp3" };
		for (int i = 0; i < sizeof(ext) / sizeof(const char*); i++) {
			extensions.push_back(ext[i]);
		}
	}

	importplugin* create_importer() {
		return new import_modfile();
	}

	virtual bool is_container() {
		return false;
	}

	virtual zzub_wave_importer_type get_type() {
		return zzub_wave_importer_type_wave_file;
	}
};

waveimporter::waveimporter() {
	// should be four types: wavfile (.wav), instrumentfile (.xi), wavarchive (.mod), instrumentarchive (.it, sfz)
	plugins.push_back(new import_wave_file_factory());
	plugins.push_back(new import_wave_archive_factory());
	plugins.push_back(new import_instrument_archive_factory());
}

waveimporter::~waveimporter() {
	for (size_t i = 0; i < plugins.size(); i++)
		delete plugins[i];
	plugins.clear();
}

importfactory* waveimporter::get_factory(std::string filename) {
	size_t dp = filename.find_last_of('.');
	if (dp == std::string::npos) return 0;
	std::string ext = filename.substr(dp + 1);
	transform(ext.begin(), ext.end(), ext.begin(), (int(*)(int))std::tolower);
	std::vector<importfactory*>::iterator i;
	for (i = plugins.begin(); i != plugins.end(); ++i) {
		std::vector<std::string>& exts = (*i)->extensions;
		std::vector<std::string>::iterator j = find(exts.begin(), exts.end(), ext);
		if (j != exts.end()) return *i;
	}

	return 0;
}

importplugin* waveimporter::open(std::string filename) {
	importfactory* factory = get_factory(filename);
	if (!factory) return 0;
	importplugin* plugin = factory->create_importer();
	if (!plugin) return 0;
	if (!plugin->open(filename)) {
		//plugin->destroy();
		delete plugin;
		return 0;
	}
	return plugin;
}

} // namespace zzub
