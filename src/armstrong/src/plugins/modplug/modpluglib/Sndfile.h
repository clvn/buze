/*
 * OpenMPT
 *
 * Sndfile.h
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
 *          OpenMPT devs
*/

#ifndef __SNDFILE_H
#define __SNDFILE_H

#include "SoundFilePlayConfig.h"
//#include "misc_util.h"
#include "mod_specifications.h"
#include <vector>
#include <bitset>
#include "Snd_defs.h"

class CTuningBase;
typedef CTuningBase CTuning;


// Sample Struct
struct MODSAMPLE
{
	UINT nLength,nLoopStart,nLoopEnd;
		//nLength <-> Number of 'frames'?
	UINT nSustainStart, nSustainEnd;
	LPSTR pSample;
	UINT nC5Speed;
	WORD nPan;
	WORD nVolume;
	WORD nGlobalVol;
	WORD uFlags;
	signed char RelativeTone;
	signed char nFineTune;
	BYTE nVibType;
	BYTE nVibSweep;
	BYTE nVibDepth;
	BYTE nVibRate;
	CHAR name[32];
	CHAR filename[22];

	// Return the size of one (elementary) sample in bytes.
	uint8 GetElementarySampleSize() const {return (uFlags & CHN_16BIT) ? 2 : 1;}

	// Return the number of channels in the sample.
	uint8 GetNumChannels() const {return (uFlags & CHN_STEREO) ? 2 : 1;}

	// Return the number of bytes per sampling point. (Channels * Elementary Sample Size)
	uint8 GetBytesPerSample() const {return GetElementarySampleSize() * GetNumChannels();}

	// Return the size which pSample is at least.
	DWORD GetSampleSizeInBytes() const {return nLength * GetBytesPerSample();}

	// Returns sample rate of the sample. The argument is needed because 
	// the sample rate is obtained differently for different module types.
	uint32 GetSampleRate(const MODTYPE type) const;
};


// -> CODE#0027
// -> DESC="per-instrument volume ramping setup (refered as attack)"

/*---------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------
MODULAR STRUCT DECLARATIONS :
-----------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------*/

// Instrument Envelopes
struct INSTRUMENTENVELOPE
{
	DWORD dwFlags;				// envelope flags
	WORD Ticks[MAX_ENVPOINTS];	// envelope point position (x axis)
	BYTE Values[MAX_ENVPOINTS];	// envelope point value (y axis)
	UINT nNodes;				// amount of nodes used
	BYTE nLoopStart;			// loop start node
	BYTE nLoopEnd;				// loop end node
	BYTE nSustainStart;			// sustain start node
	BYTE nSustainEnd;			// sustain end node
	BYTE nReleaseNode;			// release node
};

// Instrument Struct
struct MODINSTRUMENT
{
	UINT nFadeOut;
	DWORD dwFlags;
	UINT nGlobalVol;
	UINT nPan;

	INSTRUMENTENVELOPE VolEnv;
	INSTRUMENTENVELOPE PanEnv;
	INSTRUMENTENVELOPE PitchEnv;

	BYTE NoteMap[128];
	WORD Keyboard[128];

	BYTE nNNA;
	BYTE nDCT;
	BYTE nDNA;
	BYTE nPanSwing;
	BYTE nVolSwing;
	BYTE nIFC;
	BYTE nIFR;

	WORD wMidiBank;
	BYTE nMidiProgram;
	BYTE nMidiChannel;
	BYTE nMidiDrumKey;
	signed char nPPS; //Pitch to Pan Separator?
	unsigned char nPPC; //Pitch Centre?

	CHAR name[32];		// Note: not guaranteed to be null-terminated.
	CHAR filename[32];

	BYTE nMixPlug;							//rewbs.instroVSTi
// -> CODE#0027
// -> DESC="per-instrument volume ramping setup (refered as attack)"
	USHORT nVolRamp;
// -! NEW_FEATURE#0027
	UINT nResampling;
	BYTE nCutSwing;
	BYTE nResSwing;
	BYTE nFilterMode;
	WORD wPitchToTempoLock;
	BYTE nPluginVelocityHandling;
	BYTE nPluginVolumeHandling;
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// WHEN adding new members here, ALSO update Sndfile.cpp (instructions near the top of this file)!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	CTuning* pTuning;
	static CTuning* s_DefaultTuning;

	void SetTuning(CTuning* pT)
	{
		pTuning = pT;
	}

	

};

//MODINSTRUMENT;

// -----------------------------------------------------------------------------------------
// MODULAR MODINSTRUMENT FIELD ACCESS : body content at the (near) top of Sndfile.cpp !!!
// -----------------------------------------------------------------------------------------
extern void WriteInstrumentHeaderStruct(MODINSTRUMENT * input, FILE * file);
extern BYTE * GetInstrumentHeaderFieldPointer(MODINSTRUMENT * input, __int32 fcode, __int16 fsize);

// -! NEW_FEATURE#0027

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

#pragma warning(disable : 4324) //structure was padded due to __declspec(align())

// Channel Struct
typedef struct __declspec(align(32)) _MODCHANNEL
{
	// First 32-bytes: Most used mixing information: don't change it
	LPSTR pCurrentSample;		
	DWORD nPos;
	DWORD nPosLo;	// actually 16-bit
	LONG nInc;		// 16.16
	LONG nRightVol;
	LONG nLeftVol;
	LONG nRightRamp;
	LONG nLeftRamp;
	// 2nd cache line
	DWORD nLength;
	DWORD dwFlags;
	DWORD nLoopStart;
	DWORD nLoopEnd;
	LONG nRampRightVol;
	LONG nRampLeftVol;
	LONG nFilter_Y1, nFilter_Y2, nFilter_Y3, nFilter_Y4;
	LONG nFilter_A0, nFilter_B0, nFilter_B1, nFilter_HP;
	LONG nROfs, nLOfs;
	LONG nRampLength;
	// Information not used in the mixer
	LPSTR pSample;
	LONG nNewRightVol, nNewLeftVol;
	LONG nRealVolume, nRealPan;
	LONG nVolume, nPan, nFadeOutVol;
	LONG nPeriod, nC5Speed, nPortamentoDest;
	MODINSTRUMENT *pModInstrument;
	MODSAMPLE *pModSample;
	DWORD nVolEnvPosition, nPanEnvPosition, nPitchEnvPosition;
	LONG nVolEnvValueAtReleaseJump, nPanEnvValueAtReleaseJump, nPitchEnvValueAtReleaseJump;
	DWORD nMasterChn, nVUMeter;
	LONG nGlobalVol, nInsVol;
	LONG nFineTune, nTranspose;
	LONG nPortamentoSlide, nAutoVibDepth;
	UINT nAutoVibPos, nVibratoPos, nTremoloPos, nPanbrelloPos;
	LONG nVolSwing, nPanSwing;
	LONG nCutSwing, nResSwing;
	LONG nRestorePanOnNewNote; //If > 0, nPan should be set to nRestorePanOnNewNote - 1 on new note. Used to recover from panswing.
	UINT nOldGlobalVolSlide;
	DWORD nEFxOffset; // offset memory for Invert Loop (EFx, .MOD only)
	// 8-bit members
	BYTE nRestoreResonanceOnNewNote; //Like above
	BYTE nRestoreCutoffOnNewNote; //Like above
	BYTE nNote, nNNA;
	BYTE nNewNote, nNewIns, nCommand, nArpeggio;
	BYTE nOldVolumeSlide, nOldFineVolUpDown;
	BYTE nOldPortaUpDown, nOldFinePortaUpDown;
	BYTE nOldPanSlide, nOldChnVolSlide;
    UINT nNoteSlideCounter, nNoteSlideSpeed, nNoteSlideStep;
	BYTE nVibratoType, nVibratoSpeed, nVibratoDepth;
	BYTE nTremoloType, nTremoloSpeed, nTremoloDepth;
	BYTE nPanbrelloType, nPanbrelloSpeed, nPanbrelloDepth;
	BYTE nOldCmdEx, nOldVolParam, nOldTempo;
	BYTE nOldOffset, nOldHiOffset;
	BYTE nCutOff, nResonance;
	int nRetrigCount, nRetrigParam;
	BYTE nTremorCount, nTremorParam;
	//BYTE nPatternLoop, nPatternLoopCount;
	BYTE nRowNote, nRowInstr;
	BYTE nRowVolCmd, nRowVolume;
	BYTE nRowCommand, nRowParam;
	BYTE nLeftVU, nRightVU;
	BYTE nActiveMacro, nFilterMode;
	BYTE nEFxSpeed, nEFxDelay; // memory for Invert Loop (EFx, .MOD only)

	uint16 m_RowPlugParam;			//NOTE_PCs memory.
	float m_nPlugParamValueStep;  //rewbs.smoothVST 
	float m_nPlugInitialParamValue; //rewbs.smoothVST
	PLUGINDEX m_RowPlug;			//NOTE_PCs memory.
	
	void ClearRowCmd() {nRowNote = NOTE_NONE; nRowInstr = 0; nRowVolCmd = VOLCMD_NONE; nRowVolume = 0; nRowCommand = CMD_NONE; nRowParam = 0;}

	typedef UINT VOLUME;
	VOLUME GetVSTVolume() {return (pModInstrument) ? pModInstrument->nGlobalVol*4 : nVolume;}

	//-->Variables used to make user-definable tuning modes work with pattern effects.
		bool m_ReCalculateFreqOnFirstTick;
		//If true, freq should be recalculated in ReadNote() on first tick.
		//Currently used only for vibrato things - using in other context might be 
		//problematic.

		bool m_CalculateFreq;
		//To tell whether to calculate frequency.

		int32 m_PortamentoFineSteps;
		long m_PortamentoTickSlide;

		UINT m_Freq;
		float m_VibratoDepth;
	//<----
} MODCHANNEL;

#define CHNRESET_CHNSETTINGS	1 //  1 b 
#define CHNRESET_SETPOS_BASIC	2 // 10 b
#define	CHNRESET_SETPOS_FULL	7 //111 b
#define CHNRESET_TOTAL			255 //11111111b


struct MODCHANNELSETTINGS
{
	UINT nPan;
	UINT nVolume;
	DWORD dwFlags;
	PLUGINDEX nMixPlugin;
	CHAR szName[MAX_CHANNELNAME];
};

#include "modcommand.h"

//class CSoundFile;

struct SNDMIXSONGEQ
{
	ULONG nEQBands;
	ULONG EQFreq_Gains[MAX_EQ_BANDS];
};
typedef SNDMIXSONGEQ* PSNDMIXSONGEQ;

////////////////////////////////////////////////////////////////////////
// Reverberation

struct SNDMIX_REVERB_PROPERTIES
{
	LONG  lRoom;                   // [-10000, 0]      default: -10000 mB
    LONG  lRoomHF;                 // [-10000, 0]      default: 0 mB
    FLOAT flDecayTime;             // [0.1, 20.0]      default: 1.0 s
    FLOAT flDecayHFRatio;          // [0.1, 2.0]       default: 0.5
    LONG  lReflections;            // [-10000, 1000]   default: -10000 mB
    FLOAT flReflectionsDelay;      // [0.0, 0.3]       default: 0.02 s
    LONG  lReverb;                 // [-10000, 2000]   default: -10000 mB
    FLOAT flReverbDelay;           // [0.0, 0.1]       default: 0.04 s
    FLOAT flDiffusion;             // [0.0, 100.0]     default: 100.0 %
    FLOAT flDensity;               // [0.0, 100.0]     default: 100.0 %
};
typedef SNDMIX_REVERB_PROPERTIES* PSNDMIX_REVERB_PROPERTIES;

#ifndef NO_REVERB

#define NUM_REVERBTYPES			29

LPCSTR GetReverbPresetName(UINT nPreset);

#endif

////////////////////////////////////////////////////////////////////

enum {
	MIDIOUT_START=0,
	MIDIOUT_STOP,
	MIDIOUT_TICK,
	MIDIOUT_NOTEON,
	MIDIOUT_NOTEOFF,
	MIDIOUT_VOLUME,
	MIDIOUT_PAN,
	MIDIOUT_BANKSEL,
	MIDIOUT_PROGRAM,
};


struct MODMIDICFG
{
	CHAR szMidiGlb[9*32];
	CHAR szMidiSFXExt[16*32];
	CHAR szMidiZXXExt[128*32];
};
typedef MODMIDICFG* LPMODMIDICFG;

typedef VOID (__cdecl * LPSNDMIXHOOKPROC)(int *, unsigned long, unsigned long); // buffer, samples, channels




class CSoundFile;

//Note: These are bit indeces. MSF <-> Mod(Specific)Flag.
//If changing these, ChangeModTypeTo() might need modification.
const BYTE MSF_COMPATIBLE_PLAY		= 0;		//IT/MPT/XM
const BYTE MSF_OLDVOLSWING			= 1;		//IT/MPT
const BYTE MSF_MIDICC_BUGEMULATION	= 2;		//IT/MPT/XM


class CTuningCollection;

class CWaveTable {
public:
	virtual ~CWaveTable() {}
	virtual MODINSTRUMENT* GetInstrument(UINT n) = 0;
	virtual MODSAMPLE* GetSample(UINT n) = 0;
};

//==============
class CSoundFile
//==============
{
public:
	//Return true if title was changed.
	bool SetTitle(const char*, size_t strSize);

public: //Misc
	void ChangeModTypeTo(const MODTYPE& newType);
	
	//Returns value in seconds. If given position won't be played at all, returns -1.
	//double GetPlaybackTimeAt(ORDERINDEX, ROWINDEX);

	uint16 GetModFlags() const {return m_ModFlags;}
	void SetModFlags(const uint16 v) {m_ModFlags = v;}
	bool GetModFlag(BYTE i) const {return ((m_ModFlags & (1<<i)) != 0);}
	void SetModFlag(BYTE i, bool val) {if(i < 8*sizeof(m_ModFlags)) {m_ModFlags = (val) ? m_ModFlags |= (1 << i) : m_ModFlags &= ~(1 << i);}}

	// Is compatible mode for a specific tracker turned on?
	// Hint 1: No need to poll for MOD_TYPE_MPT, as it will automatically be linked with MOD_TYPE_IT
	// Hint 2:  Always returns true for MOD / S3M format (if that is the format of the current file)
	bool IsCompatibleMode(MODTYPE type) {
		if(GetType() & type & (MOD_TYPE_MOD | MOD_TYPE_S3M))
			return true; // those formats don't have flags so we will always return true
		return ((GetType() & ((type & MOD_TYPE_IT) ? type | MOD_TYPE_MPT : type)) && GetModFlag(MSF_COMPATIBLE_PLAY)) ? true : false;
	}
	
	//Tuning-->
public:
	static bool LoadStaticTunings();
	static bool SaveStaticTunings();
	static void DeleteStaticdata();
	static CTuningCollection& GetBuiltInTunings() {return *s_pTuningsSharedBuiltIn;}
	static CTuningCollection& GetLocalTunings() {return *s_pTuningsSharedLocal;}
	CTuningCollection& GetTuneSpecificTunings() {return *m_pTuningsTuneSpecific;}

	//std::string GetNoteName(const int16&, const int inst = -1) const;
private:
	CWaveTable* wavetable;
	CTuningCollection* m_pTuningsTuneSpecific;
	static CTuningCollection* s_pTuningsSharedBuiltIn;
	static CTuningCollection* s_pTuningsSharedLocal;
	//<--Tuning

private: //Effect functions
	void PortamentoMPT(MODCHANNEL*, int);
	void PortamentoFineMPT(MODCHANNEL*, int);

private: //Misc private methods.
	static void SetModSpecsPointer(const CModSpecifications*& pModSpecs, const MODTYPE type);
	uint16 GetModFlagMask(const MODTYPE oldtype, const MODTYPE newtype) const;

private: //Misc data
	uint16 m_ModFlags;
	const CModSpecifications* m_pModSpecs;


public:	// Static Members
	static UINT m_nXBassDepth, m_nXBassRange;
	static float m_nMaxSample;
	static UINT m_nReverbDepth, gnReverbType;
	static UINT m_nProLogicDepth, m_nProLogicDelay;
	static UINT m_nStereoSeparation;
	static UINT m_nMaxMixChannels;
	static LONG m_nStreamVolume;
	static DWORD gdwSysInfo, gdwSoundSetup, gdwMixingFreq, gnBitsPerSample, gnChannels;
	static UINT gnAGC, gnVolumeRampSamples, gnCPUUsage;
	static LPSNDMIXHOOKPROC gpSndMixHook;
	static uint8 s_DefaultPlugVolumeHandling;


public:	// for Editing
	UINT m_nType;
	CHANNELINDEX m_nChannels;
	SAMPLEINDEX m_nSamples;
	INSTRUMENTINDEX m_nInstruments;
	UINT m_nDefaultSpeed, m_nDefaultTempo, m_nDefaultGlobalVolume;
	DWORD m_dwSongFlags;							// Song flags SONG_XXXX
	bool m_bIsRendering;
	UINT m_nMixChannels, m_nMixStat, m_nBufferCount;
	double m_dBufferDiff;
	UINT m_nTickCount, m_nTotalCount, m_nPatternDelay, m_nFrameDelay;
	ULONG m_lTotalSampleCount;	// rewbs.VSTTimeInfo
	UINT m_nSamplesPerTick;	// rewbs.betterBPM
	UINT m_nRowsPerBeat;	// rewbs.betterBPM
	UINT m_nRowsPerMeasure;	// rewbs.betterBPM
	BYTE m_nTempoMode;		// rewbs.betterBPM
	BYTE m_nMixLevels;
    UINT m_nMusicSpeed, m_nMusicTempo;
	UINT m_nMasterVolume, m_nGlobalVolume, m_nSamplesToGlobalVolRampDest,
		 m_nGlobalVolumeDestination, m_nSamplePreAmp, m_nVSTiVolume;
	long m_lHighResRampingGlobalVolume;
	UINT m_nFreqFactor, m_nTempoFactor, m_nOldGlbVolSlide;
	LONG m_nMinPeriod, m_nMaxPeriod, m_nRepeatCount;
	DWORD m_nGlobalFadeSamples, m_nGlobalFadeMaxSamples;
	LPSTR m_lpszSongComments, m_lpszPatternNames;
	UINT ChnMix[MAX_CHANNELS];							// Channels to be mixed
	MODCHANNEL Chn[MAX_CHANNELS];						// Channels
	MODCHANNELSETTINGS ChnSettings[MAX_BASECHANNELS];	// Channels settings
	CHAR m_szNames[MAX_SAMPLES][32];					// Song and sample names
	MODMIDICFG m_MidiCfg;								// Midi macro config table
	SNDMIXSONGEQ m_SongEQ;								// Default song EQ preset
	CHAR CompressionTable[16];
	bool m_bChannelMuteTogglePending[MAX_BASECHANNELS];

	CSoundFilePlayConfig* m_pConfig;
	DWORD m_dwCreatedWithVersion;
	DWORD m_dwLastSavedWithVersion;

// -> CODE#0023
// -> DESC="IT project files (.itp)"
	CHAR m_szInstrumentPath[MAX_INSTRUMENTS][_MAX_PATH];
	bool instrumentModified[MAX_INSTRUMENTS];
// -! NEW_FEATURE#0023

public:
	CSoundFile(CWaveTable* _wavetable);
	~CSoundFile();

public:
	BOOL Create(LPCBYTE lpStream, DWORD dwMemLength=0);
	BOOL Destroy();
	MODTYPE GetType() const { return m_nType; }
	inline bool TypeIsIT_MPT() const {return (m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT)) != 0;}
	inline bool TypeIsIT_MPT_XM() const {return (m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT | MOD_TYPE_XM)) != 0;}
	inline bool TypeIsS3M_IT_MPT() const {return (m_nType & (MOD_TYPE_S3M | MOD_TYPE_IT | MOD_TYPE_MPT)) != 0;}
	inline bool TypeIsXM_MOD() const {return (m_nType & (MOD_TYPE_XM | MOD_TYPE_MOD)) != 0;}
	inline bool TypeIsMOD_S3M() const {return (m_nType & (MOD_TYPE_MOD | MOD_TYPE_S3M)) != 0;}

	//Return the number of channels in the pattern. In 1.17.02.45
	//it returned the number of channels with volume != 0
	CHANNELINDEX GetNumChannels() const {return static_cast<CHANNELINDEX>(m_nChannels);}

	void SetMasterVolume(UINT vol, bool adjustAGC = false);
	UINT GetMasterVolume() const { return m_nMasterVolume; }
	INSTRUMENTINDEX GetNumInstruments() const { return m_nInstruments; } 
	SAMPLEINDEX GetNumSamples() const { return m_nSamples; }
	UINT GetSongComments(LPSTR s, UINT cbsize, UINT linesize=32);
	UINT GetRawSongComments(LPSTR s, UINT cbsize, UINT linesize=32);

	const CModSpecifications& GetModSpecifications() const {return *m_pModSpecs;}
	static const CModSpecifications& GetModSpecifications(const MODTYPE type);

	double GetCurrentBPM() const;
	void GetTitle(LPSTR s) const { lstrcpyn(s,m_szNames[0],32); }
	LPCSTR GetTitle() const { return m_szNames[0]; }
	LPCTSTR GetSampleName(UINT nSample) const;
	std::string GetInstrumentName(UINT nInstr) const;
	UINT GetMusicSpeed() const { return m_nMusicSpeed; }
	UINT GetMusicTempo() const { return m_nMusicTempo; }
private:

public:
	void SetRepeatCount(int n) { m_nRepeatCount = n; }
	int GetRepeatCount() const { return m_nRepeatCount; }
	BOOL IsPaused() const {	return (m_dwSongFlags & SONG_PAUSED) ? TRUE : FALSE; }
	void CheckCPUUsage(UINT nCPU);
	CHANNELINDEX ReArrangeChannels(const std::vector<CHANNELINDEX>& fromToArray);
	bool MoveChannel(UINT chn_from, UINT chn_to);

	bool InitChannel(CHANNELINDEX nChn);
	void ResetChannelState(CHANNELINDEX chn, BYTE resetStyle);

	void SetupMODPanning(bool bForceSetup = false); // Setup LRRL panning, max channel volume

	// MOD Convert function
	MODTYPE GetBestSaveFormat() const;
	MODTYPE GetSaveFormats() const;

public:
	// Real-time sound functions
	VOID ResetChannels();
	UINT CreateStereoMix(int count);
	UINT GetResamplingFlag(const MODCHANNEL *pChannel);
	BOOL FadeSong(UINT msec);
	BOOL GlobalFadeSong(UINT msec);
	UINT GetTotalTickCount() const { return m_nTotalCount; }
	VOID ResetTotalTickCount() { m_nTotalCount = 0;}
	VOID ProcessPlugins(UINT nCount);

public:
	// Mixer Config
	static BOOL InitPlayer(BOOL bReset=FALSE);
	static BOOL SetWaveConfig(UINT nRate,UINT nBits,UINT nChannels,BOOL bMMX=FALSE);
	static BOOL SetDspEffects(BOOL bSurround,BOOL bReverb,BOOL xbass,BOOL dolbynr=FALSE,BOOL bEQ=FALSE);
	static BOOL SetResamplingMode(UINT nMode); // SRCMODE_XXXX
	static BOOL IsStereo() { return (gnChannels > 1) ? TRUE : FALSE; }
	static DWORD GetSampleRate() { return gdwMixingFreq; }
	static DWORD GetBitsPerSample() { return gnBitsPerSample; }
	static DWORD InitSysInfo();
	static DWORD GetSysInfo() { return gdwSysInfo; }
	static void EnableMMX(BOOL b) { if (b) gdwSoundSetup |= SNDMIX_ENABLEMMX; else gdwSoundSetup &= ~SNDMIX_ENABLEMMX; }
	// AGC
	static BOOL GetAGC() { return (gdwSoundSetup & SNDMIX_AGC) ? TRUE : FALSE; }
	static void SetAGC(BOOL b);
	static void ResetAGC();
	static void ProcessAGC(int count);
	// DSP Effects
	static void InitializeDSP(BOOL bReset);
	static void ProcessStereoDSP(int count);
	static void ProcessMonoDSP(int count);
	// [Reverb level 0(quiet)-100(loud)], [REVERBTYPE_XXXX]
	static BOOL SetReverbParameters(UINT nDepth, UINT nType);
	// [XBass level 0(quiet)-100(loud)], [cutoff in Hz 10-100]
	static BOOL SetXBassParameters(UINT nDepth, UINT nRange);
	// [Surround level 0(quiet)-100(heavy)] [delay in ms, usually 5-40ms]
	static BOOL SetSurroundParameters(UINT nDepth, UINT nDelay);
#ifdef ENABLE_EQ
	// EQ
	static void InitializeEQ(BOOL bReset=TRUE);
	static void SetEQGains(const UINT *pGains, UINT nBands, const UINT *pFreqs=NULL, BOOL bReset=FALSE);	// 0=-12dB, 32=+12dB
	/*static*/ void EQStereo(int *pbuffer, UINT nCount);
	/*static*/ void EQMono(int *pbuffer, UINT nCount);
#endif
	// Float <-> Int conversion routines
	/*static */VOID StereoMixToFloat(const int *pSrc, float *pOut1, float *pOut2, UINT nCount);
	/*static */VOID FloatToStereoMix(const float *pIn1, const float *pIn2, int *pOut, UINT nCount);
	/*static */VOID MonoMixToFloat(const int *pSrc, float *pOut, UINT nCount);
	/*static */VOID FloatToMonoMix(const float *pIn, int *pOut, UINT nCount);

public:
	void ProcessEvents(MODCOMMAND* pCommands);
	void ProcessSamples(float** pouts, UINT lCount);
	BOOL ReadNote();
	void ProcessTick();
	BOOL ProcessRow();
	BOOL ProcessEffects();
	UINT GetNNAChannel(UINT nChn) const;
	void CheckNNA(UINT nChn, UINT instr, int note, BOOL bForceCut);
	void NoteChange(UINT nChn, int note, bool bPorta = false, bool bResetEnv = true, bool bManual = false);
	void InstrumentChange(MODCHANNEL *pChn, UINT instr, BOOL bPorta=FALSE,BOOL bUpdVol=TRUE,BOOL bResetEnv=TRUE);

	// Channel Effects
	void KeyOff(UINT nChn);
	// Global Effects
	void SetTempo(UINT param, bool setAsNonModcommand = false);
	void SetSpeed(UINT param);

private:
	// Channel Effects
	void PortamentoUp(MODCHANNEL *pChn, UINT param, const bool fineAsRegular = false);
	void PortamentoDown(MODCHANNEL *pChn, UINT param, const bool fineAsRegular = false);
	void MidiPortamento(MODCHANNEL *pChn, int param);
	void FinePortamentoUp(MODCHANNEL *pChn, UINT param);
	void FinePortamentoDown(MODCHANNEL *pChn, UINT param);
	void ExtraFinePortamentoUp(MODCHANNEL *pChn, UINT param);
	void ExtraFinePortamentoDown(MODCHANNEL *pChn, UINT param);
	void NoteSlide(MODCHANNEL *pChn, UINT param, int sign);
	void TonePortamento(MODCHANNEL *pChn, UINT param);
	void Vibrato(MODCHANNEL *pChn, UINT param);
	void FineVibrato(MODCHANNEL *pChn, UINT param);
	void VolumeSlide(MODCHANNEL *pChn, UINT param);
	void PanningSlide(MODCHANNEL *pChn, UINT param);
	void ChannelVolSlide(MODCHANNEL *pChn, UINT param);
	void FineVolumeUp(MODCHANNEL *pChn, UINT param);
	void FineVolumeDown(MODCHANNEL *pChn, UINT param);
	void Tremolo(MODCHANNEL *pChn, UINT param);
	void Panbrello(MODCHANNEL *pChn, UINT param);
	void RetrigNote(UINT nChn, int param, UINT offset=0);  //rewbs.volOffset: added last param
	void SampleOffset(UINT nChn, UINT param, bool bPorta);	//rewbs.volOffset: moved offset code to own method
	void NoteCut(UINT nChn, UINT nTick);
	void ExtendedMODCommands(UINT nChn, UINT param);
	void ExtendedS3MCommands(UINT nChn, UINT param);
	void ExtendedChannelEffect(MODCHANNEL *, UINT param);
	inline void InvertLoop(MODCHANNEL* pChn);
	void ProcessMidiMacro(UINT nChn, LPCSTR pszMidiMacro, UINT param=0);
	void ProcessSmoothMidiMacro(UINT nChn, LPCSTR pszMidiMacro, UINT param=0); //rewbs.smoothVST
	void SetupChannelFilter(MODCHANNEL *pChn, bool bReset, int flt_modifier = 256) const;
	// Low-Level effect processing
	void DoFreqSlide(MODCHANNEL *pChn, LONG nFreqSlide);
	void GlobalVolSlide(UINT param, UINT * nOldGlobalVolSlide);
public:
	// I/O from another sound file
	bool ReadInstrumentFromSong(INSTRUMENTINDEX nInstr, CSoundFile *pSrcSong, UINT nSrcInstrument);
	bool ReadSampleFromSong(SAMPLEINDEX nSample, CSoundFile *pSrcSong, UINT nSrcSample);
	// Period/Note functions
	UINT GetNoteFromPeriod(UINT period) const;
	UINT GetPeriodFromNote(UINT note, int nFineTune, UINT nC5Speed) const;
	UINT GetFreqFromPeriod(UINT period, UINT nC5Speed, int nPeriodFrac=0) const;
	// Misc functions
	MODSAMPLE *GetSample(UINT n) const;
	MODINSTRUMENT* GetInstrument(UINT n) const;
	void ResetMidiCfg();
	UINT MapMidiInstrument(DWORD dwProgram, UINT nChannel, UINT nNote);
#ifndef NO_FILTER
	DWORD CutOffToFrequency(UINT nCutOff, int flt_modifier=256) const; // [0-255] => [1-10KHz]
#endif
#ifdef MODPLUG_TRACKER
	VOID ProcessMidiOut(UINT nChn, MODCHANNEL *pChn);		//rewbs.VSTdelay : added arg.
#endif
	VOID ApplyGlobalVolume(int SoundBuffer[], long lTotalSampleCount);

	// Static helper functions
public:
	static DWORD TransposeToFrequency(int transp, int ftune=0);
	static int FrequencyToTranspose(DWORD freq);
	static void FrequencyToTranspose(MODSAMPLE *psmp);

	// System-Dependant functions
public:
	static UINT Normalize24BitBuffer(LPBYTE pbuffer, UINT cbsizebytes, DWORD lmax24, DWORD dwByteInc);

public:
	int getVolEnvValueFromPosition(int position, MODINSTRUMENT* pIns);
    void resetEnvelopes(MODCHANNEL* pChn, int envToReset = ENV_RESET_ALL);
	void SetDefaultInstrumentValues(MODINSTRUMENT *pIns);
private:
	UINT GetBestMidiChan(MODCHANNEL *pChn);

	void BuildDefaultInstrument();

public:
	// "importance" of every FX command. Table is used for importing from formats with multiple effect colums
	// and is approximately the same as in SchismTracker.
	static uint16 CSoundFile::GetEffectWeight(MODCOMMAND::COMMAND cmd);
};

#pragma warning(default : 4324) //structure was padded due to __declspec(align())


inline uint32 MODSAMPLE::GetSampleRate(const MODTYPE type) const
//--------------------------------------------------------------
{
	uint32 nRate;
	if(type & (MOD_TYPE_MOD|MOD_TYPE_XM))
		nRate = CSoundFile::TransposeToFrequency(RelativeTone, nFineTune);
	else
		nRate = nC5Speed;
	return (nRate > 0) ? nRate : 8363;
}

// Ending swaps:
// BigEndian(x) may be used either to:
// -Convert DWORD x, which is in big endian format(for example read from file),
//		to endian format of current architecture.
// -Convert value x from endian format of current architecture to big endian format.
// Similarly LittleEndian(x) converts known little endian format to format of current
// endian architecture or value x in format of current architecture to little endian 
// format.
#ifdef PLATFORM_BIG_ENDIAN
// PPC
inline DWORD LittleEndian(DWORD x)	{ return ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8) | ((x & 0xFF000000) >> 24); }
inline WORD LittleEndianW(WORD x)	{ return (WORD)(((x >> 8) & 0xFF) | ((x << 8) & 0xFF00)); }
#define BigEndian(x)				(x)
#define BigEndianW(x)				(x)
#else
// x86
inline DWORD BigEndian(DWORD x)	{ return ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8) | ((x & 0xFF000000) >> 24); }
inline WORD BigEndianW(WORD x)	{ return (WORD)(((x >> 8) & 0xFF) | ((x << 8) & 0xFF00)); }
#define LittleEndian(x)			(x)
#define LittleEndianW(x)		(x)
#endif

///////////////////////////////////////////////////////////
// Low-level Mixing functions

#define MIXBUFFERSIZE		512
#define SCRATCH_BUFFER_SIZE 64 //Used for plug's final processing (cleanup)
#define MIXING_ATTENUATION	4
#define VOLUMERAMPPRECISION	12	
#define FADESONGDELAY		100
#define EQ_BUFFERSIZE		(MIXBUFFERSIZE)
#define AGC_PRECISION		10
#define AGC_UNITY			(1 << AGC_PRECISION)

// Calling conventions
#ifdef WIN32
#define MPPASMCALL	__cdecl
#define MPPFASTCALL	__fastcall
#else
#define MPPASMCALL
#define MPPFASTCALL
#endif

#define MOD2XMFineTune(k)	((int)( (signed char)((k)<<4) ))
#define XM2MODFineTune(k)	((int)( (k>>4)&0x0f ))

int _muldiv(long a, long b, long c);
int _muldivr(long a, long b, long c);

///////////////////////////////////////////////////////////
// File Formats Information (name, extension, etc)

#ifndef FASTSOUNDLIB

typedef struct MODFORMATINFO
{
	DWORD dwFormatId;		// MOD_TYPE_XXXX
	LPCSTR lpszFormatName;	// "ProTracker"
	LPCSTR lpszExtension;	// ".xxx"
	DWORD dwPadding;
} MODFORMATINFO;

extern MODFORMATINFO gModFormatInfo[MAX_MODTYPE];

#endif

#endif
