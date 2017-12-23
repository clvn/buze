#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewFrame.h>
#include <sys/stat.h>
#include <fstream>
#include "utils.h"
#include "PvstMachines.h"

#include "Configuration.h"

#include "BuzeConfiguration.h"
#include "AboutDlg.h"
#include "MainFrm.h"

// ---------------------------------------------------------------------------------------------------------------
// DEPENDENCIES & SERIALIZER
// ---------------------------------------------------------------------------------------------------------------

#include "CustomView.h"

#include "DockTabFrame/DockTabSerializer.h"
#include "DockTabFrame/DockTabViewManager.h"

namespace DockSplitTab {

template <class MainFrameT, class ViewT>
HWND DockTabViewManager<MainFrameT, ViewT>::ViewGetHWND(ViewT* view) {
	return view->m_hWnd;
}

template <class MainFrameT, class ViewT>
HWND DockTabViewManager<MainFrameT, ViewT>::ViewCreateWindow(ViewT* view, HWND hWndParent, RECT rect, std::string const& caption, void* userdata) {
	return view->Create(hWndParent, rect, caption.c_str(), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, userdata);
}

template <class MainFrameT, class ViewT>
LRESULT DockTabViewManager<MainFrameT, ViewT>::ViewCloseWindow(ViewT* view) {
	return view->SendMessage(WM_CLOSE);
}
/*
template <class MainFrameT, class ViewT>
void DockTabViewManager<MainFrameT, ViewT>::ViewDelete(ViewT* view) {
	delete view;
}
*/
template <class MainFrameT, class ViewT>
std::string DockTabClientViewSerializer<MainFrameT, ViewT>::serialize() {
	return "";
}

template <class MainFrameT, class ViewT>
ViewT* DockTabClientViewSerializer<MainFrameT, ViewT>::deserialize(std::string const& data) {
	return new ViewT(mainFrame);
}


template <>
HWND DynamicDockTabViewManager<CMainFrame, CView>::ViewGetHWND(CView* view) {
	return view->GetHwnd();
}

template <>
LRESULT DynamicDockTabViewManager<CMainFrame, CView>::ViewCloseWindow(CView* view) {
	return ::SendMessage(ViewGetHWND(view), WM_CLOSE, 0, 0);
}
// --- input mixer ---
/*
template <>
std::string DockTabClientViewSerializer<CMainFrame, CInputMixerView>::serialize() {
	return "";
}

template <>
CInputMixerView* DockTabClientViewSerializer<CMainFrame, CInputMixerView>::deserialize(std::string const& data) {
	mainFrame->createInputMixerViewPlugin = zzub_player_get_plugin_by_name(mainFrame->player, data.c_str()); 
	return new CInputMixerView(mainFrame);
}

template <>
CAnalyzerView* DockTabClientViewSerializer<CMainFrame, CAnalyzerView>::deserialize(std::string const& data) {
	return new CAnalyzerView(mainFrame, mainFrame->document, mainFrame->player);
}*/

} // END namespace DockSplitTab

// ---------------------------------------------------------------------------------------------------------------
// MAINFRAME
// ---------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------
// GLOBAL HELPERS
// ---------------------------------------------------------------------------------------------------------------

//extern std::string rewriteBuzzWrapperName(std::string const& uri);

// from bmxreader.cpp:
std::string rewriteBuzzWrapperUri(std::string const& fileName) {
	std::string uri = "@zzub.org/buzz2zzub/" + fileName;
	replace(uri.begin(), uri.end(), ' ', '+');
	return uri;
}

// ---------------------------------------------------------------------------------------------------------------
// MASTER TOOLBAR
// ---------------------------------------------------------------------------------------------------------------

CMasterToolbar::CMasterToolbar(CMainFrame* mainFrm) {
	editing_bpm = false;
	editing_tpb = false;
	mainFrame = mainFrm;
}

CMasterToolbar::~CMasterToolbar() {
}

LRESULT CMasterToolbar::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	ModifyStyleEx(0, WS_EX_COMPOSITED);
	DefWindowProc();

	volumeSlider.Create(m_hWnd, rcDefault, "", WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_MASTERSLIDER);

// 	tpbDropDown.Create(m_hWnd, rcDefault, "4", WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_WANTRETURN|ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, IDC_EDIT_TPB);
// 	tpbDropDown.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
// 	bpmDropDown.Create(m_hWnd, rcDefault, "125", WS_CHILD|WS_TABSTOP|WS_VISIBLE|ES_WANTRETURN|ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, IDC_EDIT_BPM);
// 	bpmDropDown.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	tpbDropDown.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE, IDC_EDIT_TPB);
	tpbDropDown.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	bpmDropDown.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE, IDC_EDIT_BPM);
	bpmDropDown.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	bpmLabel.Create(m_hWnd, rcDefault, "", WS_CHILD|WS_VISIBLE);
	bpmLabel.SetLabel("BPM");
	tpbLabel.Create(m_hWnd, rcDefault, "", WS_CHILD|WS_VISIBLE);
	tpbLabel.SetLabel("TPB");

	UpdateSettings();

	return 0;
}

LRESULT CMasterToolbar::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	RECT rc;
	GetClientRect(&rc);

	int txtwidth = 30;
	int volwidth = rc.right-(txtwidth*4);

	SetRect(&rc, 0, 0, 100, 20);

	SetRedraw(FALSE);
	{
		volumeSlider.MoveWindow(0,0,volwidth,20);
		bpmLabel.MoveWindow(volwidth,0,txtwidth,20);
		bpmDropDown.MoveWindow(volwidth+txtwidth,0,txtwidth,20);
		tpbLabel.MoveWindow(volwidth+txtwidth+txtwidth,0,txtwidth,20);
		tpbDropDown.MoveWindow(volwidth+txtwidth+txtwidth+txtwidth,0,txtwidth,20);
	}
	SetRedraw(TRUE);
	RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);

	return 0;
}

LRESULT CMasterToolbar::OnSetBpmFocus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/) {
	editing_bpm = true;
	return 0;
}

LRESULT CMasterToolbar::OnKillBpmFocus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/){
	int value;
	bool valid = bpmDropDown.GetInt(value);

	int oldbpm = (int)zzub_player_get_bpm(mainFrame->player);

	if (valid && value != oldbpm) {
		static int const minvalue = 1;
		static int const maxvalue = 512;
		valid &= (value >= minvalue);
		valid &= (value <= maxvalue);
		if (valid) {	
			zzub_player_set_bpm(mainFrame->player, (float)value);
			zzub_player_history_commit(mainFrame->player, 0, 0, "Set BPM");
		}
	}

	if (!valid)
		bpmDropDown.SetInt(oldbpm);

	editing_bpm = false;
	return 0;
}

LRESULT CMasterToolbar::OnBpmAccept(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) {
	mainFrame->SetFocus();
	return 0;
}

LRESULT CMasterToolbar::OnBpmCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) {
	int value = (int)zzub_player_get_bpm(mainFrame->player);
	bpmDropDown.SetInt(value);
	mainFrame->SetFocus();
	return 0;
}

LRESULT CMasterToolbar::OnSetTpbFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) {
	editing_tpb = true;
	return 0;
}

LRESULT CMasterToolbar::OnKillTpbFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) {
	int value;
	bool valid = tpbDropDown.GetInt(value);

	int oldtpb = zzub_player_get_tpb(mainFrame->player);

	if (valid && value != oldtpb) {
		static int const minvalue = 1;
		static int const maxvalue = 254;
		valid &= (value >= minvalue);
		valid &= (value <= maxvalue);
		if (valid) {	
			zzub_player_set_tpb(mainFrame->player, value);
			zzub_player_history_commit(mainFrame->player, 0, 0, "Set TPB");
		}
	}

	if (!valid)
		tpbDropDown.SetInt(oldtpb);

	editing_tpb = false;
	return 0;
}

LRESULT CMasterToolbar::OnTpbAccept(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) {
	mainFrame->SetFocus();
	return 0;
}

LRESULT CMasterToolbar::OnTpbCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) {
	int value = zzub_player_get_tpb(mainFrame->player);
	tpbDropDown.SetInt(value);
	mainFrame->SetFocus();
	return 0;
}

LRESULT CMasterToolbar::OnSetValue(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	zzub_plugin_t* masterplugin = zzub_player_get_plugin_by_name(mainFrame->player, "Master");
	if (masterplugin != 0) {
		int value = volumeSlider.current_value;
		zzub_plugin_set_parameter_value(masterplugin, 1, 0, 0, value, true);
		zzub_player_history_commit(mainFrame->player, 0, 0, "Set Master Volume");
	}
	return 0;
}

LRESULT CMasterToolbar::OnSetValuePreview(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	zzub_plugin_t* masterplugin = zzub_player_get_plugin_by_name(mainFrame->player, "Master");
	if (masterplugin != 0) {
		int value = volumeSlider.current_value;
		zzub_plugin_set_parameter_value_direct(masterplugin, 1, 0, 0, value, true);
	}
	//zzub_plugin_set_parameter_value_direct(machine, group, track, column, drag_value, true);
	return 0;
}

void CMasterToolbar::UpdateSettings() {
	volumeSlider.SetDropRate(mainFrame->configuration->getVUDropSpeed());
	volumeSlider.SetTimerRate(mainFrame->configuration->getVUTimerSpeed());
}

LRESULT CMainFrame::OnMainFrameBpmUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int bpm = (int)zzub_player_get_bpm(player);
	static int const minvalue = 1;
	static int const maxvalue = 512;
	int newbpm = clamp(bpm + 1, minvalue, maxvalue);
	if (newbpm == bpm) return 0;
	zzub_player_set_bpm(player, (float)newbpm);
	zzub_player_history_commit(player, 0, 0, "Increase BPM");
	updateTick();
	return 0;							
}

LRESULT CMainFrame::OnMainFrameBpmDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int bpm = (int)zzub_player_get_bpm(player);
	static int const minvalue = 1;
	static int const maxvalue = 512;
	int newbpm = clamp(bpm - 1, minvalue, maxvalue);
	if (newbpm == bpm) return 0;
	zzub_player_set_bpm(player, (float)newbpm);
	zzub_player_history_commit(player, 0, 0, "Decrease BPM");
	updateTick();
	return 0;						
}

// ---------------------------------------------------------------------------------------------------------------
// SONG TOOLBAR
// ---------------------------------------------------------------------------------------------------------------

CSongToolbar::CSongToolbar(CMainFrame* mainFrm) {
	mainFrame = mainFrm;

	last_time = 0.0;
	last_current = 0.0f;
	last_loop = 0.0f;
}

CSongToolbar::~CSongToolbar() {
}

LRESULT CSongToolbar::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DefWindowProc();

	elapsedTimeLabel.Create(m_hWnd, rcDefault, "", WS_CHILD|WS_VISIBLE);
	elapsedTimeLabel.SetLabel("Elapsed 00:00:00:0");
	currentTimeLabel.Create(m_hWnd, rcDefault, "", WS_CHILD|WS_VISIBLE);
	currentTimeLabel.SetLabel("Current 00:00:00:0");
	loopTimeLabel.Create(m_hWnd, rcDefault, "", WS_CHILD|WS_VISIBLE);
	loopTimeLabel.SetLabel("Loop 00:00:00:0");

	return 0;
}

LRESULT CSongToolbar::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	RECT rc;
	GetClientRect(&rc);

	SetRect(&rc, 0, 0, 100, 20);

	elapsedTimeLabel.MoveWindow(2, 0, 100,  20);
	currentTimeLabel.MoveWindow(2+100+2, 0, 100, 20);
	loopTimeLabel.MoveWindow(2+100+2+100+2, 0, 100, 20);

	return 0;
}

void CSongToolbar::update() {
	float ticksCurrent = (float)zzub_player_get_position_row(mainFrame->player);
	int orderpos = zzub_player_get_position_order(mainFrame->player);
	zzub_pattern_t* orderpat = zzub_player_get_order_pattern(mainFrame->player, orderpos);

	float ticksLoop;
	if (orderpat)
		ticksLoop = (float)zzub_pattern_get_loop_end(orderpat) - zzub_pattern_get_loop_start(orderpat);
	else
		ticksLoop = 0;

	float bpm = zzub_player_get_bpm(mainFrame->player);
	float tpb = (float)zzub_player_get_tpb(mainFrame->player);
	float samplerate = (float)zzub_audiodriver_get_samplerate((zzub_audiodriver_t*)buze_application_get_audio_driver(mainFrame->application));
	float frac = ((float)60.0 * samplerate) / ((float)bpm*tpb);
	float tps = (float)samplerate / frac;

	char pc[64];

	//if (zzub_player_get_state(mainFrame->player) == player_state_playing)
	//	mainFrame->lastPlayFrame = mainFrame->player->timer.frame();
	double time = 0;
	if (zzub_player_get_state(mainFrame->player) == zzub_player_state_playing)
		time = mainFrame->elapsedTimeGet();

	if (time != last_time) {
		formatTime(pc, "Elapsed", (float)time);
		elapsedTimeLabel.SetLabel(pc);
	}
	last_time = time;

	float current = ticksCurrent / tps;
	if (current != last_current) {
		formatTime(pc, "Current", current);
		currentTimeLabel.SetLabel(pc);
	}
	last_current = current;

	float loop = ticksLoop / tps;
	if (loop != last_loop) {
		formatTime(pc, "Loop", loop);
		loopTimeLabel.SetLabel(pc);
	}
	last_loop = loop;
}

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DESTRUCTION
// ---------------------------------------------------------------------------------------------------------------

CMainFrame::CMainFrame()
:
	frame(this) // Frame(FrameListener* cbListener)
	, customView(this, DockSplitTab::placeMAINPANE, DockSplitTab::dockUNKNOWN, "Custom View", "", false)
	, masterToolbar(this)
	, songToolbar(this)
	//, selectMidiController(this)
{
	viewStackPosition = -1;
	isStandalone = true;
	document = 0;
	dirtyTitlebarAsterisk = false;
	hideAllParameters = false;
	timerCount = 0;
	current_screenset = 0;
	m_bLockedToolbars = false;
	menu_has_accelerators = false;

	screensets_parser.create();
	QueryPerformanceFrequency((LARGE_INTEGER*)&qpfFreq);

	nextEventCommand = ID_MAINFRAME_EVENT_FIRST;
	
	const int buze_event_type_custom_first = 10000; // some number larger than the higest event code in buze.zidl
	nextEventCode = buze_event_type_custom_first;
}

CMainFrame::~CMainFrame() {
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	zzub_player_clear(player);
	player = 0;

	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		delete *i;
	}
	views.clear();

	return DefWindowProc();
}

//void moveAndReplaceIndexItem(MachineIndex& index, std::string dest, std::string src);

bool CMainFrame::CreateFrame(HWND hParentWnd) {
	if (CreateEx(
			(HWND)hParentWnd, 0,
			WS_BORDER|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_OVERLAPPED|WS_SYSMENU|WS_THICKFRAME|WS_VISIBLE,
			WS_EX_APPWINDOW|WS_EX_CONTROLPARENT|WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR|WS_EX_WINDOWEDGE|WS_EX_ACCEPTFILES
		) == NULL) {
	//if (mainFrame->CreateEx(hParentWnd, 0, 0, WS_EX_ACCEPTFILES) == NULL) {
		ATLTRACE(_T("Main window creation failed!\n"));
		return false;
	}
	ShowWindow(SW_SHOW);// nCmdShow);
	SetForegroundWindow(m_hWnd);	// Win95 needs this
	return true;
}

HWND CMainFrame::GetHwnd() {
	return m_hWnd;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	//document = new CDocument(player, _Module.configuration);

//	SetRedraw(FALSE);
	{
		createToolbars();

		m_hWndClient = frame.create(m_hWnd, rcDefault);

		// load default hotkeys from json
		hotkeys.Initialize();

		RegisterAccelerator("mainframe", "play", "f5", ID_PLAY);
		RegisterAccelerator("mainframe", "playfromcursor", "f6", ID_PLAYFROMCURSOR);
		RegisterAccelerator("mainframe", "record", "f7", ID_RECORD);
		RegisterAccelerator("mainframe", "stop", "f8", ID_STOP);
		RegisterAccelerator("mainframe", "device_reset", "f10", ID_DEVICE_RESET);
		RegisterAccelerator("mainframe", "view_help", "f1", ID_VIEW_HELP);
		RegisterAccelerator("mainframe", "view_showhide_params", "f12", ID_VIEW_SHOWHIDEALLPARAMETERFRAMES );
		RegisterAccelerator("mainframe", "view_nextpane", "tab ctrl", ID_NEXT_PANE);
		RegisterAccelerator("mainframe", "view_prevpane", "tab ctrl shift", ID_PREV_PANE);
		RegisterAccelerator("mainframe", "view_closepane", "w ctrl, f4 ctrl", ID_CLOSEPANE);
		RegisterAccelerator("mainframe", "file_new", "n ctrl", ID_FILE_NEW);
		RegisterAccelerator("mainframe", "file_open", "n ctrl", ID_FILE_OPEN);
		RegisterAccelerator("mainframe", "file_save", "n ctrl", ID_FILE_SAVE);
		RegisterAccelerator("mainframe", "edit_undo", "n ctrl", ID_EDIT_UNDO);
		RegisterAccelerator("mainframe", "edit_redo", "n ctrl", ID_EDIT_REDO);
		RegisterAccelerator("mainframe", "screenset_recall_0", "numpad0", ID_SCREENSET_RECALL_0);
		RegisterAccelerator("mainframe", "screenset_recall_1", "numpad1", ID_SCREENSET_RECALL_1);
		RegisterAccelerator("mainframe", "screenset_recall_2", "numpad2", ID_SCREENSET_RECALL_2);
		RegisterAccelerator("mainframe", "screenset_recall_3", "numpad3", ID_SCREENSET_RECALL_3);
		RegisterAccelerator("mainframe", "screenset_recall_4", "numpad4", ID_SCREENSET_RECALL_4);
		RegisterAccelerator("mainframe", "screenset_recall_5", "numpad5", ID_SCREENSET_RECALL_5);
		RegisterAccelerator("mainframe", "screenset_recall_6", "numpad6", ID_SCREENSET_RECALL_6);
		RegisterAccelerator("mainframe", "screenset_recall_7", "numpad7", ID_SCREENSET_RECALL_7);
		RegisterAccelerator("mainframe", "screenset_recall_8", "numpad8", ID_SCREENSET_RECALL_8);
		RegisterAccelerator("mainframe", "screenset_recall_9", "numpad9", ID_SCREENSET_RECALL_9);

		RegisterAccelerator("mainframe", "screenset_store_0", "numpad0 ctrl", ID_SCREENSET_STORE_0);
		RegisterAccelerator("mainframe", "screenset_store_1", "numpad1 ctrl", ID_SCREENSET_STORE_1);
		RegisterAccelerator("mainframe", "screenset_store_2", "numpad2 ctrl", ID_SCREENSET_STORE_2);
		RegisterAccelerator("mainframe", "screenset_store_3", "numpad3 ctrl", ID_SCREENSET_STORE_3);
		RegisterAccelerator("mainframe", "screenset_store_4", "numpad4 ctrl", ID_SCREENSET_STORE_4);
		RegisterAccelerator("mainframe", "screenset_store_5", "numpad5 ctrl", ID_SCREENSET_STORE_5);
		RegisterAccelerator("mainframe", "screenset_store_6", "numpad6 ctrl", ID_SCREENSET_STORE_6);
		RegisterAccelerator("mainframe", "screenset_store_7", "numpad7 ctrl", ID_SCREENSET_STORE_7);
		RegisterAccelerator("mainframe", "screenset_store_8", "numpad8 ctrl", ID_SCREENSET_STORE_8);
		RegisterAccelerator("mainframe", "screenset_store_9", "numpad9 ctrl", ID_SCREENSET_STORE_9);

		RegisterAccelerator("mainframe", "keyjazz_octave_up", "multiply", ID_KEYJAZZ_OCTAVE_UP);
		RegisterAccelerator("mainframe", "keyjazz_octave_down", "divide", ID_KEYJAZZ_OCTAVE_DOWN);
		RegisterAccelerator("mainframe", "bpm_up", "oem_6 ctrl", ID_MAINFRAME_BPM_UP);
		RegisterAccelerator("mainframe", "bpm_down", "oem_4 ctrl", ID_MAINFRAME_BPM_DOWN);

		// menu & frame is ready: initialize the document, enumerate view plugins, register hotkeys
		buze_main_frame_initialize(this, 0, 0);

		// view plugins must have registered their accelerators by now
		hotkeys.CreateAccelTables();

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		// attach master to global volumeslider
		zzub_plugin_t* masterplugin = zzub_player_get_plugin_by_name(player, "Master");
		assert(masterplugin != 0);
		masterToolbar.volumeSlider.SetMinMax(0, 0x4000);
		//masterToolbar.volumeSlider.SetSource(masterplugin, 1, 0, 0);

		buze_document_add_view(document, this);
		zzub_player_set_host_info(player, 42, 0x0503, m_hWnd);
		buze_document_create_default_document(document);
		zzub_player_history_commit(player, 0, 0, "");
		zzub_player_history_reset(player);

		// open index.txt, causes OnUpdate(buze_event_type_update_index)
		buze_document_load_plugin_index(document);

		setWindowTitle("");

		initializeThemes();
		initializeRecent();

		m_bLockedToolbars = configuration->getLockedToolbars();

		if (!loadScreensetsFromFile(buze_application_map_path(application, "gui.xml", buze_path_type_user_path))) {
			if (!loadScreensetsFromFile(buze_application_map_path(application, "Gear/gui.xml", buze_path_type_app_path))) {
				// gui xml not found, start with a default setup
				pug::xml_node doc = screensets_parser.document();
				pug::xml_node xmldesc = doc.append_child(pug::node_pi);
				xmldesc.name("xml");
				xmldesc.attribute("version") = "1.0";
				xmldesc.attribute("encoding") = "iso-8859-1";   // TODO fix for utf 8

				for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
					if ((*i)->defaultView) {
						ClientView* cv = (*i)->createClientWindow(m_hWnd);
						assert(cv);
						(*i)->insertClient(cv);
					}
				}
	/*
				showMachineFolderView();
				showPrimaryPatternEditor(false, false, 0);
				showMachineView();
				showWaveTableView();
				//showAnalyzerView();
				showPropertyView();
				showFileBrowser();
				// select startup panes
				showMachineFolderView();
				showMachineView();*/

				storeScreenset(current_screenset);
			} else {
				initScreenset(current_screenset);
			}
		} else {
			initScreenset(current_screenset);
		}

		//if (_Module.wantsAudioPreferences) 
		//	showPreferencesView();

		// TODO: toolbar state should be restored inside deserializeFromFrame, 
		// after the window size is restored but before restoring the view frames
		loadToolbarState();

		if (configuration->getShowAccelerators()) {
			menu_has_accelerators = true;
			hotkeys.UpdateMenuKeys("mainframe", m_CmdBar.m_hMenu, true);
		} else {
			menu_has_accelerators = false;
		}
	}
// 	SetRedraw(TRUE);
// 	RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);

	initTimer();

	return 0;
}

bool CMainFrame::isRunningAsStandalone() {
	return isStandalone;
}

LRESULT CMainFrame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {

	if (isRunningAsStandalone()) {
		// if we are minimized, bring the window back before closing down
		if (IsIconic()) ShowWindow(SW_SHOWNORMAL);
		zzub_player_set_state(player, zzub_player_state_muted, -1);
		int confirm = MessageBox("Lame?", programName, MB_YESNOCANCEL|MB_ICONSTOP);
		if (confirm != IDYES)
			return 0;

		return OnCloseNoQuestions(uMsg, wParam, lParam, bHandled);
	} else {
		ShowWindow(SW_MINIMIZE);
	}

	return 0;
}

LRESULT CMainFrame::OnCloseNoQuestions(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	// ensure clearSong doesnt scream about saving
	//document->lastSaveUndoPosition = zzub_player_history_get_position(player);

	clearSong();

	if (isRunningAsStandalone()) {
		std::string userPath = buze_application_map_path(application, "", buze_path_type_user_path);
		bool success = false;
		if (PathFileExists(userPath.c_str()) != 0) {
			success = true;
		} else {
			success = CreateDirectory(userPath.c_str(), NULL) != 0;
		}
		std::string filename = buze_application_map_path(application, "gui.xml", buze_path_type_user_path);
		saveScreensetsToFile(filename.c_str());
		//saveScreensetsToFile("Gear\\gui.xml");
	}

	saveToolbarState();
	customView.closeAll();

	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		(*i)->closeAll();
	}

	buze_document_remove_view(document, this);

	DestroyWindow();
	// wtl doesnt set hwnd to NULL properly during DestroyWindow (in atlwin.h 
	// its only setting to null if a certain local variable "oldMsg" is also
	// null, but its never null for the mainframe, probably because it
	// received the wm_destroy from a message handler or something)
	m_hWnd = 0;

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// SIZING
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = { (unsigned short)LOWORD(lParam), (unsigned short)HIWORD(lParam) };

	if (m_hWndStatusBar) {
		int statusWidths[] = { pt.x - 230, pt.x - 80, -1 };
		CStatusBarCtrl statusBar(m_hWndStatusBar);
		statusBar.SetParts(3, statusWidths);
	}
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// VIEW UPDATES
// ---------------------------------------------------------------------------------------------------------------

void CMainFrame::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	switch (lHint) {
		case buze_event_type_update_settings:
			UpdateSettings();
			break;
		case buze_event_type_update_index: {
			std::string plurFile = buze_application_map_path(application, "Gear/index.plur", buze_path_type_app_path);
			char pc[1024];
			GetFullPathName(plurFile.c_str(), array_size(pc), pc, 0);
			plur.open(pc);
			rebuildMachineMenus();
			break;
		}
	}
}

LRESULT CMainFrame::OnMainFrameEventCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	std::map<WORD, int>::iterator i = commandEventMap.find(wID);
	if (i == commandEventMap.end()) {
		// unmapped - could this happen?
		return 0;
	}

	buze_document_notify_views(document, 0, i->second, 0);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// DOCKTABFRAME SUPPORT
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnClosePane(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HWND clientViewWnd = frame.focusedClientView();
	if (clientViewWnd == 0) return 0;
	clientViewHide(clientViewWnd);
	return 0;
}

/// not being used!!
void CMainFrame::clientViewHide(HWND clientViewWnd) {
	ATLASSERT( ::IsWindow(clientViewWnd));
	if (0 == ::SendMessage(clientViewWnd, WM_CLOSE, 0, 0)) {
		ClientView* clientView = this->frame.getClientView( clientViewWnd);
		frame.detachClientView(clientViewWnd);
		this->clientDetached(clientView);
		// to destroy the client view we get out of client view call stack
		//this->SendMessage(WM_CLOSE_CLIENTVIEW, 0, reinterpret_cast<LPARAM>(clientViewWnd));
	}
	return;
}

/// not being used!!
/*
LRESULT CMainFrame::OnCloseClientView(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	HWND clientViewWnd = reinterpret_cast< HWND>( lParam);
	ClientView* clientView = this->frame.getClientView( clientViewWnd);
	if ( NULL == clientView)
		return 0;

	this->clientDetached(clientView);
	return 0;
}*/

// FrameListener interface
void CMainFrame::clientChangedPlace(ClientView* clientView, DockSplitTab::FramePlace place, DockSplitTab::DockSide side) {
	// TODO: whats this? 
	/*	DynamicDockTabViewManager<CMainFrame, CView>* preferencesView = GetViewInfo("PreferencesView");
	DynamicDockTabViewManager<CMainFrame, CView>* parameterView = GetViewInfo("ParameterView");

	if (parameterView->lookupView(clientView)) {
		parameterView->framePlace = place;
		parameterView->dockSide = side;
	}
	else if (preferencesView->lookupView(clientView)) {
		preferencesView->framePlace = place;
		preferencesView->dockSide = side;
	}*/
}

// FrameListener interface
void CMainFrame::clientActivated(ClientView* clientView) {
	ViewStackMoveToFront(clientView->wnd);
}

void CMainFrame::clientDetached(ClientView* clientView) {
	ViewStackRemove(clientView->wnd); // used to be called by each view .. 
	checkDetachedForPrimarySecondary(clientView);
	customView.destroy(clientView);

	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		(*i)->destroy(clientView);
	}

	delete clientView;
}

void CMainFrame::clientDetachedForRecycle(ClientView* clientView) {
	//checkDetachedForPrimarySecondary(clientView);
	ViewStackRemove(clientView->wnd); // used to be called by each view .. 
}

void CMainFrame::checkDetachedForPrimarySecondary(ClientView* clientView) {
	//assert(false);
/*	HWND clientViewWnd = clientView->wnd;

	if (primaryPatternEditor && primaryPatternEditor->m_hWnd == clientViewWnd)
		primaryPatternEditor = 0;

	if (secondaryPatternEditor && secondaryPatternEditor->m_hWnd == clientViewWnd)
		secondaryPatternEditor = 0;*/
}

ClientView* CMainFrame::createClientWindow(HWND hWnd, std::string const& caption, std::string const& toolTip, int imageIndex) {
	ClientView* result = new ClientView(caption.c_str(), hWnd, toolTip.c_str(), imageIndex);
	this->clientViews[hWnd] = result;
	return result;
}

ClientView* CMainFrame::recycleClientWindow(ClientView* clientView) {
	this->clientViews[clientView->wnd] = clientView;
	return clientView;
}

void CMainFrame::ViewStackRemove(HWND hWnd) {
	std::vector<std::pair<HWND, bool> >::iterator i;
	for (i = viewStack.begin(); i != viewStack.end(); ++i) {
		if (i->first == hWnd) break;
	}

	if (i != viewStack.end()) {
		viewStack.erase(i);
		if (viewStackPosition >= (int)viewStack.size()) {
			viewStackPosition = viewStack.size() - 1;
		}
	}

	if (!viewStack.empty())
		frame.setFocusTo(viewStack.front().first);

	this->clientViews.RemoveKey(hWnd);
}

// ---------------------------------------------------------------------------------------------------------------
// SCREENSETS
// ---------------------------------------------------------------------------------------------------------------

template <class ViewT>
void registerFactory(DockTabSerializer& serializer, DockTabClientFactoryImpl<CMainFrame, ViewT>& clients) {
	std::string className = clients.className; //ViewT::GetWndClassInfo().m_wc.lpszClassName;
	serializer.registerFactory(className, &clients);
}

bool CMainFrame::initScreenset(int screenset) {
	std::vector<ClientView*> current_views;
	return recallScreenset(screenset, current_views);
}

bool CMainFrame::recallScreenset(int screenset, std::vector<ClientView*>& current_views) {
	DockTabSerializer serializer;

	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		if ((*i)->serializable)
			registerFactory<CView>(serializer, **i);
	}

	bool result = serializer.deserializeScreenset(&frame, screensets_parser, screenset, current_views);

	// restore Primary/Secondary PE's
	/*{
		primaryPatternEditor = 0;
		secondaryPatternEditor = 0;	

		int pe_view_count = patternEditors.getViewCount();
		for (int i = 0; i < pe_view_count; ++i) {
			CPatternView* pe = patternEditors.getView(i);
			ClientView* cv = frame.getClientView(pe->m_hWnd);
			if (cv->caption == "Primary Pattern Editor") {
				primaryPatternEditor = pe;
			} else
			if (cv->caption == "Secondary Pattern Editor") {
				secondaryPatternEditor = pe;
			}
		}
	}*/

	return result;
}

bool CMainFrame::storeScreenset(int screenset) {
	DockTabSerializer serializer;

	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		if ((*i)->serializable)
			registerFactory<CView>(serializer, **i);
	}

	return serializer.serializeScreenset(&frame, screensets_parser, screenset);
}

bool CMainFrame::loadScreensetsFromFile(std::string const& fileName) {
	struct _stat st;
	if (-1 == _stat(fileName.c_str(), &st)) return false;

	return screensets_parser.parse_file(fileName.c_str());
}

bool CMainFrame::saveScreensetsToFile(std::string const& fileName) {
	//struct _stat st;
	//if (-1 == _stat(fileName.c_str(), &st)) return false;

	std::fstream osf;
	osf.open(fileName.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	if (!osf) {
		return false;
	}

	screensets_parser.document().outer_xml(osf);
	osf.close();

	return true;
}

LRESULT CMainFrame::OnScreensetRecall(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	current_screenset = wID - ID_SCREENSET_RECALL_FIRST;

	frame.SetRedraw(FALSE);
	{
		std::vector<ClientView*> current_views;

		// step 0: save focused window
		HWND focused_wnd = frame.focusedClientView();

		// step 1: detach all views and push their clientviews into vector
		customView.detachAllForRecycler(current_views);

		///machineParameters ? showHideAllParameterFrames(false, false);

		for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
			if ((*i)->serializable)
				(*i)->detachAllForRecycler(current_views);
		}
		frame.DefWindowProc();

		// step 2: recall screenset -- removes each successfully recycled view from the vector
		recallScreenset(current_screenset, current_views);

		frame.DefWindowProc();
		// step 3: destroy the leftover views that were not recycled
		customView.destroyUnrecycled(current_views);

		for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
			if ((*i)->serializable)
				(*i)->destroyUnrecycled(current_views);
		}
		// step 4: restore focus
		ClientView* cv = frame.getClientView(focused_wnd);
		if (cv != NULL) {
			frame.setFocusTo(focused_wnd);
		}
	}
	frame.SetRedraw(TRUE);
	frame.RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);

	return 0;
}

LRESULT CMainFrame::OnScreensetStore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	current_screenset = wID - ID_SCREENSET_STORE_FIRST;
	storeScreenset(current_screenset);

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// TABABLE VIEWS
// ---------------------------------------------------------------------------------------------------------------

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	switch (pMsg->message) {
		/// TODO: somewhat broken because this stuff is hardcoded to 'ctrl', but the hotkeys system allows you to use a different modifier.
		case WM_KEYDOWN:
			if (pMsg->wParam == VK_CONTROL) {
				ctrlDown();
			}
			break;
		case WM_KEYUP:
			if (pMsg->wParam == VK_CONTROL) {
				ctrlUp();
			}
			break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			ctrlUp(); // this is a hax to fix the possibility of ctrlDown() getting stuck
			break;
	}

	if (::GetWindowLongPtr(pMsg->hwnd, GWLP_HINSTANCE) == (LONG_PTR)_Module.m_hInst)
		if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

	HACCEL mainframe_hAccel= GetAccelerators("mainframe");
	if (::TranslateAccelerator(m_hWnd, mainframe_hAccel, pMsg))
		return TRUE;

	return FALSE;
}

void CMainFrame::ViewStackInsert(HWND hViewWnd, bool keyboard_tabable) {
	viewStack.insert(viewStack.begin(), std::pair<HWND, bool>(hViewWnd, keyboard_tabable));
}

LRESULT CMainFrame::OnNextWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (viewStackPosition == -1) return 0;
	if (viewStack.empty()) return 0;

	if (false
		|| viewStack[viewStackPosition].first == GetFocus()
		|| ::IsChild(viewStack[viewStackPosition].first, GetFocus())
	) {
		++viewStackPosition;
		if (viewStackPosition >= (int)viewStack.size())
			viewStackPosition = 0;
	}

	frame.setFocusTo(viewStack[viewStackPosition].first);
	return 0;
}

LRESULT CMainFrame::OnPrevWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (viewStackPosition == -1) return 0;
	if (viewStack.empty()) return 0;

	if (false
		|| viewStack[viewStackPosition].first == GetFocus()
		|| ::IsChild(viewStack[viewStackPosition].first, GetFocus())
	) {
		--viewStackPosition;
		if (viewStackPosition < 0)
			viewStackPosition = viewStack.size() - 1;
	}

	frame.setFocusTo(viewStack[viewStackPosition].first);
	return 0;
}

void CMainFrame::ViewStackMoveToFront(HWND viewWnd) {
	// move to front of view stack (if it exists)
	if (viewStackPosition == -1) {
		std::vector<std::pair<HWND, bool> >::iterator i;
		for (i = viewStack.begin(); i != viewStack.end(); ++i) {
			if (i->first == viewWnd) break;
		}

		if (i != viewStack.end()) {
			std::pair<HWND, bool> current = *i;
			viewStack.erase(i);
			viewStack.insert(viewStack.begin(), current);
		}
	}
}

void CMainFrame::ctrlDown() {
	if (!viewStack.empty())
		viewStackPosition = 0;
}

void CMainFrame::ctrlUp() {
	// invoked when ctrl is released =)

	if (viewStack.empty() || viewStackPosition == -1) {
		viewStackPosition = -1;
		return;
	}

	std::pair<HWND, bool> current = viewStack[viewStackPosition];
	viewStack.erase(viewStack.begin() + viewStackPosition);
	viewStack.insert(viewStack.begin(), current);
	viewStackPosition = -1;
}

// ---------------------------------------------------------------------------------------------------------------
// CUSTOM VIEWS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnCustomViewCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	// TODO: disallow creating several views with the same name

	const char* name = (const char*)lParam;
	ClientView* cv = customView.createClientWindow(m_hWnd, name);
	customView.insertClient(cv);
	CCustomView* view = customView.lookupView(cv);

	if (view != 0) return (LRESULT)view->m_hWnd;

	return 0;
}

LRESULT CMainFrame::OnCustomViewGet(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	const char* name = (const char*)lParam;
	for (size_t i = 0; i < customView.getViewCount(); i++) {
		HWND hViewWnd = *customView.getView(i);
		ClientView* cv = frame.getClientView(hViewWnd);
		if (cv->caption == name) return (LRESULT)hViewWnd;
	}
	return 0;
}

LRESULT CMainFrame::OnCustomViewFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	const char* name = (const char*)lParam;
	for (size_t i = 0; i < customView.getViewCount(); i++) {
		HWND hViewWnd = *customView.getView(i);
		ClientView* cv = frame.getClientView(hViewWnd);
		if (cv->caption == name) {
			frame.setFocusTo(hViewWnd);
			return 0;
		}
	}
	return -1;
}

LRESULT CMainFrame::OnCustomViewSetChild(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// TODO: THIS CODE COULD GO IN CUSTOMVIEW!
	CCustomView* view = customView.lookupView((HWND)wParam);
	if (!view) return 0;
	view->hViewWnd = (HWND)lParam;

	RECT rc;
	view->GetClientRect(&rc);
	::SetParent(view->hViewWnd, view->m_hWnd);	// ?? necessary? make sure to SetParent(, 0) in CCustomView::WM_CLOSE
	::ShowWindow(view->hViewWnd, SW_SHOW);
	::MoveWindow(view->hViewWnd, 0, 0, rc.right, rc.bottom, TRUE);
	return 0;
}

LRESULT CMainFrame::OnCustomViewSetPreTranslate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	CCustomView* view = customView.lookupView((HWND)wParam);
	if (!view) return 0;

	view->fnPreTranslateMessage = (BOOL (*)(MSG*))lParam;

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// SHOW VIEWS
// ---------------------------------------------------------------------------------------------------------------

// --- Parameter Editors ---

LRESULT CMainFrame::OnShowHideAllParameterFrames(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	showHideAllParameterFrames(true, false);
	return 0;
}

void CMainFrame::showHideAllParameterFrames(bool toggle, bool show)
{
	// TODO: support PVST-hax for ctrl+f11 = send a custom master-event for buzz2zzub, which passes CMachine* etc to PVST
	if (toggle)
		hideAllParameters = !hideAllParameters;
	else
		hideAllParameters = show;

	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		DynamicDockTabViewManager<CMainFrame, CView>* viewinfo = *i;
		for (size_t j = 0; j < (*i)->getViewCount(); j++) {
			CView* mpv = viewinfo->getView(j);
			ClientView* cv = frame.getClientView(mpv->GetHwnd());
			if (frame.isFloatClient(mpv->GetHwnd())) {
				HWND ff = frame.getFloatFrame(mpv->GetHwnd());
				frame.showFloatFrame(ff, !hideAllParameters);
			}
		}
	}
/*
	DynamicDockTabViewManager<CMainFrame, CView>* parameterView = GetViewInfo("CMachineParameterView");
	for (size_t i = 0; i < parameterView->getViewCount(); i++) {
		CView* mpv = parameterView->getView(i);
		ClientView* cv = frame.getClientView(mpv->GetHwnd());
		if (frame.isFloatClient(mpv->GetHwnd())) {
			HWND ff = frame.getFloatFrame(mpv->GetHwnd());
			frame.showFloatFrame(ff, !hideAllParameters);
		}
	}*/
	pvst_show_hide_all(!hideAllParameters);
	SetFocus();
}

void CMainFrame::ShowMachineParameters(zzub_plugin_t* plugin, MachineParameterViewMode modehint, int x, int y) {
	buze_event_data_t ev;
	ev.show_parameters.plugin = plugin;
	ev.show_parameters.mode = modehint;
	ev.show_parameters.x = x;
	ev.show_parameters.y = y;
	buze_document_notify_views(document, 0, buze_event_type_show_parameter_view, &ev);
}

// --- Help View ---
/*
void CMainFrame::showHelpView(std::string helptext) {
	DynamicDockTabViewManager<CMainFrame, CView>* helpView = GetViewInfo("HelpView");
	if (helpView->getViewCount() == 0) {
		ClientView* cv = helpView->createClientWindow(m_hWnd);
		assert(cv);
		helpView->insertClient(cv);
	} else {
		frame.setFocusTo(helpView->getViewHWND(0));
	}
	assert(false);
	//CHelpView* view = helpView->getView(0);
	//view->helpText.SetWindowText(helptext.c_str());
}

void CMainFrame::showHelpGlobal() {
	std::string helptext = PeekString(GetModuleHandle(0), IDT_HELP_HELPVIEW);
	std::string acceltext = CreateAccelTableString(hotkeys.mainframe_hAccel);
	showHelpView(helptext + acceltext);
}

void CMainFrame::showHelpPatternEditor() {
	std::string helptext = PeekString(GetModuleHandle(0), IDT_HELP_PATTERNVIEW);
	std::string acceltext = CreateAccelTableString(hotkeys.patternview_hAccel);
	showHelpView(helptext + acceltext);
}

void CMainFrame::showHelpMachineView() {
	std::string helptext = PeekString(GetModuleHandle(0), IDT_HELP_MACHINEVIEW);
	std::string acceltext = CreateAccelTableString(hotkeys.machineview_hAccel);
	showHelpView(helptext + acceltext);
}

void CMainFrame::showHelpWavetable() {
	std::string helptext = PeekString(GetModuleHandle(0), IDT_HELP_WAVETABLEVIEW);
	std::string acceltext = CreateAccelTableString(hotkeys.wavetable_hAccel);
	showHelpView(helptext + acceltext);
}

void CMainFrame::showHelpParameters() {
	std::string helptext = PeekString(GetModuleHandle(0), IDT_HELP_PARAMETERVIEW);
	std::string acceltext = CreateAccelTableString(hotkeys.parameter_hAccel);
	showHelpView(helptext + acceltext);
}
*/
// --- Preferences View ---
/*
LRESULT CMainFrame::OnShowPreferences(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	showPreferencesView();
	return 0;
}

void CMainFrame::showPreferencesView() {
	DynamicDockTabViewManager<CMainFrame, CView>* preferencesView = GetViewInfo("PreferencesView");
	if (preferencesView->getViewCount() == 0) {
		ClientView* cv = preferencesView->createClientWindow(m_hWnd);
		assert(cv);

		RECT rcWindow;
		frame.GetWindowRect(&rcWindow);
		RECT rcClient;
		frame.GetClientRect(&rcClient);

		int width = 500;
		int height = 600;

		RECT rcCreate;
		rcCreate.left = rcWindow.left + (rcClient.right / 2) - (width / 2);
		rcCreate.right = rcCreate.left + width;
		rcCreate.top = rcWindow.top + (rcClient.bottom / 2) - (height / 2);
		rcCreate.bottom = rcCreate.top + height;

		CWindow prefsWnd = preferencesView->insertClient(cv, &rcCreate);
	} else {
		frame.setFocusTo(preferencesView->getViewHWND(0));
	}
}
*/

// --- Generic view access ---


// a docktabframe view factory adapter for the general view factories
class DynamicDockTabViewInfoFactory : public DockSplitTab::DynamicDockTabViewFactory<CView> {
public:
	CViewInfo* factory;

	DynamicDockTabViewInfoFactory(CViewInfo* factory) {
		this->factory = factory;
		factory->Attach();
	}

	~DynamicDockTabViewInfoFactory() {
		factory->Detach();
		factory->Destroy();
	}

	virtual CView* CreateView(HWND hWndParent, void* lpParam) {
		return factory->CreateView(hWndParent, lpParam);
	}

	virtual std::string GetClassName() {
		return factory->uri;
	}

	virtual std::string GetLabel() {
		return factory->label;
	}

	virtual std::string GetToolTip() {
		return factory->tooltip;
	}
	virtual DockSplitTab::FramePlace GetPlace() {
		return (DockSplitTab::FramePlace)factory->place;
	}

	virtual DockSplitTab::DockSide GetSide() {
		return (DockSplitTab::DockSide)factory->side;
	}

	virtual bool GetSerializable() {
		return factory->serializable;
	}

	virtual bool GetAllowFloat() {
		return factory->allowfloat;
	}

	virtual bool GetDefaultView() {
		return factory->defaultview;
	}

	virtual int GetDefaultFloatWidth() {
		return factory->defaultfloatwidth;
	}

	virtual int GetDefaultFloatHeight() {
		return factory->defaultfloatheight;
	}
};

void CMainFrame::CloseView(HWND hWnd) {
	clientViewHide(hWnd);
}

DynamicDockTabViewManager<CMainFrame, CView>* CMainFrame::GetViewInfo(std::string name) {
	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		if ((*i)->className == name)
			return *i;
	}
	return 0;
}

CView* CMainFrame::GetView(const char* viewname, int view_id) {
	DynamicDockTabViewManager<CMainFrame, CView>* viewinfo = GetViewInfo(viewname);
	if (!viewinfo) return 0;

	for (size_t i = 0; i < viewinfo->getViewCount(); i++) {
		CView* view = viewinfo->getView(i);
		ClientView* cv = frame.getClientView(view->GetHwnd());
		if (cv->viewId == view_id)
		//if (view->GetUserData() == userdata)
			return view;
	}
	return 0;
}


void CMainFrame::RegisterViewInfo(CViewInfo* viewInfo) {

	DynamicDockTabViewManager<CMainFrame, CView>* viewMgr = 
		new DynamicDockTabViewManager<CMainFrame, CView>(
			this, 
			new DynamicDockTabViewInfoFactory(viewInfo)
		);

	//viewInfo->Attach();
	views.push_back(viewMgr);
}

HACCEL CMainFrame::GetAccelerators(const char* viewname) {
	return hotkeys.GetAccelerators(viewname);
}

HWND CMainFrame::GetFocusedClientView() {
	return frame.focusedClientView();
}

CView* CMainFrame::GetFocusedView() {
	HWND hViewWnd = GetFocusedClientView();

	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		CView* v = (*i)->lookupView(hViewWnd);
		if (v != 0)
			return v;
	}

	return 0; // at least we tried!
}


bool CMainFrame::IsFloatView(HWND hViewWnd) {
	return frame.isFloatClient(hViewWnd);
}

HMENU CMainFrame::GetMachineMenuCreate() {
	return createMachineMenu.m_hMenu;
}

HMENU CMainFrame::GetMachineMenuInsertAfter() {
	return insertafterMachineMenu.m_hMenu;
}

HMENU CMainFrame::GetMachineMenuInsertBefore() {
	return insertbeforeMachineMenu.m_hMenu;
}

HMENU CMainFrame::GetMachineMenuReplace() {
	return replaceMachineMenu.m_hMenu;
}

void CMainFrame::AddMenuKeys(const char* viewname, HMENU hMenu)  {
	hotkeys.AddMenuKeys(viewname, hMenu);
}

int CMainFrame::RegisterEvent() {
	nextEventCode++;
	return nextEventCode - 1;
}

WORD CMainFrame::RegisterAcceleratorEvent(const char* name, const char* hotkey, int event) {
	// register id<->event mapping in the mainframe -> OnMainFrameEventCommand
	WORD wID = nextEventCommand;
	nextEventCommand++;

	commandEventMap[wID] = event;

	hotkeys.RegisterAcceleratorId("mainframe", name, wID, hotkey);

	return wID;
}

void CMainFrame::RegisterAccelerator(const char* viewname, const char* name, const char* hotkey, WORD id) {
	hotkeys.RegisterAcceleratorId(viewname, name, id, hotkey);
}

void* CMainFrame::GetKeyjazzMap() {
	return &hotkeys.keyjazz_key_map;
}

class CCreateViewInfo {
public:
	std::string viewname;
	int view_id;
	std::string label;
	int x, y;
};


CView* CMainFrame::OpenView(const char* viewname, const char* label, int view_id, int x, int y) {

	CView* mpv = GetView(viewname, view_id);
	CCreateViewInfo* cvi = new CCreateViewInfo();
	cvi->viewname = viewname;
	cvi->label = label;
	cvi->view_id = view_id;
	cvi->x = x;
	cvi->y = y;

	if (mpv != 0) {
		clientViewHide(mpv->GetHwnd());
	}

	CView* result = CreateView(cvi);
	delete cvi;
	return result;
}


CView* CMainFrame::CreateView(CCreateViewInfo* cvi) {
	DynamicDockTabViewManager<CMainFrame, CView>* parameterView = GetViewInfo(cvi->viewname);

	ClientView* cv = parameterView->createClientWindow(m_hWnd, cvi->label, "", -1, cvi->view_id);

	CView* mpv = parameterView->lookupView(cv);

	RECT rc;
	if (parameterView->framePlace == DockSplitTab::placeFLOATFRAME) { //allowFloat) {

		// the view can suggest its own size after creation before appearing
		mpv->GetClientSize(&rc);

		// if the view has no coded size preference, use size from viewinfo
		if (rc.right == 0) {
			//cv->floatRect; last floating size?
			//parameterView->factory->hasFloatRect(cvi->label)
			rc.right = parameterView->factory->GetDefaultFloatWidth();
			rc.bottom = parameterView->factory->GetDefaultFloatHeight();
		}

		// TODO: GetDesktopWindow() + SHAppBarMessage(ABM_GETTASKBARPOS) to get the taskbar size?
		// TODO: Use frame client window styles with AdjustWindowRectEx to get the real window size
		RECT rcDesktop;
		GetWindowRect(&rcDesktop);
		//OffsetRect(&rcDesktop, -rcDesktop.left, -rcDesktop.top);
		rcDesktop.bottom -= 24; // tab height: TODO: should calculate tab height based on the font size calculations in TabControl::create()

		POINT position;
		if (cvi->x == -1) {
			// default window pos = mouse position or nearest fit
			GetCursorPos(&position);
			if (rc.bottom + position.y > rcDesktop.bottom) {
				// find height of floating window titlebar
				RECT rcTitle;
				SetRect(&rcTitle, 0, 0, 0, 0);
				DWORD dwStyle = DockSplitTab::Frame::getFloatFrameStyle();
				DWORD dwStyleEx = DockSplitTab::Frame::getFloatFrameStyleEx();
				AdjustWindowRectEx(&rcTitle, dwStyle, FALSE, dwStyleEx);

				LONG titleHeight = -rcTitle.top;

				position.y = std::max(titleHeight, rcDesktop.bottom - rc.bottom);
			}
			if (rc.right + position.x > rcDesktop.right) {
				position.x = std::max(0L, rcDesktop.right - rc.right);
			}
		} else {
			position.x = cvi->x;
			position.y = cvi->y;
		}

		OffsetRect(&rc, position.x, position.y);
	} else {
		rc = CWindow::rcDefault;
	}
	parameterView->insertClient(cv, &rc);

	return mpv;
}

void CMainFrame::SetFocusTo(HWND hWnd) {

	// show if it was hidden with hideallparameters
	ClientView* cv = frame.getClientView(hWnd);
	if (cv != 0 && frame.isFloatClient(hWnd)) {
		HWND ff = frame.getFloatFrame(hWnd);
		frame.showFloatFrame(ff, true);
	}

	frame.setFocusTo(hWnd);
}

HMENU CMainFrame::GetMainMenu() {
	return m_CmdBar.GetMenu();
}

CView* CMainFrame::GetViewByHwnd(HWND hWnd) {
	// hWnd could be a child control, the view itself, or the parent pane - find the view where it belongs
	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		for (size_t j = 0; j < (*i)->getViewCount(); j++) {
			CView* view = (*i)->getView(j);
			HWND hViewWnd = view->GetHwnd();
			// doing ischild both ways because the hwnd could be the parent container frame;
			// if its the mainframe or some parent splitter - the return value is most likely wrong, there must be a better way
			if (hViewWnd == hWnd || ::IsChild(hViewWnd, hWnd) || ::IsChild(hWnd, hViewWnd))
				return view;
		}
	}
	return 0;	
}


// ---------------------------------------------------------------------------------------------------------------
// PALETTE WINDOWS
// ---------------------------------------------------------------------------------------------------------------

/*
Instead, lParam is a handle to the other window being activated/deactivated in our place
(i.e., if we are being deactivated, lParam  will be the handle to the window being activated).
This is not always the case, specifically when the other window being activated/deactivated
belongs to another process. In this case, lParam will be zero.
*/

LRESULT CMainFrame::OnAddPaletteWindow(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	paletteWnds.insert((HWND)wParam);
	return 0;
}

LRESULT CMainFrame::OnRemovePaletteWindow(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	std::set<HWND>::iterator i = paletteWnds.find((HWND)wParam);
	if (i != paletteWnds.end())
		paletteWnds.erase(i);
	return 0;
}

LRESULT CMainFrame::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (GetForegroundWindow() == m_hWnd || (lParam == -1)) {
		for (std::set<HWND>::iterator i = paletteWnds.begin(); i != paletteWnds.end(); ++i) {
			CWindow paletteWnd(*i);
			paletteWnd.SendMessage(WM_NCACTIVATE, (WPARAM)TRUE, -1);
		}

		return DefWindowProc(WM_NCACTIVATE, (WPARAM)TRUE, 0);
	}
	else {
		for (std::set<HWND>::iterator i = paletteWnds.begin(); i != paletteWnds.end(); ++i) {
			CWindow paletteWnd(*i);
			paletteWnd.SendMessage(WM_NCACTIVATE, (WPARAM)FALSE, 0);
		}

		return DefWindowProc(WM_NCACTIVATE, (WPARAM)FALSE, 0);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// TITLE / STATUSBAR
// ---------------------------------------------------------------------------------------------------------------

// if window title contains "- Buzz - ", pvst seems to enter a deadlock on TrackPopupMenu when right-clicking it
// it is known that vst looks for both a window with title "- Buzz -", and another one called 'Machines' for refreshing the machine view
void CMainFrame::setWindowTitle(std::string const& text) {
	char pc[1024];
	if (text.length()>0) {
		//sprintf(pc, "%s - %s - Buzz - ", text.c_str(), programName);
		sprintf(pc, "%s - %s", text.c_str(), programName);
	} else {
		//sprintf(pc, "%s - Buzz - ", programName);
		sprintf(pc, "%s", programName);
	}

	SetWindowText(pc);
}

void CMainFrame::setStatus(std::string const& text) {
	CStatusBarCtrl statusBar(m_hWndStatusBar);
	statusBar.SetText(0, text.c_str());

	//::SetWindowText(m_hWndStatusBar, text.c_str());		
}

// ---------------------------------------------------------------------------------------------------------------
// MENU / TOOLBAR UI UPDATES
// ---------------------------------------------------------------------------------------------------------------

BOOL CMainFrame::OnIdle() {
	if (!player) return FALSE;

	UISetCheck(ID_LOCK_TOOLBARS, m_bLockedToolbars?TRUE:FALSE);

/*	UISetCheck(ID_VIEW_PRIMARYPATTERNEDITOR, GetViewInfo("PatternView")->getViewCount()>0?TRUE:FALSE);
	//UISetCheck(ID_VIEW_SECONDARYPATTERNEDITOR, secondaryPatternEditor?TRUE:FALSE);	
	UISetCheck(ID_VIEW_MACHINES, GetViewInfo("MachineView")->getViewCount()>0?TRUE:FALSE);
	UISetCheck(ID_VIEW_WAVETABLE, GetViewInfo("WaveTableView")->getViewCount()>0?TRUE:FALSE);
	UISetCheck(ID_VIEW_ANALYZER, GetViewInfo("AnalyzerView")->getViewCount()>0?TRUE:FALSE);
	UISetCheck(ID_VIEW_FILES, GetViewInfo("FileBrowserView")->getViewCount()>0?TRUE:FALSE);
	UISetCheck(ID_VIEW_ALLMACHINES, GetViewInfo("MachineFolderView")->getViewCount()>0?TRUE:FALSE);
	UISetCheck(ID_VIEW_CPUMETER, GetViewInfo("CpuMeterView")->getViewCount()>0?TRUE:FALSE);
	UISetCheck(ID_VIEW_COMMENT, GetViewInfo("CommentView")->getViewCount()>0?TRUE:FALSE);
	UISetCheck(ID_VIEW_HISTORY, GetViewInfo("HistoryView")->getViewCount()>0?TRUE:FALSE);
	UISetCheck(ID_VIEW_PATTERNFORMAT, GetViewInfo("PatternFormatView")->getViewCount()>0?TRUE:FALSE);
*/
	bool playing = zzub_player_get_state(player) == zzub_player_state_playing;
	UISetCheck(ID_PLAY_REPEAT, zzub_player_get_order_loop_enabled(player)?1:0);
	UISetCheck(ID_PLAY, playing?1:0);
	UISetCheck(ID_RECORD, zzub_player_get_automation(player)?1:0);
	UISetCheck(ID_DEVICE_RESET, zzub_audiodriver_get_enabled((zzub_audiodriver_t*)buze_application_get_audio_driver(application)) != 0?0:1);
	UISetCheck(ID_PLAY_SYNC, zzub_player_get_midi_transport(player)?1:0);

	UIEnable(ID_EDIT_TOGGLE_AUTOSAVE, FALSE);
	UIEnable(ID_EDIT_RENDER_SEQUENCE, FALSE);

	bool can_undo = zzub_player_history_get_position(player) > 0;
	bool can_redo = zzub_player_history_get_size(player) > 0 && zzub_player_history_get_position(player) < zzub_player_history_get_size(player);
	UIEnable(ID_EDIT_UNDO, can_undo);
	UIEnable(ID_EDIT_REDO, can_redo);

	HWND hViewWnd = frame.focusedClientView();
	DWORD editFlags = ::SendMessage(hViewWnd, WM_GET_EDITFLAGS, 0, 0);
	UIEnable(ID_EDIT_CUT, (editFlags & EDIT_COPY) != 0);
	UIEnable(ID_EDIT_COPY, (editFlags & EDIT_COPY) != 0);
	UIEnable(ID_EDIT_PASTE, (editFlags & EDIT_PASTE) != 0);

	UIUpdateToolBar();

	std::string currentFile = buze_document_get_current_filename(document);
	if (buze_document_is_dirty(document) == 0) {
		if (dirtyTitlebarAsterisk)
			setWindowTitle(currentFile); 
		dirtyTitlebarAsterisk = false;
	} else {
		if (!dirtyTitlebarAsterisk)
			setWindowTitle(currentFile + "*");
		dirtyTitlebarAsterisk = true;
	}

	return FALSE;
}

// ---------------------------------------------------------------------------------------------------------------
// FILE MENU
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	if (!saveIfDirty())
		return 0;
	clearSong();

	buze_document_create_default_document(document);
	zzub_player_history_commit(player, 0, 0, "");
	zzub_player_history_reset(player);

	// update and redraw panes
	buze_document_notify_views(document, 0, buze_event_type_update_new_document, 0);

	return 0;
}


bool CMainFrame::saveIfDirty() {
	std::string currentFile = buze_document_get_current_filename(document);

	if (buze_document_is_dirty(document)) {
		std::string question = "Save changes to " + currentFile + "?";
		int confirm = MessageBox(question.c_str(), programName, MB_YESNOCANCEL|MB_ICONEXCLAMATION);
		if (confirm == IDYES) {
			if (!saveCurrentSong(false))
				return false;
		} else
		if (confirm == IDCANCEL) {
			return false;
		}
	}
	return true;
}

bool CMainFrame::clearSong() {
	zzub_player_set_state(player, zzub_player_state_muted, -1);

	// close all song owned windows

	for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		if (!(*i)->serializable)
			(*i)->closeAll(true);
	}

	this->customView.closeAll(true);

	buze_document_clear_song(document);
	setWindowTitle("");

	return true;
}

LRESULT CMainFrame::OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	//MessageBox("Dropped file");
	// this oen works right away but is considered deprecated in windows
	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	std::string fileName = GetOpenFileName();
	if (fileName.empty()) return 0;

	openSongFromFile(fileName);
	zzub_player_history_commit(player, 0, 0, "");
	zzub_player_history_reset(player);

	zzub_player_set_position(player, 0, 0);
	return 0;
}

const char* CMainFrame::GetOpenFileName() {
	std::string currentFile = buze_document_get_current_filename(document);
	std::string currentDir = buze_document_get_current_path(document);

	OPENFILENAME ofn;       // common dialog box structure
	static char szFile[260];       // buffer for file name
	char szCurDir[260];
	strcpy(szCurDir, currentDir.c_str());

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All songs (*.armz, *.bmx, *.bmw, *.ccm, *.mod, *.s3m)\0*.armz;*.bmx;*.bmw;*.ccm;*.mod;*.s3m\0All files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	if (!currentDir.empty())
		ofn.lpstrInitialDir = szCurDir;

	ofn.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 
	if (::GetOpenFileName(&ofn)!=TRUE) return "";

	return ofn.lpstrFile;
}

bool CMainFrame::openSongFromFile(std::string const& fileName) {
	zzub_player_history_enable(player, 0);
	buze_document_delete_stream_plugin(document);
	zzub_player_history_commit(player, 0, 0, "");
	zzub_player_history_enable(player, 1);

	if (!saveIfDirty())
		return false;
	clearSong();

	// this prevents machines from doing gui stuff in the audio thread while loading (e.g live slice)
	zzub_player_set_state(player, zzub_player_state_muted, -1);

	buze_application_show_wait_window(application);
	buze_document_notify_views(document, 0, buze_event_type_update_pre_open_document, 0);

	char error_messages[1024];
	bool loadState = buze_document_import_song(document, fileName.c_str(), 0, 0, 0, error_messages, 1024) != 0 ? true : false;

	if (loadState) {
		buze_document_set_current_file(document, fileName.c_str());
		std::string currentFile = buze_document_get_current_filename(document);
		setWindowTitle(currentFile);
		setMostRecent(fileName);
		zzub_player_set_position(player, 0, 0);
	}

	buze_document_notify_views(document, 0, buze_event_type_update_post_open_document, 0);

	buze_application_hide_wait_window(application, m_hWnd);

	if (strlen(error_messages) > 0) {
		MessageBox(error_messages, "Load Warnings / Errors", MB_OK | MB_ICONINFORMATION);
	}

	zzub_player_set_state(player, zzub_player_state_stopped, -1);

	return true;
}

LRESULT CMainFrame::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	saveCurrentSong(false);
	return 0;
}

const char* CMainFrame::GetSaveFileName() {
	std::string currentFile = buze_document_get_current_filename(document);
	std::string currentDir = buze_document_get_current_path(document);

	OPENFILENAME ofn;       // common dialog box structure
	static char szFile[260];       // buffer for file name
	strcpy(szFile, currentFile.c_str());
	char szCurDir[260];
	strcpy(szCurDir, currentDir.c_str());

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	//ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All songs (*.armz)\0*.armz\0All files\0*.*\0\0";
	ofn.lpstrDefExt = "armz";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	if (!currentDir.empty())
		ofn.lpstrInitialDir = szCurDir;

	ofn.Flags = OFN_NOCHANGEDIR |OFN_OVERWRITEPROMPT|OFN_EXTENSIONDIFFERENT|OFN_NOREADONLYRETURN;

	// Display the Open dialog box. 
	if (::GetSaveFileName(&ofn)!=TRUE) return "";

	return ofn.lpstrFile;
}

LRESULT CMainFrame::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	saveCurrentSong(true);
	return 0;
}

bool CMainFrame::saveCurrentSong(bool saveAs) {
	std::string fileName;
	std::string currentFile = buze_document_get_current_filename(document);
	std::string currentExt = buze_document_get_current_extension(document);
	std::string currentDir = buze_document_get_current_path(document);
	if (!saveAs && currentFile.size() && currentFile != "Untitled" && currentExt == "armz") {
		fileName = currentDir + "\\" + currentFile + ".armz";
	} else {
		fileName = GetSaveFileName();
		if (fileName == "") return false;
	}
	return saveFile(fileName, true);
}

bool CMainFrame::saveFile(std::string const& filename, bool withWaves) {

	if (buze_document_save_file(document, filename.c_str(), withWaves)) {
		// TODO: handle UpdatePostSaveDocument instead
		std::string currentFile = buze_document_get_current_filename(document);
		setWindowTitle(currentFile);
		setMostRecent(filename);
	}
	return 0;
}

LRESULT CMainFrame::OnSetRecent(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int recentIndex = wID - ID_SET_RECENT_FIRST;
	openSongFromFile(getRecentName(recentIndex));
	zzub_player_history_commit(player, 0, 0, "");
	zzub_player_history_reset(player);
	zzub_player_set_position(player, 0, 0);
//	zzub_player_history_flush_last(player);
	return 0;
}

void CMainFrame::setMostRecent(std::string const& fileName) {
	configuration->insertRecentSong(fileName);
	initializeRecent(true);
}

void CMainFrame::initializeRecent(bool update) {
	CMenuHandle mainMenu;
	CMenuHandle viewMenu;
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU;
	int i;
	std::string fileName;
	char pc[1024];

	if (update)
		viewMenu = recentMenu;
	else {
		mainMenu = m_CmdBar.GetMenu();
		viewMenu = mainMenu.GetSubMenu(0);
		if (recentMenu.m_hMenu) recentMenu.DestroyMenu();
		recentMenu.CreatePopupMenu();
	}
	recents.clear();

	if (update)
		while(recentMenu.GetMenuItemCount())
			recentMenu.RemoveMenu(0, MF_BYPOSITION);

	for (i = 0; i < configuration->getMaxRecentSongs(); ++i) {
		fileName = configuration->getRecentSong(i);
		if (fileName.size()) {
			recentMenu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, ID_SET_RECENT_FIRST + recents.size(), fileName.c_str());
			recents.push_back(fileName);
		}
	}

	for (i = 0; i < viewMenu.GetMenuItemCount(); i++) {
		UINT menuID = viewMenu.GetMenuItemID(i);
		viewMenu.GetMenuString(menuID, pc, 1024, 0);
		if (std::string(pc) == "Recent Files") {
			mii.hSubMenu = recentMenu.m_hMenu;
			viewMenu.SetMenuItemInfo(i, TRUE, &mii);
			break;
		}
	}
}

std::string CMainFrame::getRecentName(size_t index) {
	return recents[index];
}

// ---------------------------------------------------------------------------------------------------------------
// EDIT MENU
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnUndo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_undo(player);
	return 0;
}

LRESULT CMainFrame::OnRedo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_redo(player);
	return 0;
}

LRESULT CMainFrame::OnCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HWND hViewWnd = frame.focusedClientView();
	DWORD editFlags = ::SendMessage(hViewWnd, WM_GET_EDITFLAGS, 0, 0);
	if (editFlags & EDIT_COPY) ::SendMessage(hViewWnd, WM_COMMAND, MAKEWPARAM(ID_EDIT_CUT, 0), 0);
	return 0;
}

LRESULT CMainFrame::OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HWND hViewWnd = frame.focusedClientView();
	DWORD editFlags = ::SendMessage(hViewWnd, WM_GET_EDITFLAGS, 0, 0);
	if (editFlags & EDIT_COPY) ::SendMessage(hViewWnd, WM_COMMAND, MAKEWPARAM(ID_EDIT_COPY, 0), 0);
	return 0;
}

LRESULT CMainFrame::OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HWND hViewWnd = frame.focusedClientView();
	DWORD editFlags = ::SendMessage(hViewWnd, WM_GET_EDITFLAGS, 0, 0);
	if (editFlags & EDIT_PASTE) ::SendMessage(hViewWnd, WM_COMMAND, MAKEWPARAM(ID_EDIT_PASTE, 0), 0);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// HELP MENU
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	buze_event_data_t ev;
	buze_document_notify_views(document, 0, buze_event_type_show_help_view, &ev);
	return 0;
}

#include "HelpHost.h"
#include <htmlhelp.h>

LRESULT CMainFrame::OnViewManual(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HWND hHelp = HtmlHelp(m_hWnd, "Docs\\Buze.chm", HH_DISPLAY_TOPIC, 0);

	/*

	proof-of-concept: set a queryservice and handle events from the help viewers web browser:

	if (helpHost == NULL) {
		// TODO: create in OnCreate, release in OnClose
		CComObject<CHelpHost>::CreateInstance(&helpHost);
		helpHost->mainframe = this;
		helpHost->AddRef();
	}

	HtmlHelp(m_hWnd, "Docs\\Buze.chm", HH_SET_QUERYSERVICE, (DWORD_PTR)helpHost);

	*/
	return 0;
}

void WaitAndPump(HWND hWndParent, HANDLE hHandle) {
	if (hWndParent != NULL)
		EnableWindow(hWndParent, FALSE);

	MSG msg;

	for (;;) {
		if (WaitForSingleObject(hHandle, 0) == WAIT_OBJECT_0) 
			break;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) 
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (hWndParent != NULL)
		EnableWindow(hWndParent, TRUE);

}

LRESULT CMainFrame::OnAppUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	// this commenting-out is matched in buze.rc where ID_APP_UPDATE is commented out of the help menu
	/*
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	BOOL result = CreateProcess(0, "updater.exe", 0, 0, FALSE, CREATE_NO_WINDOW, 0, 0, &si, &pi);

	if (result) {
		buze_application_show_wait_window(application);

		buze_application_set_wait_text(application, "Checking for update...");

		WaitAndPump(m_hWnd, pi.hProcess);

		buze_application_hide_wait_window(application, m_hWnd);
	} else {
		MessageBox("Could not check for update", GetProgramName());
	}*/
	return 0;
}


// ---------------------------------------------------------------------------------------------------------------
// REBARS / TOOLBARS
// ---------------------------------------------------------------------------------------------------------------

// based on AddSimpleReBarBandCtrl from atlframe.h 
static BOOL AddComplexReBarBand(HWND hWndReBar, HWND hWndBand, int nID = 0, BOOL bNewRow = FALSE, int cxWidth = 0, int cxMinWidth = 0, BOOL bHidden = FALSE, BOOL bLock = FALSE) {
	// Get number of buttons on the toolbar
	int nBtnCount = (int)::SendMessage(hWndBand, TB_BUTTONCOUNT, 0, 0L);

	// Set band info structure
	REBARBANDINFO rbBand = { RunTimeHelper::SizeOf_REBARBANDINFO() };
#if (_WIN32_IE >= 0x0400)
	rbBand.fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE | RBBIM_IDEALSIZE;
#else
	rbBand.fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE;
#endif // !(_WIN32_IE >= 0x0400)

	rbBand.fStyle = RBBS_CHILDEDGE;
#if (_WIN32_IE >= 0x0500)
	if(nBtnCount > 0)   // add chevron style for toolbar with buttons
		rbBand.fStyle |= RBBS_USECHEVRON;
#endif // (_WIN32_IE >= 0x0500)
	if(bNewRow)
		rbBand.fStyle |= RBBS_BREAK;

	if (bHidden)
		rbBand.fStyle |= RBBS_HIDDEN;

	if (bLock) {
		rbBand.fStyle &= ~RBBS_GRIPPERALWAYS;
		rbBand.fStyle |= RBBS_NOGRIPPER;
	}
	else {
		rbBand.fStyle &= ~RBBS_NOGRIPPER;
		rbBand.fStyle |= RBBS_GRIPPERALWAYS;
	}

	rbBand.lpText = 0;
	rbBand.hwndChild = hWndBand;
	if(nID == 0)   // calc band ID
		nID = ATL_IDW_BAND_FIRST + (int)::SendMessage(hWndReBar, RB_GETBANDCOUNT, 0, 0L);
	rbBand.wID = nID;

	// Calculate the size of the band
	BOOL bRet = FALSE;
	RECT rcTmp = { 0 };
	if(nBtnCount > 0)
	{
		bRet = (BOOL)::SendMessage(hWndBand, TB_GETITEMRECT, nBtnCount - 1, (LPARAM)&rcTmp);
		ATLASSERT(bRet);
		rbBand.cx = (cxWidth != 0) ? cxWidth : rcTmp.right;
		rbBand.cyMinChild = rcTmp.bottom - rcTmp.top;
		rbBand.cxIdeal = rcTmp.right;
		if (cxMinWidth != 0) {
			rbBand.cxMinChild = cxMinWidth;
		} else {
			//bRet = (BOOL)::SendMessage(hWndBand, TB_GETITEMRECT, 0, (LPARAM)&rcTmp);
			//ATLASSERT(bRet);
			rbBand.cxMinChild = rcTmp.right;
		}
	}
	else	// no buttons, either not a toolbar or really has no buttons
	{
		bRet = ::GetWindowRect(hWndBand, &rcTmp);
		ATLASSERT(bRet);
		rbBand.cx = (cxWidth != 0) ? cxWidth : (rcTmp.right - rcTmp.left);
		rbBand.cxMinChild = (cxMinWidth != 0) ? cxMinWidth : rbBand.cx;
		rbBand.cyMinChild = rcTmp.bottom - rcTmp.top;
		rbBand.cxIdeal = rbBand.cxMinChild;
	}

	// Add the band
	LRESULT lRes = ::SendMessage(hWndReBar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
	if(lRes == 0)
	{
		ATLTRACE2(atlTraceUI, 0, _T("Failed to add a band to the rebar.\n"));
		return FALSE;
	}

#if (_WIN32_IE >= 0x0501)
	DWORD dwExStyle = (DWORD)::SendMessage(hWndBand, TB_GETEXTENDEDSTYLE, 0, 0L);
	::SendMessage(hWndBand, TB_SETEXTENDEDSTYLE, 0, dwExStyle | TBSTYLE_EX_HIDECLIPPEDBUTTONS);
#endif // (_WIN32_IE >= 0x0501)
	return TRUE;
}

void AdjustToolbarSizeForSeparators(HWND hWndRebar, HWND hWndToolbar) {
	SIZE size;
	SendMessage(hWndToolbar, TB_GETMAXSIZE, 0, (LPARAM)&size);

	CReBarCtrl rebar = hWndRebar;
	int index = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band

	REBARBANDINFO rbbi;
	memset(&rbbi, 0, sizeof(REBARBANDINFO));
	rbbi.cbSize = sizeof(REBARBANDINFO);
	rbbi.fMask = RBBIM_SIZE;
	rbbi.cx = size.cx+20;	// oops - hardkoder denne!

	rebar.SetBandInfo(index, &rbbi);
}

struct PERSISTTOOLBAR {
	int id;
	HWND hWnd;
	bool bNewRow;
	int cxWidth;
	int position;
};

bool operator<(const PERSISTTOOLBAR& a, const PERSISTTOOLBAR& b) {
	return a.position < b.position;
}

// toolbar defaults - the hWnd is filled in createToolbars()
PERSISTTOOLBAR toolbars[] = { 
	{ ATL_IDW_COMMAND_BAR, 0, true, 0 }, 
	{ ID_VIEW_TOOLBAR, 0, false, 0 },
	//{ ID_VIEW_VIEWS, 0, false, 0 },
	{ ID_VIEW_TRANSPORTTOOLBAR, 0, false, 0 },
	{ ID_VIEW_TIMETOOLBAR, 0, false, 3*(100+2) },
	{ ID_VIEW_MASTERTOOLBAR, 0, false, 400 }
};
int num_toolbars = array_size(toolbars);

void CMainFrame::OnInitMenuPopup(CMenu menuPopup, UINT nIndex, BOOL bSysMenu) {
	SetMsgHandled(FALSE);
	//DefWindowProc();
}

void CMainFrame::OnInitMenu(CMenu menu) {
	SetMsgHandled(FALSE);
	//DefWindowProc();
}

void CMainFrame::createToolbars() {
	CMenuHandle mainToolbars;
	mainToolbars.LoadMenu(MAKEINTRESOURCE(IDR_MAINFRAME_TOOLBARS));

	//CMenuHandle patternToolbars;
	//patternToolbars.LoadMenu(MAKEINTRESOURCE(IDR_PATTERNVIEW_TOOLBARS));

	CMenuHandle mainMenu = GetMenu();
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(2, MF_BYPOSITION|MF_POPUP, (UINT_PTR)(HMENU)mainToolbars.GetSubMenu(0), "Main Toolbars");
	//viewMenu.InsertMenu(3, MF_BYPOSITION|MF_POPUP, (UINT_PTR)(HMENU)patternToolbars.GetSubMenu(0), "Pattern Toolbars");

	// create command bar window
	m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE, 0, ATL_IDW_COMMAND_BAR);
	// attach menu
	m_CmdBar.AttachMenu(mainMenu);
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);
// 	m_CmdBar.SetCommandBarExtendedStyle(0, CBR_EX_TRACKALWAYS);
// 	m_CmdBar.SetCommandBarExtendedStyle(0, CBR_EX_ALTFOCUSMODE);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	HWND hWndTransport = CreateSimpleToolBarCtrl(m_hWnd, IDR_TRANSPORT, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	//HWND hWndViews = CreateSimpleToolBarCtrl(m_hWnd, IDR_VIEWS, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	transportCtrl.Attach(hWndTransport);

	RECT rc;
	SetRect(&rc, 0, 0, 200, 20);
	masterToolbar.Create(m_hWnd, rc, FALSE, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);

	SetRect(&rc, 0, 0, 3*(100+2), 20);
	songToolbar.Create(m_hWnd, rc, FALSE, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);

	AdjustToolbarSizeForSeparators(m_hWndToolBar, hWndToolBar);
	AdjustToolbarSizeForSeparators(m_hWndToolBar, hWndTransport);

	CreateSimpleStatusBar();
	//CStatusBarCtrl statusBar(m_hWndStatusBar);
	statusBar.SubclassWindow(m_hWndStatusBar);
	int statusWidths[] = { -1, -1, -1 };
	statusBar.SetParts(3, statusWidths);

	int arrPanes[] = { ID_DEFAULT_PANE, IDR_MIDILOCK_LOCKED, IDR_STATUS_CPU };
	statusBar.SetPanes(arrPanes, sizeof(arrPanes) / sizeof(int), false);
	lockIcon = AtlLoadIconImage(IDR_MIDILOCK_LOCKED, LR_DEFAULTCOLOR);
	unlockIcon = AtlLoadIconImage(IDR_MIDILOCK_UNLOCKED, LR_DEFAULTCOLOR);

	// litt om UI updating: http://www.idevresource.com/COM/library/articles/wtlarch.asp
	UIAddToolBar(hWndToolBar);
	UIAddToolBar(hWndTransport);
	//UIAddToolBar(hWndViews);

	toolbars[0].hWnd = m_CmdBar;
	toolbars[1].hWnd = hWndToolBar;
	//toolbars[2].hWnd = hWndViews;
	toolbars[2].hWnd = hWndTransport;
	toolbars[3].hWnd = songToolbar;
	toolbars[4].hWnd = masterToolbar;
}

LRESULT CMainFrame::OnClickStatus(int /*idCtrl*/, LPNMHDR pnmh, BOOL& bHandled) {
	LPNMMOUSE nm = (LPNMMOUSE)pnmh;
	if (nm->dwItemSpec == 1) {
		if (zzub_player_get_midi_plugin(player) != 0) {
			zzub_player_set_midi_lock(player, !(zzub_player_get_midi_lock(player) != 0));
		}
	} else
		bHandled = false;
	return 0;
}

void CMainFrame::loadToolbarState() {
	std::vector<PERSISTTOOLBAR> tb;

	for (int i = 0; i < num_toolbars; ++i) {
		BOOL bNewRow = configuration->getToolbarBreak(toolbars[i].id, toolbars[i].bNewRow);
		int cxWidth = configuration->getToolbarWidth(toolbars[i].id, toolbars[i].cxWidth);
		int cxMinWidth = toolbars[i].cxWidth;
		BOOL bHidden = !configuration->getToolbarVisible(toolbars[i].id, true);
		toolbars[i].position = configuration->getToolbarPosition(toolbars[i].id, i);
		tb.push_back(toolbars[i]);
		//AddSimpleReBarBand(toolbars[i].hWnd, NULL, bNewRow, cxWidth, TRUE);
		AddComplexReBarBand(m_hWndToolBar, toolbars[i].hWnd, toolbars[i].id, bNewRow, cxWidth, cxMinWidth, bHidden, m_bLockedToolbars);
	}

	CReBarCtrl rebar = m_hWndToolBar;
	for (std::vector<PERSISTTOOLBAR>::iterator i = tb.begin(); i != tb.end(); ++i) {
		int index = rebar.IdToIndex(i->id); 
		rebar.MoveBand(index, i->position);
	}

	::ShowWindow(m_hWndStatusBar, getToolbarVisibility(ID_VIEW_STATUS_BAR) ? SW_SHOWNOACTIVATE : SW_HIDE);
	setToolbarVisibility(ID_VIEW_STATUS_BAR, getToolbarVisibility(ID_VIEW_STATUS_BAR));

	setMainFrameToolbarVisibility(ID_VIEW_TOOLBAR, getToolbarVisibility(ID_VIEW_TOOLBAR), FALSE, FALSE);
	//setMainFrameToolbarVisibility(ID_VIEW_VIEWS, getToolbarVisibility(ID_VIEW_VIEWS), FALSE, FALSE);
	setMainFrameToolbarVisibility(ID_VIEW_TRANSPORTTOOLBAR, getToolbarVisibility(ID_VIEW_TRANSPORTTOOLBAR), FALSE, FALSE);
	setMainFrameToolbarVisibility(ID_VIEW_TIMETOOLBAR, getToolbarVisibility(ID_VIEW_TIMETOOLBAR), FALSE, FALSE);
	setMainFrameToolbarVisibility(ID_VIEW_MASTERTOOLBAR, getToolbarVisibility(ID_VIEW_MASTERTOOLBAR), FALSE, FALSE);
}

void CMainFrame::saveToolbarState() {
	CReBarCtrl rebar = m_hWndToolBar;

	for (int i = 0; i < num_toolbars; i++) {
		int index = rebar.IdToIndex(toolbars[i].id);
		REBARBANDINFO rbbi;
		rbbi.cbSize = sizeof(REBARBANDINFO);
		rbbi.fMask = RBBIM_SIZE | RBBIM_STYLE | RBBIM_ID;
		rebar.GetBandInfo(index, &rbbi);
		configuration->setToolbarPosition(toolbars[i].id, index);
		configuration->setToolbarWidth(toolbars[i].id, rbbi.cx);
		configuration->setToolbarBreak(toolbars[i].id, rbbi.fStyle & RBBS_BREAK);
	}

	configuration->setLockedToolbars(m_bLockedToolbars);
}

BOOL CMainFrame::getToolbarVisibility(WORD wID) {
	return configuration->getToolbarVisible(wID, true);
}

void CMainFrame::setToolbarVisibility(WORD wID, BOOL bVisible, BOOL bSave, BOOL bUpdate) {
	if (bSave) configuration->setToolbarVisible(wID, bVisible != 0);

	UISetCheck(wID, bVisible);
	if (bUpdate) UpdateLayout();
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !getToolbarVisibility(ID_VIEW_TOOLBAR);
	setMainFrameToolbarVisibility(ID_VIEW_TOOLBAR, bVisible);
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !getToolbarVisibility(ID_VIEW_STATUS_BAR);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	setToolbarVisibility(ID_VIEW_STATUS_BAR, bVisible);
	return 0;
}

LRESULT CMainFrame::OnViewTimeToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !getToolbarVisibility(ID_VIEW_TIMETOOLBAR);
	setMainFrameToolbarVisibility(ID_VIEW_TIMETOOLBAR, bVisible);
	return 0;
}
/*
LRESULT CMainFrame::OnViewViewsToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = !getToolbarVisibility(ID_VIEW_VIEWS);
	setMainFrameToolbarVisibility(ID_VIEW_VIEWS, bVisible);
	return 0;
}
*/
LRESULT CMainFrame::OnViewMasterToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !getToolbarVisibility(ID_VIEW_MASTERTOOLBAR);
	setMainFrameToolbarVisibility(ID_VIEW_MASTERTOOLBAR, bVisible);
	return 0;
}

LRESULT CMainFrame::OnViewTransportToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !getToolbarVisibility(ID_VIEW_TRANSPORTTOOLBAR);
	setMainFrameToolbarVisibility(ID_VIEW_TRANSPORTTOOLBAR, bVisible);
	return 0;
}

void CMainFrame::setMainFrameToolbarVisibility(WORD wID, BOOL bVisible, BOOL bSave, BOOL bUpdate) {
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(wID);
	rebar.ShowBand(nBandIndex, bVisible);

	setToolbarVisibility(wID, bVisible, bSave, bUpdate);
}

// ---------------------------------------------------------------------------------------------------------------
// TOOLBAR LOCKING
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnLockToolbars(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	m_bLockedToolbars = !m_bLockedToolbars;

	CReBarCtrl rebar(m_hWndToolBar);
	rebar.LockBands(m_bLockedToolbars);///FIX

	configuration->setLockedToolbars(m_bLockedToolbars);
	buze_document_notify_views(document, 0, buze_event_type_update_settings, 0);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// MACHINE MENU
// ---------------------------------------------------------------------------------------------------------------
/*
void moveAndReplaceIndexItem(MachineIndex& index, std::string dest, std::string src) {
	MachineMenu* destItem = index.root.getMenuByName(dest);
	if (!destItem) {
		return ;
	}

	MachineMenu* srcItem = index.root.getMenuByName(src);
	if (!srcItem) {
		return ;
	}

	srcItem->parent->removeItem(srcItem);
	destItem->parent->replaceItem(destItem, srcItem);
}*/

void SetMenuItemData(CMenuHandle menu, int pos, void* data) {
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_DATA;
	mii.dwItemData = (ULONG_PTR)data;
	menu.SetMenuItemInfo(pos, TRUE, &mii);
}

void CMainFrame::rebuildMachineMenus()
{
	int machines;

	buze_plugin_index_item_t* rootItem = buze_document_get_plugin_index_root(document);

	machines = 0;
	if (createMachineMenu.IsMenu()) createMachineMenu.DestroyMenu();
	createMachineMenu.CreatePopupMenu();
	buildMachineMenu(createMachineMenu.m_hMenu, rootItem, ID_CREATEMACHINECOMMANDS, machines);

	machines = 0;
	if (insertbeforeMachineMenu.IsMenu()) insertbeforeMachineMenu.DestroyMenu();
	insertbeforeMachineMenu.CreatePopupMenu();
	buildMachineMenu(insertbeforeMachineMenu.m_hMenu, rootItem, ID_INSERTBEFOREMACHINECOMMANDS, machines);

	machines = 0;
	if (insertafterMachineMenu.IsMenu()) insertafterMachineMenu.DestroyMenu();
	insertafterMachineMenu.CreatePopupMenu();
	buildMachineMenu(insertafterMachineMenu.m_hMenu, rootItem, ID_INSERTAFTERMACHINECOMMANDS, machines);

	machines = 0;
	if (replaceMachineMenu.IsMenu()) replaceMachineMenu.DestroyMenu();
	replaceMachineMenu.CreatePopupMenu();
	buildMachineMenu(replaceMachineMenu.m_hMenu, rootItem, ID_REPLACEMACHINECOMMANDS, machines);
}

int CMainFrame::buildMachineMenu(CMenuHandle parentMenu, buze_plugin_index_item_t* parentMachineMenu, int firstCommand, int& index) {

	int visibleCounter = 0;
	int depthCounter = 0;
	int maxEntriesPerPage;

	CWindow desktop(GetDesktopWindow());

	RECT rcClient;
	desktop.GetClientRect(&rcClient);
	maxEntriesPerPage = (rcClient.bottom - 16) / 20;	// assume 16px margins, 22px per item

	int subitemcount = buze_plugin_index_item_get_sub_item_count(parentMachineMenu);
	for (int i = 0; i < subitemcount; i++) {

		DWORD extraFlags = ((depthCounter%maxEntriesPerPage)==(maxEntriesPerPage-1)?MF_MENUBARBREAK:0);

		buze_plugin_index_item_t* subitem = buze_plugin_index_item_get_sub_item(parentMachineMenu, i);
		//IndexItem* ii = parentMachineMenu.items[i];
		if (buze_plugin_index_item_is_hidden(subitem)) continue;
		int type = buze_plugin_index_item_get_type(subitem);
		std::string name = buze_plugin_index_item_get_name(subitem);
		if (type == 0) {

			CMenuHandle newMenu;
			newMenu.CreatePopupMenu();

			int start_index = index;
			// only insert if there are any child nodes worth mentioning
			int insertedCount = buildMachineMenu(newMenu.m_hMenu, subitem, firstCommand, index);
			if (!insertedCount) continue;

			// hack to support PVST menu-hack - replace the auto-generated menu with a nicer one :(
			if (name.find("[PVSTi]") == 0) {
				HMENU pvstmenu = pvst_get_menu(1, firstCommand + start_index);
				if (pvstmenu) {
					newMenu.DestroyMenu();
					newMenu = pvstmenu;
				}
			} else
			if (name.find("[PVST]") == 0) {
				HMENU pvstmenu = pvst_get_menu(0, firstCommand + start_index);
				if (pvstmenu) {
					newMenu.DestroyMenu();
					newMenu = pvstmenu;
				}
			}

			parentMenu.InsertMenu(-1, MF_BYPOSITION|MF_POPUP|extraFlags, (UINT_PTR)newMenu.m_hMenu, name.c_str());
			SetMenuItemData(parentMenu, parentMenu.GetMenuItemCount() - 1, subitem);
			visibleCounter += insertedCount;
			depthCounter++;
		} else
		if (type==1) {
			// only insert this machine if it exists

			std::string fileName = buze_plugin_index_item_get_filename(subitem);

			bool is_template = fileName == "@zzub.org/buze/template";

			zzub_pluginloader_t* loader = 0;
			if (!is_template) {
				loader = zzub_player_get_pluginloader_by_name(player, fileName.c_str());

				if (loader == 0) {
					// try to rewrite as an buzz2zzub-uri
					fileName = rewriteBuzzWrapperUri(fileName);
					// TODO: before the menu item uri was rewritten! the set-filename-api hasn't been propagated yet!
					loader = zzub_player_get_pluginloader_by_name(player, fileName.c_str());
				}
			}

			if (is_template || loader != 0) {
				parentMenu.InsertMenu(-1, MF_BYCOMMAND|extraFlags, (UINT_PTR)(firstCommand+index), name.c_str());

				SetMenuItemData(parentMenu, parentMenu.GetMenuItemCount() - 1, subitem);

				visibleCounter++;
				depthCounter++;
			}
			index++;
		} else
		if (type==2) {
			//MachineSeparator& msep = *(MachineSeparator*)ii;
			std::string sepid = buze_plugin_index_item_get_separator_id(subitem);
			MachineMenuItem* item = plur.getItem(sepid);

			if (item != 0)
				extraFlags |= MF_OWNERDRAW; else
				extraFlags |= MF_SEPARATOR;

			parentMenu.InsertMenu(-1, MF_BYPOSITION|extraFlags, (UINT_PTR)0, (LPCTSTR)item);
			depthCounter++;
		}
	}

	return visibleCounter;
}

LRESULT CMainFrame::OnMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	LPMEASUREITEMSTRUCT lpMeasureItem = (LPMEASUREITEMSTRUCT)lParam;
	MachineMenuItem* item = (MachineMenuItem*)lpMeasureItem->itemData;

	lpMeasureItem->itemWidth = item->style->headerWidth;
	lpMeasureItem->itemHeight = item->style->headerHeight;

	return TRUE;
}

LRESULT CMainFrame::OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
	MachineMenuItem* item = (MachineMenuItem*)lpDrawItem->itemData;
	plur.drawItem(lpDrawItem, item);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// TIMER
// ---------------------------------------------------------------------------------------------------------------

void CMainFrame::AddTimerHandler(CView* handler) {
	timerHandlers.push_back(handler);
}

void CMainFrame::RemoveTimerHandler(CView* handler) {
	for (size_t i = 0; i < timerHandlers.size(); i++) {
		if (timerHandlers[i] == handler) {
			timerHandlers.erase(timerHandlers.begin() + i);
			break;
		}
	}
}

void CMainFrame::initTimer() {
	SetTimer(ID_MAINFRAME_TIMER, 10);
}

LRESULT CMainFrame::OnTimer(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if (wParam != ID_MAINFRAME_TIMER) return 0;

	float peaks[64];
	memset(peaks, 0, sizeof(peaks));
	int channels = 0;
	zzub_player_get_peaks(player, peaks, &channels);
	for (int i = 1; i < channels / 2; i++) {
		peaks[0] += peaks[i * 2 + 0];
		peaks[1] += peaks[i * 2 + 1];
	}
	
	masterToolbar.volumeSlider.SetPeak(peaks[0], peaks[1]);

	if ((timerCount % 5) == 0) { /// 50ms
		zzub_player_handle_events(player);

		// NOTE: "get_uncomitted_operations" is a misleading name
		if (zzub_player_history_get_uncomitted_operations(player)) {
			zzub_player_history_commit(player, 0, 0, "Edits from audio thread");
		}
	}

	if ((timerCount % 1) == 0) { /// 10ms
		for (size_t i = 0; i < timerHandlers.size(); ++i) {
			timerHandlers[i]->UpdateTimer(timerCount);
		}
	}

	if ((timerCount % 8) == 0) { /// 80ms
		songToolbar.update(); // song toolbar -- time readouts
	}

	if ((timerCount % 20) == 0) { /// 200ms
		updateTick(); // master toolbar -- BPM, TPB, Tempo, Visual Volume Slider

		std::stringstream midiStatus;
		zzub_plugin_t* midi_plugin = zzub_player_get_midi_plugin(player);
		if (midi_plugin != 0) {
			const char* name = zzub_plugin_get_name(midi_plugin);
			midiStatus << "MIDI Focus: " << name;
			if (zzub_player_get_midi_lock(player) != 0) 
				statusBar.SetPaneIcon(IDR_MIDILOCK_LOCKED, lockIcon);
			else
				statusBar.SetPaneIcon(IDR_MIDILOCK_LOCKED, unlockIcon);
		} else {
			statusBar.SetPaneIcon(IDR_MIDILOCK_LOCKED, 0);
		}

		statusBar.SetText(1, midiStatus.str().c_str());

		std::stringstream cpuStatus;
		float audioload = (float)zzub_audiodriver_get_cpu_load((zzub_audiodriver_t*)buze_application_get_audio_driver(application)) * 100.0f;
		cpuStatus << "CPU: " << std::fixed << std::setprecision(2) << audioload << "%";
		statusBar.SetText(2, cpuStatus.str().c_str());

		// get status message from focused view
		HWND hViewWnd = frame.focusedClientView();

		const char* statusResult = 0;
		if (hViewWnd) 
			statusResult = (const char*)SendMessage(hViewWnd, WM_GET_STATUS_TEXT, 0, 0);

		if (statusResult != 0) {
			setStatus(statusResult);
		} else {
			const int cchMax = 128;   // max text length is 127 for status bars (+1 for null)
			TCHAR szText[cchMax];
			szText[0] = 0;
			::LoadString(ModuleHelper::GetResourceInstance(), ATL_IDS_IDLEMESSAGE, szText, cchMax);
			setStatus(szText);
		}
	}

	++timerCount;
	return 0;
}

void CMainFrame::updateTick() {
	int value, prevValue;

	if (!masterToolbar.editing_bpm) {
		masterToolbar.bpmDropDown.GetInt(prevValue);
		value = (int)zzub_player_get_bpm(player);
		if (value != 65535 && value != prevValue) {
			masterToolbar.bpmDropDown.SetInt(value);
		}
	}

	if (!masterToolbar.editing_tpb) {
		masterToolbar.tpbDropDown.GetInt(prevValue);
		value = zzub_player_get_tpb(player);
		if (value != 255 && value != prevValue) {
			masterToolbar.tpbDropDown.SetInt(value);
		}
	}

	//if (GetFocus() != masterToolbar.volumeSlider.trackBar) {
		if (zzub_player_get_plugin_count(player) > 0) {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_name(player, "Master");
			if (plugin != 0) {
				int v = zzub_plugin_get_parameter_value(plugin, 1, 0, 0);
				masterToolbar.volumeSlider.SetValue(v);
			}
		}
	//}
}

// ---------------------------------------------------------------------------------------------------------------
// THEMES
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnGetTheme(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return (LRESULT)buze_application_get_theme_color(application, (const char*)lParam);
}

LRESULT CMainFrame::OnSetTheme(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int themeIndex = wID - ID_SET_THEME_FIRST;
	std::string themeName = buze_application_get_theme_name(application, themeIndex);
	setTheme(themeName);
	return 0;
}

void CMainFrame::initializeThemes() {
	CMenuHandle mainMenu = m_CmdBar.GetMenu();
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU;
	char pc[1024];

	if (themeMenu.m_hMenu) themeMenu.DestroyMenu();

	themeMenu.CreatePopupMenu();

	for (int i = 0; i < buze_application_get_theme_count(application); i++) {
		std::string name = buze_application_get_theme_name(application, i);
		themeMenu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, ID_SET_THEME_FIRST + i, name.c_str());
	}

	for (int i = 0; i < viewMenu.GetMenuItemCount(); i++) {
		UINT menuID = viewMenu.GetMenuItemID(i);
		viewMenu.GetMenuString(menuID, pc, 1024, 0);
		if (std::string(pc) == "Themes") {
			mii.hSubMenu = themeMenu.m_hMenu;
			viewMenu.SetMenuItemInfo(i, TRUE, &mii);
			break;
		}
	}

	std::string themeName = configuration->getTheme();
	setTheme(themeName);
}

void CMainFrame::setTheme(std::string const& themeName) {
	buze_application_load_theme(application, themeName.c_str());
	buze_document_notify_views(document, 0, buze_event_type_update_theme, 0);

	char menuName[1024];
	for (int i = 0; i < themeMenu.GetMenuItemCount(); i++) {
		themeMenu.GetMenuString(i, menuName, 1024, MF_BYPOSITION);
		MENUITEMINFO mi;
		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_STATE;
		mi.fState = (themeName == menuName) ? MFS_CHECKED : 0;
		themeMenu.SetMenuItemInfo(i, TRUE, &mi);
	}

	configuration->setTheme(themeName);
}

// ---------------------------------------------------------------------------------------------------------------
// PLAYER TRANSPORT
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnPlay(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_set_state(player, zzub_player_state_playing, -1);
	elapsedTimeReset();

	return 1;
}

LRESULT CMainFrame::OnPlayRepeat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_set_order_loop_enabled(player, zzub_player_get_order_loop_enabled(player) ^ 1);
	zzub_player_history_commit(player, 0, 0, "Toggle Song Looping");
	return 1;
}

LRESULT CMainFrame::OnPlaySync(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_set_midi_transport(player, zzub_player_get_midi_transport(player) ^ 1);
	return 1;
}

LRESULT CMainFrame::OnRecord(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_set_automation(player, zzub_player_get_automation(player) ^ 1);
	return 1;
}

LRESULT CMainFrame::OnPlayFromStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	zzub_player_set_position(player, 0, 0);
	if (zzub_player_get_state(player) != zzub_player_state_playing)
		zzub_player_set_state(player, zzub_player_state_playing, -1);

	elapsedTimeReset();

	return 1;
}

LRESULT CMainFrame::OnPlayFromCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
/*	int orderindex = zzub_player_get_position_order(player);
	zzub_pattern_t* orderpat = zzub_player_get_order_pattern(player, orderindex);
	if (!orderpat) return 0;

	int loopstart = zzub_pattern_get_loop_start(orderpat);
	*/
	int orderindex = buze_document_get_current_order_index(document);
	zzub_pattern_t* orderpat = zzub_player_get_order_pattern(player, orderindex);
	if (!orderpat) return 0;

	int loopstart = buze_document_get_current_order_pattern_row(document);

	zzub_player_set_position(player, orderindex, loopstart);
	if (zzub_player_get_state(player) != zzub_player_state_playing) {
		zzub_player_set_state(player, zzub_player_state_playing, -1);
	}
	elapsedTimeReset();

	return 1;
}

LRESULT CMainFrame::OnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_player_set_state(player, zzub_player_state_stopped, -1);
	zzub_player_set_automation(player, false);
	elapsedTimeReset();
	return 1;
}

LRESULT CMainFrame::OnResetDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_audiodriver_t* driver = buze_application_get_audio_driver(application);
	if (driver == 0) return 0;

	if (!zzub_audiodriver_get_enabled(driver)) {
		zzub_audiodriver_enable(driver, 1);
	} else {
		zzub_audiodriver_enable(driver, 0);
	}
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// ELAPSED TIME READOUT
// ---------------------------------------------------------------------------------------------------------------

void CMainFrame::elapsedTimeReset() {
	QueryPerformanceCounter((LARGE_INTEGER*)&qpfStart);
}

double CMainFrame::elapsedTimeGet() {
	signed __int64 frame;
	QueryPerformanceCounter((LARGE_INTEGER*)&frame);
	return (static_cast<double>(frame) - static_cast<double>(qpfStart)) / static_cast<double>(qpfFreq);
}

// ---------------------------------------------------------------------------------------------------------------
// XXX
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMainFrame::OnKeyjazzOctaveUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	int octave = buze_document_get_octave(document);
	if (octave < 8)
		buze_document_set_octave(document, octave + 1);
	return 0;
}

LRESULT CMainFrame::OnKeyjazzOctaveDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	int octave = buze_document_get_octave(document);
	if (octave > 1)
		buze_document_set_octave(document, octave - 1);
	return 0;	
}

void CMainFrame::UpdateSettings() {
	masterToolbar.UpdateSettings();

	{	bool show_accels = configuration->getShowAccelerators();

		if (show_accels != menu_has_accelerators) {
			if (show_accels) {
				hotkeys.UpdateMenuKeys("mainframe", m_CmdBar.m_hMenu, true);
			} else {
				hotkeys.UpdateMenuKeys("mainframe", m_CmdBar.m_hMenu, false);
			}

			menu_has_accelerators = show_accels;
		}
	}
}

// Used by buzz2zzub
LRESULT CMainFrame::OnForwardPreTranslate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	BOOL res = PreTranslateMessage((MSG*)lParam);
	return res != FALSE ? 0 : -1;
}

const char* CMainFrame::GetProgramName(){
	return programName;
}

// ---------------------------------------------------------------------------------------------------------------
// OBSOLETE / UNUSED
// ---------------------------------------------------------------------------------------------------------------

// from http://www.winehq.com/hypermail/wine-patches/2002/12/0179.html
// inline short u8tos16(unsigned char b) {	// wavs are unsigned
// 	return (short)((b+(b << 8))-32768);
// }
// 
// inline short s8tos16(char b) {	// mods are singed
// 	return ((short)b) * 256;
// }

// int noteFromMidiNote(int twelve) {
// 	//twelve--;
// 	int note=(twelve%12)+1;
// 	int oct=twelve/12;
// 	return (note) + (oct<<4);
// }
