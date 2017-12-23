#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "module.h"
#include "it.h"
#include "module_it.h"

int itsex_decompress8 (std::istream& module, void *dst, int len, int it215);
int itsex_decompress16 (std::istream& module, void *dst, int len, int it215);
int itsex_decompress16_chunk (std::istream& module, void *dst, int len, int it215);
int itsex_decompress8_chunk (std::istream& module, void *dst, int len, int it215);

namespace modimport {
namespace it {

bool module_it::open(std::string fileName) {
	strm.open(fileName.c_str(), std::ios::binary | std::ios::in);
	if (!strm) return false;

	strm.read((char*)&header, sizeof(_ITHEADER));

	// skip orders
	char c;
	for (int i = 0; i < header.OrdNum; i++) {
		strm.read(&c, sizeof(char));
		orders.push_back(c);
	}

	unsigned int ins;
	for (int i = 0; i < header.InsNum; i++) {
		strm.read((char*)&ins, sizeof(unsigned int));
		instrument_offsets.push_back(ins);
	}

	for (int i = 0; i < header.SmpNum; i++) {
		strm.read((char*)&ins, sizeof(unsigned int));
		sample_offsets.push_back(ins);
	}


	for (int i = 0; i < header.PatNum; i++) {
		strm.read((char*)&ins, sizeof(unsigned int));
		pattern_offsets.push_back(ins);
	}

	instruments = new _ITINSTRUMENT_NEW[header.InsNum];
	instrument_samples.resize(instrument_offsets.size());
	for (int i = 0; i < instrument_offsets.size(); i++) {
		strm.seekg(instrument_offsets[i]);
		strm.read((char*)&instruments[i], sizeof(_ITINSTRUMENT_NEW));
		get_instrument_samples(instruments[i], instrument_samples[i]);
	}

	samples = new _ITSAMPLE[header.SmpNum];
	for (int i = 0; i < sample_offsets.size(); i++) {
		strm.seekg(sample_offsets[i]);
		strm.read((char*)&samples[i], sizeof(_ITSAMPLE));
	}

	num_channels = 64;
	/*for (int i = 0; i<64; i++) {
	if (header.ChnPan[i]&0x80) numChannels = i+1;
	}*/
	max_channel = 0;
	for (int i = 0; i < pattern_offsets.size(); i++) {
		strm.seekg(pattern_offsets[i]);
		unsigned short size, rows;
		strm.read((char*)&size, sizeof(unsigned short));
		strm.read((char*)&rows, sizeof(unsigned short));

		unsigned int wtf;
		strm.read((char*)&wtf, sizeof(unsigned int));

		it_pattern pattern;
		pattern.resize(num_channels);
		for (int chn = 0; chn < num_channels; chn++) {
			pattern[chn].resize(rows);
		}
		std::vector<unsigned char> prevpack;
		prevpack.resize(num_channels);

		std::vector<unsigned char> prevnote;
		prevnote.resize(num_channels);

		std::vector<unsigned char> prevsample;
		prevsample.resize(num_channels);

		std::vector<unsigned char> prevvolume;
		prevvolume.resize(num_channels);

		std::vector<unsigned char> preveffect;
		preveffect.resize(num_channels);

		std::vector<unsigned char> preveffect_value;
		preveffect_value.resize(num_channels);

		for (int row = 0; row < rows; row++) {
			for (;;) {
				it_note itnote = { 0, 0, 0, 0, 0 };
				unsigned char chnpack;
				strm.read((char*)&chnpack, sizeof(unsigned char));
				if (chnpack == 0) break;
				int track = (chnpack-1) & 63;
				if (track > max_channel)
					max_channel = track;
				unsigned char pack;
				if (chnpack & 128) 
					strm.read((char*)&pack, sizeof(unsigned char)); else
					pack = prevpack[track];
				prevpack[track] = pack;

				if (pack & 1) {
					strm.read((char*)&itnote.note, sizeof(unsigned char));
					itnote.note++;
				}
				if (pack & 2) strm.read((char*)&itnote.sample, sizeof(unsigned char));
				if (pack & 4) strm.read((char*)&itnote.volume, sizeof(unsigned char));
				if (pack & 8) {
					strm.read((char*)&itnote.effect, sizeof(unsigned char));
					strm.read((char*)&itnote.effect_value, sizeof(unsigned char));
				}

				if (pack & 1) prevnote[track] = itnote.note;
				if (pack & 2) prevsample[track] = itnote.sample;
				if (pack & 4) prevvolume[track] = itnote.volume;
				if (pack & 8) {
					preveffect[track] = itnote.effect;
					preveffect_value[track] = itnote.effect_value;
				}

				if (pack & 16) itnote.note = prevnote[track];
				if (pack & 32) itnote.sample = prevsample[track];
				if (pack & 64) itnote.volume = prevvolume[track];
				if (pack & 128) {
					itnote.effect = preveffect[track];
					itnote.effect_value = preveffect_value[track];
				}


				// TODO: split volume byte into volume and panning

				pattern[track][row] = itnote;
			}

		}
		patterns.push_back(pattern);
	}

	return true;
}

void module_it::get_instrument_samples(_ITINSTRUMENT_NEW& instrument, std::vector<int>& sample_indices) {
	std::map<int, int> indices;
	for (int i = 0; i < 120; i++) {
		int sample_index = instrument.NodeSampleKeyboardTable[i * 2 + 1];

		if (sample_index == 0) continue;
		sample_index--;
		if (std::find(sample_indices.begin(), sample_indices.end(), sample_index) == sample_indices.end()) {
			sample_indices.push_back(sample_index);
		}
	}
}

void module_it::close() {
	strm.close();
}

std::string module_it::name() {
	return header.songName;
}

fxtype module_it::type() {
	return impulsetracker;
}

int module_it::sample_count(int instrument) {
	return instrument_samples[instrument].size();
}

std::string module_it::sample_name(int instrument, int sample) {
	int sample_index = instrument_samples[instrument][sample];
	return samples[sample_index].sampleName;
}

int module_it::sample_samplespersecond(int instrument, int sample) { 
	int sample_index = instrument_samples[instrument][sample];
	return samples[sample_index].C5Speed;
}

bool module_it::sample_looping(int instrument, int sample) { 
	int sample_index = instrument_samples[instrument][sample];
	return samples[sample_index].LoopEnd>0;
}

bool module_it::sample_bidir_looping(int instrument, int sample) { 
	int sample_index = instrument_samples[instrument][sample];
	return (samples[sample_index].Flags & sample_flag_bidir_loop) != 0;
}

bool module_it::sample_stereo(int instrument, int sample) { 
	int sample_index = instrument_samples[instrument][sample];
	return (samples[sample_index].Flags & sample_flag_stereo) != 0;
}

unsigned long module_it::sample_loop_start(int instrument, int sample) { 
	int sample_index = instrument_samples[instrument][sample];
	return samples[sample_index].LoopBegin;
}

unsigned long module_it::sample_loop_end(int instrument, int sample) { 
	int sample_index = instrument_samples[instrument][sample];
	return samples[sample_index].LoopEnd;
}

float module_it::sample_amp(int instrument, int sample) { 
	int sample_index = instrument_samples[instrument][sample];
	return (float)samples[sample_index].Vol / 63.0f;
}

int module_it::sample_note_c4_rel(int instrument, int sample) { 
	return 0; 
}

int module_it::sample_bits_per_sample(int instrument, int sample) { 
	int sample_index = instrument_samples[instrument][sample];
	if (samples[sample_index].Flags & sample_flag_16bit) 
		return 16; else
		return 8;
}

unsigned long module_it::sample_samples(int instrument, int sample) {
	int sample_index = instrument_samples[instrument][sample];
	return (unsigned long)samples[sample_index].Length;
}

void module_it::sample_seek(int instrument, int sample) {
	int sample_index = instrument_samples[instrument][sample];
	strm.seekg(samples[sample_index].SamplePointer);
	current_instrument = instrument;
	current_sample = sample;
	current_sample_position = 0;
	sample_buffer_size = 0;
	sample_buffer_ptr = sample_buffer;
}

bool module_it::sample_read(void* buffer, unsigned long bytes) {
	int sample_index = instrument_samples[current_instrument][current_sample];
	bool compressed = (samples[sample_index].Flags & sample_flag_compressed) != 0;
	int bits = sample_bits_per_sample(current_instrument, current_sample);
	int total_bytes = sample_bytes(current_instrument, current_sample);
	bool delta = (samples[sample_index].Cvt & sample_convert_delta) != 0;

	char* dst = (char*)buffer;
	if (compressed) {
		while (bytes > 0) {
			if (sample_buffer_size > 0) {
				int copylen = std::min((int)bytes, sample_buffer_size);
				memcpy(dst, sample_buffer_ptr, copylen);
				dst += copylen;
				sample_buffer_size -= copylen;
				bytes -= copylen;
				if (sample_buffer_size == 0) {
					sample_buffer_ptr = sample_buffer;
				} else {
					sample_buffer_ptr += copylen;
				}
				continue;
			}

			int bufferlen = std::min(total_bytes - current_sample_position, 0x8000);
			int status;
			switch (bits) {
				case 8:
					status = itsex_decompress8_chunk(strm, sample_buffer, bufferlen, delta);
					break;
				case 16:
					status = itsex_decompress16_chunk(strm, sample_buffer, bufferlen / 2, delta);
					break;
				default:
					return false;
			}
			if (!status) {
				return false;
			}

			current_sample_position += bufferlen;
			sample_buffer_size = bufferlen;
		}
	} else {
		strm.read((char*)buffer, bytes);
	}
	return true;
}

bool module_it::sample_signed(int instrument, int sample) {
	int sample_index = instrument_samples[instrument][sample];
	return (header.Cwtv >= 0x0202);
}

int module_it::instrument_count() { 
	return instrument_offsets.size();
}

std::string module_it::instrument_name(int instrument) { 
	return instruments[instrument].instrumentName;
}

int module_it::pattern_count() { 
	return patterns.size();
}

int module_it::pattern_rows(int pattern) { 
	return patterns[pattern][0].size();
}

int module_it::pattern_track_columns(int pattern) { 
	return 1 + 1 + 1 + 1 + 1;   // note, wave, volume, fx, fx value
}

int module_it::pattern_global_columns(int pattern) { 
	return 0; 
}

int module_it::pattern_extra_columns(int pattern) { 
	return 0; 
}

int module_it::pattern_tracks(int pattern) { 
	return max_channel;//patterns[pattern].size();
}

int module_it::pattern_column_type(int pattern, int column) { 
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

int module_it::pattern_column_value(int pattern, int column, int row) { 
	int chn = column / 5;
	it_note& note = patterns[pattern][chn][row];
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


int module_it::order_length() {
	return (int)orders.size();
}

int module_it::order_pattern(int index) {
	return orders[index];
}

}
}
