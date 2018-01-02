#include "stdafx.h"
#include "PackageView.h"

CHostDllModule _Module;

/***

CBgTabViewCtrl

***/


LRESULT CBgTabViewCtrl::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle bgDC((HDC)wParam);
	RECT rcClient;
	GetClientRect(&rcClient);
	bgDC.FillRect(&rcClient, COLOR_3DFACE);
	return DefWindowProc();
}

//
// View
//

CPackageView::CPackageView(buze_main_frame_t* m) : CViewImpl(m), searchTab(this), installTab(this), packageList(this) {
}

CPackageView::~CPackageView() {
}

void CPackageView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CPackageView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lres = DefWindowProc();
	/*
	splitter.Create(*this, rcDefault, NULL, 0, 0);

	edit.Create(splitter, rcDefault, "", WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL);
	edit.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	edit.SetWindowText("hello world!");


	tabs.Create(splitter, rcDefault, 0, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TCS_TABS, 0, IDC_PACKAGETABS);
	edit2.Create(tabs, rcDefault, "", WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL);
	edit2.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	searchTab.Create(tabs, rcDefault, 0, WS_CHILD);
	tabs.AddTab("Search", searchTab);

	installTab.Create(tabs, rcDefault, 0, WS_CHILD);
	tabs.AddTab("Install", installTab);

	packageList.Create(splitter, rcDefault, 0, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	*/
	packageList.Create(*this, rcDefault, 0, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	buze_document_add_view(document, this);

	buze_application_t* app = buze_main_frame_get_application(mainframe);
	boost::filesystem::path program_directory = boost::filesystem::path(buze_application_map_path(app, ".", buze_path_type_app_path));
	boost::filesystem::path user_directory = boost::filesystem::path(buze_application_map_path(app, ".", buze_path_type_user_path));

	pkgmgr.download_directory = buze_application_map_path(app, "PackageDownload", buze_path_type_user_path);
	pkgmgr.user_directory = user_directory.string();
	pkgmgr.program_directory = program_directory.string();

	// this should be set per program directory; can write to program directory, so form a filename baed on the path??
	
	std::string userInstalledBaseName = "installed-" + package_manager::urlencode(boost::filesystem::canonical(program_directory).string()) + ".json";

	userInstalledJson = buze_application_map_path(app, userInstalledBaseName.c_str(), buze_path_type_user_path);
	
	pkgmgr.read_installed(userInstalledJson);
	Search("");
	return 0;
}

LRESULT CPackageView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);

	if (width == 0 || height == 0) return 0;

	packageList.MoveWindow(0, 0, width, height);

	/*

	int splitterPosition = splitter.GetSplitterPos();

	splitter.MoveWindow(0, 0, width, height);

	if (splitterPosition == -1) {
		splitter.SetSplitterPanes(tabs, packageList);
		splitter.SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
		splitter.SetSplitterPosPct(30);
	}
	*/
	return 0;
}

LRESULT CPackageView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	buze_document_remove_view(document, this);
	return 0;
}

void CPackageView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
}

void FilterPackage(const std::string text, const package& pkg, std::vector<package>& result) {
#if defined(_WIN64)
	if (pkg.platform != package_platform::windows_x64)
		return;
#else
	if (pkg.platform != package_platform::windows_x86)
		return;
#endif
	
	result.push_back(pkg);
}

void CPackageView::Search(const std::string& text) {
	buze_application_t* app = buze_main_frame_get_application(mainframe);

	std::vector<package> packages;

	// Scan *.json in AppData\Roaming\Buze\PackageSources, allows to create packages with package sources:
	std::string packageSourcesPath = buze_application_map_path(app, "PackageSources", buze_path_type_user_path);

	if (boost::filesystem::is_directory(packageSourcesPath)) {
		for (boost::filesystem::directory_iterator i(packageSourcesPath); i != boost::filesystem::directory_iterator(); ++i) {
			const boost::filesystem::path& packageSource(*i);
			if (boost::filesystem::is_directory(packageSource))
				continue;

			if (packageSource.extension() != ".json")
				continue;

			pkgmgr.scan(packageSource.string(), [&text, &packages](package& pkg) { FilterPackage(text, pkg, packages);  });
		}
	}

	// but can include more package indexes: "buzzmachines repo", "robotplanet", "unstable"
	std::string userPackageJson = buze_application_map_path(app, "packages.json", buze_path_type_app_path);

	pkgmgr.scan(userPackageJson, [&text, &packages](package& pkg) { FilterPackage(text, pkg, packages);  });

	packageList.BindPackages(packages);
}

bool CPackageView::IsPackageInstalled(const package& package) {
	bool installed = false;
	for (auto i : pkgmgr.installed) {
		if (i.id == package.id && i.version == package.version && i.platform == package.platform) {
			installed = true;
		}
	}
	return installed;
}

void CPackageView::Install(const package& package) {
	// TODO: list of tasks, in a different thread. output log. progress bar. configurable dl timeouts
	// warn if (different) file exists
	std::stringstream log;
	pkgmgr.install(package, log);
	pkgmgr.write_installed(userInstalledJson);

	Search("");

	std::string logstr = log.str();
	if (!logstr.empty()) {
		MessageBox(logstr.c_str(), "Install Log");
	}
}

void CPackageView::Uninstall(const package& package) {
	pkgmgr.uninstall(package);
	pkgmgr.write_installed(userInstalledJson);

	Search("");
}
