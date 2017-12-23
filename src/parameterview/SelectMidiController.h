#pragma once

class CSelectMidiController 
	: public CDialogImpl<CSelectMidiController>
	, public CWinDataExchange<CSelectMidiController>
{
	CEdit channelEdit, controllerEdit, valueEdit;
public:
	HWND hListenerWnd; // the dialog posts WM_SELECTMIDICONTROLLER to this wnd
	int channel, controller, value, currentMidiValue;

	zzub_plugin_t* bindMidiMachine;
	size_t bindMidiGroup, bindMidiTrack, bindMidiColumn;

	enum { IDD = IDD_LEARNMIDICONTROLLER };

    BEGIN_DDX_MAP(CSelectMidiController)
        DDX_CONTROL_HANDLE(IDC_MIDI_CHANNEL_EDIT, channelEdit)
        DDX_CONTROL_HANDLE(IDC_MIDI_CONTROLLER_EDIT, controllerEdit)
        DDX_CONTROL_HANDLE(IDC_MIDI_VALUE_EDIT, valueEdit)

        DDX_INT(IDC_MIDI_CONTROLLER_EDIT, controller)
        DDX_INT(IDC_MIDI_VALUE_EDIT, value)
		DDX_INT(IDC_MIDI_CHANNEL_EDIT, channel);
    END_DDX_MAP()

	BEGIN_MSG_MAP(CSelectMidiController)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDOK, OnBnClickedOk)
	END_MSG_MAP()

	CSelectMidiController();
	virtual ~CSelectMidiController();

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void MidiEvent(int status, int data1, int data2);
};
