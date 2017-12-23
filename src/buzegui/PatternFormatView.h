#pragma once

#include "ToolbarWindow.h"
#include "ToolbarBands.h"
#include "XHtmlTreeWTL/XHtmlTree.h"

#include "boost/unordered_map.hpp"

class CPatternFormatViewInfo : public CViewInfoImpl {
public:
	CPatternFormatViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
};

struct COLUMNINFO {
	zzub_plugin_t* plugin;
	int group, track, column;
	bool is_plugin_category;
};

class CPatternFormatView 
:
	public CToolbarWindow<CPatternFormatView>,
	public CViewImpl,
	public CMessageFilter
{
  public:
	CComboListBand formatDropDown;
	CEditBand pluginFilter;
	CEditBand parameterFilter;
	CImageList m_Images;
	CXHtmlTree formatTree;
	HWND hReturnView;
	zzub_pattern_format_t* format;
	typedef boost::unordered_map<HTREEITEM, COLUMNINFO> columnmap_t;
	columnmap_t columnmap; // HTREEITEM -> COLUMNINFO
	typedef boost::unordered_map<std::pair<zzub_parameter_t*, int>, HTREEITEM> treemap_t;
	treemap_t treemap; // <param, track> -> HTREEITEM
	bool dirtyFormat;
	bool dirtyChecks;
	bool dirtyToolbar;
	bool dirtyToolbarSel;

	DECLARE_WND_CLASS("PatternFormatView")

	BEGIN_MSG_MAP_EX(CPatternFormatView)		
		CHAIN_MSG_MAP(CToolbarWindow<CPatternFormatView>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		//MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
		COMMAND_HANDLER(IDC_FORMATDROPDOWN, CBN_SELCHANGE, OnSelChangeFormat)
		COMMAND_HANDLER(IDC_FORMATDROPDOWN, CBN_CLOSEUP, OnRestoreFocus)
		COMMAND_HANDLER(IDC_FORMATPLUGINFILTER, EN_CHANGE, OnPluginFilterChange)
		COMMAND_HANDLER(IDC_FORMATPARAMETERFILTER, EN_CHANGE, OnParameterFilterChange)
		MESSAGE_HANDLER(WM_XHTMLTREE_CHECKBOX_CLICKED, OnCheckChange)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	CPatternFormatView(CViewFrame* mainFrm);
	~CPatternFormatView();
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	//LRESULT OnErase(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRestoreFocus(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCheckChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSelChangeFormat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPluginFilterChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnParameterFilterChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void Init(zzub_pattern_format_t* format);
	void SetFormatChecks();
	void BindFormat();
	void BindPlugin(zzub_plugin_t* plugin, COLUMNINFO* ciSelItem, HTREEITEM& foundSel);
	void BindParameter(HTREEITEM hPlugItem, zzub_plugin_t* plugin, int group, int track, int column, COLUMNINFO* ciSelItem, HTREEITEM& foundSel);
	HTREEITEM fmtCol2ht(zzub_pattern_format_column_t* col) const;
	HTREEITEM paramTrk2ht(zzub_parameter_t*, int track) const;
	void bindDropDown();
	void setDropDown();
	void GetPluginFilter(std::vector<std::string>& result);
	void GetParameterFilter(std::vector<std::string>& result);
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
};
