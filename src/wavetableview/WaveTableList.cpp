#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "WaveTableView.h"
#include "WaveTableList.h"
#include "DragDropImpl.h"

using std::string;

/***

	CWaveTableList

***/

CWaveTableList::CWaveTableList(CWaveTableView* v) {
	view = v;
}

CWaveTableList::~CWaveTableList(void) {
}

LRESULT CWaveTableList::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	int curWave = this->HitTest(pt, 0);
	if (curWave == -1) return 0;

	SetRedraw(FALSE);
	SelectItem(curWave);
	
	buze_application_t *a = buze_main_frame_get_application(view->mainframe);
	buze_application_show_wait_window(a);
	buze_application_set_wait_text(a, "Loading sample..");	

	HDROP hDropInfo = (HDROP)wParam;
	UINT i;
	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT) -1, NULL, 0);
	int wavecount = 0;
	for (i = 0; i < nFiles; i++) {
		TCHAR szFileName[_MAX_PATH];
		::DragQueryFile(hDropInfo, i, szFileName, _MAX_PATH);

		zzub_wave_t* wave = zzub_player_get_wave(view->player, curWave + wavecount);
		if (wave && buze_document_import_wave(view->document, szFileName, wave) != -1) 
			wavecount++;

	}  // end for
	::DragFinish(hDropInfo);

	SetRedraw(TRUE);
	buze_application_hide_wait_window(a, view);

	if (wavecount == 0)
		MessageBox("Cannot load sample(s)", "Error");

	zzub_player_history_commit(view->player, 0, 0, "Load Sample(s)");
	return 0;
}

LRESULT CWaveTableList::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);

	SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	DragAcceptFiles();

	return lRes;
}

LRESULT CWaveTableList::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	switch (wParam) {
		case VK_UP:
		case VK_DOWN:
		case VK_HOME:
		case VK_END:
		case VK_NEXT:
		case VK_PRIOR:
			if (!DefWindowProc()) return 0;
			break;
	}
	return GetParent().SendMessage(WM_KEYDOWN, wParam, lParam);
}

LRESULT CWaveTableList::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return GetParent().SendMessage(WM_KEYUP, wParam, lParam);
}

LRESULT CWaveTableList::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 1;
}

LRESULT CWaveTableList::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	ClientToScreen(&pt);
	return SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(pt.x, pt.y));
}

LRESULT CWaveTableList::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	int waveIndex;
	
	if (pt.x == -1 && pt.y == -1) {
		waveIndex = GetSelectedIndex();
		RECT rcItem;
		GetItemRect(waveIndex, &rcItem, LVIR_SELECTBOUNDS);
		pt.x = rcItem.left;
		pt.y = rcItem.top;
		ClientToScreen(&pt);
	} else {
		POINT clientpt = pt;
		ScreenToClient(&clientpt);
		waveIndex = HitTest(clientpt, 0);
	}

	if (waveIndex == -1) return 0;

	view->currentWave = zzub_player_get_wave(buze_main_frame_get_player(view->mainframe), waveIndex);

	SelectItem(waveIndex);

	CMenu scripts;
	CreateScriptMenu(scripts);

	CMenu menu; 
	menu.CreatePopupMenu();

	//view->mainframe->document->selectWave(wave);
	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_VIEW_PROPERTIES, "Properties");
	menu.AppendMenu(MF_SEPARATOR);
	menu.InsertMenu( -1, MF_BYPOSITION|MF_POPUP, (UINT_PTR)scripts.m_hMenu, "Run Script");
	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVE_XEDIT, "Export And Open With External Editor");
	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVE_IEDIT, "Re-Import");

	// send context commands to the view:
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, GetParent(), 0);

	return 0;
}

LRESULT CWaveTableList::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	RECT rcClient;
	GetClientRect(&rcClient);
	SetColumnWidth(0, rcClient.right - rcClient.left - 2);
	return 0;
}

LRESULT CWaveTableList::OnBlur(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

void CWaveTableList::LoadScripts(){
	wscripts.clear();
	std::string search_path( "Gear\\Scripts\\WaveTable\\" ); // *FIXME* where do I get (rootpath + path) ? 
	search_path += "*.*";

	WIN32_FIND_DATA fd;
	HANDLE hFind=::FindFirstFile(search_path.c_str(), &fd);
	while(hFind != INVALID_HANDLE_VALUE) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// ? *FIXME* recursive loading?
		} else {
			char* ext = strrchr(fd.cFileName, '.');
			if (ext != 0 ){
				if( stricmp(ext, ".js")   == 0 ||
					stricmp(ext, ".bat")  == 0 ||
					stricmp(ext, ".hta")  == 0 ||
					stricmp(ext, ".py")   == 0 ||
					stricmp(ext, ".armz") == 0 
					) 
				wscripts.push_back( string(fd.cFileName) );
			}
		}
		if (!::FindNextFile(hFind, &fd)) break;
	}
	::FindClose(hFind);
	printf("loaded wscripts = %i\n", wscripts.size() );
}

void CWaveTableList::CreateScriptMenu(CMenu& scripts) {
	LoadScripts();
	scripts.DestroyMenu();
	scripts.CreatePopupMenu();
	for( int i = 0; i < wscripts.size(); i++ ){ // strip extensions
		int max = wscripts[i].rfind(".");
		scripts.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVE_SCRIPTCOMMANDS+i, wscripts[i].substr(0,max).c_str() );
	}
}
