#pragma once

class CWaveTabEdit : public CWindowImpl<CWaveTabEdit> {
	CWaveTableView* view;
public:
	CWaveTabEdit(CWaveTableView* view);

	CStatic volumeSliderLabel;
	CTrackBarCtrl volumeSlider;

	DECLARE_WND_CLASS("WaveTabEdit")

	BEGIN_MSG_MAP(CWaveTabEdit)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void SetSliderValue(float f); // 0..2
	void UpdateFromWavetable();
};

class CWaveTabEnvelope : public CWindowImpl<CWaveTabEnvelope> {
	CWaveTableView* view;
public:
	CWaveTabEnvelope(CWaveTableView* view);

	CComboBox waveMachineDropDown;
	CComboBox envelopeDropDown;
	CButton disabledButton;

	CStatic envelopeBackground;
	CEnvelopeCtrl envelopeCtrl;
	bool autoCreateEnvelopes;

	DECLARE_WND_CLASS("WaveTabEnvelope")

	BEGIN_MSG_MAP(CWaveTabEnvelope)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ENVELOPECHANGED, OnEnvelopeChanged)

		COMMAND_HANDLER(IDC_DISABLEDBUTTON, BN_CLICKED, OnToggleDisabled)
		COMMAND_HANDLER(IDC_ENVELOPEDROPDOWN, CBN_SELCHANGE, OnSelChangeEnvelope)
		COMMAND_HANDLER(IDC_WAVEMACHINEDROPDOWN, CBN_SELCHANGE, OnSelChangeWaveMachine)
	END_MSG_MAP()

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEnvelopeChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnToggleDisabled(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelChangeWaveMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSelChangeEnvelope(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	int GetSelectedEnvelope();
	void UpdateMachines();
	void UpdateEnvelope();
	void UpdateMachineEnvelopes();

};

class CListViewKeyCtrl : public CWindowImpl<CListViewKeyCtrl, CListViewCtrl> {
public:
	DECLARE_WND_SUPERCLASS("ListViewKeyCtrl", CListViewCtrl::GetWndClassName())

	BEGIN_MSG_MAP(CListViewKeyCtrl)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
	END_MSG_MAP()

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		switch (wParam) {
			case VK_UP:
			case VK_DOWN:
			case VK_HOME:
			case VK_END:
			case VK_NEXT:
			case VK_PRIOR:
				if (!DefWindowProc()) return 0;
				break;
		}
		return GetParent().SendMessage(WM_KEYDOWN, wParam, lParam);
	}

	LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return GetParent().SendMessage(WM_KEYUP, wParam, lParam);
	}

	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return 1;
	}
};

class CWaveTabSlice : public CWindowImpl<CWaveTabSlice> {
	CWaveTableView* view;
	zzub_wavelevel_t* wavelevel;
	bool noselect;
public:
	CWaveTabSlice(CWaveTableView* view);

	CStatic sliceSliderLabel;
	CListViewKeyCtrl sliceList;

	DECLARE_WND_CLASS("WaveTabSlice")

	BEGIN_MSG_MAP(CWaveTabSlice)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_KEYDOWN, OnForwardMessage)
		MESSAGE_HANDLER(WM_KEYUP, OnForwardMessage)

		NOTIFY_HANDLER(IDC_WAVETABLE_SLICELIST, LVN_ITEMCHANGED, OnSliceChange)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSliceChange(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);

	LRESULT OnForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		return GetParent().SendMessage(uMsg, wParam, lParam);
	}

	void SetWaveLevel(zzub_wavelevel_t* _wavelevel);
};

class CWaveTabEffects : public CWindowImpl<CWaveTabEffects> {
	zzub_player_t* player;
	CWaveTableView* view;
public:
	CWaveTabEffects(CWaveTableView* _view, zzub_player_t* player);

	zzub_plugin_t* effectplugin;

	CMachineParameterScrollView effectParameters;
	CListViewCtrl effectsList;
	CButton applyButton;
	
	DECLARE_WND_CLASS("WaveTableEffects")

	BEGIN_MSG_MAP(CWaveTabEffects)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground);
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		COMMAND_HANDLER(IDC_WAVETABLE_EFFECTBUTTON, BN_CLICKED, OnApplyEffect)
		NOTIFY_HANDLER(IDC_WAVETABLE_EFFECTSLIST, LVN_ITEMCHANGED, OnChangeEffect)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChangeEffect(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnApplyEffect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);

	void BindEffectsList();
	void BindParameters();
	void CreateOfflinePlugin();
	void DestroyOfflinePlugin();

};
