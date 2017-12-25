#include <atlbase.h>
#include <atlwin.h>
#include <wtl/atlapp.h>
#include <wtl/atlctrls.h>
#include <wtl/atlframe.h>
#include <wtl/atldlgs.h>
#include <wtl/atlctrlw.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <cassert>
#include <zzub/zzub.h>
#include "resource.h"
#include "ParametersControl.h"
#include "utils.h"
#include "Keymaps.h"

using namespace std;

// TODO: make a drop down arrow on the copy toolbar like this: http://www.codeproject.com/docking/toolbar_droparrow.asp

namespace {

const int labelWidth = 100;
const int valueWidth = 80;

size_t getParameterWidth(zzub_parameter_t* param) {
	switch (zzub_parameter_get_type(param)) {
		case zzub_parameter_type_note:
			return 3;
		case zzub_parameter_type_switch:
			return 1;
		case zzub_parameter_type_byte:
			return 2;
		case zzub_parameter_type_word:
			return 4;
	}
	return 0;
}

std::string getValueString(zzub_parameter_t* para, int value, bool hex) {
	int len = hex?getParameterWidth(para):0;
	if (value == zzub_parameter_get_value_none(para)) return fillString('.', len);
	if (zzub_parameter_get_type(para) == zzub_parameter_type_note) return noteFromInt(value);
	if (zzub_parameter_get_type(para) == zzub_parameter_type_switch) {
		if (value == zzub_parameter_get_value_none(para))
			return ".";
		if (value == zzub_switch_value_on)
			return "1"; else
			return "0";
	}
	return hex?hexFromInt(value, len, '0'):stringFromInt(value);
}

}


std::string zzub_plugin_describe_value2(zzub_plugin_t* plugin, int group, int track, int column, int value) {
	const char* desc = zzub_plugin_describe_value(plugin, group, column, value);

	std::string text;
	if (desc == 0 || strlen(desc) == 0) {
		zzub_parameter_t* p = zzub_plugin_get_parameter(plugin, group, track, column);
		text = getValueString(p, value, false);
	} else
		text = desc;

	// prepend a space for margin
	text = " " + text;

	return text;
}

/***

	CValueEdit

***/

LRESULT CValueEdit::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (wParam == VK_SPACE) {
		PostMessage(WM_KEYDOWN, VK_RETURN);
		return 0;
	}
	return DefWindowProc();
}

LRESULT CValueEdit::OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if (wParam == VK_RETURN || wParam == VK_TAB || wParam == VK_ESCAPE || wParam == VK_SPACE) return 0;
	return DefWindowProc();
}

LRESULT CValueEdit::OnGetDlgCode(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// edit control sends only WM_GETDLGCODE on return etc, so handle them here
	// NOTE: this is redundant on windows but is needed on wine
	if (wParam == VK_RETURN) {
		return GetParent().PostMessage(WM_COMMAND, IDOK);
	} else if (wParam == VK_ESCAPE) {
		return GetParent().PostMessage(WM_COMMAND, IDCANCEL);
	}
	return DefWindowProc();
		
}
/***

	CValueDialog

***/

CValueDialog::CValueDialog(HWND hwndOwner) {
	m_wndOwner=hwndOwner;
}

CValueDialog::~CValueDialog() {
}

LRESULT CValueDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DefWindowProc();

	m_edit.SubclassWindow(GetDlgItem(IDC_SLIDER_EDIT));
	char pc[16];
	sprintf(pc, "%i", m_value);
	m_edit.SetWindowText(pc);
	m_edit.SetFocus();

	int len = strlen(pc);
	m_edit.SetSel(len, len, FALSE);

	// position ourself centered on top of the parent
	RECT rcWindow;
	RECT rcParent;
	GetWindowRect(&rcWindow);
	m_wndOwner.GetWindowRect(&rcParent);

	int width = rcWindow.right-rcWindow.left;
	int height = rcWindow.bottom-rcWindow.top;

	int centerY = rcParent.top + (rcParent.bottom-rcParent.top)/2;

	MoveWindow(rcParent.left + ((rcParent.right-rcParent.left) / 2), centerY-(height/2), width, height);
	return 0; // An application can return FALSE only if it has set the keyboard focus to one of the controls of the dialog box. 
}

LRESULT CValueDialog::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	EndDialog(wID);
	return 0;
}

LRESULT CValueDialog::OnChanged(WORD /*wNotify	Code*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	char pc[16];
	m_edit.GetWindowText(pc, 16);
	m_value = atoi(pc);
	return 0;
}

void CValueDialog::SetValue(int value) {
	m_value = value;
}

int CValueDialog::GetValue() {
	return m_value;
}


// ---------------------------------------------------------------------------------------------------------------
// MOUSE
// ---------------------------------------------------------------------------------------------------------------

int CMachineParameterScrollView::GetSliderIndexFromHwnd(HWND hWnd) {
	for (size_t i = 0; i < sliders.size(); ++i) {
		if (sliders[i]->m_hWnd == hWnd)
			return i;
	}
	return -1;
}

CParameterSliderBar* CMachineParameterScrollView::GetSliderFromHwnd(HWND hWnd) {
	for (size_t i = 0; i < sliders.size(); ++i) {
		if (sliders[i]->m_hWnd == hWnd)
			return sliders[i];
	}
	return 0;
}




/***

	CMachineParameterScrollView

***/

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DESTRUCTION
// ---------------------------------------------------------------------------------------------------------------

CMachineParameterScrollView::CMachineParameterScrollView(zzub_player_t* _player) {
	player = _player;
	undoable = true;
	selectedSlider = 0;
}

CMachineParameterScrollView::~CMachineParameterScrollView() {
	sliders.clear();
}

LRESULT CMachineParameterScrollView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return DefWindowProc();
}

// ---------------------------------------------------------------------------------------------------------------
// SCROLLING + SIZING
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineParameterScrollView::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int width = GET_X_LPARAM(lParam);
	int height = GET_Y_LPARAM(lParam);

	for (size_t i = 0; i < sliders.size(); ++i) {
		sliders[i]->MoveWindow(0, i*CParameterSliderBar::sliderHeight, width, CParameterSliderBar::sliderHeight);
	}

	UpdateScrollbars();

	return 0;
}

LRESULT CMachineParameterScrollView::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// paint values and labels
	return DefWindowProc();
}

LRESULT CMachineParameterScrollView::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD req=LOWORD(wParam);

	WORD trackValue=HIWORD(wParam);
	int prevPos=GetScrollPos(SB_VERT);

	RECT rc;
	GetClientRect(&rc);
	int height=rc.bottom;
	int pageHeight=height;

	int y=0;
	switch (req) {
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			SetScrollPos(SB_VERT, trackValue);
			break;
		case SB_LINEDOWN:
			SetScrollPos(SB_VERT, prevPos+CParameterSliderBar::sliderHeight);
			break;
		case SB_LINEUP:
			SetScrollPos(SB_VERT, prevPos+-CParameterSliderBar::sliderHeight);
			break;
		case SB_PAGEDOWN:
			SetScrollPos(SB_VERT, prevPos+pageHeight);
			break;
		case SB_PAGEUP:
			SetScrollPos(SB_VERT, prevPos+-pageHeight);
			break;
		default:
			return 0;
	}

	int nextPos=GetScrollPos(SB_VERT);

	this->ScrollWindow(0, prevPos-nextPos);
	return TRUE;
}

void CMachineParameterScrollView::UpdateScrollbars() {
	RECT rcClient;
	GetClientRect(&rcClient);
	int height = rcClient.bottom;

	int prevScroll = GetScrollPos(SB_VERT);
	int totalSliderHeight = sliders.size() * CParameterSliderBar::sliderHeight;
	if (totalSliderHeight < 0 || height == 0) totalSliderHeight = 0;

	if (prevScroll + height > totalSliderHeight) 
		if (height > totalSliderHeight)
			prevScroll = 0; else
			prevScroll = totalSliderHeight - height;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = height;
	si.nMin = 0;
	si.nMax = totalSliderHeight-2;
	si.nPos = prevScroll;
	SetScrollInfo(SB_VERT, &si);
	ScrollWindow(0, -prevScroll);
}

void CMachineParameterScrollView::ScrollToView(int selectedSlider) {
	RECT rc;
	GetClientRect(&rc);

	int prevPos = GetScrollPos(SB_VERT);
	int nextPos = prevPos;

	if ((selectedSlider+1)*CParameterSliderBar::sliderHeight > rc.bottom+nextPos - 1)
		nextPos = (selectedSlider+1)*CParameterSliderBar::sliderHeight - rc.bottom - 1;

	if (selectedSlider * CParameterSliderBar::sliderHeight < nextPos)
		nextPos = selectedSlider * CParameterSliderBar::sliderHeight;

	if (prevPos-nextPos != 0) {
		SetScrollPos(SB_VERT, nextPos);
		ScrollWindow(0, prevPos-nextPos);
		// should invalidate only scrolled area
		Invalidate(FALSE);
	}
}

void CMachineParameterScrollView::SelectSlider(zzub_plugin_t* plugin, int group, int track, int column) {
	int pluginid = zzub_plugin_get_id(plugin);

	for (size_t i = 0; i < sliders.size(); ++i) {
		CParameterSliderBar& slider = *sliders[i];
		if (slider.m_pluginid == pluginid && slider.m_group == group && slider.m_track == track && slider.m_column == column) {
			SelectSlider(i);
			return ;
		}
	}
}

void CMachineParameterScrollView::SelectSlider(int index) {
	selectedSlider = index;

	for (size_t i = 0; i < sliders.size(); ++i) {
		sliders[i]->SetSelected(i == selectedSlider);
	}

	sliders[index]->SetFocus();
	Invalidate(FALSE);
}

int CMachineParameterScrollView::GetSelectedSliderIndex() {
	return selectedSlider;
}

CParameterSliderBar* CMachineParameterScrollView::GetSelectedSlider() {
	if (selectedSlider < 0 || selectedSlider >= (int)sliders.size()) return 0;
	return sliders[selectedSlider];
}

int CMachineParameterScrollView::GetSliderCount() {
	return (int)sliders.size();
}




// ---------------------------------------------------------------------------------------------------------------
// PAINTING
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineParameterScrollView::OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	CDCHandle dc((HDC)wParam);
	COLORREF fc = GetSysColor(COLOR_BTNFACE);
	RECT rcClient;
	GetClientRect(&rcClient);

	RECT rc = rcClient;
	rc.top = CParameterSliderBar::sliderHeight * sliders.size();
	dc.FillSolidRect(&rc, fc);

	return 1;
}

// ---------------------------------------------------------------------------------------------------------------
// BINDING
// ---------------------------------------------------------------------------------------------------------------

void CMachineParameterScrollView::SetUndo(bool state) {
	undoable = state;
}

void CMachineParameterScrollView::SetParameters(const std::vector<paramid>& params) {
	if (sliders.size() == params.size()) return ;

	for (size_t i = params.size(); i < sliders.size(); ++i) {
		// the slider deletes itself in OnFinalMessage, 
		// we cant delete here those times a slider invoked the deleting of itself
		sliders[i]->SendMessage(WM_CLOSE); 
	}

	RECT rcClient;
	GetClientRect(&rcClient);

	int oldslidercount = sliders.size();
	sliders.resize(params.size());

	for (std::vector<paramid>::const_iterator i = params.begin(); i != params.end(); ++i) {
		int sliderIndex = (int)(i - params.begin());
		RECT rcSlider = { 0, sliderIndex*CParameterSliderBar::sliderHeight, rcClient.right, sliderIndex*CParameterSliderBar::sliderHeight + CParameterSliderBar::sliderHeight };
		if (sliderIndex >= oldslidercount) {
			sliders[sliderIndex] = new CParameterSliderBar();
			sliders[sliderIndex]->Create(m_hWnd, rcSlider, "", WS_CHILD|WS_VISIBLE);
		} else {
			sliders[sliderIndex]->MoveWindow(&rcSlider, FALSE);
		}

		zzub_parameter_t* param = zzub_plugin_get_parameter(i->plugin, i->group, i->track, i->column);
		int minvalue = zzub_parameter_get_value_min(param);
		int maxvalue = zzub_parameter_get_value_max(param);
		int value = zzub_plugin_get_parameter_value(i->plugin, i->group, i->track, i->column);
		std::string valueName = zzub_plugin_describe_value2(i->plugin, i->group, i->track, i->column, value);
		int pluginid = zzub_plugin_get_id(i->plugin);

		sliders[sliderIndex]->SetUserData(pluginid, i->group, i->track, i->column);
		sliders[sliderIndex]->SetMinMax(minvalue, maxvalue);
		std::stringstream strm;
		if (i->group == 2) strm << i->track << ":";
		strm << zzub_parameter_get_name(param);
		sliders[sliderIndex]->SetName(strm.str());
		sliders[sliderIndex]->SetValue(value, valueName);
		sliders[sliderIndex]->m_selected = sliderIndex == selectedSlider;
	}

	UpdateScrollbars();
	Invalidate(TRUE); // TRUE = clear background not occupied by any sliders
}

void CMachineParameterScrollView::SetParameter(zzub_plugin_t* plugin, int group, int track, int column, int value) {

	int pluginid = zzub_plugin_get_id(plugin);
	//std::string valueName = zzub_plugin_describe_value2(plugin, group, track, column, value);

	for (size_t i = 0; i < sliders.size(); ++i) {
		CParameterSliderBar& slider = *sliders[i];
		if (slider.m_pluginid == pluginid && slider.m_group == group && slider.m_track == track && slider.m_column == column) {
			if (!slider.m_dragging)
				slider.NotifyChange(value);//, valueName);
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------
// INPUT
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineParameterScrollView::OnSliderClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	int sel = GetSliderIndexFromHwnd(pnmh->hwndFrom);
	SelectSlider(sel);
	return 0;
}

LRESULT CMachineParameterScrollView::OnSliderChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	VALUENMHDR* nm = (VALUENMHDR*)pnmh;

	CParameterSliderBar* slider = GetSliderFromHwnd(pnmh->hwndFrom);
	assert(slider != 0);

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, slider->m_pluginid);
	slider->SetValue(nm->value, zzub_plugin_describe_value2(plugin, slider->m_group, slider->m_track, slider->m_column, nm->value));

	zzub_plugin_set_parameter_value_direct(plugin, slider->m_group, slider->m_track, slider->m_column, nm->value, true);
	return 0;
}

LRESULT CMachineParameterScrollView::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, lParam);
}

LRESULT CMachineParameterScrollView::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	return GetParent().SendMessage(WM_CONTEXTMENU, wParam, MAKELPARAM(pt.x, pt.y));
}


LRESULT CMachineParameterScrollView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
// NOTE: if somebody bitches about having to press ENTER to open the numbox, enable the following code by a preference:
// the purpose is to allow the virtual keyboard to work, which also uses numeric keys
	/*CParameterSliderBar* slider = GetSelectedSlider();
	if (slider != 0) {
		switch (wParam) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				CValueDialog valueDialog(*slider);
				valueDialog.SetValue(wParam-'0');
				if (IDOK == valueDialog.DoModal(*this)) {
					int value = valueDialog.GetValue();
					slider->NotifyChange(value);
				}
				GetWindow(GW_OWNER).SetActiveWindow();
				slider->SetFocus();
				return 0;
			}
		}
	}*/
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

LRESULT CMachineParameterScrollView::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

LRESULT CMachineParameterScrollView::OnBlur(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}
