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

#pragma once

#include <algorithm>

size_t sizeFromWaveFormat(int waveFormat);

// buffer tools
//void AddS2SPanMC(float** output, float** input, int numSamples, float inAmp, float inPan);
void Amp(float *pout, int numsamples, float amp);
void add_samples(float *pout, float *pin, int numsamples, float amp);
void dspamp(float *pout, int numsamples, float amp);
void dspcopyamp(float* pout, const float* pin, int numsamples, float amp);

void CopyMonoToStereoEx(void* srcbuf, void* targetbuf, size_t numSamples, int waveFormat);
void CopyStereoToMonoEx(void* srcbuf, void* targetbuf, size_t numSamples, int waveFormat);

// from 16 bit conversion
void Copy16To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void Copy16ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void Copy16ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);

// from 32 bit floating point conversion
void CopyF32To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void CopyF32To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void CopyF32ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);

// from 32 bit integer conversion
void CopyS32To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void CopyS32To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void CopyS32ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);

// from 24 bit integer conversion
void Copy24To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void Copy24ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void Copy24ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);

// trivial conversions
void Copy16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void Copy24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void CopyS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);
void CopyF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);

#pragma pack(push, 1)
struct S24 {
	union {
		struct {
			unsigned char c3[3];
		};
		struct {
			unsigned short s;
			unsigned char c;
		};
	};
};
#pragma pack(pop)

// auto select based on waveformat
void CopySamples(void *srcbuf, void *targetbuf, size_t numSamples, int srcWaveFormat, int dstWaveFormat, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0);

inline void ConvertSample(const short &src, short &dst) { dst = src; }
inline void ConvertSample(const short &src, S24 &dst) { dst.s = src; dst.c = 0; }
inline void ConvertSample(const short &src, int &dst) { dst = (int)src * (1<<16); }
inline void ConvertSample(const short &src, float &dst) { dst = (float)src / 32767.0f; }

inline void ConvertSample(const S24 &src, short &dst) { dst = src.s; }
inline void ConvertSample(const S24 &src, S24 &dst) { dst = src; }
inline void ConvertSample(const S24 &src, int &dst) { 
	dst = src.c3[0] | (src.c3[1] << 8) | (src.c3[2] << 16);
	if (dst & 0x800000) dst |= ~0xffffff;
	dst <<= 8;
}
inline void ConvertSample(const S24 &src, float &dst) { 
	int i = src.c3[0] | (src.c3[1] << 8) | (src.c3[2] << 16);
	if (i & 0x800000) i |= ~0xffffff;
	dst = (float)i * (1.0f / 8388607.5f);
}

inline void ConvertSample(const int &src, short &dst) { dst = (short)(src / (1<<16)); }
inline void ConvertSample(const int &src, S24 &dst) { 
	dst.c3[0] = (src & 0x0000ff00) >> 8;
	dst.c3[1] = (src & 0x00ff0000) >> 16;
	dst.c3[2] = (src & 0xff000000) >> 24;
}
inline void ConvertSample(const int &src, int &dst) { dst = src; }
inline void ConvertSample(const int &src, float &dst) { dst = (float)src / 2147483648.0f; }

inline void ConvertSample(const float &src, short &dst) { dst = (short)(std::max(std::min(src,1.0f),-1.0f) * 32767.0f); }
inline void ConvertSample(const float &src, S24 &dst) { 	
	int i = (int)(src * 0x007fffff);
	dst.c3[0] = (i & 0x000000ff);
	dst.c3[1] = (i & 0x0000ff00) >> 8;
	dst.c3[2] = (i & 0x00ff0000) >> 16;
}
inline void ConvertSample(const float &src, int &dst) { dst = (int)(std::max(std::min(src,1.0f),-1.0f) * 2147483648.0f); }
inline void ConvertSample(const float &src, float &dst) { dst = src; }

template <typename srctype, typename dsttype>
inline void CopySamplesT(const srctype *src, dsttype *dst, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0) {
	src += srcoffset;
	dst += dstoffset;
	while (numSamples--) {
		ConvertSample(*src, *dst);
		src += srcstep;
		dst += dststep;
	}
}
