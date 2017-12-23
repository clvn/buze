#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "ToolbarWindow.h" // for CIntegralEdit..
#include "utils.h"
#include "Configuration.h"
#include "mixdown.h"
#include "BuzeConfiguration.h"

#include "PatternView.h"
#include "PatternEditorOrderlist.h"

/*
void zzub_player_set_order_length(zzub_player_t* player, int length);
int zzub_player_get_order_length(zzub_player_t* player);
void zzub_player_set_order_pattern(zzub_player_t* player, int index, zzub_pattern_t* pattern);
zzub_pattern_t *zzub_player_get_order_pattern(zzub_player_t* player, int index);
zzub_pattern_iterator_t *zzub_player_get_order_iterator(zzub_player_t* player);
int zzub_player_get_order_loop_start(zzub_player_t* player);
void zzub_player_set_order_loop_start(zzub_player_t* player, int pos);
int zzub_player_get_order_loop_end(zzub_player_t* player);
void zzub_player_set_order_loop_end(zzub_player_t* player, int pos);
int zzub_player_get_order_loop_enabled(zzub_player_t* player);
void zzub_player_set_order_loop_enabled(zzub_player_t* player, int enable);
void zzub_player_set_queue_order_index(zzub_player_t* player, int pos);
int zzub_player_get_queue_order_index(zzub_player_t* player);
int zzub_player_get_position_order(zzub_player_t* player);
int zzub_player_get_position_row(zzub_player_t* player);
int zzub_player_get_position_samples(zzub_player_t* player);
void zzub_player_set_position(zzub_player_t* player, int orderindex, int tick);
*/

// ---------------------------------------------------------------------------------------------------------------
// SETTINGS
// ---------------------------------------------------------------------------------------------------------------

static const int box_width = 64;

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DESTRUCTION
// ---------------------------------------------------------------------------------------------------------------

CPatternEditorOrderlist::CPatternEditorOrderlist(CViewFrame* mainFrame, CPatternView* owner)
:
	mainFrame(mainFrame),
	owner(owner),
	document(owner->document)
{
	colorText = GetSysColor(COLOR_WINDOWTEXT);
	colorTextLight = GetSysColor(COLOR_3DLIGHT);
	colorTextSel = GetSysColor(COLOR_HIGHLIGHTTEXT);
	colorTextSelLight = GetSysColor(COLOR_3DSHADOW);
	colorWindow = GetSysColor(COLOR_WINDOW);
	colorHighlight = GetSysColor(COLOR_MENUHILIGHT);
	colorShadow = RGB(0x80, 0x00, 0x80);
	clientDC = 0;
}

CPatternEditorOrderlist::~CPatternEditorOrderlist()
{
	if (m_hWnd)
		DestroyWindow();
}

int CPatternEditorOrderlist::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	clientDC = new CClientDC(m_hWnd);

	Init();

	ModifyStyleEx(0, WS_EX_STATICEDGE);

	CModuleMessageLoop* messageLoop = _Module.GetMessageLoop();
	messageLoop->AddMessageFilter(this);

	ShowScrollBar(SB_HORZ, true);

	return 0;
}

void CPatternEditorOrderlist::Init()
{
	scroll_index = 0;
	select_from = 0;
	select_to = 0;
	drag_mode = drag_mode_off;
	drop_pos = drop_pos_none;
	box_height = 0;
	got_repeat = false;
	mouse_activating = false;
}

void CPatternEditorOrderlist::OnDestroy()
{
	if (clientDC) {
		delete clientDC;
		clientDC = 0;
	}

	CModuleMessageLoop* messageLoop = _Module.GetMessageLoop();
	messageLoop->RemoveMessageFilter(this);	

	SetMsgHandled(FALSE);///
}

// ---------------------------------------------------------------------------------------------------------------
// UPDATES
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorOrderlist::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint)///
{
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
//		case zzub_event_type_player_state_changed:
// 		case zzub_event_type_insert_orderlist:
// 		case zzub_event_type_delete_orderlist:
// 		case zzub_event_type_update_orderlist:
// 		case zzub_event_type_delete_pattern:
		case zzub_event_type_insert_orderlist:
		case zzub_event_type_update_orderlist:
		case zzub_event_type_delete_orderlist:
			//TODO: event-based invalidation (for undo/redo)
			//InvalidateOrder(data.zzubData->insert_orderlist.index);
			break;
		case zzub_event_type_player_order_queue_changed:
			Invalidate(FALSE);
			break;
		case zzub_event_type_player_order_changed:
			SyncOrderToPlayer(zzubData->player_order_changed.orderindex);
			Invalidate(FALSE);
			break;
		case zzub_event_type_update_song:
			Invalidate(FALSE);
			break;
		case zzub_event_type_update_pattern:
			Invalidate(FALSE);
			break;
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_new_document:
			SetSelect(0, sel_mode_single);
			break;
	}
}

// ---------------------------------------------------------------------------------------------------------------
// SIZING / SCROLLING
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorOrderlist::OnSize(UINT nType, CSize size)
{
//	if (buze_main_frame_get_player(mainFrame) == 0) return;

	UpdateScrollInfo();

	CRect rcClient;
	GetClientRect(&rcClient);
	box_height = rcClient.bottom;
}

void CPatternEditorOrderlist::UpdateScrollInfo()
{
	SCROLLINFO si;
	GetScrollInfo(SB_HORZ, &si);
	si.fMask = SIF_PAGE|SIF_RANGE|SIF_DISABLENOSCROLL;
	si.nMin = 0;

	int nPage = GetBoxesOnscreenCount();
	int nMax = max(zzub_player_get_order_length(buze_main_frame_get_player(mainFrame)) + 1, nPage) - 1;

	if ((nMax != si.nMax) || (nPage != si.nPage)) {
		si.nPage = nPage;
		si.nMax = nMax;
		SetScrollInfo(SB_HORZ, &si);
	}

	ScrollTo(GetScrollPos(SB_HORZ));
}

void CPatternEditorOrderlist::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar)
{
	switch (nSBCode) {
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			ScrollTo(nPos);
			break;
		case SB_LINELEFT:
			ScrollTo(scroll_index - 1);
			break;
		case SB_LINERIGHT:
			ScrollTo(scroll_index + 1);
			break;
		case SB_PAGELEFT:
			ScrollTo(scroll_index - 4);
			break;
		case SB_PAGERIGHT:
			ScrollTo(scroll_index + 4);
			break;
		default:
			return;
	}
}

void CPatternEditorOrderlist::ScrollTo(int nPos)
{
	SetScrollPos(SB_HORZ, nPos);
	int new_scroll_index = GetScrollPos(SB_HORZ);

	if (new_scroll_index != scroll_index) {
		int scroll_amount = (scroll_index - new_scroll_index) * box_width;
		ScrollWindow(scroll_amount, 0);
		scroll_index = new_scroll_index;
	}
}

void CPatternEditorOrderlist::EnsureVisibleCursor()
{
	int last_box = GetLastBoxOnscreen();

	if (select_from < scroll_index) {
		ScrollTo(select_from);
	} else
	if (select_from > last_box) {
		ScrollTo(select_from - GetBoxesOnscreenCount() + 1);
	}
}

void CPatternEditorOrderlist::EnsureVisibleCursorRange() // todo: double scroll is occurring
{
	UpdateScrollInfo();

	int last_box = GetLastBoxOnscreen();
	PE_order_selection sel = GetSortedSel();

	if ((sel.index_low > last_box) || (sel.index_high > last_box)) {
		ScrollTo(min(sel.index_low, sel.index_high - GetBoxesOnscreenCount() + 1));
	}
}

// ---------------------------------------------------------------------------------------------------------------
// FOCUS
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorOrderlist::OnSetFocus(CWindow wndOld)
{
	InvalidateSelection();

	if (!(mouse_activating && IsAltDown()))
		EditAndSync();

	mouse_activating = false;
}

void CPatternEditorOrderlist::OnKillFocus(CWindow wndFocus)
{
	StopDragging();

	if (select_from != select_to)
		SetSelect(select_from, sel_mode_single);
	else
		InvalidateSelection();
}

int CPatternEditorOrderlist::OnMouseActivate(CWindow wndTopLevel, UINT nHitTest, UINT message)
{
	if (GetFocus() == m_hWnd)
		return MA_ACTIVATE;
	else {
		mouse_activating = true;
		return DefWindowProc();
	}
}

// ---------------------------------------------------------------------------------------------------------------
// LOGIC
// ---------------------------------------------------------------------------------------------------------------

int CPatternEditorOrderlist::GetBoxesOnscreenCount() const
{
	CRect rcClient;
	GetClientRect(&rcClient);

	return rcClient.right / box_width;
}

int CPatternEditorOrderlist::GetBoxesTotalCount() const
{
	return max(zzub_player_get_order_length(buze_main_frame_get_player(mainFrame)) + 1, GetBoxesOnscreenCount());
}

int CPatternEditorOrderlist::GetLastBoxOnscreen() const
{
	return scroll_index + GetBoxesOnscreenCount() - 1;
}

int CPatternEditorOrderlist::GetOrderFromPoint(CPoint pt) const
{
	clientDC->DPtoLP(&pt);
	return pt.x / box_width;
}

int CPatternEditorOrderlist::GetDropOrderFromPoint(CPoint pt) const
{
	clientDC->DPtoLP(&pt);
	return (pt.x + (box_width / 2)) / box_width;
}

PE_order_selection CPatternEditorOrderlist::GetSortedSel() const
{
	PE_order_selection result;

	result.index_low = min(select_from, select_to);
	result.index_high = max(select_from, select_to);

	return result;
}

bool CPatternEditorOrderlist::SetSelect(int sel, int sel_mode)
{
	int len = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));
	sel = clamp(sel, 0, len);

	InvalidateSelection();

	switch (sel_mode) {
		case sel_mode_single:
			select_from = sel;
			select_to = sel;
			break;
		case sel_mode_from:
			select_from = sel;
			break;
		case sel_mode_to:
			select_to = sel;
			break;
	};

	InvalidateSelection();

	buze_event_data_t ev;
	ev.change_pattern_order.index = select_from;
	buze_document_notify_views(document, owner, buze_event_type_change_pattern_order, &ev);

	return true;
}

bool CPatternEditorOrderlist::SetSelectRange(int from, int to)
{
	int len = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));
	from = clamp(from, 0, len);
	to = clamp(to, 0, len);

	InvalidateSelection();

	select_from = from;
	select_to = to;

	InvalidateSelection();

	return true;
}

// ---------------------------------------------------------------------------------------------------------------
// PAINTING
// ---------------------------------------------------------------------------------------------------------------

namespace{
enum {
	zzub_order_index_pattern = 0,
	zzub_order_index_invalid = 1,
	zzub_order_index_skip = 2,
};
} // END namespace

void CPatternEditorOrderlist::OnPaint(CDCHandle /*dc*/)
{
	UpdateScrollInfo();

	int width = GetBoxesTotalCount() * box_width;
	int scroll_x = scroll_index * box_width;

	CPaintDC screenDC(m_hWnd);
	screenDC.SetWindowExt(width, box_height);
	screenDC.SetWindowOrg(scroll_x, 0);
	clientDC->SetWindowOrg(scroll_x, 0);

	CMemDC dc(screenDC);
	CFontHandle prevfont = dc.SelectFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	CPen thinPen; thinPen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
	dc.SelectPen(thinPen);
	dc.SetBkMode(TRANSPARENT);

	int first_index = dc.m_rc.left / box_width;
	int last_index = dc.m_rc.right / box_width;

	PE_order_selection sel = GetSortedSel();

	int order_count = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));
	int playing_index = zzub_player_get_position_order(buze_main_frame_get_player(mainFrame));
	int queue_index = zzub_player_get_queue_order_index(buze_main_frame_get_player(mainFrame));
	int loop_start_index = zzub_player_get_order_loop_start(buze_main_frame_get_player(mainFrame));
	int loop_end_index = zzub_player_get_order_loop_end(buze_main_frame_get_player(mainFrame));

	bool hasFocus = (GetFocus() == m_hWnd);

	static std::vector<char> bytes(128);

	for (int i = first_index; i <= last_index; ++i)
	{
		CRect rect;
		rect.left = i * box_width;
		rect.top = 0;
		rect.right = rect.left + box_width - 1;
		rect.bottom = box_height;

		// Check pattern
		int order_type;
		zzub_pattern_t* pat;
		if (i < order_count) {
			order_type = zzub_order_index_pattern;
			pat = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), i);
		} else {
			order_type = zzub_order_index_invalid;
			pat = 0;
		}

		// Highlight the selection range
		bool bHighLight = hasFocus && (i >= sel.index_low) && (i <= sel.index_high);

		// Fill the box background color
		if (i > order_count) {
			dc.FillSolidRect(&rect, colorTextLight);
		} else if (bHighLight) {
			dc.FillSolidRect(&rect, colorHighlight);
		} else {
			dc.FillSolidRect(&rect, colorWindow);
		}

		// Draw the divider line
		dc.MoveTo(rect.right, rect.top);
		dc.LineTo(rect.right, rect.bottom);

		// Draw the 'currently edited pattern' indicator
		if (pat == owner->GetPattern() && i <= order_count) {
			CRect rc(rect.right-6, rect.top+5, rect.right-8, rect.top+7);
			dc.FillSolidRect(rc, colorShadow);
		}

		// Past the end, no more work required
		if (i > order_count) continue;

		// Draw the 'cursor'
		if (i == select_from) {
			CRect rc = rect;
			dc.SetBkColor(InvertRGB(colorHighlight));
			dc.SetTextColor(0);

			//rc.InflateRect(-1, -1);
			dc.DrawFocusRect(&rc);

			rc.InflateRect(-1, -1);
			dc.DrawFocusRect(&rc);
			rc.InflateRect(-1, -1);
			dc.DrawFocusRect(&rc);
		}

		// Draw the 'queue' indicator
		if (i == queue_index) {
			CRect rc(rect.left+4, rect.top+2, rect.right-4, rect.top+4);
			if (bHighLight) {
				dc.FillSolidRect(rc, colorWindow);
			} else {
				dc.FillSolidRect(rc, colorHighlight);
			}
		}

		// Draw the 'playing' indicator
		if (i == playing_index) {
			CRect rc(rect.left+4, rect.bottom-4, rect.right-4, rect.bottom-2);
			if (bHighLight) {
				dc.FillSolidRect(rc, colorWindow);
			} else {
				dc.FillSolidRect(rc, colorHighlight);
			}
		}

		// Draw the 'begin loop' indicator
		if (i == loop_start_index) {
			CRect rc(rect.left+3, rect.top+3, rect.left+5, rect.bottom-3);
			COLORREF color = bHighLight ? colorTextLight : colorTextSelLight;
			dc.FillSolidRect(rc, color);
			dc.SetPixel(rc.left+2, rc.top, color);
			dc.SetPixel(rc.left+2, rc.bottom-1, color);
		}

		// Draw the 'end loop' indicator
		if (i == loop_end_index) {
			CRect rc(rect.right-3, rect.top+3, rect.right-5, rect.bottom-3);
			COLORREF color = bHighLight ? colorTextLight : colorTextSelLight;
			dc.FillSolidRect(rc, color);
			dc.SetPixel(rc.left-3, rc.top, color);
			dc.SetPixel(rc.left-3, rc.bottom-1, color);
		}

		// Draw the 'drop position'
		if ((drag_mode == drag_mode_valid) && (i == drop_pos)) {
			CRect rc;
			rc.top = rect.top;
			rc.bottom = rect.bottom;

			if (i != 0) {
				rc.left = rect.left - 3;
				rc.right = rect.left + 3;
			} else {
				rc.left = rect.left;
				rc.right = rect.left + 4;
			}

			dc.FillSolidRect(&rc, colorText);
		}

		// Draw the label
		if (order_type == zzub_order_index_invalid) {
			strcpy(&bytes.front(), "---");
		} else
		if (order_type == zzub_order_index_skip) {
			strcpy(&bytes.front(), "+++");
		} else
		if (order_type == zzub_order_index_pattern) {
			if (pat) {
				strncpy(&bytes.front(), zzub_pattern_get_name(pat), 128);
			} else {
				strcpy(&bytes.front(), "---");
			}
		} else {
			strcpy(&bytes.front(), "???");
		}

		dc.SetTextColor(bHighLight ? colorTextSel : colorText);
		dc.DrawText(&bytes.front(), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	dc.SelectFont(prevfont);
}

void CPatternEditorOrderlist::InvalidateOrder(int nOrder)
{
	CRect rect;
	rect.left = nOrder * box_width;
	rect.top = 0;
	rect.right = rect.left + box_width;
	rect.bottom = box_height;

	clientDC->LPtoDP(&rect); // invalidate in DP
	InvalidateRect(&rect, FALSE);
}

void CPatternEditorOrderlist::InvalidateSelection()
{
	PE_order_selection sel = GetSortedSel();

	CRect rect;
	rect.left = sel.index_low * box_width;
	rect.top = 0;
	rect.right = rect.left + (box_width * sel.GetSelCount());
	rect.bottom = box_height;

	clientDC->LPtoDP(&rect);
	InvalidateRect(&rect, FALSE);
}

void CPatternEditorOrderlist::InvalidateDropPos()
{
	if (drop_pos == drop_pos_none) return;

	int from_box = max(0, drop_pos - 1);

	CRect rect;
	rect.left = from_box * box_width;
	rect.top = 0;
	rect.right = rect.left + (box_width * 2);
	rect.bottom = box_height;

	clientDC->LPtoDP(&rect);
	InvalidateRect(&rect, FALSE);
}

// ---------------------------------------------------------------------------------------------------------------
// MOUSE
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorOrderlist::StopDragging()
{
	drag_mode = drag_mode_off;
	InvalidateDropPos();
	drop_pos = drop_pos_none;
}

void CPatternEditorOrderlist::OnLButtonDown(UINT nFlags, CPoint pt)
{
	if (GetFocus() != m_hWnd) SetFocus();

	int nOrder = GetOrderFromPoint(pt);

	if (nOrder > zzub_player_get_order_length(buze_main_frame_get_player(mainFrame))) {
		EditAndSync();
		return;
	}

	if (IsAltDown()) {
		SetSelect(nOrder, sel_mode_single);
		CmdInsertEditorPattern();
		EditAndSync();
		//SyncPlayerToOrder();
	} else
	if (IsCtrlDown() && IsShiftDown()) {
		//
	} else
	if (IsCtrlDown()) {
		if (owner->follow_mode) {
			SetSelect(nOrder, sel_mode_single);
			EditAndSync();///
		}
		DoPlayPattern(nOrder);
	} else
	if (IsShiftDown()) {
		SetSelect(nOrder, sel_mode_to);
	} else {
		PE_order_selection sel = GetSortedSel();
		if ((nOrder < sel.index_low) || (nOrder > sel.index_high)) {
			SetSelect(nOrder, sel_mode_single);
		}
		EditAndSync();
		drag_mode = drag_mode_on;
	}

	SetCapture();
}

void CPatternEditorOrderlist::OnLButtonUp(UINT nFlags, CPoint pt)
{
	if (GetCapture() == m_hWnd) ReleaseCapture();

	CRect rect;
	GetClientRect(&rect);

	if (rect.PtInRect(pt)) {
		if (drag_mode == drag_mode_valid) {
			InvalidateDropPos();
			CmdDragSelection(IsShiftDown());
			//EditAndSync();///
		} else
		if (drag_mode == drag_mode_on) {
			int nOrder = GetOrderFromPoint(pt);
			if (select_from != select_to) {
				SetSelect(nOrder, sel_mode_single);
				EditAndSync();
			}
		}
	}

	drag_mode = drag_mode_off;
	drop_pos = drop_pos_none;
}

void CPatternEditorOrderlist::OnLButtonDblClk(UINT nFlags, CPoint pt)
{
	if (IsAltDown() || IsCtrlDown() || IsShiftDown()) {
		OnLButtonDown(nFlags, pt);
	} else {
		FocusPatternEditor();
	}
}

void CPatternEditorOrderlist::OnMButtonDown(UINT nFlags, CPoint pt)
{
	if (GetFocus() != m_hWnd) SetFocus();

	StopDragging();

	int nOrder = GetOrderFromPoint(pt);
	if (nOrder > zzub_player_get_order_length(buze_main_frame_get_player(mainFrame))) return;

	DoQueuePattern(nOrder);
}

void CPatternEditorOrderlist::OnXButtonDown(int fwButton, int dwKeys, CPoint ptPos)
{
	if ((fwButton & MK_XBUTTON2) == MK_XBUTTON2) {
		///
	}
}

void CPatternEditorOrderlist::OnMouseMove(UINT nFlags, CPoint pt)
{
	if (pt == mousemove_previous) return;
	mousemove_previous = pt;

	if (drag_mode == drag_mode_off) return;

	int new_drop_pos = drop_pos_none;
	drag_mode = drag_mode_moved;

	CRect rect;
	GetClientRect(&rect);

	if (rect.PtInRect(pt)) {
		int nOrder = GetDropOrderFromPoint(pt);

		PE_order_selection sel = GetSortedSel();
		int len = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));

		if ((nOrder <= len) && ((nOrder <= sel.index_low) || (nOrder >= sel.index_high + 1))) {
			new_drop_pos = nOrder;
			drag_mode = drag_mode_valid;
		}
	}

	if (drag_mode == drag_mode_valid) {
		//SetCursor(CMainFrame::curDragging);
	} else {
		//SetCursor(CMainFrame::curNoDrop);
	}

	if (new_drop_pos != drop_pos) {
		InvalidateDropPos();
		drop_pos = new_drop_pos;
		InvalidateDropPos();
	}
}

void CPatternEditorOrderlist::OnRButtonDown(UINT nFlags, CPoint pt)
{
	if (GetFocus() != m_hWnd) SetFocus();

	if (drag_mode != drag_mode_off) {
		StopDragging();
	}

	if (pt.y >= box_height) return;

	int nOrder = GetOrderFromPoint(pt);
	int len = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));
	if (nOrder > len) return;

	PE_order_selection sel = GetSortedSel();
	if ((nOrder < sel.index_low) || (nOrder > sel.index_high)) {
		SetSelect(nOrder, sel_mode_single);
		EditAndSync();
	}

	CBuzeConfiguration configuration = buze_document_get_configuration(document);

	CMenu selformat_menu;
	{
		selformat_menu.CreatePopupMenu();

		zzub_pattern_format_iterator_t* fit = zzub_player_get_pattern_format_iterator(buze_main_frame_get_player(mainFrame));
		while (zzub_pattern_format_iterator_valid(fit)) {
			zzub_pattern_format_t* format = zzub_pattern_format_iterator_current(fit);
			const char* name = zzub_pattern_format_get_name(format);
			int id = zzub_pattern_format_get_id(format);
			selformat_menu.AppendMenu(MF_STRING, (UINT_PTR)(ID_ORDERLIST_NEW_SETFORMAT_FIRST + id), name);
			zzub_pattern_format_iterator_next(fit);
		}
		zzub_pattern_format_iterator_destroy(fit);
	}

	CMenu selpattern_menu;
	std::vector<CMenuHandle> subformat_menus;
	{
		selpattern_menu.CreatePopupMenu();

		int nFormat = 0;
		const char* name;
		zzub_pattern_format_iterator_t* fit = zzub_player_get_pattern_format_iterator(buze_main_frame_get_player(mainFrame));
		while (zzub_pattern_format_iterator_valid(fit)) {
			subformat_menus.push_back(CMenuHandle());
			subformat_menus.back().CreatePopupMenu();		

			zzub_pattern_format_t* format = zzub_pattern_format_iterator_current(fit);

			int pattern_count = zzub_player_get_pattern_count(buze_main_frame_get_player(mainFrame));
			for (int i = 0; i < pattern_count; ++i) {
				zzub_pattern_t* pat = zzub_player_get_pattern_by_index(buze_main_frame_get_player(mainFrame), i);
				zzub_pattern_format_t* found_format = zzub_pattern_get_format(pat);
				if (found_format != format) continue;
				int id = zzub_pattern_get_id(pat);
				name = zzub_pattern_get_name(pat);
				subformat_menus.back().AppendMenu(MF_STRING, (UINT_PTR)(ID_ORDERLIST_SETPATTERN_FIRST + id), name);
			}

			name = zzub_pattern_format_get_name(format);
			selpattern_menu.AppendMenu(MF_STRING, subformat_menus.back(), name);
			zzub_pattern_format_iterator_next(fit);
		}
		zzub_pattern_format_iterator_destroy(fit);

		selpattern_menu.AppendMenu(MF_STRING, (UINT_PTR)(ID_ORDERLIST_SETPATTERN_FIRST + 0), "---");
	}

	CMenu menu;
	{
		menu.CreatePopupMenu();

		DWORD greyed = (len == 0 || nOrder == len) ? MF_GRAYED : 0;

		if (select_from != select_to) {
			menu.AppendMenu(MF_STRING, ID_ORDERLIST_INSERT, "&Insert Orders");
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_DUPLICATE, "Cl&one Patterns");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_REMOVE, "&Remove Orders");
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_REMOVE_DELETE, "&Delete Patterns");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING | greyed, selpattern_menu, "&Select Pattern...");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, ID_EDIT_COPY, "&Copy Orders");
			menu.AppendMenu(MF_STRING, ID_EDIT_CUT, "C&ut Orders");
			menu.AppendMenu(MF_STRING, ID_EDIT_PASTE, "&Paste Orders");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_SETSELECTIONLOOP, "Set &Loop");
		} else {
			menu.AppendMenu(MF_STRING, ID_ORDERLIST_INSERT, "&Insert Order");
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_DUPLICATE, "Cl&one Pattern");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, ID_ORDERLIST_NEW, "&New Pattern");
			//menu.AppendMenu(MF_STRING, ID_ORDERLIST_NEW_NEWFORMAT, "New Pattern + New &Format");
			menu.AppendMenu(MF_STRING, selformat_menu, "New &Pattern...");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_REMOVE, "&Remove Order");
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_REMOVE_DELETE, "&Delete Pattern");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING | greyed, selpattern_menu, "&Select Pattern...");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, ID_EDIT_COPY, "&Copy Order");
			menu.AppendMenu(MF_STRING, ID_EDIT_CUT, "C&ut Order");
			menu.AppendMenu(MF_STRING, ID_EDIT_PASTE, "&Paste Order");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_PROPERTIES, "Pattern Pr&operties");
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_RENDER, "Render to &Wave");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_QUEUE, "&Queue");
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_SETBEGINLOOP, "Set Loop &Begin");
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_SETENDLOOP, "Set Loop &End");
			menu.AppendMenu(MF_STRING | greyed, ID_ORDERLIST_PLAYORDER, "&Play");
		}

		if (configuration->getShowAccelerators())
			buze_main_frame_add_menu_keys(mainFrame, "orderlist", menu);
	}

	ClientToScreen(&pt);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd, 0);
}

// ---------------------------------------------------------------------------------------------------------------
// KEYBOARD
// ---------------------------------------------------------------------------------------------------------------

BOOL CPatternEditorOrderlist::PreTranslateMessage(MSG* pMsg)
{
	if (GetFocus() == m_hWnd) {
// 		if (pMsg->message == WM_KEYDOWN && (pMsg->lParam & (0x1 << 30))) {
// 			// block accelerators repeating at keyboard repeat-rate
// 		} else
		HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainFrame, "orderlist");
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg)) {
			return TRUE;
		}
	}
	return FALSE;
}

void CPatternEditorOrderlist::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (true
		&& drag_mode != drag_mode_off
		&& nChar != VK_SHIFT
		&& nChar != VK_CONTROL
		&& nChar != VK_MENU
	) {
		StopDragging();
		return;
	}

	int mode = IsShiftDown() ? sel_mode_from : sel_mode_single;

	bool is_repeat = (nFlags & (0x1 << 14)); // 14th bit is previous state
	got_repeat |= is_repeat;

	switch (nChar)
	{
		// motion
		case VK_RIGHT:
			SetSelect(select_from + 1, mode);
			EnsureVisibleCursor();
			if (!is_repeat) EditAndSync();
			break;
		case VK_LEFT:
			SetSelect(select_from - 1, mode);
			EnsureVisibleCursor();
			if (!is_repeat) EditAndSync();
			break;
		case VK_HOME:
			SetSelect(0, mode);
			EnsureVisibleCursor();
			EditAndSync();
			break;
		case VK_END:
			SetSelect(zzub_player_get_order_length(buze_main_frame_get_player(mainFrame)), mode);
			EnsureVisibleCursor();
			EditAndSync();
			break;

		// mutational
		case VK_UP:
			CmdCyclePattern(+1, false);
			if (!is_repeat) EditAndSync();///true);
			break;
		case VK_DOWN:
			CmdCyclePattern(-1, false);
			if (!is_repeat) EditAndSync();///true);
			break;

		// x
		default:
			SetMsgHandled(FALSE);
			break;
	}
}

void CPatternEditorOrderlist::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar) {
		case VK_RIGHT:
		case VK_LEFT:
		case VK_UP:
		case VK_DOWN:
			if (got_repeat) {
				EditAndSync();///true);
				got_repeat = false;
			}
			break;

		default:
			SetMsgHandled(FALSE);
			break;
	}
}

// ---------------------------------------------------------------------------------------------------------------
// PATTERN EDITOR CONNECTIVITY / FOLLOW SYNC
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorOrderlist::MoveRight()
{
	SetSelect(select_from + 1, sel_mode_single);
	EnsureVisibleCursor();
	EditAndSync();
}

void CPatternEditorOrderlist::MoveLeft()
{
	SetSelect(select_from - 1, sel_mode_single);
	EnsureVisibleCursor();
	EditAndSync();
}

// SyncOrderToPlayer, SyncPlayerToOrder, EditAndSync were tricky.
// They kinda freak me out. They should maybe be done a different way.

void CPatternEditorOrderlist::SyncOrderToPlayer(int nOrder)
{
	if (owner->follow_mode == true) {
		//if (nOrder != select_from) {
		//	if (IsShiftDown() && (GetFocus() == m_hWnd))
		//		SetSelect(nOrder, sel_mode_from);
		//	else
				SetSelect(nOrder, sel_mode_single);
			EnsureVisibleCursor();
			EditPattern(select_from);
		//}
	}
}

void CPatternEditorOrderlist::SyncPlayerToOrder(bool force /*= false*/)
{
	int playing_order = zzub_player_get_position_order(buze_main_frame_get_player(mainFrame));
	zzub_pattern_t* pat = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), select_from);

	if (select_from != playing_order || pat != owner->GetPattern() || force) { // the logic here is gray
		zzub_player_set_position(buze_main_frame_get_player(mainFrame), select_from, 0);
	}
}

void CPatternEditorOrderlist::EditAndSync(bool force /*= false*/)
{
	if (owner->follow_mode == true) {
		SyncPlayerToOrder(force);
	} else {
		EditPattern(select_from);
	}
}



void CPatternEditorOrderlist::EditPattern(int nOrder)
{
	zzub_pattern_t* pat = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), nOrder);
	owner->SetPattern(pat);
}

void CPatternEditorOrderlist::FocusPatternEditor()
{
	owner->SetFocus();
}

// ---------------------------------------------------------------------------------------------------------------
// COMMANDS
// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorOrderlist::OnEditCut()
{
	DoCopyOrders();
	DoRemoveOrDeleteOrders(false);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Cut Order(s)");

	EditAndSync();
}

void CPatternEditorOrderlist::OnEditCopy()
{
	DoCopyOrders();
}

void CPatternEditorOrderlist::OnEditPaste()
{
	DoPasteOrders();
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Paste Order(s)");

	EditAndSync(/*true*/);
}

void CPatternEditorOrderlist::OnInsertOrder()
{
	DoInsertOrders();
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Insert Order(s)");

	EditAndSync();
}

void CPatternEditorOrderlist::OnCreatePattern()
{
	DoCreatePattern(false, -1);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Create Order Pattern");

	EnsureVisibleCursorRange();
	EditAndSync();
}

void CPatternEditorOrderlist::OnCreatePatternSetFormat(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int id = (nID - ID_ORDERLIST_NEW_SETFORMAT_FIRST);
	DoCreatePattern(false, id);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Create Order Pattern");

	EnsureVisibleCursorRange();
	EditAndSync();
}

void CPatternEditorOrderlist::OnCreatePatternNewFormat()
{
	DoCreatePattern(true, -1);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Create Order Pattern+Format");

	EnsureVisibleCursorRange();
	EditAndSync();
}

void CPatternEditorOrderlist::OnDuplicatePattern()
{
	DoDuplicatePatterns();
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Duplicate Order Pattern(s)");

	EnsureVisibleCursorRange();
	EditAndSync();
}

void CPatternEditorOrderlist::OnRemoveOrder()
{
	DoRemoveOrDeleteOrders(false);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Remove Order(s)");
	EditAndSync();
}

void CPatternEditorOrderlist::OnRemovePattern()
{
	DoRemoveOrDeleteOrders(true);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Delete Order Pattern(s)");
	EditAndSync();
}

void CPatternEditorOrderlist::OnBackspaceOrder()
{
	SetSelect(select_from - 1, sel_mode_single);
	DoRemoveOrDeleteOrders(false);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Remove Order(s)");
	EnsureVisibleCursor();
	EditAndSync();
}

void CPatternEditorOrderlist::OnBackspacePattern()
{
	SetSelect(select_from - 1, sel_mode_single);
	DoRemoveOrDeleteOrders(true);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Delete Order Pattern(s)");

	EnsureVisibleCursor();
	EditAndSync();
}

void CPatternEditorOrderlist::OnSetPattern(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int id = (nID - ID_ORDERLIST_SETPATTERN_FIRST);
	DoSetPatterns(id);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Set Order Pattern(s)");

	{	int playing_index = zzub_player_get_position_order(buze_main_frame_get_player(mainFrame));///
		PE_order_selection sel = GetSortedSel();
		if (playing_index >= sel.index_low && playing_index <= sel.index_high)
			zzub_player_set_position(buze_main_frame_get_player(mainFrame), playing_index, 0);///
	}

	EditAndSync();///true);
}

void CPatternEditorOrderlist::OnSetBeginLoop()
{
	DoSetBeginLoop(select_from);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Set Loop Start Order");
}

void CPatternEditorOrderlist::OnSetEndLoop()
{
	DoSetEndLoop(select_from);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Set Loop End Order");
}

void CPatternEditorOrderlist::OnSetSelectionLoop()
{
	PE_order_selection sel = GetSortedSel();
	DoSetBeginLoop(sel.index_low);
	DoSetEndLoop(sel.index_high);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Set Loop Start/End Order");
}

// ---

void CPatternEditorOrderlist::OnQueueOrder()
{
	DoQueuePattern(select_from);
}

void CPatternEditorOrderlist::OnPlayOrder()
{
	DoPlayPattern(select_from);
}

void CPatternEditorOrderlist::OnDeselect()
{
	SetSelect(select_from, sel_mode_single);
}

void CPatternEditorOrderlist::OnGotoEditor()
{
	SetSelect(select_from, sel_mode_single);
	FocusPatternEditor();
}

void CPatternEditorOrderlist::OnToggleFollow()
{
	owner->OnToggleFollow();
}

// ---

void CPatternEditorOrderlist::OnRenderOrder()
{
	zzub_plugin_t* masterplugin = zzub_player_get_plugin_by_name(buze_main_frame_get_player(mainFrame), "Master");
	if (masterplugin == 0) return ;

	zzub_pattern_t* pat = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), select_from);
	if (pat == 0) return ;

	std::string filename = GetMixdownFileName("", m_hWnd);
	if (filename.empty()) return ;

	zzub_player_history_enable(buze_main_frame_get_player(mainFrame), 0);

	buze_application_t* application = buze_main_frame_get_application(mainFrame);
	mixdownhelper md(application, buze_main_frame_get_player(mainFrame), masterplugin);
	bool success = md.init_file_recorder(filename);

	if (success) {
		buze_application_show_wait_window(application);
		buze_application_set_wait_text(application, "Recording...");

		zzub_audiodriver_t* driver = (zzub_audiodriver_t*)buze_application_get_audio_driver(application);
		int samplerate = zzub_audiodriver_get_samplerate(driver);
		success = md.mixdown(samplerate, select_from);

		buze_application_hide_wait_window(application, m_hWnd);
	}

	md.uninit();

	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Mixdown To File");
	zzub_player_history_enable(buze_main_frame_get_player(mainFrame), 1);

	if (!success)
		MessageBox("Mixdown fail.");
}

void CPatternEditorOrderlist::OnPatternProperties()
{
	if (select_from >= zzub_player_get_order_length(buze_main_frame_get_player(mainFrame))) return;

	zzub_pattern_t* pat = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), select_from);

	if (pat == 0) return ;
	assert(false);
/*
	mainFrame->showPropertyView();
	if (document->selectedPropertyProvider) delete document->selectedPropertyProvider;
	document->selectedPropertyProvider = new CPatternPropertyProvider(*this, document, pat);
	document->updateAllViews(0, UpdateProperties);*/
}

void CPatternEditorOrderlist::CmdInsertEditorPattern()
{
	DoInsertEditorPattern();

	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Set Order Pattern(s)");
}

// ---------------------------------------------------------------------------------------------------------------
// HELPERS
// ---------------------------------------------------------------------------------------------------------------

void zzub_player_get_orders(zzub_player_t* player, int index, zzub_pattern_t** orders, int numorders) {
	int count = 0;
	zzub_pattern_iterator_t* patit = zzub_player_get_order_iterator(player);
	while (zzub_pattern_iterator_valid(patit) && count < index + numorders) {
		zzub_pattern_t* pat = zzub_pattern_iterator_current(patit);
		if (count >= index) 
			orders[count - index] = pat;
		zzub_pattern_iterator_next(patit);
		count++;
	}
	zzub_pattern_iterator_destroy(patit);
}

void zzub_player_insert_orders(zzub_player_t* player, int index, zzub_pattern_t** orders, int insertcount) {
	zzub_player_timeshift_order(player, index, insertcount);
	
	int orderlen = zzub_player_get_order_length(player);
	zzub_player_set_order_length(player, orderlen + insertcount);

	int movedpatterncount = orderlen - index;
	if (movedpatterncount > 0) {
		std::vector<zzub_pattern_t*> movedpatterns(movedpatterncount);
		zzub_player_get_orders(player, index, &movedpatterns.front(), movedpatterncount);

		for (int i = 0; i < movedpatterncount; i++)
			zzub_player_set_order_pattern(player, index + insertcount + i, movedpatterns[i]);
	}

	for (int i = 0; i < insertcount; i++)
		zzub_player_set_order_pattern(player, index + i, orders[i]);
}

void zzub_player_delete_orders(zzub_player_t* player, int index, int deletecount) {
	zzub_player_timeshift_order(player, index, -deletecount);

	int orderlen = zzub_player_get_order_length(player);

	int movedpatterncount = orderlen - index + deletecount;
	std::vector<zzub_pattern_t*> movedpatterns(movedpatterncount);
	zzub_player_get_orders(player, index + deletecount, &movedpatterns.front(), movedpatterncount);

	for (int i = 0; i < movedpatterncount; i++)
		zzub_player_set_order_pattern(player, index + i, movedpatterns[i]);

	zzub_player_set_order_length(player, orderlen - deletecount);
}

void CPatternEditorOrderlist::DoCopyOrders()
{
	std::stringstream strm;

	PE_order_selection sel = GetSortedSel();

	int sel_size = sel.GetSelCount();
	strm << sel_size << std::endl;

	int len = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));

	for (int i = sel.index_low; i <= sel.index_high; ++i) {
		zzub_pattern_t* pat = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), i);
		int patid = pat ? zzub_pattern_get_id(pat) : 0;
		strm << patid << std::endl;
	}

	std::string out = strm.str();
	CopyBinary(m_hWnd, "Buze:OrderSelection", out.c_str(), out.length());
}

void CPatternEditorOrderlist::DoPasteOrders()
{
	UINT format = RegisterClipboardFormat("Buze:OrderSelection");
	if (!OpenClipboard()) return;

	HANDLE hData = GetClipboardData(format);
	if (hData == 0) {
		CloseClipboard();
		return;
	}

	char* charbuf = (char*)GlobalLock(hData);
	int bufferSize = GlobalSize(hData);

	if (charbuf != 0)
	{
		std::stringstream strm(charbuf);
		int paste_len;
		strm >> paste_len;
		std::vector<zzub_pattern_t*> insertlist;
		for (int i = 0; i < paste_len; ++i) {
			int patid;
			strm >> patid;
			zzub_pattern_t* pat = patid ? zzub_player_get_pattern_by_id(buze_main_frame_get_player(mainFrame), patid) : 0;
			insertlist.push_back(pat);
		}

		PE_order_selection sel = GetSortedSel();

		zzub_player_insert_orders(buze_main_frame_get_player(mainFrame), sel.index_low, &insertlist.front(), (int)insertlist.size());
		int from = sel.index_low;
		int to = from + (insertlist.size() - 1);

		SetSelectRange(from, to);

		Invalidate(FALSE);
	}

	GlobalUnlock(hData);
	CloseClipboard();
}

void CPatternEditorOrderlist::DoCreatePattern(bool newformat, int id)
{
	int len = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));
	if (select_from > len) return;

	zzub_pattern_format_t* format;
	if (newformat) {
		format = zzub_player_create_pattern_format(buze_main_frame_get_player(mainFrame), 0);
	} else
	if (id == -1) {
		zzub_pattern_t* pattern = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), select_from);
		if (pattern == 0) {
			format = zzub_player_get_pattern_format_by_id(buze_main_frame_get_player(mainFrame), 1);
			if (format == 0) {
				format = zzub_player_create_pattern_format(buze_main_frame_get_player(mainFrame), 0);
			}
		} else {
			format = zzub_pattern_get_format(pattern);
		}
	} else {
		format = zzub_player_get_pattern_format_by_id(buze_main_frame_get_player(mainFrame), id);
	}

	CBuzeConfiguration configuration = buze_document_get_configuration(document);
	int pattern_length = configuration->getGlobalPatternLength();

	char const* description = 0;
	if (configuration->getPatternNamingMode() == 1) {
		description = zzub_pattern_format_get_name(format);
	}
	zzub_pattern_t* newpattern = zzub_player_create_pattern(buze_main_frame_get_player(mainFrame), format, description, pattern_length);

	int insert_after_offset; // insert_after_offset controls if we insert before or after the selected position
	std::vector<zzub_pattern_t*> insertlist;
	insertlist.push_back(newpattern);

	if (len == 0) {
		//orderlist.push_back(newpattern);
		insert_after_offset = 0;
	} else {
		insert_after_offset = 1;
	}

	int insert_pos = select_from + insert_after_offset;

	zzub_player_insert_orders(buze_main_frame_get_player(mainFrame), insert_pos, &insertlist.front(), 1);

	SetSelect(insert_pos, sel_mode_single);

	Invalidate(FALSE);
}

void CPatternEditorOrderlist::DoDuplicatePatterns()
{
	PE_order_selection sel = GetSortedSel();

	int len = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));
	CBuzeConfiguration configuration = buze_document_get_configuration(document);

	std::vector<zzub_pattern_t*> insertlist;
	int idx = 0;

	zzub_pattern_iterator_t* patit = zzub_player_get_order_iterator(buze_main_frame_get_player(mainFrame));
	while (zzub_pattern_iterator_valid(patit)) {
		zzub_pattern_t* pat = zzub_pattern_iterator_current(patit);
		if (idx >= sel.index_low && idx <= sel.index_high) {
			zzub_pattern_t* clonepat;
			if (pat) {
				char const* description = 0;
				if (configuration->getPatternNamingMode() == 1) {
					description = zzub_pattern_get_name(pat);
				}
				clonepat = zzub_player_clone_pattern(buze_main_frame_get_player(mainFrame), pat, description);
			} else {
				clonepat = 0;
			}
			insertlist.push_back(clonepat);
		}
		zzub_pattern_iterator_next(patit);
		++idx;
	}
	zzub_pattern_iterator_destroy(patit);

	if (sel.index_high == len) {
		insertlist.push_back(0);
	}

	int insert_after_offset = sel.GetSelCount();
	if (len == 0) {
		insert_after_offset = 0;
	}

	int insert_pos = sel.index_low + insert_after_offset;
	zzub_player_insert_orders(buze_main_frame_get_player(mainFrame), insert_pos, &insertlist.front(), (int)insertlist.size());
	
	int from = insert_pos;
	int to = from + (sel.GetSelCount() - 1);
	if (sel.index_low == select_from) {
		SetSelectRange(from, to);
	} else {
		SetSelectRange(to, from);
	}

	Invalidate(FALSE);
}

void CPatternEditorOrderlist::DoInsertOrders()
{
	PE_order_selection sel = GetSortedSel();
	int selcount = sel.index_high - sel.index_low + 1;
	std::vector<zzub_pattern_t*> insertlist(selcount);
	zzub_player_get_orders(buze_main_frame_get_player(mainFrame), sel.index_low, &insertlist.front(), selcount);
	zzub_player_insert_orders(buze_main_frame_get_player(mainFrame), sel.index_low, &insertlist.front(), selcount);

	Invalidate(FALSE);
}

void CPatternEditorOrderlist::DoInsertEditorPattern()
{
	PE_order_selection sel = GetSortedSel();
	std::vector<zzub_pattern_t*> insertlist;
	insertlist.push_back(owner->GetPattern());
	zzub_player_insert_orders(buze_main_frame_get_player(mainFrame), sel.index_low, &insertlist.front(), 1);
	Invalidate(FALSE);
}

void CPatternEditorOrderlist::DoRemoveOrDeleteOrders(bool deletepat)
{
	PE_order_selection sel = GetSortedSel();

	std::vector<zzub_pattern_t*> deletelist;
	if (deletepat) {
		int idx = 0;
		zzub_pattern_iterator_t* patit = zzub_player_get_order_iterator(buze_main_frame_get_player(mainFrame));
		while (zzub_pattern_iterator_valid(patit)) {
			zzub_pattern_t* pat = zzub_pattern_iterator_current(patit);
			if (idx >= sel.index_low && idx <= sel.index_high) {
				if (pat) deletelist.push_back(pat);
			}
			zzub_pattern_iterator_next(patit);
			++idx;
		}
		zzub_pattern_iterator_destroy(patit);

		std::vector<zzub_pattern_t*>::iterator endunique = std::unique(deletelist.begin(), deletelist.end());
		for (std::vector<zzub_pattern_t*>::iterator i = deletelist.begin(); i != endunique; ++i) {
			if (*i != 0)
				zzub_pattern_destroy(*i);
		}
	}

	int deletecount = sel.index_high - sel.index_low + 1;
	zzub_player_delete_orders(buze_main_frame_get_player(mainFrame), sel.index_low, deletecount);

	int len = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));

	if (sel.index_low <= len) {
		SetSelect(sel.index_low, sel_mode_single);
	} else {
		SetSelect(len - 1, sel_mode_single);
	}

	Invalidate(FALSE);
}

void CPatternEditorOrderlist::DoSetPatterns(int id)
{
	int len = zzub_player_get_order_length(buze_main_frame_get_player(mainFrame));
	if (len == 0) return;

	zzub_pattern_t* pat = id ? zzub_player_get_pattern_by_id(buze_main_frame_get_player(mainFrame), id) : 0;

	PE_order_selection sel = GetSortedSel();
	for (int i = sel.index_low; i <= sel.index_high; ++i) {
		if (i == len) break;
		zzub_player_set_order_pattern(buze_main_frame_get_player(mainFrame), i, pat);
	}

	InvalidateSelection();
}

void CPatternEditorOrderlist::DoSetBeginLoop(int nOrder)
{
	zzub_pattern_t* pat = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), nOrder);
	if (pat == 0) return;

	int loop_start_index = zzub_player_get_order_loop_start(buze_main_frame_get_player(mainFrame));
	InvalidateOrder(loop_start_index);
	InvalidateOrder(nOrder);

	zzub_player_set_order_loop_start(buze_main_frame_get_player(mainFrame), nOrder);
}

void CPatternEditorOrderlist::DoSetEndLoop(int nOrder)
{
	zzub_pattern_t* pat = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), nOrder);
	if (pat == 0) return;

	int loop_end_index = zzub_player_get_order_loop_end(buze_main_frame_get_player(mainFrame));
	InvalidateOrder(loop_end_index);
	InvalidateOrder(nOrder);

	zzub_player_set_order_loop_end(buze_main_frame_get_player(mainFrame), nOrder);
}

// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorOrderlist::CmdDragSelection(bool makecopy)
{
	PE_order_selection sel = GetSortedSel();

	if (!makecopy && ((drop_pos == sel.index_low) || (drop_pos == sel.index_high + 1))) return;

	int offset = 0;
	if (!makecopy && (drop_pos > select_from)) {
		// subtract the chunk cause it wont be there after it moved
		offset = -sel.GetSelCount();
	}
	int insert_pos = drop_pos + offset;

	int playing_index = zzub_player_get_position_order(buze_main_frame_get_player(mainFrame));///
	int queue_index = zzub_player_get_queue_order_index(buze_main_frame_get_player(mainFrame));
	int loop_start_index = zzub_player_get_order_loop_start(buze_main_frame_get_player(mainFrame));
	int loop_end_index = zzub_player_get_order_loop_end(buze_main_frame_get_player(mainFrame));

	std::vector<PE_order_pos> orderlist;
	std::vector<PE_order_pos> insertlist;
	{
		int idx = 0;
		zzub_pattern_iterator_t* patit = zzub_player_get_order_iterator(buze_main_frame_get_player(mainFrame));
		while (zzub_pattern_iterator_valid(patit)) {
			zzub_pattern_t* pat = zzub_pattern_iterator_current(patit);

			PE_order_pos order_pos;
			order_pos.pat           = pat;
			order_pos.is_playing    = (idx == playing_index);
			order_pos.is_queue      = (idx == queue_index);
			order_pos.is_begin_loop = (idx == loop_start_index);
			order_pos.is_end_loop   = (idx == loop_end_index);

			if (idx >= sel.index_low && idx <= sel.index_high) {
				insertlist.push_back(order_pos);
				if (makecopy) {
					PE_order_pos order_pos_src;
					order_pos_src.pat = pat;
					orderlist.push_back(order_pos_src);
				}
			} else {
				orderlist.push_back(order_pos);
			}

			zzub_pattern_iterator_next(patit);
			++idx;
		}
		zzub_pattern_iterator_destroy(patit);
	}

	if (sel.index_high == zzub_player_get_order_length(buze_main_frame_get_player(mainFrame)))
		insertlist.push_back(PE_order_pos());

	orderlist.insert(orderlist.begin() + (size_t)insert_pos, insertlist.begin(), insertlist.end());

	{
		zzub_player_set_order_length(buze_main_frame_get_player(mainFrame), orderlist.size());
		for (int i = 0; i < orderlist.size(); ++i) {
			PE_order_pos const& order_pos = orderlist[i];

			zzub_player_set_order_pattern(buze_main_frame_get_player(mainFrame), i, order_pos.pat);

			if (order_pos.is_begin_loop && i != loop_start_index)
				zzub_player_set_order_loop_start(buze_main_frame_get_player(mainFrame), i);
			if (order_pos.is_end_loop && i != loop_end_index)
				zzub_player_set_order_loop_end(buze_main_frame_get_player(mainFrame), i);

			if (order_pos.is_playing && i != playing_index)
				zzub_player_adjust_position_order(buze_main_frame_get_player(mainFrame), i);///
			if (order_pos.is_queue && i != queue_index)
				zzub_player_set_queue_order_index(buze_main_frame_get_player(mainFrame), i);
		}
	}

	int from = insert_pos;
	int to = from + (sel.GetSelCount() - 1);
	if (sel.index_low == select_from) {
		SetSelectRange(from, to);
	} else {
		SetSelectRange(to, from);
	}

	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Move Order(s)");

	Invalidate(FALSE);	
}

void CPatternEditorOrderlist::CmdCyclePattern(int n, bool same_format)
{
	SetSelect(select_from, sel_mode_single);
	zzub_pattern_t* sel_pat = zzub_player_get_order_pattern(buze_main_frame_get_player(mainFrame), select_from);

	zzub_pattern_format_t* this_format = 0;
	if (sel_pat != 0 && same_format)
		this_format = zzub_pattern_get_format(sel_pat);
	else
		same_format = false;

	std::vector<zzub_pattern_t*> patterns;

	zzub_pattern_iterator_t* patit = zzub_player_get_pattern_iterator(buze_main_frame_get_player(mainFrame));
	while (zzub_pattern_iterator_valid(patit)) {
		zzub_pattern_t* pat = zzub_pattern_iterator_current(patit);
		bool got_pat = true;
		if (same_format) {
			zzub_pattern_format_t* fmt = zzub_pattern_get_format(pat);
			if (fmt != this_format) got_pat = false;
		}
		if (got_pat) patterns.push_back(pat);
		zzub_pattern_iterator_next(patit);
	}
	zzub_pattern_iterator_destroy(patit);

	patterns.push_back(0);

	std::vector<zzub_pattern_t*>::iterator i = std::find(patterns.begin(), patterns.end(), sel_pat);
	if (n == +1) {
		if (i == patterns.end() - 1)
			i = patterns.begin();
		else
			++i;
	} else
	if (n == -1) {
		if (i == patterns.begin())
			i = patterns.end() - 1;
		else
			--i;
	}

	zzub_player_set_order_pattern(buze_main_frame_get_player(mainFrame), select_from, *i);
	zzub_player_history_commit(buze_main_frame_get_player(mainFrame), 0, 0, "Change Order Pattern");

	int playing_index = zzub_player_get_position_order(buze_main_frame_get_player(mainFrame));///
	if (playing_index == select_from)
		zzub_player_set_position(buze_main_frame_get_player(mainFrame), select_from, 0);

	InvalidateSelection();
}

// ---------------------------------------------------------------------------------------------------------------

void CPatternEditorOrderlist::DoQueuePattern(int nOrder)
{
	int prev_queue = zzub_player_get_queue_order_index(buze_main_frame_get_player(mainFrame));

	if (nOrder == prev_queue) {
		zzub_player_set_queue_order_index(buze_main_frame_get_player(mainFrame), -1); // unqueue
	} else {
		InvalidateOrder(prev_queue);
		zzub_player_set_queue_order_index(buze_main_frame_get_player(mainFrame), nOrder); // queue
	}

	InvalidateOrder(nOrder);
}

void CPatternEditorOrderlist::DoPlayPattern(int nOrder)
{
	if (nOrder < zzub_player_get_order_length(buze_main_frame_get_player(mainFrame))) {
		zzub_player_set_position(buze_main_frame_get_player(mainFrame), nOrder, 0);
		zzub_player_set_state(buze_main_frame_get_player(mainFrame), zzub_player_state_playing, -1);
	}

	InvalidateOrder(nOrder);
}
