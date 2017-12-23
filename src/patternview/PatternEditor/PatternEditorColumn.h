#pragma once

// the data type
enum {
	pattern_column_type_note   = 0,
	pattern_column_type_switch = 1,
	pattern_column_type_byte   = 2,
	pattern_column_type_word   = 3,
};

// the control type
enum {
	pattern_column_control_default,
	pattern_column_control_note,
	pattern_column_control_switch,
	pattern_column_control_byte,
	pattern_column_control_word,
	pattern_column_control_slider,
	pattern_column_control_button,
	pattern_column_control_pianoroll,
	pattern_column_control_pattern,
	pattern_column_control_collapsed,
	pattern_column_control_envelope,
	pattern_column_control_char,
	pattern_column_control_harmonic,
	pattern_column_control_matrix,
	pattern_column_control_wave,
};

// visual flags
enum {
	pattern_column_flagtype_none,
	pattern_column_flagtype_wave,
	pattern_column_flagtype_volume,
};

class CPatternEditorInner;
struct PE_column;

class CColumnEditor
{
  public:

	CPatternEditorInner* owner;
	PE_column* column;

	virtual ~CColumnEditor() {}

	// painting
	COLORREF GetBkColor(int ptn_row) const;
	COLORREF GetTextColor(int ptn_row, int v) const;
	COLORREF GetUnderlineColor(int ptn_row, int mode) const;
	virtual void PreviewValues(CDC& dc, int row, int rows) = 0;
	virtual void RenderValues(CDC& dc, int row, int rows) = 0;
	virtual bool IsSelectable();

	// binding
	virtual int GetWidth() const = 0;
	virtual int GetDigits() const = 0;
	virtual int CreateCursorColumns(int idx, int x) = 0;

	// values
	virtual void PreInsertValue(int time, int value);
	virtual void PostInsertValue(int time, int value);
	virtual void PreDeleteValue(int time, int value);
	virtual void PostDeleteValue(int time, int value);
	virtual void SetFilteredValues(const std::vector<int>& values) = 0;

	// actions
	virtual bool Char(WPARAM wParam, LPARAM lParam);
	virtual bool LButtonDown(WPARAM wParam, LPARAM lParam);
	virtual bool LButtonUp(WPARAM wParam, LPARAM lParam);
	virtual bool MouseMove(WPARAM wParam, LPARAM lParam);
	virtual bool DoubleClick(WPARAM wParam, LPARAM lParam);
	virtual bool Special1(); // space
	virtual bool Special2(); // ctrl+space
	virtual bool Special3(); // ctrl+shift+space
	virtual bool Special4(); // shift+space
	virtual bool Special5(); // enter
	virtual bool Special6(); // shift+enter
	virtual bool KeyDown(WPARAM wParam, LPARAM lParam, bool first_press);
	virtual bool KeyUp(WPARAM wParam, LPARAM lParam);
	virtual void OctaveChange(int nOctave);

	// static factories / info
	static CColumnEditor* Create(int control);
	static std::string GetColumnControlName(int mode);
	static int GetAvailableColumnControls(int type, int defaultcontrol, std::vector<int>& modes);
	static int GetDefaultTypeControl(int type/*, int flags*/);
	static std::string FormatValue(int v, int m, int type, int novalue);
};
