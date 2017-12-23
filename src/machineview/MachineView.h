#pragma once

#include "DragDropImpl.h"
#include "MachineDropTarget.h"
#include "ConnectionDialog.h"
#include "GraphCtrl.h"

class CViewFrame;
class CBuzeConfiguration;

class CMachineViewInfo : public CViewInfoImpl {
public:
	CMachineViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

static const int WM_GET_EDITFLAGS = WM_USER+9;
static const int MAX_MACHINE_FONTS = 10;

struct CONNECTMENUITEM {
	zzub_connection_type type;
	std::string target_midi_out_device;
	int source_param_index;
	int target_group_index;
	int target_track_index;
	int target_param_index;
	int first_input, first_output;
	int inputs, outputs, flags;
	zzub_plugin_t* from_plugin;
	zzub_plugin_t* to_plugin;
};

enum {
	machine_createtype_create,
	machine_createtype_insert,
	machine_createtype_insertbefore,
	machine_createtype_insertafter,
	machine_createtype_replace,
};

class CMachineView 
	: public CViewImpl
	, public CUpdateUI<CMachineView>
	, public CMachineDropTargetWindow
	, public CWindowImpl<CMachineView>
	, public CMessageFilter
    , public CIdleHandler
{

public:
	CBuzeConfiguration configuration;
	float pxSizeX, pxSizeY; // measures that helds value of 1px's size. @see calcPixelSize()
	bool dirtyVisibleMachines;
	std::map<std::string, std::string> midiAliases;
	std::list<float> machinesListX, machinesListY; // used for aligning machines. @see getNearestMachineLocation(..);
	zzub_plugin_t* lastMidiMachine;	// copy of midi plugin when unfocused
	bool useSkins;
	CEventConnectionDialog eventConnDlg;
	CAudioConnectionDialog audioConnDlg;
	float createMachinePosX, createMachinePosY;	// brukes ved høyreklikk og når man lager en maskin
	std::vector<CONNECTMENUITEM> connectOptions;
	CGraphCtrl graphCtrl;

	zzub_plugin_group_t* currentlayer;

	DECLARE_WND_CLASS("MachineView")

	BEGIN_UPDATE_UI_MAP(CMachineView)
		UPDATE_ELEMENT(ID_EDIT_CUT, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_PASTE_NO_DATA, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_DELETE, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMachineView)
		CHAIN_MSG_MAP(CUpdateUI<CMachineView>)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnBlur)
//		MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMButtonDown)
//		MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		//MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_GET_EDITFLAGS, OnGetEditFlags)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		COMMAND_ID_HANDLER(ID_HELP, OnHelp)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnCut)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnPaste)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE_NO_DATA, OnPasteNoData)
		COMMAND_ID_HANDLER(ID_EDIT_COPY_AUDIOCONNECTIONPARAMETERS, OnCopyAudioConnectionParameters)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE_AUDIOCONNECTIONPARAMETERS, OnPasteAudioConnectionParameters)

		COMMAND_ID_HANDLER(ID_EDIT_DELETE, OnDelete)
		COMMAND_ID_HANDLER(ID_MACHINE_DELETE_AND_RESTORE, OnDeleteAndRestoreConnections)
		COMMAND_ID_HANDLER(ID_EDIT_SELECTALL, OnSelectAll)
		COMMAND_ID_HANDLER(ID_EDIT_CLEARSELECTION, OnClearSelection)
		COMMAND_ID_HANDLER(ID_AUDIO_CONNECTION_PROPERTIES, OnAudioConnectionProperties)
		COMMAND_ID_HANDLER(ID_MIDI_CONNECTION_PROPERTIES, OnMidiConnectionProperties)
		COMMAND_ID_HANDLER(ID_EVENT_CONNECTION_PROPERTIES, OnEventConnectionProperties)
		COMMAND_ID_HANDLER(ID_NOTE_CONNECTION_PROPERTIES, OnNoteConnectionProperties)
		COMMAND_ID_HANDLER(ID_AUDIO_CONNECTION_PARAMETERS, OnAudioConnectionParameters)
		COMMAND_ID_HANDLER(ID_MIDI_CONNECTION_PARAMETERS, OnMidiConnectionParameters)
		COMMAND_ID_HANDLER(ID_EVENT_CONNECTION_PARAMETERS, OnEventConnectionParameters)
		COMMAND_ID_HANDLER(ID_NOTE_CONNECTION_PARAMETERS, OnNoteConnectionParameters)
		COMMAND_ID_HANDLER(ID_EVENT_CONNECTION_BINDINGS, OnEventConnectionBindings)
		COMMAND_ID_HANDLER(ID_MACHINE_PARAMETERS, OnMachineParameters)
		// Section for moving machines with keyboard keys
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_UP, OnMachineMoveUp)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_DOWN, OnMachineMoveDown)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_LEFT, OnMachineMoveLeft)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_RIGHT, OnMachineMoveRight)

		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_UP_STEP, OnMachineMoveUpByStep)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_DOWN_STEP, OnMachineMoveDownByStep)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_LEFT_STEP, OnMachineMoveLeftByStep)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_RIGHT_STEP, OnMachineMoveRightByStep)

		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_UP_LEFT, OnMachineMoveUpLeft)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_DOWN_LEFT, OnMachineMoveDownLeft)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_DOWN_RIGHT, OnMachineMoveDownRight)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_UP_RIGHT, OnMachineMoveUpRight)

		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_UP_LEFT_STEP, OnMachineMoveUpLeftByStep)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_DOWN_LEFT_STEP, OnMachineMoveDownLeftByStep)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_DOWN_RIGHT_STEP, OnMachineMoveDownRightByStep)
		COMMAND_ID_HANDLER(ID_MACHINE_MOVE_UP_RIGHT_STEP, OnMachineMoveUpRightByStep)
		//
		COMMAND_ID_HANDLER(ID_MACHINE_DISCONNECT_AUDIO, OnDisconnectMachinesAudio)
		COMMAND_ID_HANDLER(ID_MACHINE_DISCONNECT_MIDI, OnDisconnectMachinesMidi)
		COMMAND_ID_HANDLER(ID_MACHINE_DISCONNECT_EVENT, OnDisconnectMachinesEvent)
		COMMAND_ID_HANDLER(ID_MACHINE_DISCONNECT_NOTE, OnDisconnectMachinesNote)
		COMMAND_ID_HANDLER(ID_MACHINE_HELP, OnMachineHelp)
		COMMAND_ID_HANDLER(ID_MACHINE_PROPERTIES, OnMachineProperties)
		COMMAND_ID_HANDLER(ID_MACHINE_MINIMIZE, OnMinimizeMachine)
		COMMAND_ID_HANDLER(ID_MACHINEVIEW_UNMINIMIZE, OnUnMinimizeMachine)
		COMMAND_ID_HANDLER(ID_MACHINE_HIDE_IN_CONNECTIONS, OnMachineHideIncomingConnections)

		COMMAND_ID_HANDLER(ID_MACHINE_TOGGLE_CONNECTION_TEXT, OnToggleConnectionText)
		CMD_ID_HANDLER(ID_CREATE_GROUP, OnCreateGroup)

		CMD_HANDLER(IDD_CONNECTION_EVENT, IDOK, OnEventConnectionOK)
		CMD_HANDLER(IDD_CONNECTION_AUDIO, IDOK, OnAudioConnectionOK)

		CMD_HANDLER(ID_GRAPHCTRL, GN_NODE_MOVE, OnGraphMoveSelection)
		CMD_HANDLER(ID_GRAPHCTRL, GN_VIEW_MOVE, OnGraphMoveView)
		CMD_HANDLER(ID_GRAPHCTRL, GN_CONN_AMP_SET, OnGraphSetAmp)
		CMD_HANDLER(ID_GRAPHCTRL, GN_CONN_AMP_TRACK, OnGraphTrackAmp)
		CMD_HANDLER(ID_GRAPHCTRL, GN_SCALE_CHANGE, OnGraphChangeScale)
		CMD_HANDLER(ID_GRAPHCTRL, GN_NODE_MUTE, OnGraphMute)
		CMD_HANDLER(ID_GRAPHCTRL, GN_CONNECT_CONTEXT, OnGraphConnectContext)
		CMD_HANDLER(ID_GRAPHCTRL, GN_NODE_DBLCLICK, OnGraphDblClick)
		CMD_HANDLER(ID_GRAPHCTRL, GN_NODE_SELECT, OnGraphSelectNode)
		CMD_HANDLER(ID_GRAPHCTRL, GN_CONN_CLICK, OnGraphClickConnection)
		CMD_HANDLER(ID_GRAPHCTRL, GN_CONN_DISCONNECT, OnGraphDisconnectConnection)
		CMD_HANDLER(ID_GRAPHCTRL, GN_CLICK, OnGraphClick)

		//
		// Section for aligning machines with keyboard keys
		//
		COMMAND_ID_HANDLER(ID_MACHINE_ALIGN_UP, OnMachineAlignUp)
		COMMAND_ID_HANDLER(ID_MACHINE_ALIGN_DOWN, OnMachineAlignDown)
		COMMAND_ID_HANDLER(ID_MACHINE_ALIGN_LEFT, OnMachineAlignLeft)
		COMMAND_ID_HANDLER(ID_MACHINE_ALIGN_RIGHT, OnMachineAlignRight)

		COMMAND_RANGE_CODE_HANDLER(ID_MACHINECOMMANDS, ID_MACHINECOMMANDS_LAST, 0, OnMachineCommand);
		COMMAND_RANGE_CODE_HANDLER(ID_CREATEMACHINECOMMANDS, ID_CREATEMACHINECOMMANDS_LAST, 0, OnCreateMachineCommand);
		COMMAND_RANGE_CODE_HANDLER(ID_INSERTBEFOREMACHINECOMMANDS, ID_INSERTBEFOREMACHINECOMMANDS_LAST, 0, OnInsertBeforeMachineCommand);
		COMMAND_RANGE_CODE_HANDLER(ID_INSERTAFTERMACHINECOMMANDS, ID_INSERTAFTERMACHINECOMMANDS_LAST, 0, OnInsertAfterMachineCommand);
		COMMAND_RANGE_CODE_HANDLER(ID_REPLACEMACHINECOMMANDS, ID_REPLACEMACHINECOMMANDS_LAST, 0, OnReplaceMachineCommand);
		COMMAND_RANGE_CODE_HANDLER(ID_MACHINE_TIMESOURCE_FIRST, ID_MACHINE_TIMESOURCE_LAST, 0, OnSetTimeSource);
		COMMAND_RANGE_CODE_HANDLER(ID_CONNECT_GRAPH_FIRST, ID_CONNECT_GRAPH_LAST, 0, OnGraphConnect);
		COMMAND_ID_HANDLER(ID_IMPORT_SONG, OnImportSong)
		COMMAND_ID_HANDLER(ID_MACHINE_MUTE, OnMuteMachine)
		COMMAND_ID_HANDLER(ID_MACHINE_SOLO, OnSoloMachine)
		COMMAND_ID_HANDLER(ID_MACHINE_BYPASS, OnBypassMachine)
		COMMAND_ID_HANDLER(ID_UNMUTE_ALL, OnUnmuteAll)
		COMMAND_ID_HANDLER(ID_MIXDOWN_FILE, OnMixdownFile)
		COMMAND_ID_HANDLER(ID_MIXDOWN_WAVETABLE, OnMixdownWavetable)
		COMMAND_ID_HANDLER(ID_MIXDOWN_MULTITRACK, OnMixdownMultitrack)
//		COMMAND_ID_HANDLER(ID_PATTERNVIEW_PATTERNCOLUMN_GOTOPATTERN, OnGotoSequencer)
		CMD_ID_HANDLER(ID_MAKE_TEMPLATE, OnMakeTemplate)
		CMD_ID_HANDLER(ID_MACHINE_CREATEDEFAULTFORMAT, OnCreateDefaultFormat)
		CMD_ID_HANDLER(ID_MACHINE_CREATESIMPLEFORMAT, OnCreateSimpleFormat)
		CMD_ID_HANDLER(ID_CREATE_GROUP_FROM_SELECTION, OnCreateSelectionGroup)
		COMMAND_RANGE_CODE_HANDLER(ID_MOVE_TO_GROUP_FIRST, ID_MOVE_TO_GROUP_LAST, 0, OnMoveToGroup);

		//REFLECT_NOTIFICATIONS_EX()
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	CMachineView(CViewFrame* mainFrm);
	~CMachineView(void);
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) ;
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnBlur(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	//LRESULT OnLButtonDblClick(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
//	LRESULT OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
//	LRESULT OnMButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetEditFlags(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPasteNoData(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopyAudioConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPasteAudioConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDeleteAndRestoreConnections(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClearSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAudioConnectionProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMidiConnectionProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEventConnectionProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNoteConnectionProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAudioConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMidiConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEventConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNoteConnectionParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineParameters(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEventConnectionBindings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDisconnectMachinesAudio(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDisconnectMachinesEvent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDisconnectMachinesMidi(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDisconnectMachinesNote(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineHelp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMinimizeMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUnMinimizeMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	// Moving machines with keyboard keys
	LRESULT OnMachineMoveUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnMachineMoveUpByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveDownByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveLeftByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveRightByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);	

	LRESULT OnMachineMoveDownLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveDownRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveUpLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveUpRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnMachineMoveDownLeftByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveDownRightByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveUpLeftByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineMoveUpRightByStep(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnMachineAlignUp(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineAlignDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineAlignLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineAlignRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	LRESULT OnMachineHideIncomingConnections(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);	

	LRESULT OnMuteMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSoloMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBypassMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnCreateMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnInsertBeforeMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnInsertAfterMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnReplaceMachineCommand(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnSetTimeSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnUnmuteAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMixdownFile(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMixdownWavetable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMixdownMultitrack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnImportSong(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnToggleConnectionText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnMoveToGroup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

	void OnCreateGroup();
	void OnCreateDefaultFormat();
	void OnCreateSimpleFormat();
	void OnCreateSelectionGroup();
	void OnMakeTemplate();
	void OnEventConnectionDialog();
	void OnAudioConnectionDialog();
	void OnAudioConnectionOK();
	void OnEventConnectionOK();
	void OnGraphMoveSelection();
	void OnGraphMoveView();
	void OnGraphSetAmp();
	void OnGraphTrackAmp();
	void OnGraphChangeScale();
	void OnGraphMute();
	void OnGraphDblClick();
	void OnGraphSelectNode();
	void OnGraphConnectContext();
	void OnGraphClickConnection();
	void OnGraphDisconnectConnection();
	void OnGraphClick();
	LRESULT OnGraphConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);

	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	virtual void UpdateTimer(int count);

	bool OnDragOver(const POINTL& pt, FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect);
	void OnDragLeave();
	bool OnDropMachine(std::string machineName, std::string instrumentName, int x, int y);
	void InvalidateStatus();
	void UpdateTheme();

	// selection API:
	void SelectMachine(zzub_plugin_t* machine);
	void UnselectMachine(zzub_plugin_t* machine);
	bool IsSelectedMachine(zzub_plugin_t* machine);
	int GetSelectedMachines();
	zzub_plugin_t* GetSelectedMachine(int index);
	zzub_connection_t* GetEdgeConnection(int to_node_id, int from_node_id, edgetype etype, zzub_connection_type ctype);
	zzub_connection_t* GetSelectedEdgeConnection(edgetype etype, zzub_connection_type ctype);

	void DeleteSelectedMachines();
	void DeleteSelectedMachinesAndRestoreConnections();

	int GetEditFlags();

	void CreateMachine(int index, int type);
	//bool ConnectMachinesAudioCompatible(zzub_plugin_t* from_plugin, zzub_plugin_t* to_plugin, zzub_connection_t* old_out, zzub_connection_t* old_in, int vol, int pan);

	void SetMidiAliases();
	void ShowAudioConnectionDialog(zzub_connection_t* audioconn);

protected:
	void ShowPluginContext(POINT pt, zzub_plugin_t* mac);
	void ShowConnectionContext(POINT pt, CGraphNodePair const& conn);
	void ShowBackgroundContext(POINT pt);
	void ShowGroupContext(POINT pt, zzub_plugin_group_t* group);
	void ShowKeyboardContext();
	void ShowNodeSelectionContext(POINT pt);
	void ShowMultiPluginContext(POINT pt);
	void ShowMultiGroupContext(POINT pt);
	void ShowMultiMixedContext(POINT pt);
	void InvalidateMachine(zzub_plugin_t* machine);

	//int GetVolumeSliderVolume(const POINT& pt);
	//void ResetVolumeSlider();

	void MoveSelectedMachines(int x, int y);
	float GetNearestMachineLocation(float pos, int direction);
	inline float RoundToPixel(float x);
	void PasteFromClipboard(int flags, float x, float y);
	void BindGraphControl();
	std::string DescribeConnection(zzub_connection_t* conn);
	int SaveSelection(std::string fileName, const std::vector<zzub_plugin_t*>& selectedplugins);
	void UpdateConnectionToolTip();
	void BindSong();
	void CalcPixelSize();

	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	virtual void GetHelpText(char* text, int* len);
	virtual bool DoesKeyjazz();

	bool AddPatternTrack(zzub_plugin_t* plugin);
	void ExtendPatternFormat(zzub_pattern_format_t* format, zzub_plugin_t* plugin, bool simple);
	zzub_pattern_format_t* CreateDefaultFormat(zzub_plugin_t* plugin, bool simple);
	zzub_plugin_t* CreateMachine(std::string const& uri, std::string const& instrumentName, float x, float y);
	bool ConnectMachinesAudio(zzub_plugin_t* output, zzub_plugin_t* input, int amp=0x4000, int first_input = -1, int input_count = -1, int first_output = -1, int output_count = -1);
	bool ConnectMachinesEvent(zzub_plugin_t* output, zzub_plugin_t* input, int source_param_index, int target_group_index, int target_track_index, int target_param_index);
	bool ConnectMachinesNote(zzub_plugin_t* output, zzub_plugin_t* input);
	bool ConnectMachinesMidi(zzub_plugin_t* output, zzub_plugin_t* input, std::string const& midiOutDevice);
	bool DisconnectMachines(zzub_plugin_t* output, zzub_plugin_t* input, zzub_connection_type type);
	bool DisconnectEdge(int to_plugin_id, int from_plugin_id, edgetype etype, zzub_connection_type ctype);

	void GetExpandedSelection(std::vector<zzub_plugin_t*>* result);
	void GetExpandedGroup(zzub_plugin_group_t* plugingroup, std::vector<zzub_plugin_t*>* result);
	void MoveSelectionToGroup(zzub_plugin_group_t* plugingroup);

};
