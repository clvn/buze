#pragma once

#define WINVER			0x0501
#define _WIN32_WINNT	0x0600
#define _WIN32_IE		0x0600
#define _RICHEDIT_VER	0x0100

//#define _WTL_NO_CSTRING
#define _WTL_USE_CSTRING
//#define _WTL_NO_WTYPES
#define NOMINMAX

#if defined(_DEBUG)
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#if (_MSC_VER >= 1400)
#if defined(_M_X64)
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include <algorithm>
using std::min;
using std::max;

// --- include ATL headers first ---
#include <atlbase.h>
#include <atlcoll.h>
//#include <atlstr.h> -- we're using _WTL_USE_CSTRING+atlmisc.h instead
//#include <atltypes.h> -- we're using atlmisc.h instead
#include <atlwin.h>
// --- include WTL headers after ---
#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atlctrlx.h>
#include <atlddx.h>
#include <atldlgs.h>
#include <atlframe.h>
#include <atlmisc.h> // CSize, CPoint, CRect, CString
#include <atlsplit.h>
#include "atlgdix.h"
#include "WtlUtils.h"

#define _USE_MATH_DEFINES

#include <vector>
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cassert>

#include "cpputils.h"

namespace std
{
    #ifdef _UNICODE
        typedef wstring tstring;
        typedef wstringstream tstringstream;
    #else
        typedef string tstring;
        typedef stringstream tstringstream;
    #endif
};

static const char* programName = "buzé 0.9";

extern CAppModule _Module;
