#include "stdafx.h"
#include "resource.h"
#include "ToolbarWindow.h"
#include "LuaView.h"
#include "fbgui.h"

CHostDllModule _Module;

//
// View
//

CLuaView::CLuaView(buze_main_frame_t* m, const std::string& path) : CViewImpl(m) {
	scriptpath = path;
	l = 0;

}

CLuaView::~CLuaView() {
}

void CLuaView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

extern int luaopen_fbgui(lua_State* L);
extern void luaclose_fbgui(lua_State* L);
extern int luaopen_zzub(lua_State* L); 
extern void luaclose_zzub(lua_State* L);
extern int luaopen_buze(lua_State* L); 
extern void luaclose_buze(lua_State* L);
extern void report_errors(lua_State *L, int status);

void set_lua_path(lua_State* L, const std::string& path) {
	// http://stackoverflow.com/questions/4125971/setting-the-global-lua-path-variable-from-c-c
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path"); // get field "path" from table at top of stack (-1)
    
	std::string cur_path = lua_tostring(L, -1); // grab path string from top of stack
    cur_path.append(";"); // do your path magic here*
    cur_path.append(path);
    cur_path.append("/?.lua"); // do your path magic here

	lua_pop(L, 1); // get rid of the string on the stack we just pushed on line 5
    lua_pushstring(L, cur_path.c_str()); // push the new one
    lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with value at top of stack
    lua_pop(L, 1); // get rid of package table from top of stack
}

LRESULT CLuaView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lres = DefWindowProc();

	l = luaL_newstate();
	luaL_openlibs(l);
	luaopen_zzub(l);
	luaopen_buze(l);
	luaopen_fbgui(l);
	
	// add Gear/Scripts/LuaView to package.path to allow require() from the LuaView folder
	set_lua_path(l, "Gear/Scripts/LuaView");

	context = fbgui_context_create_child(m_hWnd);	

	// add fbgui to lua globals
	lua_pushlightuserdata(l, context);
	lua_setglobal(l, "context");

	// add buze to lua globals
	lua_pushlightuserdata(l, mainframe);
	lua_setglobal(l, "mainframe");

	// add armstrong to lua globals
	lua_pushlightuserdata(l, buze_main_frame_get_player(mainframe));
	lua_setglobal(l, "player");
	lua_pushlightuserdata(l, buze_application_get_audio_driver(buze_main_frame_get_application(mainframe)));
	lua_setglobal(l, "audiodriver");

	lua_pushstring(l, scriptpath.c_str());
	lua_setglobal(l, "script_path");

	// run main script
	std::string scriptfile = scriptpath + "/main.lua";
	int status = luaL_dofile(l, scriptfile.c_str());
	report_errors(l, status);
/*
	std::string stylefile = scriptpath + "/main.nss";
	fbgui_context_load_stylesheet(context, stylefile.c_str());

	std::string markupfile = scriptpath + "/main.nml";
	fbgui_node_t* view = fbgui_context_load_markup(context, markupfile.c_str());

	lua_pushlightuserdata(l, view);
	lua_setglobal(l, "view");

	fbgui_context_run(context, view);
	*/
	hContextWnd = GetWindow(GW_CHILD);
	SetTimer(1, 1); // WM_TIMER every ms

	buze_document_add_view(document, this);

	return 0;
}

LRESULT CLuaView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);

	if (width == 0 || height == 0) return 0;
	if (hContextWnd == 0) return 0;

	hContextWnd.MoveWindow(0, 0, width, height, FALSE);
	return 0;
}

LRESULT CLuaView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	KillTimer(1);

	fbgui_context_destroy(context);
	context = 0;

	luaclose_fbgui(l);
	
	luabuze_release_callbacks(l);
	luaclose_buze(l);

	luazzub_release_callbacks(l);
	luaclose_zzub(l);

	lua_close(l);
	l = 0;

	// TODO: lua_fbgui callback datas must be cleared, or the next session can accidentally use the old callbacks!

	buze_document_remove_view(document, this);
	return 0;
}

LRESULT CLuaView::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	fbgui_context_poll(context);
	return 0;
}

void CLuaView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	switch (lHint) {
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_new_document:
		case zzub_event_type_update_song:
			//edit.SetWindowText(zzub_player_get_infotext(player));
			break;
	}
}
