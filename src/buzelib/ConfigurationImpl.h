#pragma once

class CConfigurationImpl : public CConfiguration
{
public:
	CConfigurationImpl();
	bool getConfigString(const char* section, const char* key, char* value);
	bool getConfigNumber(const char* section, const char* key, DWORD* value);
	bool setConfigString(const char* section, const char* key, const char* value);
	bool setConfigNumber(const char* section, const char* key, int value);
};
