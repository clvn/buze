#pragma once

//#include "MainFrm.h"

struct PE_order_selection
{
	int index_low;
	int index_high;

	int GetSelCount() const {
		return index_high - index_low + 1;
	}
};

struct PE_order_pos
{
	PE_order_pos() :
		pat(0), is_playing(false), is_queue(false), is_begin_loop(false), is_end_loop(false)
	{}

	zzub_pattern_t* pat;
	bool is_playing;
	bool is_queue;
	bool is_begin_loop;
	bool is_end_loop;
};

class CPatternEditorOrderlist
:
	public CWindowImpl<CPatternEditorOrderlist>,
	public CMessageFilter
{
  public:

	DECLARE_WND_CLASS("PatternEditorOrderlist")

	BEGIN_MSG_MAP_EX(CPatternEditorOrderlist)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_MBUTTONDOWN(OnMButtonDown)
		MSG_WM_XBUTTONDOWN(OnXButtonDown)
		MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_SIZE(OnSize)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_KEYUP(OnKeyUp)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_MOUSEACTIVATE(OnMouseActivate)

		CMD_ID_HANDLER(ID_EDIT_COPY, OnEditCopy)
		CMD_ID_HANDLER(ID_EDIT_CUT, OnEditCut)
		CMD_ID_HANDLER(ID_EDIT_PASTE, OnEditPaste)
		CMD_ID_HANDLER(ID_ORDERLIST_INSERT, OnInsertOrder)
		CMD_ID_HANDLER(ID_ORDERLIST_DUPLICATE, OnDuplicatePattern)
		CMD_ID_HANDLER(ID_ORDERLIST_NEW, OnCreatePattern)
		CMD_ID_HANDLER(ID_ORDERLIST_REMOVE, OnRemoveOrder)
		CMD_ID_HANDLER(ID_ORDERLIST_REMOVE_DELETE, OnRemovePattern)
		CMD_ID_HANDLER(ID_ORDERLIST_BACKSPACE, OnBackspaceOrder)
		CMD_ID_HANDLER(ID_ORDERLIST_BACKSPACE_DELETE, OnBackspacePattern)
		CMD_ID_HANDLER(ID_ORDERLIST_SETBEGINLOOP, OnSetBeginLoop)
		CMD_ID_HANDLER(ID_ORDERLIST_SETENDLOOP, OnSetEndLoop)
		CMD_ID_HANDLER(ID_ORDERLIST_SETSELECTIONLOOP, OnSetSelectionLoop)
		CMD_ID_HANDLER(ID_ORDERLIST_PLAYORDER, OnPlayOrder)
		CMD_ID_HANDLER(ID_ORDERLIST_QUEUE, OnQueueOrder)
		CMD_ID_HANDLER(ID_ORDERLIST_DESELECT, OnDeselect)
		CMD_ID_HANDLER(ID_ORDERLIST_GOTO_EDITOR, OnGotoEditor)
		CMD_ID_HANDLER(ID_ORDERLIST_TOGGLEFOLLOW, OnToggleFollow)

///		CMD_ID_HANDLER(ID_ORDERLIST_DELETE, OnDeletePattern)used by menu
//CMD_ID_HANDLER(ID_ORDERLIST_NEW_NEWFORMAT, OnCreatePatternNewFormat)

		CMD_ID_HANDLER(ID_ORDERLIST_PROPERTIES, OnPatternProperties)
		CMD_ID_HANDLER(ID_ORDERLIST_RENDER, OnRenderOrder)
		COMMAND_RANGE_HANDLER_EX(ID_ORDERLIST_NEW_SETFORMAT_FIRST, ID_ORDERLIST_NEW_SETFORMAT_LAST, OnCreatePatternSetFormat)
		COMMAND_RANGE_HANDLER_EX(ID_ORDERLIST_SETPATTERN_FIRST, ID_ORDERLIST_SETPATTERN_LAST, OnSetPattern)
	END_MSG_MAP()

  // ---

	CViewFrame* mainFrame;
	CPatternView* owner;
	CDocument* document;

	CClientDC* clientDC;
	int scroll_index;
	int select_from;
	int select_to;
	int drag_mode;
	int drop_pos;
	COLORREF colorText;
	COLORREF colorTextLight;
	COLORREF colorTextSel;
	COLORREF colorTextSelLight;
	COLORREF colorWindow;
	COLORREF colorHighlight;
	COLORREF colorShadow;
	int box_height;
	CPoint mousemove_previous;
	bool got_repeat;
	bool mouse_activating;

	enum {
		drop_pos_none = -1,
	};

	enum {
		drag_mode_off,
		drag_mode_on,
		drag_mode_moved,
		drag_mode_valid,
	};

	enum {
		sel_mode_single,
		sel_mode_from,
		sel_mode_to,
	};

  // ---

	CPatternEditorOrderlist(CViewFrame* mainFrame, CPatternView* owner);
	virtual ~CPatternEditorOrderlist();
	void Init();

	void UpdateScrollInfo();
	void ScrollTo(int nPos);
	void EnsureVisibleCursor();
	void EnsureVisibleCursorRange();

	int GetBoxesOnscreenCount() const;
	int GetBoxesTotalCount() const;
	int GetLastBoxOnscreen() const;
	int GetOrderFromPoint(CPoint pt) const;
	int GetDropOrderFromPoint(CPoint pt) const;
	PE_order_selection GetSortedSel() const;
	bool SetSelect(int sel, int sel_mode);
	bool SetSelectRange(int from, int to);
	void StopDragging();

	void InvalidateOrder(int nOrder);
	void InvalidateSelection();
	void InvalidateDropPos();

	void MoveRight();
	void MoveLeft();
	void SyncOrderToPlayer(int nOrder);
	void SyncPlayerToOrder(bool force = false);
	void EditAndSync(bool force = false);
	void EditPattern(int nOrder);
	void FocusPatternEditor();

  // Template requirements
  public:

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);

  // Window Messages
  protected:

	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnPaint(CDCHandle dc);
	BOOL OnEraseBkgnd(CDCHandle dc) { return TRUE; }
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonDblClk(UINT nFlags, CPoint point);
	void OnRButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnMButtonDown(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar);
	void OnSize(UINT nType, CSize size);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSetFocus(CWindow wndOld);
	void OnKillFocus(CWindow wndFocus);
	void OnXButtonDown(int fwButton, int dwKeys, CPoint ptPos);
	int OnMouseActivate(CWindow wndTopLevel, UINT nHitTest, UINT message);

  // Commands
  protected:

	void CmdInsertEditorPattern();

	void OnEditCopy();
	void OnEditCut();
	void OnEditPaste();
	void OnInsertOrder();
	void OnRemoveOrder();
	void OnPatternProperties();
	void OnRenderOrder();
	void OnCreatePattern();
	void OnCreatePatternNewFormat();
	void OnCreatePatternSetFormat(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnDeletePattern();
	void OnDuplicatePattern();
	void OnSetBeginLoop();
	void OnSetEndLoop();
	void OnSetSelectionLoop();
	void OnSetPattern(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnPlayOrder();
	void OnQueueOrder();
	void OnRemovePattern();
	void OnBackspaceOrder();
	void OnBackspacePattern();
	void OnDeselect();
	void OnGotoEditor();
	void OnToggleFollow();

	void DoCopyOrders();
	void DoPasteOrders();
	void DoCreatePattern(bool newformat, int id);
	void DoDuplicatePatterns();
	void DoInsertOrders();
	void DoInsertEditorPattern();
	void DoSetBeginLoop(int nOrder);
	void DoSetEndLoop(int nOrder);
	void DoRemoveOrDeleteOrders(bool deletepat);
	void CmdDragSelection(bool makecopy);
	void CmdCyclePattern(int n, bool same_format);
	void DoSetPatterns(int id);
	void DoQueuePattern(int nOrder);
	void DoPlayPattern(int nOrder);
	void DoPlayUpdate(int mode, int idx);
};
