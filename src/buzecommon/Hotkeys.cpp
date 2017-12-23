#include "stdafx.h"
#include "resource.h"

#include "Hotkeys.h"

#include <fstream>
#include <string>
#include <boost/tokenizer.hpp>
#include "utils/utils.h"

// ---------------------------------------------------------------------------------------------------------------
// COMMANDS
// ---------------------------------------------------------------------------------------------------------------

static id_desc_t const id_descs_mainframe[] =
{
	// ------------------------------------------------------------------------------------------------
	// MAINFRAME
	// ------------------------------------------------------------------------------------------------

	{ "play", ID_PLAY },
	{ "playfromcursor", ID_PLAYFROMCURSOR },
	{ "replay", ID_REPLAY },
	{ "record", ID_RECORD },
	{ "stop", ID_STOP },
	{ "device_reset", ID_DEVICE_RESET },

	{ "view_help", ID_VIEW_HELP },
	{ "view_primary", ID_VIEW_PRIMARYPATTERNEDITOR },
	{ "view_secondary", ID_VIEW_SECONDARYPATTERNEDITOR },
	{ "view_machines", ID_VIEW_MACHINES },
	{ "view_wavetable", ID_VIEW_WAVETABLE },
	{ "view_patternformat", ID_VIEW_PATTERNFORMAT },
	{ "view_patternlist", ID_VIEW_PATTERNLIST },
	{ "view_allmachines", ID_VIEW_ALLMACHINES },
	{ "view_files", ID_VIEW_FILES },
	{ "view_comment", ID_VIEW_COMMENT },
	{ "view_showhide_params", ID_VIEW_SHOWHIDEALLPARAMETERFRAMES },
	{ "view_analyzer", ID_VIEW_ANALYZER },
	{ "view_cpumeter", ID_VIEW_CPUMETER },
	{ "view_history", ID_VIEW_HISTORY },
	{ "view_preferences", ID_VIEW_PREFERENCES },

	{ "view_nextpane", ID_NEXT_PANE },
	{ "view_prevpane", ID_PREV_PANE },
	{ "view_closepane", ID_CLOSEPANE },

	{ "file_new", ID_FILE_NEW },
	{ "file_open", ID_FILE_OPEN },
	{ "file_save", ID_FILE_SAVE },

	{ "edit_undo", ID_EDIT_UNDO },
	{ "edit_redo", ID_EDIT_REDO },

	{ "screenset_recall_0", ID_SCREENSET_RECALL_0 },
	{ "screenset_recall_1", ID_SCREENSET_RECALL_1 },
	{ "screenset_recall_2", ID_SCREENSET_RECALL_2 },
	{ "screenset_recall_3", ID_SCREENSET_RECALL_3 },
	{ "screenset_recall_4", ID_SCREENSET_RECALL_4 },
	{ "screenset_recall_5", ID_SCREENSET_RECALL_5 },
	{ "screenset_recall_6", ID_SCREENSET_RECALL_6 },
	{ "screenset_recall_7", ID_SCREENSET_RECALL_7 },
	{ "screenset_recall_8", ID_SCREENSET_RECALL_8 },
	{ "screenset_recall_9", ID_SCREENSET_RECALL_9 },
	{ "screenset_store_0", ID_SCREENSET_STORE_0 },
	{ "screenset_store_1", ID_SCREENSET_STORE_1 },
	{ "screenset_store_2", ID_SCREENSET_STORE_2 },
	{ "screenset_store_3", ID_SCREENSET_STORE_3 },
	{ "screenset_store_4", ID_SCREENSET_STORE_4 },
	{ "screenset_store_5", ID_SCREENSET_STORE_5 },
	{ "screenset_store_6", ID_SCREENSET_STORE_6 },
	{ "screenset_store_7", ID_SCREENSET_STORE_7 },
	{ "screenset_store_8", ID_SCREENSET_STORE_8 },
	{ "screenset_store_9", ID_SCREENSET_STORE_9 },

	{ "keyjazz_octave_up", ID_KEYJAZZ_OCTAVE_UP },
	{ "keyjazz_octave_down", ID_KEYJAZZ_OCTAVE_DOWN },

	{ "bpm_up", ID_MAINFRAME_BPM_UP },
	{ "bpm_down", ID_MAINFRAME_BPM_DOWN },
};

static id_desc_t const id_descs_patternview[] =
{
	// ------------------------------------------------------------------------------------------------
	// PATTERN EDITOR
	// ------------------------------------------------------------------------------------------------

	{ "help", ID_HELP },

	{ "pattern_play", ID_PATTERN_PLAY },
	{ "pattern_replay", ID_PATTERN_REPLAY },
	{ "pattern_playfromcursor", ID_PATTERN_PLAYFROMCURSOR },
	{ "pattern_play_track", ID_PATTERNVIEW_PLAY_TRACKROW },
	{ "pattern_play_row", ID_PATTERNVIEW_PLAY_ROW },

	{ "edit_copy", ID_EDIT_COPY },
	{ "edit_cut", ID_EDIT_CUT },
	{ "edit_paste", ID_EDIT_PASTE },
	{ "cut_splice", ID_PATTERNVIEW_CUT_SPLICE },
	{ "paste_splice", ID_PATTERNVIEW_PASTE_SPLICE },
	{ "paste_step", ID_PATTERNVIEW_PASTE_STEP },
	{ "paste_mixover", ID_PATTERNVIEW_PASTE_MIXOVER },
	{ "paste_mixunder", ID_PATTERNVIEW_PASTE_MIXUNDER },

	{ "clear_value", ID_PATTERNVIEW_CLEARVALUE },//add nostep versions?
	{ "clear_track_row", ID_PATTERNVIEW_CLEARTRACKROW },
	{ "clear_pattern_row", ID_PATTERNVIEW_CLEARPATTERNROW },
	{ "insert_column_row", ID_PATTERNVIEW_INSERTCOLUMNROW },
	{ "insert_track_row", ID_PATTERNVIEW_INSERTTRACKROW },
	{ "insert_pattern_row", ID_PATTERNVIEW_INSERTPATTERNROW },
	{ "edit_delete", ID_EDIT_DELETE },
	{ "delete_track_row", ID_PATTERNVIEW_DELETETRACKROW },
	{ "delete_pattern_row", ID_PATTERNVIEW_DELETEPATTERNROW },
	{ "backspace_value", ID_PATTERNVIEW_BACKSPACECOLUMNROW },
	{ "backspace_track_row", ID_PATTERNVIEW_BACKSPACETRACKROW },
	{ "backspace_pattern_row", ID_PATTERNVIEW_BACKSPACEPATTERNROW },

	{ "select_begin", ID_PATTERNVIEW_SELECTBEGIN },
	{ "select_end", ID_PATTERNVIEW_SELECTEND },
	{ "select_down", ID_PATTERNVIEW_SELECTDOWN },
	{ "select_beat", ID_PATTERNVIEW_SELECTBEAT },
	{ "select_columns", ID_PATTERNVIEW_SELECTCOLUMNS },
	{ "select_all", ID_EDIT_SELECTALL },
	{ "select_none", ID_PATTERNVIEW_UNSELECT },

	{ "track_mute", ID_PATTERNVIEW_TRACKMUTE },
	{ "track_solo", ID_PATTERNVIEW_TRACKSOLO },
	{ "machine_mute", ID_MACHINE_MUTE },
	{ "machine_solo", ID_MACHINE_SOLO },

	{ "track_add", ID_MACHINE_ADDTRACK },
	{ "track_remove", ID_MACHINE_REMOVETRACK },

	{ "loop_set_begin", ID_PATTERNVIEW_SETLOOPBEGIN },
	{ "loop_set_end", ID_PATTERNVIEW_SETLOOPEND },
	{ "loop_set_pattern", ID_PATTERNVIEW_SETLOOPPATTERN },
	{ "loop_set_selection", ID_PATTERNVIEW_SETLOOPSELECTION },

	{ "format_up", ID_PATTERNVIEW_FORMATUP },
	{ "format_down", ID_PATTERNVIEW_FORMATDOWN },
	{ "pattern_up", ID_PATTERNVIEW_PATTERNUP },
	{ "pattern_down", ID_PATTERNVIEW_PATTERNDOWN },
	{ "dropdown_wave", ID_PATTERNVIEW_DROPDOWNWAVE },
	{ "dropdown_format", ID_PATTERNVIEW_DROPDOWNFORMAT },
	{ "dropdown_pattern", ID_PATTERNVIEW_DROPDOWNPATTERN },
	{ "dropdown_octave", ID_PATTERNVIEW_DROPDOWNOCTAVE },
	{ "dropdown_step", ID_PATTERNVIEW_DROPDOWNSTEP },
	{ "toggle_playnotes", ID_PATTERNVIEW_TOGGLEPLAYNOTES },
	{ "toggle_showinfo", ID_PATTERNVIEW_TOGGLESHOWINFO },
	{ "toggle_follow", ID_PATTERNVIEW_TOGGLEFOLLOW },
	{ "show_notemask", ID_PATTERNVIEW_SHOWNOTEMASK },
	{ "show_harmonictranspose", ID_PATTERNVIEW_SHOWHARMONICTRANSPOSE },
	{ "toggle_patternloop", ID_PATTERNVIEW_TOGGLEPATTERNLOOP },
	{ "dropdown_patternscale", ID_PATTERNVIEW_DROPDOWNPATTERNSCALE },
	{ "dropdown_patternbeat", ID_PATTERNVIEW_DROPDOWNPATTERNBEAT },
	{ "dropdown_patternrows", ID_PATTERNVIEW_DROPDOWNPATTERNROWS },
	{ "dropdown_patternname", ID_PATTERNVIEW_DROPDOWNPATTERNNAME },

	{ "patternstack_back", ID_PATTERNVIEW_PATTERNSTACK_BACK },
	{ "patternstack_forward", ID_PATTERNVIEW_PATTERNSTACK_FORWARD },
	{ "patternstack_reset", ID_PATTERNVIEW_PATTERNSTACK_RESET },

	{ "step_set_0", ID_PATTERNVIEW_STEP_SET_0 },
	{ "step_set_1", ID_PATTERNVIEW_STEP_SET_1 },
	{ "step_set_2", ID_PATTERNVIEW_STEP_SET_2 },
	{ "step_set_3", ID_PATTERNVIEW_STEP_SET_3 },
	{ "step_set_4", ID_PATTERNVIEW_STEP_SET_4 },
	{ "step_set_5", ID_PATTERNVIEW_STEP_SET_5 },
	{ "step_set_6", ID_PATTERNVIEW_STEP_SET_6 },
	{ "step_set_7", ID_PATTERNVIEW_STEP_SET_7 },
	{ "step_set_8", ID_PATTERNVIEW_STEP_SET_8 },
	{ "step_set_9", ID_PATTERNVIEW_STEP_SET_9 },
	{ "step_up", ID_PATTERNVIEW_STEP_UP },
	{ "step_down", ID_PATTERNVIEW_STEP_DOWN },

	{ "keyjazz_octave_up", ID_PATTERNVIEW_OCTAVE_UP },
	{ "keyjazz_octave_down", ID_PATTERNVIEW_OCTAVE_DOWN },
	{ "wave_next", ID_PATTERNVIEW_WAVE_NEXT },
	{ "wave_previous", ID_PATTERNVIEW_WAVE_PREVIOUS },
	{ "toggle_horizontalentry", ID_PATTERNVIEW_TOGGLE_HORIZONTALENTRY },
	{ "cycle_notesaffect", ID_PATTERNVIEW_CYCLE_NOTESAFFECT },
	{ "toggle_transposeset", ID_PATTERNVIEW_TOGGLE_TRANSPOSESET },
	{ "toggle_volmask", ID_PATTERNVIEW_TOGGLE_VOLMASK },

	{ "pattern_rows_double", ID_PATTERN_DOUBLEROWS },
	{ "pattern_rows_halve", ID_PATTERN_HALVEROWS },
	{ "pattern_length_double", ID_PATTERN_DOUBLELENGTH },
	{ "pattern_length_halve", ID_PATTERN_HALVELENGTH },

	{ "transpose_notes_up", ID_PATTERNVIEW_TRANSPOSENOTESUP },
	{ "transpose_notes_down", ID_PATTERNVIEW_TRANSPOSENOTESDOWN },
	{ "transpose_notes_octaveup", ID_PATTERNVIEW_TRANSPOSENOTESOCTAVEUP },
	{ "transpose_notes_octavedown", ID_PATTERNVIEW_TRANSPOSENOTESOCTAVEDOWN },
	{ "transpose_all_up", ID_PATTERNVIEW_TRANSPOSESELECTIONUP },
	{ "transpose_all_down", ID_PATTERNVIEW_TRANSPOSESELECTIONDOWN },
	{ "transpose_rekey", ID_TRANSPOSESET_REKEY },

	{ "transform_randomize", ID_PATTERNVIEW_RANDOMIZE_SELECTION },
	{ "transform_randomrange", ID_PATTERNVIEW_RANDOMIZERANGE_SELECTION },
	{ "transform_randomfrom", ID_PATTERNVIEW_RANDOMIZEUSING_SELECTION },
	{ "transform_humanize", ID_PATTERNVIEW_HUMANIZE_SELECTION },
	{ "transform_shuffle", ID_PATTERNVIEW_SHUFFLE_SELECTION },
	{ "transform_interpolate", ID_PATTERNVIEW_INTERPOLATE_SELECTION },
	{ "transform_gradiate", ID_PATTERNVIEW_GRADIATE_SELECTION },
	{ "transform_smooth", ID_PATTERNVIEW_SMOOTH_SELECTION },
	{ "transform_reverse", ID_PATTERNVIEW_REVERSE_SELECTION },
	{ "transform_compact", ID_PATTERNVIEW_COMPACT_SELECTION },
	{ "transform_expand", ID_PATTERNVIEW_EXPAND_SELECTION },
	{ "transform_thin", ID_PATTERNVIEW_THIN_SELECTION },
	{ "transform_repeat", ID_PATTERNVIEW_REPEAT_SELECTION },
	{ "transform_echo", ID_PATTERNVIEW_ECHO_SELECTION },
	{ "transform_unique", ID_PATTERNVIEW_UNIQUE_SELECTION },
	{ "transform_scale", ID_PATTERNVIEW_SCALE_SELECTION },
	{ "transform_fade", ID_PATTERNVIEW_FADE_SELECTION },
	{ "transform_curvemap", ID_PATTERNVIEW_CURVEMAP_SELECTION },
	{ "transform_invert", ID_PATTERNVIEW_INVERT_SELECTION },
	{ "transform_rotaterows", ID_PATTERNVIEW_ROTATEROWS_SELECTION },
	{ "transform_rotaterows_up", ID_PATTERNVIEW_ROTATEROWS_UP_SELECTION },
	{ "transform_rotaterows_down", ID_PATTERNVIEW_ROTATEROWS_DOWN_SELECTION },
	{ "transform_rotatevalues", ID_PATTERNVIEW_ROTATEVALUES_SELECTION },
	{ "transform_rotatevalues_up", ID_PATTERNVIEW_ROTATEVALUES_UP_SELECTION },
	{ "transform_rotatevalues_down", ID_PATTERNVIEW_ROTATEVALUES_DOWN_SELECTION },
	{ "transform_rotaterhythms", ID_PATTERNVIEW_ROTATERHYTHMS_SELECTION },
	{ "transform_rotaterhythms_up", ID_PATTERNVIEW_ROTATERHYTHMS_UP_SELECTION },
	{ "transform_rotaterhythms_down", ID_PATTERNVIEW_ROTATERHYTHMS_DOWN_SELECTION },
	{ "transform_rotatenotes", 0 },
	{ "transform_rotatenotes_up", 0 },
	{ "transform_rotatenotes_down", 0 },
	{ "transform_notelength", ID_PATTERNVIEW_NOTELENGTH_SELECTION },
	{ "transform_volumes", ID_PATTERNVIEW_VOLUMES_SELECTION },
	{ "transform_alltofirst", ID_PATTERNVIEW_ALLTOFIRST_SELECTION },
	{ "transform_firsttolast", ID_PATTERNVIEW_FIRSTTOLAST_SELECTION },
	{ "transform_removefirst", ID_PATTERNVIEW_REMOVEFIRST_SELECTION },
	{ "transform_replacewaves", ID_PATTERNVIEW_REPLACEWAVES_SELECTION },
	{ "transform_invertchord_up", ID_PATTERNVIEW_INVERTCHORDUP_SELECTION },
	{ "transform_invertchord_down", ID_PATTERNVIEW_INVERTCHORDDOWN_SELECTION },
	{ "transform_trackswap", ID_PATTERNVIEW_TRACKSWAP_SELECTION },
	{ "transform_rowswap", ID_PATTERNVIEW_ROWSWAP_SELECTION },
	{ "transform_clearsamecolumn", ID_PATTERNVIEW_CLEARSAMECOLUMN_SELECTION },

	{ "column_action_1", ID_PATTERNVIEW_SPECIAL1 },
	{ "column_action_2", ID_PATTERNVIEW_SPECIAL2 },
	{ "column_action_3", ID_PATTERNVIEW_SPECIAL3 },
	{ "column_action_4", ID_PATTERNVIEW_SPECIAL4 },
	{ "column_action_5", ID_PATTERNVIEW_SPECIAL5 },
	{ "column_action_6", ID_PATTERNVIEW_SPECIAL6 },

	{ "column_toggle_columncontrol", ID_PATTERNVIEW_TOGGLECOLUMNCONTROL },
	{ "column_collapse_selection", ID_PATTERNVIEW_COLLAPSESELECTION },
	{ "column_uncollapse_selection", ID_PATTERNVIEW_UNCOLLAPSESELECTION },
	{ "column_toggle_collapse_selection", ID_PATTERNVIEW_TOGGLECOLLAPSESELECTION },
	{ "column_toggle_collapse_track", ID_PATTERNVIEW_TOGGLECOLLAPSETRACK },

	{ "pickupvalue", ID_PATTERNVIEW_PICKUPVALUE },

	{ "infopane_up", ID_PATTERNVIEW_INFOPANE_UP },
	{ "infopane_down", ID_PATTERNVIEW_INFOPANE_DOWN },

	{ "track_swap_right", ID_PATTERNVIEW_TRACKSWAPRIGHT },
	{ "track_swap_left", ID_PATTERNVIEW_TRACKSWAPLEFT },
	{ "format_layoutplugin_right", ID_PATTERNVIEW_FORMATLAYOUTPLUGINRIGHT },
	{ "format_layoutplugin_left", ID_PATTERNVIEW_FORMATLAYOUTPLUGINLEFT },

	{ "pattern_create", ID_PATTERNVIEW_CREATEPATTERN },
	{ "pattern_clone", ID_PATTERN_CLONE },
	{ "pattern_delete", ID_PATTERN_DELETE },
	{ "pattern_properties", ID_VIEW_PROPERTIES },

	{ "format_create", ID_PATTERN_CREATEFORMAT },
	{ "format_clone", ID_PATTERNVIEW_CLONE_FORMAT },
	{ "format_delete", ID_PATTERN_DELETEFORMAT },
	{ "format_properties", ID_PATTERN_FORMATPROPERTIES },

	{ "machine_parameters", ID_MACHINE_PARAMETERS },
	{ "machine_properties", ID_MACHINE_PROPERTIES },

	{ "orderlist_right", ID_PATTERNVIEW_ORDERLIST_RIGHT },
	{ "orderlist_left", ID_PATTERNVIEW_ORDERLIST_LEFT },
};

static id_desc_t const id_descs_machineview[] =
{
	// ------------------------------------------------------------------------------------------------
	// MACHINE VIEW
	// ------------------------------------------------------------------------------------------------

	{ "help", ID_HELP },

	{ "edit_copy", ID_EDIT_COPY },
	{ "edit_cut", ID_EDIT_CUT },
	{ "edit_paste", ID_EDIT_PASTE },
	{ "edit_paste_no_data", ID_EDIT_PASTE_NO_DATA },
	{ "edit_selectall", ID_EDIT_SELECTALL },
	{ "edit_delete", ID_EDIT_DELETE },
	{ "delete_and_restore", ID_MACHINE_DELETE_AND_RESTORE },

	{ "align_down", ID_MACHINE_ALIGN_DOWN },
	{ "align_left", ID_MACHINE_ALIGN_LEFT },
	{ "align_right", ID_MACHINE_ALIGN_RIGHT },
	{ "align_up", ID_MACHINE_ALIGN_UP },

	{ "move_down", ID_MACHINE_MOVE_DOWN },
	{ "move_down_step", ID_MACHINE_MOVE_DOWN_STEP },
	{ "move_down_left", ID_MACHINE_MOVE_DOWN_LEFT },
	{ "move_down_left_step", ID_MACHINE_MOVE_DOWN_LEFT_STEP },
	{ "move_down_right", ID_MACHINE_MOVE_DOWN_RIGHT },
	{ "move_down_right_step", ID_MACHINE_MOVE_DOWN_RIGHT_STEP },
	{ "move_left", ID_MACHINE_MOVE_LEFT },
	{ "move_left_step", ID_MACHINE_MOVE_LEFT_STEP },
	{ "move_right", ID_MACHINE_MOVE_RIGHT },
	{ "move_right_step", ID_MACHINE_MOVE_RIGHT_STEP },
	{ "move_up", ID_MACHINE_MOVE_UP },
	{ "move_up_step", ID_MACHINE_MOVE_UP_STEP },
	{ "move_up_left", ID_MACHINE_MOVE_UP_LEFT },
	{ "move_up_left_step", ID_MACHINE_MOVE_UP_LEFT_STEP },
	{ "move_up_right", ID_MACHINE_MOVE_UP_RIGHT },
	{ "move_up_right_step", ID_MACHINE_MOVE_UP_RIGHT_STEP },

	{ "machine_mute", ID_MACHINE_MUTE },
	{ "machine_solo", ID_MACHINE_SOLO },
	{ "machine_bypass", ID_MACHINE_BYPASS },
	{ "machine_unmute_all", ID_UNMUTE_ALL },

	{ "machine_parameters", ID_MACHINE_PARAMETERS },
	{ "machine_properties", ID_MACHINE_PROPERTIES },

	{ "toggle_connection_text", ID_MACHINE_TOGGLE_CONNECTION_TEXT },
};

static id_desc_t const id_descs_wavetable[] =
{
	// ------------------------------------------------------------------------------------------------
	// WAVETABLE
	// ------------------------------------------------------------------------------------------------

	{ "help", ID_HELP },

	{ "edit_copy", ID_EDIT_COPY },
	{ "edit_cut", ID_EDIT_CUT },
	{ "edit_paste", ID_EDIT_PASTE },
	{ "edit_selectall", ID_EDIT_SELECTALL },
	{ "edit_delete", ID_EDIT_DELETE },

	{ "wave_zoom_in", ID_WAVE_ZOOM_IN },
	{ "wave_zoom_out", ID_WAVE_ZOOM_OUT },
	{ "wave_zoom_selection", ID_WAVE_ZOOM_SELECTION },
	{ "wave_zoom_all", ID_WAVE_ZOOM_ALL },

	{ "wave_properties", ID_VIEW_PROPERTIES },

	{ "wave_clear", ID_WAVE_CLEARWAVE },
	{ "wave_trim", ID_WAVE_TRIM },
};

static id_desc_t const id_descs_parameter[] =
{
	// ------------------------------------------------------------------------------------------------
	// PARAMETER EDITOR
	// ------------------------------------------------------------------------------------------------

	{ "help", ID_HELP },

	{ "slider_next", ID_PARAMETERVIEW_NEXTSLIDER },
	{ "slider_previous", ID_PARAMETERVIEW_PREVIOUSSLIDER },

	{ "move_right", ID_PARAMETERVIEW_MOVERIGHT },
	{ "move_someright", ID_PARAMETERVIEW_MOVESOMERIGHT },
	{ "move_pageright", ID_PARAMETERVIEW_MOVEPAGERIGHT },
	{ "move_left", ID_PARAMETERVIEW_MOVELEFT },
	{ "move_someleft", ID_PARAMETERVIEW_MOVESOMELEFT },
	{ "move_pageleft", ID_PARAMETERVIEW_MOVEPAGELEFT },

	{ "entervalue", ID_PARAMETERVIEW_ENTERVALUE },

	{ "preset_dropdown", ID_PARAMETERVIEW_PRESETS },
	{ "preset_previous", ID_PARAMETERVIEW_PREVIOUSPRESET },
	{ "preset_next", ID_PARAMETERVIEW_NEXTPRESET },

	{ "preset_randomize", ID_PRESET_RANDOMIZE },
	{ "preset_humanize", ID_PRESET_HUMANIZE },
};

static id_desc_t const id_descs_orderlist[] =
{
	// ------------------------------------------------------------------------------------------------
	// ORDER LIST
	// ------------------------------------------------------------------------------------------------

	{ "help", ID_HELP },

	{ "edit_copy", ID_EDIT_COPY },
	{ "edit_cut", ID_EDIT_CUT },
	{ "edit_paste", ID_EDIT_PASTE },

	{ "order_insert", ID_ORDERLIST_INSERT },
	{ "order_duplicate", ID_ORDERLIST_DUPLICATE },
	{ "order_new", ID_ORDERLIST_NEW },
	{ "order_remove", ID_ORDERLIST_REMOVE },
	{ "order_remove_delete", ID_ORDERLIST_REMOVE_DELETE },
	{ "order_backspace", ID_ORDERLIST_BACKSPACE },
	{ "order_backspace_delete", ID_ORDERLIST_BACKSPACE_DELETE },
	{ "loop_set_begin", ID_ORDERLIST_SETBEGINLOOP },
	{ "loop_set_end", ID_ORDERLIST_SETENDLOOP },
	{ "loop_set_selection", ID_ORDERLIST_SETSELECTIONLOOP },

	{ "toggle_follow", ID_ORDERLIST_TOGGLEFOLLOW },

	{ "play", ID_ORDERLIST_PLAYORDER },
	{ "queue", ID_ORDERLIST_QUEUE },

	{ "deselect", ID_ORDERLIST_DESELECT },
	{ "focus_editor", ID_ORDERLIST_GOTO_EDITOR },
};

static id_desc_t const id_descs_filebrowser[] =
{
	// ------------------------------------------------------------------------------------------------
	// FILE BROWSER
	// ------------------------------------------------------------------------------------------------

	{ "help", ID_HELP },

	{ "import", ID_FILEBROWSER_IMPORT },

	{ "previous", ID_FILEBROWSER_PREV },
	{ "next", ID_FILEBROWSER_NEXT },
};

static id_desc_t const id_descs_properties[] =
{
	// ------------------------------------------------------------------------------------------------
	// PROPERTY EDITOR
	// ------------------------------------------------------------------------------------------------

	{ "help", ID_HELP },

	{ "returntoview", ID_PROPERTYVIEW_RETURNTOVIEW },
};

static id_desc_t const id_descs_patternlistview[] =
{
	// ------------------------------------------------------------------------------------------------
	// PATTERN LIST
	// ------------------------------------------------------------------------------------------------

	{ "help", ID_HELP },

	{ "returntoview", ID_PROPERTYVIEW_RETURNTOVIEW },
};

static id_desc_t const id_descs_patternformatview[] =
{
	// ------------------------------------------------------------------------------------------------
	// PATTERN FORMAT EDITOR
	// ------------------------------------------------------------------------------------------------

	{ "help", ID_HELP },

	{ "returntoview", ID_PROPERTYVIEW_RETURNTOVIEW },
};

// ---------------------------------------------------------------------------------------------------------------
// VIRTUAL KEYS
// ---------------------------------------------------------------------------------------------------------------

static vkey_desc_t const vkey_descs[] =
{
	{ "backspace", VK_BACK },
	{ "tab", VK_TAB },
	{ "clear", VK_CLEAR },
	{ "enter", VK_RETURN },
	{ "pause", VK_PAUSE },
	{ "capslock", VK_CAPITAL },
	{ "escape", VK_ESCAPE },
	{ "space", VK_SPACE },
	{ "pageup", VK_PRIOR },
	{ "pagedown", VK_NEXT },
	{ "end", VK_END },
	{ "home", VK_HOME },
	{ "left", VK_LEFT },
	{ "up", VK_UP },
	{ "right", VK_RIGHT },
	{ "down", VK_DOWN },
	{ "select", VK_SELECT },
	{ "print", VK_PRINT },
	{ "execute", VK_EXECUTE },
	{ "printscreen", VK_SNAPSHOT },
	{ "insert", VK_INSERT },
	{ "delete", VK_DELETE },
	{ "help", VK_HELP },
	{ "numpad0", VK_NUMPAD0 },
	{ "numpad1", VK_NUMPAD1 },
	{ "numpad2", VK_NUMPAD2 },
	{ "numpad3", VK_NUMPAD3 },
	{ "numpad4", VK_NUMPAD4 },
	{ "numpad5", VK_NUMPAD5 },
	{ "numpad6", VK_NUMPAD6 },
	{ "numpad7", VK_NUMPAD7 },
	{ "numpad8", VK_NUMPAD8 },
	{ "numpad9", VK_NUMPAD9 },
	{ "multiply", VK_MULTIPLY },
	{ "add", VK_ADD },
	{ "separator", VK_SEPARATOR },
	{ "subtract", VK_SUBTRACT },
	{ "decimal", VK_DECIMAL },
	{ "divide", VK_DIVIDE },
	{ "f1", VK_F1 },
	{ "f2", VK_F2 },
	{ "f3", VK_F3 },
	{ "f4", VK_F4 },
	{ "f5", VK_F5 },
	{ "f6", VK_F6 },
	{ "f7", VK_F7 },
	{ "f8", VK_F8 },
	{ "f9", VK_F9 },
	{ "f10", VK_F10 },
	{ "f11", VK_F11 },
	{ "f12", VK_F12 },
	{ "f13", VK_F13 },
	{ "f14", VK_F14 },
	{ "f15", VK_F15 },
	{ "f16", VK_F16 },
	{ "numlock", VK_NUMLOCK },
	{ "scrolllock", VK_SCROLL },
	{ "oem_1", VK_OEM_1 },
	{ "oem_plus", VK_OEM_PLUS },
	{ "oem_comma", VK_OEM_COMMA },
	{ "oem_minus", VK_OEM_MINUS },
	{ "oem_period", VK_OEM_PERIOD },
	{ "oem_2", VK_OEM_2 },
	{ "oem_3", VK_OEM_3 },
	{ "oem_4", VK_OEM_4 },
	{ "oem_5", VK_OEM_5 },
	{ "oem_6", VK_OEM_6 },
	{ "oem_7", VK_OEM_7 },
	{ "oem_8", VK_OEM_8 },
};
static size_t const vkey_descs_count = array_size(vkey_descs);

// ---------------------------------------------------------------------------------------------------------------
// KEYJAZZ
// ---------------------------------------------------------------------------------------------------------------

struct keyjazz_desc_t {
	char const* name;
	int note_or_cmd;
};

static keyjazz_desc_t const keyjazz_descs[] =
{
	{ "C-0", 0  },
	{ "C#0", 1  },
	{ "D-0", 2  },
	{ "D#0", 3  },
	{ "E-0", 4  },
	{ "F-0", 5  },
	{ "F#0", 6  },
	{ "G-0", 7  },
	{ "G#0", 8  },
	{ "A-0", 9  },
	{ "A#0", 10 },
	{ "B-0", 11 },
	{ "C-1", 12 },
	{ "C#1", 13 },
	{ "D-1", 14 },
	{ "D#1", 15 },
	{ "E-1", 16 },
	{ "F-1", 17 },
	{ "F#1", 18 },
	{ "G-1", 19 },
	{ "G#1", 20 },
	{ "A-1", 21 },
	{ "A#1", 22 },
	{ "B-1", 23 },
	{ "C-2", 24 },
	{ "C#2", 25 },
	{ "D-2", 26 },
	{ "D#2", 27 },
	{ "E-2", 28 },
	{ "note_off",     255 },
	{ "note_cut",     254 },
	{ "jazz_lastnote", -2 },
	{ "jazz_track",    -3 },
	{ "jazz_row",      -4 },
	{ "octave_up",     -5 },
	{ "octave_down",   -6 },
};
static size_t const keyjazz_descs_count = array_size(keyjazz_descs);

// ---------------------------------------------------------------------------------------------------------------
// CHOTKEYS
// ---------------------------------------------------------------------------------------------------------------

CHotkeys::CHotkeys() {
	InitMaps();
	bool success = ReadJson();
	if (success)
		CreateAccelTables();
}

CHotkeys::~CHotkeys() {
	DestroyAccelTables();
}

void CHotkeys::Reload() {
	view_accel_reverse_maps.clear();
	view_accel_maps.clear();
	keyjazz_key_map.clear();
	DestroyAccelTables();
	ReadJson();
}

void CHotkeys::InitMaps() {
	BuildIdMap("mainframe", id_descs_mainframe);
	BuildIdMap("patternview", id_descs_patternview);
	BuildIdMap("machineview", id_descs_machineview);
	BuildIdMap("wavetable", id_descs_wavetable);
	BuildIdMap("parameter", id_descs_parameter);
	BuildIdMap("orderlist", id_descs_orderlist);
	BuildIdMap("filebrowser", id_descs_filebrowser);
	BuildIdMap("properties", id_descs_properties);
	BuildIdMap("patternlistview", id_descs_patternlistview);
	BuildIdMap("patternformatview", id_descs_patternformatview);

	for (int i = 0; i < vkey_descs_count; ++i) {
		vkey_desc_t const& vkey_desc = vkey_descs[i];
		vkey_map[vkey_desc.name] = vkey_desc.vkey;
	}

	for (int i = 0; i < keyjazz_descs_count; ++i) {
		keyjazz_desc_t const& keyjazz_desc = keyjazz_descs[i];
		keyjazz_map[keyjazz_desc.name] = keyjazz_desc.note_or_cmd;
	}
}

template <size_t N>
void CHotkeys::BuildIdMap(std::string const& view, id_desc_t const (&id_descs)[N]) {
	id_map_t& id_map = view_id_maps[view];
	for (int i = 0; i < N; ++i) {
		id_desc_t const& id_desc = id_descs[i];
		id_map[id_desc.name] = id_desc.id;
	}
}

std::string CHotkeys::GetPath() const {
	HMODULE module_handle = ::GetModuleHandle(0);
	if (!module_handle) return "";

	char path[MAX_PATH + 32] = { 0 };
	::GetModuleFileName(module_handle, path, MAX_PATH);
	std::size_t n = std::strlen(path);
	if (!n) return "";
	while (n--) {
		if (path[n]=='\\') {
			path[n] = 0;
			break;
		}
	}

	std::string s = path;
	s += "\\hotkeys.json";

	return s;
}

bool CHotkeys::ReadJson() {
	std::string path = GetPath();
	std::string input = read_file(path);

	Json::Reader reader;
	Json::Value root;

	bool parsingSuccessful = reader.parse(input, root);

	if (!parsingSuccessful) {
		std::cerr << "hotkeys: failed to parse file: " << path << std::endl;
		std::cerr << "hotkeys: parsing error: " << reader.getFormatedErrorMessages() << std::endl;
		std::stringstream ss; ss << path << "\n\n" << reader.getFormatedErrorMessages() << std::endl;
		::MessageBox(::GetForegroundWindow(), ss.str().c_str(), "hotkeys parse error!", MB_OK);
		return false;
	}

	bool success = true;
	success &= CreateViewAccels(root, "mainframe");
	success &= CreateViewAccels(root, "patternview");
	success &= CreateViewAccels(root, "machineview");
	success &= CreateViewAccels(root, "wavetable");
	success &= CreateViewAccels(root, "parameter");
	success &= CreateViewAccels(root, "orderlist");
	success &= CreateViewAccels(root, "filebrowser");
	success &= CreateViewAccels(root, "properties");
	success &= CreateViewAccels(root, "patternlistview");
	success &= CreateViewAccels(root, "patternformatview");

	success &= CreateKeyjazzKeyMap(root);

	return true;
}

bool CHotkeys::CreateKeyjazzKeyMap(Json::Value& root) {
	Json::Value v = root["keyjazz"];

	if (v.type() != Json::objectValue)
		return false;

	if (!v.empty()) {
		for (Json::Value::iterator val_it = v.begin(); val_it != v.end(); ++val_it) {
			Json::Value& kv = *val_it;

			if (kv.type() == Json::stringValue) {
				CreateKeyjazzKey(val_it.memberName(), kv.asString());
			} else
			if (kv.type() == Json::arrayValue) {
				for (Json::Value::iterator val_it2 = kv.begin(); val_it2 != kv.end(); ++val_it2) {
					Json::Value& kkv = *val_it2;
					if (kkv.type() == Json::stringValue) {
						CreateKeyjazzKey(val_it.memberName(), kkv.asString());
					} else {
						return false;
					}
				}
			} else {
				return false;
			}
		}
	}

	return true;
}

bool CHotkeys::CreateKeyjazzKey(std::string const& name, std::string const& s) {
	keyjazz_map_t::const_iterator keyjazz_it = keyjazz_map.find(name);
	if (keyjazz_it == keyjazz_map.end())
		return false;

	if (s == "") // skip
		return true;

	WORD keycode;

	if (s.length() == 1) {
		keycode = toupper(s[0]);
	} else {
		vkey_map_t::const_iterator vkey_it = vkey_map.find(s);
		if (vkey_it == vkey_map.end())
			return false;

		keycode = (*vkey_it).second;
	}

	keyjazz_key_map[keycode] = (*keyjazz_it).second;
	return true;
}

bool CHotkeys::CreateViewAccels(Json::Value& root, std::string const& view) {
	Json::Value v = root[view];

	if (v.type() != Json::objectValue)
		return false;

	if (!v.empty()) {
		id_map_t const& id_map = view_id_maps[view];

//		accel_vec_t& accel_vec = view_accel_maps.insert(std::make_pair(view, accel_vec_t())).first->second;
//		accel_reverse_map_t& accel_reverse_map = view_accel_reverse_maps.insert(std::make_pair(view, accel_reverse_map_t())).first->second;
		accel_vec_t& accel_vec = view_accel_maps[view];
		accel_reverse_map_t& accel_reverse_map = view_accel_reverse_maps[view];

		accel_vec.reserve(1024);

		for (Json::Value::iterator val_it = v.begin(); val_it != v.end(); ++val_it) {
			Json::Value& vk = *val_it;

			if (vk.type() == Json::stringValue) {
				CreateStringAccel(view, val_it.memberName(), vk.asString(), accel_vec, accel_reverse_map, id_map);
			} else
			if (vk.type() == Json::arrayValue) {
				for (Json::Value::iterator val2_it = vk.begin(); val2_it != vk.end(); ++val2_it) {
					Json::Value& vkk = *val2_it;

					if (vkk.type() == Json::stringValue) {
						CreateStringAccel(view, val_it.memberName(), vkk.asString(), accel_vec, accel_reverse_map, id_map);
					} else {
						return false;
					}
				}
			} else {
				return false;
			}
		}
	}

	return true;
}

void CHotkeys::ShowHotkeyError(std::string const& view, std::string const& name) {
	std::cerr << "hotkeys: error in accelerator: " << view << "." << name << std::endl;
	std::stringstream ss; ss << "error in accelerator: " << view << "." << name << std::endl;
	::MessageBox(::GetForegroundWindow(), ss.str().c_str(), "hotkeys accelerator error!", MB_OK);
}

int CHotkeys::CreateStringAccel(std::string const& view, std::string const& name, std::string const& hotkey, accel_vec_t& accel_vec, accel_reverse_map_t& accel_reverse_map, id_map_t const& id_map) {
	ACCEL acc = { 0 };

	CreateAccelResult result = CreateAccel(name, hotkey, acc, id_map);

	if (result == create_ok) {
		accel_vec.push_back(acc);
		accel_reverse_map.insert(std::make_pair(acc.cmd, &accel_vec.back()));
	} else
	if (result == create_skip) {
		// skip
	} else {
		ShowHotkeyError(view, name);
	}

	return 0;///
}

CHotkeys::CreateAccelResult CHotkeys::CreateAccel(std::string const& name, std::string const& hotkey, ACCEL& acc, id_map_t const& id_map) {
	id_map_t::const_iterator id_it = id_map.find(name);
	if (id_it == id_map.end())
		return create_error;

	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(" ");
	tokenizer tokens(hotkey, sep);

// 	if (tokens.size() == 0)
// 		return create_skip;

	acc.cmd = (*id_it).second;

	bool got_key = false;
	bool got_modifier = false;
	bool is_char = false;

	int tok_count = 0;

	for (tokenizer::iterator tok_it = tokens.begin(); tok_it != tokens.end(); ++tok_it) {
		std::string s = *tok_it;

		if (s == "shift") {
			acc.fVirt |= FSHIFT;
			got_modifier = true;
		} else
		if (s == "ctrl") {
			acc.fVirt |= FCONTROL;
			got_modifier = true;
		} else
		if (s == "alt") {
			acc.fVirt |= FALT;
			got_modifier = true;
		} else
		if (s.length() == 1) {
			if (got_key)
				return create_error;

			acc.key = s[0];
			got_key = true;
			is_char = true;
		} else {
			if (got_key)
				return create_error;

			vkey_map_t::const_iterator vkey_it = vkey_map.find(s);
			if (vkey_it == vkey_map.end())
				return create_error;

			acc.key = (*vkey_it).second;
			got_key = true;
			got_modifier = true;
		}

		++tok_count;
	}

	if (tok_count == 0)
		return create_skip;

	if (!got_key)
		return create_error;

	if (got_modifier || !is_char) ///|| !is_char
		acc.fVirt |= FVIRTKEY;

	if (got_modifier && is_char)
		acc.key = toupper(acc.key);

	return create_ok;
}

void CHotkeys::CreateAccelTables() {
	CreateAccelTable("mainframe", mainframe_hAccel);
	CreateAccelTable("patternview", patternview_hAccel);
	CreateAccelTable("machineview", machineview_hAccel);
	CreateAccelTable("wavetable", wavetable_hAccel);
	CreateAccelTable("parameter", parameter_hAccel);
	CreateAccelTable("orderlist", orderlist_hAccel);
	CreateAccelTable("filebrowser", filebrowser_hAccel);
	CreateAccelTable("properties", properties_hAccel);
	CreateAccelTable("patternlistview", patternlistview_hAccel);
	CreateAccelTable("patternformatview", patternformatview_hAccel);
}

void CHotkeys::CreateAccelTable(std::string const& view, HACCEL& hAccel) {
	accel_vec_t& accel_vec = view_accel_maps[view];
	if (!accel_vec.empty()) {
		hAccel = ::CreateAcceleratorTable(&accel_vec.front(), accel_vec.size());
	}
}

void CHotkeys::DestroyAccelTables() {
	if (mainframe_hAccel) ::DestroyAcceleratorTable(mainframe_hAccel);
	if (patternview_hAccel) ::DestroyAcceleratorTable(patternview_hAccel);
	if (machineview_hAccel) ::DestroyAcceleratorTable(machineview_hAccel);
	if (wavetable_hAccel) ::DestroyAcceleratorTable(wavetable_hAccel);
	if (parameter_hAccel) ::DestroyAcceleratorTable(parameter_hAccel);
	if (orderlist_hAccel) ::DestroyAcceleratorTable(orderlist_hAccel);
	if (filebrowser_hAccel) ::DestroyAcceleratorTable(filebrowser_hAccel);
	if (properties_hAccel) ::DestroyAcceleratorTable(properties_hAccel);
	if (patternlistview_hAccel) ::DestroyAcceleratorTable(patternlistview_hAccel);
	if (patternformatview_hAccel) ::DestroyAcceleratorTable(patternformatview_hAccel);
}

// ---------------------------------------------------------------------------------------------------------------
// MENU UPDATING
// ---------------------------------------------------------------------------------------------------------------

// this helper from codeproject.com
// (c) 2004 Jörgen Sigvardsson <jorgen@profitab.com>
inline CString NameFromVKey(UINT nVK) {
	UINT nScanCode = ::MapVirtualKeyEx(nVK, 0, ::GetKeyboardLayout(GetCurrentThreadId()));

	switch (nVK) { // Keys which are "extended" (except for Return which is Numeric Enter as extended)
		case VK_INSERT:
		case VK_DELETE:
		case VK_HOME:
		case VK_END:
		case VK_NEXT:
		case VK_PRIOR:
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			nScanCode |= 0x100; // Add extended bit
	}

	// GetKeyNameText() expects the scan code to be on the same format as WM_KEYDOWN hence the left shift
	CString str;
	LPTSTR prb = str.GetBuffer(80);
	BOOL bResult = ::GetKeyNameText(nScanCode << 16, prb, 79);

	// these key names are capitalized and look a bit daft
	int len = lstrlen(prb);
	if (len > 1) {
		LPTSTR p2 = ::CharNext(prb);
		::CharLowerBuff(p2, len - (p2 - prb));
	}

	str.ReleaseBuffer();
	ATLASSERT(str.GetLength());
	return str; // internationalization ready, sweet!
}

inline CString NameFromAccel(ACCEL const& acc) {
	CString name;

	if (acc.fVirt & FCONTROL)
		name = "Ctrl+";
	if (acc.fVirt & FALT)
		name += "Alt+";
	if (acc.fVirt & FSHIFT)
		name += "Shift+";

	if (acc.fVirt & FVIRTKEY) {
		name += NameFromVKey(acc.key);
	} else {
		// key field is an ASCII key code.
#ifdef _UNICODE
		char    ca = (char)acc.key;
		wchar_t cu;

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, &ca, 1, &cu, 1);
		name += cu;
#else
		name += (char)acc.key;
#endif
	}

	ATLASSERT(name.GetLength());
	return name;
}

void CHotkeys::UpdateMenuKeys(std::string const& view, HMENU hMenu, bool show_accels) {
	accel_reverse_map_t const& accel_reverse_map = view_accel_reverse_maps[view];
	UpdateMenuKeys(accel_reverse_map, hMenu, show_accels);
}

void CHotkeys::UpdateMenuKeys(accel_reverse_map_t const& accel_reverse_map, HMENU hMenu, bool show_accels) {
	int nItems = ::GetMenuItemCount(hMenu);

	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_SUBMENU;

	TCHAR buf[512];
	CString name;

	for (int i = 0; i < nItems; ++i) {
		::GetMenuItemInfo(hMenu, i, TRUE, &mi); // by position

		if (mi.hSubMenu) {
			UpdateMenuKeys(accel_reverse_map, mi.hSubMenu, show_accels);
		} else
		if (mi.wID != 0) {
			// see if there's accelerator info in text
			ATLASSERT(!(buf[0] = 0));
			::GetMenuString(hMenu, i, buf, array_size(buf), MF_BYPOSITION);
			ATLASSERT(buf[0]);

			int len = lstrlen(buf);
			int k = len;

			while (k--)
				if (_T('\t') == buf[k])
					break;

			bool has_tab = (k > 0);
			bool changed = true;

			// is there any accelerator for this command nowadays?
			accel_reverse_map_t::const_iterator j = accel_reverse_map.find((WORD)mi.wID);
			if (!show_accels || j == accel_reverse_map.end()) {
				if (has_tab)
					buf[k] = 0; // remove old one
				else
					changed = 0;
			} else {
				if (!has_tab) {
					k = len;
					buf[k] = _T('\t');
				}
				++k;

				ACCEL const& acc = *((*j).second);
				name = NameFromAccel(acc);
				ATLASSERT(k + name.GetLength() < array_size(buf));
				lstrcpy(buf + k, name);
			}

			if (changed) {
				ATLASSERT(lstrlen(buf));
				::ModifyMenu(hMenu, i, MF_BYPOSITION, mi.wID, buf);
				// $TSEK no need to update item enable/icon states? (see wtl's command bar atlctrlw.h line 2630)
			}
		}
	}
}

void CHotkeys::AddMenuKeys(std::string const& view, HMENU hMenu) {
	accel_reverse_map_t const& accel_reverse_map = view_accel_reverse_maps[view];
	AddMenuKeys(accel_reverse_map, hMenu);
}

void CHotkeys::AddMenuKeys(accel_reverse_map_t const& accel_reverse_map, HMENU hMenu) {
	int nItems = ::GetMenuItemCount(hMenu);

	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_SUBMENU;

	TCHAR buf[512];
	std::stringstream name;

	for (int i = 0; i < nItems; ++i) {
		::GetMenuItemInfo(hMenu, i, TRUE, &mi);

		if (mi.hSubMenu) {
			AddMenuKeys(accel_reverse_map, mi.hSubMenu);
		} else
		if (mi.wID != 0) {
			accel_reverse_map_t::const_iterator j = accel_reverse_map.find((WORD)mi.wID);
			if (j != accel_reverse_map.end()) {
				::GetMenuString(hMenu, i, buf, array_size(buf), MF_BYPOSITION);

				ACCEL const& acc = *((*j).second);

				name.str("");
				name << buf << _T('\t') << NameFromAccel(acc);

				::ModifyMenu(hMenu, i, MF_BYPOSITION, mi.wID, name.str().c_str());
			}
		}
	}
}
