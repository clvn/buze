#pragma once

#import "envelope.ocx" no_namespace 

const DWORD WM_ENVELOPECHANGED = WM_USER + 1;

struct ECINFO {
    long numPoints;
    long sustainPoint;
    long* points;
};

class CEnvelopeCtrl : public CWTLAxControl<CEnvelopeCtrl, _DEnvelope> {
public:
	DECLARE_WND_SUPERCLASS("CEnvelopeCtrl", CAxWindow::GetWndClassName())

	BEGIN_MSG_MAP(CEnvelopeCtrl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate) 
    END_MSG_MAP() 
        
	BEGIN_SINK_MAP(CEnvelopeCtrl)
        SINK_ENTRY(0, 1, &EnvelopeChanged)
    END_SINK_MAP()

    LRESULT OnCreate(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled);
	HRESULT __stdcall EnvelopeChanged(long numPoints, long SustainPoint, long * Points);
};
