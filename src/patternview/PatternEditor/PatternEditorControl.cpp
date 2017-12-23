#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include <sstream>
#include <iomanip>
#include "Configuration.h"
#include "BuzeConfiguration.h"
#include "PatternView.h"
#include "PatternEditorInner.h"
#include "PatternEditorControl.h"
#include "PatternEditorColumn.h"
#include "utils.h"

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DESTRUCTION
// ---------------------------------------------------------------------------------------------------------------

CPatternEditorControl::CPatternEditorControl(CPatternEditorScroller& editorScroller)
:
	editorInner(editorScroller)
{
	margin_x = 0;
	margin_y = 0;
	replay_row = -1;
	last_scroll_x = 0;
	last_scroll_y = 0;
	subrow_mode = false;
	row_held = false;
}

CPatternEditorControl::~CPatternEditorControl() {
}

LRESULT CPatternEditorControl::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	mark_pen.CreatePen(PS_SOLID, 1, RGB(0x1F,0x1F,0x1F));

	editorInner.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_HSCROLL|WS_VSCROLL);

	// http://codewiz51.blogspot.com/2008/04/implementing-tooltips-in-wtl.html
	headertip.Create(m_hWnd, rcDefault, 0, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP);
	//headertip.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	headertip.Activate(TRUE);
	return 0;
}

LRESULT CPatternEditorControl::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	for (std::vector<boost::shared_ptr<CToolInfo> >::iterator i = headerinfos.begin(); i != headerinfos.end(); ++i) {
		headertip.DelTool(i->get());
	}
	headerinfos.clear();
	return 0;
}

LRESULT CPatternEditorControl::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	// must call DestroyWindow on the tooltip in OnDestroy to prevent leaking gdi font handles
	headertip.DestroyWindow();
	SetMsgHandled(FALSE);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// SIZING / SCROLLING
// ---------------------------------------------------------------------------------------------------------------

LRESULT CPatternEditorControl::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	CRect rcClient;
	GetClientRect(&rcClient);

	int pe_width = rcClient.right - margin_x;
	int pe_height = rcClient.bottom - margin_y;

	editorInner.MoveWindow(margin_x, margin_y, pe_width, pe_height);

	return 0;
}

LRESULT CPatternEditorControl::OnScrolled(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/) {
	if (last_scroll_x != editorInner.scroll.x) {
		last_scroll_x = editorInner.scroll.x;
		InvalidateCols();
	}

	if (last_scroll_y != editorInner.scroll.y) {
		last_scroll_y = editorInner.scroll.y;
		InvalidateRows();
	}

	SetTooltips();

	SetMsgHandled(FALSE);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// FOCUS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CPatternEditorControl::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	editorInner.SetFocus();
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// BINDING
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorControl::SetPatternRows(int num) {
	editorInner.SetPatternRows(num);
	Invalidate(FALSE);
}

void CPatternEditorControl::SetSkip(int num) {
	editorInner.SetSkip(num);
	Invalidate(FALSE);
}

void CPatternEditorControl::SetResolution(int num) {
	editorInner.SetResolution(num);
	Invalidate(FALSE);
}

void CPatternEditorControl::SetThemeColor(PE_themeindex index, COLORREF color) {
	editorInner.SetThemeColor(index, color);
	Invalidate(FALSE);
}

void CPatternEditorControl::SetFont(HFONT hFont, bool bRedraw) {
	if (bRedraw) {
		editorInner.HideCaret();
		DestroyCaret();
	}

	editorInner.SetFont(hFont, FALSE);

	margin_x = editorInner.font_size.cx * 6;
	margin_y = editorInner.font_size.cy * 2;

	if (bRedraw) {
		editorInner.UpdateScrollbars();
		editorInner.UpdateCaret();
		editorInner.Invalidate(FALSE);

		OnSize(0, 0, 0);
		Invalidate(FALSE);
	}
}

void CPatternEditorControl::SetTooltips() {

	for (std::vector<boost::shared_ptr<CToolInfo> >::iterator i = headerinfos.begin(); i != headerinfos.end(); ++i) {
		headertip.DelTool(i->get());
	}
	headerinfos.clear();

	int scroll_x = editorInner.scroll.x * editorInner.font_size.cx;

	for (int i = 0; i < editorInner.tracks.size(); ++i) {
		PE_track const& track = editorInner.tracks[i];

		int track_x = track.unit * editorInner.font_size.cx;
		int adjusted_x = margin_x + track_x - scroll_x;
		int trackwidth_x = (track.width - 1) * editorInner.font_size.cx;

		RECT rcTool;
		SetRect(&rcTool, adjusted_x, 0, adjusted_x + trackwidth_x, editorInner.font_size.cy);

		TCHAR toolstr[1024];
		_tcscpy(toolstr, track.name.c_str());

		boost::shared_ptr<CToolInfo> toolinfo(new CToolInfo(0, m_hWnd, i + 1, &rcTool, toolstr));
		headertip.AddTool(toolinfo.get());
		headerinfos.push_back(toolinfo);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// PAINTING
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorControl::InvalidateCols() {
	RECT rcClient;
	GetClientRect(&rcClient);

	RECT rc = { margin_x - editorInner.font_size.cx, 0, rcClient.right, rcClient.bottom };
	InvalidateRect(&rc, FALSE);
}

void CPatternEditorControl::InvalidateRows() {
	RECT rcClient;
	GetClientRect(&rcClient);

	RECT rc = { 0, 0, margin_x - editorInner.font_size.cx, rcClient.bottom };
	InvalidateRect(&rc, FALSE);
}

LRESULT CPatternEditorControl::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	USES_CONVERSION;

	RECT rcClient;
	GetClientRect(&rcClient);

// 	CPaintDC screenDC(m_hWnd);
// 	CMemDC dc(screenDC);
	CPaintDC dc(m_hWnd);
	RECT m_rc; dc.GetClipBox(&m_rc);

	CRect rcCorner(0, 0, margin_x, margin_y);
	CRect rcColumns(margin_x - editorInner.font_size.cx, 0, rcClient.right, margin_y);
	CRect rcRows(0, margin_y, margin_x - editorInner.font_size.cx, rcClient.bottom);
	CRect rcFirstLine(margin_x - editorInner.font_size.cx, margin_y, margin_x, rcClient.bottom);

	COLORREF bgcolor = editorInner.colors[PE_BG];
	CFontHandle prevfont = dc.SelectFont(editorInner.track_font);
	dc.SetTextAlign(TA_LEFT);

	int scroll_x = editorInner.scroll.x * editorInner.font_size.cx;

	if (IntersectRect(&rcCorner, &rcCorner, &/*dc.*/m_rc))
	{
		dc.FillSolidRect(0, 0, margin_x, margin_y, bgcolor);
	}

	if (IntersectRect(&rcRows, &rcRows, &/*dc.*/m_rc))
	{
		// render row numbers
		{
			RECT rcClientInner;
			editorInner.GetClientRect(&rcClientInner);
			int screenrows = rcClientInner.bottom / editorInner.font_size.cy;

			dc.SetBkColor(bgcolor);
			dc.SetTextColor(editorInner.colors[PE_TextRows]);

			int last_row = 0;
			for (int i = 0; (i <= screenrows) && ((i + editorInner.scroll.y) < editorInner.editor_rows); ++i) {
				std::stringstream ss;
				std::string rowstr;
				int rownum = (i + editorInner.scroll.y) * editorInner.skip;

				if (editorInner.resolution == 1) {
					ss << rownum;
					rowstr = ss.str();
				} else
				if (subrow_mode) {
					int row_major = rownum / editorInner.resolution;
					int row_minor = rownum % editorInner.resolution;

					if (editorInner.skip >= editorInner.resolution)
						ss << row_major;
					else
						ss << row_major << "·" << row_minor;

					rowstr = ss.str();
				} else {
					float rowfloat = (float)rownum / (float)editorInner.resolution;

					ss << std::setw(5) << std::fixed << rowfloat;
					rowstr = ss.str();

					while (rowstr.length() > 0 && rowstr[rowstr.length() - 1] == '0') {
						rowstr = rowstr.substr(0, rowstr.length() - 1);
					}
					if (rowstr.length() > 0 && rowstr[rowstr.length() - 1] == '.') {
						rowstr = rowstr.substr(0, rowstr.length() - 1);
					}
				}

				ss.str("");
				ss << std::right << std::setfill(' ') << std::setw(5) << rowstr << " ";
				rowstr = ss.str();

				//ss << std::right << std::setfill(' ') << std::setw(5) << std::fixed << rowfloat << " ";
				//ss << std::right << std::setfill(' ') << std::setw(5) << rownum << " ";
				//std::string row = ss.str();
				int y_pos = margin_y + (i * editorInner.font_size.cy);
				dc.TextOut(0, y_pos, CA2T(rowstr.c_str()));

				// render replay row
				if (rownum == replay_row) {
					CPenHandle oldPen = dc.SelectPen(mark_pen);

					int x = 3;
					int y = y_pos + 3;
					dc.MoveTo(x, y);
					dc.LineTo(x+8, y);
					dc.LineTo(x+4, y+4);
					dc.LineTo(x, y);

					dc.SelectPen(oldPen);
				}

				last_row = i;
			}

			// clear space below numbers
 			int bottom_y = margin_y + ((last_row + 1) * editorInner.font_size.cy);
 			dc.FillSolidRect(0, bottom_y, margin_x, rcClient.bottom - bottom_y, bgcolor);
		}
	}

	if (IntersectRect(&rcColumns, &rcColumns, &/*dc.*/m_rc))
	{
		// render track names
		{
			dc.FillSolidRect(0, 0, rcClient.right, margin_y, bgcolor);

			for (int i = 0; i < editorInner.tracks.size(); ++i) {
				PE_track const& track = editorInner.tracks[i];

				int track_x = track.unit * editorInner.font_size.cx;
				int adjusted_x = margin_x + track_x - scroll_x;
				int trackwidth_x = (track.width - 1) * editorInner.font_size.cx;

				if ((adjusted_x + trackwidth_x) > (margin_x + editorInner.font_size.cx)) {
					COLORREF track_color;
					COLORREF track_color_bg;

					if (track.is_muted) {
						track_color = editorInner.colors[PE_TextTrackMuted];
						track_color_bg = editorInner.colors[PE_TextTrackMuted_BG];
					} else {
						track_color = editorInner.colors[PE_TextTrack];
						track_color_bg = bgcolor;
					}

					dc.FillSolidRect(adjusted_x, 0, trackwidth_x, margin_y, track_color_bg);

					std::string name = track.name.substr(0, track.width - 1);
					dc.SetTextColor(track_color);
					dc.SetBkColor(track_color_bg);
					dc.TextOut(adjusted_x, 0, CA2T(name.c_str()));
				}
			}
		}

		// render plugin dividers
		{
			CPenHandle oldPen = dc.SelectPen(editorInner.divider_pen);
			int oldBkMode = dc.SetBkMode(TRANSPARENT);

			int last_plug = -1;
			for (int i = 0; i < editorInner.columns.size(); ++i) {
				PE_column const& col = editorInner.GetColumn(i);
				// render paramnames
				int plug_x = col.unit * editorInner.font_size.cx;
				int adjusted_x = margin_x + plug_x - scroll_x;
				dc.SetTextColor(editorInner.colors[PE_TextShade]);
			    //dc.SetBkColor(editorInner.colors[PE_BG]);	
				int chars = (plug_x - scroll_x) / editorInner.font_size.cx;
				std::string paramname = col.paramname.substr( 0, col.editor->GetWidth() );
				dc.TextOut(adjusted_x + 1, editorInner.font_size.cy, CA2T( paramname.c_str()));
 
				if (col.plugin_id != last_plug) {
					//int adjusted_x = margin_x + plug_x - scroll_x - 4;

					if (adjusted_x > (margin_x - editorInner.font_size.cx)) {
						dc.MoveTo(adjusted_x, 0);
						dc.LineTo(adjusted_x, margin_y);
					}
				}

				last_plug = col.plugin_id;
			}

			dc.SetBkMode(oldBkMode);
			dc.SelectPen(oldPen);
		}
	}

	// render first divider
	if (IntersectRect(&rcFirstLine, &rcFirstLine, &/*dc.*/m_rc))
	{
		CRect rcFill(margin_x - editorInner.font_size.cx, margin_y, margin_x, rcClient.bottom);
		dc.FillSolidRect(rcFill.left, rcFill.top, rcFill.Width(), rcFill.Height(), bgcolor);

		int adjusted_x = margin_x - scroll_x - 4;

		if (adjusted_x > (margin_x - editorInner.font_size.cx)) {
			CPenHandle oldPen = dc.SelectPen(editorInner.divider_pen);
			dc.MoveTo(adjusted_x, 0);
			dc.LineTo(adjusted_x, rcClient.bottom);
			dc.SelectPen(oldPen);
		}
	}

	// render selection range
	{
		
	}

	dc.SelectFont(prevfont);
	return 0;
}

LRESULT CPatternEditorControl::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	return 1;
}

// ---------------------------------------------------------------------------------------------------------------
// MOUSE
// ---------------------------------------------------------------------------------------------------------------

int CPatternEditorControl::TrackFromPoint(POINT pt) const {
	for (int i = 0; i < editorInner.tracks.size(); ++i) {
		PE_track const& track = editorInner.tracks[i];

		int track_x = track.unit * editorInner.font_size.cx;
		int scroll_x = editorInner.scroll.x * editorInner.font_size.cx;
		int adjusted_x = margin_x + track_x - scroll_x;
		int trackwidth_x = track.width * editorInner.font_size.cx;

		if ((pt.x >= adjusted_x) && (pt.x < (adjusted_x + trackwidth_x))) {
			return i;
		}
	}

	return -1;
}

int CPatternEditorControl::RowFromPoint(POINT pt) const {
	int row = (((pt.y - margin_y) / editorInner.font_size.cy) + editorInner.scroll.y) * editorInner.skip;

	if ((row >= 0) && (row < editorInner.GetPatternRows()))
		return row;

	return -1;
}

LRESULT CPatternEditorControl::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };

	if ((pt.x < margin_x) && (pt.y < margin_y)) { // Gutter
		// something special
	} else
	if ((pt.x > margin_x) && (pt.y < margin_y)) { // Tracks
		int track_idx = TrackFromPoint(pt);
		if (track_idx != -1)
			GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORCONTROL_TRACKMUTE, track_idx), 0);
	} else
	if ((pt.x < margin_x) && (pt.y > margin_y)) { // Rows
		int row = RowFromPoint(pt);
		if (row != -1) {
			if (IsCtrlDown()) {
				row_held = true;
				editorInner.HoldAllNotes();
				GetParent().GetParent().GetParent().SendMessage(WM_HOLDROW, row, 0); // we know parent and parents parent are splitters and im not sure how to send non-WM_COMMANDS through the splitters
			} else
			if (IsCtrlDown() && IsShiftDown()) {
				// something special
			} else {
				GetParent().GetParent().GetParent().SendMessage(WM_PLAYFROMROW, row, 0); // we know parent and parents parent are splitters and im not sure how to send non-WM_COMMANDS through the splitters
			}
		}
	} else {
		///
	}

	return 0;
}

LRESULT CPatternEditorControl::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	if (row_held) {
		row_held = false;
		editorInner.ReleaseAllNotes();
	}

	return 0;
}

LRESULT CPatternEditorControl::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };

	if ((pt.x < margin_x) && (pt.y < margin_y)) { // Gutter
		// something special
	}
	if ((pt.x > margin_x) && (pt.y < margin_y)) { // Tracks
		int track_idx = TrackFromPoint(pt);
		if (track_idx != -1)
			GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(ID_PATTERNEDITORCONTROL_TRACKSOLO, track_idx), 0);
	} else
	if ((pt.x < margin_x) && (pt.y > margin_y)) { // Rows
		// something special
	} else {
		// something special
	}

	return 0;
}

// fixes clicks being ignored
LRESULT CPatternEditorControl::OnLButtonDblClk(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	return OnLButtonDown(WM_LBUTTONDOWN, wParam, lParam);
}

LRESULT CPatternEditorControl::OnForward(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

LRESULT CPatternEditorControl::OnAnyMouseMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	bHandled = FALSE;
	if (headertip.IsWindow())
		headertip.RelayEvent((LPMSG)GetCurrentMessage());
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// KEYBOARD
// ---------------------------------------------------------------------------------------------------------------

LRESULT CPatternEditorControl::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return -1;
}

// ---------------------------------------------------------------------------------------------------------------
// P,G,T,C ADAPTERS
// ---------------------------------------------------------------------------------------------------------------

// only called from CPatternView::BindPatternPosition()
bool CPatternEditorControl::SetCursor(int plugin_id, int group, int track, int column, int digit, int row) {
	if (plugin_id == -1) {
		if (row == -1) {
			editorInner.SetCursor(0, 0);
		} else {
			editorInner.SetCursor(0, row / editorInner.skip);
		}
	} else {
		column_map_t::iterator i = column_map.find(PE_column_pos(plugin_id, group, track, column));
		if (i == column_map.end()) return false;

		int cursor_x = editorInner.ScreenUnitsToCursorColumn(i->second->unit);
		if (cursor_x != -1)
			cursor_x += digit;
		else
			cursor_x = 0;

		editorInner.SetCursor(cursor_x, row / editorInner.skip);
	}

	return true;
}

void CPatternEditorControl::SelectRange(
	int from_plugin_id, int from_group, int from_track, int from_column, int from_row,
	int to_plugin_id, int to_group, int to_track, int to_column, int to_row)
{
	PE_column* from_col = GetColumn(from_plugin_id, from_group, from_track, from_column);
	PE_column* to_col = GetColumn(to_plugin_id, to_group, to_track, to_column);
	if (!from_col || !to_col) return ; // could happen if the column was removed from the format since it was cached - and therefore attempted selectranged

	editorInner.SelectRangeAbsolute(from_col->index, from_row, to_col->index, to_row);
}

PE_column* CPatternEditorControl::GetColumn(int plugin_id, int group, int track, int column) {
	column_map_t::iterator i = column_map.find(PE_column_pos(plugin_id, group, track, column));

	if (i != column_map.end())
		return (i->second);
	else
		return (PE_column*)0;
}

PE_column* CPatternEditorControl::GetColumnByEvent(int patternevent_id) {
	for (std::vector<PE_column*>::iterator i = editorInner.columns.begin(); i != editorInner.columns.end(); ++i) {
		PE_values_by_id::const_iterator idvalue = (*i)->values_by_id.find(patternevent_id);
		if (idvalue != (*i)->values_by_id.end()) 
			return *i;
	}
	return 0;
}


int CPatternEditorControl::GetColumnIndex(int plugin_id, int group, int track, int column) const {
	column_map_t::const_iterator i = column_map.find(PE_column_pos(plugin_id, group, track, column));

	if (i != column_map.end())
		return i->second->index;
	else
		return -1;
}

int CPatternEditorControl::GetColumnCount(int plugin_id, int group, int track) const {
	int count = 0;
	for (int i = 0; i < editorInner.GetColumnCount(); ++i) {
		PE_column const& col = editorInner.GetColumn(i);
		if (col.plugin_id == plugin_id && col.group == group && col.track == track)
			++count;
	}
	return count;
}

int CPatternEditorControl::GetTrackCount(int plugin_id, int group) const {
	int count = 0;
// 	int last_track = -1;
// 	for (int i = 0; i < editorInner.GetColumnCount(); ++i) {
// 		PE_column const& col = editorInner.GetColumn(i);
// 		if (col.plugin_id == plugin_id && col.group == group) {
// 			if (col.track != last_track)
// 				++count;
// 			last_track = col.track;
// 		}
// 	}
	return count;
}

PE_track const& CPatternEditorControl::GetTrack(int plugin_id, int group, int track) const {
	for (int i = 0; i < editorInner.tracks.size(); ++i) {
		PE_track const& pe_track = editorInner.tracks[i];
		if (true
			&& pe_track.plugin_id == plugin_id
			&& pe_track.group == group
			&& pe_track.track == track
		) {
			return pe_track;
		}
	}
	return *(PE_track*)0;
}
