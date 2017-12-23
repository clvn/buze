#pragma once

#include "VisualStylesXP.h"

// ---------------------------------------------------------------------------------------------------------------
// DOUBLE BUFFERED LABEL
// ---------------------------------------------------------------------------------------------------------------

class CTextLabel
:
	public CWindowImpl<CTextLabel>,
	public CDoubleBufferImpl<CTextLabel>
{
  public:

	DECLARE_WND_CLASS("TextLabel")

	BEGIN_MSG_MAP_EX(CTextLabel)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CDoubleBufferImpl<CTextLabel>)
	END_MSG_MAP()

	CTextLabel() {
		label = "";
		color = ::GetSysColor(COLOR_WINDOWTEXT);
		bkcolor = ::GetSysColor(COLOR_BTNFACE);
		themebg = true;
		center = true;
		xLeft = 0;
		yTop = 0;
		fontHeight = 0;
	}

	void SetLabel(std::string const& label) {
		if (this->label != label) {
			this->label = label;
			Invalidate();
		}
	}

	void SetColor(COLORREF color) {
		if (this->color != color) {
			this->color = color;
			Invalidate();
		}
	}

	void SetBkColor(COLORREF color) {
		if (this->bkcolor != bkcolor) {
			this->bkcolor = bkcolor;
			Invalidate();
		}
	}

	void SetThemeBG(bool themebg) {
		if (this->themebg != themebg) {
			this->themebg = themebg;
			Invalidate();
		}
	}

	void SetCenter(bool center) {
		if (this->center != center) {
			this->center = center;
			Invalidate();
		}
	}

	void DoPaint(CDCHandle dcPaint) {
		if (themebg && g_xpStyle.UseVisualStyles()) {
			g_xpStyle.DrawThemeParentBackground(m_hWnd, dcPaint, 0);
		} else {
			RECT rc;
			GetClientRect(&rc);
			dcPaint.FillSolidRect(0, 0, rc.right, rc.bottom, bkcolor);
		}

		CFontHandle prevFont = dcPaint.SelectFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		int prevMode = dcPaint.SetBkMode(TRANSPARENT);
		COLORREF prevColor = dcPaint.SetTextColor(color);
		int prevAlign = dcPaint.SetTextAlign((center ? TA_CENTER : TA_LEFT) | TA_TOP);

		dcPaint.TextOut(xLeft, yTop, label.c_str());

		dcPaint.SetTextAlign(prevAlign);
		dcPaint.SetTextColor(prevColor);
		dcPaint.SetBkMode(prevMode);
		dcPaint.SelectFont(prevFont);
	}

  protected:

	std::string label;
	COLORREF color;
	COLORREF bkcolor;
	bool themebg;
	bool center;
	int xLeft, yTop;
	int fontHeight;

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		LRESULT lRes = DefWindowProc();

		CWindowDC dc(m_hWnd);
		CFontHandle prevFont = dc.SelectFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

		SIZE sz;
		dc.GetTextExtent("X", 1, &sz);
		fontHeight = sz.cy;

		dc.SelectFont(prevFont);

		return lRes;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		RECT rc;
		GetClientRect(&rc);
		if (center)
			xLeft = rc.right/2;
		else
			xLeft = 0;
		yTop = (rc.bottom - fontHeight) / 2; // vcenter

		return 0;
	}
};
