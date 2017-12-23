#ifndef __FFT_H_
#define __FFT_H_

void fft(float *re, float *im, int m);
void inverse_fft(float *re, float *im, int m);
void real_fft(float *re, float *im, int m);
void inverse_real_fft(float *re, float *im, int m);

#endif
