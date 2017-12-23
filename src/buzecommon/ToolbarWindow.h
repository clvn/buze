#pragma once

// ---------------------------------------------------------------------------------------------------------------
// FORWARDING REBAR
// ---------------------------------------------------------------------------------------------------------------

// code == HIWORD
// id   == LOWORD
#define FORWARD_NOTIFICATIONS_DLGCTRLID() { \
		bHandled = TRUE; \
		WPARAM wParamDlg = MAKELONG(GetDlgCtrlID(), HIWORD(wParam)); \
		lResult = ForwardNotifications(uMsg, wParamDlg, lParam, bHandled); \
		if (bHandled) return TRUE; \
	}

class CReBarForwardCtrl : public CWindowImpl<CReBarForwardCtrl, CReBarCtrl>
{
  public:

	DECLARE_WND_SUPERCLASS("ReBarForwardCtrl", CReBarCtrl::GetWndClassName())

	BEGIN_MSG_MAP(CReBarForwardCtrl)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
		::SendMessage(GetParent(), WM_CONTEXTMENU, (WPARAM)m_hWnd, GetMessagePos());
		return 0;
	}
};

// ---------------------------------------------------------------------------------------------------------------
// TOOLBARWINDOW BASECLASS
// ---------------------------------------------------------------------------------------------------------------

template <class T>
class CToolbarWindow
:
	public CWindowImpl<T>
{
  public:

	DECLARE_WND_CLASS("ToolbarWindow")

	BEGIN_MSG_MAP(CToolbarWindow<T>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		NOTIFY_CODE_HANDLER(RBN_HEIGHTCHANGE, OnHeightChange);
		NOTIFY_CODE_HANDLER(TTN_NEEDTEXT, OnToolbarNeedText)
	END_MSG_MAP()

	CReBarForwardCtrl toolBar;

	LRESULT OnHeightChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		RECT rc;
		GetClientRect(&rc);
		if (rc.right == 0 || rc.bottom == 0) return 0;
		SendMessage(WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
		toolBar.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CCS_NODIVIDER|CCS_NOPARENTALIGN|RBS_VARHEIGHT|RBS_TOOLTIPS, WS_EX_COMPOSITED);
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled) {
		bHandled = FALSE;
        if (toolBar.m_hWnd == 0) return 0;
		RECT rc;
		GetClientRect(&rc);
		if (rc.right == 0 || rc.bottom == 0) return 0;
		toolBar.MoveWindow(0, 0, rc.right, getToolbarHeight());
		return 0;
	}

	LRESULT OnToolbarNeedText(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
		std::string tooltip;
		//-- make sure this 1is not a seperator
		LPNMTTDISPINFO pttdi = reinterpret_cast<LPNMTTDISPINFO> (pnmh);
		pttdi->hinst = _Module.GetResourceInstance();
		pttdi->uFlags = TTF_DI_SETITEM;

		if (idCtrl != 0) {
			char pc[1024];
			if (::LoadString(_Module.GetResourceInstance(), idCtrl, pc, 1024)) {
				pttdi->lpszText = MAKEINTRESOURCE(idCtrl);
			} else {
				pttdi->lpszText = MAKEINTRESOURCE(ID_BLANK_STRING);
			}
		}
		return 0;
	}

	int getToolbarHeight() const {
        if (!toolBar.m_hWnd) return 0;
        return toolBar.GetBarHeight();
	}

	int getMinMaxHeight() const {
		int comboHeight = GetSystemMetrics(SM_CYVSCROLL) + (GetSystemMetrics(SM_CYEDGE) * 2);
		int toolHeight = 24;
		return max(toolHeight, comboHeight + 2);
	}

	bool insertToolbarBand(HWND ctrlWnd, std::string const& label, int minWidth, int minmaxHeight = -1, BOOL bVisible = TRUE, BOOL bLock = FALSE, BOOL bBreak = FALSE, WORD nID = 0) {
		if (minmaxHeight == -1) {
			minmaxHeight = getMinMaxHeight();
		}

		REBARBANDINFO rbi;
		rbi.cbSize = sizeof(REBARBANDINFO);
		rbi.fMask = 0
			//| RBBIM_BACKGROUND // hbmBack
			| RBBIM_CHILD // hwndChild
			| RBBIM_STYLE // fStyle
			| RBBIM_SIZE // cx
			| RBBIM_CHILDSIZE // cxMinChild, cyMinChild   +   cyChild, cyMaxChild if (RBBS_VARIABLEHEIGHT)
			| RBBIM_IDEALSIZE // cxIdeal
			| RBBIM_ID // wID
		;

		if (bLock)
			rbi.fStyle = RBBS_NOGRIPPER;
		else
			rbi.fStyle = RBBS_GRIPPERALWAYS;

		if (bBreak) 
			rbi.fStyle |= RBBS_BREAK;

		if (!bVisible) 
			rbi.fStyle |= RBBS_HIDDEN;

		rbi.cx = minWidth;
		rbi.cxMinChild = minWidth;
		rbi.cxIdeal = minWidth;
		rbi.cyChild = minmaxHeight;
		rbi.cyMinChild = minmaxHeight;
		rbi.cyMaxChild = minmaxHeight;
		if (label.length()) {
			rbi.fMask |= RBBIM_TEXT;
			rbi.lpText = new char[label.length()+1]; // TODO: find out if this memory is ever released by someone
			strcpy(rbi.lpText, label.c_str());
			rbi.cch = (UINT)label.length();
		}
		//rbi.hbmBack = 0;
		rbi.hwndChild = ctrlWnd;
		if (nID == 0)
			rbi.wID = ATL_IDW_BAND_FIRST + toolBar.GetBandCount();
		else
			rbi.wID = nID;
		toolBar.InsertBand(-1, &rbi);
		if (!bVisible) toolBar.ShowBand(toolBar.GetBandCount() - 1, bVisible);
		return true;
	}

	bool insertBlankToolbarBand() {
		int minmaxHeight = getMinMaxHeight();
		REBARBANDINFO rbi;
		rbi.cbSize= sizeof(REBARBANDINFO);
		rbi.fMask = RBBIM_BACKGROUND | RBBIM_STYLE | RBBIM_SIZE;
		rbi.fStyle = RBBS_NOGRIPPER;
		rbi.cxMinChild = 50;
		rbi.cyMinChild = minmaxHeight;
		rbi.cyMaxChild = minmaxHeight;
		rbi.cx = 0;
		rbi.hbmBack = 0;
		toolBar.InsertBand(-1, &rbi);
		return true;
	}

    int getBandIndex(HWND ctrl) const {
        for (UINT i = 0; i < toolBar.GetBandCount(); ++i) {
            REBARBANDINFO rbbi;
            rbbi.cbSize = sizeof(REBARBANDINFO);
            rbbi.fMask = RBBIM_CHILD;
            toolBar.GetBandInfo(i, &rbbi);
            if (rbbi.hwndChild == ctrl) return i;
        }
        return -1;
    }

    bool getBandRect(HWND ctrl, RECT* rc) const {
        return getBandRect(getBandIndex(ctrl), rc);
    }

    bool getBandRect(int index, RECT* rc) const {
        REBARBANDINFO rbbi;
        rbbi.cbSize = sizeof(REBARBANDINFO);
        rbbi.fMask = RBBIM_HEADERSIZE;
        if (toolBar.GetBandInfo(index, &rbbi) != TRUE) return false;
        if (toolBar.GetRect(index, rc) != TRUE) return false;
        rc->left += rbbi.cxHeader;
        return true;
    }

	// replace atlctrls' broken lockbands
	void LockBands(bool bLock) {
		int nBandCount = toolBar.GetBandCount();
		for (int i =0; i < nBandCount; ++i) {
			REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO() };
			rbbi.fMask = RBBIM_STYLE;
			BOOL bRet = toolBar.GetBandInfo(i, &rbbi);
			ATLASSERT(bRet);

			if (bLock) {
				rbbi.fStyle &= ~RBBS_GRIPPERALWAYS;
				rbbi.fStyle |= RBBS_NOGRIPPER;
			} else {
				rbbi.fStyle &= ~RBBS_NOGRIPPER;
				rbbi.fStyle |= RBBS_GRIPPERALWAYS;
			}

			bRet = toolBar.SetBandInfo(i, &rbbi);
			ATLASSERT(bRet);
		}
	}


	// duplicated from atlframe

	struct _AtlToolBarData
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;
		//WORD aItems[wItemCount]

		WORD* items()
			{ return (WORD*)(this+1); }
	};

	static HWND CreateSimpleToolBarCtrl(HWND hWndParent, UINT nResourceID, BOOL bInitialSeparator = FALSE, 
			DWORD dwStyle = ATL_SIMPLE_TOOLBAR_STYLE, UINT nID = ATL_IDW_TOOLBAR)
	{
		HINSTANCE hInst = ModuleHelper::GetResourceInstance();
		HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nResourceID), RT_TOOLBAR);
		if (hRsrc == NULL)
			return NULL;

		HGLOBAL hGlobal = ::LoadResource(hInst, hRsrc);
		if (hGlobal == NULL)
			return NULL;

		_AtlToolBarData* pData = (_AtlToolBarData*)::LockResource(hGlobal);
		if (pData == NULL)
			return NULL;
		ATLASSERT(pData->wVersion == 1);

		WORD* pItems = pData->items();
		int nItems = pData->wItemCount + (bInitialSeparator ? 1 : 0);
		CTempBuffer<TBBUTTON, _WTL_STACK_ALLOC_THRESHOLD> buff;
		TBBUTTON* pTBBtn = buff.Allocate(nItems);
		ATLASSERT(pTBBtn != NULL);
		if(pTBBtn == NULL)
			return NULL;

		const int cxSeparator = 8;

		// set initial separator (half width)
		if(bInitialSeparator)
		{
			pTBBtn[0].iBitmap = cxSeparator / 2;
			pTBBtn[0].idCommand = 0;
			pTBBtn[0].fsState = 0;
			pTBBtn[0].fsStyle = TBSTYLE_SEP;
			pTBBtn[0].dwData = 0;
			pTBBtn[0].iString = 0;
		}

		int nBmp = 0;
		for(int i = 0, j = bInitialSeparator ? 1 : 0; i < pData->wItemCount; i++, j++)
		{
			if(pItems[i] != 0)
			{
				pTBBtn[j].iBitmap = nBmp++;
				pTBBtn[j].idCommand = pItems[i];
				pTBBtn[j].fsState = TBSTATE_ENABLED;
				pTBBtn[j].fsStyle = TBSTYLE_BUTTON;
				pTBBtn[j].dwData = 0;
				pTBBtn[j].iString = 0;
			}
			else
			{
				pTBBtn[j].iBitmap = cxSeparator;
				pTBBtn[j].idCommand = 0;
				pTBBtn[j].fsState = 0;
				pTBBtn[j].fsStyle = TBSTYLE_SEP;
				pTBBtn[j].dwData = 0;
				pTBBtn[j].iString = 0;
			}
		}

#ifndef _WIN32_WCE
		HWND hWnd = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, dwStyle, 0, 0, 100, 100, hWndParent, (HMENU)LongToHandle(nID), ModuleHelper::GetModuleInstance(), NULL);
		if(hWnd == NULL)
		{
			ATLASSERT(FALSE);
			return NULL;
		}
#else // CE specific
		dwStyle;
		nID;
		// The toolbar must go onto the existing CommandBar or MenuBar
		HWND hWnd = hWndParent;
#endif // _WIN32_WCE

		::SendMessage(hWnd, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0L);

		// check if font is taller than our bitmaps
		CFontHandle font = (HFONT)::SendMessage(hWnd, WM_GETFONT, 0, 0L);
		if(font.IsNull())
			font = AtlGetDefaultGuiFont();
		LOGFONT lf = { 0 };
		font.GetLogFont(lf);
		WORD cyFontHeight = (WORD)abs(lf.lfHeight);

#ifndef _WIN32_WCE
		WORD bitsPerPixel = AtlGetBitmapResourceBitsPerPixel(nResourceID);
		if(bitsPerPixel > 4)
		{
			COLORREF crMask = CLR_DEFAULT;
			if(bitsPerPixel == 32)
			{
				// 32-bit color bitmap with alpha channel (valid for Windows XP and later)
				crMask = CLR_NONE;
			}
			HIMAGELIST hImageList = ImageList_LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(nResourceID), pData->wWidth, 1, crMask, IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
			ATLASSERT(hImageList != NULL);
			::SendMessage(hWnd, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
		}
		else
#endif // !_WIN32_WCE
		{
			TBADDBITMAP tbab = { 0 };
			tbab.hInst = hInst;
			tbab.nID = nResourceID;
			::SendMessage(hWnd, TB_ADDBITMAP, nBmp, (LPARAM)&tbab);
		}

		::SendMessage(hWnd, TB_ADDBUTTONS, nItems, (LPARAM)pTBBtn);
		::SendMessage(hWnd, TB_SETBITMAPSIZE, 0, MAKELONG(pData->wWidth, max(pData->wHeight, cyFontHeight)));
		const int cxyButtonMargin = 7;
		::SendMessage(hWnd, TB_SETBUTTONSIZE, 0, MAKELONG(pData->wWidth + cxyButtonMargin, max(pData->wHeight, cyFontHeight) + cxyButtonMargin));

		return hWnd;
	}

};
