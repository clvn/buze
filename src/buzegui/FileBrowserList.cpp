#include "stdafx.h"
#include "FileBrowserList.h"

LRESULT CFileBrowserList::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lRes = DefWindowProc(uMsg, wParam, lParam);
	
	SetFont((HFONT)GetStockObject( DEFAULT_GUI_FONT ));

	AddColumn("Filename", 0);
	AddColumn("Size", 1);
	SetColumnWidth(0, 150);
	SetColumnWidth(1, 80);
	return lRes;
}

LRESULT CFileBrowserList::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 1;
}

LRESULT CFileBrowserList::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	switch (wParam) {
		case VK_BACK:
		case VK_TAB:
		case VK_SPACE:
		case VK_RETURN:
		case VK_RIGHT:
		case VK_LEFT:
			return GetParent().SendMessage(uMsg, wParam, lParam);
		case VK_UP:
		case VK_DOWN:
		case VK_HOME:
		case VK_END:
		case VK_NEXT:
		case VK_PRIOR:
			if (!DefWindowProc()) return 0;
			break;
	}

	LRESULT lRes = GetParent().SendMessage(uMsg, wParam, lParam);
	return lRes;
}

LRESULT CFileBrowserList::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

LRESULT CFileBrowserList::OnBlur(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}
