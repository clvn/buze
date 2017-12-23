#pragma once

static const int BUTTONX = 50;
static const int BUTTONY = 30;

class CSkeletonViewInfo : public CViewInfoImpl {
public:
	int show_eventcode;
	buze_main_frame_t* mainframe;
	CSkeletonViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CSkeletonView 
	:public CWindowImpl<CSkeletonView>,
	public CViewImpl
{
public:
	CEdit edit;

	DECLARE_WND_CLASS("SkeletonView")

	BEGIN_MSG_MAP(CSkeletonView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	CSkeletonView(buze_main_frame_t* m);
	~CSkeletonView();
	virtual void OnFinalMessage(HWND /*hWnd*/);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	int GetEditFlags();
};
