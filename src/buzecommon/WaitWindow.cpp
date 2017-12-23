// WaitWindow.cpp : implementation file
//

#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <buze/HostModule.h>
#include <buze/WtlDllModule.h>
#include "WaitWindow.h"
#include "resource.h"

extern CHostDllModule _Module;

// from MSDN on ::SetFocus
//By using the AttachThreadInput function, a thread can attach its input processing to another thread. 
// This allows a thread to call SetFocus to set the keyboard focus to a window attached to another thread's message queue. 

CWaitWindow::CWaitWindow() {
	hWaitEvent = CreateEvent(0, FALSE, FALSE, "waitev");
}

CWaitWindow::~CWaitWindow() {
	if (m_hWnd)
		DestroyWindow();
	if (hWaitEvent)
		CloseHandle(hWaitEvent);
}

LRESULT CWaitWindow::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	RECT rcClient;
	GetClientRect(&rcClient);
	SetWindowPos(HWND_TOPMOST, width / 2 - rcClient.right / 2, height / 2 - rcClient.bottom / 2, 0, 0, SWP_NOSIZE);
	return 0;
}

LRESULT CWaitWindow::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DestroyWindow();
	return 0;
}

LRESULT CWaitWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	// terminate the waitwindow-thread
	PostQuitMessage(0);
	return 0;
}

LRESULT CWaitWindow::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	ptMoveFrom.x = GET_X_LPARAM(lParam);
	ptMoveFrom.y = GET_Y_LPARAM(lParam);
	SetCapture();
	return 0;
}

LRESULT CWaitWindow::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ReleaseCapture();
	return 0;
}

LRESULT CWaitWindow::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	if (m_hWnd != GetCapture()) {
		return 0;
	}
	POINT ptMoveTo = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	RECT rcWindow;
	GetWindowRect(&rcWindow);

	LONG x = ptMoveTo.x - ptMoveFrom.x;
	LONG y = ptMoveTo.y - ptMoveFrom.y;
	LONG width = rcWindow.right - rcWindow.left;
	LONG height = rcWindow.bottom - rcWindow.top;
	MoveWindow(rcWindow.left + x, rcWindow.top + y, width, height, FALSE);
	return 0;
}

LRESULT CWaitWindow::OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CWaitWindow::OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	ShowWindow(SW_SHOW);
	ModifyStyle(0, WS_VISIBLE);
	SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	return 0;
}

LRESULT CWaitWindow::OnSetText(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	SetDlgItemText(IDC_WAITTEXT, (char*)lParam);
	UpdateWindow();
	// send some event we changed text
	//SetEvent(hWaitEvent);
	return 0;
}

LRESULT CWaitWindow::OnHideWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	SetWindowPos(HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_HIDEWINDOW);
	return 0;
}

DWORD WINAPI WaitWindowProc(LPVOID lpParam) {
	CWaitWindow* self = (CWaitWindow*)lpParam;
	self->Create(GetDesktopWindow());

	//CMessageLoop msgLoop;
	CModuleMessageLoop* msgLoop = _Module.AddMessageLoop();

	// send some event we were initialized
	SetEvent(self->hWaitEvent);

	int result = msgLoop->Run();

	_Module.RemoveMessageLoop();

	SetEvent(self->hWaitEvent);
	return (DWORD)result;
}

bool CWaitWindow::Initialize(DWORD _dwMainThreadID) {
	dwMainThreadID = _dwMainThreadID;
	DWORD dwThread;
	HANDLE hThread = CreateThread(0, 0, WaitWindowProc, this, 0, &dwThread);	
	CloseHandle(hThread);
	WaitForSingleObject(hWaitEvent, INFINITE);
	return true;
}

void CWaitWindow::SetText(const char* text) {
	static char pcText[1024];
	strcpy(pcText, text);
	SendMessage(MYWM_SETTEXT, 0, (LPARAM)pcText);
}

void CWaitWindow::Show() {
	SendMessage(MYWM_SHOWWINDOW);

	SetText("Loading...");
}

void CWaitWindow::Hide(HWND hFocusWnd) {
	SendMessage(MYWM_HIDEWINDOW);

	if (hFocusWnd != 0) {
		::SetForegroundWindow(hFocusWnd);
		::SetFocus(hFocusWnd);
	}
}

void CWaitWindow::Uninitialize() {
	SendMessage(WM_CLOSE);
	// wait until waitthread is finished
	WaitForSingleObject(hWaitEvent, INFINITE);
}
