#pragma once

struct vsteditor {
	static std::map<HWND, vsteditor*> editors;
	AEffect* effect;
	HWND hWnd;
	HWND pluginWnd;
	WNDPROC pluginWndProc;
	static ATOM atomWndClass;

	vsteditor();
	~vsteditor();
	void create(AEffect* _effect, std::string name, HWND hParentWnd, bool asControl);
	void init_dialog(HWND _hWnd);
	void update_size(bool center = false);
	void idle();
	void destroy();
	void show(bool visible);
};
