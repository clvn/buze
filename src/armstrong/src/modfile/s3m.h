#pragma once

namespace modimport {
namespace s3m {

#pragma pack (push, 1)

typedef struct {
	char songName[28];
	char magic1a;
	char type;
	short pad;
	unsigned short ordNum, insNum, patNum, flags, cwtv, ffi;
	char scrm[4];
	char gv, is, it, mv, uc, dp;
	char pad2[8];
	short special;
	unsigned char channelSettings[32];
} _S3MHEADER;

typedef struct {
	char t;
	char fileName[12];
	unsigned char segl;
	unsigned short seg;
	unsigned int len;
	unsigned int loopbeg;
	unsigned int loopend;
	short vol;
	char pack;
	char flags; // +1=loop on, 
				// +2=stereo (after Length bytes for LEFT channel, another Length bytes for RIGHT channel)
				// +4=16-bit sample (intel LO-HI byteorder)
	unsigned int c2spd, pad;
	short intGp, int512, lastused1, lastused2;
	char samplename[28];
	char srcs[4];
} _S3MSAMPLE;
#pragma pack (pop)

}
}
