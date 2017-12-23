#pragma once

#include "ToolbarWindow.h"
#include "ToolbarBands.h"

class CMixerViewInfo : public CViewInfoImpl {
public:
	int show_eventcode;

	std::string plugin_filter;
	std::string parameter_filter;

	CMixerViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	void OnLoadState(zzub_archive_t*);
	void OnSaveState(zzub_archive_t*);
};

class CMixerKnob {
public:
	RECT rcKnob;
	zzub_plugin_t* plugin;
	int group, column;
};

class CMixerTrack {
public:
	std::string name;
	zzub_plugin_t* eqinplugin;
	zzub_plugin_t* eqoutplugin;
	zzub_plugin_t* outplugin;
	zzub_connection_t* outconnection;
	zzub_plugin_t* connplugin;
	std::vector<zzub_plugin_t*> inplugins;
	std::vector<zzub_plugin_t*> inconnplugins;

	float lastpeak[2];
	RECT rcTrack;
	RECT rcHeader;
	RECT rcFooter;
	RECT rcInputs[100];
	RECT rcMute;
	RECT rcSolo;
	CMixerKnob knobs[100];
	int knobcount;
	RECT rcAmps[2];
};

class CMixerView 
	: public CToolbarWindow<CMixerView>
	, public CViewImpl
{
public:

	enum dragtype {
		dragtype_move_none,
		dragtype_move_input,
		dragtype_move_eq,
		dragtype_move_amp
	};

	CEditBand pluginFilter;
	CEditBand parameterFilter;
	zzub_plugin_t* outplugin;
	CMixerViewInfo* info;
	std::vector<CMixerTrack*> tracks;
	int selectedtrack;
	dragtype dragmode;
	int dragindex;
	bool dirtystate;
	bool linked;

	DECLARE_WND_CLASS("MixerView")

	BEGIN_MSG_MAP(CMixerView)
		CHAIN_MSG_MAP(CToolbarWindow<CMixerView>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnBlur)
		COMMAND_RANGE_HANDLER(ID_SELECT_INPUT_FIRST, ID_SELECT_INPUT_LAST, OnSelectInput)
		COMMAND_RANGE_HANDLER(ID_SELECT_OUTPUT_FIRST, ID_SELECT_OUTPUT_LAST, OnSelectOutput)
		COMMAND_ID_HANDLER(ID_SHOW_EQ_PARAMETERS, OnEqParameters)
		COMMAND_ID_HANDLER(ID_SHOW_CONNECTION_PARAMETERS, OnConnectionParameters)
		COMMAND_ID_HANDLER(ID_LINKED_AMP, OnLinkedAmp)
		COMMAND_HANDLER(IDC_PLUGINFILTER, EN_CHANGE, OnPluginFilterChange)
		COMMAND_HANDLER(IDC_PARAMETERFILTER, EN_CHANGE, OnParameterFilterChange)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	CMixerView(buze_main_frame_t* m, CMixerViewInfo* i);
	~CMixerView();
	virtual void OnFinalMessage(HWND /*hWnd*/);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBlur(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSelectInput(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelectOutput(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEqParameters(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnConnectionParameters(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnLinkedAmp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPluginFilterChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnParameterFilterChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	void UpdateTimer(int count);
	virtual HWND GetHwnd();
	int GetEditFlags();
	bool DoesKeyjazz();

	void DrawTrackSlider(CDC& dc, const std::string& label, RECT& rcSlider, float value, bool showValue);
	void DrawAmpSlider(CDC& dc, RECT& rcAmp, float peak, float amp);
	void RestoreState();
	void ResetState();
	void ClearTracks();
	void InsertPluginTrack(zzub_plugin_t* plugin);
	void InsertPluginGroupTrack(zzub_plugin_group_t* plugingroup);
	void InsertPluginKnobs(CMixerTrack* track, zzub_plugin_t* eqplugin, std::vector<std::string>& paramFilters);
	void InitTrack(CMixerTrack* track);
	void UpdateLayout();
	bool IsCompatiblePlugin(zzub_plugin_t* plugin);
	bool IsTrackPlugin(zzub_plugin_t* plugin);
	void GetPluginFilter(std::vector<std::string>& result);
	void GetParameterFilter(std::vector<std::string>& result);
	bool GetAmpParameterRect(zzub_plugin_t* plugin, int group, int column, RECT* result);
	bool GetEqParameterRect(zzub_plugin_t* plugin, int group, int column, RECT* result);
	bool GetInputParameterRect(zzub_plugin_t* plugin, int group, int column, RECT* result);
};
