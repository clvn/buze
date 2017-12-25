#pragma once

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501
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

//#if (_MSC_VER >= 1400)
//#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#endif

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
#include <wtl/atlapp.h>
#include <wtl/atlcrack.h>
#include <wtl/atlctrls.h>
#include <wtl/atlctrlw.h>
#include <wtl/atlctrlx.h>
#include <wtl/atlddx.h>
#include <wtl/atldlgs.h>
#include <wtl/atlframe.h>
#include <wtl/atlmisc.h> // CSize, CPoint, CRect, CString
#include <wtl/atlsplit.h>
#include "atlgdix.h"
#include "WtlUtils.h"

#define _USE_MATH_DEFINES

// TODO: reference additional headers your program requires here

#include <vector>
#include <list>
#include <map>
#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cassert>

#include <buze/HostModule.h>
#include <buze/WtlDllModule.h>

#include <zzub/zzub.h>

extern CHostDllModule _Module;
