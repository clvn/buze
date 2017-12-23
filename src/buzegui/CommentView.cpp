#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "CommentView.h"

enum EDITFLAGS {
	EDIT_COPY = 1,	// set if selection can be copied
	EDIT_PASTE = 2,	// set if clipboard format is recognized
};

// 
// Factory
//

CCommentViewInfo::CCommentViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CCommentView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Comments";
	tooltip = "Song comments";
	place = 1; //DockSplitTab::placeMAINPANE;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = true;
	allowfloat = false;
	defaultview = false;
}

CView* CCommentViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CCommentView* view = new CCommentView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CCommentViewInfo::Attach() {
	buze_document_add_view(document, this);

	// global accelerators - these generate global document events caught in OnUpdate
	WORD ID_SHOW_COMMENTS = buze_main_frame_register_accelerator_event(mainframe, "view_comment", "f11", buze_event_type_show_comment_view);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_COMMENTS, "Song Comments");
}

void CCommentViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CCommentViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	const char* infotext;
	switch (lHint) {
		case buze_event_type_show_comment_view:
			ShowCommentView();
			break;
		case buze_event_type_update_post_open_document:
			infotext = zzub_player_get_infotext(buze_main_frame_get_player(mainframe));
			if (infotext && strlen(infotext) > 0)
				ShowCommentView();
			break;
	}
}

void CCommentViewInfo::ShowCommentView() {
	CView* view = buze_main_frame_get_view(mainframe, "CommentView", 0);
	if (view)
		buze_main_frame_set_focus_to(mainframe, view);
	else
		buze_main_frame_open_view(mainframe, "CommentView", "Comments", 0, -1, -1);
}

//
// View
//

CCommentView::CCommentView(buze_main_frame_t* mainFrm)
	:CViewImpl(mainFrm)
{
}

CCommentView::~CCommentView() {
}

void CCommentView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CCommentView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	edit.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL);
	edit.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	
	edit.SetWindowText(zzub_player_get_infotext(player));

	buze_document_add_view(document, this);

	return 0;
}

LRESULT CCommentView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);
	edit.MoveWindow(0, 0, width, height);
	return 0;
}

LRESULT CCommentView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	buze_document_remove_view(document, this);
	return 0;
}

LRESULT CCommentView::OnEditKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	int len = edit.GetWindowTextLength();
	char* pc = new char[len+1];
	edit.GetWindowText(pc, len+1);

	zzub_player_set_infotext(player, pc);
	zzub_player_history_commit(player, 0, 0, "Edit Song Comment");

	delete[] pc;
	return 0;
}

void CCommentView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	switch (lHint) {
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_new_document:
		case zzub_event_type_update_song:
			edit.SetWindowText(zzub_player_get_infotext(player));
			break;
	}
}

LRESULT CCommentView::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	edit.SetFocus();
	return 0;
}


LRESULT CCommentView::OnCut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	edit.Cut();
	return 0;
}

LRESULT CCommentView::OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	edit.Copy();
	return 0;
}

LRESULT CCommentView::OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	edit.Paste();
	return 0;
}

LRESULT CCommentView::OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	//edit.Clear();	//??
	return 0;
}

LRESULT CCommentView::OnSelectAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	return 0;
}

LRESULT CCommentView::OnClearSelection(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	return 0;
}

LRESULT CCommentView::OnGetEditFlags(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return GetEditFlags();
}

int CCommentView::GetEditFlags() {
	int flags = 0;
	flags |= EDIT_COPY;
	flags |= EDIT_PASTE;
	return flags;
}
