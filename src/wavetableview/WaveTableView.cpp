#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "utils.h"
#include "Keymaps.h"
#include "Configuration.h"
#include "WaveTableView.h"
#include "BuzeConfiguration.h"

using namespace std;

CHostDllModule _Module;

// quick forwards:
void ParseFileName(const std::string& fullname, std::string* filename, int* instrument, int* sample);
bool LoadSampleByFileName(zzub_player_t* player, const char *szFileName, int instrument, int sample, int curWave);

// 
// Factory
//

CWavetableViewInfo::CWavetableViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CWaveTableView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Wavetable";
	tooltip = "Machine view";
	place = 1; //DockSplitTab::placeMAINPANE;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = true;
	allowfloat = false;
	defaultview = true;
}

CView* CWavetableViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CWaveTableView* view = new CWaveTableView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CWavetableViewInfo::Attach() {
	buze_document_add_view(document, this);

	//buze_main_frame_register_accelerator(mainframe, "wavetable", "help", ID_HELP);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "edit_copy", "c ctrl", ID_EDIT_COPY);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "edit_cut", "x ctrl", ID_EDIT_CUT);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "edit_paste", "v ctrl", ID_EDIT_PASTE);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "edit_selectall", "a ctrl", ID_EDIT_SELECTALL);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "edit_delete", "delete", ID_EDIT_DELETE);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "wave_zoom_in", "", ID_WAVE_ZOOM_IN);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "wave_zoom_out", "", ID_WAVE_ZOOM_OUT);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "wave_zoom_selection", "", ID_WAVE_ZOOM_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "wave_zoom_all", "", ID_WAVE_ZOOM_ALL);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "wave_properties", "", ID_VIEW_PROPERTIES);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "wave_clear", "", ID_WAVE_CLEARWAVE);
	buze_main_frame_register_accelerator(mainframe, "wavetable", "wave_trim", "", ID_WAVE_TRIM);

	WORD ID_SHOW_WAVETABLE = buze_main_frame_register_accelerator_event(mainframe, "view_wavetable", "f9", buze_event_type_show_wavetable_view);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_WAVETABLE, "Wavetable");
}

void CWavetableViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CWavetableViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CWaveTableView* view;
	switch (lHint) {
		case buze_event_type_show_wavetable_view:
			view = (CWaveTableView*)buze_main_frame_get_view(mainframe, "WaveTableView", 0);
			if (view) {
				buze_main_frame_set_focus_to(mainframe, view);
			} else
				buze_main_frame_open_view(mainframe, "WaveTableView", "Wavetable", 0, -1, -1);
			break;
	}
}

class CWavetableViewLibrary : public CViewLibrary {
public:
	virtual void Initialize(CViewFrame* host) {
		_Module.m_hostModule = buze_application_get_host_module(buze_main_frame_get_application(host));
		buze_main_frame_register_window_factory(host, new CWavetableViewInfo(host));
	}

	virtual void Destroy() {
		delete this;
	}

	virtual int GetVersion() {
		return CViewLibrary::version;
	}

};

extern "C" CViewLibrary* buze_create_viewlibrary() {
	return new CWavetableViewLibrary();
}

//
// View
//

enum EDITFLAGS {
	EDIT_COPY = 1,	// set if selection can be copied
	EDIT_PASTE = 2,	// set if clipboard format is recognized
};


/***

	CBgTabViewCtrl

***/


LRESULT CBgTabViewCtrl::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle bgDC((HDC)wParam);
	RECT rcClient;
	GetClientRect(&rcClient);
	bgDC.FillRect(&rcClient, COLOR_3DFACE);
	return DefWindowProc();
}

/***

	CWaveTableView

***/

CWaveTableView::CWaveTableView(CViewFrame* mainFrm)
	:CViewImpl(mainFrm)
	,waveList(this)
	,waveLevelList(this)
	,waveEditorCtrl()
	,envelopeTab(this)
	,editTab(this)
	,sliceTab(this)
	,effectsTab(this, buze_main_frame_get_player(mainFrm))
{
	dirtyWavelevels = false;
	dirtyWavelist = false;
	dirtyStatus = false;
	dirtyWaveEditorReset = false;
	dirtyWaveEditor = false;
	dirtyEditTab = false;
	dirtyEnvelope = false;
	dirtySlices = false;
	currentWave = 0;
	currentWavelevel = 0;
	currentWaveIndex = -1;
}

CWaveTableView::~CWaveTableView(void) {
}

//
// Creation / destruction

LRESULT CWaveTableView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	DefWindowProc();

	SIZE size;

	hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_WAVETABLE, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	SendMessage(hWndToolBar, TB_GETMAXSIZE, 0, (LPARAM)&size);

	hWndEditToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_WAVE_EDIT, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	SendMessage(hWndEditToolBar, TB_GETMAXSIZE, 0, (LPARAM)&size);

	mainSplitter.Create(*this, rcDefault, NULL, 0, 0);
	mainSplitter.SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	waveLevelSplitter.Create(mainSplitter, rcDefault, NULL, 0, 0);
	waveLevelSplitter.SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
	waveTabSplitter.Create(mainSplitter, rcDefault, NULL, 0, 0);
	waveTabSplitter.SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);

 	waveList.Create(waveLevelSplitter, rcDefault, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | LVS_SINGLESEL | LVS_NOCOLUMNHEADER | LVS_REPORT | LVS_SHOWSELALWAYS, 0, IDC_WAVELIST);
	waveList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
	waveList.AddColumn("Instrument", 0);

	waveLevelList.Create(waveLevelSplitter, rcDefault, 0, WS_CHILD|WS_CLIPSIBLINGS |WS_CLIPCHILDREN | WS_VSCROLL | LVS_SINGLESEL | LVS_REPORT | LVS_SHOWSELALWAYS, 0, IDC_WAVELEVELLIST);
	waveLevelList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);

	waveEditorCtrl.Create(waveTabSplitter, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE, IDC_WAVEEDITOR);

	waveTabs.Create(waveTabSplitter, rcDefault, 0, WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN |TCS_TABS, 0, IDC_WAVETABS);

	editTab.Create(waveTabs, rcDefault, 0, WS_CHILD);
	waveTabs.AddTab("Edit", editTab);

	envelopeTab.Create(waveTabs, rcDefault, 0, WS_CHILD);
	waveTabs.AddTab("Envelopes", envelopeTab);

	sliceTab.Create(waveTabs, rcDefault, 0, WS_CHILD);
	waveTabs.AddTab("Slices", sliceTab);

	effectsTab.Create(waveTabs, rcDefault, 0, WS_CHILD);
	waveTabs.AddTab("Effects", effectsTab);

	UpdateToolbar();

	int statusWidths[] = { 200, 400, -1 };
	statusBar.Create(m_hWnd, rcDefault, 0, WS_VISIBLE|WS_CHILD);
	statusBar.SetMinHeight(18);
	statusBar.SetParts(3, statusWidths);

	BindStatus();

	// add the toolbar bands as late as possible since they will invoke a WM_SIZE on us
	CBuzeConfiguration configuration = buze_document_get_configuration(document);
	bool bLock = configuration->getLockedToolbars();
	insertToolbarBand(hWndToolBar, "", size.cx, -1, true, bLock);
	insertToolbarBand(hWndEditToolBar, "", size.cx, -1, true, bLock);

	buze_document_add_view(document, this);

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddIdleHandler(this);
	pLoop->AddMessageFilter(this);

	buze_main_frame_viewstack_insert(mainframe, this); // false

	PostMessage(WM_POSTCREATE);

	return 0;
}

LRESULT CWaveTableView::OnPostCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	RECT rcClient;
	GetClientRect(&rcClient);

	mainSplitter.SetSplitterPanes(waveLevelSplitter, waveTabSplitter);
	waveTabSplitter.SetSplitterPanes(waveEditorCtrl, waveTabs);
	waveLevelSplitter.SetSplitterPanes(waveList, waveLevelList);

	if (rcClient.right > 0 && rcClient.bottom > 0) {
		mainSplitter.SetSplitterPos(200);
		waveLevelSplitter.SetSplitterPos(rcClient.bottom - 200); 
		waveTabSplitter.SetSplitterPos(rcClient.bottom - 200);
	}

	waveTabs.ModifyStyle(0, WS_VISIBLE);
	waveLevelList.ModifyStyle(0, WS_VISIBLE);
	waveList.ModifyStyle(0, WS_VISIBLE);
	waveEditorCtrl.ModifyStyle(0, WS_VISIBLE);

	// redraw tab control after enabling WS_VISIBLE
	waveTabs.SetActiveTab(0);
	waveTabs.Invalidate();

	waveList.SetRedraw(FALSE);
	int wavecount = zzub_player_get_wave_count(player);
	for (int i = 0; i < wavecount; i++)
		waveList.InsertItem(i, "");
	waveList.SetRedraw(TRUE);

	BindWaveInfos();
	BindWaveList();

	// SelectItem causes OnSelectItem to happen
	waveList.SelectItem(0);

	return 0;
}

LRESULT CWaveTableView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveIdleHandler(this);
	pLoop->RemoveMessageFilter(this);

	buze_document_remove_view(document, this);
	//mainframe->closeClientWindow(m_hWnd);

	return 0;
}

LRESULT CWaveTableView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	HIMAGELIST hImageList = (HIMAGELIST)::SendMessage(hWndToolBar, TB_SETIMAGELIST, 0, (LPARAM)0);
	ImageList_Destroy(hImageList);

	hImageList = (HIMAGELIST)::SendMessage(hWndEditToolBar, TB_SETIMAGELIST, 0, (LPARAM)0);
	ImageList_Destroy(hImageList);

	return DefWindowProc();
}

void CWaveTableView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

BOOL CWaveTableView::OnIdle() {
	return FALSE;
}

BOOL CWaveTableView::PreTranslateMessage(MSG* pMsg) {

	if (GetFocus() == m_hWnd || IsChild(GetFocus())) {
		HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "wavetable");
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg))
			return TRUE;
	}
	return FALSE;
}

//
// Sizing / focus
//

LRESULT CWaveTableView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD( lParam);
	WORD cy = HIWORD( lParam);

	RECT rc;
	GetClientRect(&rc);

	RECT rcStatus;
	statusBar.GetClientRect(&rcStatus);
	int statusHeight = rcStatus.bottom - rcStatus.top;

	mainSplitter.MoveWindow(0, getToolbarHeight(), rc.right, rc.bottom - getToolbarHeight() - statusHeight); 
	statusBar.MoveWindow(0, 0, 0, 0);

	int ms = mainSplitter.GetSplitterPos();
	int ws = waveLevelSplitter.GetSplitterPos();
	int ts = waveTabSplitter.GetSplitterPos();

	if (rc.right > 0 && rc.bottom > 0 && ms == 4 && ws == 4 && ts == 4) {
		mainSplitter.SetSplitterPos(200);
		waveLevelSplitter.SetSplitterPos(rc.bottom - 200); 
		waveTabSplitter.SetSplitterPos(rc.bottom - 200);

		int selwave = waveList.GetSelectedIndex();
		if (selwave != -1)
			waveList.EnsureVisible(selwave, FALSE);
	}

	return 0;
}

LRESULT CWaveTableView::OnFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	waveList.SetFocus();
	return 0;
}

LRESULT CWaveTableView::OnBlur(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	buze_document_keyjazz_release(document, true);
	zzub_player_reset_keyjazz(player);	// reset midi keyjazz
	return 0;
}

//
// Controls binding using WM_PAINT
//

LRESULT CWaveTableView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if (dirtyWavelevels) {
		BindWaveLevels();
		dirtyWavelevels = false;
	}

	if (dirtyWavelist) {
		BindWaveList();
		BindCurrentWave(); // after song-open, this updates document_get_current_wave()
		dirtyWavelist = false;
	}

	if (dirtyStatus) {
		BindStatus();
		dirtyStatus = false;
	}

	if (dirtyWaveEditorReset) {
		UpdateWaveEditor(true);
		dirtyWaveEditorReset = false;
		dirtyWaveEditor = false;
	} else
	if (dirtyWaveEditor) {
		UpdateWaveEditor(false);
		dirtyWaveEditorReset = false;
		dirtyWaveEditor = false;
	}
	if (dirtyEditTab) {
		editTab.UpdateFromWavetable();
		dirtyEditTab = false;
	}
	if (dirtyEnvelope) {
		envelopeTab.UpdateEnvelope();
		envelopeTab.UpdateMachines();
		dirtyEnvelope = false;
	}
	if (dirtySlices) {
		sliceTab.SetWaveLevel(currentWavelevel);
		dirtySlices = false;
	}
	return DefWindowProc();
}

LRESULT CWaveTableView::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 1;
}

//
// Keyboard / keyjazz
// 

LRESULT CWaveTableView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// lParam bit 30 : Specifies the previous key state. The value is 1 if the key is down before the message is sent, or it is zero if the key is up.

	if ((lParam & (1 << 30)) != 0) return 0;	// repeat >1
	if ((lParam & (1 << 24)) != 0) return 0;	// extended key

	if (wParam == VK_RETURN) {
		buze_document_notify_views(document, this, buze_event_type_show_filebrowser, 0);
		return 0;
	}

	int note = keyboard_mapper::map_code_to_note(buze_document_get_octave(document), (int)wParam);
	if (note >= 0 && note != 255 && note != 254) note = midi_to_buzz_note(note);
	if (note == -1) return 0;

	int num_waves = zzub_player_get_wave_count(player);
	int index = waveList.GetSelectedIndex();
	if (index >= num_waves) return 0;
	if (index < 0) return 0;
	zzub_wave_t* wave = zzub_player_get_wave(player, index);
	int id = zzub_wave_get_id(wave);

	long beginSelect = min(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);
	long endSelect = max(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);
	if (endSelect - beginSelect == 0)
		beginSelect = waveEditorCtrl.cursorSamplePosition;

	buze_document_play_stream(document, note, beginSelect, endSelect - beginSelect, "@zzub.org/stream/wavetable;1", stringFromInt(id).c_str());
	zzub_plugin_t* streamplayer = (zzub_plugin_t*)buze_document_get_stream_plugin(document);
	if (streamplayer == 0) return 0;
	buze_document_keyjazz_key_down(document, streamplayer, (int)wParam, note);
	return 0;
}

LRESULT CWaveTableView::OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int lastnote;
	zzub_plugin_t* lastplugin = 0;

	buze_document_keyjazz_key_up(document, (int)wParam, &lastnote, &lastplugin);
	if (lastplugin == 0) return 0;

	buze_document_play_plugin_note(document, lastplugin, zzub_note_value_off, lastnote);
	return 0;
}

//
// Wave list binding
//

void CWaveTableView::BindWaveList() {
	//int sel = waveList.GetSelectedIndex();
	POINT zeroPt = { 0, 0 };
	int first = waveList.HitTest(zeroPt, 0);

	waveList.SetRedraw(FALSE);

	int wavecount = zzub_player_get_wave_count(player);
	for (int i = 0; i < wavecount; i++) {
		BindWaveListWave(zzub_player_get_wave(player, i));
	}

	int first2 = waveList.HitTest(zeroPt, 0);

	RECT rcItem;
	waveList.GetSubItemRect(0, 0, LVIR_BOUNDS, &rcItem);
	int itemHeight = rcItem.bottom - rcItem.top;
	SIZE scrollSize = { 0, (first - first2) * itemHeight };
	waveList.Scroll(scrollSize);

	waveList.SetRedraw(TRUE);
}

void CWaveTableView::BindWaveListWave(zzub_wave_t* wave) {
	char pc[256];
	int index = zzub_wave_get_index(wave);
	sprintf(pc, "%02X %s", index+1, zzub_wave_get_name(wave));
	waveList.SetItemText(index, 0, pc);
}

//
// Wave list events
//

LRESULT CWaveTableView::OnWaveListDoubleClicked(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	if (zzub_wave_get_level_count(currentWave) == 0) return 0;

	int id = zzub_wave_get_id(currentWave);
	buze_document_play_stream(document, zzub_note_value_c4, 0, 0, "@zzub.org/stream/wavetable;1", stringFromInt(id).c_str());
	return 0;
}

LRESULT CWaveTableView::OnWaveListSelChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLISTVIEW pnmhlv = (LPNMLISTVIEW)pnmh;
	if ((int)pnmh == -1) return 0;	// applies to all items, we arent interested
	if (pnmhlv->uChanged & LVIF_TEXT) return 0;	// dont update on rename

	int sel = waveList.GetSelectedIndex();
	if (sel == -1 || sel >= zzub_player_get_wave_count(player)) return 0;
	if (sel == currentWaveIndex) return 0; // ignore deselect state change
	currentWave = zzub_player_get_wave(player, sel);
	currentWavelevel = 0;

	int levelsel = waveLevelList.GetSelectedIndex();

	// save current waveinfo
	CWaveInfo* currentInfo = GetCurrentWaveInfo();
	if (currentInfo && waveEditorCtrl.numDisplaySamples > 0) {
		currentInfo->level = levelsel;
		currentInfo->sel_from = waveEditorCtrl.beginSelectSample;
		currentInfo->sel_length = waveEditorCtrl.endSelectSample - waveEditorCtrl.beginSelectSample;
		currentInfo->display_from = waveEditorCtrl.beginDisplaySample;
		currentInfo->display_length = waveEditorCtrl.numDisplaySamples;
	}

	BindCurrentWave();

	currentWaveIndex = sel;

	dirtyWavelevels = true;
	dirtyWaveEditorReset = true;
	dirtyEditTab = true;
	dirtyEnvelope = true;
	dirtySlices = true;

	waveEditorCtrl.SetEditWave(0, true);
	Invalidate(FALSE);
	return 0;
}

//
// Wavelevel list binding
// 

int bits_from_format(int format) {
	switch (format) {
		case zzub_wave_buffer_type_si16:
			return 16;
		case zzub_wave_buffer_type_si24:
			return 24;
		case zzub_wave_buffer_type_f32:
		case zzub_wave_buffer_type_si32:
			return 32;
		default:
			return 0;
	}
}

void CWaveTableView::BindWaveLevels() {
	//int prevLevel = waveLevelList.GetSelectedIndex();
	if (!currentWave) return ;

	waveLevelList.DeleteAllItems();
	for (int i = 0; i < zzub_wave_get_level_count(currentWave); i++) {
		zzub_wavelevel_t* level = zzub_wave_get_level(currentWave, i);
		string note = noteFromInt(zzub_wavelevel_get_root_note(level));
		string len = stringFromInt(zzub_wavelevel_get_sample_count(level), 0);
		string rate = stringFromInt(zzub_wavelevel_get_samples_per_second(level), 0);
		string lbeg = stringFromInt(zzub_wavelevel_get_loop_start(level), 0);
		string lend = stringFromInt(zzub_wavelevel_get_loop_end(level), 0);
		string bits = stringFromInt(bits_from_format(zzub_wavelevel_get_format(level)), 0);// entry->get_bits_per_sample(i), 0);

		waveLevelList.InsertItem(i, note.c_str());
		waveLevelList.SetItem(i, 1, LVIF_TEXT, len.c_str(), -1, 0, 0, 0);
		waveLevelList.SetItem(i, 2, LVIF_TEXT, rate.c_str(), -1, 0, 0, 0);
		waveLevelList.SetItem(i, 3, LVIF_TEXT, lbeg.c_str(), -1, 0, 0, 0);
		waveLevelList.SetItem(i, 4, LVIF_TEXT, lend.c_str(), -1, 0, 0, 0);
		waveLevelList.SetItem(i, 5, LVIF_TEXT, bits.c_str(), -1, 0, 0, 0);
	}

	CWaveInfo* currentInfo = GetCurrentWaveInfo();
	int prevLevel = currentInfo->level;

	if (prevLevel >= 0 && prevLevel < waveLevelList.GetItemCount())
		waveLevelList.SelectItem(prevLevel); else
		waveLevelList.SelectItem(0);
	
}

// 
// Wavelevel list events
//

LRESULT CWaveTableView::OnWaveLevelListSelChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	// NOTE: setting looping points and similar operations on forces a re-selection of the wave level, which in turn 
	// invokes this redraw, therefore we dont redraw the entry if we already
	zzub_wavelevel_t* newLevel = 0;
	int sel = waveLevelList.GetSelectedIndex();
	if (sel != -1)
		newLevel = zzub_wave_get_level(currentWave, sel);

	if (currentWavelevel != newLevel) {
		if (newLevel != 0) {
			// update current waveinfo with the new level
			CWaveInfo* currentInfo = GetCurrentWaveInfo();
			if (currentInfo->level != sel) {
				currentInfo->level = sel;
				currentInfo->sel_from = -1;
				currentInfo->sel_length = 0;
				currentInfo->display_from = 0;
				currentInfo->display_length = zzub_wavelevel_get_sample_count(newLevel);//waveEditorCtrl.numDisplaySamples;
				currentInfo->length = currentInfo->display_length;
			}
			//buze_document_notify_views(document, this, buze_event_type_update_select_wavelevel, newLevel);
			buze_event_data_t ev;
			ev.show_properties.return_view = this;
			ev.show_properties.type = buze_property_type_wave_level;
			ev.show_properties.wavelevel = newLevel;
			buze_document_notify_views(document, this, buze_event_type_update_properties, &ev);
		}

		currentWavelevel = newLevel;
		dirtyEnvelope = true;
		dirtyWaveEditorReset = true;
		dirtySlices = true;
		Invalidate(FALSE);
	}
	return 0;
}

LRESULT CWaveTableView::OnWaveLevelAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (currentWave == 0) return 0;

	int next_level = zzub_wave_get_level_count(currentWave);
	zzub_wave_add_level(currentWave);
	zzub_player_history_commit(player, 0, 0, "Insert Wavelevel");
	return 0;
}

LRESULT CWaveTableView::OnWaveLevelDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (currentWave == 0) return 0;

	int next_level = zzub_wave_get_level_count(currentWave);
	zzub_wave_remove_level(currentWave, next_level - 1);
	zzub_player_history_commit(player, 0, 0, "Delete Wavelevel");
	return 0;
}

//
// Wave load/save/clear (toolbar buttons)
//

LRESULT CWaveTableView::OnLoadWave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	printf("load wave\n");
	std::string currentDir = buze_document_get_current_path(document);

	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	char szCurDir[260];
	strcpy(szCurDir, currentDir.c_str());

    // open a file name
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = "All waves (*.wav)\0*.wav\0mp3 files (*.mp3)\0*.mp3\0All files\0*.*\0\0";
	// All waves (*.wav)\0*.wav\0mp3 files (*.mp3)\0*.mp3\0All files\0*.*\0\0"
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	if (strlen(szCurDir) > 0)
    	ofn.lpstrInitialDir = szCurDir;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
	
	// Display the Open dialog box. 
	if (GetOpenFileName(&ofn)!=TRUE) return 1;

	buze_application_t *a = buze_main_frame_get_application(mainframe);
	buze_application_show_wait_window(a);
	buze_application_set_wait_text(a, "Loading sample..");	

	// load file if succesfull
	bool result = buze_document_import_wave(document, ofn.lpstrFile, currentWave) != -1;
	buze_application_hide_wait_window(a,this);

	if (!result)
		MessageBox("Cannot load sample(s)", "Error");

	return 0;
}

LRESULT CWaveTableView::OnSaveWave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (currentWavelevel == 0) return 0;

	std::string currentFile = buze_document_get_current_filename(document);
	std::string currentDir = buze_document_get_current_path(document);

	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
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
	ofn.lpstrFilter = "All waves (*.wav)\0*.wav\0All files\0*.*\0\0";
	ofn.lpstrDefExt = "wav";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	if (strlen(szCurDir) > 0)
		ofn.lpstrInitialDir = szCurDir;

	ofn.Flags = OFN_NOCHANGEDIR |OFN_OVERWRITEPROMPT|OFN_EXTENSIONDIFFERENT|OFN_NOREADONLYRETURN;

	// Display the Open dialog box. 
	if (GetSaveFileName(&ofn)!=TRUE) return 1;

	zzub_output_t* outf = zzub_output_create_file(szFile);
	zzub_wavelevel_save_wav(currentWavelevel, outf);
	zzub_output_destroy(outf);
	return 0;
}

LRESULT CWaveTableView::OnClearWave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (currentWave == 0) return 0;
	zzub_wave_clear(currentWave);
	zzub_player_history_commit(player, 0, 0, "Clear Wave");
	return 0;
}

//
// Show properties for waves and wavelevels (via contextmenu or accelerator)
//

LRESULT CWaveTableView::OnViewProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	assert(currentWave != 0);
	buze_event_data_t ev;
	ev.show_properties.type = buze_property_type_wave;
	ev.show_properties.wave = currentWave;
	ev.show_properties.return_view = this;
	buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
	return 0;
}

LRESULT CWaveTableView::OnViewLevelProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	assert(currentWavelevel != 0);
	buze_event_data_t ev;
	ev.show_properties.type = buze_property_type_wave_level;
	ev.show_properties.wavelevel = currentWavelevel;
	ev.show_properties.return_view = this;
	buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
	return 0;
}

// 
// External wave editor, via context menu
//

LRESULT CWaveTableView::OnWaveXEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int wavetableIndex = zzub_wave_get_index(currentWave);

	// Get global settings

	CBuzeConfiguration configuration = buze_document_get_configuration(document);
	std::string waveEditor = configuration->getExternalWaveEditor();

	if (!waveEditor.length()) {
		MessageBox("No wave editor specified", "Error");
		return 0;
	}

	char tmpPath[MAX_PATH];
	GetTempPath(MAX_PATH, tmpPath);

	std::string tempFile = tmpPath + stringFromInt(wavetableIndex) + ".wav";

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	memset(&si,0,sizeof(si));
	si.cb= sizeof(si);

	// Export document->selectedWave as tempFile

	zzub_output_t* outf = zzub_output_create_file(tempFile.c_str());
	if (!outf || zzub_wavelevel_save_wav(currentWavelevel, outf))
	{
		zzub_output_destroy(outf);
		MessageBox("Unable to create temporary file", tempFile.c_str());
		return 0;
	}
	zzub_output_destroy(outf);

	// Create a process for the wave editor, editing the temporary file

	std::string command = "\"" + waveEditor + "\" " + tempFile;
	char commandStr[1024];
	strncpy(commandStr, command.c_str(), 1024);
	if(!CreateProcess(NULL, commandStr, NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
		MessageBox("Unable to spawn the external wave editor.", command.c_str());
	}

	return 0;
}

LRESULT CWaveTableView::OnWaveIEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int wavetableIndex = zzub_wave_get_index(currentWave);

	// Get global settings

	char tmpPath[MAX_PATH];
	GetTempPath(MAX_PATH, tmpPath);

	std::string tempFile = tmpPath + stringFromInt(wavetableIndex) + ".wav";

	// Re-Import tmpFileName into document->selectedWave

	std::string originalFileName = zzub_wave_get_path(currentWave);
	std::string originalName = zzub_wave_get_name(currentWave);

	zzub_input_t* inf = zzub_input_open_file(tempFile.c_str());
	if (!inf)
		return 0;

	int result = zzub_wavelevel_load_wav(currentWavelevel, 0, 1, inf);
	zzub_input_destroy(inf);

	if (!result) {
		MessageBox("Unable to import the temporary wave file", tempFile.c_str());
		return 0;
	}

	zzub_wave_set_path(currentWave, originalFileName.c_str());
	zzub_wave_set_name(currentWave, originalName.c_str());

	zzub_player_history_commit(player, 0, 0, "Re-Load Externally Edited Wave");

	// This UI refresh is required as the names were reinstated

	if(remove(tempFile.c_str()))
	{
		MessageBox("Unable to delete the temporary wave file", tempFile.c_str());
	}
	return 0;
}

//
// Execute wave script (via wave context menu)
//

LRESULT CWaveTableView::OnWaveScript(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	int index = wID - ID_WAVE_SCRIPTCOMMANDS;
	std::string script = waveList.wscripts.at(index);
	int wavetableIndex = zzub_wave_get_index(currentWave);
	char tmpPath[MAX_PATH];
	GetTempPath(MAX_PATH, tmpPath);
	std::string tempFile;
 
	std::string wavefile = stringFromInt(wavetableIndex);

	if (currentWavelevel == 0) return MessageBox("Impossible to run script on empty sample","oops..");
	// *FIXME* how to check if wave exist? zzub_wave_t* zzub_wavelevel_get_wave(zzub_wavelevel_t* level) <- where to get level? or is there a better function?
	tempFile = tmpPath + wavefile + ".wav";
	// Export document->selectedWave as tempFile
	zzub_output_t* outf = zzub_output_create_file(tempFile.c_str());
	if (!outf || zzub_wavelevel_save_wav(currentWavelevel, outf))
	{
		zzub_output_destroy(outf);
		MessageBox("Unable to create temporary file", tempFile.c_str());
		return 0;
	}
	zzub_output_destroy(outf);
	ExecuteWaveScript( script, tempFile );
	return 0;
}

void CWaveTableView::ExecuteWaveScript( std::string script, std::string tempFile ){
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset(&si,0,sizeof(si));
	si.cb= sizeof(si);
	// Create a process for the script, 
	// its nonblocking (because what is more important..an external script or a stable buze..I think the latter)
	std::string command;
	char abspath[1024];
	GetCurrentDirectory(1024, abspath);
	//_getcwd( (char *)&abspath, 1024 );
	if( script.find( ".bat"  ) != string::npos ) command.append("cmd.exe \"/c\" ");
	if( script.find( ".py"   ) != string::npos ) command.append("python.exe ");
	if( script.find( ".js"   ) != string::npos ) command.append("wscript.exe ");
	if( script.find( ".hta"  ) != string::npos ) command.append("mshta.exe ");
	if( script.find( ".armz" ) != string::npos ) command.append("armz2wav.exe ");
	command.append("\"");
	command.append(abspath);
	command.append( "\\Gear\\Scripts\\WaveTable\\" );
	command.append( script );
	command.append( "\" \"" + tempFile );
	printf("running: %s\n",command.c_str() );
	char commandStr[1024];
	strncpy(commandStr, command.c_str(), 1024);
	if(!CreateProcess(NULL, commandStr, NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
		MessageBox("Unable to spawn script", command.c_str());
	}
}

// 
// Wave editor binding
//

void CWaveTableView::GetSamplesDigest(int channel, int start, int end, std::vector<float>& mindigest, std::vector<float>& maxdigest, std::vector<float>& ampdigest, int digestsize) {
	// NOTE: the following assert and if-test was added to notify but not fail miserably - happens when the wave editor control requests wave data at inappropriate times. should probably be fixed somewhere
	// NOTE: its fixed: the waveeditor is cleared on zzub_event_delete_wavelevel
	assert(currentWavelevel != 0);
	if (currentWavelevel != 0) {
		mindigest.resize(digestsize);
		maxdigest.resize(digestsize);
		ampdigest.resize(digestsize);
		if (digestsize > 0) {
			zzub_wavelevel_get_samples_digest(currentWavelevel, channel, start, end, &mindigest.front(), &maxdigest.front(), &ampdigest.front(), digestsize);
		}
	}
}

void CWaveTableView::BindStatus() {
	
	float num_samples = 0;
	
	if (currentWavelevel != 0)
		num_samples = zzub_wavelevel_get_sample_count(currentWavelevel);

	WaveEditorGridMode gridType = waveEditorCtrl.gridType;
	float samples_per_unit = waveEditorCtrl.GetSamplesPerUnit(gridType);

	std::stringstream pos_strm;
	std::stringstream select_strm;

	std::string unit_name;
	switch (gridType) {
		case WaveEditorSamples:
			unit_name = "samples";
			break;
		case WaveEditorSeconds:
			unit_name = "sec";
			break;
		case WaveEditorTicks:
			unit_name = "ticks";
			break;
		case WaveEditorWord:
			unit_name = "hex";
			break;
		default:
			unit_name = "??";
			break;
	}

	if (waveEditorCtrl.beginSelectSample != -1) {
		float min_select = (float)std::min(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);
		float max_select = (float)std::max(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);

		float select_start_unit = min_select / samples_per_unit;
		float select_end_unit = max_select / samples_per_unit;

		std::stringstream selectStrm;
		select_strm << "Select " << waveEditorCtrl.FormatSampleUnit(select_start_unit, gridType) << " to " << waveEditorCtrl.FormatSampleUnit(select_end_unit, gridType) << " (" << unit_name << ")";
	}
	
	if (num_samples != 0) {
		int sample_unit = (float)waveEditorCtrl.mouseSamplePosition / samples_per_unit;
		std::string sample_unit_string = waveEditorCtrl.FormatSampleUnit(sample_unit, gridType);

		pos_strm << "Pos: " << sample_unit_string << " (" << unit_name << ")";
	}

	statusBar.SetText(0, pos_strm.str().c_str());
	statusBar.SetText(1, select_strm.str().c_str());
}

void CWaveTableView::UpdateWaveEditor(bool reset) {
	if (currentWavelevel == 0) {
		waveEditorCtrl.SetEditWave(0, true);
		return ;
	}

	if (currentWave == 0) {
		waveEditorCtrl.SetEditWave(0, true);
		return ;
	}

	CWaveInfo* currentInfo = GetCurrentWaveInfo();

	EDITWAVEINFO ewi;
	ewi.endLoop = zzub_wavelevel_get_loop_end(currentWavelevel);
	ewi.beginLoop = zzub_wavelevel_get_loop_start(currentWavelevel);
	ewi.looping = (zzub_wave_get_flags(currentWave) & (zzub_wave_flag_loop|zzub_wave_flag_pingpong)) != 0;
	ewi.waveProvider = this;
	ewi.samples = zzub_wavelevel_get_sample_count(currentWavelevel);
	ewi.stereo = (zzub_wave_get_flags(currentWave) & zzub_wave_flag_stereo) != 0;
	ewi.type = (zzub_wave_buffer_type)zzub_wavelevel_get_format(currentWavelevel);

	float sps = (float)zzub_wavelevel_get_samples_per_second(currentWavelevel);
	double tpm = zzub_player_get_bpm(player) * (float)zzub_player_get_tpb(player);

	ewi.samplesPerSec = sps / 1000.0f;
	ewi.samplesPerTick = (60.0f * sps) / tpm;

	int numslices;
	zzub_wavelevel_get_slices(currentWavelevel, &numslices, 0);
	ewi.slices.resize(numslices);
	if (numslices > 0) {
		zzub_wavelevel_get_slices(currentWavelevel, &numslices, &ewi.slices.front());
	}
	waveEditorCtrl.SetEditWave(&ewi, reset);
	waveEditorCtrl.SetSelection(currentInfo->sel_from, currentInfo->sel_from + currentInfo->sel_length);
	if (currentInfo->display_length != 0) {
		waveEditorCtrl.SetZoomDisplay(currentInfo->display_from, currentInfo->display_length);
	} else {
		int numsamples = zzub_wavelevel_get_sample_count(currentWavelevel);
		waveEditorCtrl.SetZoomDisplay(0, numsamples);
	}
}

CWaveInfo* CWaveTableView::GetCurrentWaveInfo() {
	if (currentWaveIndex < 0) return 0;
	return &waveInfos[currentWaveIndex];
}

void CWaveTableView::BindWaveInfos() {
	waveInfos.clear();
	size_t wavecount = zzub_player_get_wave_count(player);
	for (size_t i = 0; i < wavecount; i++) {
		CWaveInfo wi;
		wi.level = 0;
		wi.sel_from = -1;
		wi.sel_length = 0;
		wi.display_from = 0;
		wi.display_length = 0;
		wi.slice = 0;
		wi.length = 0;
		waveInfos.push_back(wi);
	}
}

void CWaveTableView::UpdateZoomLevels(zzub_wavelevel_t* wavelevel) {
	assert(wavelevel != 0);

	zzub_wave_t* wave = zzub_wavelevel_get_wave(wavelevel);
	int waveindex = zzub_wave_get_index(wave);

	CWaveInfo* currentInfo = &waveInfos[waveindex];

	zzub_wavelevel_t* infolevel = zzub_wave_get_level(wave, currentInfo->level);
	if (infolevel != wavelevel) return ; // not visible

	int oldsamplecount = currentInfo->length;
	int newsamplecount = zzub_wavelevel_get_sample_count(wavelevel);

	if (oldsamplecount != newsamplecount) {
		// change zoom level by the difference
		int sizediff = (newsamplecount - oldsamplecount);
		if (newsamplecount < oldsamplecount) {
			currentInfo->sel_from = -1; 
			currentInfo->sel_length = -1;
		}
		currentInfo->length += sizediff; // hrm..
		if (currentInfo->display_length + sizediff >= 0) {
			currentInfo->display_length += sizediff;
		} else {
			currentInfo->display_from = 0;
			currentInfo->display_length = currentInfo->length;
		}
	}
}

//
// Wave editor display modes (via toolbar buttons)
//

void CWaveTableView::UpdateToolbar() {
	SendMessage(hWndEditToolBar, TB_PRESSBUTTON, ID_WAVE_ZOOM_SAMPLES, FALSE);
	SendMessage(hWndEditToolBar, TB_PRESSBUTTON, ID_WAVE_ZOOM_SECONDS, FALSE);
	SendMessage(hWndEditToolBar, TB_PRESSBUTTON, ID_WAVE_ZOOM_TICKS, FALSE);
	SendMessage(hWndEditToolBar, TB_PRESSBUTTON, ID_WAVE_ZOOM_WORD, FALSE); // test 014

	switch (waveEditorCtrl.gridType) {
		case WaveEditorSamples:
			SendMessage(hWndEditToolBar, TB_PRESSBUTTON, ID_WAVE_ZOOM_SAMPLES, TRUE);
			break;
		case WaveEditorSeconds:
			SendMessage(hWndEditToolBar, TB_PRESSBUTTON, ID_WAVE_ZOOM_SECONDS, TRUE);
			break;
		case WaveEditorTicks:
			SendMessage(hWndEditToolBar, TB_PRESSBUTTON, ID_WAVE_ZOOM_TICKS, TRUE);
			break;
		case WaveEditorWord:
			SendMessage(hWndEditToolBar, TB_PRESSBUTTON, ID_WAVE_ZOOM_WORD, TRUE);
			break;
	}
}

LRESULT CWaveTableView::OnZoomSamples(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	waveEditorCtrl.gridType = WaveEditorSamples;
	UpdateToolbar();
	waveEditorCtrl.Invalidate(FALSE);
	BindStatus();
	return 0;
}

LRESULT CWaveTableView::OnZoomTicks(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	waveEditorCtrl.gridType = WaveEditorTicks;
	UpdateToolbar();
	waveEditorCtrl.Invalidate(FALSE);
	BindStatus();
	return 0;
}

LRESULT CWaveTableView::OnZoomSeconds(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	waveEditorCtrl.gridType = WaveEditorSeconds;
	UpdateToolbar();
	waveEditorCtrl.Invalidate(FALSE);
	BindStatus();
	return 0;
}

LRESULT CWaveTableView::OnZoomWord(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	waveEditorCtrl.gridType = WaveEditorWord;
	UpdateToolbar();
	waveEditorCtrl.Invalidate(FALSE);
	BindStatus();
	return 0;
}

// 
// Wave editor control- f.ex copy/paste/select/zoom (via accelerators)
// 

bool CWaveTableView::CopySelection() {
	if (currentWavelevel == 0) return false;

	long beginSelect = min(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);
	long endSelect = max(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);

	if (beginSelect == -1) {
		beginSelect = 0;
		endSelect = waveEditorCtrl.waveInfo.samples - 1;
	}
    
	if (currentWave == 0) return false;

	zzub_archive_t* arc = zzub_archive_create_memory();
	zzub_output_t* outf = zzub_archive_get_output(arc, "");
	if (outf) {
		int numSamples = (endSelect - beginSelect) + 1;
		zzub_wavelevel_save_wav_range(currentWavelevel, outf, beginSelect, numSamples);

		zzub_input_t* inf = zzub_archive_get_input(arc, "");
		int size = zzub_input_size(inf);
		char* bytes = new char[size];
		zzub_input_read(inf, bytes, size);

		CopyBinary((HWND)buze_main_frame_get_wnd(mainframe), CF_WAVE, bytes, size);
		delete[] bytes;
	}

	zzub_archive_destroy(arc);
	return true;
}

void CWaveTableView::DeleteSelection() {
	if (currentWavelevel == 0) return ;

	if (waveEditorCtrl.beginSelectSample == -1) return ;

	long beginSelect = min(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);
	long endSelect = max(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);

	int w = waveList.GetSelectedIndex();
	if (w == -1) return ;

	int numSamples = (endSelect - beginSelect) + 1;
	zzub_wavelevel_remove_sample_range(currentWavelevel, beginSelect, numSamples);
}

LRESULT CWaveTableView::OnCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	if (CopySelection())
		DeleteSelection();
	zzub_player_history_commit(player, 0, 0, "Cut Wave Selection");
	return 0;
}

LRESULT CWaveTableView::OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	CopySelection();
	return 0;
}

LRESULT CWaveTableView::OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	if (currentWave == 0) return 0;

	if (!OpenClipboard()) return 0;

	//get the buffer
	HANDLE hData = GetClipboardData(CF_WAVE);
	if (hData == 0) {
		CloseClipboard();
		return 0;
	}

	char* charbuf = (char*)GlobalLock( hData);
	int buffersize = (int)GlobalSize(hData);
	if (charbuf != 0) {
		zzub_archive_t* arc = zzub_archive_create_memory();
		zzub_output_t* outf = zzub_archive_get_output(arc, "");
		zzub_output_write(outf, charbuf, buffersize);
		zzub_input_t* inf = zzub_archive_get_input(arc, "");
		
		zzub_wavelevel_t* wavelevel;
		bool new_wave = false;
		if (currentWavelevel) {
			wavelevel = currentWavelevel;
		} else {
			wavelevel = zzub_wave_add_level(currentWave);
			new_wave = true;
		}

		int paste_offset = waveEditorCtrl.cursorSamplePosition;
		int length = zzub_wavelevel_load_wav(wavelevel, paste_offset, 0, inf);
		if (new_wave) {
			zzub_wave_set_volume(currentWave, 1.0f);
		}
		zzub_archive_destroy(arc);

		zzub_player_history_commit(player, 0, 0, "Paste Wave");

		int samplecount = zzub_wavelevel_get_sample_count(wavelevel);

		// select pasted stuff
		BindWaveLevels();
		UpdateWaveEditor();
		waveEditorCtrl.SetSelection(paste_offset, paste_offset + length - 1);
		//waveEditorCtrl.SetZoomDisplay(0, samplecount);
		waveEditorCtrl.SetCursorPosition(paste_offset + length);
	}

	GlobalUnlock(hData);
	CloseClipboard();

	return 0;
}

LRESULT CWaveTableView::OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	DeleteSelection();
	zzub_player_history_commit(player, 0, 0, "Delete Wave Range");
	return 0;
}

LRESULT CWaveTableView::OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {

	if (currentWavelevel == 0) return 0;

	waveEditorCtrl.SetSelection(0, zzub_wavelevel_get_sample_count(currentWavelevel)-1);
	waveEditorCtrl.SetCursorPosition(0);
	return 0;
}

LRESULT CWaveTableView::OnClearSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	return 0;
}

LRESULT CWaveTableView::OnZoomIn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	waveEditorCtrl.ZoomIn();
	return 0;
}

LRESULT CWaveTableView::OnZoomOut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	waveEditorCtrl.ZoomOut();
	return 0;
}

LRESULT CWaveTableView::OnZoomSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	waveEditorCtrl.ZoomShowSelection();
	return 0;
}

LRESULT CWaveTableView::OnZoomAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	waveEditorCtrl.ZoomShowAll();
	return 0;
}

//
// Wave editor events - f.ex zoom change, selection change, loop change, mousemove
// 

// the wave editor sends looping points updates which we commit here
void CWaveTableView::SetLoopingPoints(bool enabled, int beginSample, int endSample) {
	if (currentWave == 0) return ;
	if (currentWavelevel == 0) return ;

	int numSamples = zzub_wavelevel_get_sample_count(currentWavelevel);
	if (endSample > numSamples) endSample = numSamples - 1;
	if (beginSample < 0) beginSample = 0;

	zzub_wavelevel_set_loop_start(currentWavelevel, beginSample);
	zzub_wavelevel_set_loop_end(currentWavelevel, endSample);

	dirtyWavelevels = true;
	Invalidate(FALSE);
}

LRESULT CWaveTableView::OnWaveEditorSelectionChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	CWaveInfo* currentInfo = GetCurrentWaveInfo();
	currentInfo->sel_from = waveEditorCtrl.beginSelectSample;
	currentInfo->sel_length = waveEditorCtrl.endSelectSample - waveEditorCtrl.beginSelectSample;
	BindStatus();
	return 0;
}

LRESULT CWaveTableView::OnWaveEditorZoomChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CWaveInfo* currentInfo = GetCurrentWaveInfo();
	if (currentInfo) {
		currentInfo->display_from = (int)waveEditorCtrl.beginDisplaySample;
		currentInfo->display_length = waveEditorCtrl.numDisplaySamples;
	}
	return 0;
}

LRESULT CWaveTableView::OnWaveEditorLoopChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	SetLoopingPoints(true, waveEditorCtrl.waveInfo.beginLoop, waveEditorCtrl.waveInfo.endLoop);
	zzub_player_history_commit(player, 0, 0, "Set Looping Points");
	return 0;
}


LRESULT CWaveTableView::OnWaveEditorMouseMove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	BindStatus();
	return 0;
}

LRESULT CWaveTableView::OnTrim(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	if (currentWavelevel == 0) return 0;					//  do some checking first
	if (waveEditorCtrl.beginSelectSample == -1) return 0;	//  do some checking first

	long beginSelect = min(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);
	long endSelect = max(waveEditorCtrl.beginSelectSample, waveEditorCtrl.endSelectSample);

	int numSamples = zzub_wavelevel_get_sample_count(currentWavelevel);

	zzub_wavelevel_remove_sample_range(currentWavelevel, endSelect, numSamples - endSelect - 1);	// right of selection
	if (beginSelect > 0)
		zzub_wavelevel_remove_sample_range(currentWavelevel, 0, beginSelect);			// left of selection

	zzub_player_history_commit(player, 0, 0, "Trim Wave");
	return 0;
}

// 
// Envelope tab
//

// the envelope editor sends envelopes updates which we commit here
void CWaveTableView::SetEnvelope(int numPoints, int sustainIndex, long* points) {
	// create envelopes that do not exist
	if (currentWave == 0) return ;

	int e = envelopeTab.GetSelectedEnvelope();
	if (e == -1) return ;

	zzub_envelope_t* env = zzub_wave_get_envelope(currentWave, e);
	assert(env);

	if (zzub_envelope_get_point_count(env) < numPoints)
		zzub_envelope_insert_point(env, 1);

	if (zzub_envelope_get_point_count(env) > numPoints)
		zzub_envelope_delete_point(env, 1);

	for (int i = 0; i < numPoints; i++) {

		unsigned short x = (unsigned short)points[i * 2 + 0];
		unsigned short y = (unsigned short)(65535 - points[i * 2 + 1]);
		unsigned char flags = sustainIndex == i ? 1 : 0;

		zzub_envelope_set_point(env, i, x, y, flags);
	}
}

//
// Handle Armstrong and Buze events and interfaces
//

void CWaveTableView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	effectsTab.OnUpdate(pSender, lHint, pHint);

	switch (lHint) {
		case buze_event_type_update_post_open_document:
			waveList.SelectItem(0);
			BindWaveInfos();
			dirtyWavelist = true;
			dirtyEnvelope = true;
			dirtyWavelevels = true;
			dirtyWaveEditor = true;
			dirtyStatus = true;
			Invalidate(FALSE);
			break;
		case buze_event_type_update_new_document:
			waveList.SelectItem(0);
			BindWaveInfos();
			dirtyWavelist = true;
			dirtyWaveEditorReset = true;
			dirtyStatus = true;
			Invalidate(FALSE);
			break;
		/*case zzub_event_type_update_wavelevel_samples:
			if (zzubData->update_wavelevel_samples.wavelevel == currentWavelevel) {
				dirtyWaveEditor = true;
				Invalidate(FALSE);
			}
			break;*/
		case zzub_event_type_insert_wavelevel:
			if (zzub_wavelevel_get_wave(zzubData->insert_wavelevel.wavelevel) == currentWave) {
				dirtyWavelevels = true;

				if (currentWavelevel == 0) {
					currentWavelevel = zzubData->insert_wavelevel.wavelevel;
					dirtyWaveEditorReset = true;
					dirtyStatus = true;
					dirtyEditTab = true;
					dirtyEnvelope = true;
					dirtySlices = true;
				}

				Invalidate(FALSE);
			}
			break;
		case zzub_event_type_update_wavelevel:
			if (zzub_wavelevel_get_wave(zzubData->update_wavelevel.wavelevel) == currentWave) {
				dirtyWavelevels = true;
				if (zzubData->update_wavelevel.wavelevel == currentWavelevel) {
					dirtyWaveEditor = true;
					dirtyStatus = true;
					dirtyEditTab = true;
					dirtyEnvelope = true;
					dirtySlices = true;
				}
				Invalidate(FALSE);
			}
			break;
		case zzub_event_type_update_wavelevel_samples:
			UpdateZoomLevels(zzubData->update_wavelevel_samples.wavelevel);
			if (zzubData->update_wavelevel_samples.wavelevel == currentWavelevel) {
				dirtyWaveEditor = true;
				dirtyStatus = true;
				dirtyEditTab = true;
				dirtyEnvelope = true;
				dirtySlices = true;
				Invalidate(FALSE);
			}
			break;
		case zzub_event_type_delete_wavelevel:
			if (currentWavelevel == zzubData->delete_wavelevel.wavelevel) {
				currentWavelevel = 0;
				waveEditorCtrl.SetEditWave(0, true);
			}
			dirtyWavelist = true;
			dirtyWavelevels = true;
			dirtyWaveEditorReset = true;
			dirtyStatus = true;
			dirtyEditTab = true;
			dirtyEnvelope = true;
			dirtySlices = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_insert_wave:
		case zzub_event_type_update_wave:
			if (currentWave == 0) {
				int sel = waveList.GetSelectedIndex();
				if (sel >= 0 && sel < zzub_player_get_wave_count(player)) {
					currentWave = zzub_player_get_wave(player, sel);
					currentWavelevel = 0;
				}
			}
			dirtyWavelist = true;
			dirtyWavelevels = true;
			dirtyWaveEditor = true;
			dirtyEditTab = true;
			dirtyStatus = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_delete_wave:
			if (currentWave == zzubData->delete_wave.wave) {
				currentWave = 0;
				currentWavelevel = 0;
			}
			dirtyWavelist = true;
			dirtyWavelevels = true;
			dirtyWaveEditor = true;
			dirtyStatus = true;
			dirtySlices = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_insert_plugin:
		case zzub_event_type_update_plugin:
			dirtyEnvelope = true;
			Invalidate(FALSE);
			break;
	}
}

LRESULT CWaveTableView::OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	//mainframe->showHelpWavetable();
	return 0;
}

LRESULT CWaveTableView::OnGetEditFlags(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return GetEditFlags();
}

LRESULT CWaveTableView::OnGetStatusText(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// TODO: return a more directed status text?
	return (LRESULT)"Press Enter or Shift+F9 to open the File Browser. Drag and drop from the File Browser or Explorer into the wave list.";
}

int CWaveTableView::GetEditFlags() {
	int flags = 0;

	flags |= EDIT_COPY;
	flags |= EDIT_PASTE;

	return flags;
}

void CWaveTableView::GetHelpText(char* text, int* len) {

	std::string helptext = PeekString(_Module.GetResourceInstance(), IDT_HELP_WAVETABLEVIEW);
	HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "wavetable");
	std::string acceltext = CreateAccelTableString(hAccel);

	helptext += acceltext;
	*len = (int)helptext.length();
	if (text)
		strcpy(text, helptext.c_str());
}

void CWaveTableView::BindCurrentWave() {
	buze_event_data_t ev;
	ev.show_properties.return_view = this;
	ev.show_properties.type = buze_property_type_wave;
	ev.show_properties.wave = currentWave;
	buze_document_notify_views(document, this, buze_event_type_update_properties, &ev);
}
