#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#if defined(POSIX)
#include <unistd.h>
#endif
#include "module.h"
#include "mod.h"
#include "module_sndfile.h"

namespace modimport {
namespace sndfile {

#pragma pack(push, 1)
struct S24 {
	union {
		struct {
			unsigned char c3[3];
		};
		struct {
			unsigned short s;
			unsigned char c;
		};
	};
};
#pragma pack(pop)

static sf_count_t instream_filelen (void *user_data) {
	std::istream* strm = (std::istream*)user_data ;
	sf_count_t pos = strm->tellg();
	strm->seekg(0, std::ios::end);
	sf_count_t size = strm->tellg();
	strm->seekg(pos, std::ios::beg);
	return size;
}

static sf_count_t instream_seek (sf_count_t offset, int whence, void *user_data) {
	std::istream* strm = (std::istream*)user_data ;
	strm->seekg((long)offset, (int)whence);
	return strm->tellg();
}

static sf_count_t instream_read (void *ptr, sf_count_t count, void *user_data){
	std::istream* strm = (std::istream*)user_data ;
	strm->read((char*)ptr, (int)count);
	return count;
}

static sf_count_t instream_write (const void *ptr, sf_count_t count, void *user_data) {
	std::istream* strm = (std::istream*)user_data ;
	assert(false);
	return 0;
}

static sf_count_t instream_tell (void *user_data){
	std::istream* strm = (std::istream*)user_data ;
	return strm->tellg();
}

bool module_sndfile::open(std::string fileName) {
	strm.open(fileName.c_str(), std::ios::binary | std::ios::in);
	if (!strm) return false;

	memset(&sfinfo, 0, sizeof(sfinfo));
	SF_VIRTUAL_IO vio;
	vio.get_filelen = instream_filelen ;
	vio.seek = instream_seek;
	vio.read = instream_read;
	vio.write = instream_write;
	vio.tell = instream_tell;
	sf = sf_open_virtual(&vio, SFM_READ, &sfinfo, &strm);

	if (!sf || !sfinfo.frames || sfinfo.channels > 2) {
		sf_close(sf);
		sf = 0;
		strm.close();
		return false;
	}
	return true;
}

void module_sndfile::close() {
	sf_close(sf);
	sf = 0;
	strm.close();
}

std::string module_sndfile::name() {
	std::string songName;
	return songName;
}

fxtype module_sndfile::type() {
	return protracker;
}

int module_sndfile::sample_count(int instrument) {
	return 1;
}


std::string module_sndfile::sample_name(int instrument, int sample) {
	return "sndfile";
}

int module_sndfile::sample_samplespersecond(int instrument, int sample) { 
	return sfinfo.samplerate;
}

bool module_sndfile::sample_float(int instrument, int sample) { 
	switch (sfinfo.format & SF_FORMAT_SUBMASK) {
		case SF_FORMAT_FLOAT:
		case SF_FORMAT_DOUBLE:
			return true;
	}
	return false;
}

bool module_sndfile::sample_looping(int instrument, int sample) { 
	return false;
}

bool module_sndfile::sample_bidir_looping(int instrument, int sample) { 
	return false;
}

bool module_sndfile::sample_stereo(int instrument, int sample) { 
	return (sfinfo.channels == 2);
}

unsigned long module_sndfile::sample_loop_start(int instrument, int sample) { 
	return 0;
}

unsigned long module_sndfile::sample_loop_end(int instrument, int sample) { 
	return 0;
}

float module_sndfile::sample_amp(int instrument, int sample) { 
	return 1.0f;
}

int module_sndfile::sample_note_c4_rel(int instrument, int sample) { 
	return 0; 
}

int module_sndfile::sample_bits_per_sample(int instrument, int sample) { 
	switch (sfinfo.format & SF_FORMAT_SUBMASK) {
		case SF_FORMAT_PCM_U8: // convert anything 8 bit to 16-bit
		case SF_FORMAT_PCM_S8:
		case SF_FORMAT_PCM_16:
		case SF_FORMAT_IMA_ADPCM:
		case SF_FORMAT_MS_ADPCM:
		case SF_FORMAT_ALAW:
		case SF_FORMAT_ULAW:
		case SF_FORMAT_GSM610:
			return 16;
		case SF_FORMAT_PCM_24:
			return 24;
		case SF_FORMAT_PCM_32:
			return 32;
		case SF_FORMAT_FLOAT:
		case SF_FORMAT_DOUBLE:
			return 32;
	}
	return 0;
}

unsigned long module_sndfile::sample_samples(int instrument, int sample) {
	return sfinfo.frames;
}

void module_sndfile::sample_seek(int instrument, int sample) {
	sf_seek(sf, 0, SEEK_SET);
}

inline void ConvertSample(const float &src, S24 &dst) {
	int i = (int)(src * 0x007fffff);
	dst.c3[0] = (i & 0x000000ff);
	dst.c3[1] = (i & 0x0000ff00) >> 8;
	dst.c3[2] = (i & 0x00ff0000) >> 16;
}

bool module_sndfile::sample_read(void* buffer, unsigned long bytes) {
	int bytes_per_sample = sample_bits_per_sample(0, 0) / 8;
	int channels = sample_stereo(0, 0) ? 2 : 1;
	int frames = bytes / bytes_per_sample / channels;
	S24* s24buffer;
	switch (bytes_per_sample) {
		case 2:
			sf_read_short(sf, (short*)buffer, frames * channels);
			break;
		case 3:
			s24buffer = (S24*)buffer;
			for (int i = 0; i < frames; i++) {
				float f[2];
				sf_read_float(sf, f, sfinfo.channels);
				for (int j = 0; j < sfinfo.channels; j++) {
					ConvertSample(f[j], *s24buffer);
					s24buffer++;
				}
			}
			break;
		case 4:
			if (sample_float(0, 0)) {
				sf_read_float(sf, (float*)buffer, frames * channels);
			} else {
				sf_read_int(sf, (int*)buffer, frames * channels);
			}
			break;
		default:
			return false;
	}
	return true;
}

bool module_sndfile::sample_signed(int instrument, int sample) {
	return true;
}


int module_sndfile::instrument_count() { 
	return 1;
}

std::string module_sndfile::instrument_name(int instrument) { 
	return "";
}


int module_sndfile::pattern_count() { 
	return 0;
}

int module_sndfile::pattern_rows(int pattern) { 
	return 64; 
}

int module_sndfile::pattern_track_columns(int pattern) { 
	return 0;
}

int module_sndfile::pattern_global_columns(int pattern) { 
	return 0; 
}

int module_sndfile::pattern_extra_columns(int pattern) { 
	return 0; 
}

int module_sndfile::pattern_tracks(int pattern) { 
	return 0; 
}

int module_sndfile::pattern_column_type(int pattern, int column) { 
	return -1; 
}

int module_sndfile::pattern_column_value(int pattern, int column, int row) { 
	return 0; 
}

int module_sndfile::order_length() {
	return 0;
}

int module_sndfile::order_pattern(int index) {
	return 0;
}


}
}
