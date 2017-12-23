#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "HistoryView.h"

// 
// Factory
//

CHistoryViewInfo::CHistoryViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CHistoryView::GetWndClassInfo().m_wc.lpszClassName;
	label = "History";
	tooltip = "Browse the undo/redo-buffers";
	place = 2; //DockSplitTab::placeDOCKPANE;
	side = 2; //DockSplitTab::dockLEFT;
	serializable = true;
	allowfloat = true;
	defaultview = false;
}

CView* CHistoryViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CHistoryView* view = new CHistoryView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CHistoryViewInfo::Attach() {
	buze_document_add_view(document, this);

	// global accelerators - these generate global document events caught in OnUpdate
	WORD ID_SHOW_HISTORY = buze_main_frame_register_accelerator_event(mainframe, "view_history", "h ctrl shift", buze_event_type_show_history);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_HISTORY, "History");

}

void CHistoryViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CHistoryViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CView* view;
	switch (lHint) {
		case buze_event_type_show_history:
			view = buze_main_frame_get_view(mainframe, "HistoryView", 0);
			if (view) {
				buze_main_frame_set_focus_to(mainframe, view);
			} else
				buze_main_frame_open_view(mainframe, "HistoryView", "History", 0, -1, -1);
			break;
	}
}

//
// View
//

DWORD WINAPI HistoryThreadProc(LPVOID lpParam) {
	CHistoryView* self = (CHistoryView*)lpParam;
	
	while (!self->quit) {
		if (self->dirtyHistory) {
			EnterCriticalSection(&self->csDirty);
			self->dirtyHistory = false;
			LeaveCriticalSection(&self->csDirty);
			self->BindHistory();
		}
		Sleep(100);
	}
	SetEvent(self->hHistorySignal);
	return 0;
}

CHistoryView::CHistoryView(buze_main_frame_t* mainFrm)
	:CViewImpl(mainFrm)
{
	dirtyHistory = true;
	quit = false;
	hHistorySignal = CreateEvent(0, FALSE, FALSE, "");
	assert(hHistorySignal != 0);

	InitializeCriticalSection(&csDirty);
}

CHistoryView::~CHistoryView(void) {
	DeleteCriticalSection(&csDirty);
}

void CHistoryView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CHistoryView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	historyList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | LVS_REPORT | LVS_SHOWSELALWAYS, 0, IDC_HISTORYLIST);
	historyList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	historyList.AddColumn("Event", 0);

	DWORD dwThreadID;
	HANDLE hThread = CreateThread(0, 0, HistoryThreadProc, this, 0, &dwThreadID);
	assert(hThread != 0);

	BindHistoryInThread();

	buze_document_add_view(document, this);

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);

	return 0;
}

LRESULT CHistoryView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD( lParam);
	WORD cy = HIWORD( lParam);

	historyList.MoveWindow(0, 0, cx, cy);
	historyList.SetColumnWidth(0, cx - 2);
	return 0;
}

bool WaitAndPump(HANDLE hHandle) {
	MSG msg;
	for (;;) {
		if (WaitForSingleObject(hHandle, 0) == WAIT_OBJECT_0) 
			return true;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) return false;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

LRESULT CHistoryView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	quit = true;
	WaitAndPump(hHistorySignal);
	CloseHandle(hHistorySignal);
	hHistorySignal = 0;

	buze_document_remove_view(document, this);

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	return 0;
}

LRESULT CHistoryView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CHistoryView::OnUpdateHistory(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	BindHistoryInThread();
	return 0;
}

void CHistoryView::BindHistoryInThread() {
	EnterCriticalSection(&csDirty);
	dirtyHistory = true;
	LeaveCriticalSection(&csDirty);
}

LRESULT CHistoryView::OnDblClkHistory(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	//int sel = historyList.GetSelectedIndex();
	//if (sel == -1) return 0;
	//int size = zzub_player_history_get_size(mainFrame->player);
	//int pos = zzub_player_history_get_position(mainFrame->player);

	// TODO: undo or redo x times from position to sel
	MessageBox("TODO: Go to this point in history");
	return 0;
}

BOOL CHistoryView::PreTranslateMessage(MSG* pMsg) {
	return FALSE;
}

void CHistoryView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	if (lHint != zzub_event_type_barrier) return ;

	// post a message to ourself so that we update after everything else has been done
	PostMessage(MYWM_UPDATE);
}

void CHistoryView::BindHistory() {
	historyList.SetRedraw(FALSE);

	historyList.DeleteAllItems();

	int size = zzub_player_history_get_size(player);
	int pos = zzub_player_history_get_position(player);
	for (int i = 0; i < size; i++) {
		int index = size - 1 - i;
		historyList.AddItem(i, 0, zzub_player_history_get_description(player, index));
		if (pos == index) 
			historyList.SetItemState(i, LVIS_SELECTED|LVIS_DROPHILITED, LVIS_SELECTED|LVIS_DROPHILITED);
	}

	historyList.SetRedraw(TRUE);
}
