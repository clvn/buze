#include "stdafx.h"
#include <zzub/zzub.h>
#include "dbghelp.h"
#include "resource.h"
#include "ExceptionDialog.h"
// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

bool GetDlgItemChecked(HWND hWnd, WORD wID) {
	HWND hWndItem = GetDlgItem(hWnd, wID);
	if (!hWndItem) return false;
	return SendMessage(hWndItem, BM_GETCHECK, 0, 0)?true:false;
}

void SetDlgItemChecked(HWND hWnd, WORD wID, bool state) {
	HWND hWndItem = GetDlgItem(hWnd, wID);
	if (!hWndItem) return ;
	SendMessage(hWndItem, BM_SETCHECK, state?TRUE:FALSE, 0);
}

CExceptionDlg::CExceptionDlg(zzub_player_t* player, struct _EXCEPTION_POINTERS *pExceptionInfo) {
	this->player = player;
	this->pExceptionInfo = pExceptionInfo;
}

LRESULT CExceptionDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	SetDlgItemChecked(m_hWnd, IDC_EXCEPTIONDIALOG_CLOSERADIO, true);
	return TRUE;
}

LRESULT CExceptionDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	UINT exitradio = GetDlgItemChecked(m_hWnd, IDC_EXCEPTIONDIALOG_EXITRADIO);
	UINT closeradio = GetDlgItemChecked(m_hWnd, IDC_EXCEPTIONDIALOG_CLOSERADIO);
	UINT ignoreradio = GetDlgItemChecked(m_hWnd, IDC_EXCEPTIONDIALOG_IGNORERADIO);
	if (exitradio)
		EndDialog(0); else
	if (closeradio)
		EndDialog(1); else
		EndDialog(2);
	return 0;
}
LRESULT CExceptionDlg::OnSaveSong(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	char* szResult = 0;
	char szDumpPath[_MAX_PATH];
	char szScratch [_MAX_PATH];

	// work out a good place for the dump file
	if (!GetTempPath( _MAX_PATH, szDumpPath ))
		_tcscpy( szDumpPath, "c:\\temp\\" );

	_tcscat( szDumpPath, "crash.bmx" );
	/*zzub::file_outstream outf;
	if (outf.create(szDumpPath)) {
		zzub::BuzzWriter writer(&outf);
		std::vector<zzub::plugin_descriptor> plugins;
		if (writer.writePlayer(player, plugins, true)) {
			sprintf(szScratch, "Successfully saved current song to %s.", szDumpPath);
		} else {
			sprintf(szScratch, "Error while saving song to %s.", szDumpPath);
		}
		outf.close();
		szResult = szScratch;
	} else {
		szResult = "Could not create temp file.";
	}
*/
	if (szResult)
		MessageBox(szResult, programName);

	return 0;
}

LRESULT CExceptionDlg::OnSaveDump(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	HWND hParent = NULL;						// find a better value for your app

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	char szDbgHelpPath[_MAX_PATH];

	if (GetModuleFileName( NULL, szDbgHelpPath, _MAX_PATH ))
	{
		char *pSlash = _tcsrchr( szDbgHelpPath, '\\' );
		if (pSlash)
		{
			_tcscpy( pSlash+1, "DBGHELP.DLL" );
			hDll = ::LoadLibrary( szDbgHelpPath );
		}
	}

	if (hDll==NULL)
	{
		// load any version we can
		hDll = ::LoadLibrary( "DBGHELP.DLL" );
	}

	LPCTSTR szResult = NULL;

	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{
			char szDumpPath[_MAX_PATH];
			char szScratch [_MAX_PATH];

			// work out a good place for the dump file
			if (!GetTempPath( _MAX_PATH, szDumpPath ))
				_tcscpy( szDumpPath, "c:\\temp\\" );

			_tcscat( szDumpPath, "buze.dmp" );

			// create the file
			HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
										FILE_ATTRIBUTE_NORMAL, NULL );

			if (hFile!=INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pExceptionInfo;
				ExInfo.ClientPointers = NULL;

				// write the dump
				BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
				if (bOK)
				{
					sprintf( szScratch, "Saved dump file to '%s'", szDumpPath );
					szResult = szScratch;
					//retval = EXCEPTION_EXECUTE_HANDLER;
				}
				else
				{
					sprintf( szScratch, "Failed to save dump file to '%s' (error %d)", szDumpPath, GetLastError() );
					szResult = szScratch;
				}
				::CloseHandle(hFile);
			}
			else
			{
				sprintf( szScratch, "Failed to create dump file '%s' (error %d)", szDumpPath, GetLastError() );
				szResult = szScratch;
			}
		}
		else
		{
			szResult = "DBGHELP.DLL too old";
		}
	}
	else
	{
		szResult = "DBGHELP.DLL not found";
	}

	if (szResult)
		::MessageBox( NULL, szResult, programName, MB_OK );

	return 0;
}
