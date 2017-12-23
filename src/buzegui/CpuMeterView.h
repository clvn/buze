#pragma once

class CCpuMeterViewInfo : public CViewInfoImpl {
public:
	CCpuMeterViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CCpuMeterView 
	: public CWindowImpl<CCpuMeterView> 
	, public CViewImpl
{
public:
	CListViewCtrl pluginList;
	CStatic details;

	DECLARE_WND_CLASS("CpuMeterView")

	BEGIN_MSG_MAP(CCpuMeterView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	CCpuMeterView(buze_main_frame_t* m);
	~CCpuMeterView(void);
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	void UpdateTimer(int count);
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
};
