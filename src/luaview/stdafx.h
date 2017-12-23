#pragma once

#define WINVER           0x0501
#define _WIN32_WINNT     0x0501
#define _WIN32_IE        0x0600
#define _RICHEDIT_VER    0x0100

// --- include ATL headers first ---
#include <atlbase.h>
// --- include WTL headers after ---
#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlsplit.h>

// TODO: reference additional headers your program requires here

#include <cassert>
#include <string>
#include <vector>
#include <buze/HostModule.h>
#include <buze/WtlDllModule.h>
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>

extern CHostDllModule _Module;
