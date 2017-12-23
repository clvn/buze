#pragma once

template <class T, class T2>
class CChainForwardKey : public T2
{
	BEGIN_MSG_MAP_EX(T)
		MESSAGE_HANDLER_EX(WM_XBUTTONDOWN, OnForward)
		MESSAGE_HANDLER_EX(WM_RBUTTONDOWN, OnForwardMouse)
		MESSAGE_HANDLER_EX(WM_KILLFOCUS, OnForward)
		MESSAGE_HANDLER_EX(WM_KEYDOWN, OnForward)
		MESSAGE_HANDLER_EX(WM_KEYUP, OnForward)
		MESSAGE_HANDLER_EX(WM_CHAR, OnForward)
		MESSAGE_HANDLER_EX(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		CHAIN_MSG_MAP(T2)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	int dblclick_split_pos;

	CChainForwardKey() {
		dblclick_split_pos = -1; // -1 is automatic middle
	}

	LRESULT OnForward(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		return GetParent().SendMessage(uMsg, wParam, lParam);
	}

	LRESULT OnForwardMouse(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		// adjust clicked x, y to parent
		RECT rc;
		GetClientRect(&rc);
		ClientToScreen(&rc);
		GetParent().ScreenToClient(&rc);
		POINT pt = { (signed short)LOWORD(lParam) + rc.left, (signed short)HIWORD(lParam) + rc.top };
		return GetParent().SendMessage(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
	}

	void SetDoubleClickSplitterPos(int xyPos) {
		dblclick_split_pos = xyPos;
	}

	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		T2* pT = static_cast<T2*>(this);
		pT->SetSplitterPos(dblclick_split_pos);
		return 0;
	}
};

class CSplitterWindowKey : public CChainForwardKey<CSplitterWindowKey, CSplitterWindow> {
  public:
	DECLARE_WND_SUPERCLASS("SplitterWindowKey", CSplitterWindow::GetWndClassName())
};

class CHorSplitterWindowKey : public CChainForwardKey<CHorSplitterWindowKey, CHorSplitterWindow> {
  public:
	DECLARE_WND_SUPERCLASS("HorSplitterWindowKey", CHorSplitterWindow::GetWndClassName())
};
