/* .-----------------------------------.
   | XHtmlDrawLink                     |
   | 2009 Update by Megz               |
   | Original release notes are below. |
   '-----------------------------------' */

///////////////////////////////////////////////////////////////////////////////
//
// XHtmlDrawLink.h  Version 1.2 - article available at www.codeproject.com
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

#pragma once

#include "XHtmlDraw.h"

class CXHtmlDrawLink
{
  // Construction / Destruction
  // --------------------------
  public:

	CXHtmlDrawLink();
	virtual ~CXHtmlDrawLink();

  // Operations
  // ----------
  public:

	BOOL ProcessAppCommand(LPCTSTR lpszCommand, LPARAM lParam);
	void SetAppCommands(CXHtmlDraw::XHTMLDRAW_APP_COMMAND* paAppCommands, int nAppCommands);
	BOOL SetLinkCursor();

  // Implementation
  // --------------
  protected:

    HCURSOR m_hLinkCursor; // cursor for links
	int m_nAppCommands;
	CXHtmlDraw::XHTMLDRAW_APP_COMMAND* m_paAppCommands;

	void SetDefaultCursor();
};
