#pragma once

class CHelpViewInfo : public CViewInfoImpl {
public:
	CHelpViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CHelpView 
	: public CWindowImpl<CHelpView>
	, public CViewImpl
	, public CMessageFilter
{
public:
    CFont helpFont;
	CEdit helpText;
	HACCEL hAccel;

	DECLARE_WND_CLASS("HelpView")

	BEGIN_MSG_MAP(CHelpView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		COMMAND_ID_HANDLER(ID_HELP, OnHelp)
	END_MSG_MAP()

    CHelpView(buze_main_frame_t* m);
	~CHelpView();
	virtual void OnFinalMessage(HWND /*hWnd*/);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {}
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
};
