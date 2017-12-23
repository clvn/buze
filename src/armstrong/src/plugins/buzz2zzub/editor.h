#pragma once

namespace buzz2zzub {

struct plugin;

struct buzzeditor {
	buzz2zzub::plugin* editorplugin;
	HWND hContainerWnd;
	HWND hFrameWnd;
	HWND hCustomWnd;
	HWND hWnd;
	HWND hPatternCombo;
	static ATOM atomWndClass;
	CPattern* currentPattern;

	buzzeditor();
	void create(buzz2zzub::plugin* _effect, std::string name);
	void init_dialog(HWND _hWnd);
	void idle();
	void close();
	BOOL pre_translate_message(MSG* pMsg);

	void create_hostwindow(std::string name);
	void create_framewindow();

	void create_pattern();
	void delete_pattern();
	void show_properties();
	void next_pattern();
	void previous_pattern();
	void add_track();
	void remove_last_track();

	void bind_pattern_combo();
	void update_pattern_combo_selection();

	void set_editor_pattern(CPattern* pat);
	CPattern* get_pattern_by_name(std::string name);
	CPattern* get_pattern_by_id(int id);
	CPattern* get_pattern_by_index(int index);
	int get_pattern_index(CPattern* pat);
	int get_pattern_count();
};

}
