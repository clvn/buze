/*
 * OpenMPT
 *
 * Sndfile.cpp
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
 *          OpenMPT devs
*/

#include "stdafx.h"
#include "mainfrm.h"
#include "sndfile.h"
#include "tuningcollection.h"
#include <vector>
#include <algorithm>

#define str_SampleAllocationError	(GetStrI18N(_TEXT("Sample allocation error")))
#define str_Error					(GetStrI18N(_TEXT("Error")))

#ifndef NO_COPYRIGHT
#ifndef NO_MMCMP_SUPPORT
#define MMCMP_SUPPORT
#endif // NO_MMCMP_SUPPORT
#ifndef NO_ARCHIVE_SUPPORT
#define UNRAR_SUPPORT
#define UNLHA_SUPPORT
#define ZIPPED_MOD_SUPPORT
LPCSTR glpszModExtensions = "mod|s3m|xm|it|stm|nst|ult|669|wow|mtm|med|far|mdl|ams|dsm|amf|okt|dmf|ptm|psm|mt2|umx|gdm|imf|j2b"
#ifndef NO_UNMO3_SUPPORT
"|mo3"
#endif
;
//Should there be mptm?
#endif // NO_ARCHIVE_SUPPORT
#else // NO_COPYRIGHT: EarSaver only loads mod/s3m/xm/it/wav
#define MODPLUG_BASIC_SUPPORT
#endif
/*
#ifdef ZIPPED_MOD_SUPPORT
#include "unzip32.h"
#endif

#ifdef UNRAR_SUPPORT
#include "unrar32.h"
#endif

#ifdef UNLHA_SUPPORT
#include "../unlha/unlha32.h"
#endif

#ifdef MMCMP_SUPPORT
extern BOOL MMCMP_Unpack(LPCBYTE *ppMemFile, LPDWORD pdwMemLength);
#endif

// External decompressors
extern void AMSUnpack(const char *psrc, UINT inputlen, char *pdest, UINT dmax, char packcharacter);
extern WORD MDLReadBits(DWORD &bitbuf, UINT &bitnum, LPBYTE &ibuf, CHAR n);
extern int DMFUnpack(LPBYTE psample, LPBYTE ibuf, LPBYTE ibufmax, UINT maxlen);
extern DWORD ITReadBits(DWORD &bitbuf, UINT &bitnum, LPBYTE &ibuf, CHAR n);
extern void ITUnpack8Bit(LPSTR pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, BOOL b215);
extern void ITUnpack16Bit(LPSTR pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, BOOL b215);
*/

// Like Limit, but returns value
#ifndef CLAMP
#define CLAMP(number, low, high) min(high, max(low, number))
#endif

#define MAX_PACK_TABLES		3

CRITICAL_SECTION CMainFrame::m_csAudio;
BYTE CMainFrame::gbWFIRType;
double CMainFrame::gdWFIRCutoff;
long CMainFrame::glVolumeRampSamples;
const LPCTSTR szDefaultNoteNames[NOTE_MAX] = {
	TEXT("C-0"), TEXT("C#0"), TEXT("D-0"), TEXT("D#0"), TEXT("E-0"), TEXT("F-0"), TEXT("F#0"), TEXT("G-0"), TEXT("G#0"), TEXT("A-0"), TEXT("A#0"), TEXT("B-0"),
	TEXT("C-1"), TEXT("C#1"), TEXT("D-1"), TEXT("D#1"), TEXT("E-1"), TEXT("F-1"), TEXT("F#1"), TEXT("G-1"), TEXT("G#1"), TEXT("A-1"), TEXT("A#1"), TEXT("B-1"),
	TEXT("C-2"), TEXT("C#2"), TEXT("D-2"), TEXT("D#2"), TEXT("E-2"), TEXT("F-2"), TEXT("F#2"), TEXT("G-2"), TEXT("G#2"), TEXT("A-2"), TEXT("A#2"), TEXT("B-2"),
	TEXT("C-3"), TEXT("C#3"), TEXT("D-3"), TEXT("D#3"), TEXT("E-3"), TEXT("F-3"), TEXT("F#3"), TEXT("G-3"), TEXT("G#3"), TEXT("A-3"), TEXT("A#3"), TEXT("B-3"),
	TEXT("C-4"), TEXT("C#4"), TEXT("D-4"), TEXT("D#4"), TEXT("E-4"), TEXT("F-4"), TEXT("F#4"), TEXT("G-4"), TEXT("G#4"), TEXT("A-4"), TEXT("A#4"), TEXT("B-4"),
	TEXT("C-5"), TEXT("C#5"), TEXT("D-5"), TEXT("D#5"), TEXT("E-5"), TEXT("F-5"), TEXT("F#5"), TEXT("G-5"), TEXT("G#5"), TEXT("A-5"), TEXT("A#5"), TEXT("B-5"),
	TEXT("C-6"), TEXT("C#6"), TEXT("D-6"), TEXT("D#6"), TEXT("E-6"), TEXT("F-6"), TEXT("F#6"), TEXT("G-6"), TEXT("G#6"), TEXT("A-6"), TEXT("A#6"), TEXT("B-6"),
	TEXT("C-7"), TEXT("C#7"), TEXT("D-7"), TEXT("D#7"), TEXT("E-7"), TEXT("F-7"), TEXT("F#7"), TEXT("G-7"), TEXT("G#7"), TEXT("A-7"), TEXT("A#7"), TEXT("B-7"),
	TEXT("C-8"), TEXT("C#8"), TEXT("D-8"), TEXT("D#8"), TEXT("E-8"), TEXT("F-8"), TEXT("F#8"), TEXT("G-8"), TEXT("G#8"), TEXT("A-8"), TEXT("A#8"), TEXT("B-8"),
	TEXT("C-9"), TEXT("C#9"), TEXT("D-9"), TEXT("D#9"), TEXT("E-9"), TEXT("F-9"), TEXT("F#9"), TEXT("G-9"), TEXT("G#9"), TEXT("A-9"), TEXT("A#9"), TEXT("B-9"),
};

CTuning* MODINSTRUMENT::s_DefaultTuning = 0;

//////////////////////////////////////////////////////////
// CSoundFile

CTuningCollection* CSoundFile::s_pTuningsSharedBuiltIn(0);
CTuningCollection* CSoundFile::s_pTuningsSharedLocal(0);
uint8 CSoundFile::s_DefaultPlugVolumeHandling = PLUGIN_VOLUMEHANDLING_IGNORE;


CSoundFile::CSoundFile(CWaveTable* _wavetable) :
	wavetable(_wavetable),
	m_pModSpecs(&ModSpecs::itEx)
//----------------------
{
	m_nType = MOD_TYPE_NONE;
	m_dwSongFlags = 0;
	m_nChannels = 0;
	m_nMixChannels = 0;
	m_nSamples = 0;
	m_nInstruments = 0;
	m_lpszPatternNames = NULL;
	m_lpszSongComments = NULL;
	m_nFreqFactor = m_nTempoFactor = 128;
	m_nMasterVolume = 128;
	m_nMinPeriod = MIN_PERIOD;
	m_nMaxPeriod = 0x7FFF;
	m_nRepeatCount = 0;
	m_nRowsPerBeat = 4;
	m_nRowsPerMeasure = 16;
	m_nTempoMode = tempo_mode_classic;
	m_bIsRendering = false;
	m_nMaxSample = 0;

	m_ModFlags = 0;

	m_dwLastSavedWithVersion=0;
	m_dwCreatedWithVersion=0;
	memset(m_bChannelMuteTogglePending, 0, sizeof(m_bChannelMuteTogglePending));


// -> CODE#0023
// -> DESC="IT project files (.itp)"
	for(UINT i = 0 ; i < MAX_INSTRUMENTS ; i++){
		m_szInstrumentPath[i][0] = '\0';
		instrumentModified[i] = FALSE;
	}
// -! NEW_FEATURE#0023

	memset(Chn, 0, sizeof(Chn));
	memset(ChnMix, 0, sizeof(ChnMix));
	memset(ChnSettings, 0, sizeof(ChnSettings));
	memset(m_szNames, 0, sizeof(m_szNames));
	memset(&m_SongEQ, 0, sizeof(m_SongEQ));
	m_lTotalSampleCount=0;

	m_pConfig = new CSoundFilePlayConfig();
	m_pTuningsTuneSpecific = new CTuningCollection("Tune specific tunings");
	
//	BuildDefaultInstrument();
}


CSoundFile::~CSoundFile()
//-----------------------
{
	delete m_pConfig;
	delete m_pTuningsTuneSpecific;
	Destroy();
}


BOOL CSoundFile::Create(LPCBYTE lpStream, DWORD dwMemLength)
//---------------------------------------------------------------------------
{
	m_nType = MOD_TYPE_NONE;
	m_dwSongFlags = 0;
	m_nChannels = 0;
	m_nMixChannels = 0;
	m_nSamples = 0;
	m_nInstruments = 0;
	m_nFreqFactor = m_nTempoFactor = 128;
	m_nMasterVolume = 128;
	m_nDefaultGlobalVolume = 256;
	m_nGlobalVolume = 256;
	m_nOldGlbVolSlide = 0;
	m_nDefaultSpeed = 6;
	m_nDefaultTempo = 125;
	m_nPatternDelay = 0;
	m_nFrameDelay = 0;
	m_nMinPeriod = 16;
	m_nMaxPeriod = 32767;
	m_nSamplePreAmp = 48;
	m_nVSTiVolume = 48;
	m_lpszPatternNames = NULL;
	m_lpszSongComments = NULL;
	//m_nMixLevels = mixLevels_original;	// Will be overridden if appropriate.
	m_nMixLevels = mixLevels_117RC3;	// override
	memset(ChnMix, 0, sizeof(ChnMix));
	memset(Chn, 0, sizeof(Chn));
	memset(m_szNames, 0, sizeof(m_szNames));
	memset(&m_SongEQ, 0, sizeof(m_SongEQ));
	ResetMidiCfg();
	for (CHANNELINDEX nChn = 0; nChn < MAX_BASECHANNELS; nChn++)
	{
		InitChannel(nChn);
	}
	// New song
	m_dwCreatedWithVersion = 1;//MptVersion::num;

	// Adjust song names
	for (UINT iSmp=0; iSmp<MAX_SAMPLES; iSmp++)
	{
		LPSTR p = m_szNames[iSmp];
		int j = 31;
		p[j] = 0;
		while ((j>=0) && (p[j]<=' ')) p[j--] = 0;
		while (j>=0)
		{
			if (((BYTE)p[j]) < ' ') p[j] = ' ';
			j--;
		}
	}
	// Adjust channels
	for (UINT ich=0; ich<MAX_BASECHANNELS; ich++)
	{
		if (ChnSettings[ich].nVolume > 64) ChnSettings[ich].nVolume = 64;
		if (ChnSettings[ich].nPan > 256) ChnSettings[ich].nPan = 128;
		Chn[ich].nPan = ChnSettings[ich].nPan;
		Chn[ich].nGlobalVol = ChnSettings[ich].nVolume;
		Chn[ich].dwFlags = ChnSettings[ich].dwFlags;
		Chn[ich].nVolume = 256;
		Chn[ich].nCutOff = 0x7F;
		Chn[ich].nEFxSpeed = 0;
		//IT compatibility 15. Retrigger
		if(IsCompatibleMode(TRK_IMPULSETRACKER))
		{
			Chn[ich].nRetrigParam = Chn[ich].nRetrigCount = 1;
		}
	}
	// Checking instruments
	for (UINT iIns=0; iIns<MAX_INSTRUMENTS; iIns++)
	{
		MODSAMPLE *pSmp = GetSample(iIns);
		if (pSmp->pSample)
		{
			if (pSmp->nLoopEnd > pSmp->nLength) pSmp->nLoopEnd = pSmp->nLength;
			if (pSmp->nLoopStart >= pSmp->nLoopEnd)
			{
				pSmp->nLoopStart = 0;
				pSmp->nLoopEnd = 0;
			}
			if (pSmp->nSustainEnd > pSmp->nLength) pSmp->nSustainEnd = pSmp->nLength;
			if (pSmp->nSustainStart >= pSmp->nSustainEnd)
			{
				pSmp->nSustainStart = 0;
				pSmp->nSustainEnd = 0;
			}
		} else
		{
			pSmp->nLength = 0;
			pSmp->nLoopStart = 0;
			pSmp->nLoopEnd = 0;
			pSmp->nSustainStart = 0;
			pSmp->nSustainEnd = 0;
		}
		if (!pSmp->nLoopEnd) pSmp->uFlags &= ~CHN_LOOP;
		if (!pSmp->nSustainEnd) pSmp->uFlags &= ~CHN_SUSTAINLOOP;
		if (pSmp->nGlobalVol > 64) pSmp->nGlobalVol = 64;
	}
	// Check invalid instruments
	while ((m_nInstruments > 0) && (!GetInstrument(m_nInstruments))) m_nInstruments--;
	// Set default values
	if (m_nDefaultTempo < 32) m_nDefaultTempo = 125;
	if (!m_nDefaultSpeed) m_nDefaultSpeed = 6;
	m_nMusicSpeed = m_nDefaultSpeed;
	m_nMusicTempo = m_nDefaultTempo;
	m_nGlobalVolume = m_nDefaultGlobalVolume;
	m_lHighResRampingGlobalVolume = m_nGlobalVolume<<VOLUMERAMPPRECISION;
	m_nGlobalVolumeDestination = m_nGlobalVolume;
	m_nSamplesToGlobalVolRampDest=0;
	m_nBufferCount = 0;
	m_dBufferDiff = 0;
	m_nTickCount = m_nMusicSpeed;

	switch(m_nTempoMode) {
		case tempo_mode_alternative: 
			m_nSamplesPerTick = gdwMixingFreq / m_nMusicTempo; break;
		case tempo_mode_modern: 
			m_nSamplesPerTick = gdwMixingFreq * (60/m_nMusicTempo / (m_nMusicSpeed * m_nRowsPerBeat)); break;
		case tempo_mode_classic: default:
			m_nSamplesPerTick = (gdwMixingFreq * 5 * m_nTempoFactor) / (m_nMusicTempo << 8);
	}


	// Set up mix levels
	m_pConfig->SetMixLevels(m_nMixLevels);

	if (m_nType)
	{
		SetModSpecsPointer(m_pModSpecs, m_nType);
		return TRUE;
	}

	return FALSE;
}


BOOL CSoundFile::Destroy()
//------------------------
{
	size_t i;

	delete[] m_lpszSongComments;
	m_lpszSongComments = NULL;

	m_nType = MOD_TYPE_NONE;
	m_nChannels = m_nSamples = m_nInstruments = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// Misc functions

void CSoundFile::ResetMidiCfg()
//-----------------------------
{
	memset(&m_MidiCfg, 0, sizeof(m_MidiCfg));
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_START*32], "FF");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_STOP*32], "FC");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_NOTEON*32], "9c n v");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_NOTEOFF*32], "9c n 0");
	lstrcpy(&m_MidiCfg.szMidiGlb[MIDIOUT_PROGRAM*32], "Cc p");
	lstrcpy(&m_MidiCfg.szMidiSFXExt[0], "F0F000z");
	for (int iz=0; iz<16; iz++) wsprintf(&m_MidiCfg.szMidiZXXExt[iz*32], "F0F001%02X", iz*8);
}


UINT CSoundFile::GetSongComments(LPSTR s, UINT len, UINT linesize)
//----------------------------------------------------------------
{
	LPCSTR p = m_lpszSongComments;
	if (!p) return 0;
	UINT i = 2, ln=0;
	if ((len) && (s)) s[0] = '\x0D';
	if ((len > 1) && (s)) s[1] = '\x0A';
	while ((*p)	&& (i+2 < len))
	{
		BYTE c = (BYTE)*p++;
		if ((c == 0x0D) || ((c == ' ') && (ln >= linesize)))
			{ if (s) { s[i++] = '\x0D'; s[i++] = '\x0A'; } else i+= 2; ln=0; }
		else
		if (c >= 0x20) { if (s) s[i++] = c; else i++; ln++; }
	}
	if (s) s[i] = 0;
	return i;
}


UINT CSoundFile::GetRawSongComments(LPSTR s, UINT len, UINT linesize)
//-------------------------------------------------------------------
{
	LPCSTR p = m_lpszSongComments;
	if (!p) return 0;
	UINT i = 0, ln=0;
	while ((*p)	&& (i < len-1))
	{
		BYTE c = (BYTE)*p++;
		if ((c == 0x0D)	|| (c == 0x0A))
		{
			if (ln) 
			{
				while (ln < linesize) { if (s) s[i] = ' '; i++; ln++; }
				ln = 0;
			}
		} else
		if ((c == ' ') && (!ln))
		{
			UINT k=0;
			while ((p[k]) && (p[k] >= ' '))	k++;
			if (k <= linesize)
			{
				if (s) s[i] = ' ';
				i++;
				ln++;
			}
		} else
		{
			if (s) s[i] = c;
			i++;
			ln++;
			if (ln == linesize) ln = 0;
		}
	}
	if (ln)
	{
		while ((ln < linesize) && (i < len))
		{
			if (s) s[i] = ' ';
			i++;
			ln++;
		}
	}
	if (s) s[i] = 0;
	return i;
}


BOOL CSoundFile::SetWaveConfig(UINT nRate,UINT nBits,UINT nChannels,BOOL bMMX)
//----------------------------------------------------------------------------
{
	BOOL bReset = FALSE;
	DWORD d = gdwSoundSetup & ~SNDMIX_ENABLEMMX;
	if (bMMX) d |= SNDMIX_ENABLEMMX;
	if ((gdwMixingFreq != nRate) || (gnBitsPerSample != nBits) || (gnChannels != nChannels) || (d != gdwSoundSetup)) bReset = TRUE;
	gnChannels = nChannels;
	gdwSoundSetup = d;
	gdwMixingFreq = nRate;
	gnBitsPerSample = nBits;
	InitPlayer(bReset);
	return TRUE;
}


BOOL CSoundFile::SetDspEffects(BOOL bSurround,BOOL bReverb,BOOL bMegaBass,BOOL bNR,BOOL bEQ)
//------------------------------------------------------------------------------------------
{
	DWORD d = gdwSoundSetup & ~(SNDMIX_SURROUND | SNDMIX_REVERB | SNDMIX_MEGABASS | SNDMIX_NOISEREDUCTION | SNDMIX_EQ);
	if (bSurround) d |= SNDMIX_SURROUND;
	if ((bReverb) && (gdwSysInfo & SYSMIX_ENABLEMMX)) d |= SNDMIX_REVERB;
	if (bMegaBass) d |= SNDMIX_MEGABASS;
	if (bNR) d |= SNDMIX_NOISEREDUCTION;
	if (bEQ) d |= SNDMIX_EQ;
	gdwSoundSetup = d;
	InitPlayer(FALSE);
	return TRUE;
}


BOOL CSoundFile::SetResamplingMode(UINT nMode)
//--------------------------------------------
{
	DWORD d = gdwSoundSetup & ~(SNDMIX_NORESAMPLING|SNDMIX_SPLINESRCMODE|SNDMIX_POLYPHASESRCMODE|SNDMIX_FIRFILTERSRCMODE);
	switch(nMode)
	{
	case SRCMODE_NEAREST:	d |= SNDMIX_NORESAMPLING; break;
	case SRCMODE_LINEAR:	break; // default
	//rewbs.resamplerConf
	//case SRCMODE_SPLINE:	d |= SNDMIX_HQRESAMPLER; break;
	//case SRCMODE_POLYPHASE:	d |= (SNDMIX_HQRESAMPLER|SNDMIX_ULTRAHQSRCMODE); break;
	case SRCMODE_SPLINE:	d |= SNDMIX_SPLINESRCMODE; break;
	case SRCMODE_POLYPHASE:	d |= SNDMIX_POLYPHASESRCMODE; break;
	case SRCMODE_FIRFILTER:	d |= SNDMIX_FIRFILTERSRCMODE; break;
	default: return FALSE;
	//end rewbs.resamplerConf
	}
	gdwSoundSetup = d;
	return TRUE;
}


void CSoundFile::SetMasterVolume(UINT nVol, bool adjustAGC)
//---------------------------------------------------------
{
	if (nVol < 1) nVol = 1;
	if (nVol > 0x200) nVol = 0x200;	// x4 maximum
	if ((nVol < m_nMasterVolume) && (nVol) && (gdwSoundSetup & SNDMIX_AGC) && (adjustAGC))
	{
		gnAGC = gnAGC * m_nMasterVolume / nVol;
		if (gnAGC > AGC_UNITY) gnAGC = AGC_UNITY;
	}
	m_nMasterVolume = nVol;
}


void CSoundFile::SetAGC(BOOL b)
//-----------------------------
{
	if (b)
	{
		if (!(gdwSoundSetup & SNDMIX_AGC))
		{
			gdwSoundSetup |= SNDMIX_AGC;
			gnAGC = AGC_UNITY;
		}
	} else gdwSoundSetup &= ~SNDMIX_AGC;
}

double  CSoundFile::GetCurrentBPM() const
//---------------------------------------
{
	double bpm;

	if (m_nTempoMode == tempo_mode_modern) {		// With modern mode, we trust that true bpm 
		bpm = static_cast<double>(m_nMusicTempo);	// is  be close enough to what user chose.
	}												// This avoids oscillation due to tick-to-tick corrections.

	else {												//with other modes, we calculate it:
		double ticksPerBeat = m_nMusicSpeed*m_nRowsPerBeat;		//ticks/beat = ticks/row  * rows/beat
		double samplesPerBeat = m_nSamplesPerTick*ticksPerBeat;	//samps/beat = samps/tick * ticks/beat
		bpm =  gdwMixingFreq/samplesPerBeat*60;					//beats/sec  = samps/sec  / samps/beat
	}															//beats/min  =  beats/sec * 60
	
	return bpm;
}

//end rewbs.VSTCompliance

void CSoundFile::ResetChannels()
//------------------------------
{
	m_dwSongFlags &= ~(SONG_CPUVERYHIGH|SONG_FADINGSONG|SONG_ENDREACHED|SONG_GLOBALFADE);
	m_nBufferCount = 0;
	for (UINT i=0; i<MAX_CHANNELS; i++)
	{
		Chn[i].nROfs = Chn[i].nLOfs = 0;
	}
}


MODTYPE CSoundFile::GetBestSaveFormat() const
//-------------------------------------------
{
	if ((!m_nSamples) || (!m_nChannels)) return MOD_TYPE_NONE;
	if (!m_nType) return MOD_TYPE_NONE;
	if (m_nType & (MOD_TYPE_MOD|MOD_TYPE_OKT))
		return MOD_TYPE_MOD;
	if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_STM|MOD_TYPE_ULT|MOD_TYPE_FAR|MOD_TYPE_PTM))
		return MOD_TYPE_S3M;
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MED|MOD_TYPE_MTM|MOD_TYPE_MT2))
		return MOD_TYPE_XM;
	if(m_nType & MOD_TYPE_MPT)
		return MOD_TYPE_MPT;
	return MOD_TYPE_IT;
}


MODTYPE CSoundFile::GetSaveFormats() const
//----------------------------------------
{
	UINT n = 0;
	if ((!m_nSamples) || (!m_nChannels) || (m_nType == MOD_TYPE_NONE)) return 0;
	switch(m_nType)
	{
	case MOD_TYPE_MOD:	n = MOD_TYPE_MOD;
	case MOD_TYPE_S3M:	n = MOD_TYPE_S3M;
	}
	n |= MOD_TYPE_XM | MOD_TYPE_IT | MOD_TYPE_MPT;
	if (!m_nInstruments)
	{
		if (m_nSamples < 32) n |= MOD_TYPE_MOD;
		n |= MOD_TYPE_S3M;
	}
	return n;
}


LPCTSTR CSoundFile::GetSampleName(UINT nSample) const
//---------------------------------------------------
{
	if (nSample<MAX_SAMPLES) {
		return m_szNames[nSample];
	} else {
		return gszEmpty;
	}
}


std::string CSoundFile::GetInstrumentName(UINT nInstr) const
//------------------------------------------------------
{
	if ((nInstr >= MAX_INSTRUMENTS) || (!GetInstrument(nInstr)))
		return TEXT("");

	const size_t nSize = ARRAYELEMCOUNT(GetInstrument(nInstr)->name);
	return GetInstrument(nInstr)->name;
}


bool CSoundFile::InitChannel(CHANNELINDEX nChn)
//---------------------------------------------
{
	if(nChn >= MAX_BASECHANNELS) return true;

	ChnSettings[nChn].nPan = 128;
	ChnSettings[nChn].nVolume = 64;
	ChnSettings[nChn].dwFlags = 0;
	ChnSettings[nChn].nMixPlugin = 0;
	ChnSettings[nChn].szName[0] = 0;

	ResetChannelState(nChn, CHNRESET_TOTAL);

	m_bChannelMuteTogglePending[nChn] = false;

	return false;
}

void CSoundFile::ResetChannelState(CHANNELINDEX i, BYTE resetMask)
//-------------------------------------------------------
{
	if(i >= MAX_CHANNELS) return;

	if(resetMask & 2)
	{
		Chn[i].nNote = Chn[i].nNewNote = Chn[i].nNewIns = 0;
		Chn[i].pModSample = nullptr;
		Chn[i].pModInstrument = nullptr;
		Chn[i].nPortamentoDest = 0;
		Chn[i].nCommand = 0;
		Chn[i].nFadeOutVol = 0;
		Chn[i].dwFlags |= CHN_KEYOFF|CHN_NOTEFADE;
		//IT compatibility 15. Retrigger
		if(IsCompatibleMode(TRK_IMPULSETRACKER))
		{
			Chn[i].nRetrigParam = 1;
			Chn[i].nRetrigCount = 0;
		}
		Chn[i].nTremorCount = 0;
		Chn[i].nEFxSpeed = 0;
	}

	if(resetMask & 4)
	{
		Chn[i].nPeriod = 0;
		Chn[i].nPos = Chn[i].nLength = 0;
		Chn[i].nLoopStart = 0;
		Chn[i].nLoopEnd = 0;
		Chn[i].nROfs = Chn[i].nLOfs = 0;
		Chn[i].pSample = nullptr;
		Chn[i].pModSample = nullptr;
		Chn[i].pModInstrument = nullptr;
		Chn[i].nCutOff = 0x7F;
		Chn[i].nResonance = 0;
		Chn[i].nFilterMode = 0;
		Chn[i].nLeftVol = Chn[i].nRightVol = 0;
		Chn[i].nNewLeftVol = Chn[i].nNewRightVol = 0;
		Chn[i].nLeftRamp = Chn[i].nRightRamp = 0;
		Chn[i].nVolume = 256;
		Chn[i].nVibratoPos = Chn[i].nTremoloPos = Chn[i].nPanbrelloPos = 0;

		//-->Custom tuning related
		Chn[i].m_ReCalculateFreqOnFirstTick = false;
		Chn[i].m_CalculateFreq = false;
		Chn[i].m_PortamentoFineSteps = 0;
		Chn[i].m_PortamentoTickSlide = 0;
		Chn[i].m_Freq = 0;
		Chn[i].m_VibratoDepth = 0;
		//<--Custom tuning related.
	}

	if(resetMask & 1)
	{
		if(i < MAX_BASECHANNELS)
		{
			Chn[i].dwFlags = ChnSettings[i].dwFlags;
			Chn[i].nPan = ChnSettings[i].nPan;
			Chn[i].nGlobalVol = ChnSettings[i].nVolume;
		}
		else
		{
			Chn[i].dwFlags = 0;
			Chn[i].nPan = 128;
			Chn[i].nGlobalVol = 64;
		}
		Chn[i].nRestorePanOnNewNote = 0;
		Chn[i].nRestoreCutoffOnNewNote = 0;
		Chn[i].nRestoreResonanceOnNewNote = 0;
		
	}
}


/////////////////////////////////////////////////////////////
// Transpose <-> Frequency conversions

// returns 8363*2^((transp*128+ftune)/(12*128))
DWORD CSoundFile::TransposeToFrequency(int transp, int ftune)
//-----------------------------------------------------------
{
	const float _fbase = 8363;
	const float _factor = 1.0f/(12.0f*128.0f);
	int result;
	DWORD freq;

	transp = (transp << 7) + ftune;
	_asm {
	fild transp
	fld _factor
	fmulp st(1), st(0)
	fist result
	fisub result
	f2xm1
	fild result
	fld _fbase
	fscale
	fstp st(1)
	fmul st(1), st(0)
	faddp st(1), st(0)
	fistp freq
	}
	UINT derr = freq % 11025;
	if (derr <= 8) freq -= derr;
	if (derr >= 11015) freq += 11025-derr;
	derr = freq % 1000;
	if (derr <= 5) freq -= derr;
	if (derr >= 995) freq += 1000-derr;
	return freq;
}


// returns 12*128*log2(freq/8363)
int CSoundFile::FrequencyToTranspose(DWORD freq)
//----------------------------------------------
{
	const float _f1_8363 = 1.0f / 8363.0f;
	const float _factor = 128 * 12;
	LONG result;
	
	if (!freq) return 0;
	_asm {
	fld _factor
	fild freq
	fld _f1_8363
	fmulp st(1), st(0)
	fyl2x
	fistp result
	}
	return result;
}


void CSoundFile::FrequencyToTranspose(MODSAMPLE *psmp)
//--------------------------------------------------------
{
	int f2t = FrequencyToTranspose(psmp->nC5Speed);
	int transp = f2t >> 7;
	int ftune = f2t & 0x7F; //0x7F == 111 1111
	if (ftune > 80)
	{
		transp++;
		ftune -= 128;
	}
	if (transp > 127) transp = 127;
	if (transp < -127) transp = -127;
	psmp->RelativeTone = transp;
	psmp->nFineTune = ftune;
}


void CSoundFile::CheckCPUUsage(UINT nCPU)
//---------------------------------------
{
	if (nCPU > 100) nCPU = 100;
	gnCPUUsage = nCPU;
	if (nCPU < 90)
	{
		m_dwSongFlags &= ~SONG_CPUVERYHIGH;
	} else
	if ((m_dwSongFlags & SONG_CPUVERYHIGH) && (nCPU >= 94))
	{
		UINT i=MAX_CHANNELS;
		while (i >= 8)
		{
			i--;
			if (Chn[i].nLength)
			{
				Chn[i].nLength = Chn[i].nPos = 0;
				nCPU -= 2;
				if (nCPU < 94) break;
			}
		}
	} else
	if (nCPU > 90)
	{
		m_dwSongFlags |= SONG_CPUVERYHIGH;
	}
}


void CSoundFile::BuildDefaultInstrument() 
//---------------------------------------
{
// m_defaultInstrument is currently only used to get default values for extented properties. 
// In the future we can make better use of this.
	assert(false);
	/*
	memset(&m_defaultInstrument, 0, sizeof(MODINSTRUMENT));
	m_defaultInstrument.nResampling = SRCMODE_DEFAULT;
	m_defaultInstrument.nFilterMode = FLTMODE_UNCHANGED;
	m_defaultInstrument.nPPC = 5*12;
	m_defaultInstrument.nGlobalVol=64;
	m_defaultInstrument.nPan = 0x20 << 2;
	m_defaultInstrument.nIFC = 0xFF;
	m_defaultInstrument.PanEnv.nReleaseNode=ENV_RELEASE_NODE_UNSET;
	m_defaultInstrument.PitchEnv.nReleaseNode=ENV_RELEASE_NODE_UNSET;
	m_defaultInstrument.VolEnv.nReleaseNode=ENV_RELEASE_NODE_UNSET;
	m_defaultInstrument.wPitchToTempoLock = 0;
	m_defaultInstrument.pTuning = m_defaultInstrument.s_DefaultTuning;
	m_defaultInstrument.nPluginVelocityHandling = PLUGIN_VELOCITYHANDLING_CHANNEL;
	m_defaultInstrument.nPluginVolumeHandling = CSoundFile::s_DefaultPlugVolumeHandling;
*/
}


void CSoundFile::DeleteStaticdata()
//---------------------------------
{
	delete s_pTuningsSharedLocal; s_pTuningsSharedLocal = 0;
	delete s_pTuningsSharedBuiltIn; s_pTuningsSharedBuiltIn = 0;
}

void SimpleMessageBox(const char* message, const char* title)
//-----------------------------------------------------------
{
	MessageBox(0, message, title, MB_ICONINFORMATION);
}

bool CSoundFile::LoadStaticTunings()
//-----------------------------------
{
	if(s_pTuningsSharedLocal || s_pTuningsSharedBuiltIn) return true;
	//For now not allowing to reload tunings(one should be careful when reloading them
	//since various parts may use addresses of the tuningobjects).

	CTuning::MessageHandler = &SimpleMessageBox;

	s_pTuningsSharedBuiltIn = new CTuningCollection;
	s_pTuningsSharedLocal = new CTuningCollection("Local tunings");

	// Load built-in tunings.
	const char* pData = nullptr;
	HGLOBAL hglob = nullptr;
	size_t nSize = 0;
/*	if (LoadResource(MAKEINTRESOURCE(IDR_BUILTIN_TUNINGS), TEXT("TUNING"), pData, nSize, hglob) != nullptr)
	{
		std::istrstream iStrm(pData, nSize);
		s_pTuningsSharedBuiltIn->Deserialize(iStrm);
		FreeResource(hglob);
	}*/
	if(s_pTuningsSharedBuiltIn->GetNumTunings() == 0)
	{
		assert(false);
		CTuningRTI* pT = new CTuningRTI;
		//Note: Tuning collection class handles deleting.
		pT->CreateGeometric(1,1);
		if(s_pTuningsSharedBuiltIn->AddTuning(pT))
			delete pT;
	}
		
	// Load local tunings.
	//CString sPath;
	//sPath.Format(TEXT("%slocal_tunings%s"), CMainFrame::GetDefaultDirectory(DIR_TUNING), CTuningCollection::s_FileExtension);
	//s_pTuningsSharedLocal->SetSavefilePath(sPath);
	//s_pTuningsSharedLocal->Deserialize();

	// Enabling adding/removing of tunings for standard collection
	// only for debug builds.
	#ifdef DEBUG
		s_pTuningsSharedBuiltIn->SetConstStatus(CTuningCollection::EM_ALLOWALL);
	#else
		s_pTuningsSharedBuiltIn->SetConstStatus(CTuningCollection::EM_CONST);
	#endif

	MODINSTRUMENT::s_DefaultTuning = NULL;

	return false;
}



void CSoundFile::SetDefaultInstrumentValues(MODINSTRUMENT *pIns) 
//-----------------------------------------------------------------
{
	assert(false);
	/*pIns->nResampling = m_defaultInstrument.nResampling;
	pIns->nFilterMode = m_defaultInstrument.nFilterMode;
	pIns->PitchEnv.nReleaseNode = m_defaultInstrument.PitchEnv.nReleaseNode;
	pIns->PanEnv.nReleaseNode = m_defaultInstrument.PanEnv.nReleaseNode;
	pIns->VolEnv.nReleaseNode = m_defaultInstrument.VolEnv.nReleaseNode;
	pIns->pTuning = m_defaultInstrument.pTuning;
	pIns->nPluginVelocityHandling = m_defaultInstrument.nPluginVelocityHandling;
	pIns->nPluginVolumeHandling = m_defaultInstrument.nPluginVolumeHandling;*/

}


void CSoundFile::SetModSpecsPointer(const CModSpecifications*& pModSpecs, const MODTYPE type)
//------------------------------------------------------------------------------------------
{
	switch(type)
	{
		case MOD_TYPE_MPT:
			pModSpecs = &ModSpecs::mptm;
		break;

		case MOD_TYPE_IT:
			pModSpecs = &ModSpecs::itEx;
		break;

		case MOD_TYPE_XM:
			pModSpecs = &ModSpecs::xmEx;
		break;

		case MOD_TYPE_S3M:
			pModSpecs = &ModSpecs::s3mEx;
		break;

		case MOD_TYPE_MOD:
		default:
			pModSpecs = &ModSpecs::modEx;
			break;
	}
}

uint16 CSoundFile::GetModFlagMask(const MODTYPE oldtype, const MODTYPE newtype) const
//-----------------------------------------------------------------------------------
{
	const MODTYPE combined = oldtype | newtype;

	// XM <-> IT/MPT conversion.
	if(combined == (MOD_TYPE_IT|MOD_TYPE_XM) || combined == (MOD_TYPE_MPT|MOD_TYPE_XM))
		return (1 << MSF_COMPATIBLE_PLAY) + (1 << MSF_MIDICC_BUGEMULATION);

	// IT <-> MPT conversion.
	if(combined == (MOD_TYPE_IT|MOD_TYPE_MPT))
		return uint16_max;

	return 0;
}

void CSoundFile::ChangeModTypeTo(const MODTYPE& newType)
//---------------------------------------------------
{
	const MODTYPE oldtype = m_nType;
	m_nType = newType;
	SetModSpecsPointer(m_pModSpecs, m_nType);
	SetupMODPanning(); // Setup LRRL panning scheme if needed

	m_ModFlags = m_ModFlags & GetModFlagMask(oldtype, newType);

	//Order.OnModTypeChanged(oldtype);
	//Patterns.OnModTypeChanged(oldtype);
}


bool CSoundFile::SetTitle(const char* titleCandidate, size_t strSize)
//-------------------------------------------------------------------
{
	if(strcmp(m_szNames[0], titleCandidate))
	{
		memset(m_szNames[0], 0, sizeof(m_szNames[0]));
		memcpy(m_szNames[0], titleCandidate, min(sizeof(m_szNames[0])-1, strSize));
		return true;
	}
	return false;
}

const CModSpecifications& CSoundFile::GetModSpecifications(const MODTYPE type)
//----------------------------------------------------------------------------
{
	const CModSpecifications* p = 0;
	SetModSpecsPointer(p, type);
	return *p;
}

void CSoundFile::SetupMODPanning(bool bForceSetup)
//------------------------------------------------
{
	// Setup LRRL panning, max channel volume
	if((m_nType & MOD_TYPE_MOD) == 0 && bForceSetup == false) return;

	for (CHANNELINDEX nChn = 0; nChn < MAX_BASECHANNELS; nChn++)
	{
		ChnSettings[nChn].nVolume = 64;
		if (gdwSoundSetup & SNDMIX_MAXDEFAULTPAN)
			ChnSettings[nChn].nPan = (((nChn & 3) == 1) || ((nChn & 3) == 2)) ? 256 : 0;
		else
			ChnSettings[nChn].nPan = (((nChn & 3) == 1) || ((nChn & 3) == 2)) ? 0xC0 : 0x40;
	}
}

MODSAMPLE* CSoundFile::GetSample(UINT n) const {
	return wavetable->GetSample(n);
}

MODINSTRUMENT* CSoundFile::GetInstrument(UINT n) const {
	return wavetable->GetInstrument(n);
}
