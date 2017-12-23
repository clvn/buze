#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#if defined(POSIX)
#include <unistd.h>
#endif
#include "module.h"
#include "mod.h"
#include "module_mad.h"

namespace modimport {
namespace mad {

bool module_mad::open(std::string filename) {
	strm.open(filename.c_str(), std::ios::binary | std::ios::in);
	if (!strm) return false;

	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);
	bad_last_frame = 0;
	out_of_sync = true;
	framepos = 0;
	outbuffer_size = 0;
	
	samplerate = 0;
	channels = 0;

	// run through all frame headers once 
	// determine total length, set samplerate and channels
	int total_samples = 0;
	while (!strm.eof()) {
		if (!run_frame(true)) {
			close();
			return false;
		}
		if (!out_of_sync) {
			total_samples += 32 * MAD_NSBSAMPLES(&frame.header);
		}
	}

	return true;
}

void module_mad::close() {
	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	strm.close();
}

std::string module_mad::name() {
	std::string songName;
	return songName;
}

fxtype module_mad::type() {
	return protracker;
}

int module_mad::sample_count(int instrument) {
	return 1;
}


std::string module_mad::sample_name(int instrument, int sample) {
	return "mp3";
}

int module_mad::sample_samplespersecond(int instrument, int sample) { 
	return samplerate;
}

bool module_mad::sample_looping(int instrument, int sample) { 
	return false;
}

bool module_mad::sample_bidir_looping(int instrument, int sample) { 
	return false;
}

bool module_mad::sample_stereo(int instrument, int sample) { 
	return channels == 2 ? true : false;
}

unsigned long module_mad::sample_loop_start(int instrument, int sample) { 
	return 0;
}

unsigned long module_mad::sample_loop_end(int instrument, int sample) { 
	return 0;
}

float module_mad::sample_amp(int instrument, int sample) { 
	return 1.0f;
}

int module_mad::sample_note_c4_rel(int instrument, int sample) { 
	return 0; 
}

int module_mad::sample_bits_per_sample(int instrument, int sample) { 
	return 32;
}

unsigned long module_mad::sample_samples(int instrument, int sample) {
	return 1000000;
}

void module_mad::sample_seek(int instrument, int sample) {
	strm.clear();
	strm.seekg(0, std::ios::beg);
	reset_stream();
	outbuffer_ptr = outbuffer;
	outbuffer_size = 0;
}

bool module_mad::sample_read(void* buffer, unsigned long bytes) {
	float* dst = (float*)buffer;

	int numsamples = bytes / sizeof(float) / channels;
	while (numsamples > 0) {
		if (outbuffer_size > 0) {
			int copylen = std::min((int)numsamples, outbuffer_size);
			memcpy(dst, outbuffer_ptr, copylen * sizeof(float) * channels);
			dst += copylen * channels;
			outbuffer_size -= copylen;
			numsamples -= copylen;

			if (outbuffer_size == 0) {
				outbuffer_ptr = outbuffer;
			} else {
				outbuffer_ptr += copylen * channels;
			}
			continue;
		}

		if (!run_frame(false)) {
			return false;
		}
	}
	return true;
}

bool module_mad::sample_signed(int instrument, int sample) {
	return true;
}

bool module_mad::sample_float(int instrument, int sample) {
	return true;
}


int module_mad::instrument_count() { 
	return 1;
}

std::string module_mad::instrument_name(int instrument) { 
	return "";
}


int module_mad::pattern_count() { 
	return 0;
}

int module_mad::pattern_rows(int pattern) { 
	return 64; 
}

int module_mad::pattern_track_columns(int pattern) { 
	return 0;
}

int module_mad::pattern_global_columns(int pattern) { 
	return 0; 
}

int module_mad::pattern_extra_columns(int pattern) { 
	return 0; 
}

int module_mad::pattern_tracks(int pattern) { 
	return 0; 
}

int module_mad::pattern_column_type(int pattern, int column) { 
	return -1; 
}

int module_mad::pattern_column_value(int pattern, int column, int row) { 
	return 0; 
}

int module_mad::order_length() {
	return 0;
}

int module_mad::order_pattern(int index) {
	return 0;
}


bool module_mad::run_frame(bool only_header) {
	if (out_of_sync) {
		out_of_sync = false;
		switch (zzub_mad_input(&stream)) {
			case MAD_FLOW_STOP:
			case MAD_FLOW_BREAK:
				return false;
			case MAD_FLOW_IGNORE:
				return true;
			case MAD_FLOW_CONTINUE:
				break;
		}
	}

	if (mad_header_decode(&frame.header, &stream) == -1) {
		if (!MAD_RECOVERABLE(stream.error)) {
			// needs more data
			out_of_sync = true;
			return true;
		}

		switch (zzub_mad_error(&stream, &frame)) {
			case MAD_FLOW_STOP:
				goto done;
			case MAD_FLOW_BREAK:
				goto fail;
			case MAD_FLOW_IGNORE:
			case MAD_FLOW_CONTINUE:
			default:
				out_of_sync = true;
				return true;
		}
	}

	switch (zzub_mad_header(&frame.header)) {
		case MAD_FLOW_STOP:
			goto done;
		case MAD_FLOW_BREAK:
			goto fail;
		case MAD_FLOW_IGNORE:
			return true;
		case MAD_FLOW_CONTINUE:
			break;
	}


	if (mad_frame_decode(&frame, &stream) == -1) {
		if (!MAD_RECOVERABLE(stream.error)) {
			out_of_sync = true;
			return true;
		}
		/*
		if (seekSkipSamples > 0) {
			// we dont add anything to outbuffer here, so we need to adjust 
			// seekSkipSamples to not chop off to much later
			int numSamples = 32 * MAD_NSBSAMPLES(&frame.header);
			if (seekSkipSamples > numSamples)
				seekSkipSamples -= numSamples; else
				seekSkipSamples = 0;
		}*/

		switch (zzub_mad_error(&stream, &frame)) {
			case MAD_FLOW_STOP:
				goto done;
			case MAD_FLOW_BREAK:
				goto fail;
			case MAD_FLOW_IGNORE:
				break;
			case MAD_FLOW_CONTINUE:
			default:
				//outOfSync = true;
				return true;
		}
	} else
		bad_last_frame = 0;

	if (!only_header) {
		mad_synth_frame(&synth, &frame);

		switch (zzub_mad_output(&frame.header, &synth.pcm)) {
			case MAD_FLOW_STOP:
				goto done;
			case MAD_FLOW_BREAK:
				goto fail;
			case MAD_FLOW_IGNORE:
			case MAD_FLOW_CONTINUE:
				break;
		}
	}

done:
	return true;
fail:
	return false;
}

void module_mad::reset_stream() {
	framepos = 0;

	mad_stream_init(&stream);
}


enum mad_flow module_mad::zzub_mad_input(struct mad_stream *stream) {
	if (strm.eof())
		return MAD_FLOW_STOP;

	int bufferleft = 0;
	if (stream->next_frame) {
		bufferleft = &framebuffer[framepos] - stream->next_frame;
	}
	//printf("stream->next_frame = %p, bufferleft = %i\n", stream->next_frame, bufferleft);

	if (bufferleft) {
		memmove(framebuffer, &framebuffer[framepos - bufferleft], bufferleft);
		framepos = bufferleft;
	}

	assert((MAD_FRAMESIZE-bufferleft) >= 0);
	if ((MAD_FRAMESIZE-bufferleft) > 0) {

		strm.read((char*)&framebuffer[framepos], MAD_FRAMESIZE - bufferleft);
		int bytes_read = strm.gcount();//MAD_FRAMESIZE - bufferleft;
		//printf("bytes_read = %i\n", bytes_read);
		framepos += bytes_read;
		//~ printf("%i bytes read\n", bytes_read);
		if (!bytes_read)
			return MAD_FLOW_STOP;
	}

	mad_stream_buffer(stream, framebuffer, framepos);

	return MAD_FLOW_CONTINUE;
}

enum mad_flow module_mad::zzub_mad_header(struct mad_header const *header) {
	//int numSamples = 32 * MAD_NSBSAMPLES(header);
	samplerate = header->samplerate;
	channels = MAD_NCHANNELS(header); 

	/*
	if (currentFrame >= frames.size()) {
		// remember all frames we see for seeking
		frame_info fi;
		fi.bitrate = header->bitrate;
		fi.offset = currentSample;
		fi.filepos = currentPosition;
		fi.samples = numSamples;

		frames.push_back(fi);
	}

	currentSample += numSamples;
	*/
	//int padding = (header->flags & MAD_FLAG_PADDING) != 0 ? 1 : 0;
	//int bytes = (144 * header->bitrate) / (header->samplerate + padding);

	/*
	int bytes = stream.next_frame - stream.this_frame;
	currentPosition += bytes;

	currentFrame++;
	*/
	return MAD_FLOW_CONTINUE;
}

inline float module_mad::zzub_mad_scale(mad_fixed_t sample) {
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return (float)sample / (float)MAD_F_ONE;
}

enum mad_flow module_mad::zzub_mad_output(struct mad_header const *header, struct mad_pcm *pcm) {

	unsigned int nsamples  = pcm->length;
	const mad_fixed_t* left_ch  = pcm->samples[0];
	const mad_fixed_t* right_ch  = pcm->samples[1];

	//samplerate = header->samplerate;
	//channels = pcm->channels;
	outbuffer_size = pcm->length;

	float* buffer = outbuffer;
	while (nsamples--) {
		*buffer = zzub_mad_scale(*left_ch);
		left_ch++;
		buffer++;

		if (channels == 2) {
			*buffer = zzub_mad_scale(*right_ch);
			right_ch++;
			buffer++;
		}
	}

	return MAD_FLOW_CONTINUE;
}

enum mad_flow module_mad::zzub_mad_error(struct mad_stream *stream, struct mad_frame *frame) {

	/*fprintf(stderr, "decoding error 0x%04x (%s) at frame %u\n",
		stream->error, mad_stream_errorstr(stream),
		stream->this_frame - framebuf);
*/
	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

	return MAD_FLOW_CONTINUE;
}

}
}
