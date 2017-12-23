/*
 * OpenMPT
 *
 * Snd_fx.cpp
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>
 *          OpenMPT devs
*/

#include "stdafx.h"
#include "sndfile.h"
#include "tuning.h"

#pragma warning(disable:4244)

#ifndef CLAMP
#define CLAMP(number, low, high) min(high, max(low, number))
#endif

// Tables defined in tables.cpp
extern BYTE ImpulseTrackerPortaVolCmd[16];
extern WORD S3MFineTuneTable[16];
extern WORD ProTrackerPeriodTable[6*12];
extern WORD ProTrackerTunedPeriods[15*12];
extern WORD FreqS3MTable[];
extern WORD XMPeriodTable[96+8];
extern UINT XMLinearTable[768];
extern DWORD FineLinearSlideUpTable[16];
extern DWORD FineLinearSlideDownTable[16];
extern DWORD LinearSlideUpTable[256];
extern DWORD LinearSlideDownTable[256];
extern signed char retrigTable1[16];
extern signed char retrigTable2[16];
extern short int ModRandomTable[64];
extern BYTE ModEFxTable[16];

//////////////////////////////////////////////////////////////////////////////////////////////////
// Effects

void CSoundFile::InstrumentChange(MODCHANNEL *pChn, UINT instr, BOOL bPorta, BOOL bUpdVol, BOOL bResetEnv)
//--------------------------------------------------------------------------------------------------------
{
	BOOL bInstrumentChanged = FALSE;

	if (instr >= MAX_INSTRUMENTS) return;
	MODINSTRUMENT *pIns = GetInstrument(instr);
	MODSAMPLE *pSmp = GetSample(instr);//&Samples[instr];
	UINT note = pChn->nNewNote;

	if(note == 0 && IsCompatibleMode(TRK_IMPULSETRACKER)) return;

	if ((pIns) && (note) && (note <= 128))
	{
		if (pIns->NoteMap[note-1] >= NOTE_MIN_SPECIAL) return;
		UINT n = pIns->Keyboard[note-1];
		pSmp = ((n) && (n < MAX_SAMPLES)) ? GetSample(n) : NULL;
	} else
	if (m_nInstruments)
	{
		if (note >= 0xFE) return;
		pSmp = NULL;
	}

	const bool bNewTuning = (m_nType == MOD_TYPE_MPT && pIns && pIns->pTuning);
	//Playback behavior change for MPT: With portamento don't change sample if it is in
	//the same instrument as previous sample.
	if(bPorta && bNewTuning && pIns == pChn->pModInstrument)
		return;

	bool returnAfterVolumeAdjust = false;
	// bInstrumentChanged is used for IT carry-on env option
	if (pIns != pChn->pModInstrument)
	{
		bInstrumentChanged = TRUE;
		pChn->pModInstrument = pIns;
	} 
	else 
	{	
		// Special XM hack
		if ((bPorta) && (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) && (pIns)
		&& (pChn->pModSample) && (pSmp != pChn->pModSample))
		{
			// FT2 doesn't change the sample in this case,
			// but still uses the sample info from the old one (bug?)
			returnAfterVolumeAdjust = true;
		}
	}
	
	// Update Volume
	if (bUpdVol)
	{
		pChn->nVolume = 0;
		if(pSmp)
			pChn->nVolume = pSmp->nVolume;
		else
		{
			if(pIns && pIns->nMixPlug)
				pChn->nVolume = pChn->GetVSTVolume();
		}
	}

	if(returnAfterVolumeAdjust) return;

	
	// Instrument adjust
	pChn->nNewIns = 0;
	
	if (pIns && (pIns->nMixPlug || pSmp))		//rewbs.VSTiNNA
		pChn->nNNA = pIns->nNNA;

	if (pSmp)
	{
		if (pIns)
		{
			pChn->nInsVol = (pSmp->nGlobalVol * pIns->nGlobalVol) >> 6;
			if (pIns->dwFlags & INS_SETPANNING) pChn->nPan = pIns->nPan;
		} else
		{
			pChn->nInsVol = pSmp->nGlobalVol;
		}
		if (pSmp->uFlags & CHN_PANNING) pChn->nPan = pSmp->nPan;
	}


	// Reset envelopes
	if (bResetEnv)
	{
		if ((!bPorta) || (!(m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT))) || (m_dwSongFlags & SONG_ITCOMPATMODE)
		 || (!pChn->nLength) || ((pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol))
		 //IT compatibility tentative fix: Reset envelopes when instrument changes.
		 || (IsCompatibleMode(TRK_IMPULSETRACKER) && bInstrumentChanged))
		{
			pChn->dwFlags |= CHN_FASTVOLRAMP;
			if ((m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT)) && (!bInstrumentChanged) && (pIns) && (!(pChn->dwFlags & (CHN_KEYOFF|CHN_NOTEFADE))))
			{
				if (!(pIns->VolEnv.dwFlags & ENV_CARRY)) resetEnvelopes(pChn, ENV_RESET_VOL);
				if (!(pIns->PanEnv.dwFlags & ENV_CARRY)) resetEnvelopes(pChn, ENV_RESET_PAN);
				if (!(pIns->PitchEnv.dwFlags & ENV_CARRY)) resetEnvelopes(pChn, ENV_RESET_PITCH);
			} else {
				resetEnvelopes(pChn);
			}
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		} else if ((pIns) && (!(pIns->VolEnv.dwFlags & ENV_ENABLED)))
		{
			resetEnvelopes(pChn);		
		}
	}
	// Invalid sample ?
	if (!pSmp)
	{
		pChn->pModSample = nullptr;
		pChn->nInsVol = 0;
		return;
	}

	// Tone-Portamento doesn't reset the pingpong direction flag
	if ((bPorta) && (pSmp == pChn->pModSample))
	{
		if(m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT)) return;
		pChn->dwFlags &= ~(CHN_KEYOFF|CHN_NOTEFADE);
		pChn->dwFlags = (pChn->dwFlags & (CHN_CHANNELFLAGS | CHN_PINGPONGFLAG)) | (pSmp->uFlags & CHN_SAMPLEFLAGS);
	} else
	{
		pChn->dwFlags &= ~(CHN_KEYOFF|CHN_NOTEFADE|CHN_VOLENV|CHN_PANENV|CHN_PITCHENV);

		//IT compatibility tentative fix: Don't change bidi loop direction when 
		//no sample nor instrument is changed.
		if(IsCompatibleMode(TRK_IMPULSETRACKER) && pSmp == pChn->pModSample && !bInstrumentChanged)
			pChn->dwFlags = (pChn->dwFlags & (CHN_CHANNELFLAGS | CHN_PINGPONGFLAG)) | (pSmp->uFlags & CHN_SAMPLEFLAGS);
		else
			pChn->dwFlags = (pChn->dwFlags & CHN_CHANNELFLAGS) | (pSmp->uFlags & CHN_SAMPLEFLAGS);


		if (pIns)
		{
			if (pIns->VolEnv.dwFlags & ENV_ENABLED) pChn->dwFlags |= CHN_VOLENV;
			if (pIns->PanEnv.dwFlags & ENV_ENABLED) pChn->dwFlags |= CHN_PANENV;
			if (pIns->PitchEnv.dwFlags & ENV_ENABLED) pChn->dwFlags |= CHN_PITCHENV;
			if ((pIns->PitchEnv.dwFlags & ENV_ENABLED) && (pIns->PitchEnv.dwFlags & ENV_FILTER))
			{
				pChn->dwFlags |= CHN_FILTERENV;
				if (!pChn->nCutOff) pChn->nCutOff = 0x7F;
			} else
			{
				// Special case: Reset filter envelope flag manually (because of S7D/S7E effects).
				// This way, the S7x effects can be applied to several notes, as long as they don't come with an instrument number.
				pChn->dwFlags &= ~CHN_FILTERENV;
			}
			if (pIns->nIFC & 0x80) pChn->nCutOff = pIns->nIFC & 0x7F;
			if (pIns->nIFR & 0x80) pChn->nResonance = pIns->nIFR & 0x7F;
		}
		pChn->nVolSwing = pChn->nPanSwing = 0;
		pChn->nResSwing = pChn->nCutSwing = 0;
	}
	pChn->pModSample = pSmp;
	pChn->nLength = pSmp->nLength;
	pChn->nLoopStart = pSmp->nLoopStart;
	pChn->nLoopEnd = pSmp->nLoopEnd;

	if(bNewTuning)
	{
		pChn->nC5Speed = pSmp->nC5Speed;
		pChn->m_CalculateFreq = true;
		pChn->nFineTune = 0;
	}
	else
	{
		pChn->nC5Speed = pSmp->nC5Speed;
		pChn->nFineTune = pSmp->nFineTune;
	}

	
	
	pChn->pSample = pSmp->pSample;
	pChn->nTranspose = pSmp->RelativeTone;

	pChn->m_PortamentoFineSteps = 0;
	pChn->nPortamentoDest = 0;

	if (pChn->dwFlags & CHN_SUSTAINLOOP)
	{
		pChn->nLoopStart = pSmp->nSustainStart;
		pChn->nLoopEnd = pSmp->nSustainEnd;
		pChn->dwFlags |= CHN_LOOP;
		if (pChn->dwFlags & CHN_PINGPONGSUSTAIN) pChn->dwFlags |= CHN_PINGPONGLOOP;
	}
	if ((pChn->dwFlags & CHN_LOOP) && (pChn->nLoopEnd < pChn->nLength)) pChn->nLength = pChn->nLoopEnd;
}


void CSoundFile::NoteChange(UINT nChn, int note, bool bPorta, bool bResetEnv, bool bManual)
//-----------------------------------------------------------------------------------------
{
	if (note < 1) return;
	MODCHANNEL * const pChn = &Chn[nChn];
	MODSAMPLE *pSmp = pChn->pModSample;
	MODINSTRUMENT *pIns = pChn->pModInstrument;

	const bool bNewTuning = (m_nType == MOD_TYPE_MPT && pIns && pIns->pTuning);

	if ((pIns) && (note <= 0x80))
	{
		UINT n = pIns->Keyboard[note - 1];
		if ((n) && (n < MAX_SAMPLES)) pSmp = GetSample(n);
		note = pIns->NoteMap[note-1];
	}
	// Key Off
	if (note > NOTE_MAX)
	{
		// Key Off (+ Invalid Note for XM - TODO is this correct?)
		if (note == NOTE_KEYOFF || !(m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT)))
			KeyOff(nChn);
		else // Invalid Note -> Note Fade
		//if (note == NOTE_FADE)
			if(m_nInstruments)	
				pChn->dwFlags |= CHN_NOTEFADE;

		// Note Cut
		if (note == NOTE_NOTECUT)
		{
			pChn->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
			if ((!(m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT))) || (m_nInstruments)) pChn->nVolume = 0;
			pChn->nFadeOutVol = 0;
		}

		//IT compatibility tentative fix: Clear channel note memory.
		if(IsCompatibleMode(TRK_IMPULSETRACKER))
		{
			pChn->nNote = NOTE_NONE;
			pChn->nNewNote = NOTE_NONE;
		}
		return;
	}

	if(bNewTuning)
	{
		if(!bPorta || pChn->nNote == NOTE_NONE)
			pChn->nPortamentoDest = 0;
		else
		{
			pChn->nPortamentoDest = pIns->pTuning->GetStepDistance(pChn->nNote, pChn->m_PortamentoFineSteps, note, 0);
			//Here pCnh->nPortamentoDest means 'steps to slide'.
			pChn->m_PortamentoFineSteps = -pChn->nPortamentoDest;
		}
	}

	if ((!bPorta) && (m_nType & (MOD_TYPE_XM|MOD_TYPE_MED|MOD_TYPE_MT2))) {
		if (pSmp) {
			pChn->nTranspose = pSmp->RelativeTone;
			pChn->nFineTune = pSmp->nFineTune;
		}
	}
	// IT Compatibility: Update multisample instruments frequency even if instrument is not specified
	if(!bPorta && pSmp && IsCompatibleMode(TRK_IMPULSETRACKER)) pChn->nC5Speed = pSmp->nC5Speed;

	// XM Compatibility: Ignore notes with portamento if there was no note
	if(bPorta && (pChn->nPeriod == 0) && IsCompatibleMode(TRK_FASTTRACKER2))
		return;

	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2|MOD_TYPE_MED)) note += pChn->nTranspose;
	note = CLAMP(note, 1, 132);
	pChn->nNote = note;
	pChn->m_CalculateFreq = true;

	if ((!bPorta) || (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT)))
		pChn->nNewIns = 0;

	UINT period = GetPeriodFromNote(note, pChn->nFineTune, pChn->nC5Speed);

	if (!pSmp) return;
	if (period)
	{
		if ((!bPorta) || (!pChn->nPeriod)) pChn->nPeriod = period;
		if(!bNewTuning) pChn->nPortamentoDest = period;
		if ((!bPorta) || ((!pChn->nLength) && (!(m_nType & MOD_TYPE_S3M))))
		{
			pChn->pModSample = pSmp;
			pChn->pSample = pSmp->pSample;
			pChn->nLength = pSmp->nLength;
			pChn->nLoopEnd = pSmp->nLength;
			pChn->nLoopStart = 0;
			pChn->dwFlags = (pChn->dwFlags & CHN_CHANNELFLAGS) | (pSmp->uFlags & CHN_SAMPLEFLAGS);
			if (pChn->dwFlags & CHN_SUSTAINLOOP)
			{
				pChn->nLoopStart = pSmp->nSustainStart;
				pChn->nLoopEnd = pSmp->nSustainEnd;
				pChn->dwFlags &= ~CHN_PINGPONGLOOP;
				pChn->dwFlags |= CHN_LOOP;
				if (pChn->dwFlags & CHN_PINGPONGSUSTAIN) pChn->dwFlags |= CHN_PINGPONGLOOP;
				if (pChn->nLength > pChn->nLoopEnd) pChn->nLength = pChn->nLoopEnd;
			} else
			if (pChn->dwFlags & CHN_LOOP)
			{
				pChn->nLoopStart = pSmp->nLoopStart;
				pChn->nLoopEnd = pSmp->nLoopEnd;
				if (pChn->nLength > pChn->nLoopEnd) pChn->nLength = pChn->nLoopEnd;
			}
			pChn->nPos = 0;
			pChn->nPosLo = 0;
			// Handle "retrigger" waveform type
			if (pChn->nVibratoType < 4)
			{
				if(!IsCompatibleMode(TRK_IMPULSETRACKER) && (GetType() & (MOD_TYPE_IT|MOD_TYPE_MPT)) && (!(m_dwSongFlags & SONG_ITOLDEFFECTS)))
					pChn->nVibratoPos = 0x10;
				else
					pChn->nVibratoPos = 0;
			}
			if(!IsCompatibleMode(TRK_IMPULSETRACKER) && pChn->nTremoloType < 4)
			{
				pChn->nTremoloPos = 0;
			}
		}
		if (pChn->nPos >= pChn->nLength) pChn->nPos = pChn->nLoopStart;
	} 
	else 
		bPorta = false;

	if ((!bPorta) || (!(m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT)))
	 || ((pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol))
	 || ((m_dwSongFlags & SONG_ITCOMPATMODE) && (pChn->nRowInstr)))
	{
		if ((m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT)) && (pChn->dwFlags & CHN_NOTEFADE) && (!pChn->nFadeOutVol))
		{
			resetEnvelopes(pChn);
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
			pChn->dwFlags &= ~CHN_NOTEFADE;
			pChn->nFadeOutVol = 65536;
		}
		if ((!bPorta) || (!(m_dwSongFlags & SONG_ITCOMPATMODE)) || (pChn->nRowInstr))
		{
			if ((!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) || (pChn->nRowInstr))
			{
				pChn->dwFlags &= ~CHN_NOTEFADE;
				pChn->nFadeOutVol = 65536;
			}
		}
	}
	pChn->dwFlags &= ~(CHN_EXTRALOUD|CHN_KEYOFF);
	// Enable Ramping
	if (!bPorta)
	{
		pChn->nVUMeter = 0x100;
		pChn->nLeftVU = pChn->nRightVU = 0xFF;
		pChn->dwFlags &= ~CHN_FILTER;
		pChn->dwFlags |= CHN_FASTVOLRAMP;
		if(!IsCompatibleMode(TRK_IMPULSETRACKER))
		{
			//IT compatibility 15. Retrigger will not be reset (Tremor doesn't store anything here, so we just don't reset this as well)
			if(!IsCompatibleMode(TRK_FASTTRACKER2)) pChn->nRetrigCount = 0;
			pChn->nTremorCount = 0;
		}
		if (bResetEnv)
		{
			pChn->nVolSwing = pChn->nPanSwing = 0;
			pChn->nResSwing = pChn->nCutSwing = 0;
			if (pIns)
			{
				if(IsCompatibleMode(TRK_IMPULSETRACKER)) pChn->nNNA = pIns->nNNA;
				if (!(pIns->VolEnv.dwFlags & ENV_CARRY)) pChn->nVolEnvPosition = 0;
				if (!(pIns->PanEnv.dwFlags & ENV_CARRY)) pChn->nPanEnvPosition = 0;
				if (!(pIns->PitchEnv.dwFlags & ENV_CARRY)) pChn->nPitchEnvPosition = 0;
				if (m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT))
				{
					// Volume Swing
					if (pIns->nVolSwing)
					{
						if(IsCompatibleMode(TRK_IMPULSETRACKER))
						{
							double d = 2 * (((double) rand()) / RAND_MAX) - 1;
							pChn->nVolSwing = d * pIns->nVolSwing / 100.0 * pChn->nVolume;
						} else
						{
							int d = ((LONG)pIns->nVolSwing * (LONG)((rand() & 0xFF) - 0x7F)) / 128;
							pChn->nVolSwing = (signed short)((d * pChn->nVolume + 1)/128);
						}
					}
					// Pan Swing
					if (pIns->nPanSwing)
					{
						if(IsCompatibleMode(TRK_IMPULSETRACKER))
						{
							double d = 2 * (((double) rand()) / RAND_MAX) - 1;
							pChn->nPanSwing = d * pIns->nPanSwing * 4;
						} else
						{
							int d = ((LONG)pIns->nPanSwing * (LONG)((rand() & 0xFF) - 0x7F)) / 128;
							pChn->nPanSwing = (signed short)d;
							pChn->nRestorePanOnNewNote = pChn->nPan + 1;
						}
					}
					// Cutoff Swing
					if (pIns->nCutSwing)
					{
						int d = ((LONG)pIns->nCutSwing * (LONG)((rand() & 0xFF) - 0x7F)) / 128;
						pChn->nCutSwing = (signed short)((d * pChn->nCutOff + 1)/128);
						pChn->nRestoreCutoffOnNewNote = pChn->nCutOff + 1;
					}
					// Resonance Swing
					if (pIns->nResSwing)
					{
						int d = ((LONG)pIns->nResSwing * (LONG)((rand() & 0xFF) - 0x7F)) / 128;
						pChn->nResSwing = (signed short)((d * pChn->nResonance + 1)/128);
						pChn->nRestoreResonanceOnNewNote = pChn->nResonance + 1;
					}
				}
			}
			pChn->nAutoVibDepth = 0;
			pChn->nAutoVibPos = 0;
		}
		pChn->nLeftVol = pChn->nRightVol = 0;
		bool bFlt = (m_dwSongFlags & SONG_MPTFILTERMODE) ? false : true;
		// Setup Initial Filter for this note
		if (pIns)
		{
			if (pIns->nIFR & 0x80) { pChn->nResonance = pIns->nIFR & 0x7F; bFlt = true; }
			if (pIns->nIFC & 0x80) { pChn->nCutOff = pIns->nIFC & 0x7F; bFlt = true; }
			if (bFlt && (pIns->nFilterMode != FLTMODE_UNCHANGED)) {
				pChn->nFilterMode = pIns->nFilterMode;
			}
		} else
		{
			pChn->nVolSwing = pChn->nPanSwing = 0;
			pChn->nCutSwing = pChn->nResSwing = 0;
		}
#ifndef NO_FILTER
		if ((pChn->nCutOff < 0x7F) && (bFlt)) SetupChannelFilter(pChn, true);
#endif // NO_FILTER
	}
	// Special case for MPT
	if (bManual) pChn->dwFlags &= ~CHN_MUTE;
	if (((pChn->dwFlags & CHN_MUTE) && (gdwSoundSetup & SNDMIX_MUTECHNMODE))
	 || ((pChn->pModSample) && (pChn->pModSample->uFlags & CHN_MUTE) && (!bManual))
	 || ((pChn->pModInstrument) && (pChn->pModInstrument->dwFlags & INS_MUTE) && (!bManual)))
	{
		if (!bManual) pChn->nPeriod = 0;
	}

}


UINT CSoundFile::GetNNAChannel(UINT nChn) const
//---------------------------------------------
{
	const MODCHANNEL *pChn = &Chn[nChn];
	// Check for empty channel
	const MODCHANNEL *pi = &Chn[m_nChannels];
	for (UINT i=m_nChannels; i<MAX_CHANNELS; i++, pi++) if (!pi->nLength) return i;
	if (!pChn->nFadeOutVol) return 0;
	// All channels are used: check for lowest volume
	UINT result = 0;
	DWORD vol = 64*65536;	// 25%
	DWORD envpos = 0xFFFFFF;
	const MODCHANNEL *pj = &Chn[m_nChannels];
	for (UINT j=m_nChannels; j<MAX_CHANNELS; j++, pj++)
	{
		if (!pj->nFadeOutVol) return j;
		DWORD v = pj->nVolume;
		if (pj->dwFlags & CHN_NOTEFADE)
			v = v * pj->nFadeOutVol;
		else
			v <<= 16;
		if (pj->dwFlags & CHN_LOOP) v >>= 1;
		if ((v < vol) || ((v == vol) && (pj->nVolEnvPosition > envpos)))
		{
			envpos = pj->nVolEnvPosition;
			vol = v;
			result = j;
		}
	}
	return result;
}


void CSoundFile::CheckNNA(UINT nChn, UINT instr, int note, BOOL bForceCut)
//------------------------------------------------------------------------
{
	MODCHANNEL *pChn = &Chn[nChn];
	MODINSTRUMENT* pHeader = 0;
	LPSTR pSample;
	if (note > 0x80) note = NOTE_NONE;
	if (note < 1) return;
	// Always NNA cut - using
	if ((!(m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT|MOD_TYPE_MT2))) || (!m_nInstruments) || (bForceCut))
	{
		if ((m_dwSongFlags & SONG_CPUVERYHIGH)
		 || (!pChn->nLength) || (pChn->dwFlags & CHN_MUTE)
		 || ((!pChn->nLeftVol) && (!pChn->nRightVol))) return;
		UINT n = GetNNAChannel(nChn);
		if (!n) return;
		MODCHANNEL *p = &Chn[n];
		// Copy Channel
		*p = *pChn;
		p->dwFlags &= ~(CHN_VIBRATO|CHN_TREMOLO|CHN_PANBRELLO|CHN_MUTE|CHN_PORTAMENTO);
		p->nMasterChn = nChn+1;
		p->nCommand = 0;
		// Cut the note
		p->nFadeOutVol = 0;
		p->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
		// Stop this channel
		pChn->nLength = pChn->nPos = pChn->nPosLo = 0;
		pChn->nROfs = pChn->nLOfs = 0;
		pChn->nLeftVol = pChn->nRightVol = 0;
		return;
	}
	if (instr >= MAX_INSTRUMENTS) instr = 0;
	pSample = pChn->pSample;
	pHeader = pChn->pModInstrument;
	if ((instr) && (note))
	{
		pHeader = GetInstrument(instr);
		if (pHeader)
		{
			UINT n = 0;
			if (note <= 0x80)
			{
				n = pHeader->Keyboard[note-1];
				note = pHeader->NoteMap[note-1];
				if ((n) && (n < MAX_SAMPLES)) pSample = GetSample(n)->pSample;
			}
		} else pSample = nullptr;
	}
	MODCHANNEL *p = pChn;
	//if (!pIns) return;
	if (pChn->dwFlags & CHN_MUTE) return;

	bool applyDNAtoPlug;	//rewbs.VSTiNNA
	for (UINT i=nChn; i<MAX_CHANNELS; p++, i++)
	if ((i >= m_nChannels) || (p == pChn))
	{
		applyDNAtoPlug = false; //rewbs.VSTiNNA
		if (((p->nMasterChn == nChn+1) || (p == pChn)) && (p->pModInstrument))
		{
			BOOL bOk = FALSE;
			// Duplicate Check Type
			switch(p->pModInstrument->nDCT)
			{
			// Note
			case DCT_NOTE:
				if ((note) && (p->nNote == note) && (pHeader == p->pModInstrument)) bOk = TRUE;
				if (pHeader && pHeader->nMixPlug) applyDNAtoPlug = true; //rewbs.VSTiNNA
				break;
			// Sample
			case DCT_SAMPLE:
				if ((pSample) && (pSample == p->pSample)) bOk = TRUE;
				break;
			// Instrument
			case DCT_INSTRUMENT:
				if (pHeader == p->pModInstrument) bOk = TRUE;
				//rewbs.VSTiNNA
				if (pHeader && pHeader->nMixPlug) applyDNAtoPlug = true;
				break;
			// Plugin
			case DCT_PLUGIN:
				if (pHeader && (pHeader->nMixPlug) && (pHeader->nMixPlug == p->pModInstrument->nMixPlug))
				{
					applyDNAtoPlug = true;
					bOk = TRUE;
				}
				//end rewbs.VSTiNNA
				break;

			}
			// Duplicate Note Action
			if (bOk)
			{
				//rewbs.VSTiNNA
				if (applyDNAtoPlug)
				{
					switch(p->pModInstrument->nDNA)
					{
					case DNA_NOTECUT:
					case DNA_NOTEOFF:
					case DNA_NOTEFADE:	
						//switch off duplicated note played on this plugin 
						break;
					}
				}
				//end rewbs.VSTiNNA

				switch(p->pModInstrument->nDNA)
				{
				// Cut
				case DNA_NOTECUT:
					KeyOff(i);
					p->nVolume = 0;
					break;
				// Note Off
				case DNA_NOTEOFF:
					KeyOff(i);
					break;
				// Note Fade
				case DNA_NOTEFADE:
					p->dwFlags |= CHN_NOTEFADE;
					break;
				}
				if (!p->nVolume)
				{
					p->nFadeOutVol = 0;
					p->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
				}
			}
		}
	}

	// New Note Action
	//if ((pChn->nVolume) && (pChn->nLength))
	if (((pChn->nVolume) && (pChn->nLength))/* || applyNNAtoPlug*/) //rewbs.VSTiNNA
	{
		UINT n = GetNNAChannel(nChn);
		if (n)
		{
			MODCHANNEL *p = &Chn[n];
			// Copy Channel
			*p = *pChn;
			p->dwFlags &= ~(CHN_VIBRATO|CHN_TREMOLO|CHN_PANBRELLO|CHN_MUTE|CHN_PORTAMENTO);
			
			//rewbs: Copy mute and FX status from master chan.
			//I'd like to copy other flags too, but this would change playback behaviour.
			p->dwFlags |= (pChn->dwFlags & CHN_MUTE) | (pChn->dwFlags & CHN_NOFX);

			p->nMasterChn = nChn+1;
			p->nCommand = 0;
			//rewbs.VSTiNNA	

			//end rewbs.VSTiNNA	
			// Key Off the note
			switch(pChn->nNNA)
			{
			case NNA_NOTEOFF:	KeyOff(n); break;
			case NNA_NOTECUT:
				p->nFadeOutVol = 0;
			case NNA_NOTEFADE:	p->dwFlags |= CHN_NOTEFADE; break;
			}
			if (!p->nVolume)
			{
				p->nFadeOutVol = 0;
				p->dwFlags |= (CHN_NOTEFADE|CHN_FASTVOLRAMP);
			}
			// Stop this channel
			pChn->nLength = pChn->nPos = pChn->nPosLo = 0;
			pChn->nROfs = pChn->nLOfs = 0;
		}
	}
}


BOOL CSoundFile::ProcessEffects()
//-------------------------------
{
	MODCHANNEL *pChn = Chn;
	int nBreakRow = -1, nPosJump = -1, nPatLoopRow = -1;

// -> CODE#0010
// -> DESC="add extended parameter mechanism to pattern effects"
	MODCOMMAND* m = nullptr;
// -! NEW_FEATURE#0010
	for (UINT nChn=0; nChn<m_nChannels; nChn++, pChn++)
	{
		UINT instr = pChn->nRowInstr;
		UINT volcmd = pChn->nRowVolCmd;
		UINT vol = pChn->nRowVolume;
		UINT cmd = pChn->nRowCommand;
		UINT param = pChn->nRowParam;
		bool bPorta = ((cmd != CMD_TONEPORTAMENTO) && (cmd != CMD_TONEPORTAVOL) && (volcmd != VOLCMD_TONEPORTAMENTO)) ? FALSE : TRUE;

		UINT nStartTick = 0;

		pChn->dwFlags &= ~CHN_FASTVOLRAMP;

		// Process continuous parameter control note.
		// Row data is cleared after first tick so on following
		// ticks using channels m_nPlugParamValueStep to identify
		// the need for parameter control. The condition cmd == 0
		// is to make sure that m_nPlugParamValueStep != 0 because 
		// of NOTE_PCS, not because of macro.

		// Apart from changing parameters, parameter control notes are intended to be 'invisible'.
		// To achieve this, clearing the note data so that rest of the process sees the row as empty row.
		if(pChn->nRowNote == NOTE_PC || pChn->nRowNote == NOTE_PCS)
		{
			pChn->ClearRowCmd();
			instr = 0;
			volcmd = 0;
			vol = 0;
			cmd = 0;
			param = 0;
			bPorta = false;
		}

		// Process Invert Loop (MOD Effect, called every row)
		if((m_dwSongFlags & SONG_FIRSTTICK) == 0) InvertLoop(&Chn[nChn]);

		// Process special effects (note delay, pattern delay, pattern loop)
		if ((cmd == CMD_MODCMDEX) || (cmd == CMD_S3MCMDEX))
		{
			if ((!param) && (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT))) param = pChn->nOldCmdEx; else pChn->nOldCmdEx = param;
			// Note Delay ?
			if ((param & 0xF0) == 0xD0)
			{
				nStartTick = param & 0x0F;
				if(nStartTick == 0)
				{
					//IT compatibility 22. SD0 == SD1
					if(IsCompatibleMode(TRK_IMPULSETRACKER))
						nStartTick = 1;
					//ST3 ignores notes with SD0 completely
					else if(GetType() == MOD_TYPE_S3M)
						nStartTick = m_nMusicSpeed;
				}

				//IT compatibility 08. Handling of out-of-range delay command.
				if(nStartTick >= m_nMusicSpeed && IsCompatibleMode(TRK_IMPULSETRACKER))
				{
					if(instr)
					{
						if(GetNumInstruments() < 1 && instr < MAX_SAMPLES)
						{
							pChn->pModSample = GetSample(instr);
						}
						else
						{	
							if(instr < MAX_INSTRUMENTS)
								pChn->pModInstrument = GetInstrument(instr);
						}
					}
					continue;
				}
			} else
			if(m_dwSongFlags & SONG_FIRSTTICK)
			{
				// Pattern Loop ?
				if ((((param & 0xF0) == 0x60) && (cmd == CMD_MODCMDEX))
				 || (((param & 0xF0) == 0xB0) && (cmd == CMD_S3MCMDEX)))
				{
					//int nloop = PatternLoop(pChn, param & 0x0F);
					//if (nloop >= 0) nPatLoopRow = nloop;
				} else
				// Pattern Delay
				if ((param & 0xF0) == 0xE0)
				{
					m_nPatternDelay = param & 0x0F;
				}
			}
		}
		
		// Handles note/instrument/volume changes
		if (m_nTickCount == nStartTick) // can be delayed by a note delay effect
		{
			UINT note = pChn->nRowNote;
			if (instr) pChn->nNewIns = instr;
			if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
			{
				// XM: FT2 ignores a note next to a K00 effect, and a fade-out seems to be done when no volume envelope is present (not exactly the Kxx behaviour)
				if(cmd == CMD_KEYOFF && param == 0 && IsCompatibleMode(TRK_FASTTRACKER2))
				{
					note = instr = 0;
				}

				// XM: Key-Off + Sample == Note Cut (BUT: Only if no instr number or volume effect is present!)
				if ((note == NOTE_KEYOFF) && ((!instr && !vol && cmd != CMD_VOLUME) || !IsCompatibleMode(TRK_FASTTRACKER2)) && ((!pChn->pModInstrument) || (!(pChn->pModInstrument->VolEnv.dwFlags & ENV_ENABLED))))
				{
					pChn->dwFlags |= CHN_FASTVOLRAMP;
					pChn->nVolume = 0;
					note = instr = 0;
				}

				// XM: Rogue note delays cause retrig
				if ((note == NOTE_NONE) && IsCompatibleMode(TRK_FASTTRACKER2) && !(m_dwSongFlags & SONG_FIRSTTICK))
				{
					note = pChn->nNote - pChn->nTranspose;
				}
			}
			if ((!note) && (instr)) //Case: instrument with no note data. 
			{
				//IT compatibility: Instrument with no note.
				if(IsCompatibleMode(TRK_IMPULSETRACKER) || (m_dwSongFlags & SONG_PT1XMODE))
				{
					if(m_nInstruments)
					{
						if(instr < MAX_INSTRUMENTS && pChn->pModInstrument != GetInstrument(instr))
							note = pChn->nNote;
					}
					else //Case: Only samples used
					{
						if(instr < MAX_SAMPLES && pChn->pSample != GetSample(instr)->pSample)
							note = pChn->nNote;
					}
				}

				if (m_nInstruments)
				{
					if (pChn->pModSample) pChn->nVolume = pChn->pModSample->nVolume;
					if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
					{
						pChn->dwFlags |= CHN_FASTVOLRAMP;
						resetEnvelopes(pChn);	
						pChn->nAutoVibDepth = 0;
						pChn->nAutoVibPos = 0;
						pChn->dwFlags &= ~CHN_NOTEFADE;
						pChn->nFadeOutVol = 65536;
					}
				} else //Case: Only samples are used; no instruments.
				{
					if (instr < MAX_SAMPLES) pChn->nVolume = GetSample(instr)->nVolume;
				}
				if (!(m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT))) instr = 0;
			}
			// Invalid Instrument ?
			if (instr >= MAX_INSTRUMENTS) instr = 0;

			// Note Cut/Off/Fade => ignore instrument
			if (note >= NOTE_MIN_SPECIAL) instr = 0;

			if ((note) && (note <= NOTE_MAX)) pChn->nNewNote = note;

			// New Note Action ?
			if ((note) && (note <= NOTE_MAX) && (!bPorta))
			{
				CheckNNA(nChn, instr, note, FALSE);
			}

		//rewbs.VSTnoteDelay
		#ifdef MODPLUG_TRACKER
//			if (m_nInstruments) ProcessMidiOut(nChn, pChn); 
		#endif // MODPLUG_TRACKER
		//end rewbs.VSTnoteDelay

			if(note)
			{
				if(pChn->nRestorePanOnNewNote > 0)
				{
					pChn->nPan = pChn->nRestorePanOnNewNote - 1;
					pChn->nRestorePanOnNewNote = 0;
				}
				if(pChn->nRestoreResonanceOnNewNote > 0)
				{
					pChn->nResonance = pChn->nRestoreResonanceOnNewNote - 1;
					pChn->nRestoreResonanceOnNewNote = 0;
				}
				if(pChn->nRestoreCutoffOnNewNote > 0)
				{
					pChn->nCutOff = pChn->nRestoreCutoffOnNewNote - 1;
					pChn->nRestoreCutoffOnNewNote = 0;
				}
				
			}

			// Instrument Change ?
			if (instr)
			{
				MODSAMPLE *psmp = pChn->pModSample;
				InstrumentChange(pChn, instr, bPorta, TRUE);
				pChn->nNewIns = 0;
				// Special IT case: portamento+note causes sample change -> ignore portamento
				if ((m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT))
				 && (psmp != pChn->pModSample) && (note) && (note < 0x80))
				{
					bPorta = FALSE;
				}
			}
			// New Note ?
			if (note)
			{
				if ((!instr) && (pChn->nNewIns) && (note < 0x80))
				{
					InstrumentChange(pChn, pChn->nNewIns, bPorta, FALSE, (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) ? FALSE : TRUE);
					pChn->nNewIns = 0;
				}
				NoteChange(nChn, note, bPorta, (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) ? false : true);
				if ((bPorta) && (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) && (instr))
				{
					pChn->dwFlags |= CHN_FASTVOLRAMP;
					resetEnvelopes(pChn);
					pChn->nAutoVibDepth = 0;
					pChn->nAutoVibPos = 0;
				}
			}
			// Tick-0 only volume commands
			if (volcmd == VOLCMD_VOLUME)
			{
				if (vol > 64) vol = 64;
				pChn->nVolume = vol << 2;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			} else
			if (volcmd == VOLCMD_PANNING)
			{
				if (vol > 64) vol = 64;
				pChn->nPan = vol << 2;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
				pChn->nRestorePanOnNewNote = 0;
				//IT compatibility 20. Set pan overrides random pan
				if(IsCompatibleMode(TRK_IMPULSETRACKER))
					pChn->nPanSwing = 0;
			}

		//rewbs.VSTnoteDelay
		#ifdef MODPLUG_TRACKER
			if (m_nInstruments) ProcessMidiOut(nChn, pChn); 
		#endif // MODPLUG_TRACKER
		}


		// Volume Column Effect (except volume & panning)
		/*	A few notes, paraphrased from ITTECH.TXT by Storlek (creator of schismtracker):
			Ex/Fx/Gx are shared with Exx/Fxx/Gxx; Ex/Fx are 4x the 'normal' slide value
			Gx is linked with Ex/Fx if Compat Gxx is off, just like Gxx is with Exx/Fxx
			Gx values: 1, 4, 8, 16, 32, 64, 96, 128, 255
			Ax/Bx/Cx/Dx values are used directly (i.e. D9 == D09), and are NOT shared with Dxx
			(value is stored into nOldVolParam and used by A0/B0/C0/D0)
			Hx uses the same value as Hxx and Uxx, and affects the *depth*
			so... hxx = (hx | (oldhxx & 0xf0))  ???
			TODO is this done correctly?
		*/
      	if ((volcmd > VOLCMD_PANNING) && (m_nTickCount >= nStartTick))
		{
			if (volcmd == VOLCMD_TONEPORTAMENTO)
			{
				if (m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT))
					TonePortamento(pChn, ImpulseTrackerPortaVolCmd[vol & 0x0F]);
				else
					TonePortamento(pChn, vol * 16);
			} else
			{
				if (vol) pChn->nOldVolParam = vol; else vol = pChn->nOldVolParam;
				switch(volcmd)
				{
				case VOLCMD_VOLSLIDEUP:
					VolumeSlide(pChn, vol << 4);
					break;

				case VOLCMD_VOLSLIDEDOWN:
					VolumeSlide(pChn, vol);
					break;

				case VOLCMD_FINEVOLUP:
					if (m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT))
					{
						if (m_nTickCount == nStartTick) VolumeSlide(pChn, (vol << 4) | 0x0F);
					} else
						FineVolumeUp(pChn, vol);
					break;

				case VOLCMD_FINEVOLDOWN:
					if (m_nType & (MOD_TYPE_IT | MOD_TYPE_MPT))
					{
						if (m_nTickCount == nStartTick) VolumeSlide(pChn, 0xF0 | vol);
					} else
						FineVolumeDown(pChn, vol);
					break;

				case VOLCMD_VIBRATOSPEED:
					if(IsCompatibleMode(TRK_FASTTRACKER2))
						pChn->nVibratoSpeed = vol & 0x0F;
					else
						Vibrato(pChn, vol << 4);
					break;

				case VOLCMD_VIBRATODEPTH:
					Vibrato(pChn, vol);
					break;

				case VOLCMD_PANSLIDELEFT:
					PanningSlide(pChn, vol);
					break;

				case VOLCMD_PANSLIDERIGHT:
					PanningSlide(pChn, vol << 4);
					break;

				case VOLCMD_PORTAUP:
					//IT compatibility (one of the first - link effect memory)
					if(IsCompatibleMode(TRK_IMPULSETRACKER))
						PortamentoUp(pChn, vol << 2, true);
					else
						PortamentoUp(pChn, vol << 2, false);
					break;

				case VOLCMD_PORTADOWN:
					//IT compatibility (one of the first - link effect memory)
					if(IsCompatibleMode(TRK_IMPULSETRACKER))
						PortamentoDown(pChn, vol << 2, true);
					else
						PortamentoDown(pChn, vol << 2, false);
					break;
				
				case VOLCMD_VELOCITY:					//rewbs.velocity	TEMP algorithm (crappy :)
					pChn->nVolume = vol * 28;			//Max nVolume is 255; max vol is 9; 255/9=28
					pChn->dwFlags |= CHN_FASTVOLRAMP;
					if (m_nTickCount == nStartTick) SampleOffset(nChn, 48-(vol << 3), bPorta); //Max vol is 9; 9 << 3 = 48
					break;
							
				case VOLCMD_OFFSET:					//rewbs.volOff
					if (m_nTickCount == nStartTick) 
						SampleOffset(nChn, vol<<3, bPorta);
					break;
				}
			}
		}

		// Effects
		if (cmd) switch (cmd)
		{
// -> CODE#0010
// -> DESC="add extended parameter mechanism to pattern effects"
		case CMD_XPARAM:
			break;
// -> NEW_FEATURE#0010
		// Set Volume
		case CMD_VOLUME:
			if(m_dwSongFlags & SONG_FIRSTTICK)
			{
				pChn->nVolume = (param < 64) ? param*4 : 256;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
			break;

		// Portamento Up
		case CMD_PORTAMENTOUP:
			if ((!param) && (m_nType & MOD_TYPE_MOD)) break;
			PortamentoUp(pChn, param);
			break;

		// Portamento Down
		case CMD_PORTAMENTODOWN:
			if ((!param) && (m_nType & MOD_TYPE_MOD)) break;
			PortamentoDown(pChn, param);
			break;

		// Volume Slide
		case CMD_VOLUMESLIDE:
			if ((param) || (m_nType != MOD_TYPE_MOD)) VolumeSlide(pChn, param);
			break;

		// Tone-Portamento
		case CMD_TONEPORTAMENTO:
			TonePortamento(pChn, param);
			break;

		// Tone-Portamento + Volume Slide
		case CMD_TONEPORTAVOL:
			if ((param) || (m_nType != MOD_TYPE_MOD)) VolumeSlide(pChn, param);
			TonePortamento(pChn, 0);
			break;

		// Vibrato
		case CMD_VIBRATO:
			Vibrato(pChn, param);
			break;

		// Vibrato + Volume Slide
		case CMD_VIBRATOVOL:
			if ((param) || (m_nType != MOD_TYPE_MOD)) VolumeSlide(pChn, param);
			Vibrato(pChn, 0);
			break;

		// Set Speed
		case CMD_SPEED:
			if(m_dwSongFlags & SONG_FIRSTTICK)
				SetSpeed(param);
			break;

		// Set Tempo
		case CMD_TEMPO:
// -> CODE#0010
// -> DESC="add extended parameter mechanism to pattern effects"
				/*m = NULL;
				if (m_nRow < PatternSize[m_nPattern]-1) {
					m = Patterns[m_nPattern] + (m_nRow+1) * m_nChannels + nChn;
				}
				if (m && m->command == CMD_XPARAM) { 
					if (m_nType & MOD_TYPE_XM) {
                        param -= 0x20; //with XM, 0x20 is the lowest tempo. Anything below changes ticks per row.
					}
					param = (param<<8) + m->param;
				}*/
// -! NEW_FEATURE#0010
				if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT))
				{
					if (param) pChn->nOldTempo = param; else param = pChn->nOldTempo;
				}
				if (param > GetModSpecifications().tempoMax) param = GetModSpecifications().tempoMax; // rewbs.merge: added check to avoid hyperspaced tempo!
				SetTempo(param);
			break;

		// Set Offset
		case CMD_OFFSET:
			if (m_nTickCount) break;
			//rewbs.volOffset: moved sample offset code to own method
			SampleOffset(nChn, param, bPorta);
			break;

		// Arpeggio
		case CMD_ARPEGGIO:
			// IT compatibility 01. Don't ignore Arpeggio if no note is playing
			if ((m_nTickCount) || (((!pChn->nPeriod) || !pChn->nNote) && !IsCompatibleMode(TRK_IMPULSETRACKER | TRK_SCREAMTRACKER))) break;
			if ((!param) && (!(m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT)))) break;
			pChn->nCommand = CMD_ARPEGGIO;
			if (param) pChn->nArpeggio = param;
			break;

		// Retrig
		case CMD_RETRIG:
			if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
			{
				if (!(param & 0xF0)) param |= pChn->nRetrigParam & 0xF0;
				if (!(param & 0x0F)) param |= pChn->nRetrigParam & 0x0F;
				param |= 0x100; // increment retrig count on first row
			}
			if(IsCompatibleMode(TRK_IMPULSETRACKER))
			{
				// IT compatibility 15. Retrigger
				if (param)
					pChn->nRetrigParam = (BYTE)(param & 0xFF);

				if (volcmd == VOLCMD_OFFSET)
					RetrigNote(nChn, pChn->nRetrigParam, vol << 3);
				else if (volcmd == VOLCMD_VELOCITY)
					RetrigNote(nChn, pChn->nRetrigParam, 48 - (vol << 3));
				else
					RetrigNote(nChn, pChn->nRetrigParam);
			}
			else
			{
				// XM Retrig
				if (param) pChn->nRetrigParam = (BYTE)(param & 0xFF); else param = pChn->nRetrigParam;
				//rewbs.volOffset
				//RetrigNote(nChn, param);
				if (volcmd == VOLCMD_OFFSET)
					RetrigNote(nChn, param, vol << 3);
				else if (volcmd == VOLCMD_VELOCITY)
					RetrigNote(nChn, param, 48 - (vol << 3));
				else
					RetrigNote(nChn, param);
				//end rewbs.volOffset:
			}
			break;

		// Tremor
		case CMD_TREMOR:
			if (!(m_dwSongFlags & SONG_FIRSTTICK)) break;

			if(IsCompatibleMode(TRK_IMPULSETRACKER))
			{
				// IT compatibility 12. / 13. Tremor (using modified DUMB's Tremor logic here because of old effects - http://dumb.sf.net/)

				if (param && !(m_dwSongFlags & SONG_ITOLDEFFECTS)) {
					// Old effects have different length interpretation (+1 for both on and off)
					if (param & 0xf0) param -= 0x10;
					if (param & 0x0f) param -= 0x01;
				}
				pChn->nTremorCount |= 128; // set on/off flag

			}
			else
			{
				// XM Tremor. Logic is being processed in sndmix.cpp
			}
				
			pChn->nCommand = CMD_TREMOR;
			if (param) pChn->nTremorParam = param;

			break;

		// Set Global Volume
		case CMD_GLOBALVOLUME:
			if (!(m_dwSongFlags & SONG_FIRSTTICK)) break;
			
			if (!(m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT))) param <<= 1;
			if(IsCompatibleMode(TRK_IMPULSETRACKER | TRK_FASTTRACKER2))
			{
				//IT compatibility 16. Both FT2 and IT ignore out-of-range values
				if (param <= 128)
					m_nGlobalVolume = param << 1;
			}
			else
			{
				if (param > 128) param = 128;
				m_nGlobalVolume = param << 1;
			}
			break;

		// Global Volume Slide
		case CMD_GLOBALVOLSLIDE:
			//IT compatibility 16. Saving last global volume slide param per channel (FT2/IT)
			if(IsCompatibleMode(TRK_IMPULSETRACKER | TRK_FASTTRACKER2))
				GlobalVolSlide(param, &pChn->nOldGlobalVolSlide);
			else
				GlobalVolSlide(param, &m_nOldGlbVolSlide);
			break;

		// Set 8-bit Panning
		case CMD_PANNING8:
			if (!(m_dwSongFlags & SONG_FIRSTTICK)) break;
			if ((m_dwSongFlags & SONG_PT1XMODE)) break;
			if (!(m_dwSongFlags & SONG_SURROUNDPAN)) pChn->dwFlags &= ~CHN_SURROUND;
			if (m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT|MOD_TYPE_XM|MOD_TYPE_MOD|MOD_TYPE_MT2))
			{
				pChn->nPan = param;
			} else
			if (param <= 0x80)
			{
				pChn->nPan = param << 1;
			} else
			if (param == 0xA4)
			{
				pChn->dwFlags |= CHN_SURROUND;
				pChn->nPan = 0x80;
			}
			pChn->dwFlags |= CHN_FASTVOLRAMP;
			pChn->nRestorePanOnNewNote = 0;
			//IT compatibility 20. Set pan overrides random pan
			if(IsCompatibleMode(TRK_IMPULSETRACKER))
				pChn->nPanSwing = 0;
			break;
			
		// Panning Slide
		case CMD_PANNINGSLIDE:
			PanningSlide(pChn, param);
			break;

		// Tremolo
		case CMD_TREMOLO:
			Tremolo(pChn, param);
			break;

		// Fine Vibrato
		case CMD_FINEVIBRATO:
			FineVibrato(pChn, param);
			break;

		// MOD/XM Exx Extended Commands
		case CMD_MODCMDEX:
			ExtendedMODCommands(nChn, param);
			break;

		// S3M/IT Sxx Extended Commands
		case CMD_S3MCMDEX:
			ExtendedS3MCommands(nChn, param);
			break;

		// Key Off
		case CMD_KEYOFF:
			if(IsCompatibleMode(TRK_FASTTRACKER2))
			{
				// This is how it's supposed to sound... (in FT2)
				if (m_nTickCount == param)
				{
					// XM: Key-Off + Sample == Note Cut
					if ((!pChn->pModInstrument) || (!(pChn->pModInstrument->VolEnv.dwFlags & ENV_ENABLED)))
					{
						if(param == 0) // FT2 is weird....
						{
							pChn->dwFlags |= CHN_NOTEFADE;
						}
						else
						{
							pChn->dwFlags |= CHN_FASTVOLRAMP;
							pChn->nVolume = 0;
						}
					}
					KeyOff(nChn);
				}
			}
			else
			{
				// This is how it's NOT supposed to sound...
				if(m_dwSongFlags & SONG_FIRSTTICK)
					KeyOff(nChn);
			}
			break;

		// Extra-fine porta up/down
		case CMD_XFINEPORTAUPDOWN:
			switch(param & 0xF0)
			{
			case 0x10: ExtraFinePortamentoUp(pChn, param & 0x0F); break;
			case 0x20: ExtraFinePortamentoDown(pChn, param & 0x0F); break;
			// Modplug XM Extensions
			case 0x50: 
			case 0x60: 
			case 0x70:
			case 0x90: 
			case 0xA0: if(!IsCompatibleMode(TRK_FASTTRACKER2)) ExtendedS3MCommands(nChn, param);
						break;
			}
			break;

		// Set Channel Global Volume
		case CMD_CHANNELVOLUME:
			if (m_nTickCount) break;
			if (param <= 64)
			{
				pChn->nGlobalVol = param;
				pChn->dwFlags |= CHN_FASTVOLRAMP;
			}
			break;

		// Channel volume slide
		case CMD_CHANNELVOLSLIDE:
			ChannelVolSlide(pChn, param);
			break;

		// Panbrello (IT)
		case CMD_PANBRELLO:
			Panbrello(pChn, param);
			break;

		// Set Envelope Position
		case CMD_SETENVPOSITION:
			if(m_dwSongFlags & SONG_FIRSTTICK)
			{
				pChn->nVolEnvPosition = param;

				if(!IsCompatibleMode(TRK_FASTTRACKER2))
				{
					// FT2 only sets the position of the Volume envelope
					pChn->nPanEnvPosition = param;
					pChn->nPitchEnvPosition = param;
					if (pChn->pModInstrument)
					{
						MODINSTRUMENT *pIns = pChn->pModInstrument;
						if ((pChn->dwFlags & CHN_PANENV) && (pIns->PanEnv.nNodes) && (param > pIns->PanEnv.Ticks[pIns->PanEnv.nNodes-1]))
						{
							pChn->dwFlags &= ~CHN_PANENV;
						}
					}
				}

			}
			break;

		// Position Jump
		case CMD_POSITIONJUMP:
			nPosJump = param;
			//if((m_dwSongFlags & SONG_PATTERNLOOP && m_nSeqOverride == 0)) {
			//	 m_nSeqOverride = param + 1;
				 //Releasing pattern loop after position jump could cause 
				 //instant jumps - modifying behavior so that now position jumps
				 //occurs also when pattern loop is enabled.
			//}
			// see http://lpchip.com/modplug/viewtopic.php?t=2769 - FastTracker resets Dxx if Bxx is called _after_ Dxx
			if(GetType() == MOD_TYPE_XM) nBreakRow = 0;
			break;

		// Pattern Break
		case CMD_PATTERNBREAK:
			break;

		// Midi Controller
		case CMD_MIDI:
			if(!(m_dwSongFlags & SONG_FIRSTTICK)) break;
			if (param < 0x80){
				//ProcessMidiMacro(nChn, &m_MidiCfg.szMidiSFXExt[pChn->nActiveMacro << 5], param);
			} else {
				//ProcessMidiMacro(nChn, &m_MidiCfg.szMidiZXXExt[(param & 0x7F) << 5], 0);
			}
			break;

		//rewbs.smoothVST: Smooth Macro slide
		case CMD_SMOOTHMIDI:
			if (param < 0x80) {
				//ProcessSmoothMidiMacro(nChn, &m_MidiCfg.szMidiSFXExt[pChn->nActiveMacro << 5], param);
			} else	{
				//ProcessSmoothMidiMacro(nChn, &m_MidiCfg.szMidiZXXExt[(param & 0x7F) << 5], 0);
			}
			break;
		//rewbs.smoothVST end 
		
		//rewbs.velocity
		case CMD_VELOCITY:
			break;
		//end rewbs.velocity

		// IMF Commands
		case CMD_NOTESLIDEUP:
			NoteSlide(pChn, param, 1);
			break;
		case CMD_NOTESLIDEDOWN:
			NoteSlide(pChn, param, -1);
			break;
		}

	} // for(...) end

	return TRUE;
}

void CSoundFile::resetEnvelopes(MODCHANNEL* pChn, int envToReset)
//---------------------------------------------------------------
{
	switch (envToReset) {
		case ENV_RESET_ALL:
			pChn->nVolEnvPosition = 0;
			pChn->nPanEnvPosition = 0;
			pChn->nPitchEnvPosition = 0;
			pChn->nVolEnvValueAtReleaseJump = NOT_YET_RELEASED;
			pChn->nPitchEnvValueAtReleaseJump = NOT_YET_RELEASED;
			pChn->nPanEnvValueAtReleaseJump = NOT_YET_RELEASED;
			break;
		case ENV_RESET_VOL:
			pChn->nVolEnvPosition = 0;
			pChn->nVolEnvValueAtReleaseJump = NOT_YET_RELEASED;
			break;
		case ENV_RESET_PAN:
			pChn->nPanEnvPosition = 0;
			pChn->nPanEnvValueAtReleaseJump = NOT_YET_RELEASED;
			break;
		case ENV_RESET_PITCH:
			pChn->nPitchEnvPosition = 0;
			pChn->nPitchEnvValueAtReleaseJump = NOT_YET_RELEASED;
			break;				
	}
}


////////////////////////////////////////////////////////////
// Channels effects

void CSoundFile::PortamentoUp(MODCHANNEL *pChn, UINT param, const bool doFinePortamentoAsRegular)
//---------------------------------------------------------
{
	MidiPortamento(pChn, param); //Send midi pitch bend event if there's a plugin

	if(GetType() == MOD_TYPE_MPT && pChn->pModInstrument && pChn->pModInstrument->pTuning)
	{
		if(param)
			pChn->nOldPortaUpDown = param;
		else
			param = pChn->nOldPortaUpDown;

		if(param >= 0xF0 && !doFinePortamentoAsRegular)
			PortamentoFineMPT(pChn, param - 0xF0);
		else
			PortamentoMPT(pChn, param);
		return;
	}

	if (param) pChn->nOldPortaUpDown = param; else param = pChn->nOldPortaUpDown;
	if (!doFinePortamentoAsRegular && (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT|MOD_TYPE_STM)) && ((param & 0xF0) >= 0xE0))
	{
		if (param & 0x0F)
		{
			if ((param & 0xF0) == 0xF0)
			{
				FinePortamentoUp(pChn, param & 0x0F);
			} else
			if ((param & 0xF0) == 0xE0)
			{
				ExtraFinePortamentoUp(pChn, param & 0x0F);
			}
		}
		return;
	}
	// Regular Slide
	if (!(m_dwSongFlags & SONG_FIRSTTICK) || (m_nMusicSpeed == 1))  //rewbs.PortaA01fix
	{
		DoFreqSlide(pChn, -(int)(param * 4));
	}
}


void CSoundFile::PortamentoDown(MODCHANNEL *pChn, UINT param, const bool doFinePortamentoAsRegular)
//-----------------------------------------------------------
{
	MidiPortamento(pChn, -1*param); //Send midi pitch bend event if there's a plugin

	if(m_nType == MOD_TYPE_MPT && pChn->pModInstrument && pChn->pModInstrument->pTuning)
	{
		if(param)
			pChn->nOldPortaUpDown = param;
		else
			param = pChn->nOldPortaUpDown;

		if(param >= 0xF0 && !doFinePortamentoAsRegular)
			PortamentoFineMPT(pChn, -1*(param - 0xF0));
		else
			PortamentoMPT(pChn, -1*param);
		return;
	}

	if (param) pChn->nOldPortaUpDown = param; else param = pChn->nOldPortaUpDown;
	if (!doFinePortamentoAsRegular && (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT|MOD_TYPE_STM)) && ((param & 0xF0) >= 0xE0))
	{
		if (param & 0x0F)
		{
			if ((param & 0xF0) == 0xF0)
			{
				FinePortamentoDown(pChn, param & 0x0F);
			} else
			if ((param & 0xF0) == 0xE0)
			{
				ExtraFinePortamentoDown(pChn, param & 0x0F);
			}
		}
		return;
	}
	if (!(m_dwSongFlags & SONG_FIRSTTICK)  || (m_nMusicSpeed == 1)) {  //rewbs.PortaA01fix
		DoFreqSlide(pChn, (int)(param << 2));
	}
}

void CSoundFile::MidiPortamento(MODCHANNEL *pChn, int param)
//----------------------------------------------------------
{
	//Send midi pitch bend event if there's a plugin:
	/*MODINSTRUMENT *pHeader = pChn->pModInstrument;
	if (pHeader && pHeader->nMidiChannel>0 && pHeader->nMidiChannel<17) { // instro sends to a midi chan
		UINT nPlug = pHeader->nMixPlug;
		if ((nPlug) && (nPlug <= MAX_MIXPLUGINS)) {
			IMixPlugin *pPlug = (IMixPlugin*)m_MixPlugins[nPlug-1].pMixPlugin;
			if (pPlug) {
				pPlug->MidiPitchBend(pHeader->nMidiChannel, param, 0);
			}
		}
	}*/
}

void CSoundFile::FinePortamentoUp(MODCHANNEL *pChn, UINT param)
//-------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideDownTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod -= (int)(param * 4);
			}
			if (pChn->nPeriod < 1) pChn->nPeriod = 1;
		}
	}
}


void CSoundFile::FinePortamentoDown(MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideUpTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod += (int)(param * 4);
			}
			if (pChn->nPeriod > 0xFFFF) pChn->nPeriod = 0xFFFF;
		}
	}
}


void CSoundFile::ExtraFinePortamentoUp(MODCHANNEL *pChn, UINT param)
//------------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, FineLinearSlideDownTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod -= (int)(param);
			}
			if (pChn->nPeriod < 1) pChn->nPeriod = 1;
		}
	}
}


void CSoundFile::ExtraFinePortamentoDown(MODCHANNEL *pChn, UINT param)
//--------------------------------------------------------------------
{
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (param) pChn->nOldFinePortaUpDown = param; else param = pChn->nOldFinePortaUpDown;
	}
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		if ((pChn->nPeriod) && (param))
		{
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				pChn->nPeriod = _muldivr(pChn->nPeriod, FineLinearSlideUpTable[param & 0x0F], 65536);
			} else
			{
				pChn->nPeriod += (int)(param);
			}
			if (pChn->nPeriod > 0xFFFF) pChn->nPeriod = 0xFFFF;
		}
	}
}

// Implemented for IMF compatibility, can't actually save this in any formats
// sign should be 1 (up) or -1 (down)
void CSoundFile::NoteSlide(MODCHANNEL *pChn, UINT param, int sign)
//----------------------------------------------------------------
{
	BYTE x, y;
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		x = param & 0xf0;
		if (x)
			pChn->nNoteSlideSpeed = (x >> 4);
		y = param & 0xf;
		if (y)
			pChn->nNoteSlideStep = y;
		pChn->nNoteSlideCounter = pChn->nNoteSlideSpeed;
	} else
	{
		if (--pChn->nNoteSlideCounter == 0)
		{
			pChn->nNoteSlideCounter = pChn->nNoteSlideSpeed;
			// update it
			pChn->nPeriod = GetPeriodFromNote
				(sign * pChn->nNoteSlideStep + GetNoteFromPeriod(pChn->nPeriod), 8363, 0);
		}
	}
}

// Portamento Slide
void CSoundFile::TonePortamento(MODCHANNEL *pChn, UINT param)
//-----------------------------------------------------------
{
	pChn->dwFlags |= CHN_PORTAMENTO;

	//IT compatibility 03
	if(!(m_dwSongFlags & SONG_ITCOMPATMODE) && IsCompatibleMode(TRK_IMPULSETRACKER))
	{
		if(param == 0) param = pChn->nOldPortaUpDown;
		pChn->nOldPortaUpDown = param;
	}

	if(m_nType == MOD_TYPE_MPT && pChn->pModInstrument && pChn->pModInstrument->pTuning)
	{
		//Behavior: Param tells number of finesteps(or 'fullsteps'(notes) with glissando)
		//to slide per row(not per tick).	
		const long old_PortamentoTickSlide = (m_nTickCount != 0) ? pChn->m_PortamentoTickSlide : 0;

		if(param)
			pChn->nPortamentoSlide = param;
		else
			if(pChn->nPortamentoSlide == 0)
				return;


		if((pChn->nPortamentoDest > 0 && pChn->nPortamentoSlide < 0) ||
			(pChn->nPortamentoDest < 0 && pChn->nPortamentoSlide > 0))
			pChn->nPortamentoSlide = -pChn->nPortamentoSlide;

		pChn->m_PortamentoTickSlide = (m_nTickCount + 1.0) * pChn->nPortamentoSlide / m_nMusicSpeed;

		if(pChn->dwFlags & CHN_GLISSANDO)
		{
			pChn->m_PortamentoTickSlide *= pChn->pModInstrument->pTuning->GetFineStepCount() + 1;
			//With glissando interpreting param as notes instead of finesteps.
		}

		const long slide = pChn->m_PortamentoTickSlide - old_PortamentoTickSlide;

		if(abs(pChn->nPortamentoDest) <= abs(slide))
		{
			if(pChn->nPortamentoDest != 0)
			{
				pChn->m_PortamentoFineSteps += pChn->nPortamentoDest;
				pChn->nPortamentoDest = 0;
				pChn->m_CalculateFreq = true;
			}
		}
		else
		{
			pChn->m_PortamentoFineSteps += slide;
			pChn->nPortamentoDest -= slide;
			pChn->m_CalculateFreq = true;
		}

		return;
	} //End candidate MPT behavior.

	if (param) pChn->nPortamentoSlide = param * 4;
	if ((pChn->nPeriod) && (pChn->nPortamentoDest) && ((m_nMusicSpeed == 1) || !(m_dwSongFlags & SONG_FIRSTTICK)))  //rewbs.PortaA01fix
	{
		if (pChn->nPeriod < pChn->nPortamentoDest)
		{
			LONG delta = (int)pChn->nPortamentoSlide;
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				UINT n = pChn->nPortamentoSlide >> 2;
				if (n > 255) n = 255;
				// Return (a*b+c/2)/c - no divide error
				// Table is 65536*2(n/192)
				delta = _muldivr(pChn->nPeriod, LinearSlideUpTable[n], 65536) - pChn->nPeriod;
				if (delta < 1) delta = 1;
			}
			pChn->nPeriod += delta;
			if (pChn->nPeriod > pChn->nPortamentoDest) pChn->nPeriod = pChn->nPortamentoDest;
		} else
		if (pChn->nPeriod > pChn->nPortamentoDest)
		{
			LONG delta = - (int)pChn->nPortamentoSlide;
			if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
			{
				UINT n = pChn->nPortamentoSlide >> 2;
				if (n > 255) n = 255;
				delta = _muldivr(pChn->nPeriod, LinearSlideDownTable[n], 65536) - pChn->nPeriod;
				if (delta > -1) delta = -1;
			}
			pChn->nPeriod += delta;
			if (pChn->nPeriod < pChn->nPortamentoDest) pChn->nPeriod = pChn->nPortamentoDest;
		}
	}

	//IT compatibility 23. Portamento with no note
	if(pChn->nPeriod == pChn->nPortamentoDest && IsCompatibleMode(TRK_IMPULSETRACKER))
		pChn->nPortamentoDest = 0;

}


void CSoundFile::Vibrato(MODCHANNEL *p, UINT param)
//-------------------------------------------------
{
	p->m_VibratoDepth = param % 16 / 15.0F;
	//'New tuning'-thing: 0 - 1 <-> No depth - Full depth.


	if (param & 0x0F) p->nVibratoDepth = (param & 0x0F) * 4;
	if (param & 0xF0) p->nVibratoSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_VIBRATO;
}


void CSoundFile::FineVibrato(MODCHANNEL *p, UINT param)
//-----------------------------------------------------
{
	if (param & 0x0F) p->nVibratoDepth = param & 0x0F;
	if (param & 0xF0) p->nVibratoSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_VIBRATO;
}


void CSoundFile::Panbrello(MODCHANNEL *p, UINT param)
//---------------------------------------------------
{
	if (param & 0x0F) p->nPanbrelloDepth = param & 0x0F;
	if (param & 0xF0) p->nPanbrelloSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_PANBRELLO;
}


void CSoundFile::VolumeSlide(MODCHANNEL *pChn, UINT param)
//--------------------------------------------------------
{
	if (param) pChn->nOldVolumeSlide = param; else param = pChn->nOldVolumeSlide;
	LONG newvolume = pChn->nVolume;
	if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT|MOD_TYPE_STM|MOD_TYPE_AMF))
	{
		if ((param & 0x0F) == 0x0F) //Fine upslide or slide -15
		{
			if (param & 0xF0) //Fine upslide
			{
				FineVolumeUp(pChn, (param >> 4));
				return;
			} else //Slide -15
			{
				if ((m_dwSongFlags & SONG_FIRSTTICK) && (!(m_dwSongFlags & SONG_FASTVOLSLIDES)))
				{
					newvolume -= 0x0F * 4;
				}
			}
		} else
		if ((param & 0xF0) == 0xF0) //Fine downslide or slide +15
		{
			if (param & 0x0F) //Fine downslide
			{
				FineVolumeDown(pChn, (param & 0x0F));
				return;
			} else //Slide +15
			{
				if ((m_dwSongFlags & SONG_FIRSTTICK) && (!(m_dwSongFlags & SONG_FASTVOLSLIDES)))
				{
					newvolume += 0x0F * 4;
				}
			}
		}
	}
	if ((!(m_dwSongFlags & SONG_FIRSTTICK)) || (m_dwSongFlags & SONG_FASTVOLSLIDES))
	{
		if (param & 0x0F) newvolume -= (int)((param & 0x0F) * 4);
		else newvolume += (int)((param & 0xF0) >> 2);
		if (m_nType & MOD_TYPE_MOD) pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
	newvolume = CLAMP(newvolume, 0, 256);

	pChn->nVolume = newvolume;
}


void CSoundFile::PanningSlide(MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------
{
	LONG nPanSlide = 0;
	if (param) pChn->nOldPanSlide = param; else param = pChn->nOldPanSlide;
	if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT|MOD_TYPE_STM))
	{
		if (((param & 0x0F) == 0x0F) && (param & 0xF0))
		{
			if (m_dwSongFlags & SONG_FIRSTTICK)
			{
				param = (param & 0xF0) >> 2;
				nPanSlide = - (int)param;
			}
		} else
		if (((param & 0xF0) == 0xF0) && (param & 0x0F))
		{
			if (m_dwSongFlags & SONG_FIRSTTICK)
			{
				nPanSlide = (param & 0x0F) << 2;
			}
		} else
		{
			if (!(m_dwSongFlags & SONG_FIRSTTICK))
			{
				if (param & 0x0F) nPanSlide = (int)((param & 0x0F) << 2);
				else nPanSlide = -(int)((param & 0xF0) >> 2);
			}
		}
	} else
	{
		if (!(m_dwSongFlags & SONG_FIRSTTICK))
		{
			if (param & 0x0F) nPanSlide = -(int)((param & 0x0F) << 2);
			else nPanSlide = (int)((param & 0xF0) >> 2);
			if(IsCompatibleMode(TRK_FASTTRACKER2))
				nPanSlide >>= 2;
		}
	}
	if (nPanSlide)
	{
		nPanSlide += pChn->nPan;
		nPanSlide = CLAMP(nPanSlide, 0, 256);
		pChn->nPan = nPanSlide;
		pChn->nRestorePanOnNewNote = 0;
	}
}


void CSoundFile::FineVolumeUp(MODCHANNEL *pChn, UINT param)
//---------------------------------------------------------
{
	if (param) pChn->nOldFineVolUpDown = param; else param = pChn->nOldFineVolUpDown;
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		pChn->nVolume += param * 4;
		if (pChn->nVolume > 256) pChn->nVolume = 256;
		if (m_nType & MOD_TYPE_MOD) pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CSoundFile::FineVolumeDown(MODCHANNEL *pChn, UINT param)
//-----------------------------------------------------------
{
	if (param) pChn->nOldFineVolUpDown = param; else param = pChn->nOldFineVolUpDown;
	if (m_dwSongFlags & SONG_FIRSTTICK)
	{
		pChn->nVolume -= param * 4;
		if (pChn->nVolume < 0) pChn->nVolume = 0;
		if (m_nType & MOD_TYPE_MOD) pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CSoundFile::Tremolo(MODCHANNEL *p, UINT param)
//-------------------------------------------------
{
	if (param & 0x0F) p->nTremoloDepth = (param & 0x0F) << 2;
	if (param & 0xF0) p->nTremoloSpeed = (param >> 4) & 0x0F;
	p->dwFlags |= CHN_TREMOLO;
}


void CSoundFile::ChannelVolSlide(MODCHANNEL *pChn, UINT param)
//------------------------------------------------------------
{
	LONG nChnSlide = 0;
	if (param) pChn->nOldChnVolSlide = param; else param = pChn->nOldChnVolSlide;
	if (((param & 0x0F) == 0x0F) && (param & 0xF0))
	{
		if (m_dwSongFlags & SONG_FIRSTTICK) nChnSlide = param >> 4;
	} else
	if (((param & 0xF0) == 0xF0) && (param & 0x0F))
	{
		if (m_dwSongFlags & SONG_FIRSTTICK) nChnSlide = - (int)(param & 0x0F);
	} else
	{
		if (!(m_dwSongFlags & SONG_FIRSTTICK))
		{
			if (param & 0x0F) nChnSlide = -(int)(param & 0x0F);
			else nChnSlide = (int)((param & 0xF0) >> 4);
		}
	}
	if (nChnSlide)
	{
		nChnSlide += pChn->nGlobalVol;
		nChnSlide = CLAMP(nChnSlide, 0, 64);
		pChn->nGlobalVol = nChnSlide;
	}
}


void CSoundFile::ExtendedMODCommands(UINT nChn, UINT param)
//---------------------------------------------------------
{
	MODCHANNEL *pChn = &Chn[nChn];
	UINT command = param & 0xF0;
	param &= 0x0F;
	switch(command)
	{
	// E0x: Set Filter
	// E1x: Fine Portamento Up
	case 0x10:	if ((param) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) FinePortamentoUp(pChn, param); break;
	// E2x: Fine Portamento Down
	case 0x20:	if ((param) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) FinePortamentoDown(pChn, param); break;
	// E3x: Set Glissando Control
	case 0x30:	pChn->dwFlags &= ~CHN_GLISSANDO; if (param) pChn->dwFlags |= CHN_GLISSANDO; break;
	// E4x: Set Vibrato WaveForm
	case 0x40:	pChn->nVibratoType = param & 0x07; break;
	// E5x: Set FineTune
	case 0x50:	if(!(m_dwSongFlags & SONG_FIRSTTICK)) break;
				pChn->nC5Speed = S3MFineTuneTable[param];
				if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
					pChn->nFineTune = param*2;
				else
					pChn->nFineTune = MOD2XMFineTune(param);
				if (pChn->nPeriod) pChn->nPeriod = GetPeriodFromNote(pChn->nNote, pChn->nFineTune, pChn->nC5Speed);
				break;
	// E6x: Pattern Loop
	// E7x: Set Tremolo WaveForm
	case 0x70:	pChn->nTremoloType = param & 0x07; break;
	// E8x: Set 4-bit Panning
	case 0x80:	if((m_dwSongFlags & SONG_FIRSTTICK) && !(m_dwSongFlags & SONG_PT1XMODE))
				{ 
					//IT compatibility 20. (Panning always resets surround state)
					if(IsCompatibleMode(TRK_ALLTRACKERS))
					{
						if (!(m_dwSongFlags & SONG_SURROUNDPAN)) pChn->dwFlags &= ~CHN_SURROUND;
					}
					pChn->nPan = (param << 4) + 8; pChn->dwFlags |= CHN_FASTVOLRAMP;
				}
				break;
	// E9x: Retrig
	case 0x90:	RetrigNote(nChn, param); break;
	// EAx: Fine Volume Up
	case 0xA0:	if ((param) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) FineVolumeUp(pChn, param); break;
	// EBx: Fine Volume Down
	case 0xB0:	if ((param) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) FineVolumeDown(pChn, param); break;
	// ECx: Note Cut
	case 0xC0:	NoteCut(nChn, param); break;
	// EDx: Note Delay
	// EEx: Pattern Delay
	case 0xF0:	
		if(GetType() == MOD_TYPE_MOD) // MOD: Invert Loop
		{
			pChn->nEFxSpeed = param;
			if(m_dwSongFlags & SONG_FIRSTTICK) InvertLoop(pChn);
		}	
		else // XM: Set Active Midi Macro
		{
			pChn->nActiveMacro = param;
		}
		break;
	}
}


void CSoundFile::ExtendedS3MCommands(UINT nChn, UINT param)
//---------------------------------------------------------
{
	MODCHANNEL *pChn = &Chn[nChn];
	UINT command = param & 0xF0;
	param &= 0x0F;
	switch(command)
	{
	// S0x: Set Filter
	// S1x: Set Glissando Control
	case 0x10:	pChn->dwFlags &= ~CHN_GLISSANDO; if (param) pChn->dwFlags |= CHN_GLISSANDO; break;
	// S2x: Set FineTune
	case 0x20:	if(!(m_dwSongFlags & SONG_FIRSTTICK)) break;
				pChn->nC5Speed = S3MFineTuneTable[param];
				pChn->nFineTune = MOD2XMFineTune(param);
				if (pChn->nPeriod) pChn->nPeriod = GetPeriodFromNote(pChn->nNote, pChn->nFineTune, pChn->nC5Speed);
				break;
	// S3x: Set Vibrato Waveform
	case 0x30:	if(GetType() == MOD_TYPE_S3M)
				{
					pChn->nVibratoType = param & 0x03;
				} else
				{
					if(IsCompatibleMode(TRK_IMPULSETRACKER))
						pChn->nVibratoType = (param < 0x04) ? param : 0;
					else
						pChn->nVibratoType = param & 0x07;
				}
				break;
	// S4x: Set Tremolo Waveform
	case 0x40:	if(GetType() == MOD_TYPE_S3M)
				{
					pChn->nTremoloType = param & 0x03;
				} else
				{
					if(IsCompatibleMode(TRK_IMPULSETRACKER))
						pChn->nTremoloType = (param < 0x04) ? param : 0;
					else
						pChn->nTremoloType = param & 0x07;
				}
				break;
	// S5x: Set Panbrello Waveform
	case 0x50:	if(IsCompatibleMode(TRK_IMPULSETRACKER))
					pChn->nPanbrelloType = (param < 0x04) ? param : 0;
				else
					pChn->nPanbrelloType = param & 0x07;
				break;
	// S6x: Pattern Delay for x frames
	case 0x60:	m_nFrameDelay = param; break;
	// S7x: Envelope Control / Instrument Control
	case 0x70:	if(!(m_dwSongFlags & SONG_FIRSTTICK)) break;
				switch(param)
				{
				case 0:
				case 1:
				case 2:
					{
						MODCHANNEL *bkp = &Chn[m_nChannels];
						for (UINT i=m_nChannels; i<MAX_CHANNELS; i++, bkp++)
						{
							if (bkp->nMasterChn == nChn+1)
							{
								if (param == 1)
								{
									KeyOff(i);
								} else if (param == 2)
								{
									bkp->dwFlags |= CHN_NOTEFADE;
								} else
								{
									bkp->dwFlags |= CHN_NOTEFADE;
									bkp->nFadeOutVol = 0;
								}
							}
						}
					}
					break;
				case 3:		pChn->nNNA = NNA_NOTECUT; break;
				case 4:		pChn->nNNA = NNA_CONTINUE; break;
				case 5:		pChn->nNNA = NNA_NOTEOFF; break;
				case 6:		pChn->nNNA = NNA_NOTEFADE; break;
				case 7:		pChn->dwFlags &= ~CHN_VOLENV; break;
				case 8:		pChn->dwFlags |= CHN_VOLENV; break;
				case 9:		pChn->dwFlags &= ~CHN_PANENV; break;
				case 10:	pChn->dwFlags |= CHN_PANENV; break;
				case 11:	pChn->dwFlags &= ~CHN_PITCHENV; break;
				case 12:	pChn->dwFlags |= CHN_PITCHENV; break;
				case 13:	
				case 14:
					if(GetType() == MOD_TYPE_MPT)
					{
						pChn->dwFlags |= CHN_PITCHENV;
						if(param == 13)	// pitch env on, filter env off
						{
							pChn->dwFlags &= ~CHN_FILTERENV;
						} else	// filter env on
						{
							pChn->dwFlags |= CHN_FILTERENV;
						}
					}
					break;
				}
				break;
	// S8x: Set 4-bit Panning
	case 0x80:	if(m_dwSongFlags & SONG_FIRSTTICK)
				{ 
					if(IsCompatibleMode(TRK_ALLTRACKERS))
					{
						if (!(m_dwSongFlags & SONG_SURROUNDPAN)) pChn->dwFlags &= ~CHN_SURROUND;
					}
					pChn->nPan = (param << 4) + 8; pChn->dwFlags |= CHN_FASTVOLRAMP;

					//IT compatibility 20. Set pan overrides random pan
					if(IsCompatibleMode(TRK_IMPULSETRACKER))
						pChn->nPanSwing = 0;
				}
				break;
	// S9x: Sound Control
	case 0x90:	ExtendedChannelEffect(pChn, param); break;
	// SAx: Set 64k Offset
	case 0xA0:	if(m_dwSongFlags & SONG_FIRSTTICK)
				{
					pChn->nOldHiOffset = param;
					if ((pChn->nRowNote) && (pChn->nRowNote < 0x80))
					{
						DWORD pos = param << 16;
						if (pos < pChn->nLength) pChn->nPos = pos;
					}
				}
				break;
	// SBx: Pattern Loop
	// SCx: Note Cut
	case 0xC0:	NoteCut(nChn, param); break;
	// SDx: Note Delay
	// SEx: Pattern Delay for x rows
	// SFx: S3M: Not used, IT: Set Active Midi Macro
	case 0xF0:	pChn->nActiveMacro = param; break;
	}
}


void CSoundFile::ExtendedChannelEffect(MODCHANNEL *pChn, UINT param)
//------------------------------------------------------------------
{
	// S9x and X9x commands (S3M/XM/IT only)
	if(!(m_dwSongFlags & SONG_FIRSTTICK)) return;
	switch(param & 0x0F)
	{
	// S90: Surround Off
	case 0x00:	pChn->dwFlags &= ~CHN_SURROUND;	break;
	// S91: Surround On
	case 0x01:	pChn->dwFlags |= CHN_SURROUND; pChn->nPan = 128; break;
	////////////////////////////////////////////////////////////
	// Modplug Extensions
	// S98: Reverb Off
	case 0x08:
		pChn->dwFlags &= ~CHN_REVERB;
		pChn->dwFlags |= CHN_NOREVERB;
		break;
	// S99: Reverb On
	case 0x09:
		pChn->dwFlags &= ~CHN_NOREVERB;
		pChn->dwFlags |= CHN_REVERB;
		break;
	// S9A: 2-Channels surround mode
	case 0x0A:
		m_dwSongFlags &= ~SONG_SURROUNDPAN;
		break;
	// S9B: 4-Channels surround mode
	case 0x0B:
		m_dwSongFlags |= SONG_SURROUNDPAN;
		break;
	// S9C: IT Filter Mode
	case 0x0C:
		m_dwSongFlags &= ~SONG_MPTFILTERMODE;
		break;
	// S9D: MPT Filter Mode
	case 0x0D:
		m_dwSongFlags |= SONG_MPTFILTERMODE;
		break;
	// S9E: Go forward
	case 0x0E:
		pChn->dwFlags &= ~(CHN_PINGPONGFLAG);
		break;
	// S9F: Go backward (set position at the end for non-looping samples)
	case 0x0F:
		if ((!(pChn->dwFlags & CHN_LOOP)) && (!pChn->nPos) && (pChn->nLength))
		{
			pChn->nPos = pChn->nLength - 1;
			pChn->nPosLo = 0xFFFF;
		}
		pChn->dwFlags |= CHN_PINGPONGFLAG;
		break;
	}
}


inline void CSoundFile::InvertLoop(MODCHANNEL *pChn)
//--------------------------------------------------
{
	// EFx implementation for MOD files (PT 1.1A and up: Invert Loop)
	// This effect trashes samples. Thanks to 8bitbubsy for making this work. :)
	if((m_nType & MOD_TYPE_MOD) == 0 || pChn->nEFxSpeed == 0) return;

	// we obviously also need a sample for this
	MODSAMPLE *pModSample = pChn->pModSample;
	if(pModSample == nullptr || pModSample->pSample == nullptr || !(pModSample->uFlags & CHN_LOOP)) return;

	pChn->nEFxDelay += ModEFxTable[pChn->nEFxSpeed & 0x0F];
	if((pChn->nEFxDelay & 0x80) == 0) return; // only applied if the "delay" reaches 128
	pChn->nEFxDelay = 0;

	if (++pChn->nEFxOffset >= pModSample->nLoopEnd - pModSample->nLoopStart)
		pChn->nEFxOffset = 0;

	// TRASH IT!!! (Yes, the sample!)
	pModSample->pSample[pModSample->nLoopStart + pChn->nEFxOffset] = ~pModSample->pSample[pModSample->nLoopStart + pChn->nEFxOffset];
}


//rewbs.volOffset: moved offset code to own method as it will be used in several places now
void CSoundFile::SampleOffset(UINT nChn, UINT param, bool bPorta)
//---------------------------------------------------------------
{

	MODCHANNEL *pChn = &Chn[nChn];
// -! NEW_FEATURE#0010
// -> CODE#0010
// -> DESC="add extended parameter mechanism to pattern effects"
// rewbs.NOTE: maybe move param calculation outside of method to cope with [ effect.
			//if (param) pChn->nOldOffset = param; else param = pChn->nOldOffset;
			//param <<= 8;
			//param |= (UINT)(pChn->nOldHiOffset) << 16;
/*			MODCOMMAND *m;
			m = NULL;

			if(m_nRow < PatternSize[m_nPattern]-1) m = Patterns[m_nPattern] + (m_nRow+1) * m_nChannels + nChn;

			if(m && m->command == CMD_XPARAM){
				UINT tmp = m->param;
				m = NULL;
				if(m_nRow < PatternSize[m_nPattern]-2) m = Patterns[m_nPattern] + (m_nRow+2) * m_nChannels  + nChn;

				if(m && m->command == CMD_XPARAM) param = (param<<16) + (tmp<<8) + m->param;
				else param = (param<<8) + tmp;
			}
			else*/{
				if (param) pChn->nOldOffset = param; else param = pChn->nOldOffset;
				param <<= 8;
				param |= (UINT)(pChn->nOldHiOffset) << 16;
			}
// -! NEW_FEATURE#0010

	if ((pChn->nRowNote) && (pChn->nRowNote < 0x80))
	{
		if (bPorta)
			pChn->nPos = param;
		else
			pChn->nPos += param;
		if (pChn->nPos >= pChn->nLength)
		{
			// Offset beyond sample size
			if (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)))
			{
				// IT Compatibility: Offset
				if(IsCompatibleMode(TRK_IMPULSETRACKER))
					if(m_dwSongFlags & SONG_ITOLDEFFECTS)
						pChn->nPos = pChn->nLength; // Old FX: Clip to end of sample
					else
						pChn->nPos = 0; // Reset to beginning of sample
				else
					pChn->nPos = pChn->nLoopStart;
				if ((m_dwSongFlags & SONG_ITOLDEFFECTS) && (pChn->nLength > 4))
				{
					pChn->nPos = pChn->nLength - 2;
				}
			} else if(IsCompatibleMode(TRK_FASTTRACKER2))
			{
				// XM Compatibility: Don't play note
				pChn->dwFlags |= CHN_FASTVOLRAMP;
				pChn->nVolume = pChn->nPeriod = 0;
			}
		}
	} else
	if ((param < pChn->nLength) && (m_nType & (MOD_TYPE_MTM|MOD_TYPE_DMF)))
	{
		pChn->nPos = param;
	}

	return;
}
//end rewbs.volOffset:

void CSoundFile::RetrigNote(UINT nChn, int param, UINT offset)	//rewbs.VolOffset: added offset param.
//------------------------------------------------------------
{
	// Retrig: bit 8 is set if it's the new XM retrig
	MODCHANNEL *pChn = &Chn[nChn];
	int nRetrigSpeed = param & 0x0F;
	int nRetrigCount = pChn->nRetrigCount;
	bool bDoRetrig = false;

	if(IsCompatibleMode(TRK_IMPULSETRACKER))
	{
		//IT compatibility 15. Retrigger
		if ((m_dwSongFlags & SONG_FIRSTTICK) && pChn->nRowNote)
		{
			pChn->nRetrigCount = param & 0xf;
		}
		else if (!pChn->nRetrigCount || !--pChn->nRetrigCount)
		{
			pChn->nRetrigCount = param & 0xf;
			bDoRetrig = true;
		}
	}
	else if(IsCompatibleMode(TRK_FASTTRACKER2) && (param & 0x100))
	{
		// buggy-like-hell FT2 Rxy retrig!

		if(m_dwSongFlags & SONG_FIRSTTICK)
		{
			// here are some really stupid things FT2 does
			if(pChn->nRowVolCmd == VOLCMD_VOLUME) return;
			if(pChn->nRowInstr > 0 && pChn->nRowNote == NOTE_NONE) nRetrigCount = 1;
			if(pChn->nRowNote != NOTE_NONE && pChn->nRowNote <= GetModSpecifications().noteMax) nRetrigCount++;
		}
		if (nRetrigCount >= nRetrigSpeed)
		{
			if (!(m_dwSongFlags & SONG_FIRSTTICK) || (pChn->nRowNote == NOTE_NONE)) 
			{
				bDoRetrig = true;
				nRetrigCount = 0;
			}
		}
	} else
	{
		// old routines

		if (m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT))
		{
			if (!nRetrigSpeed) nRetrigSpeed = 1;
			if ((nRetrigCount) && (!(nRetrigCount % nRetrigSpeed))) bDoRetrig = true;
			nRetrigCount++;
		} else
		{
			int realspeed = nRetrigSpeed;
			// FT2 bug: if a retrig (Rxy) occours together with a volume command, the first retrig interval is increased by one tick
			if ((param & 0x100) && (pChn->nRowVolCmd == VOLCMD_VOLUME) && (pChn->nRowParam & 0xF0)) realspeed++;
			if (!(m_dwSongFlags & SONG_FIRSTTICK) || (param & 0x100))
			{
				if (!realspeed) realspeed = 1;
				if ((!(param & 0x100)) && (m_nMusicSpeed) && (!(m_nTickCount % realspeed))) bDoRetrig = true;
				nRetrigCount++;
			} else if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2)) nRetrigCount = 0;
			if (nRetrigCount >= realspeed)
			{
				if ((m_nTickCount) || ((param & 0x100) && (!pChn->nRowNote))) bDoRetrig = true;
			}
		}
	}

	if (bDoRetrig)
	{
		UINT dv = (param >> 4) & 0x0F;
		if (dv)
		{
			int vol = pChn->nVolume;

			// FT2 compatibility: Retrig + volume will not change volume of retrigged notes
			if(!IsCompatibleMode(TRK_FASTTRACKER2) || !(pChn->nRowVolCmd == VOLCMD_VOLUME))
			{
				if (retrigTable1[dv])
					vol = (vol * retrigTable1[dv]) >> 4;
				else
					vol += ((int)retrigTable2[dv]) << 2;
			}

			vol = CLAMP(vol, 0, 256);

			pChn->nVolume = vol;
			pChn->dwFlags |= CHN_FASTVOLRAMP;
		}
		UINT nNote = pChn->nNewNote;
		LONG nOldPeriod = pChn->nPeriod;
		if ((nNote) && (nNote <= NOTE_MAX) && (pChn->nLength)) CheckNNA(nChn, 0, nNote, TRUE);
		bool bResetEnv = false;
		if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
		{
			if ((pChn->nRowInstr) && (param < 0x100))
			{
				InstrumentChange(pChn, pChn->nRowInstr, FALSE, FALSE);
				bResetEnv = true;
			}
			if (param < 0x100) bResetEnv = true;
		}
		NoteChange(nChn, nNote, IsCompatibleMode(TRK_IMPULSETRACKER) ? true : false, bResetEnv);
		if (m_nInstruments) {
#ifdef MODPLUG_TRACKER
			ProcessMidiOut(nChn, pChn);	//Send retrig to Midi
#endif
		}
		if ((m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT)) && (!pChn->nRowNote) && (nOldPeriod)) pChn->nPeriod = nOldPeriod;
		if (!(m_nType & (MOD_TYPE_S3M|MOD_TYPE_IT|MOD_TYPE_MPT))) nRetrigCount = 0;
		if(IsCompatibleMode(TRK_IMPULSETRACKER)) pChn->nPos = pChn->nPosLo = 0;

		if (offset)									//rewbs.volOffset: apply offset on retrig
		{
			if (pChn->pModSample)
				pChn->nLength = pChn->pModSample->nLength;
			SampleOffset(nChn, offset, false);
		}
	}

	// buggy-like-hell FT2 Rxy retrig!
	if(IsCompatibleMode(TRK_FASTTRACKER2) && (param & 0x100)) nRetrigCount++;

	if(!IsCompatibleMode(TRK_IMPULSETRACKER))
		pChn->nRetrigCount = (BYTE)nRetrigCount;
}


void CSoundFile::DoFreqSlide(MODCHANNEL *pChn, LONG nFreqSlide)
//-------------------------------------------------------------
{
	// IT Linear slides
	if (!pChn->nPeriod) return;
	if ((m_dwSongFlags & SONG_LINEARSLIDES) && (!(m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))))
	{
		LONG nOldPeriod = pChn->nPeriod;
		if (nFreqSlide < 0)
		{
			UINT n = (- nFreqSlide) >> 2;
			if (n)
			{
				if (n > 255) n = 255;
				pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideDownTable[n], 65536);
				if (pChn->nPeriod == nOldPeriod) pChn->nPeriod = nOldPeriod-1;
			}
		} else
		{
			UINT n = (nFreqSlide) >> 2;
			if (n)
			{
				if (n > 255) n = 255;
				pChn->nPeriod = _muldivr(pChn->nPeriod, LinearSlideUpTable[n], 65536);
				if (pChn->nPeriod == nOldPeriod) pChn->nPeriod = nOldPeriod+1;
			}
		}
	} else
	{
		pChn->nPeriod += nFreqSlide;
	}
	if (pChn->nPeriod < 1)
	{
		pChn->nPeriod = 1;
		if (m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT))
		{
			pChn->dwFlags |= CHN_NOTEFADE;
			pChn->nFadeOutVol = 0;
		}
	}
}


void CSoundFile::NoteCut(UINT nChn, UINT nTick)
//---------------------------------------------
{
	if(nTick == 0)
	{
		//IT compatibility 22. SC0 == SC1
		if(IsCompatibleMode(TRK_IMPULSETRACKER))
			nTick = 1;
		// ST3 doesn't cut notes with SC0
		else if(m_nType == MOD_TYPE_S3M)
			return;
	}

	if (m_nTickCount == nTick)
	{
		MODCHANNEL *pChn = &Chn[nChn];
		// if (m_nInstruments) KeyOff(pChn); ?
		pChn->nVolume = 0;
		pChn->dwFlags |= CHN_FASTVOLRAMP;
	}
}


void CSoundFile::KeyOff(UINT nChn)
//--------------------------------
{
	MODCHANNEL *pChn = &Chn[nChn];
	BOOL bKeyOn = (pChn->dwFlags & CHN_KEYOFF) ? FALSE : TRUE;
	pChn->dwFlags |= CHN_KEYOFF;
	//if ((!pChn->pModInstrument) || (!(pChn->dwFlags & CHN_VOLENV)))
	if ((pChn->pModInstrument) && (!(pChn->dwFlags & CHN_VOLENV)))
	{
		pChn->dwFlags |= CHN_NOTEFADE;
	}
	if (!pChn->nLength) return;
	if ((pChn->dwFlags & CHN_SUSTAINLOOP) && (pChn->pModSample) && (bKeyOn))
	{
		MODSAMPLE *psmp = pChn->pModSample;
		if (psmp->uFlags & CHN_LOOP)
		{
			if (psmp->uFlags & CHN_PINGPONGLOOP)
				pChn->dwFlags |= CHN_PINGPONGLOOP;
			else
				pChn->dwFlags &= ~(CHN_PINGPONGLOOP|CHN_PINGPONGFLAG);
			pChn->dwFlags |= CHN_LOOP;
			pChn->nLength = psmp->nLength;
			pChn->nLoopStart = psmp->nLoopStart;
			pChn->nLoopEnd = psmp->nLoopEnd;
			if (pChn->nLength > pChn->nLoopEnd) pChn->nLength = pChn->nLoopEnd;
		} else
		{
			pChn->dwFlags &= ~(CHN_LOOP|CHN_PINGPONGLOOP|CHN_PINGPONGFLAG);
			pChn->nLength = psmp->nLength;
		}
	}
	if (pChn->pModInstrument)
	{
		MODINSTRUMENT *pIns = pChn->pModInstrument;
		if (((pIns->VolEnv.dwFlags & ENV_LOOP) || (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))) && (pIns->nFadeOut)) {
			pChn->dwFlags |= CHN_NOTEFADE;
		}
	
		if (pIns->VolEnv.nReleaseNode != ENV_RELEASE_NODE_UNSET) {
			pChn->nVolEnvValueAtReleaseJump=getVolEnvValueFromPosition(pChn->nVolEnvPosition, pIns);
			pChn->nVolEnvPosition= pIns->VolEnv.Ticks[pIns->VolEnv.nReleaseNode];
		}

	}
}


//////////////////////////////////////////////////////////
// CSoundFile: Global Effects


void CSoundFile::SetSpeed(UINT param)
//-----------------------------------
{
	// Modplug Tracker and Mod-Plugin don't do this check
#ifndef MODPLUG_TRACKER
#ifndef FASTSOUNDLIB
	// Big Hack!!!
	if ((!param) || (param >= 0x80) || ((m_nType & (MOD_TYPE_MOD|MOD_TYPE_XM|MOD_TYPE_MT2)) && (param >= 0x1E)))
	{
		if ((!m_nRepeatCount) && (IsSongFinished(m_nCurrentPattern, m_nRow+1)))
		{
			GlobalFadeSong(1000);
		}
	}
#endif // FASTSOUNDLIB
#endif // MODPLUG_TRACKER
	//if ((m_nType & MOD_TYPE_S3M) && (param > 0x80)) param -= 0x80;
	if ((param) && (param <= GetModSpecifications().speedMax)) m_nMusicSpeed = param;
}


void CSoundFile::SetTempo(UINT param, bool setAsNonModcommand)
//------------------------------------------------------------
{
	const CModSpecifications& specs = GetModSpecifications();
	if(setAsNonModcommand)
	{
		if(param < specs.tempoMin) m_nMusicTempo = specs.tempoMin;
		else
		{
			if(param > specs.tempoMax) m_nMusicTempo = specs.tempoMax;
			else m_nMusicTempo = param;
		}
	}
	else
	{
		if (param >= 0x20 && (m_dwSongFlags & SONG_FIRSTTICK)) //rewbs.tempoSlideFix: only set if not (T0x or T1x) and tick is 0
		{
			m_nMusicTempo = param;
		}
		// Tempo Slide
		else if (param < 0x20 && m_nTickCount) //rewbs.tempoSlideFix: only slide if (T0x or T1x) and tick is not 0
		{
			if ((param & 0xF0) == 0x10)
				m_nMusicTempo += (param & 0x0F); //rewbs.tempoSlideFix: no *2
			else
				m_nMusicTempo -= (param & 0x0F); //rewbs.tempoSlideFix: no *2

		// -> CODE#0016
		// -> DESC="default tempo update"
			if(IsCompatibleMode(TRK_ALLTRACKERS))
				m_nMusicTempo = CLAMP(m_nMusicTempo, 32, 255);
			else
				m_nMusicTempo = CLAMP(m_nMusicTempo, specs.tempoMin, specs.tempoMax);
		// -! BEHAVIOUR_CHANGE#0016
		}
	}
}

void CSoundFile::GlobalVolSlide(UINT param, UINT * nOldGlobalVolSlide)
//-----------------------------------------
{
	LONG nGlbSlide = 0;
	if (param) *nOldGlobalVolSlide = param; else param = *nOldGlobalVolSlide;
	if (((param & 0x0F) == 0x0F) && (param & 0xF0))
	{
		if (m_dwSongFlags & SONG_FIRSTTICK) nGlbSlide = (param >> 4) * 2;
	} else
	if (((param & 0xF0) == 0xF0) && (param & 0x0F))
	{
		if (m_dwSongFlags & SONG_FIRSTTICK) nGlbSlide = - (int)((param & 0x0F) * 2);
	} else
	{
		if (!(m_dwSongFlags & SONG_FIRSTTICK))
		{
			if (param & 0xF0) nGlbSlide = (int)((param & 0xF0) >> 4) * 2;
			else nGlbSlide = -(int)((param & 0x0F) * 2);
		}
	}
	if (nGlbSlide)
	{
		if (!(m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT))) nGlbSlide *= 2;
		nGlbSlide += m_nGlobalVolume;
		nGlbSlide = CLAMP(nGlbSlide, 0, 256);
		m_nGlobalVolume = nGlbSlide;
	}
}

//////////////////////////////////////////////////////
// Note/Period/Frequency functions

UINT CSoundFile::GetNoteFromPeriod(UINT period) const
//---------------------------------------------------
{
	if (!period) return 0;
	if (m_nType & (MOD_TYPE_MED|MOD_TYPE_MOD|MOD_TYPE_MTM|MOD_TYPE_669|MOD_TYPE_OKT|MOD_TYPE_AMF0))
	{
		period >>= 2;
		for (UINT i=0; i<6*12; i++)
		{
			if (period >= ProTrackerPeriodTable[i])
			{
				if ((period != ProTrackerPeriodTable[i]) && (i))
				{
					UINT p1 = ProTrackerPeriodTable[i-1];
					UINT p2 = ProTrackerPeriodTable[i];
					if (p1 - period < (period - p2)) return i+36;
				}
				return i+1+36;
			}
		}
		return 6*12+36;
	} else
	{
		for (UINT i=1; i<NOTE_MAX; i++)
		{
			LONG n = GetPeriodFromNote(i, 0, 0);
			if ((n > 0) && (n <= (LONG)period)) return i;
		}
		return NOTE_MAX;
	}
}



UINT CSoundFile::GetPeriodFromNote(UINT note, int nFineTune, UINT nC5Speed) const
//-------------------------------------------------------------------------------
{
	if ((!note) || (note >= NOTE_MIN_SPECIAL)) return 0;
	if (m_nType & (MOD_TYPE_IT|MOD_TYPE_MPT|MOD_TYPE_S3M|MOD_TYPE_STM|MOD_TYPE_MDL|MOD_TYPE_ULT|MOD_TYPE_WAV
				|MOD_TYPE_FAR|MOD_TYPE_DMF|MOD_TYPE_PTM|MOD_TYPE_AMS|MOD_TYPE_DBM|MOD_TYPE_AMF|MOD_TYPE_PSM))
	{
		note--;
		if (m_dwSongFlags & SONG_LINEARSLIDES)
		{
			return (FreqS3MTable[note % 12] << 5) >> (note / 12);
		} else
		{
			if (!nC5Speed) nC5Speed = 8363;
			//(a*b)/c
			return _muldiv(8363, (FreqS3MTable[note % 12] << 5), nC5Speed << (note / 12));
			//8363 * freq[note%12] / nC5Speed * 2^(5-note/12)
		}
	} else
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (note < 13) note = 13;
		note -= 13;
		if (m_dwSongFlags & SONG_LINEARSLIDES)
		{
			LONG l = ((NOTE_MAX - note) << 6) - (nFineTune / 2);
			if (l < 1) l = 1;
			return (UINT)l;
		} else
		{
			int finetune = nFineTune;
			UINT rnote = (note % 12) << 3;
			UINT roct = note / 12;
			int rfine = finetune / 16;
			int i = rnote + rfine + 8;
			if (i < 0) i = 0;
			if (i >= 104) i = 103;
			UINT per1 = XMPeriodTable[i];
			if ( finetune < 0 )
			{
				rfine--;
				finetune = -finetune;
			} else rfine++;
			i = rnote+rfine+8;
			if (i < 0) i = 0;
			if (i >= 104) i = 103;
			UINT per2 = XMPeriodTable[i];
			rfine = finetune & 0x0F;
			per1 *= 16-rfine;
			per2 *= rfine;
			return ((per1 + per2) << 1) >> roct;
		}
	} else
	{
		note--;
		nFineTune = XM2MODFineTune(nFineTune);
		if ((nFineTune) || (note < 36) || (note >= 36+6*12))
			return (ProTrackerTunedPeriods[nFineTune*12 + note % 12] << 5) >> (note / 12);
		else
			return (ProTrackerPeriodTable[note-36] << 2);
	}
}


UINT CSoundFile::GetFreqFromPeriod(UINT period, UINT nC5Speed, int nPeriodFrac) const
//-----------------------------------------------------------------------------------
{
	if (!period) return 0;
	if (m_nType & (MOD_TYPE_MED|MOD_TYPE_MOD|MOD_TYPE_MTM|MOD_TYPE_669|MOD_TYPE_OKT|MOD_TYPE_AMF0))
	{
		return (3546895L*4) / period;
	} else
	if (m_nType & (MOD_TYPE_XM|MOD_TYPE_MT2))
	{
		if (m_dwSongFlags & SONG_LINEARSLIDES)
			return XMLinearTable[period % 768] >> (period / 768);
		else
			return 8363 * 1712L / period;
	} else
	{
		if (m_dwSongFlags & SONG_LINEARSLIDES)
		{
			if (!nC5Speed) nC5Speed = 8363;
			return _muldiv(nC5Speed, 1712L << 8, (period << 8)+nPeriodFrac);
		} else
		{
			return _muldiv(8363, 1712L << 8, (period << 8)+nPeriodFrac);
		}
	}
}

UINT CSoundFile::GetBestMidiChan(MODCHANNEL *pChn) {
//--------------------------------------------------
	if (pChn && pChn->pModInstrument) {
		if (pChn->pModInstrument->nMidiChannel) {
			return (pChn->pModInstrument->nMidiChannel-1)&0x0F;
		}
	}
	return 0;
}

void CSoundFile::PortamentoMPT(MODCHANNEL* pChn, int param)
//---------------------------------------------------------
{
	//Behavior: Modifies portamento by param-steps on every tick.
	//Note that step meaning depends on tuning.

	pChn->m_PortamentoFineSteps += param;
	pChn->m_CalculateFreq = true;
}


void CSoundFile::PortamentoFineMPT(MODCHANNEL* pChn, int param)
//-------------------------------------------------------------
{
	//Behavior: Divides portamento change between ticks/row. For example
	//if Ticks/row == 6, and param == +-6, portamento goes up/down by one tuning-dependent
	//fine step every tick.

	if(m_nTickCount == 0)
		pChn->nOldFinePortaUpDown = 0;

	const int tickParam = (m_nTickCount + 1.0) * param / m_nMusicSpeed;
	pChn->m_PortamentoFineSteps += (param >= 0) ? tickParam - pChn->nOldFinePortaUpDown : tickParam + pChn->nOldFinePortaUpDown;
	if(m_nTickCount + 1 == m_nMusicSpeed)
		pChn->nOldFinePortaUpDown = abs(param);
	else
		pChn->nOldFinePortaUpDown = abs(tickParam);

	pChn->m_CalculateFreq = true;
}
