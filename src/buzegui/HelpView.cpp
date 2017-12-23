#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "HelpView.h"
//#include "BuzeConfiguration.h"

// 
// Factory
//

CHelpViewInfo::CHelpViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CHelpView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Help";
	tooltip = "Help";
	place = 1; //DockSplitTab::placeMAINPANE;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = true;
	allowfloat = false;
	defaultview = false;
}

CView* CHelpViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CHelpView* view = new CHelpView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CHelpViewInfo::Attach() {
	buze_document_add_view(document, this);

	// global accelerators - these generate global document events caught in OnUpdate
	buze_main_frame_register_accelerator_event(mainframe, "view_help", "f1", buze_event_type_show_help_view);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);
}

void CHelpViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CHelpViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CHelpView* view;
	CView* focusView;
	char* helptext = 0;
	switch (lHint) {
		case buze_event_type_show_help_view:
			focusView = buze_main_frame_get_focused_view(mainframe);
			if (focusView) {
				int len;
				focusView->GetHelpText(0, &len);
				if (len > 0) {
					helptext = new char[len + 1];
					focusView->GetHelpText(helptext, &len);
				}
			}
			view = (CHelpView*)buze_main_frame_get_view(mainframe, "HelpView", 0);
			if (view) {
				buze_main_frame_set_focus_to(mainframe, view);
			} else
				view = (CHelpView*)buze_main_frame_open_view(mainframe, "HelpView", "Help", 0, -1, -1); // this one gets the message at once :/

			if (helptext)
				view->helpText.SetWindowText(helptext);
			else
				view->helpText.SetWindowText("No help available");
			delete[] helptext;
			break;
	}
}

//
// View
//

CHelpView::CHelpView(buze_main_frame_t* mainFrm) 
	:CViewImpl(mainFrm)
{
}

CHelpView::~CHelpView() {
}

void CHelpView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CHelpView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	std::string fontName = "";
	buze_configuration_t* config = buze_document_get_configuration(document);
	fontName = buze_configuration_get_fixed_width_font(config);
	//_Module.configuration->getConfigString("Settings", "FixedWidthFont", &fontName);
	helpFont.CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH|FF_DONTCARE, fontName.c_str());
	helpText.Create(*this, rcDefault, "", WS_VISIBLE|WS_CHILD|WS_VSCROLL|WS_HSCROLL|ES_MULTILINE|ES_AUTOVSCROLL);
	helpText.SetReadOnly();
	helpText.SetFont(helpFont);

	hAccel = AtlLoadAccelerators(IDR_HELPVIEW);

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);

	buze_main_frame_viewstack_insert(mainframe, this);

	return 0;
}

BOOL CHelpView::PreTranslateMessage(MSG* pMsg) {
	if (GetFocus() == *this || GetFocus() == helpText)
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg))
			return TRUE;
	return FALSE;
}

LRESULT CHelpView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);
	helpText.MoveWindow(0, 0, width, height);
	return 0;
}

LRESULT CHelpView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	return 0;
}

LRESULT CHelpView::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	helpText.SetFocus();
	return 0;
}

LRESULT CHelpView::OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	//buze_main_frame_show_help_global(mainframe);
	return 0;
}
