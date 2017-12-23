#pragma once

#if defined(WIN32)

#define _ATL_NO_UUIDOF
#define NOMINMAX
#include <algorithm>

using std::min;
using std::max;

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlctrlx.h>

#endif

struct lunar_fx;
class lunar_opengl_context_t;

struct lunar_gui_point_t {
	long x, y;
};

struct lunar_gui_info_t {
	lunar_gui_point_t suggestsize;
	lunar_gui_point_t minsize;
	lunar_gui_point_t maxsize;
};

struct lunar_gui_params_t {
	lunar_fx* fx;
	const lunar_gui_info_t* info;
};


#if defined(WIN32)

class CLunarPluginGui
	: public CWindowImpl<CLunarPluginGui>
{
public:
	lunar_gui_params_t* params;
	lunar_opengl_context_t* glcontext;
	bool painting;
	bool redrawFlag;
	bool initDoneFlag;

	//DECLARE_WND_CLASS("CLunarContainer")
	static ATL::CWndClassInfo& GetWndClassInfo()
	{
		static ATL::CWndClassInfo wc =
		{
			{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC, StartWindowProc,
			  0, 0, NULL, NULL, NULL, NULL, NULL, "CLunarContainer", NULL },
			NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
		};
		return wc;
	}

	BEGIN_MSG_MAP(CLunarPluginGui)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)

		MESSAGE_HANDLER(WM_TIMER, OnTimer)
	END_MSG_MAP()

	CLunarPluginGui() {}
	~CLunarPluginGui(void) {}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void UpdateSize(int* outwidth, int* outheight);

	// callbacks from lunar.cpp
	void OpenGLInit();
	void RequestRedraw();
};

#endif // WIN32
