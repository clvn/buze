#pragma once

class CMainFrame;

class CCustomView 
	: public CWindowImpl<CCustomView>
	, public CMessageFilter
{
public:
	CMainFrame* mainFrame;
	HWND hViewWnd;
	BOOL (*fnPreTranslateMessage)(MSG*);

	DECLARE_WND_CLASS("CustomView")

	BEGIN_MSG_MAP(CCustomView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	CCustomView(CMainFrame* mf);
	~CCustomView();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	HWND GetHwnd() {
		return m_hWnd;
	}
};
