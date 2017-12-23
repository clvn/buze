#ifndef __PROPERTYITEMIMPL__H
#define __PROPERTYITEMIMPL__H

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CPropertyItemImpl - Property implementations for the Property controls
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001-2002 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef __PROPERTYITEM__H
  #error PropertyItemImpl.h requires PropertyItem.h to be included first
#endif

#ifndef __PROPERTYITEMEDITORS__H
  #error PropertyItemImpl.h requires PropertyItemEditors.h to be included first
#endif

#ifndef __ATLBASE_H__
  #error PropertyItem.h requires atlbase.h to be included first
#endif



/////////////////////////////////////////////////////////////////////////////
// Base CProperty class

class CProperty : public IProperty
{
protected:
   HWND   m_hWndOwner;
   LPTSTR m_pszName;
   bool   m_fEnabled;
   LPARAM m_lParam;

public:
   CProperty(LPCTSTR pstrName, LPARAM lParam) : m_fEnabled(true), m_lParam(lParam), m_hWndOwner(NULL)
   {
      ATLASSERT(!::IsBadStringPtr(pstrName, -1));
      ATLTRY(m_pszName = new TCHAR[ (::lstrlen(pstrName) * sizeof(TCHAR)) + 1 ]);
      ATLASSERT(m_pszName);
      ::lstrcpy(m_pszName, pstrName);
   }
   virtual ~CProperty()
   {
      delete [] m_pszName;
   }
   virtual void SetOwner(HWND hWnd, LPVOID /*pData*/)
   {
      ATLASSERT(::IsWindow(hWnd));
      ATLASSERT(m_hWndOwner==NULL); // Cannot set it twice
      m_hWndOwner = hWnd;
   }
   virtual LPCTSTR GetName() const
   {
      return m_pszName; // Dangerous!
   }
   virtual void SetEnabled(BOOL bEnable)
   {
      m_fEnabled = bEnable == TRUE;
   }
   virtual BOOL IsEnabled() const
   {
      return m_fEnabled;
   }
   virtual void SetItemData(LPARAM lParam)
   {
      m_lParam = lParam;
   }
   virtual LPARAM GetItemData() const
   {
      return m_lParam;
   }
   virtual void DrawName(PROPERTYDRAWINFO& di)
   {
      CDCHandle dc(di.hDC);
      COLORREF clrBack, clrFront;
      if( di.state & ODS_DISABLED ) {
         clrFront = di.clrDisabled;
         clrBack = di.clrBack;
      }
      else if( di.state & ODS_SELECTED ) {
         clrFront = di.clrSelText;
         clrBack = di.clrSelBack;
      }
      else {
         clrFront = di.clrText;
         clrBack = di.clrBack;
      }
      RECT rcItem = di.rcItem;
      dc.FillSolidRect(&rcItem, clrBack);
      rcItem.left += 2; // Indent text
      dc.SetBkMode(TRANSPARENT);
      dc.SetBkColor(clrBack);
      dc.SetTextColor(clrFront);
      dc.DrawText(m_pszName, -1, &rcItem, DT_LEFT | DT_SINGLELINE | DT_EDITCONTROL | DT_NOPREFIX | DT_VCENTER);
   }
   virtual void DrawValue(PROPERTYDRAWINFO& /*di*/) 
   { 
   }
   virtual HWND CreateInplaceControl(HWND /*hWnd*/, const RECT& /*rc*/) 
   { 
      return NULL; 
   }
   virtual BOOL Activate(UINT /*action*/, LPARAM /*lParam*/) 
   { 
      return TRUE; 
   }
   virtual BOOL GetDisplayValue(LPTSTR /*pstr*/, UINT /*cchMax*/) const 
   { 
      return FALSE; 
   }
   virtual UINT GetDisplayValueLength() const 
   { 
      return 0; 
   }
   virtual BOOL GetValue(VARIANT* /*pValue*/) const 
   { 
      return FALSE; 
   }
   virtual BOOL SetValue(const VARIANT& /*value*/) 
   { 
      ATLASSERT(false);
      return FALSE; 
   }
   virtual BOOL SetValue(HWND /*hWnd*/) 
   { 
      ATLASSERT(false);
      return FALSE; 
   }
};


/////////////////////////////////////////////////////////////////////////////
// Simple property (displays text)

class CPropertyItem : public CProperty
{
protected:
   CComVariant m_val;

public:
   CPropertyItem(LPCTSTR pstrName, LPARAM lParam) : CProperty(pstrName, lParam)
   {
   }
   BYTE GetKind() const 
   { 
      return PROPKIND_SIMPLE; 
   }
   void DrawValue(PROPERTYDRAWINFO& di)
   {
      UINT cchMax = GetDisplayValueLength() + 1;
      LPTSTR pszText = (LPTSTR) _alloca(cchMax * sizeof(TCHAR));
      ATLASSERT(pszText);
      if( !GetDisplayValue(pszText, cchMax) ) return;
      CDCHandle dc(di.hDC);
      dc.SetBkMode(TRANSPARENT);
      dc.SetTextColor(di.state & ODS_DISABLED ? di.clrDisabled : di.clrText);
      dc.SetBkColor(di.clrBack);
      RECT rcText = di.rcItem;
      rcText.left += PROP_TEXT_INDENT;
      dc.DrawText(pszText, -1, 
         &rcText, 
         DT_LEFT | DT_SINGLELINE | DT_EDITCONTROL | DT_NOPREFIX | DT_END_ELLIPSIS | DT_VCENTER);
   }
   BOOL GetDisplayValue(LPTSTR pstr, UINT cchMax) const
   {
      ATLASSERT(!::IsBadStringPtr(pstr, cchMax));
      // Convert VARIANT to string and use that as display string...
      CComVariant v;
      if( FAILED( v.ChangeType(VT_BSTR, &m_val) ) ) return FALSE;
      USES_CONVERSION;
      ::lstrcpyn(pstr, OLE2CT(v.bstrVal), cchMax);
      return TRUE;
   }
   UINT GetDisplayValueLength() const
   {
      // Hmm, need to convert it to display string first and
      // then take the length...
      // TODO: Call GetDisplayValue() instead...
      CComVariant v;
      if( FAILED( v.ChangeType(VT_BSTR, &m_val) ) ) return 0;
      USES_CONVERSION;
      return v.bstrVal == NULL ? 0 : ::lstrlen(OLE2CT(v.bstrVal));
   }
   BOOL GetValue(VARIANT* pVal) const
   {
      return SUCCEEDED( CComVariant(m_val).Detach(pVal) );
   }
   BOOL SetValue(const VARIANT& value)
   {
      m_val = value;
      return TRUE;
   }
};


/////////////////////////////////////////////////////////////////////////////
// ReadOnly property (enhanced display feature)

class CPropertyReadOnlyItem : public CPropertyItem
{
protected:
   UINT m_uStyle;
   HICON m_hIcon;
   COLORREF m_clrBack;
   COLORREF m_clrText;

public:
   CPropertyReadOnlyItem(LPCTSTR pstrName, LPARAM lParam) : 
      CPropertyItem(pstrName, lParam), 
      m_uStyle( DT_LEFT | DT_SINGLELINE | DT_EDITCONTROL | DT_NOPREFIX | DT_END_ELLIPSIS | DT_VCENTER ),
      m_clrBack( (COLORREF) -1 ),
      m_clrText( (COLORREF) -1 ),
      m_hIcon(NULL)
   {
   }
   void DrawValue(PROPERTYDRAWINFO& di)
   {
      // Get property text
      UINT cchMax = GetDisplayValueLength() + 1;
      LPTSTR pszText = (LPTSTR) _alloca(cchMax * sizeof(TCHAR));
      ATLASSERT(pszText);
      if( !GetDisplayValue(pszText, cchMax) ) return;
      // Prepare paint
      RECT rcText = di.rcItem;
      CDCHandle dc(di.hDC);
      dc.SetBkMode(OPAQUE);
      // Set background color
      COLORREF clrBack = di.clrBack;
      if( m_clrBack != (COLORREF) -1 ) clrBack = m_clrBack;
      dc.SetBkColor(clrBack);
      // Set text color
      COLORREF clrText = di.clrText;
      if( m_clrText != (COLORREF) -1 ) clrText = m_clrText;
      if( di.state & ODS_DISABLED ) clrText = di.clrDisabled; 
      dc.SetTextColor(clrText);

      // Draw icon if available
      if( m_hIcon ) {
         POINT pt = { rcText.left + 2, rcText.top + 2 };
         SIZE sz = { ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON) };
         ::DrawIconEx(dc, pt.x, pt.y, m_hIcon, sz.cx, sz.cy, 0, NULL, DI_NORMAL);
         rcText.left += sz.cx + 4;
      }
      // Draw text with custom style
      rcText.left += PROP_TEXT_INDENT;
      dc.DrawText(pszText, -1, 
         &rcText, 
         m_uStyle);
   }

   // Operations

   // NOTE: To use these methods, you must cast the HPROPERTY 
   //       handle back to the CPropertyReadOnlyItem class.
   //       Nasty stuff, but so far I've settled with this approach.

   COLORREF SetBkColor(COLORREF clrBack)
   {
      COLORREF clrOld = m_clrBack;
      m_clrBack = clrBack;
      return clrOld;
   }
   COLORREF SetTextColor(COLORREF clrText)
   {
      COLORREF clrOld = m_clrText;
      m_clrText = clrText;
      return clrOld;
   }
   HICON SetIcon(HICON hIcon)
   {
      HICON hOldIcon = m_hIcon;
      m_hIcon = hIcon;
      return hOldIcon;
   }
   void ModifyDrawStyle(UINT uRemove, UINT uAdd)
   {
      m_uStyle = (m_uStyle & ~uRemove) | uAdd;
   }
};


/////////////////////////////////////////////////////////////////////////////
// Simple Value property

class CPropertyEditItem : public CPropertyItem
{
protected:
   POINT m_ptClick;
   HWND m_hwndEdit;

public:
   CPropertyEditItem(LPCTSTR pstrName, LPARAM lParam) : 
      CPropertyItem(pstrName, lParam), 
      m_hwndEdit(NULL)
   {
   }
   BYTE GetKind() const 
   { 
      return PROPKIND_EDIT; 
   }
   HWND CreateInplaceControl(HWND hWnd, const RECT& rc) 
   {
      // Get default text
      UINT cchMax = GetDisplayValueLength() + 1;
      LPTSTR pszText = (LPTSTR) _alloca(cchMax * sizeof(TCHAR));
      ATLASSERT(pszText);
      if( !GetDisplayValue(pszText, cchMax) ) return NULL;
      // Create EDIT control
      CPropertyEditWindow* win = new CPropertyEditWindow();
      ATLASSERT(win);
      RECT rcWin = rc;
      m_hwndEdit = win->Create(hWnd, rcWin, pszText, WS_VISIBLE | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL);
      ATLASSERT(::IsWindow(m_hwndEdit));
      // Simple hack to validate numbers
      switch( m_val.vt ) {
      case VT_UI1:
      case VT_UI2:
      case VT_UI4:
         win->ModifyStyle(0, ES_NUMBER);
         break;
      }
	  if (m_ptClick.x != -1) {
		  ClientToScreen(m_hWndOwner, &m_ptClick);
		  ScreenToClient(m_hwndEdit, &m_ptClick);
		  if (m_ptClick.x >= 0) {
			int clickpos = (int)::SendMessage(m_hwndEdit, EM_CHARFROMPOS, 0, m_ptClick.x);
			::SetFocus(m_hwndEdit);
			::SendMessage(m_hwndEdit, EM_SETSEL, clickpos, clickpos);
		  }

	  }
	  m_ptClick.x = m_ptClick.y = -1;
      return m_hwndEdit;
   }
   BOOL SetValue(const VARIANT& value)
   {
      if( m_val.vt == VT_EMPTY ) m_val = value;
      return SUCCEEDED( m_val.ChangeType(m_val.vt, &value) );
   }
   BOOL SetValue(HWND hWnd) 
   { 
      ATLASSERT(::IsWindow(hWnd));
      int len = ::GetWindowTextLength(hWnd) + 1;
      LPTSTR pstr = (LPTSTR) _alloca(len * sizeof(TCHAR));
      ATLASSERT(pstr);
      if( ::GetWindowText(hWnd, pstr, len) == 0 ) {
         // Bah, an empty string AND an error causes the same return code!
         if( ::GetLastError() != 0 ) return FALSE;
      }
      CComVariant v = pstr;
      return SetValue(v);
   }
   BOOL Activate(UINT action, LPARAM lParam)
   {
      switch( action ) {
      case PACT_TAB:
      case PACT_SPACE:
      case PACT_DBLCLICK:
         m_ptClick.x = m_ptClick.y = -1;
         if( ::IsWindow(m_hwndEdit) ) {
            ::SetFocus(m_hwndEdit);
            ::SendMessage(m_hwndEdit, EM_SETSEL, 0, -1);
         }
         break;
	  case PACT_CLICK:
        m_ptClick.x = GET_X_LPARAM(lParam);
		m_ptClick.y = GET_Y_LPARAM(lParam);
		break;
      }
      return TRUE;
   }
};


/////////////////////////////////////////////////////////////////////////////
// Simple Value property

class CPropertyDateItem : public CPropertyEditItem
{
public:
   CPropertyDateItem(LPCTSTR pstrName, LPARAM lParam) : 
      CPropertyEditItem(pstrName, lParam)
   {
   }
   HWND CreateInplaceControl(HWND hWnd, const RECT& rc) 
   {
      // Get default text
      UINT cchMax = GetDisplayValueLength() + 1;
      LPTSTR pszText = (LPTSTR) _alloca(cchMax * sizeof(TCHAR));
      ATLASSERT(pszText);
      if( !GetDisplayValue(pszText, cchMax) ) return NULL;
      // Create window
      CPropertyDateWindow* win = new CPropertyDateWindow();
      ATLASSERT(win);
      RECT rcWin = rc;
      m_hwndEdit = win->Create(hWnd, rcWin, pszText);
      ATLASSERT(win->IsWindow());
      return *win;
   }
   BOOL GetDisplayValue(LPTSTR pstr, UINT cchMax) const
   {
      if( m_val.date == 0.0 ) {
         ::lstrcpy(pstr, _T(""));
         return TRUE;
      }
      return CPropertyEditItem::GetDisplayValue(pstr, cchMax);
   }
   BOOL SetValue(const VARIANT& value)
   {
      if( value.vt == VT_BSTR && ::SysStringLen(value.bstrVal) == 0 ) {
         m_val.date = 0.0;
         return TRUE;
      }
      return CPropertyEditItem::SetValue(value);
   }
   BOOL SetValue(HWND hWnd)
   {
      if( ::GetWindowTextLength(hWnd) == 0 ) {
         m_val.date = 0.0;
         return TRUE;
      }
      return CPropertyEditItem::SetValue(hWnd);
   }
};


/////////////////////////////////////////////////////////////////////////////
// Checkmark button

class CPropertyCheckButtonItem : public CProperty
{
protected:
   bool m_bValue;

public:
   CPropertyCheckButtonItem(LPCTSTR pstrName, bool bValue, LPARAM lParam) : 
      CProperty(pstrName, lParam),
      m_bValue(bValue)
   {
   }
   BYTE GetKind() const 
   { 
      return PROPKIND_CHECK; 
   }
   BOOL GetValue(VARIANT* pVal) const
   {
      return SUCCEEDED( CComVariant(m_bValue).Detach(pVal) );
   }
   BOOL SetValue(const VARIANT& value)
   {
      // Set a new value
      switch( value.vt ) {
      case VT_BOOL:
         m_bValue = value.boolVal != VARIANT_FALSE;
         return TRUE;
      default:
         ATLASSERT(false);
         return FALSE;
      }
   }
   void DrawValue(PROPERTYDRAWINFO& di)
   {
      int cxThumb = ::GetSystemMetrics(SM_CXMENUCHECK);
      int cyThumb = ::GetSystemMetrics(SM_CYMENUCHECK);

      RECT rcMark = di.rcItem;
      rcMark.left += 3;
      rcMark.right = rcMark.left + cxThumb;
      rcMark.top += 1;
      if( rcMark.top + cyThumb >= rcMark.bottom ) rcMark.top -= rcMark.top + cyThumb - rcMark.bottom + 1;
      rcMark.bottom = rcMark.top + cyThumb;

      UINT uState = DFCS_BUTTONCHECK | DFCS_FLAT;
      if( m_bValue ) uState |= DFCS_CHECKED;
      if( di.state & ODS_DISABLED ) uState |= DFCS_INACTIVE;
      ::DrawFrameControl(di.hDC, &rcMark, DFC_BUTTON, uState);
   }
   BOOL Activate(UINT action, LPARAM /*lParam*/) 
   { 
      switch( action ) {
      case PACT_SPACE:
      case PACT_CLICK:
      case PACT_DBLCLICK:
         if( IsEnabled() ) {
            CComVariant v = !m_bValue;
            ::SendMessage(m_hWndOwner, WM_USER_PROP_CHANGEDPROPERTY, (WPARAM) (VARIANT*) &v, (LPARAM) this);
         }
         break;
      }
      return TRUE;
   }
};


/////////////////////////////////////////////////////////////////////////////
// FileName property

class CPropertyFileNameItem : public CPropertyItem
{
public:
   CPropertyFileNameItem(LPCTSTR pstrName, LPARAM lParam) : CPropertyItem(pstrName, lParam)
   {
   }
   HWND CreateInplaceControl(HWND hWnd, const RECT& rc) 
   {
      // Get default text
      TCHAR szText[MAX_PATH] = { 0 };
      if( !GetDisplayValue(szText, (sizeof(szText) / sizeof(TCHAR)) - 1) ) return NULL;      
      // Create EDIT control
      CPropertyButtonWindow* win = new CPropertyButtonWindow();
      ATLASSERT(win);
      RECT rcWin = rc;
      win->m_prop = this;
      win->Create(hWnd, rcWin, szText);
      ATLASSERT(win->IsWindow());
      return *win;
   }
   BOOL SetValue(const VARIANT& value)
   {
      ATLASSERT(V_VT(&value)==VT_BSTR);
      m_val = value;
      return TRUE;
   }
   BOOL SetValue(HWND /*hWnd*/) 
   {
      // Do nothing... A value should be set on reacting to the button notification.
      // In other words: Use SetItemValue() in response to the PLN_BROWSE notification!
      return TRUE;
   }
   BOOL Activate(UINT action, LPARAM /*lParam*/)
   {
      switch( action ) {
      case PACT_BROWSE:
      case PACT_DBLCLICK:
         // Let control owner know
         NMPROPERTYITEM nmh = { m_hWndOwner, ::GetDlgCtrlID(m_hWndOwner), PIN_BROWSE, this };
         ::SendMessage(::GetParent(m_hWndOwner), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh);
      }
      return TRUE;
   }
   BOOL GetDisplayValue(LPTSTR pstr, UINT cchMax) const
   {
      ATLASSERT(!::IsBadStringPtr(pstr, cchMax));
      *pstr = _T('\0');
      if( m_val.bstrVal == NULL ) return TRUE;
      // Only display actual filename (strip path)
      USES_CONVERSION;
      LPCTSTR pstrFileName = OLE2CT(m_val.bstrVal);
      LPCTSTR p = pstrFileName;
      while( *p ) {
         if( *p == _T(':') || *p == _T('\\') ) pstrFileName = p + 1;
         p = ::CharNext(p);
      }
      ::lstrcpyn(pstr, pstrFileName, cchMax);
      return TRUE;
   }
   UINT GetDisplayValueLength() const
   {
      TCHAR szPath[MAX_PATH] = { 0 };
      if( !GetDisplayValue(szPath, (sizeof(szPath) / sizeof(TCHAR)) - 1) ) return 0;
      return ::lstrlen(szPath);
   }
};


/////////////////////////////////////////////////////////////////////////////
// Color property

class CPropertyColorItem : public CPropertyReadOnlyItem
{
   COLORREF m_color;
public:
   CPropertyColorItem(LPCTSTR pstrName, LPARAM lParam) : CPropertyReadOnlyItem(pstrName, lParam)
   {
   }

   void DrawValue(PROPERTYDRAWINFO& di)
   {
      CPropertyReadOnlyItem::DrawValue(di);
      if (di.state & ODS_SELECTED) {
         CDCHandle dc(di.hDC);
         RECT rc = di.rcItem;
         rc.left = rc.right - 16;
         // Paint as ellipsis button
         dc.DrawFrameControl(&rc, DFC_BUTTON, DFCS_BUTTONPUSH);
         //dc.DrawFrameControl(&rc, DFC_BUTTON, (di.state & ODS_SELECTED) ? DFCS_BUTTONPUSH | DFCS_PUSHED : DFCS_BUTTONPUSH);
         dc.SetBkMode(TRANSPARENT);
         LPCTSTR pstrEllipsis = _T("...");
         dc.DrawText(pstrEllipsis, ::lstrlen(pstrEllipsis), &rc, DT_CENTER | DT_EDITCONTROL | DT_SINGLELINE | DT_VCENTER);
      }
   }

   BOOL SetValue(const VARIANT& value)
   {
      ATLASSERT(V_VT(&value)==VT_UI4);
      m_val = value;
      m_color = m_val.uintVal;

	  // create an icon with the new color
      CDC dc;
      dc.CreateCompatibleDC(0);
      CBitmap iconBitmap;
      SIZE sz = { ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON) };

      iconBitmap.CreateBitmap(sz.cx, sz.cy, 1, 32, 0);
      RECT rcFull;
      RECT rc;
      SetRect(&rcFull, 0, 0, sz.cx, sz.cy );
      SetRect(&rc, 0, 0, sz.cx, sz.cy-5 );
      CBrush brush;
      CBrush whiteBrush;
      brush.CreateSolidBrush(m_color);
      whiteBrush.CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
      dc.SelectBitmap(iconBitmap);
      dc.FillRect(&rcFull, whiteBrush);
      dc.FillRect(&rc, brush);

      CBitmap iconMaskBitmap;
      iconMaskBitmap.CreateBitmap(sz.cx, sz.cy, 1, 32, 0);
      brush.DeleteObject();
      brush.CreateSolidBrush(0);
      dc.SelectBitmap(iconMaskBitmap);
      dc.FillRect(&rcFull, whiteBrush);
      dc.FillRect(&rc, brush);

      ICONINFO ii;
      ii.fIcon = TRUE;
      ii.hbmColor = iconBitmap;
      ii.hbmMask = iconMaskBitmap;
      ii.xHotspot = 0;
      ii.yHotspot = 0;
      HICON hIcon = CreateIconIndirect(&ii);
      HICON hOldIcon = SetIcon(hIcon);
      if (hOldIcon != 0)
         DestroyIcon(hOldIcon);
      return TRUE;
   }
   BOOL SetValue(HWND /*hWnd*/) 
   {
      // Do nothing... A value should be set on reacting to the button notification.
      // In other words: Use SetItemValue() in response to the PLN_BROWSE notification!
      return TRUE;
   }
   BOOL Activate(UINT action, LPARAM lParam)
   {
      RECT rc;
      GetClientRect(m_hWndOwner, &rc);
      WTL::CColorDialog clg(m_color, CC_FULLOPEN);
      switch( action ) {
         case PACT_CLICK:
            if (m_fEnabled) {
               int x = GET_X_LPARAM(lParam);
               if (x > rc.right - 20) {
                  return Activate(PACT_BROWSE, 0);
               }
            }
            break;
         case PACT_BROWSE:
            if (IDOK == clg.DoModal(m_hWndOwner)) {
               CComVariant vCol = CComVariant(clg.m_cc.rgbResult);
               ::SendMessage(m_hWndOwner, WM_USER_PROP_CHANGEDPROPERTY, (WPARAM) (VARIANT*) &vCol, (LPARAM) this);
            }
            break;
         case PACT_DBLCLICK:
            // Let control owner know
            NMPROPERTYITEM nmh = { m_hWndOwner, ::GetDlgCtrlID(m_hWndOwner), PIN_BROWSE, this };
            ::SendMessage(::GetParent(m_hWndOwner), WM_NOTIFY, nmh.hdr.idFrom, (LPARAM) &nmh);
      }
      return TRUE;
   }
   BOOL GetDisplayValue(LPTSTR pstr, UINT cchMax) const
   {
#define RGBA_GETRED(rgb)        (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)      (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)       ((rgb) & 0xff)
      ATLASSERT(V_VT(&m_val)==VT_UI4);
      lstrcpy(pstr, "#");
      TCHAR szValue[64] = { 0 };
      _itot(RGBA_GETRED(m_val.uintVal), szValue, 16);
      if (szValue[1] == '\0') lstrcat(pstr, "0");
      lstrcat(pstr, szValue);

      _itot(RGBA_GETGREEN(m_val.uintVal), szValue, 16);
      if (szValue[1] == '\0') lstrcat(pstr, "0");
      lstrcat(pstr, szValue);

      _itot(RGBA_GETBLUE(m_val.uintVal), szValue, 16);
      if (szValue[1] == '\0') lstrcat(pstr, "0");
      lstrcat(pstr, szValue);
      return TRUE;
   }
   UINT GetDisplayValueLength() const
   {
      TCHAR szPath[MAX_PATH] = { 0 };
      if( !GetDisplayValue(szPath, (sizeof(szPath) / sizeof(TCHAR)) - 1) ) return 0;
      return ::lstrlen(szPath);
   }
};


/////////////////////////////////////////////////////////////////////////////
// DropDown List property

class CPropertyListItem : public CPropertyItem
{
protected:
   CSimpleArray<CComBSTR> m_arrList;
   HWND m_hwndCombo;

public:
   CPropertyListItem(LPCTSTR pstrName, LPARAM lParam) : 
      CPropertyItem(pstrName, lParam), 
      m_hwndCombo(NULL)
   {
      m_val = -1L;
   }
   BYTE GetKind() const 
   { 
      return PROPKIND_LIST; 
   }
   HWND CreateInplaceControl(HWND hWnd, const RECT& rc) 
   {
      // Get default text
      UINT cchMax = GetDisplayValueLength() + 1;
      LPTSTR pszText = (LPTSTR) _alloca(cchMax * sizeof(TCHAR));
      ATLASSERT(pszText);
      if( !GetDisplayValue(pszText, cchMax) ) return NULL;
      // Create 'faked' DropDown control
      CPropertyListWindow* win = new CPropertyListWindow();
      ATLASSERT(win);
      RECT rcWin = rc;
      m_hwndCombo = win->Create(hWnd, rcWin, pszText);
      ATLASSERT(win->IsWindow());
      // Add list
      USES_CONVERSION;      
      for( int i = 0; i < m_arrList.GetSize(); i++ ) win->AddItem(OLE2CT(m_arrList[i]));
      win->SelectItem(m_val.lVal);
      // Go...
      return *win;
   }
   BOOL Activate(UINT action, LPARAM /*lParam*/)
   {
      switch( action ) {
      case PACT_CLICK:
      case PACT_SPACE:
         if( ::IsWindow(m_hwndCombo) ) {
            // Fake button click...
            ::SendMessage(m_hwndCombo, WM_COMMAND, MAKEWPARAM(0, BN_CLICKED), 0);
         }
         break;
      case PACT_DBLCLICK:
         // Simulate neat VB control effect. DblClick cycles items in list...
         // Set value and recycle edit control
         if( IsEnabled() ) {
            CComVariant v = m_val.lVal + 1;
            ::SendMessage(m_hWndOwner, WM_USER_PROP_CHANGEDPROPERTY, (WPARAM) (VARIANT*) &v, (LPARAM) this);
         }
         break;
      }
      return TRUE;
   }
   BOOL GetDisplayValue(LPTSTR pstr, UINT cchMax) const
   {
      ATLASSERT(m_val.vt==VT_I4);
      ATLASSERT(!::IsBadStringPtr(pstr, cchMax));
      *pstr = _T('\0');
      if( m_val.lVal < 0 || m_val.lVal >= m_arrList.GetSize() ) return FALSE;
      USES_CONVERSION;
      ::lstrcpyn( pstr, OLE2CT(m_arrList[m_val.lVal]), cchMax) ;
      return TRUE;
   }
   UINT GetDisplayValueLength() const
   {
      ATLASSERT(m_val.vt==VT_I4);
      if( m_val.lVal < 0 || m_val.lVal >= m_arrList.GetSize() ) return 0;
      BSTR bstr = m_arrList[m_val.lVal];
      USES_CONVERSION;
      return bstr == NULL ? 0 : ::lstrlen(OLE2CT(bstr));
   };

   BOOL SetValue(const VARIANT& value)
   {
      switch( value.vt ) {
      case VT_BSTR:
         {
            m_val = 0;
            for( long i = 0; i < m_arrList.GetSize(); i++ ) {
               if( ::wcscmp(value.bstrVal, m_arrList[i]) == 0 ) {
                  m_val = i;
                  return TRUE;
               }
            }
            return FALSE;
         }
         break;
      default:
         // Treat as index into list
         if( FAILED( m_val.ChangeType(VT_I4, &value) ) ) return FALSE;
         if( m_val.lVal >= m_arrList.GetSize() ) m_val.lVal = 0;
         return TRUE;
      }
   }
   BOOL SetValue(HWND hWnd)
   { 
      ATLASSERT(::IsWindow(hWnd));
      int len = ::GetWindowTextLength(hWnd) + 1;
      LPTSTR pstr = (LPTSTR) _alloca(len * sizeof(TCHAR));
      ATLASSERT(pstr);
      if( !::GetWindowText(hWnd, pstr, len) ) {
         if( ::GetLastError() != 0 ) return FALSE;
      }
      CComVariant v = pstr;
      return SetValue(v);
   }
   void SetList(LPCTSTR* ppList)
   {
      ATLASSERT(ppList);
      m_arrList.RemoveAll();
      while( *ppList ) {
         CComBSTR bstr(*ppList);
         m_arrList.Add(bstr);
         ppList++;
      }
      if( m_val.lVal == -1 ) m_val = 0;
   }
   void AddListItem(LPCTSTR pstrText)
   {
      ATLASSERT(!::IsBadStringPtr(pstrText,-1));
      CComBSTR bstr(pstrText);
      m_arrList.Add(bstr);
      if( m_val.lVal == -1 ) m_val = 0;
   }
};


/////////////////////////////////////////////////////////////////////////////
// Boolean property

class CPropertyBooleanItem : public CPropertyListItem
{
public:
   CPropertyBooleanItem(LPCTSTR pstrName, LPARAM lParam) : CPropertyListItem(pstrName, lParam)
   {
#ifdef IDS_TRUE
      TCHAR szBuffer[32];
      ::LoadString(_Module.GetResourceInstance(), IDS_FALSE, szBuffer, sizeof(szBuffer) / sizeof(TCHAR));
      AddListItem(szBuffer);
      ::LoadString(_Module.GetResourceInstance(), IDS_TRUE, szBuffer, sizeof(szBuffer) / sizeof(TCHAR));
      AddListItem(szBuffer);
#else
      AddListItem(_T("False"));
      AddListItem(_T("True"));
#endif
   }
};


/////////////////////////////////////////////////////////////////////////////
// ListBox Control property

class CPropertyComboItem : public CPropertyItem
{
public:
   CListBox m_ctrl;

   CPropertyComboItem(LPCTSTR pstrName, LPARAM lParam) : 
      CPropertyItem(pstrName, lParam)
   {
   }
   HWND CreateInplaceControl(HWND hWnd, const RECT& rc) 
   {
      ATLASSERT(::IsWindow(m_ctrl));
      // Create window
      CPropertyComboWindow* win = new CPropertyComboWindow();
      ATLASSERT(win);
      RECT rcWin = rc;
      win->m_hWndCombo = m_ctrl;
      win->Create(hWnd, rcWin);
      ATLASSERT(::IsWindow(*win));
      return *win;
   }
   BYTE GetKind() const 
   { 
      return PROPKIND_CONTROL; 
   }
   void DrawValue(PROPERTYDRAWINFO& di) 
   { 
      RECT rc = di.rcItem;
      ::InflateRect(&rc, 0, -1);
      DRAWITEMSTRUCT dis = { 0 };
      dis.hDC = di.hDC;
      dis.hwndItem = m_ctrl;
      dis.CtlID = m_ctrl.GetDlgCtrlID();
      dis.CtlType = ODT_LISTBOX;
      dis.rcItem = rc;
      dis.itemState = ODS_DEFAULT | ODS_COMBOBOXEDIT;
      dis.itemID = m_ctrl.GetCurSel();
      dis.itemData = (int) m_ctrl.GetItemData(dis.itemID);
      ::SendMessage(m_ctrl, OCM_DRAWITEM, dis.CtlID, (LPARAM) &dis);
   }
   BOOL GetValue(VARIANT* pValue) const 
   { 
      CComVariant v = (int) m_ctrl.GetItemData(m_ctrl.GetCurSel());
      return SUCCEEDED( v.Detach(pValue) );
   }
   BOOL SetValue(HWND /*hWnd*/) 
   {      
      int iSel = m_ctrl.GetCurSel();
      CComVariant v = (int) m_ctrl.GetItemData(iSel);
      return SetValue(v); 
   }
   BOOL SetValue(const VARIANT& value)
   {
      ATLASSERT(value.vt==VT_I4);
      for( int i = 0; i < m_ctrl.GetCount(); i++ ) {
         if( m_ctrl.GetItemData(i) == (DWORD_PTR) value.lVal ) {
            m_ctrl.SetCurSel(i);
            return TRUE;
         }
      }
      return FALSE;
   }
};


/////////////////////////////////////////////////////////////////////////////
//
// CProperty creators
//

inline HPROPERTY PropCreateVariant(LPCTSTR pstrName, const VARIANT& vValue, LPARAM lParam = 0)
{
   CPropertyEditItem* prop = NULL;
   ATLTRY( prop = new CPropertyEditItem(pstrName, lParam) );

   ATLASSERT(prop);
   if( prop ) prop->SetValue(vValue);
   return prop;
}

inline HPROPERTY PropCreateSimple(LPCTSTR pstrName, LPCTSTR pstrValue, LPARAM lParam = 0)
{
   CComVariant vValue = pstrValue;
   return PropCreateVariant(pstrName, vValue, lParam);
}

inline HPROPERTY PropCreateSimple(LPCTSTR pstrName, int iValue, LPARAM lParam = 0)
{
   CComVariant vValue = iValue;
   return PropCreateVariant(pstrName, vValue, lParam);
}

inline HPROPERTY PropCreateSimple(LPCTSTR pstrName, bool bValue, LPARAM lParam = 0)
{
   // NOTE: Converts to integer, since we're using value as an index to dropdown
   CComVariant vValue = (int) bValue & 1;
   CPropertyBooleanItem* prop = NULL;
   ATLTRY( prop = new CPropertyBooleanItem(pstrName, lParam) );
   ATLASSERT(prop);
   if( prop ) prop->SetValue(vValue);
   return prop;
}

inline HPROPERTY PropCreateFileName(LPCTSTR pstrName, LPCTSTR pstrFileName, LPARAM lParam = 0)
{
   ATLASSERT(!::IsBadStringPtr(pstrFileName,-1));
   CPropertyFileNameItem* prop = NULL;
   ATLTRY( prop = new CPropertyFileNameItem(pstrName, lParam) );
   ATLASSERT(prop);
   if( prop == NULL ) return NULL;
   CComVariant vValue = pstrFileName;
   prop->SetValue(vValue);
   return prop;
}

inline HPROPERTY PropCreateDate(LPCTSTR pstrName, const SYSTEMTIME stValue, LPARAM lParam = 0)
{
   IProperty* prop = NULL;
   ATLTRY( prop = new CPropertyDateItem(pstrName, lParam) );
   ATLASSERT(prop);
   if( prop == NULL ) return NULL;
   CComVariant vValue;
   vValue.vt = VT_DATE;
   vValue.date = 0.0; // NOTE: Clears value in case of conversion error below!
   if( stValue.wYear > 0 ) ::SystemTimeToVariantTime( (LPSYSTEMTIME) &stValue, &vValue.date );
   prop->SetValue(vValue);
   return prop;
}

inline HPROPERTY PropCreateList(LPCTSTR pstrName, LPCTSTR* ppList, int iValue = 0, LPARAM lParam = 0)
{
   ATLASSERT(ppList);
   CPropertyListItem* prop = NULL;
   ATLTRY( prop = new CPropertyListItem(pstrName, lParam) );
   ATLASSERT(prop);
   if( prop && ppList ) {
      prop->SetList(ppList);
      CComVariant vValue = iValue;
      prop->SetValue(vValue);
   }
   return prop;
}

inline HPROPERTY PropCreateComboControl(LPCTSTR pstrName, HWND hWnd, int iValue, LPARAM lParam = 0)
{
   ATLASSERT(::IsWindow(hWnd));
   CPropertyComboItem* prop = NULL;
   ATLTRY( prop = new CPropertyComboItem(pstrName, lParam) );
   ATLASSERT(prop);
   if( prop ) {
      prop->m_ctrl = hWnd;
      CComVariant vValue = iValue;
      prop->SetValue(vValue);
   }
   return prop;
}

inline HPROPERTY PropCreateCheckButton(LPCTSTR pstrName, bool bValue, LPARAM lParam = 0)
{
   return new CPropertyCheckButtonItem(pstrName, bValue, lParam);
}

inline HPROPERTY PropCreateReadOnlyItem(LPCTSTR pstrName, LPCTSTR pstrValue = _T(""), LPARAM lParam = 0)
{
   ATLASSERT(!::IsBadStringPtr(pstrValue,-1));
   CPropertyItem* prop = NULL;
   ATLTRY( prop = new CPropertyReadOnlyItem(pstrName, lParam) );
   ATLASSERT(prop);
   if( prop ) {
      CComVariant v = pstrValue;
      prop->SetValue(v);
   }
   return prop;
}

inline HPROPERTY PropCreateColor(LPCTSTR pstrName, COLORREF color, LPARAM lParam = 0)
{
   CPropertyColorItem* prop = NULL;
   ATLTRY( prop = new CPropertyColorItem(pstrName, lParam) );
   ATLASSERT(prop);
   if( prop == NULL ) return NULL;
   CComVariant vValue = color;
   prop->SetValue(vValue);
   return prop;
}



#endif // __PROPERTYITEMIMPL__H
