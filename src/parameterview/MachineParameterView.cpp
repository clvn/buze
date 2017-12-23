#include "stdafx.h"
#include <sstream>
#include <iomanip>
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "MachineParameterView.h"
#include "PresetDialog.h"
#include "utils.h"
#include "Keymaps.h"

//#include "..\armstrong\src\mixing\timer.h"

static const int WM_SHOW_PARAMVIEW = WM_USER+15; // wParam = mode, lParam = machine

CHostDllModule _Module;

// 
// Factory
//

CParameterViewInfo::CParameterViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CMachineParameterView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Parameters";
	tooltip = "Machine parameters and presets";
	place = 3; //DockSplitTab::placeFLOATFRAME;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = false;
	allowfloat = true;
	defaultview = false;
}


void CParameterViewInfo::Attach() {
	buze_document_add_view(document, this);

	//buze_main_frame_register_accelerator(mainframe, "parameter", "help", ID_HELP);

	buze_main_frame_register_accelerator(mainframe, "parameter", "slider_next", "down, tab", ID_PARAMETERVIEW_NEXTSLIDER);
	buze_main_frame_register_accelerator(mainframe, "parameter", "slider_previous", "up, tab shift", ID_PARAMETERVIEW_PREVIOUSSLIDER);
	buze_main_frame_register_accelerator(mainframe, "parameter", "slider_pageup", "pageup", ID_PARAMETERVIEW_PAGEUPSLIDER);
	buze_main_frame_register_accelerator(mainframe, "parameter", "slider_pagedown", "pagedown", ID_PARAMETERVIEW_PAGEDOWNSLIDER);

	buze_main_frame_register_accelerator(mainframe, "parameter", "move_right", "right", ID_PARAMETERVIEW_MOVERIGHT);
	buze_main_frame_register_accelerator(mainframe, "parameter", "move_someright", "right shift", ID_PARAMETERVIEW_MOVESOMERIGHT);
	buze_main_frame_register_accelerator(mainframe, "parameter", "move_pageright", "right ctrl shift", ID_PARAMETERVIEW_MOVEPAGERIGHT);
	buze_main_frame_register_accelerator(mainframe, "parameter", "move_left", "left", ID_PARAMETERVIEW_MOVELEFT);
	buze_main_frame_register_accelerator(mainframe, "parameter", "move_someleft", "left shift", ID_PARAMETERVIEW_MOVESOMELEFT);
	buze_main_frame_register_accelerator(mainframe, "parameter", "move_pageleft", "left ctrl shift", ID_PARAMETERVIEW_MOVEPAGELEFT);

	buze_main_frame_register_accelerator(mainframe, "parameter", "entervalue", "enter", ID_PARAMETERVIEW_ENTERVALUE);

	buze_main_frame_register_accelerator(mainframe, "parameter", "preset_dropdown", "p alt", ID_PARAMETERVIEW_PRESETS);
	buze_main_frame_register_accelerator(mainframe, "parameter", "preset_previous", "up ctrl", ID_PARAMETERVIEW_PREVIOUSPRESET);
	buze_main_frame_register_accelerator(mainframe, "parameter", "preset_next", "down ctrl", ID_PARAMETERVIEW_NEXTPRESET);

	buze_main_frame_register_accelerator(mainframe, "parameter", "preset_randomize", "r ctrl", ID_PRESET_RANDOMIZE);
	buze_main_frame_register_accelerator(mainframe, "parameter", "preset_humanize", "h ctrl", ID_PRESET_HUMANIZE);
}

void CParameterViewInfo::Detach() {
	buze_document_remove_view(document, this);
}


void CParameterViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	switch (lHint) {
		case buze_event_type_show_parameter_view:
			ShowParameterView(ev);
			break;
	}
}

void CParameterViewInfo::ShowParameterView(buze_event_data* ev) {
	zzub_plugin_t* plugin = (zzub_plugin_t*)ev->show_parameters.plugin;
	int plugin_id = zzub_plugin_get_id(plugin);
	int modehint = ev->show_parameters.mode;

	// TODO: store hints ++ locally in the parameterviewinfo

	int paramViewMode = buze_document_get_plugin_parameter_view_mode(document, plugin);

	if (modehint == parametermode_default) {
		if (paramViewMode == parametermode_default) {
			zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(plugin);
			zzub_plugincollection_t* collection = zzub_pluginloader_get_plugincollection(loader);

			const char* collnamestr = zzub_plugincollection_get_name(collection);
			std::string collname = collnamestr ? collnamestr : "";
			if (collname == "VST" && zzub_plugin_has_embedded_gui(plugin))
				modehint = parametermode_custom_only; // vst plugins default to embedded 
			else if (collname == "MFX" && zzub_plugin_has_embedded_gui(plugin))
				modehint = parametermode_both; // mfx plugins default to both
			else if (collname == "Lunar" && zzub_plugin_has_embedded_gui(plugin))
				modehint = parametermode_both; // Lunar plugins default to both
			else
				modehint = parametermode_internal_only;
		} else
			modehint = (MachineParameterViewMode)paramViewMode;
	}

	int prev_view_id = plugin_id * 4 + paramViewMode;
	if (prev_view_id == modehint) return ;

	CView* prevview = buze_main_frame_get_view(mainframe, "CMachineParameterView", prev_view_id);
	if (prevview != 0) buze_main_frame_close_view(mainframe, prevview);

	int view_id = plugin_id * 4 + modehint;

	std::string label = zzub_plugin_get_name(plugin);
	buze_document_set_plugin_parameter_view_mode(document, plugin, modehint);
	ev->show_parameters.mode = modehint;

	buze_main_frame_open_view(mainframe, "CMachineParameterView", label.c_str(), view_id, ev->show_parameters.x, ev->show_parameters.y);
}

CView* CParameterViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CMachineParameterView* view = new CMachineParameterView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, pCreateData);
	return view;
}


class CParameterViewLibrary : public CViewLibrary {
public:
	virtual void Initialize(CViewFrame* host) {
		_Module.m_hostModule = buze_application_get_host_module(buze_main_frame_get_application(host));
		buze_main_frame_register_window_factory(host, new CParameterViewInfo(host));
	}

	virtual void Destroy() {
		delete this;
	}

	virtual int GetVersion() {
		return CViewLibrary::version;
	}

};

extern "C" CViewLibrary* buze_create_viewlibrary() {
	return new CParameterViewLibrary();
}

//
// View
//

using namespace std;

// TODO: make a drop down arrow on the copy toolbar like this: http://www.codeproject.com/docking/toolbar_droparrow.asp


// http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=1953619&SiteID=1
std::string hexencode(const std::string& input) {
	std::ostringstream ssOut;
	ssOut << std::setbase(16);
	for(std::string::const_iterator i = input.begin(); i != input.end(); ++i) {
		if(isalnum((unsigned char)*i))
			ssOut << *i;
		else
			ssOut << '%' << std::setw(2) << std::setfill('0') << ((unsigned int)(unsigned char)*i);
	}
	return ssOut.str();
}

std::string rewriteBuzzWrapperName(std::string const& uri) {
	if (uri.find("@zzub.org/buzz2zzub/") != 0) return uri;
	std::string out_uri = uri.substr(20);
	replace(out_uri.begin(), out_uri.end(), '+', ' ');
	return out_uri;
}

std::string getPresetName(std::string uri) {
	string name = rewriteBuzzWrapperName(uri);
	if (name == uri) return hexencode(name);
	return name;
}

int get_notecolumn_in_group(zzub_pluginloader_t* info, int group) {
	for (int i = 0; i < zzub_pluginloader_get_parameter_count(info, group); ++i) {
		zzub_parameter_t* param = zzub_pluginloader_get_parameter(info, group, i);
		int type = zzub_parameter_get_type(param);
		if (type == zzub_parameter_type_note) return i;
	}
	return -1;
}

bool get_note_info(zzub_pluginloader_t* info, int &note_group, int& note_column) {
	note_group = 1;
	note_column = get_notecolumn_in_group(info, note_group);
	if (note_column != -1) return true;

	note_group = 2;
	note_column = get_notecolumn_in_group(info, note_group);
	if (note_column != -1) return true;

	note_group = -1;
	note_column = -1;
	return false;
}


/***

	CMachineEmbeddedView

***/


CMachineEmbeddedView::CMachineEmbeddedView() {
	machine = 0;
}

CMachineEmbeddedView::~CMachineEmbeddedView(void) {
}

LRESULT CMachineEmbeddedView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// TODO: CreateMachine from lpCreateParam?
	return DefWindowProc();
}

LRESULT CMachineEmbeddedView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 0;
}

LRESULT CMachineEmbeddedView::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return DefWindowProc(); // TODO! only erase outside to the right of the embedded view
}

void CMachineEmbeddedView::SetMachine(zzub_plugin_t* p) {
	assert(m_hWnd != 0);

	machine = p;
	if (!machine) return ;

	if (!zzub_plugin_create_embedded_gui(machine, m_hWnd)) {
		machine = 0;
		return ;
	}
}

void CMachineEmbeddedView::Resize(int x, int y, int* width, int* height) {
	if (!machine) {
		*width = 0;
		*height = 0;
		MoveWindow(x, y, 0, 0);
		return ;
	}

	int hostwidth = *width;
	zzub_plugin_resize_embedded_gui(machine, m_hWnd, width, height);

	hostwidth = std::max(hostwidth, *width);
	MoveWindow(x, y, hostwidth, *height);

}


/***

	CMachineParameterView

***/

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DESTRUCTION
// ---------------------------------------------------------------------------------------------------------------

CMachineParameterView::CMachineParameterView(CViewFrame* mainFrm)
:
	CViewImpl(mainFrm),
	sliderView(buze_main_frame_get_player(mainFrm))
{
	machine = 0;
}

CMachineParameterView::~CMachineParameterView(void) {
	if (m_hWnd)
		DestroyWindow();
}

void CMachineParameterView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CMachineParameterView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	LRESULT lres = DefWindowProc();

	LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
	//buze_event_data* args = (buze_event_data*)cs->lpCreateParams;
	int view_id = (int)cs->lpCreateParams;
	int plugin_id = view_id / 4;
	zzub_plugin_t* argplugin = zzub_player_get_plugin_by_id(player, plugin_id);
	mode = (MachineParameterViewMode)(view_id % 4);

	//mode = (MachineParameterViewMode)args->show_parameters.mode;
	// TODO: sanitize modehint? 
	bool has_embeddedview = (mode == parametermode_custom_only) || (mode == parametermode_both);
	bool has_sliderview = (mode == parametermode_internal_only) || (mode == parametermode_both);

	SetRedraw(FALSE);
	{
		presetDropDown.Create(m_hWnd, rcDefault, "Preset", WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PRESETDROPDOWN);
		presetDropDown.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
		hWndButtonToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_PARAMETERS, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	
		sliderView.Create(m_hWnd, rcDefault, 0, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS);
		embeddedView.Create(m_hWnd, rcDefault, 0, WS_CHILD | WS_VISIBLE);

		SetMachine(argplugin);
		//SetMachine((zzub_plugin_t*)args->show_parameters.plugin);

		CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
	
		// set rebar width to 400 to make sure its height is calculated correctly when calling GetClientSize()
		// this works because the toolbarwindow ignores heightchanges and resizes of (0,0), 
		// otherwise the rebar would return its height given a width of 0, which is band-height * number 
		// of bands which is mostly wrong when there is more than one toolbar band:
		toolBar.SetWindowPos(0, 0, 0, 400, 100, SWP_NOMOVE|SWP_NOZORDER);

		bool bLock = buze_configuration_get_toolbars_locked(buze_document_get_configuration(document)) != 0;
		insertToolbarBand(presetDropDown, "&Preset", 100, -1, true, bLock);
		SIZE btbSize; SendMessage(hWndButtonToolBar, TB_GETMAXSIZE, 0, (LPARAM)&btbSize);
		insertToolbarBand(hWndButtonToolBar, "", btbSize.cx, -1, true, bLock);
	
		buze_document_add_view(document, this);
	}
	SetRedraw(TRUE);
	//RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);

	return lres;
}

void CMachineParameterView::GetClientSize(RECT* rc) {
	assert(machine != 0);

	RECT rcEmbed;
	embeddedView.GetClientRect(&rcEmbed);

	rc->left = 0;
	rc->top = 0;
	rc->right = 400;
	rc->bottom = getToolbarHeight();

	bool has_embeddedview = (mode == parametermode_custom_only) || (mode == parametermode_both);
	bool has_sliderview = (mode == parametermode_internal_only) || (mode == parametermode_both);

	if (has_embeddedview) {
		rc->right = std::max(rc->right, rcEmbed.right);
		rc->bottom += rcEmbed.bottom;
	}
	if (has_sliderview) {
		rc->bottom +=  sliderView.sliders.size() * CParameterSliderBar::sliderHeight;
	}

}

BOOL CMachineParameterView::PreTranslateMessage(MSG* pMsg) {
	//if (GetFocus() == *this || (selectedSlider < sliderView.sliders.size() && GetFocus() == sliderView.sliders[selectedSlider]->trackBar.m_slider))
	if (GetFocus() == *this || IsChild(GetFocus()) ) {
		HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "parameter");
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg))
			return TRUE;
	}
	return FALSE;
}

LRESULT CMachineParameterView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	buze_document_remove_view(document, this);

	this->machine = 0;  // in case machine is deleted and window updated before window is fully destroyed

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);

	//mainframe->closeClientWindow(m_hWnd);

	return 0;
}

LRESULT CMachineParameterView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HIMAGELIST hImageList = (HIMAGELIST)::SendMessage(hWndButtonToolBar, TB_SETIMAGELIST, 0, 0);
	ImageList_Destroy(hImageList);
	return 0;
}

LRESULT CMachineParameterView::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	if (machine == 0) return 0;

	RECT rc;
	GetClientRect(&rc);

	int tbh = getToolbarHeight();
	int embedwidth = rc.right;
	int embedheight = 250; //has_embeddedview ? 250 : 0; // default to 250px for embedded gui height

	embeddedView.Resize(0, tbh, &embedwidth, &embedheight);
	sliderView.MoveWindow(0, tbh + embedheight, rc.right, rc.bottom - tbh - embedheight);

	return 0;
}

LRESULT CMachineParameterView::OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return 1;
}

LRESULT CMachineParameterView::OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	//mainframe->showHelpParameters();
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// KEYBOARD
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineParameterView::OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// velg dette patternet i Patterns-vinduet
	return 0;
}

LRESULT CMachineParameterView::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if ((lParam & (1 << 30)) != 0) return 0;

	int note = keyboard_mapper::map_code_to_note(buze_document_get_octave(document), wParam);
	if (note > 0 && note != zzub_note_value_off && note != zzub_note_value_cut) note = midi_to_buzz_note(note);
	if (note == -1) return 0;
/*
	zzub_plugin_t* plugin = machine;
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(plugin);

	int note_group = -1, note_column = -1;
	if (!get_note_info(info, note_group, note_column)) {
		// paramview plugin doesnt have a note column, try the currently selected one
		if (mainframe->machineView.getViewCount() > 0) {
			CMachineView* machineView = mainframe->machineView.getView(0);
			if (machineView->getSelectedMachines() > 0) {
				plugin = machineView->getSelectedMachine(0);
				info = zzub_plugin_get_pluginloader(plugin);
				if (!get_note_info(info, note_group, note_column))
					return 0;
			} else
				return 0;
		} else
			return 0;
	}
*/
	zzub_plugin_t* plugin = zzub_player_get_midi_plugin(player);
	//zzub_plugin_t* m = zzub_player_get_midi_lock(player) ? zzub_player_get_midi_plugin(player): getSelectedMachine(0);
	if (plugin == 0) return 0;


	buze_document_keyjazz_key_down(document, plugin, wParam, note);
	buze_document_play_plugin_note(document, plugin, note, 0);
	return 0;
}

LRESULT CMachineParameterView::OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int lastnote;
	zzub_plugin_t* lastplugin = 0;

	buze_document_keyjazz_key_up(document, wParam, &lastnote, &lastplugin);
	if (lastplugin == 0) return 0;

	buze_document_play_plugin_note(document, lastplugin, zzub_note_value_off, lastnote);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// FOCUS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineParameterView::OnBlur(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// do not kill keyjazz if the target focus is (a child of) the machineview or something that handles keyjazz!
	HWND hFocusWnd = (HWND)wParam;
	CView* view = buze_main_frame_get_view_by_wnd(mainframe, hFocusWnd);
	if (view != 0 && view->DoesKeyjazz())
		return 0;

	// assume our child windows have forwards us their WM_KILLFOCUS-message
	//if (hFocusWnd != 0 && (hFocusWnd == m_hWnd || IsChild(hFocusWnd)))
		//return 0;

	buze_document_keyjazz_release(document, true);
	return 0;
}

LRESULT CMachineParameterView::OnSetFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if (machine == 0) return 0;

	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	assert(info);

	int note_group = -1, note_column = -1;
	if (get_note_info(info, note_group, note_column)) {
		// paramview plugin has a note column so we use the previously selected midi plugin
		zzub_player_set_midi_plugin(player, machine);
	}
	return 0;
}

LRESULT CMachineParameterView::OnRestoreFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	SetFocus();
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// SELECTED SLIDER OPERATIONS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineParameterView::OnMoveRight(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CParameterSliderBar* trackBar = sliderView.GetSelectedSlider();
	if (trackBar == 0) return 0;
	
	int value = trackBar->GetValue() + 1;
	trackBar->NotifyChange(value);
	return 0;
}

LRESULT CMachineParameterView::OnMoveSomeRight(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CParameterSliderBar* trackBar = sliderView.GetSelectedSlider();
	if (trackBar == 0) return 0;
	
	int value = trackBar->GetValue() + 4;
	trackBar->NotifyChange(value);
	return 0;
}

LRESULT CMachineParameterView::OnMovePageRight(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CParameterSliderBar* trackBar = sliderView.GetSelectedSlider();
	if (trackBar == 0) return 0;

	int value = trackBar->GetValue() + 32;
	trackBar->NotifyChange(value);
	return 0;
}

LRESULT CMachineParameterView::OnMoveLeft(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CParameterSliderBar* trackBar = sliderView.GetSelectedSlider();
	if (trackBar == 0) return 0;

	int value = trackBar->GetValue() - 1;
	trackBar->NotifyChange(value);
	return 0;
}

LRESULT CMachineParameterView::OnMoveSomeLeft(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CParameterSliderBar* trackBar = sliderView.GetSelectedSlider();
	if (trackBar == 0) return 0;

	int value = trackBar->GetValue() - 4;
	trackBar->NotifyChange(value);
	return 0;
}

LRESULT CMachineParameterView::OnMovePageLeft(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CParameterSliderBar* trackBar = sliderView.GetSelectedSlider();
	if (trackBar == 0) return 0;

	int value = trackBar->GetValue() - 32;
	trackBar->NotifyChange(value);
	return 0;
}

LRESULT CMachineParameterView::OnEnterValue(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CParameterSliderBar* slider = sliderView.GetSelectedSlider();
	if (slider == 0) return 0;

	CWindow focusWnd(GetFocus());
	CValueDialog valueDialog(*slider);
	valueDialog.SetValue(slider->GetValue());
	if (IDOK == valueDialog.DoModal(*this)) {
		int value = valueDialog.GetValue();
		slider->NotifyChange(value);
	}
	focusWnd.SetFocus();
	return 0;
}

LRESULT CMachineParameterView::OnNextSlider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (sliderView.sliders.size() == 0) return 0;

	int sel = (sliderView.GetSelectedSliderIndex() + 1) % sliderView.sliders.size();
	sliderView.SelectSlider(sel);
	sliderView.ScrollToView(sel);
	return 0;
}

LRESULT CMachineParameterView::OnPrevSlider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (sliderView.sliders.size() == 0) return 0;

	int sel = (sliderView.GetSelectedSliderIndex() + sliderView.sliders.size() - 1) % sliderView.sliders.size();
	sliderView.SelectSlider(sel);
	sliderView.ScrollToView(sel);
	return 0;
}

LRESULT CMachineParameterView::OnPageDownSlider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (sliderView.sliders.size() == 0) return 0;

	int sel = std::min(sliderView.GetSelectedSliderIndex() + 15, (int)sliderView.sliders.size() - 1);
	sliderView.SelectSlider(sel);
	sliderView.ScrollToView(sel);
	return 0;
}

LRESULT CMachineParameterView::OnPageUpSlider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	if (sliderView.sliders.size() == 0) return 0;

	int sel = std::max(sliderView.GetSelectedSliderIndex() - 15, 0);
	sliderView.SelectSlider(sel);
	sliderView.ScrollToView(sel);
	return 0;
}

LRESULT CMachineParameterView::OnHideSlider(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
/*	if (selectedSliderMachine == 0) return 0;
	document->setMachineParameterHidden(selectedSliderMachine, selectedSliderGroup, selectedSliderParam, 1);
	BindParameters();
	sliderView.Invalidate(TRUE);*/
	return 0;
}

LRESULT CMachineParameterView::OnUnhideSlider(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
/*	if (selectedSliderMachine == 0) return 0;
	int paramIndex = wID - ID_PARAMETER_UNHIDE_FIRST;

	int group = -1, column;
	int counter = 0;
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(selectedSliderMachine);
	for (int k = 0; k < 3; ++k) {
		for (int i = 0; i < zzub_pluginloader_get_parameter_count(loader, k); ++i) {
			zzub_parameter_t* param = zzub_plugin_get_parameter(selectedSliderMachine, k, 0, i);
			if ((zzub_parameter_get_flags(param) & zzub_parameter_flag_state) == 0) continue;
			
			bool paramHide = mainframe->document->getMachineParameterHidden(selectedSliderMachine, k, i);
			if (paramHide) {
				if (paramIndex == counter)  {
					group = k;
					column = i;
					break;
				}
				counter ++;
			}
		}
		if (group != -1) break;
	}

	if (group == -1) return 0;

	document->setMachineParameterHidden(selectedSliderMachine, group, column, 0);
	BindParameters();*/
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// PRESETS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineParameterView::OnNextPreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	int sel = presetDropDown.ctrl().GetCurSel();

	if (++sel < presetDropDown.ctrl().GetCount()) {
		presetDropDown.ctrl().SetCurSel(sel);
		SetPreset(sel);
		zzub_player_history_commit(player, 0, 0, "Set Next Preset");

	}
	return 0;
}

LRESULT CMachineParameterView::OnPreviousPreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	int sel = presetDropDown.ctrl().GetCurSel();
	if (--sel >= 0) {
		presetDropDown.ctrl().SetCurSel(sel);
		SetPreset(sel);
		zzub_player_history_commit(player, 0, 0, "Set Previous Preset");
	}
	return 0;
}

void CMachineParameterView::InitPresets() {
	if (machine == 0) {
		presets.machineName = "";
		BindPresets();
		return ;
	}

	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	std::string uri = zzub_pluginloader_get_uri(loader);
	std::string buzzPresetName = getPresetName(uri);
	std::string nativePresetName = hexencode(uri);
	std::string userPresetPath = buze_application_map_path(buze_main_frame_get_application(mainframe), "Presets\\", buze_path_type_user_path);
	std::string appPresetPath = buze_application_map_path(buze_main_frame_get_application(mainframe), "Gear\\Native\\", buze_path_type_app_path);
	std::string fxPresetPath = buze_application_map_path(buze_main_frame_get_application(mainframe), "Gear\\Effects\\", buze_path_type_app_path);
	std::string genPresetPath = buze_application_map_path(buze_main_frame_get_application(mainframe), "Gear\\Generators\\", buze_path_type_app_path);

	if (!presets.load(userPresetPath + nativePresetName + ".prs"))
		if (!presets.load(appPresetPath + nativePresetName + ".prs"))
			if (!presets.load(fxPresetPath + buzzPresetName + ".prs"))
				if (!presets.load(genPresetPath + buzzPresetName + ".prs"))
					presets.machineName = nativePresetName;

	BindPresets();

	// default to the last preset

	std::string lastPreset = buze_document_get_plugin_last_preset(document, machine);
	if (lastPreset.size()) {
		int index = presetDropDown.ctrl().FindStringExact(0, lastPreset.c_str());
		if (index > 0) {
			presetDropDown.ctrl().SetCurSel(index);
		}
	}
}

void CMachineParameterView::BindPresets() {
	presetDropDown.ctrl().ResetContent();

	if (machine == 0) return ;

	presetDropDown.SetRedraw(FALSE);

	// create and add default settings 
	defaultPreset.name = "<default>";
	defaultPreset.parameters = 0;

	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);

	for (int k = 1; k < 3; ++k) {
		for (int j = 0; j < zzub_plugin_get_track_count(machine, k); ++j) {
			for (int i = 0; i < zzub_pluginloader_get_parameter_count(info, k); ++i) {
				zzub_parameter_t* param = zzub_pluginloader_get_parameter(info, k, i);
				if ((zzub_parameter_get_flags(param) & zzub_parameter_flag_state) == 0) continue;

				defaultPreset.values[defaultPreset.parameters] = zzub_parameter_get_value_default(param);
				defaultPreset.parameters++;
			}
		}
	}

	presetDropDown.ctrl().InsertString(0, defaultPreset.name.c_str());
	presetDropDown.ctrl().SetItemData(0, (DWORD_PTR)&defaultPreset);

	for (size_t i = 0; i < presets.getPresetCount(); ++i) {
		PresetInfo& pi = presets.getPreset(i);
		presetDropDown.ctrl().InsertString(i+1, pi.name.c_str());
		presetDropDown.ctrl().SetItemData(i+1, (DWORD_PTR)&pi);
	}
	presetDropDown.SetRedraw(TRUE);
}

bool TrySavePreset(PresetManager& presets, const std::string& path, const std::string& filename) {
	bool success = false;
	if (PathFileExists(path.c_str()) != 0) {
		success = true;
	} else {
		success = CreateDirectory(path.c_str(), NULL) != 0;
	}
			
	if (success) {
		success = presets.save(path + "\\" + filename);
	}
	return success;
}

LRESULT CMachineParameterView::OnPresetEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (machine == 0) return 0;

	CPresetDialog presetDlg(&presets, player, machine);
	if (IDOK != presetDlg.DoModal()) {
		return 0;
	}
	std::string appPresetPath = buze_application_map_path(buze_main_frame_get_application(mainframe), "Gear\\Native\\", buze_path_type_app_path);
	std::string userPresetPath = buze_application_map_path(buze_main_frame_get_application(mainframe), "Presets\\", buze_path_type_user_path);

	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	std::string uri = zzub_pluginloader_get_uri(loader);
	std::string pluginName = hexencode(uri);

	bool success = false;
	// first try to save where the preset was loaded from, if any
	if (!presets.presetFile.empty()) {
		success = presets.save(presets.presetFile);
	}

	if (!success) {
		// then try to save in "Presets" in the user directory
		success = TrySavePreset(presets, userPresetPath, pluginName + ".prs");

		/* then try to save in "Gear\Native" in the application directory - or not!
		if (!success) {
			success = TrySavePreset(presets, appPresetPath, pluginName + ".prs");
		}*/
	}

	BindPresets();
	return 0;
}

LRESULT CMachineParameterView::OnMachineHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	//zzub_plugincollection_t *col  = zzub_pluginloader_get_plugincollection(info);
	std::vector<std::string> ext;
	bool found = false;
	ext.push_back("html");
	ext.push_back("txt");
	ext.push_back("pdf");
	std::string file = string( (char *)zzub_pluginloader_get_plugin_file(info) );
	if( file.size() != 0 ){
		file = file.substr( 0, file.rfind(".")+1 ); // strip extension
		// replace extension by possible (guessed) readible extensions
		for( int i = 0; i < ext.size(); i++ ){
			std::string doc = file + ext.at(i);
			if( GetFileAttributes( doc.c_str() ) == INVALID_FILE_ATTRIBUTES ) continue;
			found = true;		
			ShellExecute(this->m_hWnd,"open",doc.c_str(),"","",SW_SHOW );
		}
	}
	if( !found ) MessageBox("Sorry..no help file found","Machine help");
	return 0;
}

LRESULT CMachineParameterView::OnShowPresetsToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	presetDropDown.ctrl().ShowDropDown();
	presetDropDown.ctrl().SetFocus(); 
	return 0;
}

LRESULT CMachineParameterView::OnPresetRandomize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);

	for (int k = 1; k < 3; ++k) { // group
		for (int j = 0; j < zzub_plugin_get_track_count(machine, k); ++j) { // track
			for (int i = 0; i < zzub_pluginloader_get_parameter_count(info, k); ++i) { // param
				zzub_parameter_t* param = zzub_plugin_get_parameter(machine, k, j, i);
				if ((zzub_parameter_get_flags(param) & zzub_parameter_flag_state) == 0) continue;

				// scan for corresponding slider
				int found_slider = -1;
				for (int h = 0; h < sliderView.sliders.size(); ++h) {
					CParameterSliderBar* psc = sliderView.sliders[h];
					if ((k == psc->m_group) && (j == psc->m_track) && (i == psc->m_column)) {
						found_slider = h;
						break;
					}
				}
				if (found_slider == -1) continue;

				// exclude this slider from randomization if it's not in the random set
				if (!sliderView.sliders[found_slider]->m_shaded) continue;

				int minvalue = zzub_parameter_get_value_min(param);
				int maxvalue = zzub_parameter_get_value_max(param);
				int range = (maxvalue - minvalue) + 1;
				int val = minvalue + (rand() % range);

				zzub_plugin_set_parameter_value(machine, k, j, i, val, true);
			}
		}
	}

	zzub_plugin_tick(machine, 0);
	zzub_player_history_commit(player, 0, 0, "Randomize Parameter Editor");

	return 0;
}

LRESULT CMachineParameterView::OnPresetHumanize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);

	int deviation = 20;

	for (int k = 1; k < 3; ++k) { // group
		for (int j = 0; j < zzub_plugin_get_track_count(machine, k); ++j) { // track
			for (int i = 0; i < zzub_pluginloader_get_parameter_count(info, k); ++i) { // param
				zzub_parameter_t* param = zzub_plugin_get_parameter(machine, k, j, i);
				if ((zzub_parameter_get_flags(param) & zzub_parameter_flag_state) == 0) continue;

				// scan for corresponding slider
				int found_slider = -1;
				for (int h = 0; h < sliderView.sliders.size(); ++h) {
					CParameterSliderBar* psc = sliderView.sliders[h];
					if ((k == psc->m_group) && (j == psc->m_track) && (i == psc->m_column)) {
						found_slider = h;
						break;
					}
				}
				if (found_slider == -1) continue;

				// exclude this slider from randomization if it's not in the random set
				if (!sliderView.sliders[found_slider]->m_shaded) continue;

				int minvalue = zzub_parameter_get_value_min(param);
				int maxvalue = zzub_parameter_get_value_max(param);
				int range = (maxvalue - minvalue) + 1;
				int deviance = std::max(2, range / deviation);

				
				CParameterSliderBar& trackBar = *sliderView.sliders[found_slider];
				int old_val = trackBar.GetValue();
				int rand_offset = (deviance % 2 == 1) ?			// odd or even?
					(rand() % deviance) - (deviance / 2)		// odd
				:	(rand() % (deviance + 1)) - (deviance / 2)	// even
				;
				int val = clamp(old_val + rand_offset, minvalue, maxvalue);

				zzub_plugin_set_parameter_value(machine, k, j, i, val, true);
				
			}
		}
	}

	zzub_plugin_tick(machine, 0);
	zzub_player_history_commit(player, 0, 0, "Humanize Parameter Editor");

	return 0;
}

LRESULT CMachineParameterView::OnPresetMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	// get out of all handlers (for wine, where sometimes the rebards LBUTTONUP causes problems)
	PostMessage(WM_TOGGLEPRESETMODE);
	return 0;
}

LRESULT CMachineParameterView::OnTogglePresetMode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	RECT rcWindow;
	GetWindowRect(&rcWindow);

	// if possible, close existing param view, and reopen with custom gui
	if (mode == parametermode_internal_only) {
		if (zzub_plugin_has_embedded_gui(machine))
			buze_main_frame_show_plugin_parameters(mainframe, machine, parametermode_custom_only, rcWindow.left, rcWindow.top);
	} else if (mode == parametermode_custom_only) {
		buze_main_frame_show_plugin_parameters(mainframe, machine, parametermode_both, rcWindow.left, rcWindow.top);
	} else if (mode == parametermode_both) {
		buze_main_frame_show_plugin_parameters(mainframe, machine, parametermode_internal_only, rcWindow.left, rcWindow.top);
	}
	return 0;
}

LRESULT CMachineParameterView::OnPresetCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
/*	bool hasGlobal = machine->loader->plugin_info->global_parameters.size() > 0;
	bool hasTrack = machine->loader->plugin_info->track_parameters.size() > 0;

	CMenu menu; 
	menu.CreatePopupMenu();
	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_PRESET_COPY_ALL, "Copy All Values");
	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING|(hasGlobal?0:MF_GRAYED), (UINT_PTR)ID_PRESET_COPY_GLOBAL, "Copy Global Values");
	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING|(hasTrack?0:MF_GRAYED), (UINT_PTR)ID_PRESET_COPY_TRACK, "Copy Track Values");

	CToolBarCtrl toolBar;
	toolBar.Attach(hWndButtonToolBar);
	RECT rcBar, rcButton;
	toolBar.GetItemRect(1, &rcButton);
	toolBar.ClientToScreen(&rcButton);
	toolBar.Detach();

	int buttonWidth = rcButton.right - rcButton.left;
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rcButton.left + buttonWidth, rcButton.top, m_hWnd, 0);
	*/
	return 0;
}

LRESULT CMachineParameterView::OnPresetCopyAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {

	// create a temp 1-row pattern w/0 inputs
/*	pattern temp(machine->loader->plugin_info, 0, machine->getTracks(), 1);

	// paste global state into pattern
	patterntrack* track = machine->getStateTrackCopy(1, 0);
	temp.pasteTrack(0, 0, track);

	// paste track states into pattern
	int col = machine->loader->plugin_info->global_parameters.size();
	for (int i = 0; i < machine->getTracks(); ++i) {
		track = machine->getStateTrackCopy(2, i);
		temp.pasteTrack(0, col, track);

		col += machine->loader->plugin_info->track_parameters.size();
	}

	// create a serializable track and serialize it into the clipboard
	track = temp.createRangeTrack(0, 0, 0, col-1);

	vector<char> buffer;
	zzub::mem_outstream outf(buffer);

	track->serialize(&outf);
	CopyBinary(m_hWnd, "Buze:PatternSelection", &buffer.front(), buffer.size());
	delete track;
*/
	return 0;
}

LRESULT CMachineParameterView::OnPresetCopyGlobal(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	// serialize global states to the clipboard
/*	patterntrack* track =machine->getStateTrackCopy(1, 0);
	vector<char> buffer;
	zzub::mem_outstream outf(buffer);

	track->serialize(&outf);
	CopyBinary(m_hWnd, "Buze:PatternSelection", &buffer.front(), buffer.size());
	*/
	return 0;
}

LRESULT CMachineParameterView::OnPresetCopyTrack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	// create a temp 1-row pattern w/0 inputs
	/*pattern temp(machine->loader->plugin_info, 0, machine->getTracks(), 1);

	// paste track states into pattern
	int first_col = machine->loader->plugin_info->global_parameters.size();
	int col = first_col;
	for (int i = 0; i < machine->getTracks(); ++i) {
		patterntrack* track = machine->getStateTrackCopy(2, i);
		temp.pasteTrack(0, col, track);

		col += machine->loader->plugin_info->track_parameters.size();
	}

	// create a serializable track with track values only
	patterntrack* track = temp.createRangeTrack(0, 0, first_col, col-1);

	vector<char> buffer;
	zzub::mem_outstream outf(buffer);

	track->serialize(&outf);
	CopyBinary(m_hWnd, "Buze:PatternSelection", &buffer.front(), buffer.size());
	delete track;
	*/
	return 0;
}

void CMachineParameterView::SetPreset(int presetIndex) {
	const PresetInfo& pi = *(const PresetInfo*)presetDropDown.ctrl().GetItemData(presetIndex);

	if (pi.savedata.size() > 0) {
		zzub_archive_t* archive = zzub_archive_create_memory();
		zzub_output_t* outf = zzub_archive_get_output(archive, "");
		zzub_output_write(outf, &pi.savedata.front(), pi.savedata.size());
		zzub_input_t* input = zzub_archive_get_input(archive, "");
		zzub_plugin_load(machine, input);

		zzub_archive_destroy(archive);
	}

	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);

	int valueIndex = 0;

	for (int k = 1; k < 3; ++k) {
		for (int j = 0; j < zzub_plugin_get_track_count(machine, k); ++j) {
			for (int i = 0; valueIndex < pi.parameters, i < zzub_pluginloader_get_parameter_count(info, k); ++i) {
				zzub_parameter_t* param = zzub_plugin_get_parameter(machine, k, j, i);
				if ((zzub_parameter_get_flags(param) & zzub_parameter_flag_state) == 0) continue;

				zzub_plugin_set_parameter_value(machine, k, j, i, pi.values[valueIndex], true);
				valueIndex++;
			}
		}
	}

	zzub_plugin_tick(machine, 0);

	buze_document_set_plugin_last_preset(document, machine, pi.name.c_str());
}

LRESULT CMachineParameterView::OnSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int presetIndex = presetDropDown.ctrl().GetCurSel();
	SetPreset(presetIndex);
	zzub_player_history_commit(player, 0, 0, "Set Preset");

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// BINDING
// ---------------------------------------------------------------------------------------------------------------

void CMachineParameterView::SetMachine(zzub_plugin_t* _machine) {
	machine = _machine;
	InitPresets();

	bool has_embeddedview = (mode == parametermode_custom_only) || (mode == parametermode_both);
	bool has_sliderview = (mode == parametermode_internal_only) || (mode == parametermode_both);

	if (has_embeddedview) embeddedView.SetMachine(machine);
	if (has_sliderview) BindParameters();
}

zzub_plugin_t* CMachineParameterView::GetMachine() {
	return machine;
}

void CMachineParameterView::BindParameters() {
	assert(machine != 0);

	std::vector<paramid> parameters;

	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	for (int k = 0; k < 3; ++k) {
		for (int j = 0; j < zzub_plugin_get_track_count(machine, k); ++j) {
			for (int i = 0; i < zzub_pluginloader_get_parameter_count(info, k); ++i) {
				zzub_parameter_t* param = zzub_plugin_get_parameter(machine, k, j, i);
				if ((zzub_parameter_get_flags(param) & zzub_parameter_flag_state) == 0) continue;

				//bool paramHide = mainframe->document->getMachineParameterHidden(machine, k, i);
				//if (paramHide) continue;

				paramid pp = { machine, k, j, i };
				parameters.push_back(pp);
			}
		}
	}

	sliderView.SetParameters(parameters);
}

// ---------------------------------------------------------------------------------------------------------------
// MOUSE
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineParameterView::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int d=(signed short)HIWORD(wParam);

	if (d<0) {
		this->sliderView.SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
	} else {
		this->sliderView.SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
	}
	return 0;
}


LRESULT CMachineParameterView::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (machine == 0) return 0;

	int selectedSlider = sliderView.GetSliderIndexFromHwnd((HWND)wParam);
	if (selectedSlider == -1) return 0;

	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	if (pt.x == -1 && pt.y == -1) {
		CParameterSliderBar* sliderCtrl = sliderView.sliders[selectedSlider];
		if (sliderCtrl) {
			// context menu from kb, set x, y from selected slider
			RECT rcTemp;
			sliderCtrl->GetClientRect(&rcTemp);
			pt.x = rcTemp.left + CParameterSliderBar::labelWidth;
			pt.y = rcTemp.top;
		}
	}

	CMenu menu; 
	menu.CreatePopupMenu();

	sliderView.SelectSlider(selectedSlider);

	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_PARAMETER_BIND, "Bind to MIDI-controller");
	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_PARAMETER_UNBIND, "Clear MIDI-binding");

	::ClientToScreen((HWND)wParam, &pt);

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// UPDATES
// ---------------------------------------------------------------------------------------------------------------

void CMachineParameterView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	zzub_plugin_t* updatePlugin = 0;
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;
	switch (lHint) {
		case zzub_event_type_delete_plugin: {
			if (zzubData->delete_plugin.plugin == machine)
				buze_main_frame_close_view(mainframe, this); 

			//if (data.zzubData->delete_plugin.plugin == selectedSliderMachine)
			//	selectedSliderMachine = 0;
			break;
		}
		case zzub_event_type_update_plugin:
			if (zzubData->update_plugin.plugin == machine) 
				BindParameters();
			break;
		case zzub_event_type_update_pluginparameter:
			if (zzubData->update_pluginparameter.plugin == machine)
				sliderView.SetParameter(zzubData->update_pluginparameter.plugin, zzubData->update_pluginparameter.group, zzubData->update_pluginparameter.track, zzubData->update_pluginparameter.param, zzubData->update_pluginparameter.value);
			break;
		case zzub_event_type_midi_control:
			selectMidiController.MidiEvent(zzubData->midi_message.status, zzubData->midi_message.data1, zzubData->midi_message.data2);
			break;
		default:
			break;
	}
}

// ---------------------------------------------------------------------------------------------------------------
// MIDI
// ---------------------------------------------------------------------------------------------------------------

#define midiValue(val, min, max) ((val * 128) / (max - min))

LRESULT CMachineParameterView::OnBindMidi(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	CParameterSliderBar* slider = sliderView.GetSelectedSlider();
	if (slider == 0) return 0;

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, slider->m_pluginid);

	zzub_parameter_t* parameter = zzub_plugin_get_parameter(plugin, slider->m_group, slider->m_track, slider->m_column);

	selectMidiController.bindMidiMachine = plugin;
	selectMidiController.bindMidiGroup = slider->m_group;
	selectMidiController.bindMidiTrack = slider->m_track;
	selectMidiController.bindMidiColumn = slider->m_column;

	int value = zzub_plugin_get_parameter_value(plugin, slider->m_group, slider->m_track, slider->m_column);

	selectMidiController.currentMidiValue = midiValue(value, zzub_parameter_get_value_min(parameter), zzub_parameter_get_value_max(parameter));
	selectMidiController.hListenerWnd = m_hWnd;

	if (selectMidiController.m_hWnd == 0) {
		selectMidiController.Create(m_hWnd);
	}
	selectMidiController.ShowWindow(SW_SHOW);
	return 0;
}


LRESULT CMachineParameterView::OnUnbindMidi(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CParameterSliderBar* slider = sliderView.GetSelectedSlider();
	if (slider == 0) return 0;

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, slider->m_pluginid);

	zzub_player_remove_midimapping(plugin, slider->m_group, slider->m_track, slider->m_column);
	zzub_player_history_commit(player, 0, 0, "Remove MIDI Mapping");
	return 0;
}

LRESULT CMachineParameterView::OnSelectMidiController(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	zzub_player_add_midimapping(selectMidiController.bindMidiMachine, 
		selectMidiController.bindMidiGroup, selectMidiController.bindMidiTrack, selectMidiController.bindMidiColumn,
		selectMidiController.channel, selectMidiController.controller);

	zzub_player_history_commit(player, 0, 0, "Add MIDI Mapping");
	//assert(false);
	return 0;
}

void CMachineParameterView::GetHelpText(char* text, int* len) {

	std::string helptext = PeekString(_Module.GetResourceInstance(), IDT_HELP_PARAMETERVIEW);
	HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "parameter");
	std::string acceltext = CreateAccelTableString(hAccel);

	helptext += acceltext;
	*len = (int)helptext.length();
	if (text)
		strcpy(text, helptext.c_str());
}


bool CMachineParameterView::DoesKeyjazz() {
	return true;
}
