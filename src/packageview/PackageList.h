#pragma once

#define ID_INSTALL_COMMAND 33000
#define ID_UNINSTALL_COMMAND 33001
#define ID_REINSTALL_COMMAND 33002

class CPackageView;

class CPackageList : public CWindowImpl<CPackageList> {
	CPackageView* view;
public:
	CPackageList(CPackageView* view);

	std::vector<package> packages;
	CListViewCtrl packageList;

	DECLARE_WND_CLASS("PackageList")

	BEGIN_MSG_MAP(CPackageList)
		COMMAND_ID_HANDLER(ID_INSTALL_COMMAND, OnInstall)
		COMMAND_ID_HANDLER(ID_UNINSTALL_COMMAND, OnUninstall)
		COMMAND_ID_HANDLER(ID_REINSTALL_COMMAND, OnReinstall)

		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInstall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUninstall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnReinstall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void BindPackages(const std::vector<package>& groups);
};
