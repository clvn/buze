#pragma once

#define REFRESH_DRAG_TIMER_ID 1077
#define REFRESH_DRAG_INTERVAL 15

class CPatternEditorScroller
:
	public CWindowImpl<CPatternEditorScroller>
{
  public:

	int m_pageSize;
	int m_scrollPos;
	int m_scrollMin;
	int m_scrollMax;
	bool m_dragging;
	int m_normalizedCursor;
	int m_normalizedPage;

	int m_patternWidth;
	int m_patternHeight;
	CBitmap m_patternImg;
	CDC m_patternDC;

	int m_screenbufferWidth;
	int m_screenbufferHeight;
	CBitmap m_screenbufferImg;
	CDC m_screenbufferDC;

	bool m_scaledbufferDirty;
	CBitmap m_scaledbufferImg;
	CDC m_scaledbufferDC;

	BLENDFUNCTION m_blendFunction;
	CBitmap m_cursorImg;
	CDC m_cursorDC;

	DECLARE_WND_CLASS("PatternEditorScroller")

	BEGIN_MSG_MAP(CPatternEditorScroller)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_TIMER(OnTimer)
	END_MSG_MAP()

	CPatternEditorScroller() {
		m_pageSize = 1;
		m_scrollPos = 0;
		m_scrollMin = 0;
		m_scrollMax = 1;
		m_dragging = false;
		m_normalizedCursor = 0;
		m_normalizedPage = 1;

		m_patternWidth = 1;
		m_patternHeight = 1;

		m_screenbufferWidth = 1;
		m_screenbufferHeight = 1;
		m_scaledbufferDirty = false;

		m_bgColor = RGB(0x00, 0x00, 0x00);

		m_blockDrag = false;

		m_playPos = -1;
		m_normPlayPos = 0;
	}

	~CPatternEditorScroller() {
		if (m_hWnd) {
			DestroyWindow();
		}
	}

	CWindow m_innerWnd;
	void SetInner(HWND innerWnd) {
		m_innerWnd = innerWnd;
	}

	COLORREF m_bgColor;
	void SetBgColor(COLORREF color) {
		m_bgColor = color;
	}

	CPen m_playPen;
	void SetPlayColor(COLORREF color) {
		if (m_playPen.m_hPen)
			m_playPen.DeleteObject();
		m_playPen.CreatePen(PS_SOLID, 1, color);
	}

	void SetSelColor(COLORREF color) {
		m_cursorDC.FillSolidRect(0, 0, 4, 4, color);
	}

	void AllocatePatternImg(int cx, int cy) {
		m_patternWidth = cx;
		m_patternHeight = cy;

		if (m_patternImg.m_hBitmap)
			m_patternImg.DeleteObject();
		CDC dc(GetDC());
		m_patternImg.CreateCompatibleBitmap(dc, m_patternWidth, m_patternHeight);
		m_patternDC.SelectBitmap(m_patternImg);

		BlankPatternImg();
		BlankBackbuffers();

		CalcCursor();
		ClearPlayPos();
		///SetPlayPos(m_playPos);
	}

	void BlankPatternImg() {
		if (m_patternDC.m_hDC)
			m_patternDC.FillSolidRect(0, 0, m_patternWidth, m_patternHeight, m_bgColor);
	}

	void CreateBackbuffers() {
		CRect rcClient;
		GetClientRect(&rcClient);

		m_screenbufferWidth = rcClient.Width();
		m_screenbufferHeight = rcClient.Height();

		if (m_screenbufferImg.m_hBitmap)
			m_screenbufferImg.DeleteObject();
		CDC dc(GetDC());
		m_screenbufferImg.CreateCompatibleBitmap(dc, m_screenbufferWidth, m_screenbufferHeight);
		m_screenbufferDC.SelectBitmap(m_screenbufferImg);
		m_screenbufferDC.SetStretchBltMode(COLORONCOLOR);///

		if (m_scaledbufferImg.m_hBitmap)
			m_scaledbufferImg.DeleteObject();
		m_scaledbufferImg.CreateCompatibleBitmap(dc, m_screenbufferWidth, m_screenbufferHeight);
		m_scaledbufferDC.SelectBitmap(m_scaledbufferImg);
		m_scaledbufferDC.SetStretchBltMode(HALFTONE);

		BlankBackbuffers();
	}

	void BlankBackbuffers() {
		if (m_screenbufferDC.m_hDC)
			m_screenbufferDC.FillSolidRect(0, 0, m_screenbufferWidth, m_screenbufferHeight, m_bgColor);

		if (m_scaledbufferDC.m_hDC)
			m_scaledbufferDC.FillSolidRect(0, 0, m_screenbufferWidth, m_screenbufferHeight, m_bgColor);
	}

	int OnCreate(LPCREATESTRUCT lpCreateStruct) {
		m_playPen.CreatePen(PS_SOLID, 1, RGB(0x80, 0x00, 0x00));

		CDC dc(GetDC());
		m_patternDC.CreateCompatibleDC(dc);
		m_screenbufferDC.CreateCompatibleDC(dc);
		m_scaledbufferDC.CreateCompatibleDC(dc);
		m_cursorDC.CreateCompatibleDC(dc);

		{	m_blendFunction.BlendOp = AC_SRC_OVER;
			m_blendFunction.BlendFlags = 0;
			m_blendFunction.SourceConstantAlpha = 32;//255;
			m_blendFunction.AlphaFormat = 0;//AC_SRC_ALPHA;
			m_cursorImg.CreateCompatibleBitmap(dc, 4, 4);
			m_cursorDC.SelectBitmap(m_cursorImg);
			m_cursorDC.FillSolidRect(0, 0, 4, 4, RGB(0x00, 0x00, 0xFF));
		}

		AllocatePatternImg(m_patternWidth, m_patternHeight);
		CreateBackbuffers();

		return FALSE;
	}

	void OnClose() {
	}

	void OnSize(UINT nType, CSize size) { // todo: horiz resize only?
		CreateBackbuffers();
		CalcCursor();
		SetPlayPos(m_playPos);
		if (!IsUnscaled())
			m_scaledbufferDirty = true;
	}

	void OnPaint(CDCHandle /*dc*/) {
		CPaintDC screenDC(m_hWnd);
		CRect rcClip;
		screenDC.GetClipBox(&rcClip);

		if (IsUnscaled()) {
			m_screenbufferDC.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), m_patternDC, rcClip.left, rcClip.top, SRCCOPY);
		} else {
			if (m_scaledbufferDirty) {
				double normFactor = (double)m_patternHeight / (double)m_screenbufferHeight;
				int y_pos = (int)(((double)rcClip.top * normFactor) + 0.5);///
				int y_height = (int)(((double)rcClip.Height() * normFactor) + 0.5);///
				int limit_width = min((int)rcClip.right, m_patternWidth) - rcClip.left;
				m_scaledbufferDC.StretchBlt(rcClip.left, rcClip.top, limit_width, rcClip.Height(), m_patternDC, rcClip.left, y_pos, limit_width, y_height, SRCCOPY);
				m_scaledbufferDirty = false;
			}

			m_screenbufferDC.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), m_scaledbufferDC, rcClip.left, rcClip.top, SRCCOPY);
		}

		if ((m_playPos != -1) && (m_normPlayPos >= rcClip.top) && (m_normPlayPos < rcClip.bottom)) {
			CPenHandle oldPen = m_screenbufferDC.SelectPen(m_playPen);
			m_screenbufferDC.MoveTo(0, m_normPlayPos);
			m_screenbufferDC.LineTo(m_patternWidth, m_normPlayPos);
			m_screenbufferDC.SelectPen(oldPen);
		}

		{	CRect rcCursor(0, m_normalizedCursor, m_patternWidth, m_normalizedCursor + m_normalizedPage);
			IntersectRect(&rcCursor, &rcCursor, &rcClip);
			if (rcCursor.Height() != 0) {
				int cursorWidth_min = max(rcCursor.Width(), 8);
				m_screenbufferDC.AlphaBlend(rcCursor.left, rcCursor.top, cursorWidth_min, rcCursor.Height(), m_cursorDC, 0, 0, 4, 4, m_blendFunction);
			}
		}

		screenDC.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), m_screenbufferDC, rcClip.left, rcClip.top, SRCCOPY);
	}

	BOOL OnEraseBkgnd(CDCHandle /*dc*/) {
		return FALSE;
	}

	int m_playPos;
	int m_normPlayPos;
	void SetPlayPos(int y) {
		InvalidatePlayPos();

		if (y >= m_patternHeight)
			y = -1;

		if (y != -1) {
			m_playPos = y;

			double normFactor = IsUnscaled() ? 1.0 : (double)m_screenbufferHeight / (double)m_patternHeight;
			m_normPlayPos = (int)(((double)y * normFactor) + 0.5);

			InvalidatePlayPos();
		} else {
			ClearPlayPos();
		}
	}

	void ClearPlayPos() {
		m_playPos = -1;
		m_normPlayPos = -1;
	}

	void InvalidatePlayPos() {
		CRect rcInvalid(0, m_normPlayPos, m_patternWidth, m_normPlayPos + 1);
		InvalidateRect(&rcInvalid, FALSE);
	}

	void CalcCursor() {
		//int range = m_scrollMax - m_scrollMin - m_pageSize + 2;
		//int range1 = (m_scrollMax - m_scrollMin);///+ 1;
		int range0 = (m_scrollMax - m_scrollMin);
		int cursor = m_scrollPos - m_scrollMin;
		double normFactor = IsUnscaled() ? 1.0 : (double)m_screenbufferHeight / (double)range0;
		m_normalizedCursor = (int)((double)cursor * normFactor);/// + 0.5;
		m_normalizedPage = (int)(((double)(m_pageSize - 1) * normFactor) + 0.5);
		m_normalizedPage = std::max(8, m_normalizedPage);

		if (IsUnscaled()) {
			if (m_normalizedPage > m_patternHeight)
				m_normalizedPage = m_patternHeight;
		} else {
			if (m_normalizedPage > m_screenbufferHeight)
				m_normalizedPage = m_screenbufferHeight;
		}
// 		if ((m_normalizedCursor + m_normalizedPage) > m_screenbufferHeight)
// 			m_normalizedCursor = m_screenbufferHeight - m_normalizedPage;
	}

	void OnLButtonDown(UINT nFlags, CPoint point) {
		m_dragging = true;
		SetCapture();
		DoDrag(point, true);
	}

	void OnMouseMove(UINT nFlags, CPoint point) {
		if (m_dragging) {
			DoDrag(point, false);
		}
	}

	void OnLButtonUp(UINT nFlags, CPoint point) {
		if (m_dragging) {
			ReleaseCapture();
			m_dragging = false;
			KillTimer(REFRESH_DRAG_TIMER_ID);
			m_blockDrag = false;
		}
	}

	bool m_blockDrag;
	CPoint m_blockedPoint;

	void OnTimer(UINT_PTR nIDEvent) {
		KillTimer(REFRESH_DRAG_TIMER_ID);
		m_blockDrag = false;

		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);
		if (point.y != m_blockedPoint.y)
			DoDrag(point, false);
	}

	void DoDrag(CPoint point, bool initial) {
		if (!initial && m_blockDrag) return;

		double normFactor = IsUnscaled() ? 1.0 : (double)m_patternHeight / (double)m_screenbufferHeight;
//		int line = ((double)point.y * normFactor) + 0.5;
//		line -= m_pageSize / 2;

		int line = (int)((((double)point.y * normFactor) - ((double)m_pageSize / 2.0)) + 0.5);
		if (line < 0)
			line = 0;
		if (line >= m_patternHeight)
			line = m_patternHeight - 1;

		if (line != m_scrollPos) {
			WPARAM wParam = (line << 16);
			wParam |= initial ? SB_THUMBPOSITION : SB_THUMBTRACK;
			m_innerWnd.PostMessage(WM_VSCROLL, wParam, (LPARAM)m_hWnd);
			if (initial) {
				m_innerWnd.PostMessage(WM_VSCROLL, SB_ENDSCROLL, (LPARAM)m_hWnd);
			}
		}

		SetTimer(REFRESH_DRAG_TIMER_ID, REFRESH_DRAG_INTERVAL, 0);
		m_blockDrag = true;
		m_blockedPoint = point;
	}

	void SetScrollInfo(SCROLLINFO* si) {
		if (si->fMask & SIF_PAGE)
			m_pageSize = si->nPage;

		if (si->fMask & SIF_POS)
			m_scrollPos = si->nPos;

		if (si->fMask & SIF_RANGE) {
			if ((m_scrollMin != si->nMin) || (m_scrollMax != si->nMax)) {
				///m_scaledbufferDirty = true;
			}

			m_scrollMin = si->nMin;
			m_scrollMax = si->nMax;
		}

		CalcCursor();
		Invalidate(FALSE);
	}

	void SetScrollPos(int y) {
		if (m_scrollPos != y) {
			InvalidateCursor();

			m_scrollPos = y;

			CalcCursor();
			InvalidateCursor();
		}
	}

	void InvalidateCursor() {
		CRect rcInvalid(0, m_normalizedCursor, m_patternWidth, m_normalizedCursor + m_normalizedPage);
		InvalidateRect(&rcInvalid, FALSE);
	}

	void InvalidateUnits(CRect rcInvalidUnits) {
		CRect rcInvalid;

		if (IsUnscaled()) {
			rcInvalid.SetRect(rcInvalidUnits.left, rcInvalidUnits.top, rcInvalidUnits.right, rcInvalidUnits.bottom);
		} else {
			double normFactor = (double)m_screenbufferHeight / (double)m_patternHeight;
			int inv_top = (int)((double)rcInvalidUnits.top * normFactor);/// + 0.5;
			int inv_bottom = (int)(((double)rcInvalidUnits.bottom * normFactor) + 0.5);
			rcInvalid.SetRect(rcInvalidUnits.left, inv_top - 1, rcInvalidUnits.right, inv_bottom + 1); // -1 +1 or the stretch may not look right
			m_scaledbufferDirty = true;
		}

		InvalidateRect(&rcInvalid, FALSE);
	}

	bool IsUnscaled() const {
		return m_patternHeight <= m_screenbufferHeight;
	}
};
