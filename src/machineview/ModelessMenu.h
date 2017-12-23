#pragma once

// this paints on your window. call SetPaintWindow() with the handle the menu should invalidate
// the HWND passed to SetMenu is the window receiving the menu command, not necessarily the same as the paint window
// both must be set!

class CModelessMenu {
public:
	POINT m_pt;
	CMenu m_menu;
	CWindow m_parent;
	CWindow m_paintParent;
	std::vector<std::string> m_labels;
	std::vector<CModelessMenu*> m_submenus;
	std::vector<int> m_ids;
	RECT m_rcMenu;
	int m_selectedIndex;
	int m_itemHeight;
	int m_left, m_top;

	CModelessMenu();
	void SetPaintWindow(HWND paintParent);
	void SetMenu(HWND parent, HMENU menu);
	void UpdateFromMenu();
	void SetPosition(int x, int y);
	void PaintMenu(CDC& dc);
	void GetMenuRect(RECT* rcMenu);
	int HitTest(int x, int y);
	int HitTestID(int x, int y, bool deep);
	bool OnMouseMove(int x, int y);
	bool IsEmpty() const;
};
