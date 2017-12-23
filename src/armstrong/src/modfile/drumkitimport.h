/*
parser for the .drumkit format from PSI Drum 2 / Drumkit Manager 3

Based on the VB code with spanish variable names from http://dkm3.sourceforge.net/ :)

This code is public domain. Or at least LGPL if there was a judicial problem by porting the spanish VB.
*/

#pragma once
#include <iostream>
#include <fstream>
#include <vector>

struct drumkit_header {
	char psid[4];
	unsigned int sample_count;
	char author[64];
	char comment[128];
};

struct drumkit_sample_header {
	char name[32];
	char sampletype;
	char tone;
	char volume;
	unsigned int tamano; // number of samples
	unsigned int offset; // file offset location of the sample
};

struct drumkitreader {
	std::ifstream f;
	drumkit_header header;
	std::vector<drumkit_sample_header> samples;

	bool open(const char* filename) {
		f.open(filename, std::ios::binary);
		if (!f) return false;
		f.read(header.psid, 4);
		f.read((char*)&header.sample_count, 4);
		f.read(header.author, 64);
		f.read(header.comment, 128);
		for (int i = 0; i < (int)header.sample_count; i++) {
			drumkit_sample_header smphdr;
			f.read(smphdr.name, 32);
			f.read((char*)&smphdr.sampletype, 1);
			f.read((char*)&smphdr.tone, 1);
			f.read((char*)&smphdr.volume, 1);
			f.read((char*)&smphdr.tamano, 4);
			smphdr.offset = (unsigned int)f.tellg();
			samples.push_back(smphdr);

			if (smphdr.tamano > 0) { // skip 16 bit sample data
				f.seekg(smphdr.tamano * 2, std::ios::cur);
			}
		}
		return true;
	}


	void seek_sample_data(int index) {
		f.seekg(samples[index].offset, std::ios::beg);
	}

	void read_sample_data(int index, void* buffer, int samples) {
		f.read((char*)buffer, samples * 2);
		//f.read((char*)buffer, samples[index].tamano * 2);
	}

	void close() {
		f.close();
	}
};
/*
int main() {
	drumkitreader f;
	if (!f.open("BongoTranceKit.drumkit")) {
		return 1;
	}

	return 0;
}
*/