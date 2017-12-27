/*
Copyright (C) 2003-2007 Anders Ervik <calvin@countzero.no>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <string>

#if defined(POSIX)
#include <dlfcn.h>
#endif

#if defined(__GNUC__)
// it seems these defines are also missing from the gnu headers
#endif

#include <zzub/plugin.h>
#include "convertsample.h"

void Amp(float *pout, int numsamples, float amp) {
	for (int i=0; i<numsamples; i++) {
		pout[i]*=amp;
	}
}

void add_samples(float *pout, float *pin, int numsamples, float amp) {
	do {
		*pout++ += *pin++ * amp;
	} while(--numsamples);
}

void dspamp(float *pout, int numsamples, float amp) {
	for (int i = 0; i < numsamples; i++) {
		pout[i] *= amp;
	}
}

void dspcopyamp(float* pout, const float* pin, int numsamples, float amp) {
	for (int i = 0; i < numsamples; i++) {
		pout[i] = pin[i] * amp;
	}
}

void AddM2SPan(float* output, float* input, int numSamples, float inAmp, float inPan) {
	float panR=1.0f, panL=1.0f;
	if (inPan<1) {
		panR=inPan;	// when inPan<1, fade out right
	}
	if (inPan>1) {
		panL=2-inPan;	// when inPan>1, fade out left
	}
	for (int i=0; i<numSamples; i++) {
		float L=input[i]*panL * inAmp;
		float R=input[i]*panR * inAmp;

		output[i*2]+=L;
		output[i*2+1]+=R;

	}
}

void AddS2SPan(float* output, float* input, int numSamples, float inAmp, float inPan) {
	float panR=1.0f, panL=1.0f;
	if (inPan<1) {
		panR=inPan;	// when inPan<1, fade out right
	}
	if (inPan>1) {
		panL=2-inPan;	// when inPan>1, fade out left
	}
	for (int i=0; i<numSamples; i++) {
		float L=input[i*2]*panL * inAmp;
		float R=input[i*2+1]*panR * inAmp;

		output[i*2]+=L;
		output[i*2+1]+=R;

	}
}

void AddS2SPanMC(float** output, float** input, int numSamples, float inAmp, float inPan) {
	if (!numSamples)
		return;
	float panR=1.0f, panL=1.0f;
	if (inPan<1) {
		panR=inPan;	// when inPan<1, fade out right
	}
	if (inPan>1) {
		panL=2-inPan;	// when inPan>1, fade out left
	}
	float *pI0 = input[0];
	float *pI1 = input[1];
	float *pO0 = output[0];
	float *pO1 = output[1];
	do
	{
		*pO0++ += *pI0++ * panL * inAmp;
		*pO1++ += *pI1++ * panR * inAmp;
	} while (--numSamples);
}

int sizeFromWaveFormat(int waveFormat) {
	switch (waveFormat) {
		case zzub_wave_buffer_type_si16:
			return 2;
		case zzub_wave_buffer_type_si24:
			return 3;
		case zzub_wave_buffer_type_f32:
		case zzub_wave_buffer_type_si32:
			return 4;
		default:
			return -1;
	}
}

// disse trenger vi for lavnivå redigering på flere typer bitformater, waveFormat er buzz-style
// det er kanskje mulig å oppgradere copy-metodene med en interleave på hver buffer for å gjøre konvertering mellom stereo/mono integrert
void CopyMonoToStereoEx(void* srcbuf, void* targetbuf, size_t numSamples, int waveFormat) {

	int sampleSize = sizeFromWaveFormat(waveFormat);
	char* tbl=(char*)targetbuf;
	char* tbr=(char*)targetbuf;
	tbr+=sampleSize;
	char* sb=(char*)srcbuf;

	int temp;

	for (size_t i=0; i<numSamples; i++) {
		switch (waveFormat) {
			case zzub_wave_buffer_type_si16:
				*((short*)tbr)=*((short*)tbl)=*(short*)sb;
				break;
			case zzub_wave_buffer_type_si24:
				temp=(*(int*)sb) >> 8;
				*((int*)tbr)=*((int*)tbl)=temp;
				break;
			case zzub_wave_buffer_type_f32:
			case zzub_wave_buffer_type_si32:
				temp=*(int*)sb;
				*((int*)tbr)=*((int*)tbl)=temp;
				break;
		}
		tbl+=sampleSize*2;
		tbr+=sampleSize*2;
		sb+=sampleSize;
	}
}

void CopyStereoToMonoEx(void* srcbuf, void* targetbuf, size_t numSamples, int waveFormat) {
}

// from 16 bit conversion
void Copy16To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((short*)srcbuf, (S24*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy16ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((short*)srcbuf, (int*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy16ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((short*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}


// from 32 bit floating point conversion
void CopyF32To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((float*)srcbuf, (short*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyF32To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((float*)srcbuf, (S24*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyF32ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((float*)srcbuf, (int*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}


// from 32 bit integer conversion
void CopyS32To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((int*)srcbuf, (short*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyS32To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((int*)srcbuf, (S24*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyS32ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((int*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}


// from 24 bit integer conversion
void Copy24To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((S24*)srcbuf, (short*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy24ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((S24*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy24ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((S24*)srcbuf, (int*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((short*)srcbuf, (short*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((S24*)srcbuf, (S24*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((int*)srcbuf, (int*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesT((float*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

    //~ zzub_wave_buffer_type_si16	= 0,    // signed int 16bit
    //~ zzub_wave_buffer_type_f32	= 1,    // float 32bit
    //~ zzub_wave_buffer_type_si32	= 2,    // signed int 32bit
    //~ zzub_wave_buffer_type_si24	= 3,    // signed int 24bit

typedef void (*CopySamplesPtr)(void *, void *, size_t, size_t, size_t, size_t, size_t);

CopySamplesPtr CopySamplesMatrix[4][4] = {
	// si16 -> si16, f32, si32, si24
	Copy16, Copy16ToF32, Copy16ToS32, Copy16To24,
	// f32 -> si16, f32, si32, si24
	CopyF32To16, CopyF32, CopyF32ToS32, CopyF32To24,
	// si32 -> si16, f32, si32, si24
	CopyS32To16, CopyS32ToF32, CopyS32, CopyS32To24,
	// si24 -> si16, f32, si32, si24
	Copy24To16, Copy24ToF32, Copy24ToS32, Copy24,
};

// auto select based on waveformat
void CopySamples(void *srcbuf, void *targetbuf, size_t numSamples, int srcWaveFormat, int dstWaveFormat, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset) {
	CopySamplesMatrix[srcWaveFormat][dstWaveFormat](srcbuf, targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}
