//#pragma comment (linker, "/STACK:0x1000000") // Default stack size is 24Mb

#include "stdafx.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <zzub/signature.h>
#include <buze/buzesdk.h>
#include <buze/ViewFrame.h>
#include "resource.h"

#include "../buzecommon/Configuration.h"

#include "../buzecommon/FileVersionInfo.h"
#include "../buzecommon/BuzeConfiguration.h"
#include "MainFrm.h"
#include "splashwnd.h"

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif

#include "ExceptionDialog.h"
#include <buze/HostModule.h>

// set aside some TLS data for the buzzexe-wrapper. note that this means declspec(thread) 
// cannot be used anywhere in buze since it would collide with the tls-usage in plugins in 
// buzz.exe. their tls-access is hard coded to the first index which is provided here:
int __declspec(thread) forcetls[0x100]; 

CAppModule _Module;
zzub_player_t* exception_handler_player = 0;

/*
kan vi lage en lame dialog?
	"An unexpected error occurred in Buzé or one of its plugins. What do you want to do?
		[Save As...]
		(*) Exit immediately
		( ) Close open audio and midi devices and exit
		( ) Ignore
		[OK]
*/
LONG WINAPI ExceptionCleanupFilter(struct _EXCEPTION_POINTERS *pExceptionInfo) {
	LONG retval = EXCEPTION_CONTINUE_SEARCH;

	assert(exception_handler_player != 0);
	CExceptionDlg dlg(exception_handler_player, pExceptionInfo);
	INT_PTR result = dlg.DoModal();
	switch (result) {
		case 0:
			retval = EXCEPTION_EXECUTE_HANDLER;
			break;
		case 1:
			//_Module.midiDriver.closeAllDevices();
			//_Module.midiDriver.close();
			//_Module.driver.destroyDevice();
			_Module.RemoveMessageLoop();
			//_BuzeModule.uninitialize();
			retval = EXCEPTION_EXECUTE_HANDLER;
			break;
		case 2:
			retval = EXCEPTION_CONTINUE_SEARCH;
			break;
	}

	return retval;
}

void RedirectIOToConsole() {
	static const WORD MAX_CONSOLE_LINES = 9999;

	int hConHandle;
	intptr_t lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();
	AttachConsole(ATTACH_PARENT_PROCESS);// GetCurrentProcessId());

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// http://stackoverflow.com/questions/311955/redirecting-cout-to-a-console-in-windows
	// Redirect the CRT standard input, output, and error handles to the console
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	//Clear the error state for each of the C++ standard stream objects. We need to do this, as
	//attempts to access the standard streams before they refer to a valid target will cause the
	//iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
	//to always occur during startup regardless of whether anything has been read from or written to
	//the console or not.
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
}

void InitializeConsole() {
	RedirectIOToConsole();
	CFileVersionInfo version;
	version.Open();
	cout << version.GetProductName() << " " << version.GetProductVersion() << ", "
		<< ZZUB_SIGNATURE << ", "
		<< version.GetLegalCopyright() << endl;
}

bool InitializePlayer(buze_application_t* app, zzub_player_t* player) {

	CBuzeConfiguration configuration = buze_application_get_configuration(app);
	std::string vstpaths = configuration->getVstPaths();
	if (!vstpaths.empty()) {
		cout << "overriding default vst2zzub paths: " << vstpaths << endl;
		zzub_plugincollection_t* vstcollection = zzub_plugincollection_get_by_uri(player, "@zzub.org/vst");
		assert(vstcollection != 0);
		zzub_plugincollection_configure(vstcollection, "vstpaths", vstpaths.c_str());
	}
		
	if (zzub_player_initialize(player, 0) == -1) {
		return false;
	}

	return true;
}

int zzub_init_callback(zzub_player_t* player, zzub_plugin_t* machine, zzub_event_data_t* data, void* tag) {
	buze_application_t* app = (buze_application_t*)tag;
	if (data->type == zzub_event_type_user_alert && data->alert.type == zzub_alert_type_enumerating_plugins) {
		std::stringstream strm;
		const char* collname = zzub_plugincollection_get_name(data->alert.collection);
		strm << "Enumerating ";
		if (collname) strm << collname << " ";
		strm << "plugins...";
		buze_application_set_wait_text(app, strm.str().c_str());
	} else
	if (data->type == zzub_event_type_user_alert && data->alert.type == zzub_alert_type_enumerating_plugins_done) {
		buze_application_set_wait_text(app, "Initializing project...");
	}
	return -1; // -1 = false, did nothing
}

void SetBuzzPath() {
	// set buzz registry before any machines are loaded
	CRegKey buzzPath;
	LONG err = buzzPath.Open(HKEY_CURRENT_USER, "Software\\Jeskola\\Buzz\\Settings");
	// create key if it doesnt exist
	if (err) {
		buzzPath.Create(HKEY_CURRENT_USER, "Software\\Jeskola\\Buzz\\Settings");
	}
	if (!err) {
		char szPath[MAX_PATH];
		GetModuleFileName(_Module.m_hInst, szPath, MAX_PATH);
		*strrchr(szPath, '\\') = '\0';
		strcat(szPath, "\\");
		buzzPath.SetStringValue("BuzzPath", szPath);
		buzzPath.Close();
	}
}

class CBuzeMessageLoopImpl : public CModuleMessageLoop {
public:
	CMessageLoop* pLoop;

	int Run() {
		return pLoop->Run();
	}

	virtual BOOL AddMessageFilter(CMessageFilter* pFilter) {
		return pLoop->AddMessageFilter(pFilter);
	}

	virtual BOOL RemoveMessageFilter(CMessageFilter* pFilter) {
		return pLoop->RemoveMessageFilter(pFilter);
	}

	virtual BOOL AddIdleHandler(CIdleHandler* pHandler) {
		return pLoop->AddIdleHandler(pHandler);
	}

	virtual BOOL RemoveIdleHandler(CIdleHandler* pHandler) {
		return pLoop->RemoveIdleHandler(pHandler);
	}
};

class CBuzeHostModuleImpl : public CHostModule {
public:
	std::map<DWORD, CBuzeMessageLoopImpl*> msgLoops;

	virtual CModuleMessageLoop* GetMessageLoop() {
		DWORD dwThreadID = ::GetCurrentThreadId();

		std::map<DWORD, CBuzeMessageLoopImpl*>::iterator itMsgLoop = msgLoops.find(dwThreadID);
		if (itMsgLoop != msgLoops.end())
			return itMsgLoop->second;

		CMessageLoop* pLoop = _Module.GetMessageLoop();
		if (pLoop == 0) return 0;

		CBuzeMessageLoopImpl* pLoopImpl = new CBuzeMessageLoopImpl();
		pLoopImpl->pLoop = pLoop;
		msgLoops[dwThreadID] = pLoopImpl;
		return pLoopImpl;
	}

	virtual CModuleMessageLoop* AddMessageLoop() {
		CMessageLoop* pLoop = new CMessageLoop();

		if (!_Module.AddMessageLoop(pLoop)) {
			delete pLoop;
			return 0;
		}

		DWORD dwThreadID = ::GetCurrentThreadId();
		CBuzeMessageLoopImpl* pLoopImpl = new CBuzeMessageLoopImpl();
		pLoopImpl->pLoop = pLoop;
		msgLoops[dwThreadID] = pLoopImpl;
		return pLoopImpl;
	}

	virtual void RemoveMessageLoop() {
		DWORD dwThreadID = ::GetCurrentThreadId();

		std::map<DWORD, CBuzeMessageLoopImpl*>::iterator itMsgLoop = msgLoops.find(dwThreadID);
		if (itMsgLoop != msgLoops.end()) {
			delete itMsgLoop->second->pLoop;
			delete itMsgLoop->second;
			msgLoops.erase(itMsgLoop);
		}

		_Module.RemoveMessageLoop();
	}

};

typedef CHostModule buze_host_module_t;

HANDLE hSplashEvent;

DWORD WINAPI SplashWindowProc(LPVOID lpParam) {

	new CSplashWnd(IDR_SPLASH, 10000, 0, RGB(0x80, 0, 0xFF), true);

	CMessageLoop msgloop;
	_Module.AddMessageLoop(&msgloop);

	SetEvent(hSplashEvent);

	int result = msgloop.Run();

	_Module.RemoveMessageLoop();

	return (DWORD)result;
}

std::tstring GetAppPath(HINSTANCE hInstance) {
	TCHAR path[MAX_PATH + 32] = { 0 };
	::GetModuleFileName(hInstance, path, MAX_PATH);
	
	TCHAR* filePart = _tcsrchr(path, _T('\\'));
	if (filePart != 0) {
		filePart[0] = 0;
		return path;
	}
	return _T("");
}

std::tstring GetUserPath(HINSTANCE hInstance) {
	// http://en.wikipedia.org/wiki/Environment_variable
	 // USERPROFILE should only be used in dialogs that allow user to choose
	// between paths like Documents/Pictures/Downloads/Music, for programmatic
	// purposes APPDATA (roaming), LOCALAPPDATA or PROGRAMDATA (shared between
	// users) is used.
	char* path = getenv("APPDATA");
	if (path != 0) {
		std::tstring result = std::tstring(path, path + strlen(path));
		return result + "\\Buze";
	}
	
	// ??
	path = getenv("USERPROFILE");
	if (path != 0) {
		std::tstring result = std::tstring(path, path + strlen(path));
		return result + "\\.buze";
	}
	return _T("");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow) {

	forcetls[0] = 0;

	// linker flags should indicate /NXCOMPAT:NO to support plugins compiled with older atl/wtls.
	// the default /NXCOMPAT:YES enables "full DEP" and disallows all forms of override-per-process.
	// SetProcessDEPPolicy() enables DEP and tells windows to emulate ATL thunks.
	// can add PROCESS_DEP_DISABLE_ATL_THUNK_EMULATION to get "full DEP"
	SetProcessDEPPolicy(PROCESS_DEP_ENABLE);

	std::tstring appPath = GetAppPath(hInstance);
	std::tstring userPath = GetUserPath(hInstance);
	std::string tempPath = boost::filesystem::temp_directory_path().string();

	// no longer neccessary when buze_application_map_path actually works:
	//if (!appPath.empty())
	//	SetCurrentDirectory(appPath.c_str());

	HRESULT hRes = OleInitialize(NULL);	// for opy and paste
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_WIN95_CLASSES/*|ICC_USEREX_CLASSES*/);

	hRes = _Module.Init(NULL, hInstance, &LIBID_ATLLib);
	ATLASSERT(SUCCEEDED(hRes));

	SetBuzzPath();
	
	// hack: better control of python paths by clearing the PYTHONPATH before pythonXX.dll is loaded via pythonview.dll
	SetEnvironmentVariable(_T("PYTHONPATH"), NULL);

	bool cmdline_debug = strstr(lpstrCmdLine, "/debug") != 0;
	bool cmdline_nosplash = strstr(lpstrCmdLine, "/nosplash") != 0;
	bool cmdline_maximize = strstr(lpstrCmdLine, "/maximize") != 0;

	if (cmdline_debug)
		InitializeConsole();

	//_BuzeModule.initialize(hInstance, true);
	//_BuzeModule.setBuzzPath();

	CBuzeHostModuleImpl module;

	buze_application_t* app = buze_application_create(&module, appPath.c_str(), userPath.c_str(), tempPath.c_str());
	zzub_player_t* player = zzub_player_create(appPath.c_str(), userPath.c_str(), tempPath.c_str());

	if (!cmdline_nosplash) {
		hSplashEvent = CreateEvent(0, FALSE, FALSE, "splashev");
		DWORD dwThread;
		HANDLE hThread = CreateThread(0, 0, SplashWindowProc, 0, 0, &dwThread);	
		CloseHandle(hThread);
		WaitForSingleObject(hSplashEvent, INFINITE);
		CloseHandle(hSplashEvent);

		buze_application_show_wait_window(app);
		buze_application_set_wait_text(app, "Initializing player...");
		zzub_player_add_callback(player, &zzub_init_callback, app);
	}

	if (!InitializePlayer(app, player)) {
		return 4;
	}

	if (!cmdline_nosplash)
		buze_application_set_wait_text(app, "Initializing audio...");

	zzub_audiodriver_t* driver = zzub_audiodriver_create(player);
	//zzub_audiodriver_t* driver = zzub_audiodriver_create_rtaudio(player);
	assert(driver != 0); // the requested api wasnt compiled with armstrong

	buze_application_initialize(app, player, driver);

	buze_application_open_midi_devices_from_config(app);

	if (!buze_application_create_audio_device_from_config(app)) {
		MessageBox(GetDesktopWindow(), "Could not enable an audio device. Please select another driver in Preferences.", programName, MB_OK);
		//_BuzeModule.wantsAudioPreferences = true;
	}

	if (!cmdline_nosplash) {
		zzub_player_remove_callback(player, &zzub_init_callback, 0);
		buze_application_set_wait_text(app, "Initializing user interface...");
	}

	CMessageLoop msgLoop;
	_Module.AddMessageLoop(&msgLoop);

	CMainFrame mainframe;// = new CMainFrame();
	mainframe.application = app;
	mainframe.configuration = buze_application_get_configuration(app);
	mainframe.player = player;
	mainframe.isStandalone = true;

	mainframe.CreateFrame(0);
	if( cmdline_maximize ) mainframe.ShowWindow(SW_SHOW|WS_MAXIMIZE);

	// CMainFrame must inherit from CViewFrame, typedeffed to buze_main_frame_t
	// this lets the c api call back into the host for these calls
	//buze_main_frame_initialize(mainframe, 0, player);

	HWND hMainFrameWnd = (HWND)buze_main_frame_get_wnd(&mainframe);

	if (hMainFrameWnd == 0)
		return 5;

	if (!cmdline_nosplash) {
		buze_application_hide_wait_window(app, hMainFrameWnd);
	}

#if defined(ENABLE_CRASHDIALOG)
	::SetUnhandledExceptionFilter(ExceptionCleanupFilter);
#endif

	int ret = msgLoop.Run();

	buze_main_frame_destroy(&mainframe);

	_Module.RemoveMessageLoop();
	zzub_mididriver_close_all(player);
	zzub_audiodriver_destroy((zzub_audiodriver_t*)buze_application_get_audio_driver(app));
	zzub_player_destroy(player);

	//_BuzeModule.uninitialize();
	buze_application_destroy(app);

	_Module.Term();
	::CoUninitialize();

	return ret;
}
