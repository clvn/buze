#pragma once

namespace modimport {
namespace s3m {

struct s3m_note {
	unsigned char note;
	unsigned char sample;
	unsigned char volume;
	unsigned char effect;
	unsigned char effect_value;
};

typedef std::vector<s3m_note> s3m_track;
typedef std::vector<s3m_track> s3m_pattern;

struct module_s3m : module {
	std::ifstream strm;
	
	_S3MHEADER header;
	_S3MSAMPLE* samples;
	std::vector<char> orders;
	std::vector<short> instrumentOffsets;
	std::vector<short> patternOffsets;
	unsigned char channelPan[32];
	std::vector<s3m_pattern> patterns;
	int numChannels;
	
	virtual bool open(std::string fileName);
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
