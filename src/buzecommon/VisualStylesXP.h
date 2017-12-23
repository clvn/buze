/* .-----------------------------------.
   | XNamedColors                      |
   | 2009 Update by Megz               |
   | Original release notes are below. |
   '-----------------------------------' */

/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2001-2002 by David Yuheng Zhao
//
// Distribute and change freely, except: don't remove my name from the source 
//
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Partly based on the _ThemeHelper struct in MFC7.0 source code (winctrl3.cpp), 
// and the difference is that this implementation wraps the full set of 
// visual style APIs from the platform SDK August 2001
//
// If you have any questions, I can be reached as follows:
//	yuheng_zhao@yahoo.com
//
// How to use:
// Instead of calling the API directly, 
//    OpenThemeData(...);
// use the global variable
//    g_xpStyle.OpenThemeData(...);
//
// Date: 2002-07-27
// This update was made by Mathias Tunared. 
// He changed the function variable to static to make the code faster. 
// He also added the function 'UseVisualStyles()' so you can check if to
// draw with the visual styles or not. That function checks first the 
// version of the 'ComCtl32.dll' and the checks if the theme is activated 
// and the HTHEME handle is not NULL.
//
// CAdvComboBox Control
// Version: 2.1
// Date: August 2002
// Author: Mathias Tunared
// Email: Mathias@inorbit.com
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#pragma warning(push, 3)
#include <uxtheme.h>
#if _MSC_VER >= 1500	// vssym32.h on vs2008 or newer
#include <vssym32.h>
#else
#include <tmschema.h>
#endif
#include <Shlwapi.h>

class CVisualStylesXP
{
  private:

	HMODULE m_hThemeDll;

	void* GetProc(LPCSTR szProc, void* pfnFail) {
		void* pRet = pfnFail;
		if (m_hThemeDll != NULL) {
			pRet = ::GetProcAddress(m_hThemeDll, szProc);
		}
		return pRet;
	}

	typedef HTHEME (__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
	static HTHEME OpenThemeDataFail(HWND, LPCWSTR) { return NULL; }

	typedef HRESULT (__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
	static HRESULT CloseThemeDataFail(HTHEME) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect);
	static HRESULT DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT*, const RECT*) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT* pRect);
	static HRESULT DrawThemeTextFail(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, const RECT*) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDCONTENTRECT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pBoundingRect, RECT* pContentRect);
	static HRESULT GetThemeBackgroundContentRectFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pBoundingRect, RECT* pContentRect) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDEXTENT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pContentRect, RECT* pExtentRect);
	static HRESULT GetThemeBackgroundExtentFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pContentRect, RECT* pExtentRect) { return E_FAIL; }

	typedef HRESULT(__stdcall *PFNGETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, RECT* pRect, enum THEMESIZE eSize, SIZE* psz);
	static HRESULT GetThemePartSizeFail(HTHEME, HDC, int, int, RECT*, enum THEMESIZE, SIZE*) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMETEXTEXTENT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, const RECT* pBoundingRect, RECT* pExtentRect);
	static HRESULT GetThemeTextExtentFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, const RECT* pBoundingRect, RECT* pExtentRect) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMETEXTMETRICS)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, TEXTMETRIC* ptm);
	static HRESULT GetThemeTextMetricsFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, TEXTMETRIC* ptm) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEBACKGROUNDREGION)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, HRGN* pRegion);
	static HRESULT GetThemeBackgroundRegionFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, HRGN* pRegion) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNHITTESTTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, DWORD dwOptions, const RECT* pRect, HRGN hrgn, POINT ptTest, WORD* pwHitTestCode);
	static HRESULT HitTestThemeBackgroundFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, DWORD dwOptions, const RECT* pRect, HRGN hrgn, POINT ptTest, WORD* pwHitTestCode) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNDRAWTHEMEEDGE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pDestRect, UINT uEdge, UINT uFlags, RECT* pContentRect);
	static HRESULT DrawThemeEdgeFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pDestRect, UINT uEdge, UINT uFlags, RECT* pContentRect) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNDRAWTHEMEICON)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, HIMAGELIST himl, int iImageIndex);
	static HRESULT DrawThemeIconFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, HIMAGELIST himl, int iImageIndex) { return E_FAIL; }

	typedef BOOL (__stdcall *PFNISTHEMEPARTDEFINED)(HTHEME hTheme, int iPartId, int iStateId);
	static BOOL IsThemePartDefinedFail(HTHEME hTheme, int iPartId, int iStateId) { return FALSE; }

	typedef BOOL (__stdcall *PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)(HTHEME hTheme, int iPartId, int iStateId);
	static BOOL IsThemeBackgroundPartiallyTransparentFail(HTHEME hTheme, int iPartId, int iStateId) { return FALSE; }

	typedef HRESULT (__stdcall *PFNGETTHEMECOLOR)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor);
	static HRESULT GetThemeColorFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEMETRIC)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int* piVal);
	static HRESULT GetThemeMetricFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int* piVal) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMESTRING)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPWSTR pszBuff, int cchMaxBuffChars);
	static HRESULT GetThemeStringFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPWSTR pszBuff, int cchMaxBuffChars) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEBOOL)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, BOOL* pfVal);
	static HRESULT GetThemeBoolFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, BOOL* pfVal) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEINT)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int* piVal);
	static HRESULT GetThemeIntFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int* piVal) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEENUMVALUE)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int* piVal);
	static HRESULT GetThemeEnumValueFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int* piVal) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEPOSITION)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT* pPoint);
	static HRESULT GetThemePositionFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT* pPoint) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEFONT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LOGFONT* pFont);
	static HRESULT GetThemeFontFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LOGFONT* pFont) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMERECT)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, RECT* pRect);
	static HRESULT GetThemeRectFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, RECT* pRect) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEMARGINS)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, RECT* prc, MARGINS* pMargins);
	static HRESULT GetThemeMarginsFail(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, RECT* prc, MARGINS* pMargins) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEINTLIST)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, INTLIST* pIntList);
	static HRESULT GetThemeIntListFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, INTLIST* pIntList) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEPROPERTYORIGIN)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, enum PROPERTYORIGIN* pOrigin);
	static HRESULT GetThemePropertyOriginFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, enum PROPERTYORIGIN* pOrigin) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNSETWINDOWTHEME)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
	static HRESULT SetWindowThemeFail(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEFILENAME)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPWSTR pszThemeFileName, int cchMaxBuffChars);
	static HRESULT GetThemeFilenameFail(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPWSTR pszThemeFileName, int cchMaxBuffChars) { return E_FAIL; }

	typedef COLORREF (__stdcall *PFNGETTHEMESYSCOLOR)(HTHEME hTheme, int iColorId);
	static COLORREF GetThemeSysColorFail(HTHEME hTheme, int iColorId) { return RGB(255,255,255); }

	typedef HBRUSH (__stdcall *PFNGETTHEMESYSCOLORBRUSH)(HTHEME hTheme, int iColorId);
	static HBRUSH GetThemeSysColorBrushFail(HTHEME hTheme, int iColorId) { return NULL; }

	typedef BOOL (__stdcall *PFNGETTHEMESYSBOOL)(HTHEME hTheme, int iBoolId);
	static BOOL GetThemeSysBoolFail(HTHEME hTheme, int iBoolId) { return FALSE; }

	typedef int (__stdcall *PFNGETTHEMESYSSIZE)(HTHEME hTheme, int iSizeId);
	static int GetThemeSysSizeFail(HTHEME hTheme, int iSizeId) { return 0; }

	typedef HRESULT (__stdcall *PFNGETTHEMESYSFONT)(HTHEME hTheme, int iFontId, LOGFONT* plf);
	static HRESULT GetThemeSysFontFail(HTHEME hTheme, int iFontId, LOGFONT* plf) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMESYSSTRING)(HTHEME hTheme, int iStringId, LPWSTR pszStringBuff, int cchMaxStringChars);
	static HRESULT GetThemeSysStringFail(HTHEME hTheme, int iStringId, LPWSTR pszStringBuff, int cchMaxStringChars) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMESYSINT)(HTHEME hTheme, int iIntId, int* piValue);
	static HRESULT GetThemeSysIntFail(HTHEME hTheme, int iIntId, int* piValue) { return E_FAIL; }

	typedef BOOL (__stdcall *PFNISTHEMEACTIVE)();
	static BOOL IsThemeActiveFail() { return FALSE; }

	typedef BOOL(__stdcall *PFNISAPPTHEMED)();
	static BOOL IsAppThemedFail() { return FALSE; }

	typedef HTHEME (__stdcall *PFNGETWINDOWTHEME)(HWND hwnd);
	static HTHEME GetWindowThemeFail(HWND hwnd) { return NULL; }

	typedef HRESULT (__stdcall *PFNENABLETHEMEDIALOGTEXTURE)(HWND hwnd, DWORD dwFlags);
	static HRESULT EnableThemeDialogTextureFail(HWND hwnd, DWORD dwFlags) { return E_FAIL; }

	typedef BOOL (__stdcall *PFNISTHEMEDIALOGTEXTUREENABLED)(HWND hwnd);
	static BOOL IsThemeDialogTextureEnabledFail(HWND hwnd) { return FALSE; }

	typedef DWORD (__stdcall *PFNGETTHEMEAPPPROPERTIES)();
	static DWORD GetThemeAppPropertiesFail() { return 0; }

	typedef void (__stdcall *PFNSETTHEMEAPPPROPERTIES)(DWORD dwFlags);
	static void SetThemeAppPropertiesFail(DWORD dwFlags) { return; }

	typedef HRESULT (__stdcall *PFNGETCURRENTTHEMENAME)(LPWSTR pszThemeFileName, int cchMaxNameChars, LPWSTR pszColorBuff, int cchMaxColorChars, LPWSTR pszSizeBuff, int cchMaxSizeChars);
	static HRESULT GetCurrentThemeNameFail(LPWSTR pszThemeFileName, int cchMaxNameChars, LPWSTR pszColorBuff, int cchMaxColorChars, LPWSTR pszSizeBuff, int cchMaxSizeChars) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNGETTHEMEDOCUMENTATIONPROPERTY)(LPCWSTR pszThemeName, LPCWSTR pszPropertyName, LPWSTR pszValueBuff, int cchMaxValChars);
	static HRESULT GetThemeDocumentationPropertyFail(LPCWSTR pszThemeName, LPCWSTR pszPropertyName, LPWSTR pszValueBuff, int cchMaxValChars) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNDRAWTHEMEPARENTBACKGROUND)(HWND hwnd, HDC hdc, RECT* prc);
	static HRESULT DrawThemeParentBackgroundFail(HWND hwnd, HDC hdc, RECT* prc) { return E_FAIL; }

	typedef HRESULT (__stdcall *PFNENABLETHEMING)(BOOL fEnable);
	static HRESULT EnableThemingFail(BOOL fEnable) { return E_FAIL; }

  public:

	CVisualStylesXP() {
		m_hThemeDll = ::LoadLibrary(_T("UxTheme.dll"));
	}

	~CVisualStylesXP() {
		if (m_hThemeDll != NULL) {
		  ::FreeLibrary(m_hThemeDll);
		}
		m_hThemeDll = NULL;
	}

	HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList) {
		static PFNOPENTHEMEDATA pfn = NULL;
		if (!pfn) pfn = (PFNOPENTHEMEDATA)GetProc("OpenThemeData", (void*)OpenThemeDataFail);
		return (*pfn)(hwnd, pszClassList);
	}

	HRESULT CloseThemeData(HTHEME hTheme) {
		static PFNCLOSETHEMEDATA pfn = NULL;
		if (!pfn) pfn = (PFNCLOSETHEMEDATA)GetProc("CloseThemeData", (void*)CloseThemeDataFail);
		return (*pfn)(hTheme);
	}

	HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect) {
		static PFNDRAWTHEMEBACKGROUND pfn = NULL;
		if (!pfn) pfn = (PFNDRAWTHEMEBACKGROUND)GetProc("DrawThemeBackground", (void*)DrawThemeBackgroundFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
	}

	HRESULT DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT* pRect) {
		static PFNDRAWTHEMETEXT pfn = NULL;
		if (!pfn) pfn = (PFNDRAWTHEMETEXT)GetProc("DrawThemeText", (void*)DrawThemeTextFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
	}

	HRESULT GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pBoundingRect, RECT* pContentRect) {
		static PFNGETTHEMEBACKGROUNDCONTENTRECT pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEBACKGROUNDCONTENTRECT)GetProc("GetThemeBackgroundContentRect", (void*)GetThemeBackgroundContentRectFail);
		return (*pfn)(hTheme,  hdc, iPartId, iStateId,  pBoundingRect, pContentRect);
	}

	HRESULT GetThemeBackgroundExtent(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pContentRect, RECT* pExtentRect) {
		static PFNGETTHEMEBACKGROUNDEXTENT pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEBACKGROUNDEXTENT)GetProc("GetThemeBackgroundExtent", (void*)GetThemeBackgroundExtentFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, pContentRect, pExtentRect);
	}

	HRESULT GetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, RECT* pRect, enum THEMESIZE eSize, SIZE* psz) {
		static PFNGETTHEMEPARTSIZE pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEPARTSIZE)GetProc("GetThemePartSize", (void*)GetThemePartSizeFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, pRect, eSize, psz);
	}

	HRESULT GetThemeTextExtent(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, const RECT* pBoundingRect, RECT* pExtentRect) {
		static PFNGETTHEMETEXTEXTENT pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMETEXTEXTENT)GetProc("GetThemeTextExtent", (void*)GetThemeTextExtentFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, pBoundingRect, pExtentRect);
	}

	HRESULT GetThemeTextMetrics(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, TEXTMETRIC* ptm) {
		static PFNGETTHEMETEXTMETRICS pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMETEXTMETRICS)GetProc("GetThemeTextMetrics", (void*)GetThemeTextMetricsFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, ptm);
	}

	HRESULT GetThemeBackgroundRegion(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, HRGN* pRegion) {
		static PFNGETTHEMEBACKGROUNDREGION pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEBACKGROUNDREGION)GetProc("GetThemeBackgroundRegion", (void*)GetThemeBackgroundRegionFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, pRect, pRegion);
	}

	HRESULT HitTestThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, DWORD dwOptions, const RECT* pRect, HRGN hrgn, POINT ptTest, WORD* pwHitTestCode) {
		static PFNHITTESTTHEMEBACKGROUND pfn = NULL;
		if (!pfn) pfn = (PFNHITTESTTHEMEBACKGROUND)GetProc("HitTestThemeBackground", (void*)HitTestThemeBackgroundFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, dwOptions, pRect, hrgn, ptTest, pwHitTestCode);
	}

	HRESULT DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pDestRect, UINT uEdge, UINT uFlags, RECT* pContentRect) {
		static PFNDRAWTHEMEEDGE pfn = NULL;
		if (!pfn) pfn = (PFNDRAWTHEMEEDGE)GetProc("DrawThemeEdge", (void*)DrawThemeEdgeFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, pDestRect, uEdge, uFlags, pContentRect);
	}

	HRESULT DrawThemeIcon(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, HIMAGELIST himl, int iImageIndex) {
		static PFNDRAWTHEMEICON pfn = NULL;
		if (!pfn) pfn = (PFNDRAWTHEMEICON)GetProc("DrawThemeIcon", (void*)DrawThemeIconFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, pRect, himl, iImageIndex);
	}

	BOOL IsThemePartDefined(HTHEME hTheme, int iPartId, int iStateId) {
		static PFNISTHEMEPARTDEFINED pfn = NULL;
		if (!pfn) pfn = (PFNISTHEMEPARTDEFINED)GetProc("IsThemePartDefined", (void*)IsThemePartDefinedFail);
		return (*pfn)(hTheme, iPartId, iStateId);
	}

	BOOL IsThemeBackgroundPartiallyTransparent(HTHEME hTheme, int iPartId, int iStateId) {
		static PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT pfn = NULL;
		if (!pfn) pfn = (PFNISTHEMEBACKGROUNDPARTIALLYTRANSPARENT)GetProc("IsThemeBackgroundPartiallyTransparent", (void*)IsThemeBackgroundPartiallyTransparentFail);
		return (*pfn)(hTheme, iPartId, iStateId);
	}

	HRESULT GetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor) {
		static PFNGETTHEMECOLOR pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMECOLOR)GetProc("GetThemeColor", (void*)GetThemeColorFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, pColor);
	}

	HRESULT GetThemeMetric(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int* piVal) {
		static PFNGETTHEMEMETRIC pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEMETRIC)GetProc("GetThemeMetric", (void*)GetThemeMetricFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, iPropId, piVal);
	}

	HRESULT GetThemeString(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPWSTR pszBuff, int cchMaxBuffChars) {
		static PFNGETTHEMESTRING pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMESTRING)GetProc("GetThemeString", (void*)GetThemeStringFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, pszBuff, cchMaxBuffChars);
	}

	HRESULT GetThemeBool(HTHEME hTheme, int iPartId, int iStateId, int iPropId, BOOL* pfVal) {
		static PFNGETTHEMEBOOL pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEBOOL)GetProc("GetThemeBool", (void*)GetThemeBoolFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, pfVal);
	}

	HRESULT GetThemeInt(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int* piVal) {
		static PFNGETTHEMEINT pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEINT)GetProc("GetThemeInt", (void*)GetThemeIntFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, piVal);
	}

	HRESULT GetThemeEnumValue(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int* piVal) {
		static PFNGETTHEMEENUMVALUE pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEENUMVALUE)GetProc("GetThemeEnumValue", (void*)GetThemeEnumValueFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, piVal);
	}

	HRESULT GetThemePosition(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT* pPoint) {
		static PFNGETTHEMEPOSITION pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEPOSITION)GetProc("GetThemePosition", (void*)GetThemePositionFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, pPoint);
	}

	HRESULT GetThemeFont(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LOGFONT* pFont) {
		static PFNGETTHEMEFONT pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEFONT)GetProc("GetThemeFont", (void*)GetThemeFontFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, iPropId, pFont);
	}

	HRESULT GetThemeRect(HTHEME hTheme, int iPartId, int iStateId, int iPropId, RECT* pRect) {
		static PFNGETTHEMERECT pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMERECT)GetProc("GetThemeRect", (void*)GetThemeRectFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, pRect);
	}

	HRESULT GetThemeMargins(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, RECT* prc, MARGINS* pMargins) {
		static PFNGETTHEMEMARGINS pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEMARGINS)GetProc("GetThemeMargins", (void*)GetThemeMarginsFail);
		return (*pfn)(hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);
	}

	HRESULT GetThemeIntList(HTHEME hTheme, int iPartId, int iStateId, int iPropId, INTLIST* pIntList) {
		static PFNGETTHEMEINTLIST pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEINTLIST)GetProc("GetThemeIntList", (void*)GetThemeIntListFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, pIntList);
	}

	HRESULT GetThemePropertyOrigin(HTHEME hTheme, int iPartId, int iStateId, int iPropId, enum PROPERTYORIGIN* pOrigin) {
		static PFNGETTHEMEPROPERTYORIGIN pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEPROPERTYORIGIN)GetProc("GetThemePropertyOrigin", (void*)GetThemePropertyOriginFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, pOrigin);
	}

	HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList) {
		static PFNSETWINDOWTHEME pfn = NULL;
		if (!pfn) pfn = (PFNSETWINDOWTHEME)GetProc("SetWindowTheme", (void*)SetWindowThemeFail);
		return (*pfn)(hwnd, pszSubAppName, pszSubIdList);
	}

	HRESULT GetThemeFilename(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPWSTR pszThemeFileName, int cchMaxBuffChars) {
		static PFNGETTHEMEFILENAME pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEFILENAME)GetProc("GetThemeFilename", (void*)GetThemeFilenameFail);
		return (*pfn)(hTheme, iPartId, iStateId, iPropId, pszThemeFileName, cchMaxBuffChars);
	}

	COLORREF GetThemeSysColor(HTHEME hTheme, int iColorId) {
		static PFNGETTHEMESYSCOLOR pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMESYSCOLOR)GetProc("GetThemeSysColor", (void*)GetThemeSysColorFail);
		return (*pfn)(hTheme, iColorId);
	}

	HBRUSH GetThemeSysColorBrush(HTHEME hTheme, int iColorId) {
		static PFNGETTHEMESYSCOLORBRUSH pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMESYSCOLORBRUSH)GetProc("GetThemeSysColorBrush", (void*)GetThemeSysColorBrushFail);
		return (*pfn)(hTheme, iColorId);
	}

	BOOL GetThemeSysBool(HTHEME hTheme, int iBoolId) {
		static PFNGETTHEMESYSBOOL pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMESYSBOOL)GetProc("GetThemeSysBool", (void*)GetThemeSysBoolFail);
		return (*pfn)(hTheme, iBoolId);
	}

	int GetThemeSysSize(HTHEME hTheme, int iSizeId) {
		static PFNGETTHEMESYSSIZE pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMESYSSIZE)GetProc("GetThemeSysSize", (void*)GetThemeSysSizeFail);
		return (*pfn)(hTheme, iSizeId);
	}

	HRESULT GetThemeSysFont(HTHEME hTheme, int iFontId, LOGFONT* plf) {
		static PFNGETTHEMESYSFONT pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMESYSFONT)GetProc("GetThemeSysFont", (void*)GetThemeSysFontFail);
		return (*pfn)(hTheme, iFontId, plf);
	}

	HRESULT GetThemeSysString(HTHEME hTheme, int iStringId, LPWSTR pszStringBuff, int cchMaxStringChars) {
		static PFNGETTHEMESYSSTRING pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMESYSSTRING)GetProc("GetThemeSysString", (void*)GetThemeSysStringFail);
		return (*pfn)(hTheme, iStringId, pszStringBuff, cchMaxStringChars);
	}

	HRESULT GetThemeSysInt(HTHEME hTheme, int iIntId, int* piValue) {
		static PFNGETTHEMESYSINT pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMESYSINT)GetProc("GetThemeSysInt", (void*)GetThemeSysIntFail);
		return (*pfn)(hTheme, iIntId, piValue);
	}

	BOOL IsThemeActive() {
		static PFNISTHEMEACTIVE pfn = NULL;
		if (!pfn) pfn = (PFNISTHEMEACTIVE)GetProc("IsThemeActive", (void*)IsThemeActiveFail);
		return (*pfn)();
	}

	BOOL IsAppThemed() {
		static PFNISAPPTHEMED pfn = NULL;
		if (!pfn) pfn = (PFNISAPPTHEMED)GetProc("IsAppThemed", (void*)IsAppThemedFail);
		return (*pfn)();
	}

	HTHEME GetWindowTheme(HWND hwnd) {
		static PFNGETWINDOWTHEME pfn = NULL;
		if (!pfn) pfn = (PFNGETWINDOWTHEME)GetProc("GetWindowTheme", (void*)GetWindowThemeFail);
		return (*pfn)(hwnd);
	}

	HRESULT EnableThemeDialogTexture(HWND hwnd, DWORD dwFlags) {
		static PFNENABLETHEMEDIALOGTEXTURE pfn = NULL;
		if (!pfn) pfn = (PFNENABLETHEMEDIALOGTEXTURE)GetProc("EnableThemeDialogTexture", (void*)EnableThemeDialogTextureFail);
		return (*pfn)(hwnd, dwFlags);
	}

	BOOL IsThemeDialogTextureEnabled(HWND hwnd) {
		static PFNISTHEMEDIALOGTEXTUREENABLED pfn = NULL;
		if (!pfn) pfn = (PFNISTHEMEDIALOGTEXTUREENABLED)GetProc("IsThemeDialogTextureEnabled", (void*)IsThemeDialogTextureEnabledFail);
		return (*pfn)(hwnd);
	}

	DWORD GetThemeAppProperties() {
		static PFNGETTHEMEAPPPROPERTIES pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEAPPPROPERTIES)GetProc("GetThemeAppProperties", (void*)GetThemeAppPropertiesFail);
		return (*pfn)();
	}

	void SetThemeAppProperties(DWORD dwFlags) {
		static PFNSETTHEMEAPPPROPERTIES pfn = NULL;
		if (!pfn) pfn = (PFNSETTHEMEAPPPROPERTIES)GetProc("SetThemeAppProperties", (void*)SetThemeAppPropertiesFail);
		(*pfn)(dwFlags);
	}

	HRESULT GetCurrentThemeName(LPWSTR pszThemeFileName, int cchMaxNameChars, LPWSTR pszColorBuff, int cchMaxColorChars, LPWSTR pszSizeBuff, int cchMaxSizeChars) {
		static PFNGETCURRENTTHEMENAME pfn = NULL;
		if (!pfn) pfn = (PFNGETCURRENTTHEMENAME)GetProc("GetCurrentThemeName", (void*)GetCurrentThemeNameFail);
		return (*pfn)(pszThemeFileName, cchMaxNameChars, pszColorBuff, cchMaxColorChars, pszSizeBuff, cchMaxSizeChars);
	}

	HRESULT GetThemeDocumentationProperty(LPCWSTR pszThemeName, LPCWSTR pszPropertyName, LPWSTR pszValueBuff, int cchMaxValChars) {
		static PFNGETTHEMEDOCUMENTATIONPROPERTY pfn = NULL;
		if (!pfn) pfn = (PFNGETTHEMEDOCUMENTATIONPROPERTY)GetProc("GetThemeDocumentationProperty", (void*)GetThemeDocumentationPropertyFail);
		return (*pfn)(pszThemeName, pszPropertyName, pszValueBuff, cchMaxValChars);
	}

	HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT* prc) {
		static PFNDRAWTHEMEPARENTBACKGROUND pfn = NULL;
		if (!pfn) pfn = (PFNDRAWTHEMEPARENTBACKGROUND)GetProc("DrawThemeParentBackground", (void*)DrawThemeParentBackgroundFail);
		return (*pfn)(hwnd, hdc, prc);
	}

	HRESULT EnableTheming(BOOL fEnable) {
		static PFNENABLETHEMING pfn = NULL;
		if (!pfn) pfn = (PFNENABLETHEMING)GetProc("EnableTheming", (void*)EnableThemingFail);
		return (*pfn)(fEnable);
	}

	BOOL UseVisualStyles() {
		static BOOL bRet = -1;
		if (bRet != -1) {
			return bRet;
		} else {
			bRet = FALSE;
		}

		if (m_hThemeDll != NULL) {
			if (IsAppThemed() && IsThemeActive()) {
				HMODULE hModComCtl = ::LoadLibrary(_T("comctl32.dll"));

				if (hModComCtl != NULL) {
					DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hModComCtl, _T("DllGetVersion"));

					if (pDllGetVersion != NULL) {
						DLLVERSIONINFO dvi = { 0 };
						dvi.cbSize = sizeof(DLLVERSIONINFO);

						if (pDllGetVersion(&dvi) == NOERROR) {
							bRet = dvi.dwMajorVersion >= 6;
						}
					}

					::FreeLibrary(hModComCtl);                    
				}
			}
		}

		return bRet;
	}
};

extern CVisualStylesXP g_xpStyle;

#pragma warning(pop)
