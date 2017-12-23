// aboutdlg.h : interface of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FileVersionInfo.h"
#include "utils.h"
#include <zzub\signature.h>

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		CEdit aboutText;
		aboutText.Attach(GetDlgItem(IDC_ABOUTTEXT));
		aboutText.SetWindowText(PeekString(GetModuleHandle(0), IDC_ABOUTTEXT));

		CFileVersionInfo version;
		version.Open();
		CStatic versionText;
		versionText.Attach(GetDlgItem(IDC_ABOUTVERSION));
		std::stringstream strm;
		strm << version.GetProductName() << " " << version.GetProductVersion() << "\n"
			<< ZZUB_SIGNATURE << "\n"
			<< version.GetLegalCopyright();
		versionText.SetWindowText(strm.str().c_str());

		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		EndDialog(wID);
		return 0;
	}
};
