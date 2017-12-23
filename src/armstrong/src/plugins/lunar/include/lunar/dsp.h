#if !defined(LUNAR_DSP_H)
#define LUNAR_DSP_H

#if defined(__cplusplus)
extern "C" {
#endif

inline float dsp_slope(float *b, int ns, float start, float step) {	
	while (ns--) {
		*b++ = start;
		start += step;
	}
	return start;
}

inline void dsp_amp(float *b, int numsamples, float s) {
	while (numsamples--) {
		*b++ = (*b) * s;
	}
}
	
inline void dsp_copy(float *i, float *o, int numsamples) {
	while (numsamples--) {
		*o++ = *i++;
	}
}

inline void dsp_add(float *i, float *o, int numsamples) {
	while (numsamples--) {
		*o++ += *i++;
	}
}

inline void dsp_mul(float *i, float *o, int numsamples) {
	while (numsamples--) {
		*o++ *= *i++;
	}
}

inline void dsp_powmap(float *b, int numsamples, float c, float base, float offset, float factor) {
	while (numsamples--) {
		*b++ = c * pow(base, offset + (*b * factor));
	}
}

inline void dsp_copyamp(float *i, float *o, int numsamples, float s) {
	while (numsamples--) {
		*o++ = *i++ * s;
	}
}

inline void dsp_addamp(float *i, float *o, int numsamples, float s) {
	while (numsamples--) {
		*o++ += *i++ * s;
	}
}

inline void dsp_set(float *b, int numsamples, float s) {
	while (numsamples--) {
		*b++ = s;
	}
}

inline void dsp_clip(float *b, int numsamples, float s) {
	while (numsamples--) {
		if (*b > s)
			*b = s;
		if (*b < -s)
			*b = -s;
		*b++;
	}
}

inline void dsp_zero(float *b, int numsamples) {
	while (numsamples--) {
		*b++ = 0;
	}
}

inline float dbtoamp(float db, float limit) {
	if (db <= limit)
		return 0.0f;
	return pow(10.0f, db / 20.0f);
}

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // LUNAR_DSP_H
