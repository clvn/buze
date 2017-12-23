
#include <math.h>
#include <stdlib.h>


#include "filter2p.h"

void filter2p::Compute(double f, double q, int t) {
	double omega = 2.f*3.141592654f*f;
	double sn = (float)sin(omega);
	double cs = (float)cos(omega);
	double alpha;
	
	if( t<2)
		alpha = sn / q;
	else
		alpha = sn * (float)sinh( q * omega/sn);

	double a0;
	
	switch( t) {
	case 0: // LP
		b0 =  (1 - cs)/2;
		b1 =   1 - cs;
		b2 =  (1 - cs)/2;
		a0 =   1 + alpha;
		a1 =  -2*cs;
		a2 =   1 - alpha;
		break;
	case 1: // HP
		b0 =  (1 + cs)/2;
		b1 = -(1 + cs);
		b2 =  (1 + cs)/2;
		a0 =   1 + alpha;
		a1 =  -2*cs;
		a2 =   1 - alpha;
		break;
	case 2: // BP
		b0 =   alpha;
		b1 =   0;
		b2 =  -alpha;
		a0 =   1 + alpha;
		a1 =  -2*cs;
		a2 =   1 - alpha;
		break;
	case 3: // BR
		b0 =   1;
		b1 =  -2*cs;
		b2 =   1;
		a0 =   1 + alpha;
		a1 =  -2*cs;
		a2 =   1 - alpha;
		break;
	}

	b0 /= a0;
	b1 /= a0;
	b2 /= a0;
	a1 /= -a0;
	a2 /= -a0;
}

void filter2p::ComputeCoefs( double *coefs, int freq, int r, int t, int SampleRate)
{
	//double f = (pow(50, (freq+1)/(128.f))*380 - 361)/(double)SampleRate;
	double gaincomp = 1.0;
	double f = 440.0 * pow(2.0, (((double)freq/4.0) - 62.0) / 12.0) / (double)SampleRate;
	double q;
	if(t<2) {
		q = pow(256.0, (double)r / 127.0);//(pow(((double)(r + 30) * (98.0 / 128.0) * (((f * (double)SampleRate/19000.f) + 1.9) / 2.9))/127.0, 4)*160.f+0.1f);
		gaincomp = pow(0.25, (double)r / 127.0);
	} else {
		q = (pow( (127-r)/127.0, 6)*4+0.1);
		gaincomp = pow(2.0, (double)r / 127.0);
	}

	Compute(f, q, t);

	coefs[0] = b0 * gaincomp;
	coefs[1] = b1 * gaincomp;
	coefs[2] = b2 * gaincomp;
	coefs[3] = a1;
	coefs[4] = a2;
}

void filter2p::init(int SampleRate, double **coefstab) {
	
	if(*coefstab == 0) {
		filtercoefsTab = (double*)malloc(8*4*512*128*8);
		*coefstab = filtercoefsTab;
		
		for( int t=0; t<4; t++)
			for( int f=0; f<512; f++)
				for( int r=0; r<128; r++)
					ComputeCoefs( filtercoefsTab+(t*512*128+f*128+r)*8, f, r, t, SampleRate);
				
				
	} else {
		filtercoefsTab = *coefstab;
	}
			
	coefsTabOffs = filtercoefsTab; // lp
	reset();
}

void filter2p::settype(unsigned char type) {
	if(filtertype == type) return;
	
	coefsTabOffs = filtercoefsTab + type * 512 * 128 * 8;
	filtertype = type;
	reset();
}

void filter2p::dofilter(float *data, int length, float cutoffvalue, float resonancevalue) {
	
	int c;
	c = (int)(cutoffvalue * 127.0 * 4.0);
	if(c < 0) c = 0;
	else if(c > 511) c = 511;
	
	int r = (int)(resonancevalue * 127);
	if(r < 0) r = 0;
	else if(r > 127) r = 127;
	
	int ofs = ((c << 7) + r) << 3;
	
	while(length--) {

		float in = *data;
		*data = coefsTabOffs[ofs] * in + d0;
		d0 = coefsTabOffs[ofs+1] * in + coefsTabOffs[ofs+3] * (*data) + d1;
		d1 = coefsTabOffs[ofs+2] * in + coefsTabOffs[ofs+4] * (*data);
		
		data++;
	}
}

void filter2p::dofilter_s(float *data, int length, float cutoffvalue, float resonancevalue) {
	
	int c;
	c = (int)(cutoffvalue * 127.0 * 4.0);
	if(c < 0) c = 0;
	else if(c > 511) c = 511;
	
	int r = (int)floor(resonancevalue * 127 + 0.5);
	if(r < 0) r = 0;
	else if(r > 127) r = 127;
	
	int ofs = ((c << 7) + r) << 3;
	
	while(length--) {

		float in = *data;
		*data = coefsTabOffs[ofs] * in + d0;
		d0 = coefsTabOffs[ofs+1] * in + coefsTabOffs[ofs+3] * (*data) + d1;
		d1 = coefsTabOffs[ofs+2] * in + coefsTabOffs[ofs+4] * (*data);
		
		data++;

		in = *data;
		*data = coefsTabOffs[ofs] * in + d0r;
		d0r = coefsTabOffs[ofs+1] * in + coefsTabOffs[ofs+3] * (*data) + d1r;
		d1r = coefsTabOffs[ofs+2] * in + coefsTabOffs[ofs+4] * (*data);
		
		data++;
	}
}
