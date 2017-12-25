#include <atlbase.h>
#include <atlwin.h>
#include <wtl/atlapp.h>
#include <wtl/atlctrls.h>
#include "WTLExt.h"
#include "EnvelopeCtrl.h"

LRESULT CEnvelopeCtrl::OnCreate(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL& bHandled)  { 
	return CWTLAxControl<CEnvelopeCtrl,_DEnvelope>::OnCreate(uMsg, wParam, lParam, bHandled );
}

HRESULT __stdcall CEnvelopeCtrl::EnvelopeChanged(long numPoints, long SustainPoint, long * Points) {
	ECINFO eci;
	eci.numPoints = numPoints;
	eci.sustainPoint = SustainPoint;
	eci.points = Points;
	GetParent().SendMessage(WM_ENVELOPECHANGED, (WPARAM)&eci, (LPARAM)m_hWnd);
	return S_OK;
}
