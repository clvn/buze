
#pragma once

#define PI 3.1415926535897932384626433832795

#define undenormalise(sample) if(((*(unsigned int*)&(sample))&0x7f800000)==0) (sample)=0.0f

float clamp(float x);
float clamp2(float x);
double otanh(double x);
double otanh_32768(double x);

extern short *wavetable;
extern short *lfotable;
void initwavetable(void);

inline short linint(short *osc, unsigned int phase)
{
	int a = osc[phase >> 21];
	int b = osc[((phase >> 21) + 1) & 2047];
	int frac = (phase >> 6) & 32767;// (phase & ((1<<21)-1)) >> 6;

	b -= a;

	return a + ((b * frac) >> 15);
}


//Filtres décimateurs
// T.Rochebois
// Based on
//Traitement numérique du signal, 5eme edition, M Bellanger, Masson pp. 339-346
class Decimateur5
{
  private:
  float R1,R2,R3,R4,R5;
  const float h0;
  const float h1;
  const float h3;
  const float h5;
  public:
  Decimateur5::Decimateur5():h0(346/692.0f),h1(208/692.0f),h3(-44/692.0f),h5(9/692.0f)
  {
    R1=R2=R3=R4=R5=0.0f;
  }
  float Calc(const float x0,const float x1)
  {
    float h5x0=h5*x0;
    float h3x0=h3*x0;
    float h1x0=h1*x0;
    float R6=R5+h5x0;
    R5=R4+h3x0;
    R4=R3+h1x0;
    R3=R2+h1x0+h0*x1;
    R2=R1+h3x0;
    R1=h5x0;
    return R6;
  }
};
