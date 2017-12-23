#pragma once

#include "DragDropImpl.h"
#include "MachineDropTarget.h"

class CMachineFolderViewInfo : public CViewInfoImpl {
public:
	CMachineFolderViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CViewFrame;

const int IDC_MACHINETREE = 4321;

//using namespace DockSplitTab;

class CEditableTreeViewCtrl : public CWindowImpl<CEditableTreeViewCtrl, CTreeViewCtrl> {
public:
    DECLARE_WND_SUPERCLASS("EditableTreeViewCtrl", CTreeViewCtrl::GetWndClassName())

    BEGIN_MSG_MAP(CEditableTreeViewCtrl)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove) 
    END_MSG_MAP()

    LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class CMachineFolderView 
	: public CViewImpl
    , public CWindowImpl<CMachineFolderView>
    , public CMachineDropTargetWindow
{
public:
	CEditableTreeViewCtrl treeCtrl;
	HANDLE hMachineFolderSignal;

	DECLARE_WND_CLASS("MachineFolderView")

	BEGIN_MSG_MAP(CMachineFolderView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
//		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		NOTIFY_HANDLER(IDC_MACHINETREE, TVN_SELCHANGED, OnSelChange)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)

		NOTIFY_CODE_HANDLER(TVN_BEGINDRAG, OnBeginDrag);
		//COMMAND_ID_HANDLER(ID_VIEW_PROPERTIES, OnViewProperties)

	END_MSG_MAP()

	CMachineFolderView(CViewFrame* mainFrm);
	~CMachineFolderView(void);
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnBeginDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);	
	LRESULT OnSetFocus( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	//LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	//LRESULT OnViewProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    bool OnDropMachine(std::string machineName, std::string instrumentName, int x, int y);
	bool OnDragOver(const POINTL& pt, FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect);
	void OnDragLeave();
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {}

	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	int BindMachineItems(buze_plugin_index_item_t* item, HTREEITEM parent);
	void LoadFromIndex();

};
