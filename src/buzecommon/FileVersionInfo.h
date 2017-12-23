// FileVersionInfo.h: interface for the CFileVersionInfo class.
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2000.
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
////////////////////////////////////////////////////////////////////////
//
// Slightly modified by andyw for WTL

#if !defined(AFX_FILEVERSIONINFO_H__C93F5002_F88A_11D1_93C1_A41808C10000__INCLUDED_)
#define AFX_FILEVERSIONINFO_H__C93F5002_F88A_11D1_93C1_A41808C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <winver.h>
#pragma comment(lib, "version.lib")

class CFileVersionInfo
{
public:
   CFileVersionInfo();
   virtual ~CFileVersionInfo();

// Implementation
public:
   BOOL Open( LPCTSTR pszFilename );
   BOOL Open( HINSTANCE handle=NULL );
   //
   CString GetComments() const { return GetItem(_T("Comments")); };
   CString GetCompanyName() const { return GetItem(_T("CompanyName")); };
   CString GetFileDescription() const { return GetItem(_T("FileDescription")); };
   CString GetFileVersion() const { return GetItem(_T("FileVersion")); };
   CString GetInternalName() const { return GetItem(_T("InternalName")); };
   CString GetLegalCopyright() const { return GetItem(_T("LegalCopyright")); };
   CString GetLegalTrademarks() const { return GetItem(_T("LegalTrademarks")); };
   CString GetOriginalFilename() const { return GetItem(_T("OriginalFilename")); };
   CString GetPrivateBuild() const { return GetItem(_T("PrivateBuild")); };
   CString GetProductName() const { return GetItem(_T("ProductName")); };
   CString GetProductVersion() const { return GetItem(_T("ProductVersion")); };
   CString GetSpecialBuild() const { return GetItem(_T("SpecialBuild")); };

protected:
   CString GetItem( LPCTSTR pszItem ) const;

// Overrides
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
#endif

// Variables
protected:
   CString     m_strFilename;
   TCHAR       m_szTransBlock[255];   // Language translation block
   LPVOID      m_lpVerInfoBlock;      // Pointer to version info block
};

#endif // !defined(AFX_FILEVERSIONINFO_H__C93F5002_F88A_11D1_93C1_A41808C10000__INCLUDED_)
