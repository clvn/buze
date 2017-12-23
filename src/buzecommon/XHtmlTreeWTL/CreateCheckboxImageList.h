// CreateCheckboxImageList.h  Version 1.0
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

namespace HDCheckboxImageList
{
	static const UINT HOT_INDEX			= 8;
	static const UINT DISABLED_INDEX	= 4;

	///BOOL CreateCheckboxImageList(CDC* pDC, CImageList& imagelist, int nSize, COLORREF crBackground);
	BOOL CreateCheckboxImageList(CDCHandle pDC, CImageList& imagelist, int nSize, COLORREF crBackground);
}
