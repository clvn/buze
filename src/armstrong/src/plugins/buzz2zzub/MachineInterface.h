// Copyright (C) 1997-2009 Oskari Tammelin (ot@iki.fi)
// This header file may be used to write _freeware_ DLL "machines" for Buzz
// Using it for anything else is not allowed without a permission from the author
   
#ifndef __MACHINE_INTERFACE_H
#define __MACHINE_INTERFACE_H

#include <stdio.h>
#include <assert.h>
#include <string.h>

#define MI_VERSION				37
  
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;

double const PI = 3.14159265358979323846;

#define MAX_BUFFER_LENGTH		256			// in number of samples

// machine types
#define MT_MASTER				0 
#define MT_GENERATOR			1
#define MT_EFFECT				2

// special parameter values
#define NOTE_NO					0
#define NOTE_OFF				255
#define NOTE_MIN				1					// C-0
#define NOTE_MAX				((16 * 9) + 12)		// B-9
#define SWITCH_OFF				0
#define SWITCH_ON				1
#define SWITCH_NO				255
#define WAVE_MIN				1
#define WAVE_MAX				200
#define WAVE_NO					0

// CMachineParameter flags
#define MPF_WAVE				1
#define MPF_STATE				2	
#define MPF_TICK_ON_EDIT		4				

// CMachineInfo flags
#define MIF_MONO_TO_STEREO		(1<<0)
#define MIF_PLAYS_WAVES			(1<<1)
#define MIF_USES_LIB_INTERFACE	(1<<2)
#define MIF_USES_INSTRUMENTS	(1<<3)
#define MIF_DOES_INPUT_MIXING	(1<<4)
#define MIF_NO_OUTPUT			(1<<5)		// used for effect machines that don't actually use any outputs (WaveOutput, AuxSend etc.)
#define MIF_CONTROL_MACHINE		(1<<6)		// used to control other (non MIF_CONTROL_MACHINE) machines
#define MIF_INTERNAL_AUX		(1<<7)		// uses internal aux bus (jeskola mixer and jeskola mixer aux)
#define MIF_EXTENDED_MENUS		(1<<8)		// uses submenus in machine menus. syntax: "submenu/another submenu/item"
#define MIF_PATTERN_EDITOR		(1<<9)		// implements it's own pattern editor, does not use buzz patterns
#define MIF_PE_NO_CLIENT_EDGE	(1<<10)		// remove sunken border from the pattern editor window
#define MIF_GROOVE_CONTROL		(1<<11)		
#define MIF_DRAW_PATTERN_BOX	(1<<12)
#define MIF_STEREO_EFFECT		(1<<13)
#define MIF_MULTI_IO			(1<<14)

// work modes
#define WM_NOIO					0
#define WM_READ					1
#define WM_WRITE				2
#define WM_READWRITE			3

// state flags
#define SF_PLAYING				1
#define SF_RECORDING			2


enum BEventType
{
	DoubleClickMachine,					// return true to ignore default handler (open parameter dialog), no parameters
	gDeleteMachine,						// data = CMachine *, param = ThisMac
	gAddMachine,						// data = CMachine *, param = ThisMac
	gRenameMachine,						// data = CMachine *, param = ThisMac
	gUndeleteMachine,					// data = CMachine *, param = ThisMac
	gWaveChanged						// (int)data = wave number
};

class CMachineInterface;
typedef bool (CMachineInterface::*EVENT_HANDLER_PTR)(void *);


enum CMPType { pt_note, pt_switch, pt_byte, pt_word, pt_internal=127 };		// pt_internal can be used by machines internally. don't use it in CMachineParameter.

class CMachineParameter
{
public:

	CMPType Type;			// pt_byte
	char const *Name;		// Short name: "Cutoff"
	char const *Description;// Longer description: "Cutoff Frequency (0-7f)"
	int MinValue;			// 0
	int MaxValue;			// 127
	int NoValue;			// 255
	int Flags;
	int DefValue;			// default value for params that have MPF_STATE flag set
};

class CMachineAttribute
{
public:
	char const *Name;
	int MinValue;
	int MaxValue;
	int DefValue;
};

class CMasterInfo
{
public:
	int BeatsPerMin;		// [16..500] 	
	int TicksPerBeat;		// [1..32]
	int SamplesPerSec;		// usually 44100, but machines should support any rate from 11050 to 96000
	int SamplesPerTick;		// (int)((60 * SPS) / (BPM * TPB))  
	int PosInTick;			// [0..SamplesPerTick-1]
	float TicksPerSec;		// (float)SPS / (float)SPT  

	// do not write to these values directly, use pCB->SetGroovePattern()
	int GrooveSize;			
	int PosInGroove;		// [0..GrooveSize-1]
	float *GrooveData;		// GrooveSize floats containing relative lengths of ticks

};

class CSubTickInfo
{
public:
	int SubTicksPerTick;
	int CurrentSubTick;		// [0..SubTicksPerTick-1]
	int SamplesPerSubTick;
	int PosInSubTick;		// [0..SamplesPerSubTick-1]
};

// CWaveInfo flags
#define WF_LOOP			1
#define WF_NOT16BIT		4
#define WF_STEREO		8
#define WF_BIDIR_LOOP	16

#define WFE_32BITF		1
#define WFE_32BITI		2
#define WFE_24BIT		3

class CWaveInfo
{
public:
	int Flags;
	float Volume;

};

class CWaveLevel
{
public:
	int numSamples;
	short *pSamples;
	int RootNote;
	int SamplesPerSec;
	int LoopStart;
	int LoopEnd;
};

// oscillator waveforms (used with GetOscillatorTable function)
#define OWF_SINE			0
#define OWF_SAWTOOTH		1
#define OWF_PULSE			2		// square 
#define OWF_TRIANGLE		3
#define OWF_NOISE			4	
#define OWF_303_SAWTOOTH	5

// each oscillator table contains one full cycle of a bandlimited waveform at 11 levels
// level 0 = 2048 samples  
// level 1 = 1024 samples
// level 2 = 512 samples
// ... 
// level 9 = 8 samples 
// level 10 = 4 samples
// level 11 = 2 samples
//
// the waves are normalized to 16bit signed integers   
//
// GetOscillatorTable retusns pointer to a table 
// GetOscTblOffset returns offset in the table for a specified level 
 
inline int GetOscTblOffset(int const level)
{
	assert(level >= 0 && level <= 10);
	return (2048+1024+512+256+128+64+32+16+8+4) & ~((2048+1024+512+256+128+64+32+16+8+4) >> level);
}

class CPattern;
class CSequence;
class CMachineInterfaceEx;
class CMachine;

class CMachineDataOutput;
class CMachineInfo;

class CDrawPatternBoxContext
{
public:
	CPattern *Pattern;
	void *HDC;
	int Left;
	int Top;
	int Width;
	int Height;
	float TicksPerPixel;
};

class CPatternWriteInfo
{
public:
	int Row;					// pattern editor machine's internal row
	float BuzzTickPosition;		// offset in number of buzz ticks (not rows) from the start of the row
};

enum MidiInputMode { MIM_Immediate, MIM_AudioThread, MIM_AudioThreadSubTick };

class CMICallbacks
{
public:
	virtual CWaveInfo const *GetWave(int const i) = 0;
	virtual CWaveLevel const *GetWaveLevel(int const i, int const level) = 0;
	virtual void MessageBox(char const *txt) = 0;
	virtual void Lock() = 0;
	virtual void Unlock() = 0;
	virtual int GetWritePos() = 0;			
	virtual int GetPlayPos() = 0;	
	virtual float *GetAuxBuffer() = 0;
	virtual void ClearAuxBuffer() = 0;
	virtual int GetFreeWave() = 0;
	virtual bool AllocateWave(int const i, int const size, char const *name) = 0;
	virtual void ScheduleEvent(int const time, dword const data) = 0;
	virtual void MidiOut(int const dev, dword const data) = 0;
	virtual short const *GetOscillatorTable(int const waveform) = 0;

	// envelopes
	virtual int GetEnvSize(int const wave, int const env) = 0;
	virtual bool GetEnvPoint(int const wave, int const env, int const i, word &x, word &y, int &flags) = 0;

	virtual CWaveLevel const *GetNearestWaveLevel(int const i, int const note) = 0;
	
	// pattern editing
	virtual void SetNumberOfTracks(int const n) = 0;
	virtual CPattern *CreatePattern(char const *name, int const length) = 0;
	virtual CPattern *GetPattern(int const index) = 0;
	virtual char const *GetPatternName(CPattern *ppat) = 0;
	virtual void RenamePattern(char const *oldname, char const *newname) = 0;
	virtual void DeletePattern(CPattern *ppat) = 0;
	virtual int GetPatternData(CPattern *ppat, int const row, int const group, int const track, int const field) = 0;
	virtual void SetPatternData(CPattern *ppat, int const row, int const group, int const track, int const field, int const value) = 0;
 		
	// sequence editing
	virtual CSequence *CreateSequence() = 0;
	virtual void DeleteSequence(CSequence *pseq) = 0;
	

	// special ppat values for GetSequenceData and SetSequenceData 
	// empty = NULL
	// <break> = (CPattern *)1
	// <mute> = (CPattern *)2
	// <thru> = (CPattern *)3
	virtual CPattern *GetSequenceData(int const row) = 0;
	virtual void SetSequenceData(int const row, CPattern *ppat) = 0;
		

	// buzz v1.2 (MI_VERSION 15) additions start here
	
	virtual void SetMachineInterfaceEx(CMachineInterfaceEx *pex) = 0;
	// group 1=global, 2=track
	virtual void ControlChange__obsolete__(int group, int track, int param, int value) = 0;						// set value of parameter
	
	// direct calls to audiodriver, used by WaveInput and WaveOutput
	// shouldn't be used for anything else
	virtual int ADGetnumChannels(bool input) = 0;
	virtual void ADWrite(int channel, float *psamples, int numsamples) = 0;
	virtual void ADRead(int channel, float *psamples, int numsamples) = 0;

	virtual CMachine *GetThisMachine() = 0;	// only call this in Init()!
	virtual void ControlChange(CMachine *pmac, int group, int track, int param, int value) = 0;		// set value of parameter (group & 16 == don't record)

	// returns pointer to the sequence if there is a pattern playing
	virtual CSequence *GetPlayingSequence(CMachine *pmac) = 0;

	// gets ptr to raw pattern data for row of a track of a currently playing pattern (or something like that)
	virtual void *GetPlayingRow(CSequence *pseq, int group, int track) = 0;

	virtual int GetStateFlags() = 0;

	virtual void SetnumOutputChannels(CMachine *pmac, int n) = 0;	// if n=1 Work(), n=2 WorkMonoToStereo()

	virtual void SetEventHandler(CMachine *pmac, BEventType et, EVENT_HANDLER_PTR p, void *param) = 0;

	virtual char const *GetWaveName(int const i) = 0;

	virtual void SetInternalWaveName(CMachine *pmac, int const i, char const *name) = 0;	// i >= 1, NULL name to clear

	virtual void GetMachineNames(CMachineDataOutput *pout) = 0;		// *pout will get one name per Write()
	virtual CMachine *GetMachine(char const *name) = 0;
	virtual CMachineInfo const *GetMachineInfo(CMachine *pmac) = 0;
	virtual char const *GetMachineName(CMachine *pmac) = 0;

	virtual bool GetInput(int index, float *psamples, int numsamples, bool stereo, float *extrabuffer) = 0;

	// MI_VERSION 16

	virtual int GetHostVersion() = 0;	// available if GetNearestWaveLevel(-2, -2) returns non-zero

	// if host version >= 2
	virtual int GetSongPosition() = 0;
	virtual void SetSongPosition(int pos) = 0;
	virtual int GetTempo() = 0;
	virtual void SetTempo(int bpm) = 0;
	virtual int GetTPB() = 0;
	virtual void SetTPB(int tpb) = 0;
	virtual int GetLoopStart() = 0;
	virtual int GetLoopEnd() = 0;
	virtual int GetSongEnd() = 0;
	virtual void Play() = 0;
	virtual void Stop() = 0;
	virtual bool RenameMachine(CMachine *pmac, char const *name) = 0;	// returns false if name is invalid
	virtual void SetModifiedFlag() = 0;
	virtual int GetAudioFrame() = 0;
	virtual bool HostMIDIFiltering() = 0;	// if true, the machine should always accept midi messages on all channels
	virtual dword GetThemeColor(char const *name) = 0;
	virtual void WriteProfileInt(char const *entry, int value) = 0;
	virtual void WriteProfileString(char const *entry, char const *value) = 0;
	virtual void WriteProfileBinary(char const *entry, byte *data, int nbytes) = 0;
	virtual int GetProfileInt(char const *entry, int defvalue) = 0;
	virtual void GetProfileString(char const *entry, char const *value, char const *defvalue) = 0;	
	virtual void GetProfileBinary(char const *entry, byte **data, int *nbytes) = 0;
	virtual void FreeProfileBinary(byte *data) = 0;
	virtual int GetNumTracks(CMachine *pmac) = 0;
	virtual void SetNumTracks(CMachine *pmac, int n) = 0;		// bonus trivia question: why is calling this SetNumberOfTracks not a good idea?
	virtual void SetPatternEditorStatusText(int pane, char const *text) = 0;
	virtual char const *DescribeValue(CMachine *pmac, int const param, int const value) = 0;
	virtual int GetBaseOctave() = 0;
	virtual int GetSelectedWave() = 0;
	virtual void SelectWave(int i) = 0;
	virtual void SetPatternLength(CPattern *p, int length) = 0;
	virtual int GetParameterState(CMachine *pmac, int group, int track, int param) = 0;
	virtual void ShowMachineWindow(CMachine *pmac, bool show) = 0;
	virtual void SetPatternEditorMachine(CMachine *pmac, bool gotoeditor) = 0;
	virtual CSubTickInfo const *GetSubTickInfo() = 0;		// returns NULL if subtick timing is disabled in buzz options
	virtual int GetSequenceColumn(CSequence *s) = 0;		// returns zero-based index to the columns in the editor or -1 if s is not a valid sequence pointer
	virtual void SetGroovePattern(float *data, int size) = 0;	// call only in CMachineInterface::Tick()
	virtual void ControlChangeImmediate(CMachine *pmac, int group, int track, int param, int value) = 0;
	virtual void SendControlChanges(CMachine *pmac) = 0;
	virtual int GetAttribute(CMachine *pmac, int index) = 0;
	virtual void SetAttribute(CMachine *pmac, int index, int value) = 0;
	virtual void AttributesChanged(CMachine *pmac) = 0;
	virtual void GetMachinePosition(CMachine *pmac, float &x, float &y) = 0;
	virtual void SetMachinePosition(CMachine *pmac, float x, float y) = 0;
	virtual void MuteMachine(CMachine *pmac, bool mute) = 0;
	virtual void SoloMachine(CMachine *pmac) = 0;
	virtual void UpdateParameterDisplays(CMachine *pmac) = 0;
	virtual void WriteLine(char const *text) = 0;	// write to debug console
	virtual bool GetOption(char const *name) = 0;	// returns the state of a View->Options item
	virtual bool GetPlayNotesState() = 0;
	virtual void EnableMultithreading(bool enable) = 0;		// enable/disable multithreaded Work calls for the machine. can be called at any time.
	virtual CPattern *GetPatternByName(CMachine *pmac, char const *patname) = 0;
	virtual void SetPatternName(CPattern *p, char const *name) = 0;
	virtual int GetPatternLength(CPattern *p) = 0;
	virtual CMachine *GetPatternOwner(CPattern *p) = 0;
	virtual bool MachineImplementsFunction(CMachine *pmac, int vtblindex, bool miex) = 0;
	virtual void SendMidiNote(CMachine *pmac, int const channel, int const value, int const velocity) = 0;
	virtual void SendMidiControlChange(CMachine *pmac, int const ctrl, int const channel, int const value) = 0;
	virtual int GetBuildNumber() = 0;
	virtual void SetMidiFocus(CMachine *pmac) = 0;
	virtual void BeginWriteToPlayingPattern(CMachine *pmac, int quantization, CPatternWriteInfo &outpwi) = 0;
	virtual void WriteToPlayingPattern(CMachine *pmac, int group, int track, int param, int value) = 0;
	virtual void EndWriteToPlayingPattern(CMachine *pmac) = 0;
	virtual void *GetMainWindow() = 0;	// returns HWND
	virtual void DebugLock(char const *sourcelocation) = 0;
	virtual void SetInputChannelCount(int count) = 0;		// MIF_MULTI_IO
	virtual void SetOutputChannelCount(int count) = 0;		// MIF_MULTI_IO
	virtual bool IsSongClosing() = 0;
	virtual void SetMidiInputMode(MidiInputMode mode) = 0;
	virtual int RemapLoadedMachineParameterIndex(CMachine *pmac, int i) = 0;	// returns -1 if no mapping found, see pattern xp for example usage
	virtual char const *GetThemePath() = 0;
	virtual void InvalidateParameterValueDescription(CMachine *pmac, int index) = 0;
	virtual void RemapLoadedMachineName(char *name, int bufsize) = 0;			// needed by machines that refer other machines by name when importing (machines may change names)
	virtual bool IsMachineMuted(CMachine *pmac) = 0;
	virtual int GetInputChannelConnectionCount(CMachine *pmac, int channel) = 0;
	virtual int GetOutputChannelConnectionCount(CMachine *pmac, int channel) = 0;
	virtual void ToggleRecordMode() = 0;
};


class CLibInterface
{
public:
	virtual void GetInstrumentList(CMachineDataOutput *pout) {}			
	
	// make some space to vtable so this interface can be extended later 
	virtual void Dummy1() {}
	virtual void Dummy2() {}
	virtual void Dummy3() {}
	virtual void Dummy4() {}
	virtual void Dummy5() {}
	virtual void Dummy6() {}
	virtual void Dummy7() {}
	virtual void Dummy8() {}
	virtual void Dummy9() {}
	virtual void Dummy10() {}
	virtual void Dummy11() {}
	virtual void Dummy12() {}
	virtual void Dummy13() {}
	virtual void Dummy14() {}
	virtual void Dummy15() {}
	virtual void Dummy16() {}
	virtual void Dummy17() {}
	virtual void Dummy18() {}
	virtual void Dummy19() {}
	virtual void Dummy20() {}
	virtual void Dummy21() {}
	virtual void Dummy22() {}
	virtual void Dummy23() {}
	virtual void Dummy24() {}
	virtual void Dummy25() {}
	virtual void Dummy26() {}
	virtual void Dummy27() {}
	virtual void Dummy28() {}
	virtual void Dummy29() {}
	virtual void Dummy30() {}
	virtual void Dummy31() {}
	virtual void Dummy32() {}


};


class CMachineInfo
{
public:
	int Type;								// MT_GENERATOR or MT_EFFECT, 
	int Version;							// MI_VERSION
											// v1.2: high word = internal machine version
											// higher version wins if two copies of machine found
	int Flags;				
	int minTracks;							// [0..256] must be >= 1 if numTrackParameters > 0 
	int maxTracks;							// [minTracks..256] 
	int numGlobalParameters;				
	int numTrackParameters;					
	CMachineParameter const **Parameters;
	int numAttributes;
	CMachineAttribute const **Attributes;
	char const *Name;						// "Jeskola Reverb"
	char const *ShortName;					// "Reverb"
	char const *Author;						// "Oskari Tammelin"
	char const *Commands;					// "Command1\nCommand2\nCommand3..."
	CLibInterface *pLI;						// ignored if MIF_USES_LIB_INTERFACE is not set
};

class CMachineDataInput
{
public:
	virtual void Read(void *pbuf, int const numbytes) = 0;

	void Read(int &d) { Read(&d, sizeof(int)); }
	void Read(dword &d) { Read(&d, sizeof(dword)); }
	void Read(short &d) { Read(&d, sizeof(short)); }
	void Read(word &d) { Read(&d, sizeof(word)); }
	void Read(char &d) { Read(&d, sizeof(char)); }
	void Read(byte &d) { Read(&d, sizeof(byte)); }
	void Read(float &d) { Read(&d, sizeof(float)); }
	void Read(double &d) { Read(&d, sizeof(double)); }
	void Read(bool &d) { Read(&d, sizeof(bool)); }

#ifdef _AFX

	CString ReadString()
	{
		CString s;

		while(true)
		{
			char ch;
			Read(ch);
			if (ch == 0)
				break;
			s += ch;
		}

		return s;
	}

#endif

};

class CMachineDataOutput
{
public:
	virtual void Write(void *pbuf, int const numbytes) = 0;

	void Write(int d) { Write(&d, sizeof(int)); }
	void Write(dword d) { Write(&d, sizeof(dword)); }
	void Write(short d) { Write(&d, sizeof(short)); }
	void Write(word d) { Write(&d, sizeof(word)); }
	void Write(char d) { Write(&d, sizeof(char)); }
	void Write(byte d) { Write(&d, sizeof(byte)); }
	void Write(float d) { Write(&d, sizeof(float)); }
	void Write(double d) { Write(&d, sizeof(double)); }
	void Write(bool d) { Write(&d, sizeof(bool)); }
	void Write(char const *str) { Write((void *)str, strlen(str)+1); }

};

// envelope info flags
#define EIF_SUSTAIN			1
#define EIF_LOOP			2

class CEnvelopeInfo
{
public:
	char const *Name;
	int Flags;
};

class CMachineInterface
{
public:
	virtual ~CMachineInterface() {}
	virtual void Init(CMachineDataInput * const pi) {}
	virtual void Tick() {}
	virtual bool Work(float *psamples, int numsamples, int const mode) { return false; }
	virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode) { return false; }
	virtual void Stop() {}
	virtual void Save(CMachineDataOutput * const po) {}
	virtual void AttributesChanged() {}
	virtual void Command(int const i) {}

	virtual void SetNumTracks(int const n) {}
	virtual void MuteTrack(int const i) {}
	virtual bool IsTrackMuted(int const i) const { return false; }

	virtual void MidiNote(int const channel, int const value, int const velocity) {}
	virtual void Event(dword const data) {}

	virtual char const *DescribeValue(int const param, int const value) { return NULL; }

	virtual CEnvelopeInfo const **GetEnvelopeInfos() { return NULL; }

	virtual bool PlayWave(int const wave, int const note, float const volume) { return false; }
	virtual void StopWave() {}
	virtual int GetWaveEnvPlayPos(int const env) { return -1; }


public:
	// initialize these members in the constructor 
	void *GlobalVals;
	void *TrackVals;
	int *AttrVals;
		
	// these members are initialized by the 
	// engine right after it calls CreateMachine()
	// don't touch them in the constructor
	CMasterInfo *pMasterInfo;
	CMICallbacks *pCB;					

};

// buzz v1.2 extended machine interface
class CMachineInterfaceEx
{
public:
	virtual char const *DescribeParam(int const param) { return NULL; }		// use this to dynamically change name of parameter
	virtual bool SetInstrument(char const *name) { return false; }

	virtual void GetSubMenu(int const i, CMachineDataOutput *pout) {}

	virtual void AddInput(char const *macname, bool stereo) {}	// called when input is added to a machine
	virtual void DeleteInput(char const *macename) {}			
	virtual void RenameInput(char const *macoldname, char const *macnewname) {}

	virtual void Input(float *psamples, int numsamples, float amp) {} // if MIX_DOES_INPUT_MIXING

	virtual void MidiControlChange(int const ctrl, int const channel, int const value) {}

	virtual void SetInputChannels(char const *macname, bool stereo) {}

	virtual bool HandleInput(int index, int amp, int pan) { return false; }

	// if MIF_PATTERN_EDITOR
	virtual void CreatePattern(CPattern *p, int numrows) {}
	virtual void CreatePatternCopy(CPattern *pnew, CPattern const *pold) {}
	virtual void DeletePattern(CPattern *p) {}
	virtual void RenamePattern(CPattern *p, char const *name) {}
	virtual void SetPatternLength(CPattern *p, int length) {}
	virtual void PlayPattern(CPattern *p, CSequence *s, int offset) {}
	virtual void *CreatePatternEditor(void *parenthwnd) { return NULL; }		// must return a HWND or NULL
	virtual void SetEditorPattern(CPattern *p) {}
	virtual void AddTrack() {}
	virtual void DeleteLastTrack() {}
	virtual bool EnableCommandUI(int id) { return false; }
	virtual void DrawPatternBox(CDrawPatternBoxContext *ctx) {}
	virtual void SetPatternTargetMachine(CPattern *p, CMachine *pmac) {}

	virtual void *CreateEmbeddedGUI(void *parenthwnd) { return NULL; }			// must return a HWND or NULL
	virtual void SelectWave(int i) {}
	
	virtual void SetDeletedState(bool deleted) {}

	virtual bool ShowPatternProperties() { return false; }
	virtual bool ImportPattern(CPattern *p) { return false; }

	virtual int GetLatency() { return 0; }		// for machine delay compensation

	virtual void RecordControlChange(CMachine *pmac, int group, int track, int param, int value) {}		// for pattern editors
	virtual void GotMidiFocus() {}
	virtual void LostMidiFocus() {}

	virtual void BeginWriteToPlayingPattern(CMachine *pmac, int quantization, CPatternWriteInfo &outpwi) {}
	virtual void WriteToPlayingPattern(CMachine *pmac, int group, int track, int param, int value) {}
	virtual void EndWriteToPlayingPattern(CMachine *pmac) {}

	virtual bool ShowPatternEditorHelp() { return false; }
	virtual void SetBaseOctave(int bo) {}

	virtual int GetEditorPatternPosition() { return 0; }	// cursor position in ticks

	virtual void MultiWork(float const * const *inputs, float **outputs, int numsamples) {}	// MIF_MULTI_IO
	virtual char const *GetChannelName(bool input, int index) { return NULL; }			// MIF_MULTI_IO

	// make some space to vtable so this interface can be extended later 
	virtual void Dummy1() {}
	virtual void Dummy2() {}
	virtual void Dummy3() {}
	virtual void Dummy4() {}
	virtual void Dummy5() {}
	virtual void Dummy6() {}
	virtual void Dummy7() {}
	virtual void Dummy8() {}
	virtual void Dummy9() {}
	virtual void Dummy10() {}
	virtual void Dummy11() {}
	virtual void Dummy12() {}
	virtual void Dummy13() {}
	virtual void Dummy14() {}
	virtual void Dummy15() {}
	virtual void Dummy16() {}
	virtual void Dummy17() {}
	virtual void Dummy18() {}
	virtual void Dummy19() {}
	virtual void Dummy20() {}
	virtual void Dummy21() {}
	virtual void Dummy22() {}
	virtual void Dummy23() {}
	virtual void Dummy24() {}
	virtual void Dummy25() {}
	virtual void Dummy26() {}
	virtual void Dummy27() {}
	virtual void Dummy28() {}
	virtual void Dummy29() {}
	virtual void Dummy30() {}
	virtual void Dummy31() {}
	virtual void Dummy32() {}

};
 
class CMILock
{
public:
#ifdef _DEBUG
	CMILock(CMICallbacks *p, char const *sourcelocation) 
	{ 
		pCB = p;
		
		if (pCB->GetNearestWaveLevel(-2, -2) != 0 && pCB->GetHostVersion() >= 36)
			pCB->DebugLock(sourcelocation); 
		else
			pCB->Lock(); 
	}
#else
	CMILock(CMICallbacks *p) { pCB = p; pCB->Lock(); }
#endif
	~CMILock() { pCB->Unlock(); }
private:
	CMICallbacks *pCB;
};

#ifdef _DEBUG
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MACHINE_LOCK CMILock __machinelock(pCB, __FILE__ ": " __FUNCTION__ " (" TOSTRING(__LINE__) ")");
#define MACHINE_LOCK_PCB(pcb) CMILock __machinelock(pcb, __FILE__ ": " __FUNCTION__ " (" TOSTRING(__LINE__) ")");
#else
#define MACHINE_LOCK CMILock __machinelock(pCB);
#define MACHINE_LOCK_PCB(pcb) CMILock __machinelock(pcb);
#endif

#ifdef STATIC_BUILD

	typedef CMachineInfo const *(__cdecl *GET_INFO)();												
	typedef CMachineInterface *(__cdecl *CREATE_MACHINE)();										

	extern void RegisterMachine(CMachineInfo const *pmi, GET_INFO gi, CREATE_MACHINE cm);	

#define DLL_EXPORTS(INIT_FUNC)															\
	static CMachineInfo const * __cdecl GetInfo() { return &MacInfo; }					\
	static CMachineInterface * __cdecl CreateMachine() { return new mi; }				\
	void INIT_FUNC() { RegisterMachine(&MacInfo, GetInfo, CreateMachine); }				\


#define DLL_EXPORTS_NS(NS, INIT_FUNC) /* namespaced version */ 								\
	static CMachineInfo const * __cdecl GetInfo() { return &NS::MacInfo; }					\
	static CMachineInterface * __cdecl CreateMachine() { return new NS::mi; }				\
	void INIT_FUNC() { RegisterMachine(&NS::MacInfo, GetInfo, CreateMachine); }				\



#else

	#define DLL_EXPORTS extern "C" { \
	__declspec(dllexport) CMachineInfo const * __cdecl GetInfo() \
	{ \
		return &MacInfo; \
	} \
	__declspec(dllexport) CMachineInterface * __cdecl CreateMachine() \
	{ \
		return new mi; \
	} \
	} 

	#define DLL_EXPORTS_AFX extern "C" { \
	__declspec(dllexport) CMachineInfo const * __cdecl GetInfo() \
	{ \
		return &MacInfo; \
	} \
	__declspec(dllexport) CMachineInterface * __cdecl CreateMachine() \
	{ \
		AFX_MANAGE_STATE(AfxGetStaticModuleState()); \
		return new mi; \
	} \
	} 



#endif

#endif 
