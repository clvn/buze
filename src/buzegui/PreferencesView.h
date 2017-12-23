#pragma once
#include "PropertyList/PropertyList.h"
#include "Properties.h"
#include "PropertyListView.h"
#include "PreferencesViewProperties.h"

class CPreferencesViewInfo : public CViewInfoImpl {
public:
	CPreferencesViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CPreferencesView 
	: public CViewImpl
	, public CWindowImpl<CPreferencesView>
	, public CIdleHandler
	, public CPropertyView
{
public:
	CPropertyListCtrl propertyList;
	CListViewCtrl pageList;
	CPreferencePropertyProvider* provider;
	CButton applyButton;
	CButton configureButton;
	HWND hReturnView;
	int lastPageListItem;

	DECLARE_WND_CLASS("PreferencesView")

	BEGIN_MSG_MAP(CPreferencesView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)

		NOTIFY_HANDLER(IDC_PREFERENCESPROPERTYLIST, PIN_ITEMCHANGED, OnSelChangeProperty)
		NOTIFY_HANDLER(IDC_PREFERENCESPROPERTYLIST, PIN_ITEMDELETE, OnDeleteProperty)
		NOTIFY_HANDLER(IDC_PREFERENCESPAGELIST, LVN_ITEMCHANGED, OnSelChangePage)
		COMMAND_HANDLER(IDC_APPLYBUTTON, BN_CLICKED, OnApplyClicked)
		COMMAND_HANDLER(IDC_CONFIGUREBUTTON, BN_CLICKED, OnConfigureClicked)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	CPreferencesView(CViewFrame* mainFrm);
	~CPreferencesView();
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSelChangeProperty(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnDeleteProperty(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnSelChangePage(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnApplyClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnConfigureClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	virtual void BindProvider();
	void SetConfigureButtonState();

	virtual BOOL OnIdle();
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
};
