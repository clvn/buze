#include "stdafx.h"
#include <iomanip>
#include <strstream>
#include <limits>
#include <fstream>
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "Configuration.h"
#include "Properties.h"
#include "DragDropImpl.h"
#include "MachineDropTarget.h"
#include "utils.h"
#include "Keymaps.h"

#include "BuzeConfiguration.h"
#include "Mixdown.h"
#include "MachineView.h"
#include "MenuHelpers.h"
#include "MachineHelpers.h"

const int plugingroup_id_first = 1000000; // subtract this from container node ids to get the group id

CHostDllModule _Module;

// 
// Factory
//

CMachineViewInfo::CMachineViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CMachineView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Machines";
	tooltip = "Machine view";
	place = 1; //DockSplitTab::placeMAINPANE;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = true;
	allowfloat = false;
	defaultview = true;
}

CView* CMachineViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	// remove the WS_CLIPCHILDREN style - this causes the view to receive WM_PAINTs when the view is
	// invalidated (in OnUpdate) in an inactive tab view. there was a problem with the view didnt rebind
	// e.g after renaming a plugin in the property view; where the property view and patternformat view 
	// are in the same pane but different tabs. in this case WM_PAINT wasnt invoked for the 
	// patternformatview.  removing WS_CLIPCHILDREN fixed it. 
	// this is more an indicator of a deeper problem by (ab)using the painting subsystem for rebinding:
	CMachineView* view = new CMachineView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CMachineViewInfo::Attach() {
	buze_document_add_view(document, this);

	//buze_main_frame_register_accelerator(mainframe, "machineview", "help", ID_HELP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "edit_copy", "c ctrl", ID_EDIT_COPY);
	buze_main_frame_register_accelerator(mainframe, "machineview", "edit_cut", "x ctrl", ID_EDIT_CUT);
	buze_main_frame_register_accelerator(mainframe, "machineview", "edit_paste", "v ctrl", ID_EDIT_PASTE);
	buze_main_frame_register_accelerator(mainframe, "machineview", "edit_paste_no_data", "v ctrl shift", ID_EDIT_PASTE_NO_DATA);
	buze_main_frame_register_accelerator(mainframe, "machineview", "edit_selectall", "a ctrl", ID_EDIT_SELECTALL);
	buze_main_frame_register_accelerator(mainframe, "machineview", "edit_delete", "delete", ID_EDIT_DELETE);
	buze_main_frame_register_accelerator(mainframe, "machineview", "delete_and_restore", "delete shift", ID_MACHINE_DELETE_AND_RESTORE);
	buze_main_frame_register_accelerator(mainframe, "machineview", "align_down", "down alt", ID_MACHINE_ALIGN_DOWN);
	buze_main_frame_register_accelerator(mainframe, "machineview", "align_left", "left alt", ID_MACHINE_ALIGN_LEFT);
	buze_main_frame_register_accelerator(mainframe, "machineview", "align_right", "right alt", ID_MACHINE_ALIGN_RIGHT);
	buze_main_frame_register_accelerator(mainframe, "machineview", "align_up", "up alt", ID_MACHINE_ALIGN_UP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_down", "down ctrl", ID_MACHINE_MOVE_DOWN);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_down_step", "down ctrl shift", ID_MACHINE_MOVE_DOWN_STEP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_down_left", "end ctrl", ID_MACHINE_MOVE_DOWN_LEFT);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_down_left_step", "end ctrl shift", ID_MACHINE_MOVE_DOWN_LEFT_STEP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_down_right", "pagedown ctrl", ID_MACHINE_MOVE_DOWN_RIGHT);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_down_right_step", "pagedown ctrl shift", ID_MACHINE_MOVE_DOWN_RIGHT_STEP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_left", "left ctrl", ID_MACHINE_MOVE_LEFT);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_left_step", "left ctrl shift", ID_MACHINE_MOVE_LEFT_STEP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_right", "right ctrl", ID_MACHINE_MOVE_RIGHT);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_right_step", "right ctrl shift", ID_MACHINE_MOVE_RIGHT_STEP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_up", "up ctrl", ID_MACHINE_MOVE_UP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_up_step", "up ctrl shift", ID_MACHINE_MOVE_UP_STEP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_up_left", "home ctrl", ID_MACHINE_MOVE_UP_LEFT);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_up_left_step", "home ctrl shift", ID_MACHINE_MOVE_UP_LEFT_STEP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_up_right", "pageup ctrl", ID_MACHINE_MOVE_UP_RIGHT);
	buze_main_frame_register_accelerator(mainframe, "machineview", "move_up_right_step", "pageup ctrl shift", ID_MACHINE_MOVE_UP_RIGHT_STEP);
	buze_main_frame_register_accelerator(mainframe, "machineview", "machine_mute", "m ctrl", ID_MACHINE_MUTE);
	buze_main_frame_register_accelerator(mainframe, "machineview", "machine_solo", "s ctrl", ID_MACHINE_SOLO);
	buze_main_frame_register_accelerator(mainframe, "machineview", "machine_bypass", "b ctrl", ID_MACHINE_BYPASS);
	buze_main_frame_register_accelerator(mainframe, "machineview", "machine_unmute_all", "u ctrl", ID_UNMUTE_ALL);
	buze_main_frame_register_accelerator(mainframe, "machineview", "machine_parameters", "enter shift", ID_MACHINE_PARAMETERS);
	buze_main_frame_register_accelerator(mainframe, "machineview", "machine_properties", "backspace alt", ID_MACHINE_PROPERTIES);
	buze_main_frame_register_accelerator(mainframe, "machineview", "toggle_connection_text", "tab", ID_MACHINE_TOGGLE_CONNECTION_TEXT);

	// global accelerators - these generate global document events caught in OnUpdate
	// buze_main_frame_add_global_accelerator("view_patternformat", buze_event_type_show_pattern_format_view );
	WORD ID_SHOW_MACHINEVIEW = buze_main_frame_register_accelerator_event(mainframe, "view_machines", "f4", buze_event_type_show_machine_view);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//buze_main_frame_register_accelerator(mainframe, "machineview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_MACHINEVIEW, "Machine View");
}

void CMachineViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CMachineViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CMachineView* view;
	switch (lHint) {
		case buze_event_type_show_machine_view:
			view = (CMachineView*)buze_main_frame_get_view(mainframe, "MachineView", 0);
			if (view) {
				buze_main_frame_set_focus_to(mainframe, view);
			} else
				buze_main_frame_open_view(mainframe, "MachineView", "Machines", 0, -1, -1);
			break;
	}
}

class CMachineViewLibrary : public CViewLibrary {
public:
	virtual void Initialize(CViewFrame* host) {
		_Module.m_hostModule = buze_application_get_host_module(buze_main_frame_get_application(host));
		buze_main_frame_register_window_factory(host, new CMachineViewInfo(host));
	}

	virtual void Destroy() {
		delete this;
	}

	virtual int GetVersion() {
		return CViewLibrary::version;
	}

};

extern "C" CViewLibrary* buze_create_viewlibrary() {
	return new CMachineViewLibrary();
}

//
// View
//

enum EDITFLAGS {
	EDIT_COPY = 1,	// set if selection can be copied
	EDIT_PASTE = 2,	// set if clipboard format is recognized
};

using namespace std;

// ---------------------------------------------------------------------------------------------------------------
// OPTIONS
// ---------------------------------------------------------------------------------------------------------------

//static const int ID_MACHINE_UPDATE_TIMER = 1000;
static const float MAX_MACHINE_SCALE = 4.0f;
static const float MIN_MACHINE_SCALE = 0.3f;
static const int machineMovementPixel = 1;
static const int machineMovementStep = 7;
static const float machine_width = 100.0f;
static const float machine_height = 50.0f;
static const float machine_width_minimized = 12.0f;
static const float machine_height_minimized = 12.0f;
static const float led_width = 8.0f;
static const float led_height = 8.0f;
static const int led_width_minimum = 3;
static const int led_height_minimum = 3;

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DESTRUCTION
// ---------------------------------------------------------------------------------------------------------------

CMachineView::CMachineView(CViewFrame* mainFrm)
:
	CViewImpl(mainFrm)
{
	configuration = buze_application_get_configuration(buze_main_frame_get_application(mainFrm));

	graphCtrl.scale = configuration->getMachineScale();
	//useSkins = configuration->getMachineSkinVisibility();
	graphCtrl.scale_machine_points = configuration->getScaleByWindowSize();
	dirtyVisibleMachines = true;
	lastMidiMachine = 0;
	//show_connection_text = false;
}

CMachineView::~CMachineView(void) {
}

void CMachineView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

void SearchBoxMenuCallback(CSearchBox& self, MENUITEMINFO& mii, void* userdata) {
	zzub_player_t* player = (zzub_player_t*)userdata;
	buze_plugin_index_item_t* item = (buze_plugin_index_item_t*)mii.dwItemData ;
	std::string uri = buze_plugin_index_item_get_filename(item);
	bool is_template = uri == "@zzub.org/buze/template";

	LPTSTR buffer = (LPTSTR)mii.dwTypeData;
	std::tstring name(buffer, buffer + mii.cch);
	if (!is_template) {
		zzub_pluginloader_t* loader = zzub_player_get_pluginloader_by_name(player, uri.c_str());
		if (loader != 0) {
			std::tstring text;
			text = zzub_pluginloader_get_author(loader);
			text += " ";
			text += zzub_pluginloader_get_name(loader);
			// TODO: create tags like !generator !effect !note !midifx !midigen
			// and then use these tags as default search term when searching on connections
			self.AddString(name.c_str(), text.c_str(), mii.wID);
		}
	} else {
		self.AddString(name.c_str(), mii.wID);
	}

}

LRESULT CMachineView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	LRESULT lres = DefWindowProc();

	UpdateTheme();

	currentlayer = 0;
	graphCtrl.Create(m_hWnd, 0, 0, 0, 0, ID_GRAPHCTRL);
	graphCtrl.searchbox.AddMenu((HMENU)buze_main_frame_get_plugin_menu_create(mainframe), &SearchBoxMenuCallback, player);
	graphCtrl.searchbox.IndexNames();

	SetMidiAliases();

	if (!createDropTarget(*this)) return 0;

	FORMATETC ftetc = {0}; 
	ftetc.cfFormat = CF_TEXT; 
	ftetc.dwAspect = DVASPECT_CONTENT; 
	ftetc.lindex = -1; 
	ftetc.tymed = TYMED_HGLOBAL; 
	dropTarget->AddSuportedFormat(ftetc); 
	ftetc.cfFormat=CF_HDROP; 
	dropTarget->AddSuportedFormat(ftetc);

	buze_main_frame_add_timer_handler(mainframe, this);

	buze_document_add_view(document, this);

	buze_main_frame_viewstack_insert(mainframe, this); // false

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	SelectMachine(zzub_player_get_plugin(player, 0));

	return 0;
}

LRESULT CMachineView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveIdleHandler(this);
	pLoop->RemoveMessageFilter(this);

	buze_main_frame_remove_timer_handler(mainframe, this);
	buze_document_remove_view(document, this);
	return 0;
}

LRESULT CMachineView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	releaseDropTarget();
	return 0;
}

BOOL CMachineView::PreTranslateMessage(MSG* pMsg) {
	if (GetFocus() == *this || IsChild(GetFocus())) {
		HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "machineview");
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg))
			return TRUE;
	}
	return FALSE;
}

void CMachineView::UpdateTimer(int count) {
	if ((count % 8) != 0) return; /// 80ms

	InvalidateStatus();
}

// ---------------------------------------------------------------------------------------------------------------
// SIZING / SCROLLING
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	CalcPixelSize();

	RECT rcClient;
	GetClientRect(&rcClient);
	graphCtrl.MoveWindow(&rcClient);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// FOCUS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	//scale = configuration->getMachineScale();
	zzub_player_set_midi_plugin(player, lastMidiMachine);
	graphCtrl.moveType = MachineViewMoveNothing;
	graphCtrl.SetFocus();
	return 0;
}

LRESULT CMachineView::OnBlur( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	lastMidiMachine = zzub_player_get_midi_plugin(player);

	// do not stop active keyjazz notes if the target focus is (a child of) the paramview or something that handles keyjazz!
	HWND hFocusWnd = (HWND)wParam;
	CView* view = buze_main_frame_get_view_by_wnd(mainframe, hFocusWnd);
	if (view != 0 && view->DoesKeyjazz())
		return 0;

	buze_document_keyjazz_release(document, true);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// UPDATES
// ---------------------------------------------------------------------------------------------------------------

void CMachineView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;
	switch (lHint) {
		case buze_event_type_update_theme:
			UpdateTheme();
			break;
		case buze_event_type_update_new_document:
		case buze_event_type_update_post_open_document:
			graphCtrl.nodes.clear();
			graphCtrl.edges.clear();
			graphCtrl.ClearSelectedMachines();
			//if (zzub_player_get_plugin_count(player) > 0)
			//	selectMachine(zzub_player_get_plugin(player, 0));

			dirtyVisibleMachines = true;
			Invalidate(FALSE);
			break;
		//case zzub_event_type_before_insert_connection:
			// the main frame will suspend message passing, and our updateTimer
			// may have left dirtyStatus to true, so we UpdateWindow here to 
			// ensure the window is redrawn and dirty flags cleared - before 
			// any hackish plugins open GUIs and send us WM_PAINTs.
		//	UpdateWindow();
		//	break;
		case zzub_event_type_delete_plugin:
			if (IsSelectedMachine(zzubData->delete_plugin.plugin))
				UnselectMachine(zzubData->delete_plugin.plugin);	// this is a quickfix to fix when a machine is deleted with undo while selected
			if (lastMidiMachine == zzubData->delete_plugin.plugin)
				lastMidiMachine = 0;
			dirtyVisibleMachines = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_delete_plugin_group:
			if (zzubData->delete_plugin_group.group == currentlayer)
				currentlayer = 0;
			dirtyVisibleMachines = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_update_pluginparameter:
			if (zzubData->update_pluginparameter.group == 0) {
				InvalidateMachine(zzubData->update_pluginparameter.plugin);
			}
			break;
		case zzub_event_type_insert_plugin_group:
		case zzub_event_type_update_plugin_group:
		case zzub_event_type_update_plugin:
		case zzub_event_type_insert_plugin:
			dirtyVisibleMachines = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_insert_connection:
			dirtyVisibleMachines = true;
			Invalidate(FALSE);
			break;
		case zzub_event_type_delete_connection:
			//if (zzubData->delete_connection.from_plugin == selectedConnection.to_plugin &&
			//		zzubData->delete_connection.to_plugin == selectedConnection.from_plugin) {
			//	connection_is_selected = false;
			//}
			dirtyVisibleMachines = true;
			Invalidate(FALSE);
			break;
		case buze_event_type_update_settings:
			graphCtrl.scale = configuration->getMachineScale();
			//useSkins = configuration->getMachineSkinVisibility();
			graphCtrl.scale_machine_points = configuration->getScaleByWindowSize();
			graphCtrl.InvalidateMachines(false);
			break;
		case zzub_event_type_barrier:
			graphCtrl.InvalidateMachines(false); // needed for connection arrows volume shading
			break;
		case zzub_event_type_update_song:
			BindSong();
			break;
	}
}

// ---------------------------------------------------------------------------------------------------------------
// PREFERENCES
// ---------------------------------------------------------------------------------------------------------------

void CMachineView::UpdateTheme() {
	buze_application_t* application = buze_main_frame_get_application(mainframe);
	graphCtrl.SetTheme(text, buze_application_get_theme_color(application, "MV Machine Text"));
	graphCtrl.SetTheme(background, buze_application_get_theme_color(application, "MV Background"));
	graphCtrl.SetTheme(border, buze_application_get_theme_color(application, "MV Machine Border"));
	graphCtrl.SetTheme(audio_line, buze_application_get_theme_color(application, "MV Line"));
	graphCtrl.SetTheme(midi_line, buze_application_get_theme_color(application, "MV MIDI Line"));
	graphCtrl.SetTheme(event_line, buze_application_get_theme_color(application, "MV Event Line"));
	graphCtrl.SetTheme(note_line, buze_application_get_theme_color(application, "MV Note Line"));

	graphCtrl.SetTheme(amp_bg, buze_application_get_theme_color(application, "MV Amp BG"));
	graphCtrl.SetTheme(amp_handle, buze_application_get_theme_color(application, "MV Amp Handle"));
	graphCtrl.SetTheme(arrow, buze_application_get_theme_color(application, "MV Arrow"));
	graphCtrl.SetTheme(arrow_low, buze_application_get_theme_color(application, "MV Arrow Volume Low"));
	graphCtrl.SetTheme(arrow_high, buze_application_get_theme_color(application, "MV Arrow Volume High"));

	graphCtrl.SetTheme(master_bg, buze_application_get_theme_color(application, "MV Master"));
	graphCtrl.SetTheme(master_led_off, buze_application_get_theme_color(application, "MV Master LED Off"));
	graphCtrl.SetTheme(master_led_on, buze_application_get_theme_color(application, "MV Master LED On"));
	graphCtrl.SetTheme(generator_bg, buze_application_get_theme_color(application, "MV Generator"));
	graphCtrl.SetTheme(generator_mute, buze_application_get_theme_color(application, "MV Generator Mute"));
	graphCtrl.SetTheme(generator_led_off, buze_application_get_theme_color(application, "MV Generator LED Off"));
	graphCtrl.SetTheme(generator_led_on, buze_application_get_theme_color(application, "MV Generator LED On"));
	graphCtrl.SetTheme(effect_bg, buze_application_get_theme_color(application, "MV Effect"));
	graphCtrl.SetTheme(effect_mute, buze_application_get_theme_color(application, "MV Effect Mute"));
	graphCtrl.SetTheme(effect_led_off, buze_application_get_theme_color(application, "MV Effect LED Off"));
	graphCtrl.SetTheme(effect_led_on, buze_application_get_theme_color(application, "MV Effect LED On"));
	graphCtrl.SetTheme(controller_bg, buze_application_get_theme_color(application, "MV Controller"));
	graphCtrl.SetTheme(controller_mute, buze_application_get_theme_color(application, "MV Controller Mute"));
	graphCtrl.SetTheme(controller_led_off, buze_application_get_theme_color(application, "MV Controller LED Off"));
	graphCtrl.SetTheme(controller_led_on, buze_application_get_theme_color(application, "MV Controller LED On"));

	graphCtrl.SetTheme(container_bg, buze_application_get_theme_color(application, "MV Container"));
	graphCtrl.SetTheme(container_mute, buze_application_get_theme_color(application, "MV Container Mute"));
	graphCtrl.SetTheme(container_led_off, buze_application_get_theme_color(application, "MV Container LED Off"));
	graphCtrl.SetTheme(container_led_on, buze_application_get_theme_color(application, "MV Container LED On"));
	
	if (graphCtrl.m_hWnd) {
		graphCtrl.UpdateFromTheme();
		graphCtrl.Invalidate(FALSE);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// UI UPDATES
// ---------------------------------------------------------------------------------------------------------------

BOOL CMachineView::OnIdle() {
	int editFlags = GetEditFlags();
	UIEnable(ID_EDIT_CUT, (editFlags & EDIT_COPY) != 0);
	UIEnable(ID_EDIT_COPY, (editFlags & EDIT_COPY) != 0);
	UIEnable(ID_EDIT_PASTE, (editFlags & EDIT_PASTE) != 0);
	UIEnable(ID_EDIT_PASTE_NO_DATA, (editFlags & EDIT_PASTE) != 0);
	UIEnable(ID_EDIT_DELETE, (editFlags & EDIT_COPY) != 0);

	return FALSE;
}

LRESULT CMachineView::OnGetEditFlags(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return GetEditFlags();
}

// Only applies to Buze:BMX format related Copy/Paste functionality, not used for Buze:AudioConnectionParameters stuff
int CMachineView::GetEditFlags() {
	int flags = 0;

	if (graphCtrl.selectedMachines.size() > 0)
		flags |= EDIT_COPY;

	if (ClipboardHasFormat(m_hWnd, "Buze:ARMZ"))
		flags |= EDIT_PASTE;

	return flags;
}

// ---------------------------------------------------------------------------------------------------------------
// KEYBOARD
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if (wParam == 27) {
		// esc -> go to parent group
		if (currentlayer != 0) {
			currentlayer = zzub_plugin_group_get_parent(currentlayer);
			dirtyVisibleMachines = true;
			Invalidate(FALSE);
		}
		return 0;
	}

	return DefWindowProc();
}

LRESULT CMachineView::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if ((lParam & (1 << 30)) != 0) return 0;
	if (GetSelectedMachines() == 0) return 0;

	int note = keyboard_mapper::map_code_to_note(buze_document_get_octave(document), wParam);
	if (note > 0 && note != zzub_note_value_off && note != zzub_note_value_cut) note = midi_to_buzz_note(note);
	if (note == -1) return 0;

	zzub_plugin_t* plugin = zzub_player_get_midi_plugin(player);
	if (plugin == 0) return 0;

	buze_document_keyjazz_key_down(document, plugin, wParam, note);
	buze_document_play_plugin_note(document, plugin, note, 0);
	return 0;
}

LRESULT CMachineView::OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	int lastnote;
	zzub_plugin_t* lastplugin = 0;

	buze_document_keyjazz_key_up(document, wParam, &lastnote, &lastplugin);
	if (lastplugin == 0) return 0;

	buze_document_play_plugin_note(document, lastplugin, zzub_note_value_off, lastnote);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// CONTEXT MENUS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// disallow context menus while dragging
	if (graphCtrl.moveType != MachineViewMoveNothing) return 0;

	POINT screenpt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	POINT clientpt = screenpt;
	ScreenToClient(&clientpt);

	// invoked by keyboard?
	if (screenpt.x == -1) {
		ShowKeyboardContext();
		return 0;
	} 

	// rclick connection?
	CGraphNodePair conn;
	if (graphCtrl.GetConnectionAtPt(clientpt, &conn)) {
		ShowConnectionContext(screenpt, conn);
		return 0;
	}

	graphCtrl.UnselectConnection();

	// rclick machine?
	CGraphNode* propMachine = graphCtrl.GetMachineAtPt(clientpt, 0);
	if (propMachine != 0) {
		// if r-click on non-selected machine, clear selection and show context for one machine, otherwise show a group context
		if (!graphCtrl.IsSelectedMachine(propMachine->id)) {
			graphCtrl.ClearSelectedMachines();
			graphCtrl.SelectMachine(propMachine);
		}
		ShowNodeSelectionContext(screenpt);
		return 0;
	}

	// rclick background
	ShowBackgroundContext(screenpt);

	return 0;
}

void CMachineView::ShowKeyboardContext() {
	if (graphCtrl.GetSelectedMachines() > 0) {
		// show context menu at the first selected machine
		RECT rc;
		CGraphNode* node = graphCtrl.GetSelectedMachine(0); 
		graphCtrl.GetMachineRect(node, &rc);
		POINT pt = { rc.right, rc.top };
		ClientToScreen(&pt);
		ShowNodeSelectionContext(pt);
	} else {
		// show background context menu
		RECT rcClient;
		GetClientRect(&rcClient);
		int plugins_size = 10;
		int num_plugins = zzub_player_get_plugin_count(player) - 1;	// -1 for master so we start in top left corner
		int xsize = rcClient.right / (plugins_size + 1);
		int ysize = rcClient.bottom / (plugins_size + 1);
		POINT pt;
		pt.x = xsize + (num_plugins % plugins_size) * xsize;
		pt.y = ysize + (num_plugins / plugins_size) * ysize;
		ClientToScreen(&pt);
		ShowBackgroundContext(pt);
	}
}

void CMachineView::ShowNodeSelectionContext(POINT pt) {
	if (graphCtrl.GetSelectedMachines() == 1) {
		// show single item context menu
		CGraphNode* node = graphCtrl.GetSelectedMachine(0);
		if (node->type == node_container)
			ShowGroupContext(pt, zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first));
		else
			ShowPluginContext(pt, zzub_player_get_plugin_by_id(player, node->id));
	} else
	if (graphCtrl.GetSelectedMachines() > 1) {
		// show context for groups, machines or mixed
		int numGroups = 0;
		int numPlugins = 0;
		for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
			CGraphNode* node = graphCtrl.GetSelectedMachine(i);
			if (node->type == node_container)
				numGroups ++;
			else
				numPlugins++;
		}
        
		if (numPlugins > 0 && numGroups == 0)
			ShowMultiPluginContext(pt);
		else if (numGroups > 0 && numPlugins == 0)
			ShowMultiGroupContext(pt);
		else
			ShowMultiMixedContext(pt);
	}
}

void CMachineView::ShowPluginContext(POINT pt, zzub_plugin_t* machine) {

	bool isMaster = (zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) == IS_ROOT_PLUGIN_FLAGS;
	bool isEffect = ((zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) & IS_EFFECT_PLUGIN_FLAGS) != 0;
	bool isGenerator = (zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) == IS_GENERATOR_PLUGIN_FLAGS;
	bool isController = ((zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) & IS_CONTROLLER_PLUGIN_FLAGS) != 0;
	bool isBypassed = zzub_plugin_get_bypass(machine) != 0 ? true : false;
	bool isMinimized = zzub_plugin_get_minimize(machine) != 0 ? true : false;
	bool isMuted = zzub_plugin_get_mute(machine) != 0 ? true : false;
	bool isHiddenIn = graphCtrl.GetMachineHideIncomingConnections(zzub_plugin_get_id(machine));
	zzub_plugin_t* soloplugin = (zzub_plugin_t*)buze_document_get_solo_plugin(document);
	bool isSolo = (soloplugin != 0) && soloplugin == machine;

	CMenu menu; 
	menu.CreatePopupMenu();

	menu.AppendMenu(isMaster?MF_GRAYED:0|isMuted?MF_CHECKED:0|MF_STRING, (UINT_PTR)ID_MACHINE_MUTE, "Mute");
	menu.AppendMenu(isSolo?MF_CHECKED:0|!isGenerator?MF_GRAYED:0|MF_STRING, (UINT_PTR)ID_MACHINE_SOLO, "Solo");
	menu.AppendMenu(isBypassed?MF_CHECKED:0|MF_STRING, (UINT_PTR)ID_MACHINE_BYPASS, "Bypass");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_MINIMIZE, "M&inimize");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINEVIEW_UNMINIMIZE, "&Unminimize");
	menu.AppendMenu(isHiddenIn?MF_CHECKED:0|MF_STRING, (UINT_PTR)ID_MACHINE_HIDE_IN_CONNECTIONS, "H&ide Incoming");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_PARAMETERS, "Parameters");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_PROPERTIES, "Properties");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_CUT, "Cut");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_COPY, "Copy");
	menu.AppendMenu(isMaster?MF_GRAYED:0|MF_STRING, (UINT_PTR)ID_EDIT_DELETE, "Delete");
	menu.AppendMenu(isMaster?MF_GRAYED:0|MF_STRING, (UINT_PTR)ID_MACHINE_DELETE_AND_RESTORE, "Smart Delete");

	menu.AppendMenu(MF_SEPARATOR);

	int machinemenusidx = menu.GetMenuItemCount();
	// ^-- save index so we can add it underneath after calling hotkeys.AddMenuKeys()
	// so that AddMenuKeys() doesn't have to recurse through the whole machine menu.

	menu.AppendMenu(MF_SEPARATOR);

	bool hasWave = buze_document_get_current_wave(document) != 0;

	CMenuHandle mixdownMenu;
	mixdownMenu.CreatePopupMenu();
	mixdownMenu.AppendMenu(MF_STRING, (UINT_PTR)ID_MIXDOWN_FILE, "To File...");
	mixdownMenu.AppendMenu(hasWave?0:MF_GRAYED|MF_STRING, (UINT_PTR)ID_MIXDOWN_WAVETABLE, "To Selected Wave In Wavetable");
	mixdownMenu.AppendMenu(MF_GRAYED|MF_STRING, (UINT_PTR)ID_MIXDOWN_MULTITRACK, "With Multitrack Recorder");

	menu.AppendMenu(MF_POPUP, (UINT_PTR)mixdownMenu.m_hMenu, "Render Output");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MAKE_TEMPLATE, "Save Selection To File...");

	BindPluginTimingMenu(player, menu, machine);
	BindPluginGroupMenu(player, menu, 0);

	BindPluginCommandsMenu(player, menu, machine);

	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	std::string helpfile = buze_document_get_plugin_helpfile(document, loader);
	if (helpfile.length() > 0) {
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_HELP, "Help");
	}

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_CREATEDEFAULTFORMAT, "Create Full Pattern Format");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_CREATESIMPLEFORMAT, "Create Simple Pattern Format");

	if (configuration->getShowAccelerators())
		buze_main_frame_add_menu_keys(mainframe, "machineview", menu);

	// must add these after so AddMenuKeys does not recurse into the machine menus
	menu.InsertMenu(machinemenusidx++, isMaster?MF_GRAYED:0|MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_replace(mainframe), "Replace");
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), "Insert Before");
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), "Insert After");

	//ClientToScreen(&pt);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	// We must do this or the submenus aren't reusable
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_replace(mainframe), MF_BYCOMMAND);
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), MF_BYCOMMAND);
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), MF_BYCOMMAND);
}

void CMachineView::ShowMultiPluginContext(POINT pt) {

	CMenu menu; 
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_MUTE, "Mute");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_BYPASS, "Bypass");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_MINIMIZE, "M&inimize");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINEVIEW_UNMINIMIZE, "&Unminimize");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_HIDE_IN_CONNECTIONS, "H&ide Incoming");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_CUT, "Cut");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_COPY, "Copy");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_DELETE, "Delete");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_DELETE_AND_RESTORE, "Smart Delete");

	menu.AppendMenu(MF_SEPARATOR);

	int machinemenusidx = menu.GetMenuItemCount();
	// ^-- save index so we can add it underneath after calling hotkeys.AddMenuKeys()
	// so that AddMenuKeys() doesn't have to recurse through the whole machine menu.

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MAKE_TEMPLATE, "Save Selection To File...");

	BindPluginTimingMenu(player, menu, 0);

	BindPluginGroupMenu(player, menu, 0);

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_CREATEDEFAULTFORMAT, "Create Full Pattern Format");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_CREATESIMPLEFORMAT, "Create Simple Pattern Format");

	if (configuration->getShowAccelerators())
		buze_main_frame_add_menu_keys(mainframe, "machineview", menu);

	// must add these after so AddMenuKeys does not recurse into the machine menus
//	menu.InsertMenu(machinemenusidx++, isMaster?MF_GRAYED:0|MF_BYPOSITION|MF_POPUP, (UINT_PTR)mainframe->GetMachineMenuReplace(), "Replace");
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), "Insert Before");
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), "Insert After");

	//ClientToScreen(&pt);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	// We must do this or the submenus aren't reusable
	//menu.RemoveMenu((UINT_PTR)mainframe->GetMachineMenuReplace(), MF_BYCOMMAND);
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), MF_BYCOMMAND);
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), MF_BYCOMMAND);
}

void CMachineView::ShowGroupContext(POINT pt, zzub_plugin_group_t* group) {
	CMenu menu; 
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_PROPERTIES, "Properties");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_CUT, "Cut");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_COPY, "Copy");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_DELETE, "Delete");
	menu.AppendMenu(MF_SEPARATOR);

	int machinemenusidx = menu.GetMenuItemCount();
	// ^-- save index so we can add it underneath after calling hotkeys.AddMenuKeys()
	// so that AddMenuKeys() doesn't have to recurse through the whole machine menu.

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MAKE_TEMPLATE, "Save Selection To File...");

	BindPluginGroupMenu(player, menu, group);

	if (configuration->getShowAccelerators())
		buze_main_frame_add_menu_keys(mainframe, "machineview", menu);

	// must add these after so AddMenuKeys does not recurse into the machine menus
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), "Insert Before");
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), "Insert After");

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	// We must do this or the submenus aren't reusable
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), MF_BYCOMMAND);
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), MF_BYCOMMAND);
}

void CMachineView::ShowMultiGroupContext(POINT pt) {
	CMenu menu; 
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_CUT, "Cut");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_COPY, "Copy");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_DELETE, "Delete");
	menu.AppendMenu(MF_SEPARATOR);

	int machinemenusidx = menu.GetMenuItemCount();
	// ^-- save index so we can add it underneath after calling hotkeys.AddMenuKeys()
	// so that AddMenuKeys() doesn't have to recurse through the whole machine menu.

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MAKE_TEMPLATE, "Save Selection To File...");

	BindPluginGroupMenu(player, menu, 0);

	if (configuration->getShowAccelerators())
		buze_main_frame_add_menu_keys(mainframe, "machineview", menu);

	// must add these after so AddMenuKeys does not recurse into the machine menus
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), "Insert Before");
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), "Insert After");

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	// We must do this or the submenus aren't reusable
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), MF_BYCOMMAND);
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), MF_BYCOMMAND);
}

void CMachineView::ShowMultiMixedContext(POINT pt) {
	CMenu menu; 
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_CUT, "Cut");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_COPY, "Copy");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_DELETE, "Delete");
	menu.AppendMenu(MF_SEPARATOR);

	int machinemenusidx = menu.GetMenuItemCount();
	// ^-- save index so we can add it underneath after calling hotkeys.AddMenuKeys()
	// so that AddMenuKeys() doesn't have to recurse through the whole machine menu.

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MAKE_TEMPLATE, "Save Selection To File...");

	BindPluginGroupMenu(player, menu, 0);

	if (configuration->getShowAccelerators())
		buze_main_frame_add_menu_keys(mainframe, "machineview", menu);

	// must add these after so AddMenuKeys does not recurse into the machine menus
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), "Insert Before");
	menu.InsertMenu(machinemenusidx++, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), "Insert After");

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	// We must do this or the submenus aren't reusable
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_before(mainframe), MF_BYCOMMAND);
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_insert_after(mainframe), MF_BYCOMMAND);
}

void CMachineView::ShowConnectionContext(POINT pt, CGraphNodePair const& conn) {

	graphCtrl.SelectConnection(conn);

	zzub_connection_t* audioconn = GetEdgeConnection(conn.to_node_id, conn.from_node_id, edge_audio, zzub_connection_type_audio);
	zzub_connection_t* midiconn = GetEdgeConnection(conn.to_node_id, conn.from_node_id, edge_midi, zzub_connection_type_midi);
	zzub_connection_t* eventconn = GetEdgeConnection(conn.to_node_id, conn.from_node_id, edge_event, zzub_connection_type_event);
	zzub_connection_t* noteconn = GetEdgeConnection(conn.to_node_id, conn.from_node_id, edge_note, zzub_connection_type_note);

	CMenu menu;
	menu.CreatePopupMenu();
	if (audioconn != 0) {
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_DISCONNECT_AUDIO, "Disconnect Audio");
	}
	if (midiconn != 0) {
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_DISCONNECT_MIDI, "Disconnect MIDI");
	}
	if (eventconn != 0) {
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_DISCONNECT_EVENT, "Disconnect All Controls");
	}
	if (noteconn != 0) {
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MACHINE_DISCONNECT_NOTE, "Disconnect Note");
	}

	if (audioconn != 0) {
		bool can_paste = ClipboardHasFormat(m_hWnd, "Buze:AudioConnectionParameters") != 0;
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_COPY_AUDIOCONNECTIONPARAMETERS, "Copy");
		menu.AppendMenu(can_paste?0:MF_GRAYED|MF_STRING, (UINT_PTR)ID_EDIT_PASTE_AUDIOCONNECTIONPARAMETERS, "Paste");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_create(mainframe), "Insert");
	}

	menu.AppendMenu(MF_SEPARATOR);
	// Only add one "properties"-entry
	if (audioconn)
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_AUDIO_CONNECTION_PROPERTIES, "Properties");
	else if (midiconn)
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MIDI_CONNECTION_PROPERTIES, "Properties");
	else if (eventconn) {
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EVENT_CONNECTION_BINDINGS, "Edit Controller Bindings");
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EVENT_CONNECTION_PROPERTIES, "Properties");
	} else if (noteconn)
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_NOTE_CONNECTION_PROPERTIES, "Properties");

	if (audioconn)
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_AUDIO_CONNECTION_PARAMETERS, "Audio Parameters");
	if (midiconn)
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_MIDI_CONNECTION_PARAMETERS, "MIDI Parameters");
	if (eventconn)
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EVENT_CONNECTION_PARAMETERS, "Controller Parameters");
	if (noteconn)
		menu.AppendMenu(MF_STRING, (UINT_PTR)ID_NOTE_CONNECTION_PARAMETERS, "Note Parameters");

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	if (audioconn != 0) {
		menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_create(mainframe), MF_BYCOMMAND);
	}
}

void CMachineView::ShowBackgroundContext(POINT pt) {

	// må nullstille connectionen sånn at vi ikke inserter
	graphCtrl.UnselectConnection();
	
	CMenu menu; 
	menu.CreatePopupMenu();

	int machinemenusidx = menu.GetMenuItemCount(); /// added below after setting accelerators

	//int editFlags = getEditFlags();
	//bool songInClipboard = (editFlags & EDIT_PASTE) != 0;
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_CUT, "Cut");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_COPY, "Copy");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_PASTE, "Paste");
	//menu.AppendMenu(MF_STRING, (UINT_PTR)ID_EDIT_PASTE_NO_DATA, "Paste Machine(s) Only");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_CREATE_GROUP, "Create New Group");
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_IMPORT_SONG, "Import Song...");
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_UNMUTE_ALL, "Unmute All Machines");

	if (configuration->getShowAccelerators()) buze_main_frame_add_menu_keys(mainframe, "machineview", menu);

	menu.InsertMenu(machinemenusidx, MF_BYPOSITION|MF_POPUP, (UINT_PTR)buze_main_frame_get_plugin_menu_create(mainframe), "New");

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);
	menu.RemoveMenu((UINT_PTR)buze_main_frame_get_plugin_menu_create(mainframe), MF_BYCOMMAND);
}

LRESULT CMachineView::OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	/*
	if (dirtyVisibleMachines || dirtyBackground || dirtyMachines || dirtyOffscreenBitmap) return 0;

	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	// zero volume level if right clicking while setting volume
	if (moveType == MachineViewMoveVolumeSlider) {
		moveFromPoint = pt;
		resetVolumeSlider();
	} else
	if (moveType == MachineViewMoveNothing) {
		// single rclick machine = select machine
		RECT rcPlugin;
		zzub_plugin_t* propMachine = getMachineAtPt(pt, &rcPlugin);

		if (propMachine != 0) {
			// if r-click on non-selected machine, clear selection and show context for one machine, otherwise show a group context
			if (!isSelectedMachine(propMachine)) {
				ClearSelectedMachines();
				selectMachine(propMachine);
			}
		}
	}
*/
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// MOUSE HELPERS
// ---------------------------------------------------------------------------------------------------------------
/*
void CMachineView::resetVolumeSlider()
{
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(selectedConnection.to_plugin, selectedConnection.from_plugin, zzub_connection_type_audio);
	assert(audioconn != 0);
	
	zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(audioconn);
	zzub_parameter_t* volparam = zzub_plugin_get_parameter(connplug, 2, 0, 0);
	assert(volparam != 0);
	
	initialVolume = zzub_parameter_get_value_default(volparam);
	
	zzub_audio_connection_set_amp(connplug, initialVolume, false);
	
	invalidateConnectionVolume();
}

*/
// ---------------------------------------------------------------------------------------------------------------
// PAINTING
// ---------------------------------------------------------------------------------------------------------------

void CMachineView::BindGraphControl() {

	graphCtrl.nodes.clear();
	graphCtrl.edges.clear();

	// only bind plugins and layers on the current layer

	// get layers in the current layer

	zzub_plugin_group_iterator_t* groupit = zzub_player_get_plugin_group_iterator(player, currentlayer);
	while (zzub_plugin_group_iterator_valid(groupit)) {
		zzub_plugin_group_t* plugingroup = zzub_plugin_group_iterator_current(groupit);
		int layer_node_id = plugingroup_id_first + zzub_plugin_group_get_id(plugingroup);
		CGraphNode* node = graphCtrl.AddNode(layer_node_id, zzub_plugin_group_get_name(plugingroup), zzub_plugin_group_get_position_x(plugingroup), zzub_plugin_group_get_position_y(plugingroup), node_container);
		node->bypassed = false;
		node->muted = false;
		node->minimized = false;

		zzub_plugin_group_iterator_next(groupit);
	}
	zzub_plugin_group_iterator_destroy(groupit);

	// get plugins on the current layer (= has no layer = must enumerate all plugins and exclude by layer)
	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		zzub_plugin_group_t* pluginlayer = zzub_plugin_get_plugin_group(plugin);

		if (!buze_document_get_plugin_non_song(document, plugin) && pluginlayer == currentlayer) {

			nodetype type = GetNodeType(plugin);
			CGraphNode* node = graphCtrl.AddNode(zzub_plugin_get_id(plugin), zzub_plugin_get_name(plugin), zzub_plugin_get_position_x(plugin), zzub_plugin_get_position_y(plugin), type);
			node->bypassed = zzub_plugin_get_bypass(plugin) != 0;
			node->muted = zzub_plugin_get_mute(plugin) != 0;
			node->minimized = zzub_plugin_get_minimize(plugin) != 0;
			node->led = (zzub_plugin_get_last_audio_result(plugin) != 0) || (zzub_plugin_get_last_midi_result(plugin) != 0);

			for (int j = 0; j < zzub_plugin_get_input_connection_count(plugin); j++) {
				zzub_connection_t* conn = zzub_plugin_get_input_connection(plugin, j);
				zzub_plugin_t* from_plugin = zzub_connection_get_from_plugin(conn);
				zzub_plugin_group_t* from_layer = zzub_plugin_get_plugin_group(from_plugin);
				zzub_plugin_group_t* from_child_layer = zzub_plugin_group_get_child_group(currentlayer, from_layer);
				if (buze_document_get_plugin_non_song(document, from_plugin)) {
					// this input is hidden, try to find visible inputs further up the chain
					std::vector<zzub_plugin_t*> hiddenConnections; // visible plugins connected through hidden plugins
					FindVisibleInputs(document, from_plugin, hiddenConnections);
					// TODO: add hidden edges
				} else
				if (from_layer == currentlayer) {
					// connected on the same layer
					edgetype etype = GetEdgeType((zzub_connection_type)zzub_connection_get_type(conn));
					float volume = get_scaled_amp(conn);
					graphCtrl.AddEdge(0, zzub_plugin_get_id(plugin), zzub_plugin_get_id(from_plugin), etype, volume, DescribeConnection(conn));
				} else
				if (from_child_layer != 0) {
					// connected to a plugin on a child layer
					int layer_node_id = plugingroup_id_first + zzub_plugin_group_get_id(from_child_layer);
					edgetype etype = GetEdgeType((zzub_connection_type)zzub_connection_get_type(conn));
					float volume = get_scaled_amp(conn);
					CGraphEdge* edge = graphCtrl.AddEdge(0, zzub_plugin_get_id(plugin), layer_node_id, etype, volume, DescribeConnection(conn));
					edge->to_user_id = zzub_plugin_get_id(plugin);
					edge->from_user_id = zzub_plugin_get_id(from_plugin);
					
					zzub_plugin_t* outplugin = zzub_plugin_group_get_output_plugin(from_layer);
					if (outplugin != from_plugin) {
						edge->is_indirect = true;
					}
				}
			}

			for (int j = 0; j < zzub_plugin_get_output_connection_count(plugin); j++) {
				zzub_connection_t* conn = zzub_plugin_get_output_connection(plugin, j);
				zzub_plugin_t* to_plugin = zzub_connection_get_to_plugin(conn);
				zzub_plugin_group_t* to_layer = zzub_plugin_get_plugin_group(to_plugin);
				zzub_plugin_group_t* to_child_layer = zzub_plugin_group_get_child_group(currentlayer, to_layer);
				if (buze_document_get_plugin_non_song(document, to_plugin)) {
					// hidden
				} else
				if (to_child_layer != 0) {
					// connected from plugin on a different layer
					int layer_node_id = plugingroup_id_first + zzub_plugin_group_get_id(to_child_layer);
					edgetype etype = GetEdgeType((zzub_connection_type)zzub_connection_get_type(conn));
					float volume = get_scaled_amp(conn);
					CGraphEdge* edge = graphCtrl.AddEdge(0, layer_node_id, zzub_plugin_get_id(plugin), etype, volume, DescribeConnection(conn));
					edge->to_user_id = zzub_plugin_get_id(to_plugin);
					edge->from_user_id = zzub_plugin_get_id(plugin);

					zzub_plugin_t* inplugin = zzub_plugin_group_get_input_plugin(to_layer);
					if (inplugin != to_plugin) {
						edge->is_indirect = true;
					}
				}
			}
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);
}

LRESULT CMachineView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if (dirtyVisibleMachines) {
		BindGraphControl();
		dirtyVisibleMachines = false;
	}

	return DefWindowProc();
}

LRESULT CMachineView::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return 1;
}

// ---------------------------------------------------------------------------------------------------------------
// GETXXX
// ---------------------------------------------------------------------------------------------------------------
/*


bool CMachineView::getConnectionTextRect(CGraphNodePair const& conn, RECT* rc) {
	RECT cRect;
	getConnectionRect(conn, &cRect);
	// these numbers aren't exact, just tried to cover it.
	// still getting trails now and then.
	rc->left = cRect.right;// + (scale*24);
	rc->top = cRect.top;// - 8;
	rc->right = cRect.right + (scale*125);
	rc->bottom = cRect.bottom;
	return true;
}

*/
void CMachineView::BindSong() {
	graphCtrl.view_offset_x = zzub_player_get_machineview_offset_x(player);
	graphCtrl.view_offset_y = zzub_player_get_machineview_offset_y(player);

	graphCtrl.InvalidateMachines();
}

void CMachineView::InvalidateStatus() {
	if (dirtyVisibleMachines || graphCtrl.dirtyBackground || graphCtrl.dirtyMachines || graphCtrl.dirtyOffscreenBitmap) return ;

	graphCtrl.dirtyStatus = false;

	for (std::vector<boost::shared_ptr<CGraphNode> >::iterator i = graphCtrl.nodes.begin(); i != graphCtrl.nodes.end(); ++i) {
		if ((*i)->type == node_container) {
			// TODO: get led state from layer output
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, (*i)->id);

			//std::map<int, bool>::iterator ledState = graphCtrl.ledStates.find((*i)->id);

			bool led = (zzub_plugin_get_last_audio_result(plugin) != 0) || (zzub_plugin_get_last_midi_result(plugin) != 0);

			if (led != (*i)->led) {
				RECT rc;
				(*i)->led = led;
				graphCtrl.GetStatusRect(i->get(), &rc);
				graphCtrl.InvalidateRect(&rc, FALSE);
				graphCtrl.dirtyStatus = true;
			}
		}
	}
}


void CMachineView::InvalidateMachine(zzub_plugin_t* plugin) {
	CGraphNode* node = graphCtrl.GetNode(zzub_plugin_get_id(plugin));
	if (node) {
		node->bypassed = zzub_plugin_get_bypass(plugin) != 0;
		node->muted = zzub_plugin_get_mute(plugin) != 0;
		node->minimized = zzub_plugin_get_minimize(plugin) != 0;
		RECT rc;
		graphCtrl.GetMachineRect(node, &rc);
		graphCtrl.InvalidateRect(&rc, FALSE);
		graphCtrl.dirtyMachines = true;
	}

}

#if 0

// ---------------------------------------------------------------------------------------------------------------
// DRAWING METHODS
// ---------------------------------------------------------------------------------------------------------------


CBrush& CMachineView::getMachinePanningBrush(zzub_plugin_t* machine) {
	if ((zzub_plugin_get_flags(machine) & zzub_plugin_flag_is_root) != 0) {
		assert(false);
		return generatorPanBrush;
	} else
	if ((zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) & IS_CONTROLLER_PLUGIN_FLAGS) {
		assert(false);
		return generatorPanBrush;
	} else
	if ((zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) & IS_EFFECT_PLUGIN_FLAGS) {
		return effectPanBrush;
	} else
	if ((zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) == IS_GENERATOR_PLUGIN_FLAGS) {
		return generatorPanBrush;
	} else // fallback to generator
		return generatorPanBrush;
}

void CMachineView::getPanSliderRect(CGraphNodePair const& connpair, RECT* rc) {
/*	RECT mRect;
	getMachineRect(conn->plugin_in, &mRect);

	int pan = 0x4000;
	for (size_t i=0; i<conn->plugin_out->getConnections(); i++) {
		if (conn->plugin_out->getConnection(i)==conn) {
			pan = conn->plugin_out->getParameter(0, i, 1);
			break;
		}
	}

	// TODO: bruk getParameter istedet for conn->pan
	float delta=(float)(mRect.right-mRect.left-4-8) / (float)0x8000;
	int pixelPan=(float)pan*delta;

	int left=mRect.left+2 + pixelPan;
	SetRect(rc, left, mRect.bottom-(10.0*scale), left+(8.0*scale), mRect.bottom-2);
	*/
}

#endif

std::string CMachineView::DescribeConnection(zzub_connection_t* conn) {
	std::stringstream s;
	int first_input, first_output, input_count, output_count;
	zzub_connection_binding_iterator_t* bindings;

	zzub_plugin_t* to_plugin = zzub_connection_get_to_plugin(conn);
	zzub_plugin_t* from_plugin = zzub_connection_get_from_plugin(conn);

	switch (zzub_connection_get_type(conn)) {
		case zzub_connection_type_audio:
			first_input = zzub_connection_get_first_input(conn);
			first_output = zzub_connection_get_first_output(conn);
			input_count = zzub_connection_get_input_count(conn);
			output_count = zzub_connection_get_output_count(conn);

			s << first_output;
			if (output_count > 1) s << "-" << (first_output + output_count - 1);
			s << " » " << first_input;
			if (input_count > 1) s << "-" << (first_input + input_count - 1);

			return s.str();
		case zzub_connection_type_event:
			bindings = zzub_connection_get_event_binding_iterator(conn);
			while (zzub_connection_binding_iterator_valid(bindings)) {
				zzub_connection_binding_t* binding = zzub_connection_binding_iterator_current(bindings);
				
				int sourceindex = zzub_connection_binding_get_source_column(binding);
				zzub_parameter_t* sourceparam = zzub_plugin_get_parameter(from_plugin, 3, 0, sourceindex);
				s << zzub_parameter_get_name(sourceparam) << "->";

				int targetgroup = zzub_connection_binding_get_target_group(binding);
				int targettrack = zzub_connection_binding_get_target_track(binding);
				int targetcolumn = zzub_connection_binding_get_target_column(binding);

				zzub_parameter_t* targetparam = zzub_plugin_get_parameter(to_plugin, targetgroup, targettrack, targetcolumn);
				s << zzub_parameter_get_name(targetparam) << endl;

				zzub_connection_binding_iterator_next(bindings);
			}
			zzub_connection_binding_iterator_destroy(bindings);
			return s.str();
		case zzub_connection_type_note:
			return "note";
		case zzub_connection_type_midi:
			s << zzub_connection_get_midi_device(conn);
			return s.str();
		default:
			return "";
	}
}

// ---------------------------------------------------------------------------------------------------------------
// CUT / COPY / PASTE
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	OnCopy(0, 0, 0, bHandled);
	OnDelete(0, 0, 0, bHandled);
	return 0;
}

int CMachineView::SaveSelection(std::string fileName, const std::vector<zzub_plugin_t*>& selectedplugins) {
	std::vector<zzub_plugin_t*> plugins = selectedplugins;
	std::vector<zzub_plugin_t*> connplugs;

	// add connection plugins between selected plugins
	for (std::vector<zzub_plugin_t*>::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		int conncount = zzub_plugin_get_input_connection_count(*i);
		for (int j = 0; j < conncount; j++) {
			zzub_connection_t* conn = zzub_plugin_get_input_connection(*i, j);
			zzub_plugin_t* fromplug = zzub_connection_get_from_plugin(conn);
			zzub_plugin_t* toplug = zzub_connection_get_to_plugin(conn);
			zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(conn);

			if (true
				&& std::find(plugins.begin(), plugins.end(), fromplug) != plugins.end()
				&& std::find(plugins.begin(), plugins.end(), toplug) != plugins.end()
				&& std::find(connplugs.begin(), connplugs.end(), connplug) == connplugs.end()
			) {
				connplugs.push_back(connplug);
			}
		}
	}

	plugins.insert(plugins.end(), connplugs.begin(), connplugs.end());

	return zzub_player_save_armz(player, fileName.c_str(), (const zzub_plugin_t**)&plugins.front(), (int)plugins.size(), currentlayer);
}

void CMachineView::GetExpandedGroup(zzub_plugin_group_t* plugingroup, std::vector<zzub_plugin_t*>* result) {
	// add plugins in group
	zzub_plugin_iterator_t* plugit = zzub_plugin_group_get_plugins(plugingroup);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);

		int flags = zzub_plugin_get_flags(plugin);
		if ((flags & zzub_plugin_flag_is_connection) == 0 && buze_document_get_plugin_non_song(document, plugin) == 0) {
			result->push_back(plugin);
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	// scan groups in group
	zzub_plugin_group_iterator_t* groupit = zzub_player_get_plugin_group_iterator(player, plugingroup);
	while (zzub_plugin_group_iterator_valid(groupit)) {
		zzub_plugin_group_t* subgroup = zzub_plugin_group_iterator_current(groupit);

		GetExpandedGroup(subgroup, result);

		zzub_plugin_group_iterator_next(groupit);
	}
	zzub_plugin_group_iterator_destroy(groupit);
}

void CMachineView::GetExpandedSelection(std::vector<zzub_plugin_t*>* result) {
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			zzub_plugin_group_t* plugingroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
			GetExpandedGroup(plugingroup, result);
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			int flags = zzub_plugin_get_flags(plugin);
			if ((flags & zzub_plugin_flag_has_group_input) != 0 || (flags & zzub_plugin_flag_has_group_output) != 0) {
				// dont add group in/outs on the root layer
				continue;
			}
			result->push_back(plugin);
		}
	}
}

LRESULT CMachineView::OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	if (graphCtrl.selectedMachines.empty()) return 0;

	TCHAR temppath[MAX_PATH];
	TCHAR tempfilename[MAX_PATH];
	GetTempPath(MAX_PATH, temppath);
	GetTempFileName(temppath, _T("bmvc"), 0, tempfilename);

	std::vector<zzub_plugin_t*> plugins;
	GetExpandedSelection(&plugins);
	SaveSelection(tempfilename, plugins);

	char* buffer;
	int size;
	load_binary(tempfilename, &buffer, &size);
	CopyBinary(m_hWnd, "Buze:ARMZ", buffer, size);
	_unlink(tempfilename);
	delete[] buffer;
	return 0;
}

void CMachineView::OnMakeTemplate() {
	if (graphCtrl.selectedMachines.empty()) return ;

	std::string fileName = buze_main_frame_get_save_filename(mainframe);
	if (fileName.length() == 0) return ;

	std::vector<zzub_plugin_t*> plugins;
	GetExpandedSelection(&plugins);
	SaveSelection(fileName, plugins);
}

LRESULT CMachineView::OnCopyAudioConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
/*	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(selectedConnection.to_plugin, selectedConnection.from_plugin, zzub_connection_type_audio);
	zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(audioconn);
	int volume = zzub_audio_connection_get_amp(connplug);

	connection_is_selected = false;
	
	WORD vals[2] = { volume, volume }; // TODO

	CopyBinary(m_hWnd, "Buze:AudioConnectionParameters", (char*)&vals[0], 4);
*/
	return 0;
}

LRESULT CMachineView::OnPasteAudioConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
/*
	UINT format = RegisterClipboardFormat("Buze:AudioConnectionParameters");
	if (OpenClipboard()) {
		HANDLE hData = GetClipboardData(format);
		if (hData != 0) {
			char* charbuf = (char*)GlobalLock(hData);
			int bufferSize = (int)GlobalSize(hData);
			if ((charbuf != 0) && (bufferSize == 4)) {
				int volume = *(WORD*)&charbuf[0];
				//int panning = *(WORD*)&charbuf[2];

				zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(selectedConnection.to_plugin, selectedConnection.from_plugin, zzub_connection_type_audio);
				zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(audioconn);
				zzub_audio_connection_set_amp(connplug, volume, true);

				connection_is_selected = false;
				invalidateMachines(false);

				zzub_player_history_commit(player, 0, 0, "Paste Audio Connection Parameters");
			}
			GlobalUnlock(hData);
		}
		CloseClipboard();
	}
*/
	return 0;
}

void CMachineView::PasteFromClipboard(int flags, float x, float y) {
	
	UINT format = RegisterClipboardFormat("Buze:ARMZ");
	if (OpenClipboard()) {
		//get the buffer
		HANDLE hData = GetClipboardData(format);
		if (hData != 0) {
			char* buffer = (char*)GlobalLock( hData );
			int buffersize = (int)GlobalSize(hData);

			if (buffer != 0) {
				TCHAR temppath[MAX_PATH];
				TCHAR tempfilename[MAX_PATH+5];
				GetTempPath(MAX_PATH, temppath);
				GetTempFileName(temppath, _T("bmvc"), 0, tempfilename);
				_tcscat(tempfilename, ".armz"); // needs .armz ext for importSong

				save_binary(tempfilename, buffer, buffersize);
				//char error_messages[4096];
				if (zzub_player_load_armz(player, tempfilename, 1, currentlayer) == -1) {
				//if (!buze_document_import_song(document, tempfilename, flags|8, x, y, error_messages, 4096)) {
					MessageBox("Cannot import song from clipboard");
					zzub_player_history_commit(player, 0, 0, "Import Song (partially failed)");
				} else {
					zzub_player_history_commit(player, 0, 0, "Import Song");
				}
				_unlink(tempfilename);
			}
			GlobalUnlock(hData);
		}
		CloseClipboard();
	}
}

LRESULT CMachineView::OnPasteNoData(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	// 1 | 2 | 4 = ignore patterns, sequences, waves
	PasteFromClipboard(1 | 4, 0.05f, 0.05f);
	return 0;
}

LRESULT CMachineView::OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	PasteFromClipboard(0, 0.05f, 0.05f);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// DRAG & DROP
// ---------------------------------------------------------------------------------------------------------------

bool CMachineView::OnDragOver(const POINTL& pt, FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect) { 
	// TODO: maybe check if we are over a connection, a group machine or something
	return true; 
}

void CMachineView::OnDragLeave() { 
}

bool CMachineView::OnDropMachine(std::string uri, std::string instrumentName, int x, int y) {

	RECT rc;
	graphCtrl.GetClientScale(&rc);
	float dx = ((((float)x) / (float)rc.right) - 0.5f ) * 2.0f / graphCtrl.scale;
	float dy = ((((float)y) / (float)rc.bottom) - 0.5f ) * 2.0f / graphCtrl.scale;

	if (uri == "@zzub.org/buze/template") {
		char error_messages[4096];
		bool result = buze_document_import_song(document, instrumentName.c_str(), 8, 0, 0, error_messages, 4096) != 0;
		zzub_player_history_commit(player, 0, 0, "Import Template");
		return result;
	} else {
		zzub_plugin_t* plugin = CreateMachine(uri, instrumentName, dx, dy);
		zzub_player_history_commit(player, 0, 0, "Create Plugin");
		return plugin != 0;
	}
}

// ---------------------------------------------------------------------------------------------------------------
// XXX
// ---------------------------------------------------------------------------------------------------------------

/**
* Calculates 1 pixel measure depending 
* on current zoom and cleint's window size
*/

void CMachineView::CalcPixelSize() {
	RECT rc;
	graphCtrl.GetClientScale(&rc);

	pxSizeX = (1.0f/(float)rc.right * 2.0f) / graphCtrl.scale; 
	pxSizeY = (1.0f/(float)rc.bottom * 2.0f) / graphCtrl.scale; 
}

// ---------------------------------------------------------------------------------------------------------------
// MACHINE MOVING / ALIGNING / NEAREST
// ---------------------------------------------------------------------------------------------------------------

void CMachineView::MoveSelectedMachines(int x, int y){

	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		float ox = node->position.x;//zzub_plugin_get_position_x(selplugin);
		float oy = node->position.y;//zzub_plugin_get_position_y(selplugin);
		if (node->type == node_container) {
			zzub_plugin_group_t* selgroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
			zzub_plugin_group_set_position(selgroup, ox + (float)x * pxSizeX, oy + (float)y * pxSizeY);
		} else {
			zzub_plugin_t* selplugin = zzub_player_get_plugin_by_id(player, node->id);
			zzub_plugin_set_position(selplugin, ox + (float)x * pxSizeX, oy + (float)y * pxSizeY);
		}
		// TODO: When you have a few machines selected and press Ctrl+Shift+UP for 2 seconds,
		//       machines would not redraw until key is unpressed. Don't know how to fix it yet :(
	}
	
	zzub_player_history_commit(player, 0, 0, "Move Machine(s)");
}

LRESULT CMachineView::OnMachineMoveUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(0, -machineMovementPixel);
	return 0;
}

LRESULT CMachineView::OnMachineMoveDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(0, machineMovementPixel);
	return 0;
}

LRESULT CMachineView::OnMachineMoveLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(-machineMovementPixel, 0);
	return 0;
}

LRESULT CMachineView::OnMachineMoveRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(machineMovementPixel, 0);
	return 0;
}

LRESULT CMachineView::OnMachineMoveUpByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(0, -machineMovementStep);
	return 0;
}

LRESULT CMachineView::OnMachineMoveDownByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(0, machineMovementStep);
	return 0;
}

LRESULT CMachineView::OnMachineMoveLeftByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(-machineMovementStep, 0);
	return 0;
}

LRESULT CMachineView::OnMachineMoveRightByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(machineMovementStep, 0);
	return 0;
}

LRESULT CMachineView::OnMachineMoveUpLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(-machineMovementPixel, -machineMovementPixel);
	return 0;
}

LRESULT CMachineView::OnMachineMoveDownLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(-machineMovementPixel, machineMovementPixel);
	return 0;
}

LRESULT CMachineView::OnMachineMoveDownRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(machineMovementPixel, machineMovementPixel);
	return 0;
}

LRESULT CMachineView::OnMachineMoveUpRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(machineMovementPixel, -machineMovementPixel);
	return 0;
}

LRESULT CMachineView::OnMachineMoveUpRightByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(machineMovementStep, -machineMovementStep);
	return 0;
}

LRESULT CMachineView::OnMachineMoveDownRightByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(machineMovementStep, machineMovementStep);
	return 0;
}

LRESULT CMachineView::OnMachineMoveUpLeftByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(-machineMovementStep, -machineMovementStep);
	return 0;
}

LRESULT CMachineView::OnMachineMoveDownLeftByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	MoveSelectedMachines(-machineMovementStep, machineMovementStep);
	return 0;
}

// View does not deal with precision more then 0.001, thus, in few cases we have to round coordinates 
inline float CMachineView::RoundToPixel(float x) {
	return floor(x*100 + pxSizeX )/100;
}

// 0 - up 
// 1 - down
// 2 - left
// 3 - right
float CMachineView::GetNearestMachineLocation(float pos, int direction) {
	int machines_count = zzub_player_get_plugin_count(player);
	
	list<float>::iterator i;

	machinesListX.clear();
	machinesListY.clear();
	
	// filling list with machine's coordinates
	for (std::vector<boost::shared_ptr<CGraphNode> >::iterator j = graphCtrl.nodes.begin(); j != graphCtrl.nodes.end(); ++j) {
		CGraphNode* plugin = j->get();

		float ox = plugin->position.x;
		float oy = plugin->position.y;

		machinesListX.push_back(ox);
		machinesListY.push_back(oy);
	}

	switch(direction) {
		case 0: // UP
			machinesListY.sort(greater<float>( )); // sorting desc			
			i = machinesListY.begin();
			while(i != machinesListY.end()) {
				if (RoundToPixel(*i) < RoundToPixel(pos))
					return *i;
				i++;
			}
		break;
		case 1: // DOWN
			machinesListY.sort(); // sorting asc
			i = machinesListY.begin();
			while(i != machinesListY.end()) {
				if (RoundToPixel(*i) > RoundToPixel(pos))
					return *i;
				i++;
			}
		break;
		case 2: // LEFT
			machinesListX.sort(greater<float>( )); // sorting desc			
			i = machinesListX.begin();
			while(i != machinesListX.end()) {
				if(RoundToPixel(*i) < RoundToPixel(pos))
					return *i;
				i++;
			}
		break;
		case 3: // RIGHT
			machinesListX.sort(); // sorting asc
			list<float>::iterator i = machinesListX.begin();
			while(i != machinesListX.end()) {
				if(RoundToPixel(*i) > RoundToPixel(pos))
					return *i;
				i++;
			}
		break;
	}

	return -3; // since viewport is in range of -1..1, `-3` would mean there is no machines to align to
}

LRESULT CMachineView::OnMachineAlignUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (graphCtrl.GetSelectedMachines() == 0) return 0;

	float Y = 1;

	// if there is more then 1 machine selected - looking for very left machine in the selection
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		float y;
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			zzub_plugin_group_t* plugingroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
			y = zzub_plugin_group_get_position_y(plugingroup);
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			y = zzub_plugin_get_position_y(plugin);
		}

		if(y < Y) Y = y;
	}

	float pos = GetNearestMachineLocation(Y, 0);
	if (pos != -3) {
		int step = (int)RoundToPixel(abs(Y - pos) / pxSizeY);
		MoveSelectedMachines(0, -step);
	}
	return 0;
}

LRESULT CMachineView::OnMachineAlignDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (graphCtrl.GetSelectedMachines() == 0) return 0;

	float Y = -1;

	// if there is more then 1 machine selected - looking for very left machine in the selection
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		float y;
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			zzub_plugin_group_t* plugingroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
			y = zzub_plugin_group_get_position_y(plugingroup);
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			y = zzub_plugin_get_position_y(plugin);
		}
		if(y > Y) Y = y;
	}

	float pos = GetNearestMachineLocation(Y, 1);
	if (pos != -3) {
		int step = (int)RoundToPixel(abs(Y - pos) / pxSizeY);
		MoveSelectedMachines(0, step);
	}
	return 0;
}

LRESULT CMachineView::OnMachineAlignLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	if (graphCtrl.GetSelectedMachines() == 0) return 0;

	float X = 1;

	// if there is more then 1 machine selected - looking for very left machine in the selection
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		float x;
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			zzub_plugin_group_t* plugingroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
			x = zzub_plugin_group_get_position_x(plugingroup);
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			x = zzub_plugin_get_position_x(plugin);
		}
		if(x < X) X = x;
	}

	float pos = GetNearestMachineLocation(X, 2);
	if (pos != -3) {
		int step = (int)RoundToPixel(abs(X - pos) / pxSizeX);
		MoveSelectedMachines(-step, 0);
	}
	return 0;
}

LRESULT CMachineView::OnMachineAlignRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (graphCtrl.GetSelectedMachines() == 0) return 0;

	float X = -1;

	// if there is more then 1 machine selected - looking for very left machine in the selection
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		float x;
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			zzub_plugin_group_t* plugingroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
			x = zzub_plugin_group_get_position_x(plugingroup);
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			x = zzub_plugin_get_position_x(plugin);
		}
		if(x > X) X = x;
	}
	float pos = GetNearestMachineLocation(X, 3);

	if (pos != -3) {
		int step = (int)RoundToPixel(abs(X - pos) / pxSizeX);
		MoveSelectedMachines(step, 0);
	}
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// MINIMIZATION
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnMinimizeMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type != node_container) {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			zzub_plugin_set_minimize(plugin, 1);
		}
	}

	zzub_player_history_commit(player, 0, 0, "Minimize Plugin");
	return 0;
}

LRESULT CMachineView::OnUnMinimizeMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type != node_container) {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
		zzub_plugin_set_minimize(plugin, 0);
		}
	}

	zzub_player_history_commit(player, 0, 0, "Unminimize Plugins");
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// SELECTION
// ---------------------------------------------------------------------------------------------------------------

void CMachineView::SelectMachine(zzub_plugin_t* machine) {
	if (machine == 0) return ;
	CGraphNode* node = graphCtrl.GetNode(zzub_plugin_get_id(machine));
	if (node != 0)
		graphCtrl.SelectMachine(node);
}

void CMachineView::UnselectMachine(zzub_plugin_t* machine) {
	CGraphNode* node = graphCtrl.GetNode(zzub_plugin_get_id(machine));
	if (node != 0)
		graphCtrl.UnselectMachine(node);
}

bool CMachineView::IsSelectedMachine(zzub_plugin_t* machine) {
	return graphCtrl.IsSelectedMachine(zzub_plugin_get_id(machine));
}

int CMachineView::GetSelectedMachines() {
	return (int)graphCtrl.selectedMachines.size();
}

zzub_plugin_t* CMachineView::GetSelectedMachine(int index) {
	if (index < 0 || index >= (int)graphCtrl.selectedMachines.size()) return 0;
	int plugin_id = graphCtrl.selectedMachines[index];
	return zzub_player_get_plugin_by_id(player, plugin_id);//selectedMachines[index];
}

zzub_connection_t* CMachineView::GetEdgeConnection(int to_node_id, int from_node_id, edgetype etype, zzub_connection_type ctype) {
    CGraphEdge* edge = graphCtrl.GetEdge(to_node_id, from_node_id, etype);
	if (edge == 0) return 0;
	zzub_plugin_t* to_plugin = zzub_player_get_plugin_by_id(player, edge->to_user_id);
	zzub_plugin_t* from_plugin = zzub_player_get_plugin_by_id(player, edge->from_user_id);
	return zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, ctype);
}

zzub_connection_t* CMachineView::GetSelectedEdgeConnection(edgetype etype, zzub_connection_type ctype) {
	CGraphNodePair* nodepair = graphCtrl.GetSelectedEdge();
	if (nodepair == 0) return 0;
	return GetEdgeConnection(nodepair->to_node_id, nodepair->from_node_id, etype, ctype);
}

LRESULT CMachineView::OnClearSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	graphCtrl.ClearSelectedMachines();
	return 0;
}

LRESULT CMachineView::OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	for (std::vector<boost::shared_ptr<CGraphNode> >::iterator i = graphCtrl.nodes.begin(); i != graphCtrl.nodes.end(); ++i) {
		graphCtrl.SelectMachine(i->get());
	}
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// CONNECTION MENU
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnGraphConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	int index = wID - ID_CONNECT_GRAPH_FIRST;

	CONNECTMENUITEM* data = &connectOptions[index];
	bool success = false;

	int defaultAmp = configuration->getDefaultAmp();
	zzub_plugin_t* connectTargetMachine = data->to_plugin;
	zzub_plugin_t* draggingMachine = data->from_plugin;
//	zzub_plugin_t* connectTargetMachine = zzub_player_get_plugin_by_id(player, graphCtrl.connectTargetMachine->id);
//	zzub_plugin_t* draggingMachine = zzub_player_get_plugin_by_id(player, graphCtrl.draggingMachine->id);

	// now connect them
	switch (data->type) {
		case zzub_connection_type_audio:
			if (data->first_input != -1)
				success = ConnectMachinesAudio(draggingMachine, connectTargetMachine, defaultAmp, data->first_input, data->inputs, data->first_output, data->outputs);
			else {
				success = ConnectMachinesAudio(draggingMachine, connectTargetMachine, defaultAmp, 0, 0, 0, 0);
				if (success) {
					zzub_connection_t* conn = zzub_plugin_get_input_connection_by_type(data->to_plugin, data->from_plugin, zzub_connection_type_audio);
					assert(conn != 0);
					ShowAudioConnectionDialog(conn);
				}
			}
			break;
		case zzub_connection_type_event:
			success = ConnectMachinesEvent(draggingMachine, connectTargetMachine, data->source_param_index, data->target_group_index, data->target_track_index, data->target_param_index);
			// TODO: SelectConnection(), OnEventConnectionDialog();
			break;
		case zzub_connection_type_note:
			success = ConnectMachinesNote(draggingMachine, connectTargetMachine);
			break;
		case zzub_connection_type_midi:
			success = ConnectMachinesMidi(draggingMachine, connectTargetMachine, data->target_midi_out_device);
			break;
	}

	if (success)
		zzub_player_history_commit(player, 0, 0, "Connect Plugins");
	return 0;
}

void CMachineView::OnGraphConnectContext() {

	connectOptions.clear();

	zzub_plugin_t* connectTargetMachine;
	zzub_plugin_t* draggingMachine;
	if (graphCtrl.connectTargetMachine->type == node_container) {
		// find the layer input machine
		zzub_plugin_group_t* layer = zzub_player_get_plugin_group_by_id(player, graphCtrl.connectTargetMachine->id - plugingroup_id_first);
		connectTargetMachine = zzub_plugin_group_get_input_plugin(layer);
		assert(connectTargetMachine != 0);
	} else {
		connectTargetMachine = zzub_player_get_plugin_by_id(player, graphCtrl.connectTargetMachine->id);
	}

	if (graphCtrl.draggingMachine->type == node_container) {
		zzub_plugin_group_t* layer = zzub_player_get_plugin_group_by_id(player, graphCtrl.draggingMachine->id - plugingroup_id_first);
		draggingMachine = zzub_plugin_group_get_output_plugin(layer);
		assert(draggingMachine != 0);
	} else {
		draggingMachine = zzub_player_get_plugin_by_id(player, graphCtrl.draggingMachine->id);
	}

	CMenuHandle menu;
	menu.CreatePopupMenu();

	bool has_audio_in = (zzub_plugin_get_flags(connectTargetMachine) & zzub_plugin_flag_has_audio_input) != 0;
	bool has_audio_out = (zzub_plugin_get_flags(draggingMachine) & zzub_plugin_flag_has_audio_output) || (zzub_plugin_get_flags(draggingMachine) & zzub_plugin_flag_is_root && !(zzub_plugin_get_flags(connectTargetMachine) & zzub_plugin_flag_has_audio_output));

//	bool has_event_in = zzub_plugin_get_flags(connectTargetMachine) & zzub_plugin_flag_has_event_input;
	bool has_event_out = (zzub_plugin_get_flags(draggingMachine) & zzub_plugin_flag_has_event_output) != 0;

	bool has_note_out = (zzub_plugin_get_flags(draggingMachine) & zzub_plugin_flag_has_note_output) != 0;

	bool has_midi_in = (zzub_plugin_get_flags(connectTargetMachine) & zzub_plugin_flag_has_midi_input) != 0;
	bool has_midi_out = (zzub_plugin_get_flags(draggingMachine) & zzub_plugin_flag_has_midi_output) != 0;

	int id_counter = ID_CONNECT_GRAPH_FIRST;

	if (has_audio_out && has_audio_in) {
		zzub_pluginloader_t* from_info = zzub_plugin_get_pluginloader(draggingMachine);
		zzub_pluginloader_t* to_info = zzub_plugin_get_pluginloader(connectTargetMachine);
        
		int from_channels = zzub_pluginloader_get_output_channel_count(from_info);
		int to_channels = zzub_pluginloader_get_input_channel_count(to_info);
		// this wont work with some buzz machines being initialized to mono but can accept stereo connections
		//int from_channels = zzub_plugin_get_output_channel_count(draggingMachine);
		//int to_channels = zzub_plugin_get_input_channel_count(connectTargetMachine);

		if (from_channels * to_channels > 16) {
			menu.AppendMenu(MF_STRING, (UINT_PTR)id_counter++, "Select Audio Channels...");
			CONNECTMENUITEM data;
			data.from_plugin = draggingMachine;
			data.to_plugin = connectTargetMachine;
			data.type = zzub_connection_type_audio;
			data.first_input = -1;
			data.first_output = -1;
			data.inputs = -1;
			data.outputs = -1;
			data.flags = 0;
			connectOptions.push_back(data);
		} else {

			// suggest stereo
			for (int i = 0; i < from_channels - 1; i++) {
				std::string fromname0 = GetOutputChannelName(draggingMachine, i);
				std::string fromname1 = GetOutputChannelName(draggingMachine, i + 1);

				for (int j = 0; j < to_channels - 1; j++) {
					std::string toname0 = GetInputChannelName(connectTargetMachine, j);
					std::string toname1 = GetInputChannelName(connectTargetMachine, j + 1);
					std::stringstream strm;
					strm << fromname0 << "+" << fromname1 << " to " << toname0 << "+" << toname1 << " (Stereo)";
					menu.AppendMenu(MF_STRING, (UINT_PTR)id_counter++, strm.str().c_str());
					CONNECTMENUITEM data;
					data.from_plugin = draggingMachine;
					data.to_plugin = connectTargetMachine;
					data.type = zzub_connection_type_audio;
					data.first_input = j;
					data.first_output = i;
					data.inputs = 2;
					data.outputs = 2;
					data.flags = 0;
					connectOptions.push_back(data);
					//SetMenuItemData(menu, menu.GetMenuItemCount() - 1, data);
				}
			}

			// suggest mono->stereo
			for (int i = 0; i < from_channels; i++) {
				std::string fromname0 = GetOutputChannelName(draggingMachine, i);
				for (int j = 0; j < to_channels - 1; j++) {
					std::string toname0 = GetInputChannelName(connectTargetMachine, j);
					std::string toname1 = GetInputChannelName(connectTargetMachine, j + 1);
					std::stringstream strm;
					strm << fromname0 << " to " << toname0 << "+" << toname1 << " (Mono to Stereo)";

					//strm << "Mono Out " << i << " to Stereo In " << j << "-" << (j + 1);
					menu.AppendMenu(MF_STRING, (UINT_PTR)id_counter++, strm.str().c_str());
					CONNECTMENUITEM data;
					data.from_plugin = draggingMachine;
					data.to_plugin = connectTargetMachine;
					data.type = zzub_connection_type_audio;
					data.first_input = j;
					data.first_output = i;
					data.inputs = 2;
					data.outputs = 1;
					data.flags = 1;
					connectOptions.push_back(data);
					//SetMenuItemData(menu, menu.GetMenuItemCount() - 1, data);
				}
			}

			// suggest mono
			for (int i = 0; i < from_channels; i++) {
				std::string fromname0 = GetOutputChannelName(draggingMachine, i);
				for (int j = 0; j < to_channels; j++) {
					std::string toname0 = GetInputChannelName(connectTargetMachine, j);
					std::stringstream strm;
					strm << fromname0 << " to " << toname0 << " (Mono)";
					menu.AppendMenu(MF_STRING, (UINT_PTR)id_counter++, strm.str().c_str());
					CONNECTMENUITEM data;
					data.from_plugin = draggingMachine;
					data.to_plugin = connectTargetMachine;
					data.type = zzub_connection_type_audio;
					data.first_input = j;
					data.first_output = i;
					data.inputs = 1;
					data.outputs = 1;
					data.flags = 0;
					connectOptions.push_back(data);
					//SetMenuItemData(menu, menu.GetMenuItemCount() - 1, data);
				}
			}
		}

/*		menu.AppendMenu(MF_STRING, (UINT_PTR)id_counter++, "Audio Connection");
		CONNECTMENUITEM* data = new CONNECTMENUITEM();
		data->type = zzub_connection_type_audio;
		SetMenuItemData(menu, menu.GetMenuItemCount() - 1, data);*/
	}

	if (has_midi_out && has_midi_in) {
		// fetch possible midi output devices from connectTargetMachine
		for (int i = 0; i < zzub_plugin_get_midi_output_device_count(connectTargetMachine); i++) {
			std::stringstream strm;
			std::string name = zzub_plugin_get_midi_output_device(connectTargetMachine, i);
			std::map<string, string>::iterator alias = midiAliases.find(name);
			strm << "MIDI: ";
			if(alias != midiAliases.end()) {
				printf("alias of \"%s\" found: \"%s\"\n", name.c_str(), alias->second.c_str());
				strm << alias->second;
			} else
				strm << name;
			menu.AppendMenu(MF_STRING, (UINT_PTR)id_counter++, strm.str().c_str());

			CONNECTMENUITEM data;
			data.from_plugin = draggingMachine;
			data.to_plugin = connectTargetMachine;
			data.type = zzub_connection_type_midi;
			data.target_midi_out_device = name;
			connectOptions.push_back(data);
			//SetMenuItemData(menu, menu.GetMenuItemCount() - 1, data);
		}
	}

	if (has_event_out) {

		// allow creating unbound event connections when there are no controller params on the source plugin
		menu.AppendMenu(MF_STRING, (UINT_PTR)id_counter++, "Controller Connection");
		CONNECTMENUITEM data;
		data.from_plugin = draggingMachine;
		data.to_plugin = connectTargetMachine;
		data.type = zzub_connection_type_event;
		data.source_param_index = -1;
		connectOptions.push_back(data);
		//SetMenuItemData(menu, menu.GetMenuItemCount() - 1, data);
	}

	if (has_note_out) {

		int notegroup = zzub_plugin_get_note_group(connectTargetMachine);
		if (notegroup != -1) {
			menu.AppendMenu(MF_STRING, (UINT_PTR)id_counter++, "Note Connection");
			CONNECTMENUITEM data;// = new CONNECTMENUITEM();
			data.from_plugin = draggingMachine;
			data.to_plugin = connectTargetMachine;
			data.type = zzub_connection_type_note;
			data.source_param_index = -1;
			connectOptions.push_back(data);
			//SetMenuItemData(menu, menu.GetMenuItemCount() - 1, data);
		}
	}

	graphCtrl.connectTargetMenu.SetMenu(m_hWnd, menu.m_hMenu);

	int halfHeight = (int)((graphCtrl.GetMachineHeight(graphCtrl.connectTargetMachine)/2) * graphCtrl.scale);
	RECT rcPlugin;
	graphCtrl.GetMachineRect(graphCtrl.connectTargetMachine, &rcPlugin);

	graphCtrl.connectTargetMenu.m_selectedIndex = -1;
	graphCtrl.connectTargetMenu.SetPosition(rcPlugin.left, rcPlugin.top + halfHeight);

}

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DELETION / INSERTION / CONNECTION / SMART FUNCTIONS
// ---------------------------------------------------------------------------------------------------------------


bool CMachineView::ConnectMachinesAudio(zzub_plugin_t* output, zzub_plugin_t* input, int amp, int first_input, int input_count, int first_output, int output_count) {
	zzub_connection_t* conn = zzub_plugin_create_audio_connection(input, output, first_input, input_count, first_output, output_count);
	if (conn == 0) return false;

	zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(conn);
	zzub_audio_connection_set_amp(connplug, amp, true);
	return true;
}

bool CMachineView::ConnectMachinesEvent(zzub_plugin_t* output, zzub_plugin_t* input, int source_param_index, int target_group_index, int target_track_index, int target_param_index) {
	// check for an existing event connection; on duplicate event connections, only add the new binding
	zzub_connection_t* eventconn = zzub_plugin_get_input_connection_by_type(input, output, zzub_connection_type_event);
	if (eventconn == 0 && (eventconn = zzub_plugin_create_event_connection(input, output)) == 0) return false;

	if (source_param_index != -1)
		zzub_connection_add_event_connection_binding(eventconn, source_param_index, target_group_index, target_track_index, target_param_index);

	return true;
}

bool CMachineView::ConnectMachinesNote(zzub_plugin_t* output, zzub_plugin_t* input) {
	zzub_connection_t* conn = zzub_plugin_create_note_connection(input, output);
	if (conn == 0) return false;
	return true;
}

bool CMachineView::ConnectMachinesMidi(zzub_plugin_t* output, zzub_plugin_t* input, std::string const& midiOutDevice) {
	zzub_connection_t* conn = zzub_plugin_create_midi_connection(input, output, midiOutDevice.c_str());
	if (conn == 0) return false;
	return true;
}

bool CMachineView::DisconnectMachines(zzub_plugin_t* output, zzub_plugin_t* input, zzub_connection_type type) {
	zzub_connection_t* conn = zzub_plugin_get_input_connection_by_type(input, output, type);
	assert(conn != 0);
	zzub_connection_destroy(conn);
	return true;
}


LRESULT CMachineView::OnDisconnectMachinesAudio(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CGraphNodePair* edge = graphCtrl.GetSelectedEdge();
	if (edge) {
		if (DisconnectEdge(edge->to_node_id, edge->from_node_id, edge_audio, zzub_connection_type_audio))
			zzub_player_history_commit(player, 0, 0, "Disconnect Audio Connection");
	}
	return 0;
}

LRESULT CMachineView::OnDisconnectMachinesMidi(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CGraphNodePair* edge = graphCtrl.GetSelectedEdge();
	if (edge) {
		if (DisconnectEdge(edge->to_node_id, edge->from_node_id, edge_midi, zzub_connection_type_midi))
			zzub_player_history_commit(player, 0, 0, "Disconnect MIDI Connection");
	}
	return 0;
}

LRESULT CMachineView::OnDisconnectMachinesEvent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CGraphNodePair* edge = graphCtrl.GetSelectedEdge();
	if (edge) {
		if (DisconnectEdge(edge->to_node_id, edge->from_node_id, edge_event, zzub_connection_type_event))
			zzub_player_history_commit(player, 0, 0, "Disconnect Event Connection");
	}

	return 0;
}

LRESULT CMachineView::OnDisconnectMachinesNote(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CGraphNodePair* edge = graphCtrl.GetSelectedEdge();
	if (edge) {
		if (DisconnectEdge(edge->to_node_id, edge->from_node_id, edge_note, zzub_connection_type_note))
			zzub_player_history_commit(player, 0, 0, "Disconnect Note Connection");
	}

	return 0;
}


LRESULT CMachineView::OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	DeleteSelectedMachines();
	zzub_player_history_commit(player, 0, 0, "Delete Plugin(s)");
	return 0;
}

LRESULT CMachineView::OnDeleteAndRestoreConnections(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	DeleteSelectedMachinesAndRestoreConnections();
	zzub_player_history_commit(player, 0, 0, "Smart Delete Plugin(s)");
	return 0;
}

void CMachineView::DeleteSelectedMachines() {
	std::vector<zzub_plugin_t*> plugins;
	std::vector<zzub_plugin_group_t*> groups;

	for (int i = 0; i < graphCtrl.GetSelectedMachines(); ++i) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			groups.push_back(zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first));
		} else {
			plugins.push_back(zzub_player_get_plugin_by_id(player, node->id));
		}
	}

	graphCtrl.ClearSelectedMachines();

	for (std::vector<zzub_plugin_group_t*>::iterator i = groups.begin(); i != groups.end(); ++i) {
		zzub_plugin_group_destroy(*i);
	}

	for (std::vector<zzub_plugin_t*>::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		zzub_plugin_destroy(*i);
	}
}

// This is designed for audio only
void CMachineView::DeleteSelectedMachinesAndRestoreConnections() {
	std::vector<zzub_plugin_t*> plugins;
	std::vector<zzub_plugin_group_t*> groups;

	for (int i = 0; i < graphCtrl.GetSelectedMachines(); ++i) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			groups.push_back(zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first));
		} else {
			plugins.push_back(zzub_player_get_plugin_by_id(player, node->id));
		}
	}

	// TODO: something smart about the groups

	for (std::vector<zzub_plugin_t*>::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		zzub_plugin_t* plugin = *i;
		buze_document_delete_plugin_smart(document, plugin);
	}
}

LRESULT CMachineView::OnCreateMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	int index = wID - ID_CREATEMACHINECOMMANDS;

	if (graphCtrl.GetSelectedEdge() != 0)
		CreateMachine(index, machine_createtype_insert);
	else
		CreateMachine(index, machine_createtype_create);

	return 0;
}

LRESULT CMachineView::OnInsertBeforeMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	int index = wID - ID_INSERTBEFOREMACHINECOMMANDS;
	CreateMachine(index, machine_createtype_insertbefore);
	return 0;
}

LRESULT CMachineView::OnInsertAfterMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	int index = wID - ID_INSERTAFTERMACHINECOMMANDS;
	CreateMachine(index, machine_createtype_insertafter);
	return 0;
}

LRESULT CMachineView::OnReplaceMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	int index = wID - ID_REPLACEMACHINECOMMANDS;
	CreateMachine(index, machine_createtype_replace);
	return 0;
}

zzub_plugin_t* CMachineView::CreateMachine(std::string const& uri, std::string const& instrumentName, float x, float y) {
	zzub_plugin_t* plugin = buze_document_create_plugin(document, uri.c_str(), instrumentName.c_str(), x, y, currentlayer);
	return plugin;
}

void CMachineView::CreateMachine(int index, int type) {
	buze_plugin_index_item_t* mi = buze_document_get_plugin_index_item_by_index(document, index);
	//MachineItem* mi = (MachineItem*)document->machineIndex.root.getItemByIndex(index);
	if (!mi) return;

	string uri = buze_plugin_index_item_get_filename(mi); //mi->fileName;
	std::string instrumentName = buze_plugin_index_item_get_instrumentname(mi);
	bool is_template = uri == "@zzub.org/buze/template";

	if (!is_template) {
		const zzub_pluginloader_t* loader = zzub_player_get_pluginloader_by_name(player, uri.c_str());
		if (!loader) return;
	}

	if (is_template) {
		char error_messages[4096];
		buze_document_import_song(document, instrumentName.c_str(), 8, 0, 0, error_messages, 4096);
		zzub_player_history_commit(player, 0, 0, "Import Template");
	} else {
		// Insert machine at arrow
		if (type == machine_createtype_insert) {
			// NOTE/TODO: supports audio only
			CGraphEdge* edge = graphCtrl.GetSelectedEdge(edge_audio);
			if (edge != 0) {
				zzub_plugin_t* to_plugin = zzub_player_get_plugin_by_id(player, edge->to_user_id);
				zzub_plugin_t* from_plugin = zzub_player_get_plugin_by_id(player, edge->from_user_id);
				buze_document_create_plugin_between(document, to_plugin, from_plugin, uri.c_str(), instrumentName.c_str());
				zzub_player_history_commit(player, 0, 0, "Insert Plugin");
			}
		} else
		// Insert before each selected machine
		if (type == machine_createtype_insertbefore) {
			for (int i = 0; i < graphCtrl.GetSelectedMachines(); ++i) {
				CGraphNode* node = graphCtrl.GetSelectedMachine(i);
				zzub_plugin_t* plugin;
				if (node->type == node_container) {
					zzub_plugin_group_t* plugingroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
					plugin = zzub_plugin_group_get_input_plugin(plugingroup);
				} else {
					plugin = zzub_player_get_plugin_by_id(player, node->id);
				}
				zzub_plugin_t* new_plugin = buze_document_create_plugin_before(document, plugin, uri.c_str(), instrumentName.c_str());
				if (node->type == node_container)
					zzub_plugin_set_plugin_group(new_plugin, currentlayer);
			}

			zzub_player_history_commit(player, 0, 0, "Insert Plugin Before");
		} else
		// Insert after each selected machine
		if (type == machine_createtype_insertafter) {
			for (int i = 0; i < graphCtrl.GetSelectedMachines(); ++i) {
				CGraphNode* node = graphCtrl.GetSelectedMachine(i);
				zzub_plugin_t* plugin;
				if (node->type == node_container) {
					zzub_plugin_group_t* plugingroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
					plugin = zzub_plugin_group_get_output_plugin(plugingroup);
				} else {
					plugin = zzub_player_get_plugin_by_id(player, node->id);
				}
				zzub_plugin_t* new_plugin = buze_document_create_plugin_after(document, plugin, uri.c_str(), instrumentName.c_str());
				if (node->type == node_container)
					zzub_plugin_set_plugin_group(new_plugin, currentlayer);
			}

			zzub_player_history_commit(player, 0, 0, "Insert Plugin After");
		} else
		// Replace each selected machine
		if (type == machine_createtype_replace) {
			std::vector<zzub_plugin_t*> new_plugins;

			for (int i = 0; i < graphCtrl.GetSelectedMachines(); ++i) {
				CGraphNode* node = graphCtrl.GetSelectedMachine(i);
				zzub_plugin_t* plugin;
				if (node->type == node_container) {
					//zzub_plugin_group_t* plugingroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
					//plugin = zzub_plugin_group_get_input_plugin(plugingroup);
					// TODO: replace group?
					continue;
				} else {
					plugin = zzub_player_get_plugin_by_id(player, node->id);
				}
				zzub_plugin_t* new_plugin = buze_document_create_plugin_replace(document, plugin, uri.c_str(), instrumentName.c_str());
				new_plugins.push_back(new_plugin);
			}

			DeleteSelectedMachines();
			
			// reselect
			for (std::vector<zzub_plugin_t*>::iterator i = new_plugins.begin(); i != new_plugins.end(); ++i) {
				SelectMachine(*i);
			}

			zzub_player_history_commit(player, 0, 0, "Replace Plugin");
		} else
		// Create a regular plugin
		if (type == machine_createtype_create) {
			zzub_plugin_t* plugin = CreateMachine(uri, instrumentName, createMachinePosX, createMachinePosY);
			zzub_player_history_commit(player, 0, 0, "Create Plugin");
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------
// MIXDOWN
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnMixdownFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	zzub_plugin_t* recordplugin = GetSelectedMachine(0);
	if (!recordplugin) return 0;

	std::string filename = GetMixdownFileName("", m_hWnd);
	if (filename.empty()) return 0;

	zzub_player_history_enable(player, 0);

	buze_application_t* application = buze_main_frame_get_application(mainframe);
	mixdownhelper md(application, player, recordplugin);
	bool success = md.init_file_recorder(filename);

	if (success) {
		buze_application_show_wait_window(application);
		buze_application_set_wait_text(application, "Recording...");

		zzub_audiodriver_t* driver = buze_application_get_audio_driver(application);
		int samplerate = zzub_audiodriver_get_samplerate(driver);

		success = md.mixdown(samplerate);

		buze_application_hide_wait_window(application, m_hWnd);
	}

	md.uninit();

	zzub_player_history_commit(player, 0, 0, "Mixdown To File");
	zzub_player_history_enable(player, 1);

	if (!success)
		MessageBox("Mixdown fail. Check for looping patterns in the orderlist; cannot record songs with indefinite song length.");

	return 0;
}

LRESULT CMachineView::OnMixdownWavetable(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {

	// find which wave index is highlighted in the wavetable
	// return with warning if none selected
	// if a sample exists on selected spot, ask if we should overwrite
	
	zzub_wave_t* wave = buze_document_get_current_wave(document);

	if (wave == 0) {
		MessageBox("Please select a wave in the wavetable.");
		return 0;
	}

	zzub_plugin_t* recordplugin = GetSelectedMachine(0);
	if (recordplugin == 0) return 0;

	// current wavelevel does not necessarily belong to the current wave
	zzub_wavelevel_t* level = buze_document_get_current_wavelevel(document);
	if (zzub_wave_get_level_count(wave) == 0 || zzub_wavelevel_get_wave(level) != wave)
		level = 0;

	if (level && zzub_wavelevel_get_sample_count(level) > 0) {
		int confirm = MessageBox("Overwrite exising wave?", "Buze", MB_YESNO|MB_ICONWARNING);
		if (confirm != IDYES) return 0;
	}

	if (level == 0) {
		level = zzub_wave_add_level(wave);
		if (level == 0) return 0;
	}

	zzub_player_history_enable(player, 0);

	buze_application_t* application = buze_main_frame_get_application(mainframe);
	mixdownhelper md(application, player, recordplugin);

	bool success = md.init_wave_recorder(level);
	if (success) {
		buze_application_show_wait_window(application);
		buze_application_set_wait_text(application, "Recording...");

		zzub_audiodriver_t* driver = (zzub_audiodriver_t*)buze_application_get_audio_driver(application);
		int samplerate = zzub_audiodriver_get_samplerate(driver);

		success = md.mixdown(samplerate);

		buze_application_hide_wait_window(application, m_hWnd);
	}

	md.uninit();

	// TODO: we are now inserting wavedata in the wavetable, but not undoable!! 
	// this may lead to undesirable side effects if working on the wavedata and then undo/redo
	zzub_player_history_commit(player, 0, 0, "Mixdown To Wavetable");
	zzub_player_history_enable(player, 1);

	if (!success)
		MessageBox("Mixdown fail. Check for looping patterns in the orderlist; cannot record songs with indefinite song length.");

	return 0;
}

LRESULT CMachineView::OnMixdownMultitrack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	// åpne recorder-viewet og add de valgte pluginene som sources
	// recorder-viewet viser ui for å velge recordertype/filnavn/etc
	MessageBox("Mixdown multitrack");
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// MUTE / SOLO / BYPASS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnMuteMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	for (int i=0; i < graphCtrl.GetSelectedMachines(); i++) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			// TODO: mute group out and/or in
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			bool is_muted = zzub_plugin_get_mute(plugin) ? true : false;
			zzub_plugin_set_mute(plugin, is_muted ? 0 : 1);
		}
	}
	
	zzub_player_history_commit(player, 0, 0, "Mute/Unmute Plugin(s)");
	return 0;
}

LRESULT CMachineView::OnSoloMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (graphCtrl.GetSelectedMachines() != 1) return 0;
	zzub_plugin_t* selectedMachine = GetSelectedMachine(0);

	zzub_plugin_t* soloplugin = (zzub_plugin_t*)buze_document_get_solo_plugin(document);
	buze_document_set_solo_plugin(document, selectedMachine, soloplugin != selectedMachine);
	
	zzub_player_history_commit(player, 0, 0, "Solo Plugin");
	return 0;
}

LRESULT CMachineView::OnBypassMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	for (int i=0; i < graphCtrl.GetSelectedMachines(); i++) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			// TODO: mute group out and/or in
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			bool is_bypassed = zzub_plugin_get_bypass(plugin) ? true : false;
			zzub_plugin_set_bypass(plugin, is_bypassed ? 0 : 1);
		}
	}
	zzub_player_history_commit(player, 0, 0, "Bypass Plugin(s)");

	return 0;
}

LRESULT CMachineView::OnUnmuteAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	for (std::vector<boost::shared_ptr<CGraphNode> >::iterator i = graphCtrl.nodes.begin(); i != graphCtrl.nodes.end(); ++i) {
		CGraphNode* node = i->get();
		if (node->type == node_container) {
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			zzub_plugin_set_mute(plugin, 0);
		}
	}

	buze_document_set_solo_plugin(document, 0, 0);

	zzub_player_history_commit(player, 0, 0, "Unmute All Plugins");

	graphCtrl.InvalidateMachines(false);

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// OTHER VIEW COUPLING
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	//mainframe->showHelpMachineView();
	return 0;
}

LRESULT CMachineView::OnMachineParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			// TODO: ??
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			buze_main_frame_show_plugin_parameters(mainframe, plugin, parametermode_default, -1, -1);
		}
	}
	return 0;
}

void CMachineView::OnCreateDefaultFormat() {
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); ++i) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			// create format for all plugins in the container??
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			buze_document_create_default_format(document, plugin, false);
		}
	}

	zzub_player_history_commit(player, 0, 0, "Create Default Pattern Format(s)");
}

void CMachineView::OnCreateSimpleFormat() {
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); ++i) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			// create format for all plugins in the container??
		} else {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
			buze_document_create_default_format(document, plugin, true);
		}
	}

	zzub_player_history_commit(player, 0, 0, "Create Simple Pattern Format(s)");
}

// ---------------------------------------------------------------------------------------------------------------
// COMMANDS / HELP / PROPERTIES
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	if (graphCtrl.GetSelectedMachines() != 1) return 0;
	zzub_plugin_command(GetSelectedMachine(0), wID - ID_MACHINECOMMANDS);
	return 0;
}

LRESULT CMachineView::OnMachineHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_plugin_t* machine = GetSelectedMachine(0);
	if (machine == 0) return 0;

	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	std::string helpFile = buze_document_get_plugin_helpfile(document, loader);
	if (helpFile.length() != 0) {
		int result = (int)ShellExecute(m_hWnd, "open", helpFile.c_str(), NULL, NULL, SW_SHOWNORMAL); 
		if (result < 33) helpFile = "";
	}
	if (helpFile.length() == 0) {
		MessageBox("Sorry, no help for this machine.", buze_main_frame_get_program_name(mainframe));
	}
	return 0;
}

LRESULT CMachineView::OnMachineProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (graphCtrl.GetSelectedMachines() == 0) return 0;

	CGraphNode* node = graphCtrl.GetSelectedMachine(0);

	buze_event_data_t ev;
	if (node->type == node_container) {
		ev.show_properties.type = buze_property_type_plugin_group;
		ev.show_properties.plugin_group = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
	} else {
		ev.show_properties.type = buze_property_type_plugin;
		ev.show_properties.plugin = zzub_player_get_plugin_by_id(player, node->id);
	}
	ev.show_properties.return_view = this;
	buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
	return 0;
}

LRESULT CMachineView::OnAudioConnectionProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_connection_t* audioconn = GetSelectedEdgeConnection(edge_audio, zzub_connection_type_audio);
	if (audioconn != 0) {
		buze_event_data_t ev;
		ev.show_properties.type = buze_property_type_connection;
		ev.show_properties.connection = audioconn;
		ev.show_properties.return_view = this;
		buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
	}
	return 0;
}

LRESULT CMachineView::OnMidiConnectionProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_connection_t* midiconn = GetSelectedEdgeConnection(edge_midi, zzub_connection_type_midi);
	if (midiconn != 0) {
		buze_event_data_t ev;
		ev.show_properties.type = buze_property_type_connection;
		ev.show_properties.connection = midiconn;
		ev.show_properties.return_view = this;
		buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
	}
	return 0;
}

LRESULT CMachineView::OnEventConnectionProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_connection_t* eventconn = GetSelectedEdgeConnection(edge_event, zzub_connection_type_event);
	if (eventconn != 0) {
		buze_event_data_t ev;
		ev.show_properties.type = buze_property_type_connection;
		ev.show_properties.connection = eventconn;
		ev.show_properties.return_view = this;
		buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
	}
	return 0;
}

LRESULT CMachineView::OnNoteConnectionProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_connection_t* noteconn = GetSelectedEdgeConnection(edge_note, zzub_connection_type_note);
	if (noteconn != 0) {
		buze_event_data_t ev;
		ev.show_properties.type = buze_property_type_connection;
		ev.show_properties.connection = noteconn;
		ev.show_properties.return_view = this;
		buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
	}
	return 0;
}

LRESULT CMachineView::OnAudioConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_connection_t* audioconn = GetSelectedEdgeConnection(edge_audio, zzub_connection_type_audio);
	if (audioconn != 0) {
		zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(audioconn);
		buze_main_frame_show_plugin_parameters(mainframe, connplug, parametermode_default, -1, -1);
	}
	return 0;
}

LRESULT CMachineView::OnMidiConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_connection_t* midiconn = GetSelectedEdgeConnection(edge_midi, zzub_connection_type_midi);
	if (midiconn) {
		zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(midiconn);
		buze_main_frame_show_plugin_parameters(mainframe, connplug, parametermode_default, -1, -1);
	}
	return 0;
}

LRESULT CMachineView::OnEventConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_connection_t* eventconn = GetSelectedEdgeConnection(edge_event, zzub_connection_type_event);
	if (eventconn != 0) {
		zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(eventconn);
		buze_main_frame_show_plugin_parameters(mainframe, connplug, parametermode_default, -1, -1);
	}
	return 0;
}

LRESULT CMachineView::OnNoteConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	zzub_connection_t* noteconn = GetSelectedEdgeConnection(edge_note, zzub_connection_type_note);
	if (noteconn != 0) {
		zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(noteconn);
		buze_main_frame_show_plugin_parameters(mainframe, connplug, parametermode_default, -1, -1);
	}
	return 0;
}

LRESULT CMachineView::OnEventConnectionBindings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (graphCtrl.GetSelectedEdge(edge_event) != 0)
		OnEventConnectionDialog();
	return 0;
}


// ---------------------------------------------------------------------------------------------------------------
// XXX
// ---------------------------------------------------------------------------------------------------------------

LRESULT CMachineView::OnImportSong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	// basically the same as load song, except we dont clear first
	std::string fileName = buze_main_frame_get_open_filename(mainframe);
	if (fileName.empty()) return false;

	char error_messages[4096];
	buze_document_import_song(document, fileName.c_str(), 8, 0, 0, error_messages, 4096);
	zzub_player_history_commit(player, 0, 0, "Import BMX");
	return 0;
}

LRESULT CMachineView::OnToggleConnectionText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	//show_connection_text = !show_connection_text;
	graphCtrl.showConnectionText = !graphCtrl.showConnectionText;
	graphCtrl.InvalidateMachines(false);

	return 0;
}

void CMachineView::OnCreateGroup() {
	zzub_player_create_group_with_io(player, currentlayer, createMachinePosX, createMachinePosY);
	zzub_player_history_commit(player, 0, 0, "Create Group");
}

void CMachineView::OnCreateSelectionGroup() {
	// TODO: replace connections before / after?

	zzub_plugin_group_t* plugingroup = zzub_player_create_group_with_io(player, currentlayer, createMachinePosX, createMachinePosY);
	MoveSelectionToGroup(plugingroup);
	zzub_player_history_commit(player, 0, 0, "Create Group From Selection");
}

void CMachineView::MoveSelectionToGroup(zzub_plugin_group_t* plugingroup) {
	for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
		CGraphNode* node = graphCtrl.GetSelectedMachine(i);
		if (node->type == node_container) {
			zzub_plugin_group_t* selgroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);

			// cant move to a child of itself
			if (is_valid_parent(selgroup, plugingroup))
				zzub_plugin_group_set_parent(selgroup, plugingroup);
		} else {
			zzub_plugin_t* selplugin = zzub_player_get_plugin_by_id(player, node->id);
			zzub_plugin_set_plugin_group(selplugin, plugingroup);
		}
	}
}

LRESULT CMachineView::OnMoveToGroup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	int groupindex = wID - ID_MOVE_TO_GROUP_FIRST;

	zzub_plugin_group_t* plugingroup;
	if (groupindex > 0) {
		plugingroup = zzub_player_get_plugin_group_by_index(player, groupindex);
		if (!plugingroup) return 0;
	} else
		plugingroup = 0;

	MoveSelectionToGroup(plugingroup);

	zzub_player_history_commit(player, 0, 0, "Move Plugin(s) To Group");

	return 0;
}

void CMachineView::SetMidiAliases() {
	FILE *f;
	size_t size;
	char *buf;
	char *e;
	char *first;
	char *line;

	buze_application_t* application = buze_main_frame_get_application(mainframe);
	if (f = fopen(buze_application_map_path(application, "gear\\midi_aliases.txt", buze_path_type_app_path), "rt")) {
		midiAliases.clear();
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		rewind(f);
		buf = (char*)calloc(1, size + 2);
		if (buf) {
			fread(buf, size, 1, f);
			fclose(f);
			for(line = buf; *line; line = e + 1) {
				for (e = line; *e && *e != '\n'; e++);
				for(*e = '\0'; *line == ' ' || *line == '\t'; line++);
				if (*line != '#' && *line != '\0') {
					for(first = line; *line && *line != '\t'; line++);
					if(*line == '\t')
						for(*line++ = '\0'; *line == '\t'; line++);
					if(*line) {
						printf("Reading MIDI alias: \"%s\" --> \"%s\"\n", first, line);
						midiAliases[first] = line;
					}
				}
			}
			free(buf);
		}
		else fclose(f);
	}
}

LRESULT CMachineView::OnSetTimeSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/) {
	if (GetSelectedMachines() == 0) return 0;

	int index = wID - ID_MACHINE_TIMESOURCE_FIRST;
	zzub_plugin_t* timeplugin = zzub_player_get_timesource_plugin(player, index);
	int timegroup = zzub_player_get_timesource_group(player, index);
	int timetrack = zzub_player_get_timesource_track(player, index);

	for (int i = 0; i< GetSelectedMachines(); ++i) {
		zzub_plugin_set_timesource(GetSelectedMachine(i), timeplugin, timegroup, timetrack);
	}
	zzub_player_history_commit(player, 0, 0, "Set Plugin Time Source");
	return 0;
}

LRESULT CMachineView::OnMachineHideIncomingConnections(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	for (int i = 0; i < GetSelectedMachines(); i++) {
		zzub_plugin_t* plugin = GetSelectedMachine(i);
		int node_id = zzub_plugin_get_id(plugin);

		bool is_hidden = graphCtrl.GetMachineHideIncomingConnections(node_id) ? true : false;
		graphCtrl.SetMachineHideIncomingConnections(node_id, is_hidden ? 0 : 1);
		InvalidateMachine(plugin);
	}
	return 0;
}

void CMachineView::OnAudioConnectionDialog() {

	zzub_connection_t* audioconn = GetSelectedEdgeConnection(edge_audio, zzub_connection_type_audio);
	assert(audioconn != 0);

	ShowAudioConnectionDialog(audioconn);
}

void CMachineView::ShowAudioConnectionDialog(zzub_connection_t* audioconn) {

	assert(zzub_connection_get_type(audioconn) == zzub_connection_type_audio);

	audioConnDlg.Create(m_hWnd, (LPARAM)m_hWnd);

	zzub_plugin_t* sourceplug = zzub_connection_get_from_plugin(audioconn);
	for (int i = 0; i < zzub_plugin_get_output_channel_count(sourceplug); i++) {
		std::string name = GetOutputChannelName(sourceplug, i);
		audioConnDlg.AddSourceChannel(name.c_str());
	}

	zzub_plugin_t* targetplug = zzub_connection_get_to_plugin(audioconn);
	for (int i = 0; i < zzub_plugin_get_input_channel_count(targetplug); i++) {
		std::string name = GetInputChannelName(targetplug, i);
		audioConnDlg.AddTargetChannel(name.c_str());
	}

	const char* pluginname = zzub_plugin_get_name(sourceplug);
	audioConnDlg.m_sourcePlugin.SetWindowText(pluginname);

	pluginname = zzub_plugin_get_name(targetplug);
	audioConnDlg.m_targetPlugin.SetWindowText(pluginname);

	int first_input_channel = zzub_connection_get_first_input(audioconn);
	int input_channels = zzub_connection_get_input_count(audioconn);
	int first_output_channel = zzub_connection_get_first_output(audioconn);
	int output_channels = zzub_connection_get_output_count(audioconn);
	audioConnDlg.SetChannelRange(first_input_channel, input_channels, first_output_channel, output_channels);

	audioConnDlg.connection = audioconn;
	audioConnDlg.ShowWindow(SW_SHOWNORMAL);
}

void CMachineView::OnAudioConnectionOK() {
	int first_output_channel = 0;
	int output_channel_count = 0;
	audioConnDlg.GetChannelRange(audioConnDlg.m_sourceChannels, &first_output_channel, &output_channel_count);

	int first_input_channel = 0;
	int input_channel_count = 0;
	audioConnDlg.GetChannelRange(audioConnDlg.m_targetChannels, &first_input_channel, &input_channel_count);

	zzub_connection_t* audioconn = audioConnDlg.connection;
	zzub_connection_set_first_input(audioconn, first_input_channel);
	zzub_connection_set_input_count(audioconn, input_channel_count);
	zzub_connection_set_first_output(audioconn, first_output_channel);
	zzub_connection_set_output_count(audioconn, output_channel_count);

	zzub_player_history_commit(player, 0, 0, "Edit Audio Connection");
}

void CMachineView::OnEventConnectionDialog() {
	eventConnDlg.Create(m_hWnd, (LPARAM)m_hWnd);

	//zzub_plugin_t* to_plugin = zzub_player_get_plugin_by_id(player, graphCtrl.selectedConnection.to_node_id);
	//zzub_plugin_t* from_plugin = zzub_player_get_plugin_by_id(player, graphCtrl.selectedConnection.from_node_id);
	zzub_connection_t* eventconn = GetSelectedEdgeConnection(edge_event, zzub_connection_type_event);
	assert(eventconn != 0);

	// add source parameters:
	zzub_plugin_t* sourceplug = zzub_connection_get_from_plugin(eventconn);
	int eventparams = zzub_plugin_get_parameter_count(sourceplug, 3, 0);
	for (int i = 0; i < eventparams; i++) {
		zzub_parameter_t* sourceparam = zzub_plugin_get_parameter(sourceplug, 3, 0, i);
		const char* sourceparamname = zzub_parameter_get_name(sourceparam);
		eventConnDlg.m_sourceParams.AddString(sourceparamname);
	}

	// add target parameters:
	zzub_plugin_t* targetplug = zzub_connection_get_to_plugin(eventconn);
	for (int j = 0; j < 3; j++) {
		int eventparams = zzub_plugin_get_parameter_count(targetplug, j, 0);
		int trackcount = zzub_plugin_get_track_count(targetplug, j);
		for (int k = 0; k < trackcount; k++) {
			for (int i = 0; i < eventparams; i++) {
				zzub_parameter_t* targetparam = zzub_plugin_get_parameter(targetplug, j, k, i);
				const char* targetparamname = zzub_parameter_get_name(targetparam);
				std::stringstream strm;
				if (j == 2) {
					strm << "[" << (k + 1) << "] ";
				}
				strm << targetparamname;
				eventConnDlg.AddTargetParameter(j, k, i, strm.str());
			}
		}
	}

	// add bindings
	zzub_connection_binding_iterator_t* cbit = zzub_connection_get_event_binding_iterator(eventconn);
	while (zzub_connection_binding_iterator_valid(cbit)) {
		zzub_connection_binding_t* binding = zzub_connection_binding_iterator_current(cbit);

		int srccol = zzub_connection_binding_get_source_column(binding);
		int targetgroup = zzub_connection_binding_get_target_group(binding);
		int targettrack = zzub_connection_binding_get_target_track(binding);
		int targetcolumn = zzub_connection_binding_get_target_column(binding);

		eventConnDlg.AddBinding(srccol, targetgroup, targettrack, targetcolumn);
		zzub_connection_binding_iterator_next(cbit);
	}
	zzub_connection_binding_iterator_destroy(cbit);

	// set plugin name labels:
	const char* pluginname = zzub_plugin_get_name(sourceplug);
	eventConnDlg.m_sourcePlugin.SetWindowText(pluginname);

	pluginname = zzub_plugin_get_name(targetplug);
	eventConnDlg.m_targetPlugin.SetWindowText(pluginname);
	eventConnDlg.m_sourceParams.SetCurSel(0);

	// databind and display:
	eventConnDlg.OnSourceParamSelChange();
	eventConnDlg.connection = eventconn;
	eventConnDlg.ShowWindow(SW_SHOWNORMAL);

}

void CMachineView::OnEventConnectionOK() {

	zzub_connection_t* eventconn = eventConnDlg.connection;

	// delete all old bindings:
	zzub_connection_binding_iterator_t* cbit = zzub_connection_get_event_binding_iterator(eventconn);
	while (zzub_connection_binding_iterator_valid(cbit)) {
		zzub_connection_binding_t* binding = zzub_connection_binding_iterator_current(cbit);

		int srccol = zzub_connection_binding_get_source_column(binding);
		int targetgroup = zzub_connection_binding_get_target_group(binding);
		int targettrack = zzub_connection_binding_get_target_track(binding);
		int targetcolumn = zzub_connection_binding_get_target_column(binding);
		zzub_connection_remove_event_connection_binding(eventconn, srccol, targetgroup, targettrack, targetcolumn);

		zzub_connection_binding_iterator_next(cbit);
	}
	zzub_connection_binding_iterator_destroy(cbit);

	// insert new bindings 
	for (int i = 0; i < eventConnDlg.GetBindingCount(); i++) {
		int sourceparam, group, track, column;
		eventConnDlg.GetBinding(i, &sourceparam, &group, &track, &column);

		zzub_connection_add_event_connection_binding(eventconn, sourceparam, group, track, column);
	}

	zzub_player_history_commit(player, 0, 0, "Edit Event Connection Bindings");
}

void CMachineView::GetHelpText(char* text, int* len) {

	std::string helptext = PeekString(_Module.GetResourceInstance(), IDT_HELP_MACHINEVIEW);
	HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "machineview");
	std::string acceltext = CreateAccelTableString(hAccel);

	helptext += acceltext;
	*len = (int)helptext.length();
	if (text)
		strcpy(text, helptext.c_str());
}

bool CMachineView::DoesKeyjazz() {
	return true;
}

void CMachineView::OnGraphMoveSelection() {
	RECT rc;
	graphCtrl.GetClientScale(&rc);
	POINT pt = graphCtrl.moveCurrentPoint;
	float dx = ((float)pt.x - graphCtrl.beginDragPt.x) / (float)rc.right;
	float dy = ((float)pt.y - graphCtrl.beginDragPt.y) / (float)rc.bottom;

	float mdx = dx * 2 / graphCtrl.scale;
	float mdy = dy * 2 / graphCtrl.scale;

	if (pt.x != graphCtrl.beginDragPt.x || pt.y != graphCtrl.beginDragPt.y) {
		for (int i = 0; i < graphCtrl.GetSelectedMachines(); i++) {
			CGraphNode* node = graphCtrl.GetSelectedMachine(i);
			if (node->type == node_container) {
				zzub_plugin_group_t* sellayer = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
				zzub_plugin_group_set_position(sellayer, node->position.x + mdx, node->position.y + mdy);
			} else {
				zzub_plugin_t* selplugin = zzub_player_get_plugin_by_id(player, node->id);
				zzub_plugin_set_position(selplugin, node->position.x + mdx, node->position.y + mdy);
			}
		}
		zzub_player_history_commit(player, 0, 0, "Move Machine(s)");
	}
}

void CMachineView::OnGraphMoveView() {
	zzub_player_set_machineview_offset(player, graphCtrl.view_offset_x, graphCtrl.view_offset_y);
	zzub_player_history_commit(player, 0, 0, "Offset Machine View");
}

void CMachineView::OnGraphSetAmp() {
	CGraphEdge* edge = graphCtrl.GetSelectedEdge(edge_audio);
	assert(edge != 0);

	zzub_plugin_t* to_plugin = zzub_player_get_plugin_by_id(player, edge->to_user_id);
	zzub_plugin_t* from_plugin = zzub_player_get_plugin_by_id(player, edge->from_user_id);
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	assert(audioconn != 0);
	//zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(selectedConnection.to_plugin, selectedConnection.from_plugin, zzub_connection_type_audio);

	// undo trickery for undo to the initial volume:
	zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(audioconn);
	zzub_player_history_enable(player, 0);                  // disable undo buffer
	zzub_audio_connection_set_amp(connplug, (int)(graphCtrl.initialVolume * 16384.0f), true); // set initial volume before dragging
	zzub_player_history_enable(player, 1);                  // enable undo buffer

	zzub_audio_connection_set_amp(connplug, (int)(edge->amp * 16384.0f), true);

	zzub_player_history_commit(player, 0, 0, "Set Connection Volume");///

}

void CMachineView::OnGraphTrackAmp() {
	CGraphEdge* edge = graphCtrl.GetSelectedEdge(edge_audio);
	assert(edge != 0);

	zzub_plugin_t* to_plugin = zzub_player_get_plugin_by_id(player, edge->to_user_id);
	zzub_plugin_t* from_plugin = zzub_player_get_plugin_by_id(player, edge->from_user_id);
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);

	//zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(selectedConnection.to_plugin, selectedConnection.from_plugin, zzub_connection_type_audio);
	assert(audioconn != 0);
	zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(audioconn);
	zzub_audio_connection_set_amp(connplug, (int)(edge->amp * 16384.0f), false);
}

void CMachineView::OnGraphChangeScale() {
	configuration->setMachineScale(graphCtrl.scale);
	CalcPixelSize();
}

void CMachineView::OnGraphMute() {
	if (graphCtrl.draggingMachine->type == node_container) {
		// mute layer input and/or output?
	} else {
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, graphCtrl.draggingMachine->id);
		zzub_plugin_set_mute(plugin, zzub_plugin_get_mute(plugin) == 0);
		zzub_player_history_commit(player, 0, 0, "Mute/Unmute Plugin");
	}
}

void CMachineView::OnGraphDblClick() {
	CGraphNode* node = graphCtrl.GetSelectedMachine(0);
	if (node == 0) return ;

	if (node->type == node_container) {
		currentlayer = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);
		dirtyVisibleMachines = true;
		Invalidate(FALSE);
	} else {
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);
		if (plugin == 0) return ;

		zzub_event_data eventData = { zzub_event_type_double_click };
		eventData.double_click.plugin = plugin;
		if (zzub_player_invoke_event(player, &eventData, true) == -1) {
			buze_main_frame_show_plugin_parameters(mainframe, plugin, parametermode_default, -1, -1);
		}
	}
}

void CMachineView::OnGraphSelectNode() {
	CGraphNode* node = graphCtrl.GetSelectedMachine(0);

	if (node == 0) return;

	if (node->type == node_container) {

		zzub_plugin_group_t* plugingroup = zzub_player_get_plugin_group_by_id(player, node->id - plugingroup_id_first);

		zzub_player_reset_keyjazz(player);	// reset midi keyjazz
		zzub_player_set_midi_plugin(player, 0);

		buze_event_data_t ev;
		ev.show_properties.type = buze_property_type_plugin_group;
		ev.show_properties.plugin_group = plugingroup;
		ev.show_properties.return_view = this;
		buze_document_notify_views(document, this, buze_event_type_update_properties, &ev);
	} else {
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, node->id);

		zzub_player_reset_keyjazz(player);	// reset midi keyjazz
		zzub_player_set_midi_plugin(player, plugin);

		if (plugin == 0) return ;

		buze_event_data_t ev;
		ev.show_properties.type = buze_property_type_plugin;
		ev.show_properties.plugin = plugin;
		ev.show_properties.return_view = this;
		buze_document_notify_views(document, this, buze_event_type_update_properties, &ev);
	}
}

void CMachineView::OnGraphClickConnection() {
	{
		CGraphEdge* edge = graphCtrl.GetSelectedEdge(edge_event);
		if (edge != 0) {
			OnEventConnectionDialog();
			return;
		}
	}
	{
		CGraphEdge* edge = graphCtrl.GetSelectedEdge(edge_audio);
		if (edge != 0) {
			OnAudioConnectionDialog();
			return;
		}
	}
}

bool CMachineView::DisconnectEdge(int to_plugin_id, int from_plugin_id, edgetype etype, zzub_connection_type ctype) {
	zzub_connection_t* conn = GetEdgeConnection(to_plugin_id, from_plugin_id, etype, ctype);
	if (conn == 0) return false;

	zzub_connection_destroy(conn);
	return true;
}

void CMachineView::OnGraphDisconnectConnection() {
	CGraphNodePair* nodepair = graphCtrl.GetSelectedEdge();
	if (nodepair == 0) return ;

	bool deleted = false;
	deleted |= DisconnectEdge(nodepair->to_node_id, nodepair->from_node_id, edge_audio, zzub_connection_type_audio);
	deleted |= DisconnectEdge(nodepair->to_node_id, nodepair->from_node_id, edge_midi, zzub_connection_type_midi);
	deleted |= DisconnectEdge(nodepair->to_node_id, nodepair->from_node_id, edge_event, zzub_connection_type_event);
	deleted |= DisconnectEdge(nodepair->to_node_id, nodepair->from_node_id, edge_note, zzub_connection_type_note);

	if (deleted)
		zzub_player_history_commit(player, 0, 0, "Disconnect Plugins");
}

void CMachineView::OnGraphClick() {
	RECT rc;
	graphCtrl.GetClientScale(&rc);

	POINT pt = graphCtrl.typepoint;
	createMachinePosX = (((float)pt.x / (float)rc.right) - 0.5f) * 2.0f / graphCtrl.scale;
	createMachinePosY = (((float)pt.y / (float)rc.bottom) - 0.5f) * 2.0f / graphCtrl.scale;
}
