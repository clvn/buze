#include <algorithm>
#include <cassert>
#include <stdint.h>
#include "convertsample.h"

#pragma pack(push, 1)
struct S24 {
	union {
		struct {
			uint8_t c3[3];
		};
		struct {
			uint16_t s;
			uint8_t c;
		};
	};
};
#pragma pack(pop)

inline void convert_sample(const int16_t &src, int16_t &dst) { 
	dst = src; 
}

inline void convert_sample(const int16_t &src, S24 &dst) { 
	dst.s = src; 
	dst.c = 0; 
}

inline void convert_sample(const int16_t &src, int32_t &dst) { 
	dst = (int)src * (1<<16); 
}

inline void convert_sample(const int16_t &src, float &dst) { 
	dst = (float)src / 32767.0f; 
}

inline void convert_sample(const int16_t &src, uint8_t &dst) { 
	assert(false);
}

inline void convert_sample(const int16_t &src, int8_t &dst) { 
	assert(false);
}

inline void convert_sample(const S24 &src, int16_t &dst) { 
	dst = src.s; 
}

inline void convert_sample(const S24 &src, S24 &dst) { 
	dst = src; 
}

inline void convert_sample(const S24 &src, int32_t &dst) { 
	dst = src.c3[0] | (src.c3[1] << 8) | (src.c3[2] << 16);
	if (dst & 0x800000) dst |= ~0xffffff;
	dst <<= 8;
}

inline void convert_sample(const S24 &src, float &dst) { 
	int i = src.c3[0] | (src.c3[1] << 8) | (src.c3[2] << 16);
	if (i & 0x800000) i |= ~0xffffff;
	dst = (float)i * (1.0f / 8388607.5f);
}

inline void convert_sample(const S24 &src, uint8_t &dst) { 
	assert(false);
}

inline void convert_sample(const S24 &src, int8_t &dst) { 
	assert(false);
}

inline void convert_sample(const int32_t &src, int16_t &dst) { 
	dst = (int16_t)(src / (1<<16)); 
}

inline void convert_sample(const int32_t &src, S24 &dst) { 
	dst.c3[0] = (src & 0x0000ff00) >> 8;
	dst.c3[1] = (src & 0x00ff0000) >> 16;
	dst.c3[2] = (src & 0xff000000) >> 24;
}

inline void convert_sample(const int32_t &src, int32_t &dst) { 
	dst = src; 
}

inline void convert_sample(const int32_t &src, float &dst) { 
	dst = (float)src / 2147483648.0f; 
}

inline void convert_sample(const int32_t &src, uint8_t &dst) { 
	assert(false);
}

inline void convert_sample(const int32_t &src, int8_t &dst) { 
	assert(false);
}

inline void convert_sample(const float &src, int16_t &dst) { 
	dst = (int16_t)(std::max(std::min(src,1.0f),-1.0f) * 32767.0f); 
}

inline void convert_sample(const float &src, S24 &dst) { 	
	int32_t i = (int32_t)(src * 0x007fffff);
	dst.c3[0] = (i & 0x000000ff);
	dst.c3[1] = (i & 0x0000ff00) >> 8;
	dst.c3[2] = (i & 0x00ff0000) >> 16;
}

inline void convert_sample(const float &src, int32_t &dst) { 
	dst = (int32_t)(std::max(std::min(src,1.0f),-1.0f) * 2147483648.0f); 
}

inline void convert_sample(const float &src, float &dst) { 
	dst = src; 
}

inline void convert_sample(const float &src, uint8_t &dst) { 
	assert(false);
}

inline void convert_sample(const float &src, int8_t &dst) { 
	assert(false);
}

inline void convert_sample(const uint8_t &src, float &dst) { 
	dst = ((float)src - 128.0f) / 128.0f; 
}

inline void convert_sample(const int8_t &src, float &dst) { 
	dst = (float)src / 128.0f; 
}


inline void swap_bytes(void* buf, int size) {
	register char val;
	register char *ptr = (char*)buf;
	// swap bytes first<->last, first+1<->last-1, etc
	for (int i = 0; i < size / 2; i++) {
		val = ptr[i];
		ptr[i] = ptr[size - 1 - i];
		ptr[size - 1 - i] = val;
	}
}

template <typename srctype, typename dsttype>
inline void copy_samplesT(const srctype *src, dsttype *dst, size_t numSamples, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0, bool byteswap=false) {
	src += srcoffset;
	dst += dstoffset;
	while (numSamples--) {
		srctype s = *src;
		if (byteswap) swap_bytes(&s, sizeof(srctype));
		convert_sample(s, *dst);
		src += srcstep;
		dst += dststep;
	}
}

// from 16 bit conversion
void Copy16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int16_t*)srcbuf, (int16_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy16To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int16_t*)srcbuf, (S24*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy16ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int16_t*)srcbuf, (int32_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy16ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int16_t*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy16ToU8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int16_t*)srcbuf, (uint8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy16ToS8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int16_t*)srcbuf, (int8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

// from 32 bit floating point conversion
void CopyF32To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((float*)srcbuf, (int16_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyF32To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((float*)srcbuf, (S24*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyF32ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((float*)srcbuf, (int32_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((float*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyF32ToU8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((float*)srcbuf, (uint8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyF32ToS8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((float*)srcbuf, (int8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}


// from 32 bit integer conversion
void CopyS32To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int32_t*)srcbuf, (int16_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyS32To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int32_t*)srcbuf, (S24*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int32_t*)srcbuf, (int32_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}
void CopyS32ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int32_t*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyS32ToU8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int32_t*)srcbuf, (uint8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyS32ToS8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int32_t*)srcbuf, (int8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

// from 24 bit integer conversion
void Copy24To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((S24*)srcbuf, (int16_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((S24*)srcbuf, (S24*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy24ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((S24*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy24ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((S24*)srcbuf, (int32_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy24ToU8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((S24*)srcbuf, (uint8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void Copy24ToS8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((S24*)srcbuf, (int8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

// NOTE: 8 bit converters mostly unimplemented
// from 8 bit unsigned integer conversion
void CopyU8To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((uint8_t*)srcbuf, (int16_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyU8ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((uint8_t*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyU8ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((uint8_t*)srcbuf, (int32_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyU8To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((uint8_t*)srcbuf, (S24*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyU8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((uint8_t*)srcbuf, (uint8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyU8ToS8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((uint8_t*)srcbuf, (int8_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

// from 8 bit signed integer conversion
void CopyS8To16(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int8_t*)srcbuf, (int16_t*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyS8ToF32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samplesT((int8_t*)srcbuf, (float*)targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset);
}

void CopyS8ToS32(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {}
void CopyS8To24(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {}
void CopyS8ToU8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {}
void CopyS8(void* srcbuf, void* targetbuf, size_t numSamples, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {}


typedef void (*copy_samples_callback)(void *, void *, size_t, size_t, size_t, size_t, size_t, bool);

copy_samples_callback copy_samples_matrix[6][6] = {
	// si16 -> si16, f32, si32, si24, si8
	Copy16, Copy16ToF32, Copy16ToS32, Copy16To24, Copy16ToU8, Copy16ToS8,
	// f32 -> si16, f32, si32, si24, si8
	CopyF32To16, CopyF32, CopyF32ToS32, CopyF32To24, CopyF32ToU8, CopyF32ToS8,
	// si32 -> si16, f32, si32, si24, si8
	CopyS32To16, CopyS32ToF32, CopyS32, CopyS32To24, CopyS32ToU8, CopyS32ToS8,
	// si24 -> si16, f32, si32, si24, si8
	Copy24To16, Copy24ToF32, Copy24ToS32, Copy24, Copy24ToU8, Copy24ToS8,
	// u8 -> si16, f32, si32, si24, si8
	CopyU8To16, CopyU8ToF32, CopyU8ToS32, CopyU8To24, CopyU8, CopyU8ToS8,
	// s8 -> si16, f32, si32, si24, si8
	CopyS8To16, CopyS8ToF32, CopyS8ToS32, CopyS8To24, CopyS8ToU8, CopyS8,
};

// auto select based on waveformat
void copy_samples(void *srcbuf, void *targetbuf, size_t numSamples, int srcWaveFormat, int dstWaveFormat, size_t srcstep, size_t dststep, size_t srcoffset, size_t dstoffset, bool byteswap) {
	copy_samples_matrix[srcWaveFormat][dstWaveFormat](srcbuf, targetbuf, numSamples, srcstep, dststep, srcoffset, dstoffset, byteswap);
}
