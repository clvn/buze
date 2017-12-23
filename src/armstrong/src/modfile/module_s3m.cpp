#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "module.h"
#include "s3m.h"
#include "module_s3m.h"

namespace modimport {
namespace s3m {

int s3m_to_midi_note(int value) {
	if (value == 255 || value == 254) return value;
	return 12 * (value >> 4) + (value & 0xf) + 12;
}

bool module_s3m::open(std::string fileName) {
	strm.open(fileName.c_str(), std::ios::binary | std::ios::in);
	if (!strm) return false;

	strm.read((char*)&header, sizeof(_S3MHEADER));

	for (int i = 0; i < header.ordNum; i++) {
		unsigned char ord;
		strm.read((char*)&ord, sizeof(unsigned char));
		orders.push_back(ord);
	}

	// TODO: read padding

	for (int i = 0; i < header.insNum; i++) {
		unsigned short ins;
		strm.read((char*)&ins, sizeof(unsigned short));
		instrumentOffsets.push_back(ins);
	}

	for (int i = 0; i < header.patNum; i++) {
		unsigned short pat;
		strm.read((char*)&pat, sizeof(unsigned short));
		patternOffsets.push_back(pat);
	}

	strm.read((char*)channelPan, sizeof(channelPan));
	numChannels = 0;
	for (int i = 0; i < 32; i++) {
		if (header.channelSettings[i] < 128) numChannels = i + 1;
	}

	samples = new _S3MSAMPLE[header.insNum];

	for (int i = 0; i < header.insNum; i++) {
		int ofs = instrumentOffsets[i] * 16;
		strm.seekg(ofs);
		strm.read((char*)&samples[i], sizeof(_S3MSAMPLE));
	}

/*
        So to unpack, first read one byte. If it's zero, this row is
        done (64 rows in entire pattern). If nonzero, the channel
        this entry belongs to is in BYTE AND 31. Then if bit 32
        is set, read NOTE and INSTRUMENT (2 bytes). Then if bit
        64 is set read VOLUME (1 byte). Then if bit 128 is set
        read COMMAND and INFO (2 bytes).

Note; hi=oct, lo=note, 255=empty note,
254=key off (used with adlib, with samples stops smp)

*/
	for (int i = 0; i < header.patNum; i++) {
		int ofs = patternOffsets[i] * 16;
		strm.seekg(ofs);
		unsigned short patternSize;
		strm.read((char*)&patternSize, sizeof(unsigned short));
		s3m_pattern pattern;
		pattern.resize(numChannels);
		for (int chn = 0; chn < numChannels; chn++) {
			pattern[chn].resize(64);
		}
		for (int row = 0; row < 64; ) {
			unsigned char pack;
			strm.read((char*)&pack, sizeof(unsigned char));
			if (pack == 0) {
				row++;
				continue;
			}

			int track = pack&31;
			s3m_note s3mnote = { 0, 0, 0, 0, 0 };
			if (pack & 32) {
				strm.read((char*)&s3mnote.note, sizeof(unsigned char));
				strm.read((char*)&s3mnote.sample, sizeof(unsigned char));
				s3mnote.note = s3m_to_midi_note(s3mnote.note);
			}
			if (pack & 64) strm.read((char*)&s3mnote.volume, sizeof(unsigned char));
			if (pack & 128) {
				strm.read((char*)&s3mnote.effect, sizeof(unsigned char));
				strm.read((char*)&s3mnote.effect_value, sizeof(unsigned char));
			}
			if (track >= numChannels) continue;
			pattern[track][row] = s3mnote;
		}
		patterns.push_back(pattern);

	}

	// here be sampledata

	return true;
}

void module_s3m::close() {
	strm.close();
}

std::string module_s3m::name() {
	return header.songName;
}

fxtype module_s3m::type() {
	return screamtracker;
}

int module_s3m::sample_count(int instrument) {
	if (samples[instrument].len> 0) 
		return 1; else
		return 0;
}

std::string module_s3m::sample_name(int instrument, int sample) {
	return samples[instrument].samplename;
}

int module_s3m::sample_samplespersecond(int instrument, int sample) { 
	return samples[instrument].c2spd; 
}

bool module_s3m::sample_looping(int instrument, int sample) { 
	return (samples[instrument].flags & 1) != 0;
}

bool module_s3m::sample_bidir_looping(int instrument, int sample) { 
	return false;
}

bool module_s3m::sample_stereo(int instrument, int sample) { 
	return (samples[instrument].flags & 2) != 0;
}

unsigned long module_s3m::sample_loop_start(int instrument, int sample) { 
	return samples[instrument].loopbeg;
}

unsigned long module_s3m::sample_loop_end(int instrument, int sample) { 
	return samples[instrument].loopend;
}

float module_s3m::sample_amp(int instrument, int sample) { 
	return (float)samples[instrument].vol / 63.0f;
}

int module_s3m::sample_note_c4_rel(int instrument, int sample) { 
	return 0; 
}

int module_s3m::sample_bits_per_sample(int instrument, int sample) { 
	return (samples[instrument].flags & 4) != 0 ? 16 : 8;
}

unsigned long module_s3m::sample_samples(int instrument, int sample) {
	if (samples[instrument].seg == 0) return 0;
	return samples[instrument].len;
}

void module_s3m::sample_seek(int instrument, int sample) {
	strm.seekg(samples[instrument].seg * 16);
}

bool module_s3m::sample_read(void* buffer, unsigned long bytes) {
	strm.read((char*)buffer, bytes);
	return true;
}

int module_s3m::instrument_count() { 
	return header.insNum;
}

std::string module_s3m::instrument_name(int instrument) { 
	return samples[instrument].samplename;
}

int module_s3m::pattern_count() { 
	return patterns.size();
}

int module_s3m::pattern_rows(int pattern) { 
	return patterns[pattern][0].size();
}

int module_s3m::pattern_track_columns(int pattern) { 
	return 1 + 1 + 1 + 1 + 1;   // note, wave, volume, fx, fx value
}

int module_s3m::pattern_global_columns(int pattern) { 
	return 0; 
}

int module_s3m::pattern_extra_columns(int pattern) { 
	return 0; 
}

int module_s3m::pattern_tracks(int pattern) { 
	return patterns[pattern].size();
}

int module_s3m::pattern_column_type(int pattern, int column) { 
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

int module_s3m::pattern_column_value(int pattern, int column, int row) { 
	int chn = column / 5;
	s3m_note& note = patterns[pattern][chn][row];
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

int module_s3m::order_length() {
	return (int)orders.size();
}

int module_s3m::order_pattern(int index) {
	return orders[index];
}

}
}
