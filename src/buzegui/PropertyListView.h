#pragma once

#include "ToolbarWindow.h"
#include "ToolbarBands.h"
#include "PropertyList/PropertyList.h"

class CPropertyListViewInfo : public CViewInfoImpl {
public:
	CPropertyListViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
};

// refer to this for usage of the propertylis
// http://www.viksoe.dk/code/propertylist.htm

const int IDC_PROPERTYLIST = 129;
const int IDC_OBJECTDROPDOWN = 130;

class CPropertyListView 
	: public CViewImpl
	, public CToolbarWindow<CPropertyListView>
	, public CMessageFilter
{
public:
	CComboListBand objectCombo;
	CPropertyListCtrl propertyList;
	CPropertyProvider* provider;
	CStatic infoPane;
	HWND hReturnView;
	bool dirtyProperties;

	DECLARE_WND_CLASS("PropertyListView")

	BEGIN_MSG_MAP(CPropertyListView)
		CHAIN_MSG_MAP(CToolbarWindow<CPropertyListView>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		NOTIFY_HANDLER(IDC_PROPERTYLIST, PIN_ITEMCHANGED, OnItemChangeProperty)
		NOTIFY_HANDLER(IDC_PROPERTYLIST, PIN_SELCHANGED, OnSelChangeProperty)
		COMMAND_HANDLER(IDC_OBJECTDROPDOWN, CBN_SELCHANGE, OnSelChangeObject)
		COMMAND_ID_HANDLER(ID_PROPERTYVIEW_RETURNTOVIEW, OnReturnToView)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	CPropertyListView(CViewFrame* mainFrm);
	~CPropertyListView(void);
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSelChangeProperty(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnItemChangeProperty(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnSelChangeObject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnReturnToView(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void UpdateFromProvider(CPropertyProvider* provider);
	void ClearAll();
	void UpdateInfo();
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	void UpdateFromEvent(CView* pSender, buze_event_data_t* ev);
};
