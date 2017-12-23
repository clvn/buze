#include "stdafx.h"
#include <sstream>
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <sys/types.h> 
#include <sys/stat.h>
#if defined(_WIN32)
#include "i_opndir.h"
#else
#include <dirent.h>
#endif
#include "resource.h"
#include "ToolbarWindow.h"
#include "LuaView.h"

extern void(*luacallback_zzub_player_add_callback)(lua_State* L, zzub_player_t* player, zzub_callback_t callback, void* tag);

struct zzub_player_callback_callback_t {
	lua_State* l;
	zzub_player_t* player;
	zzub_callback_t callback;
	void* tag;
};

std::vector<zzub_player_callback_callback_t> player_callbacks;

void luazzub_add_callback_callback(lua_State* L, zzub_player_t* player, zzub_callback_t callback, void* tag) {
	zzub_player_callback_callback_t cc;
	cc.l = L;
	cc.player = player;
	cc.callback = callback;
	cc.tag = tag;
	player_callbacks.push_back(cc);
}

void luazzub_release_callbacks(lua_State* L) {
	for (std::vector<zzub_player_callback_callback_t>::iterator i = player_callbacks.begin(); i != player_callbacks.end(); ) {
		if (i->l == L) {
			zzub_player_remove_callback(i->player, i->callback, i->tag);
			i = player_callbacks.erase(i);
		} else {
			++i;
		}
	}
}

extern void(*luacallback_buze_document_add_callback)(lua_State* L, buze_document_t* document, buze_callback_t callback, void* tag);

struct buze_document_callback_callback_t {
	lua_State* l;
	buze_document_t* document;
	buze_callback_t callback;
	void* tag;
};

std::vector<buze_document_callback_callback_t> document_callbacks;

void luabuze_add_callback_callback(lua_State* L, buze_document_t* document, buze_callback_t callback, void* tag) {
	buze_document_callback_callback_t cc;
	cc.l = L;
	cc.document = document;
	cc.callback = callback;
	cc.tag = tag;
	document_callbacks.push_back(cc);
}

void luabuze_release_callbacks(lua_State* L) {
	for (std::vector<buze_document_callback_callback_t>::iterator i = document_callbacks.begin(); i != document_callbacks.end(); ) {
		if (i->l == L) {
			buze_document_remove_callback(i->document, i->callback, i->tag);
			i = document_callbacks.erase(i);
		} else {
			++i;
		}
	}
}

void report_errors(lua_State *L, int status) {
	if (status != 0) {
		const char* error = lua_tostring(L, -1);
#if defined(_WIN32)
		MessageBox(GetDesktopWindow(), error, "Error", MB_OK);
#else
		using std::cout;
		using std::endl;
		cout << error << endl;
#endif
		lua_pop(L, 1); // remove error message
	}
}

class CLuaViewLibrary : public CViewLibrary {
public:
	virtual void Initialize(CViewFrame* host) {
		// TODO: scan lua views, create a luaviewinfo pr script/directory or whatever
		// create view = run the fbgui in that directory
		// run init script = get name, default dock position, 
		_Module.m_hostModule = buze_application_get_host_module(buze_main_frame_get_application(host));

		enumerate_plugins(host, "Gear/Scripts/LuaView");

	}

	virtual void Destroy() {
		delete this;
	}

	virtual int GetVersion() {
		return CViewLibrary::version;
	}

	bool run_plugin_init(const std::string& scriptpath, int* dock, std::string* title, std::string* classname) {
		lua_State *l = luaL_newstate();
		luaL_openlibs(l);

		int status = luaL_dofile(l, scriptpath.c_str());
		if (status != 0) {
			report_errors(l, status);
			lua_close(l);
			return false;
		}

		// parse dock from global
		lua_getglobal(l, "dock");
		if (!lua_isnumber(l, 1)) {
			lua_close(l);
			return false;
		}

		*dock = lua_tointeger(l, -1);
		lua_pop(l, 1);

		// parse title from global
		lua_getglobal(l, "title");
		if (!lua_isstring(l, 1)) {
			lua_close(l);
			return false;
		}
		const char* titlestring = lua_tostring(l, -1);
		if (!titlestring) {
			lua_close(l);
			return false;
		}

		*title = titlestring;
		lua_pop(l, 1);

		// parse classname from global
		lua_getglobal(l, "classname");
		if (!lua_isstring(l, 1)) {
			lua_close(l);
			return false;
		}
		const char* classnamestring = lua_tostring(l, -1);
		if (!classnamestring) {
			lua_close(l);
			return false;
		}

		*classname = classnamestring;
		lua_pop(l, 1);

		lua_close(l);
		return true;
	}

	bool load_plugin(CViewFrame* host, const std::string& dir) {
		// try to load main.lua
		// also try to load main.nml, should be made available to the main lua, or it could load itself?
		std::string scriptpath = dir + "/init.lua";
		struct stat s;
		if (stat(scriptpath.c_str(), &s) == -1)
			return false;

		int dock;
		std::string title, classname;
		if (!run_plugin_init(scriptpath, &dock, &title, &classname))
			return false;
		
		buze_main_frame_register_window_factory(host, new CLuaViewInfo(host, dir, classname, dock, title));
		return true;
	}

	void enumerate_plugins(CViewFrame* host, const std::string& basepath) {

		DIR* d = opendir(basepath.c_str());
		dirent* de = 0;
		while ((de = readdir(d)) != 0) {
			std::stringstream strm;
			strm << basepath << "/" << de->d_name;
			std::string fullpath = strm.str();
			struct stat s;
			if (stat(fullpath.c_str(), &s) == -1)
				continue;
			if (s.st_mode &  S_IFDIR) {
				if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) 
					continue;

				load_plugin(host, fullpath);
			}
		}
	
		closedir(d);
	}

};

extern "C" CViewLibrary* buze_create_viewlibrary() {
	return new CLuaViewLibrary();
}

//
// Factory
//

CLuaViewInfo::CLuaViewInfo(buze_main_frame_t* m, const std::string& scriptpath, const std::string& id, int dock, const std::string& title) : CViewInfoImpl(m) {
	int dockplace, dockside;
	switch (dock) {
		case 0:
			dockplace = 1;
			dockside = -1;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			dockplace = 2;
			dockside = dock - 1;
			break;
		default:
			dockplace = 1;
			dockside = -1;
			break;
	}

	this->id = id;
	this->title = title;
	this->path = scriptpath;
	this->mainframe = mainframe;
	this->uri = this->id.c_str();
	this->label = this->title.c_str();
	this->tooltip = "Lua view";
	this->place = dockplace; //DockSplitTab::placeMAINPANE;
	this->side = dockside; //DockSplitTab::dockUNKNOWN;
	this->serializable = false;
	this->allowfloat = false;
	this->defaultview = false;
}

CView* CLuaViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CLuaView* view = new CLuaView(mainframe, path);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

int luaview_counter = 0;

void CLuaViewInfo::Attach() {

	luacallback_zzub_player_add_callback = luazzub_add_callback_callback;

	buze_document_add_view(document, this);

	std::stringstream accname;
	accname << "view_lua" << luaview_counter;
	// global accelerators - these generate global document events caught in OnUpdate
	show_eventcode = buze_main_frame_register_event(mainframe);
	// TODO: can set hotkey in the init script
	WORD ID_SHOW_SCRIPT = buze_main_frame_register_accelerator_event(mainframe, accname.str().c_str(), "", show_eventcode);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_SCRIPT, title.c_str());

	luaview_counter++;

}

void CLuaViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CLuaViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CView* view;
	if (lHint == show_eventcode) {
		view = buze_main_frame_get_view(mainframe, uri, 0);
		if (view) {
			buze_main_frame_set_focus_to(mainframe, view);
		} else
			buze_main_frame_open_view(mainframe, uri, title.c_str(), 0, -1, -1);
	}
}
