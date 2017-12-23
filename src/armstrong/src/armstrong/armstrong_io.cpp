#include <cstring>
#include <map>
#include "player.h"
#include "archive.h"

using namespace armstrong::frontend;

extern "C" {

zzub_archive_t* zzub_archive_create_memory() {
	return new mem_archive();
}

zzub_output_t* zzub_archive_get_output(zzub_archive_t* a, const char* path) {
	return a->get_outstream(path);
}

zzub_input_t* zzub_archive_get_input(zzub_archive_t* a, const char* path) {
	return a->get_instream(path);
}

void zzub_archive_destroy(zzub_archive_t* a) {
	delete a;
}

zzub_output_t* zzub_output_create_file(const char* filename) {
	file_outstream* fout = new file_outstream();
	if (!fout->create(filename)) {
		delete fout;
		return 0;
	}
	return fout;
}

void zzub_output_destroy(zzub_output_t* fout) {
	delete fout;
}

zzub_input_t* zzub_input_open_file(const char* filename) {
	file_instream* fout = new file_instream();
	if (!fout->open(filename)) {
		delete fout;
		return 0;
	}
	return fout;
}

void zzub_input_destroy(zzub_input_t* inf) {
	delete inf;
}

int zzub_input_read(zzub_input_t* f, char* buffer, int bytes) {
	return f->read(buffer, bytes);
}

int zzub_input_size(zzub_input_t* f) {
	return f->size();
}

int zzub_input_position(zzub_input_t* f) {
	return f->position();
}

void zzub_input_seek(zzub_input_t* f, int a, int b) {
	f->seek(a, b);
}

void zzub_output_write(zzub_output_t* f, const char* buffer, int bytes) {
	f->write((void*)buffer, bytes);
}

int zzub_output_position(zzub_output_t* f) {
	return f->position();
}

void zzub_output_seek(zzub_output_t* f, int a, int b) {
	f->seek(a, b);
}

}
