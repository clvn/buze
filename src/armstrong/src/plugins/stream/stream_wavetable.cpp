#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
};
typedef struct zzub::mixer zzub_mixer_t;

#include <iostream>
#include <cstdio>
#include "stream_plugin.h"
#include "stream_info.h"
#include "stream_wavetable.h"
#include "mixing/mixer.h"

/***

	streaming of sampledata in wavetable

	handles extended buzz wavetable - supports 16/24/f32/s32 bits buffer types

***/

struct wave_source {
	int play_wave_id, play_level;
	stream_resampler* resampler;

	wave_source() {
		play_wave_id = 0;
		play_level = 0;
		resampler = 0;
	}

	~wave_source() {
		if (resampler) delete resampler;
	}
};

struct stream_wavetable : stream_plugin<wave_source>, stream_provider {
	unsigned int current_position;
	unsigned int last_current_position;
	unsigned int current_end_offset;

	stream_wavetable();
	virtual ~stream_wavetable();
	
	void init_resampler(wave_source* strm);

	virtual void init(zzub::archive* pi);
	virtual void load(zzub::archive*);
	virtual void save(zzub::archive*);
	virtual void process_events();
	virtual void attributes_changed();
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode);
	virtual bool process_offline(float **pin, float **pout, int *numsamples, int *channels, int *samplerate) { return false; }
	virtual void command(int);
	virtual void stop();
	virtual void get_sub_menu(int, zzub::outstream*);
	virtual void set_stream_source(const char* resource);
	virtual bool play_wave(int wave, int note, float volume, int offset, int length);

	virtual bool generate_samples(float** buffer, int numsamples);
	virtual int get_target_samplerate();
	zzub::wave_level* get_wavelevel(zzub::wave_info* wave, int index);

};

std::string stringFromInt(int i, int len, char fillChar) {
	char pc[16];
	sprintf(pc, "%i", i);
	std::string s=pc;
	while (s.length()<(size_t)len)
		s=fillChar+s;

	return s;
}

stream_machine_info_wavetable::stream_machine_info_wavetable() {
	this->name = "Wavetable Stream";
	this->short_name = "WavetableStream";
	this->author = "Andy Werk";
	this->uri = "@zzub.org/stream/wavetable;1";
	this->commands = "/Select Wave";
}

zzub::plugin* stream_machine_info_wavetable::create_plugin() { 
	return new stream_wavetable(); 
}

/***

	stream_wavetable

***/

stream_wavetable::stream_wavetable() {
	current_position = 0;
	source = 0;
}

stream_wavetable::~stream_wavetable() {
	if (source) delete source;
}

void stream_wavetable::init(zzub::archive * const pi) {
	this->current_position = 0;
	this->last_current_position = 0;
}

void stream_wavetable::load(zzub::archive * const pi) {
}

void stream_wavetable::save(zzub::archive* po) {
}

zzub::wave_level* stream_wavetable::get_wavelevel(zzub::wave_info* wave, int index) {
	int count = 0;
	for (std::vector<boost::shared_ptr<zzub::wave_level> >::const_iterator i = _mixer->wavelevels.top().begin(); i != _mixer->wavelevels.top().end(); ++i) {
		if (*i == 0) continue;
		if ((*i)->wave_id == wave->id) {
			if (count == index) return i->get();
			count++;
		}
	}
	return 0;
}

void stream_wavetable::init_resampler(wave_source* strm) {
	if (strm->play_wave_id == 0) {
		assert(false);//resampler = 0;
		return ;
	}

	strm->resampler = new stream_resampler(this);

	zzub::wave_info* wave = _mixer->waves.top()[strm->play_wave_id].get();
	zzub::wave_level* level = get_wavelevel(wave, strm->play_level);

	if (level) 
		strm->resampler->stream_sample_rate = level->samples_per_second;
}

void stream_wavetable::set_stream_source(const char* resource) {
	wave_source* strm = new wave_source();
	strm->play_wave_id = atoi(resource);
	strm->play_level = 0;

	if (strm->play_wave_id == 0) {
		delete strm;
		return ;
	}

	current_position = 0;
	last_current_position = 0;

	init_resampler(strm);
	source_queue.push(strm);
}

bool stream_wavetable::play_wave(int wave, int note, float volume, int offset, int length) {
	return false;
}

void stream_wavetable::attributes_changed() {
}

void stream_wavetable::process_events() {
	if (!source) return ;

	last_current_position = current_position;

	bool triggered = false;

	if (gval.note != zzub_note_value_none) {
		if (gval.note == zzub_note_value_off || gval.note == zzub_note_value_cut) {
			stop();
		} else {
			source->resampler->note = buzz_to_midi_note(gval.note);
			triggered = true;
			current_position = 0;
			current_end_offset = 0;
		}
	}

	if (gval.offset != 0xFFFFFFFF) {
		current_position = get_offset();
		current_end_offset = 0;

		triggered = true;
	}

	if (gval.length != 0xFFFFFFFF) {
		current_end_offset = current_position + get_length();
	}

	if (aval.offsetfromsong) {
		assert(false);
		/*const zzub::wave_info* wave = _host->get_wave(play_waveindex);
		if (wave) {
			const zzub::wave_level* l = _host->get_wave_level(play_waveindex, play_level);
			if (l) {
				bool looping = wave->flags&zzub_wave_flag_loop?true:false;
				unsigned int sample_count = l->sample_count;
				double samplespertick = (double)_master_info->samples_per_tick + (double)_master_info->samples_per_tick_frac;
				double samplepos = (double)_host->get_play_position() * samplespertick;
				currentPosition = (int)(samplepos+0.5f);
				triggered = (_host->get_state_flags() & zzub_player_state_flag_playing)?true:false;
			}
		}*/
	}

	if (triggered)
		source->resampler->set_stream_pos(current_position);

}

inline float sample_scale(zzub_wave_buffer_type format, void* buffer) {
	unsigned int i;
	switch (format) {
		case zzub_wave_buffer_type_si16:
			return static_cast<float>(*(short*)buffer) / 0x7fff;
		case zzub_wave_buffer_type_si24:
			i = (*(unsigned int*)buffer) & 0x00ffffff;
			if (i & 0x00800000) i = i | 0xFF000000;
			return static_cast<float>((int)i) / 0x007fffff;
		case zzub_wave_buffer_type_si32:
			return static_cast<float>(*(int*)buffer) / 0x7fffffff;
		case zzub_wave_buffer_type_f32:
			return *(float*)buffer;
		default:
			return 0;
	}
}

bool stream_wavetable::generate_samples(float** pout, int numsamples) {
	zzub::wave_info* wave = _mixer->waves.top()[source->play_wave_id].get();
	if (!wave) return false;

	zzub::wave_level* level = get_wavelevel(wave, source->play_level);
	if (!level) return false;

	bool looping = wave->flags&zzub_wave_flag_loop?true:false;
	unsigned int sample_count;
	if (current_end_offset != 0 && current_end_offset < (unsigned int)level->sample_count)
		sample_count = current_end_offset;
	else
		sample_count = level->sample_count;

	int maxread = numsamples;
	if (!looping && current_position + maxread > sample_count) 
		maxread = sample_count - current_position;
	
	if (maxread<=0) {
		return false;
	}

	float amp = wave->volume;

	char* sample_ptrc = (char*)level->samples;
	int bytes_per_sample = level->get_bytes_per_sample();
	int channels = (wave->flags & zzub_wave_flag_stereo)?2:1;
	zzub_wave_buffer_type format = (zzub_wave_buffer_type)level->format;

	sample_ptrc += (bytes_per_sample * channels) * current_position;

	for (int i = 0; i<maxread; i++) {
		pout[0][i] = sample_scale(format, sample_ptrc) * amp;
		sample_ptrc += bytes_per_sample;

		if (channels == 1) {
			pout[1][i] = pout[0][i]; 
		} else {
			pout[1][i] = sample_scale(format, sample_ptrc) * amp;
			sample_ptrc += bytes_per_sample;
		}

		if (looping && (int)current_position >= level->loop_end - 1) {
			current_position = level->loop_start;
			sample_ptrc = (char*)level->samples;
			sample_ptrc += (bytes_per_sample * channels) * current_position;
		} else
			current_position++;
	}
	return true;
}

int stream_wavetable::get_target_samplerate() {
	return _master_info->samples_per_second;
}

bool stream_wavetable::process_stereo(float **pin, float **pout, int numsamples, int mode) {
	if (mode == zzub_process_mode_read || mode == zzub_process_mode_no_io || !source || !source->resampler->playing) {
		pout[0] = 0;
		pout[1] = 0;
		return false;
	}

	return source->resampler->process_stereo(pout, numsamples);
}

void stream_wavetable::command(int index) {
	std::cout << "command " << index << std::endl;
	/*if (index >=256 && index<=512) {
		index -= 255;
		int selindex = 0;
		for (int i = 0; i < 0xC8; ++i) {
			const zzub::wave_info* wave = _host->get_wave(i+1);
			const zzub::wave_level* level = _host->get_wave_level(i+1, 0);
			if (level && level->sample_count > 0)
			{
				selindex++;
				if (selindex == index) {
					std::cout << _host->get_wave_name(i + 1) << std::endl;
					if (!level) return ;
					
					if (resampler)
						resampler->playing = false;
					this->play_waveindex = i+1;
					this->play_level = 0;
					this->currentPosition = 0;
					this->lastCurrentPosition = 0;
					char pc[256];
					
					reinit_resampler();
				}
			}
		}
	}*/
}

void stream_wavetable::get_sub_menu(int index, zzub::outstream* outs) {
	// print out all waves in the wavetable
	switch (index) {
		case 0:
			/*for (int i = 0; i<0xC8; i++) {
				const zzub::wave_info* wave = _host->get_wave(i+1);
				const zzub::wave_level* level = _host->get_wave_level(i+1, 0);
				if (level && level->sample_count > 0)
				{
					string name = "Wave " + stringFromInt(i+1, 2, ' ') + (string)": " + _host->get_wave_name(i + 1);
					outs->write(name.c_str());
				}
			}*/
			break;
	}
}


void stream_wavetable::stop() {
	if (source) source->resampler->playing = false;
}
