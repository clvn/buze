#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "utils.h"
#include "Keymaps.h"
#include "Configuration.h"
#include "PackageView.h"

/***

CSearchTab

***/

CSearchTab::CSearchTab(CPackageView* view) {
	this->view = view;
}

LRESULT CSearchTab::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DefWindowProc();

	searchLabel.Create(m_hWnd, rcDefault, "Search for text:", WS_VISIBLE | WS_CHILD);
	searchLabel.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	searchEdit.Create(m_hWnd, rcDefault, "", WS_VISIBLE | WS_CHILD, WS_EX_CLIENTEDGE);
	searchEdit.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	searchButton.Create(*this, rcDefault, "Search", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_NOTIFY, 0, IDC_SEARCHBUTTON);
	searchButton.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	return 0;
}

LRESULT CSearchTab::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD width = LOWORD(lParam);
	WORD height = HIWORD(lParam);

	searchLabel.MoveWindow(0, 0, width, 32);
	searchEdit.MoveWindow(0, 32, width, 32);
	searchButton.MoveWindow(0, 64, width, 32);
	return 0;
}

LRESULT CSearchTab::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CSearchTab::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CSearchTab::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle bgDC((HDC)wParam);
	RECT rcClient;
	GetClientRect(&rcClient);
	bgDC.FillRect(&rcClient, COLOR_3DFACE);
	return 1;
}

LRESULT CSearchTab::OnSearchClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MessageBox("Hello");
	// view->SearchForPackages(); -> updates list view, etc
	return 0;
}
