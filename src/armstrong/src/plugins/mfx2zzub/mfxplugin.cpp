#define __BUZZ2ZZUB__
#define NO_ZZUB_MIXER_TYPE

namespace zzub {
	struct mixer;
};
typedef struct zzub::mixer zzub_mixer_t;

#define _ATL_NO_UUIDOF
#include <atlbase.h>
#include <atlcom.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <MidiFilter.h>
#include <zzub/plugin.h>
#include "plugincollection.h"
#include "mfxplugin.h"
#include "mixing/mixer.h"

using std::cerr;
using std::endl;

mfxplugin::mfxplugin() {
	info = 0;
}

void mfxplugin::init(zzub::archive*) {
	assert(info != 0);
	assert(m_dwRef > 0);

	if FAILED(eventFilter.CoCreateInstance(info->pluginClsid, 0, CLSCTX_INPROC)) {
		cerr << "mfx2zzub: eventfilter not found" << endl;
		return ;
	}

	IUnknown* self;
	if FAILED(QueryInterface(IID_IUnknown, (void**)&self)) {
		cerr << "QI failed" << endl;
		return ;
	}

	if FAILED(eventFilter->Connect(self)) {
		self->Release();
		cerr << "Connect failed" << endl;
		return ;
	}

	playerstate = zzub_player_state_stopped;
}

void mfxplugin::destroy() {
	if (eventFilter != 0) {
		eventFilter->Disconnect();
	}
	Release(); // causes delete this;
}

bool mfxplugin::create_embedded_gui(void* hwnd) {
	HWND hResult = pluginGui.Create((HWND)hwnd, 0, 0, WS_CHILD|WS_VISIBLE, 0, (HMENU)0, this);
	return hResult != 0;
}

void mfxplugin::resize_embedded_gui(void* hwnd, int* outwidth, int* outheight) {
	pluginGui.UpdateSize(outwidth, outheight);
}

struct dataqueue
	: public CComObjectRootEx<CComSingleThreadModel>
	, public IMfxDataQueue
{
	std::vector<MfxData> events;

	BEGIN_COM_MAP(dataqueue)
		COM_INTERFACE_ENTRY(IMfxDataQueue)
	END_COM_MAP()

	STDMETHODIMP Add(const MfxData& data ) {
		events.push_back(data);
		return S_OK;
	}

	// GetCount() provides the number of events in the queue.
	STDMETHODIMP GetCount(int* pnCount ) {
		*pnCount = (int)events.size();
		return S_OK;
	}

	// GetAt() provides the MfxEvent at index 'ix'.
	STDMETHODIMP GetAt(int ix, MfxData* pData ) {
		*pData = events[ix];
		return S_OK;
	}
};

struct eventqueue
	: public CComObjectRootEx<CComSingleThreadModel>
	, public IMfxEventQueue
{
	std::vector<MfxEvent> events;

	BEGIN_COM_MAP(eventqueue)
		COM_INTERFACE_ENTRY(IMfxEventQueue)
	END_COM_MAP()

	STDMETHODIMP Add(const MfxEvent& data ) {
		events.push_back(data);
		return S_OK;
	}

	// GetCount() provides the number of events in the queue.
	STDMETHODIMP GetCount(int* pnCount ) {
		*pnCount = (int)events.size();
		return S_OK;
	}

	// GetAt() provides the MfxEvent at index 'ix'.
	STDMETHODIMP GetAt(int ix, MfxEvent* pData ) {
		*pData = events[ix];
		return S_OK;
	}
};

#define MAKEMIDI(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))

void mfxplugin::midi_out(int time, unsigned int data) {
	zzub::midi_message msg = { -1, data, time };
	_mixer->midi_out(_id, msg);
}

bool parse_mfx_event(int time, unsigned int message, MfxEvent* result) {
	unsigned short status  = message & 0xff;
	unsigned char data1 = (message >> 8) & 0xff;
	unsigned char data2 = (message >> 16) & 0xff;

	int channel = status&0xF;
	int command = (status & 0xf0) >> 4;
	switch (command) {
		//case 8:
		case 9:
			// notes - ignoring noteoffs - we should rather know the note duration
			if (data2 != 0) {
				result->m_lTime = time;
				result->m_eType = MfxEvent::Note;
				result->m_byKey = data1;
				result->m_byVel = data2;
				result->m_byChan = channel;
				result->m_dwDuration = 1;
				return true;
			}
			break;
		case 0xb:
			// CC
			break;
		case 0xc:
			// program change
			//midi_value = data1;
			break;
		case 0xe:
			// pitch bend
			//midi_value = (data2 << 7) | data1;
			break;
		default:
			// unsupported midi command 
			break;
	}
	return false;
}

void mfxplugin::process_midi_events(zzub::midi_message* pin, int nummessages) {
	//pin[0].message 
	if (eventFilter == 0) 
		return ;

	int tickpos = (float)_master_info->tick_position / ((float)_master_info->samples_per_tick + _master_info->samples_per_tick_frac);
	int ppqPos = (float)(_master_info->row_position + tickpos) / (float)_master_info->ticks_per_beat;


	CComObjectStackEx<eventqueue> inevents;
	CComObjectStackEx<eventqueue> outevents;
	if (playerstate != _mixer->state) {
		playerstate = (zzub_player_state)_mixer->state;

		if (playerstate == zzub_player_state_playing) {
			eventFilter->OnStart(ppqPos, &outevents);
		} else
		if (playerstate == zzub_player_state_stopped) {
			eventFilter->OnStop(ppqPos, &outevents);
		}
	}

	CComObjectStackEx<dataqueue> indata;
	CComObjectStackEx<dataqueue> outdata;
	for (int i = 0; i < nummessages; i++) {
		MfxData d;
		d.m_lTime = ppqPos + pin[i].timestamp;
		d.m_dwData = pin[i].message;
		indata.events.push_back(d);

		MfxEvent e;
		if (parse_mfx_event(pin[i].timestamp, pin[i].message, &e))
			inevents.events.push_back(e);
	}
	
	eventFilter->OnInput(&indata, &outdata); // for raw midi input!

	if (_master_info->tick_position == 0 || inevents.events.size() != 0) {
		eventFilter->OnEvents(ppqPos, ppqPos + 1, &inevents, &outevents); // for sequencer data!

		for (int i = 0; i < outevents.events.size(); i++) {
			MfxData d;
			unsigned int status;
			unsigned int message;
			switch (outevents.events[i].m_eType) {
				case MfxEvent::Note:
					cerr << "Note!" << endl;
					d.m_lTime = outevents.events[i].m_lTime;
					status = 0x90 | (outevents.events[i].m_byChan & 0x0f);
					d.m_dwData = MAKEMIDI(status, outevents.events[i].m_byKey, outevents.events[i].m_byVel);
					outdata.events.push_back(d);
					break;
				case MfxEvent::Control:
					cerr << "Control!" << endl;
					break;
				case MfxEvent::RPN:
					cerr << "RPN!" << endl;
					break;
			}
			cerr << "got event at " << outevents.events[i].m_lTime << endl;

			//midi_out(outdata.events[i].m_lTime, outdata.events[i].m_dwData);
		}
	}

	for (int i = 0; i < outdata.events.size(); i++) {
		cerr << "got data at " << outdata.events[i].m_lTime << endl;
		midi_out(outdata.events[i].m_lTime - ppqPos, outdata.events[i].m_dwData);
	}
}

void mfxplugin::get_midi_output_names(zzub::outstream *pout) {
	std::string name = "MidiFX";
	pout->write((void*)name.c_str(), (unsigned int)(name.length()) + 1);
}

// IMfxTempoMap implementation
STDMETHODIMP_(LONG) mfxplugin::TicksToMsecs(LONG lTicks) {
	cerr << "mfx2zzub: TicksToMsecs" << endl;
	return 0;
}

STDMETHODIMP_(LONG) mfxplugin::MsecsToTicks(LONG lMsecs) {
	cerr << "mfx2zzub: MsecsToTicks" << endl;
	return 0;
}

STDMETHODIMP_(int) mfxplugin::GetTicksPerQuarterNote() {
	cerr << "mfx2zzub: GetTicksPerQuarterNote" << endl;
/*	int tickpos = (float)_master_info->tick_position / ((float)_master_info->samples_per_tick + _master_info->samples_per_tick_frac);
	int ppqPos = (float)(_master_info->row_position + tickpos) / (float)_master_info->ticks_per_beat;
*/
	return 4;
}

STDMETHODIMP_(int) mfxplugin::GetTempoIndexForTime(LONG lTicks) {
	cerr << "mfx2zzub: GetTempoIndexForTime" << endl;
	return 0;
}

STDMETHODIMP_(int) mfxplugin::GetTempoCount() {
	cerr << "mfx2zzub: GetTempoCount" << endl;
	return 0;
}

STDMETHODIMP mfxplugin::GetTempoAt(int ix, LONG* plTicks, int* pnBPM100 ) {
	cerr << "mfx2zzub: GetTempoAt" << endl;
	return E_NOTIMPL;
}

// IMfxMeterMap implementation
STDMETHODIMP mfxplugin::TicksToMBT(LONG lTicks, int* pnMeasure, int* pnBeat, int* pnTicks ) {
	cerr << "mfx2zzub: TicksToMBT" << endl;
	return E_NOTIMPL;
}

STDMETHODIMP_(LONG) mfxplugin::MBTToTicks(int nMeasure, int nBeat, int nTicks ) {
	cerr << "mfx2zzub: MBTToTicks" << endl;
	return 0;
}

// Get the index of the meter change in effect for time 'dwTicks'
STDMETHODIMP_(int) mfxplugin::GetMeterIndexForTime(LONG lTicks ) {
	cerr << "mfx2zzub: GetMeterIndexForTime" << endl;
	return 0;
}

// Get the number of meter changes in the map
STDMETHODIMP_(int) mfxplugin::GetMeterCount() {
	cerr << "mfx2zzub: GetMeterCount" << endl;
	return 0;
}

// Get a meter change. Returns:
// pnMeasure: the measure number of the meter change
// pnTop: the number of beats per measure
// pnBottom: the beat value (shift 1 left by this amount to get the
// beat value; for example, 2 means 1 << 2 or 4.
//enum EBeatValue { Beat1 = 0, Beat2 = 1, Beat4 = 2, Beat8 = 3, Beat16 = 4, Beat32 = 5 };
STDMETHODIMP mfxplugin::GetMeterAt(int ix, int* pnMeasure, int* pnTop, EBeatValue* eBottom ) {
	cerr << "mfx2zzub: GetMeterAt" << endl;
	return E_NOTIMPL;
}

// IMfxInputPulse implementation
STDMETHODIMP mfxplugin::GetPulseInterval(LONG* plIntervalMsec) {
	cerr << "mfx2zzub: GetPulseInterval" << endl;
	return E_NOTIMPL;
}

STDMETHODIMP mfxplugin::BeginPulse() {
	cerr << "mfx2zzub: BeginPulse" << endl;
	return S_OK;
}

STDMETHODIMP mfxplugin::EndPulse() {
	cerr << "mfx2zzub: EndPulse" << endl;
	return S_OK;
}
