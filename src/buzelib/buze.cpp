#include <algorithm>
using std::min;
using std::max;

#define NOMINMAX
#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cassert>
#include <zzub/zzub.h>
#include <buze/View.h>
#include <buze/ViewFrame.h>
#include <buze/HostModule.h>
#include "resource.h"
#include "Configuration.h"
#include "WaitWindow.h"
#include "MachineIndex.h"
#include <buze/WtlDllModule.h>

extern CHostDllModule _Module;

class CBuzeConfiguration;

#define NO_BUZE_MAIN_FRAME_TYPE
typedef CViewFrame buze_main_frame_t;

#define NO_BUZE_WINDOW_FACTORY_TYPE
typedef CViewInfo buze_window_factory_t;

#define NO_BUZE_WINDOW_TYPE
typedef CView buze_window_t;

#define NO_BUZE_EVENT_HANDLER_TYPE
typedef CEventHandler buze_event_handler_t;

#define NO_BUZE_DOCUMENT_TYPE
typedef CDocument buze_document_t;

#define NO_BUZE_CONFIGURATION_TYPE
typedef CConfiguration buze_configuration_t;

#define NO_BUZE_APPLICATION_TYPE
typedef CApplication buze_application_t;

#define NO_BUZE_PLUGIN_INDEX_ITEM_TYPE
typedef IndexItem buze_plugin_index_item_t;

#define NO_BUZE_HOST_MODULE_TYPE
typedef CHostModule buze_host_module_t;

#include <buze/buze.h>
#include "BuzeConfiguration.h"
#include "ConfigurationImpl.h"
#include "Document.h"
#include "Application.h"

CHostDllModule _Module;

#include <boost/lexical_cast.hpp>

void ParseFileName(const std::string& fullname, std::string* filename, int* instrument, int* sample) {
// the filename must end with zero, one or two numeric directories at the end, preceded by a filename with a .

	// find last part of filename
	std::string::size_type ls = fullname.find_last_of("/\\");
	if (ls == std::string::npos) {
		// no slashes, just return the filename
		*instrument = -1;
		*sample = -1;
		*filename = fullname;
		return ;
	}

	std::string lastpart = fullname.substr(ls + 1);
	int lastint;
	try {
		lastint = boost::lexical_cast<int, std::string>(lastpart);
	} catch (boost::bad_lexical_cast& e) {
		// not a numeric part at the end
		e ;
		*instrument = -1;
		*sample = -1;
		*filename = fullname;
		return ;
	}

	std::string::size_type sls = fullname.find_last_of("/\\", ls - 1);
	if (sls == std::string::npos) {
		// input is a slash and a single numeric part. strange, but still cool:
		*instrument = -1;
		*sample = -1;
		*filename = fullname;
		return ;
	}

	std::string secondlastpart = fullname.substr(sls + 1, (ls - sls) - 1);
	int secondlastint;
	try {
		secondlastint = boost::lexical_cast<int, std::string>(secondlastpart);
	} catch (boost::bad_lexical_cast& e) {
		// just one numeric identifier
		e ;
		*instrument = lastint;
		*sample = -1;
		*filename = fullname.substr(0, ls);
		return ;
	}

	*instrument = secondlastint;
	*sample = lastint;
	*filename = fullname.substr(0, sls);
}

/* class Application */

buze_application_t *buze_application_create(buze_host_module_t* module, const char* globalPath, const char* userPath, const char* tempPath) {
	CApplication* app = new CApplication();
	_Module.m_hostModule = module; // this initializes buzelib.dll's hostModule
	app->initialize(0, true);
	app->globalPath = globalPath;
	app->userPath = userPath;
	app->tempPath = tempPath;
	return app;
}

buze_host_module_t* buze_application_get_host_module(buze_application_t *application) {
	return _Module.m_hostModule;
}

void buze_application_destroy(buze_application_t *application) {
	application->uninitialize();
}

void buze_main_frame_register_window_factory(buze_main_frame_t* mainframe, buze_window_factory_t* info) {
	mainframe->RegisterViewInfo(info);
}

void buze_application_initialize(buze_application_t *application, zzub_player_t* player, zzub_audiodriver_t* driver) {
	application->player = (zzub_player_t*)player;
	application->driver = (zzub_audiodriver_t*)driver;
	//return 0;
}

int buze_application_create_audio_device_from_config(buze_application_t *application) {
	return application->setAudioDriver();
}

int buze_application_create_audio_device(buze_application_t *application, const char *outdevicename, const char *indevicename, int rate, int buffersize, int masterchannel, int save) {
	return application->setAudioDriver(outdevicename, indevicename, rate, buffersize, masterchannel, save ? true:false) ? 1 : 0;
}

void buze_application_enable_silent_driver(buze_application_t *application, int enable) {
	int samplerate = zzub_audiodriver_get_samplerate(application->driver);
	if (enable)
		application->setSilentAudioDriver(samplerate);
	else
		application->releaseSilentAudioDriver();
}

zzub_audiodriver_t* buze_application_get_audio_driver(buze_application_t *application) {
	return application->driver;
}

void buze_application_release_audio_driver(buze_application_t *application) {
	assert(false);
}

int buze_application_open_midi_devices_from_config(buze_application_t *application) {
	return application->openMidiDevices();
}

void buze_application_show_wait_window(buze_application_t *application) {
	application->showWaitWindow();
}

void buze_application_set_wait_text(buze_application_t *application, const char *text) {
	application->setWaitWindowText(text);
}

void buze_application_hide_wait_window(buze_application_t *application, void *focusWnd) {
	application->hideWaitWindow((HWND)focusWnd);
}

buze_configuration_t* buze_application_get_configuration(buze_application_t* application) {
	return application->configuration.configuration;
}

const char *buze_application_map_path(buze_application_t *application, const char *path, int type) {
	static char pathstr[MAX_PATH];
	std::string str = application->mapPath(path, type);
	strcpy(pathstr, str.c_str());
	return pathstr;
}

unsigned int buze_application_get_theme_color(buze_application_t *application, const char *name) {
	return application->themes->getThemeColor(name);
}

int buze_application_get_theme_count(buze_application_t *application) {
	return (int)application->themes->getThemes();
}

const char* buze_application_get_theme_name(buze_application_t *application, int index) {
	static std::string str;
	str = application->themes->getThemeName(index);
	return str.c_str();
}

void buze_application_load_theme(buze_application_t *application, const char* name) {
	application->themes->loadTheme(name);
}

/* class MainFrame */

extern "C" typedef CViewLibrary* (*CREATEVIEWLIBRARY)();

bool load_dll(CViewFrame* mainFrame, std::string filename) {
	HMODULE hViewDll = LoadLibrary(filename.c_str());
	if (hViewDll) {
		CREATEVIEWLIBRARY fnCreateViewLibrary = (CREATEVIEWLIBRARY)GetProcAddress(hViewDll, "buze_create_viewlibrary");
		if (fnCreateViewLibrary) {
			CViewLibrary* viewLib = fnCreateViewLibrary();
			if (viewLib) {
				int version = viewLib->GetVersion();
				
				// version 0: 0.7.0 - 0.7.4: first version
				// version 1: 0.7.5 - x.x.x: cleanup, incompatible with 0
				if (version != CViewLibrary::version) {
					viewLib->Destroy();
					std::stringstream errormsg;
					errormsg << "Incompatible version of " << filename;
					MessageBox(GetDesktopWindow(), errormsg.str().c_str(), "Error", MB_OK);
				} else {
					viewLib->Initialize(mainFrame);
					viewLib->Destroy();
					return true;
				}
			} else {
				std::stringstream errormsg;
				errormsg << "Cannot load " << filename;
				MessageBox(GetDesktopWindow(), errormsg.str().c_str(), "Error", MB_OK);
			}
		}
		FreeLibrary(hViewDll);
	}
	return false;
}

void scan_plugins(buze_main_frame_t* mainframe, std::string const & rootpath, std::string path) {
	std::string search_path(rootpath + path);
	search_path += "*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind=::FindFirstFile(search_path.c_str(), &fd);
	while(hFind != INVALID_HANDLE_VALUE) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
				std::string relpath(path + '\\' + fd.cFileName);
				scan_plugins(mainframe, rootpath, relpath + "\\");
			}
		} else {
			char* ext = strrchr(fd.cFileName, '.');
			if (ext != 0 && stricmp(ext, ".dll") == 0) {
				char absolute_path[MAX_PATH];
				std::string relpath(path + '\\' + (std::string)fd.cFileName);
				::GetFullPathName((rootpath + relpath).c_str(), MAX_PATH, absolute_path, 0);
				load_dll(mainframe, absolute_path);
			}
		}
		if (!::FindNextFile(hFind, &fd)) break;
	}
	::FindClose(hFind);
}

int buze_main_frame_initialize(buze_main_frame_t * mainFrame, void* hParentWnd, zzub_player_t* unused) {

	mainFrame->document = new CDocument(mainFrame->application, mainFrame->player, mainFrame->application->configuration.configuration, mainFrame->application->themes);

	std::string viewPath = buze_application_map_path(mainFrame->application, "Gear\\Views\\", buze_path_type_app_path);
	scan_plugins(mainFrame, viewPath.c_str(), "");
	return 1;
}

void buze_main_frame_add_timer_handler(buze_main_frame_t *mainFrame, buze_window_t *wnd) {
	mainFrame->AddTimerHandler(wnd);
}

void buze_main_frame_remove_timer_handler(buze_main_frame_t *mainFrame, buze_window_t *wnd) {
	mainFrame->RemoveTimerHandler(wnd);
}

void buze_main_frame_viewstack_insert(buze_main_frame_t *mainFrame, buze_window_t *wnd) {
	mainFrame->ViewStackInsert(wnd->GetHwnd(), false);
}

void* buze_main_frame_get_wnd(buze_main_frame_t* mainFrame) {
	return mainFrame->GetHwnd();
}

buze_document_t *buze_main_frame_get_document(buze_main_frame_t *mainframe) {
	return mainframe->document;
}

zzub_player_t *buze_main_frame_get_player(buze_main_frame_t *mainframe) {
	return mainframe->player;
}

buze_application_t *buze_main_frame_get_application(buze_main_frame_t *mainframe) {
	return mainframe->application;
}

void buze_main_frame_destroy(buze_main_frame_t *mainframe) {
	delete mainframe->document; // copy document, must be deleted after the mainframe
	mainframe->document = 0;
}

void *buze_main_frame_get_accelerators(buze_main_frame_t *mainframe, const char *viewname) {
	return mainframe->GetAccelerators(viewname);
}

buze_window_t* buze_main_frame_get_focused_view(buze_main_frame_t* mainframe) {
	return mainframe->GetFocusedView();
}

int buze_main_frame_is_float_view(buze_main_frame_t* mainframe, buze_window_t* wnd) {
	return mainframe->IsFloatView(wnd->GetHwnd());
}

void buze_main_frame_set_focus_to(buze_main_frame_t* mainframe, buze_window_t* wnd) {
	mainframe->SetFocusTo(wnd->GetHwnd());
}

buze_window_t* buze_main_frame_get_view(buze_main_frame_t* mainframe, const char* viewname, int viewid) {
	return mainframe->GetView(viewname, viewid);
}

buze_window_t* buze_main_frame_open_view(buze_main_frame_t* mainframe, const char* viewname, const char* label, int viewid, int x, int y) {
	return mainframe->OpenView(viewname, label, viewid, x, y);
}

void buze_main_frame_close_view(buze_main_frame_t* mainframe, buze_window_t* wnd) {
	return mainframe->CloseView(wnd->GetHwnd());
}

const char* buze_main_frame_get_open_filename(buze_main_frame_t* mainframe) {
	return mainframe->GetOpenFileName();
}

const char* buze_main_frame_get_save_filename(buze_main_frame_t* mainframe) {
	return mainframe->GetSaveFileName();
}

void* buze_main_frame_get_plugin_menu_create(buze_main_frame_t* mainframe) {
	return mainframe->GetMachineMenuCreate();
}

void* buze_main_frame_get_plugin_menu_insert_after(buze_main_frame_t* mainframe) {
	return mainframe->GetMachineMenuInsertAfter();
}

void* buze_main_frame_get_plugin_menu_insert_before(buze_main_frame_t* mainframe) {
	return mainframe->GetMachineMenuInsertBefore();
}

void* buze_main_frame_get_plugin_menu_replace(buze_main_frame_t* mainframe) {
	return mainframe->GetMachineMenuReplace();
}

void* buze_main_frame_get_main_menu(buze_main_frame_t* mainframe) {
	return mainframe->GetMainMenu();
}

void buze_main_frame_add_menu_keys(buze_main_frame_t* mainframe, const char* viewname, void* menu) {
	mainframe->AddMenuKeys(viewname, (HMENU)menu);
}

int buze_main_frame_register_event(buze_main_frame_t* mainframe) {
	return mainframe->RegisterEvent();
}

unsigned short buze_main_frame_register_accelerator_event(buze_main_frame_t* mainframe, const char* viewname, const char* defaulthotkey, int evcode) {
	return mainframe->RegisterAcceleratorEvent(viewname, defaulthotkey, evcode);
}

void buze_main_frame_register_accelerator(buze_main_frame_t* mainframe, const char* viewname, const char* name, const char* defaulthotkey, unsigned short id) {
	mainframe->RegisterAccelerator(viewname, name, defaulthotkey, id);
}

void buze_main_frame_show_plugin_parameters(buze_main_frame_t* mainframe, zzub_plugin_t* p, int modehint, int x, int y) {
	mainframe->ShowMachineParameters(p, (MachineParameterViewMode)modehint, x, y);
}

void* buze_main_frame_get_keyjazz_map(buze_main_frame_t* mainframe) {
	return mainframe->GetKeyjazzMap();
}

buze_window_t* buze_main_frame_get_view_by_wnd(buze_main_frame_t* mainframe, void* wnd) {
	return mainframe->GetViewByHwnd((HWND)wnd);
}

const char* buze_main_frame_get_program_name(buze_main_frame_t* mainframe) {
	return mainframe->GetProgramName();
}

void* buze_window_get_wnd(buze_window_t* window) {
	return window->GetHwnd();
}

int buze_configuration_get_toolbars_locked(buze_configuration_t* configuration) {
	CBuzeConfiguration c(configuration);
	return c->getLockedToolbars();
}

/* class Document */

class CCallbackEventHandler : public CEventHandler {
public:
	static std::vector<CCallbackEventHandler*> handlers;

	void* tag;
	buze_callback_t callback;
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
		callback(pSender, (int)lHint, pHint, tag);
	}
};

std::vector<CCallbackEventHandler*> CCallbackEventHandler::handlers;

void buze_document_add_callback(buze_document_t* document, buze_callback_t callback, void* tag) {
	CCallbackEventHandler* handler = new CCallbackEventHandler();
	handler->callback = callback;
	handler->tag = tag;
	CCallbackEventHandler::handlers.push_back(handler);
	document->addView(handler);
}

void buze_document_remove_callback(buze_document_t* document, buze_callback_t callback, void* tag) {
	for (std::vector<CCallbackEventHandler*>::iterator i = CCallbackEventHandler::handlers.begin(); i != CCallbackEventHandler::handlers.end(); ++i) {
		if ((*i)->callback == callback && (*i)->tag == tag) {
			document->removeView(*i);
			CCallbackEventHandler::handlers.erase(i);
			return ;
		}
	}
}

void buze_document_add_view(buze_document_t *document, buze_event_handler_t *view) {
	document->addView(view);
}

void buze_document_remove_view(buze_document_t *document, buze_event_handler_t *view) {
	document->removeView(view);
}

void buze_document_notify_views(buze_document_t *document, buze_window_t *sender, int hint, void *param) {
	document->updateAllViews(sender, hint, param);
}

void buze_document_set_plugin_non_song(buze_document_t *document, zzub_plugin_t* plugin, int state) {
	document->setMachineNonSong((zzub_plugin_t*)plugin, state?TRUE:FALSE);
}

int buze_document_get_plugin_non_song(buze_document_t *document, zzub_plugin_t* plugin) {
	return document->getMachineNonSong((zzub_plugin_t*)plugin);
}

void buze_document_set_plugin_parameter_view_mode(buze_document_t *document, zzub_plugin_t* plugin, int mode) {
	PLUGININFO& pi = document->getPluginData((zzub_plugin_t*)plugin);
	pi.paramViewMode = mode;
}

int buze_document_get_plugin_parameter_view_mode(buze_document_t *document, zzub_plugin_t* plugin) {
	PLUGININFO& pi = document->getPluginData((zzub_plugin_t*)plugin);
	return pi.paramViewMode;
}

void buze_document_set_plugin_last_preset(buze_document_t *document, zzub_plugin_t* plugin, const char* preset) {
	document->setMachinePreset((zzub_plugin_t*)plugin, preset);
}

const char* buze_document_get_plugin_last_preset(buze_document_t *document, zzub_plugin_t* plugin) {
	static std::string str;
	str = document->getMachinePreset((zzub_plugin_t*)plugin);
	return str.c_str();
}

zzub_player_t* buze_document_get_player(buze_document_t* document) {
	return document->player;
}

int buze_document_get_octave(buze_document_t* document) {
	return document->octave;
}

void buze_document_set_octave(buze_document_t* document, int octave) {
	document->octave = octave;
}

int buze_document_is_dirty(buze_document_t* document) {
	return document->isDirty();
}

buze_configuration_t* buze_document_get_configuration(buze_document_t* document) {
	return document->configuration.configuration;
}

const char *buze_configuration_get_fixed_width_font(buze_configuration_t *configuration) {
	static char value[1024];
	strcpy(value, "Fixedsys");
	configuration->getConfigString("Settings", "FixedWidthFont", value);
	return value;
}

void buze_configuration_add_sample_path(buze_configuration_t *configuration, const char *path) {
	CBuzeConfiguration c(configuration);
	c->addSamplePath(path);
}

void buze_configuration_remove_sample_path(buze_configuration_t *configuration, int index) {
	CBuzeConfiguration c(configuration);
	c->removeSamplePath(index);
}

int buze_configuration_get_sample_path_count(buze_configuration_t *configuration) {
	CBuzeConfiguration c(configuration);
	return c->getNumSamplePaths();
}

const char *buze_configuration_get_sample_path(buze_configuration_t *configuration, int index) {
	static std::string str;
	CBuzeConfiguration c(configuration);
	str = c->getSamplePath(index);
	return str.c_str();
}

const char *buze_document_get_stream_plugin_uri_for_file(buze_document_t *document, const char *path) {
	std::string filepart;
	int instrument, sample;
	ParseFileName(path, &filepart, &instrument, &sample);

	static std::string str;
	str = document->getStreamPluginUriForFile(filepart);
	return str.c_str();
}

void buze_document_play_plugin_note(buze_document_t *document, zzub_plugin_t* plugin, int note, int prevnote) {
	document->playMachineNote((zzub_plugin_t*)plugin, note, prevnote);
}

void buze_document_play_stream(buze_document_t *document, int note, int offset, int length, const char *pluginuri, const char *dataurl) {
	document->playStream(note, offset, length, pluginuri, dataurl);
}

void buze_document_keyjazz_key_down(buze_document_t *document, zzub_plugin_t* plugin, int wParam, int note) {
	document->keyjazzKeyDown((zzub_plugin_t*)plugin, wParam, note);
}

/*bool*/ int buze_document_keyjazz_key_up(buze_document_t *document, int wParam, int *note, zzub_plugin_t** plugin) {
	KEYJAZZNOTE kjn = document->keyjazzKeyUp(wParam);
	*note = kjn.note;
	*plugin = kjn.plugin;
	return kjn.plugin != 0;
}

void buze_document_keyjazz_release(buze_document_t *document, int sendnoteoffs) {
	document->keyjazzRelease(sendnoteoffs?true:false);
}

zzub_plugin_t* buze_document_get_stream_plugin(buze_document_t *document) {
	return document->streamplayer;
}

void buze_document_delete_stream_plugin(buze_document_t* document) {
	document->deleteStreamPlayer();
}

zzub_plugin_t* buze_document_get_solo_plugin(buze_document_t *document) {
	return document->soloplugin;
}

void buze_document_set_solo_plugin(buze_document_t *document, zzub_plugin_t* plugin, int state) {
	document->setMachineSolo((zzub_plugin_t*)plugin, state);
}

const char *buze_document_get_plugin_helpfile(buze_document_t *document, zzub_pluginloader_t* loader) {
	static std::string helpfile;
	helpfile = document->getMachineHelpFile((zzub_pluginloader_t*)loader);
	return helpfile.c_str();
}

/*bool*/ int buze_document_import_song(buze_document_t *document, const char *filename, int flags, float x, float y, char *errormessages, int outsize) {
	std::string errormsg;
	bool result = document->importSong(filename, flags, x, y, &errormsg);
	strncpy(errormessages, errormsg.c_str(), outsize);
	return result;
}

void buze_document_clear_song(buze_document_t *document) {
	document->clearSong();
}

int buze_document_save_file(buze_document_t *document, const char* filename, int withwaves) {
	return document->saveFile(filename, withwaves?true:false);
}

void buze_document_create_default_document(buze_document_t *document) {
	document->createDefaultDocument();
}

zzub_pattern_format_t* buze_document_create_default_format(buze_document_t* document, zzub_plugin_t* plugin, int simple) {
	return document->createDefaultFormat(plugin, simple != 0);
}

zzub_plugin_t* buze_document_create_plugin(buze_document_t* document, const char* uri, const char* instrumentname, float x, float y, zzub_plugin_group_t* plugingroup) {
	return document->createMachine(uri, instrumentname, x, y, plugingroup);
}

void buze_document_extend_pattern_format(buze_document_t* document, zzub_pattern_format_t* format, zzub_plugin_t* plugin, int simple) {
	document->extendPatternFormat(format, plugin, simple != 0);
}

void zzub_audio_connection_set_amp(zzub_plugin_t* connplug, int amp, bool with_undo) {
	int tracks = zzub_plugin_get_track_count(connplug, 2);
	for (int i = 0; i < tracks; i++) {
		if (with_undo)
			zzub_plugin_set_parameter_value(connplug, 2, i, 0, amp, true);
		else
			zzub_plugin_set_parameter_value_direct(connplug, 2, i, 0, amp, true);
	}
}

int zzub_audio_connection_get_amp(zzub_plugin_t* connplug) {
	int tracks = zzub_plugin_get_track_count(connplug, 2);
	
	// during ordinary use, tracks should never be 0 here, but some plugins (pvst) might send us redraw messages while there are incomplete connections, and this was the simplest fix:
	if (tracks == 0) return 0;

	int totamp = 0;
	for (int i = 0; i < tracks; i++) {
		totamp += zzub_plugin_get_parameter_value(connplug, 2, i, 0);
	}
	return totamp / tracks;
}

// Used by "Insert", "Insert Before", "Insert After", "Replace", "Smart Delete"
// This function is necessary because if we don't do zzub_plugin_set_audio_connection_channels, then the connection channels will be invalid (press TAB to test)
// It tries to be smart and preserve output/input indexes. Limited to 2 channels to keep it simple.
// For example, if a machine's outputs 6-7 were connected to an effect, and then we used "Insert Before", the machine's outputs 6-7 would be connected to the newly inserted effect.
bool ConnectMachinesAudioCompatible(zzub_plugin_t* from_plugin, zzub_plugin_t* to_plugin, zzub_connection_t* old_out, zzub_connection_t* old_in, int vol, int pan) {
	// if a connection already exists, fail -- needed or else the new connection becomes corrupted
	// could happen with "Smart Delete"
	zzub_connection_t* conn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (conn) {
		return false;
	}

	// old out info
	int out_first_out;
	int out_first_in;
	int out_outs;
	int out_ins;
	//int out_from_channels;
	//int out_to_channels;
	//zzub_plugin_t* out_from_plugin;
	//zzub_plugin_t* out_to_plugin;
	if (old_out) {
		//out_from_plugin = zzub_connection_get_from_plugin(old_out);
		//out_to_plugin = zzub_connection_get_to_plugin(old_out);
		//zzub_pluginloader_t* out_from_info = zzub_plugin_get_pluginloader(out_from_plugin);
		//zzub_pluginloader_t* out_to_info = zzub_plugin_get_pluginloader(out_to_plugin);
		//out_from_channels = zzub_pluginloader_get_output_channel_count(out_from_info);
		//out_to_channels = zzub_pluginloader_get_input_channel_count(out_to_info);
		out_first_in = zzub_connection_get_first_input(old_out);
		out_first_out = zzub_connection_get_first_output(old_out);
		out_ins = zzub_connection_get_input_count(old_out);
		out_outs = zzub_connection_get_output_count(old_out);
		//zzub_connection_get_audio_connection_channels(old_out, &out_first_in, &out_first_out, &out_ins, &out_outs, &out_flags);
	}

	// old in info
	int in_first_out;
	int in_first_in;
	int in_outs;
	int in_ins;
	//int in_from_channels;
	//int in_to_channels;
	//zzub_plugin_t* in_from_plugin;
	//zzub_plugin_t* in_to_plugin;
	if (old_in) {
		//in_from_plugin = zzub_connection_get_from_plugin(old_in);
		//in_to_plugin = zzub_connection_get_to_plugin(old_in);
		//zzub_pluginloader_t* in_from_info = zzub_plugin_get_pluginloader(in_from_plugin);
		//zzub_pluginloader_t* in_to_info = zzub_plugin_get_pluginloader(in_to_plugin);
		//in_from_channels = zzub_pluginloader_get_output_channel_count(in_from_info);
		//in_to_channels = zzub_pluginloader_get_input_channel_count(in_to_info);
		in_first_in = zzub_connection_get_first_input(old_in);
		in_first_out = zzub_connection_get_first_output(old_in);
		in_ins = zzub_connection_get_input_count(old_in);
		in_outs = zzub_connection_get_output_count(old_in);
		//zzub_connection_get_audio_connection_channels(old_in, &in_first_in, &in_first_out, &in_ins, &in_outs, &in_flags);
	}

	// new info
	int new_first_out;
	int new_first_in;
	int new_outs;
	int new_ins;
	int new_flags;
	zzub_pluginloader_t* new_from_info = zzub_plugin_get_pluginloader(from_plugin);
	zzub_pluginloader_t* new_to_info = zzub_plugin_get_pluginloader(to_plugin);
	int new_from_channels = zzub_pluginloader_get_output_channel_count(new_from_info);
	int new_to_channels = zzub_pluginloader_get_input_channel_count(new_to_info);

	// set first_out, outs
	if (old_out) {
		new_first_out = out_first_out;
		new_outs = std::min(out_outs, 2);
	} else {
		new_first_out = 0;
		new_outs = std::min(new_from_channels, 2);
	}

	// set first_in, ins
	if (old_in) {
		new_first_in = in_first_in;
		new_ins = std::min(in_ins, 2);
	} else {
		new_first_in = 0;
		new_ins = std::min(new_to_channels, 2);
	}

	// set flags
	if ((new_outs == 1) && (new_ins == 1)) { // mono to mono
		new_flags = 0;
	} else
	if ((new_outs == 1) && (new_ins == 2)) { // mono to stereo
		new_flags = 1;
	} else
	if ((new_outs == 2) && (new_ins == 1)) { // stereo to mono <-- no stereo to mono flag? could be '2'
		//new_outs = 1; // do we need this?
		new_flags = 0;
	} else
	if ((new_outs == 2) && (new_ins == 2)) { // stereo to stereo
		new_flags = 0;
	} else { // no connection can be made
		return false;
	}

	// finally
	conn = zzub_plugin_create_audio_connection(to_plugin, from_plugin, new_first_in, new_ins, new_first_out, new_outs);
	if (conn != 0) {
		zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(conn);
		zzub_audio_connection_set_amp(connplug, vol, true);
	}

	//ConnectMachinesAudio(from_plugin, to_plugin, vol, new_first_in, new_ins, new_first_out, new_outs);
	//zzub_plugin_set_audio_connection_channels(to_plugin, from_plugin, new_first_in, new_first_out, new_ins, new_outs, new_flags);
	return true;
}

zzub_plugin_t* buze_document_create_plugin_between(buze_document_t* document, zzub_plugin_t* to_plugin, zzub_plugin_t* from_plugin, const char* uri, const char* instrumentname) {

	float from_x = zzub_plugin_get_position_x(from_plugin);
	float from_y = zzub_plugin_get_position_y(from_plugin);
	float to_x = zzub_plugin_get_position_x(to_plugin);
	float to_y = zzub_plugin_get_position_y(to_plugin);
	float midx = to_x + (from_x - to_x) / 2;
	float midy = to_y + (from_y - to_y) / 2;

	zzub_plugin_group_t* plugingroup = zzub_plugin_get_plugin_group(from_plugin);

	zzub_plugin_t* plugin = buze_document_create_plugin(document, uri, instrumentname, midx, midy, plugingroup);

	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	zzub_connection_t* midiconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_midi);
	zzub_connection_t* eventconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_event);

	if (audioconn != 0) {
		zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(audioconn);
		int inputVolume = zzub_audio_connection_get_amp(connplug);

		if (zzub_plugin_get_flags(plugin) & zzub_plugin_flag_has_audio_input) {
			ConnectMachinesAudioCompatible(from_plugin, plugin, 0, 0, 0x4000, 0x4000);
		}

		if (zzub_plugin_get_flags(plugin) & zzub_plugin_flag_has_audio_output) {
			ConnectMachinesAudioCompatible(plugin, to_plugin, 0, audioconn, inputVolume, 0);
		}

		zzub_connection_destroy(audioconn);

		//zzub_player_history_commit(player, 0, 0, "Insert Plugin");
	}

	return plugin;
}

zzub_plugin_t* buze_document_create_plugin_before(buze_document_t* document, zzub_plugin_t* plugin, const char* uri, const char* instrumentname) {

	float ox = zzub_plugin_get_position_x(plugin);
	float oy = zzub_plugin_get_position_y(plugin);

	float x_offset = -0.1f;
	float y_offset = -0.1f;

	zzub_plugin_group_t* plugingroup = zzub_plugin_get_plugin_group(plugin);
	zzub_plugin_t* new_plugin = buze_document_create_plugin(document, uri, instrumentname, ox + x_offset, oy + y_offset, plugingroup);

	if (zzub_plugin_get_flags(new_plugin) & zzub_plugin_flag_has_audio_input) {
		std::vector<zzub_connection_t*> in_conns;

		int in_count = zzub_plugin_get_input_connection_count(plugin);
		for (int i = 0; i < in_count; ++i) {
			zzub_connection_t* audioconn_in = zzub_plugin_get_input_connection(plugin, i);
			zzub_plugin_t* in_plugin = zzub_connection_get_from_plugin(audioconn_in);
			assert(audioconn_in);
			assert(in_plugin);

			if (zzub_connection_get_type(audioconn_in) != zzub_connection_type_audio) continue;

			in_conns.push_back(audioconn_in);

			zzub_plugin_t* connplug_in = zzub_connection_get_connection_plugin(audioconn_in);
			int inputVolume = zzub_audio_connection_get_amp(connplug_in);

			ConnectMachinesAudioCompatible(in_plugin, new_plugin, audioconn_in, 0, inputVolume, 0);
		}

		// disconnect them in separate loop otherwise we invalidate the previous loop range
		for (std::vector<zzub_connection_t*>::iterator i = in_conns.begin(); i != in_conns.end(); ++i) {
			zzub_connection_destroy(*i);
		}
	}

	if ((zzub_plugin_get_flags(plugin) & zzub_plugin_flag_has_audio_input) &&
		(zzub_plugin_get_flags(new_plugin) & zzub_plugin_flag_has_audio_output)) {
		ConnectMachinesAudioCompatible(new_plugin, plugin, 0, 0, 0x4000, 0x4000);
	}
	return new_plugin;
}

zzub_plugin_t* buze_document_create_plugin_after(buze_document_t* document, zzub_plugin_t* plugin, const char* uri, const char* instrumentname) {

	float ox = zzub_plugin_get_position_x(plugin);
	float oy = zzub_plugin_get_position_y(plugin);

	float x_offset = +0.1f;
	float y_offset = +0.1f;

	zzub_plugin_group_t* plugingroup = zzub_plugin_get_plugin_group(plugin);
	zzub_plugin_t* new_plugin = buze_document_create_plugin(document, uri, instrumentname, ox + x_offset, oy + y_offset, plugingroup);

	if (zzub_plugin_get_flags(new_plugin) & zzub_plugin_flag_has_audio_output) {
		std::vector<zzub_connection_t*> out_conns;

		int out_count = zzub_plugin_get_output_connection_count(plugin);
		for (int i = 0; i < out_count; ++i)
		{
			zzub_connection_t* audioconn_out = zzub_plugin_get_output_connection(plugin, i);
			zzub_plugin_t* out_plugin = zzub_connection_get_to_plugin(audioconn_out);
			assert(audioconn_out);
			assert(out_plugin);

			if (zzub_connection_get_type(audioconn_out) != zzub_connection_type_audio) continue;

			out_conns.push_back(audioconn_out);

			zzub_plugin_t* connplug_out = zzub_connection_get_connection_plugin(audioconn_out);
			int outputVolume = zzub_audio_connection_get_amp(connplug_out);

			ConnectMachinesAudioCompatible(new_plugin, out_plugin, audioconn_out, 0, outputVolume, 0);
		}

		for (std::vector<zzub_connection_t*>::iterator i = out_conns.begin(); i != out_conns.end(); ++i) {
			zzub_connection_destroy(*i);
		}
	}

	if ((zzub_plugin_get_flags(plugin) & zzub_plugin_flag_has_audio_output) &&
		(zzub_plugin_get_flags(new_plugin) & zzub_plugin_flag_has_audio_input)) {
		ConnectMachinesAudioCompatible(plugin, new_plugin, 0, 0, 0x4000, 0x4000);
	}

	return new_plugin;
}

zzub_plugin_t* buze_document_create_plugin_replace(buze_document_t* document, zzub_plugin_t* plugin, const char* uri, const char* instrumentname) {

	float ox = zzub_plugin_get_position_x(plugin);
	float oy = zzub_plugin_get_position_y(plugin);

	zzub_plugin_group_t* plugingroup = zzub_plugin_get_plugin_group(plugin);
	zzub_plugin_t* new_plugin = buze_document_create_plugin(document, uri, instrumentname, ox, oy, plugingroup);

	if (zzub_plugin_get_flags(new_plugin) & zzub_plugin_flag_has_audio_input) {
		int in_count = zzub_plugin_get_input_connection_count(plugin);
		for (int i = 0; i < in_count; ++i)
		{
			zzub_connection_t* audioconn_in = zzub_plugin_get_input_connection(plugin, i);
			zzub_plugin_t* in_plugin = zzub_connection_get_from_plugin(audioconn_in);
			assert(audioconn_in);
			assert(in_plugin);

			if (zzub_connection_get_type(audioconn_in) != zzub_connection_type_audio) continue;

			zzub_plugin_t* connplug_in = zzub_connection_get_connection_plugin(audioconn_in);
			int inputVolume = zzub_audio_connection_get_amp(connplug_in);

			ConnectMachinesAudioCompatible(in_plugin, new_plugin, audioconn_in, 0, inputVolume, 0);
		}
	}

	if (zzub_plugin_get_flags(new_plugin) & zzub_plugin_flag_has_audio_output) {
		int out_count = zzub_plugin_get_output_connection_count(plugin);
		for (int i = 0; i < out_count; ++i)
		{
			zzub_connection_t* audioconn_out = zzub_plugin_get_output_connection(plugin, i);
			zzub_plugin_t* out_plugin = zzub_connection_get_to_plugin(audioconn_out);
			assert(audioconn_out);
			assert(out_plugin);

			if (zzub_connection_get_type(audioconn_out) != zzub_connection_type_audio) continue;

			zzub_plugin_t* connplug_out = zzub_connection_get_connection_plugin(audioconn_out);
			int outputVolume = zzub_audio_connection_get_amp(connplug_out);

			ConnectMachinesAudioCompatible(new_plugin, out_plugin, 0, audioconn_out, outputVolume, 0);
		}
	}
	return new_plugin;
}

void buze_document_delete_plugin_smart(buze_document_t* document, zzub_plugin_t* plugin) {
	int in_count = zzub_plugin_get_input_connection_count(plugin);
	for (int i = 0; i < in_count; ++i) {
		zzub_connection_t* audioconn_in = zzub_plugin_get_input_connection(plugin, i);
		zzub_plugin_t* in_plugin = zzub_connection_get_from_plugin(audioconn_in);
		assert(audioconn_in);
		assert(in_plugin);

		if (zzub_connection_get_type(audioconn_in) != zzub_connection_type_audio) continue;

		zzub_plugin_t* connplug_in = zzub_connection_get_connection_plugin(audioconn_in);
		int inputVolume = zzub_audio_connection_get_amp(connplug_in);

		int out_count = zzub_plugin_get_output_connection_count(plugin);
		for (int i = 0; i < out_count; ++i) {
			zzub_connection_t* audioconn_out = zzub_plugin_get_output_connection(plugin, i);
			zzub_plugin_t* out_plugin = zzub_connection_get_to_plugin(audioconn_out);
			assert(audioconn_out);
			assert(out_plugin);

			if (zzub_connection_get_type(audioconn_out) != zzub_connection_type_audio) continue;

			//zzub_plugin_t* connplug_out = zzub_connection_get_connection_plugin(audioconn_out);
			//int outputVolume = zzub_plugin_get_parameter_value(connplug_out, 1, 0, 0);
			//int outputPan = zzub_plugin_get_parameter_value(connplug_out, 1, 0, 1);
			// Formula could be (inputVolume + outputVolume)/2 but in real situations it is better to have just `inputVolume`

			ConnectMachinesAudioCompatible(in_plugin, out_plugin, audioconn_in, 0, inputVolume, 0);
		}
	}
	zzub_plugin_destroy(plugin);
}

/*int buze_document_load_sample_by_filename(buze_document_t *document, const char* filename, int curwave) {
	return document->loadSampleByFileName(filename, curwave);
}*/

void buze_document_set_current_file(buze_document_t *document, const char* filename) {
	document->setCurrentFile(filename);
}

const char* buze_document_get_current_path(buze_document_t *document) {
	return document->currentDirectory.c_str();
}

const char* buze_document_get_current_filename(buze_document_t *document) {
	return document->currentFileName.c_str();
}

const char* buze_document_get_current_extension(buze_document_t *document) {
	return document->currentExtension.c_str();
}

int buze_document_load_plugin_index(buze_document_t *document) {
	return document->loadIndex();
}

zzub_plugin_t* buze_document_get_current_plugin(buze_document_t* document) {
	return document->currentPlugin;
}

zzub_pattern_t* buze_document_get_current_pattern(buze_document_t* document) {
	return document->currentPattern;
}

zzub_pattern_format_t* buze_document_get_current_pattern_format(buze_document_t* document) {
	return document->currentPatternformat;
}

zzub_connection_t* buze_document_get_current_connection(buze_document_t* document) {
	return document->currentConnection;
}

zzub_wave_t* buze_document_get_current_wave(buze_document_t* document) {
	return document->currentWave;
}

int buze_document_get_current_order_index(buze_document_t* document) {
	return document->currentOrderIndex;
}

int buze_document_get_current_order_pattern_row(buze_document_t* document) {
	return document->currentOrderPatternRow;
}

int buze_document_get_current_pattern_row(buze_document_t* document) {
	return document->currentPatternRow;
}

zzub_wavelevel_t* buze_document_get_current_wavelevel(buze_document_t* document) {
	return document->currentWavelevel;
}

bool LoadSampleByFileName(zzub_player_t* player, const char *szFileName, int instrument, int sample, zzub_wave_t* wave) {
	bool ret = true;

	zzub_wave_importer_t* importer = zzub_player_create_waveimporter_by_file(player, szFileName);

	if (importer == 0) return false;

	if (instrument == -1) {
		// if no instrument was specified, only continue if there is one instrument in the file:
		int count = zzub_wave_importer_get_instrument_count(importer);
		if (count != 1) {
			zzub_wave_importer_destroy(importer);
			return false;
		}
		instrument = 0;
	}

	if (sample == -1) {
		int count = zzub_wave_importer_get_instrument_sample_count(importer, instrument);
		if (count == 0) {
			zzub_wave_importer_destroy(importer);
			return false;
		}
		if (count == 1) sample = 0;
	}

	zzub_wave_clear(wave);
	if (sample == -1) {
		ret = zzub_wave_importer_load_instrument(importer, instrument, wave) != 0;
	} else {
		zzub_wavelevel_t* wavelevel = zzub_wave_add_level(wave);
		ret = zzub_wave_importer_load_instrument_sample(importer, instrument, sample, wavelevel) != 0;
	}
	zzub_wave_importer_destroy(importer);

	if (!ret) {
		return false;
	}
	std::string name = szFileName;
	
	size_t ls = name.find_last_of('\\');
	if (ls != std::string::npos) name = name.substr(ls + 1);
	
	ls = name.find_last_of('.');
	if (ls != std::string::npos) name = name.substr(0, ls);
	
	zzub_wave_set_name(wave, name.c_str());
	zzub_wave_set_path(wave, szFileName);
	zzub_wave_set_volume(wave, 1.0f);
	
	return true;
}

int buze_document_import_wave(buze_document_t* document, const char* filename, zzub_wave_t* target) {
	std::string filepart;
	int instrument, sample;
	ParseFileName(filename, &filepart, &instrument, &sample);
	return LoadSampleByFileName(document->player, filepart.c_str(), instrument, sample, target) ? 0 : -1;
}

buze_plugin_index_item_t *buze_document_get_plugin_index_item_by_index(buze_document_t *document, int index) {
	return (buze_plugin_index_item_t*)document->machineIndex.root.getItemByIndex(index);
	//return (MachineItem*)document->machineIndex.root.getItemByIndex(index);
}

buze_plugin_index_item_t *buze_document_get_plugin_index_root(buze_document_t *document) {
	return (buze_plugin_index_item_t*)&document->machineIndex.root;
}

int buze_plugin_index_item_get_type(buze_plugin_index_item_t *pluginindexitem) {
	return pluginindexitem->type;
}

const char *buze_plugin_index_item_get_name(buze_plugin_index_item_t *pluginindexitem) {
	return pluginindexitem->label.c_str();
}

const char *buze_plugin_index_item_get_filename(buze_plugin_index_item_t *pluginindexitem) {
	assert(pluginindexitem->type == 1);
	return ((MachineItem*)pluginindexitem)->fileName.c_str();
}

const char *buze_plugin_index_item_get_instrumentname(buze_plugin_index_item_t *pluginindexitem) {
	assert(pluginindexitem->type == 1);
	return ((MachineItem*)pluginindexitem)->instrumentName.c_str();
}

int buze_plugin_index_item_is_hidden(buze_plugin_index_item_t *pluginindexitem) {
	return (int)pluginindexitem->hide;
}

int buze_plugin_index_item_is_preloaded(buze_plugin_index_item_t *pluginindexitem) {
	assert(pluginindexitem->type == 1);
	return (int)((MachineItem*)pluginindexitem)->preload;
}

int buze_plugin_index_item_get_sub_item_count(buze_plugin_index_item_t *pluginindexitem) {
	return (int)pluginindexitem->items.size();
}

buze_plugin_index_item_t* buze_plugin_index_item_get_sub_item(buze_plugin_index_item_t *pluginindexitem, int index) {
	return (buze_plugin_index_item_t*)pluginindexitem->items[index];
}

const char *buze_plugin_index_item_get_separator_id(buze_plugin_index_item_t *pluginindexitem) {
	assert(pluginindexitem->type == 2);
	return ((MachineSeparator*)pluginindexitem)->identifier.c_str();
}
