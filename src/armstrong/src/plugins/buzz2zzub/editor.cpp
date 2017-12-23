#define NOMINMAX
#include <windows.h>
#include <cassert>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <list>
#include <string>
#include "buzz2zzub.h"
#include "editor.h"

using std::cerr;
using std::cout;
using std::endl;

namespace buzz2zzub {

ATOM buzzeditor::atomWndClass = 0;

LRESULT CALLBACK EditorProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	buzzeditor* self;
	CREATESTRUCT* cs;
	HDC hDC;
	RECT rcClient;
	RECT rcTop;
	HBRUSH hBgBrush;

	switch(msg) {
		case WM_CREATE:
			cs = (CREATESTRUCT*)lParam;
			self = (buzzeditor*)cs->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self);
			//self->init_dialog(hwnd);
			return 0;
		case WM_TIMER :
			self = (buzzeditor*)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			//self->idle();
			return 0;
		case WM_CLOSE:
			self = (buzzeditor*)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			SetParent(self->hFrameWnd, (HWND)self->editorplugin->_mixer->hostinfo.host_ptr);
			SetWindowLongPtr(self->hFrameWnd, GWL_STYLE, WS_CHILD);
			return 0;
		case WM_SIZE:
			self = (buzzeditor*)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			GetClientRect(hwnd, &rcClient);
			MoveWindow(self->hWnd, 0, 20, rcClient.right, rcClient.bottom - 20, TRUE);
			break;
		case WM_SETFOCUS:
			self = (buzzeditor*)(LONG_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			SetFocus(self->hWnd);
			break;
		case WM_ERASEBKGND:
			hDC = (HDC)wParam;
			GetClientRect(hwnd, &rcClient);
			SetRect(&rcTop, 0, 0, rcClient.right, 20);
			hBgBrush = (HBRUSH)(COLOR_BTNFACE + 1);
			FillRect(hDC, &rcTop, hBgBrush);
			return 1;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

buzzeditor* pretranslateeditor;

BOOL PreTranslateMessage(MSG* pMsg) {
	return pretranslateeditor->pre_translate_message(pMsg);
}

buzzeditor::buzzeditor() {
	editorplugin = 0;
	hWnd = 0;
	hFrameWnd = 0;
	currentPattern = 0;
}

bool IsCtrlDown() {
	if (GetKeyState(VK_RCONTROL)<0 || GetKeyState(VK_LCONTROL)<0) return true;
	return false;
}

BOOL buzzeditor::pre_translate_message(MSG* pMsg) {

	zzub::host_info* hi = &editorplugin->_mixer->hostinfo;
	HWND hHostWnd = (HWND)hi->host_ptr;
	if ((pMsg->hwnd == hCustomWnd || IsChild(hCustomWnd, pMsg->hwnd)) && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) {

		switch (pMsg->message) {
			case WM_KEYDOWN:
				if (IsCtrlDown()) {
					switch (pMsg->wParam) {
						case VK_RETURN:
							create_pattern();
							return TRUE;
						case VK_DELETE:
							delete_pattern();
							return TRUE;
						case VK_BACK:
							show_properties();
							break;
						case VK_ADD:
							add_track();
							break;
						case VK_SUBTRACT:
							remove_last_track();
							break;
					}
				} else {
					switch (pMsg->wParam) {
						case VK_ADD:
							next_pattern();
							break;
						case VK_SUBTRACT:
							previous_pattern();
							break;
					}
				}

				break;
		}

		HWND hOrigWnd = pMsg->hwnd;
		pMsg->hwnd = hHostWnd;
		LRESULT lres = SendMessage(hHostWnd, WM_FORWARD_PRETRANSLATE, 0, (LPARAM)pMsg);
		pMsg->hwnd = hOrigWnd;
		if (lres == 0) return TRUE; // handled, return TRUE to skip current message
		return FALSE;
	}
	return FALSE;
}

void buzzeditor::create_framewindow() {
	if (atomWndClass == 0) {
		WNDCLASS wc;
		memset(&wc, 0, sizeof(WNDCLASS));
		wc.lpfnWndProc = EditorProc;
		wc.lpszClassName = "buzz2zzubeditorwnd";
		wc.style = CS_HREDRAW|CS_VREDRAW|CS_PARENTDC;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		atomWndClass = RegisterClass(&wc);
	}

	zzub::host_info* hi = &editorplugin->_mixer->hostinfo;
	RECT rcClient;

	hFrameWnd = CreateWindowEx(0, "buzz2zzubeditorwnd", "", WS_CHILD, 0, 0, 0, 0, (HWND)hi->host_ptr, 0, 0, this);

	hPatternCombo = CreateWindowEx(0, "COMBOBOX", NULL, CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 0, 0, 100, 20, hFrameWnd, 0, 0, 0);
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hPatternCombo, WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE);
	bind_pattern_combo();

	hWnd = (HWND)editorplugin->machine2->CreatePatternEditor(hFrameWnd);
	cout << "got " << std::hex << (unsigned int)hWnd << endl;

	GetClientRect(hFrameWnd, &rcClient);
	MoveWindow(hWnd, 0, 20, rcClient.right, rcClient.bottom - 20, TRUE);

}

void buzzeditor::create_hostwindow(std::string name) {
	zzub::host_info* hi = &editorplugin->_mixer->hostinfo;

	hCustomWnd = (HWND)SendMessage((HWND)hi->host_ptr, 
		WM_CUSTOMVIEW_GET, (WPARAM)editorplugin->_plugin,
		(LPARAM)name.c_str());

	if (!hCustomWnd) {
		hCustomWnd = (HWND)SendMessage((HWND)hi->host_ptr, 
			WM_CUSTOMVIEW_CREATE, (WPARAM)editorplugin->_plugin,
			(LPARAM)name.c_str());

		if (!hCustomWnd) {
			cerr << "buzz2zzub: could not open custom view in buze" << endl;
			return ;
		}

		pretranslateeditor = this;

		(HWND)SendMessage((HWND)hi->host_ptr, 
			WM_CUSTOMVIEW_SET_PRETRANSLATE, (WPARAM)hCustomWnd,
			(LPARAM)PreTranslateMessage);

		SetParent(hFrameWnd, hCustomWnd);
		SetWindowLongPtr(hFrameWnd, GWL_STYLE, WS_CHILD | WS_VISIBLE);
		SendMessage((HWND)hi->host_ptr, WM_CUSTOMVIEW_SET_CHILD, 
			(WPARAM)hCustomWnd, (LPARAM)hFrameWnd);


	} else {
		// set focus to existing pattern editor
		SendMessage((HWND)hi->host_ptr, WM_CUSTOMVIEW_FOCUS,
			(WPARAM)editorplugin->_plugin, (LPARAM)name.c_str());
	}

}

void buzzeditor::create(buzz2zzub::plugin* _effect, std::string name) {
	editorplugin = _effect;

	cout << "Open pattern editor!" << endl;
	zzub::host_info* hi = &editorplugin->_mixer->hostinfo;
	if (hi->id == 42 && hi->version == 0x0503) {
		create_hostwindow(name);

		if (editorplugin->patterns.size() > 0 && currentPattern == 0)
			set_editor_pattern(editorplugin->patterns[0]);
	} else {
		cout << "buzz2zzub: unknown host for pattern editor" << endl;
	}
}

void buzzeditor::bind_pattern_combo() {
	SendMessage(hPatternCombo, CB_RESETCONTENT, 0, 0);

	int cursel = -1;
	int count = 0;

	for (std::vector<CPattern*>::iterator i = editorplugin->patterns.begin(); i != editorplugin->patterns.end(); ++i) {

		CPattern* pat = *i;
		if (pat == 0) continue;

		std::stringstream strm;
		int index = (int)(i - editorplugin->patterns.begin());
		strm << std::setw(2) << std::setfill('0') << (index + 1) << ". " << pat->name;

		SendMessage(hPatternCombo, CB_INSERTSTRING, index, (LPARAM)strm.str().c_str());

		if (pat == currentPattern) cursel = count;
		count++;
	}

	if (cursel != -1)
		SendMessage(hPatternCombo, CB_SETCURSEL, (WPARAM)cursel, 0);
}

void buzzeditor::update_pattern_combo_selection() {
	int count = 0;
	for (std::vector<CPattern*>::iterator i = editorplugin->patterns.begin(); i != editorplugin->patterns.end(); ++i) {
		CPattern* pat = *i;
		if (pat == 0) continue;

		if (pat == currentPattern) {
			SendMessage(hPatternCombo, CB_SETCURSEL, count, 0);
		}
		count++;
	}
}


void buzzeditor::create_pattern() {
	CPattern* pat = new CPattern();
	pat->length = 16;

	std::stringstream strm;
	strm << std::setw(2) << std::setfill('0') << (editorplugin->patterns.size() + 1);
	pat->name = strm.str();

	editorplugin->patterns.push_back(pat);

	editorplugin->machine2->CreatePattern(pat, pat->length);
	bind_pattern_combo();
	set_editor_pattern(pat);
}

void buzzeditor::delete_pattern() {
	MessageBox(hFrameWnd, "Delete", "Bla bla", MB_OK);
}

void buzzeditor::show_properties() {
	editorplugin->machine2->ShowPatternProperties();
}

void buzzeditor::next_pattern() {
	int index = get_pattern_index(currentPattern);
	int count = get_pattern_count();
	index = (index + 1) % count;
	set_editor_pattern(get_pattern_by_index(index));
}

void buzzeditor::previous_pattern() {
	int index = get_pattern_index(currentPattern);
	int count = get_pattern_count();
	index = (index + count - 1) % count;
	set_editor_pattern(get_pattern_by_index(index));
}

void buzzeditor::add_track() {
	MessageBox(hFrameWnd, "Add Track", "Bla bla", MB_OK);
}

void buzzeditor::remove_last_track() {
	MessageBox(hFrameWnd, "Remove Last Track", "Bla bla", MB_OK);
}

void buzzeditor::set_editor_pattern(CPattern* pat) {
	currentPattern = pat;
	editorplugin->machine2->SetEditorPattern(pat);
	update_pattern_combo_selection();
}

CPattern* buzzeditor::get_pattern_by_name(std::string name) {
	for (std::vector<CPattern*>::iterator i = editorplugin->patterns.begin(); i != editorplugin->patterns.end(); ++i) {
		if ((*i)->name == name)
			return *i;
	}
	return 0;
}

CPattern* buzzeditor::get_pattern_by_id(int id) {
	return editorplugin->patterns[id];
}

CPattern* buzzeditor::get_pattern_by_index(int index) {
	// TODO: skip NULL patterns
	return editorplugin->patterns[index];
}

int buzzeditor::get_pattern_index(CPattern* pat) {
	int index = 0;
	for (std::vector<CPattern*>::iterator i = editorplugin->patterns.begin(); i != editorplugin->patterns.end(); ++i) {
		if (*i != 0) {
			if (*i == pat) return index;
			index++;
		}
	}
	return 0;
}

int buzzeditor::get_pattern_count() {
	int count = 0;
	for (std::vector<CPattern*>::iterator i = editorplugin->patterns.begin(); i != editorplugin->patterns.end(); ++i) {
		if (*i != 0) count++;
	}
	return count;
}

}
