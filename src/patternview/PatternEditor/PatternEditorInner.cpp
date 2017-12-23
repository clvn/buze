#include "stdafx.h"
#include "resource.h"
#include "PatternEditorInner.h"
#include "PatternEditorColumn.h"
#include "utils.h"
#include "PrefGlobals.h"

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DESTRUCTION
// ---------------------------------------------------------------------------------------------------------------

CPatternEditorInner::CPatternEditorInner(CPatternEditorScroller& scroller)
:
	scroller(scroller)
{
	font_size.cx = font_size.cy = 0;
	cursor.x = cursor.y = 0;
	scroll.x = scroll.y = 0;
	select_from.x = select_from.y = -1;
	select_to.x = select_to.y = -1;
	select_begin.x = select_begin.y = -1;
	select_end.x = select_end.y = -1;
	mouse_mode = mouse_mode_none;
	drag_mode = drag_mode_none;
	double_clicking = false;
	octave = 4;
	skip = 1;
	pattern_rows = 0;
	editor_rows = 0;
	editor_units = 0;
	verydark_row = 16;
	dark_row = 4;
	step = 1;
	last_play_pos = -1;
	loop_begin_pos = -1;
	loop_end_pos = -1;
	noteOffStr = "off";
	noteCutStr = "^^^";
	bg_note = "---";
	bg_byte = "..";
	bg_switch = ".";
	bg_word = "....";
	horizontal_entry = false;
	sticky_selection = false;
	clientDC = 0;
	kb_delay_ms = 0;
	kb_speed_ms = 0;
	kb_fastdelay_ms = 0;
	timer_active = false;
	repeat_transition = false;
	loop_enabled = false;
	MaskNoteReset(true);
	transpose_set_mode = false;
	notecolors_enabled = true;
	transpose_set_enabled.assign(0);
	transpose_set_disabled.assign(0);
	hsys.invalidate = boost::bind(&CPatternEditorInner::HSysInvalidate, this, _1, _2);
	horizontal_scroll_mode = false;
	vertical_scroll_mode = false;
	rcPreviewInvalid.SetRect(-1, 0, 0, 0);
	caret_bitmap_size = CSize(-1, -1);
	cursor_offscreen = false;
	resizing = false;
	entering_chord = false;
	entering_chord_origin = -1;
	did_chord_step = false;
	dirty_centeroncursor = false;
	dirty_scrolltocursor = false;
	notes_affect_waves = true;
	notes_affect_volumes = true;
	keyjazz_key_map = 0;
	all_notes_held = false;

	for (int i = 0; i < PE_THEMEINDEX_SIZE; ++i) {
		colors[i] = RGB(0x00, 0x00, 0x00);
	}
}

CPatternEditorInner::~CPatternEditorInner() {
	delete clientDC;

	for (std::vector<PE_column*>::iterator i = columns.begin(); i != columns.end(); ++i) {
		delete (*i)->editor;
		delete *i;
	}
}

LRESULT CPatternEditorInner::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	clientDC = new CClientDC(m_hWnd);

	black_brush.CreateSolidBrush(RGB(0, 0, 0));
	white_brush.CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
	black_pen.CreatePen(PS_SOLID, 2, (COLORREF)0);
	looppoints_pen.CreatePen(PS_SOLID, 2, colors[PE_LoopPoints]);
	looppointsdisabled_pen.CreatePen(PS_SOLID, 2, colors[PE_LoopPoints_Disabled]);
	playbackpos_pen.CreatePen(PS_SOLID, 2, colors[PE_PlaybackPos]);
	divider_pen.CreatePen(PS_SOLID, 1, colors[PE_Divider]);
	hidden_pen.CreatePen(PS_SOLID, 1, colors[PE_Hidden]);
	thin_pen.CreatePen(PS_SOLID, 1, RGB(0x00,0x00,0x00));

	CFontHandle font;
	font.CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH|FF_ROMAN, _T(""));
	SetFont(font);

	CalcKeyboardSpeed();

	m_selectionImg.CreateCompatibleBitmap(clientDC->m_hDC, 4, 4);
	m_selectionDC.CreateCompatibleDC(clientDC->m_hDC);
	m_selectionDC.SelectBitmap(m_selectionImg);
	m_selectionDC.FillSolidRect(0, 0, 4, 4, colors[PE_Selection]);
	blendFunc.BlendOp = AC_SRC_OVER;
	blendFunc.BlendFlags = 0;
	blendFunc.SourceConstantAlpha = 32;//96;//32;//32;//255;
	blendFunc.AlphaFormat = 0;//AC_SRC_ALPHA;

	return DefWindowProc();
}

LRESULT CPatternEditorInner::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// SIZING / SCROLLING
// ---------------------------------------------------------------------------------------------------------------

LRESULT CPatternEditorInner::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	if (resizing) return 0; // block extra OnSizes while ScrollInfo changes
	resizing = true;

	UpdateScrollbars();
	GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_RESIZED, 0), 0);

	resizing = false;
	return 0;
}

CSize CPatternEditorInner::GetScreenSize() const {
	CRect rcClient;
	GetClientRect(&rcClient);
	return CSize(
		rcClient.right / font_size.cx,
		rcClient.bottom / font_size.cy
	);
}

CPoint CPatternEditorInner::GetMaxScroll() const {
	CSize screen = GetScreenSize();
	return CPoint(
		max((editor_units + 1) - (int)screen.cx, 0),
		max((editor_rows  + 0) - (int)screen.cy, 0)
	);
}

void CPatternEditorInner::InitScroll() {
	scroll.x = 0;
	scroll.y = 0;
	SetScrollPos(SB_HORZ, 0);
	SetScrollPos(SB_VERT, 0);
	ShowScrollBar(SB_VERT, FALSE);
	scroller.SetScrollPos(scroll.y);
}

void CPatternEditorInner::UpdateScrollbars() {
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);

	// horizontal
	{
		CSize screen = GetScreenSize();

		si.nMin = 0;
		si.nMax = editor_units;
		si.nPage = screen.cx;
		si.fMask = SIF_RANGE | SIF_PAGE;
		SetScrollInfo(SB_HORZ, &si);
	}

	// vertical
	{
		CSize screen = GetScreenSize();

		si.nMin = 0;
		si.nMax = editor_rows;
		si.nPage = screen.cy + 1;
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;
		SetScrollInfo(SB_VERT, &si);

		ShowScrollBar(SB_VERT, FALSE);
		scroller.SetScrollInfo(&si);
	}

	// clamp
	{
		CPoint max_scroll = GetMaxScroll();

		scroll.x = clamp(scroll.x, 0, max_scroll.x);
		scroll.y = clamp(scroll.y, 0, max_scroll.y);
	}

	// restore cursor into view if it was previously visible
	{
		bool cursor_offscreen_new = IsCursorPointOffscreen(cursor);
		if (!cursor_offscreen && cursor_offscreen_new) {
			scroll = GetScrollToCursorPointCentered(cursor, scroll);
			cursor_offscreen = false;
		} else {
			cursor_offscreen = cursor_offscreen_new;
		}
	}

	SetScrollPos(SB_HORZ, scroll.x);
	SetScrollPos(SB_VERT, scroll.y);
	scroll.x = GetScrollPos(SB_HORZ);
	scroll.y = GetScrollPos(SB_VERT);
	scroller.SetScrollPos(scroll.y);
	UpdateCaret();
}

LRESULT CPatternEditorInner::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/) {
	WORD req = LOWORD(wParam);
	WORD trackValue = HIWORD(wParam);

	switch (req) {
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			ScrollTo(trackValue, scroll.y);
			break;
		case SB_LINERIGHT:
			ScrollTo(scroll.x + 1, scroll.y);
			break;
		case SB_LINELEFT:
			ScrollTo(scroll.x - 1, scroll.y);
			break;
		case SB_PAGERIGHT:
			ScrollTo(scroll.x + 15, scroll.y);
			break;
		case SB_PAGELEFT:
			ScrollTo(scroll.x - 15, scroll.y);
			break;
		default:
			return 0;
	}

	return 0;
}

LRESULT CPatternEditorInner::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/) {
	WORD req = LOWORD(wParam);
	WORD trackValue = HIWORD(wParam);

	switch (req) {
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			ScrollTo(scroll.x, trackValue);
			break;
		case SB_LINEDOWN:
			ScrollTo(scroll.x, scroll.y + 1);
			break;
		case SB_LINEUP:
			ScrollTo(scroll.x, scroll.y - 1);
			break;
		case SB_PAGEDOWN:
			ScrollTo(scroll.x, scroll.y + 15);
			break;
		case SB_PAGEUP:
			ScrollTo(scroll.x, scroll.y - 15);
			break;
		default:
			return 0;
	}

	return 0;
}

bool CPatternEditorInner::ScrollTo(int x, int y) {
	POINT pt = { x, y };
	return ScrollTo(pt);
}

bool CPatternEditorInner::ScrollTo(CPoint new_scroll) {
	SetScrollPos(SB_HORZ, new_scroll.x);
	SetScrollPos(SB_VERT, new_scroll.y);
	new_scroll.x = GetScrollPos(SB_HORZ);
	new_scroll.y = GetScrollPos(SB_VERT);

	if ((scroll.x != new_scroll.x) || (scroll.y != new_scroll.y)) {
		int scroll_amount_x = (scroll.x - new_scroll.x) * font_size.cx;
		int scroll_amount_y = (scroll.y - new_scroll.y) * font_size.cy;

		/// v1 -- ScrollWindow scrolls the Caret
// 		{	UpdateCaret();
// 			scroll = new_scroll;
// 			ScrollWindow(scroll_amount_x, scroll_amount_y);
// 		}

		/// v2 -- ScrollWindowEx does not scroll the Caret
		{	ScrollWindowEx(scroll_amount_x, scroll_amount_y, (LPCRECT)0, 0, 0, 0, SW_INVALIDATE);
			scroll = new_scroll;
			UpdateCaret();
		}

		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_SCROLLED, 0), 0);
		scroller.SetScrollPos(scroll.y);

		cursor_offscreen = IsCursorPointOffscreen(cursor);

		return true;
	} else {
		return false;
	}
}

// ---------------------------------------------------------------------------------------------------------------
// PAINTING
// ---------------------------------------------------------------------------------------------------------------

namespace {
enum {
	FR_SIDE_LEFT	= 0x1,
	FR_SIDE_TOP		= 0x2,
	FR_SIDE_RIGHT	= 0x4,
	FR_SIDE_BOTTOM	= 0x8,
};
void DrawFocusRectEx(CDCHandle dc, RECT* rc, int thickness, int sides) {
	CRect rcFocus = *rc;

	LOGBRUSH LogBrush;
	LogBrush.lbColor = RGB(0xFF,0xFF,0xFF);
	LogBrush.lbStyle = PS_SOLID;

	CPen focusPen;
	focusPen.CreatePen(PS_COSMETIC|PS_ALTERNATE, 1, &LogBrush, 0, NULL);
	CPenHandle oldPen = dc.SelectPen(focusPen);

	int oldRop2 = dc.SetROP2(R2_XORPEN);

	for (int i = 0; i < thickness; ++i) {
		int left = rcFocus.left;
		int top = rcFocus.top;
		int right = rcFocus.right - 1;
		int bottom = rcFocus.bottom - 1;

		dc.MoveTo(left, top);

		if (sides & FR_SIDE_TOP)
			dc.LineTo(right, top);
		else
			dc.MoveTo(right, top);

		if (sides & FR_SIDE_RIGHT)
			dc.LineTo(right, bottom);
		else
			dc.MoveTo(right, bottom);

		if (sides & FR_SIDE_BOTTOM)
			dc.LineTo(left, bottom);
		else
			dc.MoveTo(left, bottom);

		if (sides & FR_SIDE_LEFT)
			dc.LineTo(left, top);

		rcFocus.InflateRect(-1, -1);
	}

	dc.SetROP2(oldRop2);
	dc.SelectPen(oldPen);
}
} // END namespace

LRESULT CPatternEditorInner::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
// 	CRect rcUpdate;
// 	GetUpdateRect(&rcUpdate);
// 	if (rcUpdate != CRect(0,0,1,1)) {

	int width = editor_units * font_size.cx;
	int height = editor_rows * font_size.cy;

	CPaintDC screenDC(m_hWnd);
	screenDC.SetWindowExt(width, height);
	screenDC.SetWindowOrg(scroll.x * font_size.cx, scroll.y * font_size.cy);
	clientDC->SetWindowOrg(scroll.x * font_size.cx, scroll.y * font_size.cy);

	CMemDC dc(screenDC);
	dc.SetBkColor(colors[PE_BG]);
	dc.SetTextColor(colors[PE_TextValue]);
	dc.FillSolidRect(&dc.m_rc, colors[PE_BG]);

	//DBG(dc.m_rc.left) DBG(dc.m_rc.top) DBG(dc.m_rc.right) DBG(dc.m_rc.bottom) END()

	if (!columns.empty()) {
		int firstcliprow = dc.m_rc.top / font_size.cy;
		int lastcliprow = std::min((int)(dc.m_rc.bottom / font_size.cy) + 1, editor_rows);
		int cliprowcount = lastcliprow - firstcliprow;

		int firstclipcol = ScreenUnitsToCursorColumn(dc.m_rc.left / font_size.cx);
		if (firstclipcol == -1)
			firstclipcol = cursorcolumns.size() - 1;
		if (firstclipcol > 0)
			firstclipcol = cursorcolumns[firstclipcol - 1].column;
		else
			firstclipcol = 0;

		int lastclipcol = ScreenUnitsToCursorColumn(dc.m_rc.right / font_size.cx);
		if (lastclipcol == -1)
			lastclipcol = cursorcolumns.size() - 1;
		if (lastclipcol > 0)
			lastclipcol = cursorcolumns[lastclipcol].column;
		else
			lastclipcol = 0;

		CFontHandle prevfont = dc.SelectFont(track_font);

		int last_plug = -1;

		for (int j = firstclipcol; j <= lastclipcol; ++j) {
			PE_column const& col = *columns[j];

			// draw column divider after the next column
			int colwidth = col.editor->GetWidth();
			int x = (col.unit + colwidth) * font_size.cx;

			for (int i = firstcliprow; i < lastcliprow; ++i) {
				int y = font_size.cy * i;
				RECT rcCell = { x, y, x + font_size.cx, y + font_size.cy };

				COLORREF cellcol;
				if (!col.spacer && (j != columns.size() - 1)) {
					cellcol = col.editor->GetBkColor(i * skip);
				} else {
					cellcol = colors[PE_BG];
				}
				dc.FillSolidRect(&rcCell, cellcol);
			}

			// draw line on plugin divider on the previous column (except the first)
			x = col.unit * font_size.cx;
			if ((col.plugin_id != last_plug) && (j != 0)) {
				dc.SelectPen(divider_pen);
				dc.MoveTo(x - 4, dc.m_rc.top);
				dc.LineTo(x - 4, min(dc.m_rc.bottom, editor_rows * font_size.cy));///
			}
			last_plug = col.plugin_id;

			col.editor->RenderValues(dc, firstcliprow, cliprowcount);
		}

	// 	if (HasSelection()) {
	// 		dc.SetTextColor(colors[PE_Selection]);
	// 		dc.SetBkColor(colors[PE_Cursor]);
	// 
	// 		RECT rcSel;
	// 		GetSelectionRectScreen(&rcSel);
	// 
	// 		dc.InvertRect(&rcSel);
	// 	}
		if (HasSelection()) {
			CRect rcSel;
			GetSelectionRectScreen(&rcSel);
			IntersectRect(&rcSel, &rcSel, &dc.m_rc);
			dc.AlphaBlend(rcSel.left, rcSel.top, rcSel.Width(), rcSel.Height(), m_selectionDC, 0, 0, 4, 4, blendFunc);
		}

		if (mouse_mode == mouse_mode_drag) {
			dc.SetTextColor(colors[PE_TextValue]);
			dc.SetBkColor(colors[PE_Cursor]);

			int sel_width = (select_end.x - select_begin.x);
			int sel_height = (select_end.y - select_begin.y);

			int sides = 0;
			if (drag_to.x >= 0)
				sides |= FR_SIDE_LEFT;
			if (drag_to.y >= 0)
				sides |= FR_SIDE_TOP;
			if ((drag_to.x + sel_width) < columns.size())
				sides |= FR_SIDE_RIGHT;
			if ((drag_to.y + sel_height) < editor_rows)
				sides |= FR_SIDE_BOTTOM;

			RECT rcDrag;
			GetDragtoRectScreen(&rcDrag);
			DrawFocusRectEx(dc.m_hDC, &rcDrag, 3, sides);
		}

		if (loop_begin_pos != -1) {
			int looppos = loop_begin_pos / skip;
			float rowpos = (float)(loop_begin_pos % skip) / skip; // 0..1
			dc.SelectPen(loop_enabled ? looppoints_pen : looppointsdisabled_pen);
			int y = 1 + (looppos) * font_size.cy + (rowpos * font_size.cy);
			dc.MoveTo(0, y);
			dc.LineTo(editor_units * font_size.cx, y);
		}

		if (loop_end_pos != -1) {
			int looppos = loop_end_pos / skip;
			float rowpos = (float)(loop_end_pos % skip) / skip; // 0..1
			dc.SelectPen(loop_enabled ? looppoints_pen : looppointsdisabled_pen);
			int y = 1 + (looppos) * font_size.cy + (rowpos * font_size.cy);
			dc.MoveTo(0, y);
			dc.LineTo(editor_units * font_size.cx, y);
		}

		if (last_play_pos != -1) {
			int looppos = last_play_pos / skip;
			float rowpos = (float)(last_play_pos % skip) / skip; // 0..1
			dc.SelectPen(playbackpos_pen);
			int y = 1 + (looppos) * font_size.cy + (rowpos * font_size.cy);
			dc.MoveTo(0, y);
			dc.LineTo(editor_units * font_size.cx, y);
		}

		dc.SelectFont(prevfont);
	}

// 	}
// 	else {
// 		SetMsgHandled(FALSE);
// 	}

	if (rcPreviewInvalid.left != -1) {
		RenderPreview(false);
	}

	return 0;
}

void CPatternEditorInner::RenderPreview(bool initial) {
	if (columns.size() == 0) return ;

	CDC& dc = scroller.m_patternDC;

	if (initial) {
		rcPreviewInvalid.left = 0;
		rcPreviewInvalid.top = 0;
		rcPreviewInvalid.right = columns.size() - 1;
		int maxrows = std::numeric_limits<int>::max() / (int)font_size.cy; // hmm
		rcPreviewInvalid.bottom = std::min(editor_rows - 1, maxrows);
	}

	RECT rcPreview = {
		columns[rcPreviewInvalid.left]->unit,
		rcPreviewInvalid.top,
		columns[rcPreviewInvalid.right]->unit + columns[rcPreviewInvalid.right]->editor->GetWidth(),
		rcPreviewInvalid.bottom + 1
	};

	int length = rcPreviewInvalid.bottom - rcPreviewInvalid.top + 1;

	// Fill bg
//	dc.FillSolidRect(&rcPreview, colors[PE_BG]);

	int last_plug = -1;
	for (int xi = rcPreviewInvalid.left; xi <= rcPreviewInvalid.right; ++xi) {
		PE_column const& col = *columns[xi];

		// row highlights
		int colunit = col.unit;
		int colwidth = col.editor->GetWidth();
		int add_spacer = col.spacer ? 0 : 1;
		for (int yi = rcPreviewInvalid.top; yi <= rcPreviewInvalid.bottom; ++yi) {
			COLORREF cellColor = col.editor->GetBkColor(yi * skip);
			RECT rcCell = { colunit, yi, colunit + colwidth + add_spacer, yi + 1 };
			dc.FillSolidRect(&rcCell, cellColor);
		}

		// plugin divider
		if (initial) {
			int colunit_edge = col.unit - 1;
			if ((col.plugin_id != last_plug) && (xi != 0)) {
				CPenHandle oldPen = dc.SelectPen(divider_pen);
				dc.MoveTo(colunit_edge, 0);
				dc.LineTo(colunit_edge, editor_rows);
				dc.SelectPen(oldPen);
			}
		}

		last_plug = col.plugin_id;

		col.editor->PreviewValues(dc, rcPreviewInvalid.top, length);
	}

	rcPreviewInvalid.left = -1;
	scroller.InvalidateUnits(&rcPreview);
}

LRESULT CPatternEditorInner::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	return 1;
}

void CPatternEditorInner::UpdatePlayPosition(int pos) {
	if (last_play_pos == pos) return;

	RECT rcClient;
	GetClientRect(&rcClient);

	// invalidate scroller
	{	if (pos == -1)
			scroller.SetPlayPos(-1);
		else if ((last_play_pos / skip) != (pos / skip))
			scroller.SetPlayPos(pos / skip);
	}

	// invalidate old position
	if (last_play_pos != -1) {
		int looppos = last_play_pos / skip;
		float rowpos = (float)(last_play_pos % skip) / skip; // 0..1	
		int y = ((looppos - scroll.y) * font_size.cy) + (rowpos * font_size.cy);
		RECT rcPlayPos = { 0, y, rcClient.right, y + 2 };
		InvalidateRect(&rcPlayPos, FALSE);
	}

	last_play_pos = pos;

	// invalidate new position
	{	int looppos = last_play_pos / skip;
		float rowpos = (float)(last_play_pos % skip) / skip; // 0..1
		int y = ((looppos - scroll.y) * font_size.cy) + (rowpos * font_size.cy);
		RECT rcPlayPos = { 0, y, rcClient.right, y + 2 };
		InvalidateRect(&rcPlayPos, FALSE);
	}
}

void CPatternEditorInner::SetFont(HFONT hFont, bool bRedraw) {
	if (track_font.m_hFont != 0) track_font.DeleteObject();
	track_font = hFont;
	CFontHandle prevfont = clientDC->SelectFont(track_font);
	GetTextExtentPoint32(*clientDC, _T("M"), 1, &font_size);
	clientDC->SelectFont(prevfont);
	AllocatePattern(false);
}

void CPatternEditorInner::UpdateTheme() {
	looppoints_pen.DeleteObject();
	looppoints_pen.CreatePen(PS_SOLID, 2, colors[PE_LoopPoints]);
	looppointsdisabled_pen.DeleteObject();
	looppointsdisabled_pen.CreatePen(PS_SOLID, 2, colors[PE_LoopPoints_Disabled]);
	playbackpos_pen.DeleteObject();
	playbackpos_pen.CreatePen(PS_SOLID, 2, colors[PE_PlaybackPos]);
	divider_pen.DeleteObject();
	divider_pen.CreatePen(PS_SOLID, 1, colors[PE_Divider]);
	hidden_pen.DeleteObject();
	hidden_pen.CreatePen(PS_SOLID, 1, colors[PE_Hidden]);
	m_selectionDC.FillSolidRect(0, 0, 4, 4, colors[PE_Selection]);
	scroller.SetBgColor(colors[PE_BG]);
	scroller.SetPlayColor(AverageRGB(colors[PE_PlaybackPos], colors[PE_BG], 2));
	scroller.SetSelColor(colors[PE_Selection]);
}

// ---------------------------------------------------------------------------------------------------------------
// UNIT CALCULATIONS
// ---------------------------------------------------------------------------------------------------------------

bool CPatternEditorInner::GetScreenPosition(int col, int row, POINT* pt) {
	if (col == -1) return false;
	if (cursorcolumns.size() == 0) return false;
	col = clamp(col, 0, (int)cursorcolumns.size() - 1);

	pt->x = (cursorcolumns[col].unit - scroll.x) * font_size.cx;
	pt->y = (row - scroll.y) * font_size.cy;

	return true;
}

bool CPatternEditorInner::GetRangeRectScreen(int from_col, int from_row, int to_col, int to_row, RECT* rc) const {
	if (from_col == -1) return false;
	if (columns.size() == 0) return false;

	int left_col = std::min(from_col, to_col); // sorted
	int right_col = std::max(from_col, to_col); // sorted
	left_col = clamp(left_col, 0, columns.size() - 1);
	right_col = clamp(right_col, 0, columns.size() - 1);
	int right_units = columns[right_col]->editor->GetWidth();

	int top_row = std::min(from_row, to_row); // sorted
	int bottom_row = std::max(from_row, to_row); // sorted

	rc->left = columns[left_col]->unit * font_size.cx;
	rc->right = (columns[right_col]->unit + right_units) * font_size.cx;
	rc->top = top_row * font_size.cy;
	rc->bottom = (bottom_row * font_size.cy) + font_size.cy;
	return true;
}

// This is similar to RangeRectScreen, but it allows negative coordinates
bool CPatternEditorInner::GetDragtoRectScreen(RECT* rc) const {
	if (columns.size() == 0) return false;

	int left_col = drag_to.x;
	int right_col = drag_to.x + (select_end.x - select_begin.x);
	left_col = clamp(left_col, 0, columns.size() - 1);
	right_col = clamp(right_col, 0, columns.size() - 1);
	int right_units = columns[right_col]->editor->GetWidth();

	int top_row = drag_to.y;
	int bottom_row = (drag_to.y + (select_end.y - select_begin.y));
	top_row = clamp(top_row, 0, editor_rows - 1);
	bottom_row = clamp(bottom_row, 0, editor_rows - 1);

	rc->left = columns[left_col]->unit * font_size.cx;
	rc->right = (columns[right_col]->unit + right_units) * font_size.cx;
	rc->top = top_row * font_size.cy;
	rc->bottom = (bottom_row * font_size.cy) + font_size.cy;
	return true;
}

bool CPatternEditorInner::GetDragtoRectAbsolute(RECT* rc) const {
	int left_col = drag_to.x;
	int top_row = drag_to.y * skip;
	int right_col = drag_to.x + (select_end.x - select_begin.x);
	int bottom_row = (drag_to.y + (select_end.y - select_begin.y)) * skip;
	bottom_row += skip - 1; // includes the data you can't see

	rc->left = clamp(left_col, 0, columns.size() - 1);
	rc->right = clamp(right_col, 0, columns.size() - 1);
	rc->top = clamp(top_row, 0, pattern_rows - 1);
	rc->bottom = clamp(bottom_row, 0, pattern_rows - 1);
	return true;
}

bool CPatternEditorInner::GetDragtoPointAbsolute(POINT* pt) const {
	pt->x = drag_to.x;
	pt->y = drag_to.y * skip;
	return true;
}

bool CPatternEditorInner::GetSelectionRectScreen(RECT* rc) const {
	return GetRangeRectScreen(select_from.x, select_from.y, select_to.x, select_to.y, rc);
}

bool CPatternEditorInner::GetSelectionRect(RECT* rc) const {
	if (select_from.x == -1) return false;

	rc->left = select_begin.x;
	rc->top = select_begin.y;
	rc->right = select_end.x;
	rc->bottom = select_end.y;
	return true;
}

int CPatternEditorInner::CursorColumnToScreenUnits(int column) const {
	if (column < 0 || column >= cursorcolumns.size()) return -1;
	return cursorcolumns[column].unit;
}

// int CPatternEditorInner::ScreenUnitsToCursorColumn(int x) const {
// 	int nearest = -1;
// 	for (int i = 0; i < cursorcolumns.size(); ++i) {
// 		PE_column const& col = *columns[cursorcolumns[i].column];
// 
// 		int left_unit = col.unit + cursorcolumns[i].offset;
// 		int right_unit = left_unit + cursorcolumns[i].width;
// 
// 		if ((x >= left_unit) && (x < right_unit))
// 			return i;
// 
// 		if (x > col.unit)
// 			nearest = i;
// 	}
// 	return nearest;
// }

int CPatternEditorInner::ScreenUnitsToCursorColumn(int x) const {
	if (screenunits.size() == 0)
		return -1;

	std::map<int, PE_cursorcolumn*>::const_iterator i = screenunits.upper_bound(x);

	if (i != screenunits.begin())
		--i;

	return i->second->index;
}

// ---------------------------------------------------------------------------------------------------------------
// PREFERENCES
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorInner::SetThemeColor(PE_themeindex index, COLORREF color) {
	colors[index] = color;
}

void CPatternEditorInner::SetNoteOffStr(std::string const& s) {
	noteOffStr = s;
}

void CPatternEditorInner::SetNoteCutStr(std::string const& s) {
	noteCutStr = s;
}

void CPatternEditorInner::SetBGNote(std::string const& s) {
	bg_note = s;
	bg_note.resize(3, '.');
}

void CPatternEditorInner::SetBGByte(std::string const& s) {
	bg_byte = s;
	bg_byte.resize(2, '.');
}

void CPatternEditorInner::SetBGSwitch(std::string const& s) {
	bg_switch = s;
	bg_switch.resize(1, '.');
}

void CPatternEditorInner::SetBGWord(std::string const& s) {
	bg_word = s;
	bg_word.resize(4, '.');
}

void CPatternEditorInner::SetStickySelection(bool state) {
	sticky_selection = state;
}

void CPatternEditorInner::SetHorizontalScrollMode(bool state) {
	horizontal_scroll_mode = state;
}

void CPatternEditorInner::SetVerticalScrollMode(bool state) {
	vertical_scroll_mode = state;
}

void CPatternEditorInner::SetColoredNotes(bool state) {
	notecolors_enabled = state;
}

// ---------------------------------------------------------------------------------------------------------------
// EDIT STATE
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorInner::SetOctave(int n) {
	octave = n;
	for (std::vector<PE_column*>::iterator i = columns.begin(); i != columns.end(); ++i) {
		(*i)->editor->OctaveChange(n);
	}

}

void CPatternEditorInner::SetStep(int n) {
	step = n;
}

void CPatternEditorInner::SetHorizontalEntry(bool state) {
	horizontal_entry = state;
}

// ---------------------------------------------------------------------------------------------------------------
// MOUSE
// ---------------------------------------------------------------------------------------------------------------

// #define SHOWMOUSEMODE() { \
// 	std::string s; \
// 	switch (mouse_mode) { \
// 		case mouse_mode_none: s = "none"; break; \
// 		case mouse_mode_down: s = "down"; break; \
// 		case mouse_mode_drag: s = "drag"; break; \
// 		case mouse_mode_mark: s = "mark"; break; \
// 		case mouse_mode_control: s  = "control"; break; \
// 		default: s = "???"; break; \
// 	} \
// 	std::cerr << __FUNCTION__ << "(" << __LINE__ << ") " << "mouse_mode_" << s << "\n"; \
// }

CPoint CPatternEditorInner::PointToCursorPoint(CPoint pt, CPoint* offset) {
	clientDC->DPtoLP(&pt);

	POINT pt_cursor;

	pt_cursor.x = pt.x / font_size.cx;
	if (pt_cursor.x < 0)
		pt_cursor.x = 0;
	pt_cursor.x = ScreenUnitsToCursorColumn(pt_cursor.x);
	if (pt_cursor.x == -1)
		pt_cursor.x = cursorcolumns.size() - 1;

	pt_cursor.y = clamp(pt.y / font_size.cy, 0, editor_rows - 1);
	if (offset) {
		offset->x = pt.x % font_size.cx; // how many pixels into the digit
		offset->y = pt.y % font_size.cy; // how many pixels into the row
	}

	return pt_cursor;
}

LRESULT CPatternEditorInner::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (cursorcolumns.size() == 0) return 0;

	if (GetFocus() != m_hWnd) SetFocus();

	POINT old_cursor = cursor;

	POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
	POINT pt_cursor = PointToCursorPoint(pt);

	SetCursor(pt_cursor.x, pt_cursor.y);

	dragslider_column = cursorcolumns[cursor.x].column;
	PE_column& col = *columns[dragslider_column];

	bool marked = false;
	if (!col.editor->IsSelectable()) {
		ClearSelection();
		mouse_mode = mouse_mode_control;
		col.editor->LButtonDown(wParam, lParam);
		marked = true;
	} else
	if (IsCtrlDown() && IsShiftDown()) {
		int cx = cursorcolumns[cursor.x].column;
		int cy = cursor.y;
		SelectRange(cx, cy, cx, cy);
		marked = true;
		mouse_mode = mouse_mode_down;
		drag_mode = drag_mode_cell;
	} else
	if (IsCtrlDown()) {
		ClearSelection();
		col.editor->LButtonDown(wParam, lParam);
	} else
	if (IsShiftDown()) {
		RECT rcSel;
		bool hasSelection = GetSelectionRectScreen(&rcSel);
		if (!IsCursorInSel()) {
			if (!hasSelection || sticky_selection) {
				// make new selection
				SelectRange(cursorcolumns[old_cursor.x].column, old_cursor.y, cursorcolumns[cursor.x].column, cursor.y);
			} else { // extend existing selection
				int cx = cursorcolumns[cursor.x].column;
				int cy = cursor.y;

				if ((cx < select_begin.x) || (cy < select_begin.y)) {
					SelectRange(cx, cy, select_end.x, select_end.y);
				} else
				if ((cx > select_end.x) || (cy > select_end.y)) {
					SelectRange(select_begin.x, select_begin.y, cx, cy);
				}
			}
			marked = true;
			mouse_mode = mouse_mode_none;
		}
	}

	if (!marked) mouse_mode = mouse_mode_down;

	SetCapture();

	///UpdateCaret();
	ScrollToCursorPoint(cursor, scroll, false);

	GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_MOVECURSOR, 0), 0);

	return 0;
}

void CPatternEditorInner::ResetMouseMode() {
	mouse_mode = mouse_mode_none;
	drag_mode = drag_mode_none;
}

LRESULT CPatternEditorInner::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	if (cursorcolumns.size() == 0) return 0;
	if (GetCapture() != m_hWnd) return 0;

	ReleaseCapture();

	if (mouse_mode == mouse_mode_down) {
		ClearSelection();
	}

	if (double_clicking) {
		double_clicking = false;
		ResetMouseMode();
		return 0;
	}

	if (mouse_mode == mouse_mode_drag) {
		if (drag_mode == drag_mode_cell) {
			GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_SELDROPCELL, 0), 0);
		} else {
			GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_SELDROP, 0), 0);
		}
	}

	if (mouse_mode == mouse_mode_control) {
		ResetMouseMode();
		PE_column& col = *columns[cursorcolumns[cursor.x].column]; // cursor.x vs dragslider_column
		col.editor->LButtonUp(wParam, lParam);
	} else {
		ResetMouseMode();

		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		POINT pt_cursor = PointToCursorPoint(pt);

		if (pt_cursor.x != cursor.x || pt_cursor.y != cursor.y) {
			SetCursor(pt_cursor.x, pt_cursor.y);
			ScrollToCursorPoint(cursor, scroll, false);
			GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_MOVECURSOR, 0), 0);
		}
	}

	return 0;
}

void CPatternEditorInner::DoMouseMove() {
	POINT pt = mousemove_previous;
	mousemove_previous.x = -1;
	mousemove_previous.y = -1;
	UpdateClientOrg();
	SendMessage(WM_MOUSEMOVE, 0, MAKELONG(pt.x, pt.y));
}

LRESULT CPatternEditorInner::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam)
{
	POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };

	// workaround cause MouseMoves are sometimes sent after LButtonDowns which causes unwarranted mouse_mode_drag's
	if ((pt.x == mousemove_previous.x) && (pt.y == mousemove_previous.y)) return 0;
	mousemove_previous = pt;

	if (GetCapture() != m_hWnd) return 0;

	POINT pt_cursor = PointToCursorPoint(pt);

	POINT range_first = select_from;

	// check if we should change mouse_mode
	if (mouse_mode == mouse_mode_down) {
		if (IsCtrlDown() && (drag_mode != drag_mode_cell)) {
			// let the column control handle mouse movement
			mouse_mode = mouse_mode_control;
		} else
		if (IsCursorInSel()) {
			mouse_mode = mouse_mode_drag;
			if (drag_mode != drag_mode_cell) {
				drag_mode = drag_mode_selection;
			}

			// find col/row offset from start of selection *measured in cols/editor_rows*
			drag_offset.x = cursorcolumns[pt_cursor.x].column - select_begin.x;
			drag_offset.y = pt_cursor.y - select_begin.y;
		} else
		// make sure the mouse has at least changed units before we decide we're marking
		if ((pt_cursor.x != cursor.x) || (pt_cursor.y != cursor.y)) {
			mouse_mode = mouse_mode_mark;

			range_first.x = cursorcolumns[cursor.x].column;
			range_first.y = cursor.y;
		}
	}

	if (mouse_mode == mouse_mode_drag) {
		RECT rcPrevDrag;
		GetDragtoRectScreen(&rcPrevDrag);

		drag_to.x = cursorcolumns[pt_cursor.x].column - drag_offset.x;
		drag_to.y = pt_cursor.y - drag_offset.y;

		// invalidate drag rect
		RECT rcDrag;
		GetDragtoRectScreen(&rcDrag);
		UnionRect(&rcDrag, &rcDrag, &rcPrevDrag);
		clientDC->LPtoDP(&rcDrag);
		InvalidateRect(&rcDrag, FALSE);

		ScrollToCursorPoint(pt_cursor, scroll, false);

		GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_SELDRAG, 0), 0);
	} else
	if (mouse_mode == mouse_mode_mark) {
		SelectRange(range_first.x, range_first.y, cursorcolumns[pt_cursor.x].column, pt_cursor.y);
		ScrollToCursorPoint(pt_cursor, scroll, false);
	} else
	if (mouse_mode == mouse_mode_control) {
		PE_column& col = *columns[dragslider_column];
		col.editor->MouseMove(wParam, lParam);
	}

	return 0;
}

LRESULT CPatternEditorInner::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (mouse_mode == mouse_mode_down) {
		GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNVIEW_TRACKSWAP_SELECTION, 0), 0);
		return 0;
	} else
	if (mouse_mode == mouse_mode_mark) {
		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		POINT pt_cursor = PointToCursorPoint(pt);
		SetCursor(pt_cursor.x, pt_cursor.y);
		int cx = cursorcolumns[cursor.x].column;
		int cy = cursor.y;
		SelectRange(cx, cy, cx, cy);
		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_MOVECURSOR, 0), 0);
		return 0;
	} else
	if (mouse_mode == mouse_mode_drag) {
		GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_SELDROP, TRUE), 0); // TRUE is makecopy
		return 0;
	} else {
		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		ClientToScreen(&pt);
		GetParent().ScreenToClient(&pt);
		return GetParent().SendMessage(WM_RBUTTONDOWN, wParam, MAKELPARAM(pt.x, pt.y));
	}
}

LRESULT CPatternEditorInner::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (mouse_mode == mouse_mode_none)
		return DefWindowProc(); // DefWindowProc for WM_RBUTTONUP sends WM_CONTEXTMENU
	return 0;
}

LRESULT CPatternEditorInner::OnRButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	return OnRButtonDown(WM_RBUTTONDOWN, wParam, lParam);
}

LRESULT CPatternEditorInner::OnXButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	return GetParent().SendMessage(WM_XBUTTONDOWN, wParam, lParam);
}

LRESULT CPatternEditorInner::OnForward(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

// ---------------------------------------------------------------------------------------------------------------
// KEYBOARD
// ---------------------------------------------------------------------------------------------------------------

namespace {
inline bool IsLeftDown() {
	if (GetAsyncKeyState(VK_LEFT)<0) return true;
	return false;
}

inline bool IsRightDown() {
	if (GetAsyncKeyState(VK_RIGHT)<0) return true;
	return false;
}

inline bool IsUpDown() {
	if (GetAsyncKeyState(VK_UP)<0) return true;
	return false;
}

inline bool IsDownDown() {
	if (GetAsyncKeyState(VK_DOWN)<0) return true;
	return false;
}
}

void CPatternEditorInner::CalcKeyboardSpeed() {
	DWORD nKBDelay;
	DWORD nKBSpeed;
	SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &nKBDelay, 0);
	SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &nKBSpeed, 0);

	// calculations from atlctrlx.h
	kb_delay_ms = (nKBDelay + 1) * 250;
	kb_speed_ms = 10000 / (10 * nKBSpeed + 25);

	kb_fastdelay_ms = kb_delay_ms / 3;
}

LRESULT CPatternEditorInner::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	if (!timer_active || repeat_transition) {
		timer_active = true;
		repeat_transition = false;
		SetTimer(ID_PATTERNEDITORINNER_TIMER, kb_speed_ms);
	}

	if (IsLeftDown() && IsRightDown()) {
		// non
	} else
	if (IsUpDown() && IsDownDown()) {
		// non
	} else
	if (IsLeftDown() && IsUpDown()) {
		MoveUpLeft();
	} else
	if (IsLeftDown() && IsDownDown()) {
		MoveDownLeft();
	} else
	if (IsRightDown() && IsUpDown()) {
		MoveUpRight();
	} else
	if (IsRightDown() && IsDownDown()) {
		MoveDownRight();
	} else
	if (IsLeftDown()) {
		DoKeyboard(VK_LEFT);
	} else
	if (IsRightDown()) {
		DoKeyboard(VK_RIGHT);
	} else
	if (IsUpDown()) {
		DoKeyboard(VK_UP);
	} else
	if (IsDownDown()) {
		DoKeyboard(VK_DOWN);
	}

	return 0;
}

void CPatternEditorInner::DoKeyboard(int kc) {
	switch (kc) {
		case VK_LEFT:
			if (IsCtrlDown()) MoveLeftCol(); else MoveLeft();
			break;
		case VK_RIGHT:
			if (IsCtrlDown()) MoveRightCol(); else MoveRight();
			break;
		case VK_UP:
			if (!IsCtrlDown()) MoveUp();
			break;
		case VK_DOWN:
			if (!IsCtrlDown()) MoveDown();
			break;
	}
}

LRESULT CPatternEditorInner::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (cursorcolumns.size() == 0) return 0;

	bool first_press = !(lParam & (0x1 << 30)); // 30th bit is previous state

	bool handled = true;

	switch (wParam) {
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			if (first_press) {
				DoKeyboard(wParam);
				repeat_transition = true;
				int time_ms = timer_active ? kb_fastdelay_ms : kb_delay_ms;
				SetTimer(ID_PATTERNEDITORINNER_TIMER, time_ms);
			}
			break;
		case VK_PRIOR:
			MovePgUp();
			break;
		case VK_NEXT:
			MovePgDn();
			break;
		case VK_TAB:
			if (IsShiftDown()) MovePrevTrack(); else MoveNextTrack();
			break;
		case VK_HOME:
			if (IsCtrlDown()) MoveBeginLoop(); else MoveBol();
			break;
		case VK_END:
			if (IsCtrlDown()) MoveEndLoop(); else MoveEol();
			break;
		case VK_SHIFT:
			{	PE_column const& col = GetColumnAtCursor();
				PE_cursorcolumn const& ccol = GetCursorColumnAtCursor();
				if ((col.type == pattern_column_type_note) && (ccol.digit == 0)) {
					entering_chord = true;
					entering_chord_origin = CPoint(cursor.x, cursor.y);
				} else {
					handled = false;
				}
			}
			break;
		default:
			handled = false;
			break;
	}

// 	if (!handled)
// 		return GetParent().SendMessage(uMsg, wParam, lParam);

	if (!handled) {
		if (cursorcolumns.size() == 0) return 0;

		PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
		PE_column& col = *columns[ccol.column];

		if (!col.editor->KeyDown(wParam, lParam, first_press))
			return -1;

		return 0;
	}

	return 0;
}

LRESULT CPatternEditorInner::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (cursorcolumns.size() == 0) return 0;

	bool handled = true;

	switch (wParam) {
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			if (!(IsLeftDown() || IsRightDown() || IsUpDown() || IsDownDown())) {
				KillTimer(ID_PATTERNEDITORINNER_TIMER);
				timer_active = false;
				repeat_transition = false;
			} else {
				repeat_transition = true;
				SetTimer(ID_PATTERNEDITORINNER_TIMER, kb_fastdelay_ms);
			}
			break;
		case VK_SHIFT:
			if (entering_chord) {
				if (did_chord_step) {
					if (entering_chord_origin.y == cursor.y) {
						SetCursorAdjust(entering_chord_origin.x, cursor.y);
						StepCursor();
					}
				}

				entering_chord = false;
				entering_chord_origin = CPoint(-1, -1);
				did_chord_step = false;
			} else {
				handled = false;
			}
			break;
		default:
			handled = false;
			break;
	}

// 	if (!handled)
// 		return GetParent().SendMessage(uMsg, wParam, lParam);
//
//	DefWindowProc();

	if (!handled) {
		if (cursorcolumns.size() == 0) return 0;

		PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
		PE_column& col = *columns[ccol.column];

		if (!col.editor->KeyUp(wParam, lParam))
			return -1;

		return 0;
	}

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// CURSOR
// ---------------------------------------------------------------------------------------------------------------

bool CPatternEditorInner::ScrollToCursorPointHorizontal(CPoint pt, CPoint old_scroll) {
	return false;
}

bool CPatternEditorInner::ScrollToCursorPoint(CPoint pt, CPoint old_scroll, bool allow_hcenter) {
	if (editor_rows == 0) return false;

	CPoint new_scroll = GetScrollToCursorPoint(pt, old_scroll, allow_hcenter);

	if (new_scroll != scroll)
		return ScrollTo(new_scroll);
	else
		return false;
}

CPoint CPatternEditorInner::GetScrollToCursorPoint(CPoint pt, CPoint old_scroll, bool allow_hcenter) {
	CSize screen = GetScreenSize();
	screen.cx -= 2;
	screen.cy -= 1;

	POINT new_scroll = old_scroll;

	// scroll to column
	{
		int cursor_unit_x = CursorColumnToScreenUnits(pt.x);
		int scroll_check_x;

		PE_column const& col = *columns[cursorcolumns[pt.x].column];

		// ensure entire column is visible
		if (col.unit < old_scroll.x) {
			scroll_check_x = col.unit;
		} else
		if (cursor_unit_x > old_scroll.x) {	
			scroll_check_x = col.unit + (col.editor->GetWidth() - 1);
		} else {
			scroll_check_x = cursor_unit_x;
		}

		// only scroll if we have to
		if (scroll_check_x < old_scroll.x) {
			if (horizontal_scroll_mode && allow_hcenter) {
				CPoint centered_scroll(scroll_check_x, old_scroll.y);
				new_scroll.x = GetScrollToCursorPointCentered(pt, centered_scroll).x;
			} else {
				new_scroll.x = scroll_check_x;
			}
		} else
		if (scroll_check_x > (old_scroll.x + screen.cx)) {
			if (horizontal_scroll_mode && allow_hcenter) {
				CPoint centered_scroll(scroll_check_x - screen.cx, old_scroll.y);
				new_scroll.x = GetScrollToCursorPointCentered(pt, centered_scroll).x;
			} else {
				new_scroll.x = scroll_check_x - screen.cx;
			}
		}
	}

	// scroll to row
	{
		pt.y = clamp(pt.y, 0, editor_rows - 1);

		if ((pt.y - new_scroll.y) < 0)
			new_scroll.y = pt.y;
		if ((pt.y - new_scroll.y) >= screen.cy)
			new_scroll.y = pt.y - screen.cy;
	}

	return new_scroll;
}

bool CPatternEditorInner::ScrollToCursorPointCenteredHorizontal(CPoint pt, CPoint old_scroll) {
	if (editor_rows == 0) return false;

	CPoint new_scroll = GetScrollToCursorPointCentered(pt, old_scroll);
	new_scroll.y = old_scroll.y;

	if (new_scroll != scroll)
		return ScrollTo(new_scroll);
	else
		return false;
}

bool CPatternEditorInner::ScrollToCursorPointCentered(CPoint pt, CPoint old_scroll) {
	if (editor_rows == 0) return false;

	CPoint new_scroll = GetScrollToCursorPointCentered(pt, old_scroll);

	if (new_scroll != scroll)
		return ScrollTo(new_scroll);
	else
		return false;
}

CPoint CPatternEditorInner::GetScrollToCursorPointCentered(CPoint pt, CPoint old_scroll) {
	CPoint new_scroll;

	CSize screen = GetScreenSize();
	CPoint max_scroll = GetMaxScroll();

	// scroll to column
	{	PE_column const& col = *columns[cursorcolumns[pt.x].column];
		new_scroll.x = col.unit - (screen.cx / 2);
		new_scroll.x = clamp(new_scroll.x, 0, max_scroll.x);
	}

	// scroll to row
	{	pt.y = clamp(pt.y, 0, editor_rows - 1);
		new_scroll.y = pt.y - (screen.cy / 2);
		new_scroll.y = clamp(new_scroll.y, 0, max_scroll.y);
	}

	return new_scroll;
}

bool CPatternEditorInner::ScrollVertical(int y_offset, CPoint old_scroll) {
	CSize screen = GetScreenSize();

	int new_scroll_y = old_scroll.y + y_offset;
	if ((editor_rows <= screen.cy) || (new_scroll_y < 0))
		new_scroll_y = 0;
	else if (new_scroll_y >= (editor_rows - screen.cy))
		new_scroll_y = editor_rows - screen.cy;

	CPoint ptTo(old_scroll.x, new_scroll_y);
	return ScrollTo(ptTo);
}

bool CPatternEditorInner::ScrollVerticalAndMove(CPoint pt, int y_offset, CPoint old_scroll) {
	CSize screen = GetScreenSize();

	int new_scroll_y = old_scroll.y + y_offset;
	if ((editor_rows <= screen.cy) || (new_scroll_y < 0))
		new_scroll_y = 0;
	else if (new_scroll_y >= (editor_rows - screen.cy))
		new_scroll_y = editor_rows - screen.cy;

	CPoint new_scroll(old_scroll.x, new_scroll_y);
	new_scroll = GetScrollToCursorPoint(pt, new_scroll, true);
	return ScrollTo(new_scroll);
}

// ---

bool CPatternEditorInner::IsCursorPointOffscreen(CPoint pt) const {
	if (cursorcolumns.size() == 0) return false;

	CSize screen = GetScreenSize();
	int cursor_x_unit = cursorcolumns[pt.x].unit;
	return (false
		|| (cursor_x_unit < scroll.x)
		|| (cursor_x_unit > (scroll.x + screen.cx))
		|| (pt.y < scroll.y)
		|| (pt.y > (scroll.y + screen.cy))
	);
}

CPoint CPatternEditorInner::GetCursor() const {
	return cursor;
}

// things that call this are doing the job of sending the movecursor msg to parent
void CPatternEditorInner::SetCursor(int x, int y) {
	if (cursorcolumns.size() == 0) return;

	CPoint last_cursor = cursor;

	cursor.x = clamp(x, 0, cursorcolumns.size() - 1);
	cursor.y = clamp(y, 0, editor_rows - 1);

	if (last_cursor != cursor)
		UpdateCaret();

	cursor_offscreen = IsCursorPointOffscreen(cursor);
}

void CPatternEditorInner::SetCursorAdjust(int x, int y) {
	AdjustCursor(x, y, false, recenter_mode_also, scroll, true);
}

void CPatternEditorInner::MoveCursor(int x, int y, bool affectsel, int recenter_mode) {
	AdjustCursor(x, y, affectsel, recenter_mode, scroll, false);
}

void CPatternEditorInner::AdjustCursor(int x, int y, bool affectsel, int recenter_mode, CPoint dest_scroll, bool set_mode) {
	if (cursorcolumns.size() == 0) return;
	if (!set_mode && x == 0 && y == 0) return;

	// change arrow key motion to note column stepping if we're in chord mode
	if (did_chord_step && !set_mode) {
		if (x != 0) {
			bool direction = x < 0 ? false : true;
			StepCursorToNoteColumn(direction);
			return;
		}
		affectsel = false;
	}

	// recenter view on cursor if it's offscreen
	if (recenter_mode == recenter_mode_only || recenter_mode == recenter_mode_also) {
		if (IsCursorPointOffscreen(cursor)) {
			if (recenter_mode == recenter_mode_only) {
				ScrollToCursorPointCentered(cursor, dest_scroll);
				return;
			} else {
				dest_scroll = GetScrollToCursorPointCentered(cursor, dest_scroll);
			}
		}
	}

	CPoint old_cursor = cursor;

	if (set_mode == false) {
		cursor.x += x;
		cursor.y += y;
	} else {
		cursor.x = x;
		cursor.y = y;
	}
	cursor.x = clamp(cursor.x, 0, (int)cursorcolumns.size() - 1);
	cursor.y = clamp(cursor.y, 0, editor_rows - 1);

	bool did_sel = false;

	if (affectsel) {
		if (IsShiftDown()) {
			if (sticky_selection) {
				if (false
					|| cursorcolumns[old_cursor.x].column < select_begin.x
					|| cursorcolumns[old_cursor.x].column > select_end.x
					|| old_cursor.y < select_begin.y
					|| old_cursor.y > select_end.y
				) {
					Unselect();
					did_sel = true;
				}
			}

			if (select_from.x == -1) { // begin selection
				SelectRange(cursorcolumns[old_cursor.x].column, old_cursor.y, cursorcolumns[cursor.x].column, cursor.y);
				did_sel = true;
			} else { // extend selection
				SelectRange(select_from.x, select_from.y, cursorcolumns[cursor.x].column, cursor.y);
				did_sel = true;
			}
		}
	}

	if (!did_sel) {
		ClearSelection();
	}

	// scrolling
	{	bool scrolled = false;

		if (vertical_scroll_mode == 1) {
			CSize screen = GetScreenSize();

			bool do_offset = true;

			int top_chop = screen.cy / 2;
			int bot_chop = editor_rows - (screen.cy / 2);

			if ((cursor.y < top_chop) && (scroll.y == 0))
				do_offset = false;
			if ((cursor.y > bot_chop) && (scroll.y >= (editor_rows - screen.cy)))
				do_offset = false;

			if (do_offset && !set_mode) {
				scrolled = ScrollVerticalAndMove(cursor, y, dest_scroll);
			} else {
				scrolled = ScrollToCursorPoint(cursor, dest_scroll, true);
			}
		} else {
			scrolled = ScrollToCursorPoint(cursor, dest_scroll, true);
		}

		if (!scrolled && (old_cursor != cursor))
			UpdateCaret();
	}

	GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_MOVECURSOR, 0), 0);
}

// ---

void CPatternEditorInner::MoveLeft() {
	MoveCursor(-1, 0, true, recenter_mode_only);
}

void CPatternEditorInner::MoveUpLeft() {
	MoveCursor(-1, -1, true, recenter_mode_only);
}

void CPatternEditorInner::MoveUp() {
	MoveCursor(0, -1, true, recenter_mode_only);
}

void CPatternEditorInner::MoveUpRight() {
	MoveCursor(1, -1, true, recenter_mode_only);
}

void CPatternEditorInner::MoveRight() {
	MoveCursor(1, 0, true, recenter_mode_only);
}

void CPatternEditorInner::MoveDownRight() {
	MoveCursor(1, 1, true, recenter_mode_only);
}

void CPatternEditorInner::MoveDown() {
	MoveCursor(0, 1, true, recenter_mode_only);
}

void CPatternEditorInner::MoveDownLeft() {
	MoveCursor(-1, 1, true, recenter_mode_only);
}

// ---

void CPatternEditorInner::MovePgUp() {
	MoveCursor(0, -16, true, recenter_mode_only);
}

void CPatternEditorInner::MovePgDn() {
	MoveCursor(0, 16, true, recenter_mode_only);
}

void CPatternEditorInner::MoveBeginLoop() {
	MoveCursor(0, -cursor.y, true, recenter_mode_only);
}

void CPatternEditorInner::MoveEndLoop() {
	MoveCursor(0, editor_rows - cursor.y - 1, true, recenter_mode_only);
}

void CPatternEditorInner::MoveBol() {
	MoveCursor(-cursor.x, 0, true, recenter_mode_only);
}

void CPatternEditorInner::MoveEol() {
	MoveCursor(cursorcolumns.size() - cursor.x - 1, 0, true, recenter_mode_only);
}

// ---

void CPatternEditorInner::MoveNextTrack() {
	if (cursorcolumns.size() == 0) return;

	int first_track_cursor = -1;
	int next_track_cursor = -1;
	for (int i = 0; i < tracks.size() - 1; ++i) {
		int track_cursor = ScreenUnitsToCursorColumn(tracks[i + 1].unit);
		if (first_track_cursor == -1) {
			if (track_cursor > cursor.x) {
				first_track_cursor = track_cursor;
			}
		} else {
			next_track_cursor = track_cursor - 1;
			break;
		}
	}
	if (first_track_cursor == -1) return; // didnt move
	if (next_track_cursor == -1) next_track_cursor = cursorcolumns.size() - 1;

	POINT pt = { next_track_cursor, cursor.y };
	POINT new_scroll = GetScrollToCursorPoint(pt, scroll, false);
	AdjustCursor(first_track_cursor - cursor.x, 0, true, recenter_mode_none, new_scroll, false);
}

void CPatternEditorInner::MovePrevTrack() {
	if (cursorcolumns.size() == 0) return;
	for (int i = tracks.size(); i > 0; --i) {
		int next_track_cursor = ScreenUnitsToCursorColumn(tracks[i - 1].unit);
		if (next_track_cursor < cursor.x) {
			MoveCursor(next_track_cursor - cursor.x, 0, false, recenter_mode_only);
			break;
		}
	}
}

void CPatternEditorInner::MoveRightCol() {
	if (cursorcolumns.size() == 0) return ;
	PE_cursorcolumn const& ccol = cursorcolumns[cursor.x];
	int nextcol = ccol.column + 1;
	if (nextcol >= columns.size()) return ;
	int nextunit = columns[nextcol]->unit;
	int nextx = ScreenUnitsToCursorColumn(nextunit);
	MoveCursor(nextx - cursor.x, 0, true, recenter_mode_only);
}

void CPatternEditorInner::MoveLeftCol() {
	if (cursorcolumns.size() == 0) return ;
	PE_cursorcolumn const& ccol = cursorcolumns[cursor.x];
	int nextcol = ccol.column - 1;
	if (nextcol < 0) return ;
	int nextunit = columns[nextcol]->unit;
	int nextx = ScreenUnitsToCursorColumn(nextunit);
	MoveCursor(nextx - cursor.x, 0, true, recenter_mode_only);
}

// ---

void CPatternEditorInner::StepCursor() {
	MoveCursor(0, step, false, recenter_mode_also);
}

void CPatternEditorInner::StepCursorSmart() {
	int col = cursorcolumns[cursor.x].column;
	int digit = cursorcolumns[cursor.x].digit;
	int positions = columns[col]->editor->GetDigits();
	if (digit < positions-1) {
		MoveCursor(1, 0, false, recenter_mode_also);
	} else {
		MoveCursor(1 - positions, step, false, recenter_mode_also);
	}
}

void CPatternEditorInner::StepCursorReset() {
	int digit = cursorcolumns[cursor.x].digit;
	MoveCursor(-digit, 0, false, recenter_mode_also);
}

void CPatternEditorInner::StepCursorToNoteColumn(bool direction) {
	int got_note_col = -1;
	int cursor_idx = GetColumnAtCursor().index;
	if (direction) ++cursor_idx; else --cursor_idx;
	for (int i = cursor_idx; i < columns.size();) {
		PE_column const& col = *columns[i];
		if (col.type == pattern_column_type_note) {
			got_note_col = col.index;
			break;
		}
		if (direction) ++i; else --i;
	}

	if (got_note_col != -1) {
		PE_column const& col = *columns[got_note_col];
		int cursor_idx = ScreenUnitsToCursorColumn(col.unit);
		SetCursorAdjust(cursor_idx, cursor.y);
	}

	did_chord_step = true;
}

// ---------------------------------------------------------------------------------------------------------------
// SELECTION
// ---------------------------------------------------------------------------------------------------------------

bool CPatternEditorInner::HasSelection() const {
	return select_from.x != -1;
}

void CPatternEditorInner::ClearSelection() {
	if (sticky_selection == false)
		if (HasSelection())
			SelectRange(-1, -1, -1, -1);
}

void CPatternEditorInner::Unselect() {
	if (HasSelection())
		SelectRange(-1, -1, -1, -1);
}

void CPatternEditorInner::Reselect() {
	SelectRange(select_from.x, select_from.y, select_to.x, select_to.y, true);
}

void CPatternEditorInner::InitSelection() {
	select_from.x = select_from.y = select_to.x = select_to.y = -1;
	select_begin.x = select_begin.y = select_end.x = select_end.y = -1;
}

void CPatternEditorInner::SelectRange(int from_col, int from_row, int to_col, int to_row, bool forceinvalid /*= false*/) {
	RECT rcPrevSel;
	if (!GetSelectionRectScreen(&rcPrevSel))
		rcPrevSel.left = -1;

	CPoint prev_select_begin = select_begin;
	CPoint prev_select_end = select_end;

	if (from_col != -1) {
		select_from.x = clamp(from_col, 0, columns.size() - 1);
		select_from.y = clamp(from_row, 0, editor_rows - 1);
		select_to.x = clamp(to_col, 0, columns.size() - 1);
		select_to.y = clamp(to_row, 0, editor_rows - 1);
		SortSelection();
	} else {
		InitSelection();
	}

	if (forceinvalid || (select_begin != prev_select_begin) || (select_end != prev_select_end)) {
		SetEventHoles();
		InvalidateSelection(&rcPrevSel);
	}
}

void CPatternEditorInner::InvalidateSelection(RECT* rcPrevSel) {
	RECT rcSel;
	if (!GetSelectionRectScreen(&rcSel))
		rcSel.left = -1;

	if (rcPrevSel == 0)
		rcPrevSel = &rcSel;

	RECT rcInvalid;
	if ((rcSel.left != -1) && (rcPrevSel->left != -1)) {
		UnionRect(&rcInvalid, &rcSel, rcPrevSel);
	} else
	if (rcSel.left == -1) {
		rcInvalid = *rcPrevSel;
	} else {
		rcInvalid = rcSel;
	}

	if (rcInvalid.left != -1) {
		clientDC->LPtoDP(&rcInvalid);
		InvalidateRect(&rcInvalid, FALSE);

		GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORINNER_SELCHANGED, 0), 0);
	}
}

void CPatternEditorInner::SelectRangeAbsolute(int from_col, int from_row, int to_col, int to_row) {
	SelectRange(from_col, from_row / skip, to_col, to_row / skip);
}

void CPatternEditorInner::SortSelection() {
	select_begin.x = std::min(select_from.x, select_to.x);
	select_begin.y = std::min(select_from.y, select_to.y);
	select_end.x = std::max(select_from.x, select_to.x);
	select_end.y = std::max(select_from.y, select_to.y);
}

bool CPatternEditorInner::IsCursorInSel() const {
	int ccol_col = cursorcolumns[cursor.x].column;
	return (HasSelection()
		&& (ccol_col >= select_begin.x)
		&& (cursor.y >= select_begin.y)
		&& (ccol_col <= select_end.x)
		&& (cursor.y <= select_end.y)
	);
}

// ---------------------------------------------------------------------------------------------------------------
// CARET
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorInner::AllocateCaretBitmap(int width, int height) {
	if ((width != caret_bitmap_size.cx) || (height != caret_bitmap_size.cy)) {
		if (caret_bitmap.m_hBitmap) caret_bitmap.DeleteObject();
		caret_bitmap.CreateCompatibleBitmap(clientDC->m_hDC, width, height);
		caret_bitmap_size = CSize(width, height);
	}

	CDC dcMem;
	dcMem.CreateCompatibleDC(clientDC->m_hDC);
	dcMem.SelectBitmap(caret_bitmap);

	// set the caret bitmap to the same color as the background
	// so that when it XOR's with the background it becomes black
	PE_column const& col = *columns[cursorcolumns[cursor.x].column];
	COLORREF caretcolor = col.editor->GetBkColor(cursor.y * skip);
	if (colors[PE_Cursor]) // allow caret to work on black backgrounds
		caretcolor = colors[PE_Cursor] & ~caretcolor;

	dcMem.FillSolidRect(0, 0, width, height, caretcolor);
}

void CPatternEditorInner::InvalidateCaret() {
// 	dirtyCaret = true;
// 
//  	RECT rcInvalid = { 0, 0, 1, 1 };
//  	InvalidateRect(&rcInvalid, FALSE);

///	::RedrawWindow(m_hWnd, NULL, NULL, RDW_INTERNALPAINT);
}

void CPatternEditorInner::UpdateCaret() {
	if (GetFocus() != m_hWnd)
		return;

	if ((cursorcolumns.size() == 0) || (editor_rows == 0)) {
		DestroyCaret();
		return;
	}

	PE_column const& col = *columns[cursorcolumns[cursor.x].column];

	int caret_width = font_size.cx * cursorcolumns[cursor.x].width;
	int caret_height = font_size.cy;
	int x = CursorColumnToScreenUnits(cursor.x) * font_size.cx;
	int y = cursor.y * font_size.cy;
	int caret_x = x - (scroll.x * font_size.cx);
	int caret_y = y - (scroll.y * font_size.cy);

	DestroyCaret();
	AllocateCaretBitmap(caret_width, caret_height);
	CreateCaret(caret_bitmap);
	SetCaretPos(caret_x, caret_y);
	ShowCaret();
}

LRESULT CPatternEditorInner::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	UpdateCaret();
	return 0;
}

LRESULT CPatternEditorInner::OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	DestroyCaret();
	return 0;
}

LRESULT CPatternEditorInner::OnMouseActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	if (GetFocus() == m_hWnd)
		return MA_ACTIVATE;
	else
		return DefWindowProc();
}

// ---------------------------------------------------------------------------------------------------------------
// HELD NOTES
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorInner::HoldNote(WPARAM wParam, int v) {
	PE_column const& col = GetColumnAtCursor();

	for (heldnotes_t::iterator i = heldnotes.begin(); i != heldnotes.end();) {
		PE_note_on const& note_on = i->second;

		if (true
			&& note_on.plugin_id == col.plugin_id
			&& note_on.group == col.group
			&& note_on.track == col.track
			&& note_on.column == col.column
		) {
			i = heldnotes.erase(i);
		} else {
			++i;
		}
	}

	heldnotes[wParam] = PE_note_on(col.plugin_id, col.group, col.track, col.column, v);
}

void CPatternEditorInner::ReleaseNote(WPARAM wParam) {
	heldnotes_t::iterator i = heldnotes.find(wParam);

	if (i != heldnotes.end()) {
		PE_note_on const& note_on = i->second;

		PE_NMHDR_note nmh;
		nmh.code =      ID_PATTERNEDITORINNER_NOTE;
		nmh.idFrom =    GetParent().GetDlgCtrlID();
		nmh.hwndFrom =  m_hWnd;
		nmh.plugin_id = note_on.plugin_id;
		nmh.group =     note_on.group;
		nmh.track =     note_on.track;
		nmh.column =    note_on.column;
		nmh.value =     255; // send a note off
		GetParent().SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmh);

		heldnotes.erase(i);
	}
}

void CPatternEditorInner::HoldAllNotes() {
	all_notes_held = true;
}

void CPatternEditorInner::ReleaseAllNotes() {
	if (all_notes_held) {
		for (int i = 0; i < columns.size(); ++i) {
			PE_column const& col = *columns[i];
			if (col.type == pattern_column_type_note) {
				PE_NMHDR_note nmh;
				nmh.code =      ID_PATTERNEDITORINNER_NOTE;
				nmh.idFrom =    GetParent().GetDlgCtrlID();
				nmh.hwndFrom =  m_hWnd;
				nmh.plugin_id = col.plugin_id;
				nmh.group =     col.group;
				nmh.track =     col.track;
				nmh.column =    col.column;
				nmh.value =     255; // send a note off
				GetParent().SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmh);
			}
		}

		all_notes_held = false;
	}
}

// ---------------------------------------------------------------------------------------------------------------
// VALUES
// ---------------------------------------------------------------------------------------------------------------

///clearvalue is no longer in use.
void CPatternEditorInner::ClearValue() {
	if (cursorcolumns.size() == 0) return;

	PE_cursorcolumn const& ccol = cursorcolumns[cursor.x];
	PE_column const& col = *columns[ccol.column];

	NotifyEdit(col.plugin_id, col.group, col.track, col.column, cursor.y * skip, ccol.digit, col.novalue, 0, true);
	StepCursor();
}

void CPatternEditorInner::NotifyEdit(int plugin_id, int group, int track, int column, int row, int digit, int value, int meta, bool reactive, int id /*= -1*/) {
	PE_NMHDR nmh;
	nmh.code      = ID_PATTERNEDITORINNER_EDIT; // Message type defined by control.
	nmh.idFrom    = GetParent().GetDlgCtrlID();	
	nmh.hwndFrom  = m_hWnd;
	nmh.plugin_id = plugin_id;
	nmh.group     = group;
	nmh.track     = track;
	nmh.column    = column;
	nmh.row       = row;
	nmh.digit     = digit;
	nmh.value     = value;
	nmh.reactive  = reactive;
	nmh.id        = id;
	nmh.meta      = meta;
	GetParent().SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmh);
}

void CPatternEditorInner::NotifyPianoTranslate(std::vector<int>& events, int timeshift, int pitchshift, int mode) {
	PE_NMHDR_piano nmh;
	nmh.code      = ID_PATTERNEDITORINNER_PIANO_TRANSLATE; // Message type defined by control.
	nmh.idFrom    = GetParent().GetDlgCtrlID();	
	nmh.hwndFrom  = m_hWnd;
	nmh.eventids  = events;
	nmh.timeshift = timeshift;
	nmh.pitchshift = pitchshift;
	nmh.mode = mode;
	GetParent().SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmh);
}

void CPatternEditorInner::NotifyPianoEdit(int id, int pluginid, int time, int note, int length) {
	PE_NMHDR_pianoedit nmh;
	nmh.code      = ID_PATTERNEDITORINNER_PIANO_EDIT; // Message type defined by control.
	nmh.idFrom    = GetParent().GetDlgCtrlID();	
	nmh.hwndFrom  = m_hWnd;
	nmh.id        = id;
	nmh.pluginid  = pluginid;
	nmh.time      = time;
	nmh.note      = note;
	nmh.length    = length;
	GetParent().SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)&nmh);
}

// ---------------------------------------------------------------------------------------------------------------

int PE_column::GetValue(int time, int* value, int* meta) const {
	PE_values_by_time::const_iterator i = values_by_time.find(time);

	if (i != values_by_time.end()) {
		*value = i->value;
		*meta = i->meta;
		return i->id;
	} else {
		*value = novalue;
		*meta = 0;
		return -1;
	}
}

int PE_column::GetPatternLength(int value) const {
	if ((value >= 0) && (value < editor->owner->patterninfos.size()))
		return editor->owner->patterninfos[value].length;
	return 0;
}

int PE_column::GetPatternValue(int time, int* value, int* length) const {

	PE_values_by_time::const_iterator i = values_by_time.lower_bound(time);
 	if (values_by_time.size() != 0) {

		// lower_bound is past current position, try previous pattern
		if (i == values_by_time.end())
			--i;
		else if (time < i->time)
			--i;

		int len = GetPatternLength(i->value);

		if (time >= i->time && time < i->time + len) {
			*value = i->value;
			*length = len;
			return i->id;
		} else {
			*value = novalue;
			*length = 0;
			return 0;
		}
	} else {
		*value = novalue;
		*length = 0;
		return 0;
	}
}

void PE_column::SetValueInitial(int id, int time, int value, int meta) {
	values.insert(PE_value(id, time, value, meta));
	///values_by_time.insert(values_by_time.end(), PE_value(id, time, value, meta));
}

void PE_column::SetValue(int id, int time, int value, int meta) {
	if (value != novalue) {
		editor->PreInsertValue(time, value);

		PE_values_by_id::const_iterator i = values_by_id.find(id);
		if (i != values_by_id.end()) {
			int prev_time = i->time;
			int prev_value = i->value;
			editor->PreDeleteValue(prev_time, prev_value);
			values_by_id.replace(i, PE_value(id, time, value, meta));
			editor->PostDeleteValue(prev_time, prev_value);
		} else {
			values.insert(PE_value(id, time, value, meta));
		}

		editor->PostInsertValue(time, value);
	} else {
		PE_values_by_id::iterator i = values_by_id.find(id);
		if (i == values_by_id.end()) return;

		int prev_time = i->time;
		int prev_value = i->value;

		editor->PreDeleteValue(prev_time, prev_value);
		values_by_id.erase(i);
		editor->PostDeleteValue(prev_time, prev_value);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// BINDING
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorInner::SetPatternRows(int n) {
	pattern_rows = n;
	editor_rows = ((n - 1) / skip) + 1;
}

void CPatternEditorInner::SetSkip(int n) {
	skip = n;
	SetPatternRows(pattern_rows);
}

void CPatternEditorInner::SetResolution(int n) {
	resolution = n;
}

void CPatternEditorInner::SetHighlightRows(int verydark_row, int dark_row) { 
	this->verydark_row = verydark_row; 
	this->dark_row = dark_row;
}

void CPatternEditorInner::AllocatePatternVertical(bool alloc_patimg /*= true*/) {
	UpdateClientExt();
	if (alloc_patimg) {
		scroller.AllocatePatternImg(editor_units, editor_rows);
		scroller.SetPlayPos(last_play_pos == -1 ? -1 : last_play_pos / skip);
	}
}

void CPatternEditorInner::AllocatePattern(bool alloc_patimg /*= true*/) {
	UpdateColumnUnits();
	UpdateClientExt();
	if (alloc_patimg) {
		scroller.AllocatePatternImg(editor_units, editor_rows);
		scroller.SetPlayPos(last_play_pos == -1 ? -1 : last_play_pos / skip);
	}
}

void CPatternEditorInner::BindPatternImg() {
	RenderPreview(true);
}

void CPatternEditorInner::UpdateColumnUnits() {
	int x = 0;
	int track_idx = 0;
	int last_track_idx = -1;
	int track_x = x;
	cursorcolumns.clear();
	screenunits.clear();

	for (int i = 0; i < columns.size(); ++i) {
		PE_column& col = *columns[i];
		PE_track& pe_track = tracks[track_idx];

		int columnunits = col.editor->CreateCursorColumns(cursorcolumns.size(), x);

		col.unit = x;
		col.track_index = track_idx;

		if (track_idx != last_track_idx)
			pe_track.first_column_idx = i;
		pe_track.last_column_idx = i;

		bool last_column = (i == (columns.size() - 1));
		bool no_spacer = last_column || col.is_collapsed;

		x += columnunits + (no_spacer ? 0 : 1);

		if (col.flagtype == pattern_column_flagtype_volume)
			pe_track.vol_idx = i;
		else
		if (col.flagtype == pattern_column_flagtype_wave)
			pe_track.wave_idx = i;

		last_track_idx = track_idx;

		// update track unit
		if (col.spacer || last_column) {
			pe_track.unit = track_x;
			pe_track.width = (x - track_x) + (last_column ? 1 : 0);
			++track_idx;
			track_x = x;
		}
	}

	editor_units = x;

	for (int i = 0; i < cursorcolumns.size(); ++i) {
		PE_cursorcolumn& ccol = cursorcolumns[i];
		screenunits[ccol.unit] = &ccol;
	}
}

void CPatternEditorInner::UpdateClientExt() {
	int width = editor_units * font_size.cx;
	int height = editor_rows * font_size.cy;
	clientDC->SetWindowExt(width, height);
}

void CPatternEditorInner::UpdateClientOrg() {
	clientDC->SetWindowOrg(scroll.x * font_size.cx, scroll.y * font_size.cy);
}

void CPatternEditorInner::AddTrack(std::string const& name, int plugin_id, int group, int track, bool is_muted) {
	if (columns.size() > 0) {
		PE_column& last_col = *columns[columns.size() - 1];
		last_col.spacer = true;
	}

	PE_track ti = { name, -1, -1, plugin_id, group, track, is_muted, -1, -1, -1, -1 };
	tracks.push_back(ti);
}

void CPatternEditorInner::AddColumn(
	int plugin_id, int group, int track, int column, int type, int novalue, int minvalue, int maxvalue, int control, int defaultcontrol, int flagtype, int is_collapsed, std::string paramname
) {
	PE_column* col = new PE_column();
	col->plugin_id = plugin_id;
	col->group = group;
	col->track = track;
	col->column = column;
	col->type = type;
	col->novalue = novalue;
	col->minvalue = minvalue;
	col->maxvalue = maxvalue;
	col->unit = -1;
	col->track_index = -1;
	col->spacer = false;
	col->control = control;
	col->is_collapsed = is_collapsed;
	col->defaultcontrol = defaultcontrol;
	col->flagtype = flagtype;
	col->editor = 0;
	col->index = columns.size();
	col->paramname = paramname;
	columns.push_back(col);
	SetColumnEditor(*col);
}

void CPatternEditorInner::ClearColumns() {
	cursorcolumns.clear();
	for (std::vector<PE_column*>::iterator i = columns.begin(); i != columns.end(); ++i) {
		delete (*i)->editor;
		delete *i;
	}
	columns.clear();
	tracks.clear();
	editor_rows = 0;
	editor_units = 0;
	cursor.x = 0;
	cursor.y = 0;
	InitSelection();
	InitScroll();
	event_holes.clear();
	rcPreviewInvalid.left = -1;
}

void CPatternEditorInner::Init() {
	// things we need to reset if the orderlist follow mode changes
	// our pattern during the middle of ie, a drag operation.
	if (GetCapture() == m_hWnd) ReleaseCapture();
	ResetMouseMode();
	double_clicking = false;
	entering_chord = false;
	entering_chord_origin = -1;
}

void CPatternEditorInner::AddPianoRoll() {
// 	int unit = 0;
// 	if (columns.size() > 0) {
// 		PE_column& last_col = *columns[columns.size()-1];
// 		unit = last_col.unit + last_col.editor->GetWidth() + 1;
// 		last_col.spacer = true;
// 	}
// 	PE_track ti = { "Piano Roll", unit };
// 	tracks.push_back(ti);
// 
// 	int type = pattern_column_type_note;
// 	PE_column* col = new PE_column;
// 	col->type = type;
// 	col->novalue = 0;
// 	col->minvalue = 1;
// 	col->maxvalue = 255;
// 	col->unit = -1;
// 	col->spacer = false;
// 	col->control = pattern_column_control_pianoroll;
// 	col->defaultcontrol = pattern_column_control_pianoroll;
// 	col->editor = 0;
// 	col->index = columns.size();
// 	col->is_collapsed = 0;
// 	columns.push_back(col);
// 	SetColumnEditor(*col);
}

void CPatternEditorInner::SetColumnEditor(PE_column& col) {
	if (col.editor)
		delete col.editor;

	if (col.is_collapsed)
		col.editor = CColumnEditor::Create(pattern_column_control_collapsed);
	else
		col.editor = CColumnEditor::Create(col.control);

	assert(col.editor != 0);
	col.editor->column = &col;
	col.editor->owner = this;
}

void CPatternEditorInner::SetPatternInfos(std::vector<PE_patterninfo> const& infos, std::map<string, int> const& names) {
	patterninfos = infos;
	patternnames = names;
	Invalidate(FALSE);
}

// ---------------------------------------------------------------------------------------------------------------
// COLUMN ACTIONS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CPatternEditorInner::OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	if (cursorcolumns.size() == 0) return 0;

	PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
	PE_column& col = *columns[ccol.column];

	if (!col.editor->Char(wParam, lParam))
		return -1;

	return 0;
}

LRESULT CPatternEditorInner::OnLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	if (cursorcolumns.size() == 0) return 0;

	double_clicking = true;

	PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
	PE_column& col = *columns[ccol.column];

	if (!col.editor->DoubleClick(wParam, lParam))
		return -1;

	return 0;
}

bool CPatternEditorInner::DoColumnSpecial1() {
	if (cursorcolumns.size() == 0) return false;

	PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
	PE_column& col = *columns[ccol.column];

	return col.editor->Special1();
}

bool CPatternEditorInner::DoColumnSpecial2() {
	if (cursorcolumns.size() == 0) return false;

	PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
	PE_column& col = *columns[ccol.column];

	return col.editor->Special2();
}

bool CPatternEditorInner::DoColumnSpecial3() {
	if (cursorcolumns.size() == 0) return false;

	PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
	PE_column& col = *columns[ccol.column];

	return col.editor->Special3();
}

bool CPatternEditorInner::DoColumnSpecial4() {
	if (cursorcolumns.size() == 0) return false;

	PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
	PE_column& col = *columns[ccol.column];

	return col.editor->Special4();
}

bool CPatternEditorInner::DoColumnSpecial5() {
	if (cursorcolumns.size() == 0) return false;

	PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
	PE_column& col = *columns[ccol.column];

	return col.editor->Special5();
}

bool CPatternEditorInner::DoColumnSpecial6() {
	if (cursorcolumns.size() == 0) return false;

	PE_cursorcolumn& ccol = cursorcolumns[cursor.x];
	PE_column& col = *columns[ccol.column];

	return col.editor->Special6();
}

LRESULT CPatternEditorInner::ForwardAction(DWORD_PTR id) {
	return GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(id, 0), 0);
}

// ---------------------------------------------------------------------------------------------------------------
// ENCAPSULATION HELPERS
// ---------------------------------------------------------------------------------------------------------------

PE_cursorcolumn const& CPatternEditorInner::GetCursorColumnAtCursor() const {
	return cursorcolumns[cursor.x];
}

PE_column const& CPatternEditorInner::GetColumnAtCursor() const {
	return *columns[cursorcolumns[cursor.x].column];
}

PE_column const& CPatternEditorInner::GetColumn(int n) const {
	return *columns[n];
}

CPoint CPatternEditorInner::GetCursorAbsolute() const {
	POINT absolute_cursor = { cursor.x, cursor.y * skip };
	return absolute_cursor;
}

int CPatternEditorInner::GetCursorRowAbsolute() const {
	return cursor.y * skip;
}

CRect CPatternEditorInner::GetSortedSelectionOrCursorAbsolute() const {
	RECT rcSel;

	if (HasSelection()) {
		GetSortedSelectionRectAbsolute(&rcSel);
	} else {
		rcSel.left = cursorcolumns[cursor.x].column;
		rcSel.right = rcSel.left;
		rcSel.top = cursor.y * skip;
		rcSel.bottom = rcSel.top + (skip - 1);
	}

	return rcSel;
}

int CPatternEditorInner::GetColumnCount() const {
	return columns.size();
}

int CPatternEditorInner::GetPatternRows() const {
	return pattern_rows;
}

int CPatternEditorInner::GetSkip() const {
	return skip;
}

int CPatternEditorInner::GetResolution() const {
	return resolution;
}

PE_cursor_pos CPatternEditorInner::GetCursorPos() const {
	PE_cursor_pos pos;
	*(PE_column_pos*)&pos = *columns[cursorcolumns[cursor.x].column];
	pos.row = cursor.y * skip;
	pos.digit = cursorcolumns[cursor.x].digit;
	return pos;
}

PE_select_pos CPatternEditorInner::GetSelectFromPos() const {
	if (select_from.x == -1) {
		PE_select_pos pos;
		pos.column = -1;
		return pos;
	} else {
		PE_select_pos pos;
		*(PE_column_pos*)&pos = *columns[select_from.x];
		if (select_from.y >= select_to.y)
			pos.row = select_from.y;
		else
			pos.row = select_from.y + skip - 1;
		return pos;
	}
}

PE_select_pos CPatternEditorInner::GetSelectToPos() const {
	if (select_to.x == -1) {
		PE_select_pos pos;
		pos.column = -1;
		return pos;
	} else {
		PE_select_pos pos;
		*(PE_column_pos*)&pos = *columns[select_to.x];
		if (select_from.y >= select_to.y)
			pos.row = select_to.y + skip - 1;
		else
			pos.row = select_to.y;
		return pos;
	}
}

// PE_scroll_pos CPatternEditorInner::GetPatternScrollPos() const {
// 	
// }

bool CPatternEditorInner::GetSortedSelectionRectAbsolute(RECT* rc) const {
	if (select_begin.x == -1) return false;

	int left_col = select_begin.x;
	int top_row = select_begin.y * skip;
	int right_col = select_end.x;
	int bottom_row = select_end.y * skip;
	bottom_row += (skip - 1); // this perception of the selection includes data you cannot see

	rc->left = clamp(left_col, 0, columns.size() - 1);
	rc->right = clamp(right_col, 0, columns.size() - 1);
	rc->top = clamp(top_row, 0, pattern_rows - 1);
	rc->bottom = clamp(bottom_row, 0, pattern_rows - 1);
	return true;
}

bool CPatternEditorInner::GetUnsortedSelectionRangeAbsolute(POINT* ptFrom, POINT* ptTo) const {
	if (select_begin.x == -1) return false;

	int from_row = select_from.y * skip;
	int to_row = select_to.y * skip;

	if (select_from.y <= select_to.y) {
		to_row += (skip - 1); // this perception of the selection includes data you cannot see
	} else {
		from_row += (skip - 1);
	}

	ptFrom->x = clamp(select_from.x, 0, columns.size() - 1);
	ptFrom->y = clamp(from_row, 0, pattern_rows - 1);
	ptTo->x = clamp(select_to.x, 0, columns.size() - 1);
	ptTo->y = clamp(to_row, 0, pattern_rows - 1);

	return true;
}

// ---------------------------------------------------------------------------------------------------------------
// NOTE MASKS
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorInner::MaskNote(int note, int meta, bool state) {
	transpose_mask[note][meta] = state;
}

void CPatternEditorInner::MaskNoteReset(bool state) {
	for (int i = 0; i < 12; ++i) {
		transpose_mask[i][0] = state;
		transpose_mask[i][1] = state;
		transpose_mask[i][2] = state;
	}
}

void CPatternEditorInner::SetEventHoles() {
	event_holes.clear();
	if (select_from.x == -1) return;

	for (int i = select_begin.x; i <= select_end.x; ++i) {
		PE_column const& col = *columns[i];
		if (col.type != pattern_column_type_note) continue;

		PE_values_by_time::const_iterator j = col.values_by_time.lower_bound(select_begin.y * skip);
		PE_values_by_time::const_iterator j_end = col.values_by_time.upper_bound(select_end.y * skip);

		for (; j != j_end; ++j) {
			if (j->value == 255 || j->value == 254) continue;

			int basenote = (j->value & 0xF) - 1;
			int meta = j->meta;
			if (!transpose_mask[basenote][meta]) {
				event_holes.insert(j->id);
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------
// HARMONY
// ---------------------------------------------------------------------------------------------------------------

int CPatternEditorInner::HSysResolveShade(int value, int meta, int time) {
	if (!hsys_enabled) return 0;

	Harmony::MetaSet* metaset = hsys.CacheResolveRow(time);
	return metaset->notes[(value & 0xF) - 1][meta];
}

void CPatternEditorInner::HSysEnable(bool state) {
	hsys_enabled = state;
	if (!state) {
		hsys.Reset();
	}
}

void CPatternEditorInner::HSysInvalidate(int time_from, int time_to) {
	RECT rcInvalid = {
		0,
		(max(time_from - 1, 0) / skip) * font_size.cy,
		editor_units * font_size.cx,
		(time_to == -1 ? editor_rows : time_to / skip) * font_size.cy
	};
	clientDC->LPtoDP(&rcInvalid);
	InvalidateRect(&rcInvalid, FALSE);

	/// scroller
}

int CPatternEditorInner::HSysGetContextLength(int x, int y) {
	if (!hsys_enabled) return 1;

	int next_ctx_row = hsys.GetNextContextRow(x, y);
	if (next_ctx_row == -1)
		return pattern_rows - y;
	else
		return next_ctx_row - y;
}

// ---------------------------------------------------------------------------------------------------------------
// SCROLLER
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorInner::InvalidatePreview(RECT* rcPreview) {
	if (rcPreviewInvalid.left == -1) {
		rcPreviewInvalid = *rcPreview;
	} else {
		rcPreviewInvalid.left   = min(rcPreviewInvalid.left, rcPreview->left);
		rcPreviewInvalid.top    = min(rcPreviewInvalid.top, rcPreview->top);
		rcPreviewInvalid.right  = max(rcPreviewInvalid.right, rcPreview->right);
		rcPreviewInvalid.bottom = max(rcPreviewInvalid.bottom, rcPreview->bottom);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// XXX
// ---------------------------------------------------------------------------------------------------------------

int CPatternEditorInner::GetNotesAffectMode() const {
	int mode = 0;

	if (notes_affect_waves)
		mode += 1;
	if (notes_affect_volumes)
		mode += 2;

	return mode;
}

void CPatternEditorInner::SetNotesAffectMode(int mode) {
	switch (mode) {
		case 0:	
			notes_affect_waves = false;
			notes_affect_volumes = false;
			break;
		case 1:
			notes_affect_waves = true;
			notes_affect_volumes = false;
			break;
		case 2:
			notes_affect_waves = false;
			notes_affect_volumes = true;
			break;
		case 3:
			notes_affect_waves = true;
			notes_affect_volumes = true;
			break;
	}
}
