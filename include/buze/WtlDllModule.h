#pragma once

// Use this instead of CAppModule _Module in view plugin DLL's stdafx.h
// This provides access to the host's message loop to support message 
// filtering and idle processing in WTL-based plugins.

// The interesting methods of CMessageLoop cannot be overridden, thus existing
// code using _Module.GetMessageLoop() etc must be modified to use the provided
// CModuleMessageLoop instead of WTL's CMessageLoop. 

class CHostDllModule : public CAtlDllModuleT<CHostDllModule> {
public:
	CHostModule* m_hostModule;

	// "Did you forget to pass the LIBID to CComModule::Init"
	DECLARE_LIBID(LIBID_ATLLib)

	HINSTANCE GetResourceInstance() {
		return ModuleHelper::GetResourceInstance();
	}

	CModuleMessageLoop* AddMessageLoop() {
		return m_hostModule->AddMessageLoop();
	}

	CModuleMessageLoop* GetMessageLoop() {
		return m_hostModule->GetMessageLoop();
	}

	void RemoveMessageLoop() {
		m_hostModule->RemoveMessageLoop();
	}
};
