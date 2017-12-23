#pragma once

typedef struct _zzub_player zzub_player_t;
typedef struct _zzub_audiodriver zzub_audiodriver_t;

class CMainFrame;
class CConfiguration;
class ThemeManager;

class CApplication {
public:
	bool standalone;
	std::string globalPath;
	std::string userPath;
	std::string tempPath;
	zzub_audiodriver_t* driver;
	zzub_audiodriver_t* olddriver; // copy of the original driver when the silent driver is in use
	zzub_player_t* player;

	CBuzeConfiguration configuration;
	CWaitWindow waitWindow;
	ThemeManager* themes;
	bool wantsAudioPreferences;

	CApplication();
	~CApplication();
	void initialize(HINSTANCE hInstance, bool standalone);
	void uninitialize();
	void redirectIOToConsole();

	bool openMidiDevices();

	bool setAudioDriver();
	bool setAudioDriver(std::string out_name, std::string in_name, unsigned int rate, unsigned int bufferSize, int channel, bool save);
	bool setSilentAudioDriver(unsigned int samplerate);
	void releaseSilentAudioDriver();
	void releaseAudioDriver();

	void setBuzzPath();
	std::string mapPath(const std::string& path, int type);

	void showWaitWindow();
	void setWaitWindowText(std::string text);
	void hideWaitWindow(HWND hFocusWnd = 0);
};
