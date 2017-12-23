#pragma once

#include "DragDropImpl.h"
#include "FileBrowserList.h"

const int IDC_SHORTCUTDROPDOWN = 1023;

class CFileBrowserViewInfo : public CViewInfoImpl {
public:
	CFileBrowserViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CFileInfo {
public:
	enum {
		type_dir,
		type_file,
		type_container
	};

	std::string fileName;
	int type;
	LONG size;
	std::string fullName;
	//std::string verb;
};

class CBrowseHistory {
public:
	std::string currentPath;
	std::string selectedFile;
};

class CFileBrowserView  
	: public CToolbarWindow<CFileBrowserView>
	, public CViewImpl
	, public CMessageFilter
{
	CFileBrowserList fileList;
	std::string currentPath;
	std::vector<CFileInfo> files;
	std::vector<std::string> lastVisitedFolders;
	std::vector<std::string> wavetableFolders;
	std::vector<CBrowseHistory> history;
	int historyPosition;
	CComboListBand shortcutDropdown;
	HWND hWndButtonToolBar;	// the button toolbar

	void BindShortcuts();
public:
	DECLARE_WND_CLASS("FileBrowserView")

	BEGIN_MSG_MAP(CFileBrowserView)
		CHAIN_MSG_MAP(CToolbarWindow<CFileBrowserView>)

		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnBlur)

		COMMAND_ID_HANDLER(ID_FILEBROWSER_ADD_PATH, OnFileBrowserAddPath)
		COMMAND_ID_HANDLER(ID_FILEBROWSER_REMOVE_PATH, OnFileBrowserRemovePath)
		COMMAND_ID_HANDLER(ID_FILEBROWSER_REFRESH, OnFileBrowserRefresh)
		COMMAND_ID_HANDLER(ID_FILEBROWSER_NEXT, OnFileBrowserNext)
		COMMAND_ID_HANDLER(ID_FILEBROWSER_PREV, OnFileBrowserPrev)
		COMMAND_ID_HANDLER(ID_FILEBROWSER_IMPORT, OnFileBrowserImport)
		COMMAND_ID_HANDLER(ID_FILEBROWSER_IMPORT_ADVANCE, OnFileBrowserImportAdvance)
		COMMAND_ID_HANDLER(ID_FILEBROWSER_PREVIEW, OnFileBrowserPreview)
		COMMAND_ID_HANDLER(ID_FILEBROWSER_PREVIEW_ADVANCE, OnFileBrowserPreviewAdvance)
		COMMAND_HANDLER(IDC_SHORTCUTDROPDOWN, CBN_SELCHANGE, OnSelChangeShortcut);

		NOTIFY_HANDLER(IDC_FILELIST, NM_DBLCLK, OnLbnDblClkFileList)
		NOTIFY_CODE_HANDLER(LVN_BEGINDRAG, OnBeginDrag);
	END_MSG_MAP()

	CFileBrowserView(buze_main_frame_t* m);
	~CFileBrowserView(void);

	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnBlur(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnLbnDblClkFileList(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnBeginDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnFileBrowserAddPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileBrowserRemovePath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileBrowserRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileBrowserNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileBrowserPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileBrowserImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileBrowserImportAdvance(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileBrowserPreview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileBrowserPreviewAdvance(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnSelChangeShortcut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void GetSelectedFiles(std::vector<CFileInfo*>* result);
	bool PlayPath(std::string path);
	bool GotoPath(std::string path);
	bool HandleSelectedItem(bool pleaseLoad);		// = double click, enter or space
	void UpdateFiles();
	void SelectFile(std::string fileName);
	void SelectNextFile();
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);

	virtual HWND GetHwnd() {
		return m_hWnd;
	}

	virtual void LockToolbars(bool state) {
		LockBands(state);
	}

};
