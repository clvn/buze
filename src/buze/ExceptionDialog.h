#pragma once

#include "resource.h"

class CExceptionDlg : public CDialogImpl<CExceptionDlg>
{
public:
	zzub_player_t* player;
	struct _EXCEPTION_POINTERS *pExceptionInfo;

	enum { IDD = IDD_ERRORDIALOG };

	BEGIN_MSG_MAP(CExceptionDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_EXCEPTIONDIALOG_SAVESONGBUTTON, OnSaveSong)
		COMMAND_ID_HANDLER(IDC_EXCEPTIONDIALOG_SAVEDEBUGBUTTON, OnSaveDump)
	END_MSG_MAP()

	CExceptionDlg(zzub_player_t* _player, struct _EXCEPTION_POINTERS *pExceptionInfo);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSaveSong(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSaveDump(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
