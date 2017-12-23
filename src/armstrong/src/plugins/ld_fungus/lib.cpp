
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"

float clamp(float x)
{
	if(x < 0) return 0;
	else if(x > 1) return 1;

	return x;
}

float clamp2(float x)
{
	if(x < -1) return -1;
	else if(x > 1) return 1;

	return x;
}

#define MUL 2.0
//65536.0
double otanh(double x)
{
	//static double dc = tanh(4.0);
	//return MUL * tanh(x * (1.0 / MUL));

	double a = fabs(2.0*x);
	a=24.0+a*(12.0+a*(6.0+a*(3.0+a)));

	if(x < 0)
		return (-1.0+24.0/a);
	else
		return (1.0-24.0/a);
}

#undef MUL
#define MUL 32768.0
//65536.0
double otanh_32768(double x)
{
	//static double dc = tanh(4.0);
	return MUL * otanh(x * (1.0 / MUL));
}

short *wavetable = 0;
short *lfotable = 0;

void initwavetable(void)
{
	int i, l;

	if(wavetable != 0) return;

	wavetable = (short*)malloc(2048 * 66 * 2 * 9);
	lfotable = (short*)malloc(2048 * 5 * 2);
	memset(wavetable, 0, 2048 * 66 * 2 * 9);
	short *wavetab = wavetable;

	int numlev = 512;

	for(int level = 0; level < 9; level++) {

		for(i = 0; i < 2048; i++)
			wavetab[i] = (short) (sin((double)i * (PI / 1024.0)) * 32767.0);

		for(l = 1; l < numlev; l++)
			for(i = 0; i < 2048; i++)
				wavetab[i + 2048] -= (short) (sin((double)(l * i) * (PI / 1024.0)) * (32767.0 / ((double)l * PI)));
			
		for(l = 1; l < numlev; l += 2)
			for(i = 0; i < 2048; i++) {
				wavetab[i + 4096] -= (short) (cos((double)(l * (i + 512)) * (PI / 1024.0)) * (32767.0 * 2.0 / (l * PI) * sin(l * PI * 0.5)));
				wavetab[i + 6144] += (short) (cos((double)(l * (i + 512)) * (PI / 1024.0)) * (4.0 * 32767.0 / (double)(l * l * (PI * PI))));
			}

		numlev >>= 1;
		wavetab += 2048 * 66;
	}

	for(i = 0; i < 2048; i++) {
		lfotable[i] = (short) (sin((double)i * (PI / 1024.0)) * 32767.0);
		lfotable[i + 2048] = (i - 1024) * 32;
		lfotable[i + 4096] = (1023 - i) * 32;
		lfotable[i + 6144] = (i < 1024) ? -32768 : 32767;
		lfotable[i + 8192] = (i < 1024) ? (i - 512) * 64 : (1535 - i) * 64;
	}
}
