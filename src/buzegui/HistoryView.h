#pragma once

#define MYWM_UPDATE WM_USER + 1

class CHistoryViewInfo : public CViewInfoImpl {
public:
	CHistoryViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CHistoryView 
	: public CWindowImpl<CHistoryView>
	, public CViewImpl
	, public CMessageFilter
{
public:
	CListViewCtrl historyList;
	volatile bool dirtyHistory;
	volatile bool quit;
	HANDLE hHistorySignal;
	CRITICAL_SECTION csDirty;

	DECLARE_WND_CLASS("HistoryView")

	BEGIN_MSG_MAP(CHistoryView)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(MYWM_UPDATE, OnUpdateHistory)

		NOTIFY_HANDLER(IDC_HISTORYLIST, NM_DBLCLK, OnDblClkHistory)
	END_MSG_MAP()

	CHistoryView(buze_main_frame_t* m);
	~CHistoryView(void);
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnUpdateHistory(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDblClkHistory(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);

	void BindHistoryInThread();
	void BindHistory();
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
};
