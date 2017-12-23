#pragma once

#include "XHtmlTreeWTL/XHtmlTree.h"
#include "boost/unordered_map.hpp"

class CViewFrame;
class CPatternView;
class CDocument;

struct PE_valueinfo {
	int value;
	std::string key;
	std::string description;
};

struct PE_treeiteminfo {
	bool is_format;
	zzub_pattern_format_t* format;
	zzub_pattern_t* pattern;
};

class CInfoPane : public CWindowImpl<CInfoPane>
{
  public:

	DECLARE_WND_CLASS("InfoPane")

	BEGIN_MSG_MAP_EX(CInfoPane)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_SETFOCUS(OnSetFocus)
		MESSAGE_HANDLER_EX(WM_CONTEXTMENU, OnContextMenu)
		// tree
		REGISTERED_MSG_HANDLER(WM_XHTMLTREE_FOCUSED, OnTreeFocused)
		CMD_ID_HANDLER(ID_INFOPANE_FORMAT_CREATE, OnFormatCreate)
		CMD_ID_HANDLER(ID_INFOPANE_FORMAT_DELETE, OnFormatDelete)
		CMD_ID_HANDLER(ID_INFOPANE_FORMAT_CLONE, OnFormatClone)
		CMD_ID_HANDLER(ID_INFOPANE_FORMAT_SETUP, OnFormatSetup)
		CMD_ID_HANDLER(ID_INFOPANE_FORMAT_PROPS, OnFormatProperties)
		CMD_ID_HANDLER(ID_INFOPANE_PATTERN_CREATE, OnPatternCreate)
		CMD_ID_HANDLER(ID_INFOPANE_PATTERN_DELETE, OnPatternDelete)
		CMD_ID_HANDLER(ID_INFOPANE_PATTERN_CLONE, OnPatternClone)
		CMD_ID_HANDLER(ID_INFOPANE_PATTERN_PROPS, OnPatternProperties)
		CMD_ID_HANDLER(ID_INFOPANE_PATTERN_EDIT, OnPatternEdit)
		NOTIFY_CODE_HANDLER_EX(TVN_BEGINLABELEDIT, OnBeginLabelEditTree)
		NOTIFY_CODE_HANDLER_EX(TVN_ENDLABELEDIT, OnEndLabelEditTree)
		// ---
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()

	CInfoPane(CViewFrame* mainFrame, CPatternView* owner);
	~CInfoPane();

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnSize(UINT nType, CSize size);
	void OnSetFocus(CWindow wndOld);
	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam);

	CViewFrame* mainFrame;
	CPatternView* owner;
	zzub_player_t* player;
	CDocument* document;

	// pane switching control
	void SwitchWindow(bool selWnd);
	bool currentWnd;

	// column list
	CListViewCtrl columnList;

	// pattern tree
	LRESULT OnTreeFocused(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnFormatCreate();
	void OnFormatDelete();
	void OnFormatClone();
	void OnFormatSetup();
	void OnFormatProperties();
	void OnPatternCreate();
	void OnPatternDelete();
	void OnPatternClone();
	void OnPatternProperties();
	void OnPatternEdit();
	LRESULT OnBeginLabelEditTree(NMHDR* pNMHDR);
	LRESULT OnEndLabelEditTree(NMHDR* pNMHDR);

	void PatternTreeRebuild();
	void PatternTreeInsertPattern(zzub_pattern_t* pat);
	void PatternTreeUpdatePattern(zzub_pattern_t* pat);
	void PatternTreeDeletePattern(zzub_pattern_t* pat);
	void PatternTreeInsertPatternFormat(zzub_pattern_format_t* fmt);
	void PatternTreeUpdatePatternFormat(zzub_pattern_format_t* fmt);
	void PatternTreeDeletePatternFormat(zzub_pattern_format_t* fmt);

	CXHtmlTree patternTree;
	CImageList m_TreeImages;
	typedef boost::unordered_map<HTREEITEM, PE_treeiteminfo> tim_t;
	tim_t treeItemMapForward;
	typedef std::pair<zzub_pattern_format_t*, zzub_pattern_t*> timr_pair_t;
	typedef boost::unordered_map<timr_pair_t, HTREEITEM> timr_t;
	timr_t treeItemMapReverse;
	HTREEITEM m_hTrackItem;

	void PatternTreeMakePatternVisible(zzub_pattern_t* pat);
	void PatternTreeMakeFormatVisible(zzub_pattern_format_t* fmt);
};
