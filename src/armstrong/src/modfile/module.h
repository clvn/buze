#pragma once
/*

    C++ interface for libmodimport

*/

namespace modimport {

enum fxtype {
	protracker,
	fasttracker,
	screamtracker,
	impulsetracker,
};

enum column_type {
	column_type_note,
	column_type_wave,
	column_type_volume,
	column_type_effect,
	column_type_byte,
};

struct module {
	// finds a loader based on filename extension and open()s it
	static module* create(std::string fileName);

	virtual ~module() { }
	virtual bool open(std::string fileName) = 0;
	virtual void close() = 0;
	virtual std::string name() { return ""; }
	virtual fxtype type() = 0;
	virtual int sample_count(int instrument) = 0;
	virtual int sample_samplespersecond(int instrument, int sample) = 0;
	virtual unsigned long sample_samples(int instrument, int sample) = 0;
	virtual std::string sample_name(int instrument, int sample) = 0;
	virtual bool sample_float(int instrument, int sample) { return false; }
	virtual bool sample_signed(int instrument, int sample) { return false; }
	virtual bool sample_looping(int instrument, int sample) = 0;
	virtual bool sample_bidir_looping(int instrument, int sample) = 0;
	virtual bool sample_stereo(int instrument, int sample) = 0;
	virtual unsigned long sample_loop_start(int instrument, int sample) = 0;
	virtual unsigned long sample_loop_end(int instrument, int sample) = 0;
	virtual float sample_amp(int instrument, int sample) = 0;
	virtual int sample_note_c4_rel(int instrument, int sample) = 0;
	virtual int sample_bits_per_sample(int instrument, int sample) = 0;
	virtual int sample_bytes_per_sample(int instrument, int sample);
	virtual unsigned long sample_bytes(int instrument, int sample);
	virtual void sample_seek(int instrument, int sample) = 0;
	virtual bool sample_read(void* buffer, unsigned long bytes) = 0;

	virtual int instrument_count() = 0;
	virtual std::string instrument_name(int instrument) = 0;

	virtual int pattern_count() { return 0; }
	virtual int pattern_rows(int pattern) { return 0; }
	virtual int pattern_track_columns(int pattern) { return 0; }
	virtual int pattern_global_columns(int pattern) { return 0; }
	virtual int pattern_extra_columns(int pattern) { return 0; }
	virtual int pattern_tracks(int pattern) { return 0; }
	
	virtual int pattern_column_type(int pattern, int column) { return 0; }
	virtual int pattern_column_value(int pattern, int column, int row) { return 0; }

	virtual int order_length() { return 0; }
	virtual int order_pattern(int index) { return 0; }
};

}
