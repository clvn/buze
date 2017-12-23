#pragma once

namespace modimport {
namespace it {

#pragma pack (push, 1)

const unsigned int it_flag_stereo = 1;
const unsigned int it_flag_mixopts = 2;
const unsigned int it_flag_use_instruments = 4;
const unsigned int it_flag_linear_slides = 8;
const unsigned int it_flag_old_effects = 16;
const unsigned int it_flag_link_fx_g_with_ef = 32;
const unsigned int it_flag_midi_pitch_controller = 64;
const unsigned int it_flag_req_midi = 128;

const unsigned int sample_flag_header_assoc = 1;
const unsigned int sample_flag_16bit = 2;
const unsigned int sample_flag_stereo = 4;
const unsigned int sample_flag_compressed = 8;
const unsigned int sample_flag_loop = 16;
const unsigned int sample_flag_sustain_loop = 32;
const unsigned int sample_flag_bidir_loop = 64;
const unsigned int sample_flag_bidir_sustain_loop = 128;

const unsigned int sample_convert_signed = 1;
const unsigned int sample_convert_delta = 4;

typedef struct {
	char magic[4];
	char songName[26];
	short PHiligt;
	short OrdNum;
	short InsNum;
	short SmpNum;
	short PatNum;
	unsigned short Cwtv;
	unsigned short Cmwt;
	unsigned short Flags;
	short Special;
	char GV, MV, IS, IT, Sep, PWD;
	short MsgLength;
	int MessageOffset;
	int Reserved;
	char ChnPan[64];
	char ChnVol[64];
} _ITHEADER;

typedef struct {
	char magic[4];
	char fileName[12];
	char nul;
	unsigned char Flags;
	char VLS;
	char VLE;
	char SLS;
	char SLE;
	short XX;
	short FadeOut;
	char NNA;
	char DNC;
	char TrackVersion;
	char NoS;
	char X;
	char instrumentName[26];
	char XXXXXX[6];
	unsigned char NodeSampleKeyboardTable[240];
	unsigned char VolumeEnvelope[200];
	unsigned short NodePoints[25];
} _ITINSTRUMENT_OLD;

typedef struct {
	char magic[4];
	char fileName[12];
	char nul;
	char Flags;
	char VLS;
	char VLE;
	char SLS;
	char SLE;
	short XX;
	short FadeOut;
	char NNA;
	char DNC;
	short TrackVersion;
	char NoS;
	char X;
	char instrumentName[26];
	char IFC;
	char IFR;
	char MCh;
	char MPr;
	short MIDIBnk;
	char NodeSampleKeyboardTable[240];
} _ITINSTRUMENT_NEW;

typedef struct {
	char magic[4];
	char fileName[12];
	char nul;
	char GvL;
	unsigned char Flags;
	unsigned char Vol;
	char sampleName[26];
	char Cvt;
	char DfP;
	unsigned int Length;
	unsigned int LoopBegin;
	unsigned int LoopEnd;
	unsigned int C5Speed;
	unsigned int SusLoopBegin;
	unsigned int SusLoopEnd;
	unsigned int SamplePointer;
	char ViS;
	char ViD;
	char ViR;
	char ViT;
} _ITSAMPLE;

#pragma pack (pop)

}

}
