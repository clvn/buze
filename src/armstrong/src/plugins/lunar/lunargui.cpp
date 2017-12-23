#ifdef WIN32

#define LUNAR_STD_BUILD

#define _ATL_NO_UUIDOF
#define NOMINMAX
#include <algorithm>

using std::min;
using std::max;

#include <atlbase.h>
#include <atlapp.h>
#include <atlcom.h>

#define NO_ZZUB_MIXER_TYPE
namespace zzub {
	struct mixer;
};

typedef struct zzub::mixer zzub_mixer_t;

#include "zzub/plugin.h"

typedef enum {} lunar_parameter_names_t;

#include "lunargui.h"
#include "lunarstd.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "include/GL/glew.h"
#include "include/GL/wglew.h"

class lunar_opengl_context_t {
public:
	lunar_opengl_context_t() {}

	HDC hDC;
	HGLRC hRC;
};

void CLunarPluginGui::OpenGLInit()
{
	if (!glcontext || glcontext->hRC) {
		// no window or already initialized
		return;
	}

	// See MSDN for the definition of these numbers
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32, // 32-bit color
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		24, // 24-bit z-buffer
		8, // 8-bit stencil buffer
		0,
		PFD_MAIN_PLANE,
		0, 0, 0, 0
	};

	glcontext->hDC = GetDC();
	glcontext->hRC = 0;

	int pf = ChoosePixelFormat(glcontext->hDC, &pfd);
	if (pf == 0) {
		goto fail;
	}

	if (!SetPixelFormat(glcontext->hDC, pf, &pfd)) {
		goto fail;
	}

	glcontext->hRC = wglCreateContext(glcontext->hDC);
	if (!glcontext->hRC) {
		goto fail;
	}

	if (!wglMakeCurrent(glcontext->hDC, glcontext->hRC)) {
		goto fail;
	}

	GLenum r = glewInit();
	wglMakeCurrent(NULL, NULL);
	if (r != GLEW_OK) {
		goto fail;
	}

	SetTimer(1, 20, 0);
	return;

fail:
	if (glcontext->hRC) wglDeleteContext(glcontext->hRC);
	glcontext->hRC = 0;
	if (glcontext->hDC) ReleaseDC(glcontext->hDC);
	glcontext->hDC = 0;
}

LRESULT CLunarPluginGui::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
	params = (lunar_gui_params_t*)pcs->lpCreateParams;
	glcontext = new lunar_opengl_context_t;
	glcontext->hDC = 0;
	glcontext->hRC = 0;

	LRESULT lres = DefWindowProc();

	redrawFlag = false;
	initDoneFlag = false;

	return lres;
}

LRESULT CLunarPluginGui::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	params = 0;
	
	if (glcontext) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(glcontext->hRC);
		ReleaseDC(glcontext->hDC);
		delete glcontext;
		glcontext = 0;
	}

	KillTimer(1);

	return DefWindowProc();
}

LRESULT CLunarPluginGui::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	
	if (!initDoneFlag) {
		OpenGLInit();
		initDoneFlag = true;
	}
	
	PAINTSTRUCT ps;
	BeginPaint(&ps);
	EndPaint(&ps);

	if (params->fx && params->fx->redraw_gui) {
		if (glcontext && glcontext->hRC) {
			RECT rWnd;
			GetClientRect(&rWnd);

			wglMakeCurrent(glcontext->hDC, glcontext->hRC);
			glViewport(0, 0, rWnd.right - rWnd.left, rWnd.bottom - rWnd.top);
			params->fx->redraw_gui(params->fx);

			SwapBuffers(glcontext->hDC);

			wglMakeCurrent(glcontext->hDC, NULL);
		}
	}
	
	return 0;
}

LRESULT CLunarPluginGui::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);
	
	UpdateSize(&x, &y);
	return 0;
}

LRESULT CLunarPluginGui::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 1;
}

LRESULT CLunarPluginGui::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (redrawFlag) {
		InvalidateRect(NULL, FALSE);
		redrawFlag = false;
	}

	return 0;
}


void CLunarPluginGui::UpdateSize(int* outwidth, int* outheight) {

	assert(outwidth);
	assert(outheight);

	// resize ourself if a fixed size in any direction was given

	if (outwidth) {
		if (params->info->suggestsize.x) {
			// TODO: maybe there are constraints from parameterview?
			*outwidth = params->info->suggestsize.x;
		}
		if (params->info->minsize.x) {
			*outwidth = std::min(*outwidth, (int)params->info->minsize.x);
		}
		if (params->info->maxsize.x) {
			*outwidth = std::max(*outwidth, (int)params->info->maxsize.x);
		}
	}

	if (outheight) {
		if (params->info->suggestsize.y) {
			// TODO: maybe there are constraints from parameterview?
			*outheight = params->info->suggestsize.y;
		}
		if (params->info->minsize.y) {
			*outheight = std::min(*outheight, (int)params->info->minsize.y);
		}
		if (params->info->maxsize.y) {
			*outheight = std::max(*outheight, (int)params->info->maxsize.y);
		}
	}

	if (outwidth && outheight) {
		SetWindowPos(0, 0, 0, *outwidth, *outheight, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER);
		if (params->fx && params->fx->resize_gui) {
			params->fx->resize_gui(params->fx, *outwidth, *outheight);
		}
	}
}

void CLunarPluginGui::RequestRedraw() {
	redrawFlag = true;
}

#endif // WIN32
