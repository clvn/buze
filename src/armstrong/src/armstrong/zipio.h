#pragma once
#include "../minizip/zip.h"
#include "../minizip/unzip.h"

struct zip_outstream : zzub::outstream {
    zipFile f;
	bool open;

	zip_outstream() {
		f = 0;
		open = false;
	}

	~zip_outstream() {
		if (f && open)
			zipCloseFileInZip(f);
	}

	virtual int write(void* v, int size) {
		if (ZIP_OK != zipWriteInFileInZip(f, v, size))
			return 0;
		return size;
	}

	long position() { assert(false); return 0; }
	void seek(long, int) { assert(false); }

    bool create_file_in_zip(std::string fileName) {
		assert(!open);
		if (ZIP_OK != zipOpenNewFileInZip(f, fileName.c_str(), 0, 0, 0, 0, 0, 0, Z_DEFLATED, Z_DEFAULT_COMPRESSION))
			return false;
		open = true;
		return true;
	}

    void closeFileInArchive() {
		zipCloseFileInZip(f);
		open = false;
	}
};

struct zip_instream : zzub::instream  {
    unzFile f;
	int uncompressed_size;
    size_t current_offset;
	bool open;

	zip_instream() {
		current_offset = 0;
		f = 0;
		open = false;
	}

	~zip_instream() {
		if (f && open)
			close_file_in_zip();
	}

    bool open_file_in_zip(std::string filename) {
		assert(f != 0);
		assert(!open);

		const int CASESENSITIVITY = (0);

		if (unzLocateFile(f, filename.c_str(), CASESENSITIVITY) != UNZ_OK) return false;

		unz_file_info fileinfo;
		char nameinzip[32768];

		if (unzGetCurrentFileInfo(f, &fileinfo, nameinzip, 32768, NULL, 0, NULL, 0) != UNZ_OK) return false;
		if (unzOpenCurrentFile(f) != UNZ_OK) return false;

		current_offset = 0;
		uncompressed_size = fileinfo.uncompressed_size;
		open = true;
		return true;
	}

    void close_file_in_zip() {
		assert(open);
		assert(f != 0);

		unzCloseCurrentFile(f);
		open = false;
	}

    virtual void seek(long pos, int mode=SEEK_SET) { assert(false); }

    virtual long position() {
		assert(open);
		assert(f != 0);
		return (long)current_offset; 
	}

    virtual int read(void* buffer, int size) {
		assert(open);
		assert(f != 0);
		assert(buffer != 0);

		int result = unzReadCurrentFile(f, buffer, size);
		if (result < 0) return 0;
		current_offset += result;
		return result;
	}

	virtual long size() { 
		assert(open); 
		return uncompressed_size; 
	}
};

struct zip_archive : zzub::archive {
	zipFile zipf;
	unzFile unzf;

	zip_archive() {
		zipf = 0;
		unzf = 0;
	}

	virtual zzub::outstream *get_outstream(const char *path) {
		if (!zipf) return 0;

		zip_outstream* outs = new zip_outstream();
		outs->f = zipf;
		if (!outs->create_file_in_zip(path)) {
			delete outs;
			return 0;
		}
		return outs; // user must destroy result before calling get_outstream again!
	}

	virtual zzub::instream *get_instream(const char *path) {
		if (!unzf) return 0;

		zip_instream* ins = new zip_instream();
		ins->f = unzf;
		if (!ins->open_file_in_zip(path))
			return 0;
		return ins;
	}
};
