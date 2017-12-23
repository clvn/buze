#pragma once

struct hackseqtype {
	char fill[0x390];
	int loopBegin;					// CSequencer + 0x390
	int loopEnd;					// CSequencer + 0x394
	int songEnd;					// CSequencer + 0x398
	int songPos;					
};

struct unhack {

	static unsigned int hackmidi[256];
	static hackseqtype* hackseq;
	static unsigned int hackbpm;
	static std::map<std::string, std::vector<std::string> > patches;
	static std::map<HMODULE, std::string > modules;
	static std::map<HINSTANCE, HMODULE > instances;
	static HWND hostwnd;

	static void enablePatch(std::string machineName, std::string expr);

	static bool isPatch(std::string machineName);

	static void process(std::string machineName, void* data, size_t len);

	static std::string machineNameFromFileName(std::string fileName);


	static void replace(void* buffer, size_t bufferlen, unsigned char* find, unsigned char* repl, size_t findlen, std::string machineName, std::string fixName);
	static void bpm(void* buffer, size_t bufferlen, std::string machineName);
	static void seq(void* buffer, size_t bufferlen, std::string machineName);
	static void midi(void* buffer, size_t bufferlen, std::string machineName);

	static HMODULE loadLibrary(LPCTSTR lpLibFileName);
	static BOOL freeLibrary(HMODULE);
	static FARPROC getProcAddress(HMODULE, LPCSTR);

	static void hackTick(int, int, int, int, int);
};

