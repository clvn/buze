#pragma once

#include "ToolbarBands.h"

static const int WM_ADD_PALETTEWINDOW = WM_USER+12;
static const int WM_REMOVE_PALETTEWINDOW = WM_USER+13;

// ---------------------------------------------------------------------------------------------------------------
// DIALOG CENTERING
// ---------------------------------------------------------------------------------------------------------------

inline BOOL CenterDialogRelative(HWND hWndDialog, HWND hWndCenter/* = NULL*/) {
	CWindow dlgWnd(hWndDialog);
	ATLASSERT(::IsWindow(dlgWnd));

	if (hWndCenter == NULL) {
		hWndCenter = ::GetParent(dlgWnd);
	}

	RECT rcDlg;
	::GetWindowRect(dlgWnd, &rcDlg);

	RECT rcCenter;
	::GetWindowRect(hWndCenter, &rcCenter);

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	return ::SetWindowPos(dlgWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

// ---------------------------------------------------------------------------------------------------------------
// BASE
// ---------------------------------------------------------------------------------------------------------------

template <class X>
class CTransformDialog 
:
	public CDialogImpl<X>,
	public CMessageFilter
{
  public:

	BEGIN_MSG_MAP_EX(X)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_ACTIVATE(OnActivate)
		CMD_ID_HANDLER(IDCLOSE, OnClose)
	END_MSG_MAP()

	CWindow parentWnd;

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
		parentWnd = (HWND)lInitParam;
		CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->AddMessageFilter(this);
		X* pX = static_cast<X*>(this);
		pX->Init();
		CenterDialogRelative(m_hWnd, parentWnd);
		return TRUE;
	}

	void OnClose() {
		DestroyWindow();
	}

	void OnDestroy() {
		CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);
		SetMsgHandled(FALSE);
	}

	BOOL PreTranslateMessage(MSG* pMsg) {
		if (::IsWindow(m_hWnd) && IsChild(pMsg->hwnd)) {
			if (pMsg->message == WM_KEYDOWN) {
				if (pMsg->wParam == VK_RETURN) {
					parentWnd.SendMessage(WM_COMMAND, MAKEWPARAM(X::IDD, IDOK), 0);
					parentWnd.SetFocus(); // ->WM_ACTIVATE->WA_INACTIVE->IDCLOSE
					return TRUE;
				} else
				if (pMsg->wParam == VK_ESCAPE) {
					parentWnd.SendMessage(WM_COMMAND, MAKEWPARAM(X::IDD, IDCANCEL), 0);
					parentWnd.SetFocus(); // ->WM_ACTIVATE->WA_INACTIVE->IDCLOSE
					return TRUE;				
				}
			}
			return IsDialogMessage(pMsg);
		} else
			return FALSE;
	}

	void OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther) {
		if (nState == WA_INACTIVE) {
			PostMessage(WM_COMMAND, MAKEWPARAM(IDCLOSE, 0), 0);
		}
		SetMsgHandled(FALSE);
	}
};

// ---------------------------------------------------------------------------------------------------------------
// TRANSFORMS
// ---------------------------------------------------------------------------------------------------------------

// Thin
// Repeat
// Scale
// Amplify
// Rotate Rows
// Rotate Values
// Rotate Notes
// Rotate Dists
// Notelength

class CThinDialog : public CTransformDialog<CThinDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_THIN };
	CIntegralEdit m_edit;

	void Init() {
		m_edit.SubclassWindow(GetDlgItem(IDC_EDIT1));
	}
};

class CRepeatDialog : public CTransformDialog<CRepeatDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_REPEAT };
	CIntegralEdit m_edit;

	void Init() {
		m_edit.SubclassWindow(GetDlgItem(IDC_EDIT1));
	}
};

class CEchoDialog : public CTransformDialog<CEchoDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_ECHO };
	CIntegralEdit m_edit;

	void Init() {
		m_edit.SubclassWindow(GetDlgItem(IDC_EDIT1));
	}
};

class CNotelengthDialog : public CTransformDialog<CNotelengthDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_NOTELENGTH };
	CIntegralEdit m_edit;
	CButton m_radioButton1;
	CButton m_radioButton2;
	CButton m_radioButton3;
	CButton m_radioButton4;

	BEGIN_MSG_MAP_EX(CNotelengthDialog)
		CMD_HANDLER(IDC_RADIO1, BN_CLICKED, OnRadioClicked)
		CMD_HANDLER(IDC_RADIO2, BN_CLICKED, OnRadioClicked)
		CMD_HANDLER(IDC_RADIO3, BN_CLICKED, OnRadioClicked)
		CMD_HANDLER(IDC_RADIO4, BN_CLICKED, OnRadioClicked)
		CHAIN_MSG_MAP(CTransformDialog<CNotelengthDialog>)
	END_MSG_MAP()

	void Init() {
		m_edit.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_radioButton1.Attach(GetDlgItem(IDC_RADIO1));
		m_radioButton2.Attach(GetDlgItem(IDC_RADIO2));
		m_radioButton3.Attach(GetDlgItem(IDC_RADIO3));
		m_radioButton4.Attach(GetDlgItem(IDC_RADIO4));
		CheckRadioButton(IDC_RADIO1, IDC_RADIO4, IDC_RADIO1);
	}

	int getCheckedRadio() const {
		if (m_radioButton1.GetCheck()) return 0;
		if (m_radioButton2.GetCheck()) return 1;
		if (m_radioButton3.GetCheck()) return 2;
		if (m_radioButton4.GetCheck()) return 3;
		return -1;
	}

	void OnRadioClicked() {
		if (m_radioButton4.GetCheck()) {
			//m_edit.ClearInt();
			m_edit.EnableWindow(FALSE);
		} else {
			m_edit.EnableWindow(TRUE);
		}
	}
};

class CScaleDialog : public CTransformDialog<CScaleDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_SCALE };
	CFloatingEdit m_edit1;
	CFloatingEdit m_edit2;
	CFloatingEdit m_edit3;
	CFloatingEdit m_edit4;

	void Init() {
		m_edit1.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_edit2.SubclassWindow(GetDlgItem(IDC_EDIT2));
		m_edit3.SubclassWindow(GetDlgItem(IDC_EDIT3));
		m_edit4.SubclassWindow(GetDlgItem(IDC_EDIT4));
	}
};

class CFadeDialog : public CTransformDialog<CFadeDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_FADE };
	CFloatingEdit m_edit1;
	CFloatingEdit m_edit2;

	void Init() {
		m_edit1.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_edit2.SubclassWindow(GetDlgItem(IDC_EDIT2));
	}
};

class CCurveMapDialog : public CTransformDialog<CCurveMapDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_CURVEMAP };
	CButton m_radioButton1;
	CButton m_radioButton2;
	CButton m_radioButton3;
	CButton m_radioButton4;
	CButton m_radioButton5;
	CButton m_radioButton6;
	CButton m_radioButton7;
	CButton m_radioButton8;

	void Init() {
		m_radioButton1.Attach(GetDlgItem(IDC_RADIO1));
		m_radioButton2.Attach(GetDlgItem(IDC_RADIO2));
		m_radioButton3.Attach(GetDlgItem(IDC_RADIO3));
		m_radioButton4.Attach(GetDlgItem(IDC_RADIO4));
		m_radioButton5.Attach(GetDlgItem(IDC_RADIO5));
		m_radioButton6.Attach(GetDlgItem(IDC_RADIO6));
		m_radioButton7.Attach(GetDlgItem(IDC_RADIO7));
		m_radioButton8.Attach(GetDlgItem(IDC_RADIO8));
		CheckRadioButton(IDC_RADIO1, IDC_RADIO8, IDC_RADIO1);
	}

	int getCheckedRadio() const {
		if (m_radioButton1.GetCheck()) return 0;
		if (m_radioButton2.GetCheck()) return 1;
		if (m_radioButton3.GetCheck()) return 2;
		if (m_radioButton4.GetCheck()) return 3;
		if (m_radioButton5.GetCheck()) return 4;
		if (m_radioButton6.GetCheck()) return 5;
		if (m_radioButton7.GetCheck()) return 6;
		if (m_radioButton8.GetCheck()) return 7;
		return -1;
	}
};

class CRotateRowsDialog : public CTransformDialog<CRotateRowsDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_ROTATEROWS };
	CIntegralEdit m_edit;

	void Init() {
		m_edit.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_edit.AllowUnsigned(true);
	}
};

class CRotateValsDialog : public CTransformDialog<CRotateValsDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_ROTATEVALS };
	CIntegralEdit m_edit;

	void Init() {
		m_edit.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_edit.AllowUnsigned(true);
	}
};

class CRotateRhythmsDialog : public CTransformDialog<CRotateRhythmsDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_ROTATERHYTHMS };
	CIntegralEdit m_edit;

	void Init() {
		m_edit.SubclassWindow(GetDlgItem(IDC_EDIT1));
		m_edit.AllowUnsigned(true);
	}
};

class CVolumesDialog : public CTransformDialog<CVolumesDialog>
{
  public:

	enum { IDD = IDD_TRANSFORM_VOLUMES };
	CButton m_radioButton1;
	CButton m_radioButton2;
	CButton m_radioButton3;

	void Init() {
		m_radioButton1.Attach(GetDlgItem(IDC_RADIO1));
		m_radioButton2.Attach(GetDlgItem(IDC_RADIO2));
		m_radioButton3.Attach(GetDlgItem(IDC_RADIO3));
		CheckRadioButton(IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);
	}

	int getCheckedRadio() const {
		if (m_radioButton1.GetCheck()) return 0;
		if (m_radioButton2.GetCheck()) return 1;
		if (m_radioButton3.GetCheck()) return 2;
		return -1;
	}
};

// ---------------------------------------------------------------------------------------------------------------
// PALETTE WINDOWS
// ---------------------------------------------------------------------------------------------------------------

struct CPaletteWindowInitParam {
	HWND mainfrmWnd;
	HWND parentWnd;
};

template <class X>
class CPaletteWindow
{
  public:

	CWindow mainfrmWnd;
	CWindow parentWnd;

	CPaletteWindow() : mainfrmWnd(0), parentWnd(0) {}

	void InitPalette(LPARAM lInitParam) {
		X* pX = static_cast<X*>(this);
		parentWnd = ((CPaletteWindowInitParam*)lInitParam)->parentWnd;
		mainfrmWnd = ((CPaletteWindowInitParam*)lInitParam)->mainfrmWnd;
		pX->SendMessage(WM_NCACTIVATE, (WPARAM)TRUE, -1);
		//pX->SetParent(0);
		pX->ModifyStyle(0, WS_CHILD);
		pX->ModifyStyleEx(0, WS_EX_NOACTIVATE|WS_EX_TOPMOST);
		mainfrmWnd.SendMessage(WM_ADD_PALETTEWINDOW, (WPARAM)pX->m_hWnd, 0);
	}

	void KillPalette() {
		X* pX = static_cast<X*>(this);
		mainfrmWnd.SendMessage(WM_REMOVE_PALETTEWINDOW, (WPARAM)pX->m_hWnd, 0);
	}

	BEGIN_MSG_MAP_EX(CPaletteWindow)
		MESSAGE_HANDLER_EX(WM_NCLBUTTONDOWN, OnNcLButtonDown)
		MESSAGE_HANDLER_EX(WM_NCACTIVATE, OnNcActivate)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_ACTIVATE(OnActivate)
	END_MSG_MAP()

	void OnSetFocus(CWindow wndOld) {}

	void OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther) {}

	LRESULT OnNcLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		X* pX = static_cast<X*>(this);
		SetPaletteFocus();
		::DefWindowProc(pX->m_hWnd, uMsg, wParam, lParam);
		KillPaletteFocus();
		return 0;
	}

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		X* pX = static_cast<X*>(this);
		if ((::GetForegroundWindow() == pX->m_hWnd) || (wParam == TRUE)) {
			if (lParam == 0) {
				return mainfrmWnd.SendMessage(WM_NCACTIVATE, (WPARAM)TRUE, -1);
			} else {
				return ::DefWindowProc(pX->m_hWnd, uMsg, (WPARAM)TRUE, 0);
			}
		} else {
			return ::DefWindowProc(pX->m_hWnd, uMsg, (WPARAM)FALSE, 0);
		}
	}

	void SetPaletteFocus() {
		X* pX = static_cast<X*>(this);
		pX->ModifyStyle(WS_CHILD, WS_POPUP);
	}

	void KillPaletteFocus() {
		X* pX = static_cast<X*>(this);
		pX->ModifyStyle(WS_POPUP, WS_CHILD);
		mainfrmWnd.SetFocus();
	}
};

// ---------------------------------------------------------------------------------------------------------------
// NOTE MASK / TRANSPOSE MASK
// ---------------------------------------------------------------------------------------------------------------

class CNoteMaskDialog
:
	public CDialogImpl<CNoteMaskDialog>,
	public CPaletteWindow<CNoteMaskDialog>
{
  public:

	enum { IDD = IDD_PATTERNVIEW_NOTEMASK };

	BEGIN_MSG_MAP_EX(CNoteMaskDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_CHECK_NOTE_FIRST, IDC_CHECK_NOTE_LAST, BN_CLICKED, OnCheckChange)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_BUTTON_NOTE_FIRST, IDC_BUTTON_NOTE_LAST, BN_CLICKED, OnButtonPress)
		COMMAND_HANDLER_EX(IDC_BUTTON1, BN_CLICKED, OnAllPressed)
		COMMAND_HANDLER_EX(IDC_BUTTON2, BN_CLICKED, OnNonePressed)
		COMMAND_HANDLER_EX(IDC_BUTTON3, BN_CLICKED, OnSelectPressed)
		CHAIN_MSG_MAP(CPaletteWindow<CNoteMaskDialog>)
	END_MSG_MAP()

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
		InitPalette(lInitParam);
		CenterDialogRelative(m_hWnd, parentWnd);
		return FALSE;
	}

	void OnClose() {
		KillPalette();
		DestroyWindow();
		SetMsgHandled(FALSE);
	}

	void SetNoteMask(int (&note_mask)[12][3]) {
		SetRedraw(FALSE);
		{
			for (int i = 0; i < 12; ++i) {
				int state;
				CButton checkbox;

				state = note_mask[i][0];
				checkbox = GetDlgItem(IDC_CHECK_NOTE_META_0_FIRST + i);
				checkbox.SetCheck(state);

				state = note_mask[i][1];
				checkbox = GetDlgItem(IDC_CHECK_NOTE_META_1_FIRST + i);
				checkbox.SetCheck(state);

				state = note_mask[i][2];
				checkbox = GetDlgItem(IDC_CHECK_NOTE_META_2_FIRST + i);
				checkbox.SetCheck(state);
			}
		}
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_ERASE|RDW_INVALIDATE|RDW_ALLCHILDREN);
	}

	void OnCheckChange(UINT uNotifyCode, int nID, CWindow wndCtl) {
		bool state = IsDlgButtonChecked(nID) != 0;

		int notebase;
		int meta;
		if (nID >= IDC_CHECK_NOTE_META_0_FIRST && nID <= IDC_CHECK_NOTE_META_0_LAST) {
			notebase = nID - IDC_CHECK_NOTE_META_0_FIRST;
			meta = 0;
		} else
		if (nID >= IDC_CHECK_NOTE_META_1_FIRST && nID <= IDC_CHECK_NOTE_META_1_LAST) {
			notebase = nID - IDC_CHECK_NOTE_META_1_FIRST;
			meta = 1;
		} else
		if (nID >= IDC_CHECK_NOTE_META_2_FIRST && nID <= IDC_CHECK_NOTE_META_2_LAST) {
			notebase = nID - IDC_CHECK_NOTE_META_2_FIRST;
			meta = 2;
		}

		parentWnd.SendMessage(WM_COMMAND,
			MAKEWPARAM(ID_NOTEMASK_TOGGLE, state), MAKELPARAM(notebase, meta)
		);
	}

	void OnButtonPress(UINT uNotifyCode, int nID, CWindow wndCtl) {
		int check_id = IDC_CHECK_NOTE_FIRST + (nID - IDC_BUTTON_NOTE_FIRST);

		for (int i = IDC_CHECK_NOTE_FIRST; i <= IDC_CHECK_NOTE_LAST; ++i) {
			CButton checkbox = GetDlgItem(i);
			checkbox.SetCheck(i == check_id ? TRUE : FALSE);
		}

		int notebase;
		int meta;
		if (nID >= IDC_BUTTON_NOTE_META_0_FIRST && nID <= IDC_BUTTON_NOTE_META_0_LAST) {
			notebase = nID - IDC_BUTTON_NOTE_META_0_FIRST;
			meta = 0;
		} else
		if (nID >= IDC_BUTTON_NOTE_META_1_FIRST && nID <= IDC_BUTTON_NOTE_META_1_LAST) {
			notebase = nID - IDC_BUTTON_NOTE_META_1_FIRST;
			meta = 1;
		} else
		if (nID >= IDC_BUTTON_NOTE_META_2_FIRST && nID <= IDC_BUTTON_NOTE_META_2_LAST) {
			notebase = nID - IDC_BUTTON_NOTE_META_2_FIRST;
			meta = 2;
		}

		parentWnd.SendMessage(WM_COMMAND,
			MAKEWPARAM(ID_NOTEMASK_SOLO, 0), MAKELPARAM(notebase, meta)
		);
	}

	void OnAllPressed(UINT uNotifyCode, int nID, CWindow wndCtl) {
		SetRedraw(FALSE);
		{
			for (int i = IDC_CHECK_NOTE_FIRST; i <= IDC_CHECK_NOTE_LAST; ++i) {
				CButton checkbox = GetDlgItem(i);
				checkbox.SetCheck(TRUE);
			}
		}
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);

		parentWnd.SendMessage(WM_COMMAND,
			MAKEWPARAM(ID_NOTEMASK_RESET, true), 0
		);
	}

	void OnNonePressed(UINT uNotifyCode, int nID, CWindow wndCtl) {
		SetRedraw(FALSE);
		{
			for (int i = IDC_CHECK_NOTE_FIRST; i <= IDC_CHECK_NOTE_LAST; ++i) {
				CButton checkbox = GetDlgItem(i);
				checkbox.SetCheck(FALSE);
			}
		}
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);

		parentWnd.SendMessage(WM_COMMAND,
			MAKEWPARAM(ID_NOTEMASK_RESET, false), 0
		);
	}

	void OnSelectPressed(UINT uNotifyCode, int nID, CWindow wndCtl) {
		parentWnd.SendMessage(WM_COMMAND,
			MAKEWPARAM(ID_PATTERNVIEW_RESELECT, 0), 0
		);
	}
};

#include "PatternEditor/PatternEditorHarmony.h"

class CHarmonicTransposeDialog
:
	public CDialogImpl<CHarmonicTransposeDialog>,
	public CPaletteWindow<CHarmonicTransposeDialog>
{
  public:

	CContainedWindowT<CComboBox> m_combo1;
	CContainedWindowT<CComboBox> m_combo2;

	enum { IDD = IDD_PATTERNVIEW_HARMONICTRANSPOSE };

	BEGIN_MSG_MAP_EX(CHarmonicTransposeDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_RANGE_CODE_HANDLER_EX(IDC_RADIO_NOTE_FIRST, IDC_RADIO_NOTE_LAST, BN_CLICKED, OnCheckChange)
		COMMAND_HANDLER_EX(IDC_BUTTON1, BN_CLICKED, OnRekey)
		COMMAND_HANDLER_EX(IDC_BUTTON2, BN_CLICKED, OnReset)
		COMMAND_HANDLER_EX(IDC_COMBO1, CBN_CLOSEUP, OnComboCloseup)
		COMMAND_HANDLER_EX(IDC_COMBO2, CBN_CLOSEUP, OnComboCloseup)
		COMMAND_HANDLER_EX(IDC_COMBO1, CBN_SELCHANGE, OnRootChange)
		COMMAND_HANDLER_EX(IDC_COMBO2, CBN_SELCHANGE, OnKeyChange)
		COMMAND_HANDLER_EX(IDC_CHECK1, BN_CLICKED, OnEnabled)
		CHAIN_MSG_MAP(CPaletteWindow<CHarmonicTransposeDialog>)
	ALT_MSG_MAP(1)
		MSG_WM_LBUTTONDOWN(OnComboLButtonDown)
	END_MSG_MAP()

	CHarmonicTransposeDialog()
	:	m_combo1(this, 1),
		m_combo2(this, 1),
		enabled(false)
	{
		swap_set[0].resize(12, -1);
		swap_set[1].resize(12, -1);
	}

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
		m_combo1.SubclassWindow(GetDlgItem(IDC_COMBO1));
		m_combo2.SubclassWindow(GetDlgItem(IDC_COMBO2));

		SetRedraw(FALSE);
		{
			SetupKeySigDropdowns();
			InitPalette(lInitParam);
			CenterDialogRelative(m_hWnd, parentWnd);
		}
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_ERASE|RDW_INVALIDATE|RDW_ALLCHILDREN);

		return FALSE;
	}

	void OnClose() {
		KillPalette();
		DestroyWindow();
		SetMsgHandled(FALSE);
	}

	bool enabled;
	Harmony::KeySigInfo::metavect_t swap_set[2];

	void SetTransposeSet(boost::array<int, 12> const& noteset, bool enabled) {
		using namespace Harmony;
		KeySigInfo::metavect_t& v = swap_set[enabled];

		v.assign(noteset.begin(), noteset.end());
	}

	void OnEnabled(UINT uNotifyCode, int nID, CWindow wndCtl) {
		CButton checkbox = GetDlgItem(IDC_CHECK1);
		this->enabled = checkbox.GetCheck() != 0;
		parentWnd.SendMessage(WM_COMMAND,
			MAKEWPARAM(ID_TRANSPOSESET_ENABLE, enabled), 0
		);
		UpdateChecks(false);
		GreyControls();
	}

	void SetEnabled(bool enabled) {
		this->enabled = enabled;
		CButton checkbox = GetDlgItem(IDC_CHECK1);
		checkbox.SetCheck(enabled);
		UpdateChecks(false);
		GreyControls();
	}

	void GreyControls() {
		for (int i = IDC_RADIO_NOTE_NONE_FIRST; i <= IDC_RADIO_NOTE_NONE_LAST; ++i) {
			CButton checkbox = GetDlgItem(i);
			if (enabled)
				checkbox.EnableWindow(TRUE);
			else
				checkbox.EnableWindow(FALSE);
		}

		CButton button = GetDlgItem(IDC_BUTTON2);
		if (enabled)
			button.EnableWindow(TRUE);
		else
			button.EnableWindow(FALSE);
	}

	void OnCheckChange(UINT uNotifyCode, int nID, CWindow wndCtl) {
		int notebase;
		int meta;

		if (nID >= IDC_RADIO_NOTE_NONE_FIRST && nID <= IDC_RADIO_NOTE_NONE_LAST) {
			notebase = nID - IDC_RADIO_NOTE_NONE_FIRST;
			meta = -1;
		} else
		if (nID >= IDC_RADIO_NOTE_META_0_FIRST && nID <= IDC_RADIO_NOTE_META_0_LAST) {
			notebase = nID - IDC_RADIO_NOTE_META_0_FIRST;
			meta = 0;
		} else
		if (nID >= IDC_RADIO_NOTE_META_1_FIRST && nID <= IDC_RADIO_NOTE_META_1_LAST) {
			notebase = nID - IDC_RADIO_NOTE_META_1_FIRST;
			meta = 1;
		} else
		if (nID >= IDC_RADIO_NOTE_META_2_FIRST && nID <= IDC_RADIO_NOTE_META_2_LAST) {
			notebase = nID - IDC_RADIO_NOTE_META_2_FIRST;
			meta = 2;
		}

		parentWnd.SendMessage(WM_COMMAND,
			MAKEWPARAM(ID_TRANSPOSESET_UPDATE, 0), MAKELPARAM(notebase, meta)
		);

		swap_set[enabled][notebase] = meta;
	}

	void OnComboLButtonDown(UINT nFlags, CPoint point) {
		SetPaletteFocus();
		SetMsgHandled(FALSE);
	}

	void OnComboCloseup(UINT uNotifyCode, int nID, CWindow wndCtl) {
		KillPaletteFocus();
	}

	void OnRootChange(UINT uNotifyCode, int nID, CWindow wndCtl) {
		UpdateKeySig();
		UpdateChecks(true);
	}

	void OnKeyChange(UINT uNotifyCode, int nID, CWindow wndCtl) {
		UpdateKeySig();
		UpdateChecks(true);
	}

	void OnRekey(UINT uNotifyCode, int nID, CWindow wndCtl) {
		parentWnd.SendMessage(WM_COMMAND,
			MAKEWPARAM(ID_TRANSPOSESET_REKEY, 0), 0
		);
	}

	void OnReset(UINT uNotifyCode, int nID, CWindow wndCtl) {
		swap_set[enabled].assign(12, -1);
		UpdateChecks(true);
	}

	void UpdateKeySig() {
		using namespace Harmony;
		KeySigInfo& info = KeySigInfo::instance();

		int curSel1 = m_combo1.GetCurSel();
		if (curSel1 == -1) return;
		char pcRootName[16];
		m_combo1.GetLBText(curSel1, pcRootName);

		int curSel2 = m_combo2.GetCurSel();
		if (curSel2 == -1) return;
		char pcKeyName[64];
		m_combo2.GetLBText(curSel2, pcKeyName);

		bool chromatic = !enabled;

		swap_set[enabled] = info.GetKeySigMetas(pcRootName, pcKeyName, chromatic);
	}

	void UpdateChecks(bool notify) {
		SetRedraw(FALSE);
		{
			for (int i = IDC_RADIO_NOTE_FIRST; i <= IDC_RADIO_NOTE_LAST; ++i) {
				CButton checkbox = GetDlgItem(i);
				checkbox.SetCheck(FALSE);
			}

			for (int i = 0; i < 12; ++i) {
				int notebase = i;
				int meta = swap_set[enabled][i];

				int dlg_x;
				if (meta == -1) {
					dlg_x = IDC_RADIO_NOTE_NONE_FIRST;
				} else
				if (meta == 0) {
					dlg_x = IDC_RADIO_NOTE_META_0_FIRST;
				} else
				if (meta == 1) {
					dlg_x = IDC_RADIO_NOTE_META_1_FIRST;
				} else
				if (meta == 2) {
					dlg_x = IDC_RADIO_NOTE_META_2_FIRST;
				}

				CButton checkbox = GetDlgItem(dlg_x + notebase);
				checkbox.SetCheck(TRUE);

				if (notify) {
					parentWnd.SendMessage(WM_COMMAND,
						MAKEWPARAM(ID_TRANSPOSESET_UPDATE, 0), MAKELPARAM(notebase, meta)
					);
				}
			}
		}
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);
	}

	void SetupKeySigDropdowns() {
		using namespace Harmony;
		KeySigInfo& info = KeySigInfo::instance();

		m_combo1.ResetContent();
		m_combo1.InitStorage(35, 128);
		for (KeySigInfo::commons_t::iterator i = info.commons.begin(); i != info.commons.end(); ++i) {
			m_combo1.AddString((*i).c_str());
		}
		//m_combo1.SetCurSel(m_combo1.FindStringExact(-1, "C"));

		m_combo2.ResetContent();
		m_combo2.InitStorage(35, 128);
		for (KeySigInfo::keynames_t::iterator i = info.keynames.begin(); i != info.keynames.end(); ++i) {
			m_combo2.AddString((*i).c_str());
		}
		//m_combo2.SelectString(-1, "Chromatic Balanced");
	}
};
