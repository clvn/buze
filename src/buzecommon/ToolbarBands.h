#pragma once

#include "VisualStylesXP.h"

// ---------------------------------------------------------------------------------------------------------------
// THEME SUPPORT MIXIN
// ---------------------------------------------------------------------------------------------------------------

template <class T>
class CThemedControl
{
  public:

	BEGIN_MSG_MAP_EX(CThemedControl<T>)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		T* pT = static_cast<T*>(this);
		CDCHandle dc = (HDC)wParam;

		if (g_xpStyle.UseVisualStyles()) {
			g_xpStyle.DrawThemeParentBackground(pT->m_hWnd, dc, 0);
		} else {
			RECT rc;
			pT->GetClientRect(&rc);
			dc.FillSolidRect(0, 0, rc.right, rc.bottom, ::GetSysColor(COLOR_BTNFACE));
		}

		return 1;
	}
};

// ---------------------------------------------------------------------------------------------------------------
// CHECKBOX
// ---------------------------------------------------------------------------------------------------------------

class CCheckboxBand
:
	public CWindowImpl<CCheckboxBand>,
	public CThemedControl<CCheckboxBand>
{
  public:

	DECLARE_WND_CLASS("CheckboxBand")

	BEGIN_MSG_MAP_EX(CCheckboxBand)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CThemedControl<CCheckboxBand>)
		FORWARD_NOTIFICATIONS_DLGCTRLID()
	END_MSG_MAP()

	CButton button;
	CButton& ctrl() { return button; }

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		LRESULT lRes = DefWindowProc();
		button.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|BS_CHECKBOX|BS_NOTIFY, 0);
		return lRes;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		RECT rc;
		GetClientRect(&rc);
		int btnWidth = GetSystemMetrics(SM_CXMENUCHECK);
		int btnHeight = GetSystemMetrics(SM_CYMENUCHECK);
		int yTop = (rc.bottom - btnHeight) / 2; // vcenter
		button.MoveWindow(0, yTop, btnWidth, btnHeight);
		return 0;
	}
};

// ---------------------------------------------------------------------------------------------------------------
// EDIT
// ---------------------------------------------------------------------------------------------------------------

#define WM_EDITBAND_ACCEPT (WM_USER+1)
#define WM_EDITBAND_CANCEL (WM_USER+2)

class CEditBand
:
	public CWindowImpl<CEditBand>,
	public CThemedControl<CEditBand>
{
  public:

	CContainedWindowT<CEdit> m_edit;
	bool dirtyReselect;
	bool blockMouse;

	DECLARE_WND_CLASS("EditBand")

	BEGIN_MSG_MAP_EX(CEditBand)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_SIZE(OnSize)
		CHAIN_MSG_MAP(CThemedControl<CEditBand>)
		FORWARD_NOTIFICATIONS_DLGCTRLID()
	ALT_MSG_MAP(1)
		MSG_WM_KEYDOWN(OnEditKeyDown)
		MSG_WM_SETFOCUS(OnEditSetFocus)
		//MSG_WM_LBUTTONDOWN(OnEditLButtonDown)
		MSG_WM_LBUTTONUP(OnEditLButtonUp)
		MSG_WM_MOUSEMOVE(OnEditMouseMove)
		MSG_WM_LBUTTONDBLCLK(OnEditLButtonDblClk)
		MSG_WM_PAINT(OnEditPaint)
		MSG_WM_MOUSEACTIVATE(OnEditMouseActivate)
	END_MSG_MAP()

	CEdit& ctrl() { return m_edit; }

	CEditBand()
	:
		m_edit(this, 1)
	{
		dirtyReselect = false;
		blockMouse = false;
	}

	int OnCreate(LPCREATESTRUCT lpCreateStruct) {
		LRESULT lRes = DefWindowProc();
		m_edit.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_WANTRETURN|ES_AUTOHSCROLL, WS_EX_CLIENTEDGE);
		m_edit.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		SetupStyles();
		return (int)lRes;
	}

	void SetupStyles() {
		m_edit.ModifyStyle(ES_NOHIDESEL, 0);
	}

	void OnEditKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
		switch (nChar) {
			case VK_ESCAPE:
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WM_EDITBAND_CANCEL), 0);
				break;
			case VK_RETURN:
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WM_EDITBAND_ACCEPT), 0);
				break;
			default:
				SetMsgHandled(FALSE);
				break;
		}
	}

	void OnEditSetFocus(CWindow wndOld) {
		if ((wndOld != m_edit) || (wndOld != m_hWnd)) {
			dirtyReselect = true;
			blockMouse = true;
		}
		SetMsgHandled(FALSE);
	}

	void OnEditPaint(CDCHandle dc) {
		if (dirtyReselect) {
			dirtyReselect = false;
			m_edit.SetSelAll();
		}
		SetMsgHandled(FALSE);
	}

// 	void OnEditLButtonDown(UINT nFlags, CPoint point) {
// 		if (!blockMouse) SetMsgHandled(FALSE);
// 	}

	void OnEditMouseMove(UINT nFlags, CPoint point) {
		if (!blockMouse) SetMsgHandled(FALSE);
	}

	void OnEditLButtonUp(UINT nFlags, CPoint point) {
		if (blockMouse) blockMouse = false;
		SetMsgHandled(FALSE);
	}

	void OnEditLButtonDblClk(UINT nFlags, CPoint point) {
		m_edit.SetSelAll();
	}

	void OnSize(UINT nType, CSize size) {
		RECT rc;
		GetClientRect(&rc);

		int comboHeight = 20;// GetSystemMetrics(SM_CYVSCROLL) + (GetSystemMetrics(SM_CYEDGE) * 2);

		int yTop = (rc.bottom - comboHeight) / 2; // vcenter
		m_edit.MoveWindow(0, yTop, rc.right, comboHeight);
	}

	// bugfix -- process before docktabframe
	int OnEditMouseActivate(CWindow wndTopLevel, UINT nHitTest, UINT message) {
		if ((GetFocus() == m_edit) || (GetFocus() == m_hWnd))
			return MA_ACTIVATE;
		else
			return (int)m_edit.DefWindowProc();
	}
};

// ---------------------------------------------------------------------------------------------------------------
// NUMERIC COMBO
// ---------------------------------------------------------------------------------------------------------------

#define WM_NUMERICCOMBOBOX_ACCEPT (WM_USER+1)
#define WM_NUMERICCOMBOBOX_CANCEL (WM_USER+2)

typedef CWinTraits<WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|CBS_DROPDOWN|WS_BORDER, 0> CNumericComboBoxTraits;

class CNumericComboBox
:
	public CWindowImpl<CNumericComboBox, CComboBox, CNumericComboBoxTraits>
{
  public:

	CContainedWindowT<CEdit> m_edit;
	bool killFocusing;

	DECLARE_WND_SUPERCLASS("NumericComboBox", CComboBox::GetWndClassName())

	BEGIN_MSG_MAP_EX(CNumericComboBox)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_SIZE(OnSize)
		MSG_WM_MOVE(OnMove)
	ALT_MSG_MAP(1)
		MSG_WM_LBUTTONDBLCLK(OnEditLButtonDblClk)
		MSG_WM_KEYDOWN(OnEditKeyDown)
		MSG_WM_CHAR(OnEditChar)
		MSG_WM_PASTE(OnEditPaste)
		MSG_WM_MOUSEACTIVATE(OnEditMouseActivate)
		MSG_WM_KILLFOCUS(OnEditKillFocus)
		MESSAGE_HANDLER_EX(EM_SETSEL, OnEditSetSel)
	END_MSG_MAP()

	CNumericComboBox()
	:
		m_edit(this, 1)
	{
		killFocusing = false;
	}

	int GetNumber() const {
		char pc[33];
		GetWindowText(pc, 33);
		return atoi(pc);
	}

	void SetNumber(int x) {
		char pc[33];
		itoa(x, pc, 10);
		int idx = FindString(-1, pc);
		if (idx == -1) {
			SetRedraw(FALSE);
			SetCurSel(-1);
			SetWindowText(pc);
			SetRedraw(TRUE);
		} else {
			SetCurSel(idx);
		}
	}

	void ClearNumber() {
		SetCurSel(-1);
	}

	BOOL SubclassWindow(HWND hWnd) {
		BOOL bRet = CWindowImplBaseT<CComboBox, CNumericComboBoxTraits>::SubclassWindow(hWnd);
		if (bRet) {
			HWND edit_hWnd = GetEditCtrl();
			ATLASSERT(::IsWindow(edit_hWnd));
			m_edit.SubclassWindow(edit_hWnd);
			SetupStyles();
		}
		return bRet;
	}

  protected:

	int OnCreate(LPCREATESTRUCT lpCreateStruct) {
		LRESULT lRes = DefWindowProc();
		HWND edit_hWnd = GetEditCtrl();
		ATLASSERT(::IsWindow(edit_hWnd));
		m_edit.SubclassWindow(edit_hWnd);
		SetupStyles();
		AlignEdit();
		return (int)lRes;
	}

	void SetupStyles() {
		m_edit.ModifyStyle(ES_NOHIDESEL, 0);
	}

	HWND GetEditCtrl() const {
		return FindWindowEx(m_hWnd, 0, CEdit::GetWndClassName(), 0);
	}

	// hack cause the Edit's text wasn't valigned with the other CBS_DROPDOWNLIST's texts
	void OnSize(UINT nType, CSize size) {
		DefWindowProc();
		AlignEdit();
	}

	void OnMove(CPoint ptPos) {
		DefWindowProc();
		AlignEdit();
	}

	void AlignEdit() {
		// NOTE: the alignment code moves the textbox a pixel down to match the text position in other dropdowns.
		// on xp this is fine. on win7 there are unwanted black pixel artifacts above. so this must be done differently:
		/*
		if (!m_edit.m_hWnd) return ;	// get here during DefWndProc in OnCreate(), where m_edit isnt hooked up yet
		CRect rcEdit;
		m_edit.GetClientRect(&rcEdit);
		CRect rcCombo;
		GetClientRect(&rcCombo);

		m_edit.ClientToScreen(&rcEdit);
		ScreenToClient(&rcEdit);

		int yTop = ((rcCombo.bottom - rcEdit.Height()) / 2)+1; // vcenter+1
		m_edit.MoveWindow(rcEdit.left, yTop, rcEdit.Width(), rcEdit.Height());*/
	}

	void OnEditLButtonDblClk(UINT nFlags, CPoint point) {
		m_edit.SetSelAll();
	}

	void OnEditKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
		switch (nChar) {
			case VK_ESCAPE:
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WM_NUMERICCOMBOBOX_CANCEL), 0);
				break;
			case VK_RETURN:///during dropdown?
				SetRedraw(FALSE);
				SetNumber(GetNumber());
				SetRedraw(TRUE);
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WM_NUMERICCOMBOBOX_ACCEPT), 0);
				break;
			case VK_SPACE:
				if (GetDroppedState() == FALSE)
					ShowDropDown(TRUE);
				else
					ShowDropDown(FALSE);
				break;
			default:
				SetMsgHandled(FALSE);
				break;
		}
	}

	// decided not to use ES_NUMBER cause it pops up tooltip
	void OnEditChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
		bool ctrl_down = (GetKeyState(VK_CONTROL) & 0x80000000) != 0;
		if (0
			|| isdigit((char)nChar)
			|| (nChar == 0x08) // backspace
			|| (nChar == 0x0A) // linefeed
			|| (nChar == 0x1B) // escape
			|| (nChar == 0x09) // tab
			|| (nChar == 0x0D) // return
			|| (ctrl_down && (nChar == 0x03)) // copy
			|| (ctrl_down && (nChar == 0x16)) // paste
			|| (ctrl_down && (nChar == 0x18)) // cut
			|| (ctrl_down && (nChar == 0x1a)) // undo
		) {
			SetMsgHandled(FALSE);
		}
	}

	// validates numeric pasting
	void OnEditPaste() {
		if (!OpenClipboard()) return;

		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData == 0) {
			CloseClipboard();
			return;
		}

		char* charbuf = (char*)GlobalLock(hData);
		size_t bufferSize = GlobalSize(hData);

		if (charbuf != 0) {
			if (strspn(charbuf, "0123456789") == bufferSize-1) {
				SetMsgHandled(FALSE);
			}
		}

		GlobalUnlock(hData);
		CloseClipboard();
	}

	// fixes various behaviours that broke due to DockTabFrame.
	int OnEditMouseActivate(CWindow wndTopLevel, UINT nHitTest, UINT message) {
		if ((GetFocus() == m_edit) || (GetFocus() == m_hWnd))
			return MA_ACTIVATE;
		else
			return (int)m_edit.DefWindowProc();
	}

	// fixes a bug in windows common control which causes the Edit text to be reselected
	// when the ComboBox is resized. happens when moving the toolbar band.
	void OnEditKillFocus(CWindow wndFocus) {
		killFocusing = true;
		SetMsgHandled(FALSE);
	}
	// ---
	LRESULT OnEditSetSel(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if ((GetFocus() == m_edit) || killFocusing) {
			killFocusing = false;
			return m_edit.DefWindowProc();
		}
		return 0;
	}
};

class CNumericComboBand
:
	public CWindowImpl<CNumericComboBand>,
	public CThemedControl<CNumericComboBand>
{
  public:

	DECLARE_WND_CLASS("NumericComboBand")

	BEGIN_MSG_MAP_EX(CNumericComboBand)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CThemedControl<CNumericComboBand>)
		FORWARD_NOTIFICATIONS_DLGCTRLID()
	END_MSG_MAP()

	CNumericComboBox combo;
	CNumericComboBox& ctrl() { return combo; }

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		LRESULT lRes = DefWindowProc();
		combo.Create(m_hWnd, rcDefault);
		combo.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		return lRes;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		RECT rc;
		GetClientRect(&rc);
		int comboHeight = GetSystemMetrics(SM_CYVSCROLL) + (GetSystemMetrics(SM_CYEDGE) * 2);
		int yTop = (rc.bottom - comboHeight) / 2; // vcenter
		combo.MoveWindow(0, yTop, rc.right, comboHeight+200);

		return 0;
	}
};

// ---------------------------------------------------------------------------------------------------------------
// FLOATING EDIT
// ---------------------------------------------------------------------------------------------------------------

#define WM_FLOATINGEDIT_ACCEPT (WM_USER+1)
#define WM_FLOATINGEDIT_CANCEL (WM_USER+2)

class CFloatingEdit
:
	public CWindowImpl<CFloatingEdit, CEdit>
{
  public:

	DECLARE_WND_SUPERCLASS("FloatingEdit", CEdit::GetWndClassName())

	BEGIN_MSG_MAP_EX(CFloatingEdit)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_CHAR(OnChar)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_PASTE(OnPaste)
	END_MSG_MAP()

	CFloatingEdit() : dirtyReselect(false) {}

	bool dirtyReselect;
	bool blockMouse;

	void OnSetFocus(CWindow wndOld) {
		//if (wndOld != m_hWnd) {
			dirtyReselect = true;
			blockMouse = true;
		//}
		SetMsgHandled(FALSE);
	}
	
	void OnMouseMove(UINT nFlags, CPoint point) {
		if (!blockMouse) SetMsgHandled(FALSE);
	}

	void OnLButtonUp(UINT nFlags, CPoint point) {
		if (blockMouse) blockMouse = false;
		SetMsgHandled(FALSE);
	}

	void OnPaint(CDCHandle dc) {
		if (dirtyReselect) {
			dirtyReselect = false;
			SetSelAll();
		}
		SetMsgHandled(FALSE);
	}

	bool GetDouble(double& x) const {
		char pc[33];
		GetWindowText(pc, 33);
		std::stringstream ss;
		ss << pc;
		if (ss >> x)
			return true;
		else
			return false;
	}

	void SetDouble(double x) {
		std::stringstream ss;
		ss << x;
		SetWindowText(ss.str().c_str());
	}

	void OnLButtonDblClk(UINT nFlags, CPoint point) {
		//SetSelAll();
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
		switch (nChar) {
			case VK_ESCAPE:
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WM_FLOATINGEDIT_CANCEL), 0);
				break;
			case VK_RETURN:
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WM_FLOATINGEDIT_ACCEPT), 0);
				break;
			default:
				SetMsgHandled(FALSE);
				break;
		}
	}

	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
		// Special chars
		{
			bool ctrl_down = (GetKeyState(VK_CONTROL) & 0x80000000) != 0;
			if (0
				|| (nChar == 0x08) // backspace
				|| (nChar == 0x0A) // linefeed
				|| (nChar == 0x1B) // escape
				|| (nChar == 0x09) // tab
				|| (nChar == 0x0D) // return
				|| (ctrl_down && (nChar == 0x03)) // copy
				|| (ctrl_down && (nChar == 0x16)) // paste
				|| (ctrl_down && (nChar == 0x18)) // cut
				|| (ctrl_down && (nChar == 0x1a)) // undo
			) {
				SetMsgHandled(FALSE);
				return;
			}
		}

		ValidateInput();
	}

	void OnPaste() {
		ValidateInput();
	}

	void ValidateInput() {
		char pc_old[50];
		GetWindowText(pc_old, 50);
		int old_from, old_to;
		GetSel(old_from, old_to);

		SetRedraw(FALSE);
		{
			DefWindowProc(); // does the WM_PASTE or WM_CHAR

			char pc_new[50];
			GetWindowText(pc_new, 50);

			bool restore = true;

			std::stringstream ss;
			ss << pc_new;
			double x;
			if (0
				|| (ss >> x)
				|| (ss.str() == ".")
			) {
				std::string remainder;
				if (!getline(ss, remainder)) {
					restore = false;
				}
			}

			if (restore) {
				SetWindowText(pc_old);
				SetSel(old_from, old_to);
			}
		}
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_INVALIDATE);
	}
};

// ---------------------------------------------------------------------------------------------------------------
// INTEGRAL EDIT
// ---------------------------------------------------------------------------------------------------------------

#define WM_INTEGRALEDIT_ACCEPT (WM_USER+1)
#define WM_INTEGRALEDIT_CANCEL (WM_USER+2)

class CIntegralEdit
:
	public CWindowImpl<CIntegralEdit, CEdit>
{
  public:

	DECLARE_WND_SUPERCLASS("IntegralEdit", CEdit::GetWndClassName())

	BEGIN_MSG_MAP_EX(CIntegralEdit)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_CHAR(OnChar)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_PASTE(OnPaste)
	END_MSG_MAP()

	CIntegralEdit() : dirtyReselect(false), blockMouse(false), allow_unsigned(false) {}

	bool dirtyReselect;
	bool blockMouse;
	bool allow_unsigned;

	void OnSetFocus(CWindow wndOld) {
		//if (wndOld != m_hWnd) {
			dirtyReselect = true;
			blockMouse = true;
		//}
		SetMsgHandled(FALSE);
	}

	void OnMouseMove(UINT nFlags, CPoint point) {
		if (!blockMouse) SetMsgHandled(FALSE);
	}

	void OnLButtonUp(UINT nFlags, CPoint point) {
		if (blockMouse) blockMouse = false;
		SetMsgHandled(FALSE);
	}

	void OnPaint(CDCHandle dc) {
		if (dirtyReselect) {
			dirtyReselect = false;
			SetSelAll();
		}
		SetMsgHandled(FALSE);
	}

	bool GetInt(int& x) const {
		char pc[33];
		GetWindowText(pc, 33);
		std::stringstream ss;
		ss << pc;
		if (ss >> x)
			return true;
		else
			return false;
	}

	void SetInt(int x) {
		std::stringstream ss;
		ss << x;
		SetWindowText(ss.str().c_str());
	}

	void ClearInt() {
		SetWindowText("");
	}

	void AllowUnsigned(bool allow) {
		allow_unsigned = allow;
	}

	void OnLButtonDblClk(UINT nFlags, CPoint point) {
		//SetSelAll();
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
		switch (nChar) {
			case VK_ESCAPE:
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WM_INTEGRALEDIT_CANCEL), 0);
				break;
			case VK_RETURN:
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WM_INTEGRALEDIT_ACCEPT), 0);
				break;
			default:
				SetMsgHandled(FALSE);
				break;
		}
	}

	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) {
		// Special chars
		{
			bool ctrl_down = (GetKeyState(VK_CONTROL) & 0x80000000) != 0;
			if (0
				|| (nChar == 0x08) // backspace
				|| (nChar == 0x0A) // linefeed
				|| (nChar == 0x1B) // escape
				|| (nChar == 0x09) // tab
				|| (nChar == 0x0D) // return
				|| (ctrl_down && (nChar == 0x03)) // copy
				|| (ctrl_down && (nChar == 0x16)) // paste
				|| (ctrl_down && (nChar == 0x18)) // cut
				|| (ctrl_down && (nChar == 0x1a)) // undo
			) {
				SetMsgHandled(FALSE);
				return;
			}
		}

		ValidateInput();
	}

	void OnPaste() {
		ValidateInput();
	}

	void ValidateInput() {
		char pc_old[50];
		GetWindowText(pc_old, 50);
		int old_from, old_to;
		GetSel(old_from, old_to);

		SetRedraw(FALSE);
		{
			DefWindowProc(); // does the WM_PASTE or WM_CHAR

			char pc_new[50];
			GetWindowText(pc_new, 50);

			bool restore = true;

			std::stringstream ss;
			ss << pc_new;
			int x;
			if (0
				|| (ss >> x)
				|| (allow_unsigned && (ss.str() == "-"))
			) {
				std::string remainder;
				if (!getline(ss, remainder)) {
					restore = false;
				}
			}

			if (!allow_unsigned && (x < 0))
				restore = true;

			if (restore) {
				SetWindowText(pc_old);
				SetSel(old_from, old_to);
			}
		}
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_INVALIDATE);
	}
};

// ---------------------------------------------------------------------------------------------------------------
// COMBO
// ---------------------------------------------------------------------------------------------------------------

class CComboBand
:
	public CWindowImpl<CComboBand>,
	public CThemedControl<CComboBand>
{
  public:

	DECLARE_WND_CLASS("ComboBand")

	BEGIN_MSG_MAP_EX(CComboBand)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CThemedControl<CComboBand>)
		FORWARD_NOTIFICATIONS_DLGCTRLID()
	END_MSG_MAP()

	CComboBox combo;
	CComboBox& ctrl() { return combo; }

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		LRESULT lRes = DefWindowProc();
		combo.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWN/*|CCS_NORESIZE*/, 0);
		combo.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		return lRes;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		RECT rc;
		GetClientRect(&rc);
		int comboHeight = GetSystemMetrics(SM_CYVSCROLL) + (GetSystemMetrics(SM_CYEDGE) * 2);
		int yTop = (rc.bottom - comboHeight) / 2; // vcenter
		combo.MoveWindow(0, yTop, rc.right, comboHeight+200);

		return 0;
	}
};

// ---------------------------------------------------------------------------------------------------------------
// COMBO LIST
// ---------------------------------------------------------------------------------------------------------------

class CComboListBand
:
	public CWindowImpl<CComboListBand>,
	public CThemedControl<CComboListBand>
{
  public:

	DECLARE_WND_CLASS("ComboListBand")

	BEGIN_MSG_MAP_EX(CComboListBand)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CThemedControl<CComboListBand>)
		FORWARD_NOTIFICATIONS_DLGCTRLID()
	END_MSG_MAP()

	CComboBox combo;
	CComboBox& ctrl() { return combo; }

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		LRESULT lRes = DefWindowProc();
		combo.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST/*|CCS_NORESIZE*/, 0);
		combo.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		return lRes;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		RECT rc;
		GetClientRect(&rc);
		int comboHeight = GetSystemMetrics(SM_CYVSCROLL) + (GetSystemMetrics(SM_CYEDGE) * 2);
		int yTop = (rc.bottom - comboHeight) / 2; // vcenter
		combo.MoveWindow(0, yTop, rc.right, comboHeight+200);

		return 0;
	}
};

// ---------------------------------------------------------------------------------------------------------------
// TWIN COMBO LIST
// ---------------------------------------------------------------------------------------------------------------

class CTwinComboListBand
:
	public CWindowImpl<CTwinComboListBand>,
	public CThemedControl<CTwinComboListBand>
{
  public:

	DECLARE_WND_CLASS("TwinComboListBand")

	BEGIN_MSG_MAP_EX(CTwinComboListBand)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CThemedControl<CTwinComboListBand>)
		FORWARD_NOTIFICATIONS_DLGCTRLID()
	END_MSG_MAP()

	CComboBox combo1;
	CComboBox combo2;
	CComboBox& ctrl(int combo) {
		CComboBox* cc[] = { &combo1, &combo2 };
		return *cc[combo];
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		LRESULT lRes = DefWindowProc();
		combo1.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST, 0);
		combo1.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		combo2.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST, 0);
		combo2.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		return lRes;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		RECT rc;
		GetClientRect(&rc);
		int comboHeight = GetSystemMetrics(SM_CYVSCROLL) + (GetSystemMetrics(SM_CYEDGE) * 2);
		int yTop = (rc.bottom - comboHeight) / 2; // vcenter
		combo1.MoveWindow(0, yTop, rc.right/2, comboHeight+200);
		combo2.MoveWindow(rc.right/2, yTop, rc.right/2, comboHeight+200);
		return 0;
	}

	int GetComboIndex(HWND hWnd) {
		if (combo1.m_hWnd == hWnd) return 0;
		if (combo2.m_hWnd == hWnd) return 1;
		return -1;
	}

	int GetCurSel(int combo) {
		CComboBox* cc[] = { &combo1, &combo2 };
		return cc[combo]->GetCurSel();
	}

	int GetCount(int combo) {
		CComboBox* cc[] = { &combo1, &combo2 };
		return cc[combo]->GetCount();
	}

	void DeleteString(int combo, int index) {
		CComboBox* cc[] = { &combo1, &combo2 };
		cc[combo]->DeleteString(index);
	}

	void InsertString(int combo, int index, LPCSTR lpcstr) {
		CComboBox* cc[] = { &combo1, &combo2 };
		cc[combo]->InsertString(index, lpcstr);
	}

	void SelectString(int combo, int index, LPCSTR lpcstr) {
		CComboBox* cc[] = { &combo1, &combo2 };
		int exact = cc[combo]->FindStringExact(index, lpcstr);
		if (exact == -1) return;
		cc[combo]->SetCurSel(exact);
	}

	void SetCurSel(int combo, int index) {
		CComboBox* cc[] = { &combo1, &combo2 };
		cc[combo]->SetCurSel(index);
	}

	virtual void SetRedraw(BOOL bRedraw = 1) {
		combo1.SetRedraw(bRedraw);
		combo2.SetRedraw(bRedraw);
	}
};
