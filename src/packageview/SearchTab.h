#pragma once

class CPackageView;
// resource.h
#define IDC_SEARCHBUTTON 1000

class CSearchTab : public CWindowImpl<CSearchTab> {
	CPackageView* view;
public:
	CSearchTab(CPackageView* view);

	CStatic searchLabel;
	CEdit searchEdit;
	CButton searchButton;
	// Checkboxlist: filter? category, effect, synth, vst, buzz, view
	// Only installed, show beta plugins

	DECLARE_WND_CLASS("SearchTab")

	BEGIN_MSG_MAP(CSearchTab)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

		COMMAND_HANDLER(IDC_SEARCHBUTTON, BN_CLICKED, OnSearchClicked)
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSearchClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

};
