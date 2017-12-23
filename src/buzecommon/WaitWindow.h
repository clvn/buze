#pragma once

#include "resource.h"

// CWaitWindow dialog

const int MYWM_SHOWWINDOW = WM_USER + 1;
const int MYWM_SETTEXT = WM_USER + 2;
const int MYWM_HIDEWINDOW = WM_USER + 3;

class CWaitWindow : public CDialogImpl<CWaitWindow> 
{

public:
	CWaitWindow();   // standard constructor
	virtual ~CWaitWindow();

// Dialog Data
	enum { IDD = IDD_WAITDIALOG };

	DWORD dwMainThreadID;
	HANDLE hWaitEvent;
	POINT ptMoveFrom;
protected:

	BEGIN_MSG_MAP(CWaitWindow)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
		MESSAGE_HANDLER(MYWM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(MYWM_SETTEXT, OnSetText)
		MESSAGE_HANDLER(MYWM_HIDEWINDOW, OnHideWindow)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnShowWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetText(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHideWindow(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

public:
	bool Initialize(DWORD dwMainThreadID);
	void SetText(const char* text);
	void Show();
	void Hide(HWND hFocusWnd);
	void Uninitialize();
};
