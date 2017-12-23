#include "stdafx.h"
#include "ModelessMenu.h"

// ---------------------------------------------------------------------------------------------------------------
// MODELESS MENU
// ---------------------------------------------------------------------------------------------------------------

CModelessMenu::CModelessMenu() {
	m_pt.x = m_pt.y = 0;
	m_selectedIndex = -1;
	m_itemHeight = 18;
	SetRect(&m_rcMenu, 0, 0, 0, 0);
}

void CModelessMenu::SetMenu(HWND parent, HMENU menu) {
	m_parent = parent;
	m_menu = menu;
	UpdateFromMenu();
}

void CModelessMenu::SetPaintWindow(HWND paintParent) {
	m_paintParent = paintParent;
}

void CModelessMenu::UpdateFromMenu() {
	std::vector<TCHAR> buffer(1024);

	CDC dc;
	dc.CreateCompatibleDC(0);

	CFontHandle font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	CFontHandle prevfont = dc.SelectFont(font);

	int count = m_menu.GetMenuItemCount();
	SetRect(&m_rcMenu, 0, 0, 0, count * m_itemHeight + 3 + 3);

	m_submenus.clear();	// TODO: free data too!
	m_labels.clear();
	m_ids.clear();

	for (int i = 0; i < count; i++) {
		MENUITEMINFO mii;
		ZeroMemory(&mii, sizeof(MENUITEMINFO));
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.cch = 1024;
		mii.dwTypeData = (LPTSTR)&buffer.front();
		mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_SUBMENU | MIIM_ID; 

		m_menu.GetMenuItemInfo(i, TRUE, &mii);
		if (mii.fType == MFT_STRING) {
			std::string label(buffer.begin(), buffer.begin() + mii.cch);
			SIZE size;

			dc.GetTextExtent(label.c_str(), (int)label.length(), &size);
			if (size.cx > m_rcMenu.right)
				m_rcMenu.right = size.cx;
			m_labels.push_back(label);
		}
		if (mii.hSubMenu) {
			CModelessMenu* m = new CModelessMenu();
			m->SetPaintWindow(m_paintParent);
			m->SetMenu(m_parent, mii.hSubMenu);
			m_submenus.push_back(m);
		} else {
			m_submenus.push_back(0);
		}
		if (mii.wID == 0)
			m_ids.push_back(-1); else
			m_ids.push_back((int)mii.wID);
	}

	m_rcMenu.right += m_itemHeight + m_itemHeight + 2 + 2;

	dc.SelectFont(prevfont);
}

void CModelessMenu::SetPosition(int x, int y) {
	m_pt.x = x;
	m_pt.y = y;
	for (int i = 0; i < (int)m_submenus.size(); i++) {
		if (m_submenus[i] == 0) continue;
		m_submenus[i]->SetPosition(x + m_rcMenu.right - 6, y + m_rcMenu.top + i * m_itemHeight);
	}
}

void CModelessMenu::PaintMenu(CDC& dc) {
	int count = m_menu.GetMenuItemCount();

// 	CFont arial;
// 	arial.CreateFont(15, 15, 0, 0, 0, FALSE, FALSE, FALSE, 0, 0, 0, 0, 0, "Arial");

	CFontHandle font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	CFontHandle prevfont = dc.SelectFont(font);

	RECT rcMenu = m_rcMenu;
	SetRect(&rcMenu, rcMenu.left + m_pt.x, rcMenu.top + m_pt.y, rcMenu.right + m_pt.x, rcMenu.bottom + m_pt.y);

	dc.FillSolidRect(&rcMenu, GetSysColor(COLOR_3DFACE));
	dc.Draw3dRect(&rcMenu, GetSysColor(COLOR_3DFACE), GetSysColor(COLOR_3DSHADOW));
	InflateRect(&rcMenu, -1, -1);
	dc.Draw3dRect(&rcMenu, GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_3DDKSHADOW));

	int prevmode = dc.SetBkMode(TRANSPARENT);

	if (m_selectedIndex != -1) {
		RECT rcSelected;
		SetRect(&rcSelected, 
			rcMenu.left + 1, rcMenu.top + 2 + (m_selectedIndex * m_itemHeight), 
			rcMenu.right - 2, rcMenu.top + 1 + ((m_selectedIndex + 1) * m_itemHeight));
		dc.FillSolidRect(&rcSelected, GetSysColor(COLOR_HIGHLIGHT));
	}

	CBrush blackBrush;
	blackBrush.CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
	CBrushHandle prevbrush = dc.SelectBrush(blackBrush);

	CBrush whiteBrush;
	whiteBrush.CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHTTEXT));

	CPen blackPen;
	blackPen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWTEXT));
	CPenHandle prevpen = dc.SelectPen(blackPen);

	CPen whitePen;
	whitePen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_HIGHLIGHTTEXT));

	for (int i = 0; i < (int)m_labels.size(); i++) {
		if (m_selectedIndex == i)
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		else
			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		int ty = rcMenu.top + 3 + i * m_itemHeight;
		dc.TextOut(rcMenu.left + m_itemHeight, ty, m_labels[i].c_str());

		if (m_submenus[i] != 0) {
			int ph = 6;
			int px = rcMenu.right - 12;

			if (m_selectedIndex == i) {
				dc.SelectBrush(whiteBrush); 
				dc.SelectPen(whitePen);
			} else {
				dc.SelectBrush(blackBrush);
				dc.SelectPen(blackPen);
			}

			POINT pts[] = { px, 3 + ty, px, 3 + ty + ph, px + 3, 3 + ty + ph/2  };
			dc.Polygon(pts, 3);
		}
	}

	if (m_selectedIndex != -1 && m_selectedIndex < (int)m_submenus.size()) {
		CModelessMenu* submenu = m_submenus[m_selectedIndex];
		if (submenu != 0) {
			submenu->PaintMenu(dc);
		}
	}

	dc.SelectFont(prevfont);
	dc.SetBkMode(prevmode);
	dc.SelectBrush(prevbrush);
	dc.SelectPen(prevpen);
}

void CModelessMenu::GetMenuRect(RECT* rcMenu) {
	*rcMenu = m_rcMenu;
	rcMenu->left += m_pt.x;
	rcMenu->right += m_pt.x;
	rcMenu->top += m_pt.y;
	rcMenu->bottom += m_pt.y;
}

int CModelessMenu::HitTest(int x, int y) {
	if (x >= 0 && x < m_rcMenu.right && y>=3 && y<m_rcMenu.bottom - 5) {
		return (y - 3) / m_itemHeight;
	}
	return -1;
}

int CModelessMenu::HitTestID(int x, int y, bool deep) {
	int test = HitTest(x - m_pt.x, y - m_pt.y);

	if (test != -1 && test < (int)m_ids.size()) {
		return m_ids[test];
	}

	if (!deep) return 0;

	if (m_selectedIndex != -1 && m_selectedIndex < (int)m_submenus.size() && m_submenus[m_selectedIndex] != 0) {
		int wID = m_submenus[m_selectedIndex]->HitTestID(x, y, true);
		if (wID != 0) return wID;
	}

	return 0;
}

bool CModelessMenu::OnMouseMove(int x, int y) {
	if (m_selectedIndex != -1 && m_selectedIndex < (int)m_submenus.size()) {
		CModelessMenu* submenu = m_submenus[m_selectedIndex];
		if (submenu != 0 && submenu->OnMouseMove(x, y)) {
			return true;
		} else 
		if (submenu != 0 && submenu->m_selectedIndex != -1) {
			return false;
		}
	}
	x -= m_pt.x;
	y -= m_pt.y;

	int sel = HitTest(x, y);
	if (sel != m_selectedIndex) {
		RECT rcMenu;
		GetMenuRect(&rcMenu);
		m_paintParent.InvalidateRect(&rcMenu, FALSE);

		// invalidate both the new and previous flyouts
		if (m_selectedIndex != -1 && m_selectedIndex < (int)m_submenus.size()) {
			CModelessMenu* submenu = m_submenus[m_selectedIndex];
			if (submenu != 0) {
				submenu->GetMenuRect(&rcMenu);
				m_paintParent.InvalidateRect(&rcMenu, FALSE);
			}
		}

		if (sel != -1 && sel < (int)m_submenus.size()) {
			CModelessMenu* submenu = m_submenus[sel];
			if (submenu != 0) {
				submenu->GetMenuRect(&rcMenu);
				m_paintParent.InvalidateRect(&rcMenu, FALSE);
			}
		}

		m_selectedIndex = sel;

		return true;
	} else
		return false;	
}

bool CModelessMenu::IsEmpty() const {
	return m_menu.GetMenuItemCount() == 0;
}
