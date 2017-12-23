#pragma once

// ---------------------------------------------------------------------------------------------------------------
// TYPES
// ---------------------------------------------------------------------------------------------------------------

static const int WM_SELECTMIDICONTROLLER = WM_USER+1;
//static const int WM_CLOSE_CLIENTVIEW = WM_USER+2;
static const int WM_CLOSE_NO_QUESTIONS_ASKED = WM_USER+3;
static const int WM_CUSTOMVIEW_CREATE = WM_USER+4;
static const int WM_CUSTOMVIEW_SET_CHILD = WM_USER+5;
static const int WM_CUSTOMVIEW_GET = WM_USER+6;
static const int WM_CUSTOMVIEW_FOCUS = WM_USER+7;
static const int WM_GET_THEME = WM_USER+8;
static const int WM_GET_EDITFLAGS = WM_USER+9;
static const int WM_CUSTOMVIEW_SET_PRETRANSLATE = WM_USER+10;
static const int WM_FORWARD_PRETRANSLATE = WM_USER+11;
static const int WM_ADD_PALETTEWINDOW = WM_USER+12;
static const int WM_REMOVE_PALETTEWINDOW = WM_USER+13;
static const int WM_GET_STATUS_TEXT = WM_USER+14; // sent by mainframe to client views = client views must reserve message!
static const int WM_SHOW_PARAMVIEW = WM_USER+15; // wParam = mode, lParam = machine

enum EDITFLAGS {
	EDIT_COPY = 1,	// set if selection can be copied
	EDIT_PASTE = 2,	// set if clipboard format is recognized
};

enum MachineParameterViewMode;

class CTimerHandler
{
  public:

	virtual ~CTimerHandler() {};
	virtual void UpdateTimer(int count) = 0;
};

// ---------------------------------------------------------------------------------------------------------------
// MASTER TOOLBAR
// ---------------------------------------------------------------------------------------------------------------

#include "ToolbarWindow.h"
#include "ToolbarBands.h"
#include "VisualVolumeSlider.h"
#include "TextLabel.h"
#include "HelpHost.h"

class CMainFrame;

class CMasterToolbar
:
	public CWindowImpl<CMasterToolbar>,
	public CThemedControl<CMasterToolbar>
{
  public:

	CMainFrame* mainFrame;

	CTextLabel volumeLabel;
	CTextLabel bpmLabel;
	CTextLabel tpbLabel;

	CVisualVolumeSlider volumeSlider;
	CIntegralEdit bpmDropDown;
	CIntegralEdit tpbDropDown;

	bool editing_bpm, editing_tpb;

	DECLARE_WND_CLASS("MasterToolbar")

	BEGIN_MSG_MAP_EX(CMasterToolbar)
		MESSAGE_HANDLER_EX(WM_CREATE, OnCreate)
		MESSAGE_HANDLER_EX(WM_SIZE, OnSize)
		COMMAND_HANDLER_EX(IDC_EDIT_BPM, EN_SETFOCUS, OnSetBpmFocus)
		COMMAND_HANDLER_EX(IDC_EDIT_BPM, EN_KILLFOCUS, OnKillBpmFocus)
		COMMAND_HANDLER_EX(IDC_EDIT_TPB, EN_SETFOCUS, OnSetTpbFocus)
		COMMAND_HANDLER_EX(IDC_EDIT_BPM, WM_INTEGRALEDIT_ACCEPT, OnBpmAccept)
		COMMAND_HANDLER_EX(IDC_EDIT_BPM, WM_INTEGRALEDIT_CANCEL, OnBpmCancel)
		COMMAND_HANDLER_EX(IDC_EDIT_TPB, EN_KILLFOCUS, OnKillTpbFocus)
		COMMAND_HANDLER_EX(IDC_EDIT_TPB, WM_INTEGRALEDIT_ACCEPT, OnTpbAccept)
		COMMAND_HANDLER_EX(IDC_EDIT_TPB, WM_INTEGRALEDIT_CANCEL, OnTpbCancel)

		NOTIFY_HANDLER(IDC_MASTERSLIDER, NM_SETVALUE, OnSetValue)
		NOTIFY_HANDLER(IDC_MASTERSLIDER, NM_SETVALUEPREVIEW, OnSetValuePreview)
		CHAIN_MSG_MAP(CThemedControl<CMasterToolbar>)
	END_MSG_MAP()

	CMasterToolbar(CMainFrame* mainFrm);
	~CMasterToolbar();

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);

	LRESULT OnSetBpmFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	LRESULT OnKillBpmFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	LRESULT OnBpmAccept(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	LRESULT OnBpmCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	LRESULT OnSetTpbFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	LRESULT OnKillTpbFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	LRESULT OnTpbAccept(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	LRESULT OnTpbCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	LRESULT OnSetValue(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnSetValuePreview(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	void UpdateSettings();
};

// ---------------------------------------------------------------------------------------------------------------
// SONG TOOLBAR
// ---------------------------------------------------------------------------------------------------------------

#include "ToolbarWindow.h"

class CSongToolbar
:
	public CWindowImpl<CSongToolbar>,
	public CThemedControl<CSongToolbar>
{
  public:

	CMainFrame* mainFrame;

	CTextLabel elapsedTimeLabel;
	CTextLabel currentTimeLabel;
	CTextLabel loopTimeLabel;

	double last_time;
	float last_current;
	float last_loop;

	DECLARE_WND_CLASS("SongToolbar")

	BEGIN_MSG_MAP(CSongToolbar)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(CThemedControl<CSongToolbar>)
	END_MSG_MAP()

	CSongToolbar(CMainFrame* mainFrm);
	~CSongToolbar();

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void update();
};

// ---------------------------------------------------------------------------------------------------------------
// MAINFRAME
// ---------------------------------------------------------------------------------------------------------------

#include "plur.h"

#include "DockTabFrame/DockTabFrame.h"
#include "DockTabFrame/DockTabViewManager.h"
using namespace DockSplitTab;

#include "CommandBar.h"

#include "pugxml.h"

#include "Hotkeys.h"

#include <set>

class CDocument;
class CCustomView;
class MachineItem;
class MachineMenu;
class IndexItem;
class CCreateViewInfo; // remove when fixing the CreateView-stuff

class CMainFrame
:
	public CFrameWindowImpl<CMainFrame>,
	public CUpdateUI<CMainFrame>,
	public CMessageFilter,
	public CIdleHandler,
	public DockSplitTab::FrameListener,
	public CViewFrame,
	public CEventHandler
{
  public:

	DECLARE_FRAME_WND_CLASS("MainFrame", IDR_MAINFRAME)

	// major
	DockSplitTab::Frame frame;

	// view managers
	CAtlMap<HWND, ClientView*> clientViews;
	DockTabViewManager<CMainFrame, CCustomView> customView;
	std::vector<DynamicDockTabViewManager<CMainFrame, CView>*> views;

	bool hideAllParameters;

	// hotkeys
	CHotkeys hotkeys;

	// screensets
	int current_screenset;
	pug::xml_parser screensets_parser;

	// view stack
	std::vector<std::pair<HWND, bool> > viewStack; // bool -> Keyboard Tabbable
	int viewStackPosition;

	// toolbars
	CBuzeCommandBarCtrl m_CmdBar;
	CToolBarCtrl transportCtrl;
	CMasterToolbar masterToolbar;
	CSongToolbar songToolbar;
	bool m_bLockedToolbars;
	CMultiPaneStatusBarCtrl statusBar;

	// x
	bool isStandalone;

	// machine menus
	CMenu createMachineMenu;
	CMenu insertbeforeMachineMenu;
	CMenu insertafterMachineMenu;
	CMenu replaceMachineMenu;

	// menus
	PlurManager plur;
	std::vector<std::string> recents;
	CMenu themeMenu;
	CMenu recentMenu;

	// x
	bool dirtyTitlebarAsterisk;

	// timer
	std::vector<CView*> timerHandlers;
	int timerCount;

	bool menu_has_accelerators;
	CBuzeConfiguration configuration;
	signed __int64 qpfFreq;
	signed __int64 qpfStart;
	std::map<WORD, int> commandEventMap;
	int nextEventCommand;
	CIcon lockIcon, unlockIcon;
	CComObject<CHelpHost>* helpHost;
	int nextEventCode;

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
		/*UPDATE_ELEMENT(ID_VIEW_PRIMARYPATTERNEDITOR, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_VIEW_SECONDARYPATTERNEDITOR, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_VIEW_MACHINES, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_VIEW_WAVETABLE, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_VIEW_NEWPATTERNEDITOR, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_VIEW_PATTERNFORMAT, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_VIEW_ANALYZER, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_CPUMETER, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_HARDDISKRECORDER, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_ALLMACHINES, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_FILES, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_HISTORY, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_COMMENT, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)*/
		UPDATE_ELEMENT(ID_DEVICE_RESET, UPDUI_TOOLBAR)

		//UPDATE_ELEMENT(ID_VIEW_VIEWS, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_MASTERTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_TRANSPORTTOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_TIMETOOLBAR, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_LOCK_TOOLBARS, UPDUI_MENUPOPUP)

		UPDATE_ELEMENT(ID_RECORD, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PLAY, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PLAY_REPEAT, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_PLAY_SYNC, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_TOGGLE_AUTOSAVE, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_RENDER_SEQUENCE, UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_UNDO, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_REDO, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_CUT, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP_EX(CMainFrame)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)

		// wm's
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_CLOSE_NO_QUESTIONS_ASKED, OnCloseNoQuestions)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		//MESSAGE_HANDLER(WM_CLOSE_CLIENTVIEW, OnCloseClientView)
		MESSAGE_HANDLER(WM_MEASUREITEM, OnMeasureItem)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)

		// custom views
		MESSAGE_HANDLER(WM_CUSTOMVIEW_CREATE, OnCustomViewCreate);
		MESSAGE_HANDLER(WM_CUSTOMVIEW_GET, OnCustomViewGet);
		MESSAGE_HANDLER(WM_CUSTOMVIEW_FOCUS, OnCustomViewFocus);
		MESSAGE_HANDLER(WM_CUSTOMVIEW_SET_CHILD, OnCustomViewSetChild);
		MESSAGE_HANDLER(WM_CUSTOMVIEW_SET_PRETRANSLATE, OnCustomViewSetPreTranslate);
		MESSAGE_HANDLER(WM_FORWARD_PRETRANSLATE, OnForwardPreTranslate); // used by buzz2zzub
		MESSAGE_HANDLER(WM_GET_THEME, OnGetTheme);

		// paramview helper
		//MESSAGE_HANDLER(WM_SHOW_PARAMVIEW, OnShowMachineParameters)

		// docktabframe
		COMMAND_ID_HANDLER(ID_CLOSEPANE, OnClosePane)

		// show views
		/*COMMAND_ID_HANDLER(ID_VIEW_PATTERNFORMAT, OnShowPatternFormatEditor)
		COMMAND_ID_HANDLER(ID_VIEW_CPUMETER, OnShowCpuMeter)
		COMMAND_ID_HANDLER(ID_VIEW_FILES, OnShowFiles)
		COMMAND_ID_HANDLER(ID_VIEW_ALLMACHINES, OnShowAllMachines)
		COMMAND_ID_HANDLER(ID_VIEW_PRIMARYPATTERNEDITOR, OnShowPrimaryPatternEditor)
		COMMAND_ID_HANDLER(ID_VIEW_SECONDARYPATTERNEDITOR, OnShowSecondaryPatternEditor)
		COMMAND_ID_HANDLER(ID_VIEW_NEWPATTERNEDITOR, OnShowNewPatternEditor)
		COMMAND_ID_HANDLER(ID_VIEW_MACHINES, OnShowMachines)
		COMMAND_ID_HANDLER(ID_VIEW_WAVETABLE, OnShowWaveTable)
		COMMAND_ID_HANDLER(ID_VIEW_ANALYZER, OnShowAnalyzer)
		COMMAND_ID_HANDLER(ID_VIEW_PREFERENCES, OnShowPreferences)
		COMMAND_ID_HANDLER(ID_VIEW_HISTORY, OnShowHistory)
		COMMAND_ID_HANDLER(ID_VIEW_PROPERTIES, OnViewProperties)
		COMMAND_ID_HANDLER(ID_VIEW_COMMENT, OnViewComment)*/

		// file menu
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
		COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnFileSaveAs)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		//COMMAND_ID_HANDLER(ID_VIEW_VIEWS, OnViewViewsToolbar)
		COMMAND_ID_HANDLER(ID_VIEW_TRANSPORTTOOLBAR, OnViewTransportToolbar)
		COMMAND_ID_HANDLER(ID_VIEW_MASTERTOOLBAR, OnViewMasterToolbar)
		COMMAND_ID_HANDLER(ID_VIEW_TIMETOOLBAR, OnViewTimeToolbar)

		// toolbar locking
		COMMAND_ID_HANDLER(ID_LOCK_TOOLBARS, OnLockToolbars)

		// x
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)

		// menu commands
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(ID_VIEW_HELP, OnHelp)
		COMMAND_ID_HANDLER(ID_VIEW_MANUAL, OnViewManual)
		COMMAND_ID_HANDLER(ID_APP_UPDATE, OnAppUpdate)
		COMMAND_ID_HANDLER(ID_EDIT_UNDO, OnUndo)
		COMMAND_ID_HANDLER(ID_EDIT_REDO, OnRedo)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnCut)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnPaste)
		COMMAND_RANGE_HANDLER(ID_SET_RECENT_FIRST, ID_SET_RECENT_LAST, OnSetRecent)
		COMMAND_RANGE_HANDLER(ID_SET_THEME_FIRST, ID_SET_THEME_LAST, OnSetTheme)

		// tabbable views
		COMMAND_ID_HANDLER(ID_NEXT_PANE, OnNextWindow)
		COMMAND_ID_HANDLER(ID_PREV_PANE, OnPrevWindow)
		COMMAND_ID_HANDLER(ID_VIEW_SHOWHIDEALLPARAMETERFRAMES, OnShowHideAllParameterFrames)

		// screenset support
		COMMAND_RANGE_HANDLER(ID_SCREENSET_RECALL_FIRST, ID_SCREENSET_RECALL_LAST, OnScreensetRecall)
		COMMAND_RANGE_HANDLER(ID_SCREENSET_STORE_FIRST, ID_SCREENSET_STORE_LAST, OnScreensetStore)

		// player engine related messages
		COMMAND_ID_HANDLER(ID_PLAY_FROM_START, OnPlayFromStart)
		COMMAND_ID_HANDLER(ID_PLAY, OnPlay)
		COMMAND_ID_HANDLER(ID_PLAYFROMCURSOR, OnPlayFromCursor)
		COMMAND_ID_HANDLER(ID_STOP, OnStop)
		COMMAND_ID_HANDLER(ID_RECORD, OnRecord)
		COMMAND_ID_HANDLER(ID_PLAY_REPEAT, OnPlayRepeat)
		COMMAND_ID_HANDLER(ID_PLAY_SYNC, OnPlaySync)
		COMMAND_ID_HANDLER(ID_DEVICE_RESET, OnResetDevice)
		COMMAND_ID_HANDLER(ID_MAINFRAME_BPM_UP, OnMainFrameBpmUp) // triggered by key combo ctrl + ]
		COMMAND_ID_HANDLER(ID_MAINFRAME_BPM_DOWN, OnMainFrameBpmDown) // triggered by key combo ctrl + [
		COMMAND_ID_HANDLER(ID_KEYJAZZ_OCTAVE_UP, OnKeyjazzOctaveUp)
		COMMAND_ID_HANDLER(ID_KEYJAZZ_OCTAVE_DOWN, OnKeyjazzOctaveDown)

		COMMAND_RANGE_HANDLER(ID_MAINFRAME_EVENT_FIRST, ID_MAINFRAME_EVENT_LAST, OnMainFrameEventCommand)

		//MSG_WM_NCACTIVATE(OnNcActivate)
		MESSAGE_HANDLER_EX(WM_NCACTIVATE, OnNcActivate);
		MESSAGE_HANDLER_EX(WM_ADD_PALETTEWINDOW, OnAddPaletteWindow)
		MESSAGE_HANDLER_EX(WM_REMOVE_PALETTEWINDOW, OnRemovePaletteWindow)

		NOTIFY_HANDLER(ATL_IDW_STATUS_BAR, NM_CLICK, OnClickStatus)
	END_MSG_MAP()

//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) ;
	LRESULT OnNextWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPrevWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) ;
	LRESULT OnShowHideAllParameterFrames(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) ;
	LRESULT OnResetDevice(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowObjectProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClosePane(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCloseNoQuestions(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPlay(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPlayFromCursor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPlayFromStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStop(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRecord(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPlayRepeat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPlaySync(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//LRESULT OnViewViewsToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewMasterToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewTransportToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewTimeToolbar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewManual(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAppUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUndo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnRedo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSetTheme(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSetRecent(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//LRESULT OnCloseClientView(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMeasureItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDrawItem(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCustomViewCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCustomViewGet(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCustomViewFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCustomViewSetChild(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCustomViewSetPreTranslate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnForwardPreTranslate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnGetTheme(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMainFrameBpmUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMainFrameBpmDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnKeyjazzOctaveUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnKeyjazzOctaveDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnScreensetRecall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnScreensetStore(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnLockToolbars(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	//LRESULT OnShowMachineParameters(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMainFrameEventCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClickStatus(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	
	// x
	CMainFrame();
	~CMainFrame();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	// CViewFrame implementation
	virtual bool CreateFrame(HWND hParentWnd);
	virtual HWND GetHwnd();

	// view updates
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);

	// docktabframe support
	void clientViewHide(HWND clientViewWnd);
	void clientActivated(ClientView* clientView); // FrameListener interface
	void clientDetached(ClientView* clientView); // FrameListener interface
	void clientChangedPlace(ClientView* clientView, DockSplitTab::FramePlace place, DockSplitTab::DockSide side); // FrameListener interface
	DockSplitTab::ClientView* createClientWindow(HWND hWnd, std::string const& caption = "", std::string const& toolTip = "", int imageIndex = -1);
	void ViewStackRemove(HWND hWnd);
	void ViewStackInsert(HWND hViewWnd, bool keyboard_tabable);
	void ViewStackMoveToFront(HWND viewWnd);

	// tabbable views
	void ctrlDown(); // ctrlUp&down are used to keep track of window toggling with ctrl-(shift-)tab
	void ctrlUp();

	// screensets
	bool initScreenset(int screenset);
	bool recallScreenset(int screenset, std::vector<ClientView*>& current_views);
	bool loadScreensetsFromFile(std::string const& fileName);
	bool storeScreenset(int screenset);
	bool saveScreensetsToFile(std::string const& fileName);
	void checkDetachedForPrimarySecondary(ClientView* clientView);
	void clientDetachedForRecycle(ClientView* clientView);
	ClientView* recycleClientWindow(ClientView* clientView);

	// show views
	void ShowMachineParameters(zzub_plugin_t* m, MachineParameterViewMode modehint, int x = -1, int y = -1);
	void showHideAllParameterFrames(bool toggle, bool show);

	DynamicDockTabViewManager<CMainFrame, CView>* GetViewInfo(std::string name);
	void CloseView(HWND hWnd);
	CView* GetView(const char* classname, int view_id);
	void RegisterViewInfo(CViewInfo* viewInfo);
	virtual HACCEL GetAccelerators(const char* viewname);
	virtual HWND GetFocusedClientView();
	virtual CView* GetFocusedView();
	virtual bool IsFloatView(HWND hViewWnd);
	virtual HMENU GetMachineMenuCreate();
	virtual HMENU GetMachineMenuInsertAfter();
	virtual HMENU GetMachineMenuInsertBefore();
	virtual HMENU GetMachineMenuReplace();
	virtual void AddMenuKeys(const char* viewname, HMENU hMenu);
	virtual int RegisterEvent();
	virtual WORD RegisterAcceleratorEvent(const char* name, const char* hotkey, int event);
	virtual void RegisterAccelerator(const char* viewname, const char* name, const char* hotkey, WORD id);
	virtual void* GetKeyjazzMap();
	virtual CView* OpenView(const char* viewname, const char* label, int view_id, int x = -1, int y = -1);
	CView* CreateView(CCreateViewInfo* cvi);

	virtual void SetFocusTo(HWND hWnd);
	virtual HMENU GetMainMenu();
	virtual CView* GetViewByHwnd(HWND hWnd);

	// open / close
	const char* GetOpenFileName();
	const char* GetSaveFileName();
	bool openSongFromFile(std::string const& fileName);
	bool saveIfDirty();
	bool saveCurrentSong(bool saveAs);
	bool saveFile(std::string const& filename, bool withWaves);
	bool clearSong();
	bool isRunningAsStandalone();

	// recent songs
	void initializeRecent(bool update = false);
	std::string getRecentName(size_t index);
	void setMostRecent(std::string const& fileName);

	// status / title
	void setStatus(std::string const& text);
	void setWindowTitle(std::string const& text);

	// timers
	void AddTimerHandler(CView* handler);
	void RemoveTimerHandler(CView* handler);
	void initTimer();
	void updateTick();

	// machine preloading
/*	void preloadMachines();	// walks through index.txt and preloads those marked as such
	void preloadMachines(IndexItem* item, std::map<zzub_pluginloader_t*, std::vector<std::string> >& libs);	// walks through index.txt and preloads those marked as such
	void preloadMachine(MachineItem* machineItem, std::map<zzub_pluginloader_t*, std::vector<std::string> >& libs);
	void populateTemplateDirectory(MachineMenu* parent, std::string const& path);
	void populateUnsortedMachines(); // adds an entry to index.txt with machines not previously listed
*/
	// create machine menu
	int buildMachineMenu(CMenuHandle uiMenu, buze_plugin_index_item_t* machineMenu, int firstCommand, int& index);
	void showCreateMachineContext(POINT pt);
	void rebuildMachineMenus();

	// themes
	void initializeThemes();
	void setTheme(std::string const& theme);

	// toolbars
	void createToolbars();
	void saveToolbarState();
	void loadToolbarState();
	BOOL getToolbarVisibility(WORD wID);
	void setToolbarVisibility(WORD wID, BOOL bVisible, BOOL bSave = TRUE, BOOL bUpdate = TRUE);
	void setPatternToolbarVisibility(int nIndex, WORD wID, BOOL bVisible, BOOL bSave = TRUE, BOOL bUpdate = TRUE);
	void setMainFrameToolbarVisibility(WORD wID, BOOL bVisible, BOOL bSave = TRUE, BOOL bUpdate = TRUE);
	void UpdateSettings();

	//BOOL OnNcActivate(BOOL bActive);
	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnAddPaletteWindow(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRemovePaletteWindow(UINT uMsg, WPARAM wParam, LPARAM lParam);
	std::set<HWND> paletteWnds;

	// xxx
	void OnInitMenuPopup(CMenu menuPopup, UINT nIndex, BOOL bSysMenu);
	void OnInitMenu(CMenu menu);

	// elapsed time readout
	void elapsedTimeReset();
	double elapsedTimeGet();
	virtual const char* GetProgramName();
};
