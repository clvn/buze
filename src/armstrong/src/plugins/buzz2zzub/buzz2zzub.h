#pragma once

#define __BUZZ2ZZUB__
#define NO_ZZUB_MIXER_TYPE

namespace zzub {
	struct mixer;
};
typedef struct zzub::mixer zzub_mixer_t;

#include "MachineInterface.h"
#include "mdk.h"
#include "dsplib.h"
#include "player/player.h"
#include "mixing/mixer.h"
#include "mixing/connections.h"
//#include "mixing/ringbuffer.h"
#include "buzztypes.h"

class CMDKImplementation;

const int MAX_BUZZ_IO_CHANNELS = 34; //32;	// enough for anybody??

static const int WM_CUSTOMVIEW_CREATE = WM_USER+4;
static const int WM_CUSTOMVIEW_SET_CHILD = WM_USER+5;
static const int WM_CUSTOMVIEW_GET = WM_USER+6;
static const int WM_CUSTOMVIEW_FOCUS = WM_USER+7;
static const int WM_GET_THEME = WM_USER+8;
static const int WM_CUSTOMVIEW_SET_PRETRANSLATE = WM_USER+10;
static const int WM_FORWARD_PRETRANSLATE = WM_USER+11;

// WM_CUSTOMVIEW_CREATE
//	wParam zzub_plugin_t*
//	lParam label of view to open.
// if a view with the specified label exists, the method fails
// WM_CUSTOMVIEW_GET
//	wParam zzub_plugin_t*
//	lParam label of view to open.
// return the view with the specified label or NULL
// WM_CUSTOMVIEW_FOCUS
//	wParam zzub_plugin_t*
//	lParam label of view to open.
// return the view with the specified label or NULL
// WM_CUSTOMVIEW_SET_CHILD
//	wParam hCustomView
//	lParam hPluginWnd
// return the view with the specified label or NULL
// WM_CUSTOMVIEW_SET_PRETRANSLATE
//	wParam hCustomView
//	lParam lpCallback
// sets a callback function to PreTranslate window messages

namespace buzz2zzub {

struct buzzeditor;

struct plugininput {
	std::string name;
	bool stereo;
	bool has_buffer;
	float buffer[256*2*2];
	int writepos, readpos;
	float amp;
	int first_output, output_count;
	int first_input, input_count;
};

struct plugin : zzub::plugin, CMICallbacks {

	struct event_wrap {
		BEventType et;
		EVENT_HANDLER_PTR p;
		void* param;
	};

	CMachineInterface* machine;
	CMachineInterfaceEx* machine2;
	CMDKImplementation* implementation;
	buzzplugininfo* machineInfo;
	int output_channels, input_channels;
	int track_count;
	DWORD dwThreadID;
	std::vector<float> aux_buffer; 
	zzub::master_info* oldinfo;
	buzzeditor* editor;
	std::vector<CPattern*> patterns;
	std::vector<event_wrap> events;
	std::map<int, plugininput*> inputbyid;
	std::vector<plugininput*> inputs; // array of connections in the order they were added
	int pattern_editor_trigger_offset;
	int last_process_events_tick_position;
	bool dirty_inputs;
	// ringbuffer<std::vector<int> > dirty_waves;

	plugin(CMachineInterface* machine, buzzplugininfo* mi);
	~plugin();
	// zzub::plugin implementations
	
	static int armstrong_callback(zzub_player_t* player, zzub_plugin_t* machine, zzub_event_data_t* data, void* tag);
	virtual void destroy();
	virtual void init(zzub::archive *arc);
	virtual void process_controller_events();
	void transfer_hacked_plugin_states();
	void transfer_no_values();
	void transfer_last_values();
	int transfer_track(int group, int track, char* param_ptr, const std::vector<const zzub::parameter*>& params, bool stateonly = false, bool novalues = false);
	virtual void process_events();
	virtual bool process_offline(float **pin, float **pout, int *numsamples, int *channels, int *samplerate);
	virtual bool process_stereo(float **pin, float **pout, int numsamples, int mode);
	virtual bool process_stereo_multi(float **pin, float **pout, int numsamples, int mode);
	virtual bool process_stereo_mono(float **pin, float **pout, int numsamples, int mode);
	virtual bool process_stereo_stereo(float **pin, float **pout, int numsamples, int mode);
	virtual void stop();
	void loadplugin(zzub::instream *ins);
	virtual void load(zzub::archive *arc);
	virtual void save(zzub::archive *arc);
	virtual void attributes_changed();
	virtual void command(int index);
	virtual void set_track_count(int count);
	virtual void mute_track(int index);
	virtual bool is_track_muted(int index) const;
	virtual void event(unsigned int data);
	virtual const char * describe_value(int param, int value);
	virtual const zzub::envelope_info ** get_envelope_infos();
	virtual bool play_wave(int wave, int note, float volume, int offset, int length);
	virtual void stop_wave();
	virtual int get_wave_envelope_play_position(int env);
	virtual const char* describe_param(int param);
	virtual bool set_instrument(const char *name);
	virtual void get_sub_menu(int index, zzub::outstream *os);
	virtual void add_input(int connection_id);
	virtual void delete_input(int connection_id);
	virtual void rename_input(const char *oldname, const char *newname);
	virtual void input(int connection_id, int first_input, int first_output, int inputs, int outputs, int flags, float **samples, int size, float amp);
	virtual bool handle_input(int index, int amp, int pan);
	virtual void process_midi_events(zzub::midi_message* pin, int nummessages);
	virtual void get_midi_output_names(zzub::outstream *pout);
	virtual void set_stream_source(const char* resource){}
	virtual int get_latency();
	virtual int get_output_channel_count();
	virtual int get_input_channel_count();
	virtual const char* get_output_channel_name(int i);
	virtual const char* get_input_channel_name(int i);
	int get_plugin_version();
	void update_inputs();

	// CMICallbacks implementations
	virtual CWaveInfo const *GetWave(const int i);
	zzub::wave_level* get_wavelevel(zzub::wave_info* wave, int index);
	zzub::wave_level* get_nearest_wavelevel(zzub::wave_info* wave, int note);
	virtual CWaveLevel const *GetWaveLevel(const int i, const int level);
	virtual void MessageBox(char const *txt);
	virtual void Lock();
	virtual void Unlock();
	virtual int GetWritePos();
	virtual int GetPlayPos();
	virtual float *GetAuxBuffer();
	virtual void ClearAuxBuffer();
	virtual int GetFreeWave();
	virtual bool AllocateWave(int const i, int const size, char const *name);
	virtual void ScheduleEvent(int const time, dword const data);
	virtual void MidiOut(int const dev, dword const data);
	virtual short const *GetOscillatorTable(int const waveform);

	virtual int GetEnvSize(int const wave, int const env);
	virtual bool GetEnvPoint(int const wave, int const env, int const i, word &x, word &y, int &flags);
	virtual CWaveLevel const *GetNearestWaveLevel(int const wave, int const note);
	virtual void SetNumberOfTracks(int const n);
	virtual CPattern *CreatePattern(char const *name, int const length);
	virtual CPattern *GetPattern(int const index);
	virtual char const *GetPatternName(CPattern *ppat);
	virtual void RenamePattern(char const *oldname, char const *newname);
	virtual void DeletePattern(CPattern *ppat);
	virtual int GetPatternData(CPattern *ppat, int const row, int const group, int const track, int const field);
	virtual void SetPatternData(CPattern *ppat, int const row, int const group, int const track, int const field, int const value);	virtual CSequence *CreateSequence();
	virtual void DeleteSequence(CSequence *pseq);
	virtual CPattern *GetSequenceData(int const row);
	virtual void SetSequenceData(int const row, CPattern *ppat);
	virtual void SetMachineInterfaceEx(CMachineInterfaceEx *pex);
	virtual void ControlChange__obsolete__(int group, int track, int param, int value);
	virtual int ADGetnumChannels(bool input);
	virtual void ADWrite(int channel, float *psamples, int numsamples);
	virtual void ADRead(int channel, float *psamples, int numsamples);
	virtual CMachine *GetThisMachine();
	 // set value of parameter (group & 16 == don't record)
	virtual void ControlChange(CMachine *pmac, int group, int track, int param, int value);
	virtual CSequence *GetPlayingSequence(CMachine *pmac);
	virtual void *GetPlayingRow(CSequence *pseq, int group, int track);
	virtual int GetStateFlags();
	virtual void SetnumOutputChannels(CMachine *pmac, int n);
	virtual void set_input_channels(int from_plugin_id, int first_input, int first_output, int inputs, int outputs, int flags);
	virtual bool invoke(BEventType buzzevent, void* data);
	virtual void SetEventHandler(CMachine *pmac, BEventType et, EVENT_HANDLER_PTR p, void *param);
	virtual char const *GetWaveName(int const i);
	virtual void SetInternalWaveName(CMachine *pmac, int const i, char const *name);
	virtual void GetMachineNames(CMachineDataOutput *pout);
	virtual CMachine *GetMachine(char const *name);
	void convertToBuzzParameter(CMachineParameter* toparam, const zzub::parameter* fromparam);
	void convertToBuzzAttribute(CMachineAttribute* toattr, const zzub::attribute* fromattr);
	virtual CMachineInfo const *GetMachineInfo(CMachine *pmac);
	virtual char const *GetMachineName(CMachine *pmac);
	virtual bool GetInput(int index, float *psamples, int numsamples, bool stereo, float *extrabuffer);

	// MI_VERSION 16

	virtual int GetHostVersion();
	// if host version >= 2
	virtual int GetSongPosition();
	virtual void SetSongPosition(int pos);
	virtual int GetTempo();
	virtual void SetTempo(int bpm);
	virtual int GetTPB();
	virtual void SetTPB(int tpb);
	virtual int GetLoopStart();
	virtual int GetLoopEnd();
	virtual int GetSongEnd();
	virtual void Play();
	virtual void Stop();
	virtual bool RenameMachine(CMachine *pmac, char const *name);
	virtual void SetModifiedFlag();
	virtual int GetAudioFrame();
	virtual bool HostMIDIFiltering();
	virtual dword GetThemeColor(char const *name);
	virtual void WriteProfileInt(char const *entry, int value);
	virtual void WriteProfileString(char const *entry, char const *value);
	virtual void WriteProfileBinary(char const *entry, byte *data, int nbytes);
	virtual int GetProfileInt(char const *entry, int defvalue);
	virtual void GetProfileString(char const *entry, char const *value, char const *defvalue);
	virtual void GetProfileBinary(char const *entry, byte **data, int *nbytes);
	virtual void FreeProfileBinary(byte *data);
	virtual int GetNumTracks(CMachine *pmac);
	virtual void SetNumTracks(CMachine *pmac, int n);
	virtual void SetPatternEditorStatusText(int pane, char const *text);
	virtual char const *DescribeValue(CMachine *pmac, int const param, int const value);
	virtual int GetBaseOctave();
	virtual int GetSelectedWave();
	virtual void SelectWave(int i);
	virtual void SetPatternLength(CPattern *p, int length);
	virtual int GetParameterState(CMachine *pmac, int group, int track, int param);
	virtual void ShowMachineWindow(CMachine *pmac, bool show);
	virtual void SetPatternEditorMachine(CMachine *pmac, bool gotoeditor);
	// returns NULL if subtick timing is disabled in buzz options
	virtual CSubTickInfo const *GetSubTickInfo();
	// returns zero-based index to the columns in the editor or -1 if s is not a valid sequence pointer
	virtual int GetSequenceColumn(CSequence *s);
	// call only in CMachineInterface::Tick()
	virtual void SetGroovePattern(float *data, int size);
	virtual void ControlChangeImmediate(CMachine *pmac, int group, int track, int param, int value);
	virtual void SendControlChanges(CMachine *pmac);
	virtual int GetAttribute(CMachine *pmac, int index);
	virtual void SetAttribute(CMachine *pmac, int index, int value);
	virtual void AttributesChanged(CMachine *pmac);
	virtual void GetMachinePosition(CMachine *pmac, float &x, float &y);
	virtual void SetMachinePosition(CMachine *pmac, float x, float y);
	virtual void MuteMachine(CMachine *pmac, bool mute);
	virtual void SoloMachine(CMachine *pmac);
	virtual void UpdateParameterDisplays(CMachine *pmac);
	// write to debug console
	virtual void WriteLine(char const *text);
	// returns the state of a View->Options item
	virtual bool GetOption(char const *name);
	virtual bool GetPlayNotesState();
	// enable/disable multithreaded Work calls for the machine. can be called at any time.
	virtual void EnableMultithreading(bool enable);
	virtual CPattern *GetPatternByName(CMachine *pmac, char const *patname);
	virtual void SetPatternName(CPattern *p, char const *name);
	virtual int GetPatternLength(CPattern *p);
	virtual CMachine *GetPatternOwner(CPattern *p);
	virtual bool MachineImplementsFunction(CMachine *pmac, int vtblindex, bool miex);
	virtual void SendMidiNote(CMachine *pmac, int const channel, int const value, int const velocity);
	virtual void SendMidiControlChange(CMachine *pmac, int const ctrl, int const channel, int const value);
	virtual int GetBuildNumber();
	virtual void SetMidiFocus(CMachine *pmac);
	virtual void BeginWriteToPlayingPattern(CMachine *pmac, int quantization, CPatternWriteInfo &outpwi);
	virtual void WriteToPlayingPattern(CMachine *pmac, int group, int track, int param, int value);
	virtual void EndWriteToPlayingPattern(CMachine *pmac);
	virtual void *GetMainWindow();	// returns HWND
	virtual void DebugLock(char const *sourcelocation);
	virtual void SetInputChannelCount(int count);		// MIF_MULTI_IO
	virtual void SetOutputChannelCount(int count);		// MIF_MULTI_IO
	virtual bool IsSongClosing();
	virtual void SetMidiInputMode(MidiInputMode mode);
	virtual int RemapLoadedMachineParameterIndex(CMachine *pmac, int i);	// returns -1 if no mapping found, see pattern xp for example usage
	virtual char const *GetThemePath();
	virtual void InvalidateParameterValueDescription(CMachine *pmac, int index);
	virtual void RemapLoadedMachineName(char *name, int bufsize);			// needed by machines that refer other machines by name when importing (machines may change names)
	virtual bool IsMachineMuted(CMachine *pmac);
	virtual int GetInputChannelConnectionCount(CMachine *pmac, int channel);
	virtual int GetOutputChannelConnectionCount(CMachine *pmac, int channel);
	virtual void ToggleRecordMode();
};

}
