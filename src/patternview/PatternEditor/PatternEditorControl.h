#pragma once

#include <boost/shared_ptr.hpp>
#include "SplitterWindowKey.h"

class CPatternEditorControl
:
	public CWindowImpl<CPatternEditorControl>
{
  public:

	int margin_x;
	int margin_y;
	int replay_row;

	int last_scroll_y;
	int last_scroll_x;

	bool subrow_mode;

	bool row_held;

	CPatternEditorInner editorInner;
	CToolTipCtrl headertip; 
	std::vector<boost::shared_ptr<CToolInfo> > headerinfos;

	typedef boost::unordered_map<PE_column_pos, PE_column*> column_map_t;
	column_map_t column_map;
	CPen mark_pen;

	DECLARE_WND_CLASS(_T("PatternEditorControl"))

	BEGIN_MSG_MAP_EX(CPatternEditorControl)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnAnyMouseMessage)
		MESSAGE_HANDLER_EX(WM_CREATE, OnCreate)
		MESSAGE_HANDLER_EX(WM_CLOSE, OnClose)
		MESSAGE_HANDLER_EX(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER_EX(WM_SIZE, OnSize)
		MESSAGE_HANDLER_EX(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER_EX(WM_PAINT, OnPaint)
		MESSAGE_HANDLER_EX(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER_EX(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER_EX(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER_EX(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		MESSAGE_HANDLER_EX(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER_EX(WM_RBUTTONDBLCLK, OnRButtonDown)
		MESSAGE_HANDLER_EX(WM_XBUTTONDOWN, OnForward)
		MESSAGE_HANDLER_EX(WM_MOUSEWHEEL, OnForward)
		MESSAGE_HANDLER_EX(WM_CHAR, OnChar)
		COMMAND_ID_HANDLER_EX(ID_PATTERNEDITORINNER_SCROLLED, OnScrolled)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	CPatternEditorControl(CPatternEditorScroller& editorScroller);
	~CPatternEditorControl();

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnForward(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnScrolled(WORD wNotifyCode, WORD wID, HWND hWndCtl);
	LRESULT OnAnyMouseMessage(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	// binding
	void SetPatternRows(int num);
	void SetSkip(int num);
	void SetResolution(int num);
	void SetFont(HFONT hFont, bool bRedraw = 1);
	void SetThemeColor(PE_themeindex index, COLORREF color);
	void SetTooltips();

	// painting
	void InvalidateCols();
	void InvalidateRows();

	// helpers
	int TrackFromPoint(POINT pt) const;
	int RowFromPoint(POINT pt) const;

	// p,g,t,c adapters
	void SelectRange(int from_plugin_id, int from_group, int from_track, int from_column, int from_row, int to_plugin_id, int to_group, int to_track, int to_column, int to_row);
	bool SetCursor(int plugin_id, int group, int track, int column, int digit, int row);
	PE_column* GetColumn(int plugin_id, int group, int track, int column);
	PE_column* GetColumnByEvent(int patternevent_id);
	int GetColumnIndex(int plugin_id, int group, int track, int column) const;
	int GetColumnCount(int plugin_id, int group, int track) const;
	int GetTrackCount(int plugin_id, int group) const;
	PE_track const& GetTrack(int plugin_id, int group, int track) const;
};
