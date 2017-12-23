#pragma once

static const int BUTTONX = 50;
static const int BUTTONY = 30;

#define IDC_SERVER			1000
#define IDC_PORT			1001
#define IDC_PROJECT			1002
#define IDC_PASSWORD		1003
#define IDC_CONNECT_OPEN	1004
#define IDC_CONNECT_CREATE	1005
#define IDC_DISCONNECT		1006

class CCollaborationViewInfo : public CViewInfoImpl {
public:
	int show_eventcode;
	buze_main_frame_t* mainframe;
	CCollaborationViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CCollaborationView 
	:public CWindowImpl<CCollaborationView>,
	public CViewImpl
{
public:
	CEdit server, port, project, password;
	CStatic serverLabel, portLabel, projectLabel, passwordLabel;
	CStatic status, info;
	CButton connectCreate, connectOpen, disconnect;
	bool isConnected;

	DECLARE_WND_CLASS("CollaborationView")

	BEGIN_MSG_MAP(CCollaborationView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		COMMAND_HANDLER(IDC_CONNECT_CREATE, BN_CLICKED, OnConnectCreate)
		COMMAND_HANDLER(IDC_CONNECT_OPEN, BN_CLICKED, OnConnectOpen)
		COMMAND_HANDLER(IDC_DISCONNECT, BN_CLICKED, OnDisconnect)
	END_MSG_MAP()

	CCollaborationView(buze_main_frame_t* m);
	~CCollaborationView();
	virtual void OnFinalMessage(HWND /*hWnd*/);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	LRESULT OnConnectCreate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnConnectOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDisconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	void UpdateTimer(int count);
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	int GetEditFlags();
};
