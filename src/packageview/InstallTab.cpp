#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "utils.h"
#include "Keymaps.h"
#include "Configuration.h"
#include "PackageView.h"


/***

CInstallTab

***/

CInstallTab::CInstallTab(CPackageView* view) {
	this->view = view;
}

LRESULT CInstallTab::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DefWindowProc();

	installLabel.Create(m_hWnd, rcDefault, "Review installation tasks:", WS_VISIBLE | WS_CHILD);
	installLabel.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	// list of installation tasks: package name + install or uninstall

	return 0;
}

LRESULT CInstallTab::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD width = LOWORD(lParam);
	WORD height = HIWORD(lParam);

	installLabel.MoveWindow(0, 0, width, 32);
	return 0;
}

LRESULT CInstallTab::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CInstallTab::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CInstallTab::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle bgDC((HDC)wParam);
	RECT rcClient;
	GetClientRect(&rcClient);
	bgDC.FillRect(&rcClient, COLOR_3DFACE);
	return 1;
}

