#pragma once
#define NOMINMAX

#if defined(USE_LIBSF2)
#include "libsf2/SF.h"
#include "libsf2/RIFF.h"
#endif
#if defined(USE_SFENABLER)
typedef SHORT sfBankID; 
#endif

#include "modfile/module.h"

#ifdef WIN32
// libsf2 defines its own POSIX interfering with the zzub POSIX define
#undef POSIX
#endif

namespace zzub {

struct importwave_info {
	int channels;
	int sample_count;
	int samples_per_second;
	zzub_wave_buffer_type format;
	std::string name;
};

typedef importwave_info exportwave_info;

struct importplugin {
	virtual ~importplugin() { }
	virtual bool open(std::string filename) = 0;
	virtual int get_wave_count() = 0;
	virtual std::string get_wave_name(int i) = 0;
	virtual int get_wave_level_count(int i) = 0;
	virtual bool get_wave_level_info(int i, int level, importwave_info& info) = 0;
	virtual void read_wave_level_samples(int i, int level, void* buffer) = 0;
	virtual void close() = 0;
};

/*
class zzub_sf2 : public RIFF::Container {
public:
	zzub_input_t* instream;

	zzub_sf2(zzub_input_t* strm) : RIFF::Container("filename.sf2") {
		instream = strm;
		Mode = RIFF::stream_mode_read;
        ulStartPos = RIFF_HEADER_SIZE;
        ReadHeader(0);
        if (ChunkID != CHUNK_ID_RIFF && ChunkID != CHUNK_ID_RIFX) {
            throw RIFF::Exception("Not a RIFF file");
        }

	}

	// abstract read-operations
	virtual bool InSeek(unsigned long pos, int mode) {
		zzub_input_seek(instream, pos, mode);
		return zzub_input_position(instream) == pos;
	}

	virtual int Read(void* buffer, int len) {
		return zzub_input_read(instream, (char*)buffer, len);
	}
	virtual bool IsOpen() {
		return true;
	}

	virtual void Close() {
	}

	// abstract write-operations
	virtual bool OutSeek(unsigned long pos, int mode) {
		assert(false);
		return false;
	}
	virtual int Write(const void* buffer, int len) {
		assert(false);
		return 0;
	}
};

*/
#if defined(USE_LIBSF2)

struct import_sf2 : importplugin {
	RIFF::File* pRIFF;
	sf2::File* file;

	import_sf2();
	bool open(std::string filename);
	int get_wave_count();
	std::string get_wave_name(int i);
	int get_wave_level_count(int i);
	bool get_wave_level_info(int i, int level, importwave_info& info);
	void read_wave_level_samples(int i, int level, void* buffer);
	void close();
};

struct import_sfz : importplugin {
	import_sfz();
	bool open(std::string filename);
	int get_wave_count();
	std::string get_wave_name(int i);
	int get_wave_level_count(int i);
	bool get_wave_level_info(int i, int level, importwave_info& info);
	void read_wave_level_samples(int i, int level, void* buffer);
	void close();
};
#endif

#if defined(USE_SFENABLER)

struct import_sf2 : importplugin {
	sfBankID sfBank;
	import_sf2();
	bool open(std::string filename);
	int get_wave_count();
	std::string get_wave_name(int i);
	int get_wave_level_count(int i);
	bool get_wave_level_info(int i, int level, importwave_info& info);
	void read_wave_level_samples(int i, int level, void* buffer);
	void close();
};
#endif

struct import_modfile : importplugin {
	modimport::module* modf;

	import_modfile();
	bool open(std::string filename);
	int get_wave_count();
	std::string get_wave_name(int i);
	int get_wave_level_count(int i);
	bool get_wave_level_info(int i, int level, importwave_info& info);
	void read_wave_level_samples(int i, int level, void* buffer);
	void close();
};

struct importfactory {
	std::vector<std::string> extensions;
	virtual ~importfactory() {}
	virtual importplugin* create_importer() = 0;
	virtual bool is_container() = 0;
	virtual zzub_wave_importer_type get_type() = 0;
};

struct waveimporter {
	std::vector<importfactory*> plugins;

	waveimporter();
	~waveimporter();
	importplugin* open(std::string filename);
	importfactory* get_factory(std::string filename);
};

};

