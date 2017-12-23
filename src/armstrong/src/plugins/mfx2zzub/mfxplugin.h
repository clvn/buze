#pragma once

#include "mfxplugingui.h"

// for CComPtr<IMfxEventFilter>::CoCreateInstance()
interface __declspec(uuid("{8D7FDE01-1B1B-11d2-A8E0-0000A0090DAF}")) IMfxEventFilter;

struct mfxplugin
	: public CComObjectRootEx<CComSingleThreadModel>
	, public IMfxTempoMap 
	, public IMfxMeterMap 
	, public IMfxInputPulse
	, public IPropertyPageSite
	, zzub::plugin 
{
	CComPtr<IMfxEventFilter> eventFilter;
	CMfxPluginGui pluginGui;
	mfxplugininfo* info;
	zzub_player_state playerstate;

	BEGIN_COM_MAP(mfxplugin)
		COM_INTERFACE_ENTRY_IID(IID_IUnknown, IMfxTempoMap)
		COM_INTERFACE_ENTRY(IMfxTempoMap)
		COM_INTERFACE_ENTRY(IMfxMeterMap)
		COM_INTERFACE_ENTRY(IMfxInputPulse)
		COM_INTERFACE_ENTRY(IPropertyPageSite)
	END_COM_MAP()

	mfxplugin();

	// zzub::plugin implementation
	virtual void init(zzub::archive*);
	virtual void destroy();
	virtual void process_midi_events(zzub::midi_message* pin, int nummessages);
	virtual void get_midi_output_names(zzub::outstream *pout);
	virtual bool has_embedded_gui() { return true; }
	virtual bool create_embedded_gui(void* hwnd);
	virtual void resize_embedded_gui(void* hwnd, int* outwidth, int* outheight);

	void midi_out(int time, unsigned int data);

	// IMfxTempoMap implementation
	STDMETHODIMP_(LONG) TicksToMsecs(LONG lTicks);
	STDMETHODIMP_(LONG) MsecsToTicks(LONG lMsecs);
	STDMETHODIMP_(int) GetTicksPerQuarterNote();
	STDMETHODIMP_(int) GetTempoIndexForTime(LONG lTicks);
	STDMETHODIMP_(int) GetTempoCount();
	STDMETHODIMP GetTempoAt(int ix, LONG* plTicks, int* pnBPM100 );

	// IMfxMeterMap implementation
	STDMETHODIMP TicksToMBT(LONG lTicks, int* pnMeasure, int* pnBeat, int* pnTicks );
	STDMETHODIMP_(LONG) MBTToTicks(int nMeasure, int nBeat, int nTicks );
	STDMETHODIMP_(int) GetMeterIndexForTime(LONG lTicks );
	STDMETHODIMP_(int) GetMeterCount();
	STDMETHODIMP GetMeterAt(int ix, int* pnMeasure, int* pnTop, EBeatValue* eBottom );

	// IMfxInputPulse implementation
	STDMETHODIMP GetPulseInterval(LONG* plIntervalMsec);
	STDMETHODIMP BeginPulse();
	STDMETHODIMP EndPulse();

	// IPropertyPageSite implementation
	STDMETHODIMP OnStatusChange(DWORD dwFlags) { return S_OK; }
	STDMETHODIMP GetLocaleID(LCID *pLocaleID) { *pLocaleID = 0; return S_OK; }
	STDMETHODIMP GetPageContainer(IUnknown **ppUnk) { *ppUnk = NULL; return E_NOTIMPL; }
	STDMETHODIMP TranslateAccelerator(LPMSG pMsg) { /**pMsg = NULL;*/ return E_NOTIMPL; }

};
