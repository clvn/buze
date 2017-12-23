/*
	This file is part of the Buzé base Buzz-library. 
	
	Please refer to LICENSE.TXT for details regarding usage.
*/

#pragma once

// Configuration persistence interface
// It is up to clients to provide an implementation of this 
// Configuration is currently used for audio driver settings and extended pr-machine and pr-parameter settings

class CConfiguration {
public:
	virtual ~CConfiguration() {}
	virtual bool getConfigString(const char* section, const char* key, char* value)=0;
	virtual bool getConfigNumber(const char* section, const char* key, DWORD* value)=0;
	virtual bool setConfigString(const char* section, const char* key, const char* value)=0;
	virtual bool setConfigNumber(const char* section, const char* key, int value)=0;
};
