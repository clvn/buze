#pragma once
#include <mad.h>

#define MAD_FRAMESIZE 8192

namespace modimport {
namespace mad {

struct module_mad : module {
	std::ifstream strm;
	struct mad_stream stream;
	struct mad_frame frame;
	struct mad_synth synth;
	int samplerate;
	int channels;
	int bad_last_frame;
	bool out_of_sync;
	int framepos;
	unsigned char framebuffer[MAD_FRAMESIZE];
	float outbuffer[0x1000];
	int outbuffer_size;
	float* outbuffer_ptr;
	
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
	virtual bool sample_float(int instrument, int sample);
	
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
	
	bool run_frame(bool only_header);
	void reset_stream();

	enum mad_flow zzub_mad_input(struct mad_stream *stream);
	enum mad_flow zzub_mad_header(struct mad_header const *header);
	float zzub_mad_scale(mad_fixed_t sample);
	enum mad_flow zzub_mad_output(struct mad_header const *header, struct mad_pcm *pcm);
	enum mad_flow zzub_mad_error(struct mad_stream *stream, struct mad_frame *frame);
};

}
}
