#include "stdafx.h"
#include "resource.h"
#include <boost/algorithm/string.hpp>
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "ToolbarWindow.h"
#include "ToolbarBands.h"
#include "Keymaps.h"
#include "Utils.h"
#include "FileReader.h"
#include "directory.h"
#include "FileBrowserView.h"

struct AlphanumericFindSort {
	bool operator()(const CFileInfo& m1, const CFileInfo& m2)
	{
		if (m1.type == CFileInfo::type_dir) {
			if (m2.type == CFileInfo::type_dir) {
				return boost::ilexicographical_compare(m1.fileName, m2.fileName);
			} else {
				return true;				
			}
		} else {
			if (m2.type == CFileInfo::type_dir) {
				return false;
			} else {
				return boost::ilexicographical_compare(m1.fileName, m2.fileName); //m1.fileName.compare(m2.fileName);
			}
		}
	}
};

// 
// Factory
//

CFileBrowserViewInfo::CFileBrowserViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CFileBrowserView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Files";
	tooltip = "Load waveforms from system";
	place = 2; //DockSplitTab::placeDOCKPANE;
	side = 2; //DockSplitTab::dockLEFT;
	serializable = true;
	allowfloat = false;
	defaultview = false;
}

CView* CFileBrowserViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CFileBrowserView* view = new CFileBrowserView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CFileBrowserViewInfo::Attach() {
	buze_document_add_view(document, this);


	//buze_main_frame_register_accelerator(mainframe, "filebrowser", "help", ID_HELP);
	buze_main_frame_register_accelerator(mainframe, "filebrowser", "import", "enter", ID_FILEBROWSER_IMPORT);
	buze_main_frame_register_accelerator(mainframe, "filebrowser", "import_advance", "backspace", ID_FILEBROWSER_IMPORT_ADVANCE);
	buze_main_frame_register_accelerator(mainframe, "filebrowser", "preview", "space ctrl", ID_FILEBROWSER_PREVIEW);
	buze_main_frame_register_accelerator(mainframe, "filebrowser", "preview_advance", "space", ID_FILEBROWSER_PREVIEW_ADVANCE);
	buze_main_frame_register_accelerator(mainframe, "filebrowser", "previous", "left", ID_FILEBROWSER_PREV);
	buze_main_frame_register_accelerator(mainframe, "filebrowser", "next", "right", ID_FILEBROWSER_NEXT);
	buze_main_frame_register_accelerator(mainframe, "filebrowser", "refresh", "r ctrl", ID_FILEBROWSER_REFRESH);

	// global accelerators - these generate global document events caught in OnUpdate
	WORD ID_SHOW_FILES = buze_main_frame_register_accelerator_event(mainframe, "view_files", "f9 shift", buze_event_type_show_filebrowser);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//buze_main_frame_register_accelerator(mainframe, "patternformatview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_FILES, "Files");
}

void CFileBrowserViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CFileBrowserViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CView* view;
	switch (lHint) {
		case buze_event_type_show_filebrowser:
			view = buze_main_frame_get_view(mainframe, "FileBrowserView", 0);
			if (view) {
				buze_main_frame_set_focus_to(mainframe, view);
			} else
				buze_main_frame_open_view(mainframe, "FileBrowserView", "Files", 0, -1, -1);
			break;
	}
}

//
// View
//

CFileBrowserView::CFileBrowserView(buze_main_frame_t* mainFrm)
	:CViewImpl(mainFrm)
{
	currentPath = "";
	historyPosition = 0;
}

CFileBrowserView::~CFileBrowserView(void) {
}

void CFileBrowserView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CFileBrowserView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// http://groups.google.no/group/microsoft.public.vc.atl/browse_thread/thread/afb91a02192bd05b/d67319f5d6b58936?lnk=st&q=CListViewCtrl+additem&rnum=1&hl=no#d67319f5d6b58936
	// this thread suggests calling defwindowproc before adding items
	// additionally, wine requires it :)

	LRESULT lres=DefWindowProc();

	fileList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS, 0, IDC_FILELIST);
	fileList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
	UpdateFiles();

	shortcutDropdown.Create(m_hWnd, rcDefault, "Quicklinks", WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_SHORTCUTDROPDOWN);
	shortcutDropdown.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	BindShortcuts();

	hWndButtonToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_FILEBROWSER, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	// add the toolbar bands as late as possible since they will invoke a WM_SIZE on us
	bool bLock = buze_configuration_get_toolbars_locked(buze_document_get_configuration(document)) != 0;
	insertToolbarBand(shortcutDropdown, "", 100, -1, true, bLock);
	SIZE btbSize; SendMessage(hWndButtonToolBar, TB_GETMAXSIZE, 0, (LPARAM)&btbSize);
	insertToolbarBand(hWndButtonToolBar, "", btbSize.cx, -1, true, bLock);

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);

	buze_document_add_view(document, this);

	CBrowseHistory bh;
	bh.currentPath = "";
	history.push_back(bh);

	return 	0;
}

BOOL CFileBrowserView::PreTranslateMessage(MSG* pMsg) {
	HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "filebrowser");
	assert(hAccel != 0);
	if (GetFocus() == *this || GetFocus() == fileList)
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg))
			return TRUE;
	return FALSE;
}

void CFileBrowserView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	switch (lHint) {
		case buze_event_type_update_settings:
			LockBands(buze_configuration_get_toolbars_locked(buze_document_get_configuration(document)));
			break;
	}
}

LRESULT CFileBrowserView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);

	buze_document_remove_view(document, this);
	return 0;
}

LRESULT CFileBrowserView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HIMAGELIST hImageList = (HIMAGELIST)::SendMessage(hWndButtonToolBar, TB_SETIMAGELIST, 0, (LPARAM)0);
	ImageList_Destroy(hImageList);
	return 0;
}

LRESULT CFileBrowserView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD( lParam);
	WORD cy = HIWORD( lParam);

	int toolbarHeight=getToolbarHeight();
	fileList.MoveWindow(0, getToolbarHeight(), cx, cy-getToolbarHeight());

	return 0;
}

LRESULT CFileBrowserView::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	BOOL b = TRUE;
	switch (wParam) {
		case VK_TAB:
			return 0;
		case VK_LEFT:
			OnFileBrowserPrev(0, 0, 0, b);
			return 0;
		case VK_RIGHT:
			OnFileBrowserNext(0, 0, 0, b);
			return 0;
		/*case 'R':
			if (isCtrlDown()) {
				UpdateFiles();
				return 0;
			}
			break;*/
		//case VK_SPACE: {
		//	handleSelectedItem(false);
		//	selectNextFile();
		//	return 0;
		//}
		//case VK_BACK: // VK_BACK doesn't work here because it's mapped to ID_FILEBROWSER_IMPORT in the resource file
		//case VK_RETURN:
		//	handleSelectedItem(true);
		//	if (fileList.GetSelectedCount() == 1) selectNextFile();
		//	return 0;
	}

	if ((lParam & (1 << 30)) != 0) return 0;	// repeat >1
	if ((lParam & (1 << 24)) != 0) return 0;	// extended key

	int note = keyboard_mapper::map_code_to_note(buze_document_get_octave(document), (int)wParam);
	if (note > 0 && note != 255 && note != 254) note = midi_to_buzz_note(note);
	if (note == -1) return 0;

	std::vector<CFileInfo*> selectedFiles;
	GetSelectedFiles(&selectedFiles);
	if (selectedFiles.size() == 0) return 0;

	CFileInfo* fileInfo = selectedFiles[0];
	std::string uri = buze_document_get_stream_plugin_uri_for_file(document, fileInfo->fullName.c_str());
	if (uri == "") {
		MessageBox("Unknown extension");
		return 0;
	}

	buze_document_play_stream(document, note, 0, 0, uri.c_str(), fileInfo->fullName.c_str());
	
	zzub_plugin_t* streamplayer = buze_document_get_stream_plugin(document);
	if (streamplayer == 0) return 0;
	buze_document_keyjazz_key_down(document, streamplayer, (int)wParam, note);
	return 0;
}

LRESULT CFileBrowserView::OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

	zzub_plugin_t* plugin;
	int note;

	if (!buze_document_keyjazz_key_up(document, (int)wParam, &note, &plugin)) return 0;

	zzub_plugin_t* streamplayer = buze_document_get_stream_plugin(document);
	if (streamplayer == 0) return 0;

	buze_document_play_plugin_note(document, plugin, zzub_note_value_off, note);
	return 0;
}


LRESULT CFileBrowserView::OnFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	fileList.SetFocus();
	return 0;
}

LRESULT CFileBrowserView::OnBlur(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// we assume FileBrowserList forwards us his WM_KILLFOCUS-message
	buze_document_keyjazz_release(document, true);
	zzub_player_reset_keyjazz(player);	// reset midi keyjazz
	return 0;
}

LRESULT CFileBrowserView::OnSelChangeShortcut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int curSel = shortcutDropdown.ctrl().GetCurSel();
	char countstr[1024];
	strcpy(countstr, "");
	if (curSel != LB_ERR) {
		shortcutDropdown.ctrl().GetLBText(curSel, countstr);
	}

	GotoPath(countstr);

	shortcutDropdown.ctrl().SelectString(-1, countstr);
	shortcutDropdown.ctrl().ShowDropDown(FALSE);

	return 0;
}

LRESULT CFileBrowserView::OnLbnDblClkFileList(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	HandleSelectedItem(false);
	return 0;
}


// her står hvordan vi implementerer drag+drop the explorer way med CF_HDROP
//http://www.codeproject.com/shell/explorerdragdrop.asp

LRESULT CFileBrowserView::OnBeginDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pnmh;
	StringWriter sw(false);
	std::vector<CFileInfo*> selectedFiles;
	GetSelectedFiles(&selectedFiles);
	for (size_t i = 0; i < selectedFiles.size(); i++) {
//		string fileName=currentPath + selectedFiles[i]->fileName;
		std::string fileName = selectedFiles[i]->fullName;
		sw.writeBytes(fileName.c_str(), fileName.length()+1);
	}

	size_t bufferSize=sizeof(DROPFILES) + sw.position() + 1;

	FORMATETC fmtetc = {0};
	fmtetc.cfFormat = CF_HDROP;
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	fmtetc.tymed = TYMED_HGLOBAL;

	STGMEDIUM medium = {0};
	medium.tymed = TYMED_HGLOBAL;

	medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE, bufferSize); //for NULL
	DROPFILES* dropFiles = (DROPFILES*)GlobalLock(medium.hGlobal);
	dropFiles->fNC=FALSE;
	dropFiles->fWide=FALSE;
	dropFiles->pFiles=sizeof(DROPFILES);
	dropFiles->pt.x=dropFiles->pt.y=0;
	char* targetPtr=(char*)(dropFiles+1);
	memcpy(targetPtr, sw.getString(), sw.position());
	targetPtr[sw.position()]=0;

	GlobalUnlock(medium.hGlobal);

	CIDropSource* pdsrc=new CIDropSource();
	CIDataObject* pdobj=new CIDataObject(pdsrc);

	pdsrc->AddRef();
	pdobj->AddRef();
	pdobj->SetData(&fmtetc,&medium,TRUE);

	CDragSourceHelper dragSrcHelper;
	dragSrcHelper.InitializeFromWindow(m_hWnd, pnmtv->ptDrag, pdobj);

	DWORD dwEffect;
	::DoDragDrop(pdobj, pdsrc, DROPEFFECT_COPY, &dwEffect);

	return 0;
}

LRESULT CFileBrowserView::OnFileBrowserAddPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	buze_configuration_t* config = buze_document_get_configuration(document);
	buze_configuration_add_sample_path(config, currentPath.c_str());
	BindShortcuts();

	std::stringstream strm;
	strm << "Folder '" << currentPath << "' is now accessible from the shortcut dropdown list";
	MessageBox(strm.str().c_str(), "Folder Shortcut added");
	return 0;
}

LRESULT CFileBrowserView::OnFileBrowserRemovePath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int cur = shortcutDropdown.ctrl().GetCurSel();
	if (cur < 0) return 0;

	buze_configuration_t* config = buze_document_get_configuration(document);
	buze_configuration_remove_sample_path(config, cur);
	BindShortcuts();
	return 0;
}

LRESULT CFileBrowserView::OnFileBrowserRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	UpdateFiles();
	return 0;
}

LRESULT CFileBrowserView::OnFileBrowserNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	std::vector<CFileInfo*> selectedFiles;
	GetSelectedFiles(&selectedFiles);
	std::string selectedFile;
	if (selectedFiles.size() > 0)
		selectedFile = selectedFiles[0]->fileName;

	if (historyPosition < (int)history.size() - 1) {
		history[historyPosition].selectedFile = selectedFile;
		historyPosition++;
		currentPath = history[historyPosition].currentPath;
		UpdateFiles();
		SelectFile(history[historyPosition].selectedFile);
	}
	return 0;
}

LRESULT CFileBrowserView::OnFileBrowserPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	std::vector<CFileInfo*> selectedFiles;
	GetSelectedFiles(&selectedFiles);
	std::string selectedFile;
	if (selectedFiles.size() > 0)
		selectedFile = selectedFiles[0]->fileName;

	if (historyPosition > 0) {
		history[historyPosition].selectedFile = selectedFile;
		historyPosition--;
		currentPath = history[historyPosition].currentPath;
		UpdateFiles();
		SelectFile(history[historyPosition].selectedFile);
	}
	return 0;
}

LRESULT CFileBrowserView::OnFileBrowserImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (HandleSelectedItem(true)) {
		buze_document_notify_views(document, this, buze_event_type_show_wavetable_view, 0);
	}
	return 0;
}

LRESULT CFileBrowserView::OnFileBrowserImportAdvance(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (HandleSelectedItem(true)) {
		SelectNextFile();
		buze_document_notify_views(document, this, buze_event_type_show_wavetable_view, 0);
	}
	return 0;
}

LRESULT CFileBrowserView::OnFileBrowserPreview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	// play sample and goto next sample
	HandleSelectedItem(false);
	return 0;
}

LRESULT CFileBrowserView::OnFileBrowserPreviewAdvance(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	// play sample and goto next sample
	HandleSelectedItem(false);
	SelectNextFile();
	return 0;
}

void CFileBrowserView::BindShortcuts() {
	while (shortcutDropdown.ctrl().GetCount() > 0)
		shortcutDropdown.ctrl().DeleteString(0);

	wavetableFolders.clear();
	buze_configuration_t* config = buze_document_get_configuration(document);
	int numWaveDirs = buze_configuration_get_sample_path_count(config);
	for (int i = 0; i < numWaveDirs; i++) {
		std::string wavePath;
		wavetableFolders.push_back(buze_configuration_get_sample_path(config, i));
	}


	for (size_t i = 0; i < wavetableFolders.size(); i++) {
		shortcutDropdown.ctrl().AddString(wavetableFolders[i].c_str());
	}
}

void CFileBrowserView::SelectFile(std::string fileName) {
	int count = fileList.GetItemCount();
	char pc[1024];
	for (int i = 0; i < count; i++) {
		fileList.GetItemText(i, 0, pc, 1024);
		LVITEM item;
		item.mask = LVIF_STATE;
		item.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
		if (fileName == pc) {
			item.state = LVIS_SELECTED|LVIS_FOCUSED ;
			fileList.EnsureVisible(i, FALSE);
		} else
			item.state = 0;

		fileList.SetItemState(i, &item);

	}
}

void CFileBrowserView::SelectNextFile() {

	LVITEM item;
	item.mask = LVIF_STATE;
	item.stateMask = LVIS_FOCUSED|LVIS_SELECTED;
	item.state = 0;

	int count = fileList.GetItemCount();
	int select = -1;
	bool next = false;

	for (int i=0; i<count; i++) {
		
		UINT state = fileList.GetItemState(i, LVIS_FOCUSED|LVIS_SELECTED);
		if (!next && !state) continue;

		if (!next && state) {
			// clear this selection
			fileList.SetItemState(i, &item);
			select=i+1;
			break;
		}
	}
	if (select == -1) return  ;

	if (select >= count) select = 0;

	item.state = LVIS_FOCUSED|LVIS_SELECTED;
	if (!fileList.SetItemState(select, &item)) {
		// at last position, move to top
		fileList.SetItemState(select, &item);
	}
	fileList.EnsureVisible(select, FALSE);
}

// updatefiles checks currentPath and gets drives, filesystem or module-iterates
void CFileBrowserView::UpdateFiles() {
	// take files from current folder and etc
	fileList.DeleteAllItems();

	files.clear();

	root_directory root;
	std::vector<directory_entry> dirfiles;
	root.factories.push_back(new zzub_directory_factory<false>(player));
	root.factories.push_back(new zzub_directory_factory<true>(player));
	if (currentPath.empty()) {
		root.get_files(dirfiles);
	} else {
		directory* dir = root.get_directory(currentPath.c_str());
		if (dir) {
			dir->get_files(dirfiles);
			delete dir;
		}

		directory_entry parentd;
		parentd.name = "..";
		parentd.fullname = "..";
		parentd.type = directory_entry::type_directory;
		dirfiles.push_back(parentd);
	}

	for (std::vector<directory_entry>::iterator i = dirfiles.begin(); i != dirfiles.end(); ++i) {
		CFileInfo fi;
		fi.fileName = i->name;
		fi.fullName = i->fullname;
		if (i->type == directory_entry::type_directory) {
			fi.type = CFileInfo::type_dir;
		} else 
		if (i->type == directory_entry::type_container) {
			fi.type = CFileInfo::type_container;
		} else {
			fi.type = CFileInfo::type_file;
		}
		fi.size = i->size;
		files.push_back(fi);
	}

	sort(files.begin(), files.end(), AlphanumericFindSort());

	int index=0;
	for (std::vector<CFileInfo>::iterator i = files.begin(); i != files.end(); ++i) {
		fileList.InsertItem(index, i->fileName.c_str());
		if (i->type == CFileInfo::type_dir) {
			fileList.SetItem(index, 1, LVIF_TEXT, "(dir)", -1, 0, 0, 0);
			//strcat(i->cFileName, "\\");	// fix directory
		} else {
			char pcSize[32];
			sprintf(pcSize, "%i", i->size);
			fileList.SetItem(index, 1, LVIF_TEXT, pcSize, -1, 0, 0, 0);
		}
		fileList.SetItemData(index, (DWORD_PTR)(CFileInfo*)&*i);

		index++;
	}
}

// TODO: if there is exactly one item in the selection, try to open as a directory
// otherwise, enumerate the files and attempt to load into successive waves by deriving next-wave-slot 
// from current_wave until all files are loaded (or end-of-wavetable was reached)
bool CFileBrowserView::HandleSelectedItem(bool pleaseLoad) {
	std::vector<CFileInfo*> selectedFiles;
	GetSelectedFiles(&selectedFiles);
	if (selectedFiles.size() == 0) return false;
	CFileInfo* selectedFile = selectedFiles[0];

	std::string topath = currentPath;
	if (selectedFile->fileName == "..") {
		std::string::size_type ls = topath.find_last_of("/\\", topath.length()-2);
		if (ls != std::string::npos) {
			topath = topath.substr(0, ls);
		} else {
			topath = "";
		}
	} else {
		topath = selectedFile->fullName;
	}

	if (selectedFile->type == CFileInfo::type_container || selectedFile->type == CFileInfo::type_dir) {
		GotoPath(topath);
		return false; // if changing directory, return false to prevent any file-actions to take place afterwards
	}

	if (selectedFile->type == CFileInfo::type_file) {

		if (pleaseLoad) {
			zzub_wave_t* wave = buze_document_get_current_wave(document);
			if (wave == 0) return false;

			if (buze_document_import_wave(document, selectedFile->fullName.c_str(), wave) == -1) {
				MessageBox("Cannot load sample(s)", "Error");
				return false;
			}

			zzub_player_history_commit(player, 0, 0, "Load sample");
			return true;

		} else {
			return PlayPath(topath);
		}
	}
	return false; // ??? :(
}

void CFileBrowserView::GetSelectedFiles(std::vector<CFileInfo*>* result) {
	int count = fileList.GetSelectedCount();
	result->resize(count);
	int nItem = -1;
	for (int i = 0; i < count; i++) {
		nItem = fileList.GetNextItem(nItem, LVNI_SELECTED);
		CFileInfo* fi = (CFileInfo*)fileList.GetItemData(nItem);
		(*result)[i] = fi;
	}
}

// like the old gotoPath with verb==play
bool CFileBrowserView::PlayPath(std::string path) {
	std::string uri = buze_document_get_stream_plugin_uri_for_file(document, path.c_str());
	if (uri == "") {
		MessageBox("Unknown extension");
		return 0;
	}

	buze_document_play_stream(document, zzub_note_value_c4, 0, 0, uri.c_str(), path.c_str());

	return true;
}

// like the old gotoPath with verb==open
bool CFileBrowserView::GotoPath(std::string path) {
	currentPath = path;

	UpdateFiles();

	std::string shortFolderName;
	std::string::size_type ls = path.find_last_of("\\/", path.length()-2);
	if (ls != std::string::npos) {
		shortFolderName = path.substr(ls + 1);
	} else
		shortFolderName = path;

	CBrowseHistory& prevhist = history[historyPosition];
	prevhist.selectedFile = shortFolderName;

	historyPosition++;
	history.erase(history.begin() + historyPosition, history.end());
	CBrowseHistory bh;
	bh.currentPath = currentPath;
	history.push_back(bh);

	SelectFile(files[0].fileName);
	return true;
}
