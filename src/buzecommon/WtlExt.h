// WtlExt.h : WTL Extensions/Helpers
// Copyright: Jesús Salas 2002 ( jesuspsalas@hotmail.com )
// License  : none, feel free to use it.
/////////////////////////////////////////////////////////////////////////////

#pragma once

template <class T,class Interface>
class CWTLDispEventHelper : public IDispEventImpl<0,T>
{
	public:
		CComPtr<IUnknown> m_pUnk;
		HRESULT EasyAdvise(IUnknown* pUnk) 
		{  
			m_pUnk = pUnk;
			AtlGetObjectSourceInterface(pUnk,&m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
			return DispEventAdvise(pUnk, &m_iid);
		}
		HRESULT EasyUnadvise() 
		{ 
			AtlGetObjectSourceInterface(m_pUnk,&m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
			return DispEventUnadvise(pUnk, &m_iid);
	  }

};


template <class T, class Interface>
class CWTLAxControl :	public CComPtr<Interface>,
						public CWindowImpl<CWTLAxControl<T, Interface>,CAxWindow>, 
						public CWTLDispEventHelper<T,Interface>
{
	public:

	BEGIN_MSG_MAP(CWTLAxControl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
	END_MSG_MAP()

	LRESULT OnCreate( UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL & bHandled )
	{
		LRESULT lRet;
		// We must call DefWindowProc before we can attach to the control.
		lRet = DefWindowProc( uMsg, wParam,lParam );
		// Get the Pointer to the control with Events (true)
		AttachControl(true);
		return lRet;
	}


	HRESULT AttachControl( BOOL bWithEvents = false ) 
	{
		HRESULT hr = S_OK;
		CComPtr<IUnknown> spUnk;
		// Get the IUnknown interface of control
		hr |= AtlAxGetControl( m_hWnd, &spUnk);

		if (SUCCEEDED(hr))
			// Query our interface
			hr |= spUnk->QueryInterface( __uuidof(Interface), (void**) (CComPtr<Interface>*)this);

		if ( bWithEvents && ! FAILED(hr) )
			 // Start events
			 hr|= EasyAdvise( spUnk );

		return hr;
	};
};


#include <ExDispid.h>

#define EVENTFN void __stdcall
class CWTLIExplorer : public CWTLAxControl<CWTLIExplorer,IWebBrowser2>
{
	public:
		// BEGIN_MSG_MAP is optional, you don't need to define or use it if you don't want
		BEGIN_MSG_MAP( CWTLIExplorer )
			MESSAGE_HANDLER(WM_CREATE, OnCreate)			
		END_MSG_MAP()


		LRESULT OnCreate(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled)
		{
			// First you must CWTLAxControl<...,...>::OnCreate ( it set this message Handled )
			return CWTLAxControl<CWTLIExplorer,IWebBrowser2>::OnCreate( uMsg, wParam, lParam, bHandled );
		}

		// BEGIN_SINK_MAP is optional, you don't need to define or use it if you don't want
		BEGIN_SINK_MAP( CWTLIExplorer )
			SINK_ENTRY(0, DISPID_NAVIGATECOMPLETE2, &OnNavigateComplete2 )
		END_SINK_MAP()
		EVENTFN OnNavigateComplete2( IDispatch* pDisp,  VARIANT* URL )
		{
			MessageBox( "OnNavigateComplete2" );
		}
};
