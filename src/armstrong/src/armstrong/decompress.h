#pragma once

//=====================================DEFINITIONS================================
#define MAXPACKEDBUFFER 2048

//=====================================STRUCTURES================================
#include "compresstypes.h"

typedef struct _COMPRESSIONVALUES
{
	WORD	wSum1;
	WORD	wSum2;
	WORD	wResult;

	LPWORD	lpwTempData;
}COMPRESSIONVALUES;

typedef struct _WAVEUNPACK
{
	zzub_output_t* pStreamOut;
	zzub_input_t* pStreamIn;
	BYTE abtPackedBuffer[MAXPACKEDBUFFER];
	DWORD dwCurIndex;
	DWORD dwCurBit;

	DWORD dwBytesInBuffer;
	DWORD dwMaxBytes;
	DWORD dwBytesInFileRemain;

}WAVEUNPACK;


BOOL InitWaveUnpack(WAVEUNPACK* waveunpackinfo, zzub_input_t* pInStrm, DWORD dwSectionSize);
BOOL DecompressWave(WAVEUNPACK* unpackinfo, LPWORD lpwOutputBuffer, DWORD dwNumSamples, BOOL bStereo);

