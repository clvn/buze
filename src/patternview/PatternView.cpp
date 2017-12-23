#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "utils.h"
#include "Configuration.h"
#include "BuzeConfiguration.h"
#include "PatternView.h"
#include "Properties.h"
#include "PatternEditor/PatternEditorColumn.h"
#include "PrefGlobals.h"
#include <iostream>
#include <deque>

enum EDITFLAGS {
	EDIT_COPY = 1,	// set if selection can be copied
	EDIT_PASTE = 2,	// set if clipboard format is recognized
};

CHostDllModule _Module;

// 
// Factory
//

CPatternViewInfo::CPatternViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CPatternView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Pattern Editor";
	tooltip = "Pattern Editor";
	place = 1; //DockSplitTab::placeMAINPANE;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = true;
	allowfloat = false;
	defaultview = true;
}

void CPatternViewInfo::Attach() {
	buze_document_add_view(document, this);

	WORD ID_SHOW_PRIMARY = buze_main_frame_register_accelerator_event(mainframe, "view_primary", "f2", buze_event_type_show_pattern_view);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_PRIMARY, "Pattern Editor");


	//buze_main_frame_register_accelerator(mainframe, "patternview", "help", ID_HELP);

	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_play", "f5 ctrl", ID_PATTERN_PLAY);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_replay", "f5 shift ctrl", ID_PATTERN_REPLAY);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_playfromcursor", "f6 ctrl", ID_PATTERN_PLAYFROMCURSOR);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_play_track", "4 shift", ID_PATTERNVIEW_PLAY_TRACKROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_play_row", "8 shift", ID_PATTERNVIEW_PLAY_ROW);

	buze_main_frame_register_accelerator(mainframe, "patternview", "edit_copy", "c ctrl", ID_EDIT_COPY);
	buze_main_frame_register_accelerator(mainframe, "patternview", "edit_cut", "x ctrl", ID_EDIT_CUT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "edit_paste", "v ctrl", ID_EDIT_PASTE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "cut_splice", "x ctrl shift", ID_PATTERNVIEW_CUT_SPLICE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "paste_splice", "v ctrl shift", ID_PATTERNVIEW_PASTE_SPLICE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "paste_step", "p ctrl", ID_PATTERNVIEW_PASTE_STEP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "paste_mixover", "p ctrl shift", ID_PATTERNVIEW_PASTE_MIXOVER);
	buze_main_frame_register_accelerator(mainframe, "patternview", "paste_mixunder", "p ctrl alt", ID_PATTERNVIEW_PASTE_MIXUNDER);

	buze_main_frame_register_accelerator(mainframe, "patternview", "clear_value", "oem_period", ID_PATTERNVIEW_CLEARVALUE);//add nostep versions?
	buze_main_frame_register_accelerator(mainframe, "patternview", "clear_track_row", "oem_period ctrl", ID_PATTERNVIEW_CLEARTRACKROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "clear_pattern_row", "oem_period ctrl shift", ID_PATTERNVIEW_CLEARPATTERNROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "insert_column_row", "insert", ID_PATTERNVIEW_INSERTCOLUMNROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "insert_track_row", "insert ctrl", ID_PATTERNVIEW_INSERTTRACKROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "insert_pattern_row", "insert ctrl shift", ID_PATTERNVIEW_INSERTPATTERNROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "edit_delete", "delete", ID_EDIT_DELETE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "delete_track_row", "delete ctrl", ID_PATTERNVIEW_DELETETRACKROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "delete_pattern_row", "delete ctrl shift", ID_PATTERNVIEW_DELETEPATTERNROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "backspace_value", "backspace", ID_PATTERNVIEW_BACKSPACECOLUMNROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "backspace_track_row", "backspace ctrl", ID_PATTERNVIEW_BACKSPACETRACKROW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "backspace_pattern_row", "backspace ctrl shift", ID_PATTERNVIEW_BACKSPACEPATTERNROW);

	buze_main_frame_register_accelerator(mainframe, "patternview", "select_begin", "", ID_PATTERNVIEW_SELECTBEGIN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "select_end", "", ID_PATTERNVIEW_SELECTEND);
	buze_main_frame_register_accelerator(mainframe, "patternview", "select_down", "d ctrl", ID_PATTERNVIEW_SELECTDOWN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "select_beat", "d ctrl shift", ID_PATTERNVIEW_SELECTBEAT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "select_columns", "oem_5 ctrl", ID_PATTERNVIEW_SELECTCOLUMNS);
	buze_main_frame_register_accelerator(mainframe, "patternview", "select_all", "oem_5 ctrl shift", ID_EDIT_SELECTALL);
	buze_main_frame_register_accelerator(mainframe, "patternview", "select_none", "u ctrl", ID_PATTERNVIEW_UNSELECT);

	buze_main_frame_register_accelerator(mainframe, "patternview", "track_mute", "m ctrl", ID_PATTERNVIEW_TRACKMUTE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "track_solo", "l ctrl", ID_PATTERNVIEW_TRACKSOLO);
	buze_main_frame_register_accelerator(mainframe, "patternview", "machine_mute", "m ctrl shift", ID_MACHINE_MUTE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "machine_solo", "l ctrl shift", ID_MACHINE_SOLO);

	buze_main_frame_register_accelerator(mainframe, "patternview", "track_add", "add ctrl", ID_MACHINE_ADDTRACK);
	buze_main_frame_register_accelerator(mainframe, "patternview", "track_remove", "subtract ctrl", ID_MACHINE_REMOVETRACK);

	buze_main_frame_register_accelerator(mainframe, "patternview", "loop_set_begin", "b ctrl", ID_PATTERNVIEW_SETLOOPBEGIN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "loop_set_end", "e ctrl", ID_PATTERNVIEW_SETLOOPEND);
	buze_main_frame_register_accelerator(mainframe, "patternview", "loop_set_pattern", "b ctrl shift", ID_PATTERNVIEW_SETLOOPPATTERN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "loop_set_selection", "e ctrl shift", ID_PATTERNVIEW_SETLOOPSELECTION);

	buze_main_frame_register_accelerator(mainframe, "patternview", "format_up", "add alt shift", ID_PATTERNVIEW_FORMATUP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "format_down", "subtract alt shift", ID_PATTERNVIEW_FORMATDOWN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_up", "add alt", ID_PATTERNVIEW_PATTERNUP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_down", "subtract alt", ID_PATTERNVIEW_PATTERNDOWN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "dropdown_wave", "w alt", ID_PATTERNVIEW_DROPDOWNWAVE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "dropdown_format", "d alt", ID_PATTERNVIEW_DROPDOWNFORMAT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "dropdown_pattern", "p alt", ID_PATTERNVIEW_DROPDOWNPATTERN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "dropdown_octave", "o alt", ID_PATTERNVIEW_DROPDOWNOCTAVE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "dropdown_step", "s alt", ID_PATTERNVIEW_DROPDOWNSTEP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "toggle_playnotes", "n alt", ID_PATTERNVIEW_TOGGLEPLAYNOTES);
	buze_main_frame_register_accelerator(mainframe, "patternview", "toggle_showinfo", "i alt", ID_PATTERNVIEW_TOGGLESHOWINFO);
	buze_main_frame_register_accelerator(mainframe, "patternview", "toggle_follow", "oem_7 alt", ID_PATTERNVIEW_TOGGLEFOLLOW);
	buze_main_frame_register_accelerator(mainframe, "patternview", "show_notemask", "m alt", ID_PATTERNVIEW_SHOWNOTEMASK);
	buze_main_frame_register_accelerator(mainframe, "patternview", "show_harmonictranspose", "x alt", ID_PATTERNVIEW_SHOWHARMONICTRANSPOSE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "toggle_patternloop", "l alt", ID_PATTERNVIEW_TOGGLEPATTERNLOOP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "dropdown_patternscale", "c alt", ID_PATTERNVIEW_DROPDOWNPATTERNSCALE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "dropdown_patternbeat", "b alt", ID_PATTERNVIEW_DROPDOWNPATTERNBEAT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "dropdown_patternrows", "r alt", ID_PATTERNVIEW_DROPDOWNPATTERNROWS);
	buze_main_frame_register_accelerator(mainframe, "patternview", "dropdown_patternname", "n alt", ID_PATTERNVIEW_DROPDOWNPATTERNNAME);

	buze_main_frame_register_accelerator(mainframe, "patternview", "patternstack_back", "escape, left alt", ID_PATTERNVIEW_PATTERNSTACK_BACK);
	buze_main_frame_register_accelerator(mainframe, "patternview", "patternstack_forward", "right alt", ID_PATTERNVIEW_PATTERNSTACK_FORWARD);
	buze_main_frame_register_accelerator(mainframe, "patternview", "patternstack_reset", "", ID_PATTERNVIEW_PATTERNSTACK_RESET);

	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_0", "0 ctrl", ID_PATTERNVIEW_STEP_SET_0);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_1", "1 ctrl", ID_PATTERNVIEW_STEP_SET_1);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_2", "2 ctrl", ID_PATTERNVIEW_STEP_SET_2);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_3", "3 ctrl", ID_PATTERNVIEW_STEP_SET_3);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_4", "4 ctrl", ID_PATTERNVIEW_STEP_SET_4);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_5", "5 ctrl", ID_PATTERNVIEW_STEP_SET_5);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_6", "6 ctrl", ID_PATTERNVIEW_STEP_SET_6);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_7", "7 ctrl", ID_PATTERNVIEW_STEP_SET_7);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_8", "8 ctrl", ID_PATTERNVIEW_STEP_SET_8);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_set_9", "9 ctrl", ID_PATTERNVIEW_STEP_SET_9);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_up", "", ID_PATTERNVIEW_STEP_UP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "step_down", "", ID_PATTERNVIEW_STEP_DOWN);

	buze_main_frame_register_accelerator(mainframe, "patternview", "keyjazz_octave_up", "multiply", ID_PATTERNVIEW_OCTAVE_UP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "keyjazz_octave_down", "divide", ID_PATTERNVIEW_OCTAVE_DOWN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "wave_next", ">", ID_PATTERNVIEW_WAVE_NEXT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "wave_previous", "<", ID_PATTERNVIEW_WAVE_PREVIOUS);
	buze_main_frame_register_accelerator(mainframe, "patternview", "toggle_horizontalentry", "h ctrl", ID_PATTERNVIEW_TOGGLE_HORIZONTALENTRY);
	buze_main_frame_register_accelerator(mainframe, "patternview", "cycle_notesaffect", "oem_5 alt", ID_PATTERNVIEW_CYCLE_NOTESAFFECT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "toggle_transposeset", "t alt", ID_PATTERNVIEW_TOGGLE_TRANSPOSESET);
	buze_main_frame_register_accelerator(mainframe, "patternview", "toggle_volmask", "", ID_PATTERNVIEW_TOGGLE_VOLMASK);

	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_rows_double", "f ctrl", ID_PATTERN_DOUBLEROWS);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_rows_halve", "g ctrl", ID_PATTERN_HALVEROWS);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_length_double", "f ctrl shift", ID_PATTERN_DOUBLELENGTH);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_length_halve", "g ctrl shift", ID_PATTERN_HALVELENGTH);

	buze_main_frame_register_accelerator(mainframe, "patternview", "transpose_notes_up", "q ctrl", ID_PATTERNVIEW_TRANSPOSENOTESUP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transpose_notes_down", "a ctrl", ID_PATTERNVIEW_TRANSPOSENOTESDOWN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transpose_notes_octaveup", "q ctrl shift", ID_PATTERNVIEW_TRANSPOSENOTESOCTAVEUP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transpose_notes_octavedown", "a ctrl shift", ID_PATTERNVIEW_TRANSPOSENOTESOCTAVEDOWN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transpose_all_up", "q alt", ID_PATTERNVIEW_TRANSPOSESELECTIONUP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transpose_all_down", "q alt", ID_PATTERNVIEW_TRANSPOSESELECTIONDOWN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transpose_rekey", "", ID_TRANSPOSESET_REKEY);

	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_randomize", "r ctrl", ID_PATTERNVIEW_RANDOMIZE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_randomrange", "r ctrl shift", ID_PATTERNVIEW_RANDOMIZERANGE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_randomfrom", "", ID_PATTERNVIEW_RANDOMIZEUSING_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_humanize", "", ID_PATTERNVIEW_HUMANIZE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_shuffle", "", ID_PATTERNVIEW_SHUFFLE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_interpolate", "i ctrl", ID_PATTERNVIEW_INTERPOLATE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_gradiate", "i ctrl shift", ID_PATTERNVIEW_GRADIATE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_smooth", "", ID_PATTERNVIEW_SMOOTH_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_reverse", "", ID_PATTERNVIEW_REVERSE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_compact", "", ID_PATTERNVIEW_COMPACT_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_expand", "", ID_PATTERNVIEW_EXPAND_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_thin", "", ID_PATTERNVIEW_THIN_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_repeat", "", ID_PATTERNVIEW_REPEAT_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_echo", "", ID_PATTERNVIEW_ECHO_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_unique", "", ID_PATTERNVIEW_UNIQUE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_scale", "", ID_PATTERNVIEW_SCALE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_fade", "", ID_PATTERNVIEW_FADE_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_curvemap", "", ID_PATTERNVIEW_CURVEMAP_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_invert", "", ID_PATTERNVIEW_INVERT_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotaterows", "", ID_PATTERNVIEW_ROTATEROWS_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotaterows_up", "", ID_PATTERNVIEW_ROTATEROWS_UP_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotaterows_down", "", ID_PATTERNVIEW_ROTATEROWS_DOWN_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotatevalues", "", ID_PATTERNVIEW_ROTATEVALUES_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotatevalues_up", "", ID_PATTERNVIEW_ROTATEVALUES_UP_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotatevalues_down", "", ID_PATTERNVIEW_ROTATEVALUES_DOWN_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotaterhythms", "", ID_PATTERNVIEW_ROTATERHYTHMS_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotaterhythms_up", "", ID_PATTERNVIEW_ROTATERHYTHMS_UP_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotaterhythms_down", "", ID_PATTERNVIEW_ROTATERHYTHMS_DOWN_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotatenotes", "", 0);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotatenotes_up", "", 0);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rotatenotes_down", "", 0);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_notelength", "", ID_PATTERNVIEW_NOTELENGTH_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_volumes", "", ID_PATTERNVIEW_VOLUMES_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_alltofirst", "", ID_PATTERNVIEW_ALLTOFIRST_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_firsttolast", "", ID_PATTERNVIEW_FIRSTTOLAST_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_removefirst", "", ID_PATTERNVIEW_REMOVEFIRST_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_replacewaves", "", ID_PATTERNVIEW_REPLACEWAVES_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_invertchord_up", "oem_2 ctrl", ID_PATTERNVIEW_INVERTCHORDUP_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_invertchord_down", "oem_2 ctrl shift", ID_PATTERNVIEW_INVERTCHORDDOWN_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_trackswap", "j ctrl", ID_PATTERNVIEW_TRACKSWAP_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_rowswap", "j ctrl shift", ID_PATTERNVIEW_ROWSWAP_SELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "transform_clearsamecolumn", "", ID_PATTERNVIEW_CLEARSAMECOLUMN_SELECTION);

	buze_main_frame_register_accelerator(mainframe, "patternview", "column_action_1", "space", ID_PATTERNVIEW_SPECIAL1);
	buze_main_frame_register_accelerator(mainframe, "patternview", "column_action_2", "space ctrl", ID_PATTERNVIEW_SPECIAL2);
	buze_main_frame_register_accelerator(mainframe, "patternview", "column_action_3", "space ctrl shift", ID_PATTERNVIEW_SPECIAL3);
	buze_main_frame_register_accelerator(mainframe, "patternview", "column_action_4", "space shift", ID_PATTERNVIEW_SPECIAL4);
	buze_main_frame_register_accelerator(mainframe, "patternview", "column_action_5", "enter", ID_PATTERNVIEW_SPECIAL5);
	buze_main_frame_register_accelerator(mainframe, "patternview", "column_action_6", "enter shift", ID_PATTERNVIEW_SPECIAL6);

	buze_main_frame_register_accelerator(mainframe, "patternview", "column_toggle_columncontrol", "t ctrl", ID_PATTERNVIEW_TOGGLECOLUMNCONTROL);
	buze_main_frame_register_accelerator(mainframe, "patternview", "column_collapse_selection", "", ID_PATTERNVIEW_COLLAPSESELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "column_uncollapse_selection", "", ID_PATTERNVIEW_UNCOLLAPSESELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "column_toggle_collapse_selection", "k ctrl", ID_PATTERNVIEW_TOGGLECOLLAPSESELECTION);
	buze_main_frame_register_accelerator(mainframe, "patternview", "column_toggle_collapse_track", "k ctrl shift", ID_PATTERNVIEW_TOGGLECOLLAPSETRACK);

	buze_main_frame_register_accelerator(mainframe, "patternview", "pickupvalue", ";", ID_PATTERNVIEW_PICKUPVALUE);

	buze_main_frame_register_accelerator(mainframe, "patternview", "infopane_up", "up alt", ID_PATTERNVIEW_INFOPANE_UP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "infopane_down", "down alt", ID_PATTERNVIEW_INFOPANE_DOWN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "infopane_pageup", "pageup alt", ID_PATTERNVIEW_INFOPANE_PAGEUP);
	buze_main_frame_register_accelerator(mainframe, "patternview", "infopane_pagedown", "pagedown alt", ID_PATTERNVIEW_INFOPANE_PAGEDOWN);

	buze_main_frame_register_accelerator(mainframe, "patternview", "track_swap_right", "right ctrl alt", ID_PATTERNVIEW_TRACKSWAPRIGHT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "track_swap_left", "left ctrl alt", ID_PATTERNVIEW_TRACKSWAPLEFT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "format_layoutplugin_right", "right ctrl alt shift", ID_PATTERNVIEW_FORMATLAYOUTPLUGINRIGHT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "format_layoutplugin_left", "left ctrl alt shift", ID_PATTERNVIEW_FORMATLAYOUTPLUGINLEFT);

	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_create", "enter ctrl", ID_PATTERNVIEW_CREATEPATTERN);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_clone", "enter ctrl shift", ID_PATTERN_CLONE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_delete", "delete alt", ID_PATTERN_DELETE);
	buze_main_frame_register_accelerator(mainframe, "patternview", "pattern_properties", "backspace alt", ID_VIEW_PROPERTIES);

	buze_main_frame_register_accelerator(mainframe, "patternview", "format_create", "", ID_PATTERN_CREATEFORMAT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "format_clone", "", ID_PATTERNVIEW_CLONE_FORMAT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "format_delete", "", ID_PATTERN_DELETEFORMAT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "format_properties", "", ID_PATTERN_FORMATPROPERTIES);
	buze_main_frame_register_accelerator(mainframe, "patternview", "format_show", "f2 shift", ID_PATTERNVIEW_SHOWPATTERNFORMAT);

	buze_main_frame_register_accelerator(mainframe, "patternview", "machine_parameters", "enter alt", ID_MACHINE_PARAMETERS);
	buze_main_frame_register_accelerator(mainframe, "patternview", "machine_properties", "", ID_MACHINE_PROPERTIES);

	buze_main_frame_register_accelerator(mainframe, "patternview", "orderlist_right", "oem_6 alt", ID_PATTERNVIEW_ORDERLIST_RIGHT);
	buze_main_frame_register_accelerator(mainframe, "patternview", "orderlist_left", "oem_4 alt", ID_PATTERNVIEW_ORDERLIST_LEFT);

	buze_main_frame_register_accelerator(mainframe, "patternview", "nudge_backward", "", ID_PATTERNVIEW_NUDGEBACKWARD);
	buze_main_frame_register_accelerator(mainframe, "patternview", "nudge_forward", "", ID_PATTERNVIEW_NUDGEFORWARD);
	buze_main_frame_register_accelerator(mainframe, "patternview", "nudge_backward_small", "", ID_PATTERNVIEW_NUDGEBACKWARDSMALL);
	buze_main_frame_register_accelerator(mainframe, "patternview", "nudge_forward_small", "", ID_PATTERNVIEW_NUDGEFORWARDSMALL);

	// orderlist accelerators:
	//buze_main_frame_register_accelerator(mainframe, "orderlist", "help", ID_HELP);

	buze_main_frame_register_accelerator(mainframe, "orderlist", "edit_copy", "c ctrl", ID_EDIT_COPY);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "edit_cut", "x ctrl", ID_EDIT_CUT);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "edit_paste", "v ctrl", ID_EDIT_PASTE);

	buze_main_frame_register_accelerator(mainframe, "orderlist", "order_insert", "insert", ID_ORDERLIST_INSERT);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "order_duplicate", "d ctrl", ID_ORDERLIST_DUPLICATE);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "order_new", "p ctrl", ID_ORDERLIST_NEW);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "order_remove", "delete", ID_ORDERLIST_REMOVE);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "order_remove_delete", "delete ctrl", ID_ORDERLIST_REMOVE_DELETE);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "order_backspace", "backspace", ID_ORDERLIST_BACKSPACE);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "order_backspace_delete", "backspace ctrl", ID_ORDERLIST_BACKSPACE_DELETE);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "loop_set_begin", "b ctrl", ID_ORDERLIST_SETBEGINLOOP);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "loop_set_end", "e ctrl", ID_ORDERLIST_SETENDLOOP);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "loop_set_selection", "l ctrl", ID_ORDERLIST_SETSELECTIONLOOP);

	buze_main_frame_register_accelerator(mainframe, "orderlist", "toggle_follow", "oem_7 alt", ID_ORDERLIST_TOGGLEFOLLOW);

	buze_main_frame_register_accelerator(mainframe, "orderlist", "play", "space", ID_ORDERLIST_PLAYORDER);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "queue", "q ctrl", ID_ORDERLIST_QUEUE);

	buze_main_frame_register_accelerator(mainframe, "orderlist", "deselect", "escape", ID_ORDERLIST_DESELECT);
	buze_main_frame_register_accelerator(mainframe, "orderlist", "focus_editor", "enter", ID_ORDERLIST_GOTO_EDITOR);

}

void CPatternViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CPatternViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
		case buze_event_type_show_pattern_view:
			ShowPatternView(ev);
			break;
		
		case zzub_event_type_delete_patternformatcolumn:
			RemovePatternPosition(zzubData->delete_pattern_format_column.patternformatcolumn);
			break;
	}
}

void CPatternViewInfo::ShowPatternView(buze_event_data* ev) {
	// get existing pattern editor if it exists, cast and set pattern etc accordign to flags
	int editor_id = ev ? ev->show_pattern.editor_id : 0;
	CPatternView* patternView = (CPatternView*)buze_main_frame_get_view(mainframe, "PatternView", editor_id);
	if (patternView != 0) {
		buze_main_frame_set_focus_to(mainframe, patternView);
	} else {
		patternView = (CPatternView*)buze_main_frame_open_view(mainframe, "PatternView", "", editor_id, -1, -1);
	}

	if (ev && ev->show_pattern.change_pattern) {
		if (ev->show_pattern.reset_stack)
			patternView->SetPattern((zzub_pattern_t*)ev->show_pattern.pattern);
		else
			patternView->SetPatternPushStack((zzub_pattern_t*)ev->show_pattern.pattern);
	}
}

CView* CPatternViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CPatternView* view = new CPatternView(mainframe, this);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, pCreateData);
	return view;
}

// ---------------------------------------------------------------------------------------------------------------
// PATTERN POSITION
// ---------------------------------------------------------------------------------------------------------------

pattern_position CPatternViewInfo::GetPatternPosition(std::string const& caption, zzub_pattern_t* pattern) {
	
	CBuzeConfiguration configuration = buze_document_get_configuration(document);
	bool cursorCachedByFormat = configuration.getPatternPositionCachePatternFormat();

	if (cursorCachedByFormat) {
		// remember cursor position per pattern format
		pattern_format_position_map_t::iterator pi;
		pi = patternFormatPositionCache.find(std::make_pair(caption, zzub_pattern_get_format(pattern)));

		if (pi != patternFormatPositionCache.end())
			return pi->second;
	} else {
		// remember cursor position per pattern
		pattern_position_map_t::iterator pi;
		pi = patternPositionCache.find(std::make_pair(caption, pattern));

		if (pi != patternPositionCache.end())
			return pi->second;
	}

	{
		pattern_position pos;

		// no position found, use first patternformatcolumn as pos
		zzub_pattern_format_t* format = zzub_pattern_get_format(pattern);
		zzub_pattern_format_column_iterator_t* colit = zzub_pattern_format_get_iterator(format);
		if (zzub_pattern_format_column_iterator_valid(colit)) {
			zzub_pattern_format_column_t* col = zzub_pattern_format_column_iterator_current(colit);
			zzub_plugin_t* plugin = zzub_pattern_format_column_get_plugin(col);
			pos.plugin_id = zzub_plugin_get_id(plugin);
			pos.group = zzub_pattern_format_column_get_group(col);
			pos.track = zzub_pattern_format_column_get_track(col);
			pos.column = zzub_pattern_format_column_get_column(col);
			pos.digit = 0;
			pos.row = -1;
			//
			pos.scroll_x_unit = -1;
			pos.scroll_y_row = -1;
			//
			pos.select_from.row = -1;
			pos.select_from.plugin_id = -1;
			pos.select_from.group = -1;
			pos.select_from.track = -1;
			pos.select_from.column = -1;
			pos.select_to.row = -1;
			pos.select_to.plugin_id = -1;
			pos.select_to.group = -1;
			pos.select_to.track = -1;
			pos.select_to.column = -1;
		} else { // no columns found, give a null position
			pos.plugin_id = -1;
			pos.group = -1;
			pos.track = -1;
			pos.column = -1;
			pos.digit = -1;
			pos.row = -1;
			//
			pos.scroll_x_unit = -1;
			pos.scroll_y_row = -1;
			//
			pos.select_from.row = -1;
			pos.select_from.plugin_id = -1;
			pos.select_from.group = -1;
			pos.select_from.track = -1;
			pos.select_from.column = -1;
			pos.select_to.row = -1;
			pos.select_to.plugin_id = -1;
			pos.select_to.group = -1;
			pos.select_to.track = -1;
			pos.select_to.column = -1;
		}

		zzub_pattern_format_column_iterator_destroy(colit);
		return pos;
	}
}

void CPatternViewInfo::RemovePatternPosition(zzub_pattern_format_column_t* pfc) {
	zzub_plugin_t* plugin = zzub_pattern_format_column_get_plugin(pfc);
	int plugin_id = zzub_plugin_get_id(plugin);
	int group = zzub_pattern_format_column_get_group(pfc);
	int track = zzub_pattern_format_column_get_track(pfc);
	int column = zzub_pattern_format_column_get_column(pfc);
	{
		pattern_position_map_t::iterator pi;
		for (pi = patternPositionCache.begin(); pi != patternPositionCache.end(); ++pi) {
			if (true
				&& pi->second.plugin_id == plugin_id
				&& pi->second.group == group
				&& pi->second.track == track
				&& pi->second.column == column
			) { // allows y scroll to be retained
				pi->second.plugin_id = -1;
				pi->second.group = -1;
				pi->second.track = -1;
				pi->second.column = -1;
				pi->second.digit = -1;
			}
		}
	}

	{
		pattern_format_position_map_t::iterator pi;
		for (pi = patternFormatPositionCache.begin(); pi != patternFormatPositionCache.end(); ++pi) {
			if (true
				&& pi->second.plugin_id == plugin_id
				&& pi->second.group == group
				&& pi->second.track == track
				&& pi->second.column == column
			) { // allows y scroll to be retained
				pi->second.plugin_id = -1;
				pi->second.group = -1;
				pi->second.track = -1;
				pi->second.column = -1;
				pi->second.digit = -1;
			}
		}
	}
}

void CPatternViewInfo::SetPatternScroll(std::string const& caption, zzub_pattern_t* pattern, int cols, int rows) {
	pattern_position pos = GetPatternPosition(caption, pattern);

	pos.scroll_x_unit = cols;
	pos.scroll_y_row = rows;

	patternPositionCache[make_pair(caption, pattern)] = pos;
	patternFormatPositionCache[make_pair(caption, zzub_pattern_get_format(pattern))] = pos;
}

void CPatternViewInfo::SetPatternPosition(std::string const& caption, zzub_pattern_t* pattern, pattern_position const& pos) {
	patternPositionCache[make_pair(caption, pattern)] = pos;
	patternFormatPositionCache[make_pair(caption, zzub_pattern_get_format(pattern))] = pos;
}

void CPatternViewInfo::SetPatternSelection(std::string const& caption, zzub_pattern_t* pattern, selection_position const& select_from, selection_position const& select_to) {
	pattern_position pos = GetPatternPosition(caption, pattern);

	pos.select_from = select_from;
	pos.select_to = select_to;

	patternPositionCache[make_pair(caption, pattern)] = pos;
	patternFormatPositionCache[make_pair(caption, zzub_pattern_get_format(pattern))] = pos;
}


class CPatternViewLibrary : public CViewLibrary {
public:
	virtual void Initialize(CViewFrame* host) {
		_Module.m_hostModule = buze_application_get_host_module(buze_main_frame_get_application(host));
		buze_main_frame_register_window_factory(host, new CPatternViewInfo(host));
	}

	virtual void Destroy() {
		delete this;
	}

	virtual int GetVersion() {
		return CViewLibrary::version;
	}

};

extern "C" CViewLibrary* buze_create_viewlibrary() {
	return new CPatternViewLibrary();
}

//
// View
//

using std::cout;
using std::endl;

// ---------------------------------------------------------------------------------------------------------------
// ENUMERATIONS / CONSTANTS
// ---------------------------------------------------------------------------------------------------------------

namespace {

struct beat_type {
	int verydark, dark;
};

static const beat_type all_beats[] = {
	{ 8, 4 },
	{ 9, 3 },
	{ 12, 3 },
	{ 12, 4 },
	{ 16, 3 },
	{ 16, 4 },
	{ 16, 8 },
	{ 18, 3 },
	{ 18, 6 },
	{ 18, 9 },
	{ 24, 4 },
	{ 24, 6 },
	{ 24, 8 },
	{ 32, 8 },
	{ 64, 16 },
	{ 256, 64 },
};
static const int all_beats_count = array_size(all_beats);

static const int scales[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 12, 15, 16, 20, 24, 32, 36, 48, 64, 128, 256, 512, 1024
};
static const int scales_count = array_size(scales);

static const int pattern_sizes[] = {
	16, 32, 64, 128, 192, 256, 384, 512, 1024, 4096, 8192
};
static const int pattern_sizes_count = array_size(pattern_sizes);

} // END namespace

// ---------------------------------------------------------------------------------------------------------------
// GLOBAL HELPERS
// ---------------------------------------------------------------------------------------------------------------

namespace {

// int find_wave_column(zzub_pluginloader_t* loader) {
// 	for (int i = 0; i < zzub_pluginloader_get_parameter_count(loader, 2); ++i) {
// 		zzub_parameter_t* param = zzub_pluginloader_get_parameter(loader, 2, i);
// 		if ((zzub_parameter_get_flags(param) & zzub_parameter_flag_wavetable_index) != 0)
// 			return i;
// 	}
// 	return -1;
// }

int pattern_format_highest_track_count(zzub_pattern_format_t* format, zzub_plugin_t* plugin) {
	int count = 0;
	zzub_pattern_format_column_iterator_t* it = zzub_pattern_format_get_iterator(format);
	while (zzub_pattern_format_column_iterator_valid(it)) {
		zzub_pattern_format_column_t* col = zzub_pattern_format_column_iterator_current(it);
		zzub_plugin_t* plug = zzub_pattern_format_column_get_plugin(col);
		if (plug == plugin) {
			int group = zzub_pattern_format_column_get_group(col);
			if (group == 2) {
				int track = zzub_pattern_format_column_get_track(col);
				if (track > count)
					count = track;
			}
		}

		zzub_pattern_format_column_iterator_next(it);
	}
	zzub_pattern_format_column_iterator_destroy(it);
	return count + 1;
}

zzub_pattern_t* get_pattern_by_format_index(zzub_player_t* player, zzub_pattern_format_t* format, int index) {
	zzub_pattern_t* result = 0;
	int i = 0;

	zzub_pattern_iterator_t* patit = zzub_player_get_pattern_iterator(player);
	while (zzub_pattern_iterator_valid(patit)) {
		result = zzub_pattern_iterator_current(patit);
		zzub_pattern_format_t* testfmt = zzub_pattern_get_format(result);
		if (testfmt == format) {
			if (i == index)
				break; 
			else
				++i;
		}
		result = 0;

		zzub_pattern_iterator_next(patit);
	}
	zzub_pattern_iterator_destroy(patit);
	return result;
}

std::string GetTrackName(zzub_pattern_format_t* format, zzub_plugin_t* plugin, int group, int track/*, int column*/) {
	std::stringstream strm;
	const char* name = zzub_pattern_format_get_track_name(format, plugin, group, track);

	if (name == 0 || strlen(name) == 0) {
		name = zzub_plugin_get_name(plugin);
		switch (group) {
			case 0:
				strm << name << " Virtual";
				return strm.str().c_str();
			case 1:
				strm << name << " Global";
				return strm.str().c_str();
			case 2:
				strm << track << ":" << name << " Track";
				return strm.str();
			default:
				strm << name << " Meta";
				return strm.str();
		}
	} else {
		return name;
	}
}

} // END namespace

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DESTRUCTION
// ---------------------------------------------------------------------------------------------------------------

CPatternView::CPatternView(CViewFrame* m, CPatternViewInfo* viewInfo)
:
	CViewImpl(m),
	viewInfo(viewInfo),
	editorControl(editorScroller),
	editorInner(editorControl.editorInner),
	infoPane(m, this),
	orderList(m, this)
{
	configuration = buze_document_get_configuration(document);

	is_primary = false;
	pattern = 0;
	patternformat = 0;
	play_notes = true;
	show_infopane = false;
	font_size = 15;
	hsys_plugin_id = -1;
	follow_mode = false;
	orderlist_enabled = true;
	volume_masked = false;
										// ok for it to be false because of:
	dirtyMachinePatternPanel = false;	// BindToolbarControls
	dirtyWaveDropDown = false;			// BindToolbarControls
	dirtySongPositions = false;			// SetPattern
	dirtyPatternEditor = false;			// SetPattern,OnUpdate
	dirtyPatternEditorVertical = false;	// OnUpdate
	dirtyPatternEditorHorizontal = false; // OnUpdate
	dirtyColumnInfo = false;			// SetPattern
	dirtyPatternInfos = false;			// SetPattern,OnUpdate
	dirtyStatus = false;				// SetPattern,UpdatePatternEvent
	dirtyTracks = false;				// ?

	editorInner.keyjazz_key_map = (keyjazz_key_map_t*)buze_main_frame_get_keyjazz_map(mainframe); //&mainframe->hotkeys.keyjazz_key_map;

	pattern_stack_pos = 0;
}

CPatternView::~CPatternView() {
}

void CPatternView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CPatternView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam) {
	LRESULT lres = DefWindowProc();

	LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
	//buze_event_data* args = (buze_event_data*)cs->lpCreateParams;
	//int editor_id = args ? (int)args->show_pattern.editor_id : 0;
	int editor_id = (int)cs->lpCreateParams;

	SetRedraw(FALSE);
	{
		// for pattern_pos
		char caption_buf[1024];
		::GetWindowText(m_hWnd, caption_buf, 1024);
		caption = caption_buf;

		// TODO: get primary and initial pattern from args?
		is_primary = editor_id == 0; //true; // (caption == "Primary Pattern Editor");

		waveDropDown.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_WAVEDROPDOWN); /// had CCS_NORESIZE
		machinePatternPanel.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_MACHINEPATTERNPANEL);
		octaveDropDown.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_OCTAVEDROPDOWN);
		stepDropDown.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_STEPDROPDOWN);
		playNotesCheckbox.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PLAYNOTESCHECKBOX);
		patternscaleDropDown.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PATTERNSCALEDROPDOWN);
		patternbeatDropDown.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PATTERNBEATDROPDOWN);
		infoCheckbox.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_SHOWINFOCHECKBOX);
			infoCheckbox.ctrl().ModifyStyle(BS_CHECKBOX, BS_3STATE);
		patternrowsDropDown.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PATTERNROWSDROPDOWN);
		patternnameEdit.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PATTERNNAMEEDIT);
		hWndButtonToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_PATTERNVIEW, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
		patternloopCheckbox.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PATTERNLOOPCHECKBOX);
		followCheckbox.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_FOLLOWCHECKBOX);

		// using the thread was crashing Buze when recalling screensets too fast so we're doing BindToolbarControls directly now instead.
		//DWORD dwID;
		//CreateThread(0, 0, BindToolbarThread, this, 0, &dwID);
		BindToolbarControls();

		// Set up the splitter panes
		{
			RECT rcClient;
			GetClientRect(&rcClient);

			int info_width = 192;
			infoSplitter.Create(m_hWnd, rcClient, NULL, 0, 0);
			infoSplitter.m_cxySplitBar = 2; // makes a nice thin splitter handle
			scrollSplitter.Create(infoSplitter, rcDefault, NULL, 0, 0);
			scrollSplitter.m_cxySplitBar = 1;
			infoPane.Create(infoSplitter, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_INFOPANE);
			infoSplitter.SetSplitterPanes(scrollSplitter, infoPane);
			infoSplitter.SetSplitterExtendedStyle(SPLIT_RIGHTALIGNED);
			//infoSplitter.SetSplitterPos(rcClient.right - info_width);

			show_infopane = configuration->getShowInfoPane();
			if (show_infopane == BST_UNCHECKED)
				infoSplitter.SetSinglePaneMode(SPLIT_PANE_LEFT);

			editorScroller.Create(scrollSplitter, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
			editorControl.Create(scrollSplitter, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PATTERNEDITOR);
			scrollSplitter.SetSplitterPanes(editorControl, editorScroller);
			scrollSplitter.SetSplitterExtendedStyle(SPLIT_RIGHTALIGNED);
			editorScroller.SetInner(editorControl.editorInner.m_hWnd);
			//BindScrollerWidth();
		}

		UIAddToolBar(hWndButtonToolBar);

		{	static const int nParts = 5;
			int statusWidths[nParts] = { 100, 300, 600, 950, -1 };
			statusBar.Create(m_hWnd, rcDefault, 0, WS_VISIBLE|WS_CHILD);///, WS_EX_COMPOSITED); <-- works on Rebar but screws up the statusBar.
			statusBar.SetMinHeight(18);
			statusBar.SetParts(nParts, statusWidths);
		}

		BindSettings();
		BindTheme();

		// add the toolbar bands as late as possible since they will invoke a WM_SIZE on us
		int checkWidth = ::GetSystemMetrics(SM_CXMENUCHECK);
		
		bool bLock = configuration->getLockedToolbars();
		// -- editor toolbands --
		insertToolbarBand(waveDropDown, "&Wave", 200, -1, GetToolbarVisibility(ID_PATTERNVIEW_WAVETOOLTOOLBAR), bLock, FALSE, ID_PATTERNVIEW_WAVETOOLTOOLBAR);
		insertToolbarBand(machinePatternPanel, "", 200, -1, GetToolbarVisibility(ID_PATTERNVIEW_MACHINEPATTERNTOOLBAR), bLock, FALSE, ID_PATTERNVIEW_MACHINEPATTERNTOOLBAR);
		insertToolbarBand(octaveDropDown, "&Octave", 35, -1, GetToolbarVisibility(ID_PATTERNVIEW_OCTAVETOOLBAR), bLock, FALSE, ID_PATTERNVIEW_OCTAVETOOLBAR);
		insertToolbarBand(stepDropDown, "&Step", 40, -1, GetToolbarVisibility(ID_PATTERNVIEW_STEPTOOLBAR), bLock, FALSE, ID_PATTERNVIEW_STEPTOOLBAR);
		insertToolbarBand(playNotesCheckbox, "Play &Notes", checkWidth, -1, GetToolbarVisibility(ID_PATTERNVIEW_PLAYNOTESTOOLBAR), bLock, FALSE, ID_PATTERNVIEW_PLAYNOTESTOOLBAR);
		insertToolbarBand(infoCheckbox, "&Info", checkWidth, -1, GetToolbarVisibility(ID_PATTERNVIEW_INFOTOOLBAR), bLock, FALSE, ID_PATTERNVIEW_INFOTOOLBAR);
		if (is_primary)
			insertToolbarBand(followCheckbox, "Follow", checkWidth, -1, GetToolbarVisibility(ID_PATTERNVIEW_FOLLOWTOOLBAR), bLock, FALSE, ID_PATTERNVIEW_FOLLOWTOOLBAR);
		// -- pattern mutating toolbands --
		SIZE btbSize; SendMessage(hWndButtonToolBar, TB_GETMAXSIZE, 0, (LPARAM)&btbSize);
		insertToolbarBand(hWndButtonToolBar, "", btbSize.cx, -1, GetToolbarVisibility(ID_PATTERNVIEW_BUTTONBAR), bLock, FALSE, ID_PATTERNVIEW_BUTTONBAR);
		///toolBar.MinimizeBand(toolBar.GetBandCount()-1); <--- didn't work
		insertToolbarBand(patternloopCheckbox, "&Loop", checkWidth, -1, GetToolbarVisibility(ID_PATTERNVIEW_PATTERNLOOPTOOLBAR), bLock, FALSE, ID_PATTERNVIEW_PATTERNLOOPTOOLBAR);
		insertToolbarBand(patternscaleDropDown, "S&cale", 50, -1, GetToolbarVisibility(ID_PATTERNVIEW_PATTERNSCALETOOLBAR), bLock, FALSE, ID_PATTERNVIEW_PATTERNSCALETOOLBAR);
		insertToolbarBand(patternbeatDropDown, "&Beat", 55, -1, GetToolbarVisibility(ID_PATTERNVIEW_PATTERNBEATTOOLBAR), bLock, FALSE, ID_PATTERNVIEW_PATTERNBEATTOOLBAR);
		insertToolbarBand(patternrowsDropDown, "&Rows", 50, -1, GetToolbarVisibility(ID_PATTERNVIEW_PATTERNROWSTOOLBAR), bLock, FALSE, ID_PATTERNVIEW_PATTERNROWSTOOLBAR);
		insertToolbarBand(patternnameEdit, "N&ame", 70, -1, GetToolbarVisibility(ID_PATTERNVIEW_PATTERNNAMETOOLBAR), bLock, FALSE, ID_PATTERNVIEW_PATTERNNAMETOOLBAR);

		UpdateToolbarsFromConfiguration();

		UpdateOctaveDropdown();
		UpdateStepDropdown();
		UpdatePlayNotesCheckbox();
		UpdateShowInfoCheckbox();
		// don't need to update Scale,Beat,Rows,Name

		infoPane.PatternTreeRebuild();

		buze_document_add_view(document, this);

		CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddIdleHandler(this);
		pLoop->AddMessageFilter(this);

		buze_main_frame_viewstack_insert(mainframe, this); // true
		buze_main_frame_add_timer_handler(mainframe, this);

		if (zzub_player_get_pattern_count(player) > 0) {
			SetPattern(zzub_player_get_pattern_by_index(player, 0));
		}

		{	// Default horiz entry mode
			editorInner.SetHorizontalEntry(configuration->getDefaultEntryMode());

			// Default center scroll mode
			editorInner.SetHorizontalScrollMode(configuration->getHorizontalScrollMode());
			editorInner.SetVerticalScrollMode(configuration->getVerticalScrollMode());

			// Default notes affect mode
			editorInner.SetNotesAffectMode(configuration->getNotesAffectMode());

			editorControl.subrow_mode = configuration->getSubrowNamingMode();
		}

		// orderlist devices
		if (is_primary)
		{
			// Follow mode
			follow_mode = configuration->getPatternFollowMode();
			UpdateFollowCheckbox();

			//orderlist_enabled = configuration->getOrderlistEnabled();
			MakeDestroyOrderlist();
		}
	}
	SetRedraw(TRUE);
	RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);

	return 0;
}

LRESULT CPatternView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	SetPattern(0); // in case pattern is deleted and window updated before window is fully destroyed

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveIdleHandler(this);
	pLoop->RemoveMessageFilter(this);

	buze_document_remove_view(document, this);
	//document->removeLinkedPatternEditor(this);
	buze_main_frame_remove_timer_handler(mainframe, this);
	//mainframe->closeClientWindow(m_hWnd);

	if (harmonicxposeDlg.m_hWnd)
		harmonicxposeDlg.SendMessage(WM_CLOSE);
	if (notemaskDlg.m_hWnd)
		notemaskDlg.SendMessage(WM_CLOSE);

	return 0;
}

LRESULT CPatternView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	HIMAGELIST hImageList = (HIMAGELIST)::SendMessage(hWndButtonToolBar, TB_SETIMAGELIST, 0, 0);
	ImageList_Destroy(hImageList);

	return 0;
}

BOOL CPatternView::PreTranslateMessage(MSG* pMsg) {
//	preTranslateMessageSucceeded = true;

	if (GetFocus() == editorInner.m_hWnd) {
		HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "patternview");
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg)/* && preTranslateMessageSucceeded*/)
			return TRUE;
	}

	return FALSE;
}

// ---------------------------------------------------------------------------------------------------------------
// SCROLLING + SIZING
// ---------------------------------------------------------------------------------------------------------------

LRESULT CPatternView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	RECT rcClient;
	GetClientRect(&rcClient);

	if (rcClient.right == 0 || rcClient.bottom == 0) return 0;

	// get the splitterposes before any splitter resizing takes place to detect if they're initialized or not
	int ss = scrollSplitter.GetSplitterPos();
	int is = infoSplitter.GetSplitterPos();

	RECT rcStatus;
	statusBar.GetClientRect(&rcStatus);

	int orderHeight = is_primary && orderlist_enabled ? 42 : 0;
	int toolbarHeight = getToolbarHeight();
	int statusHeight = rcStatus.bottom - rcStatus.top;
	int vsplitterHeight = rcClient.bottom - statusHeight - toolbarHeight - orderHeight;

	if (is_primary && orderlist_enabled && orderList.m_hWnd)
		orderList.MoveWindow(0, toolbarHeight, rcClient.right, orderHeight);

	infoSplitter.MoveWindow(0, orderHeight + toolbarHeight, rcClient.right, vsplitterHeight);
	statusBar.MoveWindow(0, 0, 0, 0, FALSE);

	if (is == -1 && ss == -1) {
		// first time splitter size initialization
		int info_width = 192;
		infoSplitter.SetSplitterPos(rcClient.right - info_width);
		BindScrollerWidth();
	}
	return 0;
}

void CPatternView::OnScrolled() {
	if (pattern == 0) return ;

	int scroll_y_absolute = editorInner.scroll.y * editorInner.skip;
	viewInfo->SetPatternScroll(caption, pattern, editorInner.scroll.x, scroll_y_absolute);

	/*std::vector<std::vector<CPatternView*> >::iterator i = document->findLinkedPatternEditors(this);
	if (i == document->linkedPatternEditors.end()) return;

	// need to set master scrolling editor to prevent get in the way recursively
	if (document->linkedScrollOwner == 0) {
		document->linkedScrollOwner = this;
	} else
		return;

	std::vector<CPatternView*>::iterator j;
	for (j = i->begin(); j != i->end(); ++j) {
		if (*j == this) continue;
		POINT scroll;
		scroll.x = (*j)->editorInner.scroll.x;	// keep x scroll
		scroll.y = scroll_y_absolute * (*j)->editorInner.GetSkip(); // sync y scrolls  *in absolute rows*!
		(*j)->editorInner.ScrollTo(scroll);
	}

	document->linkedScrollOwner = 0;*/
}

// ---------------------------------------------------------------------------------------------------------------
// FOCUS
// ---------------------------------------------------------------------------------------------------------------

LRESULT CPatternView::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	editorControl.SetFocus();
	return 0;
}

LRESULT CPatternView::OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// PAINTING
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::UpdateTimer(int count) {
	if ((count % 1) != 0) return; /// 10ms

	if (pattern) {
		int row = zzub_pattern_get_currently_playing_row(pattern);
		editorInner.UpdatePlayPosition(row);
	} else {
		editorInner.UpdatePlayPosition(-1);
	}
}

LRESULT CPatternView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {///PRN(__FUNCTION__) END()
	if (dirtyMachinePatternPanel) {
		BindMachinePatternPanel();
		dirtyMachinePatternPanel = false;
	}

	if (dirtyWaveDropDown) {
		BindWaveDropdown();
		dirtyWaveDropDown = false;
	}

	if (dirtySongPositions) {
		BindSongPositions();
		dirtySongPositions = false;
	}

	if (dirtyPatternEditor) {
		BindPatternEditor();
		dirtyPatternEditor = false;
		dirtyPatternEditorVertical = false;
		dirtyPatternEditorHorizontal = false;
		dirtyColumnInfo = true; // ensure infopane updates when adding/removing patternformatcolumns
	}

	if (dirtyPatternEditorVertical) {
		BindPatternEditorVertical();
		dirtyPatternEditorVertical = false;
	}

	if (dirtyPatternEditorHorizontal) {
		BindPatternEditorHorizontal();
		dirtyPatternEditorHorizontal = false;
	}

	if (dirtyColumnInfo) {
		BindColumnInfo();
		dirtyColumnInfo = false;
	}

	if (dirtyPatternInfos) {
		BindPatternInfos();
		dirtyPatternInfos = false;
	}

	if (dirtyStatus) {
		BindStatus();
		dirtyStatus = false;
	}

	if (dirtyTracks) {
		BindTracks();
		dirtyTracks = false;
	}

	if (dirtyInfoPane) {
		BindInfoPane();
		dirtyInfoPane = false;
	}

	return DefWindowProc();
}


LRESULT CPatternView::OnPatternViewWaveToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_WAVETOOLTOOLBAR);
	bVisible = !bVisible;
	SetToolbarVisibility(ID_PATTERNVIEW_WAVETOOLTOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewMachinePatternToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_MACHINEPATTERNTOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_MACHINEPATTERNTOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewOctaveToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_OCTAVETOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_OCTAVETOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewStepToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_STEPTOOLBAR);
	bVisible = !bVisible;
	SetToolbarVisibility(ID_PATTERNVIEW_STEPTOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewPlayNotesToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_PLAYNOTESTOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_PLAYNOTESTOOLBAR , bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewInfoToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_INFOTOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_INFOTOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewFollowToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_FOLLOWTOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_FOLLOWTOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewPatternViewToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_BUTTONBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_BUTTONBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewLoopToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_PATTERNLOOPTOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_PATTERNLOOPTOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewScaleToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_PATTERNSCALETOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_PATTERNSCALETOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewBeatToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_PATTERNBEATTOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_PATTERNBEATTOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewPatternRowsToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_PATTERNROWSTOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_PATTERNROWSTOOLBAR, bVisible);
	return 0;
}

LRESULT CPatternView::OnPatternViewPatternNameToolbar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	BOOL bVisible = GetToolbarVisibility(ID_PATTERNVIEW_PATTERNNAMETOOLBAR);
	bVisible = !bVisible;

	SetToolbarVisibility(ID_PATTERNVIEW_PATTERNNAMETOOLBAR, bVisible);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// UPDATES
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint)
{///PRN(__FUNCTION__) END()
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	// give orderlist some updates
	if (is_primary && orderlist_enabled)
		orderList.OnUpdate(pSender, lHint, pHint);

	switch (lHint)
	{
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_new_document:
			// select first pattern in orderlist, if any
			if (zzub_player_get_order_length(player) > 0) {
				SetPattern(zzub_player_get_order_pattern(player, 0));
			}
			break;
		// patterns
		case zzub_event_type_insert_pattern:
			//if (zzub_pattern_get_format(zzubData->insert_pattern.pattern) == patternformat) {
				//if (pattern == 0) { // auto-select
				//	SetPattern(zzubData->insert_pattern.pattern);
				//}
				//dirtyMachinePatternPanel = true;
			//}
			dirtyPatternInfos = true;
			dirtyInfoPane = true;
			infoPane.PatternTreeInsertPattern(zzubData->insert_pattern.pattern);
			Invalidate(FALSE);
			break;
		case zzub_event_type_delete_pattern:
			if (zzub_pattern_get_format(zzubData->delete_pattern.pattern) == patternformat) {
				if (zzubData->delete_pattern.pattern == pattern) {
					SetPattern(0, patternformat);
				}
				dirtyMachinePatternPanel = true;
			}
			dirtyPatternInfos = true;
			dirtyInfoPane = true;
			infoPane.PatternTreeDeletePattern(zzubData->delete_pattern.pattern);
			PatternStackRemovePattern(zzubData->delete_pattern.pattern);
			Invalidate(FALSE);
			break;
		case zzub_event_type_update_pattern:
			if (zzubData->update_pattern.pattern == pattern) {
				// rows
				{	int rows = zzub_pattern_get_row_count(pattern);
					if (rows != editorInner.GetPatternRows()) {
						dirtyPatternEditor = true;
						Invalidate(FALSE);
						break;
					}
				}
				// resolution, skip, beat
				{	int reso = zzub_pattern_get_resolution(pattern);
					int skip = zzub_pattern_get_display_resolution(pattern);
					int verydark, dark; zzub_pattern_get_display_beat_rows(pattern, &verydark, &dark);
					if (false
						|| reso != editorInner.GetResolution()
						|| skip != editorInner.GetSkip()
						|| verydark != editorInner.verydark_row
						||     dark != editorInner.dark_row
					) {
						dirtyPatternEditorVertical = true;
						Invalidate(FALSE);
						break;
					}
				}
				// song positions
				{	int loop_begin_pos = zzub_pattern_get_loop_start(pattern);
					int loop_end_pos = zzub_pattern_get_loop_end(pattern);
					int loop_enabled = zzub_pattern_get_loop_enabled(pattern);
					if (false
					    || loop_begin_pos != editorInner.loop_begin_pos
						|| loop_end_pos != editorInner.loop_end_pos
						|| loop_enabled != editorInner.loop_enabled
					) {
						dirtySongPositions = true;
						Invalidate(FALSE);
						break;
					}
				}
				// replay
				{	int replay_row = zzub_pattern_get_replay_row(pattern);
					if (replay_row != editorControl.replay_row) {
						editorControl.replay_row = replay_row;
						editorControl.InvalidateRows();
						break;
					}
				}
			}
			if (zzub_pattern_get_format(zzubData->update_pattern.pattern) == patternformat) {
				dirtyMachinePatternPanel = true;
			}
			dirtyPatternInfos = true;	
			dirtyInfoPane = true;	
			infoPane.PatternTreeUpdatePattern(zzubData->update_pattern.pattern);
			Invalidate(FALSE);
			break;

		// pattern formats
		case zzub_event_type_insert_patternformat:
			if (patternformat == 0) { // auto-select
				SetPattern(0, zzubData->insert_pattern_format.patternformat);
			}
			dirtyMachinePatternPanel = true;
			dirtyInfoPane = true;
			infoPane.PatternTreeInsertPatternFormat(zzubData->insert_pattern_format.patternformat);
			Invalidate(FALSE);
			break;
		case zzub_event_type_delete_patternformat:
			if (zzubData->delete_pattern_format.patternformat == patternformat) {
				SetPattern(0);
			}
			dirtyMachinePatternPanel = true;
			dirtyInfoPane = true;
			infoPane.PatternTreeDeletePatternFormat(zzubData->delete_pattern_format.patternformat);
			Invalidate(FALSE);
			break;
		case zzub_event_type_update_patternformat:
			dirtyMachinePatternPanel = true;
			dirtyInfoPane = true;
			infoPane.PatternTreeUpdatePatternFormat(zzubData->update_pattern_format.patternformat);
			Invalidate(FALSE);
			break;

		// pattern format columns
		case zzub_event_type_insert_patternformatcolumn:
			if (zzub_pattern_format_column_get_format(zzubData->insert_pattern_format_column.patternformatcolumn) == patternformat) {
				dirtyPatternEditor = true;
				Invalidate(FALSE);
			}
			break;
		case zzub_event_type_delete_patternformatcolumn:
			if (zzub_pattern_format_column_get_format(zzubData->delete_pattern_format_column.patternformatcolumn) == patternformat) {
				dirtyPatternEditor = true;
				Invalidate(FALSE);
			}
			break;
		case zzub_event_type_update_patternformatcolumn: // toggling, collapsing, rearranging
			if (zzub_pattern_format_column_get_format(zzubData->update_pattern_format_column.patternformatcolumn) == patternformat) {
				dirtyPatternEditorHorizontal = true;
				Invalidate(FALSE);
			}
			break;

		// pattern format tracks
		case zzub_event_type_insert_patternformattrack:
			///if ( == patternformat) { // todo- fix this, needs update event data
				dirtyPatternEditor = true;
				Invalidate(FALSE);
			///}
			break;
		case zzub_event_type_delete_patternformattrack:
			///if ( == patternformat) {
				dirtyPatternEditor = true;
				Invalidate(FALSE);
			///}
			break;
		case zzub_event_type_update_patternformattrack:
			///if ( == patternformat) {
				dirtyTracks = true; // track names, mute states
				Invalidate(FALSE);
			///}
			break;

		// pattern events
		case zzub_event_type_insert_patternevent:
			if (dirtyPatternEditor) break; // dont update events if a rebind is pending, attempting to do so could crashes due to invalid data
			if (zzub_pattern_event_get_pattern(zzubData->insert_patternevent.patternevent) == pattern) {
				UpdatePatternEvent(zzubData->insert_patternevent.patternevent, lHint);
			}
			break;
		case zzub_event_type_delete_patternevent:
			if (dirtyPatternEditor) break; // dont update events if a rebind is pending, attempting to do so could crashes due to invalid data
			if (zzub_pattern_event_get_pattern(zzubData->delete_patternevent.patternevent) == pattern) {
				UpdatePatternEvent(zzubData->delete_patternevent.patternevent, lHint);
			}
			break;
		case zzub_event_type_update_patternevent:
			if (dirtyPatternEditor) break; // dont update events if a rebind is pending, attempting to do so could crashes due to invalid data
			if (zzub_pattern_event_get_pattern(zzubData->update_patternevent.patternevent) == pattern) {
				UpdatePatternEvent(zzubData->update_patternevent.patternevent, lHint);
			}
			break;

		// plugins
		case zzub_event_type_insert_plugin:
		case zzub_event_type_delete_plugin:
		case zzub_event_type_update_plugin:
			dirtyMachinePatternPanel = true;
			dirtyTracks = true; // for default track names on renamed plugins 
			Invalidate(FALSE);
			break;

		// waves
		case zzub_event_type_insert_wave:
		case zzub_event_type_delete_wave:
		case zzub_event_type_update_wave:
			dirtyWaveDropDown = true;
			Invalidate(FALSE);
			break;

		// settings
		case buze_event_type_update_settings:
			BindSettings();
			UpdateToolbarsFromConfiguration();
			break;
		case buze_event_type_update_theme:
			BindTheme();
			break;

		// midi
		case zzub_event_type_midi_control:
			if (GetFocus() == editorInner) {
				HandleMidi(zzubData->midi_message);
			}
			break;
	}
}

// ---------------------------------------------------------------------------------------------------------------
// BINDING
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::BindInfoPane() {
	infoPane.RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);
}

// TODO: if patid's aren't perfectly contiguous, then "???" won't work in PatternEditorColumn
void CPatternView::BindPatternInfos() {
	zzub_pattern_iterator_t* patit = zzub_player_get_pattern_iterator(player);
	assert(patit != 0);

	std::vector<PE_patterninfo> infos;
	std::map<string, int> names;

	while (zzub_pattern_iterator_valid(patit)) {
		zzub_pattern_t* pat = zzub_pattern_iterator_current(patit);
		const char* name = zzub_pattern_get_name(pat);
		int patid = zzub_pattern_get_id(pat);

		if (patid >= infos.size()) infos.resize(patid + 1);
		infos[patid].name = name;
		infos[patid].length = zzub_pattern_get_row_count(pat) / zzub_pattern_get_resolution(pat);
		infos[patid].loop_begin = zzub_pattern_get_loop_start(pat);
		infos[patid].loop_end = zzub_pattern_get_loop_end(pat);
		infos[patid].loop_enabled = zzub_pattern_get_loop_enabled(pat);

		names[name] = patid;

		zzub_pattern_iterator_next(patit);
	}
	zzub_pattern_iterator_destroy(patit);

	editorInner.SetPatternInfos(infos, names);

	UpdatePatternNameEdit();///
}

void CPatternView::BindScrollerWidth() {

	CRect rcSplitter;
	scrollSplitter.GetClientRect(&rcSplitter);
	if (rcSplitter.right == 0) return ;

	int scroller_width_default = configuration->getDefaultScrollerWidth();

	int scroller_width;
	if (patternformat == 0) {
		scroller_width = scroller_width_default;
	} else {
		scroller_width = zzub_pattern_format_get_scroller_width(patternformat);

		if (scroller_width == -1)
			scroller_width = scroller_width_default;
	}

	scrollSplitter.SetSplitterPos(rcSplitter.right - scroller_width);
}

void CPatternView::OnInnerResized() {
	BindScrollSplitterDoubleClickPos();

	if (patternformat == 0) return;

	bool store_scroll = false;

	int scroller_width_current = scrollSplitter.m_nProportionalPos + 1;
	int scroller_width_format = zzub_pattern_format_get_scroller_width(patternformat);

	if (scroller_width_format == -1) {
		int scroller_width_default = configuration->getDefaultScrollerWidth();
		if (scroller_width_current != scroller_width_default)
			store_scroll = true;
	} else {
		if (scroller_width_current != scroller_width_format)
			store_scroll = true;
	}

	if (store_scroll) {
		zzub_pattern_format_set_scroller_width(patternformat, scroller_width_current);
		zzub_player_history_commit(player, 0, 0, "Resize Pattern Scroller");
	}
}

void CPatternView::BindSongPositions() {
	if (pattern != 0) {
		editorInner.loop_begin_pos = zzub_pattern_get_loop_start(pattern);
		editorInner.loop_end_pos = zzub_pattern_get_loop_end(pattern);
		editorInner.loop_enabled = zzub_pattern_get_loop_enabled(pattern);
			UpdatePatternLoopCheckbox();
		editorInner.Invalidate(FALSE);
		editorControl.replay_row = zzub_pattern_get_replay_row(pattern);
		editorControl.InvalidateRows();
	} else {
		editorInner.loop_begin_pos = 0;
		editorInner.loop_end_pos = 0;
		editorInner.loop_enabled = 0;
		editorControl.replay_row = -1;
	}
}

void CPatternView::BindTracks() {
	for (int i = 0; i < editorInner.tracks.size(); ++i) {
		PE_track& track = editorInner.tracks[i];

		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, track.plugin_id);

		track.name = GetTrackName(patternformat, plugin, track.group, track.track);
		track.is_muted = zzub_pattern_format_get_track_mute(patternformat, plugin, track.group, track.track);
	}

	editorControl.InvalidateCols();
}

// Restores positions from pattern position cache
void CPatternView::BindPatternPosition() {
	if (InvalidPattern()) return ;

	pattern_position pos = viewInfo->GetPatternPosition(caption, pattern);

	editorControl.SetCursor(pos.plugin_id, pos.group, pos.track, pos.column, pos.digit, pos.row);

	if (pos.select_from.column != -1) {
		editorControl.SelectRange(
			pos.select_from.plugin_id, pos.select_from.group, pos.select_from.track, pos.select_from.column, pos.select_from.row,
			pos.select_to.plugin_id, pos.select_to.group, pos.select_to.track, pos.select_to.column, pos.select_to.row
		);
	} else {
		editorInner.Unselect();
	}
}

void CPatternView::BindPatternScroll() {
	{	pattern_position pos = viewInfo->GetPatternPosition(caption, pattern);

		if (pos.scroll_y_row != -1) {
			int scroll_y_skipped = pos.scroll_y_row / editorInner.skip;
			POINT scroll = { pos.scroll_x_unit, scroll_y_skipped };
			editorInner.ScrollTo(scroll);
		}
	}

	if (editorInner.dirty_centeroncursor) {
		editorInner.dirty_centeroncursor = false;
		editorInner.dirty_scrolltocursor = false;
		editorInner.ScrollToCursorPointCenteredHorizontal(editorInner.cursor, editorInner.scroll);
	} else
	if (editorInner.dirty_scrolltocursor) {
		editorInner.dirty_scrolltocursor = false;
		editorInner.ScrollToCursorPoint(editorInner.cursor, editorInner.scroll, false);
	}
}

void CPatternView::BindPatternEditorRows() {
	if (pattern == 0) return ;

	// set rows
	editorControl.SetPatternRows(zzub_pattern_get_row_count(pattern));
	UpdatePatternRowsDropdown();

	// set skip/scale
	int skip = zzub_pattern_get_display_resolution(pattern);
	editorControl.SetSkip(skip);
	UpdateScaleDropdown();

	// set resolution
	int resolution = zzub_pattern_get_resolution(pattern);
	editorControl.SetResolution(resolution);

	// set highlight rows
	int darker, dark;
	zzub_pattern_get_display_beat_rows(pattern, &darker, &dark);
	editorInner.SetHighlightRows(darker, dark);
	UpdateBeatDropdown();

	// bind song positions
	BindSongPositions();
}

// Partial rebind -- rows/scale/reso/beats
void CPatternView::BindPatternEditorVertical() {
	SetRedraw(FALSE);
	{
		BindPatternEditorRows();

		editorInner.AllocatePatternVertical();
		BindPatternPosition();
		editorInner.UpdateScrollbars();
		BindPatternScroll();

		editorInner.BindPatternImg();
	}
	SetRedraw(TRUE);
	RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);

	BindStatus();//ack!
}

void CPatternView::BindScrollSplitterDoubleClickPos() {
	RECT rcSplitter;
	scrollSplitter.GetClientRect(&rcSplitter);
	int split_pos = rcSplitter.right - editorInner.editor_units;
	scrollSplitter.SetDoubleClickSplitterPos(split_pos);
}

void CPatternView::BindPatternEditorHorizontal() {
	{	// temporary workaround because there's no way of knowing if zzub_event_type_update_patternformatcolumn means an idx got updated
		bool do_rebind = false;

		int i = 0;
		zzub_pattern_format_column_iterator_t* colit = zzub_pattern_format_get_iterator(patternformat);
		while (zzub_pattern_format_column_iterator_valid(colit)) {
			zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_column_iterator_current(colit);
			int plugin_id = zzub_plugin_get_id(zzub_pattern_format_column_get_plugin(fmtcol));
			int group = zzub_pattern_format_column_get_group(fmtcol);
			int track = zzub_pattern_format_column_get_track(fmtcol);
			int column = zzub_pattern_format_column_get_column(fmtcol);

			PE_column const& col = *editorInner.columns[i];
			if (false
				|| col.plugin_id != plugin_id
				|| col.group != group
				|| col.track != track
				|| col.column != column
			) {
				do_rebind = true;
				break;
			}

			++i;
			zzub_pattern_format_column_iterator_next(colit);
		}
		zzub_pattern_format_column_iterator_destroy(colit);

		if (do_rebind) {
			BindPatternEditor();
			return;
		}
	}

	for (int i = 0; i < editorInner.GetColumnCount(); ++i) {
		PE_column& col = *editorInner.columns[i];
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
		zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_get_column(patternformat, plugin, col.group, col.track, col.column);

		int mode = zzub_pattern_format_column_get_mode(fmtcol);
		col.control = mode ? mode : col.defaultcontrol;

		int is_collapsed = zzub_pattern_format_column_get_collapsed(fmtcol);
		col.is_collapsed = is_collapsed;

		editorInner.SetColumnEditor(col);
	}

	editorInner.SetRedraw(FALSE);
	editorControl.SetRedraw(FALSE);
	{
		editorInner.AllocatePattern();
		BindPatternPosition();
		editorInner.UpdateScrollbars();
		BindPatternScroll();

		editorInner.BindPatternImg();
		BindScrollSplitterDoubleClickPos();
	}
	editorInner.SetRedraw(TRUE);
	editorControl.SetRedraw(TRUE);
	editorControl.RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);
}

void CPatternView::AddSpecialPlugins(zzub_plugin_t* plugin) {
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(plugin);
	std::string uri = zzub_pluginloader_get_uri(loader);
	if (uri == "@zzub.org/harmony") {
		if (hsys_plugin_id == -1) {
			hsys_plugin_id = zzub_plugin_get_id(plugin);
			editorInner.HSysEnable(true);
		}
	}
}

void CPatternView::BindSpecialPlugins() {
	if (hsys_plugin_id != -1) {
		zzub_plugin_t* hsys_plugin = zzub_player_get_plugin_by_id(player, hsys_plugin_id);
		int hsys_track_count = pattern_format_highest_track_count(patternformat, hsys_plugin);
		editorInner.hsys.SetTrackCount(hsys_track_count);

		for (int i = 0; i < hsys_track_count; ++i) {
			PE_column* col = editorControl.GetColumn(hsys_plugin_id, 2, i, 0);
			PE_values_by_time& vals = col->values_by_time;
			for (PE_values_by_time::iterator j = vals.begin(); j != vals.end(); ++j) {
				editorInner.hsys.Add(i, j->time, (Harmony::Symbols::T)j->value);
			}
		}
	}
}

void CPatternView::BindPatternEditor()
{
	SetRedraw(FALSE);

//	U64 mark = highres_clock();

	if (pattern == 0) {
		DisableToolbands();
		editorControl.column_map.clear();
		editorInner.ClearColumns();
		editorInner.HSysEnable(false);
		editorInner.AllocatePattern();
		editorInner.UpdateScrollbars();
		editorInner.UpdateCaret();
	} else {
		EnableToolbands();

		// hashed mapping
		editorControl.column_map.clear();

		// start fresh
		editorInner.Init();
		editorInner.ClearColumns();
		editorInner.HSysEnable(false);
		hsys_plugin_id = -1;

		// Pattern toolbands
		UpdatePatternNameEdit();///

		// Rows info
		BindPatternEditorRows();

		// use the associated zzub_pattern_format_t* to build a pattern editor
		zzub_pattern_format_t* format = zzub_pattern_get_format(pattern);

		int prev_plugin_id = -1;
		int prev_group = -1;
		int prev_track = -1;
		int column_idx = 0;
		zzub_pattern_format_column_iterator_t* colit = zzub_pattern_format_get_iterator(format);
		while (zzub_pattern_format_column_iterator_valid(colit))
		{
			zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_column_iterator_current(colit);
			zzub_plugin_t* plugin = zzub_pattern_format_column_get_plugin(fmtcol);
			int plugin_id = zzub_plugin_get_id(plugin);
			int group = zzub_pattern_format_column_get_group(fmtcol);
			int track = zzub_pattern_format_column_get_track(fmtcol);
			int column = zzub_pattern_format_column_get_column(fmtcol);
			int mode = zzub_pattern_format_column_get_mode(fmtcol);
			int is_collapsed = zzub_pattern_format_column_get_collapsed(fmtcol);

			zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, track, column);
			int type = zzub_parameter_get_type(param);
			int flags = zzub_parameter_get_flags(param);
			int novalue = zzub_parameter_get_value_none(param);
			int minvalue = zzub_parameter_get_value_min(param);
			int maxvalue = zzub_parameter_get_value_max(param);
			std::string paramname = zzub_parameter_get_name(param);

			int control;
			int defaultcontrol;
			int flagtype;

			if (prev_plugin_id != plugin_id)
				AddSpecialPlugins(plugin);

			// add track?
			{
				bool add_track = false;
				if (prev_plugin_id == plugin_id) {
					if (prev_group == group) {
						if (prev_track != track) { // first track in new track
							add_track = true; 
						}
					} else { // first column in new group
						add_track = true;
					}
				} else { // first track on new plugin
					add_track = true;
				}

				if (add_track) {
					std::string track_name = GetTrackName(format, plugin, group, track);
					int is_muted = zzub_pattern_format_get_track_mute(format, plugin, group, track);
					editorInner.AddTrack(track_name, plugin_id, group, track, is_muted);
				}
			}

			// choose control
			{
				if (flags & zzub_parameter_flag_pattern_index)///
					defaultcontrol = pattern_column_control_pattern;///
				else if (flags & zzub_parameter_flag_harmony_index)
					defaultcontrol = pattern_column_control_harmonic;
				else if (flags & zzub_parameter_flag_meta_note)
					defaultcontrol = pattern_column_control_pianoroll;
				else if (flags & zzub_parameter_flag_meta_wave)
					defaultcontrol = pattern_column_control_wave;
				else///
					defaultcontrol = CColumnEditor::GetDefaultTypeControl(type/*, flags*/);

				control = mode ? mode : defaultcontrol;
			}

			// set column flagtype
			{
				const char* description = zzub_parameter_get_description(param);
				if (flags & zzub_parameter_flag_wavetable_index) {
					flagtype = pattern_column_flagtype_wave;
				} else
				if (flags & zzub_parameter_flag_velocity_index) {
					flagtype = pattern_column_flagtype_volume;
				} else {
					flagtype = pattern_column_flagtype_none;
				}
			}

			// bind column
			{
				editorInner.AddColumn(plugin_id, group, track, column, type, novalue, minvalue, maxvalue, control, defaultcontrol, flagtype, is_collapsed, paramname);
				PE_column* col = editorInner.columns[column_idx];
				editorControl.column_map[PE_column_pos(plugin_id, group, track, column)] = col;
			}

			prev_plugin_id = plugin_id;
			prev_group = group;
			prev_track = track;

			++column_idx;
			zzub_pattern_format_column_iterator_next(colit);
		}
		zzub_pattern_format_column_iterator_destroy(colit);
		
		FillPatternEditor();

		BindSpecialPlugins();

		// bind units
		{
			editorInner.AllocatePattern();
			if (editorInner.GetColumnCount() > 0) BindPatternPosition();
			editorInner.UpdateScrollbars();
			if (editorInner.GetColumnCount() > 0) BindPatternScroll();
		}

		// bind preview
		if (editorInner.GetColumnCount() > 0) editorInner.BindPatternImg();

		BindScrollSplitterDoubleClickPos();

		CheckColumnInfoVisibility();

		BindStatus();
	}

	editorControl.SetTooltips();

// 	U64 done = highres_clock();
// 	std::cerr << "rebind time: " << (done - mark) << std::endl;

	SetRedraw(TRUE);
	RedrawWindow(0, 0, RDW_ALLCHILDREN|RDW_INVALIDATE);
}

// ---------------------------------------------------------------------------------------------------------------
// REARRANGING
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnTrackSwapRight() {
	PE_column const& col = editorInner.GetColumnAtCursor();

	int this_idx = col.track_index;
	int dest_idx = this_idx + 1;

	if (true
		&& this_idx < (editorInner.tracks.size() - 1)
		&& editorInner.tracks[dest_idx].plugin_id == col.plugin_id
		&& editorInner.tracks[dest_idx].group == col.group
	) {
		int left = -1;
		int right = -1;
		int top = -1;
		int bottom = -1;
		int new_left = -1;

		RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();

		if (editorInner.HasSelection()) {
			left = rcSel.left;

			PE_column const& left_col = editorInner.GetColumn(rcSel.left);
			PE_column const& right_col = editorInner.GetColumn(rcSel.right);

			if (false
				|| left_col.plugin_id != right_col.plugin_id
				|| left_col.group != right_col.group
				|| left_col.track != right_col.track
			) {
				return;
			}

			for (int i = editorInner.GetColumnCount() - 1; i >= 0; --i) {
				PE_column const& check = editorInner.GetColumn(i);
				if (true
					&& right == -1
					&& check.track_index == dest_idx
					&& check.column == right_col.column
				) {
					right = check.index;
				}

				if (true
					&& new_left == -1
					&& check.track_index == dest_idx
					&& check.column == left_col.column
				) {
					new_left = check.index;
				}

				if (right != -1 && new_left != -1) {
					break;
				}
			}

			top = rcSel.top;
			bottom = rcSel.bottom;
		} else {
			for (int i = editorInner.GetColumnCount() - 1; i >= 0; --i) {
				PE_column const& check = editorInner.GetColumn(i);
				if (check.track_index == dest_idx) {
					right = check.index;
					break;
				}
			}

			for (int i = 0; i < editorInner.GetColumnCount(); ++i) {
				PE_column const& check = editorInner.GetColumn(i);
				if (check.track_index == this_idx) {
					left = check.index;
					break;
				}
			}

			top = 0;
			bottom = editorInner.pattern_rows;
		}

		if (left != -1 && right != -1) {
			int length = bottom - top + 1;
			zzub_pattern_swap_track_events(pattern, left, right, top, length);
			zzub_player_history_commit(player, 0, 0, "Swap Track Right Selection");

			{	PE_cursorcolumn const& ccol = editorInner.GetCursorColumnAtCursor();
				int row = editorInner.GetCursorRowAbsolute();
				bool valid = editorControl.SetCursor(col.plugin_id, col.group, editorInner.GetColumn(right).track, col.column, ccol.digit, row);
				if (!valid)
					editorControl.SetCursor(col.plugin_id, col.group, editorInner.GetColumn(right).track, 0, 0, row);
			}

			if (editorInner.HasSelection()) {
				editorInner.SelectRangeAbsolute(new_left, rcSel.top, right, rcSel.bottom);
				int right_cur = editorInner.ScreenUnitsToCursorColumn(
					editorInner.GetColumn(right).unit
				);
				editorInner.ScrollToCursorPoint(CPoint(right_cur, editorInner.cursor.y), editorInner.scroll, false);
			} else {
				int right_cur = editorInner.ScreenUnitsToCursorColumn(
					editorInner.GetColumn(editorInner.tracks[editorInner.GetColumn(right).track_index].last_column_idx).unit
				);
				editorInner.ScrollToCursorPoint(CPoint(right_cur, editorInner.cursor.y), editorInner.scroll, false);
			}
		}
	}
}

void CPatternView::OnTrackSwapLeft() {
	PE_column const& col = editorInner.GetColumnAtCursor();

	int this_idx = col.track_index;
	int dest_idx = this_idx - 1;

	if (true
		&& this_idx > 0
		&& editorInner.tracks[dest_idx].plugin_id == col.plugin_id
		&& editorInner.tracks[dest_idx].group == col.group
	) {
		int left = -1;
		int right = -1;
		int top = -1;
		int bottom = -1;
		int new_right = -1;

		RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();

		if (editorInner.HasSelection()) {
			right = rcSel.right;

			PE_column const& left_col = editorInner.GetColumn(rcSel.left);
			PE_column const& right_col = editorInner.GetColumn(rcSel.right);

			if (false
				|| left_col.plugin_id != right_col.plugin_id
				|| left_col.group != right_col.group
				|| left_col.track != right_col.track
			) {
				return;
			}

			for (int i = 0; i < editorInner.GetColumnCount(); ++i) {
				PE_column const& check = editorInner.GetColumn(i);
				if (true
					&& left == -1
					&& check.track_index == dest_idx
					&& check.column == left_col.column
				) {
					left = check.index;
				}

				if (true
					&& new_right == -1
					&& check.track_index == dest_idx
					&& check.column == right_col.column
				) {
					new_right = check.index;
				}

				if (left != -1 && new_right != -1) {
					break;
				}
			}

			top = rcSel.top;
			bottom = rcSel.bottom;
		} else {
			for (int i = 0; i < editorInner.GetColumnCount(); ++i) {
				PE_column const& check = editorInner.GetColumn(i);
				if (check.track_index == dest_idx) {
					left = check.index;
					break;
				}
			}

			for (int i = editorInner.GetColumnCount() - 1; i >= 0; --i) {
				PE_column const& check = editorInner.GetColumn(i);
				if (check.track_index == this_idx) {
					right = check.index;
					break;
				}
			}

			top = 0;
			bottom = editorInner.pattern_rows;
		}

		if (left != -1 && right != -1) {
			int length = bottom - top + 1;
			zzub_pattern_swap_track_events(pattern, left, right, top, length);
			zzub_player_history_commit(player, 0, 0, "Swap Track Left Selection");

			{	PE_cursorcolumn const& ccol = editorInner.GetCursorColumnAtCursor();
				int row = editorInner.GetCursorRowAbsolute();
				bool valid = editorControl.SetCursor(col.plugin_id, col.group, editorInner.GetColumn(left).track, col.column, ccol.digit, row);
				if (!valid)
					editorControl.SetCursor(col.plugin_id, col.group, editorInner.GetColumn(left).track, 0, 0, row);
			}

			if (editorInner.HasSelection()) {
				editorInner.SelectRangeAbsolute(left, rcSel.top, new_right, rcSel.bottom);
				int left_cur = editorInner.ScreenUnitsToCursorColumn(
					editorInner.GetColumn(left).unit
				);
				editorInner.ScrollToCursorPoint(CPoint(left_cur, editorInner.cursor.y), editorInner.scroll, false);
			} else {
				int left_cur = editorInner.ScreenUnitsToCursorColumn(
					editorInner.GetColumn(editorInner.tracks[editorInner.GetColumn(left).track_index].first_column_idx).unit
				);
				editorInner.ScrollToCursorPoint(CPoint(left_cur, editorInner.cursor.y), editorInner.scroll, false);
			}
		}
	}
}

void CPatternView::OnFormatLayoutPluginLeft() {
	bool did_rearrange = DoFormatLayoutPlugin(false);
	if (did_rearrange) {
		editorInner.dirty_centeroncursor = true;
		zzub_player_history_commit(player, 0, 0, "Rearrange Pattern Format");
	}
}

void CPatternView::OnFormatLayoutPluginRight() {
	bool did_rearrange = DoFormatLayoutPlugin(true);
	if (did_rearrange) {
		editorInner.dirty_centeroncursor = true;
		zzub_player_history_commit(player, 0, 0, "Rearrange Pattern Format");
	}
}

bool CPatternView::DoFormatLayoutPlugin(bool direction) {
	PE_column const& col = editorInner.GetColumnAtCursor();

	typedef std::vector<zzub_pattern_format_column_t*> col_vec_t;
	typedef std::deque<col_vec_t> plugs_vec_t; // make sure found_pluginid stays valid
	plugs_vec_t plugs_vec;
	plugs_vec_t::iterator found_pluginid;

	// make vectors of columns for each plugin
	{	bool found = false;
		int last_pluginid = -1;

		zzub_pattern_format_column_iterator_t* colit = zzub_pattern_format_get_iterator(patternformat);
		while (zzub_pattern_format_column_iterator_valid(colit)) {
			zzub_pattern_format_column_t* formatcol = zzub_pattern_format_column_iterator_current(colit);
			int pluginid = zzub_plugin_get_id(zzub_pattern_format_column_get_plugin(formatcol));

			if (pluginid != last_pluginid)
				plugs_vec.push_back(col_vec_t());

			plugs_vec.back().push_back(formatcol);

			if (pluginid == col.plugin_id && !found) {
				found = true;
				found_pluginid = plugs_vec.end() - 1;
			}

			last_pluginid = pluginid;
			zzub_pattern_format_column_iterator_next(colit);
		}
		zzub_pattern_format_column_iterator_destroy(colit);
	}

	bool did_rearrange = false;

	// swap with next or previous plugin
	if (direction) {
		if (found_pluginid != plugs_vec.end() - 1) {
			(*(found_pluginid)).swap(*(found_pluginid + 1));
			did_rearrange = true;
		}
	} else {
		if (found_pluginid != plugs_vec.begin()) {
			(*(found_pluginid - 1)).swap(*(found_pluginid));
			did_rearrange = true;
		}
	}

	// apply the changes
	if (did_rearrange) {
		int idx = 0;
		for (plugs_vec_t::iterator i = plugs_vec.begin(); i != plugs_vec.end(); ++i) {
			for (col_vec_t::iterator j = (*i).begin(); j != (*i).end(); ++j) {
				zzub_pattern_format_column_set_index(*j, idx);
				++idx;
			}
		}
	}

	return did_rearrange;
}

// ---------------------------------------------------------------------------------------------------------------
// DATA BINDING
// ---------------------------------------------------------------------------------------------------------------

// void CPatternView::FillPatternEditor() {
// 	zzub_pattern_event_iterator_t* it = zzub_pattern_get_event_iterator(pattern, 0, -1, -1, -1);
// 	while (zzub_pattern_event_iterator_valid(it)) {
// 		zzub_pattern_event_t* ev = zzub_pattern_event_iterator_current(it);
// 		int value = zzub_pattern_event_get_value(ev);
// 		int meta = zzub_pattern_event_get_meta(ev);
// 
// 		int plugin_id = zzub_pattern_event_get_pluginid(ev);
// 		int group = zzub_pattern_event_get_group(ev);
// 		int track = zzub_pattern_event_get_track(ev);
// 		int column = zzub_pattern_event_get_column(ev);
// 
// 		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, plugin_id);
// 		zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, track, column);
// 		int novalue = zzub_parameter_get_value_none(param);
// 
// 		if (value != novalue) {
// 			int id = zzub_pattern_event_get_id(ev);
// 			int time = zzub_pattern_event_get_time(ev);
// 
// 			PE_column* col = editorControl.GetColumn(plugin_id, group, track, column);
// 			if (col) {
// 				col->SetValueInitial(id, time, value, meta);
// 			} else {
// 				std::cerr << "FillPatternEditor: Pattern data outside visible area!" << std::endl;
// 			}
// 		}
// 
// 		zzub_pattern_event_iterator_next(it);
// 	}
// 	zzub_pattern_event_iterator_destroy(it);
// }

void CPatternView::FillPatternEditor() {
	zzub_pattern_event_iterator_t* it = zzub_pattern_get_event_unsorted_iterator(pattern, 0, -1, -1, -1);

	bool unbound_data = false;

	while (zzub_pattern_event_iterator_valid(it)) {
		zzub_pattern_event_t* ev = zzub_pattern_event_iterator_current(it);

		int plugin_id = zzub_pattern_event_get_pluginid(ev);
		int group = zzub_pattern_event_get_group(ev);
		int track = zzub_pattern_event_get_track(ev);
		int column = zzub_pattern_event_get_column(ev);

		int value = zzub_pattern_event_get_value(ev);
		int meta = zzub_pattern_event_get_meta(ev);

		int id = zzub_pattern_event_get_id(ev);
		int time = zzub_pattern_event_get_time(ev);

		PE_column* col = editorControl.GetColumn(plugin_id, group, track, column);
		if (col) {
			col->SetValueInitial(id, time, value, meta);
		} else {
			unbound_data = true;
			zzub_plugin_t* pl = zzub_player_get_plugin_by_id(player, plugin_id);
			if (pl) {
				cout << "plugin: " << zzub_plugin_get_name(pl) << endl;
			}
			cout << "unbound data at " << plugin_id << ", " << group << ", " << track << ", " << column << endl;
		}

		zzub_pattern_event_iterator_next(it);
	}
	zzub_pattern_event_iterator_destroy(it);

	if (unbound_data)
		std::cerr << "FillPatternEditor: Pattern data outside visible area!" << std::endl;
}

void CPatternView::UpdatePatternEvent(zzub_pattern_event_t* ev, LPARAM lHint) {
	int plugin_id = zzub_pattern_event_get_pluginid(ev);
	int id = zzub_pattern_event_get_id(ev);
	int group = zzub_pattern_event_get_group(ev);
	int track = zzub_pattern_event_get_track(ev);
	int column = zzub_pattern_event_get_column(ev);
	int time = zzub_pattern_event_get_time(ev);

	PE_column* col = editorControl.GetColumn(plugin_id, group, track, column);
	if (col == 0) return;

	if (lHint == zzub_event_type_update_patternevent) {
		PE_column* oldcol = editorControl.GetColumnByEvent(id);
		if (oldcol == 0) return ; // mustve been added already

		if (oldcol != col) {
			// different column, delete old column value
			oldcol->SetValue(id, time, col->novalue, 0);
		}
	}

	int value;
	int meta;
	if (lHint == zzub_event_type_delete_patternevent) {
		value = col->novalue;
		meta = -1;
	} else {
		value = zzub_pattern_event_get_value(ev);
		meta = zzub_pattern_event_get_meta(ev);
	}

	if (plugin_id == hsys_plugin_id)
		UpdateHSys(track, time, value, meta, lHint);

	col->SetValue(id, time, value, meta);

	// notify meta columns about all changes in that plugin
	for (int i = 0; i < editorInner.GetColumnCount(); i++) {
		PE_column& col = *editorInner.columns[i];
		if (col.plugin_id == plugin_id) {
			zzub_parameter_t* colparam = zzub_plugin_get_parameter(zzub_player_get_plugin_by_id(player, col.plugin_id), col.group, 0, col.column);
			if (zzub_parameter_get_type(colparam) & zzub_parameter_type_meta)
				col.SetValue(id, time, value, meta);
		}
	}

	dirtyStatus = true;
	Invalidate(FALSE);
}

void CPatternView::UpdateHSys(int track, int time, int value, int meta, LPARAM lHint) {
	if (lHint == zzub_event_type_insert_patternevent) {
		editorInner.hsys.Add(track, time, (Harmony::Symbols::T)value);
	} else
	if (lHint == zzub_event_type_delete_patternevent) {	
		editorInner.hsys.Remove(track, time);
	} else
	if (lHint == zzub_event_type_update_patternevent) {
		editorInner.hsys.Update(track, time, (Harmony::Symbols::T)value); /// bugged
	}
}

// ---

LRESULT CPatternView::OnNote(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	PE_NMHDR_note* penmh = (PE_NMHDR_note*)pnmh;

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, penmh->plugin_id);
	zzub_plugin_set_parameter_value_direct(plugin, penmh->group, penmh->track, penmh->column, penmh->value, 0);
	zzub_plugin_tick(plugin, 1);

	return 0;
}

LRESULT CPatternView::OnPianoEdit(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	PE_NMHDR_pianoedit* penmh = (PE_NMHDR_pianoedit*)pnmh;
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, penmh->pluginid);
	assert(plugin != 0);

	if (penmh->note == zzub_note_value_none) {
		zzub_pattern_event_t* ev = zzub_player_get_pattern_event_by_id(player, penmh->id);
		assert(ev != 0);
		zzub_pattern_update_note(pattern, ev, penmh->time, zzub_note_value_none, 0);
	} else {
		zzub_pattern_insert_note(pattern, plugin, penmh->time, penmh->note, penmh->length);
	}
	zzub_player_history_commit(player, 0, 0, "Insert Note");
	return 0;
}

LRESULT CPatternView::OnPianoTranslate(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	PE_NMHDR_piano* penmh = (PE_NMHDR_piano*)pnmh;

	if (penmh->eventids.empty()) return 0;

	std::vector<const zzub_pattern_event_t*> events;
	for (std::vector<int>::iterator i = penmh->eventids.begin(); i != penmh->eventids.end(); ++i) {
		zzub_pattern_event_t* ev = zzub_player_get_pattern_event_by_id(player, *i);
		events.push_back(ev);
	}

	zzub_pattern_move_and_transpose_notes(pattern, &events.front(), events.size(), penmh->timeshift, penmh->pitchshift, penmh->mode);

	zzub_player_history_commit(player, 0, 0, "Move&Transpose Notes");

	return 0;
}

// ---

void CPatternView::OnPlayRow() {
	if (InvalidPattern()) return;
	POINT cursor = editorInner.GetCursorAbsolute();
	DoPlayRow(cursor.y);
	editorInner.StepCursor();
}

void CPatternView::OnJazzRow() {
	if (InvalidPattern()) return;
	POINT cursor = editorInner.GetCursorAbsolute();
	DoPlayRow(cursor.y);
}

void CPatternView::DoPlayRow(int time) {
	std::vector<zzub_plugin_t*> plugins;

	for (int i = 0; i < editorInner.GetColumnCount(); ++i) {
		PE_column const& col = editorInner.GetColumn(i);

		if (editorInner.tracks[col.track_index].is_muted)
			continue;

		int value;
		int meta;
		col.GetValue(time, &value, &meta);

		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
		zzub_plugin_set_parameter_value_direct(plugin, col.group, col.track, col.column, value, 0);

		std::vector<zzub_plugin_t*>::iterator plugit = find(plugins.begin(), plugins.end(), plugin);
		if (plugit == plugins.end())
			plugins.push_back(plugin);
	}

	for (std::vector<zzub_plugin_t*>::iterator i = plugins.begin(); i != plugins.end(); ++i) {
		zzub_plugin_tick(*i, 1);
	}
}

LRESULT CPatternView::OnControlHoldRow(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (pattern == 0) return 0;

	int row = (int)wParam;

	DoPlayRow(row);

	return 0;
}

//

void CPatternView::OnPlayTrackRow() {
	if (InvalidPattern()) return;
	PE_cursor_pos pos = editorInner.GetCursorPos();
	DoPlayTrackRow(pos.plugin_id, pos.group, pos.track, pos.row);
	editorInner.StepCursor();
}

void CPatternView::OnJazzTrackRow() {
	if (InvalidPattern()) return;
	PE_cursor_pos pos = editorInner.GetCursorPos();
	DoPlayTrackRow(pos.plugin_id, pos.group, pos.track, pos.row);
}

void CPatternView::DoPlayTrackRow(int plugin_id, int group, int track, int time) {
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, plugin_id);
	PE_track const& pe_track = editorControl.GetTrack(plugin_id, group, track);

	for (int i = pe_track.first_column_idx; i <= pe_track.last_column_idx; ++i) {
		PE_column const& this_col = editorInner.GetColumn(i);

		int value;
		int meta;
		this_col.GetValue(time, &value, &meta);

		//if (value != this_col.novalue)
			zzub_plugin_set_parameter_value_direct(plugin, this_col.group, this_col.track, this_col.column, value, 0);
	}

	zzub_plugin_tick(plugin, 1);
}

// ---

LRESULT CPatternView::OnEdit(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	PE_NMHDR* penmh = (PE_NMHDR*)pnmh;

	PE_column const& col = *editorControl.GetColumn(penmh->plugin_id, penmh->group, penmh->track, penmh->column);
	int row = penmh->row;
	int id = penmh->id;
	int value = penmh->value;
	int meta = penmh->meta;

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);

	if (id == -1 && value != col.novalue) { // do not insert novalue if its inside the minmax range
		zzub_pattern_insert_value(pattern, col.plugin_id, col.group, col.track, col.column, row, value, meta);
	} else {
		if (value == col.novalue) {
			zzub_pattern_delete_value(pattern, id);
		} else {
			zzub_pattern_update_value(pattern, id, row, value, meta);
		}
	}

	if (penmh->reactive) {
		if ((col.type == pattern_column_type_note) && (penmh->digit == 0)) {
			PE_track const& pe_track = editorInner.tracks[col.track_index];

			bool non_note = (value == 255 || value == 254 || value == col.novalue);

			if ((editorInner.notes_affect_waves) && (pe_track.wave_idx != -1)) {
				PE_column const& wave_col = editorInner.GetColumn(pe_track.wave_idx);
				int selwave = GetSelectedWave();
				int wave = (selwave == -1 || non_note) ? wave_col.novalue : selwave;
				zzub_pattern_set_value(pattern, row, plugin, col.group, col.track, wave_col.column, wave, 0);
			}

			if ((editorInner.notes_affect_volumes) && (pe_track.vol_idx != -1)) {
				PE_column const& vol_col = editorInner.GetColumn(pe_track.vol_idx);
				int vol;
				if (non_note) {
					vol = vol_col.novalue;
				} else {
					PE_values_by_time::iterator i = vol_col.values_by_time.upper_bound(row);
					if ((i != vol_col.values_by_time.begin()) && (vol_col.values_by_time.size() != 0)) {
						--i;
						vol = i->value;
					} else {
						vol = vol_col.maxvalue;
					}
				}
				zzub_pattern_set_value(pattern, row, plugin, col.group, col.track, vol_col.column, vol, 0);
			}
		} else
		if (col.flagtype == pattern_column_flagtype_wave) {
			if (value != col.novalue)
				SelectWave(value);
		}
	}

	zzub_player_history_commit(player, 0, 0, "Edit Pattern");

	if ((col.type == pattern_column_type_note) && (penmh->digit == 0) && play_notes) {
		if (value != col.novalue)
			DoPlayTrackRow(col.plugin_id, col.group, col.track, row);
	}

	return 0;
}

void CPatternView::RemoveEdit(int plugin_id, int group, int track, int column, int row) {
	PE_column const& col = *editorControl.GetColumn(plugin_id, group, track, column);

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, plugin_id);
	zzub_pattern_set_value(pattern, row, plugin, group, track, column, col.novalue, 0);

	if (col.type == pattern_column_type_note) {
		PE_track const& pe_track = editorInner.tracks[col.track_index];

		if ((editorInner.notes_affect_waves) && (pe_track.wave_idx != -1)) {
			PE_column const& wave_col = editorInner.GetColumn(pe_track.wave_idx);
			zzub_pattern_set_value(pattern, row, plugin, col.group, col.track, wave_col.column, wave_col.novalue, 0);
		}

		if ((editorInner.notes_affect_volumes) && (pe_track.vol_idx != -1)) {
			PE_column const& vol_col = editorInner.GetColumn(pe_track.vol_idx);
			zzub_pattern_set_value(pattern, row, plugin, col.group, col.track, vol_col.column, vol_col.novalue, 0);
		}
	}
}

// ---

void CPatternView::OnCycleNotesAffect() {
	int mode = editorInner.GetNotesAffectMode();

	mode = (mode + 1) % 4;

	configuration->setNotesAffectMode(mode);

	editorInner.SetNotesAffectMode(mode);

	BindStatus();
}

// ---------------------------------------------------------------------------------------------------------------
// STATUS BAR
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::BindStatus() {
	if (InvalidPattern()) {
		if (pattern == 0)
			statusBar.SetText(0, "No Pattern");
		else
			statusBar.SetText(0, "No Columns");
		statusBar.SetText(1, "");
		statusBar.SetText(2, "");
		statusBar.SetText(3, "");
		statusBar.SetText(4, "");
		return;
	}

	PE_column const& col = editorInner.GetColumnAtCursor();
	int row = editorInner.GetCursorRowAbsolute();

	std::stringstream rowStrm;
	rowStrm << row << " / " << editorInner.pattern_rows;
	if (editorInner.HasSelection()) {
		RECT rcSel;
		editorInner.GetSortedSelectionRectAbsolute(&rcSel);
		rowStrm << " (" << (rcSel.right - rcSel.left + 1) << "x" << (rcSel.bottom - rcSel.top + editorInner.GetSkip()) << ")";
	}

	std::stringstream valueStrm;
	std::string paramDesc;

	if (editorInner.GetColumnCount() > 0) {
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);

		std::string pluginName = zzub_plugin_get_name(plugin);

		int value, meta;
		col.GetValue(row, &value, &meta);

		if (value != col.novalue) {
			if (col.type == zzub_parameter_type_note) {
				valueStrm << noteFromInt(value);
			} else {
				valueStrm << std::setw(4) << std::setfill('0') << std::hex << value;
				if (value < col.minvalue || value > col.maxvalue) {
					valueStrm << " -- Invalid Value!";
				} else {
					const char* desc = zzub_plugin_describe_value(plugin, col.group, col.column, value);
					valueStrm << " (" << std::dec << value << ")";
					if (desc != 0)
						valueStrm << " " << desc;
				}
			}
		}

		zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, col.group, col.track, col.column);
		if (zzub_parameter_get_description(param) != 0)
			paramDesc = pluginName + ": " + zzub_parameter_get_description(param);
		else
			paramDesc = pluginName + ": " + zzub_parameter_get_name(param);
	}

	std::stringstream transposeStrm;
	{
		using namespace Harmony;
		KeySigInfo& info = KeySigInfo::instance();

		{	// disabled set
			if (!editorInner.transpose_set_mode) transposeStrm << "( ";

			bool empty = true;
			for (int i = 0; i < 12; ++i) {
				int meta = editorInner.transpose_set_disabled[i];
				if (meta != -1) {
					MetaInfo const& mi = Harmony::metainfos[i][meta];
					NoteInfo const& ni = Harmony::noteinfos_grouped[mi.natural][mi.sign + 2];
					transposeStrm << ni.name << " ";
					empty = false;
				}
			}

			if (empty) transposeStrm << "empty ";
			if (!editorInner.transpose_set_mode) transposeStrm << ")";
		}

		transposeStrm << "    ";

		{	// enabled set
			if (editorInner.transpose_set_mode) transposeStrm << "( ";

			bool empty = true;
			for (int i = 0; i < 12; ++i) {
				int meta = editorInner.transpose_set_enabled[i];
				if (meta != -1) {
					MetaInfo const& mi = Harmony::metainfos[i][meta];
					NoteInfo const& ni = Harmony::noteinfos_grouped[mi.natural][mi.sign + 2];
					transposeStrm << ni.name << " ";
					empty = false;
				}
			}

			if (empty) transposeStrm << "empty ";
			if (editorInner.transpose_set_mode) transposeStrm << ")";
		}
	}

	// modes
	std::stringstream modesStrm;
	{
		if (editorInner.notes_affect_waves || editorInner.notes_affect_volumes) {
			modesStrm << "Notes affect: ";

			if (editorInner.notes_affect_waves) {
				modesStrm << "Waves";
			}

			if (editorInner.notes_affect_volumes) {
				if (editorInner.notes_affect_waves)
					modesStrm << ", ";
				modesStrm << "Volumes";
			}
		}
	}

	statusBar.SetText(0, rowStrm.str().c_str());
	statusBar.SetText(1, valueStrm.str().c_str());
	statusBar.SetText(2, paramDesc.c_str());
	statusBar.SetText(3, transposeStrm.str().c_str());
	statusBar.SetText(4, modesStrm.str().c_str());
}

// ---------------------------------------------------------------------------------------------------------------
// MIDI
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::HandleMidi(zzub_event_data_midi_message_t& msg)
{
	// TODO: still needs more fixing/upgrading to work with pattern formats!

	// Direct MIDI note entry

	unsigned char data1;
	unsigned char data2;
	unsigned char status;
	static size_t trackIndex = 0;
	static int please_reinit = 0;

	status = msg.status;
	int command = (status & 0xf0) >> 4;
	data1 = msg.data1;
	data2 = msg.data2;

	// Note is being played, record is "on" but song is not playing
	if (command == 0x9 && data1 < 128 && data2 && zzub_player_get_automation(player) && (zzub_player_get_state(player) != zzub_player_state_playing)) {
		POINT cursor = editorInner.GetCursorAbsolute();
		PE_column const& col = editorInner.GetColumnAtCursor();
		int note = midi_to_buzz_note(data1);
		zzub_plugin_t* machine = zzub_player_get_plugin_by_id(player, col.plugin_id);
		zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
		int numTracks = zzub_plugin_get_track_count(machine, 2);
		int note_group = -1;
		int note_column = -1;
		int velocity_column = -1;
		int i;
		static int movedDown = 0;

		// Find closest note column and set the appropriate variable to it (TODO: This could/should be its own function)

		for (note_group = col.group, i = 0; i < zzub_pluginloader_get_parameter_count(loader, col.group); ++i) {
			zzub_parameter_t* paraGlobal = zzub_pluginloader_get_parameter(loader, col.group, i);
			int type = zzub_parameter_get_type(paraGlobal);
			const char *description = zzub_parameter_get_description(paraGlobal);

			//printf("param%i=type:%i, desc:\"%s\"\n", i, type, description);

			if (type == 0 && note_column == -1) // Note
				note_column = i;
			else {
				if (((zzub_parameter_get_flags(paraGlobal) & zzub_parameter_flag_velocity_index)
					|| strstr(description, "elocity")
					|| strstr(description, "olume"))
					&& velocity_column == -1)
				{
					velocity_column = i;
				}
			}
		}
		if (note_group < 2 && note_column == -1) {

			// Not found in previous groups, try track group

			for (note_group = 2, i = 0; i < zzub_pluginloader_get_parameter_count(loader, col.group); ++i) {
				zzub_parameter_t* paraTrack = zzub_pluginloader_get_parameter(loader, col.group, i);
				int type = zzub_parameter_get_type(paraTrack);
				const char *description = zzub_parameter_get_description(paraTrack);

				//printf("param%i=type:%i, desc:\"%s\"\n", i, type, description);

				if (type == 0 && note_column == -1) // Note
					note_column = i;
				else {
					if (((zzub_parameter_get_flags(paraTrack) & zzub_parameter_flag_velocity_index)
						|| strstr(description, "elocity")
						|| strstr(description, "olume"))
						&& velocity_column == -1)
					{
						velocity_column = i;
					}
				}
			}
		}

		//printf("\ncol.group=%i col.track=%i col.column=%i cursor.y=%i note_group=%i note_column=%i \n", col.group, col.track, col.column, cursor.y, note_group, note_column);

		if (note_column != -1) {
			zzub_pattern_format_column_t* notefmtcol = zzub_pattern_format_get_column(patternformat, machine, note_group, col.track, note_column);

			if (notefmtcol != 0 && col.track + trackIndex < numTracks) {
				please_reinit = 1;
				int meta = editorInner.transpose_set_disabled[(note & 0xF) - 1];
				zzub_pattern_set_value(pattern, cursor.y, machine, note_group, col.track + trackIndex, note_column, note, meta);
				if (velocity_column != -1) {
					zzub_pattern_format_column_t* velofmtcol = zzub_pattern_format_get_column(patternformat, machine, note_group, col.track, velocity_column);
					zzub_parameter_t* param = zzub_plugin_get_parameter(machine, note_group, col.track + trackIndex, velocity_column);
					data2 = (data2 * zzub_parameter_get_value_max(param)) / 127; // Set appropriate velocity
					if (velofmtcol != 0 && data2 < zzub_parameter_get_value_max(param)) {
						zzub_pattern_set_value(pattern, cursor.y, machine, note_group, col.track + trackIndex, velocity_column, data2, 0);
					}
				}
				if (note_group == 2) trackIndex++;
			}
		}
	} else
	// Note Off, re-initialize everything
	if (((command == 0x9 && data1 < 128 && !data2) || (command == 0x8)) && zzub_player_get_automation(player) && (zzub_player_get_state(player) != zzub_player_state_playing)) {
		if(please_reinit) {
			please_reinit = 0;
			zzub_player_history_commit(player, 0, 0, "Edit Pattern");
			editorInner.StepCursor();
			trackIndex = 0;
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------
// PATTERN TRANSPORT
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnPatternPlay() {
	if (pattern == 0) return; // Don't use InvalidPattern() here?

	zzub_player_play_pattern(player, pattern, 0, -1);
}

void CPatternView::OnPatternReplay() {
	if (pattern == 0) return; // Don't use InvalidPattern() here?

	int row = editorControl.replay_row;

	if (row == -1) return;

	zzub_player_play_pattern(player, pattern, row, -1);
}

void CPatternView::OnPatternPlayFromCursor() {
	if (pattern == 0) return; // Don't use InvalidPattern() here?

	POINT cursor = editorInner.GetCursorAbsolute();

	if (cursor.y != editorControl.replay_row) {
		zzub_pattern_set_replay_row(pattern, cursor.y);
		zzub_player_history_commit(player, 0, 0, "Set Replay Row");
	}

	zzub_player_play_pattern(player, pattern, cursor.y, -1);
}

void CPatternView::OnPatternStop() {
	zzub_player_set_state(player, zzub_player_state_stopped, -1);
}

LRESULT CPatternView::OnControlPlayFromRow(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (pattern == 0) return 0; // Don't use InvalidPattern() here

	int row = (int)wParam;

	if (row != editorControl.replay_row) {
		zzub_pattern_set_replay_row(pattern, row);
		zzub_player_history_commit(player, 0, 0, "Set Replay Row");
	}

	zzub_player_play_pattern(player, pattern, row, -1);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// PATTERN FORMAT OPERATIONS
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnCreatePatternFormat() {
	zzub_pattern_format_t* format = zzub_player_create_pattern_format(player, 0);
	zzub_player_history_commit(player, 0, 0, "Create Pattern Format");
	SetPatternUser(0, format);
}

void CPatternView::OnClonePatternFormat() {
	if (patternformat == 0) return;

	const char* name = zzub_pattern_format_get_name(patternformat);

	zzub_pattern_format_t* format = zzub_player_clone_pattern_format(player, patternformat, name);
	zzub_player_history_commit(player, 0, 0, "Clone Pattern Format");
	SetPatternUser(0, format);
}

void CPatternView::OnDeletePatternFormat() {
	if (patternformat == 0) return; // Don't use InvalidPattern() here

	zzub_pattern_format_t* format = patternformat;
	zzub_pattern_format_destroy(patternformat);
	zzub_player_history_commit(player, 0, 0, "Delete Pattern Format");
}

void CPatternView::OnPatternFormatProperties() {
	if (patternformat == 0) return; // Don't use InvalidPattern() here

	buze_event_data_t ev;
	ev.show_properties.type = buze_property_type_pattern_format;
	ev.show_properties.pattern_format = patternformat;
	ev.show_properties.return_view = this;
	buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
}

void CPatternView::OnMachineAddTrack() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();

	if (col.group != 2) return;

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
	assert(plugin != 0);

	// add a new track in the current pattern format
	/*int trackcols = editorControl.GetColumnCount(col.plugin_id, col.group, col.track);
	int firsttrackcol = col.index;
	while (firsttrackcol > 0) {
		PE_column const& testcol = editorInner.GetColumn(firsttrackcol - 1);
		if (false
			|| testcol.plugin_id != col.plugin_id
			|| testcol.track != col.track
			|| testcol.group != col.group
		) {
			break;
		}
		firsttrackcol--;
	}

	assert(firsttrackcol != -1);
	*/
	int fmt_trackcount = pattern_format_highest_track_count(patternformat, plugin);

	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(plugin);
	int plug_max_tracks = zzub_pluginloader_get_tracks_max(info);
	if (fmt_trackcount >= plug_max_tracks) return;

	// All OK, do it
	{
		int plug_trackcount = zzub_plugin_get_track_count(plugin, 2);
		if (fmt_trackcount + 1 > plug_trackcount)
			zzub_plugin_set_track_count(plugin, fmt_trackcount + 1);

		for (int i = 0; i < editorInner.GetColumnCount(); i++) {
			PE_column const& testcol = editorInner.GetColumn(i);
			if (testcol.plugin_id == col.plugin_id && testcol.group == col.group && testcol.track == col.track) {
				bool hascol = zzub_pattern_format_get_column(patternformat, plugin, 2, fmt_trackcount, testcol.column) != 0;
				if (!hascol) {
					zzub_pattern_format_add_column(patternformat, plugin, 2, fmt_trackcount, testcol.column, -1);
				}
			}
		}

		zzub_player_history_commit(player, 0, 0, "Add Pattern Format Track");
		//mainframe->SetFocus();	// this line is here because utrk changes focus in buze only
	}
}

void CPatternView::OnMachineRemoveTrack() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();

	if (col.group != 2) return;

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
	assert(plugin != 0);

	int fmt_trackcount = pattern_format_highest_track_count(patternformat, plugin);

	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(plugin);
	int plug_min_tracks = zzub_pluginloader_get_tracks_min(info);

	if (fmt_trackcount <= 1) return;

	// All OK, do it
	{
		// fix cursor/selection
		{
			pattern_position pos = viewInfo->GetPatternPosition(caption, pattern);

			if (pos.track >= (fmt_trackcount - 1))
				pos.track -= 1;

			int select_from_track = pos.select_from.track;
			int select_to_track = pos.select_to.track;
			if (select_from_track >= (fmt_trackcount - 1))
				select_from_track -= 1;
			if (select_to_track >= (fmt_trackcount - 1))
				select_to_track -= 1;

			if ((select_from_track != pos.select_from.track) ^ (select_to_track != pos.select_to.track)) {
				pos.select_from.track = select_from_track;
				pos.select_to.track = select_to_track;
			} else
			if ((select_from_track != pos.select_from.track) && (select_to_track != pos.select_to.track)) {
				pos.select_from.column = pos.select_to.column = -1;
			}

			viewInfo->SetPatternPosition(caption, pattern, pos);
		}

		// remove tracks and format columns -- must come afterwards otherwise new pattern position will be lost
		{
			int plug_trackcount = zzub_plugin_get_track_count(plugin, 2);

/// (_) PE: OnMachineRemoveTrack and AddTrack are bugged because they remove plugin tracks
/// based on the format's track count, but another format could be using the same plugin and
/// still have more tracks for that plugin. Needs to scan every format.
/// document->addPatternTrack(machine); ?
// 			if ((fmt_trackcount-1 < plug_trackcount) && (fmt_trackcount-1 >= plug_min_tracks))
// 				zzub_plugin_set_track_count(plugin, fmt_trackcount - 1);

			for (int i = 0; i < editorInner.GetColumnCount(); i++) {
				PE_column const& testcol = editorInner.GetColumn(i);
				if (testcol.plugin_id == col.plugin_id && testcol.group == col.group && testcol.track == fmt_trackcount - 1) {
					zzub_pattern_format_delete_column(patternformat, plugin, 2, fmt_trackcount-1, testcol.column);
				}
			}

		}

		zzub_player_history_commit(player, 0, 0, "Remove Pattern Format Track");
		//mainframe->SetFocus();	// this line is here because utrk changes focus in buze only
	}
}

// ---------------------------------------------------------------------------------------------------------------
// PATTERN OPERATIONS
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnPatternCreate() {
	if (patternformat == 0) return;

	int len;
	if (pattern != 0)
		len = zzub_pattern_get_row_count(pattern); 
	else
		len = configuration->getGlobalPatternLength();

	char const* description = 0;
	if (configuration->getPatternNamingMode() == 1) {
		if (pattern != 0)
			description = zzub_pattern_get_name(pattern);
		else
			description = zzub_pattern_format_get_name(patternformat);
	}
	zzub_pattern_t* pat = zzub_player_create_pattern(player, patternformat, description, len);

	zzub_player_history_commit(player, 0, 0, "Create Pattern");

	SetPatternUser(pat);
}

void CPatternView::OnPatternDelete() {
	if (InvalidPattern()) return;
	zzub_pattern_destroy(pattern);
	zzub_player_history_commit(player, 0, 0, "Delete Pattern");
}

void CPatternView::OnPatternClone() {
	if (InvalidPattern()) return;

	char const* description = 0;
	if (configuration->getPatternNamingMode() == 1) {
		description = zzub_pattern_get_name(pattern);
	}
	zzub_pattern_t* clonepattern = zzub_player_clone_pattern(player, pattern, description);
	assert(clonepattern);

	zzub_player_history_commit(player, 0, 0, "Clone Pattern");

	SetPatternUser(clonepattern);
}

void CPatternView::OnPatternDoubleRows() {
	if (InvalidPattern()) return;
	int length = zzub_pattern_get_row_count(pattern);
	zzub_pattern_set_row_count(pattern, length * 2);
	zzub_player_history_commit(player, 0, 0, "Double Pattern Rows");
}

void CPatternView::OnPatternHalveRows() {
	if (InvalidPattern()) return;
	int length = zzub_pattern_get_row_count(pattern);
	if (length > 1) {
		zzub_pattern_set_row_count(pattern, length / 2);
		zzub_player_history_commit(player, 0, 0, "Halve Pattern Rows");
	}
}

void CPatternView::OnPatternDoubleLength() {
	if (InvalidPattern()) return;
	zzub_pattern_expand_pattern(pattern, 2);
	zzub_pattern_set_loop_start(pattern, editorInner.loop_begin_pos * 2);
	zzub_pattern_set_loop_end(pattern, editorInner.loop_end_pos * 2);
	zzub_player_history_commit(player, 0, 0, "Double Pattern Length");
}

void CPatternView::OnPatternHalveLength() {
	if (InvalidPattern()) return;
	int length = zzub_pattern_get_row_count(pattern);
	if (length > 1) {
		zzub_pattern_compact_pattern(pattern, 2);
		zzub_pattern_set_loop_start(pattern, editorInner.loop_begin_pos / 2);
		zzub_pattern_set_loop_end(pattern, editorInner.loop_end_pos / 2);
		zzub_player_history_commit(player, 0, 0, "Halve Pattern Length");
	}
}

// ---------------------------------------------------------------------------------------------------------------
// OTHER VIEW COUPLING
// ---------------------------------------------------------------------------------------------------------------

zzub_pattern_t* CPatternView::GetPattern() {
	return this->pattern;
}

void CPatternView::GetHelpText(char* text, int* len) {

	std::string helptext = PeekString(_Module.GetResourceInstance(), IDT_HELP_PATTERNVIEW);
	HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "patternview");
	std::string acceltext = CreateAccelTableString(hAccel);

	helptext += acceltext;
	*len = (int)helptext.length();
	if (text)
		strcpy(text, helptext.c_str());
}

void CPatternView::OnViewProperties() {
	if (pattern == 0) return; // don't use InvalidPattern() here.

	buze_event_data_t ev;
	ev.show_properties.type = buze_property_type_pattern;
	ev.show_properties.pattern = pattern;
	ev.show_properties.return_view = this;
	buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
}

void CPatternView::OnDuplicateEditor() {
	//mainframe->showPatternEditor(true, true, pattern, "");
	buze_event_data_t ev;
	ev.show_pattern.editor_id = 1;
	ev.show_pattern.pattern = pattern;
	ev.show_pattern.reset_stack = true;
	ev.show_pattern.change_pattern = true;
	buze_document_notify_views(document, this, buze_event_type_show_pattern_view, &ev);
}

void CPatternView::OnScrollLinkEditor(WORD wID) {
	int peindex = wID - ID_PATTERNVIEW_SCROLLLINKEDITOR_FIRST;

	assert(false);
	/*DynamicDockTabViewManager<CMainFrame, CView>* patternEditors = GetViewInfo("PatternView");
	CPatternView* pe2 = patternEditors->getView(peindex);
	assert(pe2);
	std::vector<std::vector<CPatternView*> >::iterator i = document->findLinkedPatternEditors(this);
	if (i == document->linkedPatternEditors.end()) {
		document->linkPatternEditors(this, pe2);
	} else {
		document->unlinkPatternEditors(this, pe2);
	}*/
}

void CPatternView::OnShowPatternFormat() {
	buze_event_data ev;
	ev.show_pattern_format.pattern_format = patternformat;
	buze_document_notify_views(document, this, buze_event_type_show_pattern_format_view, &ev);
	//mainframe->showPatternFormatView();
}

void CPatternView::OnShowPatternList() {
	assert(false);
	//mainframe->showPatternListView();
}

// --- Machines ---

void CPatternView::OnMachineParameters() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
	assert(plugin != 0);

	buze_main_frame_show_plugin_parameters(mainframe, plugin, parametermode_default, -1, -1);

	//assert(false);
	/*for (int i = 0; i < mainframe->machineParameters.getViewCount(); ++i) {
		CMachineParameterView* mpv = mainframe->machineParameters.getView(i);
		if (mpv->GetMachine() == plugin) {
			mpv->sliderView.SelectSlider(plugin, col.group, col.track, col.column);
			break;
		}
	}*/
}

void CPatternView::OnMachineProperties() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);

	buze_event_data_t ev;
	ev.show_properties.type = buze_property_type_plugin;
	ev.show_properties.plugin = plugin;
	ev.show_properties.return_view = this;
	buze_document_notify_views(document, this, buze_event_type_show_properties, &ev);
}

void CPatternView::OnParameterInterpolateAbsolute() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
	zzub_plugin_set_parameter_interpolator(plugin, col.group, col.track, col.column, 0);
	zzub_player_history_commit(player, 0, 0, "Set Absolute Column Interpolation");
}

void CPatternView::OnParameterInterpolateInertial() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
	zzub_plugin_set_parameter_interpolator(plugin, col.group, col.track, col.column, 1);
	zzub_player_history_commit(player, 0, 0, "Set Inertial Column Interpolation");
}

void CPatternView::OnParameterInterpolateLinear() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
	zzub_plugin_set_parameter_interpolator(plugin, col.group, col.track, col.column, 2);
	zzub_player_history_commit(player, 0, 0, "Set Linear Column Interpolation");
}


// ---------------------------------------------------------------------------------------------------------------
// COLUMN ACTIONS
// ---------------------------------------------------------------------------------------------------------------

// Spacebar
void CPatternView::OnSpecial1() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, pos.group, pos.track, pos.column);

	int flags = zzub_parameter_get_flags(param);
	if (flags & zzub_parameter_flag_state) {
		PasteValue();
	} else {
		editorInner.DoColumnSpecial1(); // allow column to handle it.
	}
}

// Ctrl+Spacebar
void CPatternView::OnSpecial2() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, pos.group, pos.track, pos.column);

	int flags = zzub_parameter_get_flags(param);
	if (flags & zzub_parameter_flag_state) {
		PasteTrackValues();
	} else {
		editorInner.DoColumnSpecial2(); // allow column to handle it.
	}
}

// Ctrl+Shift+Spacebar
void CPatternView::OnSpecial3() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, pos.group, pos.track, pos.column);

	int flags = zzub_parameter_get_flags(param);
	if (flags & zzub_parameter_flag_state) {
		// Unused so far
	} else {
		editorInner.DoColumnSpecial3(); // allow column to handle it.
	}
}

// Shift+Spacebar
void CPatternView::OnSpecial4() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, pos.group, pos.track, pos.column);

	int flags = zzub_parameter_get_flags(param);
	if (flags & zzub_parameter_flag_state) {
		// Unused so far
	} else {
		editorInner.DoColumnSpecial4(); // allow column to handle it.
	}
}

// Enter
void CPatternView::OnSpecial5() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, pos.group, pos.track, pos.column);

	editorInner.DoColumnSpecial5(); // allow column to handle it.
}

// Shift+Enter
void CPatternView::OnSpecial6() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, pos.group, pos.track, pos.column);

	editorInner.DoColumnSpecial6(); // allow column to handle it.
}

void CPatternView::PasteValue() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	assert(plugin != 0);

	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, pos.group, pos.track, pos.column);
	int value = zzub_plugin_get_parameter_value(plugin, pos.group, pos.track, pos.column);

	if (zzub_parameter_get_value_none(param) != value) {// && document->editPattern(machine, pattern, pos.row, pos.group, pos.track, pos.column, 0, value, 0)) {
		zzub_pattern_set_value(pattern, pos.row, plugin, pos.group, pos.track, pos.column, value, 0);
		zzub_player_history_commit(player, 0, 0, "Paste Current Parameter");
		editorInner.StepCursor();
	}
}

void CPatternView::PasteTrackValues() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	assert(plugin != 0);

	bool yeah = false;
	zzub_pattern_format_column_iterator_t* it = zzub_pattern_format_get_iterator(patternformat);
	while (zzub_pattern_format_column_iterator_valid(it)) {
		zzub_pattern_format_column_t* col = zzub_pattern_format_column_iterator_current(it);
		zzub_plugin_t* colplug = zzub_pattern_format_column_get_plugin(col);
		int group = zzub_pattern_format_column_get_group(col);
		int track = zzub_pattern_format_column_get_track(col);

		if (colplug == plugin && group == pos.group && track == pos.track) {
			int column = zzub_pattern_format_column_get_column(col);
			int value = zzub_plugin_get_parameter_value(plugin, group, track, column);
			zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, track, column);

			if (zzub_parameter_get_value_none(param) != value) {
				zzub_pattern_set_value(pattern, pos.row, plugin, group, track, column, value, 0);
				yeah = true;
			}
		}
		zzub_pattern_format_column_iterator_next(it);
	}
	zzub_pattern_format_column_iterator_destroy(it);

	if (yeah) {
		zzub_player_history_commit(player, 0, 0, "Paste Current Parameters in Track");
		editorInner.StepCursor();
	}

}

void CPatternView::OnCloneTrigger() {
	ClonePattern();
}

void CPatternView::ClonePattern() {
	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int from_col = rcSel.left;
	int to_col = rcSel.right;
	int from_row = rcSel.top;
	int to_row = rcSel.bottom;

	bool did_clones = false;

	for (int i = from_col; i <= to_col; ++i) {
		PE_column const& col = editorInner.GetColumn(i);
		if (col.control != pattern_column_control_pattern) continue;

		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);

		PE_values_by_time::iterator j = col.values_by_time.lower_bound(from_row);
		PE_values_by_time::iterator j_end = col.values_by_time.upper_bound(to_row);
		while (j != j_end) {
			zzub_pattern_t* old_pat = zzub_player_get_pattern_by_id(player, j->value);

			char const* description = 0;
			if (configuration->getPatternNamingMode() == 1) {
				description = zzub_pattern_get_name(old_pat);
			}
			zzub_pattern_t* new_pat = zzub_player_clone_pattern(player, old_pat, description);

			int new_id = zzub_pattern_get_id(new_pat);
			zzub_pattern_set_value(pattern, j->time, plugin, col.group, col.track, col.column, new_id, 0);
			did_clones = true;
			++j;
		}
	}

	if ((from_col == to_col) && (from_row == to_row)) {
		editorInner.StepCursor();
	}

	if (did_clones)
		zzub_player_history_commit(player, 0, 0, "Clone Pattern(s)");
}

void CPatternView::OnHotPastePattern()
{
	HTREEITEM hItem = infoPane.patternTree.GetSelectedItem();
	const PE_treeiteminfo& ti = infoPane.treeItemMapForward[hItem];

	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int from_col = rcSel.left;
	int to_col = rcSel.right;
	int from_row = rcSel.top;
	int to_row = rcSel.bottom;

	if (ti.is_format) {
		int len;
		if ((from_col == to_col) && (from_row == to_row)) {
			len = configuration->getGlobalPatternLength();
		} else {
			len = to_row - from_row + 1;
		}

		for (int j = from_col; j <= to_col; ++j) {
			PE_column const& col = editorInner.GetColumn(j);
			if (col.control != pattern_column_control_pattern) continue;

			char const* description = 0;
			if (configuration->getPatternNamingMode() == 1) {
				description = zzub_pattern_format_get_name(ti.format);
			}
			zzub_pattern_t* newpat = zzub_player_create_pattern(player, ti.format, description, len);

			int value = zzub_pattern_get_id(newpat);
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);

			zzub_pattern_set_value(pattern, from_row, plugin, col.group, col.track, col.column, value, 0);
		}

		if ((from_col == to_col) && (from_row == to_row)) {
			editorInner.StepCursor();
		}

		zzub_player_history_commit(player, 0, 0, "HotCreate Pattern(s)");
	} else {
		int value = zzub_pattern_get_id(ti.pattern);
		int len = zzub_pattern_get_row_count(ti.pattern);

		for (int j = from_col; j <= to_col; ++j) {
			PE_column const& col = editorInner.GetColumn(j);
			if (col.control != pattern_column_control_pattern) continue;

			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);

			for (int i = from_row; i <= to_row; i += len) {
				zzub_pattern_set_value(pattern, i, plugin, col.group, col.track, col.column, value, 0);
			}
		}

		if ((from_col == to_col) && (from_row == to_row)) {
			editorInner.StepCursor();
		}

		zzub_player_history_commit(player, 0, 0, "HotPaste Pattern(s)");
	}
}

void CPatternView::OnPickupValue() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();
	int row = editorInner.GetCursorRowAbsolute();

	int value;
	int meta;
	col.GetValue(row, &value, &meta);

	if (col.defaultcontrol == pattern_column_control_pattern) {
		zzub_pattern_t* pat = zzub_player_get_pattern_by_id(player, value);
		if (pat != NULL) {
			infoPane.PatternTreeMakePatternVisible(pat);
		}
	} else
	if (col.flagtype & pattern_column_flagtype_wave) {
		SelectWave(value);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// INFOPANE
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnPatternTreeExpandCollapse() {
	if (!show_infopane) return;

	HTREEITEM hCurrentItem = infoPane.patternTree.GetSelectedItem();
	if (infoPane.patternTree.GetIndentLevel(hCurrentItem) == 1) {
		hCurrentItem = infoPane.patternTree.GetParentItem(hCurrentItem);
	}

	infoPane.SetRedraw(FALSE);
	{
		HTREEITEM hItemRoot = infoPane.patternTree.GetRootItem();
		HTREEITEM hItem = hItemRoot;

		if (hItem) {
			do {
				if (hItem != hCurrentItem) {
					infoPane.patternTree.CollapseBranch(hItem);
				}
			} while ((hItem = infoPane.patternTree.GetNextSiblingItem(hItem)) != NULL);
		}

		if (infoPane.patternTree.IsExpanded(hCurrentItem) == false) {
			infoPane.patternTree.ExpandBranch(hCurrentItem);
		}
	}
	infoPane.SetRedraw(TRUE);
	infoPane.RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);
	//infoPane.patternTree.UpdateWindow();
}

void CPatternView::OnInfoPaneUp() {
	if (!show_infopane) return;
	if (infoPane.currentWnd == false) return;

	infoPane.patternTree.SelectPrev();
}

void CPatternView::OnInfoPaneDown() {
	if (!show_infopane) return;
	if (infoPane.currentWnd == false) return;

	infoPane.patternTree.SelectNext();
}

void CPatternView::OnInfoPanePageUp() {
	if (!show_infopane) return;
	if (infoPane.currentWnd == false) return;

	for (int i = 0; i < 10; i++) {
		infoPane.patternTree.SelectPrev();
	}
}

void CPatternView::OnInfoPanePageDown() {
	if (!show_infopane) return;
	if (infoPane.currentWnd == false) return;

	for (int i = 0; i < 10; i++) {
		infoPane.patternTree.SelectNext();
	}
}

void CPatternView::BindPatternTree() {
	infoPane.RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);
}

void CPatternView::CheckColumnInfoVisibility() {
	if (show_infopane == BST_INDETERMINATE) {
		bool info_shown = (infoSplitter.GetSinglePaneMode() == SPLIT_PANE_NONE);
		bool show_info = false;

		if (!InvalidPattern()) {
			PE_column const& col = editorInner.GetColumnAtCursor();

			if (col.control == pattern_column_control_pattern)
				show_info = true;
			else
				show_info = false;
		}

		if (show_info && !info_shown)
			infoSplitter.SetSinglePaneMode(SPLIT_PANE_NONE);
		else
		if (!show_info && info_shown)
			infoSplitter.SetSinglePaneMode(SPLIT_PANE_LEFT);
	}
}

void CPatternView::BindColumnInfo() {
	if (InvalidPattern()) {
		infoPane.columnList.DeleteAllItems();
		return;
	}

	PE_column const& col = editorInner.GetColumnAtCursor();

	if (col.control == pattern_column_control_pattern) {
		infoPane.SwitchWindow(true);
	} else {
		infoPane.SwitchWindow(false);
		infoPane.SetRedraw(FALSE);
		{
			std::vector<PE_valueinfo> values;
			GetRangeValueInfos(col, values);
			std::vector<int> filterValues;
			infoPane.columnList.DeleteAllItems();
			for (std::vector<PE_valueinfo>::iterator i = values.begin(); i != values.end(); ++i) {
				int index = (int)(i - values.begin());
				infoPane.columnList.InsertItem(index, i->key.c_str());
				infoPane.columnList.SetItem(index, 1, LVIF_TEXT, i->description.c_str(), -1, 0, 0, 0);
				filterValues.push_back(i->value);
			}
			col.editor->SetFilteredValues(filterValues);
		}
		infoPane.SetRedraw(TRUE);
		infoPane.RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);
	}
}

void CPatternView::GetRangeValueInfos(PE_column const& col, std::vector<PE_valueinfo>& values) {
	int range1 = (col.maxvalue - col.minvalue) + 1;

	bool do_range = false;

	if (range1 > 255 || col.type == pattern_column_type_note) {
		do_range = true;
	} else {
		//std::vector<char> bytes(64);//(1024); trying to speed this function up slightly
		// otherwise strncpy in describe_value will pad this buffer with 1000 zeroes every time

		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);

		for (int i = 0; i < range1; ++i) {
			const char* desc = zzub_plugin_describe_value(plugin, col.group, col.column, col.minvalue + i);
			if (desc == 0 || desc[0] == (char)0) continue;

			PE_valueinfo vi;
			vi.value = col.minvalue + i;
			vi.key = CColumnEditor::FormatValue(col.minvalue + i, 0, col.type, col.novalue);
			vi.description = desc;
			values.push_back(vi);
		}

		if (values.empty())
			do_range = true;
	}

	if (do_range) {
		PE_valueinfo vi;
		vi.value = col.minvalue;
		vi.key = CColumnEditor::FormatValue(col.minvalue, 0, col.type, col.novalue);
		vi.description = "Min";
		values.push_back(vi);

		vi.value = col.maxvalue;
		vi.key = CColumnEditor::FormatValue(col.maxvalue, 0, col.type, col.novalue);
		vi.description = "Max";
		values.push_back(vi);
	}
}

// void CPatternView::BindColumnInfo() {
// 	if (pattern == 0) return;
//  if (editorInner.GetColumnCount() == 0) return;
// 
// 	POINT cursor = editorInner.GetCursorAbsolute();
// 
//  PE_column const& col = editorInner.GetColumnAtCursor();
// 
// 	columnInfoList.SetRedraw(FALSE);
// 	columnInfoList.DeleteAllItems();
// 
// 	std::vector<PE_valueinfo> values;
// 	GetValueInfos(col, values);
// 
// 	std::vector<int> filterValues;
// 	for (std::vector<PE_valueinfo>::iterator i = values.begin(); i != values.end(); ++i) {
// 		int index = (int)(i - values.begin());
// 		columnInfoList.InsertItem(index, i->key.c_str());
// 		columnInfoList.SetItem(index, 1, LVIF_TEXT, i->description.c_str(), -1, 0, 0, 0);
// 		filterValues.push_back(i->value);
// 	}
// 
// 	columnInfoList.SetRedraw(TRUE);
// 	col.editor->SetFilteredValues(filterValues);
// }

void CPatternView::GetValueInfos(PE_column const& col, std::vector<PE_valueinfo>& values) {
// 	if (col.control == pattern_column_control_pattern) {
// 		GetPatternValueInfos(col, values);
// 	} else {
// 		GetRangeValueInfos(col, values);
// 	}
}

void CPatternView::GetPatternValueInfos(PE_column const& col, std::vector<PE_valueinfo>& values) {
// 	std::vector<zzub_pattern_format_t*> formatFilters;
// 	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
// 	getFormatFilters(patternformat, plugin, col.group, col.track, col.column, formatFilters);
// 
// 	std::vector<char> bytes(1024);
// 
// 	const char* keys = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
// 	char pckey[8];
// 	char pclen[8];
// 	memset(pckey, 0, 8);
// 	memset(pclen, 0, 8);
// 	int index = 0;
// 	for (size_t i = 0; i < zzub_player_get_pattern_count(player); ++i) {
// 		zzub_pattern_t* pat = zzub_player_get_pattern_by_index(player, i);
// 		zzub_pattern_format_t* fmt = zzub_pattern_get_format(pat);
// 
// 		std::vector<zzub_pattern_format_t*>::iterator it = find(formatFilters.begin(), formatFilters.end(), fmt);
// 		if (it == formatFilters.end() && formatFilters.size() > 0) continue;
// 
// 		int patid = zzub_pattern_get_id(pat);
// 		zzub_pattern_get_name(pat, &bytes.front(), bytes.size());
// 		if (index < strlen(keys)) {
// 			pckey[0] = *(keys+index);
// 		} else {
// 			pckey[0] = 32;
// 		}
// 
// 		PE_valueinfo vi;
// 		vi.value = patid;
// 		vi.key = pckey;
// 		vi.description = &bytes.front();
// 		values.push_back(vi);
// 		index++;
// 	}
}

// void getFormatFilters(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, int column, std::vector<zzub_pattern_format_t*>& formatFilters) {
// 	zzub_pattern_format_iterator_t* it = zzub_pattern_format_get_column_filters(patternformat, plugin, group, track, column);
// 	while (zzub_pattern_format_iterator_valid(it)) {
// 		zzub_pattern_format_t* ffmt = zzub_pattern_format_iterator_current(it);
// 		formatFilters.push_back(ffmt);
// 		zzub_pattern_format_iterator_next(it);
// 	}
// 	zzub_pattern_format_iterator_destroy(it);
// }

void CPatternView::OnToggleFilter(WORD wID) {
// 	int formatindex = wID - ID_PATTERNEDITOR_FILTER_FIRST;
// 	zzub_pattern_format_t* toggleformat = zzub_player_get_pattern_format_by_index(player, formatindex);
// 
//  PE_column const& col = editorInner.GetColumnAtCursor();
// 
// 	std::vector<zzub_pattern_format_t*> formatFilters;
// 	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
// 	getFormatFilters(patternformat, plugin, col.group, col.track, col.column, formatFilters);
// 
// 	std::vector<zzub_pattern_format_t*>::iterator it = find(formatFilters.begin(), formatFilters.end(), toggleformat);
// 
// 	if (it == formatFilters.end()) {
// 		zzub_pattern_format_add_column_filter(patternformat, plugin, col.group, col.track, col.column, toggleformat);
// 	} else {
// 		zzub_pattern_format_remove_column_filter(patternformat, plugin, col.group, col.track, col.column, toggleformat);
// 	}
// 	zzub_player_history_commit(player, 0, 0, "Set Sequence Column Filter");
// 
 	return;
}

LRESULT CPatternView::OnRClickColumnInfo(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
// 	NMITEMACTIVATE* nmia = (NMITEMACTIVATE*)pnmh;
// 
// 	PE_column const& col = editorInner.GetColumnAtCursor();
// 
// 	CMenu menu; 
// 	menu.CreatePopupMenu();
// 	menu.AppendMenu(MF_STRING|MF_DISABLED, (UINT_PTR)0, "Filter Column List By:");
// 
// 	std::vector<zzub_pattern_format_t*> formatFilters;
// 	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
// 	getFormatFilters(patternformat, plugin, col.group, col.track, col.column, formatFilters);
// 
// 	zzub_pattern_format_iterator_t* it = zzub_player_get_pattern_format_iterator(player);
// 	int i = 0;
// 	while (zzub_pattern_format_iterator_valid(it)) {
// 		zzub_pattern_format_t* format = zzub_pattern_format_iterator_current(it);
// 		char name[1024];
// 		zzub_pattern_format_get_name(format, name, 1024);
// 
// 		std::vector<zzub_pattern_format_t*>::iterator filtit = find(formatFilters.begin(), formatFilters.end(), format);
// 		int checked = filtit != formatFilters.end() ? MF_CHECKED : 0;
// 
// 		menu.AppendMenu(MF_STRING|checked, (UINT_PTR)ID_PATTERNEDITOR_FILTER_FIRST+i, name);
// 
// 		zzub_pattern_format_iterator_next(it);
// 		++i;
// 	}
// 	zzub_pattern_format_iterator_destroy(it);
// 
// 	POINT pt = nmia->ptAction;
// 	columnInfoList.ClientToScreen(&pt);
// 	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);
// 
 	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// TRANSPOSE MASK / TRANSPOSE SET
// ---------------------------------------------------------------------------------------------------------------

// lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
/// MAKEWPARAM = l,h
/// wNotifyCode HIWORD(wParam)
/// wID         LOWORD(wParam)
/// hWndCtl     lParam

LRESULT CPatternView::OnNoteMaskToggle(WORD wNotifyCode, WORD /*wID*/, HWND hWndCtl) {
	bool state = (bool)wNotifyCode;
	int note = LOWORD(hWndCtl);
	int meta = HIWORD(hWndCtl);
	editorInner.MaskNote(note, meta, state);
	editorInner.SetEventHoles();
	editorInner.InvalidateSelection(0);
	return 0;
}

LRESULT CPatternView::OnNoteMaskSolo(WORD wNotifyCode, WORD /*wID*/, HWND hWndCtl) {
	int note = LOWORD(hWndCtl);
	int meta = HIWORD(hWndCtl);
	editorInner.MaskNoteReset(false);
	editorInner.MaskNote(note, meta, true);
	editorInner.SetEventHoles();
	editorInner.InvalidateSelection(0);
	return 0;
}

LRESULT CPatternView::OnNoteMaskReset(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/) {
	bool state = (bool)wNotifyCode;
	editorInner.MaskNoteReset(state);
	editorInner.SetEventHoles();
	editorInner.InvalidateSelection(0);
	return 0;
}

void CPatternView::OnShowNoteMaskDialog() {
	if (!notemaskDlg.m_hWnd) {
		CPaletteWindowInitParam initParam;
		initParam.mainfrmWnd = (HWND)buze_main_frame_get_wnd(mainframe);
		initParam.parentWnd = m_hWnd;
		notemaskDlg.Create((HWND)buze_main_frame_get_wnd(mainframe), (LPARAM)&initParam);
		std::stringstream s;
		s << " Transpose Mask (" << caption << ")";
		notemaskDlg.SetWindowText(s.str().c_str());
		notemaskDlg.SetNoteMask(editorInner.transpose_mask);
	}
	notemaskDlg.ShowWindow(SW_SHOWNOACTIVATE);
}

void CPatternView::OnShowHarmonicTransposeDialog() {
	if (!harmonicxposeDlg.m_hWnd) {
		CPaletteWindowInitParam initParam;
		initParam.mainfrmWnd = (HWND)buze_main_frame_get_wnd(mainframe);
		initParam.parentWnd = m_hWnd;
		harmonicxposeDlg.Create((HWND)buze_main_frame_get_wnd(mainframe), (LPARAM)&initParam);
		std::stringstream s;
		s << " Transpose Set (" << caption << ")";
		harmonicxposeDlg.SetWindowText(s.str().c_str());
		harmonicxposeDlg.SetTransposeSet(editorInner.transpose_set_enabled, true);
		harmonicxposeDlg.SetTransposeSet(editorInner.transpose_set_disabled, false);
		harmonicxposeDlg.SetEnabled(editorInner.transpose_set_mode);
	}
	harmonicxposeDlg.ShowWindow(SW_SHOWNOACTIVATE);
}

void CPatternView::OnWindowPosChanged(LPWINDOWPOS lpWndPos) {
	if (lpWndPos->flags & SWP_HIDEWINDOW) {
		if (notemaskDlg.m_hWnd != NULL)
			notemaskDlg.SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		if (harmonicxposeDlg.m_hWnd != NULL)
			harmonicxposeDlg.SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	} else
	if (lpWndPos->flags & SWP_SHOWWINDOW) {
		if (notemaskDlg.m_hWnd != NULL)
			notemaskDlg.SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
		if (harmonicxposeDlg.m_hWnd != NULL)
			harmonicxposeDlg.SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	}
	SetMsgHandled(FALSE);
}

void CPatternView::OnToggleNoteMeta() {
	POINT ptCursor = editorInner.GetCursorAbsolute();
	PE_column const& col = editorInner.GetColumnAtCursor();

	int value;
	int meta;
	int evid = zzub_pattern_get_value(pattern, ptCursor.y, col.plugin_id, col.group, col.track, col.column, &value, &meta);

	if (evid != -1) {
		meta = (meta + 1) % 3;
		int notebase = (value & 0xF) - 1;
		if (notebase == 8 && meta == 1) meta = 2; // Ab/G# only has 2 metas
		zzub_pattern_update_value(pattern, evid, ptCursor.y, value, meta);

		zzub_player_history_commit(player, 0, 0, "Toggle Note Meta");
	}
}

LRESULT CPatternView::OnTransposeSetUpdate(WORD wNotifyCode, WORD wID, HWND hWndCtl) {
	int notebase = LOWORD(hWndCtl);
	int meta = (signed short)HIWORD(hWndCtl);

	if (editorInner.transpose_set_mode)
		editorInner.transpose_set_enabled[notebase] = meta;
	else
		editorInner.transpose_set_disabled[notebase] = meta;

	BindStatus();

	return 0;
}

void CPatternView::OnToggleTransposeSet() {
	editorInner.transpose_set_mode = !editorInner.transpose_set_mode;

	if (harmonicxposeDlg.m_hWnd)
		harmonicxposeDlg.SetEnabled(editorInner.transpose_set_mode);

	BindStatus();
}

LRESULT CPatternView::OnTransposeSetEnable(WORD wNotifyCode, WORD wID, HWND hWndCtl) {
	bool enabled = (bool)wNotifyCode;

	editorInner.transpose_set_mode = enabled;

	BindStatus();

	return 0;
}

void CPatternView::OnTransposeRekey() {
	if (InvalidPattern()) return;
	TransposeSelection(0, true);
	zzub_player_history_commit(player, 0, 0, "Transpose Rekey");
}

void CPatternView::TransposeSelection(int delta, bool notesOnly) {
	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	std::vector<int> hole_ids;
	event_holes_t& event_holes = editorInner.event_holes;
	for (event_holes_t::iterator i = event_holes.begin(); i != event_holes.end(); ++i) {
		hole_ids.push_back(*i);
	}

	int* holes_v = hole_ids.empty() ? 0 : &hole_ids[0];
	int* metas_v;
	if (editorInner.transpose_set_mode)
		metas_v = &editorInner.transpose_set_enabled[0];
	else
		metas_v = &editorInner.transpose_set_disabled[0];

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		if (notesOnly && (col.type != pattern_column_type_note)) continue;
		zzub_pattern_transpose_events(
			pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height,
			delta, holes_v, hole_ids.size(), &metas_v[0], 12, !editorInner.transpose_set_mode);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// SELECTION MUTATIONS / OPERATIONS
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnTransposeSelectionUp() {
	if (InvalidPattern()) return;
	TransposeSelection(1, false);
	zzub_player_history_commit(player, 0, 0, "Transpose Values Up");
}

void CPatternView::OnTransposeSelectionDown() {
	if (InvalidPattern()) return;
	TransposeSelection(-1, false);
	zzub_player_history_commit(player, 0, 0, "Transpose Values Down");
}

void CPatternView::OnTransposeNotesUp() {
	if (InvalidPattern()) return;
	TransposeSelection(1, true);
	zzub_player_history_commit(player, 0, 0, "Transpose Notes Up");
}

void CPatternView::OnTransposeNotesDown() {
	if (InvalidPattern()) return;
	TransposeSelection(-1, true);
	zzub_player_history_commit(player, 0, 0, "Transpose Notes Down");
}

void CPatternView::OnTransposeNotesOctaveUp() {
	if (InvalidPattern()) return;
	TransposeSelection(12, true);
	zzub_player_history_commit(player, 0, 0, "Transpose Notes Octave Up");
}

void CPatternView::OnTransposeNotesOctaveDown() {
	if (InvalidPattern()) return;
	TransposeSelection(-12, true);
	zzub_player_history_commit(player, 0, 0, "Transpose Notes Octave Down");
}

// This limits mutations to certain types.
namespace {
enum {
	t_note = 0x01,
	t_vals = 0x02,
	t_ampl = 0x04,
	t_wave = 0x08,
	t_trig = 0x10,
	t_char = 0x20,
	t_harm = 0x40,
};
bool isColType(PE_column const& col, int x) {
	if (x & t_note)
		if (col.type == pattern_column_type_note)
			return true;
	if (x & t_ampl)
		if (col.flagtype == pattern_column_flagtype_volume)
			return true;
	if (x & t_wave)
		if (col.flagtype == pattern_column_flagtype_wave)
			return true;
	if (x & t_trig)
		if (col.defaultcontrol == pattern_column_control_pattern)
			return true;
	if (x & t_char)
		if (col.control == pattern_column_control_char)
			return true;
	if (x & t_harm)
		if (col.control == pattern_column_control_harmonic)
			return true;
	if (x & t_vals) {
		if (col.type == pattern_column_type_byte)
			return true;
		if (col.type == pattern_column_type_word)
			return true;
		if (col.type == pattern_column_type_switch)
			return true;
	}
	return false;
}
}

void CPatternView::OnVolMaskToggle() {
	volume_masked = !volume_masked;
}

void CPatternView::OnInvertChordUpSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	bool direction = 1;
	bool mode = 0;
	zzub_pattern_invert_chord_events(pattern, rcSel.left, rcSel.right, rcSel.top, height, direction, mode);

	zzub_player_history_commit(player, 0, 0, "Invert Chord Up Selection");
}

void CPatternView::OnInvertChordDownSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	bool direction = 0;
	bool mode = 0;
	zzub_pattern_invert_chord_events(pattern, rcSel.left, rcSel.right, rcSel.top, height, direction, mode);

	zzub_player_history_commit(player, 0, 0, "Invert Chord Down Selection");
}

void CPatternView::OnRandomizeSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		if (isColType(col, t_wave|t_trig|t_char|t_harm)) continue;
		zzub_pattern_randomize_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, editorInner.GetSkip());
	}

	zzub_player_history_commit(player, 0, 0, "Randomize Selection");
}

void CPatternView::OnRandomizeRangeSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	int tolast_row = rcSel.bottom - (editorInner.GetSkip() - 1);

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		if (isColType(col, t_wave|t_trig|t_char|t_harm)) continue;

		int from_val;
		int from_meta;
		col.GetValue(rcSel.top, &from_val, &from_meta);
		int to_val;
		int to_meta;
		col.GetValue(tolast_row, &to_val, &to_meta);

		if ((from_val != col.novalue) && (to_val != col.novalue))
			zzub_pattern_randomize_range_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, from_val, to_val, editorInner.GetSkip());
	}

	zzub_player_history_commit(player, 0, 0, "Randomize Range Selection");
}

void CPatternView::OnRandomizeFromSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_randomize_from_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, editorInner.GetSkip());
	}

	zzub_player_history_commit(player, 0, 0, "Randomize From Selection");
}

void CPatternView::OnHumanizeSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	const int deviation = 20;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		if (volume_masked && !isColType(col, t_ampl)) continue;
		if (isColType(col, t_note|t_wave|t_trig|t_char|t_harm)) continue;
		zzub_pattern_humanize_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, deviation);
	}

	zzub_player_history_commit(player, 0, 0, "Humanize Selection");
}

void CPatternView::OnShuffleSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_shuffle_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height);
	}

	zzub_player_history_commit(player, 0, 0, "Shuffle Selection");
}

void CPatternView::OnInterpolateSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		if (isColType(col, t_trig|t_char|t_harm)) continue;

		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);

		zzub_pattern_interpolate_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, editorInner.GetSkip());
	}

	zzub_player_history_commit(player, 0, 0, "Interpolate Selection");
}

void CPatternView::OnGradiateSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		if (isColType(col, t_trig|t_char|t_harm)) continue;
		zzub_pattern_gradiate_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, editorInner.GetSkip());
	}

	zzub_player_history_commit(player, 0, 0, "Gradiate Selection");
}

void CPatternView::OnSmoothSelection() {
	///
// 	if (volume_masked && !isColType(col, t_ampl)) continue;
}

void CPatternView::OnReverseSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_reverse_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height);
	}

	zzub_player_history_commit(player, 0, 0, "Reverse Selection");
}

void CPatternView::OnCompactSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	const int factor = 2;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_compact_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, factor);
	}

	editorInner.SelectRangeAbsolute(rcSel.left, rcSel.top, rcSel.right, rcSel.top + (height / factor) - 1);

	zzub_player_history_commit(player, 0, 0, "Compact Selection");
}

void CPatternView::OnExpandSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	const int factor = 2;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_expand_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, factor);
	}

	editorInner.SelectRangeAbsolute(rcSel.left, rcSel.top, rcSel.right, rcSel.top + (height * factor) - 1);

	zzub_player_history_commit(player, 0, 0, "Expand Selection");
}

void CPatternView::OnThinSelection() {
	if (InvalidPattern()) return;
	thinDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	thinDlg.ShowWindow(SW_SHOWNORMAL); 
}
void CPatternView::OnThinOK() {
	int x;
	bool valid = true;
	valid &= thinDlg.m_edit.GetInt(x);
	valid &= (x >= 2);
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			// isColType -> allows all
			zzub_pattern_thin_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
		}

		zzub_player_history_commit(player, 0, 0, "Thin Selection");
	}
}

void CPatternView::OnEchoSelection() {
	if (InvalidPattern()) return;
	echoDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	echoDlg.ShowWindow(SW_SHOWNORMAL); 
}
void CPatternView::OnEchoOK() {
	int x;
	bool valid = true;
	valid &= echoDlg.m_edit.GetInt(x);
	valid &= (x >= 1);
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			// isColType -> allows all
			zzub_pattern_echo_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
		}

		zzub_player_history_commit(player, 0, 0, "Echo Selection");
	}
}

void CPatternView::OnRepeatSelection() {
	if (InvalidPattern()) return;
	repeatDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	repeatDlg.ShowWindow(SW_SHOWNORMAL); 
}
void CPatternView::OnRepeatOK() {
	int x;
	bool valid = true;
	valid &= repeatDlg.m_edit.GetInt(x);
	valid &= (x >= 1);
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			// isColType -> allows all
			zzub_pattern_repeat_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
		}

		zzub_player_history_commit(player, 0, 0, "Repeat Selection");
	}
}

void CPatternView::OnUniqueSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_unique_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height);
	}

	zzub_player_history_commit(player, 0, 0, "Unique Selection");
}

void CPatternView::OnScaleSelection() {
	if (InvalidPattern()) return;
	scaleDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	scaleDlg.m_edit1.SetWindowText("0.0");
	scaleDlg.m_edit2.SetWindowText("1.0");
	scaleDlg.m_edit3.SetWindowText("0.0");
	scaleDlg.m_edit4.SetWindowText("1.0");
	scaleDlg.ShowWindow(SW_SHOWNORMAL); 
}
void CPatternView::OnScaleOK() {
	double in_low;
	double in_high;
	double out_low;
	double out_high;
	bool valid = true;
	valid &= scaleDlg.m_edit1.GetDouble(in_low);
	valid &= scaleDlg.m_edit2.GetDouble(in_high);
	valid &= scaleDlg.m_edit3.GetDouble(out_low);
	valid &= scaleDlg.m_edit4.GetDouble(out_high);
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			if (volume_masked && !isColType(col, t_ampl)) continue;
			if (isColType(col, t_note|t_wave|t_trig|t_char|t_harm)) continue;
			zzub_pattern_scale_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, in_low, in_high, out_low, out_high);
		}

		zzub_player_history_commit(player, 0, 0, "Scale Selection");
	}
}

void CPatternView::OnFadeSelection() {
	if (InvalidPattern()) return;
	fadeDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	fadeDlg.m_edit1.SetWindowText("1.0");
	fadeDlg.m_edit2.SetWindowText("0.0");
	fadeDlg.ShowWindow(SW_SHOWNORMAL); 
}
void CPatternView::OnFadeOK() {
	double from;
	double to;
	bool valid = true;
	valid &= fadeDlg.m_edit1.GetDouble(from);
	valid &= fadeDlg.m_edit2.GetDouble(to);
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			if (volume_masked && !isColType(col, t_ampl)) continue;
			if (isColType(col, t_note|t_wave|t_trig|t_char|t_harm)) continue;
			zzub_pattern_fade_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, from, to);
		}

		zzub_player_history_commit(player, 0, 0, "Fade Selection");
	}
}

void CPatternView::OnCurveMapSelection() {
	if (InvalidPattern()) return;
	curvemapDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	curvemapDlg.ShowWindow(SW_SHOWNORMAL);
}
void CPatternView::OnCurveMapOK() {
	int x;
	bool valid = true;
	int mode = curvemapDlg.getCheckedRadio();
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			if (volume_masked && !isColType(col, t_ampl)) continue;
			if (isColType(col, t_note|t_wave|t_trig|t_char|t_harm)) continue;
			zzub_pattern_curvemap_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, mode);
		}

		zzub_player_history_commit(player, 0, 0, "Curvemap Selection");
	}
}

void CPatternView::OnInvertSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		if (volume_masked && !isColType(col, t_ampl)) continue;
		if (isColType(col, t_note|t_wave|t_trig|t_char|t_harm)) continue;
		zzub_pattern_invert_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height);
	}

	zzub_player_history_commit(player, 0, 0, "Invert Selection");
}

void CPatternView::OnRotateRowsSelectionUp() {
	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;
	int x = -1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_rotate_rows_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
	}

	zzub_player_history_commit(player, 0, 0, "Rotate Rows Up Selection");
}
void CPatternView::OnRotateRowsSelectionDown() {
	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;
	int x = 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_rotate_rows_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
	}

	zzub_player_history_commit(player, 0, 0, "Rotate Rows Up Selection");
}
void CPatternView::OnRotateRowsSelection() {
	if (InvalidPattern()) return;
	rotaterowsDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	rotaterowsDlg.ShowWindow(SW_SHOWNORMAL); 
}
void CPatternView::OnRotateRowsOK() {
	int x;
	bool valid = true;
	valid &= rotaterowsDlg.m_edit.GetInt(x);
	valid &= (x != 0);
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			// isColType -> allows all
			zzub_pattern_rotate_rows_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
		}

		zzub_player_history_commit(player, 0, 0, "Rotate Rows Selection");
	}
}

void CPatternView::OnRotateValsSelectionUp() {
	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;
	int x = -1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_rotate_vals_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
	}

	zzub_player_history_commit(player, 0, 0, "Rotate Values Up Selection");
}
void CPatternView::OnRotateValsSelectionDown() {
	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;
	int x = 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_rotate_vals_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
	}

	zzub_player_history_commit(player, 0, 0, "Rotate Values Down Selection");
}
void CPatternView::OnRotateValsSelection() {
	if (InvalidPattern()) return;
	rotatevalsDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	rotatevalsDlg.ShowWindow(SW_SHOWNORMAL); 
}
void CPatternView::OnRotateValsOK() {
	int x;
	bool valid = true;
	valid &= rotatevalsDlg.m_edit.GetInt(x);
	valid &= (x != 0);
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			// isColType -> allows all
			zzub_pattern_rotate_vals_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
		}

		zzub_player_history_commit(player, 0, 0, "Rotate Values Selection");
	}
}

void CPatternView::OnRotateRhythmsSelectionUp() {
	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;
	int x = -1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_rotate_dist_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
	}

	zzub_player_history_commit(player, 0, 0, "Rotate Rhythms Up Selection");
}
void CPatternView::OnRotateRhythmsSelectionDown() {
	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;
	int x = 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		zzub_pattern_rotate_dist_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
	}

	zzub_player_history_commit(player, 0, 0, "Rotate Rhythms Down Selection");
}
void CPatternView::OnRotateRhythmsSelection() {
	if (InvalidPattern()) return;
	rotaterhythmsDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	rotaterhythmsDlg.ShowWindow(SW_SHOWNORMAL); 
}
void CPatternView::OnRotateRhythmsOK() {
	int x;
	bool valid = true;
	valid &= rotaterhythmsDlg.m_edit.GetInt(x);
	valid &= (x != 0);
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			// isColType -> allows all
			zzub_pattern_rotate_dist_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x);
		}

		zzub_player_history_commit(player, 0, 0, "Rotate Rhythms Selection");
	}
}

void CPatternView::OnAllToFirstSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	PE_column const& first_col = editorInner.GetColumn(rcSel.left);
	int from_value;
	int from_meta;
	first_col.GetValue(rcSel.top, &from_value, &from_meta);

	if (from_value == first_col.novalue) return;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		if (col.plugin_id == first_col.plugin_id
			&& col.group == first_col.group
			&& col.column == first_col.column
		) {
			zzub_pattern_set_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, from_value, from_meta);
		}
	}

	zzub_player_history_commit(player, 0, 0, "All To First Selection");
}

void CPatternView::OnFirstToLastSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	int tolast_row = rcSel.bottom - (editorInner.GetSkip() - 1);

	PE_column const& first_col = editorInner.GetColumn(rcSel.left);
	int from_value;
	int from_meta;
	first_col.GetValue(rcSel.top, &from_value, &from_meta);
	int to_value;
	int to_meta;
	first_col.GetValue(tolast_row, &to_value, &to_meta);

	if (to_value == first_col.novalue) return;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		if (col.plugin_id == first_col.plugin_id
			&& col.group == first_col.group
			&& col.column == first_col.column
		) {
			zzub_pattern_replace_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, from_value, from_meta, to_value, to_meta);
		}
	}

	zzub_player_history_commit(player, 0, 0, "First To Last Selection");
}

void CPatternView::OnRemoveFirstSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	PE_column const& first_col = editorInner.GetColumn(rcSel.left);
	int from_value;
	int from_meta;
	first_col.GetValue(rcSel.top, &from_value, &from_meta);

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		if (col.plugin_id == first_col.plugin_id
			&& col.group == first_col.group
			&& col.column == first_col.column
		) {
			zzub_pattern_remove_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, from_value, from_meta);
		}
	}

	zzub_player_history_commit(player, 0, 0, "Remove First Selection");
}

void CPatternView::OnSetWavesSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	int selected_wave = GetSelectedWave();

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		if (isColType(col, t_wave)) {
			zzub_pattern_set_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, selected_wave, 0);
		}
	}

	zzub_player_history_commit(player, 0, 0, "Set Waves Selection");
}

void CPatternView::OnNotelengthSelection() {
	if (InvalidPattern()) return;
	notelengthDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	notelengthDlg.ShowWindow(SW_SHOWNORMAL); 
}
void CPatternView::OnNotelengthOK() {
	int x;
	bool valid = true;
	int mode = notelengthDlg.getCheckedRadio();
	if (mode == 3) {
		x = 1;
	} else {
		valid &= notelengthDlg.m_edit.GetInt(x);
		valid &= (x >= 1);
	}
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			if (isColType(col, t_note)) {
				zzub_pattern_notelength_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height, x, mode, 255);
			}
		}

		zzub_player_history_commit(player, 0, 0, "Notelength Selection");
	}
}

void CPatternView::OnVolumesSelection() {
	if (InvalidPattern()) return;
	volumesDlg.Create(m_hWnd, (LPARAM)m_hWnd);
	volumesDlg.ShowWindow(SW_SHOWNORMAL);
}
void CPatternView::OnVolumesOK() {
	int x;
	bool valid = true;
	int mode = volumesDlg.getCheckedRadio();
	if (valid) {
		CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		int prev_plugin_id = -1;
		int prev_group = -1;
		int prev_track = -1;
		int note_col_idx = -1;
		int vol_col_idx = -1;
		bool did_volume = false;

		for (int i = 0; i < width; ++i)
		{
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);

			bool new_track = false;
			if (prev_plugin_id == col.plugin_id) {
				if (prev_group == col.group) {
					if (prev_track != col.track) {
						new_track = true;
					}
				} else {
					new_track = true;
				}
			} else {
				new_track = true;
			}

			prev_plugin_id = col.plugin_id;
			prev_group = col.group;
			prev_track = col.track;

			if (new_track) {
				note_col_idx = -1;
				vol_col_idx = -1;
				did_volume = false;
			}

			if ((col.group == 2) && !did_volume) {
				if (isColType(col, t_note)) {
					note_col_idx = col.index;
				} else
				if (isColType(col, t_ampl)) {
					vol_col_idx = col.index;
				}

				if ((note_col_idx != -1) && (vol_col_idx != -1)) {
					PE_column const& note_col = editorInner.GetColumn(note_col_idx);
					PE_column const& vol_col = editorInner.GetColumn(vol_col_idx);
					zzub_pattern_volumes_events(pattern,
						col.plugin_id, col.group, col.track, note_col.column, vol_col.column,
						rcSel.top, height, mode
					);
					did_volume = true;
				}
			}
		}

		zzub_player_history_commit(player, 0, 0, "Volumes Selection");
	}
}

void CPatternView::OnTrackSwapSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	zzub_pattern_swap_track_events(pattern, rcSel.left, rcSel.right, rcSel.top, height);

	zzub_player_history_commit(player, 0, 0, "Swap Track Values Selection");
}

void CPatternView::OnRowSwapSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	int bottom_row = rcSel.bottom - (editorInner.GetSkip() - 1);

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		zzub_pattern_swap_rows_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, bottom_row);
	}

	zzub_player_history_commit(player, 0, 0, "Row Swap Selection");
}

void CPatternView::OnClearSameColumnSelection() {
	if (InvalidPattern()) return;

	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	PE_column const& first_col = editorInner.GetColumn(rcSel.left);
	int from_value;
	int from_meta;
	first_col.GetValue(rcSel.top, &from_value, &from_meta);

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		// isColType -> allows all
		if (col.plugin_id == first_col.plugin_id
			&& col.group == first_col.group
			&& col.column == first_col.column
		) {
			zzub_pattern_delete_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height);
		}
	}

	zzub_player_history_commit(player, 0, 0, "Clear Same Column Selection");
}

// ---------------------------------------------------------------------------------------------------------------
// INSERT / DELETE
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnInsertColumnRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, pos.column, pos.row, editorInner.GetSkip());
	zzub_player_history_commit(player, 0, 0, "Insert Column Row");
}

void CPatternView::OnInsertTrackRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, -1, pos.row, editorInner.GetSkip());
	zzub_player_history_commit(player, 0, 0, "Insert Track Row");
}

void CPatternView::OnInsertPatternRow() {
	if (InvalidPattern()) return;

	int row = editorInner.GetCursorRowAbsolute();
	zzub_pattern_timeshift_events(pattern, -1, -1, -1, -1, row, editorInner.GetSkip());
	zzub_player_history_commit(player, 0, 0, "Insert Pattern Row");
}

// ---

void CPatternView::OnDelete() {
	if (InvalidPattern()) return;

	if (editorInner.HasSelection() && !editorInner.sticky_selection) {
		DeletePatternSelection();
		zzub_player_history_commit(player, 0, 0, "Delete Pattern Selection");
	} else {
		OnDeleteColumnRow();
	}
}

void CPatternView::DeletePatternSelection() {
	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);
		zzub_pattern_delete_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, height);
	}
}

void CPatternView::OnDeleteColumnRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_pattern_delete_events(pattern, pos.plugin_id, pos.group, pos.track, pos.column, pos.row, editorInner.GetSkip());
	zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, pos.column, pos.row, -editorInner.GetSkip());
	zzub_player_history_commit(player, 0, 0, "Delete Column Row");
}

void CPatternView::OnDeleteTrackRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_pattern_delete_events(pattern, pos.plugin_id, pos.group, pos.track, -1, pos.row, editorInner.GetSkip());
	zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, -1, pos.row, -editorInner.GetSkip());
	zzub_player_history_commit(player, 0, 0, "Delete Track Row");
}

void CPatternView::OnDeletePatternRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_pattern_delete_events(pattern, -1, -1, -1, -1, pos.row, editorInner.GetSkip());
	zzub_pattern_timeshift_events(pattern, -1, -1, -1, -1, pos.row, -editorInner.GetSkip());
	zzub_player_history_commit(player, 0, 0, "Delete Pattern Row");
}

// ---

void CPatternView::OnBackspaceColumnRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	if (pos.row == 0) return;

	int skip = editorInner.GetSkip();
	int step = editorInner.step;
	int row = pos.row - (skip * step);

	if (row >= 0)
		RemoveEdit(pos.plugin_id, pos.group, pos.track, pos.column, row);
	// this is cooler when it doesnt timeshift up -- lets you cancel something you just typed in
	///zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, pos.column, pos.row, -skip);
	editorInner.MoveCursor(0, -editorInner.step, false, CPatternEditorInner::recenter_mode_only);
	zzub_player_history_commit(player, 0, 0, "Backspace Value");
}

void CPatternView::OnBackspaceTrackRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	if (pos.row == 0) return;

	int skip = editorInner.GetSkip();
	int step = editorInner.step;
	int row = pos.row - (skip * step);

	if (row >= 0)
		zzub_pattern_delete_events(pattern, pos.plugin_id, pos.group, pos.track, -1, row, 1);//(skip * step));
	///zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, -1, pos.row, -skip);
	editorInner.MoveCursor(0, -editorInner.step, false, CPatternEditorInner::recenter_mode_only);
	zzub_player_history_commit(player, 0, 0, "Backspace Track Row");
}

void CPatternView::OnBackspacePatternRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	if (pos.row == 0) return;

	int skip = editorInner.GetSkip();
	int step = editorInner.step;
	int row = pos.row - (skip * step);

	if (row >= 0)
		zzub_pattern_delete_events(pattern, -1, -1, -1, -1, row, 1);//(skip * step));
	///zzub_pattern_timeshift_events(pattern, -1, -1, -1, -1, pos.row, -skip);
	editorInner.MoveCursor(0, -editorInner.step, false, CPatternEditorInner::recenter_mode_only);
	zzub_player_history_commit(player, 0, 0, "Backspace Pattern Row");
}


bool IsNoteTrack(zzub_parameter_t *param)
{
	int type = zzub_parameter_get_type(param);
	if (type == zzub_parameter_type_note) {
		return true;
	}
	char tmpbuf[64];
	strncpy(tmpbuf, zzub_parameter_get_name(param), sizeof(tmpbuf));
	tmpbuf[sizeof(tmpbuf) - 1] = '\0';
	strupr(tmpbuf);

	if (strstr(tmpbuf, "TRIGGER") || strstr(tmpbuf, "VELOCITY")) {
		return true;
	}

	return false;
}

void CPatternView::OnNudgeBackward() {
	NudgeBackwardAmount(0x10);
}

void CPatternView::OnNudgeBackwardSmall() {
	NudgeBackwardAmount(0x01);
}

void CPatternView::NudgeBackwardAmount(int delayvalue) {
	if (InvalidPattern()) return;

	// TBD: - nudge selection does not work (and cannot be done properly)
	//      - use editorInner.GetSkip() somewhere

	bool changed = false;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	PE_track& track = editorInner.tracks[pos.track];
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, track.plugin_id);

	bool is_note_track = false;

	int paramcount = zzub_plugin_get_parameter_count(plugin, pos.group, pos.track);
	int i;
	for (i = 0; i < paramcount; i++) {
		zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, pos.group, pos.track, i);
		if (IsNoteTrack(param)) {
			is_note_track = true;
			break;
		}
	}

	if (!is_note_track) {
		return;
	}

	for (i = 0; i < paramcount; i++) {
		zzub_parameter_t* param2 = zzub_plugin_get_parameter(plugin, pos.group, pos.track, i);
		char tmpbuf[64];
		strncpy(tmpbuf, zzub_parameter_get_name(param2), sizeof(tmpbuf));
		tmpbuf[sizeof(tmpbuf) - 1] = '\0';
		strupr(tmpbuf);

		if (strstr(tmpbuf, "DELAY") && (zzub_parameter_get_type(param2) == zzub_parameter_type_byte)) {
			// adjust delay
			int row = pos.row;
			int value, meta;
			int ret = zzub_pattern_get_value(pattern, row, pos.plugin_id, pos.group, pos.track, i, &value, &meta);

			if (ret < 0) {
				value = zzub_parameter_get_value_default(param2);
			}

			value -= delayvalue;

			if (value < zzub_parameter_get_value_min(param2)) {
				// move to previous row, if possible
				if (row < 1)
				{
					// reached beginning of pattern, cancel
					return;
				}
				value += zzub_parameter_get_value_max(param2) - zzub_parameter_get_value_min(param2) + 1;

				row--;
				zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, -1, row, -1);

				// move rest of lines back if necessary
				if (zzub_pattern_get_row_count(pattern) > row + 1) {
					zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, -1, row + 1, 1);
				}

				editorInner.MoveCursor(0, -1, false, true);
			}

			zzub_pattern_set_value(pattern, row, plugin, pos.group, pos.track, i, value, meta);
			changed = true;

			break;
		}
	}

	if (changed) {
		zzub_player_history_commit(player, 0, 0, "Nudge backward");
	}
}

void CPatternView::OnNudgeForward() {
	NudgeForwardAmount(0x10);
}
void CPatternView::OnNudgeForwardSmall() {
	NudgeForwardAmount(0x01);
}

void CPatternView::NudgeForwardAmount(int delayvalue) {
	if (InvalidPattern()) return;

	// TBD: - nudge selection does not work (and cannot be done properly)
	//      - use editorInner.GetSkip() somewhere

	bool changed = false;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	PE_track& track = editorInner.tracks[pos.track];
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, track.plugin_id);

	bool is_note_track = false;

	int paramcount = zzub_plugin_get_parameter_count(plugin, pos.group, pos.track);
	int i;
	for (i = 0; i < paramcount; i++) {
		zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, pos.group, pos.track, i);
		if (IsNoteTrack(param)) {
			is_note_track = true;
			break;
		}
	}

	if (!is_note_track) {
		return;
	}

	for (i = 0; i < paramcount; i++) {
		zzub_parameter_t* param2 = zzub_plugin_get_parameter(plugin, pos.group, pos.track, i);
		char tmpbuf[64];
		strncpy(tmpbuf, zzub_parameter_get_name(param2), sizeof(tmpbuf));
		tmpbuf[sizeof(tmpbuf) - 1] = '\0';
		strupr(tmpbuf);

		if (strstr(tmpbuf, "DELAY") && (zzub_parameter_get_type(param2) == zzub_parameter_type_byte)) {
			// adjust delay
			int row = pos.row;
			int value, meta;
			int ret = zzub_pattern_get_value(pattern, row, pos.plugin_id, pos.group, pos.track, i, &value, &meta);

			if (ret < 0) {
				value = zzub_parameter_get_value_default(param2);
			}

			// XXX editorInner.GetSkip() ?
			value += delayvalue;

			if (value > zzub_parameter_get_value_max(param2)) {
				// move to next row, if possible
				if (zzub_pattern_get_row_count(pattern) <= row + 1)
				{
					// reached end of pattern, cancel
					return;
				}
				value -= zzub_parameter_get_value_max(param2) - zzub_parameter_get_value_min(param2) + 1;

				// remove the target line first if necessary
				if (zzub_pattern_get_row_count(pattern) > row + 1) {
					zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, -1, row + 1, -1);
				}

				zzub_pattern_timeshift_events(pattern, pos.plugin_id, pos.group, pos.track, -1, row, 1);
				row++;
				editorInner.MoveCursor(0, 1, false, true);
			}

			zzub_pattern_set_value(pattern, row, plugin, pos.group, pos.track, i, value, meta);
			changed = true;

			break;
		}
	}
	
	if (changed) {
		zzub_player_history_commit(player, 0, 0, "Nudge forward");
	}
}


// ---

void CPatternView::OnClearValue() {
	if (InvalidPattern()) return;

//	editorInner.ClearValue();
	PE_cursor_pos pos = editorInner.GetCursorPos();
	RemoveEdit(pos.plugin_id, pos.group, pos.track, pos.column, pos.row);
	zzub_player_history_commit(player, 0, 0, "Clear Value");

	editorInner.StepCursor();
}

void CPatternView::OnClearTrackRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_pattern_delete_events(pattern, pos.plugin_id, pos.group, pos.track, -1, pos.row, editorInner.GetSkip());
	zzub_player_history_commit(player, 0, 0, "Clear Track Row");

	editorInner.StepCursor();
}

void CPatternView::OnClearPatternRow() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_pattern_delete_events(pattern, -1, -1, -1, -1, pos.row, editorInner.GetSkip());
	zzub_player_history_commit(player, 0, 0, "Clear Pattern Row");

	editorInner.StepCursor();
}

// ---------------------------------------------------------------------------------------------------------------
// COPY / CUT / PASTE / DRAG
// ---------------------------------------------------------------------------------------------------------------

BOOL CPatternView::OnIdle() {
	int editFlags = GetEditFlags();
	UIEnable(ID_EDIT_CUT, (editFlags & EDIT_COPY) != 0);
	UIEnable(ID_EDIT_COPY, (editFlags & EDIT_COPY) != 0);
	UIEnable(ID_EDIT_PASTE, (editFlags & EDIT_PASTE) != 0);
	UIEnable(ID_PATTERNVIEW_PASTE_MIXOVER, (editFlags & EDIT_PASTE) != 0);
	UIEnable(ID_PATTERNVIEW_PASTE_MIXUNDER, (editFlags & EDIT_PASTE) != 0);
	UIEnable(ID_EDIT_DELETE, (editFlags & EDIT_COPY) != 0);
	UIEnable(ID_PATTERNVIEW_CUT_SPLICE, (editFlags & EDIT_COPY) != 0);
	UIEnable(ID_PATTERNVIEW_PASTE_SPLICE, (editFlags & EDIT_PASTE) != 0);

	UIEnable(ID_PATTERNVIEW_PATTERNSTACK_BACK, !pattern_stack.empty() && (pattern_stack_pos > 0));
	UIEnable(ID_PATTERNVIEW_PATTERNSTACK_FORWARD, !pattern_stack.empty() && (pattern_stack_pos < (pattern_stack.size() - 1)));
	UIUpdateToolBar();

	return FALSE;
}

LRESULT CPatternView::OnGetEditFlags(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/) {
	return GetEditFlags();
}

LRESULT CPatternView::OnGetStatusText(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (is_primary && zzub_player_get_order_length(player) == 0) {
		if (orderlist_enabled)
			return (LRESULT)("The orderlist is empty: Add at least one pattern to the orderlist");
		else
			return (LRESULT)("The orderlist is empty and disabled: Select 'Show Orderlist' from the Pattern Editor context menu to enable");
	}

	if (pattern != 0 && editorInner.GetColumnCount() == 0)
		return (LRESULT)("Pattern has no columns, please hit Shift+F2 to add columns");

	if (pattern == 0)
		return (LRESULT)("No pattern selected");

	PE_column const& col = editorInner.GetColumnAtCursor();
	switch (col.control) {
		case pattern_column_control_pattern:
			return (LRESULT)("Trigger Column: Type the name of a pattern to insert, or use Alt+Up/Down to select and insert a pattern from the Infolist.");
		case pattern_column_control_note:
			return (LRESULT)("Note Column: Enter notes with your PC or MIDI keyboard. Hold Shift to enter chords.  Press '.' to clear current value.");
		case pattern_column_control_byte:
		case pattern_column_control_word:
			return (LRESULT)("Value Column: Enter hex digits (0..F). Press '.' to clear current value.");
		case pattern_column_control_switch:
			return (LRESULT)("Switch Column: Enter 0 or 1. Press '.' to clear current value.");
		case pattern_column_control_button:
			return (LRESULT)("Button Column: Enter 0 or 1. Press Space to toggle. Press '.' to clear current value.");
		case pattern_column_control_envelope:
			return (LRESULT)("Envelope Column: Ctrl+Click to insert points. Drag to move points. Press '.' to clear current value.");
		case pattern_column_control_pianoroll:
			return (LRESULT)("Pianoroll Column: Click to paint/move/resize notes. Shift+Click to select notes. ");
		case pattern_column_control_matrix:
			return (LRESULT)("Note Matrix Column: Click to toggle single-hit note.");
		case pattern_column_control_slider:
			return (LRESULT)("Slider Column: Ctrl+Click change value. Press '.' to clear current value.");
		default:
			return 0;
	}
}


int CPatternView::GetEditFlags() {
	if (InvalidPattern()) return 0;

	int flags = 0;
	//if (editorInner.HasSelection()) // commented because you can cut/copy the cursor position
		flags |= EDIT_COPY;
	if (ClipboardHasFormat(m_hWnd, "Buze:PatternSelection"))
		flags |= EDIT_PASTE;
	return flags;
}

void CPatternView::OnCut() {
	if (InvalidPattern()) return;

	CutPatternSelection();
	zzub_player_history_commit(player, 0, 0, "Cut Pattern Selection");
}

void CPatternView::OnCutSplice() {
	if (InvalidPattern()) return;

	CutPatternSelection();

	{	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
		int width = rcSel.right - rcSel.left + 1;
		int height = rcSel.bottom - rcSel.top + 1;

		for (int i = 0; i < width; ++i) {
			PE_column const& col = editorInner.GetColumn(rcSel.left + i);
			zzub_pattern_timeshift_events(pattern, col.plugin_id, col.group, col.track, col.column, rcSel.top, -height);
		}
	}

	editorInner.ClearSelection();

	zzub_player_history_commit(player, 0, 0, "Cut Splice Pattern Selection");
}

void CPatternView::OnCopy() {
	if (InvalidPattern()) return;

	CopyPatternSelection();
}

void CPatternView::OnPaste() {
	if (InvalidPattern()) return;

	editorInner.ClearSelection();

	PE_cursor_pos pos = editorInner.GetCursorPos();
	PastePatternSelection(pattern, pos.plugin_id, pos.group, pos.track, pos.column, pos.row, paste_mode_replace);
	zzub_player_history_commit(player, 0, 0, "Paste Pattern Selection");
}

void CPatternView::OnPasteMixOver() {
	if (InvalidPattern()) return;

	editorInner.ClearSelection();

	PE_cursor_pos pos = editorInner.GetCursorPos();
	PastePatternSelection(pattern, pos.plugin_id, pos.group, pos.track, pos.column, pos.row, paste_mode_mix_over);
	zzub_player_history_commit(player, 0, 0, "Paste (Mix Over) Pattern Selection");
}

void CPatternView::OnPasteMixUnder() {
	if (InvalidPattern()) return;

	editorInner.ClearSelection();

	PE_cursor_pos pos = editorInner.GetCursorPos();
	PastePatternSelection(pattern, pos.plugin_id, pos.group, pos.track, pos.column, pos.row, paste_mode_mix_under);
	zzub_player_history_commit(player, 0, 0, "Paste (Mix Under) Pattern Selection");
}

void CPatternView::OnPasteSplice() {
	if (InvalidPattern()) return;

	editorInner.ClearSelection();

	InsertPatternSelection();
	PE_cursor_pos pos = editorInner.GetCursorPos();
	PastePatternSelection(pattern, pos.plugin_id, pos.group, pos.track, pos.column, pos.row, paste_mode_replace);

	zzub_player_history_commit(player, 0, 0, "Paste Splice Pattern Selection");
}

void CPatternView::OnPasteStep() {
	if (InvalidPattern()) return;

	editorInner.ClearSelection();

	PE_cursor_pos pos = editorInner.GetCursorPos();
	int len = PastePatternSelection(pattern, pos.plugin_id, pos.group, pos.track, pos.column, pos.row, paste_mode_replace);
	editorInner.MoveCursor(0, len / editorInner.GetSkip(), true, CPatternEditorInner::recenter_mode_also);

	zzub_player_history_commit(player, 0, 0, "Paste+Step Pattern Selection");
}

void CPatternView::CopyPatternSelection()
{
	CRect rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	int width = rcSel.right - rcSel.left + 1;
	int height = rcSel.bottom - rcSel.top + 1;

	std::stringstream strm;
	strm << width << std::endl;
	strm << height << std::endl;
	for (int i = 0; i < width; ++i) {
		PE_column const& col = editorInner.GetColumn(rcSel.left + i);

		PE_values_by_time::const_iterator j = col.values_by_time.lower_bound(rcSel.top);
		PE_values_by_time::const_iterator j_end = col.values_by_time.upper_bound(rcSel.bottom);

		int counter = (int)std::distance(j, j_end);
		strm << counter << std::endl;

		float col_range0 = col.maxvalue - col.minvalue;

		while (j != j_end) {
			float value0 = (float(j->value - col.minvalue) / col_range0);
			strm << (j->time - rcSel.top) << std::endl << value0 << std::endl << j->meta << std::endl;
			++j;
		}
	}
	std::string out = strm.str();
	CopyBinary(m_hWnd, "Buze:PatternSelection", out.c_str(), out.length());
}

void CPatternView::CutPatternSelection() {
	CopyPatternSelection();
	DeletePatternSelection();
}

void CPatternView::InsertPatternSelection() {
	UINT format = RegisterClipboardFormat("Buze:PatternSelection");
	if (!OpenClipboard()) return;

	HANDLE hData = GetClipboardData(format);
	if (hData == 0) {
		CloseClipboard();
		return;
	}

	char* charbuf = (char*)GlobalLock(hData);
	int bufferSize = GlobalSize(hData);
	char temp[64];

	if (charbuf != 0) {
		int columns = -1;
		int length = -1;
		std::stringstream strm(charbuf);
		strm >> columns;
		strm >> length;

		PE_cursor_pos pos = editorInner.GetCursorPos();
		int firstcol = editorControl.GetColumnIndex(pos.plugin_id, pos.group, pos.track, pos.column);
		for (int i = 0; i < columns; ++i) {
			if (firstcol + i >= editorInner.GetColumnCount()) break;
			PE_column const& col = editorInner.GetColumn(firstcol + i);
			zzub_pattern_timeshift_events(pattern, col.plugin_id, col.group, col.track, col.column, pos.row, length);
		}
	}

	GlobalUnlock(hData);
	CloseClipboard();
}

int CPatternView::PastePatternSelection(zzub_pattern_t* pat, int plugin_id, int group, int track, int column, int row, PasteMode mode) {
	UINT format = RegisterClipboardFormat("Buze:PatternSelection");
	if (!OpenClipboard()) return -1;

	HANDLE hData = GetClipboardData(format);
	if (hData == 0) {
		CloseClipboard();
		return -1;
	}

	char* charbuf = (char*)GlobalLock(hData);
	int bufferSize = GlobalSize(hData);

	int columns = -1;
	int length = -1;

	if (charbuf != 0) {
		std::stringstream strm(charbuf);
		strm >> columns;
		strm >> length;

		int fromidx = editorControl.GetColumnIndex(plugin_id, group, track, column);
		zzub_pattern_paste_stream_events(pattern, fromidx, row, (int)mode, charbuf);
	}

	GlobalUnlock(hData);
	CloseClipboard();

	return length;
}

void CPatternView::DragPatternSelection(RECT rcFrom, POINT ptTo, bool makecopy, PasteMode mode) {
	int width = rcFrom.right - rcFrom.left + 1;
	int length = rcFrom.bottom - rcFrom.top + 1;

	int from_left = rcFrom.left;
	int from_top = rcFrom.top;
	int to_left = ptTo.x;
	int to_top = ptTo.y;

	zzub_pattern_move_scale_events(pattern, 
		from_left, from_top,
		to_left, to_top,
		width, length, mode, makecopy
	);
}

// ---------------------------------------------------------------------------------------------------------------
// CURSOR
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnMoveCursor() {
	if (InvalidPattern()) return;

	pattern_position pos = viewInfo->GetPatternPosition(caption, pattern);
	int last_row = pos.row;

	POINT cursor = editorInner.GetCursorAbsolute();
	PE_cursorcolumn const& ccol = editorInner.GetCursorColumnAtCursor();
	PE_column const& col = editorInner.GetColumnAtCursor();

	bool same_column = (true
		&& (pos.plugin_id == col.plugin_id)
		&& (pos.group == pos.group)
		&& (pos.track == col.track)
		&& (pos.column == col.column)
	);

	pos.row = cursor.y;
	pos.plugin_id = col.plugin_id;
	pos.group = col.group;
	pos.track = col.track;
	pos.column = col.column;
	pos.digit = ccol.digit;
	pos.scroll_x_unit = editorInner.scroll.x;
	pos.scroll_y_row = editorInner.scroll.y * editorInner.skip; // absolute

	viewInfo->SetPatternPosition(caption, pattern, pos);

/*	std::vector<std::vector<CPatternView*> >::iterator links = document->findLinkedPatternEditors(this);
	if (links != document->linkedPatternEditors.end() && document->linkedScrollOwner == 0) {
		// need to set master move editor to prevent get in the way recursively
		document->linkedScrollOwner = this;

		std::vector<CPatternView*>::iterator j;
		for (j = links->begin(); j != links->end(); ++j) {
			if (*j == this) continue;
			int yrel = cursor.y - (*j)->editorInner.cursor.y;
			(*j)->editorInner.MoveCursor(0, yrel, false, CPatternEditorInner::recenter_mode_none);
		}

		document->linkedScrollOwner = 0;
	}*/

	BindStatus();
	if (!same_column) {
		CheckColumnInfoVisibility();
		BindColumnInfo();
	}

	if (last_row != pos.row) {
		buze_event_data_t ev;
		ev.change_pattern_row.row = pos.row;
		buze_document_notify_views(document, this, buze_event_type_change_pattern_row, &ev);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// SELECTIONS
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnSelectAll() {
	if (InvalidPattern()) return;

	editorInner.SelectRangeAbsolute(0, 0, editorInner.GetColumnCount() - 1, editorInner.GetPatternRows() - 1);
}

void CPatternView::OnSelectBegin() {
	if (InvalidPattern()) return;

// 	PE_cursor_pos const& pos = editorInner.GetCursorPos();
// 	PE_column const& col = editorInner.GetColumnAtCursor();
// 	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
// 
// 	if (col.index > rcSel.left)
// 		editorInner.SelectRangeAbsolute(rcSel.left, pos.row, col.index, rcSel.bottom);
// 	else
// 		editorInner.SelectRangeAbsolute(col.index, pos.row, rcSel.right, rcSel.bottom);
}

void CPatternView::OnSelectEnd() {
	if (InvalidPattern()) return;

// 	PE_cursor_pos const& pos = editorInner.GetCursorPos();
// 	PE_column const& col = editorInner.GetColumnAtCursor();
// 	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
// 
// 	if (col.index > rcSel.left)
// 		editorInner.SelectRangeAbsolute(rcSel.left, rcSel.top, col.index, pos.row);
// 	else
// 		editorInner.SelectRangeAbsolute(col.index, rcSel.top, rcSel.right, pos.row);
}

void CPatternView::OnUnselect() {
	if (InvalidPattern()) return;
	editorInner.Unselect();
}

void CPatternView::OnReselect() {
	if (InvalidPattern()) return;
	editorInner.Reselect();
}

void CPatternView::OnSelectDown() {
	if (InvalidPattern()) return;
	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();
	rcSel.bottom = editorInner.GetPatternRows() - 1;
	editorInner.SelectRangeAbsolute(rcSel.left, rcSel.top, rcSel.right, rcSel.bottom);
}

void CPatternView::OnSelectBeat() {
	if (InvalidPattern()) return;

	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();

	int selectionLength = (rcSel.bottom - rcSel.top) + 1;
	int beatHighlight = editorInner.verydark_row;

	if (rcSel.bottom == (editorInner.pattern_rows - 1)) {
		selectionLength = beatHighlight;
	} else
	if ((selectionLength % beatHighlight) == 0) {
		selectionLength *= 2;
	} else {
		selectionLength = selectionLength + (beatHighlight - (selectionLength % beatHighlight));
	}

	rcSel.bottom = rcSel.top + selectionLength - 1;
	if (rcSel.bottom >= editorInner.pattern_rows) 
		rcSel.bottom = editorInner.pattern_rows - 1;

	editorInner.SelectRangeAbsolute(rcSel.left, rcSel.top, rcSel.right, rcSel.bottom);
}

void CPatternView::OnSelectColumns() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();

	RECT rcSel;
	editorInner.GetSortedSelectionRectAbsolute(&rcSel);

	// mark column
	int firstColumn = col.index;
	int lastColumn = col.index;

	if ((rcSel.top == 0) && (rcSel.bottom == editorInner.pattern_rows - 1)) {
		int thisPatternNumColumns = editorInner.GetColumnCount() - 1;
		int thisPluginFirstColumn = -1;
		int thisPluginLastColumn = -1;
		int thisPluginNumColumns = 0;
		int thisPluginNumGroups = 0;
		int thisGroupFirstColumn = -1;
		int thisGroupLastColumn = -1;
		int thisGroupNumColumns = 0;
		int thisGroupNumTracks = 0;
		int thisTrackFirstColumn = -1;
		int thisTrackLastColumn = -1;
		int thisTrackNumColumns = 0;

		{	bool got_plugin = false;
			int last_group = -1;
			int last_track = -1;

			for (int i = 0; i < editorInner.GetColumnCount(); ++i) {
				PE_column const& check = editorInner.GetColumn(i);

				if (check.plugin_id == col.plugin_id) {
					got_plugin = true;

					if (thisPluginFirstColumn == -1)
						thisPluginFirstColumn = i;
					thisPluginLastColumn = i;
					++thisPluginNumColumns;

					if (check.group != last_group)
						++thisPluginNumGroups;

					if (check.group == col.group) {
						if (thisGroupFirstColumn == -1)
							thisGroupFirstColumn = i;
						thisGroupLastColumn = i;
						++thisGroupNumColumns;

						if (check.track != last_track)
							++thisGroupNumTracks;

						if (check.track == col.track) {
							if (thisTrackFirstColumn == -1)
								thisTrackFirstColumn = i;
							thisTrackLastColumn = i;
							++thisTrackNumColumns;
						}
					}
				} else
				if (got_plugin) {
					break;
				}

				last_group = check.group;
				last_track = check.track;
			}
		}

		if ((rcSel.left == 0) && (rcSel.right == thisPatternNumColumns)) {
			// mark column
			firstColumn = col.index;
			lastColumn = col.index;
		} else
		if ((rcSel.left == col.index) && (rcSel.right == col.index) && (thisTrackNumColumns > 1)) {
			// mark track
			firstColumn = thisTrackFirstColumn;
			lastColumn = thisTrackLastColumn;
		} else
		if ((rcSel.left == thisTrackFirstColumn) && (rcSel.right == thisTrackLastColumn) && (thisGroupNumTracks > 1)) {
			// mark group
			firstColumn = thisGroupFirstColumn;
			lastColumn = thisGroupLastColumn;
		} else
		if ((rcSel.left == thisGroupFirstColumn) && (rcSel.right == thisGroupLastColumn) && (thisPluginNumGroups > 1)) {
			// mark plugin
			firstColumn = thisPluginFirstColumn;
			lastColumn = thisPluginLastColumn;
		} else {
			// mark pattern
			firstColumn = 0;
			lastColumn = editorInner.GetColumnCount() - 1;
		}
	}

	editorInner.SelectRangeAbsolute(firstColumn, 0, lastColumn, editorInner.pattern_rows - 1);
}

void CPatternView::OnSelChanged() {
	//if (InvalidPattern()) return;

	if (editorInner.HasSelection()) {
		POINT ptFrom;
		POINT ptTo;
		editorInner.GetUnsortedSelectionRangeAbsolute(&ptFrom, &ptTo);
		PE_column const& fromCol = editorInner.GetColumn(ptFrom.x);
		PE_column const& toCol = editorInner.GetColumn(ptTo.x);

		selection_position from_pos = { ptFrom.y, fromCol.plugin_id, fromCol.group, fromCol.track, fromCol.column };
		selection_position to_pos = { ptTo.y, toCol.plugin_id, toCol.group, toCol.track, toCol.column };

		viewInfo->SetPatternSelection(caption, pattern, from_pos, to_pos);
	} else {
		selection_position default_pos = { -1, 0, 0, 0, -1 };
		viewInfo->SetPatternSelection(caption, pattern, default_pos, default_pos);
	}

	BindStatus();
}

LRESULT CPatternView::OnSelDrop(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/) {
	bool stamp = (bool)wNotifyCode;

	RECT rcSel; // the source
	editorInner.GetSortedSelectionRectAbsolute(&rcSel);

	POINT ptDragto; // used for the destination
	editorInner.GetDragtoPointAbsolute(&ptDragto);

	RECT rcDragto; // used for the new selection we're making
	editorInner.GetDragtoRectAbsolute(&rcDragto);

	if ((rcSel.left == ptDragto.x) && (rcSel.top == ptDragto.y)) {
		editorInner.InvalidateSelection();
		return 0;
	}

	bool makecopy = IsShiftDown() || stamp;

	PasteMode pastemode;
	if (IsCtrlDown() && IsAltDown()) {
		pastemode = paste_mode_mix_under;
	} else
	if (IsCtrlDown()) {
		pastemode = paste_mode_swap;
	} else
	if (IsAltDown()) {
		pastemode = paste_mode_mix_over;
	} else {
		pastemode = paste_mode_replace;
	}

	DragPatternSelection(rcSel, ptDragto, makecopy, pastemode);
	editorInner.SelectRangeAbsolute(rcDragto.left, rcDragto.top, rcDragto.right, rcDragto.bottom);

	zzub_player_history_commit(player, 0, 0, "Drag+Drop Selection");

	return 0;
}

void CPatternView::OnSelDropCell() {
	RECT rcSel; // the source
	editorInner.GetSortedSelectionRectAbsolute(&rcSel);

	POINT ptDragto; // used for the destination + new selection
	editorInner.GetDragtoPointAbsolute(&ptDragto);

	DragPatternSelection(rcSel, ptDragto, false, paste_mode_replace);
	editorInner.SelectRangeAbsolute(ptDragto.x, ptDragto.y, ptDragto.x, ptDragto.y);

	zzub_player_history_commit(player, 0, 0, "Drag+Drop Cell Selection");
}

void CPatternView::OnSelDrag() {
}

// ---------------------------------------------------------------------------------------------------------------
// MOUSE
// ---------------------------------------------------------------------------------------------------------------

LRESULT CPatternView::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/) {
	if (InvalidPattern()) return 0;

	int d = (signed short)HIWORD(wParam);

	if (IsCtrlDown()) { // ctrl+mousewheel = change scale
		int cur_scale = editorInner.skip;
		int new_scale = -1;
		bool power_mode = !IsShiftDown();

		if (d < 0) {
			if (power_mode) {
				new_scale = cur_scale / 2;
				int remainder = cur_scale - (new_scale * 2);
				if (remainder != 0)
					return 0;
			} else {
				for (int i = scales_count - 1; i >= 0; --i) {
					if (scales[i] < cur_scale) {
						new_scale = scales[i];
						break;
					}
				}
			}
		} else
		if (d > 0) {
			if (power_mode) {
				new_scale = cur_scale * 2;
			} else {
				for (int i = 0; i < scales_count; ++i) {
					if (scales[i] > cur_scale) {
						new_scale = scales[i];
						break;
					}
				}
			}
		}

		if (new_scale != -1) {
			zzub_pattern_set_display_resolution(pattern, new_scale);
			zzub_player_history_commit(player, 0, 0, "Change Pattern Display Resolution");
		}
	} else
	if (IsShiftDown()) { // shift+mousewheel = change font size
		int new_font_size = font_size;

		if (d < 0 && new_font_size < 256) {
			new_font_size++;
		} else
		if (d > 0 && font_size > 6) {
			new_font_size--;
		}

		SetPatternEditorFont(font_name, new_font_size);
	} else { // default = scroll
		int offset = (d < 0 ? 8 : -8);

		editorInner.ScrollVertical(offset, editorInner.scroll);

		if (editorInner.mouse_mode != CPatternEditorInner::mouse_mode_none)
			editorInner.DoMouseMove();
	}

	return 0;
}

LRESULT CPatternView::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam) {
	POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
	HWND hContextWnd = (HWND)wParam;
	if (hContextWnd == toolBar) {
		CMenu menu;
		menu.LoadMenu(IDR_PATTERNVIEW_TOOLBARS);
		menu.GetSubMenu(0).TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);
	} else if (hContextWnd == infoSplitter || hContextWnd == editorInner) { // NOTE: infoSplitter == windows, editorInner = wine

		if (pt.x == -1 && pt.y == -1) {
			// get screen coords of cursor
			if (pattern != 0) {
				POINT cursor = editorInner.GetCursor();
				editorInner.GetScreenPosition(cursor.x, cursor.y, &pt);
				editorInner.ClientToScreen(&pt);
				ScreenToClient(&pt);
			} else {
				pt.x = pt.y = 0;
			}
		} else {
			POINT pt_control = pt;
			editorControl.ScreenToClient(&pt_control);

			if (pt_control.y < editorControl.margin_y || pt_control.x < editorControl.margin_x) // click on rows or header - TODO: show header context instead of solo on rdown (with solo, mute, collapse)
				return 0;

			if (!editorInner.HasSelection()) {
				// when theres no selection, move the cursor to the rclicked position
				POINT pt_inner = pt;
				editorInner.ScreenToClient(&pt_inner);
				POINT pt_cursor = editorInner.PointToCursorPoint(pt_inner);

				editorInner.SetCursor(pt_cursor.x, pt_cursor.y);
			}

			ScreenToClient(&pt);
		}

		CMenu linkedEditors;
		CMenu menu; 
		CMenu modeMenu;
		CMenu interpolateMenu;
		CMenu triggerMenu;
		CMenu simpleFormatMenu;
		CMenu fullFormatMenu;
		menu.CreatePopupMenu();

		bool hasPattern = (pattern != 0);
		bool hasFormat = (patternformat != 0);
		UINT noPatGray = !hasPattern ? MF_GRAYED : 0;
		UINT noFmtGray = !hasFormat ? MF_GRAYED : 0;
		UINT invGray = InvalidPattern() ? MF_GRAYED : 0;
		UINT volMaskCheck = volume_masked ? MF_CHECKED : MF_UNCHECKED;

		bool contextMenuMode = configuration->getPatternRightClickMode();
		if (IsCtrlDown()) contextMenuMode = !contextMenuMode;
		if (!hasPattern) contextMenuMode = false;

		if (!contextMenuMode) {
			menu.AppendMenu(MF_STRING, ID_PATTERNVIEW_CREATEPATTERN,				"Pattern Create");
			menu.AppendMenu(noPatGray|MF_STRING, ID_PATTERN_CLONE,					"Pattern Clone");
			menu.AppendMenu(noPatGray|MF_STRING, ID_PATTERN_DELETE,					"Pattern Delete");
			menu.AppendMenu(noPatGray|MF_STRING, ID_VIEW_PROPERTIES,				"Pattern Properties");
			menu.AppendMenu(MF_STRING, ID_PATTERNVIEW_SHOWPATTERNLIST,				"Pattern List");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING, ID_PATTERN_CREATEFORMAT,						"Pattern Format Create");
			menu.AppendMenu(noFmtGray|MF_STRING, ID_PATTERNVIEW_CLONE_FORMAT,		"Pattern Format Clone");
			menu.AppendMenu(noFmtGray|MF_STRING, ID_PATTERN_DELETEFORMAT,			"Pattern Format Delete");
			menu.AppendMenu(noFmtGray|MF_STRING, ID_PATTERN_FORMATPROPERTIES,		"Pattern Format Properties");
			menu.AppendMenu(noFmtGray|MF_STRING, ID_PATTERNVIEW_SHOWPATTERNFORMAT,	"Pattern Format Editor");

			// pattern format helper, optionally create pattern player and add trigger column
			{
				triggerMenu.CreatePopupMenu();
				// enumerate pattern players, add a menuitem for each trigger track, and "new track" if it has less than 128 tracks
				// also shade already present triggers
				// ID_PATTERNVIEW_ADD_TRIGGER_FIRST
				zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
				int plugindex = 0;
				while (zzub_plugin_iterator_valid(plugit) != 0) {
					zzub_plugin_t* plug = zzub_plugin_iterator_current(plugit);

					zzub_pluginloader_t* pluginfo = zzub_plugin_get_pluginloader(plug);
					std::string pluguri = zzub_pluginloader_get_uri(pluginfo);

					if (pluguri == "@zzub.org/sequence/pattern") {
						std::string name = zzub_plugin_get_name(plug);
						int numtracks = zzub_plugin_get_track_count(plug, 2);
						for (int i = 0; i < numtracks; i++) {
							std::stringstream trackname;
							trackname << name << ", Track " << (i+1);
							UINT flags = 0;
							if (patternformat != 0) {
								zzub_pattern_format_column_t* col = zzub_pattern_format_get_column(patternformat, plug, 2, i, 0);
								if (col != 0) flags |= MF_GRAYED;
							}
							triggerMenu.AppendMenu(flags|MF_STRING, (UINT_PTR)ID_PATTERNVIEW_ADD_TRIGGER_FIRST + plugindex * 128 + i, trackname.str().c_str());
						}
						if (numtracks < 127) {
							std::stringstream trackname;
							trackname << name << ", <new track>";
							triggerMenu.AppendMenu(MF_STRING, (UINT_PTR)ID_PATTERNVIEW_ADD_TRIGGER_FIRST + plugindex * 128 + 127, trackname.str().c_str());
						}
						plugindex++;
					}

					zzub_plugin_iterator_next(plugit);
				}
				zzub_plugin_iterator_destroy(plugit);

				triggerMenu.AppendMenu(MF_STRING, (UINT_PTR)ID_PATTERNVIEW_ADD_TRIGGER_NEW, "<new pattern player plugin>");
				menu.AppendMenu(noPatGray|MF_POPUP, (UINT_PTR)(HMENU)triggerMenu, "Add Trigger Column");
			}

			// pattern format helpers, "Add All Colunmns From" and "Add Simple Columns From >"
			{
				simpleFormatMenu.CreatePopupMenu();
				fullFormatMenu.CreatePopupMenu();
				zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
				int plugindex = 0;
				while (zzub_plugin_iterator_valid(plugit) != 0) {
					zzub_plugin_t* plug = zzub_plugin_iterator_current(plugit);
					int flags = zzub_plugin_get_flags(plug);
					if ((flags & zzub_plugin_flag_is_connection) == 0) {
						// TODO: if buze_document_has_simple_format_parameters(plugin)
						const char* name = zzub_plugin_get_name(plug);
						simpleFormatMenu.AppendMenu(MF_STRING, (UINT_PTR)ID_PATTERNVIEW_ADD_SIMPLE_FIRST + plugindex, name);
						fullFormatMenu.AppendMenu(MF_STRING, (UINT_PTR)ID_PATTERNVIEW_ADD_FULL_FIRST + plugindex, name);
					}
					plugindex++;
					zzub_plugin_iterator_next(plugit);
				}
				zzub_plugin_iterator_destroy(plugit);

				menu.AppendMenu(noPatGray|MF_POPUP, (UINT_PTR)(HMENU)simpleFormatMenu, "Add Note/Velocity Columns");
				menu.AppendMenu(noPatGray|MF_POPUP, (UINT_PTR)(HMENU)fullFormatMenu, "Add All Columns");
			}

			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(noPatGray|MF_STRING, ID_PATTERN_DOUBLEROWS,				"Double Rows");
			menu.AppendMenu(noPatGray|MF_STRING, ID_PATTERN_HALVEROWS,				"Halve Rows");
			menu.AppendMenu(noPatGray|MF_STRING, ID_PATTERN_DOUBLELENGTH,			"Double Length");
			menu.AppendMenu(noPatGray|MF_STRING, ID_PATTERN_HALVELENGTH,			"Halve Length");
			menu.AppendMenu(MF_SEPARATOR);


			menu.AppendMenu(MF_STRING, ID_PATTERNVIEW_DUPLICATEEDITOR,				"Edit in New Pattern Editor");
/*
			// linked editors
			{	bool hasLinkableEditors = false;
				linkedEditors.CreatePopupMenu();
				std::vector<std::vector<CPatternView*> >::iterator links = document->findLinkedPatternEditors(this);
				for (int i = 0; i < mainframe->patternEditors.getViewCount(); ++i) {
					CPatternView* pv = mainframe->patternEditors.getView(i);

					UINT editorFlags;
					editorFlags = (links != document->linkedPatternEditors.end() && 
						find(links->begin(), links->end(), pv) != links->end()) ? MF_CHECKED : 0;
					editorFlags |= pv == this ? MF_GRAYED : 0;

					ClientView* cv = mainframe->frame.getClientView(pv->m_hWnd);
					linkedEditors.AppendMenu(editorFlags|MF_STRING, (UINT_PTR)ID_PATTERNVIEW_SCROLLLINKEDITOR_FIRST + i, cv->caption);
					hasLinkableEditors = true;
				}
				menu.AppendMenu((!hasLinkableEditors?MF_GRAYED:0)|MF_POPUP, (UINT_PTR)(HMENU)linkedEditors, "Link Scrollbars To");
			}
*/
			// column context
			UINT selGray = editorInner.HasSelection() ? MF_GRAYED : 0;
			{	modeMenu.CreatePopupMenu();
				if (editorInner.GetColumnCount() > 0) {
					PE_column const& col = editorInner.GetColumnAtCursor();

					std::vector<int> modes;
					CColumnEditor::GetAvailableColumnControls(col.type, col.defaultcontrol, modes);

					for (std::vector<int>::iterator i = modes.begin(); i != modes.end(); ++i) {
						int index = (int)(i - modes.begin());
						std::string colname = CColumnEditor::GetColumnControlName(*i);
						modeMenu.InsertMenu(-1, (col.control == *i ? MF_CHECKED : 0)|selGray|MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_PATTERNVIEW_SETCOLUMNCONTROL_0 + index, colname.c_str());
					}
				}
				menu.AppendMenu(selGray|MF_POPUP, (UINT_PTR)(HMENU)modeMenu, "Column Editor");
			}

			// column interpolation
			{
				int interpolator = 0;
				UINT noteGray = 0;
				if (editorInner.GetColumnCount() > 0) {
					PE_column const& col = editorInner.GetColumnAtCursor();
					zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
					interpolator = zzub_plugin_get_parameter_interpolator(plugin, col.group, col.track, col.column);
					noteGray = (col.type == pattern_column_type_note) ? MF_GRAYED : 0;
				}
				interpolateMenu.CreatePopupMenu();
				interpolateMenu.AppendMenu((interpolator == 0 ? MF_CHECKED : 0)|noPatGray|selGray|MF_STRING, ID_PATTERNVIEW_INTERPOLATE_ABSOLUTE, "Normal (Absolute)");
				interpolateMenu.AppendMenu((interpolator == 1 ? MF_CHECKED : 0)|noPatGray|noteGray|selGray|MF_STRING, ID_PATTERNVIEW_INTERPOLATE_INERTIAL, "Normal (With Inertia)");
				interpolateMenu.AppendMenu((interpolator == 2 ? MF_CHECKED : 0)|noPatGray|noteGray|selGray|MF_STRING, ID_PATTERNVIEW_INTERPOLATE_LINEAR, "Envelope (Linear)");
				menu.AppendMenu(selGray|MF_POPUP, (UINT_PTR)(HMENU)interpolateMenu, "Column Interpolation");
			}

			menu.AppendMenu(noPatGray|selGray|MF_STRING, ID_MACHINE_PARAMETERS, "Machine Parameters");

			/*if (mainframe->primaryPatternEditor == this) {
				menu.AppendMenu(MF_SEPARATOR);
				UINT orderlistCheck = orderlist_enabled ? MF_CHECKED : MF_UNCHECKED;
				menu.AppendMenu(orderlistCheck|MF_STRING, ID_PATTERNVIEW_TOGGLE_ORDERLIST, "Show Orderlist");
			}*/
		} else {
			menu.AppendMenu(invGray|MF_STRING, ID_EDIT_CUT,									"Cut");
			menu.AppendMenu(invGray|MF_STRING, ID_EDIT_COPY,								"Copy");
			menu.AppendMenu(invGray|MF_STRING, ID_EDIT_PASTE,								"Paste");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_CUT_SPLICE,					"Cut Splice");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_PASTE_SPLICE,					"Paste Splice");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_PASTE_MIXOVER,				"Paste Mix Over");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_PASTE_MIXUNDER,				"Paste Mix Under");
			menu.AppendMenu(invGray|MF_STRING, ID_EDIT_DELETE,								"Clear");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_RANDOMIZE_SELECTION,			"Randomize");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_RANDOMIZERANGE_SELECTION,		"Random Range");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_RANDOMIZEUSING_SELECTION,		"Random From");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_HUMANIZE_SELECTION,			"Humanize");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_SHUFFLE_SELECTION,			"Shuffle");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_INTERPOLATE_SELECTION,		"Interpolate");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_GRADIATE_SELECTION,			"Gradiate");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_SMOOTH_SELECTION,				"Smooth");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_REVERSE_SELECTION,			"Reverse");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_COMPACT_SELECTION,			"Compact");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_EXPAND_SELECTION,				"Expand");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_THIN_SELECTION,				"Thin ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_ECHO_SELECTION,				"Echo ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_REPEAT_SELECTION,				"Repeat ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_UNIQUE_SELECTION,				"Unique");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_SCALE_SELECTION,				"Scale ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_FADE_SELECTION,				"Fade ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_CURVEMAP_SELECTION,			"Curvemap ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_INVERT_SELECTION,				"Invert");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_ROTATEROWS_SELECTION,			"Rotate Rows ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_ROTATEVALUES_SELECTION,		"Rotate Values ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_ROTATERHYTHMS_SELECTION,		"Rotate Rhythms ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_ROTATENOTES_SELECTION,		"Rotate Notes ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_ALLTOFIRST_SELECTION,			"All To First");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_FIRSTTOLAST_SELECTION,		"First To Last");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_REMOVEFIRST_SELECTION,		"Remove First");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_REPLACEWAVES_SELECTION,		"Replace Waves");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_NOTELENGTH_SELECTION,			"Notelengths ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_VOLUMES_SELECTION,			"Volumes ...");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_TRACKSWAP_SELECTION,			"Track Swap");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_ROWSWAP_SELECTION,			"Row Swap");
			menu.AppendMenu(invGray|MF_STRING, ID_PATTERNVIEW_CLEARSAMECOLUMN_SELECTION,	"Clear Same Column");
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(volMaskCheck|MF_STRING, ID_PATTERNVIEW_TOGGLE_VOLMASK,			"Volume Masked");
		}

		if (configuration->getShowAccelerators())
			buze_main_frame_add_menu_keys(mainframe, "patternview", menu);

		ClientToScreen(&pt);
		menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	}

	return 0;
}

LRESULT CPatternView::OnXButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/) {
// 	if (GetFocus() != editorInner) {
// 		SetFocus();
// 		return 0;
// 	}

	UINT uButton = GET_KEYSTATE_WPARAM(wParam);
	if ((uButton & MK_XBUTTON1) == MK_XBUTTON1) {
		OnBack();
	} else
	if ((uButton & MK_XBUTTON2) == MK_XBUTTON2) {
		OnForward();
	}

	return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// COLUMN CONTROLS
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnToggleColumnControl() {
	if (InvalidPattern()) return;
	assert(patternformat != 0);

	PE_column const& col = editorInner.GetColumnAtCursor();

	std::vector<int> modes;
	int n_modes = CColumnEditor::GetAvailableColumnControls(col.type, col.defaultcontrol, modes);
	if (n_modes <= 1) return;

	std::vector<int>::iterator currentmode = find(modes.begin(), modes.end(), col.control);
	if (currentmode == modes.end()) currentmode = modes.begin();

	int modeindex = (int)(currentmode - modes.begin());
	int nextmode = modes[(modeindex + 1) % modes.size()];

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
	zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_get_column(patternformat, plugin, col.group, col.track, col.column);
	assert(fmtcol != 0);
	zzub_pattern_format_column_set_mode(fmtcol, nextmode);

	// restore cursor to first digit
	pattern_position pos = viewInfo->GetPatternPosition(caption, pattern);
	pos.digit = 0;
	viewInfo->SetPatternPosition(caption, pattern, pos);

	editorInner.dirty_scrolltocursor = true;

	zzub_player_history_commit(player, 0, 0, "Toggle Column Control");
}

void CPatternView::OnSetColumnControl(WORD wID) {
	if (InvalidPattern()) return;

	int ctrlindex = wID - ID_PATTERNVIEW_SETCOLUMNCONTROL_0;

	PE_column const& col = editorInner.GetColumnAtCursor();

	std::vector<int> modes;
	int n_modes = CColumnEditor::GetAvailableColumnControls(col.type, col.defaultcontrol, modes);
	if ((ctrlindex < 0) || (ctrlindex >= n_modes)) return;

	int mode = modes[ctrlindex];

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
	zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_get_column(patternformat, plugin, col.group, col.track, col.column);
	zzub_pattern_format_column_set_mode(fmtcol, mode);

	zzub_player_history_commit(player, 0, 0, "Set Column Control");
}

void CPatternView::OnCollapseSelection() {
	if (InvalidPattern()) return;

	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();

	for (int j = rcSel.left; j <= rcSel.right; ++j) {
		PE_column const& col = editorInner.GetColumn(j);
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
		zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_get_column(patternformat, plugin, col.group, col.track, col.column);
		zzub_pattern_format_column_set_collapsed(fmtcol, 1);
	}

	zzub_player_history_commit(player, 0, 0, "Collapsed Column(s)");
}

void CPatternView::OnUncollapseSelection() {
	if (InvalidPattern()) return;

	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();

	for (int j = rcSel.left; j <= rcSel.right; ++j) {
		PE_column const& col = editorInner.GetColumn(j);
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
		zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_get_column(patternformat, plugin, col.group, col.track, col.column);
		zzub_pattern_format_column_set_collapsed(fmtcol, 0);
	}

	zzub_player_history_commit(player, 0, 0, "Uncollapsed Column(s)");
}

void CPatternView::OnToggleCollapseSelection() {
	if (InvalidPattern()) return;

	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();

	for (int j = rcSel.left; j <= rcSel.right; ++j) {
		PE_column const& col = editorInner.GetColumn(j);
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
		zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_get_column(patternformat, plugin, col.group, col.track, col.column);

		int collapse = zzub_pattern_format_column_get_collapsed(fmtcol);
		zzub_pattern_format_column_set_collapsed(fmtcol, (collapse != 0) ? 0 : 1);
	}

	zzub_player_history_commit(player, 0, 0, "Toggled Column Collapse");
}

void CPatternView::OnToggleCollapseTrack() {
	if (InvalidPattern()) return;

	RECT rcSel = editorInner.GetSortedSelectionOrCursorAbsolute();

	PE_column const& curcol = editorInner.GetColumnAtCursor();

	for (int j = 0; j < editorInner.columns.size(); ++j) {
		PE_column const& col = editorInner.GetColumn(j);

		if (col.plugin_id == curcol.plugin_id && col.group == curcol.group && col.track == curcol.track) {
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, col.plugin_id);
			zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_get_column(patternformat, plugin, col.group, col.track, col.column);

			int collapse = zzub_pattern_format_column_get_collapsed(fmtcol);
			zzub_pattern_format_column_set_collapsed(fmtcol, (collapse != 0) ? 0 : 1);
		}
	}

	zzub_player_history_commit(player, 0, 0, "Toggled Track Collapse");
}

// ---------------------------------------------------------------------------------------------------------------
// PREFERENCES / THEMES
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::BindSettings() {
	BindPatternEditorFont();

	editorInner.SetNoteOffStr(configuration->getNoteOffString());
	editorInner.SetNoteCutStr(configuration->getNoteCutString());
	editorInner.SetBGNote(configuration->getBGNote());
	editorInner.SetBGByte(configuration->getBGByte());
	editorInner.SetBGSwitch(configuration->getBGSwitch());
	editorInner.SetBGWord(configuration->getBGWord());
	editorInner.SetStickySelection(configuration->getStickySelections());
	editorInner.SetColoredNotes(configuration->getColoredNotes());
	editorInner.SetVerticalScrollMode(configuration->getVerticalScrollMode());
	editorInner.SetHorizontalScrollMode(configuration->getHorizontalScrollMode());
	editorInner.SetHorizontalEntry(configuration->getDefaultEntryMode());
	editorInner.SetNotesAffectMode(configuration->getNotesAffectMode());
	editorControl.subrow_mode = configuration->getSubrowNamingMode();
	follow_mode = configuration->getPatternFollowMode();
	show_infopane = configuration->getShowInfoPane();

	{	int old_trigger_width = trigger_width;
		trigger_width = configuration->getTriggerWidth();
		if (trigger_width != old_trigger_width) {
			dirtyPatternEditor = true;
		}
	}

	editorInner.Invalidate(FALSE);
	editorControl.Invalidate(FALSE);

	if (!InvalidPattern())
		editorInner.BindPatternImg();

	Invalidate(FALSE);
}

void CPatternView::BindTheme() {
	buze_application_t* application = buze_main_frame_get_application(mainframe);
	editorControl.SetThemeColor(PE_BG, buze_application_get_theme_color(application, "PE BG"));
	editorControl.SetThemeColor(PE_BG_Dark, buze_application_get_theme_color(application, "PE BG Dark"));
	editorControl.SetThemeColor(PE_BG_VeryDark, buze_application_get_theme_color(application, "PE BG Very Dark"));
	editorControl.SetThemeColor(PE_Selection, buze_application_get_theme_color(application, "PE Selection"));
	editorControl.SetThemeColor(PE_Cursor, buze_application_get_theme_color(application, "PE Cursor"));
	editorControl.SetThemeColor(PE_TextValue, buze_application_get_theme_color(application, "PE Text Value"));
	editorControl.SetThemeColor(PE_TextNote, buze_application_get_theme_color(application, "PE Text Note"));
	editorControl.SetThemeColor(PE_TextNoteOff, buze_application_get_theme_color(application, "PE Text Note Off"));
	editorControl.SetThemeColor(PE_TextTrigger, buze_application_get_theme_color(application, "PE Text Trigger"));
	editorControl.SetThemeColor(PE_TextWave, buze_application_get_theme_color(application, "PE Text Wave"));
	editorControl.SetThemeColor(PE_TextVolume, buze_application_get_theme_color(application, "PE Text Volume"));
	editorControl.SetThemeColor(PE_TextShade, buze_application_get_theme_color(application, "PE Text Shade"));
	editorControl.SetThemeColor(PE_TextRows, buze_application_get_theme_color(application, "PE Text Rows"));
	editorControl.SetThemeColor(PE_TextTrack, buze_application_get_theme_color(application, "PE Text Track"));
	editorControl.SetThemeColor(PE_TextTrackMuted, buze_application_get_theme_color(application, "PE Text Track Muted"));
	editorControl.SetThemeColor(PE_TextTrackMuted_BG, buze_application_get_theme_color(application, "PE Text Track Muted BG"));
	editorControl.SetThemeColor(PE_LoopPoints, buze_application_get_theme_color(application, "PE Loop Points"));
	editorControl.SetThemeColor(PE_LoopPoints_Disabled, buze_application_get_theme_color(application, "PE Loop Points Disabled"));
	editorControl.SetThemeColor(PE_PlaybackPos, buze_application_get_theme_color(application, "PE Playback Pos"));
	editorControl.SetThemeColor(PE_Divider, buze_application_get_theme_color(application, "PE Divider"));
	editorControl.SetThemeColor(PE_Hidden, buze_application_get_theme_color(application, "PE Hidden"));
	editorControl.SetThemeColor(PE_Control, buze_application_get_theme_color(application, "PE Control"));
	editorControl.SetThemeColor(PE_Trigger, buze_application_get_theme_color(application, "PE Trigger"));
	editorControl.SetThemeColor(PE_Trigger_Shadow, buze_application_get_theme_color(application, "PE Trigger Shadow"));
	editorControl.SetThemeColor(PE_Trigger_Highlight, buze_application_get_theme_color(application, "PE Trigger Highlight"));
	editorControl.SetThemeColor(PE_Note_1, buze_application_get_theme_color(application, "PE Text Note 1"));
	editorControl.SetThemeColor(PE_Note_2, buze_application_get_theme_color(application, "PE Text Note 2"));
	editorControl.SetThemeColor(PE_Note_3, buze_application_get_theme_color(application, "PE Text Note 3"));
	editorControl.SetThemeColor(PE_Note_4, buze_application_get_theme_color(application, "PE Text Note 4"));
	editorControl.SetThemeColor(PE_Note_5, buze_application_get_theme_color(application, "PE Text Note 5"));
	editorControl.SetThemeColor(PE_Note_6, buze_application_get_theme_color(application, "PE Text Note 6"));
	editorControl.SetThemeColor(PE_Note_7, buze_application_get_theme_color(application, "PE Text Note 7"));
	editorControl.SetThemeColor(PE_Note_8, buze_application_get_theme_color(application, "PE Text Note 8"));
	editorControl.SetThemeColor(PE_Note_9, buze_application_get_theme_color(application, "PE Text Note 9"));
	editorControl.SetThemeColor(PE_Note_10, buze_application_get_theme_color(application, "PE Text Note 10"));
	editorControl.SetThemeColor(PE_Note_11, buze_application_get_theme_color(application, "PE Text Note 11"));
	editorControl.SetThemeColor(PE_Note_12, buze_application_get_theme_color(application, "PE Text Note 12"));

	editorInner.UpdateTheme();
	editorInner.UpdateCaret();
	editorInner.Invalidate(FALSE);
	editorControl.Invalidate(FALSE);

	editorScroller.BlankPatternImg();
	editorScroller.BlankBackbuffers();
	if (!InvalidPattern())
		editorInner.BindPatternImg();
	editorScroller.Invalidate(FALSE);
}

void CPatternView::BindPatternEditorFont() {
	string new_font_name = buze_configuration_get_fixed_width_font(configuration.configuration);
	int new_font_size = 9;
	//if (configuration->getConfigString("Settings", "FixedWidthFont", &new_font_name))
	configuration->getNumber("Settings", "FixedWidthFontSize", (DWORD*)&new_font_size);
	SetPatternEditorFont(new_font_name, new_font_size);
}

void CPatternView::SetPatternEditorFont(std::string const& new_font_name, int new_font_size) {
	if ((new_font_name != font_name) || (new_font_size != font_size)) {
		font_name = new_font_name;
		font_size = new_font_size;
		CFontHandle font;
		font.CreateFont(font_size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH|FF_ROMAN, font_name.c_str());
		editorControl.SetFont(font);
	}
}

void CPatternView::OnToggleHorizontalEntry() {
	editorInner.horizontal_entry = !editorInner.horizontal_entry;
}

// ---------------------------------------------------------------------------------------------------------------
// TOOLBAR
// ---------------------------------------------------------------------------------------------------------------

// --- Binding ---

DWORD WINAPI BindToolbarThread(LPVOID lpParam) {
	CPatternView* view = (CPatternView*)lpParam;
	view->BindToolbarControls();
	return 0;
}

void CPatternView::BindToolbarControls() {
	BindWaveDropdown();
	BindMachinePatternPanel();
	BindToolbar();
}

void CPatternView::BindToolbar()
{
	octaveDropDown.SetRedraw(FALSE);
	stepDropDown.SetRedraw(FALSE);
	patternscaleDropDown.SetRedraw(FALSE);
	patternbeatDropDown.SetRedraw(FALSE);
	patternrowsDropDown.SetRedraw(FALSE);

	// octaveDropDown.ResetContent();
	// stepDropDown.ResetContent();
	// patternscaleDropDown.ResetContent();
	// patternbeatDropDown.ResetContent();

	///octaveDropDown.ctrl().SetItemHeight(-1, 15);

	octaveDropDown.ctrl().InitStorage(10, 4);
	stepDropDown.ctrl().InitStorage(16, 4);
	patternscaleDropDown.ctrl().InitStorage(scales_count, 8);
	patternbeatDropDown.ctrl().InitStorage(all_beats_count, 12);
	patternrowsDropDown.ctrl().InitStorage(pattern_sizes_count, 8);///...nbytes

	for (int i = 1; i <= 9; ++i) {
		std::string o = stringFromInt(i);
		octaveDropDown.ctrl().InsertString(i - 1, o.c_str());
	}

	for (int i = 0; i <= 16; ++i) {
		std::string o = stringFromInt(i);
		stepDropDown.ctrl().InsertString(i, o.c_str());
	}

	for (int i = 0; i < scales_count; ++i) {
		std::string o = stringFromInt(scales[i]);
		patternscaleDropDown.ctrl().InsertString(i, o.c_str());
	}

	// see also CPatternEditor::getRowColor() for more beat stuff
	for (int i = 0; i < all_beats_count; ++i) {
		beat_type const& bt = all_beats[i];
		std::stringstream strm;
		strm << bt.verydark << " / " << bt.dark;
		patternbeatDropDown.ctrl().InsertString(i, strm.str().c_str());
	}

	for (int i = 0; i < pattern_sizes_count; ++i) {
		std::string o = stringFromInt(pattern_sizes[i]);
		patternrowsDropDown.ctrl().InsertString(i, o.c_str());
	}

	octaveDropDown.SetRedraw();
	stepDropDown.SetRedraw();
	patternscaleDropDown.SetRedraw();
	patternbeatDropDown.SetRedraw();
	patternrowsDropDown.SetRedraw();
}

void CPatternView::BindMachinePatternPanel() {
	machinePatternPanel.SetRedraw(FALSE);
	machinePatternPanel.combo1.ResetContent();
	machinePatternPanel.combo2.ResetContent();

	formats.clear();
	patterns.clear();

	int selformatindex = -1;
	int i = 0;

	zzub_pattern_format_iterator_t* fit = zzub_player_get_pattern_format_iterator(player);
	while (zzub_pattern_format_iterator_valid(fit)) {
		zzub_pattern_format_t* format = zzub_pattern_format_iterator_current(fit);
		if (format == patternformat) selformatindex = i;

		const char* name = zzub_pattern_format_get_name(format);
		machinePatternPanel.InsertString(0, machinePatternPanel.GetCount(0), name);
		formats.push_back(format);
		zzub_pattern_format_iterator_next(fit);
		++i;
	}
	zzub_pattern_format_iterator_destroy(fit);

	// fill in pattern names
	int selpatindex = -1;

	// TODO: zzub_pattern_format_get_pattern_iterator() to return patterns using a format
	zzub_pattern_iterator_t* pit = zzub_player_get_pattern_iterator(player);
	while (zzub_pattern_iterator_valid(pit)) {
		zzub_pattern_t* pat = zzub_pattern_iterator_current(pit);
		assert(pat != 0);
		zzub_pattern_format_t* format = zzub_pattern_get_format(pat);
		if (format == patternformat) {
			if (pat == pattern) selpatindex = machinePatternPanel.GetCount(1);
			const char* name = zzub_pattern_get_name(pat);
			machinePatternPanel.InsertString(1, machinePatternPanel.GetCount(1), name);
			patterns.push_back(pat);
		}
		zzub_pattern_iterator_next(pit);
	}
	zzub_pattern_iterator_destroy(pit);

	machinePatternPanel.SetRedraw();

	/*if (selformatindex != -1) */machinePatternPanel.SetCurSel(0, selformatindex);
	/*if (selpatindex != -1) */machinePatternPanel.SetCurSel(1, selpatindex);
}

// --- Play Notes ---

void CPatternView::OnTogglePlayNotes() {
	play_notes = !play_notes;
	playNotesCheckbox.ctrl().SetCheck(play_notes);
}

void CPatternView::UpdatePlayNotesCheckbox() {
	if (playNotesCheckbox.ctrl().GetCheck() != play_notes)
		playNotesCheckbox.ctrl().SetCheck(play_notes ? TRUE : FALSE);
}

// --- Show Info Pane ---

void CPatternView::OnToggleShowInfo() {
	show_infopane = infoCheckbox.ctrl().GetCheck();
	show_infopane = (show_infopane + 1) % 3;
	infoCheckbox.ctrl().SetCheck(show_infopane);

	if (show_infopane == BST_UNCHECKED)
		infoSplitter.SetSinglePaneMode(SPLIT_PANE_LEFT);
	else
	if (show_infopane == BST_CHECKED)
		infoSplitter.SetSinglePaneMode(SPLIT_PANE_NONE);
	else
	if (show_infopane == BST_INDETERMINATE)
		CheckColumnInfoVisibility();

	configuration->setShowInfoPane(show_infopane);
}

void CPatternView::UpdateShowInfoCheckbox() {
	if (infoCheckbox.ctrl().GetCheck() != show_infopane)
		infoCheckbox.ctrl().SetCheck(show_infopane);
}

// --- Pattern Loop ---

void CPatternView::OnTogglePatternLoop() {
	patternloopCheckbox.ctrl().SetCheck(!editorInner.loop_enabled);
	zzub_pattern_set_loop_enabled(pattern, !editorInner.loop_enabled);
	zzub_player_history_commit(player, 0, 0, "Toggle Pattern Loop");
}

void CPatternView::UpdatePatternLoopCheckbox() {
	if (patternloopCheckbox.ctrl().GetCheck() != editorInner.loop_enabled)
		patternloopCheckbox.ctrl().SetCheck(editorInner.loop_enabled);
}

// --- Follow ---

void CPatternView::OnToggleFollow() {
	if (is_primary) {
		follow_mode = !follow_mode;
		UpdateFollowCheckbox();
		configuration->setPatternFollowMode(follow_mode);
		if (orderlist_enabled)
			orderList.SyncOrderToPlayer(zzub_player_get_position_order(player));
	}
}

void CPatternView::UpdateFollowCheckbox() {
	followCheckbox.ctrl().SetCheck(follow_mode);
}

// --- Octave ---

void CPatternView::OnDropdownOctave() {
	octaveDropDown.ctrl().ShowDropDown();
	octaveDropDown.ctrl().SetFocus();
}

void CPatternView::OnOctaveSelChange() {
	int sel = octaveDropDown.ctrl().GetCurSel();
	if (sel == -1) return;
	// 	char pc[32];
	// 	octaveDropDown.ctrl().GetLBText(sel, pc);
	// 	editorControl.SetOctave(atoi(pc));
	editorInner.SetOctave(sel + 1);
}

void CPatternView::OnOctaveUp() {
	if (editorInner.octave < 8) {
		editorInner.SetOctave(editorInner.octave + 1);
	}
	UpdateOctaveDropdown();
}

void CPatternView::OnOctaveDown() {
	if (editorInner.octave > 1) {
		editorInner.SetOctave(editorInner.octave - 1);
	}
	UpdateOctaveDropdown();
}

void CPatternView::UpdateOctaveDropdown() {
	//	std::string o = stringFromInt(editorInner.octave);
	//	octaveDropDown.ctrl().SelectString(-1, o.c_str());
	int sel = editorInner.octave - 1;
	if (octaveDropDown.ctrl().GetCurSel() != sel)
		octaveDropDown.ctrl().SetCurSel(sel);
}

// --- Machine Pattern Panel ---

LRESULT CPatternView::OnMachinePatternSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/) {
	int changedCombo = machinePatternPanel.GetComboIndex(hWndCtl);
	if (changedCombo == -1) return 0;

	int formatIndex = machinePatternPanel.GetCurSel(0);
	int patternIndex = machinePatternPanel.GetCurSel(1);

	zzub_pattern_format_t* new_fmt = 0;
	zzub_pattern_t* new_pat = 0;

	zzub_pattern_format_t* patfmt = zzub_player_get_pattern_format_by_index(player, formatIndex);
	if (changedCombo == 0) {
		// changed format - select last viewed pattern of that format, or the first pattern of this format
		std::map<zzub_pattern_format_t*, zzub_pattern_t*>::iterator it = format_last_viewed_pattern.find(patfmt);
		if (it != format_last_viewed_pattern.end()) {
			new_pat = it->second;
			new_fmt = it->first;
		} else {
			new_pat = get_pattern_by_format_index(player, patfmt, 0);
			new_fmt = patfmt;
		}
	} else {
		// changed pattern - select pattern
		new_pat = get_pattern_by_format_index(player, patfmt, patternIndex);
		new_fmt = 0;
	}

	if (new_pat != pattern)
		SetPatternUser(new_pat, new_fmt);

	return 0;
}

void CPatternView::OnDropdownFormat() {
	machinePatternPanel.combo1.ShowDropDown();
	machinePatternPanel.combo1.SetFocus();
}

void CPatternView::OnDropdownPattern() {
	machinePatternPanel.combo2.ShowDropDown();
	machinePatternPanel.combo2.SetFocus();
}

void CPatternView::OnPatternUp() {
	if (patterns.size() == 0) return;

	int sel = (machinePatternPanel.GetCurSel(1) + 1) % patterns.size();
	zzub_pattern_t* new_pat = patterns[sel];
	if (new_pat != pattern)
		SetPatternUser(new_pat);
}

void CPatternView::OnPatternDown() {
	if (patterns.size() == 0) return;

	int sel = (machinePatternPanel.GetCurSel(1) - 1 + patterns.size()) % patterns.size();
	zzub_pattern_t* new_pat = patterns[sel];
	if (new_pat != pattern)
		SetPatternUser(new_pat);
}

void CPatternView::OnFormatUp() {
	if (formats.size() == 0) return;

	int sel = (machinePatternPanel.GetCurSel(0) + formats.size() - 1) % formats.size();

	zzub_pattern_format_t* new_fmt = 0;
	zzub_pattern_t* new_pat = 0;

	std::map<zzub_pattern_format_t*, zzub_pattern_t*>::iterator i = format_last_viewed_pattern.find(formats[sel]);
	if (i != format_last_viewed_pattern.end()) {
		new_pat = i->second;
		new_fmt = i->first;
	} else {
		new_pat = get_pattern_by_format_index(player, formats[sel], 0);
		new_fmt = formats[sel];
	}

	if (new_pat != pattern)
		SetPatternUser(new_pat, new_fmt);
}

void CPatternView::OnFormatDown() {
	if (formats.size() == 0) return;

	int sel = (machinePatternPanel.GetCurSel(0) + 1) % formats.size();

	zzub_pattern_format_t* new_fmt = 0;
	zzub_pattern_t* new_pat = 0;

	std::map<zzub_pattern_format_t*, zzub_pattern_t*>::iterator i = format_last_viewed_pattern.find(formats[sel]);
	if (i != format_last_viewed_pattern.end()) {
		new_pat = i->second;
		new_fmt = i->first;
	} else {
		new_pat = get_pattern_by_format_index(player, formats[sel], 0);
		new_fmt = formats[sel];
	}

	if (new_pat != pattern)
		SetPatternUser(new_pat, new_fmt);
}

// --- Wave ---

int CPatternView::GetSelectedWave() {
	int curSel = waveDropDown.ctrl().GetCurSel();

	if (curSel == LB_ERR) return -1;
	return waveDropDown.ctrl().GetItemData(curSel);
}

void CPatternView::BindWaveDropdown() {
	waveDropDown.SetRedraw(FALSE);

	int curSel = waveDropDown.ctrl().GetCurSel();
	char pcSelectedWave[1024];
	strcpy(pcSelectedWave, "");
	if (curSel != LB_ERR) {
		waveDropDown.ctrl().GetLBText(curSel, pcSelectedWave);
	}

	waveDropDown.ctrl().ResetContent();
	waveDropDown.ctrl().InitStorage(200, 128);

	char pcWaveName[1024];
	int count = zzub_player_get_wave_count(player);
	int listindex = 0;
	for (int i = 0; i < count; ++i) {
		zzub_wave_t* wave = zzub_player_get_wave(player, i);
		int levelcount = zzub_wave_get_level_count(wave);
		if (levelcount) {
			sprintf(pcWaveName, "%02X. %s", i+1, zzub_wave_get_name(wave));
			int item = waveDropDown.ctrl().InsertString(listindex, pcWaveName);
			waveDropDown.ctrl().SetItemData(item, i+1);
			listindex++;
			if (curSel == LB_ERR) {
				strcpy(pcSelectedWave, pcWaveName);
				curSel=0;
			}
		}
	}
	if (curSel != LB_ERR) {
		waveDropDown.ctrl().SelectString(-1, pcSelectedWave);
	}

	waveDropDown.SetRedraw(TRUE);
}

void CPatternView::OnDropdownWave() {
	waveDropDown.ctrl().ShowDropDown();
	waveDropDown.ctrl().SetFocus();
}

void CPatternView::OnNextWave() {
	if (waveDropDown.ctrl().GetCurSel() < waveDropDown.ctrl().GetCount()-1) {
		waveDropDown.ctrl().SetCurSel(waveDropDown.ctrl().GetCurSel()+1);
	}
}

void CPatternView::OnPrevWave() {
	if (waveDropDown.ctrl().GetCurSel() > 0) {
		waveDropDown.ctrl().SetCurSel(waveDropDown.ctrl().GetCurSel()-1);
	}
}

void CPatternView::SelectWave(int patwave) {
	if ((patwave > 0) && (patwave <= zzub_wavetable_index_value_max)) {
		zzub_wave_t* wave = zzub_player_get_wave(player, patwave - 1);
		int wave_index = 0;
		for (size_t i = 0; patwave > 0 && i < zzub_player_get_wave_count(player); ++i) {
			zzub_wave_t* testwave = zzub_player_get_wave(player, i);
			int levelcount = zzub_wave_get_level_count(testwave);
			if (levelcount) {
				if (testwave == wave) {
					waveDropDown.ctrl().SetCurSel(wave_index);
					break;
				}
				++wave_index;
			}
		}
	}
}

// --- Step ---

void CPatternView::OnDropdownStep() {
	//stepDropDown.ctrl().ShowDropDown();
	stepDropDown.ctrl().SetFocus();
}

void CPatternView::OnStepSelChange() { // TODO: use array
	int sel = stepDropDown.ctrl().GetCurSel();
	if (sel == -1) return;
	editorInner.SetStep(sel);
}

void CPatternView::OnStepUp() {
	if (editorInner.step < 16) {
		editorInner.step++;
	}
	UpdateStepDropdown();
}

void CPatternView::OnStepDown() {
	if (editorInner.step > 0) {
		editorInner.step--;
	}
	UpdateStepDropdown();
}

void CPatternView::OnSetStepRange(WORD wID) {
	int step = wID - ID_PATTERNVIEW_STEP_SET_0;
	if (step < 0 || step > 9) return;
	editorInner.step = step;
	UpdateStepDropdown();
}

void CPatternView::UpdateStepDropdown() {
	int step = editorInner.step;
	if (step != stepDropDown.ctrl().GetNumber()) {
		if (step <= 16)
			stepDropDown.ctrl().SetCurSel(step);
		else
			stepDropDown.ctrl().SetNumber(step);
	}
}

void CPatternView::OnStepKillFocus() {
	OnStepEditAccept(false);
}

void CPatternView::OnStepEditAccept(bool restorefocus) {
	int step = stepDropDown.ctrl().GetNumber();
	editorInner.SetStep(step);
	if (restorefocus)
		OnRestoreFocus();
}

void CPatternView::OnStepEditCancel() {
	UpdateStepDropdown();
	OnRestoreFocus();
}

// --- Pattern Scale ---

void CPatternView::OnDropdownPatternScale() {
	///patternscaleDropDown.ctrl().ShowDropDown();
	patternscaleDropDown.ctrl().SetFocus();
}

void CPatternView::UpdateScaleDropdown() {
	int skip = editorInner.GetSkip();
	int sel_skip = patternscaleDropDown.ctrl().GetNumber();
	if (skip != sel_skip) {
		patternscaleDropDown.ctrl().SetNumber(skip);
	}
}

void CPatternView::OnScaleSelChange() {
	int sel = patternscaleDropDown.ctrl().GetCurSel();
	if (sel == -1) return;
	// TODO: set_display_resolution? per-pattern-thing?
	zzub_pattern_set_display_resolution(pattern, scales[sel]);
	zzub_player_history_commit(player, 0, 0, "Change Pattern Display Resolution");
}

void CPatternView::OnScaleKillFocus() {
	OnScaleEditAccept(false);
}

void CPatternView::OnScaleEditAccept(bool restorefocus /*=true*/) {
	int skip = patternscaleDropDown.ctrl().GetNumber();
	if (skip != editorInner.GetSkip()) {
		if (skip == 0) {
			UpdateScaleDropdown();
		} else {
			zzub_pattern_set_display_resolution(pattern, skip);
			zzub_player_history_commit(player, 0, 0, "Change Pattern Display Resolution");
		}
	}

	if (restorefocus)
		OnRestoreFocus();
}

void CPatternView::OnScaleEditCancel() {
	UpdateScaleDropdown();
	OnRestoreFocus();
}

// --- Pattern Beat ---

void CPatternView::OnDropdownPatternBeat() {
	patternbeatDropDown.ctrl().ShowDropDown();
	patternbeatDropDown.ctrl().SetFocus();
}

void CPatternView::OnSelChangeBeat() {
	int sel = patternbeatDropDown.ctrl().GetCurSel();
	if (sel == -1) return;
	beat_type const& bt = all_beats[sel];

	zzub_pattern_set_display_beat_rows(pattern, bt.verydark, bt.dark);
	zzub_player_history_commit(player, 0, 0, "Change Pattern Display Beat");
}

void CPatternView::UpdateBeatDropdown() {
	for (int i = 0; i < all_beats_count; ++i) {
		beat_type const& bt = all_beats[i];
		if ((bt.verydark == editorInner.verydark_row) && (bt.dark == editorInner.dark_row)) {
			if (patternbeatDropDown.ctrl().GetCurSel() != i)
				patternbeatDropDown.ctrl().SetCurSel(i);
			break;
		}
	}
}

// --- Pattern Rows ---

void CPatternView::OnDropdownPatternRows() {
	patternrowsDropDown.ctrl().ShowDropDown();
	patternrowsDropDown.ctrl().SetFocus();
}

void CPatternView::UpdatePatternRowsDropdown() {
	int rows = editorInner.GetPatternRows();
	int sel_rows = patternrowsDropDown.ctrl().GetNumber();
	if (rows != sel_rows) {
		patternrowsDropDown.ctrl().SetNumber(rows);
	}
}

void CPatternView::OnPatternRowsSelChange() {
	int sel = patternrowsDropDown.ctrl().GetCurSel();
	if (sel == -1) return;
	zzub_pattern_set_row_count(pattern, pattern_sizes[sel]);
	zzub_player_history_commit(player, 0, 0, "Change Pattern Length");
}

void CPatternView::OnPatternRowsKillFocus() {
	OnPatternRowsEditAccept(false);
}

void CPatternView::OnPatternRowsEditAccept(bool restorefocus /*=true*/) {
	int rows = patternrowsDropDown.ctrl().GetNumber();
	if (rows != editorInner.GetPatternRows()) {
		if (rows == 0) {
			UpdatePatternRowsDropdown();
		} else {
			zzub_pattern_set_row_count(pattern, rows);
			zzub_player_history_commit(player, 0, 0, "Change Pattern Length");
		}
	}

	if (restorefocus)
		OnRestoreFocus();
}

void CPatternView::OnPatternRowsEditCancel() {
	UpdatePatternRowsDropdown();
	OnRestoreFocus();
}

// --- Pattern Name ---

void CPatternView::OnDropdownPatternName() {
	patternnameEdit.ctrl().SetFocus();
}

void CPatternView::UpdatePatternNameEdit() {
	if (pattern == 0) {
		patternnameEdit.ctrl().SetWindowText("");
		return;
	}

	const char* name = zzub_pattern_get_name(pattern);

	char current_name[1024];
	patternnameEdit.ctrl().GetWindowText(current_name, array_size(current_name));

	if (strcmp(name, current_name) != 0)
		patternnameEdit.ctrl().SetWindowText(name);
}

void CPatternView::OnPatternNameKillFocus() {
	OnPatternNameEditAccept(false);
}

void CPatternView::OnPatternNameEditAccept(bool restorefocus /*=true*/) {
	char old_name[1024];
	strncpy(old_name, zzub_pattern_get_name(pattern), array_size(old_name));

	char new_name[1024];
	patternnameEdit.ctrl().GetWindowText(new_name, array_size(new_name));

	if (strcmp(old_name, new_name) != 0) {
		if (strlen(new_name) == 0) {
			UpdatePatternNameEdit();
		} else {
			zzub_pattern_t* pat = zzub_player_get_pattern_by_name(player, new_name);
			if (pat == 0) { // disallow dupes
				zzub_pattern_set_name(pattern, new_name);
				zzub_player_history_commit(player, 0, 0, "Rename Pattern");
			} else {
				UpdatePatternNameEdit();
			}
		}
	}

	if (restorefocus)
		OnRestoreFocus();
}

void CPatternView::OnPatternNameEditCancel() {
	UpdatePatternNameEdit();
	OnRestoreFocus();
}

// --- Enabling / Disabling ---

void CPatternView::EnableToolbands() {
	patternrowsDropDown.ctrl().ModifyStyle(WS_DISABLED, 0);
	patternscaleDropDown.ctrl().ModifyStyle(WS_DISABLED, 0);
	patternbeatDropDown.ctrl().ModifyStyle(WS_DISABLED, 0);
	patternnameEdit.ctrl().ModifyStyle(WS_DISABLED, 0);
	patternloopCheckbox.ctrl().ModifyStyle(WS_DISABLED, 0);
}

void CPatternView::DisableToolbands() {
	patternrowsDropDown.ctrl().ClearNumber();
	patternrowsDropDown.ctrl().ModifyStyle(0, WS_DISABLED);

	patternscaleDropDown.ctrl().ClearNumber();
	patternscaleDropDown.ctrl().ModifyStyle(0, WS_DISABLED);

	patternbeatDropDown.ctrl().SetCurSel(-1);
	patternbeatDropDown.ctrl().ModifyStyle(0, WS_DISABLED);

	patternnameEdit.ctrl().SetWindowText("");
	patternnameEdit.ctrl().ModifyStyle(0, WS_DISABLED);

	patternloopCheckbox.ctrl().SetCheck(0);
	patternloopCheckbox.ctrl().ModifyStyle(0, WS_DISABLED);

	//foo.EnableWindow(FALSE); looked different
}

BOOL CPatternView::GetToolbarVisibility(WORD wID) {
	return configuration->getToolbarVisible(wID, true);
}


// --- Focus ---

void CPatternView::OnRestoreFocus() {
	editorControl.SetFocus();
}

// ---------------------------------------------------------------------------------------------------------------
// LOOPS
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnSetLoopBegin() {
	if (InvalidPattern()) return;

	POINT cursor = editorInner.GetCursorAbsolute();
	int begin_loop = cursor.y;

	int cur_loop_end = zzub_pattern_get_loop_end(pattern);
	int pattern_rows = zzub_pattern_get_row_count(pattern);

	if (begin_loop >= cur_loop_end)
		zzub_pattern_set_loop_end(pattern, pattern_rows);

	zzub_pattern_set_loop_start(pattern, begin_loop);

	zzub_player_history_commit(player, 0, 0, "Set Loop Start");
}

void CPatternView::OnSetLoopEnd() {
	if (InvalidPattern()) return;

	POINT cursor = editorInner.GetCursorAbsolute();
	int end_loop = cursor.y + editorInner.GetSkip();

	int cur_loop_start = zzub_pattern_get_loop_start(pattern);

	if (end_loop <= cur_loop_start)
		zzub_pattern_set_loop_start(pattern, 0);

	zzub_pattern_set_loop_end(pattern, end_loop);

	zzub_player_history_commit(player, 0, 0, "Set Loop End");
}

void CPatternView::OnSetLoopSelection() {
	if (InvalidPattern()) return;

	RECT rcSel;
	bool has_sel = editorInner.GetSortedSelectionRectAbsolute(&rcSel);

	int begin_loop;
	int end_loop;
	if (has_sel) {
		begin_loop = rcSel.top;
		end_loop = rcSel.bottom + 1;
	} else {
		begin_loop = 0;
		end_loop = editorInner.GetPatternRows();
	}

	zzub_pattern_set_loop_start(pattern, begin_loop);
	zzub_pattern_set_loop_end(pattern, end_loop);

	zzub_player_history_commit(player, 0, 0, "Set Loop Selection");
}

void CPatternView::OnSetLoopPattern() {
	if (InvalidPattern()) return;

	int begin_loop = 0;
	int end_loop = editorInner.GetPatternRows();

	zzub_pattern_set_loop_start(pattern, 0);
	zzub_pattern_set_loop_end(pattern, end_loop);

	zzub_player_history_commit(player, 0, 0, "Set Loop Pattern");
}

// ---------------------------------------------------------------------------------------------------------------
// MUTE / SOLO
// ---------------------------------------------------------------------------------------------------------------

// lResult = func(HIWORD(wParam), LOWORD(wParam), (HWND)lParam, bHandled);
LRESULT CPatternView::OnControlTrackMute(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (InvalidPattern()) return 0;

	int clicked_idx = (int)wNotifyCode;
	DoTrackMute(clicked_idx);

	return 0;
}

LRESULT CPatternView::OnControlTrackSolo(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (InvalidPattern()) return 0;

	int clicked_idx = (int)wNotifyCode;
	DoTrackSolo(clicked_idx);

	return 0;
}

void CPatternView::OnTrackMute() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();

	for (int i = 0; i < editorInner.tracks.size(); ++i) {
		PE_track const& track = editorInner.tracks[i];

		if ((col.plugin_id == track.plugin_id) && (col.group == track.group) && (col.track == track.track)) {
			DoTrackMute(i);
			break;
		}
	}	
}

void CPatternView::OnTrackSolo() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();

	for (int i = 0; i < editorInner.tracks.size(); ++i) {
		PE_track const& track = editorInner.tracks[i];

		if ((col.plugin_id == track.plugin_id) && (col.group == track.group) && (col.track == track.track)) {
			DoTrackSolo(i);
			break;
		}
	}	
}

void CPatternView::DoTrackMute(int current_idx) {
	PE_track const& track = editorInner.tracks[current_idx];

	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, track.plugin_id);
	int is_muted = zzub_pattern_format_get_track_mute(patternformat, plugin, track.group, track.track);

	is_muted = !is_muted;

	zzub_pattern_format_set_track_mute(patternformat, plugin, track.group, track.track, is_muted);

	zzub_player_history_commit(player, 0, 0, is_muted ? "Muted Format Track" : "Unmuted Format Track");
}

void CPatternView::DoTrackSolo(int current_idx) {
	PE_track const& current_track = editorInner.tracks[current_idx];
	zzub_plugin_t* current_plugin = zzub_player_get_plugin_by_id(player, current_track.plugin_id);
	int current_is_muted = zzub_pattern_format_get_track_mute(patternformat, current_plugin, current_track.group, current_track.track);

	bool do_solo = true;

	if (!current_is_muted) { // check if this track is unmuted, but all others are muted
		do_solo = false;	

		for (int i = 0; i < editorInner.tracks.size(); ++i) {
			PE_track const& track = editorInner.tracks[i];
			zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, track.plugin_id);
			int is_muted = zzub_pattern_format_get_track_mute(patternformat, plugin, track.group, track.track);

			if (!is_muted && (i != current_idx)) {
				do_solo = true;
				break;
			}
		}
	}

	for (int i = 0; i < editorInner.tracks.size(); ++i) {
		PE_track const& track = editorInner.tracks[i];
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, track.plugin_id);
		int is_muted = (do_solo && (i != current_idx));
		zzub_pattern_format_set_track_mute(patternformat, plugin, track.group, track.track, is_muted);
	}

	zzub_player_history_commit(player, 0, 0, do_solo ? "Soloed Format Track" : "Unsoloed Format Track");
}

void CPatternView::OnMachineSolo() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	assert(plugin != 0);

	if (buze_document_get_solo_plugin(document) == plugin)
		buze_document_set_solo_plugin(document, plugin, FALSE);
	else
		buze_document_set_solo_plugin(document, plugin, TRUE);

	zzub_player_history_commit(player, 0, 0, "Solo Plugin");
}

void CPatternView::OnMachineMute() {
	if (InvalidPattern()) return;

	PE_cursor_pos pos = editorInner.GetCursorPos();
	zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, pos.plugin_id);
	assert(plugin != 0);

	bool muted = zzub_plugin_get_mute(plugin);
	if (muted)
		zzub_plugin_set_mute(plugin, 0);
	else
		zzub_plugin_set_mute(plugin, 1);

	zzub_player_history_commit(player, 0, 0, muted ? "Unmute Plugin" : "Mute Plugin");
}

// ---------------------------------------------------------------------------------------------------------------
// ORDERLIST CONTROL
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnToggleOrderlist() {
	/*if (is_primary) {
		SetRedraw(FALSE);
		orderlist_enabled = !orderlist_enabled;
		configuration->setOrderlistEnabled(orderlist_enabled);
		MakeDestroyOrderlist();
		SendMessage(WM_SIZE);
		SetRedraw(TRUE);
		RedrawWindow(0, 0, RDW_INVALIDATE|RDW_ALLCHILDREN);
	}*/
}

void CPatternView::MakeDestroyOrderlist() {
	if (is_primary) {
		if (orderlist_enabled) {
			orderList.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, 0, IDC_PEORDERLIST);
			/// re-init vars
		} else {
			orderList.DestroyWindow();
		}
	}
}

void CPatternView::OnOrderlistRight() {
	if (is_primary && orderlist_enabled)
		orderList.MoveRight();
}

void CPatternView::OnOrderlistLeft() {
	if (is_primary && orderlist_enabled)
		orderList.MoveLeft();
}

// ---------------------------------------------------------------------------------------------------------------
// PATTERN STACK
// ---------------------------------------------------------------------------------------------------------------

void CPatternView::OnResetPatternStack() {
	ResetPatternStack();
}

void CPatternView::ResetPatternStack() {
	pattern_stack.clear();
	pattern_stack.push_back(pattern);
	pattern_stack_pos = 0;
}

void CPatternView::SetPatternResetStack(zzub_pattern_t* newpattern, zzub_pattern_format_t* newformat /*= 0*/) {
	pattern_stack.clear();
	pattern_stack_pos = 0;
	SetPattern(newpattern, newformat);
}

void CPatternView::SetPatternUser(zzub_pattern_t* newpattern, zzub_pattern_format_t* newformat) {
	// TODO: call SetPattern or SetPatternPushStack depending on configuration setting
	// causes the pattern stack to effectively track "last viewed patterns" or "nested patterns"
	if (configuration->getPatternStackMode() == 0)
		SetPattern(newpattern, newformat);
	else
		SetPatternPushStack(newpattern, newformat);
}

void CPatternView::SetPatternPushStack(zzub_pattern_t* newpattern, zzub_pattern_format_t* newformat /*= 0*/) {
	pattern_stack.erase(pattern_stack.begin() + (size_t)(pattern_stack_pos + 1), pattern_stack.end());
	if (pattern != 0 && (pattern_stack[pattern_stack_pos] != newpattern)) {
		pattern_stack.push_back(0);
		++pattern_stack_pos;
	}
	SetPattern(newpattern, newformat);
}

void CPatternView::SetPattern(zzub_pattern_t* newpattern, zzub_pattern_format_t* newformat /*= 0*/) {
	// allow passing 0 or a valid format as newformat
	assert(newpattern == 0 || (newpattern != 0 && (newformat == 0 || zzub_pattern_get_format(newpattern) == newformat)));

	if (pattern_stack.empty()) {
		pattern_stack.push_back(newpattern);
	} else {
		pattern_stack[pattern_stack_pos] = newpattern;
	}

	PatternStackCleanup();

	if (newpattern != 0 && newformat == 0) {
		patternformat = zzub_pattern_get_format(newpattern);
	} else {
		patternformat = newformat;
	}

	format_last_viewed_pattern[patternformat] = newpattern;

	// dont rebind the same pattern
	if ((newpattern != 0) && (false
			|| ((pattern == newpattern) && (newformat == 0))
			|| ((pattern == newpattern) && (patternformat == newformat))
		)
	) return;

	pattern = newpattern;

	BindScrollerWidth();
	dirtyPatternEditor = true;
	dirtyPatternInfos = true;
	dirtyMachinePatternPanel = true;
	dirtyStatus = true;
	dirtySongPositions = true;
	dirtyColumnInfo = true;
	Invalidate(FALSE);


	buze_event_data_t ev;
	ev.show_properties.return_view = this;

	if (pattern != 0) {
		ev.show_properties.type = buze_property_type_pattern;
		ev.show_properties.pattern = pattern;
		buze_document_notify_views(document, this, buze_event_type_update_properties, &ev);
	}
	
	if (patternformat != 0) {
		ev.show_properties.type = buze_property_type_pattern_format;
		ev.show_properties.pattern_format = patternformat;
		buze_document_notify_views(document, this, buze_event_type_update_properties, &ev);
	}
}

// removes pattern 0's and contiguous duplicates
void CPatternView::PatternStackCleanup() {
	zzub_pattern_t* last_pat = 0;

	std::vector<zzub_pattern_t*>::iterator i = pattern_stack.begin();
	for (; i != pattern_stack.end();) {
		int i_pos = (int)std::distance(pattern_stack.begin(), i);

		zzub_pattern_t* pat = *i;

		bool do_remove = false;
		do_remove |= (i_pos != pattern_stack_pos) && (pat == 0);
		do_remove |= (i_pos != 0) && (last_pat == pat);

		last_pat = pat;

		if (do_remove) {
			i = pattern_stack.erase(i);
			if (i_pos <= pattern_stack_pos)
				--pattern_stack_pos;
		} else {
			++i;
		}
	}
}

void CPatternView::PatternStackRemovePattern(zzub_pattern_t* pat) {
	for (;;) {
		std::vector<zzub_pattern_t*>::iterator i = find(pattern_stack.begin(), pattern_stack.end(), pat);
		if (i != pattern_stack.end()) {
			if (pattern_stack_pos > (int)(i - pattern_stack.begin())) {
				--pattern_stack_pos;
			}
			pattern_stack.erase(i);
		} else {
			break;
		}
	}

	{	zzub_pattern_format_t* fmt = zzub_pattern_get_format(pat);
		std::map<zzub_pattern_format_t*, zzub_pattern_t*>::iterator i = format_last_viewed_pattern.find(fmt);
		if (i != format_last_viewed_pattern.end()) {
			if ((*i).second == pat) {
				format_last_viewed_pattern.erase(i);
			}
		}
	}
}

void CPatternView::OnGotoPattern() {
	if (InvalidPattern()) return;
	GotoPattern();
}

void CPatternView::OnCloneGotoPattern() {
	if (InvalidPattern()) return;
	ClonePattern();
	GotoPattern();
}

void CPatternView::GotoPattern() {
	if (InvalidPattern()) return;

	PE_column const& col = editorInner.GetColumnAtCursor();
	int row = editorInner.GetCursorRowAbsolute();

	if (col.defaultcontrol != pattern_column_control_pattern) return;

	int value;
	int length;
	col.GetPatternValue(row, &value, &length);

	if (value == col.novalue) return ;

	zzub_pattern_t* pat = zzub_player_get_pattern_by_id(player, value);
	if (pat == 0) return;

/*	if ((this == mainframe->primaryPatternEditor) && (mainframe->secondaryPatternEditor != 0)) {
		mainframe->secondaryPatternEditor->SetPatternResetStack(pat);
		mainframe->setCurrentFocus(mainframe->secondaryPatternEditor->m_hWnd);
	} else {*/
		SetPatternPushStack(pat);
	/*}*/
}

void CPatternView::OnBack() {
	if (pattern_stack_pos == 0) {
		if (orderlist_enabled)
			orderList.SetFocus();
	} else
	if ((pattern_stack.size() > 0) && (pattern_stack_pos > 0)) {
		--pattern_stack_pos;
		SetPattern(pattern_stack[pattern_stack_pos]);
	}

	/*
	if ((this == mainframe->primaryPatternEditor) && (pattern_stack_pos == 0)) {
// 		if (mainframe->orderlistView.getViewCount() != 0) {
// 			mainframe->frame.setFocusTo(mainframe->orderlistView.getViewHWND(0));
// 		}

		if (orderlist_enabled) {
			orderList.SetFocus();
		}
	} else
	if ((this == mainframe->secondaryPatternEditor) && (pattern_stack_pos == 0)) {
		if (mainframe->primaryPatternEditor != 0) {
			mainframe->setCurrentFocus(mainframe->primaryPatternEditor->m_hWnd);
		}
	} else
	if ((pattern_stack.size() > 0) && (pattern_stack_pos > 0)) {
		--pattern_stack_pos;
		SetPattern(pattern_stack[pattern_stack_pos]);
	}*/
}

void CPatternView::OnForward() {
	/*
	if (pattern_stack_pos == pattern_stack.size()-1) {
		if ((this == mainframe->primaryPatternEditor) && (mainframe->secondaryPatternEditor != 0)) {
			mainframe->setCurrentFocus(mainframe->secondaryPatternEditor->m_hWnd);
		}
	} else*/
	if ((pattern_stack.size() > 0) && (pattern_stack_pos < (int)pattern_stack.size()-1)) {
		++pattern_stack_pos;
		SetPattern(pattern_stack[pattern_stack_pos]);
	}
	
}

// ---------------------------------------------------------------------------------------------------------------
// HELPERS
// ---------------------------------------------------------------------------------------------------------------

bool CPatternView::InvalidPattern() const {
	return (pattern == 0) || (editorInner.GetColumnCount() == 0);
}

void CPatternView::SetToolbarVisibility(int id, bool state) {
	configuration->setToolbarVisible(id, state);
	// if there are more pattern editors, notify them to also update toolbars visibility
	buze_document_notify_views(document, this, buze_event_type_update_settings, 0);
}

void CPatternView::UpdateToolbarFromConfiguration(WORD wID) {
	bool state = GetToolbarVisibility(wID);
	int nBandIndex = toolBar.IdToIndex(wID);
	toolBar.ShowBand(nBandIndex, state);
	UISetCheck(wID, state);
}

void CPatternView::UpdateToolbarsFromConfiguration() {
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_WAVETOOLTOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_MACHINEPATTERNTOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_OCTAVETOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_STEPTOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_PATTERNSCALETOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_PATTERNBEATTOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_PLAYNOTESTOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_BUTTONBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_INFOTOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_PATTERNROWSTOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_PATTERNNAMETOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_PATTERNLOOPTOOLBAR);
	UpdateToolbarFromConfiguration(ID_PATTERNVIEW_FOLLOWTOOLBAR);

	LockBands(configuration->getLockedToolbars());
}

zzub_plugin_t* get_pattern_plugin_by_index(zzub_player_t* player, int index) {
	zzub_plugin_t* result = 0;
	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
	int plugindex = 0;
	while (zzub_plugin_iterator_valid(plugit) != 0) {
		zzub_plugin_t* plug = zzub_plugin_iterator_current(plugit);
		zzub_pluginloader_t* pluginfo = zzub_plugin_get_pluginloader(plug);
		std::string pluguri = zzub_pluginloader_get_uri(pluginfo);

		if (pluguri == "@zzub.org/sequence/pattern") {
			if (plugindex == index) {
				result = plug;
				break;
			}
			plugindex++;
		}

		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	return result;
}

LRESULT CPatternView::OnAddTrigger(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	// ID_PATTERNVIEW_ADD_TRIGGER_FIRST
	if (pattern == 0) return 0;

	zzub_plugin_t* plugin = 0;
	int plugtrack = 0;
	if (wID == ID_PATTERNVIEW_ADD_TRIGGER_NEW) {

		zzub_pluginloader_t* pluginfo = zzub_player_get_pluginloader_by_name(player, "@zzub.org/sequence/pattern");
		assert(pluginfo != 0);
		const char* name = zzub_player_get_new_plugin_name(player, "@zzub.org/sequence/pattern");
		plugin = zzub_player_create_plugin(player, 0, 0, name, pluginfo, 0);
	} else {
		int triggercode = wID - ID_PATTERNVIEW_ADD_TRIGGER_FIRST;
		int plugindex = triggercode / 128;
		plugtrack = triggercode % 128;

		plugin = get_pattern_plugin_by_index(player, plugindex);
	}
	assert(plugin != 0);

	int numtracks = zzub_plugin_get_track_count(plugin, 2);
	if (plugtrack >= numtracks) {
		zzub_plugin_set_track_count(plugin, numtracks + 1);
		plugtrack = numtracks;
	}

	// add a pattern format column to current patternformat.. 

	zzub_pattern_format_add_column(patternformat, plugin, 2, plugtrack, 0, -1);

	zzub_player_history_commit(player, 0, 0, "Add Trigger Column");
	return 0;
}

LRESULT CPatternView::OnAddSimpleFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	int plugindex = wID - ID_PATTERNVIEW_ADD_SIMPLE_FIRST;
	zzub_plugin_t* plug = zzub_player_get_plugin(player, plugindex);
	buze_document_extend_pattern_format(document, patternformat, plug, true);
	zzub_player_history_commit(player, 0, 0, "Extend Pattern Format, Note/Velocity Columns");
	return 0;
}

LRESULT CPatternView::OnAddFullFormat(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	int plugindex = wID - ID_PATTERNVIEW_ADD_FULL_FIRST;
	zzub_plugin_t* plug = zzub_player_get_plugin(player, plugindex);
	buze_document_extend_pattern_format(document, patternformat, plug, false);
	zzub_player_history_commit(player, 0, 0, "Extend Pattern Format, All Columns");
	return 0;
}
