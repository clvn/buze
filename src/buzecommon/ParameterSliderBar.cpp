#include <algorithm>
using std::min;
using std::max;

#define NOMINMAX
#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <string>
#include "ParameterSliderBar.h"

CParameterSliderBar::CParameterSliderBar(void) {
	m_value = -1;
	m_minValue = 0;
	m_maxValue = 65535;
	m_dragging = false;
	m_selected = false;
	m_shaded = false;
}

CParameterSliderBar::~CParameterSliderBar(void) {
}

LRESULT CParameterSliderBar::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	LRESULT lres = DefWindowProc();
	return 0;
}

LRESULT CParameterSliderBar::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CPaintDC dc(m_hWnd);

	RECT rc;
	GetSliderRect(&rc);

	RECT rcSlider;
	GetKnobRect(&rcSlider);
	rcSlider.bottom -= 2;

	COLORREF bg = GetSysColor(COLOR_3DFACE);
	COLORREF hc = GetSysColor(COLOR_3DHIGHLIGHT);
	COLORREF sc = GetSysColor(COLOR_3DSHADOW);
	COLORREF mc = GetSysColor(COLOR_GRADIENTINACTIVECAPTION);

	if (m_selected) {
		// draw border on the bottom with bg color
		RECT bc = rc;
		bc.top = bc.bottom-2;
		dc.FillSolidRect(&bc, bg);

		// set aside 2px margin on the bottom
		rc.bottom-=2;

		// draw slider handle with bg color
		bc = rc;
		dc.FillSolidRect(&bc, bg);

		// draw darker bg to the left of the slider
		bc = rc;
		bc.right = rcSlider.left;
		dc.FillSolidRect(&bc, mc);

		// draw darker bg to the right of the slider
		bc = rc;
		bc.left = rcSlider.right;
		dc.FillSolidRect(&bc, mc);
	} else {
		dc.FillSolidRect(&rc, bg);
		rc.bottom-=2;
	}
	dc.Draw3dRect(&rc, sc, hc);

	dc.Draw3dRect(&rcSlider, hc, sc);

	dc.SetBkColor(bg);
	dc.SetTextColor( m_shaded? sc : (COLORREF)0);
	HFONT oldfont = dc.SelectFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	RECT rcLabel;
	GetLabelRect(&rcLabel);
	dc.ExtTextOut(rcLabel.left, rcLabel.top, ETO_CLIPPED|ETO_OPAQUE, &rcLabel, m_name.c_str());

	RECT rcValue;
	GetValueRect(&rcValue);
	dc.SetTextColor((COLORREF)0);
	dc.ExtTextOut(rcValue.left, rcValue.top, ETO_CLIPPED|ETO_OPAQUE, &rcValue, m_valueName.c_str());

	dc.SelectFont(oldfont);
	return 0;
}

LRESULT CParameterSliderBar::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 1;
}

LRESULT CParameterSliderBar::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	SetFocus();
	return GetParent().SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, lParam);
}

LRESULT CParameterSliderBar::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { (short)LOWORD( lParam), (short)HIWORD(lParam) };

	RECT rcKnob;
	GetKnobRect(&rcKnob);
	
	RECT rcSlider;
	GetSliderRect(&rcSlider);

	RECT rcLabel;
	GetLabelRect(&rcLabel);

	if (PtInRect(&rcKnob, pt)) {
		SetCapture();
		m_dragging = true;
		m_ptDragStart = pt;
		m_rcDragStart = rcKnob;
		m_dragStartValue = GetValue();
	} else if (PtInRect(&rcLabel, pt)) {
		m_shaded = !m_shaded;
		InvalidateRect(&rcLabel, FALSE);
	} else if (PtInRect(&rcSlider, pt)) {
		if (pt.x<rcKnob.left) {
			int rel = ceil(float(GetMax()-GetMin()) / 36);
			int value = GetValue() - rel;

			if (value != m_value) {
				//SetValue(value);
				NotifyChange(value);
				//Invalidate(FALSE);
			}

		} else if (pt.x > rcKnob.right) {
			int rel = ceil(float(GetMax()-GetMin()) / 36);
			int value = GetValue() + rel;

			if (value != m_value) {
				//SetValue(value);
				NotifyChange(value);
				//Invalidate(FALSE);
			}
		}
	}

	NMHDR nmhdr;
	nmhdr.code = NM_CLICK;
	nmhdr.hwndFrom = *this;
	nmhdr.idFrom = 0;
	::SendMessage(GetParent(), WM_NOTIFY, 0, (LPARAM)&nmhdr);

//	SetFocus(); parent focuses

	return 0;
}

void CParameterSliderBar::SetUserData(int pluginid, int group, int track, int column) {
	m_pluginid = pluginid;
	m_group = group;
	m_track = track;
	m_column = column;
}

void CParameterSliderBar::NotifyChange(int value) {
	if (value < GetMin()) value = GetMin();
	if (value > GetMax()) value = GetMax();

	if (value == m_value) return ;

	VALUENMHDR nm;
	nm.code = SM_CHANGE;    // Message type defined by control.
	nm.idFrom = GetDlgCtrlID();
	nm.hwndFrom = *this;
	nm.value = value;
	::SendMessage(GetParent(), WM_NOTIFY, 0, (LPARAM)&nm);
}

LRESULT CParameterSliderBar::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if (m_dragging) {
		ReleaseCapture();
		m_dragging=false;
	}
	return 0;
}

LRESULT CParameterSliderBar::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int d = (signed short)HIWORD(wParam);

	int value = GetValue();
	if (d < 0) {
		value--;
	} else {
		value++;
	}

	//SetValue(value);
	NotifyChange(value);
	return 0;
}

LRESULT CParameterSliderBar::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {

	if (m_dragging) {
		POINT pt = { (short)LOWORD( lParam), (short)HIWORD(lParam) };
		RECT rcClient;
		GetSliderRect(&rcClient);

		int sliderPos = m_ptDragStart.x - m_rcDragStart.left;

		float valueDistance = GetMax() - GetMin();
		float screenWidth = (rcClient.right - rcClient.left) - GetKnobWidth();
		float valuePerPx = valueDistance / screenWidth;
		float pxPerValue = screenWidth / valueDistance;

		if (pxPerValue < GetKnobWidth()) sliderPos = 0;
		float screenDistance = pt.x - m_ptDragStart.x + sliderPos;

		int value = m_dragStartValue + (screenDistance * valuePerPx);
		if (value != m_value) {
			//SetValue(value);
			NotifyChange(value);
			//Invalidate(FALSE);
		}
	}
	return 0;
}

LRESULT CParameterSliderBar::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

LRESULT CParameterSliderBar::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

LRESULT CParameterSliderBar::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CParameterSliderBar::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return DefWindowProc();
}

LRESULT CParameterSliderBar::OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return DefWindowProc();
}

int CParameterSliderBar::GetValue() {
	return m_value;
}

void CParameterSliderBar::SetName(const std::string& name) {
	m_name = name;
	Invalidate(FALSE);
}

void CParameterSliderBar::SetValue(int value, const std::string& valueName) {
	if (value == m_value) return ;
	if (value < GetMin()) value = GetMin();
	if (value > GetMax()) value = GetMax();
	m_value = value;
	m_valueName = valueName;
	Invalidate(FALSE);
}

int CParameterSliderBar::GetMin() {
	return m_minValue;
}

int CParameterSliderBar::GetMax() {
	return m_maxValue;
}

void CParameterSliderBar::SetMinMax(int minV, int maxV) {
	m_minValue=std::min(minV, maxV);
	m_maxValue=std::max(minV, maxV);

	if (m_value != -1) {
		if (m_value<m_minValue)
			m_value=m_minValue;
		if (m_value>m_maxValue)
			m_value=m_maxValue;
	}
	Invalidate(FALSE);
}

void CParameterSliderBar::GetLabelRect(RECT* rc) {
	GetClientRect(rc);
	rc->left = 0;
	rc->right = labelWidth;
}

void CParameterSliderBar::GetSliderRect(RECT* rc) {
	GetClientRect(rc);
	rc->left += labelWidth;
	rc->right -= valueWidth;
}

void CParameterSliderBar::GetValueRect(RECT* rc) {
	GetClientRect(rc);
	rc->left = rc->right - valueWidth;
}

bool CParameterSliderBar::GetKnobRect(RECT* rc) {
	RECT rcClient;
	GetSliderRect(&rcClient);

	float valueDistance = GetMax() - GetMin();
	float screenWidth = (rcClient.right - rcClient.left) - 2 - GetKnobWidth();    // -2 = border pixels
	float valuePerPx = valueDistance / screenWidth;

	rc->left = rcClient.left + 1 + (GetValue() - GetMin()) / valuePerPx;
	rc->top = 2;
	rc->right = rc->left + GetKnobWidth();
	rc->bottom = rc->top + rcClient.bottom - 4;
	return true;
}

int CParameterSliderBar::GetKnobWidth() {
	// hvis deltaet er større enn 10, så blir widthen lik deltaet
	RECT rcClient;
	GetSliderRect(&rcClient);

	float valueDistance = (GetMax() - GetMin() + 1);
	float screenWidth = rcClient.right - rcClient.left;
	if (valueDistance == 0) return 0;
	float pxPerValue = screenWidth / valueDistance;

	return pxPerValue < 12 ? 12 : pxPerValue;
}

void CParameterSliderBar::SetSelected(bool state) {
	if (state != m_selected) {
		m_selected = state;
		Invalidate(FALSE);
	}
}
