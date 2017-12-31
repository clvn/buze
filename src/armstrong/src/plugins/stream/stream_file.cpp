#include <string>
#include <modfile/module.h>
#include <player/audiodriver/convertsample.h>
#include "stream_plugin.h"
#include "stream_info.h"
#include "stream_file.h"

/***

	streaming of wav-files with libsndfile

***/

struct stream_source {
	modimport::module* source;
	std::string filename;
	int instrument;
	int sample;
	stream_resampler* resampler;

	stream_source() {
		source = 0;
		instrument = 0;
		sample = 0;
		resampler = 0;
	}

	~stream_source() {
		if (resampler) delete resampler;
		if (source) delete source;
	}

};

struct stream_file : stream_plugin<stream_source>, stream_provider {
	char buffer[sizeof(float) * 2 * stream_resampler::max_samples_per_tick];
	int current_position;
	int trigger_song_position;
	int trigger_offset;
	int total_samples;

	stream_file();
	virtual ~stream_file();

	virtual void init(zzub::archive* pi);
	virtual void load(zzub::archive*);
	virtual void save(zzub::archive*);
	virtual void process_events();
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode);
	virtual bool process_offline(float **pin, float **pout, int *numsamples, int *channels, int *samplerate) { return false; }
	virtual void stop();
	virtual void set_stream_source(const char* resource);
	virtual bool play_wave(int wave, int note, float volume, int offset, int length);

	virtual bool generate_samples(float** buffer, int numsamples);
	virtual int get_target_samplerate();

	//bool open();
	void close();

};


stream_machine_info_file::stream_machine_info_file() {
	this->name = "File Stream";
	this->short_name = "FileStream";
	this->author = "Andy Werk";
	this->uri = "@zzub.org/stream/file;1";
	this->supported_stream_extensions.push_back("wav");
	this->supported_stream_extensions.push_back("aif");
	this->supported_stream_extensions.push_back("aifc");
	this->supported_stream_extensions.push_back("aiff");
	this->supported_stream_extensions.push_back("flac");
	this->supported_stream_extensions.push_back("xi");
	this->supported_stream_extensions.push_back("au");
	this->supported_stream_extensions.push_back("paf");
	this->supported_stream_extensions.push_back("snd");
	this->supported_stream_extensions.push_back("voc");
	this->supported_stream_extensions.push_back("smp");
	this->supported_stream_extensions.push_back("iff");
	this->supported_stream_extensions.push_back("8svx");
	this->supported_stream_extensions.push_back("16svx");
	this->supported_stream_extensions.push_back("w64");
	this->supported_stream_extensions.push_back("mat4");
	this->supported_stream_extensions.push_back("mat5");
	this->supported_stream_extensions.push_back("pvf");
	this->supported_stream_extensions.push_back("htk");
	this->supported_stream_extensions.push_back("caf");
	this->supported_stream_extensions.push_back("sd2");
	this->supported_stream_extensions.push_back("raw");
	this->supported_stream_extensions.push_back("mod");
	this->supported_stream_extensions.push_back("s3m");
	this->supported_stream_extensions.push_back("it");
	this->supported_stream_extensions.push_back("xm");
	this->supported_stream_extensions.push_back("mp3");
	this->supported_stream_extensions.push_back("drumkit");
}

zzub::plugin* stream_machine_info_file::create_plugin() { 
	return new stream_file(); 
}

/***

	stream_file

***/

stream_file::stream_file() {
	source = 0;
}

stream_file::~stream_file() {
	close();
}

void stream_file::init(zzub::archive * const pi) {
}

void stream_file::load(zzub::archive * const pi) {
}

void stream_file::save(zzub::archive* po) {
}

// duplicated from buze.cpp and FileBrowserView.cpp
#include <boost/lexical_cast.hpp>

void ParseFileName(const std::string& fullname, std::string* filename, int* instrument, int* sample) {
// the filename must end with zero, one or two numeric directories at the end, preceded by a filename with a .

	// find last part of filename
	std::string::size_type ls = fullname.find_last_of("/\\");
	if (ls == std::string::npos) {
		// no slashes, just return the filename
		*instrument = -1;
		*sample = -1;
		*filename = fullname;
		return ;
	}

	std::string lastpart = fullname.substr(ls + 1);
	int lastint;
	try {
		lastint = boost::lexical_cast<int, std::string>(lastpart);
	} catch (boost::bad_lexical_cast& e) {
		// not a numeric part at the end
		e ;
		*instrument = -1;
		*sample = -1;
		*filename = fullname;
		return ;
	}

	std::string::size_type sls = fullname.find_last_of("/\\", ls - 1);
	if (sls == std::string::npos) {
		// input is a slash and a single numeric part. strange, but still cool:
		*instrument = -1;
		*sample = -1;
		*filename = fullname;
		return ;
	}

	std::string secondlastpart = fullname.substr(sls + 1, (ls - sls) - 1);
	int secondlastint;
	try {
		secondlastint = boost::lexical_cast<int, std::string>(secondlastpart);
	} catch (boost::bad_lexical_cast& e) {
		// just one numeric identifier
		e ;
		*instrument = lastint;
		*sample = -1;
		*filename = fullname.substr(0, ls);
		return ;
	}

	*instrument = secondlastint;
	*sample = lastint;
	*filename = fullname.substr(0, sls);
}

void stream_file::set_stream_source(const char* resource) {
	stream_source* strm = new stream_source();
	ParseFileName(resource, &strm->filename, &strm->instrument, &strm->sample);

	if (strm->instrument == -1) {
		strm->instrument = 0;
	}

	if (strm->sample == -1) {
		strm->sample = 0;
	}

	strm->source = modimport::module::create(strm->filename);
	if (!strm->source) {
		delete strm;
		return ;
	}

	strm->resampler = new stream_resampler(this);
	strm->resampler->stream_sample_rate = strm->source->sample_samplespersecond(strm->instrument, strm->sample);
	source_queue.push(strm);

	current_position = 0;
}

bool stream_file::play_wave(int wave, int note, float volume, int offset, int length) {
	return false;
}

void stream_file::process_events() {

	if (!source) return ;

	bool triggered = false;
	int offset = 0;

	if (gval.note != zzub_note_value_none) {
		if (gval.note == zzub_note_value_off || gval.note == zzub_note_value_cut) {
			stop();
		} else {
			source->resampler->note = buzz_to_midi_note(gval.note);
			triggered = true;
			offset = 0;
		}
	}

	if (gval.offset != 0xFFFFFFFF) {
		offset = (int)get_offset();
		triggered = true;
	}

	if (triggered) {
		trigger_song_position = _mixer->song_position;
		trigger_offset = offset;
		current_position = offset;
		total_samples = 0;
		source->source->sample_seek(source->instrument, source->sample);
		//sf_seek(sf, offset, SEEK_SET);
		source->resampler->set_stream_pos(current_position);
	}

}

int stream_file::get_target_samplerate() {
	return _master_info->samples_per_second;
}


bool stream_file::process_stereo(float **pin, float **pout, int numsamples, int mode) {
	if (mode == zzub_process_mode_read || mode == zzub_process_mode_no_io || !source || !source->resampler->playing) {
		pout[0] = 0;
		pout[1] = 0;
		return false;
	}

	if (_mixer->state == zzub::player_state_playing) {
		if (total_samples == 0 && trigger_song_position < _mixer->song_position) {
			// sync with the song position
			//sf_seek(sf, _mixer->song_position - trigger_song_position, SEEK_SET);
			source->resampler->set_stream_pos(0);
		}

		total_samples += numsamples;
	}

	return source->resampler->process_stereo(pout, numsamples);
}

bool stream_file::generate_samples(float** pout, int numsamples) {
	bool result = true;

	int maxread = numsamples;
	int frames = source->source->sample_samples(source->instrument, source->sample);
	if (current_position + maxread > frames) 
		maxread = frames - current_position;
	
	if (maxread <= 0) {
		return false;
	}

	int channels = source->source->sample_stereo(source->instrument, source->sample) ? 2 : 1;
	int bytes_per_sample = source->source->sample_bytes_per_sample(source->instrument, source->sample);
	source->source->sample_read(buffer, maxread * bytes_per_sample * channels);

	zzub_wave_buffer_type srcformat;
	switch (bytes_per_sample) {
		case 1:
			if (source->source->sample_signed(source->instrument, source->sample)) {
				srcformat = (zzub_wave_buffer_type)5;
			} else {
				srcformat = (zzub_wave_buffer_type)4;
			}
			break;
		case 2:
			srcformat = zzub_wave_buffer_type_si16;
			break;
		case 3:
			srcformat = zzub_wave_buffer_type_si24;
			break;
		case 4:
			if (source->source->sample_float(source->instrument, source->sample)) {
				srcformat = zzub_wave_buffer_type_f32;
			} else {
				srcformat = zzub_wave_buffer_type_si32;
			}
			break;
		default:
			assert(false);
			break;
	}

	copy_samples(buffer, pout[0], maxread, srcformat, zzub_wave_buffer_type_f32, channels, 1, 0, 0, false);
	if (channels == 2) {
		copy_samples(buffer, pout[1], maxread, srcformat, zzub_wave_buffer_type_f32, channels, 1, 1, 0, false);
	} else {
		copy_samples(buffer, pout[1], maxread, srcformat, zzub_wave_buffer_type_f32, channels, 1, 0, 0, false);
	}
	memset(pout[0] + maxread, 0, numsamples - maxread);
	memset(pout[1] + maxread, 0, numsamples - maxread);

	current_position += maxread;
	return result;
}

void stream_file::stop() {
	if (source->resampler) source->resampler->playing = false;
}

/*bool stream_file::open() {
	assert(false);
	std::string fn = filename;
	close();

	source = modimport::module::create(fn);
	if (!source) return false;

	current_position = 0;
	return true;
}
	*/

void stream_file::close() {
	if (!source) return ;
	delete source;
	source = 0;
	//filename = "";

}
