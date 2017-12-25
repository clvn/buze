

// TODO
/*


  envelope temposync
  Ft2 envelopes (eg. fadeout but keep looping)
  filter balance per osc

  lfo->lfo speed mod
  glide less sensitive at the low end

*/

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <zzub/signature.h>
#include "zzub/plugin.h"

#include <stdio.h>

#include "fungus.h"
#include "lib.h"
#include "pinknoise.h"

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(__emulu)
#endif

#define LINEAR_INTERPOLATION_OSCS
//#define NONLINEAR_VCA

typedef unsigned char byte;

int envtimescale = 2;

#pragma optimize ("a", on)

const zzub::parameter *paraGain = 0;
const zzub::parameter *paraPitchbend = 0;
const zzub::parameter *paraPitchbendRange = 0;
const zzub::parameter *paraNote = 0;
const zzub::parameter *paraWaveForm1 = 0;
const zzub::parameter *paraO1PhaseShift = 0;
/*const zzub::parameter *paraO1PhaseModType = 0;*/
const zzub::parameter *paraWaveForm2 = 0;
const zzub::parameter *paraO2PhaseShift = 0;
/*const zzub::parameter *paraO2PhaseModType = 0;*/
const zzub::parameter *paraDeTuneSemi = 0;
const zzub::parameter *paraDeTuneFine = 0;
const zzub::parameter *paraSync = 0;
const zzub::parameter *paraFMmode = 0;
const zzub::parameter *paraFM = 0;
const zzub::parameter *paraFMEnvMod = 0;
const zzub::parameter *paraOscSpread = 0;
const zzub::parameter *paraMix = 0;
const zzub::parameter *paraMixEnvMod = 0;
const zzub::parameter *paraMixMorph = 0;
const zzub::parameter *paraMixMode = 0;
const zzub::parameter *paraSubOscWave = 0;
const zzub::parameter *paraSubOscVol = 0;
const zzub::parameter *paraAmpMorph = 0;
const zzub::parameter *paraPitchEnvMod = 0;
const zzub::parameter *paraPitchMorph = 0;
const zzub::parameter *paraPMmode = 0;
const zzub::parameter *paraFMMorph = 0;
const zzub::parameter *paraCutMorph1 = 0;
const zzub::parameter *paraCutMorph2 = 0;
const zzub::parameter *paraFilterType1 = 0;
const zzub::parameter *paraCutoff1 = 0;
const zzub::parameter *paraResonance1 = 0;
const zzub::parameter *paraFilterKeytrack1 = 0;
const zzub::parameter *paraFilterEnvMod1 = 0;
const zzub::parameter *paraDistType = 0;
const zzub::parameter *paraDistAmount = 0;
const zzub::parameter *paraFilterType2 = 0;
const zzub::parameter *paraCutoff2 = 0;
const zzub::parameter *paraResonance2 = 0;
const zzub::parameter *paraFilterKeytrack2 = 0;
const zzub::parameter *paraFilterEnvMod2 = 0;
const zzub::parameter *paraFilterSetup = 0;
const zzub::parameter *paraPrecision = 0;
const zzub::parameter *paraVolume = 0;
const zzub::parameter *paraDelay = 0;
const zzub::parameter *paraPan = 0;
const zzub::parameter *paraCut = 0;
const zzub::parameter *paraMorph = 0;
const zzub::parameter *paraLFO1Waveform = 0;
const zzub::parameter *paraLFO1Freq = 0;
const zzub::parameter *paraLFO1Phase = 0;
const zzub::parameter *paraLFO1Amplitude = 0;
const zzub::parameter *paraLFO1AmpEnvMod = 0;
const zzub::parameter *paraLFO1Target = 0;
const zzub::parameter *paraLFO1Mode = 0;
const zzub::parameter *paraLFO2Waveform = 0;
const zzub::parameter *paraLFO2Freq = 0;
const zzub::parameter *paraLFO2Phase = 0;
const zzub::parameter *paraLFO2Amplitude = 0;
const zzub::parameter *paraLFO2AmpEnvMod = 0;
const zzub::parameter *paraLFO2Target = 0;
const zzub::parameter *paraLFO2Mode = 0;
const zzub::parameter *paraGlide = 0;
const zzub::parameter *paraGlideRetrig = 0;
const zzub::parameter *paraNumOsc = 0;
const zzub::parameter *paraDetune = 0;
const zzub::parameter *paraDetunePan = 0;
const zzub::parameter *paraDetuneMode = 0;

#pragma pack(1)
struct funguslfovals {
	byte wave;
	byte freq;
	byte phase;
	byte amp;
	byte ampenvmod;
	byte target;
	byte mode;
};

class fungusgvals
{
public:
	byte gain;

	byte pitchbend;
	byte pitchbendrange;

	byte wave1;
	byte p1shift;
	/*byte p1modtype;*/

	byte wave2;
	byte p2shift;
	/*byte p2modtype;*/

	byte detunesemi;
	byte detunefine;

	byte sync;

	byte fmmode;
	byte fmamount;
	//byte fmmorph;
	byte fmenvmod;

	byte spread;
	byte mix;
	byte mixenvmod;
	//byte mixmorph;
	byte mixmode;

	byte subwave;
	byte subosc;

	//byte pmorph;
	byte pmode;
	byte penvmod;

	byte glide;
	byte retrig;

	byte filtertype1;
	byte cutoff1;
	byte resonance1;
	//byte fmorph1;
	byte keytrack1;
	byte fenvmod1;

	byte disttype;
	byte distamount;

	byte filtertype2;
	byte cutoff2;
	byte resonance2;
	//byte fmorph2;
	byte keytrack2;
	byte fenvmod2;

	byte filtersetup;

//	byte prec;

	//byte ampmorph;

	funguslfovals lfo[2];

	byte numosc;
	byte detune;
	byte dtpan;
	byte dtmode;
};

class fungustvals	
{
public:
	byte note;
	byte volume;
	byte delay;
	byte cut;
	byte pan;
	//byte morph;
};
#pragma pack()

static float *fmtab = 0;
static float *lfotab = 0;

#include "envelope.h"
#include "filter2p.h"
#include "filter4p.h"

#define DETUNESEMI 1.05946309435929526
#define DETUNEFINE 1.00091728179580156

float tabsizediv = powf(2, 32) / 88200.f;

static double *filter2pcoefsTab = 0;
static double *filter4pcoefsTab = 0;

class distortion {
private:
	float distamount;
	float ooamount;
public:

	void precalc(int type, float amount) {
		switch(type) {
			case 0:
			default:
				distamount = 1.0;
				break;
			case 1:
			{
				float x = clamp2(amount);
				distamount = 1.0 / (x - 0.3333*x*x*x);
				break;
			}
			case 2:
				distamount = 1.0 / (otanh(amount));
				break;
			case 3:
				distamount = 1.0 / (atan(amount));
				break;
		}

		ooamount = 16384.0 * distamount;
	}

	void dodistortion(float *psamples, int numsamples, int type, float amount) {

		amount = amount / 16384.0;

		switch(type) {
			case 0:
			default:
				/* no distortion */
				break;
			case 1:
			{
				/* x-1/3x^3 */
				for(int i = 0; i < numsamples; i++) {
					float x, y;
					x = clamp2(*psamples * amount);
					y = x - 0.3333*x*x*x;
					*psamples = y * ooamount;
					psamples++;
					x = clamp2(*psamples * amount);
					y = x - 0.3333*x*x*x;
					*psamples = y * ooamount;
					psamples++;
				}
				break;
			}
			case 2:
			{
				/* tanh */
				for(int i = 0; i < numsamples; i++) {
					float x, y;
					x = *psamples * amount;
					y = otanh(x);
					*psamples = y * ooamount;
					psamples++;
					x = *psamples * amount;
					y = otanh(x);
					*psamples = y * ooamount;
					psamples++;
				}
				break;
			}
			case 3:
			{
				/* atan */
				for(int i = 0; i < numsamples; i++) {
					float x, y;
					x = *psamples * amount;
					y = atan(x);
					*psamples = y * ooamount;
					psamples++;
					x = *psamples * amount;
					y = atan(x);
					*psamples = y * ooamount;
					psamples++;
				}
				break;
			}
		};
	}
};

class fungus;

class tracklfo {
public:
	unsigned int p;

	short value;
	short prevvalue;
	int valuediff;

	EnvelopeState e;
};

class lfohandler {
public:
	unsigned int p_add;
	unsigned int phase;

	short *waveform;
	int wave;
	int target;
	int amp;

	int envmod;
	int mode;
};

class FungusTrack {
public:

	FungusTrack()
	{
		active=false;
		f1 = -1;
	}

	void Init(void);

	void Tick(int note, int volume, int delay, int cut, int pan/*, int morph*/);
	void DoTick();
	bool dofilters(float *psamples, int numsamples, bool gotinput, float pitch);
	void WorkLFO(int numsamples);
	bool Work(float *psamples, int numsamples);
	void Stop(void);

	bool filter1quiet;
	bool filter2quiet;

	fungus *pmi;

	// oscillator 1
	short *osc1;

	// oscillator 2
	short *osc2;

	// subosc
	short *subosc;

	EnvelopeState e;
	EnvelopeState fme;
	EnvelopeState fe1, fe2;
	filter2p flt1, flt2;
	filter4p flt1b, flt2b;
	EnvelopeState pe;
	EnvelopeState me;

	PinkNoise pn1, pn2;

	float f1, f2;
	unsigned int p1[16];//, a1;
	unsigned int p2[16];//, a2;
	unsigned int psub[16];//, a2;

	float GlideMul, GlideFactor;
    int GlideCount;

	tracklfo lfo[2];

	/* lfo vars */
	float cutoff1, resonance1;
	float fenvmod1;
	float cutoff2, resonance2;
	float fenvmod2;
	float pitchlfo1, pitchlfo2;
	float mixres;
	float lfovol;
	unsigned int phase_add_1;
	unsigned int phase_add_2;
	int fmamount;

	float vol;

	int curcut;
	
	int notedelay;
	int newnote;
	int newvol;
	int newcut;
	int newpan;
	//int newmorph;

	float temporarybuffer[256];
	float temporarybuffer2[256];

	float pan;

	//float morph;

	unsigned char Note;
	
	bool active;
};

class fungus : public zzub::plugin
{
public:
	fungus();
	virtual ~fungus();

public:
	fungustvals tval[16];
	fungusgvals gval;

	distortion dist;
	int disttype;
	float distamount;

	// global gain
	float gain;
	float gaintarget;

	// pitchbend
	double pitchbend;
	double pitchbendmul;
	double pitchbendtarget;
	int pitchbendtime;
	double pitchbendrange;

	// oscillator 1
	short *osc1;
	unsigned int phase_add_1_base;
	int phasemodtype_1;

	// oscillator 2
	short *osc2;
	unsigned int phase_add_2_base;
	int phasemodtype_2;

	bool noise1, noise2;

	short *subosc;
	float subvol;

	int glidetime;
	int retrig;

	int fmmode;
	int fmamount_base;
	int fmenvmod;

	float detunesemi, detunefine;

	float spread;

	int mix;

	int mixenabled;
	float mixenvmod;
	//float mixmorph;

	int mixmode;

	int filtertype1;
	float cutoff_base1, resonance_base1;
	float cutoff_base1add, cutoff_base1time;
	//float amorph, fmorph1, fmorph2, pmorph, fmmorph;
	float fenvmod_base1;
	float penvmod;
	int pmode;

	int filtertype2;
	float cutoff_base2, resonance_base2;
	float cutoff_base2add, cutoff_base2time;
	float fenvmod_base2;

	float keytrack1;
	float keytrack2;

	int filtersetup;
	float filterlevel;

	float dtmul[16];
	float dtpos[16];
	float dtdetune;
	float dtpan;
	int dtmode;
	int dtosc;
	float outputmul;

	Envelope ampenv, pitchenv, cutoffenv1, cutoffenv2, fmenv, mixenv, lfo1env, lfo2env;

	short r1, r2, r3, r4;

	short noise(void) {
		short nse = r1 + r2 + r3 + r4;
		r1 = r2; r2 = r3; r3 = r4; r4 = nse;

		return nse;
	}

	lfohandler lfo[2];

	int sync;

	int prec;

protected:

	Decimateur5 dc5, dc5_r;

	int numTracks;
	FungusTrack t[16];

	bool active;

	float renderbuf[1024 * 2];

	inline float envtime(int v) {
		return (float)(powf(((float)v+2.0f)/(127.0f+2.0f), 3.f)*10.f * 2.f);
	}

public:
	virtual void init(zzub::archive* pi);
	virtual void process_events();
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode);
	virtual const char * describe_value(int param, int value); 

	// ::zzub::plugin methods
	virtual void destroy() { delete this; }
	virtual void stop();
	virtual void load(zzub::archive*);
	virtual void save(zzub::archive*);
	virtual void set_track_count(int count) { numTracks = count; }

/*
	virtual void AttributesChanged(void) {
		tabsizediv = powf(1.05946309435929526f, (float)(aval.semidetune - 0x40)) *
					 powf(1.00091728179580156f, (float)(aval.finedetune - 0x40)) *
					 powf(2, 32) / 88200.f;
	}
*/

	virtual void command(int i) {
		switch(i) {
		case 0:
			ShowEnvelope(&ampenv);
			break;
		case 1:
			ShowEnvelope(&cutoffenv1);
			break;
		case 2:
			ShowEnvelope(&cutoffenv2);
			break;
		case 3:
			ShowEnvelope(&fmenv);
			break;
		case 4:
			ShowEnvelope(&pitchenv);
			break;
		case 5:
			ShowEnvelope(&mixenv);
			break;
		case 6:
			ShowEnvelope(&lfo1env);
			break;
		case 7:
			ShowEnvelope(&lfo2env);
			break;
		case 8:
#ifdef _WIN32
			MessageBox(NULL, "ld fungus v0.41 preview\n(C) Lauri Koponen 2007\n\nBased loosely on ld padsyn", "About ld fungus", MB_OK);
#else
			printf("ld fungus v0.41 preview\n(C) Lauri Koponen 2007\n\nBased loosely on ld padsyn");
#endif
			break;
		}
	}

};

#include "waves/waves.h"
#include "fft.h"

void select_wave(int wavenum)
{
	float re[4097], im[4097];
	int i, j;
	short *d = wavetable + wavenum * 2048;
	static int calculated[66] = {0};

	calculated[0] = 1;
	calculated[1] = 1;
	calculated[2] = 1;
	calculated[3] = 1;

	if(calculated[wavenum]) return;
	calculated[wavenum] = 1;

	memcpy(d, waves[wavenum], 2048*2);

	memset(re, 0, 4097*4);
	memset(im, 0, 4097*4);
	for(j = 0; j < 2048; j++)
		re[j] = d[j] * (1.f / 32768.f);

	int k;
	for(i = 0; i < 9; i++) {
		fft(re, im, 10);

		k = 512 >> i;

		for(j = k; j < (4097-k); j++) {
			re[j] = 0;
			im[j] = 0;
		}

		inverse_fft(re, im, 10);
		for(j = 0; j < 2048; j++)
			(d + i * 66*2048)[j] = re[j] * 32768.f;
	}
	
}

void FungusTrack::Init(void)
{
	flt1.init(pmi->_master_info->samples_per_second * 2, &filter2pcoefsTab);
	flt2.init(pmi->_master_info->samples_per_second * 2, &filter2pcoefsTab);
	flt1b.init(pmi->_master_info->samples_per_second * 2, &filter4pcoefsTab);
	flt2b.init(pmi->_master_info->samples_per_second * 2, &filter4pcoefsTab);
}

void FungusTrack::Tick(int note, int volume, int delay, int cut, int pan/*, int morph*/)
{
	notedelay = (delay * pmi->_master_info->samples_per_tick) >> 6;
	newnote = note;
	newvol = volume;
	newcut = cut;
	newpan = pan;
	//newmorph = morph;

	if(notedelay == 0) DoTick();
}

void FungusTrack::DoTick()
{
	notedelay = 0x7FFFFFFF;

	if(newvol != 255)
		vol = (float)newvol * (1.f / 128.f);

	if(newcut != -1) {
		curcut = (newcut * pmi->_master_info->samples_per_tick) >> 6;
		newcut = -1;
	}

	if(newpan != 0xff) {
		pan = (float)newpan * (1.f / 64.f) - 1.0;
	}

	/*if(newmorph != 0xff) {
		morph = (float)newmorph * (1.f / 128.f);
	}*/

	if(newnote == 255) {
		Note = zzub_note_value_none;
		curcut = -1;
		e.End();
		fe1.End();
		fe2.End();
		pe.End();
		fme.End();
		me.End();
	}
	else if(newnote) {	

		int prevnote = Note;
		Note = newnote;

		if(prevnote == zzub_note_value_none || pmi->retrig || !pmi->glidetime) {
			TriggerEnvelope(&pmi->ampenv, &e, 1 - vol, false, pmi->_master_info->samples_per_second);
			TriggerEnvelope(&pmi->fmenv, &fme, 1 - vol, false, pmi->_master_info->samples_per_second);
			TriggerEnvelope(&pmi->cutoffenv1, &fe1, 1 - vol, false, pmi->_master_info->samples_per_second);
			TriggerEnvelope(&pmi->cutoffenv2, &fe2, 1 - vol, false, pmi->_master_info->samples_per_second);
			TriggerEnvelope(&pmi->pitchenv, &pe, 1 - vol, false, pmi->_master_info->samples_per_second);
			TriggerEnvelope(&pmi->mixenv, &me, 1 - vol, false, pmi->_master_info->samples_per_second);
			TriggerEnvelope(&pmi->lfo1env, &lfo[0].e, 1 - vol, false, pmi->_master_info->samples_per_second);
			TriggerEnvelope(&pmi->lfo2env, &lfo[1].e, 1 - vol, false, pmi->_master_info->samples_per_second);
		}

		if(GlideCount) {
			f1 = f1 * GlideFactor;
		}

		f2 = 440.0*powf(2.f,(float)((newnote>>4)*12+(newnote&0x0f)-70)/12.0f - 0.0025 + (float)rand() * (0.005 / RAND_MAX));

		if(f1 == -1) {
			f1 = f2;
		}

		if(pmi->glidetime) {

			if(f1 < f2)
				GlideMul = powf(2.0f, 1.0f / pmi->glidetime);
			else
				GlideMul = powf(0.5f, 1.0f / pmi->glidetime);

			GlideFactor = 1;
			GlideCount = (int)(log(f2 / f1) / log(GlideMul));

		} else {
			f1 = f2;
			GlideCount = 0;
		}

		if(pmi->sync == 1) {
			for(int i = 0; i < 16; i++) {
//				p1[i] = 0;
				p2[i] = p1[i];
//				psub[i] = 0;
			}
		} else if(pmi->sync == 3) {
			for(int i = 0; i < 16; i++) {
				p1[i] = 0;
				p2[i] = 0;
				psub[i] = 0;
			}
		}

		osc1 = pmi->osc1;
		osc2 = pmi->osc2;
		subosc = pmi->subosc;

		for(int i = 0; i < 2; i++) {
			if(pmi->lfo[i].mode == 1) {
				lfo[i].p = pmi->lfo[i].phase - 1;
			}
		}
	}
}

float keytrack(float cutoff, float pitch, float amount)
{
	float p = log10(pitch / 440.0) * 3.32192809489; /* 1/log10(2) */
	p = p * 12.0; /* +62 */
	p = p / 127.0;

	//printf("%f\n", p);
	
	return cutoff + p * amount;
}

bool FungusTrack::dofilters(float *psamples, int numsamples, bool gotinput, float pitch)
{
	if(!gotinput) {
		flt1.reset();
		flt2.reset();
		flt1b.reset();
		flt2b.reset();

		//if(filter1quiet && filter2quiet) {
			if(!fe1.over()) fe1.get(numsamples, false);
			if(!fe2.over()) fe2.get(numsamples, false);
			return false;
		//}

		memset(temporarybuffer, 0, numsamples * 8);
		memset(temporarybuffer2, 0, numsamples * 8);
	} else {
		filter1quiet = false;
		filter2quiet = false;
	}

	float envelope = fe1.get(numsamples, false) * fenvmod1;
	float env2 = fe2.get(numsamples, false) * fenvmod2;
	
	if(pmi->filtersetup == 0) {
		if(!filter1quiet) {
			if(pmi->filtertype1 > 0 && pmi->filtertype1 < 5) {
				flt1.dofilter_s(temporarybuffer, numsamples, keytrack(cutoff1 + envelope, pitch, pmi->keytrack1), resonance1);
			} else if(pmi->filtertype1 >= 5) {
				flt1b.dofilter_s(temporarybuffer, numsamples, keytrack(cutoff1 + envelope, pitch, pmi->keytrack1), resonance1);
			}
			if(pmi->disttype)
				pmi->dist.dodistortion(temporarybuffer, numsamples, pmi->disttype, pmi->distamount);
		}
		if(!filter2quiet) {
			if(pmi->filtertype2 > 0 && pmi->filtertype2 < 5) {
				flt2.dofilter_s(temporarybuffer, numsamples, keytrack(cutoff2 + env2, pitch, pmi->keytrack2), resonance2);
			} else if(pmi->filtertype2 >= 5) {
				flt2b.dofilter_s(temporarybuffer, numsamples, keytrack(cutoff2 + env2, pitch, pmi->keytrack2), resonance2);
			}
		}

		if(fabs(temporarybuffer[numsamples * 2 - 2]) < 0.01) {
			if(fabs(temporarybuffer[numsamples * 2 - 1]) < 0.01) {
				filter1quiet = true;
				filter2quiet = true;
			}
		}

		/*float *tmpbuf = temporarybuffer;
		numsamples <<= 1;
		while(numsamples--) {

			*psamples += *tmpbuf;
			psamples++;
			tmpbuf++;

		}*/

	} else {
		memcpy(temporarybuffer2, temporarybuffer, numsamples * 8);
		if(!filter1quiet) {
			if(pmi->filtertype1 > 0 && pmi->filtertype1 < 5) {
				flt1.dofilter_s(temporarybuffer, numsamples, keytrack(cutoff1 + envelope, pitch, pmi->keytrack1), resonance1);
			} else if(pmi->filtertype1 >= 5) {
				flt1b.dofilter_s(temporarybuffer, numsamples, keytrack(cutoff1 + envelope, pitch, pmi->keytrack1), resonance1);
			}

			if(fabs(temporarybuffer[numsamples * 2 - 2]) < 0.01) {
				if(fabs(temporarybuffer[numsamples * 2 - 1]) < 0.01) {
					filter1quiet = true;
				}
			}

			if(!filter1quiet) {
				if(pmi->disttype)
					pmi->dist.dodistortion(temporarybuffer, numsamples, pmi->disttype, pmi->distamount);
			}
		}
		if(!filter2quiet) {
			if(pmi->filtertype2 > 0 && pmi->filtertype2 < 5) {
				flt2.dofilter_s(temporarybuffer2, numsamples, keytrack(cutoff2 + env2, pitch, pmi->keytrack2), resonance2);
			} else if(pmi->filtertype2 >= 5) {
				flt2b.dofilter_s(temporarybuffer2, numsamples, keytrack(cutoff2 + env2, pitch, pmi->keytrack2), resonance2);
			}

			if(fabs(temporarybuffer2[numsamples * 2 - 2]) < 0.01) {
				if(fabs(temporarybuffer2[numsamples * 2 - 1]) < 0.01) {
					filter2quiet = true;
				}
			}
		}


		float *tmpbuf = temporarybuffer;
		float *tmpbuf2 = temporarybuffer2;
		numsamples <<= 1;
		float l1 = pmi->filterlevel;
		float l2 = 1 - pmi->filterlevel;
		while(numsamples--) {

			*tmpbuf = *tmpbuf * l2 + *tmpbuf2 * l1;
			//psamples++;
			tmpbuf++;
			tmpbuf2++;
		}
	}

	return true;
}

void FungusTrack::WorkLFO(int numsamples)
{
	int i;

	cutoff1 = pmi->cutoff_base1;
	resonance1 = pmi->resonance_base1;
	fenvmod1 = pmi->fenvmod_base1;
	cutoff2 = pmi->cutoff_base2;
	resonance2 = pmi->resonance_base2;
	fenvmod2 = pmi->fenvmod_base2;
	pitchlfo1 = 1.f;
	pitchlfo2 = 1.f;
	phase_add_1 = pmi->phase_add_1_base;
	phase_add_2 = pmi->phase_add_2_base;
	fmamount = pmi->fmamount_base;

	//float vol = 1.f;

	//int mixtmp = mix;
	mixres = pmi->mix;

	unsigned int lfoadd;
	int lfotrigger;

	for(i = 0; i < 2; i++) {
		lfotrigger = 0;

		lfoadd = pmi->lfo[i].p_add * numsamples;

		unsigned int lp = lfo[i].p;

#ifdef OLD_MSC_VER
		_asm {
			mov eax, [lp]
			adc eax, [lfoadd]
			jnc over

			mov [lfotrigger], 1

over:
			mov [lp], eax
		}
#else
		lp += lfoadd;
		if (lp < lfoadd)
			lfotrigger = 1;
#endif

		lfo[i].p = lp;

		if(pmi->lfo[i].wave < 5)
			lfo[i].value = pmi->lfo[i].waveform[lp >> 21];
		else {
			if(lfotrigger) {
				if(pmi->lfo[i].wave == 5) {
					lfo[i].value = pmi->noise();
				} else {
					lfo[i].prevvalue += lfo[i].valuediff;
					lfo[i].valuediff = pmi->noise() - lfo[i].prevvalue;
				}
			}
			if(pmi->lfo[i].wave == 6) {
				lfo[i].value = ((lfo[i].valuediff * (int)(lp >> 17)) >> 15) + lfo[i].prevvalue;
			}
		}
	}

	lfovol = 1.0;

	int lfovalue, lfoamp;
	int lfotarget;
	int lfoenv;
	for(int q = 0; q < 2; q++) {
		lfoenv = lfo[q].e.get(numsamples, false) * (pmi->lfo[q].envmod * 2.0);
		lfotarget = pmi->lfo[q].target;
		lfovalue = lfo[q].value;
		lfoamp = pmi->lfo[q].amp;

		int value = lfovalue * (lfoamp + lfoenv);
		if(value < -(32768*128))
			value = -(32768*128);
		else if(value > (32768*128))
			value = (32768*128);

		switch(lfotarget) {
		default:
		case 0:
			break;
		case 1: // osc1 pitch
			pitchlfo1 *= lfotab[((value) >> 7) + 0x8000];
			break;
		case 2: // osc2 pitch
			pitchlfo2 *= lfotab[((value) >> 7) + 0x8000];
			break;
		case 3: // osc1 phasemod
			phase_add_1 += (value) << 9;
			break;
		case 4: // osc2 phasemod
			phase_add_2 += (value) << 9;
			break;
		case 5: // fm
			fmamount += (value) >> 6;
			if(fmamount < 0) fmamount = 0;
			break;
		case 6: // mix
			mixres += (value) >> 16;
			break;
		case 7: // cutoff 1
			cutoff1 += (float)(value) * (1.f / 8323072.f);
			break;
		case 8: // resonance 1
			resonance1 += (float)(value) * (1.f / 8323072.f);
			break;
		case 9: // cutoff envmod 1
			fenvmod1 += (float)(value) * (1.f / 16646144.f);
			break;
		case 10: // cutoff 2
			cutoff2 += (float)(value) * (1.f / 8323072.f);
			break;
		case 11: // resonance 2
			resonance2 += (float)(value) * (1.f / 8323072.f);
			break;
		case 12: // cutoff envmod 2
			fenvmod2 += (float)(value) * (1.f / 16646144.f);
			break;
		case 13: // volume
			//vol += (float)(value) * (1.f / 4161536.f);
			lfovol *= pow(12.0f, (float)(((int)lfovalue - 32768) * ((int)lfoamp + (int)lfoenv)) * (1.f / 4161536.f));
			break;
		}
	}

	mixres = mixres * (1.f / 128.f);
	/*if(mixtmp < 0) mix = 0;
	if(mixtmp > 128) mix = 128;
	mix_2 = (float)mixtmp * (1.f / 128.f);
	mix_1 = 1.f - mix_2;

	mix_1 *= vol;
	mix_2 *= vol;*/
}

bool FungusTrack::Work(float *psamples, int numsamples)
{
	notedelay -= numsamples;
	if(notedelay < 0) DoTick();

	if(curcut > 0) {
		curcut -= numsamples;
		if(curcut <= 0) {
			Note = zzub_note_value_none;
			curcut = -1;
			e.End();
			fe1.End();
			fe2.End();
			pe.End();
			fme.End();
			me.End();
		}
	}

	WorkLFO(numsamples);

	float pitch = f1 * pmi->pitchbend;
	float detunemul = pmi->detunefine * pmi->detunesemi;
	if(pmi->pmode == 0) pitch *= (pe.get(numsamples, false) * pmi->penvmod + 1);
	else
		detunemul *= (pe.get(numsamples, false) * pmi->penvmod + 1);

	if(!active) {
		dofilters(psamples, numsamples, false, pitch);
		//memset(psamples, 0, numsamples * 8);
		return false;
	}

	float envelope_prev = e.last();
	e.get(numsamples, true);
	if(envelope_prev < 0.001 && e.last() < 0.001) {

		bool result = dofilters(psamples, numsamples, false, pitch);
		//memset(psamples, 0, numsamples * 8);

//		fe1.get(numsamples);
//		fe2.get(numsamples);
		fme.get(numsamples, false);
		me.get(numsamples, false);

		if(e.over() && fe1.over() && fe2.over() && fme.over() && pe.over() && me.over())
			active = false;

		if(notedelay >= 0 && notedelay < 0x100000) /* hack */
			active = true;

		return false;//result;
	}

	float outputmul = pmi->outputmul;

	int mixmode = pmi->mixmode;

	if(pmi->filtertype1 > 0 && pmi->filtertype1 < 5)
		flt1.settype(pmi->filtertype1 - 1);
	else if(pmi->filtertype1 >= 5)
		flt1b.settype(pmi->filtertype1 - 5);

	if(pmi->filtertype2 > 0 && pmi->filtertype2 < 5)
		flt2.settype(pmi->filtertype2 - 1);
	else if(pmi->filtertype2 >= 5)
		flt2b.settype(pmi->filtertype2 - 5);

	float _vol = /*vol * */ lfovol;
	float subvol = pmi->subvol * _vol;

	int o1, o2;
	float fo1, fo2;

	int _fmamount = fmamount + (int)((float)pmi->fmenvmod * fme.get(numsamples, false));

	if(pmi->fmmode == 1)
		_fmamount = -_fmamount;

	bool noise1 = pmi->noise1;
	bool noise2 = pmi->noise2;

	float mix_1, mix_2;// = pmi->mix_1 * vol, mix_2 = pmi->mix_2 * vol;
	me.get(numsamples, true);

	mix_2 = mixres;
	if(mix_2 < 0) mix_2 = 0;
	else if(mix_2 > 1.0) mix_2 = 1.0;
	mix_1 = 1.f - mix_2;

	int phase_add_1 = this->phase_add_1;
	int phasemodtype_1 = pmi->phasemodtype_1;

	int phase_add_2 = this->phase_add_2;
	int phasemodtype_2 = pmi->phasemodtype_2;

	unsigned int a1, a2;

	if(GlideCount) {
		int l = GlideCount > numsamples ? numsamples : GlideCount;

		GlideCount -= l;
		GlideFactor *= pow(GlideMul, l);
		pitch *= GlideFactor;

		if(!GlideCount) {
			f1 = f2;
		}

	}

	float tabsizediv_pitchmod = tabsizediv * pitch;

	int sync = 0;
	if(pmi->sync == 2) sync = 1;
	
	float *tmpbuf;

	short *_osc1, *_osc2, *_subosc;

	memset(temporarybuffer, 0, numsamples * 8);
//	for(int i = 0; i < numsamples; i += 2)
//		temporarybuffer[i] = 0.01;

	for(int o = 0; o < pmi->dtosc; o++) {

		float p = pmi->dtpos[o] + pan;
		float o1p = (p - pmi->spread);
		if(o1p < -1) o1p = -1; else if(o1p > 1) o1p = 1;
		float o1l = (1 - o1p) * 0.5;
		float o1r = (1 + o1p) * 0.5;

		float o2p = (p + pmi->spread);
		if(o2p < -1) o2p = -1; else if(o2p > 1) o2p = 1;
		float o2l = (1 - o2p) * 0.5;
		float o2r = (1 + o2p) * 0.5;

		float sol = (1 - p) * 0.5;
		float sor = (1 + p) * 0.5;

		// initialize phase_add for oscillators
		a1 = (int)(tabsizediv_pitchmod * pitchlfo1 * pmi->dtmul[o]);
		a2 = (int)(tabsizediv_pitchmod * detunemul * pitchlfo2 * pmi->dtmul[o]);

		unsigned int _p1 = p1[o];
		unsigned int _p2 = p2[o];
		unsigned int _psub = psub[o];

		// select band-limited waveform level

		int oct;

//		unsigned int t = (unsigned int)((double)a1 * 0.707);
		unsigned int t = a1;

#ifdef OLD_MSC_VER
		_asm {
		   bsr eax, [t]
		   mov [oct], eax
		}
#else
		oct = 0;
		while(!(t & 0x80000000)) { t <<= 1; oct++; }
#endif

//		oct = 31 - oct;
		oct -= 21;
		if(oct < 0) {
			_osc1 = osc1;
			_subosc = subosc;
		} else {
			if(oct > 8) {
				if(oct > 9) {
					_osc1 = osc1 + 8 * (66*2048);
					_subosc = subosc + 8 * (66*2048);
				} else {
					_osc1 = osc1 + 8 * (66*2048);
					_subosc = subosc + 7 * (66*2048);
				}
			} else {
				_osc1 = osc1 + oct * (66*2048);

				oct--;
				if(oct < 0) _subosc = subosc;
				else _subosc = subosc + oct * (66*2048);
			}
		}

		//t = (unsigned int)((double)a2 * 0.707);
		t = a2;

#ifdef OLD_MSC_VER
		_asm {
		   bsr eax, [t]
		   mov [oct], eax
		}
#else
		oct = 0;
		while(!(t & 0x80000000)) { t <<= 1; oct++; }
#endif

		oct -= 21;
		if(oct < 0) _osc2 = osc2;
		else {
			if(oct > 8) oct = 8;

			_osc2 = osc2 + oct * (66*2048);
		}

		tmpbuf = temporarybuffer;
		int ns = numsamples;
		float *menv = me.shape;
		while(ns--) {

			if(noise2) {

				o2 = pn2.pinkrand();

			} else {

				if(phasemodtype_2 == 0) {

#ifdef LINEAR_INTERPOLATION_OSCS
#define OSC1(x) linint(_osc1, (x))
#define OSC2(x) linint(_osc2, (x))
#define SUBOSC(x) linint(_subosc, (x))
#else
#define OSC2(x) _osc2[(x) >> 21]
#define OSC1(x) _osc1[(x) >> 21]
#define SUBOSC(x) _subosc[(x) >> 21]
#endif

					o2 = OSC2(_p2 + phase_add_2);

				} else {

					o2 = OSC2(_p2);

					if(phasemodtype_2 == 1) {

						o2 -= OSC2(_p2 + phase_add_2);

					} else if(phasemodtype_2 == 2) {

						o2 += OSC2(_p2 + phase_add_2 - 0x80000000);
						o2 >>= 1;

						/*o2 *= _osc2[(_p2 + phase_add_2) >> 21];
						o2 >>= 15;*/

					}
				}

			}

			if(noise1) {

				o1 = pn1.pinkrand();

			} else {

				unsigned int p1_current = _p1 - (o2 * _fmamount);

				if(phasemodtype_1 == 0) {
					o1 = OSC1(p1_current + phase_add_1);
				} else {
					o1 = OSC1(p1_current);

					if(phasemodtype_1 == 1) {

						o1 -= OSC1(p1_current + phase_add_1);

					} else if(phasemodtype_1 == 2) {

						o1 += OSC1(p1_current + phase_add_1 - 0x80000000);
						o1 >>= 1;

						/*o1 *= _osc1[(p1_current + phase_add_1) >> 21];
						o1 >>= 15;*/

					}
				}
			}

			fo1 = (float)o1;
			fo2 = (float)o2;

			int _a1;
			int _a2;

			_a1 = a1;

			if(pmi->mixenabled) {
				mix_2 = mixres + pmi->mixenvmod * (*menv);
				if(mix_2 < 0) mix_2 = 0;
				else if(mix_2 > 1.0) mix_2 = 1.0;
				mix_1 = 1.f - mix_2;
				menv++;
			}

			//mix_1 *= _vol;
			//mix_2 *= _vol;

			fo1 *= mix_1;
			fo2 *= mix_2;

			float outputl;
			float outputr;

			switch(mixmode) {
			case 0:
				outputl = fo1 * o1l + fo2 * o2l;
				outputr = fo1 * o1r + fo2 * o2r;
				break;
			case 1:
				outputl = fo1 * o1l - fo2 * o2l;
				outputr = fo1 * o1r - fo2 * o2r;
				break;
			case 2:
				outputl = (fo1 * o1l * fo2 * o2l) * (1.f / 4096.f);
				outputr = (fo1 * o1r * fo2 * o2r) * (1.f / 4096.f);
				break;
			}

			*tmpbuf += outputl * _vol + SUBOSC(_psub) * subvol * sol;
			tmpbuf++;
			*tmpbuf += outputr * _vol + SUBOSC(_psub) * subvol * sor;
			tmpbuf++;

			_psub += _a1 >> 1;

			_a2 = a2;
			_p2 += _a2;

			if(sync) {

#ifdef OLD_MSC_VER
				_asm {
					mov eax, [_p1]
					add eax, [_a1]
					jnc nosync
					
					mov ecx, [_a2]
					mov ebx, eax
					mul ecx
					mov ecx, [_a1]
					div ecx

					mov [_p2], eax
					mov eax, ebx
	nosync:
					mov [_p1], eax
				}
#else
				unsigned int ea = _p1 + _a1;
				if (ea < _a1) { // overflow
#ifdef _MSC_VER
					_p2 = __emulu(ea, _a2) / _a1;
#else
					_p2 = (long long)ea * _a2 / _a1;
#endif
				}
				_p1 = ea;
#endif

			} else
				_p1 += _a1;

			a2 = _a2;

		}

		p1[o] = _p1;
		p2[o] = _p2;
		psub[o] = _psub;

	}

	bool retval = dofilters(psamples, numsamples, true, pitch);

	float *env = e.shape;
	float *tmpbuf2 = temporarybuffer;
	tmpbuf = psamples;
	int ns = numsamples;
	while(ns--) {
		float t = *env * outputmul;
#ifdef NONLINEAR_VCA
		*tmpbuf += otanh_32768(*tmpbuf2 * t);
		tmpbuf++;
		tmpbuf2++;
		*tmpbuf += otanh_32768(*tmpbuf2 * t);
		tmpbuf++;
		tmpbuf2++;
#else
		*tmpbuf += *tmpbuf2 * t;;
		tmpbuf++;
		tmpbuf2++;
		*tmpbuf += *tmpbuf2 * t;;
		tmpbuf++;
		tmpbuf2++;
#endif
		env++;
	}

	return retval;//dofilters(psamples, numsamples, true);
}

void FungusTrack::Stop(void)
{
	e.End();
	fme.End();
	fe1.End();
	fe2.End();
	pe.End();
}

fungus::fungus()
{
	prec = 16;

//	renderbuf = (float*)malloc(1024 * 8);
	global_values = &gval;
	track_values = tval;
	//attributes = (int *)&aval;

	for(int i = 0; i < 16; i++) {
		t[i].pmi = this;
		t[i].filter1quiet = true;
		t[i].filter2quiet = true;
		t[i].notedelay = 0x7FFFFFFF;
	}

	active = false;

	CreateEnvelope(&ampenv, "Amp envelope");
	CreateEnvelope(&pitchenv, "Pitch envelope");
	CreateEnvelope(&cutoffenv1, "Cutoff 1 envelope");
	CreateEnvelope(&cutoffenv2, "Cutoff 2 envelope");
	CreateEnvelope(&fmenv, "FM envelope");
	CreateEnvelope(&mixenv, "Mix envelope");
	CreateEnvelope(&lfo1env, "LFO1 envelope");
	CreateEnvelope(&lfo2env, "LFO2 envelope");
}

fungus::~fungus()
{
	//free(renderbuf);

	DestroyEnvelope(&ampenv);
	DestroyEnvelope(&pitchenv);
	DestroyEnvelope(&cutoffenv1);
	DestroyEnvelope(&cutoffenv1);
	DestroyEnvelope(&fmenv);
	DestroyEnvelope(&mixenv);
	DestroyEnvelope(&lfo1env);
	DestroyEnvelope(&lfo2env);
}

void loadenvelope(Envelope* e, zzub::instream * const pi)
{
	pi->read(e->data.nodes);

	for(int i = 0; i < e->data.nodes; i++) {
		pi->read(e->data.nodedata[i].x);
		pi->read(e->data.nodedata[i].y);
		pi->read(e->data.nodedata[i].x2);
		pi->read(e->data.nodedata[i].y2);
		pi->read(e->data.nodedata[i].slope);
		pi->read(e->data.nodedata[i].mode);
	}
}

void saveenvelope(Envelope* e, zzub::outstream * const po)
{
	po->write(e->data.nodes);

	for(int i = 0; i < e->data.nodes; i++) {
		po->write(e->data.nodedata[i].x);
		po->write(e->data.nodedata[i].y);
		po->write(e->data.nodedata[i].x2);
		po->write(e->data.nodedata[i].y2);
		po->write(e->data.nodedata[i].slope);
		po->write(e->data.nodedata[i].mode);
	}
}

void fungus::save(zzub::archive * const po)
{
	zzub::outstream* strm = po->get_outstream("");

	int ver = 1;
	strm->write(ver);
	saveenvelope(&ampenv, strm);
	saveenvelope(&pitchenv, strm);
	saveenvelope(&cutoffenv1, strm);
	saveenvelope(&cutoffenv2, strm);
	saveenvelope(&fmenv, strm);
	saveenvelope(&mixenv, strm);
	saveenvelope(&lfo1env, strm);
	saveenvelope(&lfo2env, strm);
}

void fungus::load(zzub::archive * const pi)
{
	if(!pi) return;
	zzub::instream* strm = pi->get_instream("");
	if(!strm) return;

	int ver;
	strm->read(ver);

	loadenvelope(&ampenv, strm);
	RepaintEnvelope(&ampenv);
	loadenvelope(&pitchenv, strm);
	RepaintEnvelope(&pitchenv);
	loadenvelope(&cutoffenv1, strm);
	RepaintEnvelope(&cutoffenv1);
	loadenvelope(&cutoffenv2, strm);
	RepaintEnvelope(&cutoffenv2);
	loadenvelope(&fmenv, strm);
	RepaintEnvelope(&fmenv);
	loadenvelope(&mixenv, strm);
	RepaintEnvelope(&mixenv);
	loadenvelope(&lfo1env, strm);
	RepaintEnvelope(&lfo1env);
	loadenvelope(&lfo2env, strm);
	RepaintEnvelope(&lfo2env);
}

void fungus::init(zzub::archive * const pi)
{
	int i;

	tabsizediv = powf(2, 32) / (2.0f * (float)_master_info->samples_per_second);

	// initialize and create envelopes
	InitEnvelope(&ampenv);
	InitEnvelope(&pitchenv);
	InitEnvelope(&cutoffenv1);
	InitEnvelope(&cutoffenv2);
	InitEnvelope(&fmenv);
	InitEnvelope(&mixenv);
	InitEnvelope(&lfo1env);
	InitEnvelope(&lfo2env);

	// init amp envelope for velocity sensitivity
	ampenv.data.nodedata[1].y2 = 16384;

	if(pi) {
		zzub::instream* strm = pi->get_instream("");
		if(strm) {

			int ver;
			strm->read(ver);

			loadenvelope(&ampenv, strm);
			loadenvelope(&pitchenv, strm);
			loadenvelope(&cutoffenv1, strm);
			loadenvelope(&cutoffenv2, strm);
			loadenvelope(&fmenv, strm);
			loadenvelope(&mixenv, strm);
			loadenvelope(&lfo1env, strm);
			loadenvelope(&lfo2env, strm);
		}
	}

	if(!fmtab) {
		int i;

		fmtab = (float*)malloc(0x10000 * 4);
		lfotab = (float*)malloc(0x10000 * 4);
		for(i = 0; i < 0x10000; i++) {
			fmtab[i] = (float)pow(1.00004230724139582, (double)((i - 0x8000) << 2));
			lfotab[i] = (float)pow(1.00004230724139582, (double)(i - 0x8000) * 0.5);
		}
	}

	initwavetable();

	osc1 = wavetable + 2048;
	osc2 = wavetable + 2048;
	subosc = wavetable + 2048;

	gain = 1.f / 32768.f;
	gaintarget = 1.f / 32768.f;

	pitchbend = 1;
	pitchbendtarget = 1;
	pitchbendmul = 1;
	pitchbendtime = 0;

	cutoff_base1 = 0;
	cutoff_base2 = 0;

	r1 = 26474;
	r2 = 13075;
	r3 = 18376;
	r4 = 31291;

	for(int i = 0; i < 16; i++) {
		t[i].Init();

		for(int j = 0; j < 16; j++) {
			t[i].p1[j] = 0;
			t[i].p2[j] = 0;
			t[i].psub[j] = 0;
		}

		for(int j = 0; j < 2; j++) {
			t[i].lfo[j].value = 0;
			t[i].lfo[j].prevvalue = 0;
			t[i].lfo[j].valuediff = 0;
			t[i].lfo[j].p = 0;
		}

		t[i].osc1 = osc1;
		t[i].osc2 = osc2;
		t[i].subosc = subosc;

		t[i].vol = 1;

		t[i].Note = zzub_note_value_none;

		t[i].GlideCount = 0;

/*		t[i].r1 = 26474;
		t[i].r2 = 13075;
		t[i].r3 = 18376;
		t[i].r4 = 31291;*/
	}

	dtosc = 1;
	outputmul = 1.0;
	dtdetune = 0;
	dtpan = 0;
	dtmode = 0;

	for(i = 0; i < 2; i++) {
		lfo[i].p_add = 0;
		lfo[i].waveform = lfotable;
		lfo[i].wave = 0;
		lfo[i].target = 0;
		lfo[i].amp = 0;
	}
}


void fungus::process_events()
{
	int i;

	if(gval.gain != 255) {
		gaintarget = (float)pow(10.0, (double)(gval.gain - 64) * 0.5 / 20.0) / 32768.f;
	}

	if(gval.pitchbendrange != 255) {
		pitchbendrange = gval.pitchbendrange;
	}

	if(gval.pitchbend != 255) {
		
		if(gval.pitchbend == 127)
			gval.pitchbend = 128;

		pitchbendtarget = pow(2.0, (gval.pitchbend - 64) * pitchbendrange / (12.0 * 64.0));
		pitchbendtime = 2*_master_info->samples_per_tick;
		pitchbendmul = pow(pitchbendtarget / pitchbend, 1.0 / (double)pitchbendtime);
	}

	if(gval.glide != 255)
		glidetime = gval.glide * ((double)_master_info->samples_per_second * 220.0 * 4.0 / 44100.0);

	if(gval.retrig != 255)
		retrig = gval.retrig;

	for(int i = 0; i < 2; i++) {
		if(gval.lfo[i].wave != 255) {
			lfo[i].waveform = lfotable + gval.lfo[i].wave * 2048;
			lfo[i].wave = gval.lfo[i].wave;
		}

		if(gval.lfo[i].freq != 255) {
			if(gval.lfo[i].freq > 120) {

				int len;
				if(gval.lfo[i].freq > 124)
					len = 1 << (gval.lfo[i].freq - 122);
				else
					len = gval.lfo[i].freq - 120;

				lfo[i].p_add = (unsigned int)(powf(2, 32) / (float)(len * _master_info->samples_per_tick * 2));
			} else
				lfo[i].p_add = (unsigned int)(tabsizediv * envtime(gval.lfo[i].freq * 2));
		}

		if(gval.lfo[i].phase != 255) {
			for(int j = 0; j < numTracks; j++) {
				t[j].lfo[i].p = gval.lfo[i].phase << 25;
			}
			lfo[i].phase = gval.lfo[i].phase << 25;
		}

		if(gval.lfo[i].amp != 255)
			lfo[i].amp = gval.lfo[i].amp;

		if(gval.lfo[i].ampenvmod != 255)
			lfo[i].envmod = gval.lfo[i].ampenvmod - 64;

		if(gval.lfo[i].target != 255)
			lfo[i].target = gval.lfo[i].target;

		if(gval.lfo[i].mode != 255)
			lfo[i].mode = gval.lfo[i].mode;
	}

	if(gval.sync != 255)
		sync = gval.sync;

//	if(gval.prec != 255)
//		prec = 1 << gval.prec;

	if(gval.wave1 != 255)
		if(gval.wave1 == 66) {
			noise1 = true;
		} else {
			if(gval.wave1 == 2) {
				osc1 = wavetable + 1 * 2048;
				phasemodtype_1 = 1;
			} else {
				osc1 = wavetable + gval.wave1 * 2048;
				select_wave(gval.wave1);
				if(gval.wave1 > 0)
					phasemodtype_1 = 2;
				else
					phasemodtype_1 = 0;
			}
			noise1 = false;
		}

	if(gval.wave2 != 255)
		if(gval.wave2 == 66) {
			noise2 = true;
		} else {
			if(gval.wave2 == 2) {
				osc2 = wavetable + 1 * 2048;
				phasemodtype_2 = 1;
			} else {
				osc2 = wavetable + gval.wave2 * 2048;
				select_wave(gval.wave2);
				if(gval.wave2 > 0)
					phasemodtype_2 = 2;
				else
					phasemodtype_2 = 0;
			}
			noise2 = false;
		}

	if(gval.subosc != 255)
		subvol = (float)gval.subosc * (1.0f / 128.f);

	if(gval.subwave != 255)
		subosc = wavetable + gval.subwave * 2048;

	if(gval.detunesemi != 255)
		detunesemi = (float)pow(DETUNESEMI, (double)(gval.detunesemi - 0x40));

	if(gval.detunefine != 255)
		detunefine = (float)pow(DETUNEFINE, (double)(gval.detunefine - 0x40));

	if(gval.fmmode != 255) fmmode = gval.fmmode;

	if(gval.fmamount != 255) fmamount_base = gval.fmamount << 12;
 
	//if(gval.fmmorph != 255)
	//	fmmorph = (float)gval.fmmorph * (1.f / 128.f);
	
	if(gval.fmenvmod != 255) fmenvmod = (gval.fmenvmod - 64) << 11;

	if(gval.spread != 255)
		spread = (float)(gval.spread - 64) * (1.f / 64.f);

	if(gval.mixmode != 255)
		mixmode = gval.mixmode;

	if(gval.mix != 255)
		mix = gval.mix;

	if(gval.mixenvmod != 255) {
		mixenvmod = (float)(gval.mixenvmod - 64) * (1.f / 64.f);
		if(gval.mixenvmod == 64)
			mixenabled = 0;
		else
			mixenabled = 1;
	}

	//if(gval.mixmorph != 255)
	//	mixmorph = (float)gval.mixmorph * (1.f / 128.f);

	//if(gval.ampmorph != 255)
	//	amorph = (float)gval.ampmorph * (1.f / 128.f);

	if(gval.p1shift != 255)
		phase_add_1_base = gval.p1shift << 25;

	/*if(gval.p1modtype != 255)
		phasemodtype_1 = gval.p1modtype;*/

	if(gval.p2shift != 255)
		phase_add_2_base = gval.p2shift << 25;

	/*if(gval.p2modtype != 255)
		phasemodtype_2 = gval.p2modtype;*/

	if(gval.filtertype1 != 255)
		filtertype1 = gval.filtertype1;

	if(gval.cutoff1 != 255) {
		float tgt = (float)gval.cutoff1 * (1.f / 127.f);
		cutoff_base1time = 2*_master_info->samples_per_tick;
		cutoff_base1add = (tgt - cutoff_base1) / (float)cutoff_base1time;
	}

	if(gval.resonance1 != 255)
		resonance_base1 = (float)gval.resonance1 * (1.f / 127.f);

	//if(gval.fmorph1 != 255)
	//	fmorph1 = (float)gval.fmorph1 * (1.f / 128.f);

	if(gval.keytrack1 != 255)
		keytrack1 = (float)gval.keytrack1 / 128.0;

	if(gval.fenvmod1 != 255)
		fenvmod_base1 = (float)(gval.fenvmod1 - 64) * (1.f / 64.f);

	if(gval.disttype != 255) {
		disttype = gval.disttype;
		dist.precalc(disttype, distamount);
	}

	if(gval.distamount != 255) {
		distamount = pow(2.0, (float)(gval.distamount - 16) / 24.0);
		dist.precalc(disttype, distamount);
	}

	if(gval.filtertype2 != 255)
		filtertype2 = gval.filtertype2;

	if(gval.cutoff2 != 255) {
		float tgt = (float)gval.cutoff2 * (1.f / 127.f);
		cutoff_base2time = 2*_master_info->samples_per_tick;
		cutoff_base2add = (tgt - cutoff_base2) / (float)cutoff_base2time;
	}

	if(gval.resonance2 != 255)
		resonance_base2 = (float)gval.resonance2 * (1.f / 127.f);

	//if(gval.fmorph2 != 255)
	//	fmorph2 = (float)gval.fmorph2 * (1.f / 128.f);

	if(gval.keytrack2 != 255)
		keytrack2 = (float)gval.keytrack2 / 128.0;

	if(gval.fenvmod2 != 255)
		fenvmod_base2 = (float)(gval.fenvmod2 - 64) * (1.f / 64.f);

	if(gval.filtersetup != 255) {
		filtersetup = gval.filtersetup;
		filterlevel = gval.filtersetup * (1.f / 128.f);
	}

	//if(gval.pmorph != 255)
	//	pmorph = (float)gval.pmorph * (1.f / 128.f);

	if(gval.penvmod != 255)
		penvmod = powf(2.f,(float)(gval.penvmod - 64) / 12.0f) - 1;

	if(gval.pmode != 255)
		pmode = gval.pmode;

	bool recalc = false;
	if(gval.numosc != 255) {
		dtosc = gval.numosc;
		outputmul = (float)sqrt(1.f / (float)dtosc);
		recalc = true;
	}

	if(gval.detune != 255) {
		dtdetune = gval.detune;
		recalc = true;
	}

	if(gval.dtpan != 255) {
		dtpan = (float)(gval.dtpan - 64) * (1.f / 128.f);
		recalc = true;
	}

	if(gval.dtmode != 255) {
		dtmode = gval.dtmode;
		recalc = true;
	}

	if(recalc) {
		srand(dtmode);

		float s = 0;
		for(int j = 0; j < dtosc; j++) {
			s += (double)rand() * (2.0 / RAND_MAX) - 1.0;
		}

		const float fix = -s / (float)dtosc;

		srand(dtmode);

//		for(int i = 0; i < 16; i++) {
			for(int j = 0; j < dtosc; j++) {
				float oscdetune;
				if(dtmode) {
					oscdetune = (double)rand() * (2.0 / RAND_MAX) - 1.0 + fix;
				} else {
					oscdetune = ((double)j - (double)(dtosc - 1) / 2.0) / ((double)dtosc / 2.0f);
				}
				dtmul[j] = (float)pow(DETUNEFINE, ((double)dtdetune * oscdetune));
				dtpos[j] = (double)dtpan * oscdetune;
				//dtpos[j] = ((double)dtpan * ((double)j - (double)(dtosc - 1) / 2.0)) / ((double)(dtosc+1) / 8.0f);
			}
//		}
	}

	for(i = 0; i < numTracks; i++) {
		int v = tval[i].volume;
		int pan = tval[i].pan;
		//int morph = tval[i].morph;

		int delay = tval[i].delay;
		if(delay == 0xff) delay = 0;

		int cut = tval[i].cut;
		if(cut == 0xff) cut = -1;

		if(tval[i].note != 0) {
			t[i].Tick (tval[i].note, v, delay, cut, pan/*, morph*/);
			t[i].active = true;
			active = true;
		} else
			t[i].Tick(0, v, delay, cut, pan/*, morph*/);
	}
}

bool fungus::process_stereo(float **pin, float **pout, int numsamples, int mode)
{
//	if(!active) return false;

	if(active) {
		memset(renderbuf, 0, 8 * 4 * numsamples);
/*
		float *r = renderbuf;
		int i = numsamples << 2;

		while(i--) {
			*(r++) = noise() * (0.01f / 32768.0);
		}*/
	}

	bool r = false;

	int p = 0;
	int l;
	int ns = numsamples * 2;

	while(1) {

		l = ns > prec ? prec : ns;

		if(l <= 0) break;

		if(pitchbendtime) {
			int n = pitchbendtime;
			if(n > l) n = l;
			pitchbendtime -= n;
			if(!pitchbendtime) {
				pitchbend = pitchbendtarget;
			} else {
				pitchbend *= pow(pitchbendmul, n);
			}
		}

		if(cutoff_base1time) {
			int t = cutoff_base1time;
			if(l < t) t = l;
			cutoff_base1 += cutoff_base1add * (float)t;
			cutoff_base1time -= t;
		}

		if(cutoff_base2time) {
			int t = cutoff_base2time;
			if(l < t) t = l;
			cutoff_base2 += cutoff_base2add * (float)t;
			cutoff_base2time -= t;
		}

		if(active) {

			for(int i = 0; i < numTracks; i++) {
				r |= t[i].Work(renderbuf + p*2, l);
			}
		}

		ns -= l;
		p += l;

//		if(r == false) break;

	};

	active = false;
	for(int i = 0; i < numTracks; i++) {
		active |= t[i].active;
	}

	if(!active) return false;

	if(r) {
		/* halfband filter */

		float *rs = renderbuf;

		if(fabs(gain - gaintarget) < (0.001/32768.0)) {
			gain = gaintarget;

			while(numsamples--) {
				float v = dc5.Calc(*rs, *(rs + 2)) * gain;
				rs++;
				if (pout[0]) {
					*pout[0] = v;
					pout[0]++;
				}

				v = dc5_r.Calc(*rs, *(rs + 2)) * gain;
				rs += 3;
				if (pout[1]) {
					*pout[1] = v;
					pout[1]++;
				}
			}
		} else {
			while(numsamples--) {
				gain += (gaintarget - gain) * 0.004;

				float v = dc5.Calc(*rs, *(rs + 2)) * gain;
				rs++;
				if (pout[0]) {
					*pout[0] = v;
					pout[0]++;
				}

				v = dc5_r.Calc(*rs, *(rs + 2)) * gain;
				rs += 3;
				if (pout[1]) {
					*pout[1] = v;
					pout[1]++;
				}
			}
		}
	} else {
		gain = gaintarget;
	}

	return r;
}

void fungus::stop()
{
	for(int i = 0; i < numTracks; i++) {
		t[i].Stop();
	}
}

#undef SUBOSC

#define GAIN		0
#define PITCHBEND	(GAIN+1)
#define WAVEFORM1	(PITCHBEND+2)
#define WAVEFORM2	(WAVEFORM1+2)
#define SYNC		(WAVEFORM2+4)
#define FM			(SYNC+1)
#define MIX			(FM+3)
#define SUBOSC		(MIX+4)
#define PITCH       (SUBOSC+2)
#define GLIDE		(PITCH+2)
#define FILTER1		(GLIDE+2)
#define DIST		(FILTER1+5)
#define FILTER2		(DIST+2)
#define FILTERSETUP	(FILTER2+5)
#define LFO1		(FILTERSETUP+1)
#define LFO2		(LFO1+7)
#define DETUNE		(LFO2+7)
#define TRACK		(DETUNE+3)

char const *fungus::describe_value(int const param, int const value)
{
	static char txt[16];

	switch(param) {
	case GAIN:
		sprintf(txt, "%1.2f dB", (float)(value - 64) / 2.0);
		break;
	case DIST+0:
		switch(value) {
		case 0:
			return "Off";
		case 1:
			return "3rd";
		case 2:
			return "tanh";
		case 3:
			return "atan";
		default:
			sprintf(txt, "%d", value);
			break;
		}
		break;
	case WAVEFORM1+0:
	case WAVEFORM2+0:
	case SUBOSC+0:
		switch(value) {
		case 0:
			return "Sine";
		case 1:
			return "Saw";
		case 2:
			return "Square";
		case 3:
			return "Tri";
		case 66:
			return "Noise";
		default:
			sprintf(txt, "%d", value);
			break;
		}
		break;
	case LFO1+0:
	case LFO2+0:
		switch(value) {
		case 0:
			return "Sine";
		case 1:
			return "Saw";
		case 2:
			return "InverseSaw";
		case 3:
			return "Square";
		case 4:
			return "Tri";
		case 5:
			return "Noise";
		case 6:
			return "SoftNoise";
		}
	/*
	case FM+2: // MORPH
	case PITCH+0:
	case FILTER1+3:
	case FILTER2+3:
	case FILTERSETUP+1:
		sprintf(txt, "%1.3f", (float)value / 128.0);
		break;
	*/
//	case 25: // Precision
//		sprintf(txt, "%d", 1 << value);
//		break;
	/*case WAVEFORM1+2:
	case WAVEFORM2+2:
		switch(value) {
		case 0:
			return "None";
		case 1:
			return "Sub";
		case 2:
			return "Mul";
		}*/
	case WAVEFORM2+2: // -64 .. 64
		sprintf(txt, "%d semitones", value - 64);
		break;
	case WAVEFORM2+3:
		sprintf(txt, "%d cents", value - 64);
		break;
	case SYNC+0:
		switch(value) {
		case 0:
			return "None";
		case 1:
			return "NoteOn";
		case 2:
			return "On";
		case 3:
			return "NoteOnReset";
		}
	case FM+0:
	case MIX+3:
		switch(value) {
		case 0:
			return "Add";
		case 1:
			return "Sub";
		case 2:
			return "Mul";
		}
	case FILTER1+0:
	case FILTER2+0:
		switch(value) {
        case 0:
			return "Passthrough";
        case 1:
			return "12dB lp";
        case 2:
			return "12dB hp";
        case 3:
			return "12dB bp";
        case 4:
			return "12dB notch";
        case 5:
			return "24dB lp";
        case 6:
			return "24dB hp";
		}
	case FILTER1+1:
	case FILTER2+1:
		sprintf(txt, "%1.2f Hz", 440.0 * pow(2.0, ((double)value - 62.0) / 12.0));
		break;
	case FILTER1+2:
	case FILTER2+2:
		sprintf(txt, "%1.2f", log(pow(256.0, (double)value / 127.0))/log(2.0));
		break;
	case WAVEFORM1+1:  // -64 .. 64
	case WAVEFORM2+1:
	case PITCHBEND+0:
	case FM+2:
	case PITCH+1:
	case FILTER1+4:
	case FILTER2+4:
	case LFO1+4:
	case LFO2+4:
	case MIX+0:
	case MIX+2:
	case DETUNE+2:
		sprintf(txt, "%d", value - 64);
		break;
	case LFO1+1:
	case LFO2+1:
		if(value > 120)
			if(value > 124)
				sprintf(txt, "%d ticks", 1 << (value - 122));
			else
				sprintf(txt, "%d ticks", value - 120);
		else
			sprintf(txt, "%1.2f Hz", envtime(value * 2));
		break;
	case LFO1+2:
	case LFO2+2:
		sprintf(txt, "%d deg", (value * 360) / 128);
		break;
	case LFO1+5: // LFO Target
	case LFO2+5:
		switch(value) {
		case 0:
			return "None";
		case 1:
			return "OSC1";
		case 2:
			return "OSC2";
		case 3:
			return "Phase1";
		case 4:
			return "Phase2";
		case 5:
			return "FM";
		case 6:
			return "Mix";
		case 7:
			return "Cutoff1";
		case 8:
			return "Resonance1";
		case 9:
			return "CutEnv1";
		case 10:
			return "Cutoff2";
		case 11:
			return "Resonance2";
		case 12:
			return "CutEnv2";
		case 13:
			return "Volume";
		};
	case LFO1+6: // LFO Mode
	case LFO2+6:
		switch(value) {
		case 0:
			return "Free";
		case 1:
			return "Reset on note";
		}
	case MIX+1:
		sprintf(txt, "%d%% : %d%%", (128-value)*100/128, value*100/128);
		break;
	case PITCH+0:
		switch(value) {
		case 0:
			return "All Oscs";
		case 1:
			return "Osc2 only";
		}
	case GLIDE+1:
		switch(value) {
		case 0:
			return "Glide";
		case 1:
			return "Always retrig";
		}

	case DETUNE+3:
		if(value == 0) return "Normal";
		sprintf(txt, "Random %d", value);
		break;

	case FILTERSETUP+0:
		if(value == 0) return "Serial";
		sprintf(txt, "Par %d%%:%d%%", (128-value)*100/128, value*100/128);
		break;
	default:
		return 0;
	}

	return txt;
}

#if 0
void fungus::MidiNote(int const channel, int const value, int const velocity)
{
	/*
	int v2;
	if(aval.MIDIChannel != 0) 
		if(channel != aval.MIDIChannel-1)
			return;

	v2 = value + aval.MIDITranspose-24;

	if (v2 / 12 > 9)
		return;

	byte n = ((v2 / 12) << 4) | ((v2 % 12) + 1);

	if (velocity > 0)
	{
		for (int c = 0; c < numTracks; c++)
		{
			if (t[c].Note == zzub_note_value_none) 
			{
				t[c].Note = n;
				int vol = 255;
				if(aval.MIDIVelocity == 1) {// 0 = ignore velocity
					vol = velocity;//<<20;

					if(vol > 254) vol = 254;
				}
				if((pCB->GetStateFlags() & SF_RECORDING) && (pCB->GetStateFlags() & SF_PLAYING)) {
					CSequence *seq = pCB->GetPlayingSequence(ThisMachine);
					if(seq) {
						byte *data = (byte*)pCB->GetPlayingRow(seq, 2, c);
						data[0] = n;
						data[1] = vol;
					}
				}
				t[c].Tick(n, vol, 0, -1, 255);
				t[c].active = true;
				active = true;
				return;
			}
		}
	}
	else
	{
		for (int c = 0; c < numTracks; c++)
		{
			if (t[c].Note == n)
			{
				if((pCB->GetStateFlags() & SF_RECORDING) && (pCB->GetStateFlags() & SF_PLAYING)) {
					CSequence *seq = pCB->GetPlayingSequence(ThisMachine);
					if(seq) {
						byte *data = (byte*)pCB->GetPlayingRow(seq, 2, c);
						data[0] = NOTE_OFF;
					}
				}
				t[c].Tick(255, 255, 0, -1, 255);
				return;
			}
		}
	}
	*/
}
#endif

fungus_machine_info::fungus_machine_info() {
	this->name = "ld fungus";
	this->short_name = "fungus";
	this->author = "Lauri Koponen <ld0d@iki.fi>";
	this->uri = "@ld/fungus;1";
	this->min_tracks = 1;
	this->max_tracks = 16;
	this->flags = zzub_plugin_flag_has_audio_output;
	this->commands = "Edit amp envelope...\nEdit cutoff 1 envelope...\nEdit cutoff 2 envelope...\nEdit FM envelope...\nEdit pitch envelope...\nEdit mix envelope...\nEdit LFO1 envelope...\nEdit LFO2 envelope...\nAbout...";
	this->outputs = 2;
	this->inputs = 0;

	paraGain = &add_global_parameter()
		.set_byte()
		.set_name("GAIN")
		.set_description("Output gain")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraPitchbend = &add_global_parameter()
		.set_byte()
		.set_name("PITCHBEND")
		.set_description("Pitch bend")
		.set_value_min(0)
		.set_value_max(127)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraPitchbendRange = &add_global_parameter()
		.set_byte()
		.set_name("PITCHBEND: range")
		.set_description("Pitch bend range")
		.set_value_min(1)
		.set_value_max(12)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(2);

	paraWaveForm1 = &add_global_parameter()
		.set_byte()
		.set_name("OSC1: wave")
		.set_description("Oscillator 1 Waveform")
		.set_value_min(0)
		.set_value_max(66)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(1);

	paraO1PhaseShift = &add_global_parameter()
		.set_byte()
		.set_name("OSC1: phase")
		.set_description("Oscillator 1 Phase Shift")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	/*paraO1PhaseModType = &add_global_parameter()
		.set_byte()
		.set_name("O1PType")
		.set_description("Oscillator 1 Phase Modulation Type")
		.set_value_min(0)
		.set_value_max(2)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);*/


	paraWaveForm2 = &add_global_parameter()
		.set_byte()
		.set_name("OSC2: wave")
		.set_description("Oscillator 2 Waveform")
		.set_value_min(0)
		.set_value_max(66)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(1);

	paraO2PhaseShift = &add_global_parameter()
		.set_byte()
		.set_name("OSC2: phase")
		.set_description("Oscillator 2 Phase Shift")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	/*paraO2PhaseModType = &add_global_parameter()
		.set_byte()
		.set_name("O2PType")
		.set_description("Oscillator 2 Phase Modulation Type")
		.set_value_min(0)
		.set_value_max(2)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);*/


	paraDeTuneSemi = &add_global_parameter()
		.set_byte()
		.set_name("OSC2: semi dt")
		.set_description("Oscillator 2 Semi-detune")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraDeTuneFine = &add_global_parameter()
		.set_byte()
		.set_name("OSC2: fine dt")
		.set_description("Oscillator 2 Fine-detune")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraSync = &add_global_parameter()
		.set_byte()
		.set_name("OSC: sync")
		.set_description("Sync osc2 wave start to osc1 waveform")
		.set_value_min(0)
		.set_value_max(3)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraFMmode = &add_global_parameter()
		.set_byte()
		.set_name("FM: mode")
		.set_description("FM mode")
		.set_value_min(0)
		.set_value_max(1)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraFM = &add_global_parameter()
		.set_byte()
		.set_name("FM: amount")
		.set_description("FM amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	/*paraFMMorph = &add_global_parameter()
		.set_byte()
		.set_name("FM; morph")
		.set_description("FMEnv Morphing")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);*/

	paraFMEnvMod = &add_global_parameter()
		.set_byte()
		.set_name("FM: envmod")
		.set_description("FM EnvMod amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraOscSpread = &add_global_parameter()
		.set_byte()
		.set_name("STEREO")
		.set_description("Stereo spread")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraMix = &add_global_parameter()
		.set_byte()
		.set_name("MIX")
		.set_description("Oscillator Mix")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraMixEnvMod = &add_global_parameter()
		.set_byte()
		.set_name("MIX: envmod")
		.set_description("Oscillator Mix EnvMod amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	/*paraMixMorph = &add_global_parameter()
		.set_byte()
		.set_name("MIX: morph")
		.set_description("Oscillator Mix Envelope morph")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);*/

	paraMixMode = &add_global_parameter()
		.set_byte()
		.set_name("MIX: mode")
		.set_description("Oscillator mixing mode")
		.set_value_min(0)
		.set_value_max(2)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraSubOscWave = &add_global_parameter()
		.set_byte()
		.set_name("SUBOSC: wave")
		.set_description("Sub-oscillator waveform")
		.set_value_min(0)
		.set_value_max(3)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(1);

	paraSubOscVol = &add_global_parameter()
		.set_byte()
		.set_name("SUBOSC: vol")
		.set_description("Sub-oscillator volume")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	/*paraPitchMorph = &add_global_parameter()
		.set_byte()
		.set_name("PITCH: morph")
		.set_description("PitchEnv Morphing")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);*/

	paraPMmode = &add_global_parameter()
		.set_byte()
		.set_name("PITCH: mode")
		.set_description("PM mode")
		.set_value_min(0)
		.set_value_max(1)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraPitchEnvMod = &add_global_parameter()
		.set_byte()
		.set_name("PITCH: envmod")
		.set_description("Pitch EnvMod amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraGlide = &add_global_parameter()
		.set_byte()
		.set_name("GLIDE: amount")
		.set_description("Glide Amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraGlideRetrig = &add_global_parameter()
		.set_byte()
		.set_name("GLIDE: retrig")
		.set_description("Retrigger while gliding")
		.set_value_min(0)
		.set_value_max(1)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(1);

	paraFilterType1 = &add_global_parameter()
		.set_byte()
		.set_name("F1: type")
		.set_description("Filter 1 Type")
		.set_value_min(0)
		.set_value_max(6)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(1);

	paraCutoff1 = &add_global_parameter()
		.set_byte()
		.set_name("F1: cutoff")
		.set_description("Filter 1 Cutoff Frequency")
		.set_value_min(0)
		.set_value_max(127)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(127);

	paraResonance1 = &add_global_parameter()
		.set_byte()
		.set_name("F1: resonance")
		.set_description("Filter 1 Resonance/bandwidth")
		.set_value_min(0)
		.set_value_max(127)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(32);

	/*paraCutMorph1 = &add_global_parameter()
		.set_byte()
		.set_name("F1: morph")
		.set_description("CutoffEnv 1 Morphing")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);*/

	paraFilterKeytrack1 = &add_global_parameter()
		.set_byte()
		.set_name("F1: keytrack")
		.set_description("Filter 1 Keytrack amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraFilterEnvMod1 = &add_global_parameter()
		.set_byte()
		.set_name("F1: envmod")
		.set_description("Filter 1 EnvMod amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraDistType = &add_global_parameter()
		.set_byte()
		.set_name("DIST: type")
		.set_description("Distortion type")
		.set_value_min(0)
		.set_value_max(3)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraDistAmount = &add_global_parameter()
		.set_byte()
		.set_name("DIST: amount")
		.set_description("Distortion amount")
		.set_value_min(1)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(1);

	paraFilterType2 = &add_global_parameter()
		.set_byte()
		.set_name("F2: type")
		.set_description("Filter 2 Type")
		.set_value_min(0)
		.set_value_max(6)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraCutoff2 = &add_global_parameter()
		.set_byte()
		.set_name("F2: cutoff")
		.set_description("Filter 2 Cutoff Frequency")
		.set_value_min(0)
		.set_value_max(127)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(127);

	paraResonance2 = &add_global_parameter()
		.set_byte()
		.set_name("F2: resonance")
		.set_description("Filter 2 Resonance/bandwidth")
		.set_value_min(0)
		.set_value_max(127)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(32);

	/*paraCutMorph2 = &add_global_parameter()
		.set_byte()
		.set_name("F2: morph")
		.set_description("CutoffEnv 2 Morphing")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);*/

	paraFilterKeytrack2 = &add_global_parameter()
		.set_byte()
		.set_name("F2: keytrack")
		.set_description("Filter 2 Keytrack amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraFilterEnvMod2 = &add_global_parameter()
		.set_byte()
		.set_name("F2: envmod")
		.set_description("Filter 2 EnvMod amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraFilterSetup = &add_global_parameter()
		.set_byte()
		.set_name("F: setup")
		.set_description("Filter organization")
		.set_value_min(0)
		.set_value_max(127)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	/*paraAmpMorph = &add_global_parameter()
		.set_byte()
		.set_name("AMP: morph")
		.set_description("AmpEnv Morphing")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);*/

	paraLFO1Waveform = &add_global_parameter()
		.set_byte()
		.set_name("LFO1: wave")
		.set_description("LFO1 Waveform")
		.set_value_min(0)
		.set_value_max(6)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO1Freq = &add_global_parameter()
		.set_byte()
		.set_name("LFO1: freq")
		.set_description("LFO1 Frequency")
		.set_value_min(0)
		.set_value_max(130)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO1Phase = &add_global_parameter()
		.set_byte()
		.set_name("LFO1: phase")
		.set_description("LFO1 Phase")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO1Amplitude = &add_global_parameter()
		.set_byte()
		.set_name("LFO1; amp")
		.set_description("LFO1 Amplitude")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO1AmpEnvMod = &add_global_parameter()
		.set_byte()
		.set_name("LFO1: amp envmod")
		.set_description("LFO1 Amplitude Envelope Modulation")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraLFO1Target = &add_global_parameter()
		.set_byte()
		.set_name("LFO1: trgt")
		.set_description("LFO1 Target")
		.set_value_min(0)
		.set_value_max(13)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO1Mode = &add_global_parameter()
		.set_byte()
		.set_name("LFO1: mode")
		.set_description("LFO1 Mode")
		.set_value_min(0)
		.set_value_max(1)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO2Waveform = &add_global_parameter()
		.set_byte()
		.set_name("LFO2: wave")
		.set_description("LFO2 Waveform")
		.set_value_min(0)
		.set_value_max(6)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO2Freq = &add_global_parameter()
		.set_byte()
		.set_name("LFO2: freq")
		.set_description("LFO2 Frequency")
		.set_value_min(0)
		.set_value_max(130)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO2Phase = &add_global_parameter()
		.set_byte()
		.set_name("LFO2: phase")
		.set_description("LFO2 Phase")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO2Amplitude = &add_global_parameter()
		.set_byte()
		.set_name("LFO2: amp")
		.set_description("LFO2 Amplitude")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO2AmpEnvMod = &add_global_parameter()
		.set_byte()
		.set_name("LFO2: amp envmod")
		.set_description("LFO2 Amplitude Envelope Modulation")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraLFO2Target = &add_global_parameter()
		.set_byte()
		.set_name("LFO2: trgt")
		.set_description("LFO2 Target")
		.set_value_min(0)
		.set_value_max(13)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraLFO2Mode = &add_global_parameter()
		.set_byte()
		.set_name("LFO2: mode")
		.set_description("LFO2 Mode")
		.set_value_min(0)
		.set_value_max(1)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);

	paraNumOsc = &add_global_parameter()
		.set_byte()
		.set_name("UNISON: voices")
		.set_description("Unison: Number of detuned oscillator sets per track")
		.set_value_min(1)
		.set_value_max(16)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(1);

	paraDetune = &add_global_parameter()
		.set_byte()
		.set_name("UNISON: detune")
		.set_description("Unison detune amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(20);

	paraDetunePan = &add_global_parameter()
		.set_byte()
		.set_name("UNISON: pan")
		.set_description("Unison panning amount")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	paraDetunePan = &add_global_parameter()
		.set_byte()
		.set_name("UNISON: mode")
		.set_description("Unison mode")
		.set_value_min(0)
		.set_value_max(4)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);



	paraNote = &add_track_parameter()
		.set_note()
		.set_name("note")
		.set_description("Note");
		//.set_value_min(zzub::note_value_min)
		//.set_value_max(zzub::note_value_max)
		//.set_value_none(0)
		//.set_flags(0)
		//.set_value_default(0);

	paraVolume = &add_track_parameter()
		.set_byte()
		.set_name("volume")
		.set_description("Volume")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(128);

	paraDelay = &add_track_parameter()
		.set_byte()
		.set_name("delay")
		.set_description("Note delay")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_flags(0)
		.set_value_default(0);

	paraCut = &add_track_parameter()
		.set_byte()
		.set_name("cut")
		.set_description("Note cut")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_flags(0)
		.set_value_default(0);

	paraPan = &add_track_parameter()
		.set_byte()
		.set_name("pan")
		.set_description("Note pan")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(64);

	/*paraMorph = &add_track_parameter()
		.set_byte()
		.set_name("morph")
		.set_description("Note morph")
		.set_value_min(0)
		.set_value_max(128)
		.set_value_none(0xff)
		.set_state_flag()
		.set_value_default(0);*/
}

zzub::plugin* fungus_machine_info::create_plugin() { return new fungus(); }
bool fungus_machine_info::store_info(zzub::archive *data) const { return false; }


