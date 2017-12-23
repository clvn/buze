#pragma once

namespace modimport {
namespace mod {

#pragma pack (push, 1)

typedef struct {
	char samplename[22];
	unsigned short length;
	char finetune;
	char volume;
	unsigned short loopstart, loopend;
} _MODSAMPLE;

typedef struct {
	char songname[20];
	_MODSAMPLE sample[31];
	char songlen;
	char restart;
	char order[128];
	char desc[4];
} _MODHEADER;

typedef struct {
	char b1;
	char b2;
	char b3;
	char b4;
} _MODROW;

typedef struct {
	_MODROW *row[32];
} _MODCHANNEL;

typedef struct {
	_MODCHANNEL *channel[64];
} _MODPATTERN;


static short _MODPERIODS[] = {
    //C    C#   D    D#   E    F    F#   G    G#   A    A#   B
	//Octave 0:
	1712,1616,1525,1440,1357,1281,1209,1141,1077,1017, 961, 907,
	//Octave 1: 
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	//Octave 2: 
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	//Octave 3: 
	214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113,
	//Octave 4: 
	107, 101,  95,  90,  85,  80,  76,  71,  67,  64,  60,  57
};



#pragma pack (pop)

}
}
