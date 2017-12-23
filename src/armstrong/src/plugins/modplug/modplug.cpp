#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
};

typedef struct zzub::mixer zzub_mixer_t;
#include "mixing/mixer.h"
#include <zzub/plugin.h>
#include "plugins.h"
#include "modplug.h"
#include "modpluglib/stdafx.h"
#include "modpluglib/Sndfile.h"
#include <iostream>
using std::cout;
using std::endl;

struct pluginmixer {
	zzub::mixer* _mixer;

	const std::vector<boost::shared_ptr<zzub::wave_info> >& waves();
	const std::vector<boost::shared_ptr<zzub::wave_level> >& wavelevels();
	zzub::wave_level* wavelevel(zzub::wave_info* wave, int index);
};


const std::vector<boost::shared_ptr<zzub::wave_info> >& pluginmixer::waves() {
	if (_mixer->in_user_thread())
		return _mixer->waves.next();
	return _mixer->waves.top();
}

const std::vector<boost::shared_ptr<zzub::wave_level> >& pluginmixer::wavelevels() {
	if (_mixer->in_user_thread())
		return _mixer->wavelevels.next();
	return _mixer->wavelevels.top();
}

zzub::wave_level* pluginmixer::wavelevel(zzub::wave_info* wave, int index) {
	int count = 0;
	for (std::vector<boost::shared_ptr<zzub::wave_level> >::const_iterator i = wavelevels().begin(); i != wavelevels().end(); ++i) {
		if (*i == 0) continue;
		if ((*i)->wave_id == wave->id) {
			if (count == index) return i->get();
			count++;
		}
	}
	return 0;
}

namespace {
int buzz_to_modplug_note(int value) {
	if (value == 255 || value == 254) return value;
	return 12 * (value >> 4) + (value & 0xf) - 1;
}
}

class CWavetableManager : public CWaveTable {
public:
	pluginmixer _mixer;
	MODINSTRUMENT* instruments[MAX_INSTRUMENTS];
	MODSAMPLE samples[MAX_SAMPLES];

	CWavetableManager();
	virtual MODINSTRUMENT* GetInstrument(UINT n);
	virtual MODSAMPLE* GetSample(UINT n);
};

CWavetableManager::CWavetableManager() {
	memset(samples, 0, sizeof(samples));
}

MODINSTRUMENT* CWavetableManager::GetInstrument(UINT n) {
	return 0;
}

MODSAMPLE* CWavetableManager::GetSample(UINT n) {
	// TODO: ??
	if (n > 200) n = 0;
	zzub::wave_info* wave = _mixer.waves()[n].get();
	if (wave) {
		zzub::wave_level* wavelevel = _mixer.wavelevel(wave, 0);
		if (wavelevel) {
			int flags = 0;
			flags |= CHN_16BIT;
			if ((wave->flags & zzub_wave_flag_stereo) != 0)
				flags |= CHN_STEREO;
			if ((wave->flags & zzub_wave_flag_loop) != 0)
				flags |= CHN_LOOP;
			samples[n].uFlags = flags;
			samples[n].nVolume = (int)(wave->volume * 256.0f);
			samples[n].nGlobalVol = 64;
			samples[n].nPan = 128;
			samples[n].nC5Speed = wavelevel->samples_per_second;
			samples[n].pSample = (LPSTR)wavelevel->samples;
			samples[n].nLength = wavelevel->sample_count;
			samples[n].nLoopStart = wavelevel->loop_start;
			samples[n].nLoopEnd = wavelevel->loop_end;
			return &samples[n];
		}
	} 
	memset(&samples[n], 0, sizeof(MODSAMPLE));
	return &samples[n];
}

struct aval {
	int modtype;
	int resample;
};

struct tval {
	unsigned char note;
	unsigned char wave;
	unsigned char volfx;
	unsigned char volume;
	unsigned char fx;
	unsigned char fxvalue;
};


struct modplug_plugin : zzub::plugin {
	tval tvals[modplug_plugin_info::modplug_max_tracks];
	aval avals;
	CWavetableManager wavetable;
	CSoundFile sndfile;
	int track_count;
	MODCOMMAND commands[modplug_plugin_info::modplug_max_tracks];

	modplug_plugin();
	virtual void init(zzub::archive* arc);
	virtual void process_events();
	virtual bool process_stereo(float** pin, float** pout, int numsamples, int mode);
	virtual void set_track_count(int i);
	virtual void update_timesource();
	virtual void attributes_changed();
	virtual void stop();
};

modplug_plugin::modplug_plugin() : sndfile(&wavetable) {
	attributes = (int*)&avals;
	track_values = &tvals;
}

void modplug_plugin::init(zzub::archive* arc) {
	wavetable._mixer._mixer = _mixer;
	sndfile.Create(0, 0);
	sndfile.SetResamplingMode(SRCMODE_SPLINE);
}

void modplug_plugin::update_timesource() {
	cout << "modplug_plugin::update_timesource()" << endl;
	sndfile.SetTempo(_master_info->beats_per_minute);
	sndfile.SetSpeed(floor(6.0f * 4.0f / _master_info->ticks_per_beat + 0.5f));
	sndfile.SetWaveConfig(_master_info->samples_per_second, 16, 2);
}

bool modplug_plugin::process_stereo(float** pin, float** pout, int numsamples, int mode) {
	float zerobuffer[2][zzub_buffer_size] = {0.0f};
	float* plout[2] = { 
		pout[0] ? pout[0] : zerobuffer[0],  
		pout[1] ? pout[1] : zerobuffer[1]
	};
	sndfile.ProcessSamples(plout, numsamples);
	return true;
}

void modplug_plugin::attributes_changed() {
	MODTYPE modtype;
	unsigned int flags = sndfile.m_dwSongFlags;
	flags &= ~SONG_PT1XMODE;
	flags &= ~SONG_FASTVOLSLIDES;
	flags &= ~SONG_LINEARSLIDES;
	switch (avals.modtype) {
		case 0:
			flags |= SONG_LINEARSLIDES;
			flags |= SONG_FASTVOLSLIDES;
			flags |= SONG_PT1XMODE;
			modtype = TRK_ALLTRACKERS;
			break;
		case 1:
			flags |= SONG_LINEARSLIDES;
			modtype = TRK_IMPULSETRACKER;
			break;
		case 2:
			flags |= SONG_LINEARSLIDES;
			modtype = TRK_FASTTRACKER2;
			break;
		case 3:
			flags |= SONG_FASTVOLSLIDES;
			modtype = TRK_SCREAMTRACKER;
			break;
		case 4:
			flags |= SONG_PT1XMODE;
			modtype = TRK_PROTRACKER;
			break;
		default:
			assert(false);
			modtype = TRK_ALLTRACKERS;
			break;
	}
	sndfile.m_dwSongFlags = flags;
	sndfile.ChangeModTypeTo(modtype);

	UINT resamplemode;
	switch (avals.resample) {
		case 0:
			resamplemode = SRCMODE_NEAREST;
			break;
		case 1:
			resamplemode = SRCMODE_LINEAR;
			break;
		case 2:
			resamplemode = SRCMODE_SPLINE;
			break;
		case 3:
			resamplemode = SRCMODE_POLYPHASE;
			break;
		case 4:
			resamplemode = SRCMODE_FIRFILTER;
			break;
		default:
			assert(false);
			resamplemode = SRCMODE_SPLINE;
			break;
	}
	sndfile.SetResamplingMode(resamplemode);
}

void modplug_plugin::process_events() {
	memset(commands, 0, sizeof(commands));
	for (int i = 0; i < track_count; i++) {
		if (tvals[i].note != zzub_note_value_none)
			commands[i].note = buzz_to_modplug_note(tvals[i].note);
		if (tvals[i].wave != 255)
			commands[i].instr = tvals[i].wave;
		if (tvals[i].volfx != 255)
			commands[i].volcmd = tvals[i].volfx;
		if (tvals[i].volume != 255)
			commands[i].vol = tvals[i].volume;
		if (tvals[i].fx != 255)
			commands[i].command = tvals[i].fx;
		if (tvals[i].fxvalue != 255)
			commands[i].param = tvals[i].fxvalue;
	}
	sndfile.ProcessEvents(commands);
}

void modplug_plugin::set_track_count(int i) {
	track_count = i;
	sndfile.m_nChannels = i;
	sndfile.SetupMODPanning();

		// Adjust channels
	for (UINT ich=0; ich<MAX_BASECHANNELS; ich++)
	{
		sndfile.Chn[ich].nPan = sndfile.ChnSettings[ich].nPan;
		sndfile.Chn[ich].nGlobalVol = sndfile.ChnSettings[ich].nVolume;
	}
}

void modplug_plugin::stop() {
	BYTE resetMask = CHNRESET_TOTAL;// (!nPos) ? CHNRESET_SETPOS_FULL : CHNRESET_SETPOS_BASIC;

	for (CHANNELINDEX i=0; i<MAX_CHANNELS; i++)
		sndfile.ResetChannelState(i, resetMask);
	//sndfile.ResetChannels();
}

zzub::plugin* modplug_plugin_info::create_plugin() {
	return new modplug_plugin();
}

