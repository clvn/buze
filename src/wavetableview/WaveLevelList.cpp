#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "WaveTableView.h"
#include "WaveLevelList.h"

CWaveLevelList::CWaveLevelList(CWaveTableView* v) {
	view = v;
}

CWaveLevelList::~CWaveLevelList(void) {
}

LRESULT CWaveLevelList::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);

	SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	AddColumn("Note", 0);
	AddColumn("Length", 1);
	AddColumn("Rate", 2);
	AddColumn("Loop Begin", 3);
	AddColumn("Loop End", 4);
	AddColumn("Bits", 5);

	SetColumnWidth(0, 50);
	SetColumnWidth(1, 50);
	SetColumnWidth(2, 50);
	SetColumnWidth(3, 50);
	SetColumnWidth(4, 50);

	DragAcceptFiles();

	bHandled=TRUE;
	return lRes;
}

LRESULT CWaveLevelList::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
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

LRESULT CWaveLevelList::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return GetParent().SendMessage(WM_KEYUP, wParam, lParam);
}

LRESULT CWaveLevelList::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 1;
}

LRESULT CWaveLevelList::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
/*	MessageBox("Wave level drop", "Wave level", MB_OK);
	int waveIndex = view->waveTableList.GetSelectedIndex();
	if (waveIndex == -1) return 0;

	wave_info_ex& wave = view->mainFrame->player->wavetable.waves[waveIndex];

	HDROP hDropInfo=(HDROP)wParam;
	UINT i;
	UINT nFiles = ::DragQueryFile(hDropInfo, (UINT) -1, NULL, 0);
	for (i = 0; i < nFiles; i++) {
		TCHAR szFileName[_MAX_PATH];
		::DragQueryFile(hDropInfo, i, szFileName, _MAX_PATH);
		// need a valid browser class to open this file
		CBrowserIterator* browser=CArchiveBrowserBase::createBrowser(szFileName);
		if (!browser) {
			MessageBox("Unknown file extension", szFileName);
			continue;
		}
		if (browser->isLoadable()) {
			//if (view->mainFrame->importWaveEntry(browser, entry))
			//	curWave++;
		}
		browser->close();

	}
	::DragFinish(hDropInfo);
	*/
	return 0;
}

LRESULT CWaveLevelList::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	ClientToScreen(&pt);
	return SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(pt.x, pt.y));
}

LRESULT CWaveLevelList::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

	int waveIndex = view->waveList.GetSelectedIndex();
	if (waveIndex == -1) return 0;

	CMenu menu; 
	menu.CreatePopupMenu();

	int levelIndex;
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if (pt.x == -1 && pt.y == -1) {
		levelIndex = GetSelectedIndex();
		RECT rcItem;
		GetItemRect(levelIndex, &rcItem, LVIR_SELECTBOUNDS);
		pt.x = rcItem.left;
		pt.y = rcItem.top;
		ClientToScreen(&pt);
	} else {
		POINT clientpt = pt;
		ScreenToClient(&clientpt);
		levelIndex = HitTest(clientpt, 0);
	}

	if (levelIndex != -1) {
		SelectItem(levelIndex);

		zzub_wavelevel_t* level = zzub_wave_get_level(view->currentWave, levelIndex);
		if (level == 0) return 0;

		view->currentWavelevel = level;//mainFrame->document->selectWaveLevel(level);
		menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVELEVEL_PROPERTIES, "Properties");
		menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVELEVEL_DELETE, "Delete");
	}

	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVELEVEL_ADD, "Add blank level");

	// send context commands to main frame:
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, GetParent(), 0);

	return 0;
}
