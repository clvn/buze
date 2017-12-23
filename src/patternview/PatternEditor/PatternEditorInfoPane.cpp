#include "stdafx.h"
#include "resource.h"
#include "utils.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "Configuration.h"
#include "BuzeConfiguration.h"
#include "PatternEditorInfoPane.h"
#include "PatternView.h"

// for loading IDB_FOOBAR with proper colors
bool CreateImageList(WTL::CImageList &il, UINT nIDResource, int cx, int nGrow, COLORREF crMask);

CInfoPane::CInfoPane(CViewFrame* mainFrame, CPatternView* owner)
:
	mainFrame(mainFrame),
	owner(owner),
	document(owner->document),
	player(buze_main_frame_get_player(mainFrame))
{
	currentWnd = false;
}

CInfoPane::~CInfoPane() {
	if (m_hWnd) {
		DestroyWindow();
	}
}

int CInfoPane::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	columnList.Create(m_hWnd, rcDefault, NULL, WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS, WS_EX_STATICEDGE|LVS_EX_DOUBLEBUFFER, IDC_COLUMNLIST);///WS_EX_CLIENTEDGE
	columnList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	columnList.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	columnList.AddColumn("Key", 0);
	columnList.AddColumn("Value", 1);
	columnList.SetColumnWidth(0, 40);
	columnList.SetColumnWidth(1, 126);

	patternTree.Create(m_hWnd, rcDefault, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VSCROLL, WS_EX_STATICEDGE, IDC_PATTERNTREE);
	patternTree.ModifyStyle(0, TVS_HASLINES|TVS_NOHSCROLL|TVS_EDITLABELS);//|TVS_FULLROWSELECT);
	//patternTree.SetScrollTime(0);
	CreateImageList(m_TreeImages, IDB_PATTERNLIST_IMAGES, 16, 2, RGB(0,255,255));
	patternTree.Initialize(FALSE, FALSE);
	patternTree.SetImageList(m_TreeImages, TVSIL_NORMAL);
	patternTree.SetSmartCheckBox(FALSE);
	patternTree.SetHtml(FALSE);
	patternTree.SetStripHtml(FALSE);
	patternTree.SetImages(TRUE);
	patternTree.SetSeparatorColor(RGB(0xA6,0xA6,0xA6));

	return 0;
}

void CInfoPane::OnDestroy() {
	m_TreeImages.Destroy();
	SetMsgHandled(FALSE);
}

void CInfoPane::OnSize(UINT nType, CSize size) {
	if (currentWnd == true) {
		if (patternTree.m_hWnd) patternTree.MoveWindow(0, 0, size.cx, size.cy);
	} else {
		if (columnList.m_hWnd) columnList.MoveWindow(0, 0, size.cx, size.cy);
	}
}

void CInfoPane::OnSetFocus(CWindow wndOld) {
	GetParent().GetParent().SetFocus(); // todo- quick hack
	// 	if (currentWnd == true) {
	// 		if (patternTree.m_hWnd) patternTree.SetFocus();
	// 	} else {
	// 		if (columnList.m_hWnd) columnList.SetFocus();
	// 	}
}

LRESULT CInfoPane::OnTreeFocused(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	GetParent().GetParent().SetFocus(); // todo- quick hack
	return 0;
}

void CInfoPane::SwitchWindow(bool selWnd) {
	if (currentWnd == selWnd) return;

	HWND hide_window;
	HWND show_window;

	if (selWnd == true) {
		show_window = patternTree.m_hWnd;
		hide_window = columnList.m_hWnd;
	} else {
		show_window = columnList.m_hWnd;
		hide_window = patternTree.m_hWnd;
	}

	CRect rect;
	GetClientRect(&rect);

	::SetWindowPos(hide_window, NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_HIDEWINDOW|SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
	::SetWindowPos(show_window, HWND_TOP, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);

	currentWnd = selWnd;
}

void CInfoPane::PatternTreeRebuild() {
	SetRedraw(FALSE);
	{
		// bind all formats
		{
			zzub_pattern_format_iterator_t* fit = zzub_player_get_pattern_format_iterator(player);
			while (zzub_pattern_format_iterator_valid(fit)) {
				zzub_pattern_format_t* fmt = zzub_pattern_format_iterator_current(fit);
				PatternTreeInsertPatternFormat(fmt);
				zzub_pattern_format_iterator_next(fit);
			}
			zzub_pattern_format_iterator_destroy(fit);
		}

		// bind all patterns
		{
			zzub_pattern_iterator_t* pit = zzub_player_get_pattern_iterator(player);
			while (zzub_pattern_iterator_valid(pit)) {
				zzub_pattern_t* pat = zzub_pattern_iterator_current(pit);
				PatternTreeInsertPattern(pat);
				zzub_pattern_iterator_next(pit);
			}
			zzub_pattern_iterator_destroy(pit);
		}
	}
	SetRedraw(TRUE);
///	RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);
}

void CInfoPane::PatternTreeInsertPattern(zzub_pattern_t* pat) {
	SetRedraw(FALSE);
	{
		zzub_pattern_format_t* fmt = zzub_pattern_get_format(pat);
		HTREEITEM hFormatItem = treeItemMapReverse[timr_pair_t(fmt, (zzub_pattern_t*)0)];

		std::string name = zzub_pattern_get_name(pat);

		TVINSERTSTRUCT tvis = { 0 };
		tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		tvis.item.pszText = (char*)&name[0];
		tvis.item.cchTextMax = (int)_tcslen((char*)&name[0]);
		tvis.item.iImage = TV_NOIMAGE;
		tvis.item.iSelectedImage = TV_NOIMAGE;
		tvis.item.lParam = 0;
		tvis.hParent = hFormatItem;
		tvis.hInsertAfter = TVI_LAST;

		bool first_subitem = !patternTree.ItemHasChildren(hFormatItem);
		HTREEITEM hPatternItem = patternTree.InsertItem(&tvis);
		patternTree.SetItemBold(hPatternItem, TRUE);
		if (first_subitem) patternTree.ExpandBranch(hFormatItem);

		PE_treeiteminfo ti = { false, NULL, pat };
		treeItemMapForward[hPatternItem] = ti;

		treeItemMapReverse[timr_pair_t((zzub_pattern_format_t*)0, pat)] = hPatternItem;

		{ // eh
			HTREEITEM hSelItem = patternTree.GetSelectedItem();
			if (hSelItem) patternTree.EnsureVisible(hSelItem);
		}
	}
	SetRedraw(TRUE);
}

void CInfoPane::PatternTreeUpdatePattern(zzub_pattern_t* pat) {
	SetRedraw(FALSE);
	{
		std::string name = zzub_pattern_get_name(pat);
		HTREEITEM hPatItem = treeItemMapReverse[timr_pair_t((zzub_pattern_format_t*)0, pat)];
		patternTree.SetItemText(hPatItem, (char*)&name[0]);
	}
	SetRedraw(TRUE);
}

void CInfoPane::PatternTreeDeletePattern(zzub_pattern_t* pat) {
	SetRedraw(FALSE);
	{
		timr_t::iterator rev_it = treeItemMapReverse.find(timr_pair_t((zzub_pattern_format_t*)0, pat));
		HTREEITEM hPatItem = (*rev_it).second;
		tim_t::iterator fwd_it = treeItemMapForward.find(hPatItem);
		patternTree.DeleteItem(hPatItem);
		treeItemMapReverse.erase(rev_it);
		treeItemMapForward.erase(fwd_it);
	}
	SetRedraw(TRUE);
}

void CInfoPane::PatternTreeInsertPatternFormat(zzub_pattern_format_t* fmt) {
	SetRedraw(FALSE);
	{
		std::string name = zzub_pattern_format_get_name(fmt);

		TVINSERTSTRUCT tvis = { 0 };
		tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
		tvis.item.pszText = (char*)&name[0];
		tvis.item.cchTextMax = (int)_tcslen((char*)&name[0]);
		tvis.item.iImage = 0; // 0 is format icon
		tvis.item.iSelectedImage = 0; // 0 is format icon
		tvis.item.lParam = 0;
		//tvis.hParent = ;
		tvis.hInsertAfter = TVI_LAST;

		HTREEITEM hFormatItem = patternTree.InsertItem(&tvis);
		bool none_selected = (patternTree.GetSelectedItem() == NULL);
		if (none_selected) {
			patternTree.SelectItem(hFormatItem);
			patternTree.EnsureVisible(hFormatItem);
		}

		PE_treeiteminfo ti = { true, fmt, NULL };
		treeItemMapForward[hFormatItem] = ti;

		treeItemMapReverse[timr_pair_t(fmt, (zzub_pattern_t*)0)] = hFormatItem;
	}
	SetRedraw(TRUE);
}

void CInfoPane::PatternTreeUpdatePatternFormat(zzub_pattern_format_t* fmt) {
	SetRedraw(FALSE);
	{
		std::string name = zzub_pattern_format_get_name(fmt);
		HTREEITEM hFmtItem = treeItemMapReverse[timr_pair_t(fmt, (zzub_pattern_t*)0)];
		patternTree.SetItemText(hFmtItem, name.c_str());
	}
	SetRedraw(TRUE);
}

void CInfoPane::PatternTreeDeletePatternFormat(zzub_pattern_format_t* fmt) {
	SetRedraw(FALSE);
	{
		timr_t::iterator rev_it = treeItemMapReverse.find(timr_pair_t(fmt, (zzub_pattern_t*)0));
		HTREEITEM hFmtItem = (*rev_it).second;
		tim_t::iterator fwd_it = treeItemMapForward.find(hFmtItem);
		patternTree.DeleteItem(hFmtItem);
		treeItemMapReverse.erase(rev_it);
		treeItemMapForward.erase(fwd_it);
	}
	SetRedraw(TRUE);
}

void CInfoPane::OnFormatCreate() {
	zzub_pattern_format_t* format = zzub_player_create_pattern_format(player, 0);
//	if (!format) return;

	zzub_player_history_commit(player, 0, 0, "Create Pattern Format");
}

void CInfoPane::OnFormatDelete() {
	zzub_pattern_format_t* format = treeItemMapForward[m_hTrackItem].format;
	if (!format) return;

	zzub_pattern_format_destroy(format);
	zzub_player_history_commit(player, 0, 0, "Delete Pattern Format");
}

void CInfoPane::OnFormatClone() {
	zzub_pattern_format_t* format = treeItemMapForward[m_hTrackItem].format;
	if (!format) return;

	char const* description = zzub_pattern_format_get_name(format);
	zzub_pattern_format_t* cloneformat = zzub_player_clone_pattern_format(player, format, description);

	zzub_player_history_commit(player, 0, 0, "Clone Pattern Format");
}

void CInfoPane::OnFormatSetup() {
	zzub_pattern_format_t* format = treeItemMapForward[m_hTrackItem].format;
	if (!format) return;

	buze_event_data ev;
	ev.show_pattern_format.pattern_format = format;
	buze_document_notify_views(document, owner, buze_event_type_show_pattern_format_view, &ev);
}

void CInfoPane::OnFormatProperties() {
	zzub_pattern_format_t* format = treeItemMapForward[m_hTrackItem].format;
	if (!format) return;

	buze_event_data_t ev;
	ev.show_properties.return_view = owner;
	ev.show_properties.type = buze_property_type_pattern_format;
	ev.show_properties.pattern_format = format;
	buze_document_notify_views(document, owner, buze_event_type_show_properties, &ev);

}

void CInfoPane::OnPatternCreate() {
	zzub_pattern_format_t* format = treeItemMapForward[m_hTrackItem].format;
	if (!format) return;

	CBuzeConfiguration configuration = buze_document_get_configuration(document);

	int len = configuration->getGlobalPatternLength();

	char const* description = 0;
	if (configuration->getPatternNamingMode() == 1) {
		description = zzub_pattern_format_get_name(format);
	}
	zzub_pattern_t* pat = zzub_player_create_pattern(player, format, description, len);

	zzub_player_history_commit(player, 0, 0, "Create Pattern");
}

void CInfoPane::OnPatternDelete() {
	zzub_pattern_t* pattern = treeItemMapForward[m_hTrackItem].pattern;
	if (!pattern) return;

	zzub_pattern_destroy(pattern);
	zzub_player_history_commit(player, 0, 0, "Delete Pattern");
}

void CInfoPane::OnPatternClone() {
	zzub_pattern_t* pattern = treeItemMapForward[m_hTrackItem].pattern;
	if (!pattern) return;

	CBuzeConfiguration configuration = buze_document_get_configuration(document);
	char const* description = 0;
	if (configuration->getPatternNamingMode() == 1) {
		description = zzub_pattern_get_name(pattern);
	}
	zzub_pattern_t* clonepattern = zzub_player_clone_pattern(player, pattern, description);

	zzub_player_history_commit(player, 0, 0, "Clone Pattern");
}

void CInfoPane::OnPatternProperties() {
	zzub_pattern_t* pattern = treeItemMapForward[m_hTrackItem].pattern;
	if (!pattern) return;

	buze_event_data_t ev;
	ev.show_properties.type = buze_property_type_pattern;
	ev.show_properties.pattern = pattern;
	ev.show_properties.return_view = owner;
	buze_document_notify_views(document, owner, buze_event_type_show_properties, &ev);
}

void CInfoPane::OnPatternEdit() {
	zzub_pattern_t* pattern = treeItemMapForward[m_hTrackItem].pattern;
	if (!pattern) return;

	owner->SetPatternPushStack(pattern);
}

LRESULT CInfoPane::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	CPoint point_local = point;
	patternTree.ScreenToClient(&point_local);
	UINT flags = 0;
	HTREEITEM hItem = patternTree.HitTest(point_local, &flags);

	CMenu menu;
	menu.CreatePopupMenu();
	if (hItem && (flags & (TVHT_ONITEM | TVHT_ONITEMBUTTON | TVHT_ONITEMRIGHT | TVHT_ONITEMINDENT))) {
		if (treeItemMapForward[hItem].is_format) { // right clicked on pattern format
			menu.AppendMenu(MF_STRING, ID_INFOPANE_PATTERN_CREATE,	"Create Pattern");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, ID_INFOPANE_FORMAT_DELETE,	"Delete Pattern Format");
			menu.AppendMenu(MF_STRING, ID_INFOPANE_FORMAT_CLONE,	"Clone Pattern Format");
			menu.AppendMenu(MF_STRING, ID_INFOPANE_FORMAT_SETUP,	"Edit Pattern Format");
			menu.AppendMenu(MF_STRING, ID_INFOPANE_FORMAT_PROPS,	"Pattern Format Properties");
		} else { // right clicked on pattern
			menu.AppendMenu(MF_STRING, ID_INFOPANE_PATTERN_EDIT,	"Edit Pattern");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, ID_INFOPANE_PATTERN_DELETE,	"Delete Pattern");
			menu.AppendMenu(MF_STRING, ID_INFOPANE_PATTERN_CLONE,	"Clone Pattern");
			menu.AppendMenu(MF_STRING, ID_INFOPANE_PATTERN_PROPS,	"Pattern Properties");
		}

		m_hTrackItem = hItem;
	} else { // right clicked on background
		menu.AppendMenu(MF_STRING, ID_INFOPANE_FORMAT_CREATE,		"Create Pattern Format");

		m_hTrackItem = 0;
	}

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, m_hWnd, NULL);
	return 0;
}

LRESULT CInfoPane::OnBeginLabelEditTree(NMHDR* pNMHDR) {
	return FALSE; // return FALSE to allow edit
}

LRESULT CInfoPane::OnEndLabelEditTree(NMHDR* pNMHDR) {
	NMTVDISPINFO* pTVDispInfo = (NMTVDISPINFO*)pNMHDR;
	HTREEITEM hItem = pTVDispInfo->item.hItem;
	LPTSTR pszText = pTVDispInfo->item.pszText;
	if (!hItem || !pszText) return FALSE;
	if (*pszText == _T('\0')) return FALSE; // empty name, disallow.

	PE_treeiteminfo& pi = treeItemMapForward[hItem];
	if (pi.is_format) {		
		zzub_pattern_format_set_name(pi.format, (char*)pszText);
		zzub_player_history_commit(player, 0, 0, "Rename Pattern Format");
	} else {
		zzub_pattern_t* pat = zzub_player_get_pattern_by_name(player, (char*)pszText);
		if (pat != pi.pattern && pat != 0) return FALSE; // disallow dupes

		const char* patternname = zzub_pattern_get_name(pi.pattern);
		if (strcmp(patternname, (char*)pszText) == 0) return FALSE;

		zzub_pattern_set_name(pi.pattern, (char*)patternname);
		zzub_player_history_commit(player, 0, 0, "Rename Pattern");
	}

	// return TRUE to accept edit
	return FALSE;	// We return FALSE here because we allow _get_new_name to fix collisions
					// and hence only want the actual name to get updated by zzub events.
}

void CInfoPane::PatternTreeMakePatternVisible(zzub_pattern_t* pat) {
	HTREEITEM hItem = treeItemMapReverse[timr_pair_t((zzub_pattern_format_t*)0, pat)];
	patternTree.SelectItem(hItem);
}

void CInfoPane::PatternTreeMakeFormatVisible(zzub_pattern_format_t* fmt) {
	HTREEITEM hItem = treeItemMapReverse[timr_pair_t(fmt, (zzub_pattern_t*)0)];
	patternTree.SelectItem(hItem);
}
