#pragma once

// Interfaces for supporting WTL _Module.GetMessageLoop across the DLL boundary.
// Or any other toolkits supporting message filtering and idle handling.

// CHostModule must be implemented in the host (exe, whatever), and passed to
// CHostDllModule in all DLLs to bridge.

class CModuleMessageLoop {
public:
	virtual int Run() = 0;
	virtual BOOL AddMessageFilter(CMessageFilter* pFilter) = 0;
	virtual BOOL RemoveMessageFilter(CMessageFilter* pFilter) = 0;
	virtual BOOL AddIdleHandler(CIdleHandler* pHandler) = 0;
	virtual BOOL RemoveIdleHandler(CIdleHandler* pHandler) = 0;
};

class CHostModule {
public:
	virtual CModuleMessageLoop* GetMessageLoop() = 0;
	virtual CModuleMessageLoop* AddMessageLoop() = 0;
	virtual void RemoveMessageLoop() = 0;
};

