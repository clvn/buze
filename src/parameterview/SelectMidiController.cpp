#include "stdafx.h"
#include "resource.h"
#include "SelectMidiController.h"

static const int WM_SELECTMIDICONTROLLER = WM_USER+1;

CSelectMidiController::CSelectMidiController() {
	hListenerWnd = 0;
	channel = 0;
	controller = 0;
	value = 0;
	currentMidiValue = 0;
	bindMidiGroup = 0;
	bindMidiTrack = 0;
	bindMidiColumn = 0;
}

CSelectMidiController::~CSelectMidiController() {
}

LRESULT CSelectMidiController::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DoDataExchange(FALSE);
	return FALSE;
}

void CSelectMidiController::MidiEvent(int midistatus, int mididata1, int mididata2) {
	if (m_hWnd == 0) return ;
	assert(hListenerWnd != 0);

	static int startValue = -1;
	char status = midistatus >> 4;
	channel = midistatus & 0xF;

	switch (status) {
		case 0xB:
			controller = mididata1;
			value = mididata2;

			DoDataExchange();
			if(startValue >= 0) {

				/* If the controller value has reached the current slider value, bind MIDI controller and close */
				if((startValue < currentMidiValue && value >= currentMidiValue)
				|| (startValue > currentMidiValue && value <= currentMidiValue)) {
					startValue = -1;
					SendMessage(hListenerWnd, WM_SELECTMIDICONTROLLER, 0, 0);
					DestroyWindow();
				}
			}
			else {
				startValue = value;
			}
			break;
	}
	return ;
}

LRESULT CSelectMidiController::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	DestroyWindow();
	return 0;
}

LRESULT CSelectMidiController::OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	SendMessage(hListenerWnd, WM_SELECTMIDICONTROLLER, 0, 0);
	DestroyWindow();
	return TRUE;
}
