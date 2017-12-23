#pragma once

#include "PresetManager.h"
#include "ToolbarWindow.h"
#include "ToolbarBands.h"
#include "SelectMidiController.h"
#include "../buzecommon/resource.h" // pull in resource ids for the reusable ParametersControl
#include "ParametersControl.h"

static const int WM_SELECTMIDICONTROLLER = WM_USER+1;
static const int WM_TOGGLEPRESETMODE = WM_USER+2;

class CParameterViewInfo : public CViewInfoImpl {
public:
	CParameterViewInfo(CViewFrame* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);

	void ShowParameterView(buze_event_data* ev);
};

class CMachineView;
class CViewFrame;
class CMachineParameterView;

class CMachineEmbeddedView
	: public CWindowImpl<CMachineEmbeddedView> 
{
public:
	zzub_plugin_t* machine;

	DECLARE_WND_CLASS("CMachineEmbeddedView")

	BEGIN_MSG_MAP(CMachineEmbeddedView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

	CMachineEmbeddedView();
	~CMachineEmbeddedView(void);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void SetMachine(zzub_plugin_t*);
	void Resize(int x, int y, int* width, int* height);
};

/*enum MachineParameterViewMode {
	parametermode_default,
	parametermode_internal_only,
	parametermode_custom_only,
	parametermode_both
};*/

// the machineparameter view receives a pointer to CMachineParameterViewArgs in the CREATESTRUCT
class CMachineParameterViewArgs {
public:
	MachineParameterViewMode mode;
	zzub_plugin_t* plugin;
	zzub_pattern_format_t* format;
	POINT position;
};

class CMachineParameterView 
	: public CViewImpl
	, public CToolbarWindow<CMachineParameterView>
	, public CMessageFilter
{
public:
	PresetInfo defaultPreset;
	zzub_plugin_t* machine;
	PresetManager presets;
	CComboListBand presetDropDown;
	CMachineEmbeddedView embeddedView;
	CMachineParameterScrollView sliderView;
	HWND hWndButtonToolBar;
	MachineParameterViewMode mode;
	
	CSelectMidiController selectMidiController;

	DECLARE_WND_CLASS_EX("CMachineParameterView", CS_HREDRAW|CS_VREDRAW|CS_PARENTDC, NULL)

	BEGIN_MSG_MAP(CMachineParameterView)
		CHAIN_MSG_MAP(CToolbarWindow<CMachineParameterView>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnBlur)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_SELECTMIDICONTROLLER, OnSelectMidiController)
		MESSAGE_HANDLER(WM_TOGGLEPRESETMODE, OnTogglePresetMode)

		COMMAND_ID_HANDLER(ID_HELP, OnHelp)
		COMMAND_ID_HANDLER(ID_PRESET_EDIT, OnPresetEdit)
		COMMAND_ID_HANDLER(ID_PRESET_COPY, OnPresetCopy)
		COMMAND_ID_HANDLER(ID_PRESET_RANDOMIZE, OnPresetRandomize)
		COMMAND_ID_HANDLER(ID_PRESET_HUMANIZE, OnPresetHumanize)
		COMMAND_ID_HANDLER(ID_PRESET_MODE, OnPresetMode)
		COMMAND_ID_HANDLER(ID_PRESET_MACHINEHELP, OnMachineHelp)

		COMMAND_ID_HANDLER(ID_PRESET_COPY_ALL, OnPresetCopyAll)
		COMMAND_ID_HANDLER(ID_PRESET_COPY_GLOBAL, OnPresetCopyGlobal)
		COMMAND_ID_HANDLER(ID_PRESET_COPY_TRACK, OnPresetCopyTrack)

		COMMAND_HANDLER(IDC_PRESETDROPDOWN, LBN_SELCHANGE, OnSelChange)
		COMMAND_HANDLER(IDC_PRESETDROPDOWN, CBN_CLOSEUP, OnRestoreFocus)  
		
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_NEXTSLIDER, OnNextSlider)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_PREVIOUSSLIDER, OnPrevSlider)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_PAGEUPSLIDER, OnPageUpSlider)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_PAGEDOWNSLIDER, OnPageDownSlider)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_MOVERIGHT, OnMoveRight)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_MOVESOMERIGHT, OnMoveSomeRight)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_MOVEPAGERIGHT, OnMovePageRight)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_MOVELEFT, OnMoveLeft)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_MOVESOMELEFT, OnMoveSomeLeft)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_MOVEPAGELEFT, OnMovePageLeft)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_ENTERVALUE, OnEnterValue)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_PRESETS, OnShowPresetsToolbar)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_NEXTPRESET, OnNextPreset)
		COMMAND_ID_HANDLER(ID_PARAMETERVIEW_PREVIOUSPRESET, OnPreviousPreset)
		
		COMMAND_ID_HANDLER(ID_PARAMETER_BIND, OnBindMidi)
		COMMAND_ID_HANDLER(ID_PARAMETER_UNBIND, OnUnbindMidi)
		COMMAND_ID_HANDLER(ID_PARAMETER_HIDE, OnHideSlider)
		COMMAND_RANGE_HANDLER(ID_PARAMETER_UNHIDE_FIRST, ID_PARAMETER_UNHIDE_LAST, OnUnhideSlider)

		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	CMachineParameterView(CViewFrame* mainFrm);
	~CMachineParameterView(void);
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnBlur(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSelectMidiController(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTogglePresetMode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	LRESULT OnHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPresetCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPresetCopyAll(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPresetCopyGlobal(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPresetCopyTrack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPresetRandomize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPresetHumanize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPresetMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPresetEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMachineHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNextSlider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPrevSlider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPageUpSlider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPageDownSlider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNextPreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPreviousPreset(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMoveRight(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMoveSomeRight(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMovePageRight(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMoveLeft(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMoveSomeLeft(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnMovePageLeft(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEnterValue(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBindMidi(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUnbindMidi(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnHideSlider(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnUnhideSlider(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowPresetsToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRestoreFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);

	void SetMachine(zzub_plugin_t* machine);
	zzub_plugin_t* GetMachine();

	void GetClientSize(RECT* rc);

	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	virtual void GetHelpText(char* text, int* len);
	virtual bool DoesKeyjazz();
protected:
	void InitPresets();
	void BindPresets();
	void SetPreset(int sel);
	void BindParameters();	// invoke when ext.hidden or connections changes
};
