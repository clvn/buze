#pragma once

#include "zzub/zzub.h"

class PresetManager;
struct PresetInfo;

class CPresetDialog  : public CDialogImpl<CPresetDialog>
{
public:
	enum { IDD = IDD_PRESETS };
	CListBox list;
	CButton okButton;
	CButton deleteButton;
	CListBox presetList;
	CEdit commentBox;
	CEdit nameBox;

	PresetManager* presets;
	zzub_player_t* player;
	zzub_plugin_t* plugin;

	BEGIN_MSG_MAP(CPresetDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDOK, OnBnClickedOk)
		COMMAND_HANDLER(IDC_PRESET_LIST, LBN_SELCHANGE, OnPresetChange)
		COMMAND_HANDLER(IDC_PRESET_DELETE_BUTTON, BN_CLICKED, OnPresetDelete)
	END_MSG_MAP()

	CPresetDialog(PresetManager* prs, zzub_player_t* _player, zzub_plugin_t* plug);
	~CPresetDialog(void) { }

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPresetDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void updatePresetInfo(PresetInfo* pi);
};
