// dsp building blocks for lua
// Copyright (C) 2006 Leonard Ritter
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include <stdarg.h>
#include <cmath>

struct _lunar_voice {
	zzub::host *cb;
	
	int instr;
	float note;
	float speed;
	int offset;
	float frac;
	const zzub::wave_info *wi;
	const zzub::wave_level *wl;
};

struct _lunar_host {
	zzub::host *cb;
	std::vector<lunar_voice_t> voices;
};

int lunar_is_envelope(lunar_host_t *h, int instr, int envelope) {
	zzub::host *host = h->cb;
	return (host->get_envelope_size(instr, envelope) >= 2)?1:0;
}

float lunar_get_envelope_sustain_position(lunar_host_t *h, int instr, int envelope) {
	zzub::host *host = h->cb;
	int size = host->get_envelope_size(instr, envelope);
	if (size < 2)
		return -1.0f;
	for (int i = 0; i < size; ++i) {
		unsigned short x, y;
		int flags;
		if (!host->get_envelope_point(instr, envelope, i, x, y, flags))
			return -1.0f;
		if (flags & zzub::envelope_flag_sustain)
			return float(x) / 65535.0f;
	}
	return -1.0f;
}

void lunar_get_envelope(lunar_host_t *h, int instr, int envelope, float start, float size, float *buffer, int n) {
	zzub::host *host = h->cb;
	int envsize = host->get_envelope_size(instr, envelope);
	if (envsize < 2)
		return;
	for (int i = 0; i < (envsize - 1); ++i) {
		unsigned short x,y; int flags;
		host->get_envelope_point(instr, envelope, i, x, y, flags);
		float x1 = float(x)/65535.0f;
		float y1 = float(y)/65535.0f;
		host->get_envelope_point(instr, envelope, i+1, x, y, flags);
		float x2 = float(x)/65535.0f;
		float y2 = float(y)/65535.0f;
		int sp1 = (int)(((x1 - start) / size) * float(n) + 0.5f);
		int sp2 = (int)(((x2 - start) / size) * float(n) + 0.5f);
		for (int j = sp1; j < sp2; ++j) {
			if ((j >= 0) && (j < n)) {
				float pos = float(j) / float(n);
				buffer[j] = y1 + (y2 - y1)*pos;
			}
		}
	}
}

lunar_voice_t *lunar_new_voice(lunar_host_t *h) {
	lunar_voice_t v;
	v.cb = h->cb;
	v.instr = -1;
	v.note = 0.0f;
	v.speed = 1.0f;
	v.offset = 0;
	v.frac = 0.0f;
	v.wi = 0;
	v.wl = 0;
	h->voices.push_back(v);
	return &h->voices.back();
}

void lunar_voice_set_instrument(lunar_voice_t *v, int instr) {
	zzub::host *host = v->cb;
	v->instr = -1;
	v->wl = 0;
	v->wi = host->get_wave(instr);
	if (v->wi)
		v->instr = instr;
}

void lunar_voice_set_note(lunar_voice_t *v, float note) {
	zzub::host *host = v->cb;
	v->note = note;
	if (v->instr != -1) {
		if (note == 0.0f) {
			v->wl = 0;
		} else {
			int midinote = 57 + int((log(note / 440.0f)*12.0f) / log(2.0f) + 0.5f);
			int buzznote = ((midinote/12)<<4) + (midinote%12)+1;
			v->wl = host->get_nearest_wave_level(v->instr, buzznote);
		}
	}
}

void lunar_voice_process_stereo(lunar_voice_t *v, float *outL, float *outR, int n) {
}

void lunar_voice_reset(lunar_voice_t *v) {
	v->speed = 1.0f;
	v->offset = 0;
	v->frac = 0.0f;
	v->note = 0.0f;
}

// stdlib
int lunar_printf(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int ret = vprintf(format, ap);
	va_end(ap);
	return ret;
}

void *lunar_memset(void *b, int c, size_t len) { return memset(b,c,len); }
void *lunar_memcpy(void *b, const void *a, size_t num) { return memcpy(b,a,num); }
void *lunar_calloc(size_t nelem, size_t elsize) { return calloc(nelem, elsize); }
void *lunar_malloc(size_t len) { return malloc(len); }
void lunar_free (void *p) { return free(p); }
void *lunar_realloc(void *ptr, size_t len) { return realloc(ptr,len); }
// math
float lunar_pow(float a, float b) { return std::pow(a,b); }
float lunar_log(float x) { return std::log(x); }
float lunar_exp(float x) { return std::exp(x); }
float lunar_abs(float x) { return std::abs(x); }
float lunar_min(float a, float b) { return std::min(a,b); }
float lunar_max(float a, float b) { return std::max(a,b); }
float lunar_sin(float x) { return std::sin(x); }
float lunar_cos(float x) { return std::cos(x); }
float lunar_tan(float x) { return std::tan(x); }
// new:
float lunar_floor(float x) { return std::floor(x); }
float lunar_ceil(float x) { return std::ceil(x); }
long int lunar_lrint(float x) { return (long int)x; }

void dsp_fixdn(float *b, int numsamples) {
	while (numsamples--) {
		if(((*(unsigned int*)b)&0x7f800000)==0)
			*b = 0.0f;
		b++;
	}
}

float dsp_slope(float *b, int ns, float start, float step) {	
	while (ns--) {
		*b++ = start;
		start += step;
	}
	return start;
}

void dsp_amp(float *b, int numsamples, float s) {
	while (numsamples--) {
		*b++ = *b * s;
	}
}
	
void dsp_copy(float *i, float *o, int numsamples) {
	memcpy(o, i, sizeof(float) * numsamples);
}

void dsp_add(float *i, float *o, int numsamples) {
	while (numsamples--) {
		*o++ += *i++;
	}
}

void dsp_mul(float *i, float *o, int numsamples) {
	while (numsamples--) {
		*o++ *= *i++;
	}
}

void dsp_powmap(float *b, int numsamples, float c, float base, float offset, float factor) {
	while (numsamples--) {
		*b++ = c * pow(base, offset + (*b * factor));
	}
}

void dsp_copyamp(float *i, float *o, int numsamples, float s) {
	while (numsamples--) {
		*o++ = *i++ * s;
	}
}

void dsp_addamp(float *i, float *o, int numsamples, float s) {
	while (numsamples--) {
		*o++ += *i++ * s;
	}
}

void dsp_set(float *b, int numsamples, float s) {
	while (numsamples--) {
		*b++ = s;
	}
}

void dsp_clip(float *b, int numsamples, float s) {
	while (numsamples--) {
		if (*b > s)
			*b = s;
		if (*b < -s)
			*b = -s;
		*b++;
	}
}

void dsp_zero(float *b, int numsamples) {
	memset(b, 0, sizeof(float) * numsamples);
}

float dbtoamp(float db, float limit) {
	if (db <= limit)
		return 0.0f;
	return pow(10.0f, db / 20.0f);
}

struct osctable {
	float *waves[11];
	
	int get_level(float sps, float freq, float &factor) {
		float l = sps / freq;
		float e = log(l) / log(2.0f);
		int level = int(e+0.5);
		factor = l / float(1<<level);
		return level;
	}
	
	osctable() {
		waves[0] = 0;
		for (int i = 1; i <= 10; ++i) {
			waves[i] = new float[1<<i];
			memset(waves[i], 0, sizeof(float) * (1<<i));
		}
	}
	
	~osctable() {
		for (int i = 1; i <= 10; ++i) {
			delete waves[i];
		}
	}
};
