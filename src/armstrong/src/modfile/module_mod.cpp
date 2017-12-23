#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#if defined(POSIX)
#include <unistd.h>
#endif
#include "module.h"
#include "mod.h"
#include "module_mod.h"

#if defined(__ANDROID__)
#include <byteswap.h>
void swab(const void *from, void *to, ssize_t n) {
	const int16_t *in = (int16_t*)from;
	int16_t *out = (int16_t*)to;
	int i;
	n /= 2;
	for (i = 0 ; i < n; i++) {
		out[i] = bswap_16(in[i]);
	}
}
#endif

namespace modimport {
namespace mod {

typedef struct {
	const char* desc;
	int channels;
} modformat ;

modformat formats[] = {
	{ "TDZ1", 1 }, 
	{ "TDZ2", 2 }, 
	{ "TDZ3", 3 }, 
	{ "M.K.", 4 }, 
	{ "5CHN", 5 }, 
	{ "6CHN", 6 },	// there are some others for 6 channels
	{ "7CHN", 7 }, 
	{ "8CHN", 8 }, 	// there are some others for 8 channels
	{ "9CHN", 9 }, 
	{ "10CH", 10 }, 
	{ "11CH", 11 }, 
	{ "12CH", 12 }, 
	{ "13CH", 13 }, 
	{ "14CH", 14 }, 
	{ "15CH", 15 }, 
	{ "16CH", 16 }, 
	{ "17CH", 17 }, 
	{ "18CH", 18 }, 
	{ "19CH", 19 }, 
	{ "20CH", 20 }, 
	{ "21CH", 21 }, 
	{ "22CH", 22 }, 
	{ "23CH", 23 }, 
	{ "24CH", 24 }, 
	{ "25CH", 25 }, 
	{ "26CH", 26 }, 
	{ "27CH", 27 }, 
	{ "28CH", 28 }, 
	{ "29CH", 29 }, 
	{ "30CH", 30 }, 
	{ "31CH", 31 }, 
	{ "32CH", 32 }, 
};

const int formatsCount=sizeof(formats) / sizeof(modformat);

struct modfilenote {
	unsigned char a,b,c,d;
};

bool module_mod::open(std::string fileName) {
	strm.open(fileName.c_str(), std::ios::binary | std::ios::in);
	if (!strm) return false;

	strm.read((char*)&header, sizeof(_MODHEADER));

	numChannels = 0;
	for (int i = 0; i < formatsCount; i++) {
		if (strncmp(header.desc, formats[i].desc, 4) == 0) {
			numChannels = formats[i].channels;
			break;
		}
	}
	if (!numChannels) {
		strm.close();
		return false;
	}

	numPatterns = 0;
	for (int i = 0; i < 128; i++) {
		if (header.order[i] > numPatterns)
			numPatterns = header.order[i];
	}
	numPatterns++;

	int patternSize = 4 * numChannels * 64;	// 4 bytes pr column, always 64 rows

	int sampleStart = sizeof(_MODHEADER) + numPatterns * patternSize;
	int ofs = 0;
	for (int i = 0; i < 31; i++) {
		sampleOffsets.push_back(sampleStart + ofs);

		char* plen = (char*)&header.sample[i].length;
		swab(plen, plen, 2);

		plen = (char*)&header.sample[i].loopend;
		swab(plen, plen, 2);

		plen = (char*)&header.sample[i].loopstart;
		swab(plen, plen, 2);

		ofs += header.sample[i].length * 2;
	}

	int rowsize = 4*numChannels;
	char* pdata = new char[patternSize];
	for (int i = 0; i < numPatterns; i++) {
		strm.read(pdata, patternSize);
		mod_pattern pattern;
		for (int chn = 0; chn < numChannels; chn++) {
			mod_track track;
			for (int row = 0; row < 64; row++) {
				mod_note modnote;
				modfilenote* v = (modfilenote*)(&pdata[chn*4 + row*rowsize]);
				modnote.sample = (v->a & 0xF0) | (v->c >> 4);
				modnote.note = period_to_note(((v->a & 0xF) << 8) | v->b);
				modnote.effect = v->c & 0xF;
				modnote.effect_value = v->d;

				track.push_back(modnote);
			}
			pattern.push_back(track);
		}
		patterns.push_back(pattern);
		
	}
	delete[] pdata;

	return true;
}

void module_mod::close() {
	strm.close();
}

std::string module_mod::name() {
	std::string songName;
	return songName;
}

fxtype module_mod::type() {
	return protracker;
}

int module_mod::sample_count(int instrument) {
	return header.sample[instrument].length > 0 ? 1 : 0;
}


std::string module_mod::sample_name(int instrument, int sample) {
	return header.sample[instrument].samplename;
}

int module_mod::sample_samplespersecond(int instrument, int sample) { 
	// http://www.uni-giessen.de/faq/archiv/sound-file-format.mod-faq.part1-2/msg00001.html
	return 8287 * 4; // PAL  C-2 = 8287
	//return 8363 * 4; // NTSC C-2 = 8363, multiply by 4 to get C-4
}

bool module_mod::sample_looping(int instrument, int sample) { 
	if (header.sample[instrument].loopstart == 0 && 
		header.sample[instrument].loopend <= 2) return false;
	return true;
}

bool module_mod::sample_bidir_looping(int instrument, int sample) { 
	return false;
}

bool module_mod::sample_stereo(int instrument, int sample) { 
	return false;
}

unsigned long module_mod::sample_loop_start(int instrument, int sample) { 
	// TODO: return min loop start/end, e.g introfronty.mod, hendrix.mod
	int loop1 = header.sample[instrument].loopstart * 2;
	//int loop2 = header.sample[instrument].loopend * 2;
	return loop1; //std::min(loop1, loop2);
}

unsigned long module_mod::sample_loop_end(int instrument, int sample) { 
	// TODO: return max loop start/end, e.g introfronty.mod, hendrix.mod
	int loop1 = header.sample[instrument].loopstart * 2 +
				header.sample[instrument].loopend * 2;

	if (sample_looping(instrument, sample))
		return loop1;//std::max(loop1, loop2);
	return header.sample[instrument].length * 2;
}

float module_mod::sample_amp(int instrument, int sample) { 
	return (float)header.sample[instrument].volume / 0x40;
}

int module_mod::sample_note_c4_rel(int instrument, int sample) { 
	return 0; 
}

int module_mod::sample_bits_per_sample(int instrument, int sample) { 
	return 8;
}

unsigned long module_mod::sample_samples(int instrument, int sample) {
	return (float)header.sample[instrument].length * 2;
}

void module_mod::sample_seek(int instrument, int sample) {
	strm.seekg(sampleOffsets[instrument]);
}

bool module_mod::sample_read(void* buffer, unsigned long bytes) {
	strm.read((char*)buffer, bytes);
	return true;
}

bool module_mod::sample_signed(int instrument, int sample) {
	return true;
}


int module_mod::instrument_count() { 
	return 31;
}

std::string module_mod::instrument_name(int instrument) { 
	return header.sample[instrument].samplename;
}


int module_mod::pattern_count() { 
	return numPatterns;
}

int module_mod::pattern_rows(int pattern) { 
	return 64; 
}

int module_mod::pattern_track_columns(int pattern) { 
	return 1 + 1 + 1 + 1;   // note, wave, fx, fx value
}

int module_mod::pattern_global_columns(int pattern) { 
	return 0; 
}

int module_mod::pattern_extra_columns(int pattern) { 
	return 0; 
}

int module_mod::pattern_tracks(int pattern) { 
	return numChannels; 
}

int module_mod::pattern_column_type(int pattern, int column) { 
	column%=4;
	switch (column) {
		case 0:
			return column_type_note;
		case 1:
			return column_type_wave;
		case 2:
			return column_type_effect;
		case 3:
			return column_type_byte;
	}
	return -1; 
}

int module_mod::pattern_column_value(int pattern, int column, int row) { 
	int chn = column / 4;
	mod_note& note = patterns[pattern][chn][row];
	int type = column % 4;
	switch (type) {
		case 0:
			return note.note;
		case 1:
			return note.sample;
		case 2:
			return note.effect;
		case 3:
			return note.effect_value;
	}
	return 0; 
}

int module_mod::period_to_note(int period) {
	int numperiods = sizeof(_MODPERIODS) / sizeof(short);
	for (int i = 0; i<numperiods; i++) {
		if (_MODPERIODS[i] == period) return i + 1 + 36;
	}
	return 0;
}


int module_mod::order_length() {
	return header.songlen;
}

int module_mod::order_pattern(int index) {
	return header.order[index];
}

}
}
