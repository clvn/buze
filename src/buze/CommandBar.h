#pragma once

// ---------------------------------------------------------------------------------------------------------------
// BUZE COMMAND BAR
// ---------------------------------------------------------------------------------------------------------------

class CBuzeCommandBarCtrl : public CCommandBarCtrlImpl<CBuzeCommandBarCtrl>
{
  public:

	DECLARE_WND_SUPERCLASS(_T("Buze_CommandBar"), GetWndClassName())

	BEGIN_MSG_MAP_EX(CBuzeCommandBarCtrl)
		CHAIN_MSG_MAP_ALT(CCommandBarCtrlImpl<CBuzeCommandBarCtrl>, 0)
	ALT_MSG_MAP(1)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnParentSysCommand)
		CHAIN_MSG_MAP_ALT(CCommandBarCtrlImpl<CBuzeCommandBarCtrl>, 1)
	ALT_MSG_MAP(2)
		CHAIN_MSG_MAP_ALT(CCommandBarCtrlImpl<CBuzeCommandBarCtrl>, 2)
	ALT_MSG_MAP(3)
		MSG_WM_KEYUP(OnHookKeyUp)
		MSG_WM_SYSKEYUP(OnHookSysKeyUp)
		CHAIN_MSG_MAP_ALT(CCommandBarCtrlImpl<CBuzeCommandBarCtrl>, 3)
	END_MSG_MAP()

	LRESULT OnParentSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		bHandled = FALSE;

		if (wParam == SC_KEYMENU) {
			if (m_uSysKey == VK_MENU || m_uSysKey == VK_SPACE) {
				if (::GetFocus() == m_hWnd) {
					GiveFocusBack();
					PostMessage(TB_SETHOTITEM, (WPARAM)-1, 0L);
				} else
				if (m_uSysKey != VK_SPACE && !m_bSkipMsg) {
					if (m_bUseKeyboardCues && m_bShowKeyboardCues && m_bAllowKeyboardCues)
						ShowKeyboardCues(false);

 					bHandled = TRUE;
				} else
				if (m_uSysKey != VK_SPACE) {
					bHandled = TRUE;
				}
			}
		}

		m_bSkipMsg = false;
		return 0;
	}

	void OnHookKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
		if (nChar == VK_MENU) {
			if ((::GetFocus() != m_hWnd) && m_bUseKeyboardCues && m_bShowKeyboardCues && m_bAllowKeyboardCues) {
				ShowKeyboardCues(false);
			}
		}

		SetMsgHandled(FALSE);
	}

	// needed in case of Alt+Clicking or other ways it could get stuck
	void OnHookSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
		if (!m_bAllowKeyboardCues)
			m_bAllowKeyboardCues = true;

		if (nChar == VK_MENU) {
			if (m_bUseKeyboardCues && m_bShowKeyboardCues && m_bAllowKeyboardCues) {
				ShowKeyboardCues(false);
			}
		}

		SetMsgHandled(FALSE);
	}

	void GiveFocusBack() {
		if (m_bParentActive && (GetFocus() == m_hWnd)) {
			if ((m_dwExtendedStyle & CBR_EX_ALTFOCUSMODE) && ::IsWindow(m_hWndFocus)) {
				::SetFocus(m_hWndFocus);
			} else
			if (!(m_dwExtendedStyle & CBR_EX_ALTFOCUSMODE) && m_wndParent.IsWindow()) {
				m_wndParent.SetFocus();
			}
		}
		m_hWndFocus = NULL;
		SetAnchorHighlight(FALSE);
		if (m_bUseKeyboardCues && m_bShowKeyboardCues) {
			ShowKeyboardCues(false);
		}
		m_bSkipPostDown = false;
	}
};
