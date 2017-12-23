#include "stdafx.h"
#include "resource.h"
#include "PatternEditorInner.h"
#include "PatternEditorColumn.h"
#include "PatternEditorNoteKeys.h"
#include "utils.h"
#include "PrefGlobals.h"

/// todo add ID EXISTS to all notifyedits

// ---------------------------------------------------------------------------------------------------------------
// COLUMN EDITOR BASE
// ---------------------------------------------------------------------------------------------------------------

bool CColumnEditor::IsSelectable() {
	return true;
}

COLORREF CColumnEditor::GetBkColor(int ptn_row) const {
	COLORREF bkColor;

	if (ptn_row % (owner->verydark_row * owner->resolution) == 0)
		bkColor = owner->colors[PE_BG_VeryDark];
	else
	if (ptn_row % (owner->dark_row * owner->resolution) == 0)
		bkColor = owner->colors[PE_BG_Dark];
	else
		bkColor = owner->colors[PE_BG];

	if (column->control == pattern_column_control_pattern)
		return AverageRGB(owner->colors[PE_BG], bkColor, 4);

	if (column->control == pattern_column_control_harmonic)
		return AverageRGB(owner->colors[PE_BG], bkColor, 4);

	return bkColor;
}

COLORREF CColumnEditor::GetTextColor(int ptn_row, int v) const {
	if (v == column->novalue)
		return owner->colors[PE_TextShade];

	if (column->control == pattern_column_control_note || column->control == pattern_column_control_pianoroll || column->control == pattern_column_control_matrix) {
		if ((v == 255) || (v == 254)) {
			return owner->colors[PE_TextNoteOff];
		} else {
			if (owner->notecolors_enabled) {
				int basenote = (v & 0xF) - 1;
				return owner->colors[PE_Note_1 + basenote];
			} else {
				return owner->colors[PE_TextNote];
			}
		}
	}

	if (column->flagtype == pattern_column_flagtype_wave)
		return owner->colors[PE_TextWave];

	if (column->flagtype == pattern_column_flagtype_volume)
		return owner->colors[PE_TextVolume];

	if (column->control == pattern_column_control_pattern)
		return owner->colors[PE_TextTrigger];

	if (column->control == pattern_column_control_slider)
		return owner->colors[PE_Control];

	if (column->control == pattern_column_control_button)
		return owner->colors[PE_Control];

	return owner->colors[PE_TextValue];
}

COLORREF CColumnEditor::GetUnderlineColor(int ptn_row, int mode) const {
	COLORREF underline_color;

	using namespace Harmony;
	switch ((MetaSetProperty)mode) {
		case outside_tone:
			underline_color = RGB(0xFF, 0x00, 0x00);
			break;
		case active_tone:
			underline_color = RGB(0x00, 0xFF, 0x00);
			break;
		case root_tone:
			underline_color = RGB(0x00, 0xC0, 0xA0);
			break;
		case mode_tone:
			underline_color = RGB(0xFF, 0xFF, 0x00);
			break;
		case bass_tone:
			underline_color = RGB(0x80, 0x80, 0x00);
			break;
		case passing_tone:
			underline_color = RGB(0xFF, 0x00, 0xFF);
			break;
		default:
			underline_color = RGB(0x00, 0x00, 0x00);
			break;
	}

	return AverageRGB(GetBkColor(ptn_row), underline_color, 1);
}

void CColumnEditor::PreInsertValue(int time, int value) {
}

void CColumnEditor::PostInsertValue(int time, int value) {
	int skip_time = time / owner->skip;
	RECT rc;
	owner->GetRangeRectScreen(column->index, skip_time, column->index, skip_time, &rc);
	owner->clientDC->LPtoDP(&rc);
	owner->InvalidateRect(&rc, FALSE);
	RECT rcPreview = { column->index, skip_time, column->index, skip_time };
	owner->InvalidatePreview(&rcPreview);
}

void CColumnEditor::PreDeleteValue(int time, int value) {
}

void CColumnEditor::PostDeleteValue(int time, int value) {
	int skip_time = time / owner->skip;
	RECT rc;
	owner->GetRangeRectScreen(column->index, skip_time, column->index, skip_time, &rc);
	owner->clientDC->LPtoDP(&rc);
	owner->InvalidateRect(&rc, FALSE);
	RECT rcPreview = { column->index, skip_time, column->index, skip_time };
	owner->InvalidatePreview(&rcPreview);
}

bool CColumnEditor::Special1() {
	return false;
}

bool CColumnEditor::Special2() {
	return false;
}

bool CColumnEditor::Special3() {
	return false;
}

bool CColumnEditor::Special4() {
	return false;
}

bool CColumnEditor::Special5() {
	return false;
}

bool CColumnEditor::Special6() {
	return false;
}

bool CColumnEditor::LButtonDown(WPARAM wParam, LPARAM lParam) {
	return false;
}

bool CColumnEditor::LButtonUp(WPARAM wParam, LPARAM lParam) {
	return false;
}

bool CColumnEditor::MouseMove(WPARAM wParam, LPARAM lParam) {
	return false;
}

bool CColumnEditor::DoubleClick(WPARAM wParam, LPARAM lParam) {
	return false;
}

bool CColumnEditor::Char(WPARAM wParam, LPARAM lParam) {
	return false;
}

bool CColumnEditor::KeyDown(WPARAM wParam, LPARAM lParam, bool first_press) {
	return false;
}

bool CColumnEditor::KeyUp(WPARAM wParam, LPARAM lParam) {
	return false;
}

void CColumnEditor::OctaveChange(int nOctave) {
}

// ---------------------------------------------------------------------------------------------------------------

namespace {
void DrawRectangle(CDC& dc, RECT& rc, COLORREF color) {
	CPen pen; pen.CreatePen(PS_SOLID, 1, color);
	CPenHandle oldpen = dc.SelectPen(pen);
	CBrushHandle oldbrush = dc.SelectStockBrush(NULL_BRUSH);
	dc.Rectangle(&rc);
	dc.SelectBrush(oldbrush);
	dc.SelectPen(oldpen);
}
}

// ---------------------------------------------------------------------------------------------------------------
// NOTE
// ---------------------------------------------------------------------------------------------------------------

class CNoteColumn : public CColumnEditor
{
  public:

	virtual int GetWidth() const {
		return 3;
	}

	virtual int GetDigits() const {
		return 2;
	}

	virtual int CreateCursorColumns(int idx, int x) { 
		PE_cursorcolumn cnote = {
			// idx	column			digit	offset	unit	width
			idx,	column->index,	0,		0,		x+0,	2
		};
		PE_cursorcolumn coct = {
			// idx	column			digit	offset	unit	width
			idx+1,	column->index,	1,		2,		x+2,	1
		};
		owner->cursorcolumns.push_back(cnote);
		owner->cursorcolumns.push_back(coct);

		return GetWidth();
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		int end_row = row + rows;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		for (int j = row; j < end_row; ++j) {
			if ((i != column->values_by_time.end()) && (i->time == time)) {
				RECT rcCell = { column->unit, j, column->unit + GetWidth(), j + 1 };
				dc.FillSolidRect(&rcCell, GetTextColor(time, i->value));
			}
			time += owner->skip;
			while ((i != column->values_by_time.end()) && (i->time < time)) ++i;
		}	
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		USES_CONVERSION; 

		int tx = owner->font_size.cx * column->unit;
		int ty = owner->font_size.cy * row;

		int time = row * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);

		for (int j = 0; j < rows; ++j) {
			int v;
			int m;
			if ((i != column->values_by_time.end()) && (i->time == time)) {
				v = i->value;
				m = i->meta;
			} else {
				v = column->novalue;
				m = 0;
			}

			std::string str = FormatValue(v, m, column->type, column->novalue);

			COLORREF text_color = GetTextColor(time, v);
			COLORREF bk_color = GetBkColor(time);
			bool do_underline = false;

			if ((v != column->novalue) && (v != 255) && (v != 254)) {
				if (owner->hsys_enabled)
					do_underline = true;

				if (owner->HasSelection()) {
					boost::unordered_set<int>::iterator k = owner->event_holes.find(i->id);
					if (k != owner->event_holes.end()) {
						bk_color = InvertRGB(bk_color);
						text_color = InvertRGB(text_color);
					}
				}
			}

			dc.SetTextColor(text_color);
			dc.SetBkColor(bk_color);
			dc.TextOut(tx, ty, CA2T(str.c_str()), str.length());

			if (do_underline) {
				int mode = owner->HSysResolveShade(v, m, time);

				RECT rcUnderline = {
					tx,
					ty + owner->font_size.cy - 1,
					tx + (GetWidth() * owner->font_size.cx),
					ty + owner->font_size.cy
				};

				dc.FillSolidRect(&rcUnderline, GetUnderlineColor(time, mode));
			}

			time += owner->skip;

			bool draw_hidden = false;
			while ((i != column->values_by_time.end()) && (i->time < time)) {
				if ((i->time % owner->skip) != 0) draw_hidden = true;
				++i;
			}
			if (draw_hidden) {
				int hid_x = tx + (GetWidth() * owner->font_size.cx) + (owner->font_size.cx / 2);
				int hid_y_top = ty + 2;
				int hid_y_bot = ty + owner->font_size.cy - 3;

				CPenHandle old_pen = dc.SelectPen(owner->hidden_pen);
				dc.MoveTo(hid_x, hid_y_top);
				dc.LineTo(hid_x, hid_y_bot);
				dc.SelectPen(old_pen);
			}

			ty += owner->font_size.cy;
		}
	}

	void GetSmartLastNote(int* v, int* m) const {
		PE_column const& col = owner->GetColumnAtCursor();
		int time = owner->cursor.y * owner->skip;

		PE_values_by_time::iterator i = column->values_by_time.upper_bound(time);
		if ((i != column->values_by_time.begin()) && (column->values_by_time.size() != 0)) {
			--i;
			*v = i->value;
			*m = i->meta;
		} else {
			*v = 1 | (4 << 4); // default C-4
			*m = 0;
		}
	}

	int GetNoteFromKeyMap(int octave, WORD keychar) const {
		keyjazz_key_map_t const& keyjazz_key_map = *owner->keyjazz_key_map;

		keyjazz_key_map_t::const_iterator i = keyjazz_key_map.find(keychar);
		if (i != keyjazz_key_map.end()) {
			int note_or_cmd = (*i).second;
			if (note_or_cmd < 0 || note_or_cmd == 255 || note_or_cmd == 254) {
				return note_or_cmd;
			} else {
				int cX = 1 | (owner->octave << 4);
				int v = transposeNote(cX, note_or_cmd - 12);
				return v;
			}
		} else {
			return -1;
		}
	}

	int GetNote(WPARAM wParam) const {
		int keychar = getShiftedNumeric(wParam); // for supporting ie, Shift+2
		if (keychar != -1)
			keychar = '0' + keychar;
		else {
			if (owner->entering_chord) {
				// do not accept note if entering_chord (shift) and a non-alphanumeric key
				// prevents double-action if ',' is mapped to a note, and shift+',' = '<' which is mapped to an accelerator
				if (!((wParam >= 0x30 && wParam <= 0x39) || (wParam >= 0x41 && wParam <= 0x5A))) {
					return -1;
				}
			}
			keychar = wParam;
		}

		return GetNoteFromKeyMap(owner->octave, (WORD)keychar);
	}

	virtual bool KeyUp(WPARAM wParam, LPARAM lParam) {
		int v = GetNote(wParam);

		switch (v) {
			case 255: // off
			case 254: // cut
				return true;
			case -1: // invalid
				return false;
			case -2: // jazz_lastnote
				owner->ReleaseNote(wParam);
				break;
			case -3: // jazz_track
				owner->ReleaseNote(wParam);
				break;
			case -4: // jazz_row
				owner->ReleaseAllNotes();
				return true;
			case -5: // octave_up
				break;
			case -6: // octave_down
				break;
			default: // notes
				owner->ReleaseNote(wParam);
				break;
		}

		return true;
	}

	virtual bool KeyDown(WPARAM wParam, LPARAM lParam, bool first_press) {
		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());

		int v = exists ? i->value : column->novalue;
		int id = exists ? i->id : -1;
		int meta = -1;
		int nibbleval = 0;

		bool do_edit = true;

		if (ccol.digit == 0) {
			v = GetNote(wParam);

			switch (v) {
				case 255: // off
				case 254: // cut
					if (!first_press) return false;
					break;
				case -1: // invalid
					return false;
				case -2: // jazz_lastnote
					if (!first_press) return false;
					GetSmartLastNote(&v, &meta);
					owner->HoldNote(wParam, v);
					break;
				case -3: // jazz_track
					if (!first_press) return false;
					owner->HoldNote(wParam, v);
					owner->ForwardAction(ID_PATTERNVIEW_JAZZ_TRACKROW);
					return true;
				case -4: // jazz_row
					if (!first_press) return false;
					owner->HoldAllNotes();
					owner->ForwardAction(ID_PATTERNVIEW_JAZZ_ROW);
					return true;
				case -5: // octave_up
					owner->ForwardAction(ID_PATTERNVIEW_OCTAVE_UP);
					return true;
				case -6: // octave_down
					owner->ForwardAction(ID_PATTERNVIEW_OCTAVE_DOWN);
					return true;
				default: // note
					if (!first_press) return false;
					owner->HoldNote(wParam, v);
					break;
			}

			nibbleval = v;
		} else {
			int c = toupper(wParam);
			if (c >= '0' && c <= '9')
				nibbleval = c - '0';
			else
				return -1;

			// disallow octave editing when note is blank or off
			if ((v == column->novalue) || (v == 255) || (v == 254)) {
				do_edit = false;
			}
		}

		if (do_edit) {
			if (v == column->novalue)
				v = 0;

			int nibble = ccol.digit;
			int v0 = v & (0xffff ^ (0xf << (nibble * 4)));
			v = v0 | (nibbleval << (nibble * 4));

			if ((v != 255) && (v != 254) && (meta == -1))
				meta = owner->transpose_set_disabled[(v & 0xF) - 1];

			owner->NotifyEdit(
				column->plugin_id, column->group, column->track, column->column,
				time,
				ccol.digit,
				v, meta,
				true,
				id
			);
		}

		if (owner->entering_chord)
			owner->StepCursorToNoteColumn(true);
		else
			owner->StepCursor();

		return true;
	}

	virtual bool LButtonDown(WPARAM wParam, LPARAM lParam) {
		return false;
		last_pt.x = (signed short)LOWORD(lParam);
		last_pt.y = (signed short)HIWORD(lParam);
		return true;
	}

	///needs meta
	CPoint last_pt;
	virtual bool MouseMove(WPARAM wParam, LPARAM lParam) {
		return false;
		CPoint pt((signed short)LOWORD(lParam), (signed short)HIWORD(lParam));

		int y_offset = last_pt.y - pt.y;
		last_pt = pt;

		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());

		int v = exists ? i->value : column->novalue;
		int id = exists ? i->id : -1;

		if (v == column->novalue)
			v = column->minvalue;
		else {
			int scale;
			switch (ccol.digit) {
				case 0: scale = 1; break;
				case 1: scale = 12; break;
			}
			v = transposeNote(v, y_offset * scale);
		}

		v = clamp(v, column->minvalue, column->maxvalue);

		owner->NotifyEdit(
			column->plugin_id, column->group, column->track, column->column,
			time,
			0,
			v, 0,
			false,
			id
		);

		return true;
	}

	virtual bool Special1() {
		owner->ForwardAction(ID_PATTERNVIEW_NOTECOLUMN_TOGGLENOTEMETA);
		return true;
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}	
};

// ---------------------------------------------------------------------------------------------------------------
// VALUE
// ---------------------------------------------------------------------------------------------------------------

class CValueColumn : public CColumnEditor
{
  public:

	virtual int GetWidth() const {
		switch (column->type) {
			case pattern_column_type_byte:
				return 2;
			case pattern_column_type_word:
				return 4;
			case pattern_column_type_switch:
				return 1;
			default:
				return 0;
		}
	}

	virtual int GetDigits() const {
		return GetWidth();
	}

	virtual int CreateCursorColumns(int idx, int x) {
		int columnunits = GetWidth();
		for (int j = 0; j < columnunits; ++j) {
			PE_cursorcolumn ccol = {
				// idx	column			digit	offset	unit	width
				idx+j,	column->index,	j,		j,		x+j,	1
			};
			owner->cursorcolumns.push_back(ccol);
		}
		return columnunits;
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		int end_row = row + rows;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		for (int j = row; j < end_row; ++j) {
			if ((i != column->values_by_time.end()) && (i->time == time)) {
				RECT rcCell = { column->unit, j, column->unit + GetWidth(), j + 1 };
				dc.FillSolidRect(&rcCell, GetTextColor(time, i->value));
			}
			time += owner->skip;
			while ((i != column->values_by_time.end()) && (i->time < time)) ++i;
		}	
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		USES_CONVERSION; 

		int tx = owner->font_size.cx * column->unit;
		int ty = owner->font_size.cy * row;

		int time = row * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);

		for (int j = 0; j < rows; ++j) {
			int v;
			if ((i != column->values_by_time.end()) && (i->time == time))
				v = i->value;
			else
				v = column->novalue;

			std::string str = FormatValue(v, 0, column->type, column->novalue);

			dc.SetTextColor(GetTextColor(time, v));
			dc.SetBkColor(GetBkColor(time));
			dc.TextOut(tx, ty, CA2T(str.c_str()), str.length());

			time += owner->skip;

			bool draw_hidden = false;
			while ((i != column->values_by_time.end()) && (i->time < time)) {
				if ((i->time % owner->skip) != 0) draw_hidden = true;
				++i;
			}
			if (draw_hidden) {
				int hid_x = tx + (GetWidth() * owner->font_size.cx) + (owner->font_size.cx / 2);
				int hid_y_top = ty + 2;
				int hid_y_bot = ty + owner->font_size.cy - 3;

				CPenHandle old_pen = dc.SelectPen(owner->hidden_pen);
				dc.MoveTo(hid_x, hid_y_top);
				dc.LineTo(hid_x, hid_y_bot);
				dc.SelectPen(old_pen);
			}

			ty += owner->font_size.cy;
		}
	}

	virtual bool Char(WPARAM wParam, LPARAM lParam) {
		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());

		int v = exists ? i->value : column->novalue;
		int id = exists ? i->id : -1;

		int nibbleval = 0;
		int nibble = (GetWidth() - 1) - ccol.digit;

		int c = toupper(wParam);
		if (c >= 'A' && c <= 'F')
			nibbleval = 10 + c - 'A';
		else
		if (c >= '0' && c <= '9')
			nibbleval = c - '0';
		else {
			int n = getShiftedNumeric(c);
			if (n != -1)
				nibbleval = n;
			else
				return false;
		}

		if (v == column->novalue)
			v = 0;

		// ie, (0xFFFF ^ (0xF << (1 * 4))) -> 1111111100001111
		int v0 = v & (0xffff ^ (0xf << (nibble * 4)));
		v = v0 | (nibbleval << (nibble * 4));

		v = clamp(v, column->minvalue, column->maxvalue);

		owner->NotifyEdit(
			column->plugin_id, column->group, column->track, column->column,
			time,
			ccol.digit,
			v, 0,
			true,
			id
		);

		{	bool horiz = owner->horizontal_entry;
			bool shifted = IsShiftDown();
			if (horiz == shifted)
				owner->StepCursor();
			else
				owner->StepCursorSmart();
		}

		return true;
	}

	virtual bool LButtonDown(WPARAM wParam, LPARAM lParam) {
		last_pt.x = (signed short)LOWORD(lParam);
		last_pt.y = (signed short)HIWORD(lParam);
		return true;
	}

	CPoint last_pt;
	virtual bool MouseMove(WPARAM wParam, LPARAM lParam) {
		CPoint pt((signed short)LOWORD(lParam), (signed short)HIWORD(lParam));

		int y_offset = last_pt.y - pt.y;
		last_pt = pt;

		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());

		int v = exists ? i->value : column->novalue;
		int id = exists ? i->id : -1;

		if (v == column->novalue)
			v = column->minvalue;
		else {
			int nibble = (GetWidth() - 1) - ccol.digit;
			int scale = 0x1 << (nibble * 4);
			v += y_offset * scale;
		}

		v = clamp(v, column->minvalue, column->maxvalue);

		owner->NotifyEdit(
			column->plugin_id, column->group, column->track, column->column,
			time,
			0,
			v, 0,
			false,
			id
		);

		return true;
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}
};

// ---------------------------------------------------------------------------------------------------------------
// CHAR
// ---------------------------------------------------------------------------------------------------------------

class CCharColumn : public CColumnEditor
{
  public:

	virtual int GetWidth() const {
		return 1;
	}

	virtual int GetDigits() const {
		return 1;
	}

	virtual int CreateCursorColumns(int idx, int x) {
		PE_cursorcolumn ccol = {
			// idx	column			digit	offset	unit	width
			idx,	column->index,	0,		0,		x,		1
		};
		owner->cursorcolumns.push_back(ccol);
		return 1;
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		int end_row = row + rows;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		for (int j = row; j < end_row; ++j) {
			if ((i != column->values_by_time.end()) && (i->time == time)) {
				RECT rcCell = { column->unit, j, column->unit + GetWidth(), j + 1 };
				COLORREF colorVal = GetTextColor(time, i->value);
				dc.FillSolidRect(&rcCell, colorVal);
			}
			time += owner->skip;
			while ((i != column->values_by_time.end()) && (i->time < time)) ++i;
		}	
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		USES_CONVERSION; 

		int tx = owner->font_size.cx * column->unit;
		int ty = owner->font_size.cy * row;

		int time = row * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);

		for (int j = 0; j < rows; ++j) {
			int v;
			if ((i != column->values_by_time.end()) && (i->time == time))
				v = i->value;
			else
				v = column->novalue;

			COLORREF text_color;
			if ((v != column->novalue) && ((v < column->minvalue) || (v > column->maxvalue)))
				text_color = 0x0000FF; // hehe -- just for debugging
			else
				text_color = GetTextColor(time, v);

			std::string str = FormatChar(v, column->novalue);

			dc.SetBkColor(GetBkColor(time));
			dc.SetTextColor(text_color);
			dc.TextOut(tx, ty, CA2T(str.c_str()), str.length());

			time += owner->skip;

			bool draw_hidden = false;
			while ((i != column->values_by_time.end()) && (i->time < time)) {
				if ((i->time % owner->skip) != 0) draw_hidden = true;
				++i;
			}
			if (draw_hidden) {
				int hid_x = tx + (GetWidth() * owner->font_size.cx) + (owner->font_size.cx / 2);
				int hid_y_top = ty + 2;
				int hid_y_bot = ty + owner->font_size.cy - 3;

				CPenHandle old_pen = dc.SelectPen(owner->hidden_pen);
				dc.MoveTo(hid_x, hid_y_top);
				dc.LineTo(hid_x, hid_y_bot);
				dc.SelectPen(old_pen);
			}

			ty += owner->font_size.cy;
		}
	}

	std::string FormatChar(int v, int novalue) {
		static const std::string bg_char = ".";
		if (v != novalue) {
			std::string s;
			s.push_back((char)v);
			return s;
		} else {
			return bg_char;
		}
	}

	virtual bool Char(WPARAM wParam, LPARAM lParam) {
		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());

		int c = (char)wParam;
		if (c < '!' || c > '~') // 0x21 through 0x7E are valid.
			return false;

		int v = (int)c;
		int id = exists ? i->id : -1;

		v = clamp(v, column->minvalue, column->maxvalue);

		owner->NotifyEdit(
			column->plugin_id, column->group, column->track, column->column,
			time,
			ccol.digit,
			v, 0,
			true,
			id
		);

		owner->StepCursor();

		return true;
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}
};

// ---------------------------------------------------------------------------------------------------------------
// SLIDER
// ---------------------------------------------------------------------------------------------------------------

class CSliderColumn : public CColumnEditor
{
  public:

	virtual int GetWidth() const {
		return 8;
	}

	virtual int GetDigits() const {
		return 1;
	}

	virtual int CreateCursorColumns(int idx, int x) {
		PE_cursorcolumn ccol = {
			// idx	column			digit	offset	unit	width
			idx,	column->index,	0,		0,		x,		GetWidth()
		};
		owner->cursorcolumns.push_back(ccol);
		return GetWidth();
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		int end_row = row + rows;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		for (int j = row; j < end_row; ++j) {
			if ((i != column->values_by_time.end()) && (i->time == time)) {
				double norm = double(i->value - column->minvalue) / (column->maxvalue - column->minvalue);
				RECT rcCell = { column->unit, j, column->unit + (GetWidth() * norm), j + 1 };
				COLORREF colorVal = GetTextColor(time, i->value);
				dc.FillSolidRect(&rcCell, colorVal);
			}
			time += owner->skip;
			while ((i != column->values_by_time.end()) && (i->time < time)) ++i;
		}	
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);

		RECT rc = {
			owner->font_size.cx * column->unit + 2, 
			owner->font_size.cy * row + 2, 
			(GetWidth() + column->unit) * owner->font_size.cx - 2, 
			(1 + row) * owner->font_size.cy - 2, 
		};

		for (int j = 0; j < rows; ++j) {
			int v;
			if ((i != column->values_by_time.end()) && (i->time == time))
				v = i->value;
			else
				v = column->novalue;

			dc.FillSolidRect(rc.left - 2, rc.top - 2, GetWidth() * owner->font_size.cx, owner->font_size.cy, GetBkColor(time));

			DrawRectangle(dc, rc, GetTextColor(time, v));

			if (v != column->novalue) {
				float w = rc.right - rc.left;
				float range = column->maxvalue - column->minvalue;
				float f = ((float)(v - column->minvalue) / range) * w;
				RECT rcFill = { rc.left, rc.top, rc.left + f, rc.bottom };
				dc.FillSolidRect(&rcFill, GetTextColor(time, v));
			}

			time += owner->skip;

			bool draw_hidden = false;
			while ((i != column->values_by_time.end()) && (i->time < time)) {
				if ((i->time % owner->skip) != 0) draw_hidden = true;
				++i;
			}
			if (draw_hidden) {
				int tx = rc.left - 2;
				int ty = rc.top - 2;
				int hid_x = tx + (GetWidth() * owner->font_size.cx) + (owner->font_size.cx / 2);
				hid_x -= 1;///
				int hid_y_top = ty + 2;
				int hid_y_bot = ty + owner->font_size.cy - 3;

				CPenHandle old_pen = dc.SelectPen(owner->hidden_pen);
				dc.MoveTo(hid_x, hid_y_top);
				dc.LineTo(hid_x, hid_y_bot);
				dc.SelectPen(old_pen);
			}

			rc.top += owner->font_size.cy;
			rc.bottom += owner->font_size.cy;
		}
	}

	virtual bool LButtonDown(WPARAM wParam, LPARAM lParam) {
		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		POINT pt_cursor = owner->PointToCursorPoint(pt);
		///if (pt_cursor.x == -1) return 0;

		int colpos = pt.x - (column->unit - owner->scroll.x) * owner->font_size.cx;
		float w = GetWidth() * owner->font_size.cx;
		float range = column->maxvalue - column->minvalue;
		int v = column->minvalue + ((float)(colpos) / w) * range;
		v = clamp(v, column->minvalue, column->maxvalue);

		int time = pt_cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());
		int old_v = exists ? i->value : column->novalue;
		int id = exists ? i->id : -1;

		//int old_v;
		//int old_m;
		//column->GetValue(time, &old_v, &old_m);

		if (old_v != v) {
			owner->NotifyEdit(
				column->plugin_id, column->group, column->track, column->column,
				time,
				0,
				v, 0,
				false,
				id
			);
		}

		return true;
	}

	virtual bool MouseMove(WPARAM wParam, LPARAM lParam) {
		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		POINT pt_cursor = owner->PointToCursorPoint(pt);

		int colpos = pt.x - (column->unit - owner->scroll.x) * owner->font_size.cx;
		float w = GetWidth() * owner->font_size.cx;
		float range = column->maxvalue - column->minvalue;
		int v = column->minvalue + ((float)(colpos) / w) * range;
		v = clamp(v, column->minvalue, column->maxvalue);

		int time = pt_cursor.y * owner->skip;

		PE_values_by_time::const_iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());

		int old_v = exists ? i->value : column->novalue;
		int id = exists ? i->id : -1;

		if (v != old_v) {
			owner->NotifyEdit(
				column->plugin_id, column->group, column->track, column->column,
				time,
				0,
				v, 0,
				false,
				id
			);

			owner->ScrollToCursorPoint(pt_cursor, owner->scroll, false);
		}

		return true;
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}
};

// ---------------------------------------------------------------------------------------------------------------
// BUTTON
// ---------------------------------------------------------------------------------------------------------------

class CButtonColumn : public CColumnEditor
{
  public:

	virtual int GetWidth() const {
		return 2;
	}

	virtual int GetDigits() const {
		return 1;
	}

	virtual int CreateCursorColumns(int idx, int x) {
		PE_cursorcolumn ccol = {
			// idx	column			digit	offset	unit	width
			idx,	column->index,	0,		0,		x,		GetWidth()
		};
		owner->cursorcolumns.push_back(ccol);
		return GetWidth();
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		int end_row = row + rows;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		for (int j = row; j < end_row; ++j) {
			if ((i != column->values_by_time.end()) && (i->time == time)) {
				RECT rcCell = { column->unit, j, column->unit + GetWidth(), j + 1 };
				COLORREF colorVal;
				if (i->value == 0)
					colorVal = owner->colors[PE_Control];
				else
					colorVal = GetTextColor(time, i->value);
				dc.FillSolidRect(&rcCell, colorVal);
			}
			time += owner->skip;
			while ((i != column->values_by_time.end()) && (i->time < time)) ++i;
		}	
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);

		RECT rc = { 
			owner->font_size.cx * column->unit + 2, 
			owner->font_size.cy * row + 2, 
			(GetWidth() + column->unit) * owner->font_size.cx - 2, 
			(1 + row) * owner->font_size.cy - 2, 
		};

		for (int j = 0; j < rows; ++j) {
			int v;
			if ((i != column->values_by_time.end()) && (i->time == time))
				v = i->value;
			else
				v = column->novalue;

			dc.FillSolidRect(rc.left - 2, rc.top - 2, GetWidth() * owner->font_size.cx, owner->font_size.cy, GetBkColor(time));

			if ((v != column->novalue) && (v != 0)) {
				dc.FillSolidRect(&rc, owner->colors[PE_Control]);
			} else {
				DrawRectangle(dc, rc, GetTextColor(time, v));
			}

			time += owner->skip;

			bool draw_hidden = false;
			while ((i != column->values_by_time.end()) && (i->time < time)) {
				if ((i->time % owner->skip) != 0) draw_hidden = true;
				++i;
			}
			if (draw_hidden) {
				int tx = rc.left - 2;
				int ty = rc.top - 2;
				int hid_x = tx + (GetWidth() * owner->font_size.cx) + (owner->font_size.cx / 2);
				int hid_y_top = ty + 2;
				int hid_y_bot = ty + owner->font_size.cy - 3;

				CPenHandle old_pen = dc.SelectPen(owner->hidden_pen);
				dc.MoveTo(hid_x, hid_y_top);
				dc.LineTo(hid_x, hid_y_bot);
				dc.SelectPen(old_pen);
			}

			rc.top += owner->font_size.cy;
			rc.bottom += owner->font_size.cy;
		}
	}

	virtual bool LButtonDown(WPARAM wParam, LPARAM lParam) {
		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		POINT pt_cursor = owner->PointToCursorPoint(pt);

		int time = pt_cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());

		int old_v = exists ? i->value : column->novalue;
		int id = exists ? i->id : -1;
		//int old_v;
		//int old_m;
		//column->GetValue(time, &old_v, &old_m);

		int v;
		if (old_v == column->novalue)
			v = 1;
		else
			v = !old_v;

		if (old_v != v) {
			owner->NotifyEdit(
				column->plugin_id, column->group, column->track, column->column,
				time,
				0,
				v, 0,
				false,
				id
			);
		}

		return true;
	}

	virtual bool Char(WPARAM wParam, LPARAM lParam) {
		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());
		int id = exists ? i->id : -1;

		int v;
		switch (wParam) {
			case '0':
				v = 0;
				break;
			case '1':
				v = 1;
				break;
			default:
				return false;
				break;
		}

		owner->NotifyEdit(
			column->plugin_id, column->group, column->track, column->column,
			time,
			ccol.digit,
			v, 0,
			false,
			id
		);

		owner->StepCursor();

		return true;
	}

	virtual bool Special1() {
		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());

		int old_v = exists ? i->value : column->novalue;
		int id = exists ? i->id : 0;

		int v;
		if (old_v == column->novalue)
			v = 1;
		else
			v = !old_v;

		owner->NotifyEdit(
			column->plugin_id, column->group, column->track, column->column,
			time,
			ccol.digit,
			v, 0,
			false,
			id
		);

		owner->StepCursor();

		return true;
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}
};

// ---------------------------------------------------------------------------------------------------------------
// ENVELOPE
// ---------------------------------------------------------------------------------------------------------------

class CEnvelopeColumn : public CColumnEditor
{
  public:

	enum drag_mode {
		drag_mode_none,
		drag_mode_move,
	};

	drag_mode mode;
	int dragid, dragvalue, dragtime, dragfromtime, dragmintime, dragmaxtime;
	
	CEnvelopeColumn() {
		mode = drag_mode_none;
	}

	virtual int GetWidth() const {
		return 8;
	}

	virtual int GetDigits() const {
		return 1;
	}

	virtual int CreateCursorColumns(int idx, int x) {
		PE_cursorcolumn ccol = {
			// idx	column			digit	offset	unit	width
			idx,	column->index,	0,		0,		x,		GetWidth()
		};
		owner->cursorcolumns.push_back(ccol);
		return GetWidth();
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		int end_row = row + rows;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		for (int j = row; j < end_row; ++j) {
			if ((i != column->values_by_time.end()) && (i->time == time)) {
				double norm = double(i->value - column->minvalue) / (column->maxvalue - column->minvalue);
				int pt = column->unit + (GetWidth() * norm);
				RECT rcCell = { pt, j, pt + 1, j + 1 };
				COLORREF colorVal = owner->colors[PE_Cursor];
				dc.FillSolidRect(&rcCell, colorVal);
			}
			time += owner->skip;
			while ((i != column->values_by_time.end()) && (i->time < time)) ++i;
		}	
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		USES_CONVERSION;

		int time = row * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		if ((i != column->values_by_time.begin()) && (column->values_by_time.size() != 0)) --i;

		int tx = owner->font_size.cx * column->unit;

		RECT rcBg = {
			column->unit * owner->font_size.cx,
			row * owner->font_size.cy,
			(GetWidth() + column->unit) * owner->font_size.cx,
			(row + rows) * owner->font_size.cy
		};

		dc.FillSolidRect(&rcBg, owner->colors[PE_BG]);

		POINT ptPrev = { -1, -1 };

		while (i != column->values_by_time.end()) {
			int v, t;
			if (mode == drag_mode_move && i->id == dragid) {
				v = dragvalue;
				t = dragtime;
			} else {
				v = i->value;
				t = i->time;
			}
			int r = t / owner->skip;
			int ty = (owner->font_size.cy * t) / owner->skip;
			float w = (GetWidth() * owner->font_size.cx) - 6;
			float range = column->maxvalue - column->minvalue;
			float f = ((float)(v - column->minvalue) / range) * w;

			RECT rcPoint = { tx + f, ty + 1, tx + f + 6, ty + 7 };
			DrawRectangle(dc, rcPoint, 0);
			InflateRect(&rcPoint, -1, -1);
			dc.FillSolidRect(&rcPoint, owner->colors[PE_Cursor]);

			if (ptPrev.x != -1) {
				dc.MoveTo(ptPrev);
				dc.LineTo(tx + f + 3, ty + 3);
			}

			ptPrev.x = tx + f + 3;
			ptPrev.y = ty + 3;

			if (r > row + rows) break;

			++i;
 		}
	}


	PE_values_by_time::iterator FindValue(int value, int time) {

		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time - owner->skip); // subtract skip to consider the earlier events
		if ((i != column->values_by_time.begin()) && (column->values_by_time.size() != 0))
			--i;

		int nearestvaluediff = -1;
		int nearesttime = -1;

		while (i != column->values_by_time.end()) {
			int v = i->value;
			int t = i->time;

			if (t > time + owner->skip) break;

			int valuediff = abs(value - v);
			int timediff = abs(time - t);
			if ((nearestvaluediff == -1 || valuediff < nearestvaluediff) && timediff <= owner->skip ) {
				nearesttime = i->time;
				nearestvaluediff = valuediff;
			}
			++i;
		}

		if (nearesttime == -1)
			return column->values_by_time.end();

		return column->values_by_time.find(nearesttime);
	}

	virtual bool LButtonDown(WPARAM wParam, LPARAM lParam) {
		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		CPoint offset;
		POINT pt_cursor = owner->PointToCursorPoint(pt, &offset);

		int colpos = pt.x - (column->unit - owner->scroll.x) * owner->font_size.cx;
		float w = GetWidth() * owner->font_size.cx;
		float range = column->maxvalue - column->minvalue;
		int v = column->minvalue + ((float)(colpos) / w) * range;
		v = clamp(v, column->minvalue, column->maxvalue);

		int time = pt_cursor.y * owner->skip;
		int offsettime = (((pt_cursor.y * owner->font_size.cy + offset.y) * owner->skip) / owner->font_size.cy);

		PE_values_by_time::iterator i = FindValue(v, offsettime);
		bool exists = (i != column->values_by_time.end());

		if (exists) {
			// can only move within limits = (prevtime + 1) and (nexttime - 1)
			mode = drag_mode_move;
			dragid = i->id;
			dragvalue = i->value;
			dragtime = i->time;
			dragfromtime = i->time;
			SetDragLimits(i->time);
			return true;
		} else
			return false;
	}

	virtual bool MouseMove(WPARAM wParam, LPARAM lParam) {
		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		CPoint offset;
		POINT pt_cursor = owner->PointToCursorPoint(pt, &offset);

		if (mode == drag_mode_move) {

			int colpos = pt.x - (column->unit - owner->scroll.x) * owner->font_size.cx;
			float w = GetWidth() * owner->font_size.cx;
			float range = column->maxvalue - column->minvalue;
			int v = column->minvalue + ((float)(colpos) / w) * range;
			v = clamp(v, column->minvalue, column->maxvalue);

			int offsettime = clamp(
				((pt_cursor.y * owner->font_size.cy + offset.y) * owner->skip) / owner->font_size.cy, 
				dragmintime, dragmaxtime);

			dragtime = offsettime;
			dragvalue = v;
			InvalidateLines(dragfromtime, dragvalue);
		}

		return true;
	}

	virtual bool LButtonUp(WPARAM wParam, LPARAM lParam) {
		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		POINT pt_cursor = owner->PointToCursorPoint(pt);
		if (mode == drag_mode_move) {
			owner->NotifyEdit(
				column->plugin_id, column->group, column->track, column->column,
				dragtime,
				0,
				dragvalue, 0,
				false,
				dragid
			);

			owner->ScrollToCursorPoint(pt_cursor, owner->scroll, false);
		}
		mode = drag_mode_none;
		return false;
	}

	void SetDragLimits(int time) {
		PE_values_by_time::iterator i_curr = column->values_by_time.find(time);
		PE_values_by_time::iterator i_from = i_curr;
		PE_values_by_time::iterator i_to = i_curr;

		if (i_curr != column->values_by_time.begin()) {
			--i_from;
			dragmintime = i_from->time + 1;
		} else
			dragmintime = 0;

		++i_curr;
		if (i_curr != column->values_by_time.end()) {
			++i_to;
			dragmaxtime = i_to->time - 1;
		} else
			dragmaxtime = owner->pattern_rows - 1;

	}

	void InvalidateLines(int time, int value) {
		PE_values_by_time::iterator i_curr = column->values_by_time.find(time);
		PE_values_by_time::iterator i_from = i_curr;
		PE_values_by_time::iterator i_to = i_curr;

		if (i_curr != column->values_by_time.begin()) {
			--i_from;
		}

		++i_curr;
		if (i_curr != column->values_by_time.end()) {
			++i_to;
		}

		int from_time, to_time;
		if (mode == drag_mode_move && i_from->id == dragid)
			from_time = dragtime;
		else
			from_time = i_from->time;

		if (mode == drag_mode_move && i_to->id == dragid)
			to_time = dragtime;
		else
			to_time = i_to->time;

		RECT rc;
		owner->GetRangeRectScreen(column->index, from_time / owner->skip, column->index, to_time / owner->skip, &rc);
		owner->clientDC->LPtoDP(&rc);
		owner->InvalidateRect(&rc, FALSE);
		RECT rcPreview = { column->index, time / owner->skip, column->index, time / owner->skip };
		owner->InvalidatePreview(&rcPreview);
	}

	void PreInsertValue(int time, int value) {
		// ---
	}

	void PostInsertValue(int time, int value) {
		// ---
		InvalidateLines(time, value);
	}

	void PreDeleteValue(int time, int value) {
		InvalidateLines(time, value);
	}

	void PostDeleteValue(int time, int value) {
		// ---
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}
};

// ---------------------------------------------------------------------------------------------------------------
// PATTERN
// ---------------------------------------------------------------------------------------------------------------

class CPatternColumn : public CColumnEditor
{
  public:

	virtual int GetWidth() const {
		return trigger_width;
	}

	virtual int GetDigits() const {
		return trigger_width;
	}

	virtual int CreateCursorColumns(int idx, int x) {
		for (int j = 0; j < GetWidth(); ++j) {
			PE_cursorcolumn ccol = {
				// idx	column			digit	offset	unit	width
				idx+j,	column->index,	j,		j,		x+j,	1
			};
			owner->cursorcolumns.push_back(ccol);
		}
		return GetWidth();
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		int end_row = row + rows;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		for (int j = row; j < end_row; ++j) {
			if ((i != column->values_by_time.end()) && (i->time == time)) {
				int length = GetPatternLengthClipped(i->time, i->value) / owner->skip;

				RECT rcCell = { column->unit, j, column->unit + GetWidth(), j + length };
				dc.FillSolidRect(&rcCell, owner->colors[PE_Trigger]);

				rcCell.bottom = j + 1;
				dc.FillSolidRect(&rcCell, owner->colors[PE_TextTrigger]);
			}
			time += owner->skip;
			while ((i != column->values_by_time.end()) && (i->time < time)) ++i;
		}	
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		USES_CONVERSION;

		// render a pattern column like the sequencer

		int font_cx = owner->font_size.cx;
		int font_cy = owner->font_size.cy;
		int skip = owner->skip;
		int time = row * skip;
		int tx = column->unit * font_cx;
		int tx_width = GetWidth() * font_cx;

		// Row shades
		{	int row_time = time;
			int top_pos = row * font_cy;

			for (int j = 0; j < rows; ++j) {
				dc.FillSolidRect(tx, top_pos, tx_width, font_cy, GetBkColor(row_time));
				top_pos += font_cy;
				row_time += skip;
			}
		}

		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		if ((i != column->values_by_time.begin()) && (column->values_by_time.size() != 0))
			--i;

		while (i != column->values_by_time.end()) {
			int v = i->value;
			int t = i->time;
			int r = t / skip;

			if (r > row + rows) break;

			int ty = (t * font_cy) / skip;

			int length = GetPatternLengthClipped(t, v);

			if (length) {
				RECT rcPattern = {
					tx,
					ty,
					tx + tx_width,
					ty + (length * font_cy) / skip
				};

				COLORREF colorFill = owner->colors[PE_Trigger];
				COLORREF colorHL = owner->colors[PE_Trigger_Highlight];
				COLORREF colorSH = owner->colors[PE_Trigger_Shadow];
				COLORREF colorAvg = AverageRGB(colorHL, colorSH, 2);

				dc.Draw3dRect(&rcPattern, colorHL, colorSH);
				dc.SetPixel(rcPattern.left, rcPattern.bottom-1, colorAvg);
				dc.SetPixel(rcPattern.right-1, rcPattern.top, colorAvg);
				InflateRect(&rcPattern, -1, -1);
				dc.FillSolidRect(&rcPattern, colorFill);

				PE_patterninfo const& pi = owner->patterninfos[v];

				if (pi.loop_enabled) {
					CPen loops_pen;
					loops_pen.CreatePen(PS_SOLID, 1, colorAvg);
					CPenHandle oldPen = dc.SelectPen(loops_pen);

					if ((pi.loop_begin != 0) && (pi.loop_begin < length)) {
						int ty_begin = ty + ((font_cy * pi.loop_begin) / skip);
						dc.MoveTo(tx+3, ty_begin);
						dc.LineTo(tx+tx_width-3, ty_begin);
					}

					if ((pi.loop_end != 0) && (pi.loop_end < length)) {
						int ty_end = ty + ((font_cy * pi.loop_end) / skip);
						dc.MoveTo(tx+3, ty_end);
						dc.LineTo(tx+tx_width-3, ty_end);
					}

					dc.SelectPen(oldPen);
				}
			}

			dc.SetTextColor(owner->colors[PE_TextValue]);
			int oldbk = dc.SetBkMode(TRANSPARENT);
			if (length) {
				std::string const& str = owner->patterninfos[v].name;
				int len = min((int)str.length(), GetWidth());
				dc.TextOut(tx, ty, CA2T(str.c_str()), len);
			}
			else {
				dc.TextOut(tx, ty, CA2T("??"), 2);
			}
			dc.SetBkMode(oldbk);

			++i;
// 			if (i == column->values_by_time.end()) break;
// 			int timedelta = i->time - time;
// 			time += timedelta * skip;
		}
	}

	int GetPatternLengthClipped(int time, int value) const {
		int length = GetPatternLength(value);
		if (length == 0) return 0;

		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time+1);
		if (i != column->values_by_time.end()) {
			if ((time + length-1) > i->time) {
				length = i->time - time;
			}
		}
		return length;
	}

	int GetPatternLength(int value) const {
		if ((value >= 0) && (value < owner->patterninfos.size()))
			return owner->patterninfos[value].length;
		return 0;
	}

	void InvalidateColumnArea(int time, int value) {
		int from_time;
		int to_time;

		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
 		if ((i != column->values_by_time.begin()) && (column->values_by_time.size() != 0)) {
 			--i;
			int prev_time = i->time;
			int prev_value = i->value;
			int prev_len = GetPatternLengthClipped(prev_time, prev_value);

			int len = GetPatternLengthClipped(time, value);
			from_time = prev_time;
			to_time = std::max(prev_time + prev_len, time + len);
		} else {
			int len = GetPatternLengthClipped(time, value);
			from_time = time;
			to_time = time + len;
		}

		RECT rc;
		owner->GetRangeRectScreen(column->index, from_time / owner->skip, column->index, to_time / owner->skip, &rc);
		owner->clientDC->LPtoDP(&rc);
		owner->InvalidateRect(&rc, FALSE);
		RECT rcPreview = { column->index, from_time / owner->skip, column->index, to_time / owner->skip };
		owner->InvalidatePreview(&rcPreview);
	}

	void PreInsertValue(int time, int value) {
		InvalidateColumnArea(time, value);
	}

	void PostInsertValue(int time, int value) {
		// ---
	}

	void PreDeleteValue(int time, int value) {
		// ---
	}

	void PostDeleteValue(int time, int value) {
		InvalidateColumnArea(time, value);
	}
	
	virtual bool DoubleClick(WPARAM wParam, LPARAM lParam) {
		owner->ForwardAction(ID_PATTERNVIEW_PATTERNCOLUMN_GOTOPATTERN);
		return true;
	}

	virtual bool Special1() {
		owner->ForwardAction(ID_PATTERNVIEW_PATTERNCOLUMN_HOTPASTE);
		return true;
	}

	virtual bool Special2() {
		owner->ForwardAction(ID_PATTERNVIEW_PATTERNCOLUMN_CLONEPATTERN);
		return true;
	}

	virtual bool Special3() {
		owner->ForwardAction(ID_PATTERNVIEW_PATTERNCOLUMN_EXPANDCOLLAPSE);
		return true;
	}

	virtual bool Special5() {
		owner->ForwardAction(ID_PATTERNVIEW_PATTERNCOLUMN_GOTOPATTERN);
		return false;
	}

	virtual bool Special6() {
		owner->ForwardAction(ID_PATTERNVIEW_PATTERNCOLUMN_CLONEGOTOPATTERN);
		return false;
	}

	virtual bool Char(WPARAM wParam, LPARAM lParam) {
		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;

		PE_values_by_time::iterator j = column->values_by_time.find(time);
		bool exists = (j != column->values_by_time.end());
		int id = exists ? j->id : -1;
		char c = wParam;
		if (c < '!' || c > '~') // 0x21 through 0x7E are valid.
			return false;

		int v = column->novalue;
		bool reset_cursor = true;
		bool step_cursor = false;

		{	std::string current = "";
			int digit = 0;
			if (exists) {
				digit = ccol.digit;
				current = owner->patterninfos[j->value].name;
				current.resize(digit, ' ');
			}
			current += string(1, c);
			std::map<string, int>::iterator k = owner->patternnames.lower_bound(current);
			if (k != owner->patternnames.end()) {
				int new_v = k->second;
				std::string newname = owner->patterninfos[new_v].name;
				newname.resize(digit + 1, ' ');
				if (newname == current) {
					v = new_v;
					if (exists)
						reset_cursor = false;
					step_cursor = true;
				} else {
					v = column->novalue;
				}
			}
		}

		if (exists || (v != column->novalue)) {
			owner->NotifyEdit(
				column->plugin_id, column->group, column->track, column->column,
				time,
				ccol.digit,
				v, 0,
				false,
				id
			);

			if (reset_cursor)
				owner->StepCursorReset();

			if (step_cursor)
				owner->StepCursorSmart();
		}

		return true;
	}

	virtual void SetFilteredValues(const std::vector<int>& _values) {
	}
};

// ---------------------------------------------------------------------------------------------------------------
// HARMONIC
// ---------------------------------------------------------------------------------------------------------------

class CHarmonicColumn : public CColumnEditor
{
  public:

	virtual int GetWidth() const {
		return 4;
	}

	virtual int GetDigits() const {
		return 4;
	}

	virtual int CreateCursorColumns(int idx, int x) {
		for (int j = 0; j < GetWidth(); ++j) {
			PE_cursorcolumn ccol = {
				// idx	column			digit	offset	unit	width
				idx+j,	column->index,	j,		j,		x+j,	1
			};
			owner->cursorcolumns.push_back(ccol);
		}
		return GetWidth();
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;
		int end_row = row + rows;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		for (int j = row; j < end_row; ++j) {
			if ((i != column->values_by_time.end()) && (i->time == time)) {
				int length = GetSymbolLengthClipped(time) / owner->skip;

				RECT rcCell = { column->unit, j, column->unit + GetWidth(), j + length };
				dc.FillSolidRect(&rcCell, owner->colors[PE_Trigger]);

				rcCell.bottom = j + 1;
				dc.FillSolidRect(&rcCell, owner->colors[PE_TextValue]);
			}
			time += owner->skip;
			while ((i != column->values_by_time.end()) && (i->time < time)) ++i;
		}	
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		USES_CONVERSION;

		int font_cx = owner->font_size.cx;
		int font_cy = owner->font_size.cy;
		int skip = owner->skip;
		int time = row * skip;
		int tx = column->unit * font_cx;
		int tx_width = GetWidth() * font_cx;

		// Row shades
		{	int row_time = time;
			int top_pos = row * font_cy;

			for (int j = 0; j < rows; ++j) {
				dc.FillSolidRect(tx, top_pos, tx_width, font_cy, GetBkColor(row_time));
				top_pos += font_cy;
				row_time += skip;
			}
		}

		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
		if ((i != column->values_by_time.begin()) && (column->values_by_time.size() != 0))
			--i;

		while (i != column->values_by_time.end()) {
			int v = i->value;
			int t = i->time;
			int r = t / skip;

			if (r > row + rows) break;

			int ty = (t * font_cy) / skip;

			int length = GetSymbolLengthClipped(t);

			if (length) {
				RECT rcPattern = {
					tx,
					ty,
					tx + tx_width,
					ty + (length * font_cy) / skip
				};

				COLORREF colorFill = owner->colors[PE_Trigger];
				COLORREF colorHL = owner->colors[PE_Trigger_Highlight];
				COLORREF colorSH = owner->colors[PE_Trigger_Shadow];
				COLORREF colorAvg = AverageRGB(colorHL, colorSH, 2);

				dc.Draw3dRect(&rcPattern, colorHL, colorSH);
				dc.SetPixel(rcPattern.left, rcPattern.bottom-1, colorAvg);
				dc.SetPixel(rcPattern.right-1, rcPattern.top, colorAvg);
				InflateRect(&rcPattern, -1, -1);
				dc.FillSolidRect(&rcPattern, colorFill);
			}

			dc.SetTextColor(owner->colors[PE_TextTrigger]);
			int oldbk = dc.SetBkMode(TRANSPARENT);
			int h_length = Harmony::symbolinfos_count;
			std::string const& str = v < h_length ? Harmony::symbolinfos[v].name : "????";
			int len = min((int)str.length(), GetWidth());
			dc.TextOut(tx, ty, CA2T(str.c_str()), len);
			dc.SetBkMode(oldbk);

			++i;
// 			if (i == column->values_by_time.end()) break;
// 			int timedelta = i->time - time;
// 			time += timedelta * skip;
		}
	}

// 	int GetSymbolLengthClipped(int time) const {
// 		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time + 1);
// 		if (i != column->values_by_time.end()) {
// 			return i->time - time;
// 		}
// 		return owner->pattern_rows - time;
// 	}

	int GetSymbolLengthClipped(int time) const {
		return owner->HSysGetContextLength(column->track, time);
	}

	void InvalidateColumnArea(int time, int value) {
		int from_time;
		int to_time;

		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);
 		if ((i != column->values_by_time.begin()) && (column->values_by_time.size() != 0)) {
 			--i;
			int prev_time = i->time;
			int prev_len = GetSymbolLengthClipped(prev_time);

			int len = GetSymbolLengthClipped(time);
			from_time = prev_time;
			to_time = std::max(prev_time + prev_len, time + len);
		} else {
			int len = GetSymbolLengthClipped(time);
			from_time = time;
			to_time = time + len;
		}

		RECT rc;
		owner->GetRangeRectScreen(column->index, from_time / owner->skip, column->index, to_time / owner->skip, &rc);
		owner->clientDC->LPtoDP(&rc);
		owner->InvalidateRect(&rc, FALSE);
		RECT rcPreview = { column->index, from_time / owner->skip, column->index, to_time / owner->skip };
		owner->InvalidatePreview(&rcPreview);
	}

	void PreInsertValue(int time, int value) {
		InvalidateColumnArea(time, value);
	}

	void PostInsertValue(int time, int value) {
		// ---
	}

	void PreDeleteValue(int time, int value) {
		// ---
	}

	void PostDeleteValue(int time, int value) {
		InvalidateColumnArea(time, value);
	}

	virtual bool Char(WPARAM wParam, LPARAM lParam) {
		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;

		PE_values_by_time::iterator j = column->values_by_time.find(time);
		bool exists = (j != column->values_by_time.end());
		int id = exists ? j->id : -1;

		char c = wParam;
		if (c < '!' || c > '~') // 0x21 through 0x7E are valid.
			return false;

		int v = 0;
		bool reset_cursor = true;
		bool step_cursor = false;

		{	using namespace Harmony;
			std::string current = "";
			int digit = 0;
			if (exists) {
				digit = ccol.digit;
				current = symbolinfos[j->value].name;
				current.resize(digit, ' ');
			}
			current += string(1, c);
			Info& info = Info::instance();
 			Info::sym_map_t::iterator k = info.sym_map.lower_bound(current);
			if (k != info.sym_map.end()) {
				Symbols::T sym = k->second;
				std::string newname = symbolinfos[sym].name;
				newname.resize(digit + 1, ' ');
				if (newname == current) {
					v = (int)sym;
					if (exists)
						reset_cursor = false;
					step_cursor = true;
				} else {
					v = 0;
				}
			}
		}

		//if (exists || (v != column->novalue)) {
			owner->NotifyEdit(
				column->plugin_id, column->group, column->track, column->column,
				time,
				ccol.digit,
				v, 0,
				false,
				id
			);
		//}

		if (reset_cursor)
			owner->StepCursorReset();

		if (step_cursor)
			owner->StepCursorSmart();

		return true;
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}
};

// ---------------------------------------------------------------------------------------------------------------
// PIANO
// ---------------------------------------------------------------------------------------------------------------

// CNoteMetaColumn is the base class of CPianoColumn and CMatrixColumn for some shared stuff
class CNoteMetaColumn : public CColumnEditor {
public:
	bool IsSelectable() {
		return false; // not globally selectable, handle selections inside the piano roll locally
	}

	void PostInsertValue(int time, int value) {
		InvalidateScreenRows();
	}

	virtual void OctaveChange(int nOctave) {
		InvalidateScreenRows();
	}

	int GetSymbolLengthClipped(int time, PE_column const& col) const {
		PE_values_by_time::iterator i = col.values_by_time.lower_bound(time + 1);
		if (i != col.values_by_time.end()) {
			return i->time - time;
		}
		return owner->pattern_rows - time;
	}

	void RenderGrid(CDC& dc, int row, int rows) {

		int charwidth = owner->font_size.cx;
		int charheight = owner->font_size.cy;
		int left = charwidth * column->unit;
		int top = charheight * row;

		int skip = owner->skip;
		int time = row * skip;

		int sharptab[] = { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
		for (int j = 0; j < rows; ++j) {

			for (int k = 0; k < GetDigits(); k++) {

				RECT rcDigit = {
					left + charwidth * k,
					top + j * charheight,
					left + charwidth * k + charwidth + 1,
					top + j * charheight + charheight + 1
				};

				COLORREF bkcolor;
				if (sharptab[k % 12])
					bkcolor = owner->colors[PE_BG_Dark];
				else
					bkcolor = owner->colors[PE_BG];
					//bkcolor = GetBkColor(time + j * owner->skip);
				dc.FillSolidRect(rcDigit.left, rcDigit.top, charwidth, charheight, bkcolor);
				
				CPen borderpen;
				borderpen.CreatePen(PS_SOLID, 1, owner->colors[PE_TextShade]);
				CPenHandle oldpen = dc.SelectPen(borderpen);

				// vertical borders
				dc.MoveTo(rcDigit.left, rcDigit.top);
				dc.LineTo(rcDigit.left, rcDigit.bottom);
				if (k == GetDigits() - 1) {
					dc.MoveTo(rcDigit.right, rcDigit.top);
					dc.LineTo(rcDigit.right, rcDigit.bottom);
				}

				// horizontal borders at beats
				if ( (time + j * owner->skip) % (owner->verydark_row * owner->resolution) == 0 ||
					 (time + j * owner->skip) % (owner->dark_row * owner->resolution) == 0 ) {
					dc.MoveTo(rcDigit.left, rcDigit.top);
					dc.LineTo(rcDigit.right, rcDigit.top);
				}

				dc.SelectPen(oldpen);
			}
		}
	}

	void InvalidateScreenRows() {
		RECT rc;
		owner->GetRangeRectScreen(column->index, owner->scroll.y, column->index, owner->scroll.y + owner->editor_rows, &rc);
		owner->clientDC->LPtoDP(&rc);
		owner->InvalidateRect(&rc, FALSE);
	}

	const PE_value* FindNote(int index, int time, int* resultlength) {

		for (int j = 0; j < owner->columns.size(); j++) {

			PE_column const& col = *owner->columns[j];
			if (col.type != pattern_column_type_note) continue;
			if (col.plugin_id != column->plugin_id) continue;

			PE_values_by_time::iterator i = col.values_by_time.lower_bound(time);
			if ((i != col.values_by_time.begin()) && (col.values_by_time.size() != 0))
				--i;

			while (i != col.values_by_time.end()) {
				int v = i->value;
				int t = i->time;

				if (t > time) break;

				int length = GetSymbolLengthClipped(t, col);
				
				int x = buzz_to_midi_note(v) - owner->octave * 12;
				if (x == index && t + length > time) { 
					*resultlength = length;
					return &*i;
				}

				++i;
			}
		}
		return 0;
	}
};

static const int midi_note_c4 = 48;

class CPianoColumn : public CNoteMetaColumn {
public:

	enum drag_mode {
		drag_mode_none,
		drag_mode_notes,
		drag_mode_notes_start,
		drag_mode_notes_end,
		drag_mode_mark,
		drag_mode_paint,
	};
	
	drag_mode mode;
	std::vector<int> selected_events;

	CPoint last_pt;
	CPoint begin_drag_pt;
	CPoint end_drag_pt;

	CPianoColumn() {
		mode = drag_mode_none;
	}

	virtual int GetWidth() const {
		return 3 * 12 + 1;
	}

	virtual int GetDigits() const {
		return GetWidth();
	}

	virtual int CreateCursorColumns(int idx, int x) {
		for (int j = 0; j < GetWidth(); ++j) {
			PE_cursorcolumn ccol = {
				// idx	column			digit	offset	unit	width
				idx+j,	column->index,	j,		j,		x+j,	1
			};
			owner->cursorcolumns.push_back(ccol);
		}
		return GetWidth();
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
	}
	
	void RenderNoteBox(CDC& dc, int id, int time, int value, int length) {
		int font_cx = owner->font_size.cx;
		int font_cy = owner->font_size.cy;
		int skip = owner->skip;
		int tx = column->unit * font_cx;
		int ty = (time * font_cy) / skip;
		int x = buzz_to_midi_note(value) - owner->octave * 12;
		
		if (x < 0 || x >= GetWidth()) { 
			return; 
		}

		x *= font_cx;

		if (length > 0) {
			CRect rcPattern(
				tx + x,
				ty,
				tx + x + font_cx,//tx_width,
				ty + (length * font_cy) / skip
			);

			COLORREF colorFill = GetTextColor(time, value); //owner->colors[PE_TextValue];
			//COLORREF colorSelected = owner->colors[PE_Selection];

			COLORREF colorHL = owner->colors[PE_Trigger_Highlight];
			COLORREF colorSH = owner->colors[PE_Trigger_Shadow];
			COLORREF colorAvg = AverageRGB(colorHL, colorSH, 2);

			if (std::find(selected_events.begin(), selected_events.end(), id) != selected_events.end()) {
				CPoint dragdiff = end_drag_pt - begin_drag_pt;
				dragdiff.x = (dragdiff.x / font_cx) * font_cx;
				dragdiff.y = (dragdiff.y / font_cy) * font_cy;

				if (mode == drag_mode_notes) {
					rcPattern.OffsetRect(dragdiff);
				} else if (mode == drag_mode_notes_start) {
					rcPattern.top += dragdiff.y;
				} else if (mode == drag_mode_notes_end) {
					rcPattern.bottom += dragdiff.y;
				}
				colorFill = owner->colors[PE_Selection];

				rcPattern.NormalizeRect();

				CRect rcSel = rcPattern;
				InflateRect(&rcSel, 2, 2);
				SIZE sz = { 2, 2 };
				//dc.DrawDragRect(&rcSel, sz, 0, sz); // looks nice but ignores SetWindowOrg etc
				DrawRectangle(dc, rcSel, colorFill);
			}

			dc.Draw3dRect(&rcPattern, colorHL, colorSH);
			dc.SetPixel(rcPattern.left, rcPattern.bottom-1, colorAvg);
			dc.SetPixel(rcPattern.right-1, rcPattern.top, colorAvg);
			InflateRect(&rcPattern, -1, -1);

			dc.FillSolidRect(&rcPattern, colorFill);
		}
		
		if (mode == drag_mode_none) {
			dc.SetTextColor(owner->colors[PE_TextTrigger]);
			int oldbk = dc.SetBkMode(TRANSPARENT);
			//int h_length = Harmony::symbolinfos_count;
			//std::string const& str = v < h_length ? Harmony::symbolinfos[v].name : "????";
			std::string const& str = "`";
			int len = min((int)str.length(), GetWidth());
			dc.TextOut(tx+x, ty, CA2T(str.c_str()), len);
			dc.SetBkMode(oldbk);
		}
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		USES_CONVERSION;

		int time = row * owner->skip;

		RenderGrid(dc, row, rows);

		for (int j = 0; j < owner->columns.size(); j++) {

			PE_column const& col = *owner->columns[j];
			if (col.type != pattern_column_type_note) continue;
			if (col.plugin_id != column->plugin_id) continue;

			PE_values_by_time::iterator i = col.values_by_time.lower_bound(time);
			if ((i != col.values_by_time.begin()) && (col.values_by_time.size() != 0))
				--i;

			while (i != col.values_by_time.end()) {
				int screenrow = i->time / owner->skip;
				if (screenrow > row + rows) break;

				int length = GetSymbolLengthClipped(i->time, col);
				RenderNoteBox(dc, i->id, i->time, i->value, length);
				++i;
			}

			if (mode == drag_mode_paint) {
				POINT pt_cursor1 = owner->PointToCursorPoint(begin_drag_pt);
				POINT pt_cursor2 = owner->PointToCursorPoint(end_drag_pt);
				PE_cursorcolumn const& ccol = owner->cursorcolumns[pt_cursor1.x];
				int fromrow = std::min(pt_cursor1.y, pt_cursor2.y);
				int torow = std::max(pt_cursor1.y, pt_cursor2.y);
				int length = torow - fromrow;
				int note = midi_to_buzz_note(ccol.digit + owner->octave * 12);
				RenderNoteBox(dc, 0, fromrow * owner->skip, note, length * owner->skip);
			}
		}
	}

	drag_mode GetDragMode(int notetime, int notelength, int cursorrow, int offsety) {

		int notestart_px = (notetime * owner->font_size.cy) / owner->skip;
		int noteend_px = ((notetime + notelength) * owner->font_size.cy) / owner->skip;
		int cursor_px = cursorrow * owner->font_size.cy + offsety;

		if (cursor_px - notestart_px < 6) {
			return drag_mode_notes_start;
		} else 
		if (noteend_px - cursor_px < 6) {
			return drag_mode_notes_end;
		} else {
			return drag_mode_notes;
		}
	}

	virtual bool LButtonDown(WPARAM wParam, LPARAM lParam) {
		CPoint pt((signed short)LOWORD(lParam), (signed short)HIWORD(lParam));
		CPoint offset;
		POINT pt_cursor = owner->PointToCursorPoint(pt, &offset);

		last_pt = pt;
		begin_drag_pt = pt;
		end_drag_pt = pt;

		PE_cursorcolumn const& ccol = owner->cursorcolumns[pt_cursor.x];

		// click + drag a selection
		int length;
		const PE_value* value = FindNote(ccol.digit, pt_cursor.y * owner->skip, &length);
		if (value && IsShiftDown()) {
			// shift + click on a note = (add to selection and) begin drag
			if (std::find(selected_events.begin(), selected_events.end(), value->id) == selected_events.end())
				selected_events.push_back(value->id);
			mode = GetDragMode(value->time, length, pt_cursor.y, offset.y);
			InvalidateScreenRows();
			return true;
		} else
		if (value) {
			// click on a note = single select and begin drag
			selected_events.clear();
			selected_events.push_back(value->id);
			mode = GetDragMode(value->time, length, pt_cursor.y, offset.y);
			InvalidateScreenRows();
			return true;
		} else
		if (IsShiftDown()) {
			// shift + click = selection rectangle
			mode = drag_mode_mark;
			return true;
		} else {
			// click but not on a note = paint note start/length
			mode = drag_mode_paint;
			InvalidateScreenRows();
			return true;
		}

		return false;
	}

	virtual bool MouseMove(WPARAM wParam, LPARAM lParam) {
		CPoint pt((signed short)LOWORD(lParam), (signed short)HIWORD(lParam));
		CPoint offset;
		POINT pt_cursor = owner->PointToCursorPoint(pt, &offset);
		PE_cursorcolumn const& ccol = owner->cursorcolumns[pt_cursor.x];

		last_pt = pt;
		end_drag_pt = pt;

		if (mode == drag_mode_notes || mode == drag_mode_notes_start || mode == drag_mode_notes_end) {
			InvalidateScreenRows();
			return true;
		} else
		if (mode == drag_mode_paint) {
			InvalidateScreenRows();
			return true;
		} else {
			/*
			TODO: change the mouse cursor when over a note start/end

			int length;
			const PE_value* value = FindNote(ccol.digit, pt_cursor.y * owner->skip, &length);
			if (value) {
				drag_mode movemode = GetDragMode(value->time, length, pt_cursor.y, offset.y);
				if (movemode == drag_mode_notes_start) {
					SetCursor(LoadCursor(0, IDC_SIZENS));
					return true;
				}
			}*/
		}
		return false;
	}

	virtual bool LButtonUp(WPARAM wParam, LPARAM lParam) {

		CPoint dragdiff = end_drag_pt - begin_drag_pt;
		int timediff = dragdiff.y / owner->font_size.cy * owner->skip;
		int pitchdiff = dragdiff.x / owner->font_size.cx;

		if (mode == drag_mode_paint) {
			POINT pt_cursor1 = owner->PointToCursorPoint(begin_drag_pt);
			POINT pt_cursor2 = owner->PointToCursorPoint(end_drag_pt);
			PE_cursorcolumn const& ccol = owner->cursorcolumns[pt_cursor1.x];
			int fromrow = std::min(pt_cursor1.y, pt_cursor2.y);
			int torow = std::max(pt_cursor1.y, pt_cursor2.y);
			int note = midi_to_buzz_note(ccol.digit + owner->octave * 12);
			int length = torow - fromrow;
			if (length > 0) {
				owner->NotifyPianoEdit(0, column->plugin_id, fromrow * owner->skip, note, length * owner->skip);
			}
		} else
		if (mode == drag_mode_notes) {
			if ((timediff != 0 || pitchdiff != 0) && selected_events.size()) {
				owner->NotifyPianoTranslate(selected_events, timediff, pitchdiff, 0);
			}
		} else
		if (mode == drag_mode_notes_start) {
			if ((timediff != 0 || pitchdiff != 0) && selected_events.size()) {
				owner->NotifyPianoTranslate(selected_events, timediff, 0, 1);
			}
		} else
		if (mode == drag_mode_notes_end) {
			if ((timediff != 0 || pitchdiff != 0) && selected_events.size()) {
				owner->NotifyPianoTranslate(selected_events, timediff, 0, 2);
			}
		}

		mode = drag_mode_none;

		return true;
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}
};


// ---------------------------------------------------------------------------------------------------------------
// TONE MATRIX
// ---------------------------------------------------------------------------------------------------------------

class CMatrixColumn : public CNoteMetaColumn
{
  public:

	virtual int GetWidth() const {
		return 3 * 12 + 1;
	}

	virtual int GetDigits() const {
		return GetWidth();
	}

	virtual int CreateCursorColumns(int idx, int x) {
		int columnunits = GetWidth();
		for (int j = 0; j < columnunits; ++j) {
			PE_cursorcolumn ccol = {
				// idx	column			digit	offset	unit	width
				idx+j,	column->index,	j,		j,		x+j,	1
			};
			owner->cursorcolumns.push_back(ccol);
		}
		return columnunits;
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;

		int charwidth = owner->font_size.cx;
		int charheight = owner->font_size.cy;
		int left = charwidth * column->unit;
		int top = charheight * row;

		RenderGrid(dc, row, rows);

		for (int j = 0; j < owner->columns.size(); j++) {

			PE_column const& col = *owner->columns[j];
			if (col.type != pattern_column_type_note) continue;
			if (col.plugin_id != column->plugin_id) continue;

			PE_values_by_time::iterator i = col.values_by_time.lower_bound(time);
			if ((i != col.values_by_time.begin()) && (col.values_by_time.size() != 0))
				--i;

			while (i != col.values_by_time.end()) {
				int r = i->time / owner->skip;

				if (r > row + rows) break;
				if (i->value != 255 && i->value != 254 && i->value != 0) {
					int x = buzz_to_midi_note(i->value) - owner->octave * 12;
					if (x >= 0 && x < GetWidth()) {
						RECT rcNote = {
							left + charwidth * x,
							r * charheight,
							left + charwidth + charwidth * x,
							charheight + r * charheight
						};
						
						dc.FillSolidRect(&rcNote, GetTextColor(i->time, i->value));
					}
				}

				++i;
			}
		}
	}

	virtual bool LButtonDown(WPARAM wParam, LPARAM lParam) {
		POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
		POINT pt_cursor = owner->PointToCursorPoint(pt);

		int time = pt_cursor.y * owner->skip;

		PE_cursorcolumn const& ccol = owner->cursorcolumns[pt_cursor.x];

		// click + drag a selection
		int length;
		const PE_value* value = FindNote(ccol.digit, pt_cursor.y * owner->skip, &length);

		if (!value) {
			int note = midi_to_buzz_note(ccol.digit + owner->octave * 12);
			owner->NotifyPianoEdit(0, column->plugin_id, time, note, 1 * owner->skip);
		} else {
			owner->NotifyPianoEdit(value->id, column->plugin_id, time, 0, 0);
		}
		return true;
	}

	virtual bool Char(WPARAM wParam, LPARAM lParam) {
		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;

		int v;
		switch (wParam) {
			case '0':
				v = 0;
				break;
			case '1':
				v = 1;
				break;
			default:
				return false;
				break;
		}

		/*owner->NotifyEdit(
			column->plugin_id, column->group, column->track, column->column,
			time,
			ccol.digit,
			v, 0,
			false
		);

		owner->StepCursor();*/

		return true;
	}

	virtual bool Special1() {
		PE_cursorcolumn const& ccol = owner->GetCursorColumnAtCursor();

		int time = owner->cursor.y * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.find(time);
		bool exists = (i != column->values_by_time.end());

		int old_v = exists ? i->value : column->novalue;

		int v;
		if (old_v == column->novalue)
			v = 1;
		else
			v = !old_v;

		/*owner->NotifyEdit(
			column->plugin_id, column->group, column->track, column->column,
			time,
			ccol.digit,
			v, 0,
			false
		);

		owner->StepCursor();*/

		return true;
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}


};

// ---------------------------------------------------------------------------------------------------------------
// WAVE EDITOR
// ---------------------------------------------------------------------------------------------------------------

class CWaveColumn : public CColumnEditor 
{
  public:

	virtual int GetWidth() const {
		return 12;
	}

	virtual int GetDigits() const {
		return GetWidth();
	}

	virtual int CreateCursorColumns(int idx, int x) {
		PE_cursorcolumn ccol = {
			// idx	column			digit	offset	unit	width
			idx+0,	column->index,	0,		0,		x+0,	GetWidth()
		};
		owner->cursorcolumns.push_back(ccol);
		return GetWidth();
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		int time = row * owner->skip;

		int charwidth = owner->font_size.cx;
		int charheight = owner->font_size.cy;
		int left = charwidth * column->unit;
		int top = charheight * row;

		// render based on stream notes, triggers, slice, offset and length parameters of this plugin
		// pre-render waveform in another thread? by duplicating the plugin instance and setting the stream?
		// will that work with recorded plugins? how about live updating during recording?
		// column->flagtype & pattern_column_flagtype_wave
	}

	virtual bool LButtonDown(WPARAM wParam, LPARAM lParam) {
		return false;
	}

	virtual bool Char(WPARAM wParam, LPARAM lParam) {
		return false;
	}

	virtual bool Special1() {
		return false;
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}


};

// ---------------------------------------------------------------------------------------------------------------
// COLLAPSED
// ---------------------------------------------------------------------------------------------------------------

class CCollapsedColumn : public CColumnEditor
{
  public:

	virtual int GetWidth() const {
		return 1;
	}

	virtual int GetDigits() const {
		return 1;
	}

	virtual int CreateCursorColumns(int idx, int x) {
		PE_cursorcolumn ccol = {
			// idx	column			digit	offset	unit	width
			idx,	column->index,	0,		0,		x,		GetWidth()
		};
		owner->cursorcolumns.push_back(ccol);
		return GetWidth();
	}

	virtual void PreviewValues(CDC& dc, int row, int rows) {
	}

	virtual void RenderValues(CDC& dc, int row, int rows) {
		USES_CONVERSION; 

		int tx = owner->font_size.cx * column->unit;
		int ty = owner->font_size.cy * row;

		int time = row * owner->skip;
		PE_values_by_time::iterator i = column->values_by_time.lower_bound(time);

		for (int j = 0; j < rows; ++j) {
			int v;
			if ((i != column->values_by_time.end()) && (i->time == time))
				v = i->value;
			else
				v = column->novalue;

			time += owner->skip;

			bool draw_hidden = v != column->novalue;
			while ((i != column->values_by_time.end()) && (i->time < time)) {
				draw_hidden = true;
				++i;
			}
			if (draw_hidden) {
				int hid_x = tx + (owner->font_size.cx / 2);
				int hid_y_top = ty + 2;
				int hid_y_bot = ty + owner->font_size.cy - 3;

				CPenHandle old_pen = dc.SelectPen(owner->hidden_pen);
				dc.MoveTo(hid_x, hid_y_top);
				dc.LineTo(hid_x, hid_y_bot);
				dc.SelectPen(old_pen);
			}

			ty += owner->font_size.cy;
		}
	}

	virtual void SetFilteredValues(const std::vector<int>& values) {
	}
};

// ---------------------------------------------------------------------------------------------------------------
// COLUMN FACTORIES / INFO
// ---------------------------------------------------------------------------------------------------------------

CColumnEditor* CColumnEditor::Create(int control) {
	switch (control) {
		case pattern_column_control_byte:
		case pattern_column_control_word:
		case pattern_column_control_switch:
			return new CValueColumn();
		case pattern_column_control_note:
			return new CNoteColumn();
		case pattern_column_control_slider:
			return new CSliderColumn();
		case pattern_column_control_button:
			return new CButtonColumn();
		case pattern_column_control_pattern:
			return new CPatternColumn();
		case pattern_column_control_pianoroll:
			return new CPianoColumn();
		case pattern_column_control_collapsed:
			return new CCollapsedColumn();
		case pattern_column_control_envelope:
			return new CEnvelopeColumn();
		case pattern_column_control_char:
			return new CCharColumn();
		case pattern_column_control_harmonic:
			return new CHarmonicColumn();
		case pattern_column_control_matrix:
			return new CMatrixColumn();
		case pattern_column_control_wave:
			return new CWaveColumn();
		default:
			return 0;
	}
}

std::string CColumnEditor::GetColumnControlName(int mode) {
	switch (mode) {
		case pattern_column_control_note:
			return "Note";
		case pattern_column_control_byte:
			return "Byte";
		case pattern_column_control_word:
			return "Word";
		case pattern_column_control_switch:
			return "Switch";
		case pattern_column_control_slider:
			return "Slider";
		case pattern_column_control_button:
			return "Button";
		case pattern_column_control_pattern:
			return "Sequence";
		case pattern_column_control_pianoroll:
			return "Pianoroll";
		case pattern_column_control_collapsed:
			return "Collapsed";
		case pattern_column_control_envelope:
			return "Envelope";
		case pattern_column_control_char:
			return "Char";
		case pattern_column_control_harmonic:
			return "Harmonic";
		case pattern_column_control_matrix:
			return "Tone Matrix";
		case pattern_column_control_wave:
			return "Wave Editor";
		default:
			return "(unknown)";
	}
}

int CColumnEditor::GetAvailableColumnControls(int type, int defaultcontrol, std::vector<int>& modes) {
	if (defaultcontrol == pattern_column_control_char) {
		modes.push_back(pattern_column_control_char);
		modes.push_back(pattern_column_control_byte);
	} else
	if (defaultcontrol == pattern_column_control_pattern) {
		modes.push_back(pattern_column_control_pattern);
		modes.push_back(pattern_column_control_word);
	} else
	if (defaultcontrol == pattern_column_control_harmonic) {
		modes.push_back(pattern_column_control_harmonic);
	} else 
	if (defaultcontrol == pattern_column_control_pianoroll) {
		modes.push_back(pattern_column_control_pianoroll);
		modes.push_back(pattern_column_control_matrix);
	} else
	if (defaultcontrol == pattern_column_control_wave) {
		modes.push_back(pattern_column_control_wave);
	} else {
		switch (type) {
			case pattern_column_type_note:
				modes.push_back(pattern_column_control_note);
				break;
			case pattern_column_type_byte:
				///modes.push_back(pattern_column_control_char);
				modes.push_back(pattern_column_control_byte);
				modes.push_back(pattern_column_control_slider);
				modes.push_back(pattern_column_control_envelope);
				break;
			case pattern_column_type_word:
				modes.push_back(pattern_column_control_word);
				modes.push_back(pattern_column_control_slider);
				modes.push_back(pattern_column_control_envelope);
				break;
			case pattern_column_type_switch:
				modes.push_back(pattern_column_control_switch);
				modes.push_back(pattern_column_control_button);
				break;
		}
	}

	return modes.size();
}

int CColumnEditor::GetDefaultTypeControl(int type/*, int flags*/) {
// 	if (flags & zzub_parameter_flag_pattern_index)
// 		return pattern_column_control_pattern;

	switch (type) {
		case pattern_column_type_note:
			return pattern_column_control_note;
		case pattern_column_type_byte:
			return pattern_column_control_byte;
		case pattern_column_type_word:
			return pattern_column_control_word;
		case pattern_column_type_switch:
			return pattern_column_control_switch;
		default:
			return -1;
	}
}

static const char* PE_notenames[12][3] = {
//	  Norm  b-er  #-er
	{ "C-", "Dd", "B#" }, // 0
	{ "C#", "Db", "Bx" }, // 1
	{ "D-", "Ed", "Cx" }, // 2
	{ "Eb", "Fd", "D#" }, // 3
	{ "E-", "Fb", "Dx" }, // 4
	{ "F-", "Gd", "E#" }, // 5
	{ "F#", "Gb", "Ex" }, // 6
	{ "G-", "Ad", "Fx" }, // 7
	{ "Ab", "XX", "G#" }, // 8
	{ "A-", "Bd", "Gx" }, // 9
	{ "Bb", "Cd", "A#" }, // 10
	{ "B-", "Cb", "Ax" }, // 11
};

std::string noteFromIntMeta(unsigned char v, int meta) {
	if (v == 255) return noteOffStr;
	else if (v == 254) return noteCutStr;

	int notebase = (v & 0xF) - 1;
	int oct = (v & 0xF0) >> 4;

	static char pc[16];
	sprintf(pc, "%s%X", PE_notenames[notebase][meta], oct);

	return pc;
}

std::string CColumnEditor::FormatValue(int v, int m, int type, int novalue) {
	switch (type) {
		case pattern_column_type_note:
			return v != novalue ? noteFromIntMeta(v, m) : bg_note;
		case pattern_column_type_byte:
			return v != novalue ? hexFromInt(v, 2, '0') : bg_byte;
		case pattern_column_type_word:
			return v != novalue ? hexFromInt(v, 4, '0') : bg_word;
		case pattern_column_type_switch:
			return v != novalue ? hexFromInt(v!=0?1:0, 1, '0') : bg_switch;
		default:
			return "";
	}
}
