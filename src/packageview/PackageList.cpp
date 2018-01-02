#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "utils.h"
#include "Keymaps.h"
#include "Configuration.h"
#include "PackageView.h"


/***

CPackageList

***/

CPackageList::CPackageList(CPackageView* view) {
	this->view = view;
}

LRESULT CPackageList::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DefWindowProc();

	packageList.Create(*this, rcDefault, 0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | LVS_SINGLESEL | LVS_REPORT | LVS_SHOWSELALWAYS, 0);
	packageList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	packageList.AddColumn("Installed", 0);
	packageList.AddColumn("Package", 1);
	packageList.AddColumn("Version", 2);

	packageList.SetColumnWidth(0, 80);
	packageList.SetColumnWidth(1, 300);
	packageList.SetColumnWidth(2, 100);

	return 0;
}

LRESULT CPackageList::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD width = LOWORD(lParam);
	WORD height = HIWORD(lParam);

	packageList.MoveWindow(0, 0, width, height);
	return 0;
}

LRESULT CPackageList::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CPackageList::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CPackageList::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle bgDC((HDC)wParam);
	RECT rcClient;
	GetClientRect(&rcClient);
	bgDC.FillRect(&rcClient, COLOR_3DFACE);
	return 1;
}

LRESULT CPackageList::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if (pt.x == -1 && pt.y == -1) {
		// packageList.GetItemRect(i,
		pt.x = pt.y = 0;
	}

	int i = packageList.GetSelectedIndex();
	if (i == -1)
		return 0;

	package& pkg = packages[i];

	bool installed = view->IsPackageInstalled(pkg);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.InsertMenu(-1, (installed ? MF_GRAYED : 0) | MF_BYPOSITION | MF_STRING, (UINT_PTR)ID_INSTALL_COMMAND, "Install");
	menu.InsertMenu(-1, (!installed ? MF_GRAYED : 0) | MF_BYPOSITION | MF_STRING, (UINT_PTR)ID_UNINSTALL_COMMAND, "Uninstall");
	menu.InsertMenu(-1, (!installed ? MF_GRAYED : 0) | MF_BYPOSITION | MF_STRING, (UINT_PTR)ID_REINSTALL_COMMAND, "Reinstall latest");

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);
	return 1;
}

LRESULT CPackageList::OnInstall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = packageList.GetSelectedIndex();
	if (i == -1)
		return 0;

	view->Install(packages[i]);
	return 0;
}

LRESULT CPackageList::OnUninstall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = packageList.GetSelectedIndex();
	if (i == -1)
		return 0;

	view->Uninstall(packages[i]);
	return 0;
}

LRESULT CPackageList::OnReinstall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int i = packageList.GetSelectedIndex();
	if (i == -1)
		return 0;

	view->Uninstall(packages[i]);
	view->Install(packages[i]);
	return 0;
}

void CPackageList::BindPackages(const std::vector<package>& groups) {
	packageList.DeleteAllItems();

	int index = 0;
	for (std::vector<package>::const_iterator i = groups.begin(); i != groups.end(); ++i) {
		const package& group = *i;
		bool installed = view->IsPackageInstalled(group);
		packageList.InsertItem(index, installed ? "Yes" : "No");
		packageList.SetItem(index, 1, LVIF_TEXT, group.id.c_str(), -1, 0, 0, 0);
		packageList.SetItem(index, 2, LVIF_TEXT, group.version.c_str(), -1, 0, 0, 0);
		index++;
	}

	packages = groups;
}
