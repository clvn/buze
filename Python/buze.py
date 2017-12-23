#!/usr/bin/env python
# encoding: latin-1

from ctypes import *

import sys, os

def load_library(*names,**kw):
	"""
	searches for a library with given names and returns a ctypes 
	.so/.dll library object if successful. if the library can not
	be loaded, an assertion error will be thrown.
	
	@type  names: list of strings
	@param names: one or more aliases for required libraries, e.g.
				  'SDL','SDL-1.2'.
	@rtype: ctypes CDLL handle
	"""
	import ctypes, os, sys
	searchpaths = []
	if os.name in ('posix', 'mac'):
		if os.environ.has_key('LD_LIBRARY_PATH'):
			searchpaths += os.environ['LD_LIBRARY_PATH'].split(os.pathsep)
		searchpaths += [
			'/usr/local/lib64',
			'/usr/local/lib',
			'/usr/lib64',
			'/usr/lib',
		]
	elif os.name == 'nt':
		searchpaths += ['.']
		if 'PATH' in os.environ:
			searchpaths += os.environ['PATH'].split(os.pathsep)
	else:
		assert 0, "Unknown OS: %s" % os.name
	if 'paths' in kw:
		searchpaths += kw['paths']
	for name in names:
		if os.name == 'nt':
			libname = name + '.dll'
		elif sys.platform == 'darwin':
			libname = 'lib' + name + '.dylib'
			if 'version' in kw:
				libname += '.' + kw['version']
		else:
			libname = 'lib' + name + '.so'
			if 'version' in kw:
				libname += '.' + kw['version']
		m = None
		for path in searchpaths:
			if os.path.isdir(path):
				libpath = os.path.join(path,libname)
				if os.path.isfile(libpath):
					m = ctypes.CDLL(libpath)
					break
				for filename in reversed(sorted(os.listdir(path))):
					if filename.startswith(libname):
						m = ctypes.CDLL(os.path.join(path,filename))
						break
				if m:
					break
		if m:
			break
	assert m, "libraries %s not found in %s" % (','.join(["'%s'" % a for a in names]),','.join(searchpaths))
	return m
	
def dlopen(*args,**kwds):
	"""
	Opens a library by name and returns a handle object. See 
	{library.load} for more information.
	"""
	return load_library(*args,**kwds)

def dlsym(lib, name, restype, *args):
	"""
	Retrieves a symbol from a library loaded by dlopen and
	assigns correct result and argument types.
	
	@param lib: Library object.
	@type lib: ctypes.CDLL
	@param name: Name of symbol.
	@type name: str
	@param restype: Type of function return value.
	@param args: Types of function arguments.	
	"""
	if not lib:
		return None
	proc = getattr(lib,name)
	proc.restype = restype
	proc.argtypes = [argtype for argname,argtype in args]
	proc.o_restype = restype
	proc.o_args = args
	return proc

from armstrong import *
libbuzelib = dlopen("buzelib", "0.3")

# nameless enum
buze_version = 1


# enum buze_event_type
buze_event_type_update_new_document = 8192
buze_event_type_update_pre_save_document = 8193
buze_event_type_update_post_save_document = 8194
buze_event_type_update_pre_clear_document = 8195
buze_event_type_update_post_clear_document = 8196
buze_event_type_update_pre_open_document = 8197
buze_event_type_update_post_open_document = 8198
buze_event_type_update_pre_mixdown = 8199
buze_event_type_update_post_mixdown = 8200
buze_event_type_update_properties = 8201
buze_event_type_update_theme = 8202
buze_event_type_update_settings = 8203
buze_event_type_update_index = 8204
buze_event_type_change_pattern_order = 8300
buze_event_type_change_pattern_row = 8301
buze_event_type_show_parameter_view = 9000
buze_event_type_show_pattern_view = 9001
buze_event_type_show_pattern_format_view = 9002
buze_event_type_show_machine_view = 9003
buze_event_type_show_wavetable_view = 9004
buze_event_type_show_analyzer = 9006
buze_event_type_show_comment_view = 9007
buze_event_type_show_cpu_view = 9008
buze_event_type_show_filebrowser = 9009
buze_event_type_show_help_view = 9010
buze_event_type_show_history = 9011
buze_event_type_show_preferences = 9012
buze_event_type_show_properties = 9005
buze_event_type_show_all_machines = 9013


# enum buze_property_type
buze_property_type_plugin = 0
buze_property_type_connection = 1
buze_property_type_pattern = 2
buze_property_type_pattern_format = 3
buze_property_type_wave = 4
buze_property_type_wave_level = 5
buze_property_type_plugin_group = 6


# enum buze_path_type
buze_path_type_app_path = 0
buze_path_type_user_path = 1

class buze_event_data_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_event_data_change_pattern_order_t(Structure):
  _fields_ = [
    ("index", c_int),
  ]
  _anonymous_ = [
  ]

class buze_event_data_change_pattern_row_t(Structure):
  _fields_ = [
    ("row", c_int),
  ]
  _anonymous_ = [
  ]

class buze_event_data_show_machine_parameter_t(Structure):
  _fields_ = [
    ("plugin", POINTER(zzub_plugin_t)),
    ("mode", c_int),
    ("x", c_int),
    ("y", c_int),
  ]
  _anonymous_ = [
  ]

class buze_event_data_show_pattern_t(Structure):
  _fields_ = [
    ("pattern", POINTER(zzub_pattern_t)),
    ("change_pattern", c_int),
    ("reset_stack", c_int),
    ("editor_id", c_int),
  ]
  _anonymous_ = [
  ]

class buze_event_data_show_pattern_format_t(Structure):
  _fields_ = [
    ("pattern_format", POINTER(zzub_pattern_format_t)),
  ]
  _anonymous_ = [
  ]

class buze_window_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_event_data_show_properties_t(Structure):
  _fields_ = [
    ("type", c_int),
    ("return_view", POINTER(buze_window_t)),
  ]
  _anonymous_ = [
  ]

class buze_event_data_show_properties_union_00000002_t(Union):
    ("plugin", zzub_plugin_t),
    ("plugin_group", zzub_plugin_group_t),
    ("connection", zzub_connection_t),
    ("pattern", zzub_pattern_t),
    ("pattern_format", zzub_pattern_format_t),
    ("wave", zzub_wave_t),
    ("wavelevel", zzub_wavelevel_t),
class buze_event_data_union_00000003_t(Union):
    ("change_pattern_order", buze_event_data_change_pattern_order_t),
    ("change_pattern_row", buze_event_data_change_pattern_row_t),
    ("show_properties", buze_event_data_show_properties_t),
    ("show_parameters", buze_event_data_show_machine_parameter_t),
    ("show_pattern", buze_event_data_show_pattern_t),
    ("show_pattern_format", buze_event_data_show_pattern_format_t),
class buze_application_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_main_frame_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_window_factory_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_event_handler_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_message_filter_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_idle_handler_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_message_loop_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_host_module_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_document_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_plugin_index_item_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class buze_configuration_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

buze_application_create = dlsym(libbuzelib, "buze_application_create", POINTER(buze_application_t), ("module", POINTER(buze_host_module_t)), ("globalPath", c_char_p), ("userPath", c_char_p), ("tempPath", c_char_p))
buze_application_get_host_module = dlsym(libbuzelib, "buze_application_get_host_module", POINTER(buze_host_module_t), ("Application", POINTER(buze_application_t)))
buze_application_destroy = dlsym(libbuzelib, "buze_application_destroy", None, ("Application", POINTER(buze_application_t)))
buze_application_initialize = dlsym(libbuzelib, "buze_application_initialize", None, ("Application", POINTER(buze_application_t)), ("player", POINTER(zzub_player_t)), ("driver", POINTER(zzub_audiodriver_t)))
buze_application_open_midi_devices_from_config = dlsym(libbuzelib, "buze_application_open_midi_devices_from_config", c_int, ("Application", POINTER(buze_application_t)))
buze_application_create_audio_device_from_config = dlsym(libbuzelib, "buze_application_create_audio_device_from_config", c_int, ("Application", POINTER(buze_application_t)))
buze_application_create_audio_device = dlsym(libbuzelib, "buze_application_create_audio_device", c_int, ("Application", POINTER(buze_application_t)), ("outdevicename", c_char_p), ("indevicename", c_char_p), ("rate", c_int), ("buffersize", c_int), ("masterchannel", c_int), ("save", c_int))
buze_application_enable_silent_driver = dlsym(libbuzelib, "buze_application_enable_silent_driver", None, ("Application", POINTER(buze_application_t)), ("enable", c_int))
buze_application_get_audio_driver = dlsym(libbuzelib, "buze_application_get_audio_driver", POINTER(zzub_audiodriver_t), ("Application", POINTER(buze_application_t)))
buze_application_release_audio_driver = dlsym(libbuzelib, "buze_application_release_audio_driver", None, ("Application", POINTER(buze_application_t)))
buze_application_get_configuration = dlsym(libbuzelib, "buze_application_get_configuration", POINTER(buze_configuration_t), ("Application", POINTER(buze_application_t)))
buze_application_map_path = dlsym(libbuzelib, "buze_application_map_path", c_char_p, ("Application", POINTER(buze_application_t)), ("path", c_char_p), ("type", c_int))
buze_application_show_wait_window = dlsym(libbuzelib, "buze_application_show_wait_window", None, ("Application", POINTER(buze_application_t)))
buze_application_set_wait_text = dlsym(libbuzelib, "buze_application_set_wait_text", None, ("Application", POINTER(buze_application_t)), ("text", c_char_p))
buze_application_hide_wait_window = dlsym(libbuzelib, "buze_application_hide_wait_window", None, ("Application", POINTER(buze_application_t)), ("focusWnd", c_void_p))
buze_application_get_theme_color = dlsym(libbuzelib, "buze_application_get_theme_color", c_uint, ("Application", POINTER(buze_application_t)), ("name", c_char_p))
buze_application_get_theme_name = dlsym(libbuzelib, "buze_application_get_theme_name", c_char_p, ("Application", POINTER(buze_application_t)), ("index", c_int))
buze_application_get_theme_count = dlsym(libbuzelib, "buze_application_get_theme_count", c_int, ("Application", POINTER(buze_application_t)))
buze_application_load_theme = dlsym(libbuzelib, "buze_application_load_theme", None, ("Application", POINTER(buze_application_t)), ("name", c_char_p))
buze_main_frame_initialize = dlsym(libbuzelib, "buze_main_frame_initialize", c_int, ("MainFrame", POINTER(buze_main_frame_t)), ("parentwnd", c_void_p), ("player", POINTER(zzub_player_t)))
buze_main_frame_destroy = dlsym(libbuzelib, "buze_main_frame_destroy", None, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_register_window_factory = dlsym(libbuzelib, "buze_main_frame_register_window_factory", None, ("MainFrame", POINTER(buze_main_frame_t)), ("info", POINTER(buze_window_factory_t)))
buze_main_frame_get_wnd = dlsym(libbuzelib, "buze_main_frame_get_wnd", c_void_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_add_timer_handler = dlsym(libbuzelib, "buze_main_frame_add_timer_handler", None, ("MainFrame", POINTER(buze_main_frame_t)), ("wnd", POINTER(buze_window_t)))
buze_main_frame_remove_timer_handler = dlsym(libbuzelib, "buze_main_frame_remove_timer_handler", None, ("MainFrame", POINTER(buze_main_frame_t)), ("wnd", POINTER(buze_window_t)))
buze_main_frame_viewstack_insert = dlsym(libbuzelib, "buze_main_frame_viewstack_insert", None, ("MainFrame", POINTER(buze_main_frame_t)), ("wnd", POINTER(buze_window_t)))
buze_main_frame_get_accelerators = dlsym(libbuzelib, "buze_main_frame_get_accelerators", c_void_p, ("MainFrame", POINTER(buze_main_frame_t)), ("viewname", c_char_p))
buze_main_frame_get_document = dlsym(libbuzelib, "buze_main_frame_get_document", POINTER(buze_document_t), ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_application = dlsym(libbuzelib, "buze_main_frame_get_application", POINTER(buze_application_t), ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_player = dlsym(libbuzelib, "buze_main_frame_get_player", POINTER(zzub_player_t), ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_focused_view = dlsym(libbuzelib, "buze_main_frame_get_focused_view", POINTER(buze_window_t), ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_is_float_view = dlsym(libbuzelib, "buze_main_frame_is_float_view", c_int, ("MainFrame", POINTER(buze_main_frame_t)), ("wnd", POINTER(buze_window_t)))
buze_main_frame_set_focus_to = dlsym(libbuzelib, "buze_main_frame_set_focus_to", None, ("MainFrame", POINTER(buze_main_frame_t)), ("wnd", POINTER(buze_window_t)))
buze_main_frame_get_view = dlsym(libbuzelib, "buze_main_frame_get_view", POINTER(buze_window_t), ("MainFrame", POINTER(buze_main_frame_t)), ("viewname", c_char_p), ("viewid", c_int))
buze_main_frame_open_view = dlsym(libbuzelib, "buze_main_frame_open_view", POINTER(buze_window_t), ("MainFrame", POINTER(buze_main_frame_t)), ("viewname", c_char_p), ("label", c_char_p), ("viewid", c_int), ("x", c_int), ("y", c_int))
buze_main_frame_close_view = dlsym(libbuzelib, "buze_main_frame_close_view", None, ("MainFrame", POINTER(buze_main_frame_t)), ("wnd", POINTER(buze_window_t)))
buze_main_frame_get_open_filename = dlsym(libbuzelib, "buze_main_frame_get_open_filename", c_char_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_save_filename = dlsym(libbuzelib, "buze_main_frame_get_save_filename", c_char_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_plugin_menu_create = dlsym(libbuzelib, "buze_main_frame_get_plugin_menu_create", c_void_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_plugin_menu_insert_after = dlsym(libbuzelib, "buze_main_frame_get_plugin_menu_insert_after", c_void_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_plugin_menu_insert_before = dlsym(libbuzelib, "buze_main_frame_get_plugin_menu_insert_before", c_void_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_plugin_menu_replace = dlsym(libbuzelib, "buze_main_frame_get_plugin_menu_replace", c_void_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_main_menu = dlsym(libbuzelib, "buze_main_frame_get_main_menu", c_void_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_add_menu_keys = dlsym(libbuzelib, "buze_main_frame_add_menu_keys", None, ("MainFrame", POINTER(buze_main_frame_t)), ("viewname", c_char_p), ("menu", c_void_p))
buze_main_frame_register_event = dlsym(libbuzelib, "buze_main_frame_register_event", c_int, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_register_accelerator_event = dlsym(libbuzelib, "buze_main_frame_register_accelerator_event", c_ushort, ("MainFrame", POINTER(buze_main_frame_t)), ("viewname", c_char_p), ("defaulthotkey", c_char_p), ("evcode", c_int))
buze_main_frame_register_accelerator = dlsym(libbuzelib, "buze_main_frame_register_accelerator", None, ("MainFrame", POINTER(buze_main_frame_t)), ("viewname", c_char_p), ("name", c_char_p), ("defaulthotkey", c_char_p), ("id", c_ushort))
buze_main_frame_show_plugin_parameters = dlsym(libbuzelib, "buze_main_frame_show_plugin_parameters", None, ("MainFrame", POINTER(buze_main_frame_t)), ("p", POINTER(zzub_plugin_t)), ("modehint", c_int), ("x", c_int), ("y", c_int))
buze_main_frame_get_keyjazz_map = dlsym(libbuzelib, "buze_main_frame_get_keyjazz_map", c_void_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_main_frame_get_view_by_wnd = dlsym(libbuzelib, "buze_main_frame_get_view_by_wnd", POINTER(buze_window_t), ("MainFrame", POINTER(buze_main_frame_t)), ("wnd", c_void_p))
buze_main_frame_get_program_name = dlsym(libbuzelib, "buze_main_frame_get_program_name", c_char_p, ("MainFrame", POINTER(buze_main_frame_t)))
buze_window_get_wnd = dlsym(libbuzelib, "buze_window_get_wnd", c_void_p, ("Window", POINTER(buze_window_t)))
buze_callback_t = CFUNCTYPE(c_int, POINTER(buze_window_t), c_int, c_void_p, c_void_p)
buze_document_add_view = dlsym(libbuzelib, "buze_document_add_view", None, ("Document", POINTER(buze_document_t)), ("view", POINTER(buze_event_handler_t)))
buze_document_remove_view = dlsym(libbuzelib, "buze_document_remove_view", None, ("Document", POINTER(buze_document_t)), ("view", POINTER(buze_event_handler_t)))
buze_document_notify_views = dlsym(libbuzelib, "buze_document_notify_views", None, ("Document", POINTER(buze_document_t)), ("sender", POINTER(buze_window_t)), ("hint", c_int), ("param", c_void_p))
buze_document_add_callback = dlsym(libbuzelib, "buze_document_add_callback", None, ("Document", POINTER(buze_document_t)), ("callback", buze_callback_t), ("tag", c_void_p))
buze_document_remove_callback = dlsym(libbuzelib, "buze_document_remove_callback", None, ("Document", POINTER(buze_document_t)), ("callback", buze_callback_t), ("tag", c_void_p))
buze_document_get_octave = dlsym(libbuzelib, "buze_document_get_octave", c_int, ("Document", POINTER(buze_document_t)))
buze_document_set_octave = dlsym(libbuzelib, "buze_document_set_octave", None, ("Document", POINTER(buze_document_t)), ("oct", c_int))
buze_document_get_plugin_non_song = dlsym(libbuzelib, "buze_document_get_plugin_non_song", c_int, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)))
buze_document_set_plugin_non_song = dlsym(libbuzelib, "buze_document_set_plugin_non_song", None, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)), ("state", c_int))
buze_document_get_plugin_parameter_view_mode = dlsym(libbuzelib, "buze_document_get_plugin_parameter_view_mode", c_int, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)))
buze_document_set_plugin_parameter_view_mode = dlsym(libbuzelib, "buze_document_set_plugin_parameter_view_mode", None, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)), ("mode", c_int))
buze_document_get_plugin_last_preset = dlsym(libbuzelib, "buze_document_get_plugin_last_preset", c_char_p, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)))
buze_document_set_plugin_last_preset = dlsym(libbuzelib, "buze_document_set_plugin_last_preset", None, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)), ("preset", c_char_p))
buze_document_get_player = dlsym(libbuzelib, "buze_document_get_player", POINTER(zzub_player_t), ("Document", POINTER(buze_document_t)))
buze_document_get_stream_plugin_uri_for_file = dlsym(libbuzelib, "buze_document_get_stream_plugin_uri_for_file", c_char_p, ("Document", POINTER(buze_document_t)), ("path", c_char_p))
buze_document_play_plugin_note = dlsym(libbuzelib, "buze_document_play_plugin_note", None, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)), ("note", c_int), ("prevnote", c_int))
buze_document_play_stream = dlsym(libbuzelib, "buze_document_play_stream", None, ("Document", POINTER(buze_document_t)), ("note", c_int), ("offset", c_int), ("length", c_int), ("pluginuri", c_char_p), ("dataurl", c_char_p))
buze_document_keyjazz_key_down = dlsym(libbuzelib, "buze_document_keyjazz_key_down", None, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)), ("wParam", c_int), ("note", c_int))
buze_document_keyjazz_key_up = dlsym(libbuzelib, "buze_document_keyjazz_key_up", c_int, ("Document", POINTER(buze_document_t)), ("wParam", c_int), ("note", c_int), ("plugin", POINTER(zzub_plugin_t)))
buze_document_keyjazz_release = dlsym(libbuzelib, "buze_document_keyjazz_release", None, ("Document", POINTER(buze_document_t)), ("sendnoteoffs", c_int))
buze_document_get_stream_plugin = dlsym(libbuzelib, "buze_document_get_stream_plugin", POINTER(zzub_plugin_t), ("Document", POINTER(buze_document_t)))
buze_document_delete_stream_plugin = dlsym(libbuzelib, "buze_document_delete_stream_plugin", None, ("Document", POINTER(buze_document_t)))
buze_document_get_configuration = dlsym(libbuzelib, "buze_document_get_configuration", POINTER(buze_configuration_t), ("Document", POINTER(buze_document_t)))
buze_document_get_solo_plugin = dlsym(libbuzelib, "buze_document_get_solo_plugin", POINTER(zzub_plugin_t), ("Document", POINTER(buze_document_t)))
buze_document_set_solo_plugin = dlsym(libbuzelib, "buze_document_set_solo_plugin", None, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)), ("state", c_int))
buze_document_get_plugin_helpfile = dlsym(libbuzelib, "buze_document_get_plugin_helpfile", c_char_p, ("Document", POINTER(buze_document_t)), ("loader", POINTER(zzub_pluginloader_t)))
buze_document_import_song = dlsym(libbuzelib, "buze_document_import_song", c_int, ("Document", POINTER(buze_document_t)), ("filename", c_char_p), ("flags", c_int), ("x", c_float), ("y", c_float), ("errormessages", c_char_p), ("outsize", c_int))
buze_document_load_plugin_index = dlsym(libbuzelib, "buze_document_load_plugin_index", c_int, ("Document", POINTER(buze_document_t)))
buze_document_get_plugin_index_item_by_index = dlsym(libbuzelib, "buze_document_get_plugin_index_item_by_index", POINTER(buze_plugin_index_item_t), ("Document", POINTER(buze_document_t)), ("index", c_int))
buze_document_get_plugin_index_root = dlsym(libbuzelib, "buze_document_get_plugin_index_root", POINTER(buze_plugin_index_item_t), ("Document", POINTER(buze_document_t)))
buze_document_is_dirty = dlsym(libbuzelib, "buze_document_is_dirty", c_int, ("Document", POINTER(buze_document_t)))
buze_document_set_current_file = dlsym(libbuzelib, "buze_document_set_current_file", None, ("Document", POINTER(buze_document_t)), ("fullpath", c_char_p))
buze_document_get_current_filename = dlsym(libbuzelib, "buze_document_get_current_filename", c_char_p, ("Document", POINTER(buze_document_t)))
buze_document_get_current_path = dlsym(libbuzelib, "buze_document_get_current_path", c_char_p, ("Document", POINTER(buze_document_t)))
buze_document_get_current_extension = dlsym(libbuzelib, "buze_document_get_current_extension", c_char_p, ("Document", POINTER(buze_document_t)))
buze_document_clear_song = dlsym(libbuzelib, "buze_document_clear_song", None, ("Document", POINTER(buze_document_t)))
buze_document_save_file = dlsym(libbuzelib, "buze_document_save_file", c_int, ("Document", POINTER(buze_document_t)), ("filename", c_char_p), ("withwaves", c_int))
buze_document_create_plugin = dlsym(libbuzelib, "buze_document_create_plugin", POINTER(zzub_plugin_t), ("Document", POINTER(buze_document_t)), ("uri", c_char_p), ("instrumentname", c_char_p), ("x", c_float), ("y", c_float), ("plugingroup", POINTER(zzub_plugin_group_t)))
buze_document_create_plugin_between = dlsym(libbuzelib, "buze_document_create_plugin_between", POINTER(zzub_plugin_t), ("Document", POINTER(buze_document_t)), ("to_plugin", POINTER(zzub_plugin_t)), ("from_plugin", POINTER(zzub_plugin_t)), ("uri", c_char_p), ("instrumentname", c_char_p))
buze_document_create_plugin_before = dlsym(libbuzelib, "buze_document_create_plugin_before", POINTER(zzub_plugin_t), ("Document", POINTER(buze_document_t)), ("srcplugin", POINTER(zzub_plugin_t)), ("uri", c_char_p), ("instrumentname", c_char_p))
buze_document_create_plugin_after = dlsym(libbuzelib, "buze_document_create_plugin_after", POINTER(zzub_plugin_t), ("Document", POINTER(buze_document_t)), ("srcplugin", POINTER(zzub_plugin_t)), ("uri", c_char_p), ("instrumentname", c_char_p))
buze_document_create_plugin_replace = dlsym(libbuzelib, "buze_document_create_plugin_replace", POINTER(zzub_plugin_t), ("Document", POINTER(buze_document_t)), ("srcplugin", POINTER(zzub_plugin_t)), ("uri", c_char_p), ("instrumentname", c_char_p))
buze_document_create_default_document = dlsym(libbuzelib, "buze_document_create_default_document", None, ("Document", POINTER(buze_document_t)))
buze_document_create_default_format = dlsym(libbuzelib, "buze_document_create_default_format", POINTER(zzub_pattern_format_t), ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)), ("simple", c_int))
buze_document_extend_pattern_format = dlsym(libbuzelib, "buze_document_extend_pattern_format", None, ("Document", POINTER(buze_document_t)), ("format", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("simple", c_int))
buze_document_delete_plugin_smart = dlsym(libbuzelib, "buze_document_delete_plugin_smart", None, ("Document", POINTER(buze_document_t)), ("plugin", POINTER(zzub_plugin_t)))
buze_document_get_current_plugin = dlsym(libbuzelib, "buze_document_get_current_plugin", POINTER(zzub_plugin_t), ("Document", POINTER(buze_document_t)))
buze_document_get_current_pattern = dlsym(libbuzelib, "buze_document_get_current_pattern", POINTER(zzub_pattern_t), ("Document", POINTER(buze_document_t)))
buze_document_get_current_pattern_format = dlsym(libbuzelib, "buze_document_get_current_pattern_format", POINTER(zzub_pattern_format_t), ("Document", POINTER(buze_document_t)))
buze_document_get_current_connection = dlsym(libbuzelib, "buze_document_get_current_connection", POINTER(zzub_connection_t), ("Document", POINTER(buze_document_t)))
buze_document_get_current_wave = dlsym(libbuzelib, "buze_document_get_current_wave", POINTER(zzub_wave_t), ("Document", POINTER(buze_document_t)))
buze_document_get_current_wavelevel = dlsym(libbuzelib, "buze_document_get_current_wavelevel", POINTER(zzub_wavelevel_t), ("Document", POINTER(buze_document_t)))
buze_document_import_wave = dlsym(libbuzelib, "buze_document_import_wave", c_int, ("Document", POINTER(buze_document_t)), ("filename", c_char_p), ("target", POINTER(zzub_wave_t)))
buze_document_get_current_order_index = dlsym(libbuzelib, "buze_document_get_current_order_index", c_int, ("Document", POINTER(buze_document_t)))
buze_document_get_current_order_pattern_row = dlsym(libbuzelib, "buze_document_get_current_order_pattern_row", c_int, ("Document", POINTER(buze_document_t)))
buze_document_get_current_pattern_row = dlsym(libbuzelib, "buze_document_get_current_pattern_row", c_int, ("Document", POINTER(buze_document_t)))
buze_plugin_index_item_get_type = dlsym(libbuzelib, "buze_plugin_index_item_get_type", c_int, ("PluginIndexItem", POINTER(buze_plugin_index_item_t)))
buze_plugin_index_item_get_name = dlsym(libbuzelib, "buze_plugin_index_item_get_name", c_char_p, ("PluginIndexItem", POINTER(buze_plugin_index_item_t)))
buze_plugin_index_item_is_hidden = dlsym(libbuzelib, "buze_plugin_index_item_is_hidden", c_int, ("PluginIndexItem", POINTER(buze_plugin_index_item_t)))
buze_plugin_index_item_is_preloaded = dlsym(libbuzelib, "buze_plugin_index_item_is_preloaded", c_int, ("PluginIndexItem", POINTER(buze_plugin_index_item_t)))
buze_plugin_index_item_get_filename = dlsym(libbuzelib, "buze_plugin_index_item_get_filename", c_char_p, ("PluginIndexItem", POINTER(buze_plugin_index_item_t)))
buze_plugin_index_item_get_instrumentname = dlsym(libbuzelib, "buze_plugin_index_item_get_instrumentname", c_char_p, ("PluginIndexItem", POINTER(buze_plugin_index_item_t)))
buze_plugin_index_item_get_sub_item = dlsym(libbuzelib, "buze_plugin_index_item_get_sub_item", POINTER(buze_plugin_index_item_t), ("PluginIndexItem", POINTER(buze_plugin_index_item_t)), ("index", c_int))
buze_plugin_index_item_get_sub_item_count = dlsym(libbuzelib, "buze_plugin_index_item_get_sub_item_count", c_int, ("PluginIndexItem", POINTER(buze_plugin_index_item_t)))
buze_plugin_index_item_get_separator_id = dlsym(libbuzelib, "buze_plugin_index_item_get_separator_id", c_char_p, ("PluginIndexItem", POINTER(buze_plugin_index_item_t)))
buze_configuration_add_sample_path = dlsym(libbuzelib, "buze_configuration_add_sample_path", None, ("Configuration", POINTER(buze_configuration_t)), ("path", c_char_p))
buze_configuration_remove_sample_path = dlsym(libbuzelib, "buze_configuration_remove_sample_path", None, ("Configuration", POINTER(buze_configuration_t)), ("index", c_int))
buze_configuration_get_sample_path_count = dlsym(libbuzelib, "buze_configuration_get_sample_path_count", c_int, ("Configuration", POINTER(buze_configuration_t)))
buze_configuration_get_sample_path = dlsym(libbuzelib, "buze_configuration_get_sample_path", c_char_p, ("Configuration", POINTER(buze_configuration_t)), ("index", c_int))
buze_configuration_get_fixed_width_font = dlsym(libbuzelib, "buze_configuration_get_fixed_width_font", c_char_p, ("Configuration", POINTER(buze_configuration_t)))
buze_configuration_get_toolbars_locked = dlsym(libbuzelib, "buze_configuration_get_toolbars_locked", c_int, ("Configuration", POINTER(buze_configuration_t)))
class EventData(object):
  """EventDatas can be passed as pvoid arguments to document_notify_views() and used by agreeing parties"""
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

buze_event_data_t._wrapper_ = EventData

class Application(object):
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

  @staticmethod
  def create(module, globalPath, userPath, tempPath):
    """Creates an application instance."""
    return Application._new_from_handle(buze_application_create(module, globalPath, userPath, tempPath))

  def get_host_module(self):
    assert self._as_parameter_
    return HostModule._new_from_handle(buze_application_get_host_module(self))

  def destroy(self):
    """Frees all application resources, including shutting down the armstrong audio and midi drivers."""
    assert self._as_parameter_
    buze_application_destroy(self)

  def initialize(self, player, driver):
    """Initializes the application with the supplied Armstrong handles."""
    assert self._as_parameter_
    buze_application_initialize(self, player, driver)

  def open_midi_devices_from_config(self):
    """Initializes a device on the Armstrong audio driver using settings from the Buze configuration."""
    assert self._as_parameter_
    return buze_application_open_midi_devices_from_config(self)

  def create_audio_device_from_config(self):
    """Initializes a device on the Armstrong audio driver using settings from the Buze configuration."""
    assert self._as_parameter_
    return buze_application_create_audio_device_from_config(self)

  def create_audio_device(self, outdevicename, indevicename, rate, buffersize, masterchannel, save):
    """Initializes a device on the Armstrong audio driver."""
    assert self._as_parameter_
    return buze_application_create_audio_device(self, outdevicename, indevicename, rate, buffersize, masterchannel, save)

  def enable_silent_driver(self, enable):
    """Enables or disables silent processing."""
    assert self._as_parameter_
    buze_application_enable_silent_driver(self, enable)

  def get_audio_driver(self):
    """Returns an Armstrong handle to the active audio driver. If silent processing is enabled, the silent driver is returned."""
    assert self._as_parameter_
    return Audiodriver._new_from_handle(buze_application_get_audio_driver(self))

  def release_audio_driver(self):
    """Releases the audio device, driver and all resources."""
    assert self._as_parameter_
    buze_application_release_audio_driver(self)

  def get_configuration(self):
    """Returns the Buze configuration object."""
    assert self._as_parameter_
    return Configuration._new_from_handle(buze_application_get_configuration(self))

  def map_path(self, path, type):
    """Returns a full path relative to path type. The path type indicates either the application or user profile directory."""
    assert self._as_parameter_
    return buze_application_map_path(self, path, type)

  def show_wait_window(self):
    assert self._as_parameter_
    buze_application_show_wait_window(self)

  def set_wait_text(self, text):
    assert self._as_parameter_
    buze_application_set_wait_text(self, text)

  def hide_wait_window(self, focusWnd):
    assert self._as_parameter_
    buze_application_hide_wait_window(self, focusWnd)

  def get_theme_color(self, name):
    assert self._as_parameter_
    return buze_application_get_theme_color(self, name)

  def get_theme_name(self, index):
    assert self._as_parameter_
    return buze_application_get_theme_name(self, index)

  def get_theme_count(self):
    assert self._as_parameter_
    return buze_application_get_theme_count(self)

  def load_theme(self, name):
    assert self._as_parameter_
    buze_application_load_theme(self, name)

buze_application_t._wrapper_ = Application

class MainFrame(object):
  """The main application window of a Buze instance.
  The main frame is implemented by the host by deriving from the CViewFrame interface."""
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

  def initialize(self, parentwnd, player):
    """Initialize the main frame: enumerate views and create the frame window."""
    assert self._as_parameter_
    return buze_main_frame_initialize(self, parentwnd, player)

  def destroy(self):
    """Destroys the mainframe and all its resources."""
    assert self._as_parameter_
    buze_main_frame_destroy(self)

  def register_window_factory(self, info):
    assert self._as_parameter_
    buze_main_frame_register_window_factory(self, info)

  def get_wnd(self):
    """Returns the mainframes window handle."""
    assert self._as_parameter_
    return buze_main_frame_get_wnd(self)

  def add_timer_handler(self, wnd):
    """Adds the view to the mainframe's list of timers.
    The timer calls CView::UpdateTimer() approx every 10 ms."""
    assert self._as_parameter_
    buze_main_frame_add_timer_handler(self, wnd)

  def remove_timer_handler(self, wnd):
    """Removes the view from the list of timers."""
    assert self._as_parameter_
    buze_main_frame_remove_timer_handler(self, wnd)

  def viewstack_insert(self, wnd):
    """Makes the view CTRL+TAB-able."""
    assert self._as_parameter_
    buze_main_frame_viewstack_insert(self, wnd)

  def get_accelerators(self, viewname):
    """Returns an accelerator handle for the specified view."""
    assert self._as_parameter_
    return buze_main_frame_get_accelerators(self, viewname)

  def get_document(self):
    """Returns the document instance."""
    assert self._as_parameter_
    return Document._new_from_handle(buze_main_frame_get_document(self))

  def get_application(self):
    """Returns the application instance."""
    assert self._as_parameter_
    return Application._new_from_handle(buze_main_frame_get_application(self))

  def get_player(self):
    """Returns the Armstrong player instance."""
    assert self._as_parameter_
    return Player._new_from_handle(buze_main_frame_get_player(self))

  def get_focused_view(self):
    assert self._as_parameter_
    return Window._new_from_handle(buze_main_frame_get_focused_view(self))

  def is_float_view(self, wnd):
    assert self._as_parameter_
    return buze_main_frame_is_float_view(self, wnd)

  def set_focus_to(self, wnd):
    assert self._as_parameter_
    buze_main_frame_set_focus_to(self, wnd)

  def get_view(self, viewname, viewid):
    assert self._as_parameter_
    return Window._new_from_handle(buze_main_frame_get_view(self, viewname, viewid))

  def open_view(self, viewname, label, viewid, x, y):
    assert self._as_parameter_
    return Window._new_from_handle(buze_main_frame_open_view(self, viewname, label, viewid, x, y))

  def close_view(self, wnd):
    assert self._as_parameter_
    buze_main_frame_close_view(self, wnd)

  def get_open_filename(self):
    assert self._as_parameter_
    return buze_main_frame_get_open_filename(self)

  def get_save_filename(self):
    assert self._as_parameter_
    return buze_main_frame_get_save_filename(self)

  def get_plugin_menu_create(self):
    assert self._as_parameter_
    return buze_main_frame_get_plugin_menu_create(self)

  def get_plugin_menu_insert_after(self):
    assert self._as_parameter_
    return buze_main_frame_get_plugin_menu_insert_after(self)

  def get_plugin_menu_insert_before(self):
    assert self._as_parameter_
    return buze_main_frame_get_plugin_menu_insert_before(self)

  def get_plugin_menu_replace(self):
    assert self._as_parameter_
    return buze_main_frame_get_plugin_menu_replace(self)

  def get_main_menu(self):
    assert self._as_parameter_
    return buze_main_frame_get_main_menu(self)

  def add_menu_keys(self, viewname, menu):
    assert self._as_parameter_
    buze_main_frame_add_menu_keys(self, viewname, menu)

  def register_event(self):
    assert self._as_parameter_
    return buze_main_frame_register_event(self)

  def register_accelerator_event(self, viewname, defaulthotkey, evcode):
    assert self._as_parameter_
    return buze_main_frame_register_accelerator_event(self, viewname, defaulthotkey, evcode)

  def register_accelerator(self, viewname, name, defaulthotkey, id):
    assert self._as_parameter_
    buze_main_frame_register_accelerator(self, viewname, name, defaulthotkey, id)

  def show_plugin_parameters(self, p, modehint, x, y):
    assert self._as_parameter_
    buze_main_frame_show_plugin_parameters(self, p, modehint, x, y)

  def get_keyjazz_map(self):
    assert self._as_parameter_
    return buze_main_frame_get_keyjazz_map(self)

  def get_view_by_wnd(self, wnd):
    assert self._as_parameter_
    return Window._new_from_handle(buze_main_frame_get_view_by_wnd(self, wnd))

  def get_program_name(self):
    assert self._as_parameter_
    return buze_main_frame_get_program_name(self)

buze_main_frame_t._wrapper_ = MainFrame

class WindowFactory(object):
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

buze_window_factory_t._wrapper_ = WindowFactory

class Window(object):
  """A view, floating or docked in the main frame"""
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

  def get_wnd(self):
    assert self._as_parameter_
    return buze_window_get_wnd(self)

buze_window_t._wrapper_ = Window

class EventHandler(object):
  """Interface for internal events via CEventHandler::Update"""
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

buze_event_handler_t._wrapper_ = EventHandler

class MessageFilter(object):
  """Interface for WTL events via CMessageFilter::PreTranslateMessage"""
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

buze_message_filter_t._wrapper_ = MessageFilter

class IdleHandler(object):
  """Interface for WTL events CIdleHandler::OnIdle"""
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

buze_idle_handler_t._wrapper_ = IdleHandler

class MessageLoop(object):
  """Interface for the host's WTL CMessageLoop"""
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

buze_message_loop_t._wrapper_ = MessageLoop

class HostModule(object):
  """Interface for the host's CAppModule, used to forward _Module features to the DLL plugins"""
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

buze_host_module_t._wrapper_ = HostModule

class Document(object):
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

  def add_view(self, view):
    assert self._as_parameter_
    buze_document_add_view(self, view)

  def remove_view(self, view):
    assert self._as_parameter_
    buze_document_remove_view(self, view)

  def notify_views(self, sender, hint, param):
    assert self._as_parameter_
    buze_document_notify_views(self, sender, hint, param)

  def add_callback(self, callback, tag):
    assert self._as_parameter_
    # wrap callback
    cb_callback = callback._from_function(callback)
    buze_document_add_callback(self, cb_callback._function_handle, tag)

  def remove_callback(self, callback, tag):
    assert self._as_parameter_
    # wrap callback
    cb_callback = callback._from_function(callback)
    buze_document_remove_callback(self, cb_callback._function_handle, tag)

  def get_octave(self):
    assert self._as_parameter_
    return buze_document_get_octave(self)

  def set_octave(self, oct):
    assert self._as_parameter_
    buze_document_set_octave(self, oct)

  def get_plugin_non_song(self, plugin):
    assert self._as_parameter_
    return buze_document_get_plugin_non_song(self, plugin)

  def set_plugin_non_song(self, plugin, state):
    assert self._as_parameter_
    buze_document_set_plugin_non_song(self, plugin, state)

  def get_plugin_parameter_view_mode(self, plugin):
    assert self._as_parameter_
    return buze_document_get_plugin_parameter_view_mode(self, plugin)

  def set_plugin_parameter_view_mode(self, plugin, mode):
    assert self._as_parameter_
    buze_document_set_plugin_parameter_view_mode(self, plugin, mode)

  def get_plugin_last_preset(self, plugin):
    assert self._as_parameter_
    return buze_document_get_plugin_last_preset(self, plugin)

  def set_plugin_last_preset(self, plugin, preset):
    assert self._as_parameter_
    buze_document_set_plugin_last_preset(self, plugin, preset)

  def get_player(self):
    assert self._as_parameter_
    return Player._new_from_handle(buze_document_get_player(self))

  def get_stream_plugin_uri_for_file(self, path):
    assert self._as_parameter_
    return buze_document_get_stream_plugin_uri_for_file(self, path)

  def play_plugin_note(self, plugin, note, prevnote):
    assert self._as_parameter_
    buze_document_play_plugin_note(self, plugin, note, prevnote)

  def play_stream(self, note, offset, length, pluginuri, dataurl):
    assert self._as_parameter_
    buze_document_play_stream(self, note, offset, length, pluginuri, dataurl)

  def keyjazz_key_down(self, plugin, wParam, note):
    assert self._as_parameter_
    buze_document_keyjazz_key_down(self, plugin, wParam, note)

  def keyjazz_key_up(self, wParam):
    assert self._as_parameter_
    note = (c_int)()
    plugin = (POINTER(zzub_plugin_t))()
    _ret_val = buze_document_keyjazz_key_up(self, wParam, note, plugin)
    return _ret_val, note.value, plugin.value

  def keyjazz_release(self, sendnoteoffs):
    assert self._as_parameter_
    buze_document_keyjazz_release(self, sendnoteoffs)

  def get_stream_plugin(self):
    assert self._as_parameter_
    return Plugin._new_from_handle(buze_document_get_stream_plugin(self))

  def delete_stream_plugin(self):
    assert self._as_parameter_
    buze_document_delete_stream_plugin(self)

  def get_configuration(self):
    """Returns the Buze configuration object."""
    assert self._as_parameter_
    return Configuration._new_from_handle(buze_document_get_configuration(self))

  def get_solo_plugin(self):
    assert self._as_parameter_
    return Plugin._new_from_handle(buze_document_get_solo_plugin(self))

  def set_solo_plugin(self, plugin, state):
    assert self._as_parameter_
    buze_document_set_solo_plugin(self, plugin, state)

  def get_plugin_helpfile(self, loader):
    assert self._as_parameter_
    return buze_document_get_plugin_helpfile(self, loader)

  def import_song(self, filename, flags, x, y, outsize):
    assert self._as_parameter_
    errormessages = (c_char*outsize)()
    _ret_val = buze_document_import_song(self, filename, flags, x, y, errormessages, outsize)
    return _ret_val, errormessages.value

  def load_plugin_index(self):
    """Initializes the plugin index: Loads Gear/index.txt and enumerates songs in Gear/Templates."""
    assert self._as_parameter_
    return buze_document_load_plugin_index(self)

  def get_plugin_index_item_by_index(self, index):
    assert self._as_parameter_
    return PluginIndexItem._new_from_handle(buze_document_get_plugin_index_item_by_index(self, index))

  def get_plugin_index_root(self):
    assert self._as_parameter_
    return PluginIndexItem._new_from_handle(buze_document_get_plugin_index_root(self))

  def is_dirty(self):
    assert self._as_parameter_
    return buze_document_is_dirty(self)

  def set_current_file(self, fullpath):
    assert self._as_parameter_
    buze_document_set_current_file(self, fullpath)

  def get_current_filename(self):
    assert self._as_parameter_
    return buze_document_get_current_filename(self)

  def get_current_path(self):
    assert self._as_parameter_
    return buze_document_get_current_path(self)

  def get_current_extension(self):
    assert self._as_parameter_
    return buze_document_get_current_extension(self)

  def clear_song(self):
    assert self._as_parameter_
    buze_document_clear_song(self)

  def save_file(self, filename, withwaves):
    assert self._as_parameter_
    return buze_document_save_file(self, filename, withwaves)

  def create_plugin(self, uri, instrumentname, x, y, plugingroup):
    assert self._as_parameter_
    return Plugin._new_from_handle(buze_document_create_plugin(self, uri, instrumentname, x, y, plugingroup))

  def create_plugin_between(self, to_plugin, from_plugin, uri, instrumentname):
    assert self._as_parameter_
    return Plugin._new_from_handle(buze_document_create_plugin_between(self, to_plugin, from_plugin, uri, instrumentname))

  def create_plugin_before(self, srcplugin, uri, instrumentname):
    assert self._as_parameter_
    return Plugin._new_from_handle(buze_document_create_plugin_before(self, srcplugin, uri, instrumentname))

  def create_plugin_after(self, srcplugin, uri, instrumentname):
    assert self._as_parameter_
    return Plugin._new_from_handle(buze_document_create_plugin_after(self, srcplugin, uri, instrumentname))

  def create_plugin_replace(self, srcplugin, uri, instrumentname):
    assert self._as_parameter_
    return Plugin._new_from_handle(buze_document_create_plugin_replace(self, srcplugin, uri, instrumentname))

  def create_default_document(self):
    assert self._as_parameter_
    buze_document_create_default_document(self)

  def create_default_format(self, plugin, simple):
    assert self._as_parameter_
    return PatternFormat._new_from_handle(buze_document_create_default_format(self, plugin, simple))

  def extend_pattern_format(self, format, plugin, simple):
    assert self._as_parameter_
    buze_document_extend_pattern_format(self, format, plugin, simple)

  def delete_plugin_smart(self, plugin):
    assert self._as_parameter_
    buze_document_delete_plugin_smart(self, plugin)

  def get_current_plugin(self):
    assert self._as_parameter_
    return Plugin._new_from_handle(buze_document_get_current_plugin(self))

  def get_current_pattern(self):
    assert self._as_parameter_
    return Pattern._new_from_handle(buze_document_get_current_pattern(self))

  def get_current_pattern_format(self):
    assert self._as_parameter_
    return PatternFormat._new_from_handle(buze_document_get_current_pattern_format(self))

  def get_current_connection(self):
    assert self._as_parameter_
    return Connection._new_from_handle(buze_document_get_current_connection(self))

  def get_current_wave(self):
    assert self._as_parameter_
    return Wave._new_from_handle(buze_document_get_current_wave(self))

  def get_current_wavelevel(self):
    assert self._as_parameter_
    return Wavelevel._new_from_handle(buze_document_get_current_wavelevel(self))

  def import_wave(self, filename, target):
    assert self._as_parameter_
    return buze_document_import_wave(self, filename, target)

  def get_current_order_index(self):
    assert self._as_parameter_
    return buze_document_get_current_order_index(self)

  def get_current_order_pattern_row(self):
    assert self._as_parameter_
    return buze_document_get_current_order_pattern_row(self)

  def get_current_pattern_row(self):
    assert self._as_parameter_
    return buze_document_get_current_pattern_row(self)

buze_document_t._wrapper_ = Document

class PluginIndexItem(object):
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

  def get_type(self):
    """Index type, 0 = folder, 1 = plugin/template, 2 = separator"""
    assert self._as_parameter_
    return buze_plugin_index_item_get_type(self)

  def get_name(self):
    assert self._as_parameter_
    return buze_plugin_index_item_get_name(self)

  def is_hidden(self):
    assert self._as_parameter_
    return buze_plugin_index_item_is_hidden(self)

  def is_preloaded(self):
    assert self._as_parameter_
    return buze_plugin_index_item_is_preloaded(self)

  def get_filename(self):
    assert self._as_parameter_
    return buze_plugin_index_item_get_filename(self)

  def get_instrumentname(self):
    assert self._as_parameter_
    return buze_plugin_index_item_get_instrumentname(self)

  def get_sub_item(self, index):
    assert self._as_parameter_
    return PluginIndexItem._new_from_handle(buze_plugin_index_item_get_sub_item(self, index))

  def get_sub_item_count(self):
    assert self._as_parameter_
    return buze_plugin_index_item_get_sub_item_count(self)

  def get_separator_id(self):
    """Returns an identifier to a .plur image to use in place of the separator"""
    assert self._as_parameter_
    return buze_plugin_index_item_get_separator_id(self)

buze_plugin_index_item_t._wrapper_ = PluginIndexItem

class Configuration(object):
  _as_parameter_ = None
  _hash = 0

  def __init__(self, handle):
    self._as_parameter_ = handle
    self._hash = cast(self._as_parameter_, c_void_p).value

  @classmethod
  def _new_from_handle(cls,handle):
    if not handle:
      return None
    return cls(handle)

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

  def add_sample_path(self, path):
    assert self._as_parameter_
    buze_configuration_add_sample_path(self, path)

  def remove_sample_path(self, index):
    assert self._as_parameter_
    buze_configuration_remove_sample_path(self, index)

  def get_sample_path_count(self):
    assert self._as_parameter_
    return buze_configuration_get_sample_path_count(self)

  def get_sample_path(self, index):
    assert self._as_parameter_
    return buze_configuration_get_sample_path(self, index)

  def get_fixed_width_font(self):
    assert self._as_parameter_
    return buze_configuration_get_fixed_width_font(self)

  def get_toolbars_locked(self):
    """Returns true if the toolbars are locked."""
    assert self._as_parameter_
    return buze_configuration_get_toolbars_locked(self)

buze_configuration_t._wrapper_ = Configuration

class callback(object):
  _function = None
  _function_handle = None
  _hash = 0

  def __init__(self, function):
    self._function = function
    self._function_handle = buze_callback_t(self.wrapper_function)
    self._hash = cast(self._function_handle, c_void_p).value

  @classmethod
  def _from_function(cls, function):
    functionhash = hash(function)
    if functionhash in Callback._instances_:
      return Callback._instances_[functionhash]
    instance = cls(function)
    Callback._instances_[functionhash] = instance
    return instance

  def __hash__(self):
    return self._hash

  def __eq__(self,other):
    return self._hash == hash(other)

  def __ne__(self,other):
    return self._hash != hash(other)

  def wrapper_function(self, sender, hint, param, tag):
    self._function(Window._new_from_handle(sender), hint, param, tag)

callback._instances_ = {}

