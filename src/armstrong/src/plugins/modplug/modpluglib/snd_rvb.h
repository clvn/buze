#ifndef _SNDMIX_REVERB_H_
#define _SNDMIX_REVERB_H_

/////////////////////////////////////////////////////////////////////////////
//
// SW Reverb structures
//

// Length-1 (in samples) of the reflections delay buffer: 32K, 371ms@22kHz
#define SNDMIX_REFLECTIONS_DELAY_MASK	0x1fff
#define SNDMIX_PREDIFFUSION_DELAY_MASK	0x7f	// 128 samples
#define SNDMIX_REVERB_DELAY_MASK		0xfff	// 4K samples (92ms @ 44kHz)

typedef struct _SWRVBREFLECTION
{
	ULONG Delay, DelayDest;
	SHORT Gains[4];	// g_ll, g_rl, g_lr, g_rr
} SWRVBREFLECTION, *PSWRVBREFLECTION;

typedef struct _SWRVBREFDELAY
{
	ULONG nDelayPos, nPreDifPos, nRefOutPos;
	LONG lMasterGain;			// reflections linear master gain
	SHORT nCoeffs[2];			// room low-pass coefficients
	SHORT History[2];			// room low-pass history
	SHORT nPreDifCoeffs[2];		// prediffusion coefficients
	SHORT ReflectionsGain[2];	// master reflections gain
	SWRVBREFLECTION Reflections[8];	// Up to 8 SW Reflections
	short int RefDelayBuffer[(SNDMIX_REFLECTIONS_DELAY_MASK+1)*2]; // reflections delay buffer
	short int PreDifBuffer[(SNDMIX_PREDIFFUSION_DELAY_MASK+1)*2]; // pre-diffusion
	short int RefOut[(SNDMIX_REVERB_DELAY_MASK+1)*2]; // stereo output of reflections
} SWRVBREFDELAY, *PSWRVBREFDELAY;


// Late reverberation
// Tank diffusers lengths
#define RVBDIF1L_LEN		(149*2)	// 6.8ms
#define RVBDIF1R_LEN		(223*2)	// 10.1ms
#define RVBDIF2L_LEN		(421*2)	// 19.1ms
#define RVBDIF2R_LEN		(647*2)	// 29.3ms
// Tank delay lines lengths
#define RVBDLY1L_LEN		(683*2)	// 30.9ms
#define RVBDLY1R_LEN	    (811*2) // 36.7ms
#define RVBDLY2L_LEN		(773*2)	// 35.1ms
#define RVBDLY2R_LEN	    (1013*2) // 45.9ms
// Tank delay lines mask
#define RVBDLY_MASK			2047

// Min/Max reflections delay
#define RVBMINREFDELAY		96		// 96 samples
#define RVBMAXREFDELAY		7500	// 7500 samples
// Min/Max reverb delay
#define RVBMINRVBDELAY		128		// 256 samples (11.6ms @ 22kHz)
#define RVBMAXRVBDELAY		3800	// 1900 samples (86ms @ 24kHz)

typedef struct _SWLATEREVERB
{
	ULONG nReverbDelay;			// Reverb delay (in samples)
	ULONG nDelayPos;			// Delay line position
	SHORT nDifCoeffs[4];		// Reverb diffusion
	SHORT nDecayDC[4];			// Reverb DC decay
	SHORT nDecayLP[4];			// Reverb HF decay
	SHORT LPHistory[4];			// Low-pass history
	SHORT Dif2InGains[4];		// 2nd diffuser input gains
	SHORT RvbOutGains[4];		// 4x2 Reverb output gains
	LONG lMasterGain;			// late reverb master gain
	LONG lDummyAlign;
	// Tank Delay lines
	short int Diffusion1[(RVBDLY_MASK+1)*2];	// {dif1_l, dif1_r}
	short int Diffusion2[(RVBDLY_MASK+1)*2];	// {dif2_l, dif2_r}
	short int Delay1[(RVBDLY_MASK+1)*2];		// {dly1_l, dly1_r}
	short int Delay2[(RVBDLY_MASK+1)*2];		// {dly2_l, dly2_r}
} SWLATEREVERB, *PSWLATEREVERB;

#define ENVIRONMENT_NUMREFLECTIONS		8

typedef struct _ENVIRONMENTREFLECTION
{
	SHORT GainLL, GainRR, GainLR, GainRL;	// +/- 32K scale
	ULONG Delay;							// In samples
} ENVIRONMENTREFLECTION, *PENVIRONMENTREFLECTION;

typedef struct _ENVIRONMENTREVERB
{
	LONG ReverbLevel;		// Late reverb gain (mB)
	LONG ReflectionsLevel;	// Master reflections gain (mB)
	LONG RoomHF;			// Room gain HF (mB)
	ULONG ReverbDecay;		// Reverb tank decay (0-7fff scale)
	LONG PreDiffusion;		// Reverb pre-diffusion amount (+/- 32K scale)
	LONG TankDiffusion;		// Reverb tank diffusion (+/- 32K scale)
	ULONG ReverbDelay;		// Reverb delay (in samples)
	FLOAT flReverbDamping;	// HF tank gain [0.0, 1.0]
	LONG ReverbDecaySamples;// Reverb decay time (in samples)
	ENVIRONMENTREFLECTION Reflections[ENVIRONMENT_NUMREFLECTIONS];
} ENVIRONMENTREVERB, *PENVIRONMENTREVERB;


// Pre/Post resampling and filtering
UINT X86_ReverbProcessPreFiltering1x(int *pWet, UINT nSamples);
UINT X86_ReverbProcessPreFiltering2x(int *pWet, UINT nSamples);
VOID MMX_ReverbProcessPostFiltering1x(const int *pRvb, int *pDry, UINT nSamples);
VOID X86_ReverbProcessPostFiltering2x(const int *pRvb, int *pDry, UINT nSamples);
VOID MMX_ReverbDCRemoval(int *pBuffer, UINT nSamples);
VOID X86_ReverbDryMix(int *pDry, int *pWet, int lDryVol, UINT nSamples);
// Process pre-diffusion and pre-delay
VOID MMX_ProcessPreDelay(PSWRVBREFDELAY pPreDelay, const int *pIn, UINT nSamples);
// Process reflections
VOID MMX_ProcessReflections(PSWRVBREFDELAY pPreDelay, short int *pRefOut, int *pMixOut, UINT nSamples);
// Process Late Reverb (SW Reflections): stereo reflections output, 32-bit reverb output, SW reverb gain
VOID MMX_ProcessLateReverb(PSWLATEREVERB pReverb, short int *pRefOut, int *pMixOut, UINT nSamples);

/////////////////////////////////////////////////////////////////////////////////
//
// I3DL2 reverb presets
//

#define SNDMIX_REVERB_PRESET_DEFAULT \
-10000,    0, 1.00f,0.50f,-10000,0.020f,-10000,0.040f,100.0f,100.0f

#define SNDMIX_REVERB_PRESET_GENERIC \
 -1000, -100, 1.49f,0.83f, -2602,0.007f,   200,0.011f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_PADDEDCELL \
 -1000,-6000, 0.17f,0.10f, -1204,0.001f,   207,0.002f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_ROOM \
 -1000, -454, 0.40f,0.83f, -1646,0.002f,    53,0.003f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_BATHROOM \
 -1000,-1200, 1.49f,0.54f,  -370,0.007f,  1030,0.011f,100.0f, 60.0f
#define SNDMIX_REVERB_PRESET_LIVINGROOM \
 -1000,-6000, 0.50f,0.10f, -1376,0.003f, -1104,0.004f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_STONEROOM \
 -1000, -300, 2.31f,0.64f,  -711,0.012f,    83,0.017f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_AUDITORIUM \
 -1000, -476, 4.32f,0.59f,  -789,0.020f,  -289,0.030f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_CONCERTHALL \
 -1000, -500, 3.92f,0.70f, -1230,0.020f,    -2,0.029f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_CAVE \
 -1000,    0, 2.91f,1.30f,  -602,0.015f,  -302,0.022f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_ARENA \
 -1000, -698, 7.24f,0.33f, -1166,0.020f,    16,0.030f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_HANGAR \
 -1000,-1000,10.05f,0.23f,  -602,0.020f,   198,0.030f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_CARPETEDHALLWAY \
 -1000,-4000, 0.30f,0.10f, -1831,0.002f, -1630,0.030f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_HALLWAY \
 -1000, -300, 1.49f,0.59f, -1219,0.007f,   441,0.011f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_STONECORRIDOR \
 -1000, -237, 2.70f,0.79f, -1214,0.013f,   395,0.020f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_ALLEY \
 -1000, -270, 1.49f,0.86f, -1204,0.007f,    -4,0.011f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_FOREST \
 -1000,-3300, 1.49f,0.54f, -2560,0.162f,  -613,0.088f, 79.0f,100.0f
#define SNDMIX_REVERB_PRESET_CITY \
 -1000, -800, 1.49f,0.67f, -2273,0.007f, -2217,0.011f, 50.0f,100.0f
#define SNDMIX_REVERB_PRESET_MOUNTAINS \
 -1000,-2500, 1.49f,0.21f, -2780,0.300f, -2014,0.100f, 27.0f,100.0f
#define SNDMIX_REVERB_PRESET_QUARRY \
 -1000,-1000, 1.49f,0.83f,-10000,0.061f,   500,0.025f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_PLAIN \
 -1000,-2000, 1.49f,0.50f, -2466,0.179f, -2514,0.100f, 21.0f,100.0f
#define SNDMIX_REVERB_PRESET_PARKINGLOT \
 -1000,    0, 1.65f,1.50f, -1363,0.008f, -1153,0.012f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_SEWERPIPE \
 -1000,-1000, 2.81f,0.14f,   429,0.014f,   648,0.021f, 80.0f, 60.0f
#define SNDMIX_REVERB_PRESET_UNDERWATER \
 -1000,-4000, 1.49f,0.10f,  -449,0.007f,  1700,0.011f,100.0f,100.0f

// Examples simulating General MIDI 2'musical' reverb presets
//
// Name  (Decay time)  Description
//
// Small Room  (1.1s)  A small size room with a length of 5m or so.
// Medium Room (1.3s)  A medium size room with a length of 10m or so.
// Large Room  (1.5s)  A large size room suitable for live performances.
// Medium Hall (1.8s)  A medium size concert hall.
// Large Hall  (1.8s)  A large size concert hall suitable for a full orchestra.
// Plate       (1.3s)  A plate reverb simulation.

#define SNDMIX_REVERB_PRESET_SMALLROOM \
 -1000, -600, 1.10f,0.83f,  -400,0.005f,   500,0.010f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_MEDIUMROOM \
 -1000, -600, 1.30f,0.83f, -1000,0.010f,  -200,0.020f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_LARGEROOM \
 -1000, -600, 1.50f,0.83f, -1600,0.020f, -1000,0.040f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_MEDIUMHALL \
 -1000, -600, 1.80f,0.70f, -1300,0.015f,  -800,0.030f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_LARGEHALL \
 -1000, -600, 1.80f,0.70f, -2000,0.030f, -1400,0.060f,100.0f,100.0f
#define SNDMIX_REVERB_PRESET_PLATE \
 -1000, -200, 1.30f,0.90f,     0,0.002f,     0,0.010f,100.0f, 75.0f


#endif // _SNDMIX_REVERB_H_

