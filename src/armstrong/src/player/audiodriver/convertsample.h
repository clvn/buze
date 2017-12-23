#pragma once

    //~ zzub_wave_buffer_type_si16	= 0,    // signed int 16bit
    //~ zzub_wave_buffer_type_f32	= 1,    // float 32bit
    //~ zzub_wave_buffer_type_si32	= 2,    // signed int 32bit
    //~ zzub_wave_buffer_type_si24	= 3,    // signed int 24bit
    //~                             = 4,    // unsigned int 8bit

// auto select based on waveformat
void copy_samples(void *srcbuf, void *targetbuf, size_t numSamples, int srcWaveFormat, int dstWaveFormat, size_t srcstep=1, size_t dststep=1, size_t srcoffset=0, size_t dstoffset=0, bool byteswap=false);
