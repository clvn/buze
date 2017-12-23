#pragma once

#define WINVER           0x0501
#define _WIN32_WINNT     0x0501
#define _WIN32_IE        0x0600
#define _RICHEDIT_VER    0x0100
#define NOMINMAX

#include <cassert>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <cctype>

using std::min;
using std::max;

// --- include ATL headers first ---
#include <atlbase.h>
// --- include WTL headers after ---
#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlsplit.h>
#include <atlmisc.h>

// TODO: reference additional headers your program requires here

#include <buze/HostModule.h>
#include <buze/WtlDllModule.h>
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>

extern CHostDllModule _Module;
