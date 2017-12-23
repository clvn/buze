#include "stdafx.h"
#include "SkeletonView.h"

class CSkeletonViewLibrary : public CViewLibrary {
public:
	virtual void Initialize(CViewFrame* host) {
		_Module.m_hostModule = buze_application_get_host_module(buze_main_frame_get_application(host));
		buze_main_frame_register_window_factory(host, new CSkeletonViewInfo(host));
	}

	virtual void Destroy() {
		delete this;
	}

	virtual int GetVersion() {
		return CViewLibrary::version;
	}
};

extern "C" CViewLibrary* buze_create_viewlibrary() {
	return new CSkeletonViewLibrary();
}

//
// Factory
//

CSkeletonViewInfo::CSkeletonViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CSkeletonView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Skeleton";
	tooltip = "Skeleton";
	place = 1;
	side = -1;
	serializable = true;
	allowfloat = false;
	defaultview = true;
	mainframe = m;
}

CView* CSkeletonViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CSkeletonView* view = new CSkeletonView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CSkeletonViewInfo::Attach() {
	buze_document_add_view(document, this);

	show_eventcode = buze_main_frame_register_event(mainframe);
	WORD ID_SHOW_VIEW = buze_main_frame_register_accelerator_event(mainframe, "view_skeleton", "f4 ctrl", show_eventcode);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_VIEW, "Skeleton View");
}

void CSkeletonViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CSkeletonViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	buze_window_t* view;
	if (lHint == show_eventcode) {
		view = buze_main_frame_get_view(mainframe, "SkeletonView", 0);
		if (view) {
			buze_main_frame_set_focus_to(mainframe, view);
		} else
			buze_main_frame_open_view(mainframe, "SkeletonView", "Skeletons", 0, -1, -1);
	}
}
