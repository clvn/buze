#pragma once
#include "package_manager.h"
#include "SearchTab.h"
#include "InstallTab.h"
#include "PackageList.h"

static const int BUTTONX = 50;
static const int BUTTONY = 30;

class CPackageViewInfo : public CViewInfoImpl {
public:
	int show_eventcode;
	buze_main_frame_t* mainframe;
	CPackageViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

// Apparently, having a CTabCtrl inside a splitter causes drawing problems with 
// XP themes where the tab header background isnt cleared properly.
// We create a CBgTabViewVtrl class that derives from our previous tab control 
// and implement WM_ERASEBKGND to get around the problem:

class CBgTabViewCtrl : public CWTLTabViewCtrlT< CBgTabViewCtrl> {
public:
	BEGIN_MSG_MAP(CBgTabViewCtrl)
		MESSAGE_HANDLER(WM_KEYDOWN, OnForwardMessage)
		MESSAGE_HANDLER(WM_KEYUP, OnForwardMessage)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground);
	CHAIN_MSG_MAP(CWTLTabViewCtrlT< CBgTabViewCtrl>)
	END_MSG_MAP()

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return GetParent().SendMessage(uMsg, wParam, lParam);
	}

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return 1;
	}
};

// resource.h
#define IDC_PACKAGETABS 1000
// #define IDC_SEARCHBUTTON 1001

class CPackageView 
	:public CWindowImpl<CPackageView>,
	public CViewImpl
{
public:
	std::string userInstalledJson;
	package_manager pkgmgr;
	CEdit edit;
	CEdit edit2;
	CSplitterWindow splitter;
	CBgTabViewCtrl tabs;
	CSearchTab searchTab;
	CInstallTab installTab;
	CPackageList packageList;

	DECLARE_WND_CLASS("PackageView")

	BEGIN_MSG_MAP(CPackageView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		// From CWTLTabViewCtrl: 
		// Parent windows must have REFLECT_NOTIFICATIONS() in the message map
		// to pass along the TCN_SELCHANGE message to 
		REFLECT_NOTIFICATIONS_ID_FILTERED(IDC_PACKAGETABS)
	END_MSG_MAP()

	CPackageView(buze_main_frame_t* m);
	~CPackageView();
	virtual void OnFinalMessage(HWND /*hWnd*/);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	int GetEditFlags();

	bool IsPackageInstalled(const package& package);
	void Search(const std::string& text);
	void Install(const package& package);
	void Uninstall(const package& package);
};
