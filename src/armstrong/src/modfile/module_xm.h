#pragma once

namespace modimport {
namespace xm {

struct instrument_xm {
	_XMINSTRUMENT header;
	_XMSAMPLES samplesHeader;
	_XMSAMPLE* samples;
	std::vector<long> sampleOffsets;
};

struct xm_note {
	unsigned char note;
	unsigned char sample;
	unsigned char volume;
	unsigned char effect;
	unsigned char effect_value;
};

typedef std::vector<xm_note> xm_track;
typedef std::vector<xm_track> xm_pattern;

struct module_xm : module {
	std::ifstream strm;
	std::string filename;
	int current_instrument;
	int current_sample;
	
	_XMHEADER header;
	std::vector<char> orders;
	std::vector<instrument_xm> instruments;
	std::vector<xm_pattern> patterns;
	
	virtual bool open(std::string filename);
	virtual void close();
	std::string name();
	virtual fxtype type();
	
	virtual int sample_count(int instrument);
	virtual std::string sample_name(int instrument, int sample);
	virtual int sample_samplespersecond(int instrument, int sample);
	virtual bool sample_looping(int instrument, int sample);
	virtual bool sample_bidir_looping(int instrument, int sample);
	virtual bool sample_stereo(int instrument, int sample);
	virtual unsigned long sample_loop_start(int instrument, int sample);
	virtual unsigned long sample_loop_end(int instrument, int sample);
	virtual float sample_amp(int instrument, int sample);
	virtual int sample_note_c4_rel(int instrument, int sample);
	virtual int sample_bits_per_sample(int instrument, int sample);
	virtual unsigned long sample_samples(int instrument, int sample);
	virtual void sample_seek(int instrument, int sample);
	virtual bool sample_read(void* buffer, unsigned long bytes);
	virtual bool sample_signed(int instrument, int sample);
	
	virtual int instrument_count();
	virtual std::string instrument_name(int instrument);
	
	virtual int pattern_count();
	virtual int pattern_rows(int pattern);
	virtual int pattern_track_columns(int pattern);
	virtual int pattern_global_columns(int pattern);
	virtual int pattern_extra_columns(int pattern);
	virtual int pattern_tracks(int pattern);
	
	virtual int pattern_column_type(int pattern, int column);
	virtual int pattern_column_value(int pattern, int column, int row);
	
	virtual int order_length();
	virtual int order_pattern(int index);
};

}

}
