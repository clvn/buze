#define _USE_MATH_DEFINES
#include <cstring>
#include <cmath>
#if defined(USE_SNDFILE)
#include <sndfile.h>
#endif

#include "library.h"

namespace zzub {
	struct importplugin;
};

#define NO_ZZUB_WAVE_IMPORTER_TYPE
typedef zzub::importplugin zzub_wave_importer_t;

#include "player.h"
#include "waveimport.h"
#include "mixing/convertsample.h"

// Wave table

int zzub_wave_get_id(zzub_wave_t* wave) {
	return wave->datawave->id;
}

int zzub_wave_get_index(zzub_wave_t* wave) {
	return wave->datawave->get_wave_index();
}

const char* zzub_wave_get_name(zzub_wave_t* wave) {
	return wave->datawave->name.c_str();
}

void zzub_wave_set_name(zzub_wave_t* wave, const char* name) {
	wave->datawave->name = name;
	wave->datawave->update();
}

const char* zzub_wave_get_path(zzub_wave_t* wave) {
	return wave->datawave->filename.c_str();
}

void zzub_wave_set_path(zzub_wave_t* wave, const char* path) {
	wave->datawave->filename = path;
	wave->datawave->update();
}

int zzub_wave_get_flags(zzub_wave_t* wave) {
	return wave->datawave->flags;
}

void zzub_wave_set_flags(zzub_wave_t* wave, int flags) {
	wave->datawave->flags = flags;
	wave->datawave->update();
}

float zzub_wave_get_volume(zzub_wave_t* wave) {
	return wave->datawave->volume;
}

void zzub_wave_set_volume(zzub_wave_t* wave, float volume) {
	wave->datawave->volume = volume;
	wave->datawave->update();
}

int zzub_wavelevel_get_id(zzub_wavelevel_t* level) {
	return level->datawavelevel->id;
}

static sf_count_t instream_filelen (void *user_data) {
	zzub_input_t* strm = (zzub_input_t*)user_data ;
	return zzub_input_size(strm);
}

static sf_count_t instream_seek (sf_count_t offset, int whence, void *user_data) {
	zzub_input_t* strm = (zzub_input_t*)user_data ;
	zzub_input_seek(strm, (long)offset, (int)whence);
	return zzub_input_position(strm);
}

static sf_count_t instream_read (void *ptr, sf_count_t count, void *user_data){
	zzub_input_t* strm = (zzub_input_t*)user_data ;
	zzub_input_read(strm, (char*)ptr, (int)count);
	return count;
}

static sf_count_t instream_write (const void *ptr, sf_count_t count, void *user_data) {
	zzub_input_t* strm = (zzub_input_t*)user_data ;
	assert(false);
	return 0;
}

static sf_count_t instream_tell (void *user_data){
	zzub_input_t* strm = (zzub_input_t*)user_data ;
	return zzub_input_position(strm);
}

int zzub_wavelevel_load_wav(zzub_wavelevel_t* wavelevel, int offset, int clear, zzub_input_t* strm) {
#if defined(USE_SNDFILE)
	SF_INFO sfinfo;
	memset(&sfinfo, 0, sizeof(sfinfo));

	SF_VIRTUAL_IO vio;
	vio.get_filelen = instream_filelen ;
	vio.seek = instream_seek;
	vio.read = instream_read;
	vio.write = instream_write;
	vio.tell = instream_tell;

	SNDFILE* sf = sf_open_virtual(&vio, SFM_READ, &sfinfo, strm);
	if (!sf) {
		return -1;
	}

	zzub_wave_buffer_type format;
	switch (sfinfo.format & SF_FORMAT_SUBMASK) {
		case SF_FORMAT_PCM_U8: // convert anything 8 bit to 16-bit
		case SF_FORMAT_PCM_S8:
		case SF_FORMAT_PCM_16:
		case SF_FORMAT_IMA_ADPCM:
		case SF_FORMAT_MS_ADPCM:
		case SF_FORMAT_ALAW:
		case SF_FORMAT_ULAW:
		case SF_FORMAT_GSM610:
			format = zzub_wave_buffer_type_si16;
			break;
		case SF_FORMAT_PCM_24:
			format = zzub_wave_buffer_type_si24;
			break;
		case SF_FORMAT_PCM_32:
			format = zzub_wave_buffer_type_si32;
			break;
		case SF_FORMAT_FLOAT:
		case SF_FORMAT_DOUBLE:
			format = zzub_wave_buffer_type_f32;
			break;
		default:
			return -1;
	}

	int bytes_per_sample = sizeFromWaveFormat(format) * sfinfo.channels;
	char* buffer = new char[bytes_per_sample * sfinfo.frames];

	switch (format) {
		case zzub_wave_buffer_type_si16:
			sf_read_short(sf, (short*)buffer, sfinfo.frames * sfinfo.channels);
			break;
		case zzub_wave_buffer_type_si24:
			for (int i = 0; i < sfinfo.frames; i++) {
				float f[2];
				sf_read_float(sf, f, sfinfo.channels);
				CopySamples(&f, buffer, 1, zzub_wave_buffer_type_f32, zzub_wave_buffer_type_si24, 1, 1, 0, i * sfinfo.channels);
				if (sfinfo.channels == 2) 
					CopySamples(&f, buffer, 1, zzub_wave_buffer_type_f32, zzub_wave_buffer_type_si24, 1, 1, 1, (i * sfinfo.channels) + 1);
			}
			break;
		case zzub_wave_buffer_type_si32:
			sf_read_int(sf, (int*)buffer, sfinfo.frames * sfinfo.channels);
			break;
		case zzub_wave_buffer_type_f32:
			sf_read_float(sf, (float*)buffer, sfinfo.frames * sfinfo.channels);
			break;
	}

	sf_close(sf);

	// clear previous wave level contents (with undo)
	if (clear)
		zzub_wavelevel_clear(wavelevel);

	zzub_wave_t* wave = zzub_wavelevel_get_wave(wavelevel);
	int waveflags = zzub_wave_get_flags(wave);
	if (sfinfo.channels == 2) {
		if ((waveflags & zzub_wave_flag_stereo) == 0) {
			zzub_wave_set_flags(wave, waveflags | zzub_wave_flag_stereo);
		}
	} else {
		if ((waveflags & zzub_wave_flag_stereo) != 0) {
			zzub_wave_set_flags(wave, waveflags ^ zzub_wave_flag_stereo);
		}
	}

	zzub_wavelevel_set_format(wavelevel, (int)format);
	zzub_wavelevel_set_samples_per_second(wavelevel, sfinfo.samplerate);
	zzub_wavelevel_insert_sample_range(wavelevel, offset, buffer, sfinfo.channels, format, sfinfo.frames);

	return sfinfo.frames;
#else
	return 0;
#endif
}

void zzub_wavelevel_remove_sample_range(zzub_wavelevel_t* wavelevel, int start, int numsamples) {
	wavelevel->datawavelevel->delete_sample_range(start, numsamples);
}

void zzub_wavelevel_insert_sample_range(zzub_wavelevel_t* wavelevel, int start, void* buffer, int channels, int format, int numsamples) {
	// tell db layer to update the raw file, store undo data and generate an event
	// the mixer handles the event and retreives the sample bytes from the db-layer
	wavelevel->datawavelevel->insert_sample_range(buffer, start, numsamples, format, channels);
}

void zzub_wavelevel_replace_sample_range(zzub_wavelevel_t* wavelevel, int start, void* buffer, int channels, int format, int numsamples) {
	// tell db layer to update the raw file, store undo data and generate an event
	// the mixer handles the event and retreives the sample bytes from the db-layer
	wavelevel->datawavelevel->replace_sample_range(buffer, start, numsamples, format, channels);
}

int zzub_wavelevel_clear(zzub_wavelevel_t* level) {
	level->datawavelevel->delete_sample_range(0, level->datawavelevel->get_sample_count());
	return 0;
}

zzub_wave_t* zzub_wavelevel_get_wave(zzub_wavelevel_t* level) {
	return level->owner->waves[level->datawavelevel->wave_id].get();
}

int zzub_wave_clear(zzub_wave_t* wave) {
	// keep the wave itself, but delete all associated wavelevels and envelopes
	wave->datawave->clear();
	return 0;
}

zzub_wavelevel_t* zzub_wave_add_level(zzub_wave_t* wave) {
	armstrong::storage::waveleveldata data;
	wave->owner->songdata->create_wavelevel(wave->datawave->id, zzub_wave_buffer_type_si16, 44100, data);

	return wave->owner->wavelevels[data.id].get();
}

void zzub_wave_remove_level(zzub_wave_t* wave, int index) {
	armstrong::storage::wavelevel wavelevel(wave->owner->doc.get());
	wave->datawave->get_wavelevel_by_index(index, wavelevel);
	wavelevel.destroy();
}

int zzub_wave_get_level_count(zzub_wave_t* wave) {
	return wave->datawave->get_wavelevel_count();
}

zzub_wavelevel_t* zzub_wave_get_level(zzub_wave_t* wave, int index) {
	armstrong::storage::waveleveldata data;
	wave->datawave->get_wavelevel_by_index(index, data);
	return wave->owner->wavelevels[data.id].get();
}

int zzub_wave_get_envelope_count(zzub_wave_t* wave) {
	return wave->datawave->get_envelope_count();
}

void zzub_wave_set_envelope_count(zzub_wave_t* wave, int count) {
	assert(count >= 0);
	while (zzub_wave_get_envelope_count(wave) < count) {
		wave->datawave->add_envelope();
	}

	while (zzub_wave_get_envelope_count(wave) > count) {
		armstrong::storage::envelope env(wave->owner->doc.get());
		wave->datawave->get_envelope_by_index(zzub_wave_get_envelope_count(wave) - 1, env);
		env.destroy();
	}
}

zzub_envelope_t* zzub_wave_get_envelope(zzub_wave_t* wave, int index) {
	assert(index >= 0);
	armstrong::storage::envelopedata data;
	wave->datawave->get_envelope_by_index(index, data);
	return wave->owner->envelopes[data.id].get();
}

void zzub_wave_set_envelope(zzub_wave_t* wave, int index, zzub_envelope_t* env) {
	assert(false);
}

int zzub_wavelevel_get_sample_count(zzub_wavelevel_t* level) {
	return level->datawavelevel->get_sample_count();
}

void zzub_wavelevel_set_sample_count(zzub_wavelevel_t* level, int count) {
	zzub_wave_t* wave = zzub_wavelevel_get_wave(level);
	int waveflags = zzub_wave_get_flags(wave);
	int channels = (waveflags & zzub_wave_flag_stereo) != 0 ? 2 : 1;
	int format = zzub_wavelevel_get_format(level);
	int samplecount = level->datawavelevel->get_sample_count();
	if (count > samplecount) {
		int bytespersample = level->datawavelevel->get_bytes_per_sample();
		int diffbytes = (count - samplecount) * bytespersample;
		char* zerosamples = new char[diffbytes];
		memset(zerosamples, 0, diffbytes);
		zzub_wavelevel_insert_sample_range(level, samplecount, zerosamples, channels, format, count - samplecount);
		delete[] zerosamples;
	} else
	if (count < samplecount) {
		zzub_wavelevel_remove_sample_range(level, count, samplecount - count);
	}
}

int zzub_wavelevel_get_root_note(zzub_wavelevel_t* level) {
	return level->datawavelevel->basenote;
}

void zzub_wavelevel_set_root_note(zzub_wavelevel_t* level, int note) {
	level->datawavelevel->basenote = note;
	level->datawavelevel->update();
}

int zzub_wavelevel_get_samples_per_second(zzub_wavelevel_t* level) {
	return level->datawavelevel->samplerate;
}

void zzub_wavelevel_set_samples_per_second(zzub_wavelevel_t* level, int sps) {
	level->datawavelevel->samplerate = sps;
	level->datawavelevel->update();
}

int zzub_wavelevel_get_loop_start(zzub_wavelevel_t* level) {
	return level->datawavelevel->beginloop;
}

void zzub_wavelevel_set_loop_start(zzub_wavelevel_t* level, int pos) {
	level->datawavelevel->beginloop = pos;
	level->datawavelevel->update();
}

int zzub_wavelevel_get_loop_end(zzub_wavelevel_t* level) {
	return level->datawavelevel->endloop;
}

void zzub_wavelevel_set_loop_end(zzub_wavelevel_t* level, int pos) {
	level->datawavelevel->endloop = pos;
	level->datawavelevel->update();
}

int zzub_wavelevel_get_format(zzub_wavelevel_t* level) {
	return level->datawavelevel->format;
}

void zzub_wavelevel_set_format(zzub_wavelevel_t* level, int format) {
	// TODO: convert wavedata, update legacy format
	level->datawavelevel->format = format;
	level->datawavelevel->update();
}


#if defined(USE_SNDFILE)

static sf_count_t outstream_filelen (void *user_data) {
	zzub::outstream* strm = (zzub::outstream*)user_data ;
	int pos = strm->position();
	strm->seek(0, SEEK_END);
	int size = strm->position();
	strm->seek(pos, SEEK_SET);
	return size;
}

static sf_count_t outstream_seek (sf_count_t offset, int whence, void *user_data) {
	zzub::outstream* strm = (zzub::outstream*)user_data ;
	strm->seek(offset, whence);
	return strm->position();
}

static sf_count_t outstream_read (void *ptr, sf_count_t count, void *user_data){
	zzub::outstream* strm = (zzub::outstream*)user_data ;
	assert(false);
	return 0;
}

static sf_count_t outstream_write (const void *ptr, sf_count_t count, void *user_data) {
	zzub::outstream* strm = (zzub::outstream*)user_data ;
	return strm->write((void*)ptr, count);
}

static sf_count_t outstream_tell (void *user_data){
	zzub::outstream* strm = (zzub::outstream*)user_data ;
	return strm->position();
}

#endif

int zzub_wavelevel_save_wav_range(zzub_wavelevel_t* wavelevel, zzub_output_t* datastream, int start, int numsamples) {
#if defined(USE_SNDFILE)

	armstrong::storage::wave wave(wavelevel->owner->doc.get());
	wavelevel->owner->songdata->get_wave_by_id(wavelevel->datawavelevel->wave_id, wave);

	armstrong::storage::wavelevel& l = *wavelevel->datawavelevel.get();

	zzub::wave_level* mixlevel = wavelevel->owner->mix->wavelevels.next()[wavelevel->datawavelevel->id].get();

	//wave_level_ex& l = w.levels[level];

	int channels = (wave.flags & zzub_wave_flag_stereo) ?2:1;
	int result = -1;
	SF_INFO sfinfo;
	memset(&sfinfo, 0, sizeof(sfinfo));
	sfinfo.samplerate = l.samplerate;
	sfinfo.channels = channels;
	sfinfo.format = SF_FORMAT_WAV;
	switch (l.format) {
		case zzub_wave_buffer_type_si16:
			sfinfo.format |= SF_FORMAT_PCM_16;
			break;
		case zzub_wave_buffer_type_si24:
			sfinfo.format |= SF_FORMAT_PCM_24;
			break;
		case zzub_wave_buffer_type_si32:
			sfinfo.format |= SF_FORMAT_PCM_32;
			break;
		case zzub_wave_buffer_type_f32:
			sfinfo.format |= SF_FORMAT_FLOAT;
			break;
		default:
			return -1;
	}

	SF_VIRTUAL_IO vio;
	vio.get_filelen = outstream_filelen ;
	vio.seek = outstream_seek;
	vio.read = outstream_read;
	vio.write = outstream_write;
	vio.tell = outstream_tell;

	SNDFILE* sf = sf_open_virtual(&vio, SFM_WRITE, &sfinfo, datastream);
	if (!sf) {
		return -1;
	}

	int bytes_per_sample = sizeFromWaveFormat(l.format);
	char* sample_ptr = (char*)mixlevel->samples;
	sample_ptr += start * channels * bytes_per_sample;
	sf_write_raw(sf, sample_ptr, numsamples * channels * bytes_per_sample);
	sf_write_sync(sf);
	sf_close(sf); // so close it
	return 0;
#else
	fprintf(stderr,"zzub_wave_save_sample not implemented.\n");
	return -1;
#endif
}

int zzub_wavelevel_save_wav(zzub_wavelevel_t* wavelevel, zzub_output_t* datastream) {
	return zzub_wavelevel_save_wav_range(wavelevel, datastream, 0, wavelevel->datawavelevel->samplecount);
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

void zzub_wavelevel_get_samples_digest(zzub_wavelevel_t* level, int channel, int start, int end, float *mindigest, float *maxdigest, float *ampdigest, int digestsize) {
	zzub::wave_level* mixwl = level->owner->mix->wavelevels.next()[level->datawavelevel->id].get();
	assert(mixwl != 0);

	unsigned char *samples = (unsigned char*)mixwl->samples;
	if (!samples) return ;

	armstrong::storage::song song(level->owner->doc.get());
	level->owner->doc->get_song(song);

	armstrong::storage::wave wave(level->owner->doc.get());
	song.get_wave_by_id(level->datawavelevel->wave_id, wave);

	int channels = (wave.flags & zzub_wave_flag_stereo) ? 2 : 1;
	int bps = level->datawavelevel->get_bytes_per_sample() / channels;
	zzub_wave_buffer_type format = (zzub_wave_buffer_type)level->datawavelevel->format;
	int samplecount = level->datawavelevel->get_sample_count();
	int samplerange = end - start;
	assert((start >= 0) && (start < samplecount));
	assert((end > start) && (end <= samplecount));
	assert(samplerange > 0);
	float sps = (float)samplerange / (float)digestsize; // samples per sample
	float blockstart = (float)start;
	if (sps > 1)
	{
		for (int i = 0; i < digestsize; ++i) {
			float blockend = std::min(blockstart + sps, (float)end);
			float minsample = 1.0f;
			float maxsample = -1.0f;
			float amp = 0.0f;
			for (int s = (int)blockstart; s < (int)blockend; ++s) {
				int offset = (s*channels + channel)*bps;
				float sample = sample_scale(format, &samples[offset]);
				minsample = std::min(minsample, sample);
				maxsample = std::max(maxsample, sample);
				amp += sample*sample;
			}
			if (mindigest)
				mindigest[i] = minsample;
			if (maxdigest)
				maxdigest[i] = maxsample;
			if (ampdigest)
				ampdigest[i] = sqrtf(amp / (blockend - blockstart));
			blockstart = blockend;
		}
	}
	else
	{
		for (int i = 0; i < digestsize; ++i) {
			int s = (int)(blockstart + i * sps /* + 0.5f */);
			int offset = (s*channels + channel)*bps;
			float sample = sample_scale(format, &samples[offset]);
			if (mindigest)
				mindigest[i] = sample;
			if (maxdigest)
				maxdigest[i] = sample;
			if (ampdigest)
				ampdigest[i] = std::abs(sample);
		}
	}
}

void zzub_wavelevel_get_slices(zzub_wavelevel_t* wavelevel, int* numslices, int* slices) {
	if (!slices) {
		*numslices = wavelevel->datawavelevel->get_slices_count();
		return ;
	}
	std::vector<int> slicevector;
	wavelevel->datawavelevel->get_slices(slicevector);
	*numslices = (int)slicevector.size();
	if (slicevector.size())
		memcpy(slices, &slicevector.front(), slicevector.size() * sizeof(int));
}

void zzub_wavelevel_set_slices(zzub_wavelevel_t* wavelevel, int numslices, const int* slices) {
	std::vector<int> slicevector(numslices);
	if (numslices > 0)
		memcpy(&slicevector.front(), slices, numslices * sizeof(int));
	wavelevel->datawavelevel->set_slices(slicevector);
}

void zzub_wavelevel_process_sample_range_offline(zzub_wavelevel_t *wavelevel, int start, int insamples, zzub_plugin_t *plugin) {
	assert(wavelevel != 0);
	//assert(start < end);
	assert(start >= 0);
	assert(insamples > 0);
	assert(plugin != 0);
	assert((zzub_plugin_get_flags(plugin) & zzub_plugin_flag_is_offline) != 0);

	zzub_wave_t* wave = zzub_wavelevel_get_wave(wavelevel);
	int waveflags = zzub_wave_get_flags(wave);
	int samplecount = wavelevel->datawavelevel->get_sample_count();
	int samplerate = wavelevel->datawavelevel->samplerate;
	int channels = (waveflags & zzub_wave_flag_stereo) != 0 ? 2 : 1;
	//int insamples = (end - start) + 1;
	int outsamples = insamples;

	// find loop positions in the buffer for when selection extends beyond looping points (crossfading plugin)
	int beginloop = -1, endloop = -1;
	if (wavelevel->datawavelevel->beginloop >= start && wavelevel->datawavelevel->endloop < start+insamples) {
		beginloop = wavelevel->datawavelevel->beginloop - start;
		endloop = wavelevel->datawavelevel->endloop - start;
	}

	std::vector<int> slicevector;
	wavelevel->datawavelevel->get_slices(slicevector);	
	// determine final length
	plugin->userplugin->process_offline(0, 0, slicevector, &outsamples, &channels, &samplerate, beginloop, endloop);

	// convert from the internal format to f32 non-interleaved
	float** pin = new float*[channels];
	float** pout = new float*[channels];

	zzub::wave_level* mixwl = wavelevel->owner->mix->wavelevels.next()[wavelevel->datawavelevel->id].get();
	assert(mixwl != 0);

	unsigned char *samples = (unsigned char*)mixwl->samples;
	if (!samples) return ;
	zzub_wave_buffer_type format = (zzub_wave_buffer_type)wavelevel->datawavelevel->format;

	for (int i = 0; i < channels; i++) {
		pin[i] = new float[insamples];
		pout[i] = new float[outsamples];
		CopySamples(samples, pin[i], insamples, format, zzub_wave_buffer_type_f32, channels, 1, (start * channels) + i, 0);
	}

	bool result = plugin->userplugin->process_offline(pin, pout, slicevector, &insamples, &channels, &samplerate, beginloop, endloop);

	if (result) {
		// convert to interleaved f32 buffers
		float* ibuffer = new float[outsamples * channels];
		for (int i = 0; i < channels; i++) {
			CopySamples(pout[i], ibuffer, outsamples, zzub_wave_buffer_type_f32, zzub_wave_buffer_type_f32, 1, channels, 0, i);
			delete[] pin[i];
			delete[] pout[i];
		}

		// replace sample data
		if (insamples != outsamples) {
			zzub_wavelevel_remove_sample_range(wavelevel, start, insamples);
			zzub_wavelevel_insert_sample_range(wavelevel, start, ibuffer, channels, zzub_wave_buffer_type_f32, outsamples);
		} else {
			zzub_wavelevel_replace_sample_range(wavelevel, start, ibuffer, channels, zzub_wave_buffer_type_f32, outsamples);
		}
		delete[] ibuffer;

		// update and replace slices
		for (std::vector<int>::iterator i = slicevector.begin(); i != slicevector.end(); ++i)
			*i += start;
		wavelevel->datawavelevel->set_slices(slicevector);
	}
	delete[] pin;
	delete[] pout;
}

unsigned short zzub_envelope_get_attack(zzub_envelope_t *env) {
	return env->dataenvelope->attack;
}

unsigned short zzub_envelope_get_decay(zzub_envelope_t *env) {
	return env->dataenvelope->decay;
}

unsigned short zzub_envelope_get_sustain(zzub_envelope_t *env) {
	return env->dataenvelope->sustain;
}

unsigned short zzub_envelope_get_release(zzub_envelope_t *env) {
	return env->dataenvelope->release;
}

void zzub_envelope_set_attack(zzub_envelope_t *env, unsigned short attack) {
	env->dataenvelope->attack = attack;
	env->dataenvelope->update();
}

void zzub_envelope_set_decay(zzub_envelope_t *env, unsigned short decay) {
	env->dataenvelope->decay = decay;
	env->dataenvelope->update();
}

void zzub_envelope_set_sustain(zzub_envelope_t *env, unsigned short sustain) {
	env->dataenvelope->sustain = sustain;
	env->dataenvelope->update();
}

void zzub_envelope_set_release(zzub_envelope_t *env, unsigned short release) {
	env->dataenvelope->release = release;
	env->dataenvelope->update();
}

char zzub_envelope_get_subdivision(zzub_envelope_t *env) {
	return env->dataenvelope->subdivision;
}

void zzub_envelope_set_subdivision(zzub_envelope_t *env, char subdiv) {
	env->dataenvelope->subdivision = subdiv;
	env->dataenvelope->update();
}

char zzub_envelope_get_flags(zzub_envelope_t *env) {
	return env->dataenvelope->flags;
}

void zzub_envelope_set_flags(zzub_envelope_t *env, char flags) {
	env->dataenvelope->flags = flags;
	env->dataenvelope->update();
}

int zzub_envelope_is_enabled(zzub_envelope_t *env) {
	return (!env->dataenvelope->disabled)?1:0;
}

void zzub_envelope_enable(zzub_envelope_t *env, int enable) {
	env->dataenvelope->disabled = !enable;
	env->dataenvelope->update();
}

int zzub_envelope_get_point_count(zzub_envelope_t *env) {
	return env->dataenvelope->get_envelope_point_count();
}

void zzub_envelope_get_point(zzub_envelope_t *env, int index, unsigned short *x, unsigned short *y, char *flags) {
	armstrong::storage::envelopepointdata data;
	env->dataenvelope->get_point(index, data);
	if (x)
		*x = data.x;
	if (y)
		*y = data.y;
	if (flags)
		*flags = data.flags;
}

void zzub_envelope_set_point(zzub_envelope_t *env, int index, unsigned short x, unsigned short y, char flags) {
	armstrong::storage::envelopepoint data(env->owner->doc.get());
	env->dataenvelope->get_point(index, data);
	data.x = x;
	data.y = y;
	data.flags = flags;
	data.update();
	//assert(false);
/*	zzub::envelope_point *pt = &env->points[index];
	if (index == 0)
	{
		pt->x = 0;
	}
	else if (index == (env->points.size()-1))
	{
		pt->x = 65535;
	}
	else
	{
		zzub::envelope_point *ptprev = &env->points[index-1];
		zzub::envelope_point *ptnext = &env->points[index+1];
		pt->x = std::min(std::max(x,(unsigned short)(ptprev->x+1)), (unsigned short)(ptnext->x-1));
	}
	pt->y = y;
	pt->flags = flags;*/
}

void zzub_envelope_insert_point(zzub_envelope_t *env, int index) {
	armstrong::storage::envelopepoint data(env->owner->doc.get());
	env->dataenvelope->insert_point(0, 0, 0, data);

	//assert(false);
/*	index = std::max(std::min(index, (int)(env->points.size()-1)), 1); // must never insert before the first or after the last pt
	std::vector<zzub::envelope_point>::iterator pos = env->points.begin() + index;
	zzub::envelope_point pt = *pos;
	pt.flags = 0;
	env->points.insert(pos, pt);*/
}

void zzub_envelope_delete_point(zzub_envelope_t *env, int index) {
	assert(false);
/*	index = std::max(std::min(index, (int)(env->points.size()-1)), 1); // must never remove before the first or after the last pt
	std::vector<zzub::envelope_point>::iterator pos = env->points.begin() + index;
	env->points.erase(pos);*/
}









/** \brief Returns the number of registrered wave importers. */
int zzub_player_get_waveimporter_count(zzub_player_t *player) {
	zzub::waveimporter importer;
	return (int)importer.plugins.size();
}

/** \brief Returns the number of supported file extensions on a wave importer. */
int zzub_player_get_waveimporter_format_ext_count(zzub_player_t *player, int index) {
	zzub::waveimporter importer;
	return (int)importer.plugins[index]->extensions.size();
}

/** \brief Returns a supported file extension. */
const char *zzub_player_get_waveimporter_format_ext(zzub_player_t *player, int index, int extindex) {
	zzub::waveimporter importer;
	static char str[32];
	strcpy(str, importer.plugins[index]->extensions[extindex].c_str());
	return str;
}

int zzub_player_get_waveimporter_format_is_container(zzub_player_t *player, int index) {
	zzub::waveimporter importer;
	return importer.plugins[index]->is_container() ? 1 : 0;
}

int zzub_player_get_waveimporter_format_type(zzub_player_t *player, int index) {
	zzub::waveimporter importer;
	return importer.plugins[index]->get_type();
}

/** \brief Creates a waveimporter instance */
zzub_wave_importer_t *zzub_player_create_waveimporter(zzub_player_t *player, int index) {
	zzub::waveimporter importer;
	return importer.plugins[index]->create_importer();
}

/** \brief Convenience function for creating a waveimporter from a filename */
zzub_wave_importer_t *zzub_player_create_waveimporter_by_file(zzub_player_t *player, const char *filename) {
	zzub::waveimporter importer;
	zzub::importplugin* implug = importer.open(filename);
	return implug;
}

/* class WaveImporter */

/*bool*/ int zzub_wave_importer_open(zzub_wave_importer_t *waveimporter, const char *filename, zzub_input_t *strm) {
	assert(false);
	return 0;
}

void zzub_wave_importer_destroy(zzub_wave_importer_t *waveimporter) {
	waveimporter->close();
	delete waveimporter;
}

int zzub_wave_importer_get_instrument_count(zzub_wave_importer_t *waveimporter) {
	return waveimporter->get_wave_count();
}

const char* zzub_wave_importer_get_instrument_name(zzub_wave_importer_t *waveimporter, int index) {
	std::string namestr = waveimporter->get_wave_name(index);
	static char name[512];
	strncpy(name, namestr.c_str(), 512);
	return name;
}

int zzub_wave_importer_get_instrument_sample_count(zzub_wave_importer_t *waveimporter, int index) {
	return waveimporter->get_wave_level_count(index);
}

void zzub_wave_importer_get_instrument_sample_info(zzub_wave_importer_t *waveimporter, int index, int sample, char *name, int namelen, int *samplecount, int *channels, int *format, int *samplerate) {
	zzub::importwave_info info;
	if (!waveimporter->get_wave_level_info(index, sample, info))
		return ; // TODO: return fail!
	*samplerate = info.samples_per_second;
	*samplecount = info.sample_count;
	*channels = info.channels;
	*format = info.format;
	strncpy(name, info.name.c_str(), namelen);
}

/*bool*/ int zzub_wave_importer_load_instrument(zzub_wave_importer_t *waveimporter, int instrument, zzub_wave_t *wave) {
	assert(false);
	// TODO: loop samples in instrument and add new wavelevels
	return 0;
}

/*bool*/ int zzub_wave_importer_load_instrument_sample(zzub_wave_importer_t *importer, int instrument, int sample, zzub_wavelevel_t *wavelevel) {

	zzub::importwave_info wavedata;
	if (!importer->get_wave_level_info(instrument, sample, wavedata)) {
		return false;
	}

	assert(wavedata.channels != 0);

	int bytes_per_sample = sizeFromWaveFormat(wavedata.format) * wavedata.channels;
	char* buffer = new char[bytes_per_sample * wavedata.sample_count];

	importer->read_wave_level_samples(instrument, sample, buffer);

	zzub_wavelevel_clear(wavelevel);

	zzub_wave_t* wave = zzub_wavelevel_get_wave(wavelevel);
	int waveflags = zzub_wave_get_flags(wave);
	if (wavedata.channels == 2) {
		if ((waveflags & zzub_wave_flag_stereo) == 0) {
			zzub_wave_set_flags(wave, waveflags | zzub_wave_flag_stereo);
		}
	} else {
		if ((waveflags & zzub_wave_flag_stereo) != 0) {
			zzub_wave_set_flags(wave, waveflags ^ zzub_wave_flag_stereo);
		}
	}

	zzub_wavelevel_set_format(wavelevel, (int)wavedata.format);
	zzub_wavelevel_set_samples_per_second(wavelevel, wavedata.samples_per_second);
	zzub_wavelevel_insert_sample_range(wavelevel, 0, buffer, wavedata.channels, wavedata.format, wavedata.sample_count);

	return true; //wavedata.sample_count;
}

