#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "HistoryView.h"


#include "utils.h"
#include "PatternFormatView.h"

// for loading IDB_FOOBAR with proper colors
bool CreateImageList(WTL::CImageList &il, UINT nIDResource, int cx, int nGrow, COLORREF crMask);

// 
// Factory
//

CPatternFormatViewInfo::CPatternFormatViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CPatternFormatView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Pattern Format";
	tooltip = "Pattern Format";
	place = 2; //DockSplitTab::placeDOCKPANE;
	side = 2; //DockSplitTab::dockLEFT;
	serializable = true;
	allowfloat = false;
	defaultview = false;
}

CView* CPatternFormatViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CPatternFormatView* view = new CPatternFormatView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, pCreateData);
	return view;
}

void CPatternFormatViewInfo::Attach() {
	buze_document_add_view(document, this);

	// global accelerators - these generate global document events caught in OnUpdate
	// buze_main_frame_add_global_accelerator("view_patternformat", buze_event_type_show_pattern_format_view );
	WORD ID_SHOW_PATTERNFORMATIVIEW = buze_main_frame_register_accelerator_event(mainframe, "view_patternformat", "f2 shift", buze_event_type_show_pattern_format_view);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);
	buze_main_frame_register_accelerator(mainframe, "patternformatview", "returntoview", "escape", ID_PROPERTYVIEW_RETURNTOVIEW );

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_PATTERNFORMATIVIEW, "Pattern Format View");

}

void CPatternFormatViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CPatternFormatViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CPatternFormatView* view;
	switch (lHint) {
		case buze_event_type_show_pattern_format_view:
			view = (CPatternFormatView*)buze_main_frame_get_view(mainframe, "PatternFormatView", 0);
			if (view)
				buze_main_frame_set_focus_to(mainframe, view);
			else
				view = (CPatternFormatView*)buze_main_frame_open_view(mainframe, "PatternFormatView", "Pattern Format", 0, -1, -1);

			if (ev != 0 && ev->show_pattern_format.pattern_format != 0) {
				view->format = (zzub_pattern_format_t*)ev->show_pattern_format.pattern_format;
				view->dirtyFormat = true;
				view->Invalidate(FALSE);
			}
			break;
	}
}

//
// View
//

CPatternFormatView::CPatternFormatView(buze_main_frame_t* mainFrm)
:
	CViewImpl(mainFrm)
{
	format = 0;
	dirtyFormat = false;
	dirtyChecks = false;
	dirtyToolbar = false;
	dirtyToolbarSel = false;
}

CPatternFormatView::~CPatternFormatView() {
}

void CPatternFormatView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CPatternFormatView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
	buze_event_data* args = (buze_event_data*)cs->lpCreateParams;

	// remove the WS_CLIPCHILDREN style - this causes the view to receive WM_PAINTs when the view is
	// invalidated (in OnUpdate) in an inactive tab view. there was a problem with the view didnt rebind
	// e.g after renaming a plugin in the property view; where the property view and patternformat view 
	// are in the same pane but different tabs. in this case WM_PAINT wasnt invoked for the 
	// patternformatview.  removing WS_CLIPCHILDREN fixed it. 
	// this is more an indicator of a deeper problem by (ab)using the painting subsystem for rebinding:
	ModifyStyle(WS_CLIPCHILDREN, 0);

	HFONT defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	//formatDropDown.Create(m_hWnd, rcDefault, 0, /*CCS_NORESIZE|*/WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST, 0, IDC_FORMATDROPDOWN);
	formatDropDown.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_FORMATDROPDOWN);
	formatDropDown.SetFont(defaultFont);
	bool bLock = buze_configuration_get_toolbars_locked(buze_document_get_configuration(document)) != 0;
	insertToolbarBand(formatDropDown, "", 0, -1, true, bLock);

	// TODO: the filters should be label-less, and instead have a search-icon and a shaded label inside the textbox when the textbox is empty
	pluginFilter.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_FORMATPLUGINFILTER);
	pluginFilter.SetFont(defaultFont);
	insertToolbarBand(pluginFilter, "Plugin Filter", 0, -1, true, bLock, true);

	parameterFilter.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_FORMATPARAMETERFILTER);
	parameterFilter.SetFont(defaultFont);
	insertToolbarBand(parameterFilter, "Parameter Filter", 0, -1, true, bLock, true);

	formatTree.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VSCROLL, 0, IDC_PATTERNFORMATTREE);
	formatTree.ModifyStyle(0, TVS_HASLINES);
	CreateImageList(m_Images, IDB_PATTERNFORMAT_IMAGES, 16, 12, RGB(0,255,255));

	formatTree.Initialize(TRUE, FALSE);
	formatTree.SetImageList(m_Images, TVSIL_NORMAL);
	formatTree.SetSmartCheckBox(TRUE);
	formatTree.SetHtml(FALSE);
	formatTree.SetStripHtml(FALSE);
	formatTree.SetImages(TRUE);
	formatTree.SetSeparatorColor(RGB(0xA6,0xA6,0xA6));

	buze_document_add_view(document, this);
	buze_main_frame_viewstack_insert(mainframe, this); // true
	//mainframe->ViewStackInsert(m_hWnd, true);

	CModuleMessageLoop* messageLoop = _Module.GetMessageLoop();
	messageLoop->AddMessageFilter(this);

	if (args != 0) {
		format = (zzub_pattern_format_t*)args->show_pattern_format.pattern_format;
		dirtyFormat = true;
	}

	dirtyToolbar = true;

	return 0;
}

BOOL CPatternFormatView::PreTranslateMessage(MSG* pMsg)
{
	if (GetFocus() == *this || GetFocus() == formatTree) {
		HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "patternformatview");
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg)) {
			return TRUE;
		}
	}
	return FALSE;
}

LRESULT CPatternFormatView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CModuleMessageLoop* messageLoop = _Module.GetMessageLoop();
	messageLoop->RemoveMessageFilter(this);

	buze_document_remove_view(document, this);
	//mainframe->closeClientWindow(m_hWnd);

	return 0;
}

LRESULT CPatternFormatView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	m_Images.Destroy();
	bHandled = FALSE;
	return 0;
}

LRESULT CPatternFormatView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	WORD cx = LOWORD(lParam);
	WORD cy = HIWORD(lParam);

	if (formatTree.m_hWnd) {
		formatTree.MoveWindow(0, getToolbarHeight(), cx, cy-getToolbarHeight());
	}

	return 0;
}

LRESULT CPatternFormatView::OnPaint(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (dirtyFormat) {
		dirtyChecks = false;
		dirtyFormat = false;
		BindFormat();
	}

	if (dirtyChecks) {
		dirtyChecks = false;
		SetFormatChecks();
	}

	if (dirtyToolbar) {
		dirtyToolbarSel = false;
		dirtyToolbar = false;
		bindDropDown();
	}

	if (dirtyToolbarSel) {
		setDropDown();
	}

	return DefWindowProc();
}

//LRESULT CPatternFormatView::OnErase(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//{
//	return 1;
//}

LRESULT CPatternFormatView::OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	formatTree.SetFocus();
	return 0;
}

LRESULT CPatternFormatView::OnRestoreFocus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	formatTree.SetFocus();
	return 0;
}

LRESULT CPatternFormatView::OnSelChangeFormat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int sel = formatDropDown.ctrl().GetCurSel();
	if (sel == -1) return 0;

	zzub_pattern_format_t* newformat = zzub_player_get_pattern_format_by_index(player, sel);

	if (format == 0) {	
		dirtyFormat = true;
	} else
	if (format != newformat) {
		dirtyChecks = true;
	}

	format = newformat;

	buze_event_data_t ev;
	ev.show_properties.type = buze_property_type_pattern_format;
	ev.show_properties.pattern_format = format;
	ev.show_properties.return_view = this;
	buze_document_notify_views(document, this, buze_event_type_update_properties, &ev);

	Invalidate(FALSE);
	return 0;
}

LRESULT CPatternFormatView::OnPluginFilterChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	dirtyFormat = true;
	Invalidate(FALSE);
	return 0;
}

LRESULT CPatternFormatView::OnParameterFilterChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	dirtyFormat = true;
	Invalidate(FALSE);
	return 0;
}

void CPatternFormatView::GetPluginFilter(std::vector<std::string>& result) {
	char pcText[1024];
	pluginFilter.m_edit.GetWindowText(pcText, 1024);
	strlwr(pcText);
	split(pcText, result, " ");
}

void CPatternFormatView::GetParameterFilter(std::vector<std::string>& result) {
	char pcText[1024];
	parameterFilter.m_edit.GetWindowText(pcText, 1024);
	strlwr(pcText);
	split(pcText, result, " ");
}

void CPatternFormatView::bindDropDown()
{
	formatDropDown.SetRedraw(FALSE);
	{
		formatDropDown.ctrl().ResetContent();

		int i = 0;
		int selected_format = -1;
		zzub_pattern_format_iterator_t* fit = zzub_player_get_pattern_format_iterator(player);
		while (zzub_pattern_format_iterator_valid(fit)) {
			zzub_pattern_format_t* format = zzub_pattern_format_iterator_current(fit);
			if (this->format == format) selected_format = i;

			const char* name = zzub_pattern_format_get_name(format);
			formatDropDown.ctrl().InsertString(formatDropDown.ctrl().GetCount(), name);
			zzub_pattern_format_iterator_next(fit);
			++i;
		}
		zzub_pattern_format_iterator_destroy(fit);
		
		formatDropDown.ctrl().SetCurSel(selected_format);
	}
	formatDropDown.SetRedraw(TRUE);
	formatDropDown.RedrawWindow(0, 0, RDW_ALLCHILDREN|RDW_INVALIDATE);
}

void CPatternFormatView::setDropDown()
{
	int i = 0;
	int selected_format = -1;
	zzub_pattern_format_iterator_t* fit = zzub_player_get_pattern_format_iterator(player);
	while (zzub_pattern_format_iterator_valid(fit)) {
		zzub_pattern_format_t* format = zzub_pattern_format_iterator_current(fit);
		if (this->format == format) selected_format = i;
		zzub_pattern_format_iterator_next(fit);
		++i;
	}
	zzub_pattern_format_iterator_destroy(fit);

	formatDropDown.ctrl().SetCurSel(selected_format);
}

namespace {
void zzub_pattern_format_update_column(bool enabled, zzub_pattern_format_t* format, zzub_plugin_t* plugin, int group, int track, int column)
{
	zzub_pattern_format_column_t* col = zzub_pattern_format_get_column(format, plugin, group, track, column);

	if (enabled && (col == NULL)) {
		zzub_pattern_format_add_column(format, plugin, group, track, column, -1);
	} else
	if (!enabled && (col != NULL)) {
		zzub_pattern_format_delete_column(format, plugin, group, track, column);
	}
}
} // END namespace

LRESULT CPatternFormatView::OnCheckChange(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	zzub_player_history_begin(player, this); // userdata is 'this'

	XHTMLTREEMSGDATA* msgdata = (XHTMLTREEMSGDATA*)wParam;
	HTREEITEM hItem = msgdata->hItem;
	BOOL bCheck = formatTree.GetCheckState(hItem);
	COLUMNINFO& colinfo = columnmap[hItem];

	if (colinfo.is_plugin_category == true) { // clicked checkbox on plugin
		int virtualparam_count = zzub_plugin_get_parameter_count(colinfo.plugin, 4, 0);
		for (int i = 0; i < virtualparam_count; ++i) {
			zzub_pattern_format_update_column(bCheck != 0, format, colinfo.plugin, 4, 0, i);
		}
		int internalparam_count = zzub_plugin_get_parameter_count(colinfo.plugin, 0, 0);
		for (int i = 0; i < internalparam_count; ++i) {
			zzub_pattern_format_update_column(bCheck != 0, format, colinfo.plugin, 0, 0, i);
		}
		int globalparam_count = zzub_plugin_get_parameter_count(colinfo.plugin, 1, 0);
		for (int i = 0; i < globalparam_count; ++i) {
			zzub_pattern_format_update_column(bCheck != 0, format, colinfo.plugin, 1, 0, i);
		}
		int track_count = zzub_plugin_get_track_count(colinfo.plugin, 2);
		for (int j = 0; j < track_count; ++j) {
			int trackparam_count = zzub_plugin_get_parameter_count(colinfo.plugin, 2, j);
			for (int i = 0; i < trackparam_count; ++i) {
				zzub_pattern_format_update_column(bCheck != 0, format, colinfo.plugin, 2, j, i);
			}
		}
	} else { // clicked checkbox on parameter
		zzub_pattern_format_update_column(bCheck != 0, format, colinfo.plugin, colinfo.group, colinfo.track, colinfo.column);
	}

	zzub_player_history_commit(player, 0, 0, "Add/Remove Pattern Format Column(s)");
	return 0;
}

void CPatternFormatView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint)
{
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;
	buze_event_data_t* ev = (buze_event_data_t*)pHint;

	switch (lHint) {
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_new_document:
			pluginFilter.m_edit.SetWindowTextA("");
			parameterFilter.m_edit.SetWindowTextA("");
			dirtyFormat = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_insert_patternformat:
			dirtyToolbar = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_delete_patternformat:
			if (zzubData->delete_pattern_format.patternformat == format) {
				format = 0;
				dirtyFormat = true;
			}
			dirtyToolbar = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_update_patternformat:
			dirtyToolbar = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_insert_plugin:
		case zzub_event_type_delete_plugin:
		case zzub_event_type_update_plugin: // TODO: update_plugin is bad cause it rebinds this editor every time a plugin is moved in the machine view
			dirtyFormat = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_insert_patternformatcolumn:
			if (zzubData && zzubData->userdata == this) {
				break; // no rebind if we checkedboxed from this editor
			} else {
				zzub_pattern_format_column_t* col = zzubData->insert_pattern_format_column.patternformatcolumn;
				HTREEITEM hItem = fmtCol2ht(col);
				if (hItem != NULL) { // this patternformatcolumn exists, got changed from another editor
					if (formatTree.GetCheck(hItem) == FALSE) {
						formatTree.SetCheck(hItem, TRUE); // bSendWM defaults to FALSE
					}
				} else { // this patternformatcolumn doesn't exist in our columnmap, rebind
					dirtyFormat = true;
					Invalidate(FALSE);
				}
			}
			break;
		case zzub_event_type_delete_patternformatcolumn:
			if (zzubData && zzubData->userdata == this) {
				break;
			} else {
				zzub_pattern_format_column_t* col = zzubData->delete_pattern_format_column.patternformatcolumn;
				HTREEITEM hItem = fmtCol2ht(col);
				if (hItem != NULL) {
					if (formatTree.GetCheck(hItem) == TRUE) {
						formatTree.SetCheck(hItem, FALSE);
					}
				} else {
					dirtyFormat = true;
					Invalidate(FALSE);
				}
			}
			break;
		case buze_event_type_update_properties:
			if (pSender != this) {
				if (ev->show_properties.type == buze_property_type_pattern) {
					Init(zzub_pattern_get_format((zzub_pattern_t*)ev->show_properties.pattern));
				} else 
				if (ev->show_properties.type == buze_property_type_pattern_format) {
					Init((zzub_pattern_format_t*)ev->show_properties.pattern_format);
				}
			}
			break;
		
//		case zzub_event_type_insert_patternformatcolumn:
//		case zzub_event_type_delete_patternformatcolumn:
//			if (data.zzubData && data.zzubData->userdata == this) break; // no rebind if we checkedboxed from this editor
//			dirtyFormat = true;
//			Invalidate(FALSE);
//			break;
	}
}

void CPatternFormatView::Init(zzub_pattern_format_t* format)
{
	if (this->format == 0 || format == 0) {
		this->format = format;
		dirtyFormat = true;
		dirtyToolbar = true;
		Invalidate(FALSE);
	} else
	if (this->format != format) {
		this->format = format;
		dirtyChecks = true;
		dirtyToolbarSel = true;
		Invalidate(FALSE);
	}
}

HTREEITEM CPatternFormatView::fmtCol2ht(zzub_pattern_format_column_t* col) const {
	zzub_plugin_t* plugin = zzub_pattern_format_column_get_plugin(col);
	int group = zzub_pattern_format_column_get_group(col);
	int track = zzub_pattern_format_column_get_track(col);
	int column = zzub_pattern_format_column_get_column(col);
	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, track, column);
	return paramTrk2ht(param, track);
}

HTREEITEM CPatternFormatView::paramTrk2ht(zzub_parameter_t* param, int track) const {
	treemap_t::const_iterator it = treemap.find(std::make_pair(param, track));
	if (it == treemap.end()) {
		return NULL;
	} else {
		return it->second;
	}
}

bool filterPlugin(zzub_plugin_t* plugin, std::vector<std::string>& pluginfilter) {

	if (pluginfilter.size() == 0) return true;

	static char pcName[256];
	strncpy(pcName, zzub_plugin_get_name(plugin), 256);

	strlwr(pcName);

	std::vector<std::string> pluginname;
	split(pcName, pluginname, " ");

	int counter = 0;
	for (std::vector<std::string>::iterator i = pluginname.begin(); i != pluginname.end(); ++i) {
		for (std::vector<std::string>::iterator filterit = pluginfilter.begin(); filterit != pluginfilter.end(); ++filterit) {
			if (filterit->length() == 0) continue;
			counter++;
			if (i->find(*filterit) == 0) {
				return true;
			}
		}
	}
	return counter == 0;
}

bool visiblePlugin(CDocument* document, zzub_plugin_t* plugin, std::vector<std::string>& pluginfilter) {
	int flags = zzub_plugin_get_flags(plugin);
	// hide if this is a connection plugin going to or from a non-song-plugin
	if ((flags & zzub_plugin_flag_is_connection) != 0) {
		zzub_connection_t* conn = zzub_plugin_get_connection(plugin);

		zzub_plugin_t* fromplug = zzub_connection_get_from_plugin(conn);
		zzub_plugin_t* toplug = zzub_connection_get_from_plugin(conn);
		if (buze_document_get_plugin_non_song(document, fromplug) || buze_document_get_plugin_non_song(document, toplug))
			return false;
	}

	return !buze_document_get_plugin_non_song(document, plugin) && filterPlugin(plugin, pluginfilter);
}

bool visibleParameter(zzub_plugin_t* plugin, int group, int column, std::vector<std::string>& parameterfilter) {

	if (parameterfilter.size() == 0) return true;

	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, 0, column);
	static char pcName[1024];
	strcpy(pcName, zzub_parameter_get_name(param));

	strlwr(pcName);

	std::vector<std::string> tokens;
	split(pcName, tokens, " ");

	int counter = 0;
	for (std::vector<std::string>::iterator i = tokens.begin(); i != tokens.end(); ++i) {
		for (std::vector<std::string>::iterator filterit = parameterfilter.begin(); filterit != parameterfilter.end(); ++filterit) {
			if (filterit->length() == 0) continue;
			counter++;
			if (i->find(*filterit) == 0) {
				return true;
			}
		}
	}
	return counter == 0;
}

int grouporder[] = { 4, 0, 1, 2 };
int groupordercount = sizeof(grouporder) / sizeof(int);

void CPatternFormatView::SetFormatChecks()
{
	std::vector<std::string> pluginfilter, parameterfilter;
	GetPluginFilter(pluginfilter);
	GetParameterFilter(parameterfilter);

	SetRedraw(FALSE);
	{
		zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
		while (zzub_plugin_iterator_valid(plugit)) {
			zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
			if (visiblePlugin(document, plugin, pluginfilter)) {
				for (int groupindex = 0; groupindex < groupordercount; groupindex++) {
					int group = grouporder[groupindex];
					int track_count = zzub_plugin_get_track_count(plugin, group);
					for (int track = 0; track < track_count; ++track) {
						int param_count = zzub_plugin_get_parameter_count(plugin, group, track);
						for (int i = 0; i < param_count; ++i) {
							if (visibleParameter(plugin, group, i, parameterfilter)) {
	 							zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, track, i);
 								zzub_pattern_format_column_t* col = zzub_pattern_format_get_column(format, plugin, group, track, i);
 								HTREEITEM hItem = paramTrk2ht(param, track);
								const bool bChecked = (col != NULL);
								formatTree.SetCheck(hItem, bChecked, FALSE);
							}
						}
					}
				}
			}
			zzub_plugin_iterator_next(plugit);
		}
		zzub_plugin_iterator_destroy(plugit);
	}
	SetRedraw(TRUE);
	RedrawWindow(0, 0, RDW_ALLCHILDREN|RDW_INVALIDATE);
}

void CPatternFormatView::BindFormat()
{
	if (format == 0) {
		columnmap.clear();

		SetRedraw(FALSE);
		{
			formatTree.DeleteAllItems();
		}
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_ALLCHILDREN|RDW_INVALIDATE);
	} else {
		std::vector<std::string> pluginfilter;
		GetPluginFilter(pluginfilter);

		HTREEITEM hSelItem = formatTree.GetSelectedItem();
		COLUMNINFO* ciSelItem; 
		if (hSelItem != 0) {
			ciSelItem = new COLUMNINFO(columnmap[hSelItem]);
		} else {
			ciSelItem = 0;
		}

		columnmap.clear();
		treemap.clear();

		HTREEITEM foundSel = 0;
		int scroll = formatTree.GetScrollPos(SB_VERT);

		SetRedraw(FALSE);
		{
			formatTree.DeleteAllItems();

			zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
			while (zzub_plugin_iterator_valid(plugit)) {
				zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
				// do not permit binding of visualizers, built-in recorders, etc
				if (visiblePlugin(document, plugin, pluginfilter)) 
					BindPlugin(plugin, ciSelItem, foundSel);
				zzub_plugin_iterator_next(plugit);
			}
			zzub_plugin_iterator_destroy(plugit);

			formatTree.ExpandAll();

			if (foundSel) {
				formatTree.SetScrollPos(SB_VERT, scroll);
				formatTree.SelectItem(foundSel);
				formatTree.EnsureVisible(foundSel);
			}
		}
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_ALLCHILDREN|RDW_INVALIDATE);

		if (ciSelItem) delete ciSelItem;
	}
}

const char* zzub_plugin_get_connection_name(zzub_plugin_t* plugin) {
	assert(zzub_plugin_get_flags(plugin) & zzub_plugin_flag_is_connection);

	zzub_connection_t* conn = zzub_plugin_get_connection(plugin);
	assert(conn != 0);

	zzub_plugin_t* fp = zzub_connection_get_from_plugin(conn);
	zzub_plugin_t* tp = zzub_connection_get_to_plugin(conn);

	std::stringstream strm;
	strm << zzub_plugin_get_name(fp);
	strm << "->" << zzub_plugin_get_name(tp);
	strm << " (" << zzub_plugin_get_name(plugin) << ")";

	static char name[256];
	strncpy(name, strm.str().c_str(), 256);
	return name;
}

void CPatternFormatView::BindPlugin(zzub_plugin_t* plugin, COLUMNINFO* ciSelItem, HTREEITEM& foundSel)
{
	int flags = zzub_plugin_get_flags(plugin);

	const char* name;
	if (flags & zzub_plugin_flag_is_connection)
		name = zzub_plugin_get_connection_name(plugin);
	else
		name = zzub_plugin_get_name(plugin);

	std::vector<std::string> parameterfilter;
	GetParameterFilter(parameterfilter);

	TVINSERTSTRUCT tvis = { 0 };
	tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvis.item.pszText = (char*)name;
	tvis.item.cchTextMax = (int)_tcslen((char*)name);
	tvis.item.iImage = TV_NOIMAGE;
	tvis.item.iSelectedImage = TV_NOIMAGE;
	tvis.item.lParam = 0;
	//tvis.hParent = ;
	tvis.hInsertAfter = TVI_LAST;

	int img;
	if (flags & zzub_plugin_flag_is_root) {
		img = 0; // icon of master
	} else
	if (flags & zzub_plugin_flag_is_sequence) {
		img = 5; // icon of sequencer
	} else
	if (flags & zzub_plugin_flag_is_connection) {
		img = 4; // icon of arrow
	} else
	if (flags & IS_CONTROLLER_PLUGIN_FLAGS) {
		img = 3; // icon of controller
	} else
	if (flags & IS_EFFECT_PLUGIN_FLAGS) {
		img = 2; // icon of effect
	} else
	if ((flags & PLUGIN_FLAGS_MASK) == IS_GENERATOR_PLUGIN_FLAGS) {
		img = 1; // icon of generator
	} else {
		img = TV_NOIMAGE;
	}
	tvis.item.iImage = img;
	tvis.item.iSelectedImage = img;

	HTREEITEM hPlugItem = formatTree.InsertItem(&tvis);
	formatTree.SetItemBold(hPlugItem, TRUE);

	// bind virtual params
	int virtualparam_count = zzub_plugin_get_parameter_count(plugin, 4, 0);
	for (int i = 0; i < virtualparam_count; ++i) {
		if (visibleParameter(plugin, 4, i, parameterfilter))
			BindParameter(hPlugItem, plugin, 4, 0, i, ciSelItem, foundSel);
	}
	// bind internal params
	int internalparam_count = zzub_plugin_get_parameter_count(plugin, 0, 0);
	for (int i = 0; i < internalparam_count; ++i) {
		if (visibleParameter(plugin, 0, i, parameterfilter))
			BindParameter(hPlugItem, plugin, 0, 0, i, ciSelItem, foundSel);
	}
	formatTree.InsertSeparator(formatTree.GetLastItem(hPlugItem));
	// bind global params
	int globalparam_count = zzub_plugin_get_parameter_count(plugin, 1, 0);
	for (int i = 0; i < globalparam_count; ++i) {
		if (visibleParameter(plugin, 1, i, parameterfilter))
			BindParameter(hPlugItem, plugin, 1, 0, i, ciSelItem, foundSel);
	}
	// insert separator between globals and track params
	int track_count = zzub_plugin_get_track_count(plugin, 2);
	if (track_count) {
		formatTree.InsertSeparator(formatTree.GetLastItem(hPlugItem));
	}
	// bind track params
	for (int j = 0; j < track_count; ++j) {
		int trackparam_count = zzub_plugin_get_parameter_count(plugin, 2, j);
		for (int i = 0; i < trackparam_count; ++i) {
			if (visibleParameter(plugin, 2, i, parameterfilter))
				BindParameter(hPlugItem, plugin, 2, j, i, ciSelItem, foundSel);
		}
	}

	COLUMNINFO ci;
	ci.plugin = plugin;
	ci.group = -1;
	ci.track = -1;
	ci.column = -1;
	ci.is_plugin_category = true;
	columnmap[hPlugItem] = ci;

	if (ciSelItem != 0 && ciSelItem->is_plugin_category == true) {
		if (ciSelItem->plugin == ci.plugin) {
			foundSel = hPlugItem;
		}
	}
}

void CPatternFormatView::BindParameter(HTREEITEM hPlugItem, zzub_plugin_t* plugin, int group, int track, int column, COLUMNINFO* ciSelItem, HTREEITEM& foundSel)
{
	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, track, column);
	std::stringstream strm;
	if (group == 2) strm << track << ":";
	strm << zzub_parameter_get_name(param);
	std::string name = strm.str();

	TVINSERTSTRUCT tvis = { 0 };
	tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
	tvis.item.pszText = (char*)name.c_str();
	tvis.item.cchTextMax = (int)name.length();//_tcslen((char*)name);
	tvis.item.iImage = TV_NOIMAGE;
	tvis.item.iSelectedImage = TV_NOIMAGE;
	tvis.item.lParam = 0;
	tvis.hParent = hPlugItem;
	tvis.hInsertAfter = TVI_LAST;

	int img;
	int type = zzub_parameter_get_type(param);
	int flags = zzub_parameter_get_flags(param);
	if (type == zzub_parameter_type_note) {
		img = 7; // icon of note
	} else
	if (flags & zzub_parameter_flag_wavetable_index) {
		img = 9; // icon of wave
	} else
	if (flags & zzub_parameter_flag_pattern_index) {
		img = 10; // icon of trigger
	} else
	if (flags & zzub_parameter_flag_velocity_index) {
		img = 11; // icon of volume
	} else
	if (flags & zzub_parameter_flag_state)	{
		img = 6; // icon of "00"
	} else {
		img = 8; // icon of slider
	}
	tvis.item.iImage = img;
	tvis.item.iSelectedImage = img;

	HTREEITEM hParamItem = formatTree.InsertItem(&tvis);

	zzub_pattern_format_column_t* col = zzub_pattern_format_get_column(format, plugin, group, track, column);
	bool bChecked = (col != NULL);
	formatTree.SetCheck(hParamItem, bChecked, FALSE);

	COLUMNINFO ci;
	ci.plugin = plugin;
	ci.group = group;
	ci.track = track;
	ci.column = column;
	ci.is_plugin_category = false;
	columnmap[hParamItem] = ci;

	treemap[std::make_pair(param, track)] = hParamItem;

	if (ciSelItem != 0 && ciSelItem->is_plugin_category == false) {
		if (true
			&& ciSelItem->plugin == ci.plugin
			&& ciSelItem->group == ci.group
			&& ciSelItem->track == ci.track
			&& ciSelItem->column == ci.column
		) {
			foundSel = hParamItem;
		}
	}
}
