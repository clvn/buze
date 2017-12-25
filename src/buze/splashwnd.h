#pragma once

#include <wtl/atlmisc.h>

class CSplashWnd :
	public CWindowImpl<CSplashWnd, CWindow, CWinTraits<WS_POPUP|WS_VISIBLE, WS_EX_TOOLWINDOW> >
{
private:
	enum
	{
		DEF_TIMER_ID		= 1001,
		DEF_TIMER_ELAPSE	= 2500,
	};
private:
	CBitmap m_bmp;
	int m_nTimeout;
	HWND m_hParent;
	COLORREF cColor;
	bool m_bQuitOnClose;
public:
	CSplashWnd(UINT nBitmapID, int nTimeout = DEF_TIMER_ELAPSE, HWND hParent = NULL, COLORREF cTransparentColor = NULL, bool bQuitOnClose = false)
		: m_nTimeout(nTimeout)
		, m_hParent(hParent)
		, cColor(cTransparentColor)
		, m_bQuitOnClose(bQuitOnClose)
	{
		// Load the bitmap
		if (!m_bmp.LoadBitmap(nBitmapID))
		{
			ATLTRACE(_T("Failed to load spash bitmap\n"));
			return;
		}
		// Get the bitmap size
		CSize size;
		if (!m_bmp.GetSize(size))
		{
			ATLTRACE(_T("Failed to get spash bitmap size\n"));
			return;
		}
		// Create the window rect (we will centre the window later)
		CRect rect(0, 0, size.cx, size.cy);
		// Create the window
		if (!Create(NULL, rect))
		{
			ATLTRACE(_T("Failed to create splash window\n"));
			return;
		}
		UpdateWindow();
	}

	/// Called when the window is destroyed
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		if (m_bQuitOnClose) PostQuitMessage(0);
		delete this;
	}

	BEGIN_MSG_MAP(CSplashWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnButtonDown)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnButtonDown)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()
	
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CenterWindow(m_hParent);
		// Set the timer
		SetTimer(DEF_TIMER_ID, m_nTimeout);
		return 0;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// Draw the bmp
		CPaintDC dc(m_hWnd);
		
		CDC dcImage;
		if (dcImage.CreateCompatibleDC(dc.m_hDC))
		{
			CSize size;
			if (m_bmp.GetSize(size))
			{
				if(cColor == NULL)
				{
					// No Transparency bitmaps - Original splashwnd 
					HBITMAP hBmpOld = dcImage.SelectBitmap(m_bmp);
					dc.BitBlt(0, 0, size.cx, size.cy, dcImage, 0, 0, SRCCOPY);
					dcImage.SelectBitmap(hBmpOld);
				} else {
					DrawTransparentBitmap(dc, m_bmp, 0, 0, cColor);
				}
			}
		}

		return 0;
	}

	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		KillTimer(DEF_TIMER_ID);
		PostMessage(WM_CLOSE);
		return 0;
	}
	
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// No need to paint a background
		return TRUE;
	}

	LRESULT OnButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (wParam == VK_ESCAPE)
			PostMessage(WM_CLOSE);
		return 0;
	}

	void DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, short xStart, short yStart, COLORREF cTransparentColor) {
		BITMAP     bm;
		COLORREF   cColor;
		HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
		HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
		HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
		POINT      ptSize;

		hdcTemp = CreateCompatibleDC(hdc);
		SelectObject(hdcTemp, hBitmap);   // Select the bitmap

		GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
		ptSize.x = bm.bmWidth;            // Get width of bitmap
		ptSize.y = bm.bmHeight;           // Get height of bitmap
		DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device

											// to logical points

		// Create some DCs to hold temporary data.
		hdcBack   = CreateCompatibleDC(hdc);
		hdcObject = CreateCompatibleDC(hdc);
		hdcMem    = CreateCompatibleDC(hdc);
		hdcSave   = CreateCompatibleDC(hdc);

		// Create a bitmap for each DC. DCs are required for a number of
		// GDI functions.

		// Monochrome DC
		bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

		// Monochrome DC
		bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

		bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
		bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

		// Each DC must select a bitmap object to store pixel data.
		bmBackOld   = (HBITMAP)SelectObject(hdcBack, bmAndBack);
		bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
		bmMemOld    = (HBITMAP)SelectObject(hdcMem, bmAndMem);
		bmSaveOld   = (HBITMAP)SelectObject(hdcSave, bmSave);

		// Set proper mapping mode.
		SetMapMode(hdcTemp, GetMapMode(hdc));

		// Save the bitmap sent here, because it will be overwritten.
		BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

		// Set the background color of the source DC to the color.
		// contained in the parts of the bitmap that should be transparent
		cColor = SetBkColor(hdcTemp, cTransparentColor);

		// Create the object mask for the bitmap by performing a BitBlt
		// from the source bitmap to a monochrome bitmap.
		BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
				SRCCOPY);

		// Set the background color of the source DC back to the original
		// color.
		SetBkColor(hdcTemp, cColor);

		// Create the inverse of the object mask.
		BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
				NOTSRCCOPY);

		// Copy the background of the main DC to the destination.
		BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
				SRCCOPY);

		// Mask out the places where the bitmap will be placed.
		BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

		// Mask out the transparent colored pixels on the bitmap.
		BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

		// XOR the bitmap with the background on the destination DC.
		BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

		// Copy the destination to the screen.
		BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
				SRCCOPY);

		// Place the original bitmap back into the bitmap sent here.
		BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

		// Delete the memory bitmaps.
		DeleteObject(SelectObject(hdcBack, bmBackOld));
		DeleteObject(SelectObject(hdcObject, bmObjectOld));
		DeleteObject(SelectObject(hdcMem, bmMemOld));
		DeleteObject(SelectObject(hdcSave, bmSaveOld));

		// Delete the memory DCs.
		DeleteDC(hdcMem);
		DeleteDC(hdcBack);
		DeleteDC(hdcObject);
		DeleteDC(hdcSave);
		DeleteDC(hdcTemp);
	}

};
