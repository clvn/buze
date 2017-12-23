#pragma once

namespace modimport {
namespace it {

struct it_note {
	unsigned char note;
	unsigned char sample;
	unsigned char volume;
	unsigned char pan;
	unsigned char effect;
	unsigned char effect_value;
};

typedef std::vector<it_note> it_track;
typedef std::vector<it_track> it_pattern;

struct module_it : module {
	std::ifstream strm;
	std::string filename;
	
	_ITHEADER header;
	_ITINSTRUMENT_NEW* instruments;
	_ITSAMPLE* samples;
	
	std::vector<long> instrument_offsets;
	std::vector<long> sample_offsets;
	std::vector<long> pattern_offsets;
	std::vector<std::vector<int> > instrument_samples;
	std::vector<it_pattern> patterns;
	int num_channels;
	std::vector<int> orders;
	int max_channel;
	int current_instrument;
	int current_sample;
	char sample_buffer[0x8000];
	char* sample_buffer_ptr;
	int sample_buffer_size;
	int current_sample_position;
	
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
	
	void get_instrument_samples(_ITINSTRUMENT_NEW& instrument, std::vector<int>& sample_indices);
};

}
}
