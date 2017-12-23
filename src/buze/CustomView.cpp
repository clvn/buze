#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewFrame.h>
#include "BuzeConfiguration.h"
#include "MainFrm.h"
#include "CustomView.h"

using std::cout;
using std::endl;

CCustomView::CCustomView(CMainFrame* mf) {
	mainFrame = mf;
	hViewWnd = 0;
	fnPreTranslateMessage = 0;
}

CCustomView::~CCustomView() {
	if (m_hWnd)
		DestroyWindow();
}

BOOL CCustomView::PreTranslateMessage(MSG* pMsg) {
	if (pMsg->hwnd == m_hWnd || IsChild(pMsg->hwnd)) {
		if (fnPreTranslateMessage)
			return fnPreTranslateMessage(pMsg);
	}
	return FALSE;
}

LRESULT CCustomView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	//mainFrame->viewStackInsert(m_hWnd, false);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	return 0;
}

LRESULT CCustomView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	SendMessage(hViewWnd, WM_CLOSE, 0, 0);

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);

	hViewWnd = 0;
	mainFrame->ViewStackRemove(m_hWnd);
	return 0;
}

LRESULT CCustomView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (!hViewWnd) return 0;
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);

	::MoveWindow(hViewWnd, 0, 0, width, height, TRUE);
	return 0;
}

LRESULT CCustomView::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (!hViewWnd) return 0;
	::SetFocus(hViewWnd);
	return 0;
}
