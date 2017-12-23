#include "stdafx.h"
#include "SkeletonView.h"

CHostDllModule _Module;

//
// View
//

CSkeletonView::CSkeletonView(buze_main_frame_t* m) : CViewImpl(m) {
}

CSkeletonView::~CSkeletonView() {
}

void CSkeletonView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CSkeletonView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lres = DefWindowProc();
	edit.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL);
	edit.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	edit.SetWindowText("hello world!");
	buze_document_add_view(document, this);

	return 0;
}

LRESULT CSkeletonView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);
	edit.MoveWindow(0, 0, width, height);
	
	return 0;
}

LRESULT CSkeletonView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	buze_document_remove_view(document, this);
	return 0;
}

void CSkeletonView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	switch (lHint) {
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_new_document:
		case zzub_event_type_update_song:
			edit.SetWindowText(zzub_player_get_infotext(player));
			break;
	}
}
