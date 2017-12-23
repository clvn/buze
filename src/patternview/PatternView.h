#pragma once

#include "ToolbarWindow.h"
#include "PatternEditor/PatternEditorInfoPane.h"
#include "PatternEditor/PatternEditorOrderlist.h"
#include "PatternEditor/PatternEditorTransformDlg.h"
#include "PatternEditor/PatternEditorInner.h"
#include "PatternEditor/PatternEditorControl.h"
#include "SplitterWindowKey.h"

class CViewFrame;
static const int WM_GET_EDITFLAGS = WM_USER+9;
static const int WM_GET_STATUS_TEXT = WM_USER+14; // sent by mainframe to client views = client views must reserve message!
static const int WM_PLAYFROMROW = WM_USER+15; // sent by rowclicks, need >16bits rows
static const int WM_HOLDROW = WM_USER+16; // sent by rowclicks, need >16bits rows

struct selection_position {
	int row, plugin_id, group, track, column;
};

struct pattern_position {
	int row, plugin_id, group, track, column, digit;
	int scroll_x_unit, scroll_y_row;
	selection_position select_from;
	selection_position select_to;
};

typedef std::map<std::pair<std::string, zzub_pattern_t*>, pattern_position> pattern_position_map_t;
typedef std::map<std::pair<std::string, zzub_pattern_format_t*>, pattern_position> pattern_format_position_map_t;

class CPatternViewInfo : public CViewInfoImpl {
public:
	pattern_position_map_t patternPositionCache; // remember cursor positions per pattern
	pattern_format_position_map_t patternFormatPositionCache; // remember cursor positions per pattern format

	CPatternViewInfo(CViewFrame* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
	void ShowPatternView(buze_event_data* ev);

	// pattern position cache - shared across open pattern editors
	pattern_position GetPatternPosition(std::string const& caption, zzub_pattern_t* pattern);
	void SetPatternPosition(std::string const& caption, zzub_pattern_t* pattern, pattern_position const& pos);
	void SetPatternScroll(std::string const& caption, zzub_pattern_t* pattern, int cols, int rows);
	void SetPatternSelection(std::string const& caption, zzub_pattern_t* pattern, selection_position const& select_from, selection_position const& select_to);
	void RemovePatternPosition(zzub_pattern_format_column_t* format);
};

// ---------------------------------------------------------------------------------------------------------------
// PATTERNVIEW
// ---------------------------------------------------------------------------------------------------------------

class CPatternView
:
	public CViewImpl,
	public CUpdateUI<CPatternView>,
	public CToolbarWindow<CPatternView>,
	public CIdleHandler,
	public CMessageFilter
{
  public:

	bool is_primary;
	zzub_pattern_t* pattern;
	zzub_pattern_format_t* patternformat;
	HWND hWndButtonToolBar;
	int font_size;
	string font_name;
	//bool preTranslateMessageSucceeded; // support a hack allowing number keys as accelerators in note column
	std::vector<zzub_pattern_format_t*> formats;
	std::vector<zzub_pattern_t*> patterns;
	std::map<zzub_pattern_format_t*, zzub_pattern_t*> format_last_viewed_pattern;
	std::vector<zzub_pattern_t*> pattern_stack;
	int pattern_stack_pos;
	std::string caption;

	bool play_notes;
	int show_infopane;
	bool follow_mode;
	bool orderlist_enabled;

	CBuzeConfiguration configuration;
	CPatternViewInfo* viewInfo;
	CPatternEditorControl editorControl;
	CPatternEditorInner& editorInner;

	CComboListBand waveDropDown;
	CTwinComboListBand machinePatternPanel;
	CComboListBand octaveDropDown;
	CNumericComboBand stepDropDown;
	CCheckboxBand playNotesCheckbox;
	CCheckboxBand infoCheckbox;
	CCheckboxBand patternloopCheckbox;
	CComboListBand patternbeatDropDown;
	CNumericComboBand patternscaleDropDown;
	CNumericComboBand patternrowsDropDown;
	CEditBand patternnameEdit;
	CCheckboxBand followCheckbox;

	CStatusBarCtrl statusBar;

	enum PasteMode {
		paste_mode_replace,
		paste_mode_mix_over,
		paste_mode_mix_under,
		paste_mode_swap,
	};

	bool dirtyPatternEditor;
	bool dirtyPatternEditorVertical;
	bool dirtyPatternEditorHorizontal;
	bool dirtyStatus;
	bool dirtyMachinePatternPanel;
	bool dirtySongPositions;
	bool dirtyColumnInfo;
	bool dirtyWaveDropDown;
	bool dirtyPatternInfos;
	bool dirtyTracks;
	bool dirtyInfoPane;

	DECLARE_WND_CLASS("PatternView")

	BEGIN_UPDATE_UI_MAP(CPatternView)
		UPDATE_ELEMENT(ID_EDIT_CUT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PASTE_MIXOVER, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PASTE_MIXUNDER, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PASTE_SPLICE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_CUT_SPLICE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_DELETE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PATTERNSTACK_BACK, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PATTERNSTACK_FORWARD, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PATTERNVIEW_WAVETOOLTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_MACHINEPATTERNTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_OCTAVETOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_STEPTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PATTERNLOOPTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PATTERNSCALETOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PATTERNBEATTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PLAYNOTESTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_BUTTONBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_FOLLOWTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_INFOTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PATTERNROWSTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_PATTERNVIEW_PATTERNNAMETOOLBAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP_EX(CPatternView)
		CHAIN_MSG_MAP(CUpdateUI<CPatternView>)
		CHAIN_MSG_MAP(CToolbarWindow<CPatternView>)

		// wm's
		MESSAGE_HANDLER_EX(WM_CREATE, OnCreate)
		MESSAGE_HANDLER_EX(WM_CLOSE, OnClose)
		MESSAGE_HANDLER_EX(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER_EX(WM_SIZE, OnSize)
		MESSAGE_HANDLER_EX(WM_PAINT, OnPaint)
		MESSAGE_HANDLER_EX(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER_EX(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER_EX(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER_EX(WM_XBUTTONDOWN, OnXButtonDown)
		MESSAGE_HANDLER_EX(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER_EX(WM_GET_EDITFLAGS, OnGetEditFlags)
		MESSAGE_HANDLER_EX(WM_GET_STATUS_TEXT, OnGetStatusText)
		MESSAGE_HANDLER_EX(WM_PLAYFROMROW, OnControlPlayFromRow)
		MESSAGE_HANDLER_EX(WM_HOLDROW, OnControlHoldRow)

		COMMAND_ID_HANDLER(ID_PATTERNVIEW_WAVETOOLTOOLBAR, OnPatternViewWaveToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_MACHINEPATTERNTOOLBAR, OnPatternViewMachinePatternToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_OCTAVETOOLBAR, OnPatternViewOctaveToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_STEPTOOLBAR, OnPatternViewStepToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_PATTERNSCALETOOLBAR, OnPatternViewScaleToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_PATTERNBEATTOOLBAR, OnPatternViewBeatToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_PLAYNOTESTOOLBAR, OnPatternViewPlayNotesToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_BUTTONBAR, OnPatternViewPatternViewToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_INFOTOOLBAR, OnPatternViewInfoToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_PATTERNROWSTOOLBAR, OnPatternViewPatternRowsToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_PATTERNNAMETOOLBAR, OnPatternViewPatternNameToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_PATTERNLOOPTOOLBAR, OnPatternViewLoopToolbar)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_FOLLOWTOOLBAR, OnPatternViewFollowToolbar)

		// basic edit operations
		//CMD_ID_HANDLER(ID_HELP, OnHelp)
		CMD_ID_HANDLER(ID_EDIT_CUT, OnCut)
		CMD_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		CMD_ID_HANDLER(ID_EDIT_PASTE, OnPaste)
		CMD_ID_HANDLER(ID_EDIT_DELETE, OnDelete)
		CMD_ID_HANDLER(ID_EDIT_SELECTALL, OnSelectAll)
		CMD_ID_HANDLER(ID_EDIT_CLEARSELECTION, OnUnselect)

		// extended copy/paste
		CMD_ID_HANDLER(ID_PATTERNVIEW_PASTE_MIXOVER, OnPasteMixOver)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PASTE_MIXUNDER, OnPasteMixUnder)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PASTE_STEP, OnPasteStep)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PASTE_SPLICE, OnPasteSplice)
		CMD_ID_HANDLER(ID_PATTERNVIEW_CUT_SPLICE, OnCutSplice)

		// creation / deletion / cloning / properties
		CMD_ID_HANDLER(ID_PATTERNVIEW_CREATEPATTERN, OnPatternCreate)
		CMD_ID_HANDLER(ID_PATTERN_CLONE, OnPatternClone)
		CMD_ID_HANDLER(ID_PATTERN_DELETE, OnPatternDelete)
		CMD_ID_HANDLER(ID_PATTERN_CREATEFORMAT, OnCreatePatternFormat)
		CMD_ID_HANDLER(ID_PATTERNVIEW_CLONE_FORMAT, OnClonePatternFormat)
		CMD_ID_HANDLER(ID_PATTERN_DELETEFORMAT, OnDeletePatternFormat)
		CMD_ID_HANDLER(ID_PATTERN_FORMATPROPERTIES, OnPatternFormatProperties)
		CMD_ID_HANDLER(ID_VIEW_PROPERTIES, OnViewProperties)

		// pattern mutations
		CMD_ID_HANDLER(ID_PATTERN_DOUBLEROWS, OnPatternDoubleRows)
		CMD_ID_HANDLER(ID_PATTERN_HALVEROWS, OnPatternHalveRows)
		CMD_ID_HANDLER(ID_PATTERN_DOUBLELENGTH, OnPatternDoubleLength)
		CMD_ID_HANDLER(ID_PATTERN_HALVELENGTH, OnPatternHalveLength)

		// machine interactions		
		CMD_ID_HANDLER(ID_MACHINE_MUTE, OnMachineMute)///
		CMD_ID_HANDLER(ID_MACHINE_SOLO, OnMachineSolo)///
		CMD_ID_HANDLER(ID_MACHINE_ADDTRACK, OnMachineAddTrack)
		CMD_ID_HANDLER(ID_MACHINE_REMOVETRACK, OnMachineRemoveTrack)
		CMD_ID_HANDLER(ID_MACHINE_PARAMETERS, OnMachineParameters)
		CMD_ID_HANDLER(ID_MACHINE_PROPERTIES, OnMachineProperties)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INTERPOLATE_ABSOLUTE, OnParameterInterpolateAbsolute)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INTERPOLATE_INERTIAL, OnParameterInterpolateInertial)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INTERPOLATE_LINEAR, OnParameterInterpolateLinear)

		// pattern transport
		CMD_ID_HANDLER(ID_PATTERN_PLAY, OnPatternPlay)
		CMD_ID_HANDLER(ID_PATTERN_REPLAY, OnPatternReplay)
		CMD_ID_HANDLER(ID_PATTERN_PLAYFROMCURSOR, OnPatternPlayFromCursor)
		CMD_ID_HANDLER(ID_PATTERN_STOP, OnPatternStop)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PLAY_ROW, OnPlayRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PLAY_TRACKROW, OnPlayTrackRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_JAZZ_ROW, OnJazzRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_JAZZ_TRACKROW, OnJazzTrackRow)

		// hotkeys to change/open toolbars
		CMD_ID_HANDLER(ID_PATTERNVIEW_DROPDOWNWAVE, OnDropdownWave)
		CMD_ID_HANDLER(ID_PATTERNVIEW_WAVE_NEXT, OnNextWave)
		CMD_ID_HANDLER(ID_PATTERNVIEW_WAVE_PREVIOUS, OnPrevWave)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DROPDOWNFORMAT, OnDropdownFormat)
		CMD_ID_HANDLER(ID_PATTERNVIEW_FORMATUP, OnFormatUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_FORMATDOWN, OnFormatDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DROPDOWNPATTERN, OnDropdownPattern)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNUP, OnPatternUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNDOWN, OnPatternDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DROPDOWNOCTAVE, OnDropdownOctave)
		CMD_ID_HANDLER(ID_PATTERNVIEW_OCTAVE_UP, OnOctaveUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_OCTAVE_DOWN, OnOctaveDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DROPDOWNSTEP, OnDropdownStep)
		CMD_ID_HANDLER(ID_PATTERNVIEW_STEP_UP, OnStepUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_STEP_DOWN, OnStepDown)
		CMD_RANGE_HANDLER(ID_PATTERNVIEW_STEP_SET_0, ID_PATTERNVIEW_STEP_SET_9, OnSetStepRange)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLEPLAYNOTES, OnTogglePlayNotes)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLESHOWINFO, OnToggleShowInfo)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLEPATTERNLOOP, OnTogglePatternLoop)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DROPDOWNPATTERNSCALE, OnDropdownPatternScale)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DROPDOWNPATTERNBEAT, OnDropdownPatternBeat)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DROPDOWNPATTERNROWS, OnDropdownPatternRows)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DROPDOWNPATTERNNAME, OnDropdownPatternName)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLEFOLLOW, OnToggleFollow)

		// toolbar responses
		CMD_HANDLER(IDC_PATTERNBEATDROPDOWN, CBN_SELCHANGE, OnSelChangeBeat)
		COMMAND_HANDLER(IDC_MACHINEPATTERNPANEL, CBN_SELCHANGE, OnMachinePatternSelChange)
		CMD_HANDLER(IDC_OCTAVEDROPDOWN, CBN_SELCHANGE, OnOctaveSelChange)
		CMD_HANDLER(IDC_MACHINEPATTERNPANEL, CBN_CLOSEUP, OnRestoreFocus)
		CMD_HANDLER(IDC_WAVEDROPDOWN, CBN_CLOSEUP, OnRestoreFocus)
		CMD_HANDLER(IDC_OCTAVEDROPDOWN, CBN_CLOSEUP, OnRestoreFocus)
		CMD_HANDLER(IDC_PATTERNBEATDROPDOWN, CBN_CLOSEUP, OnRestoreFocus)
		CMD_HANDLER(IDC_PLAYNOTESCHECKBOX, BN_CLICKED, OnTogglePlayNotes)
		CMD_HANDLER(IDC_SHOWINFOCHECKBOX, BN_CLICKED, OnToggleShowInfo)
		CMD_HANDLER(IDC_PATTERNLOOPCHECKBOX, BN_CLICKED, OnTogglePatternLoop)
		CMD_HANDLER(IDC_FOLLOWCHECKBOX, BN_CLICKED, OnToggleFollow)

		// step
		CMD_HANDLER(IDC_STEPDROPDOWN, CBN_CLOSEUP, OnRestoreFocus)
		CMD_HANDLER(IDC_STEPDROPDOWN, CBN_SELCHANGE, OnStepSelChange)
		CMD_HANDLER(IDC_STEPDROPDOWN, WM_NUMERICCOMBOBOX_ACCEPT, OnStepEditAccept)
		CMD_HANDLER(IDC_STEPDROPDOWN, WM_NUMERICCOMBOBOX_CANCEL, OnStepEditCancel)
		CMD_HANDLER(IDC_STEPDROPDOWN, CBN_KILLFOCUS, OnStepKillFocus)

		// scale
		CMD_HANDLER(IDC_PATTERNSCALEDROPDOWN, CBN_CLOSEUP, OnRestoreFocus)
		CMD_HANDLER(IDC_PATTERNSCALEDROPDOWN, CBN_SELCHANGE, OnScaleSelChange)
		CMD_HANDLER(IDC_PATTERNSCALEDROPDOWN, WM_NUMERICCOMBOBOX_ACCEPT, OnScaleEditAccept)
		CMD_HANDLER(IDC_PATTERNSCALEDROPDOWN, WM_NUMERICCOMBOBOX_CANCEL, OnScaleEditCancel)
		CMD_HANDLER(IDC_PATTERNSCALEDROPDOWN, CBN_KILLFOCUS, OnScaleKillFocus)

		// rows
		CMD_HANDLER(IDC_PATTERNROWSDROPDOWN, CBN_CLOSEUP, OnRestoreFocus)
		CMD_HANDLER(IDC_PATTERNROWSDROPDOWN, CBN_SELCHANGE, OnPatternRowsSelChange)
		CMD_HANDLER(IDC_PATTERNROWSDROPDOWN, WM_NUMERICCOMBOBOX_ACCEPT, OnPatternRowsEditAccept)
		CMD_HANDLER(IDC_PATTERNROWSDROPDOWN, WM_NUMERICCOMBOBOX_CANCEL, OnPatternRowsEditCancel)
		CMD_HANDLER(IDC_PATTERNROWSDROPDOWN, CBN_KILLFOCUS, OnPatternRowsKillFocus)

		// name
		CMD_HANDLER(IDC_PATTERNNAMEEDIT, WM_EDITBAND_ACCEPT, OnPatternNameEditAccept)
		CMD_HANDLER(IDC_PATTERNNAMEEDIT, WM_EDITBAND_CANCEL, OnPatternNameEditCancel)
		CMD_HANDLER(IDC_PATTERNNAMEEDIT, EN_KILLFOCUS, OnPatternNameKillFocus)

		// insertion / deletion hotkeys
		CMD_ID_HANDLER(ID_PATTERNVIEW_INSERTTRACKROW, OnInsertTrackRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INSERTCOLUMNROW, OnInsertColumnRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INSERTPATTERNROW, OnInsertPatternRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DELETETRACKROW, OnDeleteTrackRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DELETECOLUMNROW, OnDeleteColumnRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_DELETEPATTERNROW, OnDeletePatternRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_BACKSPACETRACKROW, OnBackspaceTrackRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_BACKSPACECOLUMNROW, OnBackspaceColumnRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_BACKSPACEPATTERNROW, OnBackspacePatternRow)

		// value hotkeys
		CMD_ID_HANDLER(ID_PATTERNVIEW_PICKUPVALUE, OnPickupValue)

		// clearing
		CMD_ID_HANDLER(ID_PATTERNVIEW_CLEARVALUE, OnClearValue)
		CMD_ID_HANDLER(ID_PATTERNVIEW_CLEARTRACKROW, OnClearTrackRow)
		CMD_ID_HANDLER(ID_PATTERNVIEW_CLEARPATTERNROW, OnClearPatternRow)

		// selection hotkeys
		CMD_ID_HANDLER(ID_PATTERNVIEW_SELECTCOLUMNS, OnSelectColumns)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SELECTDOWN, OnSelectDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SELECTBEAT, OnSelectBeat)
		CMD_ID_HANDLER(ID_PATTERNVIEW_UNSELECT, OnUnselect)
 		CMD_ID_HANDLER(ID_PATTERNVIEW_SELECTBEGIN, OnSelectBegin)///
 		CMD_ID_HANDLER(ID_PATTERNVIEW_SELECTEND, OnSelectEnd)///

		// loops
		CMD_ID_HANDLER(ID_PATTERNVIEW_SETLOOPBEGIN, OnSetLoopBegin)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SETLOOPEND, OnSetLoopEnd)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SETLOOPPATTERN, OnSetLoopPattern)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SETLOOPSELECTION, OnSetLoopSelection)

		// column controls
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLECOLUMNCONTROL, OnToggleColumnControl)
		CMD_RANGE_HANDLER(ID_PATTERNVIEW_SETCOLUMNCONTROL_0, ID_PATTERNVIEW_SETCOLUMNCONTROL_9, OnSetColumnControl)
		CMD_ID_HANDLER(ID_PATTERNVIEW_COLLAPSESELECTION, OnCollapseSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_UNCOLLAPSESELECTION, OnUncollapseSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLECOLLAPSESELECTION, OnToggleCollapseSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLECOLLAPSETRACK, OnToggleCollapseTrack)

		// column actions
		CMD_ID_HANDLER(ID_PATTERNVIEW_SPECIAL1, OnSpecial1)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SPECIAL2, OnSpecial2)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SPECIAL3, OnSpecial3)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SPECIAL4, OnSpecial4)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SPECIAL5, OnSpecial5)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SPECIAL6, OnSpecial6)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNCOLUMN_CLONEPATTERN, OnCloneTrigger)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNCOLUMN_HOTPASTE, OnHotPastePattern)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNCOLUMN_GOTOPATTERN, OnGotoPattern)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNCOLUMN_CLONEGOTOPATTERN, OnCloneGotoPattern)

		// pattern stack
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNSTACK_BACK, OnBack)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNSTACK_FORWARD, OnForward)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNSTACK_RESET, OnResetPatternStack)

		// info pane
		CMD_ID_HANDLER(ID_PATTERNVIEW_INFOPANE_UP, OnInfoPaneUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INFOPANE_DOWN, OnInfoPaneDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INFOPANE_PAGEUP, OnInfoPanePageUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INFOPANE_PAGEDOWN, OnInfoPanePageDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_PATTERNCOLUMN_EXPANDCOLLAPSE, OnPatternTreeExpandCollapse)

		// entry modes
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLE_HORIZONTALENTRY, OnToggleHorizontalEntry)

		// other view coupling
		CMD_ID_HANDLER(ID_PATTERNVIEW_DUPLICATEEDITOR, OnDuplicateEditor)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SHOWPATTERNFORMAT, OnShowPatternFormat)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SHOWPATTERNLIST, OnShowPatternList)
		CMD_RANGE_HANDLER(ID_PATTERNVIEW_SCROLLLINKEDITOR_FIRST, ID_PATTERNVIEW_SCROLLLINKEDITOR_LAST, OnScrollLinkEditor)

		// messages sent from patterneditorinner
		NOTIFY_HANDLER(IDC_PATTERNEDITOR, ID_PATTERNEDITORINNER_EDIT, OnEdit)
		NOTIFY_HANDLER(IDC_PATTERNEDITOR, ID_PATTERNEDITORINNER_NOTE, OnNote)
		NOTIFY_HANDLER(IDC_PATTERNEDITOR, ID_PATTERNEDITORINNER_PIANO_EDIT, OnPianoEdit)
		NOTIFY_HANDLER(IDC_PATTERNEDITOR, ID_PATTERNEDITORINNER_PIANO_TRANSLATE, OnPianoTranslate)
		CMD_ID_HANDLER(ID_PATTERNEDITORINNER_SCROLLED, OnScrolled)
		CMD_ID_HANDLER(ID_PATTERNEDITORINNER_SELCHANGED, OnSelChanged)
		CMD_ID_HANDLER(ID_PATTERNEDITORINNER_SELDRAG, OnSelDrag)
		COMMAND_ID_HANDLER_EX(ID_PATTERNEDITORINNER_SELDROP, OnSelDrop)
		CMD_ID_HANDLER(ID_PATTERNEDITORINNER_SELDROPCELL, OnSelDropCell)
		CMD_ID_HANDLER(ID_PATTERNEDITORINNER_MOVECURSOR, OnMoveCursor)
		CMD_ID_HANDLER(ID_PATTERNEDITORINNER_RESIZED, OnInnerResized)

		// messages sent from patterneditorcontrol
		COMMAND_ID_HANDLER(ID_PATTERNEDITORCONTROL_TRACKMUTE, OnControlTrackMute)
		COMMAND_ID_HANDLER(ID_PATTERNEDITORCONTROL_TRACKSOLO, OnControlTrackSolo)
		//COMMAND_ID_HANDLER(ID_PATTERNEDITORCONTROL_PLAYFROMROW, OnControlPlayFromRow)
		//COMMAND_ID_HANDLER(ID_PATTERNEDITORCONTROL_HOLDROW, OnControlHoldRow)

		// muting/soloing
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRACKMUTE, OnTrackMute)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRACKSOLO, OnTrackSolo)

		// obsolete
		///CMD_RANGE_HANDLER(ID_PATTERNEDITOR_FILTER_FIRST, ID_PATTERNEDITOR_FILTER_FIRST + 1000, OnToggleFilter)
		///NOTIFY_HANDLER(IDC_PATTERNLIST, NM_RCLICK, OnRClickColumnInfo)

		// mutations
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRANSPOSESELECTIONUP, OnTransposeSelectionUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRANSPOSESELECTIONDOWN, OnTransposeSelectionDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRANSPOSENOTESOCTAVEUP, OnTransposeNotesOctaveUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRANSPOSENOTESOCTAVEDOWN, OnTransposeNotesOctaveDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRANSPOSENOTESUP, OnTransposeNotesUp)	
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRANSPOSENOTESDOWN, OnTransposeNotesDown)
		// --
		CMD_ID_HANDLER(ID_PATTERNVIEW_RANDOMIZE_SELECTION, OnRandomizeSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_RANDOMIZERANGE_SELECTION, OnRandomizeRangeSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_RANDOMIZEUSING_SELECTION, OnRandomizeFromSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_HUMANIZE_SELECTION, OnHumanizeSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SHUFFLE_SELECTION, OnShuffleSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INTERPOLATE_SELECTION, OnInterpolateSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_GRADIATE_SELECTION, OnGradiateSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SMOOTH_SELECTION, OnSmoothSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_REVERSE_SELECTION, OnReverseSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_COMPACT_SELECTION, OnCompactSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_EXPAND_SELECTION, OnExpandSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_THIN_SELECTION, OnThinSelection)
			CMD_HANDLER(IDD_TRANSFORM_THIN, IDOK, OnThinOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_REPEAT_SELECTION, OnRepeatSelection)
			CMD_HANDLER(IDD_TRANSFORM_REPEAT, IDOK, OnRepeatOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ECHO_SELECTION, OnEchoSelection)
			CMD_HANDLER(IDD_TRANSFORM_ECHO, IDOK, OnEchoOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_UNIQUE_SELECTION, OnUniqueSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SCALE_SELECTION, OnScaleSelection)
			CMD_HANDLER(IDD_TRANSFORM_SCALE, IDOK, OnScaleOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_FADE_SELECTION, OnFadeSelection)
			CMD_HANDLER(IDD_TRANSFORM_FADE, IDOK, OnFadeOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_CURVEMAP_SELECTION, OnCurveMapSelection)
			CMD_HANDLER(IDD_TRANSFORM_CURVEMAP, IDOK, OnCurveMapOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INVERT_SELECTION, OnInvertSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATEROWS_SELECTION, OnRotateRowsSelection)
			CMD_HANDLER(IDD_TRANSFORM_ROTATEROWS, IDOK, OnRotateRowsOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATEROWS_UP_SELECTION, OnRotateRowsSelectionUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATEROWS_DOWN_SELECTION, OnRotateRowsSelectionDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATEVALUES_SELECTION, OnRotateValsSelection)
			CMD_HANDLER(IDD_TRANSFORM_ROTATEVALS, IDOK, OnRotateValsOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATEVALUES_UP_SELECTION, OnRotateValsSelectionUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATEVALUES_DOWN_SELECTION, OnRotateValsSelectionDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATERHYTHMS_SELECTION, OnRotateRhythmsSelection)
			CMD_HANDLER(IDD_TRANSFORM_ROTATERHYTHMS, IDOK, OnRotateRhythmsOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATERHYTHMS_UP_SELECTION, OnRotateRhythmsSelectionUp)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATERHYTHMS_DOWN_SELECTION, OnRotateRhythmsSelectionDown)
		CMD_ID_HANDLER(ID_PATTERNVIEW_NOTELENGTH_SELECTION, OnNotelengthSelection)
			CMD_HANDLER(IDD_TRANSFORM_NOTELENGTH, IDOK, OnNotelengthOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_VOLUMES_SELECTION, OnVolumesSelection)
			CMD_HANDLER(IDD_TRANSFORM_VOLUMES, IDOK, OnVolumesOK)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRACKSWAP_SELECTION, OnTrackSwapSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ROWSWAP_SELECTION, OnRowSwapSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ALLTOFIRST_SELECTION, OnAllToFirstSelection) // set_events
		CMD_ID_HANDLER(ID_PATTERNVIEW_FIRSTTOLAST_SELECTION, OnFirstToLastSelection) // replace events
		CMD_ID_HANDLER(ID_PATTERNVIEW_REMOVEFIRST_SELECTION, OnRemoveFirstSelection) // remove_events
		CMD_ID_HANDLER(ID_PATTERNVIEW_REPLACEWAVES_SELECTION, OnSetWavesSelection) // set_events
		//CMD_ID_HANDLER(ID_PATTERNVIEW_AMPLIFY_SELECTION, OnAmplifySelection) // scale_events
		//CMD_ID_HANDLER(ID_PATTERNVIEW_ROTATENOTES_SELECTION, OnRotateNotesSelection) // rotate_vals_events
		//CMD_ID_HANDLER(ID_PATTERNVIEW_SETVOLS_SELECTION, OnSetVolsSelection) // set_events
		CMD_ID_HANDLER(ID_PATTERNVIEW_SHOWNOTEMASK, OnShowNoteMaskDialog)
		CMD_ID_HANDLER(ID_PATTERNVIEW_SHOWHARMONICTRANSPOSE, OnShowHarmonicTransposeDialog)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INVERTCHORDUP_SELECTION, OnInvertChordUpSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_INVERTCHORDDOWN_SELECTION, OnInvertChordDownSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_CLEARSAMECOLUMN_SELECTION, OnClearSameColumnSelection)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLE_VOLMASK, OnVolMaskToggle)

		COMMAND_ID_HANDLER_EX(ID_NOTEMASK_TOGGLE, OnNoteMaskToggle)
		COMMAND_ID_HANDLER_EX(ID_NOTEMASK_SOLO, OnNoteMaskSolo)
		COMMAND_ID_HANDLER_EX(ID_NOTEMASK_RESET, OnNoteMaskReset)
		MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
		CMD_ID_HANDLER(ID_PATTERNVIEW_RESELECT, OnReselect)

		CMD_ID_HANDLER(ID_PATTERNVIEW_NOTECOLUMN_TOGGLENOTEMETA, OnToggleNoteMeta)
		COMMAND_ID_HANDLER_EX(ID_TRANSPOSESET_UPDATE, OnTransposeSetUpdate)
		COMMAND_ID_HANDLER_EX(ID_TRANSPOSESET_ENABLE, OnTransposeSetEnable)
		CMD_ID_HANDLER(ID_TRANSPOSESET_REKEY, OnTransposeRekey)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLE_TRANSPOSESET, OnToggleTransposeSet)

		CMD_ID_HANDLER(ID_PATTERNVIEW_TRACKSWAPRIGHT, OnTrackSwapRight)
		CMD_ID_HANDLER(ID_PATTERNVIEW_TRACKSWAPLEFT, OnTrackSwapLeft)
		CMD_ID_HANDLER(ID_PATTERNVIEW_FORMATLAYOUTPLUGINRIGHT, OnFormatLayoutPluginRight)
		CMD_ID_HANDLER(ID_PATTERNVIEW_FORMATLAYOUTPLUGINLEFT, OnFormatLayoutPluginLeft)

		CMD_ID_HANDLER(ID_PATTERNVIEW_CYCLE_NOTESAFFECT, OnCycleNotesAffect)

		CMD_ID_HANDLER(ID_PATTERNVIEW_TOGGLE_ORDERLIST, OnToggleOrderlist)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ORDERLIST_RIGHT, OnOrderlistRight)
		CMD_ID_HANDLER(ID_PATTERNVIEW_ORDERLIST_LEFT, OnOrderlistLeft)

		CMD_ID_HANDLER(ID_PATTERNVIEW_NUDGEBACKWARD, OnNudgeBackward)
		CMD_ID_HANDLER(ID_PATTERNVIEW_NUDGEFORWARD, OnNudgeForward)
		CMD_ID_HANDLER(ID_PATTERNVIEW_NUDGEBACKWARDSMALL, OnNudgeBackwardSmall)
		CMD_ID_HANDLER(ID_PATTERNVIEW_NUDGEFORWARDSMALL, OnNudgeForwardSmall)

		COMMAND_RANGE_HANDLER(ID_PATTERNVIEW_ADD_TRIGGER_FIRST, ID_PATTERNVIEW_ADD_TRIGGER_LAST, OnAddTrigger)
		COMMAND_ID_HANDLER(ID_PATTERNVIEW_ADD_TRIGGER_NEW, OnAddTrigger)

		COMMAND_RANGE_HANDLER(ID_PATTERNVIEW_ADD_SIMPLE_FIRST, ID_PATTERNVIEW_ADD_SIMPLE_LAST, OnAddSimpleFormat)
		COMMAND_RANGE_HANDLER(ID_PATTERNVIEW_ADD_FULL_FIRST, ID_PATTERNVIEW_ADD_FULL_LAST, OnAddFullFormat)

		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	CPatternView(CViewFrame* m, CPatternViewInfo* viewInfo);
	~CPatternView();
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnXButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnGetEditFlags(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnGetStatusText(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnPatternViewWaveToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewMachinePatternToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewOctaveToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewStepToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewScaleToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewPlayNotesToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewBeatToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewPatternViewToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewInfoToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewLoopToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewFollowToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewPatternRowsToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPatternViewPatternNameToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	//void OnHelp();
	void OnSelectAll();
	void OnPatternClone();
	void OnMachineAddTrack();
	void OnMachineRemoveTrack();
	void OnPatternPlay();
	void OnPatternReplay();
	void OnPatternPlayFromCursor();
	void OnPatternStop();
	void OnPatternDelete();
	void OnPatternDoubleRows();
	void OnPatternHalveRows();
	void OnPatternDoubleLength();
	void OnPatternHalveLength();
	void OnCreatePatternFormat();
	void OnClonePatternFormat();
	void OnDeletePatternFormat();
	void OnPatternFormatProperties();
	void OnViewProperties();
	LRESULT OnMachinePatternSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void OnOctaveSelChange();
	void OnStepSelChange();
	void OnScaleSelChange();
	void OnSelChangeBeat();
	void OnDropdownWave();
	void OnDropdownFormat();
	void OnDropdownPattern();
	void OnDropdownOctave();
	void OnDropdownStep();
	void OnDropdownPatternScale();
	void OnDropdownPatternBeat();
	void OnDropdownPatternRows();
	void OnDropdownPatternName();
	void OnTogglePlayNotes();
	void OnToggleShowInfo();
	void OnTogglePatternLoop();
	void OnRestoreFocus();
	void OnToggleColumnControl();
	void OnOctaveUp();
	void OnOctaveDown();
	void OnStepUp();
	void OnStepDown();
	void OnClonePattern();
	void OnPatternCreate();
	void OnGotoPattern();
	void OnCloneGotoPattern();
	void OnPatternUp();
	void OnPatternDown();
	void OnFormatUp();
	void OnFormatDown();
	void OnMachineSolo();
	void OnMachineMute();
	void OnSelectBegin();
	void OnSelectEnd();
	void OnUnselect();
	void OnSelectDown();
	void OnSelectBeat();
	void OnSelectColumns();
	void OnSpecial1();
	void OnSpecial2();
	void OnSpecial3();
	void OnSpecial4();
	void OnSpecial5();
	void OnSpecial6();
	void OnNextWave();
	void OnPrevWave();
	void OnPickupValue();
	void OnMachineParameters();
	void OnMachineProperties();
	void OnParameterInterpolateAbsolute();
	void OnParameterInterpolateInertial();
	void OnParameterInterpolateLinear();
	void OnScrolled();
	void OnSelChanged();
	void OnSelDropCell();
	void OnSelDrag();
	LRESULT OnRClickColumnInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	void OnMoveCursor();
	void OnSetStepRange(WORD wID);
	void OnSetColumnControl(WORD wID);
	void OnDuplicateEditor();
	void OnScrollLinkEditor(WORD wID);
	void OnShowPatternFormat();
	void OnShowPatternList();
	void OnSetLoopBegin();
	void OnSetLoopEnd();
	void OnSetLoopSelection();
	void OnSetLoopPattern();
	void OnToggleFilter(WORD wID);
	void OnBack();
	void OnForward();
	void OnToggleHorizontalEntry();
	void OnClearValue();
	void OnClearTrackRow();
	void OnClearPatternRow();
	void OnHotPastePattern();
	void OnCloneTrigger();
	void OnInfoPaneUp();
	void OnInfoPaneDown();
	void OnInfoPanePageUp();
	void OnInfoPanePageDown();
	void OnResetPatternStack();
	void OnPatternTreeExpandCollapse();
	void OnCollapseSelection();
	void OnUncollapseSelection();
	void OnToggleCollapseSelection();
	void OnToggleCollapseTrack();
	LRESULT OnControlTrackMute(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnControlTrackSolo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	void OnTrackMute();
	void OnTrackSolo();
	LRESULT OnControlPlayFromRow(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnControlHoldRow(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnPatternSelChange();
	void OnPatternRowsSelChange();
	void OnPatternRowsEditAccept(bool restorefocus = true);
	void OnPatternRowsEditCancel();
	void OnPatternRowsKillFocus();
	void OnScaleKillFocus();
	void OnScaleEditAccept(bool restorefocus = true);
	void OnScaleEditCancel();
	void OnStepKillFocus();
	void OnStepEditAccept(bool restorefocus = true);
	void OnStepEditCancel();
	void OnPatternNameEditAccept(bool restorefocus = true);
	void OnPatternNameEditCancel();
	void OnPatternNameKillFocus();
	void OnVolMaskToggle();
	void OnNudgeBackward();
	void OnNudgeForward();
	void OnNudgeBackwardSmall();
	void OnNudgeForwardSmall();
	LRESULT OnAddTrigger(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAddSimpleFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAddFullFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	// events
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	virtual void UpdateTimer(int count);
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);

	// binding
	void BindPatternPosition();
	void BindPatternEditorRows();
	void BindPatternEditorVertical();
	void BindPatternEditorHorizontal();
	void BindPatternEditor();
	// ---
	void BindMachinePatternPanel();
	void BindWaveDropdown();
	void BindStatus();
	void BindToolbar();
	void BindTheme();
	void BindSettings();
	void BindSongPositions();
	void BindToolbarControls();
	void BindColumnInfo();
	void BindPatternInfos();
	void BindTracks();
	void BindPatternEditorFont();
	void SetPatternEditorFont(std::string const& new_font_name, int new_font_size);
	void BindPatternScroll();

	// note playback / entry
	void OnPlayRow();
	void OnJazzRow();
	void DoPlayRow(int time);
	void OnPlayTrackRow();
	void OnJazzTrackRow();
	void DoPlayTrackRow(int plugin_id, int group, int track, int time);
	LRESULT OnEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	void RemoveEdit(int plugin_id, int group, int track, int column, int row);
	LRESULT OnNote(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnPianoEdit(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnPianoTranslate(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	void OnCycleNotesAffect();

	// data binding
	void UpdatePatternEvent(zzub_pattern_event_t* ev, LPARAM lHint);
	void FillPatternEditorColumn(zzub_plugin_t* plugin, int group, int track, int column);
	void FillPatternEditor();

	// cut/copy/paste/drag/clear
	int GetEditFlags();
	void OnCut();
	void OnCopy();
	void OnPaste();
	void OnPasteMixOver();
	void OnPasteMixUnder();
	void OnPasteSplice();
	void OnPasteStep();
	void OnCutSplice();
	void CopyPatternSelection();
	void CutPatternSelection();
	int PastePatternSelection(zzub_pattern_t* pat, int plugin_id, int group, int track, int column, int row, PasteMode mode);
	void DragPatternSelection(RECT rcFrom, POINT ptTo, bool makecopy, PasteMode pastemode);

	// ?
	void InsertPatternSelection();
	int GetSelectedWave();
	void HandleMidi(zzub_event_data_midi_message_t& msg);
	void PasteValue();
	void PasteTrackValues();

	// other view coupling
	zzub_pattern_t* GetPattern();

	// selection mutations
	void TransposeSelection(int delta, bool notesOnly);

	// toolbars
	void UpdateOctaveDropdown();
	void UpdateStepDropdown();
	void UpdateScaleDropdown();
	void UpdateBeatDropdown();
	void UpdatePlayNotesCheckbox();
	void UpdateShowInfoCheckbox();
	void UpdatePatternRowsDropdown();
	void UpdatePatternNameEdit();
	void UpdatePatternLoopCheckbox();
	void EnableToolbands();
	void DisableToolbands();
	BOOL GetToolbarVisibility(WORD wID);

	// track muting
	void DoTrackSolo(int current_idx);
	void DoTrackMute(int current_idx);

	// pattern stack
	void ResetPatternStack();
	void SetPatternResetStack(zzub_pattern_t* newpattern, zzub_pattern_format_t* newformat = 0);
	void SetPatternPushStack(zzub_pattern_t* newpattern, zzub_pattern_format_t* newformat = 0);
	void SetPattern(zzub_pattern_t* newpattern, zzub_pattern_format_t* newformat = 0);
	void SetPatternUser(zzub_pattern_t* newpattern, zzub_pattern_format_t* newformat = 0); // calls SetPattern or SetPatternPushStack depending on configuration setting
	void PatternStackCleanup();
	void PatternStackRemovePattern(zzub_pattern_t* pat);
	void GotoPattern();

	// xxx
	void SelectWave(int patwave);

	// column actions
	void ClonePattern();

	// InfoPane
	CSplitterWindowKey infoSplitter;
	CInfoPane infoPane;
	void BindPatternTree();
	void GetValueInfos(PE_column const& col, std::vector<PE_valueinfo>& values);
	void GetPatternValueInfos(PE_column const& col, std::vector<PE_valueinfo>& values);
	void GetRangeValueInfos(PE_column const& col, std::vector<PE_valueinfo>& values);
	void CheckColumnInfoVisibility();
	void BindInfoPane();

	// helpers
	bool InvalidPattern() const;
	void NudgeBackwardAmount(int delayamount);
	void NudgeForwardAmount(int delayamount);

	// --- pattern transforms ---
	void DeletePatternSelection();
	// --- edit op transforms ---
	void OnInsertTrackRow();
	void OnInsertColumnRow();
	void OnInsertPatternRow();
	void OnDeleteTrackRow();
	void OnDeleteColumnRow();
	void OnDeletePatternRow();
	void OnBackspaceColumnRow();
	void OnBackspaceTrackRow();
	void OnBackspacePatternRow();
	void OnDelete();
	LRESULT OnSelDrop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	// --- selection transforms ---
	void OnTransposeSelectionUp();
	void OnTransposeSelectionDown();
	void OnTransposeNotesUp();
	void OnTransposeNotesDown();
	void OnTransposeNotesOctaveUp();
	void OnTransposeNotesOctaveDown();
	void OnRandomizeSelection();
	void OnRandomizeRangeSelection();
	void OnRandomizeFromSelection();
	void OnHumanizeSelection();
	void OnShuffleSelection();
	void OnInterpolateSelection();
	void OnGradiateSelection();
	void OnSmoothSelection();
	void OnReverseSelection();
	void OnExpandSelection();
	void OnCompactSelection();
	void OnThinSelection();
	void OnRepeatSelection();
	void OnEchoSelection();
	void OnUniqueSelection();
	void OnInvertSelection();
	void OnScaleSelection();
	void OnRotateRowsSelection();
	void OnRotateRowsSelectionUp();
	void OnRotateRowsSelectionDown();
	void OnRotateValsSelection();
	void OnRotateValsSelectionUp();
	void OnRotateValsSelectionDown();
	void OnRotateRhythmsSelection();
	void OnRotateRhythmsSelectionUp();
	void OnRotateRhythmsSelectionDown();
	void OnRotateNotesSelection();
	void OnRotateNotesSelectionUp();
	void OnRotateNotesSelectionDown();
	void OnAllToFirstSelection();
	void OnFirstToLastSelection();
	void OnRemoveFirstSelection();
	void OnSetWavesSelection();
	void OnTrackSwapSelection();
	void OnNotelengthSelection();
	void OnVolumesSelection();
	void OnFadeSelection();
	void OnCurveMapSelection();
	void OnRowSwapSelection();
	void OnClearSameColumnSelection();
	void OnAmplifySelection();
	void OnSetVolsSelection();
	CRepeatDialog repeatDlg;
	void OnRepeatOK();
	CEchoDialog echoDlg;
	void OnEchoOK();
	CNotelengthDialog notelengthDlg;
	void OnNotelengthOK();
	CThinDialog thinDlg;
	void OnThinOK();
	CScaleDialog scaleDlg;
	void OnScaleOK();
	CFadeDialog fadeDlg;
	void OnFadeOK();
	CCurveMapDialog curvemapDlg;
	void OnCurveMapOK();
	CRotateRowsDialog rotaterowsDlg;
	void OnRotateRowsOK();
	CRotateValsDialog rotatevalsDlg;
	void OnRotateValsOK();
	CRotateRhythmsDialog rotaterhythmsDlg;
	void OnRotateRhythmsOK();
	CVolumesDialog volumesDlg;
	void OnVolumesOK();
	void OnInvertChordUpSelection();///
	void OnInvertChordDownSelection();///
	bool volume_masked;

	// note masks
	std::vector<int> event_exclusions;
	CNoteMaskDialog notemaskDlg;
	void OnShowNoteMaskDialog();
	LRESULT OnNoteMaskToggle(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnNoteMaskSolo(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnNoteMaskReset(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	void OnReselect();

	// harmonic transpose
	CHarmonicTransposeDialog harmonicxposeDlg;
	void OnShowHarmonicTransposeDialog();
	LRESULT OnTransposeSetUpdate(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnTransposeSetEnable(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	void OnTransposeRekey();
	void OnToggleTransposeSet();
	void OnWindowPosChanged(LPWINDOWPOS lpWndPos);

	// harmony h-sys
	void OnToggleNoteMeta();
	void AddSpecialPlugins(zzub_plugin_t* plugin);
	void UpdateHSys(int track, int time, int value, int meta, LPARAM lHint);
	void BindSpecialPlugins();
	int hsys_plugin_id;

	// scroller
	CSplitterWindowKey scrollSplitter;
	CPatternEditorScroller editorScroller;
	void BindScrollerWidth();
	void OnInnerResized();
	void BindScrollSplitterDoubleClickPos();

	// format rearranging
	void OnFormatLayoutPluginRight();
	void OnFormatLayoutPluginLeft();
	bool DoFormatLayoutPlugin(bool direction);
	void OnTrackSwapRight();
	void OnTrackSwapLeft();

	// orderlist
	CPatternEditorOrderlist orderList;
	void OnToggleFollow();
	void UpdateFollowCheckbox();
	void OnToggleOrderlist();
	void OnOrderlistRight();
	void OnOrderlistLeft();
	void MakeDestroyOrderlist();

	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	virtual void SetToolbarVisibility(int id, bool state);
	virtual void GetHelpText(char* text, int* len);
	void UpdateToolbarFromConfiguration(WORD wID);
	void UpdateToolbarsFromConfiguration();
};
