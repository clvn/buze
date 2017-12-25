#pragma once

#define WINVER           0x0501
#define _WIN32_WINNT     0x0501
#define _WIN32_IE        0x0600
#define _RICHEDIT_VER    0x0100

#include <atlbase.h>
#include <wtl/atlapp.h>
#include <wtl/atlcrack.h>
#include <wtl/atlctrls.h>
#include <wtl/atlsplit.h>

#include <cassert>
#include <string>
#include <vector>
#include <buze/HostModule.h>
#include <buze/WtlDllModule.h>
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>

extern CHostDllModule _Module;
