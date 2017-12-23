#pragma once

/*

NOTE: as long as this control is in a static library:

in the linker options of whatever dll/exe using this control, remember to add 
buzecommon.res as one of the linker inputs (debug/release). apparently the
resources from static libs are not included in the including project.

primary symptom of incorrectly linked resources is the value dialog does not appear.

*/

#include "ParameterSliderBar.h"

class CValueEdit : public CWindowImpl<CValueEdit, CEdit> {
public:

	DECLARE_WND_SUPERCLASS(NULL, CEdit::GetWndClassName());

	BEGIN_MSG_MAP(CValueEdit)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
	END_MSG_MAP()

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetDlgCode(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
};

class CValueDialog : public CDialogImpl<CValueDialog> {
	CValueEdit m_edit;
	int m_value;

	CWindow m_wndOwner;
public:
	enum { IDD = IDD_VALUE_DIALOG };

	BEGIN_MSG_MAP(CValueDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_HANDLER(IDC_SLIDER_EDIT, EN_UPDATE, OnChanged)
	END_MSG_MAP()

	CValueDialog(HWND);   // standard constructor
	virtual ~CValueDialog();

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void SetValue(int value);
	int GetValue();
};

struct paramid {
	zzub_plugin_t* plugin;
	int group, track, column;
	bool in_random_set;
};

class CMachineParameterScrollView : public CWindowImpl<CMachineParameterScrollView> {
public:
	zzub_player_t* player;
	std::vector<CParameterSliderBar*> sliders;
	int selectedSlider;

	bool undoable;

	DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW|CS_VREDRAW|CS_PARENTDC, NULL)

	BEGIN_MSG_MAP(CMachineParameterScrollView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnBlur)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		NOTIFY_CODE_HANDLER(NM_CLICK, OnSliderClick)
		NOTIFY_CODE_HANDLER(SM_CHANGE, OnSliderChange);
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	CMachineParameterScrollView(zzub_player_t* _player);
	~CMachineParameterScrollView();
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBlur(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSliderChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnSliderClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	void SetParameters(const std::vector<paramid>& params);
	void SetParameter(zzub_plugin_t* plugin, int group, int track, int column, int value);
	void SetUndo(bool enabled);
	void SelectSlider(zzub_plugin_t* plugin, int group, int track, int column);
	void SelectSlider(int index);
	void SelectSlider(CParameterSliderBar*);
	int GetSelectedSliderIndex();
	CParameterSliderBar* GetSelectedSlider();
	int GetSliderCount();
	int GetSliderIndexFromHwnd(HWND hWnd);
	CParameterSliderBar* GetSliderFromHwnd(HWND hWnd);

	void ScrollToView(int slider);
	void UpdateScrollbars();
};
