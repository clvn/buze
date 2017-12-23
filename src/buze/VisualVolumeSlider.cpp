#include "stdafx.h"
#include "resource.h"
#include "VisualVolumeSlider.h"

using namespace std;
namespace {

float linear_to_dB(float val, float limit) { 
	if(val <= 0)
		return limit;
	return(20.0f * log10(val)); 
}

float dB_to_percent(float val, float limit) {
	if(val >= limit)
		return 0;
	if(val <= 0)
		return 1;

	float f = val / limit;
	return float(1.0 - f);
}

}

/***

	CVisualVolumeSlider

***/

CVisualVolumeSlider::CVisualVolumeSlider() {
	maxL = maxR = 0;
	rPeak = lPeak = rDrop = lDrop = rTop = lTop = dropRate = 0;
	rPeakColor = lPeakColor = 0;
	counter = 0;
	percent12dB = dB_to_percent(12, VU_DB_LIMIT);
	percent6dB = dB_to_percent(6, VU_DB_LIMIT);
	is_master = false;
	current_value = 0;
	timerMod = 4;
	mode = drag_mode_none;
}

CVisualVolumeSlider::~CVisualVolumeSlider(void) {
}

LRESULT CVisualVolumeSlider::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	SetTimer(ID_MAINFRAME_TIMER, 10);
	return DefWindowProc();
}

LRESULT CVisualVolumeSlider::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	KillTimer(ID_MAINFRAME_TIMER);
	return DefWindowProc();
}

LRESULT CVisualVolumeSlider::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return DefWindowProc();
}

LRESULT CVisualVolumeSlider::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rcHandle;
	GetHandleRect(&rcHandle);

	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

	RECT rcClient;
	GetClientRect(&rcClient);

	if (PtInRect(&rcHandle, pt)) {
		float valuerange = (float)(max_value - min_value);
		float pixelrange = (float)(rcClient.right - 6);
		float unitvalue = std::min(std::max(0.0f, (float)pt.x), pixelrange) / pixelrange;

		drag_value = (int)(min_value + ((1 - unitvalue) * valuerange));
		mode = drag_mode_move;
		SetCapture();
	}

	return 0;
}

LRESULT CVisualVolumeSlider::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if (mode == drag_mode_move) {
		current_value = drag_value;
		ReleaseCapture();

		NMHDR nmh;
		nmh.code =      NM_SETVALUE;
		nmh.idFrom =    GetDlgCtrlID();
		nmh.hwndFrom =  m_hWnd;
		GetParent().SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmh);
	}

	mode = drag_mode_none;
	return 0;
}

LRESULT CVisualVolumeSlider::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

	RECT rcClient;
	GetClientRect(&rcClient);

	if (mode == drag_mode_move) {
		float valuerange = (float)(max_value - min_value);
		float pixelrange = (float)(rcClient.right - 6);
		float unitvalue = std::min(std::max(0.0f, (float)pt.x), pixelrange) / pixelrange;

		drag_value = (int)(min_value + ((1 - unitvalue) * valuerange));
		current_value = drag_value;
		Invalidate(TRUE);

		NMHDR nmh;
		nmh.code =      NM_SETVALUEPREVIEW;
		nmh.idFrom =    GetDlgCtrlID();
		nmh.hwndFrom =  m_hWnd;
		GetParent().SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmh);
	}
	return 0;
}

LRESULT CVisualVolumeSlider::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	RECT rcClient;
	GetClientRect(&rcClient);

	CPaintDC dc(m_hWnd);

	RECT rcL = { 0, 0, rcClient.right, 8 };
	RECT rcR = { 0, rcClient.bottom - 8, rcClient.right, rcClient.bottom };
	RECT rcSlider = { 0, rcL.bottom, rcClient.right, rcR.top };

	COLORREF bgcol = GetSysColor(COLOR_BTNFACE);
	dc.FillSolidRect(&rcClient, bgcol);

	DrawItem(dc, rcL, &lPeak, &lTop, &lDrop, &maxL, &lPeakColor);
	DrawItem(dc, rcR, &rPeak, &rTop, &rDrop, &maxR, &rPeakColor);

	//dc.FillSolidRect(&rcSlider, bgcol);

	RECT rcHandle;
	GetHandleRect(&rcHandle);
	COLORREF hic=GetSysColor(COLOR_BTNHILIGHT);
	COLORREF shc=GetSysColor(COLOR_BTNSHADOW);
	dc.FillSolidRect(&rcHandle, bgcol);
	dc.Draw3dRect(&rcHandle, hic, shc);

	return 0;
}

LRESULT CVisualVolumeSlider::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// TODO: check if peak changed since last
	counter++;
	if ((counter % timerMod) != 0) return 0; /// in preferences
	Invalidate(FALSE);
	return 0;
}

void CVisualVolumeSlider::SetDropRate(float rate) {
	dropRate = rate;
}

void CVisualVolumeSlider::SetTimerRate(int rate) {
	timerMod = (int)std::pow(2.0, (double)rate);
}

void CVisualVolumeSlider::SetPeak(float _maxL, float _maxR) {
	maxL = _maxL;
	maxR = _maxR;

	//Invalidate(FALSE);
}

void CVisualVolumeSlider::GetHandleRect(RECT* rc) {
	RECT rcClient;
	GetClientRect(&rcClient);
	rc->top = 3;
	rc->bottom = rcClient.bottom - 3;

	float valuerange = (float)(max_value - min_value);
	float pixelrange = (float)(rcClient.right - 6);

	float unitvalue;
	
	if (mode == drag_mode_move)
		unitvalue = (drag_value - min_value) / valuerange;
	else
		unitvalue = (current_value - min_value) / valuerange;

	rc->left = (int)((1 - unitvalue) * pixelrange);
	rc->right = (int)((1 - unitvalue) * pixelrange) + 6;

}

void CVisualVolumeSlider::DrawItem(CDC& dc, RECT& rc, float* curPeak, float* curTop, float* curDrop, float* curMax, COLORREF* curPeakColor) {

	COLORREF hic = GetSysColor(COLOR_BTNHILIGHT);
	COLORREF shc = GetSysColor(COLOR_BTNSHADOW);
	dc.Draw3dRect(&rc, shc, hic);

	double f;
	long pos12db;
	long pos6db;
	long ivPos;
	long peakPos;

	float iV=0;

	iV=(linear_to_dB(*curMax, -VU_DB_LIMIT) + VU_DB_LIMIT) / VU_DB_LIMIT;

	if (iV<0) iV=0;

	rc.right -= 2;
	rc.bottom -= 2;

	if (*curTop < iV) {
		(*curTop) = iV;
	}

	if(dropRate) {
		iV = (*curTop);
		(*curTop) -= dropRate;
	}

	if (*curPeak < iV) {
		(*curPeak) = iV;
		(*curPeakColor) = (iV < percent12dB) ? VU_COLOR_LOW : (iV < percent6dB) ? VU_COLOR_12DB : VU_COLOR_6DB;
		(*curDrop) = 0; //VU_PEAK_DROPRATE;
	}

	f = (direction == SB_VERT) ? rc.bottom : rc.right;
	ivPos = (LONG)(iV * f);
	pos12db = (LONG)(double(percent12dB) * f);
	pos6db = (LONG)(double(percent6dB) * f);
	peakPos = (LONG)((*curPeak) * f);

	// Draw the VU meter
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	if (direction == SB_VERT) {
		/*RECT rcAmp = { 0, rc.bottom - ((iV > percent12dB) ? pos12db : ivPos), rc.right, rc.bottom };
		OffsetRect(&rcAmp, 1, 1);
		dc.FillSolidRect(&rcAmp, VU_COLOR_LOW);
		if (iV > percent12dB) {
			RECT rc12db = { 0, rc.bottom - pos12db + 1, rc.right, rc.bottom - ((iV > percent6dB) ? pos6db : ivPos) };
			OffsetRect(&rc12db, 1, 1);
			dc.FillSolidRect(&rc12db, VU_COLOR_12DB);
			if (iV > percent6dB) {
				RECT rc6db = { 0, rc.bottom - pos6db + 1, rc.right, rc.bottom - ivPos};
				OffsetRect(&rc6db, 1, 1);
				dc.FillSolidRect(&rc6db, VU_COLOR_6DB);
			}
		}*/
	} else {
		RECT rcAmp = {0, 0, (iV > percent12dB) ? pos12db : ivPos, height };
		OffsetRect(&rcAmp, rc.left + 1, rc.top + 1);
		dc.FillSolidRect(&rcAmp, VU_COLOR_LOW);
		if (iV > percent12dB) {
			RECT rc12db = {pos12db, 0, (iV > percent6dB) ? pos6db : ivPos, height };
			OffsetRect(&rc12db, rc.left + 1, rc.top + 1);
			dc.FillSolidRect(&rc12db, VU_COLOR_12DB);
			if (iV > percent6dB) {
				RECT rc6db = {pos6db, 0, ivPos, height };
				OffsetRect(&rc6db, rc.left + 1, rc.top + 1);
				dc.FillSolidRect(&rc6db, VU_COLOR_6DB);
			}
		}
	}

	// Draw blank part and peak line

	if (direction == SB_VERT) {
		/*RECT rcBlank = { 0, 0, rc.right, rc.bottom - ivPos + 1};
		OffsetRect(&rcBlank, 1, 1);
		dc.FillSolidRect(&rcBlank, VU_COLOR_BLANK);

		if(*curPeak) {
			RECT rcPeak={0, rc.bottom - peakPos, rc.right, rc.bottom - peakPos + 2};
			OffsetRect(&rcPeak, 1, 1);
			dc.FillSolidRect(&rcPeak, (*curPeakColor));
			(*curPeak) -= (*curDrop);
			(*curDrop) += VU_PEAK_DROPRATE;
		}*/
	} else {
		RECT rcBlank={ivPos + 1, 0, width, height};
		OffsetRect(&rcBlank, rc.left + 1, rc.top + 1);
		dc.FillSolidRect(&rcBlank, VU_COLOR_BLANK);

		if(*curPeak) {
			RECT rcPeak={peakPos, 0, peakPos + 2, height};
			OffsetRect(&rcPeak, rc.left + 1, rc.top + 1);
			dc.FillSolidRect(&rcPeak, (*curPeakColor));
			(*curPeak) -= (*curDrop);
			(*curDrop) += VU_PEAK_DROPRATE;
		}
	}
}

void CVisualVolumeSlider::SetValue(int value) {
	if (mode == drag_mode_none) {
		if (current_value != value) {
			current_value = value;
			Invalidate(FALSE);
		}
	}
}

void CVisualVolumeSlider::SetMinMax(int _min, int _max) {
	min_value = _min;
	max_value = _max;
}
