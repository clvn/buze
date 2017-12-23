#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "BuzeConfiguration.h"
#include "PreferencesView.h"

// 
// Factory
//

CPreferencesViewInfo::CPreferencesViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CPreferencesView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Preferences";
	tooltip = "Preferences";
	place = 3; //DockSplitTab::placeFLOATFRAME;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = true;
	allowfloat = false;
	defaultview = false;
	defaultfloatwidth = 600;
	defaultfloatheight = 400;
}

CView* CPreferencesViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CPreferencesView* view = new CPreferencesView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CPreferencesViewInfo::Attach() {
	buze_document_add_view(document, this);

	// global accelerators - these generate global document events caught in OnUpdate
	WORD ID_SHOW_PREFERENCES = buze_main_frame_register_accelerator_event(mainframe, "view_preferences", "s ctrl shift", buze_event_type_show_preferences);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_PREFERENCES, "Preferences");

}

void CPreferencesViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CPreferencesViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CView* view;
	switch (lHint) {
		case buze_event_type_show_preferences:
			view = buze_main_frame_get_view(mainframe, "PreferencesView", 0);
			if (view) {
				buze_main_frame_set_focus_to(mainframe, view);
			} else
				buze_main_frame_open_view(mainframe, "PreferencesView", "Preferences", 0, -1, -1);
			break;
	}
}

//
// View
//

CPreferencesView::CPreferencesView(CViewFrame* mainFrm)
	:CViewImpl(mainFrm)
{
	lastPageListItem = -1;
	provider = 0;
}

CPreferencesView::~CPreferencesView(void) {
}

void CPreferencesView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CPreferencesView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lres = DefWindowProc();

	SetRedraw(FALSE);

	propertyList.Create(*this, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS | LBS_OWNERDRAWVARIABLE, WS_EX_STATICEDGE, IDC_PREFERENCESPROPERTYLIST);
	propertyList.SetExtendedListStyle(PLS_EX_CATEGORIZED | PLS_EX_SHOWSELALWAYS | PLS_EX_SINGLECLICKEDIT);
	propertyList.SetColumnWidth(175);

	pageList.Create(*this, &rcDefault, 0, WS_VISIBLE | WS_CHILD | LVS_NOCOLUMNHEADER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, WS_EX_STATICEDGE, IDC_PREFERENCESPAGELIST);
	pageList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT); 
	pageList.AddColumn("Name", 0);
	pageList.SetColumnWidth(0, 100);
	pageList.InsertItem(0, "Audio Devices");
	pageList.InsertItem(1, "MIDI");
	pageList.InsertItem(2, "GUI");
	pageList.InsertItem(3, "Plugins");
//	pageList.InsertItem(3, "Themes");
//	pageList.InsertItem(4, "Keyboard");
	pageList.SelectItem(0);

	applyButton.Create(*this, rcDefault, "Apply", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_NOTIFY, 0, IDC_APPLYBUTTON);
	applyButton.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	configureButton.Create(*this, rcDefault, "Configure ASIO", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_NOTIFY, 0, IDC_CONFIGUREBUTTON);
	configureButton.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	SetConfigureButtonState();
	SetRedraw(TRUE);
	RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddIdleHandler(this);

	buze_document_add_view(document, this);

	return lres;
}

LRESULT CPreferencesView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	buze_document_remove_view(document, this);
	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveIdleHandler(this);
	return 0;
}

LRESULT CPreferencesView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 0;
}

LRESULT CPreferencesView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	RECT rcClient;
	GetClientRect(&rcClient);

	SetRedraw(FALSE);
	{
		int margin_height = 28;

		int page_width = 150;
		pageList.MoveWindow(0, 0, page_width, rcClient.bottom - margin_height);
		pageList.SetColumnWidth(0, page_width - 4);

		propertyList.MoveWindow(page_width, 0, rcClient.right - page_width, rcClient.bottom - margin_height);

		int button_width = 100;
		int button_height = margin_height - 4;
		int button_offset = (margin_height - button_height) / 2;
		applyButton.MoveWindow(page_width, rcClient.bottom - margin_height + button_offset + 1, button_width, button_height);

		configureButton.MoveWindow(rcClient.right - button_width - 4, rcClient.bottom - margin_height + button_offset + 1, button_width, button_height);
	}
	SetRedraw(TRUE);	
	RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);

	return 0;
}

LRESULT CPreferencesView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 0;
}

LRESULT CPreferencesView::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle dc((HDC)wParam);
	COLORREF fc=GetSysColor(COLOR_BTNFACE);
	RECT rc;
	GetClientRect(&rc);
	dc.FillSolidRect(&rc, fc);

	return 1;
}

LRESULT CPreferencesView::OnApplyClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	provider->doDataExchange(true);
	propertyList.SetFocus();
	SetConfigureButtonState();
	return 0;
}

LRESULT CPreferencesView::OnConfigureClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_audiodriver_t* driver = buze_application_get_audio_driver(buze_main_frame_get_application(mainframe));
	if (driver) zzub_audiodriver_configure(driver);
	return 0;
}

LRESULT CPreferencesView::OnSelChangeProperty(int idCtrl, LPNMHDR pNMHDR, BOOL& /*bHandled*/) {

	NMPROPERTYITEM& pi = (NMPROPERTYITEM&)*pNMHDR;
	int index = propertyList.FindProperty(pi.prop);
	if (index == -1) return 0;

	PropertyInfoBase* pib = provider->getProperty(index);

	VARIANT iv;
	VariantInit(&iv);
	propertyList.GetItemValue(pi.prop, &iv);

	pib->set(iv);

	// TODO: the setter may have adjusted the value, read back final value
	return 0;
}

LRESULT CPreferencesView::OnDeleteProperty(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/) {
	NMPROPERTYITEM& pi = (NMPROPERTYITEM&)*pNMHDR;
	int index = propertyList.FindProperty(pi.prop);
	if (index == -1) return 0;

	//PropertyInfoBase* pib = provider->getProperty(index);
	provider->deleteProperty(index);
	return 0;
}

LRESULT CPreferencesView::OnSelChangePage(int idCtrl, LPNMHDR pNMHDR, BOOL& /*bHandled*/) {

	int cursel = pageList.GetSelectedIndex();
	if (cursel == -1) {
		//if (lastPageListItem != -1) pageList.SelectItem(lastPageListItem);
		return 0;
	}

	if (cursel == lastPageListItem) return 0;

	CPreferencePropertyProvider* prov = 0;

	switch (cursel) {
		case 0:
			prov = new CAudioDriverProvider(this, mainframe);
			break;
		case 1:
			prov = new CMidiConfigProvider(this, mainframe);
			break;
		case 2:
			prov = new CGuiConfigProvider(this, mainframe);
			break;
		case 3:
			//prov = new CThemeConfigProvider(this, mainframe);
			prov = new CPluginConfigProvider(this, mainframe);
			break;
		case 4:
			prov = new CKeyboardConfigProvider(this, mainframe);
			break;
		default:
			// TODO: re-select the previous pageList item
			return 0;
	}

	lastPageListItem = cursel;
	if (provider != 0) {
		//TODO: if dirty and MessageBox("save changes?" MB_YESNO) then provider->doDataExchange(true) - same in onclose?
		delete provider;
	}
	provider = prov;
	if (provider != 0) provider->doDataExchange(false);

	BindProvider();

	return 0;
}

void CPreferencesView::BindProvider() {

	propertyList.ResetContent();

	if (provider == 0) return ;

	for (int i = 0; i<provider->getProperties(); i++) {
		PropertyInfoBase* pi = provider->getProperty(i);
		HPROPERTY hprop = pi->create();
		assert(hprop);
		propertyList.AddItem(hprop);
	}
}

void CPreferencesView::SetConfigureButtonState() {

	// since armstrong currently lacks a mechanism to get the api 
	// of the current device, fetch it via the config and audiodriver to get 
	// the api from the stored device info

	BOOL enabled = FALSE;

	buze_application_t* application = buze_main_frame_get_application(mainframe);
	CBuzeConfiguration config(buze_application_get_configuration(application));

	std::string outname, inname;
	int rate, size, outchannel;
	if (!config.getAudioDriver(outname, inname, rate, size, outchannel))
		goto done;

	zzub_audiodriver_t* driver = buze_application_get_audio_driver(application);
	if (!driver) goto done;

	zzub_device_info_t* outdevice = zzub_audiodriver_get_device_info_by_name(driver, outname.c_str());
	if (!outdevice) goto done;

	if (zzub_device_info_get_api(outdevice) == 0) // 0 == api_asio from driver_native, the only api supporting asio configuration
		enabled = TRUE;

done:
	configureButton.EnableWindow(enabled);
}


BOOL CPreferencesView::OnIdle() {
	if (provider && provider->dirty) {
		applyButton.EnableWindow(TRUE);
	} else {
		applyButton.EnableWindow(FALSE);
	}
	return FALSE;
}

void CPreferencesView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	switch (lHint) {
		case zzub_event_type_device_reset:
			if (provider) {
				// TODO: use a real id
				// TODO: altenratively check page index and cast to audioprefs and update directly
				provider->providerMessage(0);
			}
			break;
	}

}

LRESULT CPreferencesView::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// we need to process WM_MOUSEACTIVATE before the docktabframework as it sets focus causing 
	// several problems with our property list.
	buze_window_t* focusView = buze_main_frame_get_focused_view(mainframe);
	HWND hFocusWnd = (HWND)buze_window_get_wnd(focusView);
	if (hFocusWnd == *this) {
		return MA_ACTIVATE;
	} else
		return DefWindowProc();
}

LRESULT CPreferencesView::OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	propertyList.SetFocus();
	return 0;
}
