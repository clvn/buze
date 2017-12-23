#include "stdafx.h"
#include "CollaborationView.h"

CHostDllModule _Module;

//
// View
//

CCollaborationView::CCollaborationView(buze_main_frame_t* m) : CViewImpl(m) {
	isConnected = false;
}

CCollaborationView::~CCollaborationView() {
}

void CCollaborationView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CCollaborationView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lres = DefWindowProc();

	// TODO: 
	//   - status static text: "Disconnected", "Connected", "Disconnected: Error"
	//   - info static text: connected users

	serverLabel.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD);
	serverLabel.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	serverLabel.SetWindowText("Server:");

	server.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD, WS_EX_CLIENTEDGE, IDC_SERVER);
	server.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	portLabel.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD);
	portLabel.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	portLabel.SetWindowText("Port:");

	port.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD|ES_NUMBER, WS_EX_CLIENTEDGE, IDC_PORT);
	port.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	port.SetWindowTextA("8834");

	projectLabel.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD);
	projectLabel.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	projectLabel.SetWindowText("Project:");

	project.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD, WS_EX_CLIENTEDGE, IDC_PROJECT);
	project.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	
	passwordLabel.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD);
	passwordLabel.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	passwordLabel.SetWindowText("Password:");

	password.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD, WS_EX_CLIENTEDGE, IDC_PASSWORD);
	password.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	
	connectCreate.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD, 0, IDC_CONNECT_CREATE );
	connectCreate.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	connectCreate.SetWindowTextA("Connect and create");

	connectOpen.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD, 0, IDC_CONNECT_OPEN);
	connectOpen.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	connectOpen.SetWindowTextA("Connect and open");

	disconnect.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD|WS_DISABLED, 0, IDC_DISCONNECT );
	disconnect.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	disconnect.SetWindowTextA("Disconnect");

	status.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD);
	status.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	status.SetWindowText("Status: Disconnected");

	buze_main_frame_add_timer_handler(mainframe, this);
	buze_document_add_view(document, this);

	return 0;
}

LRESULT CCollaborationView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	buze_document_remove_view(document, this);
	buze_main_frame_remove_timer_handler(mainframe, this);
	return 0;
}

LRESULT CCollaborationView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);
	serverLabel.MoveWindow(0, 0, 200, 20);
	server.MoveWindow(0, 20, 200, 20);
	
	portLabel.MoveWindow(210, 0, 200, 20);
	port.MoveWindow(210, 20, 80, 20);

	projectLabel.MoveWindow(0, 52, 200, 20);
	project.MoveWindow(0, 72, 200, 20);
	
	passwordLabel.MoveWindow(210, 52, 200, 20);
	password.MoveWindow(210, 72, 200, 20);

	connectCreate.MoveWindow(0, 104, 200, 28);
	connectOpen.MoveWindow(210, 104, 200, 28);
	disconnect.MoveWindow(0, 134, 200, 28);

	status.MoveWindow(210, 134, 200, 20);
	return 0;
}

LRESULT CCollaborationView::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle dc((HDC)wParam);
	COLORREF fc = GetSysColor(COLOR_BTNFACE);
	RECT rc;
	GetClientRect(&rc);
	dc.FillSolidRect(&rc, fc);
	return 1;
}

void CCollaborationView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	switch (lHint) {
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_new_document:
		case zzub_event_type_update_song:
			break;
	}
}

void CCollaborationView::UpdateTimer(int count) {
	if ((count % 20) != 0) return; /// 200ms

	if (isConnected && zzub_player_is_remote_connected(player) == 0) {
		isConnected = false;
		status.SetWindowTextA("Status: Disconnected");
		connectCreate.EnableWindow(1);
		connectOpen.EnableWindow(1);
		disconnect.EnableWindow(0);
	} else if (!isConnected && zzub_player_is_remote_connected(player) != 0) {
		isConnected = true;
		connectCreate.EnableWindow(0);
		connectOpen.EnableWindow(0);
		disconnect.EnableWindow(1);
	}
}

LRESULT CCollaborationView::OnConnectCreate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_t* player = buze_main_frame_get_player(mainframe);
	char pcServer[1024];
	server.GetWindowTextA(pcServer, 1024);
	char pcPort[32];
	port.GetWindowTextA(pcPort, 32);

	if (strlen(pcPort) == 0) {
		// NOTE: blank port -> crash in armstrong client resolver
		status.SetWindowTextA("Status: Disconnected. Cannot connect");
		return 0;
	}

	int result = zzub_player_remote_connect(player, pcServer, pcPort);
	if (result != 0) {
		status.SetWindowTextA("Status: Disconnected. Cannot connect");
		return 0;
	}

	char pcProject[1024];
	project.GetWindowTextA(pcProject, 1024);
	char pcPassword[1024];
	password.GetWindowTextA(pcPassword, 1024);
	result = zzub_player_remote_create(player, pcProject, pcPassword);
	if (result != 0) {
		zzub_player_remote_disconnect(player);
		status.SetWindowTextA("Status: Disconnected. Cannot create");
		return 0;
	}

	status.SetWindowTextA("Status: Connected");
	//isConnected = true;
	//disable connecton buttons, enable disconnect button
	return 0;
}

LRESULT CCollaborationView::OnConnectOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_t* player = buze_main_frame_get_player(mainframe);
	char pcServer[1024];
	server.GetWindowTextA(pcServer, 1024);
	char pcPort[32];
	port.GetWindowTextA(pcPort, 32);

	if (strlen(pcPort) == 0) {
		// NOTE: blank port -> crash in armstrong client resolver
		status.SetWindowTextA("Status: Disconnected. Cannot connect");
		return 0;
	}

	int result = zzub_player_remote_connect(player, pcServer, pcPort);
	if (result != 0) {
		status.SetWindowTextA("Status: Disconnected. Cannot connect");
		return 0;
	}

	char pcProject[1024];
	project.GetWindowTextA(pcProject, 1024);
	char pcPassword[1024];
	password.GetWindowTextA(pcPassword, 1024);
	result = zzub_player_remote_open(player, pcProject, pcPassword);
	if (result != 0) {
		status.SetWindowTextA("Status: Disconnected. Cannot open");
		zzub_player_remote_disconnect(player);
		return 0;
	}

	status.SetWindowTextA("Status: Connected");
	//isConnected = true;
	//disable connecton buttons, enable disconnect button
	return 0;
}

LRESULT CCollaborationView::OnDisconnect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_remote_disconnect(player);
	return 0;
}
