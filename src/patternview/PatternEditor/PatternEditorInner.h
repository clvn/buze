#pragma once

#include "PatternEditorHarmony.h"
#include "PatternEditorScroller.h"

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

class CColumnEditor;

// ---------------------------------------------------------------------------------------------------

enum {
// Inner WM_NOTIFY codes sent to parent:
	ID_PATTERNEDITORINNER_EDIT = 1000,
// Inner WM_COMMAND codes sent to parent:
	ID_PATTERNEDITORINNER_SCROLLED = 1001,
	ID_PATTERNEDITORINNER_SELCHANGED = 1002,
	ID_PATTERNEDITORINNER_SELDRAG = 1003,
	ID_PATTERNEDITORINNER_SELDROP = 1004,
	ID_PATTERNEDITORINNER_MOVECURSOR = 1005,
	ID_PATTERNEDITORINNER_SELDROPCELL = 1006,
	ID_PATTERNEDITORINNER_RESIZED = 1007,
	ID_PATTERNEDITORINNER_NOTE = 1008,
	ID_PATTERNEDITORINNER_PIANO_TRANSLATE = 1014,
	ID_PATTERNEDITORINNER_PIANO_EDIT = 1015,
// Control WM_COMMAND codes sent to parent:
	ID_PATTERNEDITORCONTROL_TRACKMUTE = 1010,
	ID_PATTERNEDITORCONTROL_TRACKSOLO = 1011,
	//ID_PATTERNEDITORCONTROL_PLAYFROMROW = 1012,
	//ID_PATTERNEDITORCONTROL_HOLDROW = 1013,
};

// ---------------------------------------------------------------------------------------------------

struct PE_NMHDR : NMHDR {
	int plugin_id, group, track, column, row;
	int value;
	int digit;
	bool reactive;
 	int id;
 	int meta;
};

struct PE_NMHDR_note : NMHDR {
	int plugin_id, group, track, column, row;
	int value;
};

struct PE_NMHDR_piano : NMHDR {
	std::vector<int> eventids;
	int timeshift;
	int pitchshift;
	int mode;
};

struct PE_NMHDR_pianoedit : NMHDR {
	int id, pluginid, time, note, length;
};

// ---------------------------------------------------------------------------------------------------

typedef boost::unordered_set<int> event_holes_t;

// ---------------------------------------------------------------------------------------------------

struct PE_value {
	int id, time, value, meta;
	PE_value() : id(-1), time(-1), value(-1), meta(-1) {}
	PE_value(int id, int time, int value, int meta) : id(id), time(time), value(value), meta(meta) {}
};

using namespace boost::multi_index;

struct by_id {};
struct by_time {};

typedef multi_index_container<
	PE_value,
	indexed_by<
		hashed_unique<
			tag<by_id>,
			member<PE_value, int, &PE_value::id>
		>,
		ordered_non_unique<
			tag<by_time>,
			member<PE_value, int, &PE_value::time>
		>
	>
> PE_values;

typedef PE_values::index<by_id  >::type PE_values_by_id;
typedef PE_values::index<by_time>::type PE_values_by_time;

// ---------------------------------------------------------------------------------------------------

struct PE_column_pos {
	PE_column_pos() {}
	PE_column_pos(int plugin_id, int group, int track, int column) :
		plugin_id(plugin_id), group(group), track(track), column(column)
	{}
	int plugin_id;
	int group;
	int track;
	int column;
};

// support for unordered_map
inline bool operator==(PE_column_pos const& lhs, PE_column_pos const& rhs) {
	return (lhs.plugin_id == rhs.plugin_id)
		&& (lhs.group == rhs.group)
		&& (lhs.track == rhs.track)
		&& (lhs.column == rhs.column)
	;
}
inline size_t hash_value(PE_column_pos const& colpos) {
	return (colpos.plugin_id << 22) // 1024 pluginids
		 | (colpos.group     << 20) //    4 groups
		 | (colpos.track     << 10) // 1024 tracks
		 | (colpos.column         ) // 1024 columns
	;
}

// ---------------------------------------------------------------------------------------------------

struct PE_select_pos : PE_column_pos {
	int row;
};

struct PE_cursor_pos : PE_select_pos {
	int digit;
};

struct PE_scroll_pos : PE_select_pos {
	int offset;
};

struct PE_column : PE_column_pos {
	int index;
	int type;
	int novalue, minvalue, maxvalue;
	int unit;
	int track_index;
	bool spacer;
	int control, defaultcontrol, flagtype;
	int is_collapsed;
	std::string paramname;
	CColumnEditor* editor;

	PE_values values;
	PE_values_by_id& values_by_id;
	PE_values_by_time& values_by_time;

	PE_column()
	:
		values(),
		values_by_id(values.get<by_id>()),
		values_by_time(values.get<by_time>())
	{}

	int GetValue(int time, int* value, int* meta) const;
	int GetPatternValue(int time, int* value, int* length) const;
	void SetValue(int id, int time, int value, int meta);
	void SetValueInitial(int id, int time, int value, int meta);
private:
	int GetPatternLength(int value) const;
};

struct PE_cursorcolumn {
	int index;
	int column;
	int digit;
	int offset;
	int unit;
	int width;
};

struct PE_track {
	std::string name;
	int unit;
	int width;

	int plugin_id;
	int group;
	int track;
	bool is_muted;

	int first_column_idx;
	int last_column_idx;
	int wave_idx;
	int vol_idx;
};

struct PE_patterninfo {
	std::string name;
	int length;
	int loop_begin;
	int loop_end;
	int loop_enabled;
};

enum PE_themeindex {
	PE_BG,
	PE_BG_Dark,
	PE_BG_VeryDark,
	PE_Selection,
	PE_Cursor,
	PE_TextValue,
	PE_TextNote,
	PE_TextNoteOff,
	PE_TextTrigger,
	PE_TextWave,
	PE_TextVolume,
	PE_TextShade,
	PE_TextRows,
	PE_TextTrack,
	PE_TextTrackMuted,
	PE_TextTrackMuted_BG,
	PE_LoopPoints,
	PE_LoopPoints_Disabled,
	PE_PlaybackPos,
	PE_Divider,
	PE_Hidden,
	PE_Control,
	PE_Trigger,
	PE_Trigger_Shadow,
	PE_Trigger_Highlight,
	PE_Note_1,
	PE_Note_2,
	PE_Note_3,
	PE_Note_4,
	PE_Note_5,
	PE_Note_6,
	PE_Note_7,
	PE_Note_8,
	PE_Note_9,
	PE_Note_10,
	PE_Note_11,
	PE_Note_12,
	PE_THEMEINDEX_SIZE
};

struct PE_note_on {
	PE_note_on() {}
	PE_note_on(int plugin_id, int group, int track, int column, int value)
	:	plugin_id(plugin_id), group(group), track(track), column(column), value(value)
	{}
	int plugin_id, group, track, column, value;
};

typedef boost::unordered_map<WORD, int> keyjazz_key_map_t;

// ---------------------------------------------------------------------------------------------------

class CPatternEditorInner : public CWindowImpl<CPatternEditorInner>
{
  public:

	// data
	int pattern_rows;
	int editor_rows;
	int editor_units;
	std::vector<PE_column*> columns;
	std::vector<PE_cursorcolumn> cursorcolumns;
	std::map<int, PE_cursorcolumn*> screenunits;
	std::vector<PE_track> tracks;
	std::vector<PE_patterninfo> patterninfos;
	std::map<string, int> patternnames;

	// sizing / scrolling
	CPoint scroll;
	bool resizing;

	// painting
	CClientDC* clientDC;
	CBrush black_brush, white_brush;
	CPen black_pen;
	CPen looppoints_pen;
	CPen looppointsdisabled_pen;
	CPen playbackpos_pen;
	CPen divider_pen;
	CPen hidden_pen;
	CPen thin_pen;
	CBitmap caret_bitmap;
	CFont track_font;

	// preferences
	COLORREF colors[PE_THEMEINDEX_SIZE];
	bool sticky_selection;
	bool horizontal_entry;
	bool notecolors_enabled;

	// mouse
	int mouse_mode;
	enum {
		mouse_mode_none,
		mouse_mode_down,
		mouse_mode_drag,
		mouse_mode_mark,
		mouse_mode_control,
	};
	int drag_mode;
	enum {
		drag_mode_none,
		drag_mode_selection,
		drag_mode_cell,
	};
	CPoint drag_to;
	CPoint drag_offset;
	CPoint mousemove_previous;
	int dragslider_column;
	bool double_clicking;
	int mouse_down_unit;

	// cursor / selection
	enum {
		recenter_mode_none,
		recenter_mode_only,
		recenter_mode_also,
	};
	CPoint cursor;
	CPoint select_from, select_to;		// unsorted
	CPoint select_begin, select_end;	// sorted
	CSize caret_bitmap_size;
	bool cursor_offscreen;

	// presentation
	int skip;
	int resolution;
	int dark_row, verydark_row;
	CSize font_size;

	// behaviour
	int step;
	int octave;

	// loops / play pos
	int last_play_pos;
	int loop_begin_pos;
	int loop_end_pos;
	bool loop_enabled;

	// keyboard
	int kb_delay_ms;
	int kb_speed_ms;
	int kb_fastdelay_ms;
	bool timer_active;
	bool repeat_transition;

	// chord entry
	bool entering_chord;
	CPoint entering_chord_origin;
	bool did_chord_step;

	DECLARE_WND_CLASS("PatternEditorInner")

	BEGIN_MSG_MAP_EX(CPatternEditorInner)
		MESSAGE_HANDLER_EX(WM_CREATE, OnCreate)
		MESSAGE_HANDLER_EX(WM_CLOSE, OnClose)
		MESSAGE_HANDLER_EX(WM_SIZE, OnSize)
		MESSAGE_HANDLER_EX(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER_EX(WM_PAINT, OnPaint)
		MESSAGE_HANDLER_EX(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER_EX(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER_EX(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER_EX(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		MESSAGE_HANDLER_EX(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER_EX(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER_EX(WM_RBUTTONDBLCLK, OnRButtonDblClk)
		MESSAGE_HANDLER_EX(WM_XBUTTONDOWN, OnXButtonDown)
		MESSAGE_HANDLER_EX(WM_XBUTTONDBLCLK, OnXButtonDown)
		MESSAGE_HANDLER_EX(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER_EX(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER_EX(WM_CHAR, OnChar)
		MESSAGE_HANDLER_EX(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER_EX(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER_EX(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER_EX(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER_EX(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER_EX(WM_MOUSEWHEEL, OnForward)
		MESSAGE_HANDLER_EX(WM_TIMER, OnTimer)
	END_MSG_MAP()

	CPatternEditorScroller& scroller;
	CPatternEditorInner(CPatternEditorScroller& scroller);
	~CPatternEditorInner();

	// wm's
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnXButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnForward(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam);

	// scrolling
	CSize GetScreenSize() const;
	CPoint GetMaxScroll() const;
	void InitScroll();
	void UpdateScrollbars();
	bool ScrollTo(int x, int y);
	bool ScrollTo(CPoint pt);

	// painting
	void InvalidateSelection(RECT* rcPrevSel = NULL);
	void UpdatePlayPosition(int pos);

	// unit calculations
	bool GetScreenPosition(int col, int row, POINT* pt);
	bool GetSelectionRectScreen(RECT* rc) const;
	bool GetRangeRectScreen(int from_col, int from_row, int to_col, int to_row, RECT* rc) const;
	bool GetDragtoRectScreen(RECT* rc) const;
	bool GetSortedSelectionRectAbsolute(RECT* rc) const;
	bool GetDragtoRectAbsolute(RECT* rc) const;
	bool GetDragtoPointAbsolute(POINT* pt) const;
	int ScreenUnitsToCursorColumn(int x) const;
	int CursorColumnToScreenUnits(int column) const;
	bool GetSelectionRect(RECT* rc) const;

	// preferences
	void SetFont(HFONT hFont, bool bRedraw = true);
	void SetThemeColor(PE_themeindex index, COLORREF color);
	void SetNoteOffStr(std::string const& s);
	void SetNoteCutStr(std::string const& s);
	void SetBGNote(std::string const& s);
	void SetBGByte(std::string const& s);
	void SetBGSwitch(std::string const& s);
	void SetBGWord(std::string const& s);
	void SetStickySelection(bool state);
	void SetHorizontalEntry(bool state);
	void SetHorizontalScrollMode(bool state);
	void SetVerticalScrollMode(bool state);
	void SetColoredNotes(bool state);
	void UpdateTheme();

	// mouse
	void ResetMouseMode();
	void DoMouseMove();
	CPoint PointToCursorPoint(CPoint pt, CPoint* offset = 0);

	// keyboard
	void CalcKeyboardSpeed();
	void DoKeyboard(int kc);

	// cursor
	bool horizontal_scroll_mode;
	bool vertical_scroll_mode;
	bool ScrollToCursorPointHorizontal(CPoint pt, CPoint old_scroll);
	bool ScrollToCursorPoint(CPoint pt, CPoint old_scroll, bool allow_hcenter);
	CPoint GetScrollToCursorPoint(CPoint pt, CPoint old_scroll, bool allow_hcenter);
	bool ScrollToCursorPointCentered(CPoint pt, CPoint old_scroll);
	bool ScrollToCursorPointCenteredHorizontal(CPoint pt, CPoint old_scroll);
	CPoint GetScrollToCursorPointCentered(CPoint pt, CPoint old_scroll);
	bool ScrollVertical(int y_offset, CPoint old_scroll);
	bool ScrollVerticalAndMove(CPoint pt, int y_offset, CPoint old_scroll);
	bool IsCursorPointOffscreen(CPoint pt) const;
	CPoint GetCursor() const;
	void SetCursor(int x, int y);
	void SetCursorAdjust(int x, int y);
	void MoveCursor(int x, int y, bool affectsel, int recenter_mode);
	void AdjustCursor(int x, int y, bool affectsel, int recenter_mode, CPoint old_scroll, bool set_mode);
	void MoveLeft();
	void MoveUpLeft();
	void MoveUp();
	void MoveUpRight();
	void MoveRight();
	void MoveDownRight();
	void MoveDown();
	void MoveDownLeft();
	void MovePgUp();
	void MovePgDn();
	void MoveBeginLoop();
	void MoveEndLoop();
	void MoveBol();
	void MoveEol();
	void MoveNextTrack();
	void MovePrevTrack();
	void MoveRightCol();
	void MoveLeftCol();
	void StepCursor();
	void StepCursorSmart();
	void StepCursorReset();
	void StepCursorToNoteColumn(bool direction);

	// selection
	bool HasSelection() const;
	void ClearSelection();
	void Unselect();
	void Reselect();
	void SelectRange(int from_col, int from_row, int to_col, int to_row, bool forceinvalid = false);
	void SelectRangeAbsolute(int from_col, int from_row, int to_col, int to_row);
	void SortSelection();
	void InitSelection();
	bool IsCursorInSel() const;
	BLENDFUNCTION blendFunc;
	CBitmap m_selectionImg;
	CDC m_selectionDC;

	// caret
	void AllocateCaretBitmap(int width, int height);
	void UpdateCaret();
	void InvalidateCaret();

	// values
	void ClearValue();
	void NotifyEdit(int plugin_id, int group, int track, int column, int row, int digit, int value, int meta, bool reactive, int id = -1);
	void NotifyPianoTranslate(std::vector<int>& events, int timeshift, int pitchshift, int mode);
	void NotifyPianoEdit(int id, int pluginid, int time, int note, int length);

	// binding
	void Init();
	void SetHighlightRows(int verydarkrow, int darkrow);
	void SetSkip(int n);
	void SetPatternRows(int n);
	void SetResolution(int n);
	void AllocatePattern(bool alloc_patimg = true);
	void AllocatePatternVertical(bool alloc_patimg = true);
	void UpdateColumnUnits();
	void UpdateClientExt();
	void UpdateClientOrg();
	void ClearColumns();
	void AddColumn(int plugin_id, int group, int track, int column, int type, int novalue, int minvalue, int maxvalue, int control, int defaultcontrol, int flagtype, int is_collapsed, std::string paramname);
	void AddTrack(std::string const& name, int plugin_id, int group, int track, bool is_muted);
	void AddPianoRoll();
	void SetPatternInfos(std::vector<PE_patterninfo> const& infos, std::map<string, int> const& names);
	void SetColumnEditor(PE_column& col);

	// state
	void SetOctave(int n);
	void SetStep(int n);

	// column actions
	bool DoColumnSpecial1();
	bool DoColumnSpecial2();
	bool DoColumnSpecial3();
	bool DoColumnSpecial4();
	bool DoColumnSpecial5();
	bool DoColumnSpecial6();
	LRESULT ForwardAction(DWORD_PTR id);

	// encapsulation helpers
	PE_cursorcolumn const& GetCursorColumnAtCursor() const;
	PE_column const& GetColumnAtCursor() const;
	PE_column const& GetColumn(int n) const;
	CPoint GetCursorAbsolute() const;
	CRect GetSortedSelectionOrCursorAbsolute() const;
	bool GetUnsortedSelectionRangeAbsolute(POINT* ptFrom, POINT* ptTo) const;
	int GetColumnCount() const;
	int GetPatternRows() const;
	int GetSkip() const;
	int GetResolution() const;
	int GetCursorRowAbsolute() const;
	PE_cursor_pos GetCursorPos() const;
	PE_select_pos GetSelectFromPos() const;
	PE_select_pos GetSelectToPos() const;
	///PE_scroll_pos GetPatternScrollPos() const;

	// selection holes
	int transpose_mask[12][3];
	void MaskNote(int note, int meta, bool state);
	void MaskNoteReset(bool state);
	void SetEventHoles();
	event_holes_t event_holes;

	// harmonic xpose
	boost::array<int, 12> transpose_set_enabled;
	boost::array<int, 12> transpose_set_disabled;
	bool transpose_set_mode;

	// harmony
	Harmony::HSys hsys;
	bool hsys_enabled;
	void HSysEnable(bool state);
	int HSysResolveShade(int value, int meta, int time);
	void HSysInvalidate(int time, int length);
	int HSysGetContextLength(int x, int y);

	// scroller
	void InvalidatePreview(RECT* rcPreview);
	void RenderPreview(bool initial);
	void BindPatternImg();
	CRect rcPreviewInvalid;

	// rearranging
	bool dirty_centeroncursor;
	bool dirty_scrolltocursor;

	// held notes
	void HoldNote(WPARAM wParam, int v);
	void ReleaseNote(WPARAM wParam);
	void ReleaseAllNotes();
	void HoldAllNotes();
	typedef std::map<WORD, PE_note_on> heldnotes_t;
	heldnotes_t heldnotes;
	bool all_notes_held;

	// note entry modes
	int GetNotesAffectMode() const;
	void SetNotesAffectMode(int mode);
	bool notes_affect_waves;
	bool notes_affect_volumes;

	// keyjazz
	keyjazz_key_map_t* keyjazz_key_map;
};
