#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include <fstream>
#include "module.h"
#include "xm.h"
#include "module_xm.h"

#if !defined(_WIN64)

namespace modimport {
namespace xm {

struct xmfilenote {
	unsigned char a,b,c,d,e;
};

bool module_xm::open(std::string fileName) {
	strm.open(fileName.c_str(), std::ios::binary | std::ios::in);
	if (!strm) return false;

	strm.read((char*)&header, sizeof(_XMHEADER));

	for (int i = 0; i < header.numPats; i++) {
		_XMPATTERN patternHeader;
		strm.read((char*)&patternHeader, sizeof(_XMPATTERN));

		// read packed patterns
		xm_pattern pattern;
		pattern.resize(header.numChans);
		for (int chn = 0; chn < header.numChans; chn++) {
			pattern[chn].resize(patternHeader.rows);
		}
		for (int row = 0; row < patternHeader.rows; row++) {
			for (int j = 0; j < header.numChans; j++) {
				xm_note xmnote = { 0,0,0,0,0 };
				unsigned char pack;
				strm.read((char*)&pack, sizeof(unsigned char));
				if (pack & 0x80) {
					if (pack & 1) {
						strm.read((char*)&xmnote.note, sizeof(unsigned char));
						if (xmnote.note == 97) 
							xmnote.note = 255;
						else
							xmnote.note += 12;
					}
					if (pack & 2) strm.read((char*)&xmnote.sample, sizeof(unsigned char));
					if (pack & 4) strm.read((char*)&xmnote.volume, sizeof(unsigned char));
					if (pack & 8) strm.read((char*)&xmnote.effect, sizeof(unsigned char));
					if (pack & 16) strm.read((char*)&xmnote.effect_value, sizeof(unsigned char));
				} else {
					xmnote.note = pack;
					strm.read((char*)&xmnote.sample, sizeof(unsigned char));
					strm.read((char*)&xmnote.volume, sizeof(unsigned char));
					strm.read((char*)&xmnote.effect, sizeof(unsigned char));
					strm.read((char*)&xmnote.effect_value, sizeof(unsigned char));
				}
				pattern[j][row]=xmnote;
			}
		}
		patterns.push_back(pattern);
		//reader.seek(pattern.patternSize, SEEK_CUR);
	}

	for (int i = 0; i < header.numInstr; i++) {
		instrument_xm xi;
		int headerSize;
		int headerOffset = strm.tellg();//reader.position();
		strm.read((char*)&headerSize, sizeof(int));

		//if (0 == strm.read((char*)&xi.header, sizeof(_XMINSTRUMENT))) break;	// eof = no more samples?
		strm.read((char*)&xi.header, sizeof(_XMINSTRUMENT));
		if (strm.eof()) break;

		xi.header.name[21] = 0;

		xi.samples = new _XMSAMPLE[xi.header.numSamples];
		
		int samplesHeaderSize = 0;
		if (xi.header.numSamples > 0) {
			strm.read((char*)&samplesHeaderSize, sizeof(int));
			strm.read((char*)&xi.samplesHeader, sizeof(_XMSAMPLES));

			int remains = headerSize - ((int)strm.tellg() - headerOffset);
			strm.seekg(remains, SEEK_CUR);

			for (int j = 0; j < xi.header.numSamples; j++) {
				strm.read((char*)&xi.samples[j], sizeof(_XMSAMPLE));
				xi.samples[j].samplename[21] = 0;
			}

			// here be sampledata
			for (int j = 0; j < xi.header.numSamples; j++) {
				xi.sampleOffsets.push_back(strm.tellg());
				int bytesPerSample = 1;//(xi.samples[j].flags & sample_flag_16bit)?2:1;
				strm.seekg(xi.samples[j].len * bytesPerSample, SEEK_CUR);
			}
		} else {
			int remains = headerSize - ((int)strm.tellg() - headerOffset);
			strm.seekg(remains, SEEK_CUR);
		}

		instruments.push_back(xi);
	}
	return true;
}

void module_xm::close() {
	strm.close();
}

std::string module_xm::name() {
	std::string songName;
	songName.assign(header.songName, 20);
	return songName;
}

fxtype module_xm::type() {
	return fasttracker;
}

int module_xm::sample_count(int instrument) {
	if (instrument<0 || instrument>=instruments.size()) return 0;
	return instruments[instrument].header.numSamples;
}


std::string module_xm::sample_name(int instrument, int sample) {
	return instruments[instrument].samples[sample].samplename;
}

// returns 8363*2^((transp*128+ftune)/(12*128)) (from OpenMPTs soundlib)
unsigned int TransposeToFrequency(int transp, int ftune)
//-----------------------------------------------------------
{
	const float _fbase = 8363;
	const float _factor = 1.0f/(12.0f*128.0f);
	int result;
	unsigned int freq;
#if defined(_MSC_VER)
	transp = (transp << 7) + ftune;
	_asm {
	fild transp
	fld _factor
	fmulp st(1), st(0)
	fist result
	fisub result
	f2xm1
	fild result
	fld _fbase
	fscale
	fstp st(1)
	fmul st(1), st(0)
	faddp st(1), st(0)
	fistp freq
	}
	unsigned int derr = freq % 11025;
	if (derr <= 8) freq -= derr;
	if (derr >= 11015) freq += 11025-derr;
	derr = freq % 1000;
	if (derr <= 5) freq -= derr;
	if (derr >= 995) freq += 1000-derr;
	return freq;
#else
	assert(false);
	return 0;
#endif
}

int module_xm::sample_samplespersecond(int instrument, int sample) { 
	_XMSAMPLE& xms = instruments[instrument].samples[sample];
	return TransposeToFrequency(xms.relnote, xms.finetune);
}

bool module_xm::sample_looping(int instrument, int sample) { 
	unsigned int loop = instruments[instrument].samples[sample].flags & sample_flag_loop_type_mask;
	return loop == sample_loop_forward;
}

bool module_xm::sample_bidir_looping(int instrument, int sample) { 
	unsigned int loop = instruments[instrument].samples[sample].flags & sample_flag_loop_type_mask;
	return loop == sample_loop_bidir;
}

bool module_xm::sample_stereo(int instrument, int sample) { 
	return false;
}

unsigned long module_xm::sample_loop_start(int instrument, int sample) { 
	int bytesPerSample = (instruments[instrument].samples[sample].flags & sample_flag_16bit)?2:1;
	return instruments[instrument].samples[sample].loopstart / bytesPerSample;
}

unsigned long module_xm::sample_loop_end(int instrument, int sample) { 
	int bytesPerSample = (instruments[instrument].samples[sample].flags & sample_flag_16bit)?2:1;
	return (instruments[instrument].samples[sample].loopstart + instruments[instrument].samples[sample].looplen) / bytesPerSample;
}

float module_xm::sample_amp(int instrument, int sample) { 
	return (float)instruments[instrument].samples[sample].vol / 255.0f;
}

int module_xm::sample_note_c4_rel(int instrument, int sample) { 
	return 0; 
}

int module_xm::sample_bits_per_sample(int instrument, int sample) { 
	if (instruments[instrument].samples[sample].flags & sample_flag_16bit) 
		return 16; else
		return 8;
}

unsigned long module_xm::sample_samples(int instrument, int sample) {
	int bytesPerSample = (instruments[instrument].samples[sample].flags & sample_flag_16bit)?2:1;
	return instruments[instrument].samples[sample].len / bytesPerSample;
}

void module_xm::sample_seek(int instrument, int sample) {
	strm.seekg(instruments[instrument].sampleOffsets[sample]);
	current_instrument = instrument;
	current_sample = sample;
}

bool module_xm::sample_read(void* buffer, unsigned long bytes) {
	strm.read((char*)buffer, bytes);
	//if (read != bytes) return read;

	int bits = sample_bits_per_sample(current_instrument, current_sample);
	int numSamples = sample_samples(current_instrument, current_sample);

	int olds = 0;

	// these are delta values, undelta 16 bit separately
	if (bits == 8) {
		char* sample = (char*)buffer;
		for (int i = 0; i < numSamples; i++) {
			int news = sample[i] + olds;
			sample[i] = news;
			olds = news;
		}
	} else
	if (bits==16) {
		short* sample = (short*)buffer;
		for (int i = 0; i < numSamples; i++) {
			int news = sample[i] + olds;
			sample[i] = news;
			olds = news;
		}
	}
	return true;
}

bool module_xm::sample_signed(int instrument, int sample) {
	return true;
}

int module_xm::instrument_count() { 
	return instruments.size();
}

std::string module_xm::instrument_name(int instrument) { 
	if (instrument<0 || instrument>=instruments.size()) return "";
	return instruments[instrument].header.name; 
}

int module_xm::pattern_count() { 
	return patterns.size();
}

int module_xm::pattern_rows(int pattern) { 
	return patterns[pattern][0].size();
}

int module_xm::pattern_track_columns(int pattern) { 
	return 1 + 1 + 1 + 1 + 1;   // note, wave, volume, fx, fx value
}

int module_xm::pattern_global_columns(int pattern) { 
	return 0; 
}

int module_xm::pattern_extra_columns(int pattern) { 
	return 0; 
}

int module_xm::pattern_tracks(int pattern) { 
	return header.numChans;
}

int module_xm::pattern_column_type(int pattern, int column) { 
	column %= pattern_track_columns(pattern);
	switch (column) {
		case 0:
			return column_type_note;
		case 1:
			return column_type_wave;
		case 2:
			return column_type_volume;
		case 3:
			return column_type_effect;
		case 4:
			return column_type_byte;
	}
	return -1; 
}

int module_xm::pattern_column_value(int pattern, int column, int row) { 
	int chn = column / 5;
	xm_note& note = patterns[pattern][chn][row];
	int type = column % 5;
	switch (type) {
		case 0:
			return note.note;
		case 1:
			return note.sample;
		case 2:
			return note.volume;
		case 3:
			return note.effect;
		case 4:
			return note.effect_value;
	}
	return 0; 
}

int module_xm::order_length() {
	return header.songLen;
}

int module_xm::order_pattern(int index) {
	return header.orders[index];
}

}
}

#endif
