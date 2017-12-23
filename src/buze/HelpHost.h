#pragma once

#include <exdispid.h>

class CHelpHost;
class CMainFrame;

typedef IDispEventImpl<1, CHelpHost, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 0> DEWebBrowserEvents2;

class CHelpHost
	: public CComObjectRoot
	, public IServiceProvider 
	, public DEWebBrowserEvents2
{
public:
	CMainFrame* mainframe;

	BEGIN_COM_MAP(CHelpHost)
		COM_INTERFACE_ENTRY_IID(IID_IDispatch, DEWebBrowserEvents2)
		COM_INTERFACE_ENTRY_IID(DIID_DWebBrowserEvents2, DEWebBrowserEvents2)
		COM_INTERFACE_ENTRY(IServiceProvider)
	END_COM_MAP()
        
	BEGIN_SINK_MAP(CHelpHost)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_NAVIGATECOMPLETE2, &OnNavigateComplete2)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOWNLOADCOMPLETE, &OnDownloadComplete)
		SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, &OnDocumentComplete)
    END_SINK_MAP()

	HRESULT __stdcall OnNavigateComplete2(LPDISPATCH pDisp, VARIANT* URL);
	HRESULT __stdcall OnDownloadComplete();
	HRESULT __stdcall OnDocumentComplete(LPDISPATCH pDisp, VARIANT* URL);
	STDMETHODIMP QueryService(const GUID& sid, const IID& iid, void** ppv);
};
