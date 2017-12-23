#pragma once

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "fbgui.h"

void luazzub_release_callbacks(lua_State* L);
void luabuze_release_callbacks(lua_State* L);

class CLuaViewInfo : public CViewInfoImpl {
public:
	std::string id;
	std::string title;
	std::string path;
	int show_eventcode;

	CLuaViewInfo(buze_main_frame_t* m, const std::string& scriptpath, const std::string& uri, int dock, const std::string& title);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CLuaView 
	: public CViewImpl
	, public CWindowImpl<CLuaView>
{
public:
	lua_State *l;
	fbgui_context_t* context;
	CWindow hContextWnd;
	std::string scriptpath;

	DECLARE_WND_CLASS("LuaView")

	BEGIN_MSG_MAP(CLuaView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
	END_MSG_MAP()

	CLuaView(buze_main_frame_t* m, const std::string& path);
	~CLuaView();
	virtual void OnFinalMessage(HWND /*hWnd*/);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	int GetEditFlags();
};
