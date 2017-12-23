#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cassert>
#if defined(POSIX)
#include <unistd.h>
#endif
#include "module.h"
#include "drumkitimport.h"
#include "module_drumkit.h"

namespace modimport {
namespace drumkit {

bool module_drumkit::open(std::string filename) {
	return reader.open(filename.c_str());
}

void module_drumkit::close() {
	reader.close();
}

std::string module_drumkit::name() {
	std::string songName;
	return songName;
}

fxtype module_drumkit::type() {
	return protracker;
}

int module_drumkit::sample_count(int instrument) {
	return 1;
}


std::string module_drumkit::sample_name(int instrument, int sample) {
	return "drumkit";
}

int module_drumkit::sample_samplespersecond(int instrument, int sample) {
	return 44100;
}

bool module_drumkit::sample_float(int instrument, int sample) { 
	return false;
}

bool module_drumkit::sample_looping(int instrument, int sample) { 
	return false;
}

bool module_drumkit::sample_bidir_looping(int instrument, int sample) { 
	return false;
}

bool module_drumkit::sample_stereo(int instrument, int sample) { 
	return false;
}

unsigned long module_drumkit::sample_loop_start(int instrument, int sample) { 
	return 0;
}

unsigned long module_drumkit::sample_loop_end(int instrument, int sample) { 
	return 0;
}

float module_drumkit::sample_amp(int instrument, int sample) { 
	return 1.0f;
}

int module_drumkit::sample_note_c4_rel(int instrument, int sample) { 
	return 0; 
}

int module_drumkit::sample_bits_per_sample(int instrument, int sample) { 
	return 16;
}

unsigned long module_drumkit::sample_samples(int instrument, int sample) {
	return reader.samples[instrument].tamano;
}

void module_drumkit::sample_seek(int instrument, int sample) {
	reader.seek_sample_data(instrument);
	current_instrument = instrument;
}

bool module_drumkit::sample_read(void* buffer, unsigned long bytes) {
	reader.read_sample_data(current_instrument, buffer, bytes / 2);
	return true;
}

bool module_drumkit::sample_signed(int instrument, int sample) {
	return true;
}


int module_drumkit::instrument_count() { 
	return reader.samples.size();
}

std::string module_drumkit::instrument_name(int instrument) { 
	return reader.samples[instrument].name;
}


int module_drumkit::pattern_count() { 
	return 0;
}

int module_drumkit::pattern_rows(int pattern) { 
	return 64; 
}

int module_drumkit::pattern_track_columns(int pattern) { 
	return 0;
}

int module_drumkit::pattern_global_columns(int pattern) { 
	return 0; 
}

int module_drumkit::pattern_extra_columns(int pattern) { 
	return 0; 
}

int module_drumkit::pattern_tracks(int pattern) { 
	return 0; 
}

int module_drumkit::pattern_column_type(int pattern, int column) { 
	return -1; 
}

int module_drumkit::pattern_column_value(int pattern, int column, int row) { 
	return 0; 
}

int module_drumkit::order_length() {
	return 0;
}

int module_drumkit::order_pattern(int index) {
	return 0;
}


}
}
