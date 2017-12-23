// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/* .-----------------------------------.
   | XHtmlTree for WTL                 |
   | Version 1.7WTL                    |
   | Ported by Megz                    |
   | Original release notes are below. |
   '-----------------------------------' */

///////////////////////////////////////////////////////////////////////////////
//
// XHtmlTree.h  Version 1.6 - article available at www.codeproject.com
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// License:
//     This software is released into the public domain.  You are free to use
//     it in any way you like, except that you may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma once

#include "XHtmlDraw.h"

#ifdef XHTMLHTML
#include "XHtmlDrawLink.h"
#endif // XHTMLHTML

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

extern UINT WM_XHTMLTREE_CHECKBOX_CLICKED;
extern UINT WM_XHTMLTREE_ITEM_EXPANDED;
extern UINT WM_XHTMLTREE_DISPLAY_TOOLTIP;
extern UINT WM_XHTMLTREE_INIT_TOOLTIP;
extern UINT WM_XHTMLTREE_FOCUSED;
#ifdef XHTMLDRAGDROP
extern UINT WM_XHTMLTREE_BEGIN_DRAG;
extern UINT WM_XHTMLTREE_END_DRAG;
extern UINT WM_XHTMLTREE_DROP_HOVER;
#endif // XHTMLDRAGDROP
#ifdef XHTMLTREE_DEMO
extern UINT WM_XHTMLTREE_SCROLL_SPEED;
#endif // XHTMLTREE_DEMO

const int TV_NOIMAGE = 0xFFFE;

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Tree data
struct XHTMLTREEDATA
{
	XHTMLTREEDATA()
	:
		bChecked			(FALSE),
		bEnabled			(TRUE),
		bSeparator			(FALSE),

		// below are reserved for CXHtmlTree use only
		hTreeCtrl			(0),
		bExpanded			(FALSE),
		bHasBeenExpanded	(FALSE),
		bModified			(FALSE),
		nChildren			(0),
		nChecked			(0),
		nSeparators			(0),
		pszNote				(0),
		nTipWidth			(0)
	{
		++nCount;
	}

	virtual ~XHTMLTREEDATA() {
		if (pszNote) {
			delete[] pszNote;
		}
		pszNote = 0;
		--nCount;
	}

	BOOL	bChecked;				// TRUE = item checkbox is checked
	BOOL	bEnabled;				// TRUE = enabled, FALSE = disabled (gray text)
	BOOL	bSeparator;				// TRUE = item is separator; cannot have children

	// below are reserved for CXHtmlTree use only
	HWND	hTreeCtrl;				// HWND of tree control
	BOOL	bExpanded;				// TRUE = item is expanded to show its children
	BOOL	bHasBeenExpanded;		// TRUE = item has been expanded (at least once)
	BOOL	bModified;				// TRUE = item has been modified
	int		nChildren;				// total count of children of this item
	int		nChecked;				// count of children that are checked - an item in a "mixed" state is counted as being unchecked
	int		nSeparators;			// count of children that are separators
	TCHAR*	pszNote;				// note for tooltip
	int		nTipWidth;				// width of tooltip for note
	CXHtmlDraw::XHTMLDRAWSTRUCT ds;	// HTML draw info
	static int nCount;				// incremented in ctor, decremented in dtor
};

// data returned in notification messages
struct XHTMLTREEMSGDATA
{
	HWND		hCtrl;				// hwnd of XHtmlTree
	UINT		nCtrlId;			// id of XHtmlTree
	HTREEITEM	hItem;				// current item
};

// data returned in drag notification messages - not all data will be returned for every message
struct XHTMLTREEDRAGMSGDATA
{
	HTREEITEM	hItem;				// item being dragged
	HTREEITEM	hNewParent;			// proposed new parent
	HTREEITEM	hAfter;				// drop target - item being dragged will either sequentially follow this item,
									// or hAfter specifies the relationship (TVI_FIRST, TVI_LAST, etc.) the dragged item will have with hNewParent.
									// Note that TVI_xxxx constants are all defined as 0xFFFFnnnn, with the 16 high-order bits set.
	BOOL		bCopyDrag;			// TRUE = dropped item will be copied; FALSE = dropped item will be moved
};

// drag operations flags for m_dwDragOps
#define XHTMLTREE_DO_CTRL_KEY		0x0001	// TRUE = Ctrl key toggles move/copy; FALSE = Ctrl key is ignored
#define XHTMLTREE_DO_SHIFT_KEY		0x0002	// TRUE = Shift key toggles "move under" mode; FALSE = Shift key is ignored
#define XHTMLTREE_DO_SCROLL_NORMAL	0x0004	// TRUE = normal drag scroll
#define XHTMLTREE_DO_SCROLL_FAST	0x0008	// TRUE = fast drag scroll
#define XHTMLTREE_DO_COPY_DRAG		0x0010	// TRUE = change default drag from move to copy
#define XHTMLTREE_DO_AUTO_EXPAND	0x0020	// TRUE = auto-expand node when cursor hovers
#define XHTMLTREE_DO_DEFAULT		(XHTMLTREE_DO_CTRL_KEY | XHTMLTREE_DO_SHIFT_KEY | XHTMLTREE_DO_SCROLL_NORMAL | XHTMLTREE_DO_AUTO_EXPAND)

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//B: traits
//typedef ATL::CWinTraits<WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CTCS_TOOLTIPS, 0> CCustomTabCtrlWinTraits;

/*
If you use ON_NOTIFY_REFLECT_EX() in your message map, your message handler may or may not allow the parent window to handle the message.
If the handler returns FALSE, the message will be handled by the parent as well, while a call that returns TRUE does not allow the parent to handle it.
Note that the reflected message is handled before the notification message.
*/

class CXHtmlTree
:
	public CWindowImpl<CXHtmlTree, CTreeViewCtrl>,
	public CCustomDraw<CXHtmlTree>,
	public CDoubleBufferImpl<CXHtmlTree>,
	public CMessageFilter
{
  public:

	DECLARE_WND_SUPERCLASS("XHtmlTree", CTreeViewCtrl::GetWndClassName())

  // Message Map
  // -----------

	BEGIN_MSG_MAP_EX(CXHtmlTree)
		MSG_WM_CLOSE(OnClose)
		//MESSAGE_HANDLER(WM_COMMAND, OnCommand)//B
		MSG_WM_COMMAND(OnCommand)//B
		MSG_WM_CREATE(OnCreate)//B
		///ON_WM_DESTROY()
		MSG_WM_DESTROY(OnDestroy)
		///ON_WM_ERASEBKGND()
		//MSG_WM_ERASEBKGND(OnEraseBkgnd) <-- lame, not used with CDoubleBufferImpl
		///ON_WM_MOUSEMOVE()
		MSG_WM_MOUSEMOVE(OnMouseMove)
		///ON_WM_SYSCOLORCHANGE()
		MSG_WM_SYSCOLORCHANGE(OnSysColorChange)
		///ON_WM_TIMER()
		MSG_WM_TIMER(OnTimer)
		///ON_WM_LBUTTONDOWN()
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		///ON_WM_SIZE()
		MSG_WM_SIZE(OnSize)
		///ON_WM_RBUTTONDOWN()
		MSG_WM_SETFOCUS(OnSetFocus)
		//hehe
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		///ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
		//^-- NOT IN USE, using custom draw
		///ON_NOTIFY_REFLECT_EX(NM_CLICK, OnClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_CLICK, OnClick)//--OK
		///ON_NOTIFY_REFLECT_EX(NM_DBLCLK, OnDoubleClick)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnDoubleClick)//--OK
		///ON_NOTIFY_REFLECT_EX(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_BEGINLABELEDIT, OnBeginLabelEdit)//--OK
		///ON_NOTIFY_REFLECT_EX(TVN_ENDLABELEDIT, OnEndLabelEdit)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_ENDLABELEDIT, OnEndLabelEdit)//--OK
		
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_ITEMEXPANDING, OnExpanding)//B
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_ITEMEXPANDED, OnExpanded)//B
#ifdef XHTMLDRAGDROP
		///ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_BEGINDRAG, OnBeginDrag)
#endif // XHTMLDRAGDROP
		///ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
		NOTIFY_RANGE_CODE_HANDLER_EX(0, 0xFFFF, TTN_NEEDTEXTW, OnToolTipText)//?
		///ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
		NOTIFY_RANGE_CODE_HANDLER_EX(0, 0xFFFF, TTN_NEEDTEXTA, OnToolTipText)//?
		///ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnSelChanged)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGED, OnSelChanged)//--OK
		///ON_NOTIFY_REFLECT_EX(TVN_SELCHANGING, OnSelChanging)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(TVN_SELCHANGING, OnSelChanging)//--OK
		CHAIN_MSG_MAP_ALT(CCustomDraw<CXHtmlTree>, 1)//B
		CHAIN_MSG_MAP(CDoubleBufferImpl<CXHtmlTree>)//B
		DEFAULT_REFLECTION_HANDLER()//B
	END_MSG_MAP()

  // Construction / Destruction
  // --------------------------
  public:

	CXHtmlTree();
	virtual ~CXHtmlTree();

  // Attributes
  // ----------
  public:

	enum CheckedState {
		UNUSED1 = 0, 
		UNCHECKED,
		CHECKED,
		TRISTATE, 
		UNUSED2,
		UNCHECKED_DISABLED,
		CHECKED_DISABLED,
		TRISTATE_DISABLED,
	};

#ifdef XHTMLHTML
	CXHtmlDrawLink m_Links;
#endif // XHTMLHTML

	COLORREF	GetBkColor() { return m_crCustomWindow; }
	BOOL		GetBit(DWORD bits, DWORD bitmask) { return bits & bitmask; }
	BOOL		GetCheck(HTREEITEM hItem);
	int			GetCheckedCount();
	int			GetChildrenCheckedCount(HTREEITEM hItem);
	int			GetChildrenCount(HTREEITEM hItem);
	int			GetChildrenDisabledCount(HTREEITEM hItem);
	int			GetDefaultTipWidth();
	COLORREF	GetDisabledColor(COLORREF color);
	BOOL		GetDisplayToolTips() { return m_bToolTip; }
	DWORD		GetDragOps() { return m_dwDragOps; }
	HTREEITEM	GetFirstCheckedItem();
	BOOL		GetHasBeenExpanded(HTREEITEM hItem);
	BOOL		GetHasCheckBoxes() { return m_bCheckBoxes; }
	BOOL		GetHtml() { return m_bHtml; }
	BOOL		GetImages() { return m_bImages; }
	int			GetIndentLevel(HTREEITEM hItem);
	COLORREF	GetInsertMarkColor() { return m_crInsertMark; }
	BOOL		GetItemBold(HTREEITEM hItem);
	CString		GetItemNote(HTREEITEM hItem, BOOL bStripHtml = FALSE);
	int			GetItemNoteWidth(HTREEITEM hItem);
	///BOOL		GetItemPath(HTREEITEM hItem, CStringArray& sa, CPtrArray& items);
	BOOL		GetItemRect(HTREEITEM hItem, LPRECT lpRect, BOOL bTextOnly);
	COLORREF	GetItemTextBkColor(HTREEITEM hItem);
	COLORREF	GetItemTextColor(HTREEITEM hItem);
	XHTMLTREEDATA* GetItemDataStruct(HTREEITEM hItem);
	CString		GetItemText(HTREEITEM hItem, BOOL bStripHtml = FALSE) const;
	HTREEITEM	GetLastItem(HTREEITEM hItem);
	LOGFONT*	GetLogfont() { return &m_lf; }
	HTREEITEM	GetNextCheckedItem(HTREEITEM hItem);
	HTREEITEM	GetNextItem(HTREEITEM hItem);
	///HTREEITEM	GetNextItem(HTREEITEM hItem, UINT nCode) { return CTreeCtrl::GetNextItem(hItem, nCode); } // do not hide CTreeViewCtrl version
	HTREEITEM	GetNextItem(HTREEITEM hItem, UINT nCode) { return CWindowImpl<CXHtmlTree, CTreeViewCtrl>::GetNextItem(hItem, nCode); } // do not hide CTreeViewCtrl version
	HTREEITEM	GetPrevCheckedItem(HTREEITEM hItem);
	HTREEITEM	GetPrevItem(HTREEITEM hItem);
	BOOL		GetReadOnly() { return m_bReadOnly; }
	BOOL		GetSelectFollowsCheck() { return m_bSelectFollowsCheck; }
	COLORREF	GetSeparatorColor() { return m_crSeparator; } // +++ 1.6
	int			GetSeparatorCount(HTREEITEM hItem);
	BOOL		GetSmartCheckBox() { return m_bSmartCheck; }
	int			GetStateImage(HTREEITEM hItem);
	BOOL		GetStripHtml() { return m_bStripHtml; }
	COLORREF	GetTextColor() { return m_crCustomWindowText; }
	CToolTipCtrl* GetToolTips() { return m_pToolTip; }
	BOOL		GetUseLogfont() { return m_bLogFont; }

	BOOL		IsChecked(HTREEITEM hItem) { return GetCheck(hItem); }
	BOOL		IsChildNodeOf(HTREEITEM hitem, HTREEITEM hitemSuspectedParent);
	BOOL		IsEnabled(HTREEITEM hItem);
	BOOL		IsExpanded(HTREEITEM hItem);
	HTREEITEM	IsOverItem(LPPOINT lpPoint = NULL);
	BOOL		IsSelected(HTREEITEM hItem);
	BOOL		IsSeparator(HTREEITEM hItem);

	COLORREF	SetBkColor(COLORREF rgb);
	void		SetBit(DWORD& bits, DWORD bitmask, BOOL value) { if (value) bits |= bitmask; else bits &= ~bitmask; }
	CXHtmlTree&	SetCheck(HTREEITEM hItem, BOOL fCheck = TRUE, BOOL bSendWM = FALSE);
	CXHtmlTree&	SetCheckChildren(HTREEITEM hItem, BOOL fCheck);
	CXHtmlTree&	SetDefaultTipWidth(int nDefaultTipWidth) { m_nDefaultTipWidth = nDefaultTipWidth; return *this; }
	CXHtmlTree&	SetDisplayToolTips(BOOL bFlag) { m_bToolTip = bFlag; return *this; }
	CXHtmlTree&	SetDragOps(DWORD dwOps) { m_dwDragOps = dwOps; return *this; }
	CXHtmlTree&	SetDropCursors(UINT nNoDropCursor, UINT nDropCopyCursor, UINT nDropMoveCursor) {
		m_nNoDropCursor = nNoDropCursor; m_nDropCopyCursor = nDropCopyCursor; m_nDropMoveCursor = nDropMoveCursor; return *this;
	}
	CXHtmlTree&	SetHasCheckBoxes(BOOL bHasCheckBoxes) { m_bCheckBoxes = bHasCheckBoxes; return *this; }
	CXHtmlTree&	SetHtml(BOOL bFlag) { m_bHtml = bFlag; return *this; }
	CXHtmlTree&	SetImages(BOOL bFlag) { m_bImages = bFlag; return *this; }
	COLORREF	SetInsertMarkColor(COLORREF rgb);
	BOOL		SetItemBold(HTREEITEM hItem, BOOL bBold);
	CXHtmlTree&	SetItemNote(HTREEITEM hItem, LPCTSTR lpszNote, int nTipWidth = 0);
	BOOL		SetItemText(HTREEITEM hItem, LPCTSTR lpszItem);
	COLORREF	SetItemTextBkColor(HTREEITEM hItem, COLORREF rgb);
	COLORREF	SetItemTextColor(HTREEITEM hItem, COLORREF rgb);
	CXHtmlTree&	SetItemStateChildren(HTREEITEM hItem, BOOL fCheck);
	CXHtmlTree&	SetLogfont(LOGFONT* pLogFont);
	CXHtmlTree&	SetReadOnly(BOOL bReadOnly) { m_bReadOnly = bReadOnly; return *this; }
	CXHtmlTree&	SetSelectFollowsCheck(BOOL bFlag) { m_bSelectFollowsCheck = bFlag; return *this; }
	CXHtmlTree&	SetSeparatorColor(COLORREF rgb); // +++ 1.6
	CXHtmlTree&	SetSmartCheckBox(BOOL bFlag) { m_bSmartCheck = bFlag; return *this; }
	CXHtmlTree&	SetStripHtml(BOOL bFlag) { m_bStripHtml = bFlag; return *this; }
	COLORREF	SetTextColor(COLORREF rgb);
	CToolTipCtrl* SetToolTips(CToolTipCtrl* pWndTip) { CToolTipCtrl* old = m_pToolTip; m_pToolTip = pWndTip; return old; }
	CXHtmlTree&	SetUseLogfont(BOOL bFlag) { m_bLogFont = bFlag; return *this; }

  // Operations
  // ----------
  public:

	void		CheckAll(BOOL bCheck);
	void		CollapseBranch(HTREEITEM hItem);
	BOOL		CreateCheckboxImages();
	void		CreateToolTipsForTree();
	BOOL		DeleteItem(HTREEITEM hItem);
	BOOL		EnableBranch(HTREEITEM hItem, BOOL bEnabled);
	BOOL		EnableItem(HTREEITEM hItem, BOOL bEnabled);
	BOOL		EnableWindow(BOOL bEnable = TRUE);
	BOOL		Expand(HTREEITEM hItem, UINT nCode);
	void		ExpandBranch(HTREEITEM hItem);
	void		ExpandAll();
	void		CollapseAll();
	BOOL		DeleteAllItems();
	virtual HTREEITEM FindItem(CString const& sSearch, BOOL bCaseSensitive = FALSE, BOOL bDownDir = TRUE, BOOL bWholeWord = FALSE, BOOL bWrap = TRUE, HTREEITEM hItem = NULL);
	int			IncrementChildren(HTREEITEM hItem, int n = 1);
	int			IncrementSeparators(HTREEITEM hItem, int n = 1);
	CXHtmlTree&	Initialize(BOOL bCheckBoxes = FALSE, BOOL bToolTip = FALSE);
	HTREEITEM	InsertItem(LPTVINSERTSTRUCT lpInsertStruct, XHTMLTREEDATA* pData = NULL);
	HTREEITEM	InsertItem(UINT nMask, LPCTSTR lpszItem, int nImage, int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam, HTREEITEM hParent, HTREEITEM hInsertAfter);
	HTREEITEM	InsertItem(LPCTSTR lpszItem, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST );
	HTREEITEM	InsertItem(LPCTSTR lpszItem, int nImage, int nSelectedImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
	HTREEITEM	InsertSeparator(HTREEITEM hItem);
	void		RedrawItem(HTREEITEM hItem);
	BOOL		SelectItem(HTREEITEM hItem);
	BOOL		SelectNext();
	BOOL		SelectPrev();
	HCURSOR		SetCursor(HCURSOR hCursor);

  // Virtual Overrides
  // -----------------

  public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
  protected:
	///virtual void PreSubclassWindow();
	///virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	///virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	///virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

  // Static Overrides
  // ----------------
  public:

	BOOL SubclassWindow(HWND hWnd);//B
	void DoPaint(CDCHandle dcPaint); // For doublebufferimpl
	DWORD OnPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw); // For customdraw
	DWORD OnItemPrePaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw); // For customdraw
	DWORD OnItemPostPaint(int idCtrl, LPNMCUSTOMDRAW lpNMCustomDraw); // For customdraw

  // Implementation
  // --------------
  protected:

	//CMap<HTREEITEM, HTREEITEM, XHTMLTREEDATA*, XHTMLTREEDATA*&> m_DataMap; // maps HTREEITEM ==> XHTMLTREEDATA
	CAtlMap<HTREEITEM, XHTMLTREEDATA*> m_DataMap;

	BOOL			m_bReadOnly;			// TRUE = tree is read-only (checkboxes disabled)
	BOOL			m_bSmartCheck;			// TRUE = Smart Checkboxes enabled
	BOOL			m_bCheckBoxes;			// TRUE = checkboxes enabled
	BOOL			m_bSelectFollowsCheck;	// TRUE = item will be selected, after user clicks on checkbox
	BOOL			m_bHtml;
	BOOL			m_bStripHtml;
	BOOL			m_bLogFont;
	BOOL			m_bToolTip;
	BOOL			m_bImages;				// TRUE = allow images
	BOOL			m_bDestroyingTree;
	BOOL			m_bFirstTime;
	BOOL			m_bDragging;
	BOOL			m_bAutoScroll;			// TRUE = tree will automatically scroll when dragging
	HTREEITEM		m_hAnchorItem;
	HTREEITEM		m_hHotItem;
	HTREEITEM		m_hPreviousItem;
	HTREEITEM		m_hItemButtonDown;
	HTREEITEM		m_hPreviousDropItem;
	DWORD			m_dwDropHoverTime;		// number of ticks over a drop item
	int				m_nPadding;				// horizontal padding in pixels
	int				m_nImageHeight;
	int				m_nToolCount;			// no. of items added to tooltip control
	int				m_nDefaultTipWidth;
	int				m_nDeleted;
	int				m_nDeletedChecked;
	CImageList		m_StateImage;
	LOGFONT			m_lf;
	int				m_nHorzPos;				// initial horz scroll position - saved before in-place edit begins
	UINT			m_nScrollTime;			// used for scrolling while dragging
	DWORD			m_dwDragOps;			// drag features
	UINT			m_nNoDropCursor;		// resource ID for no-drop cursor
	UINT			m_nDropCopyCursor;		// resource ID for drop (copy) cursor
	UINT			m_nDropMoveCursor;		// resource ID for drop (move) cursor
	HCURSOR			m_hNoDropCursor;		// no-drop cursor handle
	HCURSOR			m_hDropCopyCursor;		// drop (copy) cursor handle
	HCURSOR			m_hDropMoveCursor;		// drop (move) cursor handle
	HCURSOR			m_hPreviousCursor;
	HCURSOR			m_hCurrentCursor;
	CToolTipCtrl*	m_pToolTip;
	COLORREF		m_crCustomWindow;
	COLORREF		m_crCustomWindowText;
	COLORREF		m_crWindow;
	COLORREF		m_crWindowText;
	COLORREF		m_crAnchorText;
	COLORREF		m_crGrayText;
	COLORREF		m_crHighlight;
	COLORREF		m_crHighlightText;
	COLORREF		m_crInsertMark;
	COLORREF		m_crSeparator;

	void		AdjustEditRect();
	void		AutoScroll(HTREEITEM hItem);
	void		DeleteBranch(HTREEITEM hItem);

	void		DeleteMap();
	//int			DrawItemText(CDC* pDC, HTREEITEM hItem, LPCTSTR lpszText, COLORREF crText, COLORREF crTextBackground, COLORREF crBackground, CRect const& rectText);
	int			DrawItemText(CDCHandle pDC, HTREEITEM hItem, LPCTSTR lpszText, COLORREF crText, COLORREF crTextBackground, COLORREF crBackground, CRect const& rectText);
#ifdef XHTMLHTML
	///int		DrawItemTextHtml(CDC* pDC, HTREEITEM hItem, LPCTSTR lpszText, COLORREF crText, COLORREF crTextBackground, COLORREF crBackground, COLORREF crAnchorText, CRect const& rectText);
	int			DrawItemTextHtml(CDCHandle pDC, HTREEITEM hItem, LPCTSTR lpszText, COLORREF crText, COLORREF crTextBackground, COLORREF crBackground, COLORREF crAnchorText, CRect const& rectText);
#endif // XHTMLHTML
	///int		DrawSeparator(CDC* pDC, HTREEITEM hItem, COLORREF crText, COLORREF crBackground, CRect rectSep);
	int			DrawSeparator(CDCHandle pDC, HTREEITEM hItem, COLORREF crText, COLORREF crBackground, CRect rectSep);
	void		EndDragScroll();
	int			GetNormalImageWidth(HTREEITEM hItem);
	BOOL		IsBadRect(CRect const& r) { return (r.IsRectEmpty() || (r.Height() <= 0) || (r.Width() <= 0)); }
	virtual BOOL IsFindValid(HTREEITEM);
	BOOL		IsLeftButtonUp();
	BOOL		IsOverAnchor(HTREEITEM hItem, CPoint point, CRect* pRect = NULL);
	BOOL		PreDisplayToolTip(BOOL bAlwaysRemoveHtml, CString& strToolTip);
	LRESULT		SendRegisteredMessage(UINT nMessage, HTREEITEM hItem, LPARAM lParam = 0);
	int			SetCheckParent(HTREEITEM hItem, int nCount);
	void		SetColors();
	void		SetHotItem(HTREEITEM hItem, UINT nFlags);

#ifdef XHTMLDRAGDROP
	HCURSOR		GetDragCursor();
	BOOL		IsCtrlDown();
	BOOL		IsDragCopy();
	HTREEITEM	MoveBranch(HTREEITEM hBranch, HTREEITEM hNewParent, HTREEITEM hAfter = TVI_LAST);
	HTREEITEM	MoveItem(HTREEITEM hItem, HTREEITEM hNewParent, HTREEITEM hAfter = TVI_LAST);
	void		SetDragCursor();
	BOOL		StartMoveBranch(HTREEITEM hItem, HTREEITEM hNewParent, HTREEITEM hAfter = TVI_LAST);
#endif // XHTMLDRAGDROP

	BOOL WindowSetup();//B

  // Message Handlers
  // ----------------
  protected:

	///BOOL OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnClick(LPNMHDR lpNMHDR);
	///void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	//replaced by wtl customdraw things
	///BOOL OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnDoubleClick(LPNMHDR lpNMHDR);
	///void OnDestroy();
	void OnDestroy();
	///BOOL OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnBeginLabelEdit(LPNMHDR lpNMHDR);
	///BOOL OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnEndLabelEdit(LPNMHDR lpNMHDR);
	///BOOL OnEraseBkgnd(CDC* pDC);
	//BOOL OnEraseBkgnd(CDCHandle pDC); // using customdraw
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnRButtonDown(UINT nFlags, CPoint point);
	///void OnSize(UINT nType, int cx, int cy);
	void OnSize(UINT nType, CSize size);
	void OnSysColorChange();
	void OnTimer(UINT nIDEvent);
#ifdef XHTMLDRAGDROP
	///void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnBeginDrag(LPNMHDR lpNMHDR);
#endif // XHTMLDRAGDROP
	///virtual BOOL OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnToolTipText(NMHDR* pNMHDR);
	///BOOL OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnSelChanged(LPNMHDR lpNMHDR);
	///BOOL OnSelChanging(NMHDR* pNMHDR, LRESULT* pResult);
	LRESULT OnSelChanging(LPNMHDR lpNMHDR);
	int OnCreate(LPCREATESTRUCT lpCreateStruct);//B
	void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnClose();
	LRESULT OnExpanding(LPNMHDR lpNMHDR);
	LRESULT OnExpanded(LPNMHDR lpNMHDR);
	void OnSetFocus(CWindow wndOld);
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
