#pragma once
#if _MSC_VER >= 1500
	#define _WIN32_WINNT	0x0500
#else
	#define WINVER	0x0401
#endif

// windows excludes
#define NOMCX
// mmreg excludes
#define NOMMIDS
#define NOJPEGDIB
#define NONEWIC
#define NOBITMAP
// mmsystem excludes
#define MMNODRV
#define MMNOMCI

#define _CRT_SECURE_NO_WARNINGS		// Define to disable the "This function or variable may be unsafe" warnings.
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES			1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT	1

#include <windows.h>
#include <string>
//#include <afxwin.h>         // MFC core and standard components
//#include <afxext.h>         // MFC extensions
//#include <afxcmn.h>			// MFC support for Windows Common Controls
//#include <afxcview.h>
//#include <afxole.h>
#include <windowsx.h>
#include <cassert>
#include "typedefs.h"
#include "modcommand.h"
#define NO_ASIO
#define NO_ARCHIVE_SUPPORT
#define MODPLUG_TRACKER

const TCHAR gszEmpty[] = TEXT("");
extern const LPCTSTR szDefaultNoteNames[NOTE_MAX];
