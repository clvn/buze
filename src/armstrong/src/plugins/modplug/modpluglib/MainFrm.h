#pragma once

#define BEGIN_CRITICAL()		EnterCriticalSection(&CMainFrame::m_csAudio)
#define END_CRITICAL()			LeaveCriticalSection(&CMainFrame::m_csAudio)

class CMainFrame {
public:
	static CRITICAL_SECTION m_csAudio;
	static BYTE gbWFIRType;
	static double gdWFIRCutoff;
	static long glVolumeRampSamples;

};
