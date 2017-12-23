#define _WIN32_WINNT 0x0501 // for WM_MOUSEWHEEL, WM_APPCOMMAND
#include <windows.h>
#include <iostream>
#include <map>
#include <cassert>
#include "pluginterfaces/vst2.x/aeffectx.h"
#include "editor.h"

using std::cout;
using std::endl;

ATOM vsteditor::atomWndClass = 0;
std::map<HWND, vsteditor*> vsteditor::editors;

// iirc this was taken from http://www.hermannseib.com/english/vsthost.htm
void MakeVstKeyCode(UINT nChar, VstKeyCode &keyCode) {
#if defined(VST_2_1_EXTENSIONS)

	static struct {
		UINT vkWin;
		unsigned char vstVirt;
	} VKeys[] = {
		{ VK_BACK,      VKEY_BACK      },
		{ VK_TAB,       VKEY_TAB       },
		{ VK_CLEAR,     VKEY_CLEAR     },
		{ VK_RETURN,    VKEY_RETURN    },
		{ VK_PAUSE,     VKEY_PAUSE     },
		{ VK_ESCAPE,    VKEY_ESCAPE    },
		{ VK_SPACE,     VKEY_SPACE     },
		//  { VK_NEXT,      VKEY_NEXT      },
		{ VK_END,       VKEY_END       },
		{ VK_HOME,      VKEY_HOME      },
		{ VK_LEFT,      VKEY_LEFT      },
		{ VK_UP,        VKEY_UP        },
		{ VK_RIGHT,     VKEY_RIGHT     },
		{ VK_DOWN,      VKEY_DOWN      },
		{ VK_PRIOR,     VKEY_PAGEUP    },
		{ VK_NEXT,      VKEY_PAGEDOWN  },
		{ VK_SELECT,    VKEY_SELECT    },
		{ VK_PRINT,     VKEY_PRINT     },
		{ VK_EXECUTE,   VKEY_ENTER     },
		{ VK_SNAPSHOT,  VKEY_SNAPSHOT  },
		{ VK_INSERT,    VKEY_INSERT    },
		{ VK_DELETE,    VKEY_DELETE    },
		{ VK_HELP,      VKEY_HELP      },
		{ VK_NUMPAD0,   VKEY_NUMPAD0   },
		{ VK_NUMPAD1,   VKEY_NUMPAD1   },
		{ VK_NUMPAD2,   VKEY_NUMPAD2   },
		{ VK_NUMPAD3,   VKEY_NUMPAD3   },
		{ VK_NUMPAD4,   VKEY_NUMPAD4   },
		{ VK_NUMPAD5,   VKEY_NUMPAD5   },
		{ VK_NUMPAD6,   VKEY_NUMPAD6   },
		{ VK_NUMPAD7,   VKEY_NUMPAD7   },
		{ VK_NUMPAD8,   VKEY_NUMPAD8   },
		{ VK_NUMPAD9,   VKEY_NUMPAD9   },
		{ VK_MULTIPLY,  VKEY_MULTIPLY  },
		{ VK_ADD,       VKEY_ADD,      },
		{ VK_SEPARATOR, VKEY_SEPARATOR },
		{ VK_SUBTRACT,  VKEY_SUBTRACT  },
		{ VK_DECIMAL,   VKEY_DECIMAL   },
		{ VK_DIVIDE,    VKEY_DIVIDE    },
		{ VK_F1,        VKEY_F1        },
		{ VK_F2,        VKEY_F2        },
		{ VK_F3,        VKEY_F3        },
		{ VK_F4,        VKEY_F4        },
		{ VK_F5,        VKEY_F5        },
		{ VK_F6,        VKEY_F6        },
		{ VK_F7,        VKEY_F7        },
		{ VK_F8,        VKEY_F8        },
		{ VK_F9,        VKEY_F9        },
		{ VK_F10,       VKEY_F10       },
		{ VK_F11,       VKEY_F11       },
		{ VK_F12,       VKEY_F12       },
		{ VK_NUMLOCK,   VKEY_NUMLOCK   },
		{ VK_SCROLL,    VKEY_SCROLL    },
		{ VK_SHIFT,     VKEY_SHIFT     },
		{ VK_CONTROL,   VKEY_CONTROL   },
		{ VK_MENU,      VKEY_ALT       },
		//  { VK_EQUALS,    VKEY_EQUALS    },
	};

	if ((nChar >= 'A') && (nChar <= 'Z'))
		keyCode.character = nChar + ('a' - 'A');
	else
		keyCode.character = nChar;

	keyCode.virt = 0;
	for (int i = 0; i < (sizeof(VKeys)/sizeof(VKeys[0])); i++)
		if (nChar == VKeys[i].vkWin) {
			keyCode.virt = VKeys[i].vstVirt;
			break;
		}

	keyCode.modifier = 0;
	if (GetKeyState(VK_SHIFT) & 0x8000)
		keyCode.modifier |= MODIFIER_SHIFT;
	if (GetKeyState(VK_CONTROL) & 0x8000)
		keyCode.modifier |= MODIFIER_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		keyCode.modifier |= MODIFIER_ALTERNATE;
#endif
} 

LRESULT CALLBACK PluginProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	std::map<HWND, vsteditor*>::iterator i = vsteditor::editors.find(hWnd);
	if (i == vsteditor::editors.end()) 
		return DefWindowProc(hWnd, msg, wParam, lParam);

	vsteditor* self = i->second;
	VstKeyCode keyCode;
	int result;

	switch (msg) {
		case WM_KEYDOWN:
		case WM_KEYUP:
			result = CallWindowProc(self->pluginWndProc, hWnd, msg, wParam, lParam);
			if (result == 0) return 0; // plugin handled WM_KEYDOWN/KEYUP
			return SendMessage(GetParent(hWnd), msg, wParam, lParam);
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_APPCOMMAND:
		case WM_MOUSEWHEEL:
			break;
	}

	return CallWindowProc(self->pluginWndProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK EditorProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	vsteditor* self;
	CREATESTRUCT* cs;
	VstKeyCode keyCode;
	int result;

	switch(msg) {
		case WM_CREATE:
			cs = (CREATESTRUCT*)lParam;
			self = (vsteditor*)cs->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)self);
			self->init_dialog(hWnd);
			return 0;
		case WM_TIMER :
			self = (vsteditor*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			self->idle();
			return 0;
		case WM_DESTROY:
			self = (vsteditor*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			self->destroy();
			return 0;
		case WM_SETFOCUS:
			self = (vsteditor*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			SetFocus(self->pluginWnd);
			return 0;
		case WM_KEYDOWN:
			self = (vsteditor*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			MakeVstKeyCode(wParam, keyCode);
			result = self->effect->dispatcher(self->effect, effEditKeyDown, keyCode.character, keyCode.virt, 0, keyCode.modifier);
			if (result == 1) return 0; // plugin handled keypress
			break;
		case WM_KEYUP:
			self = (vsteditor*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			MakeVstKeyCode(wParam, keyCode);
			result = self->effect->dispatcher(self->effect, effEditKeyUp, keyCode.character, keyCode.virt, 0, keyCode.modifier);
			if (result == 1) return 0; // plugin handled keypress
			break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


vsteditor::vsteditor() {
	effect = 0;
	hWnd = 0;
	pluginWnd = 0;
	pluginWndProc = 0;
}

vsteditor::~vsteditor() {
	std::map<HWND, vsteditor*>::iterator i = editors.find(pluginWnd);
	if (i != editors.end()) editors.erase(i);
}

void vsteditor::create(AEffect* _effect, std::string name, HWND hParentWnd, bool asControl) {
	if (hWnd != 0) {
		assert(IsWindow(hWnd));
		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	} else {
		effect = _effect;

		if (atomWndClass == 0) {
			WNDCLASS wc;
			memset(&wc, 0, sizeof(WNDCLASS));
			wc.lpfnWndProc = EditorProc;
			wc.lpszClassName = "vst2zzubwnd";
			wc.style = CS_HREDRAW|CS_VREDRAW;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			atomWndClass = RegisterClass(&wc);
		}
		if (asControl) {
			hWnd = CreateWindowEx(0, "vst2zzubwnd", name.c_str(), WS_CHILD, 0, 0, 100, 100, hParentWnd, 0, 0, this);
		} else {
			hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, "vst2zzubwnd", name.c_str(), WS_POPUPWINDOW | WS_CAPTION | WS_VISIBLE/* | WS_SIZEBOX*/, 0, 0, 100, 100, hParentWnd, 0, 0, this);
		}

		// dispatch(effKeysRequired, 0, 0, 0, 0) == 0 -> TODO: subclass only on condition
		pluginWnd = GetWindow(hWnd, GW_CHILD);
		assert(pluginWnd != 0); // now what?

		editors[pluginWnd] = this;
		pluginWndProc = (WNDPROC)GetWindowLongPtr(pluginWnd, GWLP_WNDPROC);
		SetWindowLongPtr(pluginWnd, GWLP_WNDPROC, (LONG_PTR)PluginProc);

		SetFocus(pluginWnd);
	}
}

void vsteditor::show(bool visible) {
	cout << "vsteditor::show" << endl;

	DWORD dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
	if (visible) {
		dwStyle |= WS_VISIBLE;
	} else {
		dwStyle &= ~WS_VISIBLE;
	}
	SetWindowLongPtr(hWnd, GWL_STYLE, dwStyle);
}


void vsteditor::init_dialog(HWND _hWnd) {
	hWnd = _hWnd;
	SetTimer(hWnd, 1, 20, 0);

	cout << "HOST> Open editor..." << endl;
	// audacity comment: Some effects like to have us get their rect before opening them.
	ERect* rect = nullptr;
    effect->dispatcher(effect, effEditGetRect, 0, 0, &rect, 0);
	effect->dispatcher(effect, effEditOpen, 0, 0, hWnd, 0);

	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	update_size((dwStyle & WS_POPUP) != 0 );

}

void vsteditor::update_size(bool center) {

	cout << "HOST> Get editor rect.." << endl;
	ERect* eRect = 0;

	effect->dispatcher(effect, effEditGetRect, 0, 0, &eRect, 0);
	if (eRect) {
		int width = eRect->right - eRect->left;
		int height = eRect->bottom - eRect->top;
		if (width < 100) width = 100;
		if (height < 100) height = 100;

		RECT wRect;
		SetRect(&wRect, 0, 0, width, height);
		AdjustWindowRectEx(&wRect, GetWindowLong (hWnd, GWL_STYLE), FALSE, GetWindowLong (hWnd, GWL_EXSTYLE));
		width = wRect.right - wRect.left;
		height = wRect.bottom - wRect.top;

		if (center) {
			int screenWidth = GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = GetSystemMetrics(SM_CYSCREEN);

			int x = screenWidth / 2 - width / 2;
			int y = screenHeight / 2 - height / 2;
			SetWindowPos(hWnd, HWND_TOP, x, y, width, height, 0);
		} else {
			SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);
		}
	}
}

void vsteditor::idle() {
	// kerovee vst (uniqueID = 0x32353871) idle runs { PeekMessage() DispatchMessage() Sleep(1) } in a loop as long as there are messages.
	// this messes up the hook-driven WTL CCommandBar (the main menu): of a keypress arrives while idling, the message is sent to the 
	// wrong window, because the command bar hooks are (for unknown reasons) not invoked to forward the message to the correct window.
	// another approach of skipping idling inside WM_ENTERMENULOOP/WM_EXITMENULOOP did not work out.
	// this makes kerovee hard to host in a wtl app with a commandbar. this manual idle workaround seems to work:
	if (effect && effect->uniqueID == 0x32353871) {
		InvalidateRect(pluginWnd, 0, FALSE);
		return ;
	}

	if (effect)
		effect->dispatcher(effect, effEditIdle, 0, 0, 0, 0);
}


void vsteditor::destroy() {
	cout << "vsteditor::destroy" << endl;

	if (hWnd) {
		SetWindowLongPtr(pluginWnd, GWLP_WNDPROC, (LONG_PTR)pluginWndProc);

		KillTimer(hWnd, 1);
		cout << "HOST> Close editor.." << endl;
		if (effect)
			effect->dispatcher(effect, effEditClose, 0, 0, 0, 0);
		//DestroyWindow(hWnd);

		hWnd = 0;
		pluginWndProc = 0;
	}
}

