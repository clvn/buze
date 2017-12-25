#include <atlbase.h>
#include <atlwin.h>
#include <wtl/atlapp.h>
#include <wtl/atlctrls.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cassert>
#include <boost/filesystem/path.hpp>
#include "resource.h"
#include "zzub/zzub.h"
#include "utils.h"
#include "ThemeManager.h"
#include "WaitWindow.h"
#include "Configuration.h"

#define NO_BUZE_CONFIGURATION_TYPE
typedef CConfiguration buze_configuration_t;

#include "BuzeConfiguration.h"
#include "ConfigurationImpl.h"
#include "Application.h"

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif

static const WORD MAX_CONSOLE_LINES = 9999;

/***

	CApplication

***/

// from http://www.halcyon.com/~ast/dload/guicon.htm
void CApplication::redirectIOToConsole() {

	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();
	AttachConsole(GetCurrentProcessId());

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	if (hConHandle == -1) return ;	// assume we've already redirected console

	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well

	ios::sync_with_stdio();
}


CApplication::CApplication() {
	themes = 0;
	configuration = new CConfigurationImpl();
	wantsAudioPreferences = false;
	driver = 0;
	olddriver = 0;
}

CApplication::~CApplication() {
	if (configuration.configuration) delete configuration.configuration;
}

void CApplication::initialize(HINSTANCE hInstance, bool sta) {
	standalone = sta;

	// create but do not show wait window
//	waitWindow.Create(GetDesktopWindow());
	waitWindow.Initialize(GetCurrentThreadId());

	themes = new ThemeManager();
	themes->initialize();

	// NOTE: loading any plugin that uses MFC42.DLL will also invoke 
	// SetErrorMode - which prevents dll loading error dialogs to pop up at
	// start. this should make things more consistent:
	SetErrorMode(SEM_FAILCRITICALERRORS);
}

void CApplication::uninitialize() {
	if (themes) delete themes;
	waitWindow.Uninitialize();
}

bool CApplication::openMidiDevices() {

	// init MIDI (after player)
	vector<std::string> midiInDevices;
	configuration->getMidiInputs(midiInDevices);

	for (size_t i = 0; i < midiInDevices.size(); i++) {
		std::string dev = midiInDevices[i];

		for (int j = 0; j < zzub_mididriver_get_count(player); j++) {
			const char *name = zzub_mididriver_get_name(player, j);
			if (!dev.compare(name) && zzub_mididriver_is_input(player, j)) {
				zzub_mididriver_open(player, j);
				break;
			}
		}
	}

	vector<std::string> midiOutDevices;
	configuration->getMidiOutputs(midiOutDevices);
	for (size_t i = 0; i < midiOutDevices.size(); i++) {
		std::string dev = midiOutDevices[i];

		for (int j = 0; j < zzub_mididriver_get_count(player); j++) {
			const char *name = zzub_mididriver_get_name(player, j);
			if (!dev.compare(name) && zzub_mididriver_is_output(player, j)) {
				zzub_mididriver_open(player, j);
				break;
			}
		}
	}

	return true;
}

bool CApplication::setSilentAudioDriver(unsigned int samplerate) {
	assert(olddriver == 0 && driver != 0);
	
	zzub_audiodriver_enable(driver, 0);

	olddriver = driver;

	vector<int> rates;
	rates.push_back(samplerate);
	driver = zzub_audiodriver_create_silent(player, "Silent", 2, 0, &rates.front(), rates.size());

	zzub_audiodriver_create_device(driver, "", "Silent", 256, samplerate);
	zzub_audiodriver_enable(driver, 1);
	return true;
}

void CApplication::releaseSilentAudioDriver() {
	assert(olddriver && driver);
	zzub_audiodriver_destroy(driver);
	driver = olddriver;
	olddriver = 0;

	zzub_audiodriver_enable(driver, 1);
}


void CApplication::releaseAudioDriver() {
	zzub_audiodriver_destroy(driver);
	driver = 0;
}


bool CApplication::setAudioDriver() {
	string outDevice, inDevice;
	int rate, bufferSize, channel;
	configuration->getAudioDriver(outDevice, inDevice, rate, bufferSize, channel);

	if (outDevice.empty() && zzub_audiodriver_get_count(driver) > 0) {
		zzub_device_info_t* deviceinfo = zzub_audiodriver_get_device_info(driver, 0);
		assert(deviceinfo != 0);
		outDevice = zzub_device_info_get_name(deviceinfo);
	}

	return setAudioDriver(outDevice, inDevice, rate, bufferSize, channel, false);
}

bool CApplication::setAudioDriver(std::string driverName, std::string inDriverName, unsigned int rate, unsigned int bufferSize, int channel, bool save) {
	zzub_audiodriver_destroy_device(driver);
	zzub_audiodriver_set_master_channel(driver, channel);
	if (zzub_audiodriver_create_device(driver, inDriverName.c_str(), driverName.c_str(), bufferSize, rate) == -1) return false;

	if (save)
		configuration->setAudioDriver(driverName, inDriverName, rate, bufferSize, channel);

	zzub_audiodriver_enable(driver, true);

	zzub_player_set_thread_count(player, configuration->getMixerThreads());

	return true;
}

/*
void CApplication::setBuzzPath() {
	// set buzz registry before any machines are loaded
	CRegKey buzzPath;
	LONG err = buzzPath.Open(HKEY_CURRENT_USER, "Software\\Jeskola\\Buzz\\Settings");
	// create key if it doesnt exist
	if (err) {
		buzzPath.Create(HKEY_CURRENT_USER, "Software\\Jeskola\\Buzz\\Settings");
	}
	if (!err) {
		char szPath[MAX_PATH];
		GetModuleFileName(m_hInst, szPath, MAX_PATH);
		*strrchr(szPath, '\\') = '\0';
		strcat(szPath, "\\");
		buzzPath.SetStringValue("BuzzPath", szPath);
		buzzPath.Close();
	}
}
*/

std::string CApplication::mapPath(const std::string& path, int type) {
	if (standalone) {
		if (type == 0) {
			boost::filesystem::path result = boost::filesystem::path(globalPath) / boost::filesystem::path(path);
			return result.make_preferred().string();
		} else if (type == 1) {
			boost::filesystem::path result = boost::filesystem::path(userPath) / boost::filesystem::path(path);
			return result.make_preferred().string();
		} else
			return "";
		return path;
	}

	CRegKey regBuzzPath;
	TCHAR machinePath[1024];

	LONG err = regBuzzPath.Open(HKEY_CURRENT_USER, "Software\\Jeskola\\Buzz\\Settings");
	if (!err) {
		ULONG len = 1024;
		regBuzzPath.QueryStringValue("BuzzPath", machinePath, &len);
		regBuzzPath.Close();
	}
	return machinePath + path;
}

void CApplication::showWaitWindow() {
	waitWindow.Show();
}

void CApplication::setWaitWindowText(std::string text) {
	waitWindow.SetText(text.c_str());
}

void CApplication::hideWaitWindow(HWND hFocusWnd) {
	waitWindow.Hide(hFocusWnd);
}
