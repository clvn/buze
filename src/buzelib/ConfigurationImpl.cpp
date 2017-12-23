#include <atlbase.h>
#include <string>
#include <vector>
#include <map>
#include <buze/buzesdk.h>
#include "Configuration.h"
#include "BuzeConfiguration.h"
#include "ConfigurationImpl.h"

static const char* REGISTRY_CONFIG_KEY = "Software\\zzub\\buze\\";

CConfigurationImpl::CConfigurationImpl() {
}


bool CConfigurationImpl::getConfigString(const char* section, const char* key, char* value) {
	CRegKey reg;

	std::string regKey = (std::string)REGISTRY_CONFIG_KEY + (std::string)section;
	LONG err = reg.Open(HKEY_CURRENT_USER, regKey.c_str(), KEY_READ);
	if (err != ERROR_SUCCESS) return false;

	DWORD len = 1024;
	//char pc[1024];

	err = reg.QueryStringValue(key, value, &len);
	if (err != ERROR_SUCCESS) return false;

	//if (value)
	//	*value = pc;
	return true;
}

bool CConfigurationImpl::getConfigNumber(const char* section, const char* key, DWORD* value) {
	CRegKey reg;

	std::string regKey = (std::string)REGISTRY_CONFIG_KEY + section;
	LONG err = reg.Open(HKEY_CURRENT_USER, regKey.c_str(), KEY_READ);
	if (err != ERROR_SUCCESS) return false;

	err = reg.QueryDWORDValue(key, *value);
	if (err != ERROR_SUCCESS) return false;

	return true;
}

bool CConfigurationImpl::setConfigString(const char* section, const char* key, const char* value) {
	CRegKey reg;

	std::string regKey = (std::string)REGISTRY_CONFIG_KEY + section;
	LONG err = reg.Create(HKEY_CURRENT_USER, regKey.c_str());
	if (err != ERROR_SUCCESS) return false;

	err = reg.SetStringValue(key, value);
	if (err != ERROR_SUCCESS) return false;

	return true;
}

bool CConfigurationImpl::setConfigNumber(const char* section, const char* key, int value) {
	CRegKey reg;

	std::string regKey = (std::string)REGISTRY_CONFIG_KEY + (std::string)section;
	LONG err = reg.Create(HKEY_CURRENT_USER, regKey.c_str());
	if (err != ERROR_SUCCESS) return false;

	err = reg.SetDWORDValue(key, value);
	if (err != ERROR_SUCCESS) return false;

	return true;
}
