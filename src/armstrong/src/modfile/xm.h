#pragma once

namespace modimport {
namespace xm {

// Bit 0-1: 0 = No loop, 1 = Forward loop, 2 = Ping-pong loop; 4: 16-bit sampledata

const unsigned int sample_flag_loop_type_mask = 3;
const unsigned int sample_flag_16bit = 16;

const unsigned int sample_loop_none = 0;
const unsigned int sample_loop_forward = 1;
const unsigned int sample_loop_bidir = 2;

#pragma pack (push, 1)

typedef struct {
	char idText[17];
	char songName[20];
	char magic1a;
	char trackerName[20];
	short version;	// Version number, hi-byte major and low-byte minor
	int headerSize;
	short songLen;
	short restartPos;
	short numChans;
	short numPats;	// Number of highest pattern in file, _NOT_ 'Number of patterns in file'. That is, if there are 5 patterns numbered 0 thru 4, this location should contain the value 4. 
	short numInstr;
	short flags; // Flags: bit 0: 0 = Amiga frequency table, 1 = Linear frequency table
	short tempo;
	short bpm;
	char orders[256];
} _XMHEADER;

typedef struct {
	int headerSize;
	char packType;
	short rows;
	short patternSize;
} _XMPATTERN;

typedef struct {
	//int instrumentSize;	this is read separately, and if 0 skipped
	char name[22];
	char type;
	short numSamples;
} _XMINSTRUMENT;

// If the number of samples > 0, then the this will follow:


typedef struct {
    unsigned short x;
    unsigned short y;
} _XMENVPOINT;

typedef struct {
	//int headerSize;
	char noteSample[96];
	_XMENVPOINT volEnvelopePoints[12];
	_XMENVPOINT panEnvelopePoints[12];
	char numVolPoints, numPanPoints, 
		volsuspt, volloopstartpt, volloopendpt,
		pansuspt, panloopstartpt, panloopendpt,
		voltype, pantype, vibtype, vibsweep, vibdepth, vibrate;
	short volfade;
	short reserved[11];
} _XMSAMPLES;

typedef struct {
	int len, loopstart, looplen;
	unsigned char vol;
	char finetune;	// (signed byte -128..+127)
	unsigned char flags;		// Bit 0-1: 0 = No loop, 1 = Forward loop, 2 = Ping-pong loop; 4: 16-bit sampledata
	char pan, relnote, reserved;
	char samplename[22];
} _XMSAMPLE;

typedef struct  {
	// same as xm, maybe some _TRITONHEADER =)
	char idText[21];
	char songName[22];
	char magic1a;
	char trackerName[20];
	short version;	// Version number, hi-byte major and low-byte minor
} _XIHEADER;

#pragma pack (pop)

}
}
