#pragma once

#include <stack>
//#include "FileBrowserList.h"
#include "WaveTableList.h"
#include "WtlExt.h"
#include "EnvelopeCtrl.h"
#include "WaveLevelList.h"
#include "WaveEditorCtrl.h"
#include "ToolbarWindow.h"
#include "SplitterWindowKey.h"
#include "WTLTabViewCtrl.h"

#include "../buzecommon/resource.h" // pull in resource ids for the reusable ParametersControl
#include "ParametersControl.h"
#include "WaveTableTabs.h"

static const int WM_GET_EDITFLAGS = WM_USER+9;
static const int WM_GET_STATUS_TEXT = WM_USER+14; // sent by mainframe to client views = client views must reserve message!

class CWaveTableView;

class CWavetableViewInfo : public CViewInfoImpl {
public:
	CWavetableViewInfo(CViewFrame* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};







// Apparently, having a CTabCtrl inside a splitter causes drawing problems with 
// XP themes where the tab header background isnt cleared properly.
// We create a CBgTabViewVtrl class that derives from our previous tab control 
// and implement WM_ERASEBKGND to get around the problem:

class CBgTabViewCtrl : public CWTLTabViewCtrlT< CBgTabViewCtrl> {
public:
	BEGIN_MSG_MAP(CBgTabViewCtrl)
		MESSAGE_HANDLER(WM_KEYDOWN, OnForwardMessage)
		MESSAGE_HANDLER(WM_KEYUP, OnForwardMessage)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground);
		CHAIN_MSG_MAP(CWTLTabViewCtrlT< CBgTabViewCtrl>)
	END_MSG_MAP()

	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return GetParent().SendMessage(uMsg, wParam, lParam);
	}

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return 1;
	}
};

const int WM_POSTCREATE = WM_USER + 1;

class CWaveInfo {
public:
	int level;
	int slice;
	int sel_from;
	int sel_length;
	int display_from;
	int display_length;
	int length;
};

class CWaveTableView 
	: public CViewImpl
	, public CToolbarWindow<CWaveTableView> 
	, public CMessageFilter
	, public CIdleHandler
	, public CWaveEditorWaveProvider
{
public:
	CWaveTableList waveList;
	CWaveLevelList waveLevelList;
	CWaveEditorCtrl waveEditorCtrl;
		
	HWND hWndToolBar;	// the button toolbar
	HWND hWndEditToolBar;	// the edit button toolbar

	CSplitterWindowKey mainSplitter;
	CHorSplitterWindowKey waveLevelSplitter, waveTabSplitter;

	CBgTabViewCtrl waveTabs;
	CWaveTabEdit editTab;
	CWaveTabEnvelope envelopeTab;
	CWaveTabSlice sliceTab;
	CWaveTabEffects effectsTab;

	CStatusBarCtrl statusBar;

	bool dirtyWavelevels;
	bool dirtyWavelist;
	bool dirtyStatus;
	bool dirtyWaveEditorReset;
	bool dirtyWaveEditor;
	bool dirtyEditTab;
	bool dirtyEnvelope;
	bool dirtySlices;

	zzub_wave_t* currentWave;
	int currentWaveIndex;
	zzub_wavelevel_t* currentWavelevel;
	std::vector<CWaveInfo> waveInfos;

	DECLARE_WND_CLASS("WaveTableView")

	BEGIN_MSG_MAP(CWaveTableView)
		CHAIN_MSG_MAP(CToolbarWindow<CWaveTableView>)

		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnBlur)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_POSTCREATE, OnPostCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_GET_EDITFLAGS, OnGetEditFlags)
		MESSAGE_HANDLER_EX(WM_GET_STATUS_TEXT, OnGetStatusText)

		NOTIFY_HANDLER(IDC_WAVELIST, LVN_ITEMCHANGED, OnWaveListSelChanged)
		NOTIFY_HANDLER(IDC_WAVELIST, NM_DBLCLK, OnWaveListDoubleClicked)

		NOTIFY_HANDLER(IDC_WAVELEVELLIST, LVN_ITEMCHANGED, OnWaveLevelListSelChanged)
		
		COMMAND_HANDLER(IDC_LOADWAVEBUTTON, BN_CLICKED, OnLoadWave)
		COMMAND_HANDLER(IDC_SAVEWAVEBUTTON, BN_CLICKED, OnSaveWave)
		COMMAND_HANDLER(IDC_CLEARWAVEBUTTON, BN_CLICKED, OnClearWave)

		// forwarded from mainfrm
		COMMAND_ID_HANDLER(ID_HELP, OnHelp)
		COMMAND_ID_HANDLER(ID_EDIT_CUT, OnCut)
		COMMAND_ID_HANDLER(ID_EDIT_COPY, OnCopy)
		COMMAND_ID_HANDLER(ID_EDIT_PASTE, OnPaste)
		COMMAND_ID_HANDLER(ID_EDIT_DELETE, OnDelete)
		COMMAND_ID_HANDLER(ID_EDIT_SELECTALL, OnSelectAll)
		COMMAND_ID_HANDLER(ID_EDIT_CLEARSELECTION, OnClearSelection)
		COMMAND_ID_HANDLER(ID_VIEW_PROPERTIES, OnViewProperties)
		COMMAND_ID_HANDLER(ID_WAVELEVEL_PROPERTIES, OnViewLevelProperties)
		COMMAND_ID_HANDLER(ID_WAVELEVEL_ADD, OnWaveLevelAdd)
		COMMAND_ID_HANDLER(ID_WAVELEVEL_DELETE, OnWaveLevelDelete)
		COMMAND_ID_HANDLER(ID_WAVE_XEDIT, OnWaveXEdit)
		COMMAND_ID_HANDLER(ID_WAVE_IEDIT, OnWaveIEdit)
		COMMAND_ID_HANDLER(ID_WAVE_TRIM, OnTrim) // test 29
		COMMAND_RANGE_CODE_HANDLER(ID_WAVE_SCRIPTCOMMANDS, ID_WAVE_SCRIPTCOMMANDS_LAST, 0, OnWaveScript)

		COMMAND_ID_HANDLER(ID_WAVE_ZOOM_SAMPLES, OnZoomSamples)
		COMMAND_ID_HANDLER(ID_WAVE_ZOOM_SECONDS, OnZoomSeconds)
		COMMAND_ID_HANDLER(ID_WAVE_ZOOM_TICKS, OnZoomTicks)
		COMMAND_ID_HANDLER(ID_WAVE_ZOOM_WORD, OnZoomWord) 
		COMMAND_ID_HANDLER(ID_WAVE_ZOOM_IN, OnZoomIn)
		COMMAND_ID_HANDLER(ID_WAVE_ZOOM_OUT, OnZoomOut)
		COMMAND_ID_HANDLER(ID_WAVE_ZOOM_SELECTION, OnZoomSelection)
		COMMAND_ID_HANDLER(ID_WAVE_ZOOM_ALL, OnZoomAll)
		COMMAND_ID_HANDLER(ID_WAVE_CLEARWAVE, OnClearWave)

		COMMAND_HANDLER(IDC_WAVEEDITOR, WEN_SELECTIONCHANGED, OnWaveEditorSelectionChanged)
		COMMAND_HANDLER(IDC_WAVEEDITOR, WEN_ZOOMCHANGED, OnWaveEditorZoomChanged)
		COMMAND_HANDLER(IDC_WAVEEDITOR, WEN_LOOPCHANGED, OnWaveEditorLoopChanged)
		COMMAND_HANDLER(IDC_WAVEEDITOR, WEN_MOUSEMOVE, OnWaveEditorMouseMove)

		// From CWTLTabViewCtrl: 
		// Parent windows must have REFLECT_NOTIFICATIONS() in the message map
		// to pass along the TCN_SELCHANGE message to 
		REFLECT_NOTIFICATIONS_ID_FILTERED(IDC_WAVETABS)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	CWaveTableView(CViewFrame* mainFrm);
	~CWaveTableView(void);
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPostCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnBlur(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnGetEditFlags(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetStatusText(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLoadWave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSaveWave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClearWave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClearSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTrim(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);	//29
	LRESULT OnWaveScript( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnZoomSamples(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnZoomSeconds(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnZoomTicks(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnZoomWord(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/); 
	LRESULT OnZoomIn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnZoomOut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnZoomSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnZoomAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWaveEditorLoopChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWaveEditorSelectionChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWaveEditorZoomChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWaveEditorMouseMove(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWaveListDoubleClicked(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnWaveListSelChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnWaveLevelListSelChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnTcnSelchangeWavetab(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnViewProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewLevelProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWaveXEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWaveIEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWaveLevelAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnWaveLevelDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual void GetSamplesDigest(int channel, int start, int end, std::vector<float>& mindigest, std::vector<float>& maxdigest, std::vector<float>& ampdigest, int digestsize);

	void ExecuteWaveScript( std::string script, std::string tempFile );
	void SetEnvelope(int numPoints, int sustainIndex, long* points);
	void SetLoopingPoints(bool enabled, int beginSample, int endSample);
	void BindWaveInfos();
	void BindWaveLevels();
	void BindStatus();	
	void BindWaveList();
	void BindWaveListWave(zzub_wave_t* wave);
	void BindCurrentWave();
	void UpdateZoomLevels(zzub_wavelevel_t* wavelevel);
	void UpdateToolbar();
	void UpdateWaveEditor(bool reset = true);
	int GetEditFlags();
	bool CopySelection();
	void DeleteSelection();

	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	virtual void GetHelpText(char* text, int* len);
	CWaveInfo* GetCurrentWaveInfo();
};
