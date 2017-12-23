#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "BuzeConfiguration.h"
#include "Properties.h"
#include "PropertyListView.h"
#include "PropertyViewProperties.h"
// 
// Factory
//

CPropertyListViewInfo::CPropertyListViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CPropertyListView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Properties";
	tooltip = "Properties for selected object";
	place = 2; //DockSplitTab::placeDOCKPANE;
	side = 2; //DockSplitTab::dockLEFT;
	serializable = true;
	allowfloat = true;
	defaultview = false;
}

CView* CPropertyListViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CPropertyListView* view = new CPropertyListView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, pCreateData);
	return view;
}

void CPropertyListViewInfo::Attach() {
	buze_document_add_view(document, this);

	//mainframe->RegisterAccelerator("properties", "help", ID_HELP);
	buze_main_frame_register_accelerator(mainframe, "properties", "returntoview", "escape", ID_PROPERTYVIEW_RETURNTOVIEW);
}

void CPropertyListViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CPropertyListViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;
	buze_event_data_t* ev = (buze_event_data_t*)pHint;
	CPropertyListView* view;
	switch (lHint) {
		case buze_event_type_show_properties:
			view = (CPropertyListView*)buze_main_frame_get_view(mainframe, "PropertyListView", 0);
			if (view != 0)
				buze_main_frame_set_focus_to(mainframe, view); 
			else
				buze_main_frame_open_view(mainframe, "PropertyListView", "Properties", 0, -1, -1);
			break;
	}
}

//
// View
//

CPropertyListView::CPropertyListView(CViewFrame* mainFrm)
	:CViewImpl(mainFrm)
{
	provider = 0;
	hReturnView = 0;
	dirtyProperties = false;
}

CPropertyListView::~CPropertyListView(void) {
}

void CPropertyListView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CPropertyListView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// http://groups.google.no/group/microsoft.public.vc.atl/browse_thread/thread/afb91a02192bd05b/d67319f5d6b58936?lnk=st&q=CListViewCtrl+additem&rnum=1&hl=no#d67319f5d6b58936
	// this thread suggests calling defwindowproc before adding items

	//objectCombo.Create(m_hWnd, rcDefault, "Object", WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST, 0, IDC_OBJECTDROPDOWN);
	//objectCombo.AddString("<none>");
	//objectCombo.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	propertyList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS | LBS_OWNERDRAWVARIABLE, 0, IDC_PROPERTYLIST);
	propertyList.SetExtendedListStyle(PLS_EX_SHOWSELALWAYS | PLS_EX_CATEGORIZED | PLS_EX_SINGLECLICKEDIT);
	infoPane.Create(m_hWnd, rcDefault, "", WS_CHILD | WS_VISIBLE);
	infoPane.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	// add the toolbar bands as late as possible since they will invoke a WM_SIZE on us
	//bool bLock = mainframe->GetLockedToolbarsState();
	//insertToolbarBand(objectCombo, "Object", 100, -1, true, bLock);
	buze_document_add_view(document, this);

	CModuleMessageLoop* messageLoop = _Module.GetMessageLoop();
	messageLoop->AddMessageFilter(this);

	LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
	buze_event_data* args = (buze_event_data*)cs->lpCreateParams;

	if (args != 0)
		UpdateFromEvent(0, args);
	return DefWindowProc();
}

BOOL CPropertyListView::PreTranslateMessage(MSG* pMsg) {
	if (true
		&& (false
			|| GetFocus() == *this
			|| GetFocus() == propertyList
			|| GetFocus() == propertyList.m_hwndInplace
		)
		&& !propertyList.IsInplaceEditDirty()
	) {
		HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "properties");
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg))
			return TRUE;
	}
	return FALSE;
}

LRESULT CPropertyListView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CModuleMessageLoop* messageLoop = _Module.GetMessageLoop();
	messageLoop->RemoveMessageFilter(this);

	buze_document_remove_view(document, this);
	//mainframe->closeClientWindow(m_hWnd);

	return 0;
}

LRESULT CPropertyListView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD(lParam);
	WORD cy = HIWORD(lParam);

	int infoHeight = 60;
	propertyList.MoveWindow(0, getToolbarHeight(), cx, cy - getToolbarHeight() - infoHeight);
	infoPane.MoveWindow(0, cy - infoHeight, cx, infoHeight);

	//RECT rcItem;
	//getBandRect(objectCombo, &rcItem);
	//objectCombo.MoveWindow(rcItem.left, rcItem.top, rcItem.right-rcItem.left, rcItem.bottom-rcItem.top + 200);
	return 0;
}

LRESULT CPropertyListView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	if (dirtyProperties) {
		//assert(false);
		//if (document->selectedPropertyProvider)
		if (provider != 0)
			UpdateFromProvider(provider);
		dirtyProperties = false;
	}
	return DefWindowProc();
}


LRESULT CPropertyListView::OnItemChangeProperty(int idCtrl, LPNMHDR pNMHDR, BOOL& /*bHandled*/) {

	NMPROPERTYITEM& pi = (NMPROPERTYITEM&)*pNMHDR;
	int index = propertyList.FindProperty(pi.prop);
	if (index == -1) return 0;

	PropertyInfoBase* pib = provider->getProperty(index);

	//VARIANT& v = pib->get();
	VARIANT v;
	VariantInit(&v);
	propertyList.GetItemValue(pi.prop, &v);

	bool valid = pib->set(v);
	if (valid) {
		UpdateInfo();
	} else { // restore the old value
		UpdateFromProvider(provider); /// Todo: just update the one property that was rejected
	}

	// TODO: the setter may have adjusted the value, read back final value

	return 0;
}

LRESULT CPropertyListView::OnSelChangeProperty(int idCtrl, LPNMHDR pNMHDR, BOOL& /*bHandled*/) {
	UpdateInfo();
	return 0;
}

LRESULT CPropertyListView::OnSelChangeObject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	/*int curSel = objectCombo.GetCurSel();
	char countstr[1024];
	strcpy(countstr, "");
	if (curSel != LB_ERR) {
		objectCombo.GetLBText(curSel, countstr);
	}
	curSel = atoi(countstr);

	objectCombo.SelectString(-1, countstr);
	objectCombo.ShowDropDown(FALSE);
*/
	return 0;
}

void CPropertyListView::ClearAll() {
	propertyList.ResetContent();
	provider = 0;
}

void CPropertyListView::UpdateFromProvider(CPropertyProvider* _provider) {
	int cursel = propertyList.GetCurSel();

	ClearAll();
	provider = _provider;
	provider->createProperties();

	for (int i = 0; i<provider->getProperties(); i++) {
		PropertyInfoBase* pi = provider->getProperty(i);
		HPROPERTY hprop = pi->create();
		assert(hprop);
		propertyList.AddItem(hprop);
	}

	if (propertyList.GetCount() == 0) return ;

	if (cursel >= propertyList.GetCount())
		cursel = propertyList.GetCount() - 1;
	propertyList.SetCurSel(cursel!=-1?cursel:0);

	// ae: OnSelChange was probably added for a reason, but it causes the currently selected edit property to steal focus when it shouldnt:
//	BOOL bDummy;
//	propertyList.OnSelChange(0,0,0,bDummy);
}

void CPropertyListView::UpdateFromEvent(CView* pSender, buze_event_data_t* ev) {
	buze_window_t* hReturnWnd;
	if (pSender)
		hReturnWnd = pSender;//->GetHwnd();
	else
		hReturnWnd = ev->show_properties.return_view ;

	switch (ev->show_properties.type) {
		case buze_property_type_pattern_format:
			if (provider != 0) delete provider;
			provider = new CPatternFormatPropertyProvider(hReturnWnd, document, ev->show_properties.pattern_format);
			dirtyProperties = true;
			Invalidate(FALSE);
			break;

		case buze_property_type_pattern:
			if (provider != 0) delete provider;
			provider = new CPatternPropertyProvider(hReturnWnd, document, ev->show_properties.pattern);
			dirtyProperties = true;
			Invalidate(FALSE);
			break;

		case buze_property_type_plugin:
			if (provider != 0) delete provider;
			provider = new CMachinePropertyProvider(hReturnWnd, document, ev->show_properties.plugin);
			dirtyProperties = true;
			Invalidate(FALSE);
			break;

		case buze_property_type_plugin_group:
			if (provider != 0) delete provider;
			provider = new CGroupPropertyProvider(hReturnWnd, document, ev->show_properties.plugin_group);
			dirtyProperties = true;
			Invalidate(FALSE);
			break;

		case buze_property_type_connection:
			if (provider != 0) delete provider;
			provider = new CConnectionPropertyProvider(hReturnWnd, document, (zzub_connection_t*)ev->show_properties.connection);
			dirtyProperties = true;
			Invalidate(FALSE);
			break;

		case buze_property_type_wave:
			if (provider != 0) delete provider;
			provider = new CWavePropertyProvider(hReturnWnd, document, (zzub_wave_t*)ev->show_properties.wave);
			dirtyProperties = true;
			Invalidate(FALSE);
			break;

		case buze_property_type_wave_level:
			if (provider != 0) delete provider;
			provider = new CWaveLevelPropertyProvider(hReturnWnd, document, zzub_wavelevel_get_wave((zzub_wavelevel_t*)ev->show_properties.wavelevel), (zzub_wavelevel_t*)ev->show_properties.wavelevel);
			dirtyProperties = true;
			Invalidate(FALSE);
			break;
	}
}

void CPropertyListView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	//if (document->selectedPropertyProvider == 0) return ;

	buze_event_data_t* ev = (buze_event_data_t*)pHint;
	zzub_event_data_t* zzubdata = (zzub_event_data_t*)pHint;

	switch (lHint) {
		case buze_event_type_update_new_document:
			ClearAll();
			break;
		case zzub_event_type_delete_plugin:
		case zzub_event_type_delete_plugin_group:
		case zzub_event_type_delete_pattern:
		case zzub_event_type_delete_wave:
		case zzub_event_type_delete_wavelevel:
		case zzub_event_type_delete_patternformat:
		case zzub_event_type_delete_connection:
			if (provider && provider->checkUpdate(lHint, pHint)) {
				ClearAll();
				delete provider;
				provider = 0;
			}
			break;
		case zzub_event_type_update_plugin:
		case zzub_event_type_update_plugin_group:
		case zzub_event_type_insert_connection:
		case zzub_event_type_update_connection:
		case zzub_event_type_update_pattern:
		case zzub_event_type_insert_wave:
		case zzub_event_type_update_wave:
		case zzub_event_type_insert_wavelevel:
		case zzub_event_type_update_wavelevel:
		case zzub_event_type_update_wavelevel_samples:
		case zzub_event_type_insert_patternformat:
		case zzub_event_type_update_patternformat:
		case zzub_event_type_insert_patternformatcolumn:
		case zzub_event_type_update_patternformatcolumn:
		case zzub_event_type_delete_patternformatcolumn:
		case zzub_event_type_insert_patternformattrack:
		case zzub_event_type_update_patternformattrack:
		case zzub_event_type_delete_patternformattrack:
		//case buze_event_type_update_properties:
			// bind to document->selectedPropertyProvider
			dirtyProperties = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_update_pluginparameter:
			if (zzubdata->update_pluginparameter.group == 0) {
				// mute, bypass etc
				dirtyProperties = true;
				Invalidate(FALSE);
			}
			break;
		case buze_event_type_update_properties:
			if (provider && 
				((ev->show_properties.type == buze_property_type_plugin && provider->name == "machine") ||
				(ev->show_properties.type == buze_property_type_plugin_group && provider->name == "plugingroup") ||
				(ev->show_properties.type == buze_property_type_connection && provider->name == "connection") ||
				(ev->show_properties.type == buze_property_type_pattern && provider->name == "pattern") ||
				(ev->show_properties.type == buze_property_type_pattern_format && provider->name == "patternformat") ||
				(ev->show_properties.type == buze_property_type_wave && provider->name == "wave") ||
				(ev->show_properties.type == buze_property_type_wave_level && provider->name == "wavelevel"))
			) {
				UpdateFromEvent(pSender, ev);
			}
			break;
		case buze_event_type_show_properties:
			UpdateFromEvent(pSender, ev);
			break;
	}
}

LRESULT CPropertyListView::OnReturnToView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (provider && provider->hReturnView) {
		buze_main_frame_set_focus_to(mainframe, provider->hReturnView);
	}

	if (buze_main_frame_is_float_view(mainframe, this))
		buze_main_frame_close_view(mainframe, this);
	return 0;
}

LRESULT CPropertyListView::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// we need to process WM_MOUSEACTIVATE before the docktabframework as it sets focus causing 
	// several problems with our property list.
	buze_window_t* focusView = buze_main_frame_get_focused_view(mainframe);
	HWND hFocusWnd = (HWND)buze_window_get_wnd(focusView);
	if (hFocusWnd == *this) {
		return MA_ACTIVATE;
	} else
		return DefWindowProc();
}

LRESULT CPropertyListView::OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	propertyList.SetFocus();
	return 0;
}

void CPropertyListView::UpdateInfo() {
	int cursel = propertyList.GetCurSel();
	if (cursel == -1 || !provider) return ;
	PropertyInfoBase* prop = provider->getProperty(cursel);
	infoPane.SetWindowText(prop->description.c_str());
}
