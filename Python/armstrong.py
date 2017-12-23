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

libarmstrong = dlopen("armstrong", "0.3")

# enum zzub_event_type
zzub_event_type_double_click = 0
zzub_event_type_update_song = 60
zzub_event_type_insert_plugin = 1
zzub_event_type_before_delete_plugin = 5
zzub_event_type_delete_plugin = 2
zzub_event_type_update_plugin = 30
zzub_event_type_insert_connection = 4
zzub_event_type_delete_connection = 3
zzub_event_type_update_connection = 68
zzub_event_type_update_pluginparameter = 7
zzub_event_type_insert_pattern = 25
zzub_event_type_update_pattern = 31
zzub_event_type_delete_pattern = 26
zzub_event_type_insert_patternevent = 27
zzub_event_type_update_patternevent = 48
zzub_event_type_delete_patternevent = 49
zzub_event_type_insert_orderlist = 32
zzub_event_type_delete_orderlist = 33
zzub_event_type_update_orderlist = 41
zzub_event_type_insert_patternformat = 52
zzub_event_type_update_patternformat = 61
zzub_event_type_delete_patternformat = 53
zzub_event_type_insert_patternformatcolumn = 54
zzub_event_type_update_patternformatcolumn = 62
zzub_event_type_delete_patternformatcolumn = 55
zzub_event_type_insert_patternformattrack = 63
zzub_event_type_update_patternformattrack = 64
zzub_event_type_delete_patternformattrack = 65
zzub_event_type_insert_plugin_group = 71
zzub_event_type_update_plugin_group = 72
zzub_event_type_delete_plugin_group = 73
zzub_event_type_envelope_changed = 37
zzub_event_type_slices_changed = 38
zzub_event_type_insert_wave = 56
zzub_event_type_update_wave = 39
zzub_event_type_delete_wave = 40
zzub_event_type_insert_wavelevel = 12
zzub_event_type_update_wavelevel = 57
zzub_event_type_delete_wavelevel = 58
zzub_event_type_update_wavelevel_samples = 59
zzub_event_type_user_alert = 8
zzub_event_type_midi_control = 11
zzub_event_type_player_state_changed = 20
zzub_event_type_osc_message = 21
zzub_event_type_vu = 22
zzub_event_type_player_order_changed = 69
zzub_event_type_player_order_queue_changed = 70
zzub_event_type_custom = 44
zzub_event_type_samplerate_changed = 50
zzub_event_type_latency_changed = 76
zzub_event_type_device_reset = 77
zzub_event_type_barrier = 51
zzub_event_type_player_save = 74
zzub_event_type_player_load = 75
zzub_event_type_all = 255


# enum zzub_alert_type
zzub_alert_type_enumerating_plugins = 1
zzub_alert_type_enumerating_plugins_done = 2
zzub_alert_type_mixdown_progress = 20
zzub_alert_type_loading_plugins = 100
zzub_alert_type_loading_patterns = 101
zzub_alert_type_loading_waves = 102
zzub_alert_type_pattern_recursion = 400


# enum zzub_validation_error_type
zzub_validation_error_type_parameter_count_mismatch = 1
zzub_validation_error_type_parameter_type_mismatch = 2
zzub_validation_error_type_parameter_flags_mismatch = 3
zzub_validation_error_type_parameter_value_min_mismatch = 4
zzub_validation_error_type_parameter_value_max_mismatch = 5
zzub_validation_error_type_parameter_value_none_mismatch = 6
zzub_validation_error_type_parameter_value_default_mismatch = 7
zzub_validation_error_type_plugin_not_found_using_dummy = 8
zzub_validation_error_type_plugin_validation_failed_using_dummy = 9
zzub_validation_error_type_plugin_not_found = 10
zzub_validation_error_type_plugin_inputs_mismatch = 11
zzub_validation_error_type_plugin_outputs_mismatch = 12


# enum zzub_wave_importer_type
zzub_wave_importer_type_wave_file = 0
zzub_wave_importer_type_wave_archive = 1
zzub_wave_importer_type_instrument_file = 2
zzub_wave_importer_type_instrument_archive = 3


# nameless enum
zzub_version = 15
zzub_buffer_size = 256


# enum zzub_player_state
zzub_player_state_playing = 0
zzub_player_state_stopped = 1
zzub_player_state_muted = 2
zzub_player_state_released = 3


# enum zzub_parameter_type
zzub_parameter_type_note = 0
zzub_parameter_type_switch = 1
zzub_parameter_type_byte = 2
zzub_parameter_type_word = 3
zzub_parameter_type_meta = 4


# enum zzub_wave_buffer_type
zzub_wave_buffer_type_si16 = 0
zzub_wave_buffer_type_f32 = 1
zzub_wave_buffer_type_si32 = 2
zzub_wave_buffer_type_si24 = 3


# enum zzub_oscillator_type
zzub_oscillator_type_sine = 0
zzub_oscillator_type_sawtooth = 1
zzub_oscillator_type_pulse = 2
zzub_oscillator_type_triangle = 3
zzub_oscillator_type_noise = 4
zzub_oscillator_type_sawtooth_303 = 5


# enum zzub_note_value
zzub_note_value_none = 0
zzub_note_value_off = 255
zzub_note_value_cut = 254
zzub_note_value_min = 1
zzub_note_value_max = 156
zzub_note_value_c4 = 65


# enum zzub_switch_value
zzub_switch_value_none = 255
zzub_switch_value_off = 0
zzub_switch_value_on = 1


# enum zzub_wavetable_index_value
zzub_wavetable_index_value_none = 0
zzub_wavetable_index_value_min = 1
zzub_wavetable_index_value_max = 200


# enum zzub_parameter_flag
zzub_parameter_flag_wavetable_index = 1
zzub_parameter_flag_state = 2
zzub_parameter_flag_event_on_edit = 4
zzub_parameter_flag_pattern_index = 8
zzub_parameter_flag_velocity_index = 16
zzub_parameter_flag_delay_index = 32
zzub_parameter_flag_compound = 64
zzub_parameter_flag_char_index = 128
zzub_parameter_flag_harmony_index = 256
zzub_parameter_flag_meta_note = 512
zzub_parameter_flag_meta_wave = 1024


# enum zzub_plugin_flag
zzub_plugin_flag_plays_waves = 2
zzub_plugin_flag_uses_lib_interface = 4
zzub_plugin_flag_does_input_mixing = 16
zzub_plugin_flag_is_singleton = 32768
zzub_plugin_flag_is_root = 65536
zzub_plugin_flag_has_audio_input = 131072
zzub_plugin_flag_has_audio_output = 262144
zzub_plugin_flag_is_offline = 524288
zzub_plugin_flag_has_event_output = 1048576
zzub_plugin_flag_stream = 4194304
zzub_plugin_flag_has_midi_input = 8388608
zzub_plugin_flag_has_midi_output = 16777216
zzub_plugin_flag_has_group_input = 33554432
zzub_plugin_flag_has_group_output = 67108864
zzub_plugin_flag_is_sequence = 134217728
zzub_plugin_flag_is_connection = 268435456
zzub_plugin_flag_is_interval = 536870912
zzub_plugin_flag_is_encoder = 1073741824
zzub_plugin_flag_has_note_output = 2147483648


# enum zzub_wave_flag
zzub_wave_flag_loop = 1
zzub_wave_flag_extended = 4
zzub_wave_flag_stereo = 8
zzub_wave_flag_pingpong = 16
zzub_wave_flag_envelope = 128


# enum zzub_envelope_flag
zzub_envelope_flag_sustain = 1
zzub_envelope_flag_loop = 2


# enum zzub_process_mode
zzub_process_mode_no_io = 0
zzub_process_mode_read = 1
zzub_process_mode_write = 2
zzub_process_mode_read_write = 3


# enum zzub_encoder_state
zzub_encoder_state_playing = 1
zzub_encoder_state_stopped = 2
zzub_encoder_state_deleted = 4
zzub_encoder_state_seeking = 8
zzub_encoder_state_created = 16


# enum zzub_connection_type
zzub_connection_type_audio = 0
zzub_connection_type_event = 1
zzub_connection_type_midi = 2
zzub_connection_type_note = 3


# enum zzub_parameter_group
zzub_parameter_group_internal = 0
zzub_parameter_group_global = 1
zzub_parameter_group_track = 2
zzub_parameter_group_controller = 3
zzub_parameter_group_virtual = 4

class zzub_event_data_t(Structure):
  _fields_ = [
    ("type", c_int),
    ("userdata", c_void_p),
  ]
  _anonymous_ = [
  ]

class zzub_plugin_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_double_click_t(Structure):
  _fields_ = [
    ("plugin", POINTER(zzub_plugin_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_insert_plugin_t(Structure):
  _fields_ = [
    ("plugin", POINTER(zzub_plugin_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_delete_plugin_t(Structure):
  _fields_ = [
    ("plugin", POINTER(zzub_plugin_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_plugin_t(Structure):
  _fields_ = [
    ("plugin", POINTER(zzub_plugin_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_insert_connection_t(Structure):
  _fields_ = [
    ("from_plugin", POINTER(zzub_plugin_t)),
    ("to_plugin", POINTER(zzub_plugin_t)),
    ("connection_plugin", POINTER(zzub_plugin_t)),
    ("type", c_int),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_connection_t(Structure):
  _fields_ = [
    ("connection_plugin", POINTER(zzub_plugin_t)),
    ("from_plugin", POINTER(zzub_plugin_t)),
    ("to_plugin", POINTER(zzub_plugin_t)),
    ("type", c_int),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_delete_connection_t(Structure):
  _fields_ = [
    ("connection_plugin", POINTER(zzub_plugin_t)),
    ("from_plugin", POINTER(zzub_plugin_t)),
    ("to_plugin", POINTER(zzub_plugin_t)),
    ("type", c_int),
  ]
  _anonymous_ = [
  ]

class zzub_pattern_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_insert_pattern_t(Structure):
  _fields_ = [
    ("pattern", POINTER(zzub_pattern_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_pattern_t(Structure):
  _fields_ = [
    ("pattern", POINTER(zzub_pattern_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_delete_pattern_t(Structure):
  _fields_ = [
    ("pattern", POINTER(zzub_pattern_t)),
  ]
  _anonymous_ = [
  ]

class zzub_pattern_event_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_insert_pattern_event_t(Structure):
  _fields_ = [
    ("patternevent", POINTER(zzub_pattern_event_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_pattern_event_t(Structure):
  _fields_ = [
    ("patternevent", POINTER(zzub_pattern_event_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_delete_pattern_event_t(Structure):
  _fields_ = [
    ("patternevent", POINTER(zzub_pattern_event_t)),
  ]
  _anonymous_ = [
  ]

class zzub_pattern_format_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_insert_pattern_format_t(Structure):
  _fields_ = [
    ("patternformat", POINTER(zzub_pattern_format_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_pattern_format_t(Structure):
  _fields_ = [
    ("patternformat", POINTER(zzub_pattern_format_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_delete_pattern_format_t(Structure):
  _fields_ = [
    ("patternformat", POINTER(zzub_pattern_format_t)),
  ]
  _anonymous_ = [
  ]

class zzub_pattern_format_column_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_insert_pattern_format_column_t(Structure):
  _fields_ = [
    ("patternformatcolumn", POINTER(zzub_pattern_format_column_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_pattern_format_column_t(Structure):
  _fields_ = [
    ("patternformatcolumn", POINTER(zzub_pattern_format_column_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_delete_pattern_format_column_t(Structure):
  _fields_ = [
    ("patternformatcolumn", POINTER(zzub_pattern_format_column_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_midi_message_t(Structure):
  _fields_ = [
    ("status", c_ubyte),
    ("data1", c_ubyte),
    ("data2", c_ubyte),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_plugin_parameter_t(Structure):
  _fields_ = [
    ("plugin", POINTER(zzub_plugin_t)),
    ("group", c_int),
    ("track", c_int),
    ("param", c_int),
    ("value", c_int),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_player_state_changed_t(Structure):
  _fields_ = [
    ("player_state", c_int),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_player_order_changed_t(Structure):
  _fields_ = [
    ("orderindex", c_int),
  ]
  _anonymous_ = [
  ]

class zzub_archive_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_player_load_t(Structure):
  _fields_ = [
    ("userdata", POINTER(zzub_archive_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_player_save_t(Structure):
  _fields_ = [
    ("userdata", POINTER(zzub_archive_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_vu_t(Structure):
  _fields_ = [
    ("size", c_int),
    ("left_amp", c_float),
    ("right_amp", c_float),
    ("time", c_float),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_serialize_t(Structure):
  _fields_ = [
    ("mode", c_char),
    ("archive", POINTER(zzub_archive_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_unknown_t(Structure):
  _fields_ = [
    ("param", c_void_p),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_osc_message_t(Structure):
  _fields_ = [
    ("path", c_char_p),
    ("types", c_char_p),
    ("argv", POINTER(c_void_p)),
    ("argc", c_int),
    ("msg", c_void_p),
  ]
  _anonymous_ = [
  ]

class zzub_wave_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_insert_wave_t(Structure):
  _fields_ = [
    ("wave", POINTER(zzub_wave_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_wave_t(Structure):
  _fields_ = [
    ("wave", POINTER(zzub_wave_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_delete_wave_t(Structure):
  _fields_ = [
    ("wave", POINTER(zzub_wave_t)),
  ]
  _anonymous_ = [
  ]

class zzub_wavelevel_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_insert_wavelevel_t(Structure):
  _fields_ = [
    ("wavelevel", POINTER(zzub_wavelevel_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_wavelevel_t(Structure):
  _fields_ = [
    ("wavelevel", POINTER(zzub_wavelevel_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_wavelevel_samples_t(Structure):
  _fields_ = [
    ("wavelevel", POINTER(zzub_wavelevel_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_delete_wavelevel_t(Structure):
  _fields_ = [
    ("wavelevel", POINTER(zzub_wavelevel_t)),
  ]
  _anonymous_ = [
  ]

class zzub_plugin_group_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_insert_plugin_group_t(Structure):
  _fields_ = [
    ("group", POINTER(zzub_plugin_group_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_delete_plugin_group_t(Structure):
  _fields_ = [
    ("group", POINTER(zzub_plugin_group_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_update_plugin_group_t(Structure):
  _fields_ = [
    ("group", POINTER(zzub_plugin_group_t)),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_custom_t(Structure):
  _fields_ = [
    ("id", c_int),
    ("data", c_void_p),
  ]
  _anonymous_ = [
  ]

class zzub_event_data_user_alert_t(Structure):
  _fields_ = [
    ("type", c_int),
  ]
  _anonymous_ = [
  ]

class zzub_plugincollection_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_pluginloader_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_event_data_user_alert_union_00000000_t(Union):
    ("collection", zzub_plugincollection_t),
    ("plugin", zzub_pluginloader_t),
    ("wave", zzub_wave_t),
    ("progress", c_int),
class zzub_event_data_union_00000001_t(Union):
    ("double_click", zzub_event_data_double_click_t),
    ("midi_message", zzub_event_data_midi_message_t),
    ("player_state_changed", zzub_event_data_player_state_changed_t),
    ("player_order_changed", zzub_event_data_player_order_changed_t),
    ("player_load", zzub_event_data_player_load_t),
    ("player_save", zzub_event_data_player_save_t),
    ("osc_message", zzub_event_data_osc_message_t),
    ("vu", zzub_event_data_vu_t),
    ("serialize", zzub_event_data_serialize_t),
    ("insert_plugin", zzub_event_data_insert_plugin_t),
    ("update_plugin", zzub_event_data_update_plugin_t),
    ("delete_plugin", zzub_event_data_delete_plugin_t),
    ("update_pluginparameter", zzub_event_data_update_plugin_parameter_t),
    ("insert_connection", zzub_event_data_insert_connection_t),
    ("update_connection", zzub_event_data_update_connection_t),
    ("delete_connection", zzub_event_data_delete_connection_t),
    ("insert_pattern", zzub_event_data_insert_pattern_t),
    ("update_pattern", zzub_event_data_update_pattern_t),
    ("delete_pattern", zzub_event_data_delete_pattern_t),
    ("insert_patternevent", zzub_event_data_insert_pattern_event_t),
    ("update_patternevent", zzub_event_data_update_pattern_event_t),
    ("delete_patternevent", zzub_event_data_delete_pattern_event_t),
    ("insert_pattern_format", zzub_event_data_insert_pattern_format_t),
    ("update_pattern_format", zzub_event_data_update_pattern_format_t),
    ("delete_pattern_format", zzub_event_data_delete_pattern_format_t),
    ("insert_pattern_format_column", zzub_event_data_insert_pattern_format_column_t),
    ("update_pattern_format_column", zzub_event_data_update_pattern_format_column_t),
    ("delete_pattern_format_column", zzub_event_data_delete_pattern_format_column_t),
    ("insert_wave", zzub_event_data_insert_wave_t),
    ("update_wave", zzub_event_data_update_wave_t),
    ("delete_wave", zzub_event_data_delete_wave_t),
    ("insert_wavelevel", zzub_event_data_insert_wavelevel_t),
    ("update_wavelevel", zzub_event_data_update_wavelevel_t),
    ("delete_wavelevel", zzub_event_data_delete_wavelevel_t),
    ("update_wavelevel_samples", zzub_event_data_update_wavelevel_samples_t),
    ("insert_plugin_group", zzub_event_data_insert_plugin_group_t),
    ("update_plugin_group", zzub_event_data_update_plugin_group_t),
    ("delete_plugin_group", zzub_event_data_delete_plugin_group_t),
    ("alert", zzub_event_data_user_alert_t),
    ("custom", zzub_event_data_custom_t),
    ("unknown", zzub_event_data_unknown_t),
class zzub_device_info_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_device_info_iterator_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_audiodriver_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_mididriver_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_input_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_output_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_midimapping_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_pattern_iterator_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_pattern_event_iterator_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_pattern_format_iterator_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_pattern_format_column_iterator_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_parameter_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_attribute_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_plugin_iterator_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_connection_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_connection_binding_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_connection_binding_iterator_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_envelope_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_mixer_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_validation_error_iterator_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_validation_error_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_player_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_plugin_group_iterator_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

class zzub_wave_importer_t(Structure):
  _fields_ = [
  ]
  _anonymous_ = [
  ]

zzub_callback_t = CFUNCTYPE(c_int, POINTER(zzub_player_t), POINTER(zzub_plugin_t), POINTER(zzub_event_data_t), c_void_p)
zzub_device_info_get_api = dlsym(libarmstrong, "zzub_device_info_get_api", c_int, ("DeviceInfo", POINTER(zzub_device_info_t)))
zzub_device_info_get_name = dlsym(libarmstrong, "zzub_device_info_get_name", c_char_p, ("DeviceInfo", POINTER(zzub_device_info_t)))
zzub_device_info_get_supported_buffersizes = dlsym(libarmstrong, "zzub_device_info_get_supported_buffersizes", c_int, ("DeviceInfo", POINTER(zzub_device_info_t)), ("result", POINTER(c_int)), ("maxsizes", c_int))
zzub_device_info_get_supported_samplerates = dlsym(libarmstrong, "zzub_device_info_get_supported_samplerates", c_int, ("DeviceInfo", POINTER(zzub_device_info_t)), ("result", POINTER(c_int)), ("maxrates", c_int))
zzub_device_info_get_supported_output_channels = dlsym(libarmstrong, "zzub_device_info_get_supported_output_channels", c_int, ("DeviceInfo", POINTER(zzub_device_info_t)))
zzub_device_info_get_supported_input_channels = dlsym(libarmstrong, "zzub_device_info_get_supported_input_channels", c_int, ("DeviceInfo", POINTER(zzub_device_info_t)))
zzub_device_info_iterator_next = dlsym(libarmstrong, "zzub_device_info_iterator_next", None, ("DeviceInfoIterator", POINTER(zzub_device_info_iterator_t)))
zzub_device_info_iterator_valid = dlsym(libarmstrong, "zzub_device_info_iterator_valid", c_int, ("DeviceInfoIterator", POINTER(zzub_device_info_iterator_t)))
zzub_device_info_iterator_current = dlsym(libarmstrong, "zzub_device_info_iterator_current", POINTER(zzub_device_info_t), ("DeviceInfoIterator", POINTER(zzub_device_info_iterator_t)))
zzub_device_info_iterator_reset = dlsym(libarmstrong, "zzub_device_info_iterator_reset", None, ("DeviceInfoIterator", POINTER(zzub_device_info_iterator_t)))
zzub_device_info_iterator_destroy = dlsym(libarmstrong, "zzub_device_info_iterator_destroy", None, ("DeviceInfoIterator", POINTER(zzub_device_info_iterator_t)))
zzub_audiodriver_create_silent = dlsym(libarmstrong, "zzub_audiodriver_create_silent", POINTER(zzub_audiodriver_t), ("player", POINTER(zzub_player_t)), ("name", c_char_p), ("out_channels", c_int), ("in_channels", c_int), ("supported_rates", POINTER(c_int)), ("num_rates", c_int))
zzub_audiodriver_create = dlsym(libarmstrong, "zzub_audiodriver_create", POINTER(zzub_audiodriver_t), ("player", POINTER(zzub_player_t)))
zzub_audiodriver_get_count = dlsym(libarmstrong, "zzub_audiodriver_get_count", c_int, ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_get_device_info = dlsym(libarmstrong, "zzub_audiodriver_get_device_info", POINTER(zzub_device_info_t), ("Audiodriver", POINTER(zzub_audiodriver_t)), ("index", c_int))
zzub_audiodriver_get_device_info_by_name = dlsym(libarmstrong, "zzub_audiodriver_get_device_info_by_name", POINTER(zzub_device_info_t), ("Audiodriver", POINTER(zzub_audiodriver_t)), ("name", c_char_p))
zzub_audiodriver_get_output_iterator = dlsym(libarmstrong, "zzub_audiodriver_get_output_iterator", POINTER(zzub_device_info_iterator_t), ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_get_input_iterator = dlsym(libarmstrong, "zzub_audiodriver_get_input_iterator", POINTER(zzub_device_info_iterator_t), ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_get_input_iterator_for_output = dlsym(libarmstrong, "zzub_audiodriver_get_input_iterator_for_output", POINTER(zzub_device_info_iterator_t), ("Audiodriver", POINTER(zzub_audiodriver_t)), ("info", POINTER(zzub_device_info_t)))
zzub_audiodriver_create_device = dlsym(libarmstrong, "zzub_audiodriver_create_device", c_int, ("Audiodriver", POINTER(zzub_audiodriver_t)), ("input_name", c_char_p), ("output_name", c_char_p), ("buffersize", c_int), ("samplerate", c_int))
zzub_audiodriver_get_current_device = dlsym(libarmstrong, "zzub_audiodriver_get_current_device", POINTER(zzub_device_info_t), ("Audiodriver", POINTER(zzub_audiodriver_t)), ("for_input", c_int))
zzub_audiodriver_enable = dlsym(libarmstrong, "zzub_audiodriver_enable", None, ("Audiodriver", POINTER(zzub_audiodriver_t)), ("state", c_int))
zzub_audiodriver_get_enabled = dlsym(libarmstrong, "zzub_audiodriver_get_enabled", c_int, ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_destroy = dlsym(libarmstrong, "zzub_audiodriver_destroy", None, ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_destroy_device = dlsym(libarmstrong, "zzub_audiodriver_destroy_device", None, ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_get_samplerate = dlsym(libarmstrong, "zzub_audiodriver_get_samplerate", c_uint, ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_get_buffersize = dlsym(libarmstrong, "zzub_audiodriver_get_buffersize", c_uint, ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_get_cpu_load = dlsym(libarmstrong, "zzub_audiodriver_get_cpu_load", c_double, ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_get_master_channel = dlsym(libarmstrong, "zzub_audiodriver_get_master_channel", c_int, ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_audiodriver_set_master_channel = dlsym(libarmstrong, "zzub_audiodriver_set_master_channel", None, ("Audiodriver", POINTER(zzub_audiodriver_t)), ("index", c_int))
zzub_audiodriver_configure = dlsym(libarmstrong, "zzub_audiodriver_configure", None, ("Audiodriver", POINTER(zzub_audiodriver_t)))
zzub_mididriver_get_count = dlsym(libarmstrong, "zzub_mididriver_get_count", c_int, ("player", POINTER(zzub_player_t)))
zzub_mididriver_get_name = dlsym(libarmstrong, "zzub_mididriver_get_name", c_char_p, ("player", POINTER(zzub_player_t)), ("index", c_int))
zzub_mididriver_is_input = dlsym(libarmstrong, "zzub_mididriver_is_input", c_int, ("player", POINTER(zzub_player_t)), ("index", c_int))
zzub_mididriver_is_output = dlsym(libarmstrong, "zzub_mididriver_is_output", c_int, ("player", POINTER(zzub_player_t)), ("index", c_int))
zzub_mididriver_open = dlsym(libarmstrong, "zzub_mididriver_open", c_int, ("player", POINTER(zzub_player_t)), ("index", c_int))
zzub_mididriver_close_all = dlsym(libarmstrong, "zzub_mididriver_close_all", c_int, ("player", POINTER(zzub_player_t)))
zzub_plugincollection_get_by_uri = dlsym(libarmstrong, "zzub_plugincollection_get_by_uri", POINTER(zzub_plugincollection_t), ("player", POINTER(zzub_player_t)), ("uri", c_char_p))
zzub_plugincollection_get_name = dlsym(libarmstrong, "zzub_plugincollection_get_name", c_char_p, ("Plugincollection", POINTER(zzub_plugincollection_t)))
zzub_plugincollection_configure = dlsym(libarmstrong, "zzub_plugincollection_configure", None, ("Plugincollection", POINTER(zzub_plugincollection_t)), ("key", c_char_p), ("value", c_char_p))
zzub_input_open_file = dlsym(libarmstrong, "zzub_input_open_file", POINTER(zzub_input_t), ("filename", c_char_p))
zzub_input_destroy = dlsym(libarmstrong, "zzub_input_destroy", None, ("Input", POINTER(zzub_input_t)))
zzub_input_read = dlsym(libarmstrong, "zzub_input_read", c_int, ("Input", POINTER(zzub_input_t)), ("buffer", POINTER(c_char)), ("bytes", c_int))
zzub_input_size = dlsym(libarmstrong, "zzub_input_size", c_int, ("Input", POINTER(zzub_input_t)))
zzub_input_position = dlsym(libarmstrong, "zzub_input_position", c_int, ("Input", POINTER(zzub_input_t)))
zzub_input_seek = dlsym(libarmstrong, "zzub_input_seek", None, ("Input", POINTER(zzub_input_t)), ("pos", c_int), ("mode", c_int))
zzub_output_create_file = dlsym(libarmstrong, "zzub_output_create_file", POINTER(zzub_output_t), ("filename", c_char_p))
zzub_output_destroy = dlsym(libarmstrong, "zzub_output_destroy", None, ("Output", POINTER(zzub_output_t)))
zzub_output_write = dlsym(libarmstrong, "zzub_output_write", None, ("Output", POINTER(zzub_output_t)), ("buffer", POINTER(c_char)), ("bytes", c_int))
zzub_output_position = dlsym(libarmstrong, "zzub_output_position", c_int, ("Output", POINTER(zzub_output_t)))
zzub_output_seek = dlsym(libarmstrong, "zzub_output_seek", None, ("Output", POINTER(zzub_output_t)), ("pos", c_int), ("mode", c_int))
zzub_archive_create_memory = dlsym(libarmstrong, "zzub_archive_create_memory", POINTER(zzub_archive_t))
zzub_archive_get_output = dlsym(libarmstrong, "zzub_archive_get_output", POINTER(zzub_output_t), ("Archive", POINTER(zzub_archive_t)), ("path", c_char_p))
zzub_archive_get_input = dlsym(libarmstrong, "zzub_archive_get_input", POINTER(zzub_input_t), ("Archive", POINTER(zzub_archive_t)), ("path", c_char_p))
zzub_archive_destroy = dlsym(libarmstrong, "zzub_archive_destroy", None, ("Archive", POINTER(zzub_archive_t)))
zzub_midimapping_get_plugin = dlsym(libarmstrong, "zzub_midimapping_get_plugin", c_int, ("Midimapping", POINTER(zzub_midimapping_t)))
zzub_midimapping_get_group = dlsym(libarmstrong, "zzub_midimapping_get_group", c_int, ("Midimapping", POINTER(zzub_midimapping_t)))
zzub_midimapping_get_track = dlsym(libarmstrong, "zzub_midimapping_get_track", c_int, ("Midimapping", POINTER(zzub_midimapping_t)))
zzub_midimapping_get_column = dlsym(libarmstrong, "zzub_midimapping_get_column", c_int, ("Midimapping", POINTER(zzub_midimapping_t)))
zzub_midimapping_get_channel = dlsym(libarmstrong, "zzub_midimapping_get_channel", c_int, ("Midimapping", POINTER(zzub_midimapping_t)))
zzub_midimapping_get_controller = dlsym(libarmstrong, "zzub_midimapping_get_controller", c_int, ("Midimapping", POINTER(zzub_midimapping_t)))
zzub_pattern_event_get_id = dlsym(libarmstrong, "zzub_pattern_event_get_id", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)))
zzub_pattern_event_get_pluginid = dlsym(libarmstrong, "zzub_pattern_event_get_pluginid", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)))
zzub_pattern_event_get_pattern = dlsym(libarmstrong, "zzub_pattern_event_get_pattern", POINTER(zzub_pattern_t), ("PatternEvent", POINTER(zzub_pattern_event_t)))
zzub_pattern_event_get_group = dlsym(libarmstrong, "zzub_pattern_event_get_group", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)))
zzub_pattern_event_get_track = dlsym(libarmstrong, "zzub_pattern_event_get_track", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)))
zzub_pattern_event_get_column = dlsym(libarmstrong, "zzub_pattern_event_get_column", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)))
zzub_pattern_event_get_time = dlsym(libarmstrong, "zzub_pattern_event_get_time", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)))
zzub_pattern_event_get_value = dlsym(libarmstrong, "zzub_pattern_event_get_value", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)))
zzub_pattern_event_get_meta = dlsym(libarmstrong, "zzub_pattern_event_get_meta", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)))
zzub_pattern_event_set_value = dlsym(libarmstrong, "zzub_pattern_event_set_value", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)), ("value", c_int))
zzub_pattern_event_set_meta = dlsym(libarmstrong, "zzub_pattern_event_set_meta", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)), ("meta", c_int))
zzub_pattern_event_set_time = dlsym(libarmstrong, "zzub_pattern_event_set_time", c_int, ("PatternEvent", POINTER(zzub_pattern_event_t)), ("value", c_int))
zzub_pattern_iterator_next = dlsym(libarmstrong, "zzub_pattern_iterator_next", None, ("PatternIterator", POINTER(zzub_pattern_iterator_t)))
zzub_pattern_iterator_valid = dlsym(libarmstrong, "zzub_pattern_iterator_valid", c_int, ("PatternIterator", POINTER(zzub_pattern_iterator_t)))
zzub_pattern_iterator_current = dlsym(libarmstrong, "zzub_pattern_iterator_current", POINTER(zzub_pattern_t), ("PatternIterator", POINTER(zzub_pattern_iterator_t)))
zzub_pattern_iterator_destroy = dlsym(libarmstrong, "zzub_pattern_iterator_destroy", None, ("PatternIterator", POINTER(zzub_pattern_iterator_t)))
zzub_pattern_event_iterator_next = dlsym(libarmstrong, "zzub_pattern_event_iterator_next", None, ("PatternEventIterator", POINTER(zzub_pattern_event_iterator_t)))
zzub_pattern_event_iterator_valid = dlsym(libarmstrong, "zzub_pattern_event_iterator_valid", c_int, ("PatternEventIterator", POINTER(zzub_pattern_event_iterator_t)))
zzub_pattern_event_iterator_current = dlsym(libarmstrong, "zzub_pattern_event_iterator_current", POINTER(zzub_pattern_event_t), ("PatternEventIterator", POINTER(zzub_pattern_event_iterator_t)))
zzub_pattern_event_iterator_destroy = dlsym(libarmstrong, "zzub_pattern_event_iterator_destroy", None, ("PatternEventIterator", POINTER(zzub_pattern_event_iterator_t)))
zzub_pattern_destroy = dlsym(libarmstrong, "zzub_pattern_destroy", None, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_get_name = dlsym(libarmstrong, "zzub_pattern_get_name", c_char_p, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_name = dlsym(libarmstrong, "zzub_pattern_set_name", None, ("Pattern", POINTER(zzub_pattern_t)), ("name", c_char_p))
zzub_pattern_get_row_count = dlsym(libarmstrong, "zzub_pattern_get_row_count", c_int, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_row_count = dlsym(libarmstrong, "zzub_pattern_set_row_count", None, ("Pattern", POINTER(zzub_pattern_t)), ("length", c_int))
zzub_pattern_get_id = dlsym(libarmstrong, "zzub_pattern_get_id", c_int, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_get_format = dlsym(libarmstrong, "zzub_pattern_get_format", POINTER(zzub_pattern_format_t), ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_format = dlsym(libarmstrong, "zzub_pattern_set_format", None, ("Pattern", POINTER(zzub_pattern_t)), ("format", POINTER(zzub_pattern_format_t)))
zzub_pattern_get_resolution = dlsym(libarmstrong, "zzub_pattern_get_resolution", c_int, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_resolution = dlsym(libarmstrong, "zzub_pattern_set_resolution", None, ("Pattern", POINTER(zzub_pattern_t)), ("resolution", c_int))
zzub_pattern_get_display_resolution = dlsym(libarmstrong, "zzub_pattern_get_display_resolution", c_int, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_display_resolution = dlsym(libarmstrong, "zzub_pattern_set_display_resolution", None, ("Pattern", POINTER(zzub_pattern_t)), ("resolution", c_int))
zzub_pattern_get_display_beat_rows = dlsym(libarmstrong, "zzub_pattern_get_display_beat_rows", None, ("Pattern", POINTER(zzub_pattern_t)), ("verydarkrow", c_int), ("darkrow", c_int))
zzub_pattern_set_display_beat_rows = dlsym(libarmstrong, "zzub_pattern_set_display_beat_rows", None, ("Pattern", POINTER(zzub_pattern_t)), ("verydarkrow", c_int), ("darkrow", c_int))
zzub_pattern_get_loop_start = dlsym(libarmstrong, "zzub_pattern_get_loop_start", c_int, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_loop_start = dlsym(libarmstrong, "zzub_pattern_set_loop_start", None, ("Pattern", POINTER(zzub_pattern_t)), ("pos", c_int))
zzub_pattern_get_loop_end = dlsym(libarmstrong, "zzub_pattern_get_loop_end", c_int, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_loop_end = dlsym(libarmstrong, "zzub_pattern_set_loop_end", None, ("Pattern", POINTER(zzub_pattern_t)), ("pos", c_int))
zzub_pattern_get_loop_enabled = dlsym(libarmstrong, "zzub_pattern_get_loop_enabled", c_int, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_loop_enabled = dlsym(libarmstrong, "zzub_pattern_set_loop_enabled", None, ("Pattern", POINTER(zzub_pattern_t)), ("enable", c_int))
zzub_pattern_get_replay_row = dlsym(libarmstrong, "zzub_pattern_get_replay_row", c_int, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_replay_row = dlsym(libarmstrong, "zzub_pattern_set_replay_row", None, ("Pattern", POINTER(zzub_pattern_t)), ("row", c_int))
zzub_pattern_get_currently_playing_row = dlsym(libarmstrong, "zzub_pattern_get_currently_playing_row", c_int, ("Pattern", POINTER(zzub_pattern_t)))
zzub_pattern_set_value = dlsym(libarmstrong, "zzub_pattern_set_value", None, ("Pattern", POINTER(zzub_pattern_t)), ("row", c_int), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int), ("value", c_int), ("meta", c_int))
zzub_pattern_get_value = dlsym(libarmstrong, "zzub_pattern_get_value", c_int, ("Pattern", POINTER(zzub_pattern_t)), ("row", c_int), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("value", c_int), ("meta", c_int))
zzub_pattern_get_event_iterator = dlsym(libarmstrong, "zzub_pattern_get_event_iterator", POINTER(zzub_pattern_event_iterator_t), ("Pattern", POINTER(zzub_pattern_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int))
zzub_pattern_get_event_unsorted_iterator = dlsym(libarmstrong, "zzub_pattern_get_event_unsorted_iterator", POINTER(zzub_pattern_event_iterator_t), ("Pattern", POINTER(zzub_pattern_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int))
zzub_pattern_insert_value = dlsym(libarmstrong, "zzub_pattern_insert_value", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("time", c_int), ("value", c_int), ("meta", c_int))
zzub_pattern_delete_value = dlsym(libarmstrong, "zzub_pattern_delete_value", None, ("Pattern", POINTER(zzub_pattern_t)), ("id", c_int))
zzub_pattern_update_value = dlsym(libarmstrong, "zzub_pattern_update_value", None, ("Pattern", POINTER(zzub_pattern_t)), ("id", c_int), ("time", c_int), ("value", c_int), ("meta", c_int))
zzub_pattern_update_value_full = dlsym(libarmstrong, "zzub_pattern_update_value_full", None, ("Pattern", POINTER(zzub_pattern_t)), ("id", c_int), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("time", c_int), ("value", c_int), ("meta", c_int))
zzub_pattern_compact_pattern = dlsym(libarmstrong, "zzub_pattern_compact_pattern", None, ("Pattern", POINTER(zzub_pattern_t)), ("factor", c_int))
zzub_pattern_expand_pattern = dlsym(libarmstrong, "zzub_pattern_expand_pattern", None, ("Pattern", POINTER(zzub_pattern_t)), ("factor", c_int))
zzub_pattern_timeshift_events = dlsym(libarmstrong, "zzub_pattern_timeshift_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("timeshift", c_int))
zzub_pattern_delete_events = dlsym(libarmstrong, "zzub_pattern_delete_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int))
zzub_pattern_move_scale_events = dlsym(libarmstrong, "zzub_pattern_move_scale_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("src_idx", c_int), ("src_time", c_int), ("dst_idx", c_int), ("dst_time", c_int), ("width", c_int), ("length", c_int), ("mode", c_int), ("makecopy", c_int))
zzub_pattern_paste_stream_events = dlsym(libarmstrong, "zzub_pattern_paste_stream_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("fromidx", c_int), ("fromtime", c_int), ("mode", c_int), ("charbuf", c_char_p))
zzub_pattern_transpose_events = dlsym(libarmstrong, "zzub_pattern_transpose_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("delta", c_int), ("holes", POINTER(c_int)), ("holecount", c_int), ("metas", POINTER(c_int)), ("metacount", c_int), ("chromatic", c_int))
zzub_pattern_randomize_events = dlsym(libarmstrong, "zzub_pattern_randomize_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("skip", c_int))
zzub_pattern_randomize_range_events = dlsym(libarmstrong, "zzub_pattern_randomize_range_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("from_val", c_int), ("to_val", c_int), ("skip", c_int))
zzub_pattern_randomize_from_events = dlsym(libarmstrong, "zzub_pattern_randomize_from_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("skip", c_int))
zzub_pattern_humanize_events = dlsym(libarmstrong, "zzub_pattern_humanize_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("deviation", c_int))
zzub_pattern_shuffle_events = dlsym(libarmstrong, "zzub_pattern_shuffle_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int))
zzub_pattern_interpolate_events = dlsym(libarmstrong, "zzub_pattern_interpolate_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("skip", c_int))
zzub_pattern_gradiate_events = dlsym(libarmstrong, "zzub_pattern_gradiate_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("skip", c_int))
zzub_pattern_smooth_events = dlsym(libarmstrong, "zzub_pattern_smooth_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("strength", c_int))
zzub_pattern_reverse_events = dlsym(libarmstrong, "zzub_pattern_reverse_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int))
zzub_pattern_compact_events = dlsym(libarmstrong, "zzub_pattern_compact_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("factor", c_int))
zzub_pattern_expand_events = dlsym(libarmstrong, "zzub_pattern_expand_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("factor", c_int))
zzub_pattern_thin_events = dlsym(libarmstrong, "zzub_pattern_thin_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("major", c_int))
zzub_pattern_repeat_events = dlsym(libarmstrong, "zzub_pattern_repeat_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("major", c_int))
zzub_pattern_echo_events = dlsym(libarmstrong, "zzub_pattern_echo_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("major", c_int))
zzub_pattern_unique_events = dlsym(libarmstrong, "zzub_pattern_unique_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int))
zzub_pattern_scale_events = dlsym(libarmstrong, "zzub_pattern_scale_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("min1", c_double), ("max1", c_double), ("min2", c_double), ("max2", c_double))
zzub_pattern_fade_events = dlsym(libarmstrong, "zzub_pattern_fade_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("fromvalue", c_double), ("tovalue", c_double))
zzub_pattern_curvemap_events = dlsym(libarmstrong, "zzub_pattern_curvemap_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("mode", c_int))
zzub_pattern_invert_events = dlsym(libarmstrong, "zzub_pattern_invert_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int))
zzub_pattern_rotate_rows_events = dlsym(libarmstrong, "zzub_pattern_rotate_rows_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("offset", c_int))
zzub_pattern_rotate_vals_events = dlsym(libarmstrong, "zzub_pattern_rotate_vals_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("offset", c_int))
zzub_pattern_rotate_dist_events = dlsym(libarmstrong, "zzub_pattern_rotate_dist_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("offset", c_int))
zzub_pattern_set_events = dlsym(libarmstrong, "zzub_pattern_set_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("value", c_int), ("meta", c_int))
zzub_pattern_replace_events = dlsym(libarmstrong, "zzub_pattern_replace_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("from_value", c_int), ("from_meta", c_int), ("to_value", c_int), ("to_meta", c_int))
zzub_pattern_remove_events = dlsym(libarmstrong, "zzub_pattern_remove_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("value", c_int), ("meta", c_int))
zzub_pattern_notelength_events = dlsym(libarmstrong, "zzub_pattern_notelength_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("fromtime", c_int), ("length", c_int), ("desired_len", c_int), ("mode", c_int), ("off_value", c_int))
zzub_pattern_volumes_events = dlsym(libarmstrong, "zzub_pattern_volumes_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("note_column", c_int), ("vol_column", c_int), ("fromtime", c_int), ("length", c_int), ("mode", c_int))
zzub_pattern_swap_track_events = dlsym(libarmstrong, "zzub_pattern_swap_track_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("left_idx", c_int), ("right_idx", c_int), ("fromtime", c_int), ("length", c_int))
zzub_pattern_swap_rows_events = dlsym(libarmstrong, "zzub_pattern_swap_rows_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("pluginid", c_int), ("group", c_int), ("track", c_int), ("column", c_int), ("top_row", c_int), ("bottom_row", c_int))
zzub_pattern_invert_chord_events = dlsym(libarmstrong, "zzub_pattern_invert_chord_events", None, ("Pattern", POINTER(zzub_pattern_t)), ("left_idx", c_int), ("right_idx", c_int), ("fromtime", c_int), ("length", c_int), ("direction", c_int), ("mode", c_int))
zzub_pattern_move_and_transpose_notes = dlsym(libarmstrong, "zzub_pattern_move_and_transpose_notes", None, ("Pattern", POINTER(zzub_pattern_t)), ("events", POINTER(POINTER(zzub_pattern_event_t))), ("numevents", c_int), ("timeshift", c_int), ("pitchshift", c_int), ("mode", c_int))
zzub_pattern_insert_note = dlsym(libarmstrong, "zzub_pattern_insert_note", None, ("Pattern", POINTER(zzub_pattern_t)), ("plugin", POINTER(zzub_plugin_t)), ("time", c_int), ("note", c_int), ("length", c_int))
zzub_pattern_update_note = dlsym(libarmstrong, "zzub_pattern_update_note", None, ("Pattern", POINTER(zzub_pattern_t)), ("patternevent", POINTER(zzub_pattern_event_t)), ("time", c_int), ("note", c_int), ("length", c_int))
zzub_pattern_format_add_column = dlsym(libarmstrong, "zzub_pattern_format_add_column", POINTER(zzub_pattern_format_column_t), ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int), ("idx", c_int))
zzub_pattern_format_delete_column = dlsym(libarmstrong, "zzub_pattern_format_delete_column", None, ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int))
zzub_pattern_format_get_iterator = dlsym(libarmstrong, "zzub_pattern_format_get_iterator", POINTER(zzub_pattern_format_column_iterator_t), ("PatternFormat", POINTER(zzub_pattern_format_t)))
zzub_pattern_format_get_column = dlsym(libarmstrong, "zzub_pattern_format_get_column", POINTER(zzub_pattern_format_column_t), ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int))
zzub_pattern_format_get_name = dlsym(libarmstrong, "zzub_pattern_format_get_name", c_char_p, ("PatternFormat", POINTER(zzub_pattern_format_t)))
zzub_pattern_format_set_name = dlsym(libarmstrong, "zzub_pattern_format_set_name", None, ("PatternFormat", POINTER(zzub_pattern_format_t)), ("name", c_char_p))
zzub_pattern_format_get_id = dlsym(libarmstrong, "zzub_pattern_format_get_id", c_int, ("PatternFormat", POINTER(zzub_pattern_format_t)))
zzub_pattern_format_set_track_name = dlsym(libarmstrong, "zzub_pattern_format_set_track_name", None, ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("name", c_char_p))
zzub_pattern_format_get_track_name = dlsym(libarmstrong, "zzub_pattern_format_get_track_name", c_char_p, ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int))
zzub_pattern_format_set_track_mute = dlsym(libarmstrong, "zzub_pattern_format_set_track_mute", None, ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("state", c_int))
zzub_pattern_format_get_track_mute = dlsym(libarmstrong, "zzub_pattern_format_get_track_mute", c_int, ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int))
zzub_pattern_format_add_column_filter = dlsym(libarmstrong, "zzub_pattern_format_add_column_filter", None, ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int), ("filterformat", POINTER(zzub_pattern_format_t)))
zzub_pattern_format_remove_column_filter = dlsym(libarmstrong, "zzub_pattern_format_remove_column_filter", None, ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int), ("filterformat", POINTER(zzub_pattern_format_t)))
zzub_pattern_format_get_column_filters = dlsym(libarmstrong, "zzub_pattern_format_get_column_filters", POINTER(zzub_pattern_format_iterator_t), ("PatternFormat", POINTER(zzub_pattern_format_t)), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int))
zzub_pattern_format_get_scroller_width = dlsym(libarmstrong, "zzub_pattern_format_get_scroller_width", c_int, ("PatternFormat", POINTER(zzub_pattern_format_t)))
zzub_pattern_format_set_scroller_width = dlsym(libarmstrong, "zzub_pattern_format_set_scroller_width", None, ("PatternFormat", POINTER(zzub_pattern_format_t)), ("width", c_int))
zzub_pattern_format_destroy = dlsym(libarmstrong, "zzub_pattern_format_destroy", None, ("PatternFormat", POINTER(zzub_pattern_format_t)))
zzub_pattern_format_iterator_next = dlsym(libarmstrong, "zzub_pattern_format_iterator_next", None, ("PatternFormatIterator", POINTER(zzub_pattern_format_iterator_t)))
zzub_pattern_format_iterator_valid = dlsym(libarmstrong, "zzub_pattern_format_iterator_valid", c_int, ("PatternFormatIterator", POINTER(zzub_pattern_format_iterator_t)))
zzub_pattern_format_iterator_current = dlsym(libarmstrong, "zzub_pattern_format_iterator_current", POINTER(zzub_pattern_format_t), ("PatternFormatIterator", POINTER(zzub_pattern_format_iterator_t)))
zzub_pattern_format_iterator_destroy = dlsym(libarmstrong, "zzub_pattern_format_iterator_destroy", None, ("PatternFormatIterator", POINTER(zzub_pattern_format_iterator_t)))
zzub_pattern_format_column_get_plugin = dlsym(libarmstrong, "zzub_pattern_format_column_get_plugin", POINTER(zzub_plugin_t), ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)))
zzub_pattern_format_column_get_group = dlsym(libarmstrong, "zzub_pattern_format_column_get_group", c_int, ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)))
zzub_pattern_format_column_get_track = dlsym(libarmstrong, "zzub_pattern_format_column_get_track", c_int, ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)))
zzub_pattern_format_column_get_column = dlsym(libarmstrong, "zzub_pattern_format_column_get_column", c_int, ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)))
zzub_pattern_format_column_get_format = dlsym(libarmstrong, "zzub_pattern_format_column_get_format", POINTER(zzub_pattern_format_t), ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)))
zzub_pattern_format_column_get_mode = dlsym(libarmstrong, "zzub_pattern_format_column_get_mode", c_int, ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)))
zzub_pattern_format_column_set_mode = dlsym(libarmstrong, "zzub_pattern_format_column_set_mode", None, ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)), ("mode", c_int))
zzub_pattern_format_column_get_collapsed = dlsym(libarmstrong, "zzub_pattern_format_column_get_collapsed", c_int, ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)))
zzub_pattern_format_column_set_collapsed = dlsym(libarmstrong, "zzub_pattern_format_column_set_collapsed", None, ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)), ("is_collapsed", c_int))
zzub_pattern_format_column_get_index = dlsym(libarmstrong, "zzub_pattern_format_column_get_index", c_int, ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)))
zzub_pattern_format_column_set_index = dlsym(libarmstrong, "zzub_pattern_format_column_set_index", None, ("PatternFormatColumn", POINTER(zzub_pattern_format_column_t)), ("idx", c_int))
zzub_pattern_format_column_iterator_next = dlsym(libarmstrong, "zzub_pattern_format_column_iterator_next", None, ("PatternFormatColumnIterator", POINTER(zzub_pattern_format_column_iterator_t)))
zzub_pattern_format_column_iterator_valid = dlsym(libarmstrong, "zzub_pattern_format_column_iterator_valid", c_int, ("PatternFormatColumnIterator", POINTER(zzub_pattern_format_column_iterator_t)))
zzub_pattern_format_column_iterator_current = dlsym(libarmstrong, "zzub_pattern_format_column_iterator_current", POINTER(zzub_pattern_format_column_t), ("PatternFormatColumnIterator", POINTER(zzub_pattern_format_column_iterator_t)))
zzub_pattern_format_column_iterator_destroy = dlsym(libarmstrong, "zzub_pattern_format_column_iterator_destroy", None, ("PatternFormatColumnIterator", POINTER(zzub_pattern_format_column_iterator_t)))
zzub_parameter_get_type = dlsym(libarmstrong, "zzub_parameter_get_type", c_int, ("Parameter", POINTER(zzub_parameter_t)))
zzub_parameter_get_name = dlsym(libarmstrong, "zzub_parameter_get_name", c_char_p, ("Parameter", POINTER(zzub_parameter_t)))
zzub_parameter_get_description = dlsym(libarmstrong, "zzub_parameter_get_description", c_char_p, ("Parameter", POINTER(zzub_parameter_t)))
zzub_parameter_get_value_min = dlsym(libarmstrong, "zzub_parameter_get_value_min", c_int, ("Parameter", POINTER(zzub_parameter_t)))
zzub_parameter_get_value_max = dlsym(libarmstrong, "zzub_parameter_get_value_max", c_int, ("Parameter", POINTER(zzub_parameter_t)))
zzub_parameter_get_value_none = dlsym(libarmstrong, "zzub_parameter_get_value_none", c_int, ("Parameter", POINTER(zzub_parameter_t)))
zzub_parameter_get_value_default = dlsym(libarmstrong, "zzub_parameter_get_value_default", c_int, ("Parameter", POINTER(zzub_parameter_t)))
zzub_parameter_get_flags = dlsym(libarmstrong, "zzub_parameter_get_flags", c_int, ("Parameter", POINTER(zzub_parameter_t)))
zzub_attribute_get_name = dlsym(libarmstrong, "zzub_attribute_get_name", c_char_p, ("Attribute", POINTER(zzub_attribute_t)))
zzub_attribute_get_value_min = dlsym(libarmstrong, "zzub_attribute_get_value_min", c_int, ("Attribute", POINTER(zzub_attribute_t)))
zzub_attribute_get_value_max = dlsym(libarmstrong, "zzub_attribute_get_value_max", c_int, ("Attribute", POINTER(zzub_attribute_t)))
zzub_attribute_get_value_default = dlsym(libarmstrong, "zzub_attribute_get_value_default", c_int, ("Attribute", POINTER(zzub_attribute_t)))
zzub_pluginloader_get_name = dlsym(libarmstrong, "zzub_pluginloader_get_name", c_char_p, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_short_name = dlsym(libarmstrong, "zzub_pluginloader_get_short_name", c_char_p, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_parameter_count = dlsym(libarmstrong, "zzub_pluginloader_get_parameter_count", c_int, ("Pluginloader", POINTER(zzub_pluginloader_t)), ("group", c_int))
zzub_pluginloader_get_parameter = dlsym(libarmstrong, "zzub_pluginloader_get_parameter", POINTER(zzub_parameter_t), ("Pluginloader", POINTER(zzub_pluginloader_t)), ("group", c_int), ("index", c_int))
zzub_pluginloader_get_attribute_count = dlsym(libarmstrong, "zzub_pluginloader_get_attribute_count", c_int, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_attribute = dlsym(libarmstrong, "zzub_pluginloader_get_attribute", POINTER(zzub_attribute_t), ("Pluginloader", POINTER(zzub_pluginloader_t)), ("index", c_int))
zzub_pluginloader_get_flags = dlsym(libarmstrong, "zzub_pluginloader_get_flags", c_int, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_uri = dlsym(libarmstrong, "zzub_pluginloader_get_uri", c_char_p, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_author = dlsym(libarmstrong, "zzub_pluginloader_get_author", c_char_p, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_instrument_list = dlsym(libarmstrong, "zzub_pluginloader_get_instrument_list", c_int, ("Pluginloader", POINTER(zzub_pluginloader_t)), ("result", POINTER(c_char)), ("maxbytes", c_int))
zzub_pluginloader_get_tracks_min = dlsym(libarmstrong, "zzub_pluginloader_get_tracks_min", c_int, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_tracks_max = dlsym(libarmstrong, "zzub_pluginloader_get_tracks_max", c_int, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_stream_format_count = dlsym(libarmstrong, "zzub_pluginloader_get_stream_format_count", c_int, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_stream_format_ext = dlsym(libarmstrong, "zzub_pluginloader_get_stream_format_ext", c_char_p, ("Pluginloader", POINTER(zzub_pluginloader_t)), ("index", c_int))
zzub_pluginloader_get_output_channel_count = dlsym(libarmstrong, "zzub_pluginloader_get_output_channel_count", c_int, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_input_channel_count = dlsym(libarmstrong, "zzub_pluginloader_get_input_channel_count", c_int, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_plugin_file = dlsym(libarmstrong, "zzub_pluginloader_get_plugin_file", c_char_p, ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_pluginloader_get_plugincollection = dlsym(libarmstrong, "zzub_pluginloader_get_plugincollection", POINTER(zzub_plugincollection_t), ("Pluginloader", POINTER(zzub_pluginloader_t)))
zzub_plugin_destroy = dlsym(libarmstrong, "zzub_plugin_destroy", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_load = dlsym(libarmstrong, "zzub_plugin_load", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("input", POINTER(zzub_input_t)))
zzub_plugin_save = dlsym(libarmstrong, "zzub_plugin_save", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("ouput", POINTER(zzub_output_t)))
zzub_plugin_set_name = dlsym(libarmstrong, "zzub_plugin_set_name", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("name", c_char_p))
zzub_plugin_get_name = dlsym(libarmstrong, "zzub_plugin_get_name", c_char_p, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_id = dlsym(libarmstrong, "zzub_plugin_get_id", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_position_x = dlsym(libarmstrong, "zzub_plugin_get_position_x", c_float, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_position_y = dlsym(libarmstrong, "zzub_plugin_get_position_y", c_float, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_set_position = dlsym(libarmstrong, "zzub_plugin_set_position", None, ("Plugin", POINTER(zzub_plugin_t)), ("x", c_float), ("y", c_float))
zzub_plugin_set_position_direct = dlsym(libarmstrong, "zzub_plugin_set_position_direct", None, ("Plugin", POINTER(zzub_plugin_t)), ("x", c_float), ("y", c_float))
zzub_plugin_get_flags = dlsym(libarmstrong, "zzub_plugin_get_flags", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_track_count = dlsym(libarmstrong, "zzub_plugin_get_track_count", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("group", c_int))
zzub_plugin_set_track_count = dlsym(libarmstrong, "zzub_plugin_set_track_count", None, ("Plugin", POINTER(zzub_plugin_t)), ("count", c_int))
zzub_plugin_get_mute = dlsym(libarmstrong, "zzub_plugin_get_mute", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_set_mute = dlsym(libarmstrong, "zzub_plugin_set_mute", None, ("Plugin", POINTER(zzub_plugin_t)), ("muted", c_int))
zzub_plugin_get_bypass = dlsym(libarmstrong, "zzub_plugin_get_bypass", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_set_bypass = dlsym(libarmstrong, "zzub_plugin_set_bypass", None, ("Plugin", POINTER(zzub_plugin_t)), ("muted", c_int))
zzub_plugin_get_minimize = dlsym(libarmstrong, "zzub_plugin_get_minimize", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_set_minimize = dlsym(libarmstrong, "zzub_plugin_set_minimize", None, ("Plugin", POINTER(zzub_plugin_t)), ("minimized", c_int))
zzub_plugin_configure = dlsym(libarmstrong, "zzub_plugin_configure", None, ("Plugin", POINTER(zzub_plugin_t)), ("key", c_char_p), ("value", c_char_p))
zzub_plugin_get_commands = dlsym(libarmstrong, "zzub_plugin_get_commands", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("commands", c_char_p), ("maxlen", c_int))
zzub_plugin_get_sub_commands = dlsym(libarmstrong, "zzub_plugin_get_sub_commands", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("i", c_int), ("commands", c_char_p), ("maxlen", c_int))
zzub_plugin_command = dlsym(libarmstrong, "zzub_plugin_command", None, ("Plugin", POINTER(zzub_plugin_t)), ("i", c_int))
zzub_plugin_get_pluginloader = dlsym(libarmstrong, "zzub_plugin_get_pluginloader", POINTER(zzub_pluginloader_t), ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_midi_output_device_count = dlsym(libarmstrong, "zzub_plugin_get_midi_output_device_count", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_midi_output_device = dlsym(libarmstrong, "zzub_plugin_get_midi_output_device", c_char_p, ("Plugin", POINTER(zzub_plugin_t)), ("index", c_int))
zzub_plugin_get_envelope_count = dlsym(libarmstrong, "zzub_plugin_get_envelope_count", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_envelope_flags = dlsym(libarmstrong, "zzub_plugin_get_envelope_flags", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("index", c_int))
zzub_plugin_get_envelope_name = dlsym(libarmstrong, "zzub_plugin_get_envelope_name", c_char_p, ("Plugin", POINTER(zzub_plugin_t)), ("index", c_int))
zzub_plugin_set_stream_source = dlsym(libarmstrong, "zzub_plugin_set_stream_source", None, ("Plugin", POINTER(zzub_plugin_t)), ("resource", c_char_p))
zzub_plugin_get_stream_source = dlsym(libarmstrong, "zzub_plugin_get_stream_source", c_char_p, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_set_instrument = dlsym(libarmstrong, "zzub_plugin_set_instrument", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("name", c_char_p))
zzub_plugin_describe_value = dlsym(libarmstrong, "zzub_plugin_describe_value", c_char_p, ("Plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("column", c_int), ("value", c_int))
zzub_plugin_get_parameter_value = dlsym(libarmstrong, "zzub_plugin_get_parameter_value", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int))
zzub_plugin_set_parameter_value = dlsym(libarmstrong, "zzub_plugin_set_parameter_value", None, ("Plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int), ("value", c_int), ("record", c_int))
zzub_plugin_set_parameter_value_direct = dlsym(libarmstrong, "zzub_plugin_set_parameter_value_direct", None, ("Plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int), ("value", c_int), ("record", c_int))
zzub_plugin_get_parameter_count = dlsym(libarmstrong, "zzub_plugin_get_parameter_count", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int))
zzub_plugin_get_parameter = dlsym(libarmstrong, "zzub_plugin_get_parameter", POINTER(zzub_parameter_t), ("Plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int))
zzub_plugin_get_parameter_interpolator = dlsym(libarmstrong, "zzub_plugin_get_parameter_interpolator", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int))
zzub_plugin_set_parameter_interpolator = dlsym(libarmstrong, "zzub_plugin_set_parameter_interpolator", None, ("Plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("column", c_int), ("mode", c_int))
zzub_plugin_get_input_connection_count = dlsym(libarmstrong, "zzub_plugin_get_input_connection_count", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_input_connection = dlsym(libarmstrong, "zzub_plugin_get_input_connection", POINTER(zzub_connection_t), ("Plugin", POINTER(zzub_plugin_t)), ("index", c_int))
zzub_plugin_get_input_connection_by_type = dlsym(libarmstrong, "zzub_plugin_get_input_connection_by_type", POINTER(zzub_connection_t), ("Plugin", POINTER(zzub_plugin_t)), ("from_plugin", POINTER(zzub_plugin_t)), ("type", c_int))
zzub_plugin_get_output_connection_count = dlsym(libarmstrong, "zzub_plugin_get_output_connection_count", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_output_connection = dlsym(libarmstrong, "zzub_plugin_get_output_connection", POINTER(zzub_connection_t), ("Plugin", POINTER(zzub_plugin_t)), ("index", c_int))
zzub_plugin_get_output_connection_by_type = dlsym(libarmstrong, "zzub_plugin_get_output_connection_by_type", POINTER(zzub_connection_t), ("Plugin", POINTER(zzub_plugin_t)), ("from_plugin", POINTER(zzub_plugin_t)), ("type", c_int))
zzub_plugin_create_audio_connection = dlsym(libarmstrong, "zzub_plugin_create_audio_connection", POINTER(zzub_connection_t), ("Plugin", POINTER(zzub_plugin_t)), ("from_plugin", POINTER(zzub_plugin_t)), ("first_input", c_int), ("input_count", c_int), ("first_output", c_int), ("output_count", c_int))
zzub_plugin_create_midi_connection = dlsym(libarmstrong, "zzub_plugin_create_midi_connection", POINTER(zzub_connection_t), ("Plugin", POINTER(zzub_plugin_t)), ("from_plugin", POINTER(zzub_plugin_t)), ("midi_device", c_char_p))
zzub_plugin_create_event_connection = dlsym(libarmstrong, "zzub_plugin_create_event_connection", POINTER(zzub_connection_t), ("Plugin", POINTER(zzub_plugin_t)), ("from_plugin", POINTER(zzub_plugin_t)))
zzub_plugin_create_note_connection = dlsym(libarmstrong, "zzub_plugin_create_note_connection", POINTER(zzub_connection_t), ("Plugin", POINTER(zzub_plugin_t)), ("from_plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_last_peak = dlsym(libarmstrong, "zzub_plugin_get_last_peak", c_float, ("Plugin", POINTER(zzub_plugin_t)), ("channel", c_int))
zzub_plugin_get_last_cpu_load = dlsym(libarmstrong, "zzub_plugin_get_last_cpu_load", c_double, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_last_midi_result = dlsym(libarmstrong, "zzub_plugin_get_last_midi_result", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_last_audio_result = dlsym(libarmstrong, "zzub_plugin_get_last_audio_result", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_tick = dlsym(libarmstrong, "zzub_plugin_tick", None, ("Plugin", POINTER(zzub_plugin_t)), ("immediate", c_int))
zzub_plugin_get_attribute_value = dlsym(libarmstrong, "zzub_plugin_get_attribute_value", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("index", c_int))
zzub_plugin_set_attribute_value = dlsym(libarmstrong, "zzub_plugin_set_attribute_value", None, ("Plugin", POINTER(zzub_plugin_t)), ("index", c_int), ("value", c_int))
zzub_plugin_play_midi_note = dlsym(libarmstrong, "zzub_plugin_play_midi_note", None, ("Plugin", POINTER(zzub_plugin_t)), ("note", c_int), ("prevNote", c_int), ("velocity", c_int))
zzub_plugin_set_timesource = dlsym(libarmstrong, "zzub_plugin_set_timesource", None, ("Plugin", POINTER(zzub_plugin_t)), ("timesource", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int))
zzub_plugin_get_timesource_plugin = dlsym(libarmstrong, "zzub_plugin_get_timesource_plugin", POINTER(zzub_plugin_t), ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_timesource_group = dlsym(libarmstrong, "zzub_plugin_get_timesource_group", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_timesource_track = dlsym(libarmstrong, "zzub_plugin_get_timesource_track", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_output_channel_count = dlsym(libarmstrong, "zzub_plugin_get_output_channel_count", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_input_channel_count = dlsym(libarmstrong, "zzub_plugin_get_input_channel_count", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_output_channel_name = dlsym(libarmstrong, "zzub_plugin_get_output_channel_name", c_char_p, ("Plugin", POINTER(zzub_plugin_t)), ("index", c_int))
zzub_plugin_get_input_channel_name = dlsym(libarmstrong, "zzub_plugin_get_input_channel_name", c_char_p, ("Plugin", POINTER(zzub_plugin_t)), ("index", c_int))
zzub_plugin_get_encoder_digest = dlsym(libarmstrong, "zzub_plugin_get_encoder_digest", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("type", c_int), ("buffers", POINTER(POINTER(c_float))), ("numsamples", c_int))
zzub_plugin_get_connection = dlsym(libarmstrong, "zzub_plugin_get_connection", POINTER(zzub_connection_t), ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_has_embedded_gui = dlsym(libarmstrong, "zzub_plugin_has_embedded_gui", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_create_embedded_gui = dlsym(libarmstrong, "zzub_plugin_create_embedded_gui", c_int, ("Plugin", POINTER(zzub_plugin_t)), ("hwnd", c_void_p))
zzub_plugin_resize_embedded_gui = dlsym(libarmstrong, "zzub_plugin_resize_embedded_gui", None, ("Plugin", POINTER(zzub_plugin_t)), ("hwnd", c_void_p), ("width", c_int), ("height", c_int))
zzub_plugin_set_latency = dlsym(libarmstrong, "zzub_plugin_set_latency", None, ("Plugin", POINTER(zzub_plugin_t)), ("samplecount", c_int))
zzub_plugin_get_latency = dlsym(libarmstrong, "zzub_plugin_get_latency", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_latency_actual = dlsym(libarmstrong, "zzub_plugin_get_latency_actual", c_int, ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_get_plugin_group = dlsym(libarmstrong, "zzub_plugin_get_plugin_group", POINTER(zzub_plugin_group_t), ("Plugin", POINTER(zzub_plugin_t)))
zzub_plugin_set_plugin_group = dlsym(libarmstrong, "zzub_plugin_set_plugin_group", None, ("Plugin", POINTER(zzub_plugin_t)), ("group", POINTER(zzub_plugin_group_t)))
zzub_plugin_iterator_next = dlsym(libarmstrong, "zzub_plugin_iterator_next", None, ("PluginIterator", POINTER(zzub_plugin_iterator_t)))
zzub_plugin_iterator_valid = dlsym(libarmstrong, "zzub_plugin_iterator_valid", c_int, ("PluginIterator", POINTER(zzub_plugin_iterator_t)))
zzub_plugin_iterator_current = dlsym(libarmstrong, "zzub_plugin_iterator_current", POINTER(zzub_plugin_t), ("PluginIterator", POINTER(zzub_plugin_iterator_t)))
zzub_plugin_iterator_destroy = dlsym(libarmstrong, "zzub_plugin_iterator_destroy", None, ("PluginIterator", POINTER(zzub_plugin_iterator_t)))
zzub_connection_destroy = dlsym(libarmstrong, "zzub_connection_destroy", None, ("Connection", POINTER(zzub_connection_t)))
zzub_connection_get_type = dlsym(libarmstrong, "zzub_connection_get_type", c_int, ("Connection", POINTER(zzub_connection_t)))
zzub_connection_get_from_plugin = dlsym(libarmstrong, "zzub_connection_get_from_plugin", POINTER(zzub_plugin_t), ("Connection", POINTER(zzub_connection_t)))
zzub_connection_get_to_plugin = dlsym(libarmstrong, "zzub_connection_get_to_plugin", POINTER(zzub_plugin_t), ("Connection", POINTER(zzub_connection_t)))
zzub_connection_get_connection_plugin = dlsym(libarmstrong, "zzub_connection_get_connection_plugin", POINTER(zzub_plugin_t), ("Connection", POINTER(zzub_connection_t)))
zzub_connection_get_first_input = dlsym(libarmstrong, "zzub_connection_get_first_input", c_int, ("Connection", POINTER(zzub_connection_t)))
zzub_connection_set_first_input = dlsym(libarmstrong, "zzub_connection_set_first_input", None, ("Connection", POINTER(zzub_connection_t)), ("value", c_int))
zzub_connection_get_input_count = dlsym(libarmstrong, "zzub_connection_get_input_count", c_int, ("Connection", POINTER(zzub_connection_t)))
zzub_connection_set_input_count = dlsym(libarmstrong, "zzub_connection_set_input_count", None, ("Connection", POINTER(zzub_connection_t)), ("value", c_int))
zzub_connection_get_first_output = dlsym(libarmstrong, "zzub_connection_get_first_output", c_int, ("Connection", POINTER(zzub_connection_t)))
zzub_connection_set_first_output = dlsym(libarmstrong, "zzub_connection_set_first_output", None, ("Connection", POINTER(zzub_connection_t)), ("value", c_int))
zzub_connection_get_output_count = dlsym(libarmstrong, "zzub_connection_get_output_count", c_int, ("Connection", POINTER(zzub_connection_t)))
zzub_connection_set_output_count = dlsym(libarmstrong, "zzub_connection_set_output_count", None, ("Connection", POINTER(zzub_connection_t)), ("value", c_int))
zzub_connection_set_midi_device = dlsym(libarmstrong, "zzub_connection_set_midi_device", None, ("Connection", POINTER(zzub_connection_t)), ("midi_device", c_char_p))
zzub_connection_get_midi_device = dlsym(libarmstrong, "zzub_connection_get_midi_device", c_char_p, ("Connection", POINTER(zzub_connection_t)))
zzub_connection_get_event_binding_count = dlsym(libarmstrong, "zzub_connection_get_event_binding_count", c_int, ("Connection", POINTER(zzub_connection_t)))
zzub_connection_add_event_connection_binding = dlsym(libarmstrong, "zzub_connection_add_event_connection_binding", None, ("Connection", POINTER(zzub_connection_t)), ("sourceparam", c_int), ("targetgroup", c_int), ("targettrack", c_int), ("targetparam", c_int))
zzub_connection_remove_event_connection_binding = dlsym(libarmstrong, "zzub_connection_remove_event_connection_binding", None, ("Connection", POINTER(zzub_connection_t)), ("sourceparam", c_int), ("targetgroup", c_int), ("targettrack", c_int), ("targetparam", c_int))
zzub_connection_get_event_binding_iterator = dlsym(libarmstrong, "zzub_connection_get_event_binding_iterator", POINTER(zzub_connection_binding_iterator_t), ("Connection", POINTER(zzub_connection_t)))
zzub_connection_binding_get_connection = dlsym(libarmstrong, "zzub_connection_binding_get_connection", POINTER(zzub_connection_t), ("ConnectionBinding", POINTER(zzub_connection_binding_t)))
zzub_connection_binding_get_source_column = dlsym(libarmstrong, "zzub_connection_binding_get_source_column", c_int, ("ConnectionBinding", POINTER(zzub_connection_binding_t)))
zzub_connection_binding_get_target_group = dlsym(libarmstrong, "zzub_connection_binding_get_target_group", c_int, ("ConnectionBinding", POINTER(zzub_connection_binding_t)))
zzub_connection_binding_get_target_track = dlsym(libarmstrong, "zzub_connection_binding_get_target_track", c_int, ("ConnectionBinding", POINTER(zzub_connection_binding_t)))
zzub_connection_binding_get_target_column = dlsym(libarmstrong, "zzub_connection_binding_get_target_column", c_int, ("ConnectionBinding", POINTER(zzub_connection_binding_t)))
zzub_connection_binding_iterator_next = dlsym(libarmstrong, "zzub_connection_binding_iterator_next", None, ("ConnectionBindingIterator", POINTER(zzub_connection_binding_iterator_t)))
zzub_connection_binding_iterator_valid = dlsym(libarmstrong, "zzub_connection_binding_iterator_valid", c_int, ("ConnectionBindingIterator", POINTER(zzub_connection_binding_iterator_t)))
zzub_connection_binding_iterator_current = dlsym(libarmstrong, "zzub_connection_binding_iterator_current", POINTER(zzub_connection_binding_t), ("ConnectionBindingIterator", POINTER(zzub_connection_binding_iterator_t)))
zzub_connection_binding_iterator_reset = dlsym(libarmstrong, "zzub_connection_binding_iterator_reset", None, ("ConnectionBindingIterator", POINTER(zzub_connection_binding_iterator_t)))
zzub_connection_binding_iterator_destroy = dlsym(libarmstrong, "zzub_connection_binding_iterator_destroy", None, ("ConnectionBindingIterator", POINTER(zzub_connection_binding_iterator_t)))
zzub_wave_get_id = dlsym(libarmstrong, "zzub_wave_get_id", c_int, ("Wave", POINTER(zzub_wave_t)))
zzub_wave_get_index = dlsym(libarmstrong, "zzub_wave_get_index", c_int, ("Wave", POINTER(zzub_wave_t)))
zzub_wave_clear = dlsym(libarmstrong, "zzub_wave_clear", c_int, ("Wave", POINTER(zzub_wave_t)))
zzub_wave_get_name = dlsym(libarmstrong, "zzub_wave_get_name", c_char_p, ("Wave", POINTER(zzub_wave_t)))
zzub_wave_set_name = dlsym(libarmstrong, "zzub_wave_set_name", None, ("Wave", POINTER(zzub_wave_t)), ("name", c_char_p))
zzub_wave_get_path = dlsym(libarmstrong, "zzub_wave_get_path", c_char_p, ("Wave", POINTER(zzub_wave_t)))
zzub_wave_set_path = dlsym(libarmstrong, "zzub_wave_set_path", None, ("Wave", POINTER(zzub_wave_t)), ("path", c_char_p))
zzub_wave_get_flags = dlsym(libarmstrong, "zzub_wave_get_flags", c_int, ("Wave", POINTER(zzub_wave_t)))
zzub_wave_set_flags = dlsym(libarmstrong, "zzub_wave_set_flags", None, ("Wave", POINTER(zzub_wave_t)), ("flags", c_int))
zzub_wave_get_volume = dlsym(libarmstrong, "zzub_wave_get_volume", c_float, ("Wave", POINTER(zzub_wave_t)))
zzub_wave_set_volume = dlsym(libarmstrong, "zzub_wave_set_volume", None, ("Wave", POINTER(zzub_wave_t)), ("volume", c_float))
zzub_wave_get_envelope_count = dlsym(libarmstrong, "zzub_wave_get_envelope_count", c_int, ("Wave", POINTER(zzub_wave_t)))
zzub_wave_set_envelope_count = dlsym(libarmstrong, "zzub_wave_set_envelope_count", None, ("Wave", POINTER(zzub_wave_t)), ("count", c_int))
zzub_wave_get_envelope = dlsym(libarmstrong, "zzub_wave_get_envelope", POINTER(zzub_envelope_t), ("Wave", POINTER(zzub_wave_t)), ("index", c_int))
zzub_wave_set_envelope = dlsym(libarmstrong, "zzub_wave_set_envelope", None, ("Wave", POINTER(zzub_wave_t)), ("index", c_int), ("env", POINTER(zzub_envelope_t)))
zzub_wave_get_level_count = dlsym(libarmstrong, "zzub_wave_get_level_count", c_int, ("Wave", POINTER(zzub_wave_t)))
zzub_wave_get_level = dlsym(libarmstrong, "zzub_wave_get_level", POINTER(zzub_wavelevel_t), ("Wave", POINTER(zzub_wave_t)), ("index", c_int))
zzub_wave_add_level = dlsym(libarmstrong, "zzub_wave_add_level", POINTER(zzub_wavelevel_t), ("Wave", POINTER(zzub_wave_t)))
zzub_wave_remove_level = dlsym(libarmstrong, "zzub_wave_remove_level", None, ("Wave", POINTER(zzub_wave_t)), ("level", c_int))
zzub_wavelevel_get_id = dlsym(libarmstrong, "zzub_wavelevel_get_id", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)))
zzub_wavelevel_get_wave = dlsym(libarmstrong, "zzub_wavelevel_get_wave", POINTER(zzub_wave_t), ("Wavelevel", POINTER(zzub_wavelevel_t)))
zzub_wavelevel_clear = dlsym(libarmstrong, "zzub_wavelevel_clear", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)))
zzub_wavelevel_get_sample_count = dlsym(libarmstrong, "zzub_wavelevel_get_sample_count", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)))
zzub_wavelevel_set_sample_count = dlsym(libarmstrong, "zzub_wavelevel_set_sample_count", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("count", c_int))
zzub_wavelevel_get_root_note = dlsym(libarmstrong, "zzub_wavelevel_get_root_note", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)))
zzub_wavelevel_set_root_note = dlsym(libarmstrong, "zzub_wavelevel_set_root_note", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("note", c_int))
zzub_wavelevel_get_samples_per_second = dlsym(libarmstrong, "zzub_wavelevel_get_samples_per_second", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)))
zzub_wavelevel_set_samples_per_second = dlsym(libarmstrong, "zzub_wavelevel_set_samples_per_second", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("sps", c_int))
zzub_wavelevel_get_loop_start = dlsym(libarmstrong, "zzub_wavelevel_get_loop_start", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)))
zzub_wavelevel_set_loop_start = dlsym(libarmstrong, "zzub_wavelevel_set_loop_start", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("pos", c_int))
zzub_wavelevel_get_loop_end = dlsym(libarmstrong, "zzub_wavelevel_get_loop_end", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)))
zzub_wavelevel_set_loop_end = dlsym(libarmstrong, "zzub_wavelevel_set_loop_end", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("pos", c_int))
zzub_wavelevel_get_format = dlsym(libarmstrong, "zzub_wavelevel_get_format", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)))
zzub_wavelevel_set_format = dlsym(libarmstrong, "zzub_wavelevel_set_format", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("format", c_int))
zzub_wavelevel_load_wav = dlsym(libarmstrong, "zzub_wavelevel_load_wav", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("offset", c_int), ("clear", c_int), ("datastream", POINTER(zzub_input_t)))
zzub_wavelevel_save_wav = dlsym(libarmstrong, "zzub_wavelevel_save_wav", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("datastream", POINTER(zzub_output_t)))
zzub_wavelevel_save_wav_range = dlsym(libarmstrong, "zzub_wavelevel_save_wav_range", c_int, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("datastream", POINTER(zzub_output_t)), ("start", c_int), ("numsamples", c_int))
zzub_wavelevel_insert_sample_range = dlsym(libarmstrong, "zzub_wavelevel_insert_sample_range", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("start", c_int), ("buffer", c_void_p), ("channels", c_int), ("format", c_int), ("numsamples", c_int))
zzub_wavelevel_remove_sample_range = dlsym(libarmstrong, "zzub_wavelevel_remove_sample_range", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("start", c_int), ("numsamples", c_int))
zzub_wavelevel_replace_sample_range = dlsym(libarmstrong, "zzub_wavelevel_replace_sample_range", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("start", c_int), ("buffer", c_void_p), ("channels", c_int), ("format", c_int), ("numsamples", c_int))
zzub_wavelevel_get_samples_digest = dlsym(libarmstrong, "zzub_wavelevel_get_samples_digest", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("channel", c_int), ("start", c_int), ("end", c_int), ("mindigest", POINTER(c_float)), ("maxdigest", POINTER(c_float)), ("ampdigest", POINTER(c_float)), ("digestsize", c_int))
zzub_wavelevel_get_slices = dlsym(libarmstrong, "zzub_wavelevel_get_slices", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("slicecount", c_int), ("slices", POINTER(c_int)))
zzub_wavelevel_set_slices = dlsym(libarmstrong, "zzub_wavelevel_set_slices", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("slicecount", c_int), ("slices", POINTER(c_int)))
zzub_wavelevel_process_sample_range_offline = dlsym(libarmstrong, "zzub_wavelevel_process_sample_range_offline", None, ("Wavelevel", POINTER(zzub_wavelevel_t)), ("start", c_int), ("numsamples", c_int), ("plugin", POINTER(zzub_plugin_t)))
zzub_envelope_get_attack = dlsym(libarmstrong, "zzub_envelope_get_attack", c_ushort, ("Envelope", POINTER(zzub_envelope_t)))
zzub_envelope_get_decay = dlsym(libarmstrong, "zzub_envelope_get_decay", c_ushort, ("Envelope", POINTER(zzub_envelope_t)))
zzub_envelope_get_sustain = dlsym(libarmstrong, "zzub_envelope_get_sustain", c_ushort, ("Envelope", POINTER(zzub_envelope_t)))
zzub_envelope_get_release = dlsym(libarmstrong, "zzub_envelope_get_release", c_ushort, ("Envelope", POINTER(zzub_envelope_t)))
zzub_envelope_set_attack = dlsym(libarmstrong, "zzub_envelope_set_attack", None, ("Envelope", POINTER(zzub_envelope_t)), ("attack", c_ushort))
zzub_envelope_set_decay = dlsym(libarmstrong, "zzub_envelope_set_decay", None, ("Envelope", POINTER(zzub_envelope_t)), ("decay", c_ushort))
zzub_envelope_set_sustain = dlsym(libarmstrong, "zzub_envelope_set_sustain", None, ("Envelope", POINTER(zzub_envelope_t)), ("sustain", c_ushort))
zzub_envelope_set_release = dlsym(libarmstrong, "zzub_envelope_set_release", None, ("Envelope", POINTER(zzub_envelope_t)), ("release", c_ushort))
zzub_envelope_get_subdivision = dlsym(libarmstrong, "zzub_envelope_get_subdivision", c_byte, ("Envelope", POINTER(zzub_envelope_t)))
zzub_envelope_set_subdivision = dlsym(libarmstrong, "zzub_envelope_set_subdivision", None, ("Envelope", POINTER(zzub_envelope_t)), ("subdiv", c_byte))
zzub_envelope_get_flags = dlsym(libarmstrong, "zzub_envelope_get_flags", c_byte, ("Envelope", POINTER(zzub_envelope_t)))
zzub_envelope_set_flags = dlsym(libarmstrong, "zzub_envelope_set_flags", None, ("Envelope", POINTER(zzub_envelope_t)), ("flags", c_byte))
zzub_envelope_is_enabled = dlsym(libarmstrong, "zzub_envelope_is_enabled", c_int, ("Envelope", POINTER(zzub_envelope_t)))
zzub_envelope_enable = dlsym(libarmstrong, "zzub_envelope_enable", None, ("Envelope", POINTER(zzub_envelope_t)), ("enable", c_int))
zzub_envelope_get_point_count = dlsym(libarmstrong, "zzub_envelope_get_point_count", c_int, ("Envelope", POINTER(zzub_envelope_t)))
zzub_envelope_get_point = dlsym(libarmstrong, "zzub_envelope_get_point", None, ("Envelope", POINTER(zzub_envelope_t)), ("index", c_int), ("x", c_ushort), ("y", c_ushort), ("flags", c_byte))
zzub_envelope_set_point = dlsym(libarmstrong, "zzub_envelope_set_point", None, ("Envelope", POINTER(zzub_envelope_t)), ("index", c_int), ("x", c_ushort), ("y", c_ushort), ("flags", c_byte))
zzub_envelope_insert_point = dlsym(libarmstrong, "zzub_envelope_insert_point", None, ("Envelope", POINTER(zzub_envelope_t)), ("index", c_int))
zzub_envelope_delete_point = dlsym(libarmstrong, "zzub_envelope_delete_point", None, ("Envelope", POINTER(zzub_envelope_t)), ("index", c_int))
zzub_validation_error_iterator_next = dlsym(libarmstrong, "zzub_validation_error_iterator_next", None, ("ValidationErrorIterator", POINTER(zzub_validation_error_iterator_t)))
zzub_validation_error_iterator_valid = dlsym(libarmstrong, "zzub_validation_error_iterator_valid", c_int, ("ValidationErrorIterator", POINTER(zzub_validation_error_iterator_t)))
zzub_validation_error_iterator_current = dlsym(libarmstrong, "zzub_validation_error_iterator_current", POINTER(zzub_validation_error_t), ("ValidationErrorIterator", POINTER(zzub_validation_error_iterator_t)))
zzub_validation_error_iterator_reset = dlsym(libarmstrong, "zzub_validation_error_iterator_reset", None, ("ValidationErrorIterator", POINTER(zzub_validation_error_iterator_t)))
zzub_validation_error_iterator_destroy = dlsym(libarmstrong, "zzub_validation_error_iterator_destroy", None, ("ValidationErrorIterator", POINTER(zzub_validation_error_iterator_t)))
zzub_validation_error_get_type = dlsym(libarmstrong, "zzub_validation_error_get_type", c_int, ("ValidationError", POINTER(zzub_validation_error_t)))
zzub_validation_error_get_group = dlsym(libarmstrong, "zzub_validation_error_get_group", c_int, ("ValidationError", POINTER(zzub_validation_error_t)))
zzub_validation_error_get_column = dlsym(libarmstrong, "zzub_validation_error_get_column", c_int, ("ValidationError", POINTER(zzub_validation_error_t)))
zzub_validation_error_get_found_value = dlsym(libarmstrong, "zzub_validation_error_get_found_value", c_int, ("ValidationError", POINTER(zzub_validation_error_t)))
zzub_validation_error_get_expected_value = dlsym(libarmstrong, "zzub_validation_error_get_expected_value", c_int, ("ValidationError", POINTER(zzub_validation_error_t)))
zzub_validation_error_get_parameter_name = dlsym(libarmstrong, "zzub_validation_error_get_parameter_name", c_char_p, ("ValidationError", POINTER(zzub_validation_error_t)))
zzub_validation_error_get_plugin_name = dlsym(libarmstrong, "zzub_validation_error_get_plugin_name", c_char_p, ("ValidationError", POINTER(zzub_validation_error_t)))
zzub_validation_error_get_pluginloader = dlsym(libarmstrong, "zzub_validation_error_get_pluginloader", POINTER(zzub_pluginloader_t), ("ValidationError", POINTER(zzub_validation_error_t)))
zzub_player_create = dlsym(libarmstrong, "zzub_player_create", POINTER(zzub_player_t), ("hostpath", c_char_p), ("userpath", c_char_p), ("temppath", c_char_p))
zzub_player_destroy = dlsym(libarmstrong, "zzub_player_destroy", None, ("Player", POINTER(zzub_player_t)))
zzub_player_initialize = dlsym(libarmstrong, "zzub_player_initialize", c_int, ("Player", POINTER(zzub_player_t)), ("samplesPerSecond", c_int))
zzub_player_remote_connect = dlsym(libarmstrong, "zzub_player_remote_connect", c_int, ("Player", POINTER(zzub_player_t)), ("host", c_char_p), ("port", c_char_p))
zzub_player_remote_disconnect = dlsym(libarmstrong, "zzub_player_remote_disconnect", None, ("Player", POINTER(zzub_player_t)))
zzub_player_remote_open = dlsym(libarmstrong, "zzub_player_remote_open", c_int, ("Player", POINTER(zzub_player_t)), ("project", c_char_p), ("password", c_char_p))
zzub_player_remote_create = dlsym(libarmstrong, "zzub_player_remote_create", c_int, ("Player", POINTER(zzub_player_t)), ("project", c_char_p), ("password", c_char_p))
zzub_player_remote_delete = dlsym(libarmstrong, "zzub_player_remote_delete", c_int, ("Player", POINTER(zzub_player_t)), ("project", c_char_p), ("password", c_char_p))
zzub_player_get_remote_client_count = dlsym(libarmstrong, "zzub_player_get_remote_client_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_is_remote_connected = dlsym(libarmstrong, "zzub_player_is_remote_connected", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_load_armz = dlsym(libarmstrong, "zzub_player_load_armz", c_int, ("Player", POINTER(zzub_player_t)), ("fileName", c_char_p), ("mode", c_int), ("plugingroup", POINTER(zzub_plugin_group_t)))
zzub_player_save_armz = dlsym(libarmstrong, "zzub_player_save_armz", c_int, ("Player", POINTER(zzub_player_t)), ("fileName", c_char_p), ("plugins", POINTER(POINTER(zzub_plugin_t))), ("plugincount", c_int), ("plugingroup", POINTER(zzub_plugin_group_t)))
zzub_player_load_bmx = dlsym(libarmstrong, "zzub_player_load_bmx", c_int, ("Player", POINTER(zzub_player_t)), ("datastream", POINTER(zzub_input_t)), ("flags", c_int), ("x", c_float), ("y", c_float))
zzub_player_load_module = dlsym(libarmstrong, "zzub_player_load_module", c_int, ("Player", POINTER(zzub_player_t)), ("fileName", c_char_p))
zzub_player_get_validation_errors = dlsym(libarmstrong, "zzub_player_get_validation_errors", POINTER(zzub_validation_error_iterator_t), ("Player", POINTER(zzub_player_t)))
zzub_player_get_state = dlsym(libarmstrong, "zzub_player_get_state", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_state = dlsym(libarmstrong, "zzub_player_set_state", None, ("Player", POINTER(zzub_player_t)), ("state", c_int), ("stoprow", c_int))
zzub_player_get_pluginloader_count = dlsym(libarmstrong, "zzub_player_get_pluginloader_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_pluginloader = dlsym(libarmstrong, "zzub_player_get_pluginloader", POINTER(zzub_pluginloader_t), ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_pluginloader_by_name = dlsym(libarmstrong, "zzub_player_get_pluginloader_by_name", POINTER(zzub_pluginloader_t), ("Player", POINTER(zzub_player_t)), ("name", c_char_p))
zzub_player_get_plugin_count = dlsym(libarmstrong, "zzub_player_get_plugin_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_add_midimapping = dlsym(libarmstrong, "zzub_player_add_midimapping", POINTER(zzub_midimapping_t), ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("param", c_int), ("channel", c_int), ("controller", c_int))
zzub_player_remove_midimapping = dlsym(libarmstrong, "zzub_player_remove_midimapping", c_int, ("plugin", POINTER(zzub_plugin_t)), ("group", c_int), ("track", c_int), ("param", c_int))
zzub_player_get_plugin_by_name = dlsym(libarmstrong, "zzub_player_get_plugin_by_name", POINTER(zzub_plugin_t), ("Player", POINTER(zzub_player_t)), ("name", c_char_p))
zzub_player_get_plugin_by_id = dlsym(libarmstrong, "zzub_player_get_plugin_by_id", POINTER(zzub_plugin_t), ("Player", POINTER(zzub_player_t)), ("id", c_int))
zzub_player_get_plugin = dlsym(libarmstrong, "zzub_player_get_plugin", POINTER(zzub_plugin_t), ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_plugin_iterator = dlsym(libarmstrong, "zzub_player_get_plugin_iterator", POINTER(zzub_plugin_iterator_t), ("Player", POINTER(zzub_player_t)))
zzub_player_get_pattern_iterator = dlsym(libarmstrong, "zzub_player_get_pattern_iterator", POINTER(zzub_pattern_iterator_t), ("Player", POINTER(zzub_player_t)))
zzub_player_get_pattern_by_id = dlsym(libarmstrong, "zzub_player_get_pattern_by_id", POINTER(zzub_pattern_t), ("Player", POINTER(zzub_player_t)), ("id", c_int))
zzub_player_get_pattern_by_index = dlsym(libarmstrong, "zzub_player_get_pattern_by_index", POINTER(zzub_pattern_t), ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_pattern_event_by_id = dlsym(libarmstrong, "zzub_player_get_pattern_event_by_id", POINTER(zzub_pattern_event_t), ("Player", POINTER(zzub_player_t)), ("id", c_int))
zzub_player_get_new_pattern_name = dlsym(libarmstrong, "zzub_player_get_new_pattern_name", c_char_p, ("Player", POINTER(zzub_player_t)), ("format", POINTER(zzub_pattern_format_t)), ("description", c_char_p))
zzub_player_get_pattern_count = dlsym(libarmstrong, "zzub_player_get_pattern_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_pattern_by_name = dlsym(libarmstrong, "zzub_player_get_pattern_by_name", POINTER(zzub_pattern_t), ("Player", POINTER(zzub_player_t)), ("name", c_char_p))
zzub_player_get_pattern_format_count = dlsym(libarmstrong, "zzub_player_get_pattern_format_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_new_pattern_format_name = dlsym(libarmstrong, "zzub_player_get_new_pattern_format_name", c_char_p, ("Player", POINTER(zzub_player_t)), ("description", c_char_p))
zzub_player_get_pattern_format_by_name = dlsym(libarmstrong, "zzub_player_get_pattern_format_by_name", POINTER(zzub_pattern_format_t), ("Player", POINTER(zzub_player_t)), ("name", c_char_p))
zzub_player_get_pattern_format_by_index = dlsym(libarmstrong, "zzub_player_get_pattern_format_by_index", POINTER(zzub_pattern_format_t), ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_pattern_format_by_id = dlsym(libarmstrong, "zzub_player_get_pattern_format_by_id", POINTER(zzub_pattern_format_t), ("Player", POINTER(zzub_player_t)), ("id", c_int))
zzub_player_get_pattern_format_iterator = dlsym(libarmstrong, "zzub_player_get_pattern_format_iterator", POINTER(zzub_pattern_format_iterator_t), ("Player", POINTER(zzub_player_t)))
zzub_player_work_stereo = dlsym(libarmstrong, "zzub_player_work_stereo", None, ("Player", POINTER(zzub_player_t)), ("inbuffers", POINTER(POINTER(c_float))), ("outbuffers", POINTER(POINTER(c_float))), ("inchannels", c_int), ("outchannels", c_int), ("numsamples", c_int))
zzub_player_clear = dlsym(libarmstrong, "zzub_player_clear", None, ("Player", POINTER(zzub_player_t)))
zzub_player_get_wave_count = dlsym(libarmstrong, "zzub_player_get_wave_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_wave = dlsym(libarmstrong, "zzub_player_get_wave", POINTER(zzub_wave_t), ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_add_callback = dlsym(libarmstrong, "zzub_player_add_callback", None, ("Player", POINTER(zzub_player_t)), ("callback", zzub_callback_t), ("tag", c_void_p))
zzub_player_remove_callback = dlsym(libarmstrong, "zzub_player_remove_callback", None, ("Player", POINTER(zzub_player_t)), ("callback", zzub_callback_t), ("tag", c_void_p))
zzub_player_handle_events = dlsym(libarmstrong, "zzub_player_handle_events", None, ("Player", POINTER(zzub_player_t)))
zzub_player_get_midimapping = dlsym(libarmstrong, "zzub_player_get_midimapping", POINTER(zzub_midimapping_t), ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_midimapping_count = dlsym(libarmstrong, "zzub_player_get_midimapping_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_automation = dlsym(libarmstrong, "zzub_player_get_automation", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_automation = dlsym(libarmstrong, "zzub_player_set_automation", None, ("Player", POINTER(zzub_player_t)), ("enable", c_int))
zzub_player_get_midi_transport = dlsym(libarmstrong, "zzub_player_get_midi_transport", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_midi_transport = dlsym(libarmstrong, "zzub_player_set_midi_transport", None, ("Player", POINTER(zzub_player_t)), ("enable", c_int))
zzub_player_get_infotext = dlsym(libarmstrong, "zzub_player_get_infotext", c_char_p, ("Player", POINTER(zzub_player_t)))
zzub_player_set_infotext = dlsym(libarmstrong, "zzub_player_set_infotext", None, ("Player", POINTER(zzub_player_t)), ("text", c_char_p))
zzub_player_set_midi_plugin = dlsym(libarmstrong, "zzub_player_set_midi_plugin", None, ("Player", POINTER(zzub_player_t)), ("plugin", POINTER(zzub_plugin_t)))
zzub_player_get_midi_plugin = dlsym(libarmstrong, "zzub_player_get_midi_plugin", POINTER(zzub_plugin_t), ("Player", POINTER(zzub_player_t)))
zzub_player_get_midi_lock = dlsym(libarmstrong, "zzub_player_get_midi_lock", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_midi_lock = dlsym(libarmstrong, "zzub_player_set_midi_lock", None, ("Player", POINTER(zzub_player_t)), ("state", c_int))
zzub_player_get_new_plugin_name = dlsym(libarmstrong, "zzub_player_get_new_plugin_name", c_char_p, ("Player", POINTER(zzub_player_t)), ("uri", c_char_p))
zzub_player_reset_keyjazz = dlsym(libarmstrong, "zzub_player_reset_keyjazz", None, ("Player", POINTER(zzub_player_t)))
zzub_player_create_plugin = dlsym(libarmstrong, "zzub_player_create_plugin", POINTER(zzub_plugin_t), ("Player", POINTER(zzub_player_t)), ("input", POINTER(zzub_input_t)), ("dataSize", c_int), ("instanceName", c_char_p), ("loader", POINTER(zzub_pluginloader_t)), ("group", POINTER(zzub_plugin_group_t)))
zzub_player_create_pattern = dlsym(libarmstrong, "zzub_player_create_pattern", POINTER(zzub_pattern_t), ("Player", POINTER(zzub_player_t)), ("format", POINTER(zzub_pattern_format_t)), ("description", c_char_p), ("rows", c_int))
zzub_player_clone_pattern = dlsym(libarmstrong, "zzub_player_clone_pattern", POINTER(zzub_pattern_t), ("Player", POINTER(zzub_player_t)), ("pattern", POINTER(zzub_pattern_t)), ("description", c_char_p))
zzub_player_create_pattern_format = dlsym(libarmstrong, "zzub_player_create_pattern_format", POINTER(zzub_pattern_format_t), ("Player", POINTER(zzub_player_t)), ("description", c_char_p))
zzub_player_clone_pattern_format = dlsym(libarmstrong, "zzub_player_clone_pattern_format", POINTER(zzub_pattern_format_t), ("Player", POINTER(zzub_player_t)), ("format", POINTER(zzub_pattern_format_t)), ("description", c_char_p))
zzub_player_undo = dlsym(libarmstrong, "zzub_player_undo", None, ("Player", POINTER(zzub_player_t)))
zzub_player_redo = dlsym(libarmstrong, "zzub_player_redo", None, ("Player", POINTER(zzub_player_t)))
zzub_player_history_enable = dlsym(libarmstrong, "zzub_player_history_enable", c_int, ("Player", POINTER(zzub_player_t)), ("state", c_int))
zzub_player_history_begin = dlsym(libarmstrong, "zzub_player_history_begin", None, ("Player", POINTER(zzub_player_t)), ("userdata", c_void_p))
zzub_player_history_commit = dlsym(libarmstrong, "zzub_player_history_commit", None, ("Player", POINTER(zzub_player_t)), ("redo_id", c_int), ("undo_id", c_int), ("description", c_char_p))
zzub_player_history_get_uncomitted_operations = dlsym(libarmstrong, "zzub_player_history_get_uncomitted_operations", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_history_reset = dlsym(libarmstrong, "zzub_player_history_reset", None, ("Player", POINTER(zzub_player_t)))
zzub_player_history_get_size = dlsym(libarmstrong, "zzub_player_history_get_size", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_history_get_position = dlsym(libarmstrong, "zzub_player_history_get_position", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_history_get_description = dlsym(libarmstrong, "zzub_player_history_get_description", c_char_p, ("Player", POINTER(zzub_player_t)), ("position", c_int))
zzub_player_set_host_info = dlsym(libarmstrong, "zzub_player_set_host_info", None, ("Player", POINTER(zzub_player_t)), ("id", c_int), ("version", c_int), ("host_ptr", c_void_p))
zzub_player_invoke_event = dlsym(libarmstrong, "zzub_player_invoke_event", c_int, ("Player", POINTER(zzub_player_t)), ("data", POINTER(zzub_event_data_t)), ("immediate", c_int))
zzub_player_set_order_length = dlsym(libarmstrong, "zzub_player_set_order_length", None, ("Player", POINTER(zzub_player_t)), ("length", c_int))
zzub_player_get_order_length = dlsym(libarmstrong, "zzub_player_get_order_length", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_order_pattern = dlsym(libarmstrong, "zzub_player_set_order_pattern", None, ("Player", POINTER(zzub_player_t)), ("index", c_int), ("pattern", POINTER(zzub_pattern_t)))
zzub_player_get_order_pattern = dlsym(libarmstrong, "zzub_player_get_order_pattern", POINTER(zzub_pattern_t), ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_order_iterator = dlsym(libarmstrong, "zzub_player_get_order_iterator", POINTER(zzub_pattern_iterator_t), ("Player", POINTER(zzub_player_t)))
zzub_player_get_order_loop_start = dlsym(libarmstrong, "zzub_player_get_order_loop_start", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_order_loop_start = dlsym(libarmstrong, "zzub_player_set_order_loop_start", None, ("Player", POINTER(zzub_player_t)), ("pos", c_int))
zzub_player_get_order_loop_end = dlsym(libarmstrong, "zzub_player_get_order_loop_end", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_order_loop_end = dlsym(libarmstrong, "zzub_player_set_order_loop_end", None, ("Player", POINTER(zzub_player_t)), ("pos", c_int))
zzub_player_get_order_loop_enabled = dlsym(libarmstrong, "zzub_player_get_order_loop_enabled", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_order_loop_enabled = dlsym(libarmstrong, "zzub_player_set_order_loop_enabled", None, ("Player", POINTER(zzub_player_t)), ("enable", c_int))
zzub_player_set_queue_order_index = dlsym(libarmstrong, "zzub_player_set_queue_order_index", None, ("Player", POINTER(zzub_player_t)), ("pos", c_int))
zzub_player_get_queue_order_index = dlsym(libarmstrong, "zzub_player_get_queue_order_index", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_timeshift_order = dlsym(libarmstrong, "zzub_player_timeshift_order", None, ("Player", POINTER(zzub_player_t)), ("fromindex", c_int), ("timeshift", c_int))
zzub_player_get_position_order = dlsym(libarmstrong, "zzub_player_get_position_order", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_position_row = dlsym(libarmstrong, "zzub_player_get_position_row", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_position_samples = dlsym(libarmstrong, "zzub_player_get_position_samples", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_position = dlsym(libarmstrong, "zzub_player_set_position", None, ("Player", POINTER(zzub_player_t)), ("orderindex", c_int), ("tick", c_int))
zzub_player_adjust_position_order = dlsym(libarmstrong, "zzub_player_adjust_position_order", None, ("Player", POINTER(zzub_player_t)), ("orderindex", c_int))
zzub_player_get_bpm = dlsym(libarmstrong, "zzub_player_get_bpm", c_float, ("Player", POINTER(zzub_player_t)))
zzub_player_get_tpb = dlsym(libarmstrong, "zzub_player_get_tpb", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_swing = dlsym(libarmstrong, "zzub_player_get_swing", c_float, ("Player", POINTER(zzub_player_t)))
zzub_player_set_bpm = dlsym(libarmstrong, "zzub_player_set_bpm", None, ("Player", POINTER(zzub_player_t)), ("bpm", c_float))
zzub_player_set_tpb = dlsym(libarmstrong, "zzub_player_set_tpb", None, ("Player", POINTER(zzub_player_t)), ("tpb", c_int))
zzub_player_set_swing = dlsym(libarmstrong, "zzub_player_set_swing", None, ("Player", POINTER(zzub_player_t)), ("swing", c_float))
zzub_player_set_swing_ticks = dlsym(libarmstrong, "zzub_player_set_swing_ticks", None, ("Player", POINTER(zzub_player_t)), ("swing_ticks", c_int))
zzub_player_get_timesource_count = dlsym(libarmstrong, "zzub_player_get_timesource_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_timesource_plugin = dlsym(libarmstrong, "zzub_player_get_timesource_plugin", POINTER(zzub_plugin_t), ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_timesource_group = dlsym(libarmstrong, "zzub_player_get_timesource_group", c_int, ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_timesource_track = dlsym(libarmstrong, "zzub_player_get_timesource_track", c_int, ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_play_pattern = dlsym(libarmstrong, "zzub_player_play_pattern", None, ("Player", POINTER(zzub_player_t)), ("pat", POINTER(zzub_pattern_t)), ("row", c_int), ("stoprow", c_int))
zzub_player_get_machineview_offset_x = dlsym(libarmstrong, "zzub_player_get_machineview_offset_x", c_double, ("Player", POINTER(zzub_player_t)))
zzub_player_get_machineview_offset_y = dlsym(libarmstrong, "zzub_player_get_machineview_offset_y", c_double, ("Player", POINTER(zzub_player_t)))
zzub_player_set_machineview_offset = dlsym(libarmstrong, "zzub_player_set_machineview_offset", None, ("Player", POINTER(zzub_player_t)), ("x", c_double), ("y", c_double))
zzub_player_get_thread_count = dlsym(libarmstrong, "zzub_player_get_thread_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_set_thread_count = dlsym(libarmstrong, "zzub_player_set_thread_count", None, ("Player", POINTER(zzub_player_t)), ("threads", c_int))
zzub_player_get_peaks = dlsym(libarmstrong, "zzub_player_get_peaks", None, ("Player", POINTER(zzub_player_t)), ("peaks", POINTER(c_float)), ("peakcount", c_int))
zzub_player_get_waveimporter_count = dlsym(libarmstrong, "zzub_player_get_waveimporter_count", c_int, ("Player", POINTER(zzub_player_t)))
zzub_player_get_waveimporter_format_ext_count = dlsym(libarmstrong, "zzub_player_get_waveimporter_format_ext_count", c_int, ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_waveimporter_format_ext = dlsym(libarmstrong, "zzub_player_get_waveimporter_format_ext", c_char_p, ("Player", POINTER(zzub_player_t)), ("index", c_int), ("extindex", c_int))
zzub_player_get_waveimporter_format_is_container = dlsym(libarmstrong, "zzub_player_get_waveimporter_format_is_container", c_int, ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_get_waveimporter_format_type = dlsym(libarmstrong, "zzub_player_get_waveimporter_format_type", c_int, ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_create_waveimporter = dlsym(libarmstrong, "zzub_player_create_waveimporter", POINTER(zzub_wave_importer_t), ("Player", POINTER(zzub_player_t)), ("index", c_int))
zzub_player_create_waveimporter_by_file = dlsym(libarmstrong, "zzub_player_create_waveimporter_by_file", POINTER(zzub_wave_importer_t), ("Player", POINTER(zzub_player_t)), ("filename", c_char_p))
zzub_player_create_plugin_group = dlsym(libarmstrong, "zzub_player_create_plugin_group", POINTER(zzub_plugin_group_t), ("Player", POINTER(zzub_player_t)), ("parent", POINTER(zzub_plugin_group_t)), ("name", c_char_p))
zzub_player_get_plugin_group_by_id = dlsym(libarmstrong, "zzub_player_get_plugin_group_by_id", POINTER(zzub_plugin_group_t), ("Player", POINTER(zzub_player_t)), ("id", c_int))
zzub_player_get_plugin_group_iterator = dlsym(libarmstrong, "zzub_player_get_plugin_group_iterator", POINTER(zzub_plugin_group_iterator_t), ("Player", POINTER(zzub_player_t)), ("parent", POINTER(zzub_plugin_group_t)))
zzub_plugin_group_destroy = dlsym(libarmstrong, "zzub_plugin_group_destroy", None, ("PluginGroup", POINTER(zzub_plugin_group_t)))
zzub_plugin_group_get_id = dlsym(libarmstrong, "zzub_plugin_group_get_id", c_int, ("PluginGroup", POINTER(zzub_plugin_group_t)))
zzub_plugin_group_get_name = dlsym(libarmstrong, "zzub_plugin_group_get_name", c_char_p, ("PluginGroup", POINTER(zzub_plugin_group_t)))
zzub_plugin_group_set_name = dlsym(libarmstrong, "zzub_plugin_group_set_name", None, ("PluginGroup", POINTER(zzub_plugin_group_t)), ("name", c_char_p))
zzub_plugin_group_get_parent = dlsym(libarmstrong, "zzub_plugin_group_get_parent", POINTER(zzub_plugin_group_t), ("PluginGroup", POINTER(zzub_plugin_group_t)))
zzub_plugin_group_set_parent = dlsym(libarmstrong, "zzub_plugin_group_set_parent", None, ("PluginGroup", POINTER(zzub_plugin_group_t)), ("newparent", POINTER(zzub_plugin_group_t)))
zzub_plugin_group_get_position_x = dlsym(libarmstrong, "zzub_plugin_group_get_position_x", c_float, ("PluginGroup", POINTER(zzub_plugin_group_t)))
zzub_plugin_group_get_position_y = dlsym(libarmstrong, "zzub_plugin_group_get_position_y", c_float, ("PluginGroup", POINTER(zzub_plugin_group_t)))
zzub_plugin_group_set_position = dlsym(libarmstrong, "zzub_plugin_group_set_position", None, ("PluginGroup", POINTER(zzub_plugin_group_t)), ("x", c_float), ("y", c_float))
zzub_plugin_group_get_plugins = dlsym(libarmstrong, "zzub_plugin_group_get_plugins", POINTER(zzub_plugin_iterator_t), ("PluginGroup", POINTER(zzub_plugin_group_t)))
zzub_plugin_group_iterator_next = dlsym(libarmstrong, "zzub_plugin_group_iterator_next", None, ("PluginGroupIterator", POINTER(zzub_plugin_group_iterator_t)))
zzub_plugin_group_iterator_valid = dlsym(libarmstrong, "zzub_plugin_group_iterator_valid", c_int, ("PluginGroupIterator", POINTER(zzub_plugin_group_iterator_t)))
zzub_plugin_group_iterator_current = dlsym(libarmstrong, "zzub_plugin_group_iterator_current", POINTER(zzub_plugin_group_t), ("PluginGroupIterator", POINTER(zzub_plugin_group_iterator_t)))
zzub_plugin_group_iterator_destroy = dlsym(libarmstrong, "zzub_plugin_group_iterator_destroy", None, ("PluginGroupIterator", POINTER(zzub_plugin_group_iterator_t)))
zzub_wave_importer_open = dlsym(libarmstrong, "zzub_wave_importer_open", c_int, ("WaveImporter", POINTER(zzub_wave_importer_t)), ("filename", c_char_p), ("strm", POINTER(zzub_input_t)))
zzub_wave_importer_destroy = dlsym(libarmstrong, "zzub_wave_importer_destroy", None, ("WaveImporter", POINTER(zzub_wave_importer_t)))
zzub_wave_importer_get_instrument_count = dlsym(libarmstrong, "zzub_wave_importer_get_instrument_count", c_int, ("WaveImporter", POINTER(zzub_wave_importer_t)))
zzub_wave_importer_get_instrument_name = dlsym(libarmstrong, "zzub_wave_importer_get_instrument_name", c_char_p, ("WaveImporter", POINTER(zzub_wave_importer_t)), ("index", c_int))
zzub_wave_importer_get_instrument_sample_count = dlsym(libarmstrong, "zzub_wave_importer_get_instrument_sample_count", c_int, ("WaveImporter", POINTER(zzub_wave_importer_t)), ("index", c_int))
zzub_wave_importer_get_instrument_sample_info = dlsym(libarmstrong, "zzub_wave_importer_get_instrument_sample_info", None, ("WaveImporter", POINTER(zzub_wave_importer_t)), ("index", c_int), ("sample", c_int), ("name", c_char_p), ("namelen", c_int), ("samplecount", c_int), ("channels", c_int), ("format", c_int), ("samplerate", c_int))
zzub_wave_importer_load_instrument = dlsym(libarmstrong, "zzub_wave_importer_load_instrument", c_int, ("WaveImporter", POINTER(zzub_wave_importer_t)), ("index", c_int), ("dest", POINTER(zzub_wave_t)))
zzub_wave_importer_load_instrument_sample = dlsym(libarmstrong, "zzub_wave_importer_load_instrument_sample", c_int, ("WaveImporter", POINTER(zzub_wave_importer_t)), ("index", c_int), ("sample", c_int), ("dest", POINTER(zzub_wavelevel_t)))
class EventData(object):
  """Event data sent to callbacks. The 'type' member indicates an event code and which data fields are valid."""
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

zzub_event_data_t._wrapper_ = EventData

class DeviceInfo(object):
  """Description of an audio device.
  The engine sends a device_reset-event when the DeviceInfo objects become invalid and must be re-enumerated."""
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

  def get_api(self):
    """Returns an identifier of the device API (ASIO, DS, etc)"""
    assert self._as_parameter_
    return zzub_device_info_get_api(self)

  def get_name(self):
    """Returns the name of the audio device"""
    assert self._as_parameter_
    return zzub_device_info_get_name(self)

  def get_supported_buffersizes(self, maxsizes):
    """Returns an array of supported buffersizes.
    This returns nothing for input devices, which always use the same buffer size as the output device."""
    assert self._as_parameter_
    result = (c_int*maxsizes)()
    _ret_val = zzub_device_info_get_supported_buffersizes(self, result, maxsizes)
    return _ret_val, [v for v in result]

  def get_supported_samplerates(self, maxrates):
    """Returns an array of supported sample rates.
    This returns nothing for input devices, which always use the same samplerate as the output device."""
    assert self._as_parameter_
    result = (c_int*maxrates)()
    _ret_val = zzub_device_info_get_supported_samplerates(self, result, maxrates)
    return _ret_val, [v for v in result]

  def get_supported_output_channels(self):
    """Returns the number of supported output channels as reported by the driver."""
    assert self._as_parameter_
    return zzub_device_info_get_supported_output_channels(self)

  def get_supported_input_channels(self):
    """Returns the number of supported input channels as reported by the driver."""
    assert self._as_parameter_
    return zzub_device_info_get_supported_input_channels(self)

zzub_device_info_t._wrapper_ = DeviceInfo

class DeviceInfoIterator(object):
  """Iterator for audio device infos."""
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

  def next(self):
    """Iterates to the next item."""
    assert self._as_parameter_
    zzub_device_info_iterator_next(self)

  def valid(self):
    """Returns true if the iterator is valid and it is safe to call current()"""
    assert self._as_parameter_
    return zzub_device_info_iterator_valid(self)

  def current(self):
    """Returns the current item."""
    assert self._as_parameter_
    return DeviceInfo._new_from_handle(zzub_device_info_iterator_current(self))

  def reset(self):
    """Resets the iterator to the start."""
    assert self._as_parameter_
    zzub_device_info_iterator_reset(self)

  def destroy(self):
    """Destroys the iterator."""
    assert self._as_parameter_
    zzub_device_info_iterator_destroy(self)

zzub_device_info_iterator_t._wrapper_ = DeviceInfoIterator

class Audiodriver(object):
  """Methods for enumerating, configuring and creating an audio driver instance."""
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
  def create_silent(player, name, out_channels, in_channels, num_rates):
    """Create a silent, non-processing audio driver that has one device with the specified properties.
    This driver has no audio thread. Samples must be generated manually by calling the work_stereo method on the Player object."""
    supported_rates = (c_int*num_rates)()
    _ret_val = Audiodriver._new_from_handle(zzub_audiodriver_create_silent(player, name, out_channels, in_channels, supported_rates, num_rates))
    return _ret_val, [v for v in supported_rates]

  @staticmethod
  def create(player):
    """Creates an audio driver for enumerating audio devices provided by the operating system."""
    return Audiodriver._new_from_handle(zzub_audiodriver_create(player))

  def get_count(self):
    """Get number of detected input and output audio devices."""
    assert self._as_parameter_
    return zzub_audiodriver_get_count(self)

  def get_device_info(self, index):
    """Returns info for the device at the specified index.
    The engine sends a device_reset-event when the DeviceInfo objects become invalid and must be re-enumerated."""
    assert self._as_parameter_
    return DeviceInfo._new_from_handle(zzub_audiodriver_get_device_info(self, index))

  def get_device_info_by_name(self, name):
    """Returns info for the device at the specified index.
    The engine sends a device_reset-event when the DeviceInfo objects become invalid and must be re-enumerated."""
    assert self._as_parameter_
    return DeviceInfo._new_from_handle(zzub_audiodriver_get_device_info_by_name(self, name))

  def get_output_iterator(self):
    """Returns an iterator for all output devices.
    The engine sends a device_reset-event when the DeviceInfo objects become invalid and must be re-enumerated."""
    assert self._as_parameter_
    return DeviceInfoIterator._new_from_handle(zzub_audiodriver_get_output_iterator(self))

  def get_input_iterator(self):
    """Returns an iterator for all input devices.
    The engine sends a device_reset-event when the DeviceInfo objects become invalid and must be re-enumerated."""
    assert self._as_parameter_
    return DeviceInfoIterator._new_from_handle(zzub_audiodriver_get_input_iterator(self))

  def get_input_iterator_for_output(self, info):
    """Returns an iterator with devices suitable as input for the specified output device.
    The engine sends a device_reset-event when the DeviceInfo objects become invalid and must be re-enumerated."""
    assert self._as_parameter_
    return DeviceInfoIterator._new_from_handle(zzub_audiodriver_get_input_iterator_for_output(self, info))

  def create_device(self, input_name, output_name, buffersize, samplerate):
    """Create specified audio device. Audio device is disabled by default."""
    assert self._as_parameter_
    return zzub_audiodriver_create_device(self, input_name, output_name, buffersize, samplerate)

  def get_current_device(self, for_input):
    """Returns a DeviceInfo object for the current device. Returns NULL if no device was created.
    The engine sends a device_reset-event when the DeviceInfo objects become invalid and must be re-enumerated."""
    assert self._as_parameter_
    return DeviceInfo._new_from_handle(zzub_audiodriver_get_current_device(self, for_input))

  def enable(self, state):
    """Enable or disable current audio driver."""
    assert self._as_parameter_
    zzub_audiodriver_enable(self, state)

  def get_enabled(self):
    """Returns whether current audio driver is enabled or disabled."""
    assert self._as_parameter_
    return zzub_audiodriver_get_enabled(self)

  def destroy(self):
    """Disassociate audio driver and player."""
    assert self._as_parameter_
    zzub_audiodriver_destroy(self)

  def destroy_device(self):
    """De-allocate the current device."""
    assert self._as_parameter_
    zzub_audiodriver_destroy_device(self)

  def get_samplerate(self):
    """Configuration: Retreive audio driver sample rate."""
    assert self._as_parameter_
    return zzub_audiodriver_get_samplerate(self)

  def get_buffersize(self):
    """Configuration: Retreive audio driver buffer size/latency."""
    assert self._as_parameter_
    return zzub_audiodriver_get_buffersize(self)

  def get_cpu_load(self):
    """Returns CPU load of the audio drivers callback."""
    assert self._as_parameter_
    return zzub_audiodriver_get_cpu_load(self)

  def get_master_channel(self):
    """Configuration: The audio device channel on which the Master plugin sends it output. Usually 0, but can be greater for audio devices that support more than two output channels."""
    assert self._as_parameter_
    return zzub_audiodriver_get_master_channel(self)

  def set_master_channel(self, index):
    """Configuration: The audio device channel on which the Master plugin sends it output. Usually 0, but can be greater for audio devices that support more than two output channels."""
    assert self._as_parameter_
    zzub_audiodriver_set_master_channel(self, index)

  def configure(self):
    """Open the configuration user interface for the active driver.
    Does nothing if the driver does not implement a user interface.
    When the user presses OK, the driver might request a device reset with the new settings and issue a samplerate_changed-event.
    This is only used with ASIO devices and will do nothing on other devices."""
    assert self._as_parameter_
    zzub_audiodriver_configure(self)

zzub_audiodriver_t._wrapper_ = Audiodriver

class Mididriver(object):
  """Methods for enumerating and opening MIDI input and output devices."""
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
  def get_count(player):
    """Returns the number of detected input and output MIDI devices."""
    return zzub_mididriver_get_count(player)

  @staticmethod
  def get_name(player, index):
    """Returns the name of a MIDI device."""
    return zzub_mididriver_get_name(player, index)

  @staticmethod
  def is_input(player, index):
    """Returns true if the device is an input device."""
    return zzub_mididriver_is_input(player, index)

  @staticmethod
  def is_output(player, index):
    """Returns true if the device is an output device."""
    return zzub_mididriver_is_output(player, index)

  @staticmethod
  def open(player, index):
    """Opens the MIDI device."""
    return zzub_mididriver_open(player, index)

  @staticmethod
  def close_all(player):
    """Closes all MIDI devices."""
    return zzub_mididriver_close_all(player)

zzub_mididriver_t._wrapper_ = Mididriver

class Plugincollection(object):
  """For enumerating and configuring plugin collections."""
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
  def get_by_uri(player, uri):
    return Plugincollection._new_from_handle(zzub_plugincollection_get_by_uri(player, uri))

  def get_name(self):
    """Returns the name of the plugin collection. E.g 'VST' for the VST wrapper."""
    assert self._as_parameter_
    return zzub_plugincollection_get_name(self)

  def configure(self, key, value):
    """Passes a key/value-pair to the plugin collection. Used to set e.g VST scanning paths."""
    assert self._as_parameter_
    zzub_plugincollection_configure(self, key, value)

zzub_plugincollection_t._wrapper_ = Plugincollection

class Input(object):
  """Methods for working with file- or memory-based input streams. E.g reading from a file or buffer."""
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
  def open_file(filename):
    """Create an input stream that reads from a file."""
    return Input._new_from_handle(zzub_input_open_file(filename))

  def destroy(self):
    """Closes an input stream created with zzub_create_output_XXX."""
    assert self._as_parameter_
    zzub_input_destroy(self)

  def read(self, bytes):
    """Reads bytes from the stream."""
    assert self._as_parameter_
    buffer = (c_char*bytes)()
    _ret_val = zzub_input_read(self, buffer, bytes)
    return _ret_val, [v for v in buffer]

  def size(self):
    """Returns the total number of bytes that can be read from the stream."""
    assert self._as_parameter_
    return zzub_input_size(self)

  def position(self):
    """Returns the current read position in the stream."""
    assert self._as_parameter_
    return zzub_input_position(self)

  def seek(self, pos, mode):
    """Seeks to a specified offset in the stream."""
    assert self._as_parameter_
    zzub_input_seek(self, pos, mode)

zzub_input_t._wrapper_ = Input

class Output(object):
  """Methods for working with file- or memory-based output streams. E.g writing to a file or buffer."""
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
  def create_file(filename):
    """Create an output stream that writes to a file."""
    return Output._new_from_handle(zzub_output_create_file(filename))

  def destroy(self):
    """Closes an output stream created with zzub_create_output_XXX."""
    assert self._as_parameter_
    zzub_output_destroy(self)

  def write(self, buffer, bytes):
    """Writes bytes to the stream."""
    assert self._as_parameter_
    zzub_output_write(self, buffer, bytes)

  def position(self):
    """Returns the current write position in the stream."""
    assert self._as_parameter_
    return zzub_output_position(self)

  def seek(self, pos, mode):
    """Seeks to a specified offset in the stream."""
    assert self._as_parameter_
    zzub_output_seek(self, pos, mode)

zzub_output_t._wrapper_ = Output

class Archive(object):
  """Methods for working with in-memory streams."""
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
  def create_memory():
    """Create an in-memory archive of keyed input and output streams."""
    return Archive._new_from_handle(zzub_archive_create_memory())

  def get_output(self, path):
    """Returns an output stream object for writing."""
    assert self._as_parameter_
    return Output._new_from_handle(zzub_archive_get_output(self, path))

  def get_input(self, path):
    """Returns an input stream object for reading."""
    assert self._as_parameter_
    return Input._new_from_handle(zzub_archive_get_input(self, path))

  def destroy(self):
    """Destroys the archive and frees allocated resources."""
    assert self._as_parameter_
    zzub_archive_destroy(self)

zzub_archive_t._wrapper_ = Archive

class Midimapping(object):
  """Represents a binding between a MIDI controller and a plugin parameter.
  To create a MIDI mapping, see zzub_player_add_midimapping()."""
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

  def get_plugin(self):
    """Bound plugin."""
    assert self._as_parameter_
    return zzub_midimapping_get_plugin(self)

  def get_group(self):
    """Bound plugin group."""
    assert self._as_parameter_
    return zzub_midimapping_get_group(self)

  def get_track(self):
    """Bound plugin track."""
    assert self._as_parameter_
    return zzub_midimapping_get_track(self)

  def get_column(self):
    """Bound plugin column."""
    assert self._as_parameter_
    return zzub_midimapping_get_column(self)

  def get_channel(self):
    """Bound MIDI channel."""
    assert self._as_parameter_
    return zzub_midimapping_get_channel(self)

  def get_controller(self):
    """Bound MIDI controller."""
    assert self._as_parameter_
    return zzub_midimapping_get_controller(self)

zzub_midimapping_t._wrapper_ = Midimapping

class PatternEvent(object):
  """Represents an event in a pattern."""
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

  def get_id(self):
    """Event ID."""
    assert self._as_parameter_
    return zzub_pattern_event_get_id(self)

  def get_pluginid(self):
    """Event plugin id."""
    assert self._as_parameter_
    return zzub_pattern_event_get_pluginid(self)

  def get_pattern(self):
    """Returns the pattern in which the event occurs."""
    assert self._as_parameter_
    return Pattern._new_from_handle(zzub_pattern_event_get_pattern(self))

  def get_group(self):
    """Event plugin group."""
    assert self._as_parameter_
    return zzub_pattern_event_get_group(self)

  def get_track(self):
    """Event plugin track."""
    assert self._as_parameter_
    return zzub_pattern_event_get_track(self)

  def get_column(self):
    """Event plugin column."""
    assert self._as_parameter_
    return zzub_pattern_event_get_column(self)

  def get_time(self):
    """Event timestamp. The unit of the timestamp depends on the context in which the pattern is being played."""
    assert self._as_parameter_
    return zzub_pattern_event_get_time(self)

  def get_value(self):
    """Returns the event value."""
    assert self._as_parameter_
    return zzub_pattern_event_get_value(self)

  def get_meta(self):
    """Returns the events meta value. Used with notes."""
    assert self._as_parameter_
    return zzub_pattern_event_get_meta(self)

  def set_value(self, value):
    """Set a new value on this event."""
    assert self._as_parameter_
    return zzub_pattern_event_set_value(self, value)

  def set_meta(self, meta):
    """Set a new meta value on this event."""
    assert self._as_parameter_
    return zzub_pattern_event_set_meta(self, meta)

  def set_time(self, value):
    """Set a new timestamp on this event."""
    assert self._as_parameter_
    return zzub_pattern_event_set_time(self, value)

zzub_pattern_event_t._wrapper_ = PatternEvent

class PatternIterator(object):
  """Helper iterator for a range of patterns."""
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

  def next(self):
    """Iterates to the next item."""
    assert self._as_parameter_
    zzub_pattern_iterator_next(self)

  def valid(self):
    """Returns true if the iterator is valid and it is safe to call current()"""
    assert self._as_parameter_
    return zzub_pattern_iterator_valid(self)

  def current(self):
    """Returns the current item."""
    assert self._as_parameter_
    return Pattern._new_from_handle(zzub_pattern_iterator_current(self))

  def destroy(self):
    """Destroys the iterator."""
    assert self._as_parameter_
    zzub_pattern_iterator_destroy(self)

zzub_pattern_iterator_t._wrapper_ = PatternIterator

class PatternEventIterator(object):
  """Helper iterator for a range of pattern events."""
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

  def next(self):
    """Iterates to the next item."""
    assert self._as_parameter_
    zzub_pattern_event_iterator_next(self)

  def valid(self):
    """Returns true if the iterator is valid and it is safe to call current()"""
    assert self._as_parameter_
    return zzub_pattern_event_iterator_valid(self)

  def current(self):
    """Returns the current item."""
    assert self._as_parameter_
    return PatternEvent._new_from_handle(zzub_pattern_event_iterator_current(self))

  def destroy(self):
    """Destroys the iterator."""
    assert self._as_parameter_
    zzub_pattern_event_iterator_destroy(self)

zzub_pattern_event_iterator_t._wrapper_ = PatternEventIterator

class Pattern(object):
  """Methods for working with a pattern.
  The format of a pattern is completely dynamic, defined by a zzub_pattern_format.
  To create a new pattern, see zzub_player_create_pattern()."""
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

  def destroy(self):
    """Destroys the pattern and frees all resources."""
    assert self._as_parameter_
    zzub_pattern_destroy(self)

  def get_name(self):
    """Returns the pattern name."""
    assert self._as_parameter_
    return zzub_pattern_get_name(self)

  def set_name(self, name):
    """Set the pattern name."""
    assert self._as_parameter_
    zzub_pattern_set_name(self, name)

  def get_row_count(self):
    """Returns length of pattern."""
    assert self._as_parameter_
    return zzub_pattern_get_row_count(self)

  def set_row_count(self, length):
    """Sets length of pattern."""
    assert self._as_parameter_
    zzub_pattern_set_row_count(self, length)

  def get_id(self):
    """Pattern ID."""
    assert self._as_parameter_
    return zzub_pattern_get_id(self)

  def get_format(self):
    """Returns the pattern format."""
    assert self._as_parameter_
    return PatternFormat._new_from_handle(zzub_pattern_get_format(self))

  def set_format(self, format):
    """Sets the pattern format."""
    assert self._as_parameter_
    zzub_pattern_set_format(self, format)

  def get_resolution(self):
    """Returns the pattern resolution - rows per tick."""
    assert self._as_parameter_
    return zzub_pattern_get_resolution(self)

  def set_resolution(self, resolution):
    """Sets the pattern resolution."""
    assert self._as_parameter_
    zzub_pattern_set_resolution(self, resolution)

  def get_display_resolution(self):
    """Display resolution. Has no effect on the audio output."""
    assert self._as_parameter_
    return zzub_pattern_get_display_resolution(self)

  def set_display_resolution(self, resolution):
    """Display resolution. Has no effect on the audio output."""
    assert self._as_parameter_
    zzub_pattern_set_display_resolution(self, resolution)

  def get_display_beat_rows(self):
    """Display beat coloring. Has no effect on the audio output."""
    assert self._as_parameter_
    verydarkrow = (c_int)()
    darkrow = (c_int)()
    zzub_pattern_get_display_beat_rows(self, verydarkrow, darkrow)
    return verydarkrow.value, darkrow.value

  def set_display_beat_rows(self, verydarkrow, darkrow):
    """Display beat coloring. Has no effect on the audio output."""
    assert self._as_parameter_
    zzub_pattern_set_display_beat_rows(self, verydarkrow, darkrow)

  def get_loop_start(self):
    """Returns where the pattern starts looping."""
    assert self._as_parameter_
    return zzub_pattern_get_loop_start(self)

  def set_loop_start(self, pos):
    """Sets where the pattern starts looping."""
    assert self._as_parameter_
    zzub_pattern_set_loop_start(self, pos)

  def get_loop_end(self):
    """Returns where the pattern ends looping."""
    assert self._as_parameter_
    return zzub_pattern_get_loop_end(self)

  def set_loop_end(self, pos):
    """Sets where the pattern ends looping."""
    assert self._as_parameter_
    zzub_pattern_set_loop_end(self, pos)

  def get_loop_enabled(self):
    """Returns whether pattern looping is enabled."""
    assert self._as_parameter_
    return zzub_pattern_get_loop_enabled(self)

  def set_loop_enabled(self, enable):
    """Sets whether pattern looping is enabled."""
    assert self._as_parameter_
    zzub_pattern_set_loop_enabled(self, enable)

  def get_replay_row(self):
    """Returns the position from where pattern replay starts."""
    assert self._as_parameter_
    return zzub_pattern_get_replay_row(self)

  def set_replay_row(self, row):
    """Sets the position from where pattern replay starts."""
    assert self._as_parameter_
    zzub_pattern_set_replay_row(self, row)

  def get_currently_playing_row(self):
    """Retreive the currently playing row for a pattern."""
    assert self._as_parameter_
    return zzub_pattern_get_currently_playing_row(self)

  def set_value(self, row, plugin, group, track, column, value, meta):
    """Sets the value of a pattern event.
    This checks for the existence of an existing event at the specified timestamp, and updates the value if it exists. For a faster version, see zzub_pattern_insert_value()."""
    assert self._as_parameter_
    zzub_pattern_set_value(self, row, plugin, group, track, column, value, meta)

  def get_value(self, row, pluginid, group, track, column):
    """Returns the value of a pattern event."""
    assert self._as_parameter_
    value = (c_int)()
    meta = (c_int)()
    _ret_val = zzub_pattern_get_value(self, row, pluginid, group, track, column, value, meta)
    return _ret_val, value.value, meta.value

  def get_event_iterator(self, plugin, group, track, column):
    """Returns an iterator for iterating over pattern events. Pass NULL for the plugin parameter to retreive all events. -1 can be sent to group, track and column to return only a selected group/track/column."""
    assert self._as_parameter_
    return PatternEventIterator._new_from_handle(zzub_pattern_get_event_iterator(self, plugin, group, track, column))

  def get_event_unsorted_iterator(self, plugin, group, track, column):
    """Like get_event_iterator, but not sorted by time. Performs faster."""
    assert self._as_parameter_
    return PatternEventIterator._new_from_handle(zzub_pattern_get_event_unsorted_iterator(self, plugin, group, track, column))

  def insert_value(self, pluginid, group, track, column, time, value, meta):
    """Inserts a value into the pattern without checking if an event already exists. This should only be used for new patterns: the behavior of multiple events at the same timestamp is undefined."""
    assert self._as_parameter_
    zzub_pattern_insert_value(self, pluginid, group, track, column, time, value, meta)

  def delete_value(self, id):
    """Deletes an event from the pattern."""
    assert self._as_parameter_
    zzub_pattern_delete_value(self, id)

  def update_value(self, id, time, value, meta):
    """Changes the time stamp and values of an event."""
    assert self._as_parameter_
    zzub_pattern_update_value(self, id, time, value, meta)

  def update_value_full(self, id, pluginid, group, track, column, time, value, meta):
    """Changes the parameter, time stamp and values of an event."""
    assert self._as_parameter_
    zzub_pattern_update_value_full(self, id, pluginid, group, track, column, time, value, meta)

  def compact_pattern(self, factor):
    """Transform: Compact pattern by a factor."""
    assert self._as_parameter_
    zzub_pattern_compact_pattern(self, factor)

  def expand_pattern(self, factor):
    """Transform: Expand pattern by a factor."""
    assert self._as_parameter_
    zzub_pattern_expand_pattern(self, factor)

  def timeshift_events(self, pluginid, group, track, column, fromtime, timeshift):
    """Transform: Shift events by time."""
    assert self._as_parameter_
    zzub_pattern_timeshift_events(self, pluginid, group, track, column, fromtime, timeshift)

  def delete_events(self, pluginid, group, track, column, fromtime, length):
    """Transform: Delete events."""
    assert self._as_parameter_
    zzub_pattern_delete_events(self, pluginid, group, track, column, fromtime, length)

  def move_scale_events(self, src_idx, src_time, dst_idx, dst_time, width, length, mode, makecopy):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_move_scale_events(self, src_idx, src_time, dst_idx, dst_time, width, length, mode, makecopy)

  def paste_stream_events(self, fromidx, fromtime, mode, charbuf):
    """Accepts a stream of pattern events and pastes them at a location in the pattern. All parameter ranges will be scaled."""
    assert self._as_parameter_
    zzub_pattern_paste_stream_events(self, fromidx, fromtime, mode, charbuf)

  def transpose_events(self, pluginid, group, track, column, fromtime, length, delta, holecount, metacount, chromatic):
    """Transform."""
    assert self._as_parameter_
    holes = (c_int*holecount)()
    metas = (c_int*metacount)()
    zzub_pattern_transpose_events(self, pluginid, group, track, column, fromtime, length, delta, holes, holecount, metas, metacount, chromatic)
    return [v for v in holes], [v for v in metas]

  def randomize_events(self, pluginid, group, track, column, fromtime, length, skip):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_randomize_events(self, pluginid, group, track, column, fromtime, length, skip)

  def randomize_range_events(self, pluginid, group, track, column, fromtime, length, from_val, to_val, skip):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_randomize_range_events(self, pluginid, group, track, column, fromtime, length, from_val, to_val, skip)

  def randomize_from_events(self, pluginid, group, track, column, fromtime, length, skip):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_randomize_from_events(self, pluginid, group, track, column, fromtime, length, skip)

  def humanize_events(self, pluginid, group, track, column, fromtime, length, deviation):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_humanize_events(self, pluginid, group, track, column, fromtime, length, deviation)

  def shuffle_events(self, pluginid, group, track, column, fromtime, length):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_shuffle_events(self, pluginid, group, track, column, fromtime, length)

  def interpolate_events(self, pluginid, group, track, column, fromtime, length, skip):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_interpolate_events(self, pluginid, group, track, column, fromtime, length, skip)

  def gradiate_events(self, pluginid, group, track, column, fromtime, length, skip):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_gradiate_events(self, pluginid, group, track, column, fromtime, length, skip)

  def smooth_events(self, pluginid, group, track, column, fromtime, length, strength):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_smooth_events(self, pluginid, group, track, column, fromtime, length, strength)

  def reverse_events(self, pluginid, group, track, column, fromtime, length):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_reverse_events(self, pluginid, group, track, column, fromtime, length)

  def compact_events(self, pluginid, group, track, column, fromtime, length, factor):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_compact_events(self, pluginid, group, track, column, fromtime, length, factor)

  def expand_events(self, pluginid, group, track, column, fromtime, length, factor):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_expand_events(self, pluginid, group, track, column, fromtime, length, factor)

  def thin_events(self, pluginid, group, track, column, fromtime, length, major):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_thin_events(self, pluginid, group, track, column, fromtime, length, major)

  def repeat_events(self, pluginid, group, track, column, fromtime, length, major):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_repeat_events(self, pluginid, group, track, column, fromtime, length, major)

  def echo_events(self, pluginid, group, track, column, fromtime, length, major):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_echo_events(self, pluginid, group, track, column, fromtime, length, major)

  def unique_events(self, pluginid, group, track, column, fromtime, length):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_unique_events(self, pluginid, group, track, column, fromtime, length)

  def scale_events(self, pluginid, group, track, column, fromtime, length, min1, max1, min2, max2):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_scale_events(self, pluginid, group, track, column, fromtime, length, min1, max1, min2, max2)

  def fade_events(self, pluginid, group, track, column, fromtime, length, fromvalue, tovalue):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_fade_events(self, pluginid, group, track, column, fromtime, length, fromvalue, tovalue)

  def curvemap_events(self, pluginid, group, track, column, fromtime, length, mode):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_curvemap_events(self, pluginid, group, track, column, fromtime, length, mode)

  def invert_events(self, pluginid, group, track, column, fromtime, length):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_invert_events(self, pluginid, group, track, column, fromtime, length)

  def rotate_rows_events(self, pluginid, group, track, column, fromtime, length, offset):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_rotate_rows_events(self, pluginid, group, track, column, fromtime, length, offset)

  def rotate_vals_events(self, pluginid, group, track, column, fromtime, length, offset):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_rotate_vals_events(self, pluginid, group, track, column, fromtime, length, offset)

  def rotate_dist_events(self, pluginid, group, track, column, fromtime, length, offset):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_rotate_dist_events(self, pluginid, group, track, column, fromtime, length, offset)

  def set_events(self, pluginid, group, track, column, fromtime, length, value, meta):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_set_events(self, pluginid, group, track, column, fromtime, length, value, meta)

  def replace_events(self, pluginid, group, track, column, fromtime, length, from_value, from_meta, to_value, to_meta):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_replace_events(self, pluginid, group, track, column, fromtime, length, from_value, from_meta, to_value, to_meta)

  def remove_events(self, pluginid, group, track, column, fromtime, length, value, meta):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_remove_events(self, pluginid, group, track, column, fromtime, length, value, meta)

  def notelength_events(self, pluginid, group, track, column, fromtime, length, desired_len, mode, off_value):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_notelength_events(self, pluginid, group, track, column, fromtime, length, desired_len, mode, off_value)

  def volumes_events(self, pluginid, group, track, note_column, vol_column, fromtime, length, mode):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_volumes_events(self, pluginid, group, track, note_column, vol_column, fromtime, length, mode)

  def swap_track_events(self, left_idx, right_idx, fromtime, length):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_swap_track_events(self, left_idx, right_idx, fromtime, length)

  def swap_rows_events(self, pluginid, group, track, column, top_row, bottom_row):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_swap_rows_events(self, pluginid, group, track, column, top_row, bottom_row)

  def invert_chord_events(self, left_idx, right_idx, fromtime, length, direction, mode):
    """Transform."""
    assert self._as_parameter_
    zzub_pattern_invert_chord_events(self, left_idx, right_idx, fromtime, length, direction, mode)

  def move_and_transpose_notes(self, events, numevents, timeshift, pitchshift, mode):
    """Edits notes independently of voice.
    mode 0 = move entire notes, mode 1 = move beginning of notes, mode 2 = move end of notes"""
    assert self._as_parameter_
    zzub_pattern_move_and_transpose_notes(self, events, numevents, timeshift, pitchshift, mode)

  def insert_note(self, plugin, time, note, length):
    """Inserts a voice-independent note."""
    assert self._as_parameter_
    zzub_pattern_insert_note(self, plugin, time, note, length)

  def update_note(self, patternevent, time, note, length):
    """Updates a voice-independent note."""
    assert self._as_parameter_
    zzub_pattern_update_note(self, patternevent, time, note, length)

zzub_pattern_t._wrapper_ = Pattern

class PatternFormat(object):
  """Container class for which columns to refer in a pattern.
  To create a new pattern format, see zzub_player_create_pattern_format().
  @SEEALSO: Pattern, PatternFormatIterator, PatternFormatColumn, PatternFormatColumnIterator"""
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

  def add_column(self, plugin, group, track, column, idx):
    """Adds a column to the pattern format."""
    assert self._as_parameter_
    return PatternFormatColumn._new_from_handle(zzub_pattern_format_add_column(self, plugin, group, track, column, idx))

  def delete_column(self, plugin, group, track, column):
    """Removes a column from the pattern format."""
    assert self._as_parameter_
    zzub_pattern_format_delete_column(self, plugin, group, track, column)

  def get_iterator(self):
    """Returns an iterator over the columns in the pattern format."""
    assert self._as_parameter_
    return PatternFormatColumnIterator._new_from_handle(zzub_pattern_format_get_iterator(self))

  def get_column(self, plugin, group, track, column):
    """Returns the pattern format column for a specified parameter."""
    assert self._as_parameter_
    return PatternFormatColumn._new_from_handle(zzub_pattern_format_get_column(self, plugin, group, track, column))

  def get_name(self):
    """Returns the pattern format name."""
    assert self._as_parameter_
    return zzub_pattern_format_get_name(self)

  def set_name(self, name):
    """Sets the pattern format name."""
    assert self._as_parameter_
    zzub_pattern_format_set_name(self, name)

  def get_id(self):
    """Pattern format ID."""
    assert self._as_parameter_
    return zzub_pattern_format_get_id(self)

  def set_track_name(self, plugin, group, track, name):
    """Sets the name of a pattern format track."""
    assert self._as_parameter_
    zzub_pattern_format_set_track_name(self, plugin, group, track, name)

  def get_track_name(self, plugin, group, track):
    """Returns the name of a pattern format track."""
    assert self._as_parameter_
    return zzub_pattern_format_get_track_name(self, plugin, group, track)

  def set_track_mute(self, plugin, group, track, state):
    """Sets the mute state for a pattern format track. Causes pattern events on this track to be ignored during playback."""
    assert self._as_parameter_
    zzub_pattern_format_set_track_mute(self, plugin, group, track, state)

  def get_track_mute(self, plugin, group, track):
    """Returns the mute state for a pattern format track."""
    assert self._as_parameter_
    return zzub_pattern_format_get_track_mute(self, plugin, group, track)

  def add_column_filter(self, plugin, group, track, column, filterformat):
    """Unused."""
    assert self._as_parameter_
    zzub_pattern_format_add_column_filter(self, plugin, group, track, column, filterformat)

  def remove_column_filter(self, plugin, group, track, column, filterformat):
    """Unused."""
    assert self._as_parameter_
    zzub_pattern_format_remove_column_filter(self, plugin, group, track, column, filterformat)

  def get_column_filters(self, plugin, group, track, column):
    """Unused."""
    assert self._as_parameter_
    return PatternFormatIterator._new_from_handle(zzub_pattern_format_get_column_filters(self, plugin, group, track, column))

  def get_scroller_width(self):
    """Returns the display width of the pattern preview scroller."""
    assert self._as_parameter_
    return zzub_pattern_format_get_scroller_width(self)

  def set_scroller_width(self, width):
    """Sets the display width of the pattern preview scroller."""
    assert self._as_parameter_
    zzub_pattern_format_set_scroller_width(self, width)

  def destroy(self):
    """Destroys the pattern format and releases allocated resources."""
    assert self._as_parameter_
    zzub_pattern_format_destroy(self)

zzub_pattern_format_t._wrapper_ = PatternFormat

class PatternFormatIterator(object):
  """Helper iterator for a range of pattern formats."""
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

  def next(self):
    """Iterates to the next item."""
    assert self._as_parameter_
    zzub_pattern_format_iterator_next(self)

  def valid(self):
    """Returns true if the iterator is valid and it is safe to call current()"""
    assert self._as_parameter_
    return zzub_pattern_format_iterator_valid(self)

  def current(self):
    """Returns the current item."""
    assert self._as_parameter_
    return PatternFormat._new_from_handle(zzub_pattern_format_iterator_current(self))

  def destroy(self):
    """Destroys the iterator."""
    assert self._as_parameter_
    zzub_pattern_format_iterator_destroy(self)

zzub_pattern_format_iterator_t._wrapper_ = PatternFormatIterator

class PatternFormatColumn(object):
  """Represents a column in a pattern."""
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

  def get_plugin(self):
    """Plugin referenced by this column."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_pattern_format_column_get_plugin(self))

  def get_group(self):
    """Plugin group referenced by this column."""
    assert self._as_parameter_
    return zzub_pattern_format_column_get_group(self)

  def get_track(self):
    """Plugin track referenced by this column."""
    assert self._as_parameter_
    return zzub_pattern_format_column_get_track(self)

  def get_column(self):
    """Plugin column referenced by this column."""
    assert self._as_parameter_
    return zzub_pattern_format_column_get_column(self)

  def get_format(self):
    """Returns the pattern format where the column is referenced."""
    assert self._as_parameter_
    return PatternFormat._new_from_handle(zzub_pattern_format_column_get_format(self))

  def get_mode(self):
    """Return column rendering mode.
    mode 0: default, 1: note, 2: switch, 3: byte, 4: word, 5: slider, 6: button, 7: pianoroll, 8: pattern, 9: collapsed, 10: envelope, 11: character, 12: harmonic"""
    assert self._as_parameter_
    return zzub_pattern_format_column_get_mode(self)

  def set_mode(self, mode):
    """Set column rendering mode.
    mode 0: default, 1: note, 2: switch, 3: byte, 4: word, 5: slider, 6: button, 7: pianoroll, 8: pattern, 9: collapsed, 10: envelope, 11: character, 12: harmonic"""
    assert self._as_parameter_
    zzub_pattern_format_column_set_mode(self, mode)

  def get_collapsed(self):
    """Returns true if column is visually collapsed."""
    assert self._as_parameter_
    return zzub_pattern_format_column_get_collapsed(self)

  def set_collapsed(self, is_collapsed):
    """Set whether column is visually collapsed."""
    assert self._as_parameter_
    zzub_pattern_format_column_set_collapsed(self, is_collapsed)

  def get_index(self):
    """Returns the index of the column in the current format."""
    assert self._as_parameter_
    return zzub_pattern_format_column_get_index(self)

  def set_index(self, idx):
    """Sets the index of the column in the current format.
    This is used for sorting purposes when enumerating pattern format columns.
    Columns from the same plugin should be grouped, and a format should contain columns only with indexes between between 0 and the number of columns minus one.
    None of these rules are enforced, index management must be implemented by the host."""
    assert self._as_parameter_
    zzub_pattern_format_column_set_index(self, idx)

zzub_pattern_format_column_t._wrapper_ = PatternFormatColumn

class PatternFormatColumnIterator(object):
  """Helper iterator for a range of pattern format columns."""
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

  def next(self):
    """Iterates to the next item."""
    assert self._as_parameter_
    zzub_pattern_format_column_iterator_next(self)

  def valid(self):
    """Returns true if the iterator is valid and it is safe to call current()"""
    assert self._as_parameter_
    return zzub_pattern_format_column_iterator_valid(self)

  def current(self):
    """Returns the current item."""
    assert self._as_parameter_
    return PatternFormatColumn._new_from_handle(zzub_pattern_format_column_iterator_current(self))

  def destroy(self):
    """Destroys the iterator."""
    assert self._as_parameter_
    zzub_pattern_format_column_iterator_destroy(self)

zzub_pattern_format_column_iterator_t._wrapper_ = PatternFormatColumnIterator

class Parameter(object):
  """Represents a plugin parameter."""
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
    """Returns one of the values in the zzub_parameter_type enumeration."""
    assert self._as_parameter_
    return zzub_parameter_get_type(self)

  def get_name(self):
    """The parameter name."""
    assert self._as_parameter_
    return zzub_parameter_get_name(self)

  def get_description(self):
    """Parameter description."""
    assert self._as_parameter_
    return zzub_parameter_get_description(self)

  def get_value_min(self):
    """Parameter minimum value."""
    assert self._as_parameter_
    return zzub_parameter_get_value_min(self)

  def get_value_max(self):
    """Parameter maximum value."""
    assert self._as_parameter_
    return zzub_parameter_get_value_max(self)

  def get_value_none(self):
    """Parameter novalue, a value to indicate there is no value."""
    assert self._as_parameter_
    return zzub_parameter_get_value_none(self)

  def get_value_default(self):
    """Parameter default value."""
    assert self._as_parameter_
    return zzub_parameter_get_value_default(self)

  def get_flags(self):
    """A parameter flag is combined by zero or more values in the zzub_parameter_flag enumeration."""
    assert self._as_parameter_
    return zzub_parameter_get_flags(self)

zzub_parameter_t._wrapper_ = Parameter

class Attribute(object):
  """Represents a plugin attribute.
  Attributes are similar to plugin parameters, but they cannot be automated."""
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

  def get_name(self):
    """Attribute name."""
    assert self._as_parameter_
    return zzub_attribute_get_name(self)

  def get_value_min(self):
    """Attribute minimum value."""
    assert self._as_parameter_
    return zzub_attribute_get_value_min(self)

  def get_value_max(self):
    """Attribute maximum value."""
    assert self._as_parameter_
    return zzub_attribute_get_value_max(self)

  def get_value_default(self):
    """Attribute default value."""
    assert self._as_parameter_
    return zzub_attribute_get_value_default(self)

zzub_attribute_t._wrapper_ = Attribute

class Pluginloader(object):
  """Represents the description of a plugin type.
  Provides access to static properties of a plugin, such as parameter descriptions, attributes, tracks and channel descriptions."""
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

  def get_name(self):
    """Returns the name of the plugin type."""
    assert self._as_parameter_
    return zzub_pluginloader_get_name(self)

  def get_short_name(self):
    """Returns the short name of the plugin. Used to generate names for new instances of this plugin type."""
    assert self._as_parameter_
    return zzub_pluginloader_get_short_name(self)

  def get_parameter_count(self, group):
    """Returns number of parameters. Group 1 = global parameters, group 2 = track parameters, group 3 = event parameters"""
    assert self._as_parameter_
    return zzub_pluginloader_get_parameter_count(self, group)

  def get_parameter(self, group, index):
    """Returns the parameter for a group and column. See also zzub_plugin_get_parameter() which also returns parameters in group 0."""
    assert self._as_parameter_
    return Parameter._new_from_handle(zzub_pluginloader_get_parameter(self, group, index))

  def get_attribute_count(self):
    """Returns the number of attributes."""
    assert self._as_parameter_
    return zzub_pluginloader_get_attribute_count(self)

  def get_attribute(self, index):
    """Returns an attribute."""
    assert self._as_parameter_
    return Attribute._new_from_handle(zzub_pluginloader_get_attribute(self, index))

  def get_flags(self):
    """Returns the flags for this plugin loader. Combined by zero or more values in the zzub_plugin_flag enumeration."""
    assert self._as_parameter_
    return zzub_pluginloader_get_flags(self)

  def get_uri(self):
    """Returns a string uniquely identifying this plugin type."""
    assert self._as_parameter_
    return zzub_pluginloader_get_uri(self)

  def get_author(self):
    """Returns the name of the plugin author."""
    assert self._as_parameter_
    return zzub_pluginloader_get_author(self)

  def get_instrument_list(self, maxbytes):
    """Returns a list of plugin-defined instruments."""
    assert self._as_parameter_
    result = (c_char*maxbytes)()
    _ret_val = zzub_pluginloader_get_instrument_list(self, result, maxbytes)
    return _ret_val, [v for v in result]

  def get_tracks_min(self):
    """Returns the minimum number of tracks."""
    assert self._as_parameter_
    return zzub_pluginloader_get_tracks_min(self)

  def get_tracks_max(self):
    """Returns the maximum number of tracks."""
    assert self._as_parameter_
    return zzub_pluginloader_get_tracks_max(self)

  def get_stream_format_count(self):
    """Returns the number of supported stream formats. Used with plugins flagged zzub_plugin_flag_stream."""
    assert self._as_parameter_
    return zzub_pluginloader_get_stream_format_count(self)

  def get_stream_format_ext(self, index):
    """Returns a supported stream file format extension stream. Used with plugins flagged zzub_plugin_flag_stream."""
    assert self._as_parameter_
    return zzub_pluginloader_get_stream_format_ext(self, index)

  def get_output_channel_count(self):
    """Returns the maximum number of output audio channels the plugin intends to use."""
    assert self._as_parameter_
    return zzub_pluginloader_get_output_channel_count(self)

  def get_input_channel_count(self):
    """Returns the maximum number of input audio channels the plugin intends to use."""
    assert self._as_parameter_
    return zzub_pluginloader_get_input_channel_count(self)

  def get_plugin_file(self):
    """Returns the full path to the wrapped plugin. Returns blank for built-in plugins."""
    assert self._as_parameter_
    return zzub_pluginloader_get_plugin_file(self)

  def get_plugincollection(self):
    """Returns the plugin collection where this pluginloader belongs"""
    assert self._as_parameter_
    return Plugincollection._new_from_handle(zzub_pluginloader_get_plugincollection(self))

zzub_pluginloader_t._wrapper_ = Pluginloader

class Plugin(object):
  """Plugin methods
  Retreive more details about plugins."""
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

  def destroy(self):
    """Deletes a plugin"""
    assert self._as_parameter_
    return zzub_plugin_destroy(self)

  def load(self, input):
    """Load plugin state."""
    assert self._as_parameter_
    return zzub_plugin_load(self, input)

  def save(self, ouput):
    """Save plugin state."""
    assert self._as_parameter_
    return zzub_plugin_save(self, ouput)

  def set_name(self, name):
    """Renames a plugin. Should fail and return -1 if the name already exists."""
    assert self._as_parameter_
    return zzub_plugin_set_name(self, name)

  def get_name(self):
    """Retreive the name of a plugin."""
    assert self._as_parameter_
    return zzub_plugin_get_name(self)

  def get_id(self):
    """Retreive the unique per-session id of a plugin. See also zzub_player_get_plugin_by_id()."""
    assert self._as_parameter_
    return zzub_plugin_get_id(self)

  def get_position_x(self):
    """Returns the screen position coordinates for the plugin. Values are expected to be in the range -1..1."""
    assert self._as_parameter_
    return zzub_plugin_get_position_x(self)

  def get_position_y(self):
    assert self._as_parameter_
    return zzub_plugin_get_position_y(self)

  def set_position(self, x, y):
    """Sets the plugin screen position. Values are expected to be in the range -1..1."""
    assert self._as_parameter_
    zzub_plugin_set_position(self, x, y)

  def set_position_direct(self, x, y):
    """Sets the plugin screen position. Values are expected to be in the range -1..1. This method is not undoable."""
    assert self._as_parameter_
    zzub_plugin_set_position_direct(self, x, y)

  def get_flags(self):
    """Returns flags for this plugin. Shorthand for using zzub_pluginloader_get_flags(). Combined by zero or more values in the zzub_plugin_flag enumeration."""
    assert self._as_parameter_
    return zzub_plugin_get_flags(self)

  def get_track_count(self, group):
    """Returns the number of tracks."""
    assert self._as_parameter_
    return zzub_plugin_get_track_count(self, group)

  def set_track_count(self, count):
    """Sets the number of tracks. Will call plugin::set_track_count() from the player thread."""
    assert self._as_parameter_
    zzub_plugin_set_track_count(self, count)

  def get_mute(self):
    """Returns 1 if plugin is muted, otherwise 0."""
    assert self._as_parameter_
    return zzub_plugin_get_mute(self)

  def set_mute(self, muted):
    """Set whether plugin is muted. 1 for muted, 0 for normal.
    A muted machine does not produce any sound."""
    assert self._as_parameter_
    zzub_plugin_set_mute(self, muted)

  def get_bypass(self):
    """Returns 1 if plugin is bypassed, otherwise 0."""
    assert self._as_parameter_
    return zzub_plugin_get_bypass(self)

  def set_bypass(self, muted):
    """Set whether plugin is bypassed. 1 for bypass, 0 for normal.
    Bypass causes no processing to occur in the given machine."""
    assert self._as_parameter_
    zzub_plugin_set_bypass(self, muted)

  def get_minimize(self):
    """Returns true if the plugin is (visually) minimized."""
    assert self._as_parameter_
    return zzub_plugin_get_minimize(self)

  def set_minimize(self, minimized):
    """Display the plugin as (visually) minimized."""
    assert self._as_parameter_
    zzub_plugin_set_minimize(self, minimized)

  def configure(self, key, value):
    """Configure a plugin option. this is e.g. used by the recorder plugin to
    specify a file path to write to."""
    assert self._as_parameter_
    zzub_plugin_configure(self, key, value)

  def get_commands(self, maxlen = 1024):
    """Returns a string of \\\\n-separated command strings"""
    assert self._as_parameter_
    commands = (c_char*maxlen)()
    _ret_val = zzub_plugin_get_commands(self, commands, maxlen)
    return _ret_val, commands.value

  def get_sub_commands(self, i, maxlen = 1024):
    """When a plugin command string starts with the char '\\', it has subcommands.
    Unexpectedly, zzub_plugin_get_sub_commands returns a \\\\n-separated string (like get_commands).
    Some plugins need to be ticked before calling get_sub_commands."""
    assert self._as_parameter_
    commands = (c_char*maxlen)()
    _ret_val = zzub_plugin_get_sub_commands(self, i, commands, maxlen)
    return _ret_val, commands.value

  def command(self, i):
    """Invoke a command on the plugin."""
    assert self._as_parameter_
    zzub_plugin_command(self, i)

  def get_pluginloader(self):
    """Returns the pluginloader used to create this plugin."""
    assert self._as_parameter_
    return Pluginloader._new_from_handle(zzub_plugin_get_pluginloader(self))

  def get_midi_output_device_count(self):
    """Returns the number of virtual MIDI devices implemented by the plugin."""
    assert self._as_parameter_
    return zzub_plugin_get_midi_output_device_count(self)

  def get_midi_output_device(self, index):
    """Returns the name of a virtual MIDI device."""
    assert self._as_parameter_
    return zzub_plugin_get_midi_output_device(self, index)

  def get_envelope_count(self):
    """Returns the number of envelopes the plugin supports."""
    assert self._as_parameter_
    return zzub_plugin_get_envelope_count(self)

  def get_envelope_flags(self, index):
    """Returns envelope flags."""
    assert self._as_parameter_
    return zzub_plugin_get_envelope_flags(self, index)

  def get_envelope_name(self, index):
    """Returns envelope name."""
    assert self._as_parameter_
    return zzub_plugin_get_envelope_name(self, index)

  def set_stream_source(self, resource):
    """Sets an audio stream resource identifier.
    E.g a filename, url or wavetable index. Supported by plugins flagged zzub_plugin_flag_stream."""
    assert self._as_parameter_
    zzub_plugin_set_stream_source(self, resource)

  def get_stream_source(self):
    """Returns an audio stream resource identifier.
    E.g a filename, url or wavetable index. Supported by plugins flagged zzub_plugin_flag_stream."""
    assert self._as_parameter_
    return zzub_plugin_get_stream_source(self)

  def set_instrument(self, name):
    """Sets the plugin instrument (d'oh!)"""
    assert self._as_parameter_
    return zzub_plugin_set_instrument(self, name)

  def describe_value(self, group, column, value):
    """Creates a textual description of the given value. ."""
    assert self._as_parameter_
    return zzub_plugin_describe_value(self, group, column, value)

  def get_parameter_value(self, group, track, column):
    """Returns the last written value of the requested parameter."""
    assert self._as_parameter_
    return zzub_plugin_get_parameter_value(self, group, track, column)

  def set_parameter_value(self, group, track, column, value, record):
    """Sets the value of a plugin parameter. The method will wait for the player thread to pick up the modified value and call process_events()."""
    assert self._as_parameter_
    zzub_plugin_set_parameter_value(self, group, track, column, value, record)

  def set_parameter_value_direct(self, group, track, column, value, record):
    """Sets the value of a plugin parameter. Unlike zzub_plugin_set_parameter_value(), this method returns immediately. The parameter will be changed later when the player thread notices the modified value. Is also not undoable."""
    assert self._as_parameter_
    zzub_plugin_set_parameter_value_direct(self, group, track, column, value, record)

  def get_parameter_count(self, group, track):
    """Returns number of parameters in a given plugin group. Shortcut for zzub_pluginloader_get_parameter_count()."""
    assert self._as_parameter_
    return zzub_plugin_get_parameter_count(self, group, track)

  def get_parameter(self, group, track, column):
    """Returns a parameter description."""
    assert self._as_parameter_
    return Parameter._new_from_handle(zzub_plugin_get_parameter(self, group, track, column))

  def get_parameter_interpolator(self, group, track, column):
    """Returns the parameter interpolation mode.
    Mode 0 = absolute, 1 = inertial, 2 = linear"""
    assert self._as_parameter_
    return zzub_plugin_get_parameter_interpolator(self, group, track, column)

  def set_parameter_interpolator(self, group, track, column, mode):
    """Sets the parameter interpolation mode.
    Mode 0 = absolute, 1 = inertial, 2 = linear"""
    assert self._as_parameter_
    zzub_plugin_set_parameter_interpolator(self, group, track, column, mode)

  def get_input_connection_count(self):
    """Returns the number of input connections for given plugin."""
    assert self._as_parameter_
    return zzub_plugin_get_input_connection_count(self)

  def get_input_connection(self, index):
    """Returns an index-based connection object."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_plugin_get_input_connection(self, index))

  def get_input_connection_by_type(self, from_plugin, type):
    """Returns the input connection index for given plugin and connection type."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_plugin_get_input_connection_by_type(self, from_plugin, type))

  def get_output_connection_count(self):
    """Returns the number of output connections for given plugin."""
    assert self._as_parameter_
    return zzub_plugin_get_output_connection_count(self)

  def get_output_connection(self, index):
    """Returns an index-based connection object."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_plugin_get_output_connection(self, index))

  def get_output_connection_by_type(self, from_plugin, type):
    """Returns the output connection index for given plugin and connection type."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_plugin_get_output_connection_by_type(self, from_plugin, type))

  def create_audio_connection(self, from_plugin, first_input, input_count, first_output, output_count):
    """Connects two audio plugins. Feedback loops permitted.
    Use zzub_connection_destroy() to disconnect."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_plugin_create_audio_connection(self, from_plugin, first_input, input_count, first_output, output_count))

  def create_midi_connection(self, from_plugin, midi_device):
    """Connects two MIDI plugins. Feedback loops not permitted.
    Use zzub_connection_destroy() to disconnect."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_plugin_create_midi_connection(self, from_plugin, midi_device))

  def create_event_connection(self, from_plugin):
    """Connects a controller plugin to another plugin.
    Use zzub_connection_add_event_connection_binding() to map parameters. Use zzub_connection_destroy() to disconnect."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_plugin_create_event_connection(self, from_plugin))

  def create_note_connection(self, from_plugin):
    """Connects two plugins with note parameters.
    Use zzub_connection_destroy() to disconnect."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_plugin_create_note_connection(self, from_plugin))

  def get_last_peak(self, channel):
    """Returns a value from the mixer between 0..1 for right/left to indicate current peak levels."""
    assert self._as_parameter_
    return zzub_plugin_get_last_peak(self, channel)

  def get_last_cpu_load(self):
    """Returns a value from the mixer between 0..1 for a current estimated CPU load."""
    assert self._as_parameter_
    return zzub_plugin_get_last_cpu_load(self)

  def get_last_midi_result(self):
    """Returns non-zero if the plugin recently outputted MIDI."""
    assert self._as_parameter_
    return zzub_plugin_get_last_midi_result(self)

  def get_last_audio_result(self):
    """Returns non-zero if the plugin recently outputted audio."""
    assert self._as_parameter_
    return zzub_plugin_get_last_audio_result(self)

  def tick(self, immediate):
    """Process changed parameters. immediate == true is only allowed on a plugin after create_plugin() and before the following barrier(). When immediate==false, the plugin will be processed after barrier()."""
    assert self._as_parameter_
    zzub_plugin_tick(self, immediate)

  def get_attribute_value(self, index):
    """Retreives a plugin attribute value. Refer to zzub_pluginloader_t for attribute counts and descriptions."""
    assert self._as_parameter_
    return zzub_plugin_get_attribute_value(self, index)

  def set_attribute_value(self, index, value):
    """Sets a plugin attribute value. Refer to zzub_pluginloader_t for attribute counts and descriptions."""
    assert self._as_parameter_
    zzub_plugin_set_attribute_value(self, index, value)

  def play_midi_note(self, note, prevNote, velocity):
    """Plays a note."""
    assert self._as_parameter_
    zzub_plugin_play_midi_note(self, note, prevNote, velocity)

  def set_timesource(self, timesource, group, track):
    """Sets the current time source for this plugin."""
    assert self._as_parameter_
    zzub_plugin_set_timesource(self, timesource, group, track)

  def get_timesource_plugin(self):
    """Returns the current time source plugin for this plugin."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_plugin_get_timesource_plugin(self))

  def get_timesource_group(self):
    """Returns the current time source plugin parameter group for this plugin."""
    assert self._as_parameter_
    return zzub_plugin_get_timesource_group(self)

  def get_timesource_track(self):
    """Returns the current time source plugin parameter track for this plugin."""
    assert self._as_parameter_
    return zzub_plugin_get_timesource_track(self)

  def get_output_channel_count(self):
    """Returns the actual number of output audio channels the plugin intends to use."""
    assert self._as_parameter_
    return zzub_plugin_get_output_channel_count(self)

  def get_input_channel_count(self):
    """Returns the actual number of input audio channels the plugin intends to use."""
    assert self._as_parameter_
    return zzub_plugin_get_input_channel_count(self)

  def get_output_channel_name(self, index):
    """Returns the designated name of this output channel."""
    assert self._as_parameter_
    return zzub_plugin_get_output_channel_name(self, index)

  def get_input_channel_name(self, index):
    """Returns the designated name of this input channel."""
    assert self._as_parameter_
    return zzub_plugin_get_input_channel_name(self, index)

  def get_encoder_digest(self, type, numsamples):
    """Request out-of-graph float data from a plugin. Used by e.g a visualizer plugin to return raw visualizer data to the host for rendering.
    When buffers is NULL, the chunk size is returned."""
    assert self._as_parameter_
    buffers = (c_float*2*numsamples)()
    _ret_val = zzub_plugin_get_encoder_digest(self, type, buffers, numsamples)
    return _ret_val, [v for v in buffers]

  def get_connection(self):
    """Returns the associated connection object on plugins with with the is_connection flag."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_plugin_get_connection(self))

  def has_embedded_gui(self):
    """Returns true if the plugin implements an embeddable user interface."""
    assert self._as_parameter_
    return zzub_plugin_has_embedded_gui(self)

  def create_embedded_gui(self, hwnd):
    """Attaches an embedded user interface to the provided parent window handle.
    Returns false if the plugin does not support embeddable user interface.
    This method exists as a common way to implement plugin-specific content in parameter views.
    The implementation depends on the plugin wrapper and the host operating system."""
    assert self._as_parameter_
    return zzub_plugin_create_embedded_gui(self, hwnd)

  def resize_embedded_gui(self, hwnd):
    """Request resize of the embedded user interface.
    The plugin receives a suggested target size in the width and height parameters, but can choose to resize itself to any size and return the final width and height in the respective output parameters. These could differ from the target size when the user interface has a minimum or fixed size in any direction."""
    assert self._as_parameter_
    width = (c_int)()
    height = (c_int)()
    zzub_plugin_resize_embedded_gui(self, hwnd, width, height)
    return width.value, height.value

  def set_latency(self, samplecount):
    """Sets the plugins latency compensation in number of samples. -1 = let plugin decide"""
    assert self._as_parameter_
    zzub_plugin_set_latency(self, samplecount)

  def get_latency(self):
    """Returns the plugins latency compensation in samples. If the return value is -1, the plugin decides the latency. Use get_latency_actual() to retrieve the actual plugin latency compensation."""
    assert self._as_parameter_
    return zzub_plugin_get_latency(self)

  def get_latency_actual(self):
    """Returns the actual plugins latency compensation in samples."""
    assert self._as_parameter_
    return zzub_plugin_get_latency_actual(self)

  def get_plugin_group(self):
    """Returns a group object in which this plugin resides."""
    assert self._as_parameter_
    return PluginGroup._new_from_handle(zzub_plugin_get_plugin_group(self))

  def set_plugin_group(self, group):
    """Moves the plugin to the group. Use group=NULL to set to the root group."""
    assert self._as_parameter_
    zzub_plugin_set_plugin_group(self, group)

zzub_plugin_t._wrapper_ = Plugin

class PluginIterator(object):
  """Helper iterator for a range of plugins."""
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

  def next(self):
    """Iterates to the next item."""
    assert self._as_parameter_
    zzub_plugin_iterator_next(self)

  def valid(self):
    """Returns true if the iterator is valid and it is safe to call current()"""
    assert self._as_parameter_
    return zzub_plugin_iterator_valid(self)

  def current(self):
    """Returns the current item."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_plugin_iterator_current(self))

  def destroy(self):
    """Destroys the iterator."""
    assert self._as_parameter_
    zzub_plugin_iterator_destroy(self)

zzub_plugin_iterator_t._wrapper_ = PluginIterator

class Connection(object):
  """Describes a connection between two plugins.
  There are three kinds of connections: Audio, MIDI and event connections.
  To create a connection, see zzub_player_create_audio_connection(), zzub_player_create_midi_connection() and zzub_player_create_event_connection()."""
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

  def destroy(self):
    """Destroys the connection and frees all resources."""
    assert self._as_parameter_
    zzub_connection_destroy(self)

  def get_type(self):
    """Returns the type of connection. See <see>zzub_connection_type</see> enumeration."""
    assert self._as_parameter_
    return zzub_connection_get_type(self)

  def get_from_plugin(self):
    """Returns the 'from'-plugin. The plugin whose input signal this connection relates to."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_connection_get_from_plugin(self))

  def get_to_plugin(self):
    """Returns the 'to'-plugin. The receiver plugin of signals going through this connection."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_connection_get_to_plugin(self))

  def get_connection_plugin(self):
    """Returns the underlying connection plugin which was created when the connection was made.
    E.g the audio connection plugin has automatable parameters for controlling amp on each channel."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_connection_get_connection_plugin(self))

  def get_first_input(self):
    """Audio connections: Returns the first input channel."""
    assert self._as_parameter_
    return zzub_connection_get_first_input(self)

  def set_first_input(self, value):
    """Audio connections: Set the first input channel."""
    assert self._as_parameter_
    zzub_connection_set_first_input(self, value)

  def get_input_count(self):
    """Audio connections: Returns the number of input channels."""
    assert self._as_parameter_
    return zzub_connection_get_input_count(self)

  def set_input_count(self, value):
    """Audio connections: Set the number of input channels."""
    assert self._as_parameter_
    zzub_connection_set_input_count(self, value)

  def get_first_output(self):
    """Audio connections: Returns the first output channel."""
    assert self._as_parameter_
    return zzub_connection_get_first_output(self)

  def set_first_output(self, value):
    """Audio connections: Set the first output channel."""
    assert self._as_parameter_
    zzub_connection_set_first_output(self, value)

  def get_output_count(self):
    """Audio connections: Returns the number of output channels."""
    assert self._as_parameter_
    return zzub_connection_get_output_count(self)

  def set_output_count(self, value):
    """Audio connections: Set the number of output channels."""
    assert self._as_parameter_
    zzub_connection_set_output_count(self, value)

  def set_midi_device(self, midi_device):
    """MIDI connections: Sets the name of the receiving MIDI device"""
    assert self._as_parameter_
    zzub_connection_set_midi_device(self, midi_device)

  def get_midi_device(self):
    """MIDI connections: Returns the name of the receiving MIDI device"""
    assert self._as_parameter_
    return zzub_connection_get_midi_device(self)

  def get_event_binding_count(self):
    """Event connections: Returns the number of event connection bindings."""
    assert self._as_parameter_
    return zzub_connection_get_event_binding_count(self)

  def add_event_connection_binding(self, sourceparam, targetgroup, targettrack, targetparam):
    """Event connections: Adds an event connection binding."""
    assert self._as_parameter_
    zzub_connection_add_event_connection_binding(self, sourceparam, targetgroup, targettrack, targetparam)

  def remove_event_connection_binding(self, sourceparam, targetgroup, targettrack, targetparam):
    """Event connections: Removes the event connection binding."""
    assert self._as_parameter_
    zzub_connection_remove_event_connection_binding(self, sourceparam, targetgroup, targettrack, targetparam)

  def get_event_binding_iterator(self):
    """Event connections: Returns all current event bindings for an event connection."""
    assert self._as_parameter_
    return ConnectionBindingIterator._new_from_handle(zzub_connection_get_event_binding_iterator(self))

zzub_connection_t._wrapper_ = Connection

class ConnectionBinding(object):
  """Event connection binding between a controller plugin and a parameter on the connected plugin."""
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

  def get_connection(self):
    """Returns the owner connection of this event binding."""
    assert self._as_parameter_
    return Connection._new_from_handle(zzub_connection_binding_get_connection(self))

  def get_source_column(self):
    """Returns the parameter in group 3 on the source plugin being mapped."""
    assert self._as_parameter_
    return zzub_connection_binding_get_source_column(self)

  def get_target_group(self):
    """Returns the target parameter group."""
    assert self._as_parameter_
    return zzub_connection_binding_get_target_group(self)

  def get_target_track(self):
    """Returns the target parameter track."""
    assert self._as_parameter_
    return zzub_connection_binding_get_target_track(self)

  def get_target_column(self):
    """Returns the target parameter column."""
    assert self._as_parameter_
    return zzub_connection_binding_get_target_column(self)

zzub_connection_binding_t._wrapper_ = ConnectionBinding

class ConnectionBindingIterator(object):
  """Collection of event connection bindings."""
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

  def next(self):
    """Iterates to the next item."""
    assert self._as_parameter_
    zzub_connection_binding_iterator_next(self)

  def valid(self):
    """Returns true if the iterator is valid and it is safe to call current()"""
    assert self._as_parameter_
    return zzub_connection_binding_iterator_valid(self)

  def current(self):
    """Returns the current item."""
    assert self._as_parameter_
    return ConnectionBinding._new_from_handle(zzub_connection_binding_iterator_current(self))

  def reset(self):
    """Resets the iterator to the start."""
    assert self._as_parameter_
    zzub_connection_binding_iterator_reset(self)

  def destroy(self):
    """Destroys the iterator."""
    assert self._as_parameter_
    zzub_connection_binding_iterator_destroy(self)

zzub_connection_binding_iterator_t._wrapper_ = ConnectionBindingIterator

class Wave(object):
  """Wave table"""
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

  def get_id(self):
    assert self._as_parameter_
    return zzub_wave_get_id(self)

  def get_index(self):
    assert self._as_parameter_
    return zzub_wave_get_index(self)

  def clear(self):
    assert self._as_parameter_
    return zzub_wave_clear(self)

  def get_name(self):
    assert self._as_parameter_
    return zzub_wave_get_name(self)

  def set_name(self, name):
    assert self._as_parameter_
    zzub_wave_set_name(self, name)

  def get_path(self):
    assert self._as_parameter_
    return zzub_wave_get_path(self)

  def set_path(self, path):
    assert self._as_parameter_
    zzub_wave_set_path(self, path)

  def get_flags(self):
    assert self._as_parameter_
    return zzub_wave_get_flags(self)

  def set_flags(self, flags):
    assert self._as_parameter_
    zzub_wave_set_flags(self, flags)

  def get_volume(self):
    assert self._as_parameter_
    return zzub_wave_get_volume(self)

  def set_volume(self, volume):
    assert self._as_parameter_
    zzub_wave_set_volume(self, volume)

  def get_envelope_count(self):
    assert self._as_parameter_
    return zzub_wave_get_envelope_count(self)

  def set_envelope_count(self, count):
    assert self._as_parameter_
    zzub_wave_set_envelope_count(self, count)

  def get_envelope(self, index):
    assert self._as_parameter_
    return Envelope._new_from_handle(zzub_wave_get_envelope(self, index))

  def set_envelope(self, index, env):
    assert self._as_parameter_
    zzub_wave_set_envelope(self, index, env)

  def get_level_count(self):
    assert self._as_parameter_
    return zzub_wave_get_level_count(self)

  def get_level(self, index):
    assert self._as_parameter_
    return Wavelevel._new_from_handle(zzub_wave_get_level(self, index))

  def add_level(self):
    assert self._as_parameter_
    return Wavelevel._new_from_handle(zzub_wave_add_level(self))

  def remove_level(self, level):
    assert self._as_parameter_
    zzub_wave_remove_level(self, level)

zzub_wave_t._wrapper_ = Wave

class Wavelevel(object):
  """Wavelevel"""
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

  def get_id(self):
    assert self._as_parameter_
    return zzub_wavelevel_get_id(self)

  def get_wave(self):
    assert self._as_parameter_
    return Wave._new_from_handle(zzub_wavelevel_get_wave(self))

  def clear(self):
    assert self._as_parameter_
    return zzub_wavelevel_clear(self)

  def get_sample_count(self):
    assert self._as_parameter_
    return zzub_wavelevel_get_sample_count(self)

  def set_sample_count(self, count):
    assert self._as_parameter_
    zzub_wavelevel_set_sample_count(self, count)

  def get_root_note(self):
    assert self._as_parameter_
    return zzub_wavelevel_get_root_note(self)

  def set_root_note(self, note):
    assert self._as_parameter_
    zzub_wavelevel_set_root_note(self, note)

  def get_samples_per_second(self):
    assert self._as_parameter_
    return zzub_wavelevel_get_samples_per_second(self)

  def set_samples_per_second(self, sps):
    assert self._as_parameter_
    zzub_wavelevel_set_samples_per_second(self, sps)

  def get_loop_start(self):
    assert self._as_parameter_
    return zzub_wavelevel_get_loop_start(self)

  def set_loop_start(self, pos):
    assert self._as_parameter_
    zzub_wavelevel_set_loop_start(self, pos)

  def get_loop_end(self):
    assert self._as_parameter_
    return zzub_wavelevel_get_loop_end(self)

  def set_loop_end(self, pos):
    assert self._as_parameter_
    zzub_wavelevel_set_loop_end(self, pos)

  def get_format(self):
    assert self._as_parameter_
    return zzub_wavelevel_get_format(self)

  def set_format(self, format):
    assert self._as_parameter_
    zzub_wavelevel_set_format(self, format)

  def load_wav(self, offset, clear, datastream):
    """Loads a .WAV from a file or memory stream."""
    assert self._as_parameter_
    return zzub_wavelevel_load_wav(self, offset, clear, datastream)

  def save_wav(self, datastream):
    """Saves the entire wavelevel to a .WAV on file or memory."""
    assert self._as_parameter_
    return zzub_wavelevel_save_wav(self, datastream)

  def save_wav_range(self, datastream, start, numsamples):
    """Saves a range of the wavelevel to a .WAV on file or in memory."""
    assert self._as_parameter_
    return zzub_wavelevel_save_wav_range(self, datastream, start, numsamples)

  def insert_sample_range(self, start, buffer, channels, format, numsamples):
    assert self._as_parameter_
    zzub_wavelevel_insert_sample_range(self, start, buffer, channels, format, numsamples)

  def remove_sample_range(self, start, numsamples):
    assert self._as_parameter_
    zzub_wavelevel_remove_sample_range(self, start, numsamples)

  def replace_sample_range(self, start, buffer, channels, format, numsamples):
    assert self._as_parameter_
    zzub_wavelevel_replace_sample_range(self, start, buffer, channels, format, numsamples)

  def get_samples_digest(self, channel, start, end, digestsize):
    assert self._as_parameter_
    mindigest = (c_float*digestsize)()
    maxdigest = (c_float*digestsize)()
    ampdigest = (c_float*digestsize)()
    zzub_wavelevel_get_samples_digest(self, channel, start, end, mindigest, maxdigest, ampdigest, digestsize)
    return [v for v in mindigest], [v for v in maxdigest], [v for v in ampdigest]

  def get_slices(self):
    assert self._as_parameter_
    slicecount = (c_int)()
    slices = (c_int*slicecount)()
    zzub_wavelevel_get_slices(self, slicecount, slices)
    return slicecount.value, [v for v in slices]

  def set_slices(self, slicecount, slices):
    assert self._as_parameter_
    zzub_wavelevel_set_slices(self, slicecount, slices)

  def process_sample_range_offline(self, start, numsamples, plugin):
    """Applies an offline plugin effect to the wave range. Requires the plugin_flag_is_offline."""
    assert self._as_parameter_
    zzub_wavelevel_process_sample_range_offline(self, start, numsamples, plugin)

zzub_wavelevel_t._wrapper_ = Wavelevel

class Envelope(object):
  """Envelopes"""
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

  def get_attack(self):
    assert self._as_parameter_
    return zzub_envelope_get_attack(self)

  def get_decay(self):
    assert self._as_parameter_
    return zzub_envelope_get_decay(self)

  def get_sustain(self):
    assert self._as_parameter_
    return zzub_envelope_get_sustain(self)

  def get_release(self):
    assert self._as_parameter_
    return zzub_envelope_get_release(self)

  def set_attack(self, attack):
    assert self._as_parameter_
    zzub_envelope_set_attack(self, attack)

  def set_decay(self, decay):
    assert self._as_parameter_
    zzub_envelope_set_decay(self, decay)

  def set_sustain(self, sustain):
    assert self._as_parameter_
    zzub_envelope_set_sustain(self, sustain)

  def set_release(self, release):
    assert self._as_parameter_
    zzub_envelope_set_release(self, release)

  def get_subdivision(self):
    assert self._as_parameter_
    return zzub_envelope_get_subdivision(self)

  def set_subdivision(self, subdiv):
    assert self._as_parameter_
    zzub_envelope_set_subdivision(self, subdiv)

  def get_flags(self):
    assert self._as_parameter_
    return zzub_envelope_get_flags(self)

  def set_flags(self, flags):
    assert self._as_parameter_
    zzub_envelope_set_flags(self, flags)

  def is_enabled(self):
    assert self._as_parameter_
    return zzub_envelope_is_enabled(self)

  def enable(self, enable):
    assert self._as_parameter_
    zzub_envelope_enable(self, enable)

  def get_point_count(self):
    assert self._as_parameter_
    return zzub_envelope_get_point_count(self)

  def get_point(self, index):
    assert self._as_parameter_
    x = (c_ushort)()
    y = (c_ushort)()
    flags = (c_byte)()
    zzub_envelope_get_point(self, index, x, y, flags)
    return x.value, y.value, flags.value

  def set_point(self, index, x, y, flags):
    assert self._as_parameter_
    zzub_envelope_set_point(self, index, x, y, flags)

  def insert_point(self, index):
    assert self._as_parameter_
    zzub_envelope_insert_point(self, index)

  def delete_point(self, index):
    assert self._as_parameter_
    zzub_envelope_delete_point(self, index)

zzub_envelope_t._wrapper_ = Envelope

class Mixer(object):
  """Mixer Methods - provides access to the mixer in native plugins"""
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

zzub_mixer_t._wrapper_ = Mixer

class ValidationErrorIterator(object):
  """Contains validation warnings and errors.
  Validation errors are created when plugin incompabilities are detected.
  E.g if an older plugin was used to save a song, or was in the plugin cache."""
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

  def next(self):
    """Iterates to the next item."""
    assert self._as_parameter_
    zzub_validation_error_iterator_next(self)

  def valid(self):
    """Returns true if the iterator is valid and it is safe to call current()"""
    assert self._as_parameter_
    return zzub_validation_error_iterator_valid(self)

  def current(self):
    """Returns the current item."""
    assert self._as_parameter_
    return ValidationError._new_from_handle(zzub_validation_error_iterator_current(self))

  def reset(self):
    """Resets the iterator to the start."""
    assert self._as_parameter_
    zzub_validation_error_iterator_reset(self)

  def destroy(self):
    """Destroys the iterator."""
    assert self._as_parameter_
    zzub_validation_error_iterator_destroy(self)

zzub_validation_error_iterator_t._wrapper_ = ValidationErrorIterator

class ValidationError(object):
  """Used by song importers for details about compatibility problems with plugins and parameters during load."""
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
    """Returns a value from the ValidationErrorType enum."""
    assert self._as_parameter_
    return zzub_validation_error_get_type(self)

  def get_group(self):
    """Returns the parameter group.
    Used for parameter validation errors, otherwise undefined."""
    assert self._as_parameter_
    return zzub_validation_error_get_group(self)

  def get_column(self):
    """Returns the parameter column.
    Used for parameter validation errors, otherwise undefined."""
    assert self._as_parameter_
    return zzub_validation_error_get_column(self)

  def get_found_value(self):
    """Returns the original parameter value loaded from a song or the cache.
    Used for parameter validation errors, otherwise undefined."""
    assert self._as_parameter_
    return zzub_validation_error_get_found_value(self)

  def get_expected_value(self):
    """Returns the expected parameter value as reported by the plugin itself.
    Used for parameter validation errors, otherwise undefined."""
    assert self._as_parameter_
    return zzub_validation_error_get_expected_value(self)

  def get_parameter_name(self):
    """Returns the original name of the parameter as loaded from a song or the cache.
    Used for parameter validation errors, otherwise undefined."""
    assert self._as_parameter_
    return zzub_validation_error_get_parameter_name(self)

  def get_plugin_name(self):
    """Returns the original name of the affected plugin as loaded from a song or the cache."""
    assert self._as_parameter_
    return zzub_validation_error_get_plugin_name(self)

  def get_pluginloader(self):
    """Returns the affected plugin loader. Could be a dummy.
    The returned pluginloader should not be used to create new plugin instances, but is rather intended for informational purposes. Such as displaying a load report to the user or inline song fixing."""
    assert self._as_parameter_
    return Pluginloader._new_from_handle(zzub_validation_error_get_pluginloader(self))

zzub_validation_error_t._wrapper_ = ValidationError

class Player(object):
  """Player Methods"""
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
  def create(hostpath, userpath, temppath):
    """Creates a new player instance and specifies optional directories for plugins and data.
    hostpath: the root directory from where to scan for plugins, location of buzz2zzub.ini. Defaults to the same directory as the host executable.
    userpath: directory to save plugin enumeration cache files. Defaults to %APPDATA%\Armstrong on Windows and ~/.armstrong on POSIX systems.
    temppath: used for wave data in the undo buffer and temporary files during armz import. Defaults to %TEMP% or /tmp."""
    return Player._new_from_handle(zzub_player_create(hostpath, userpath, temppath))

  def destroy(self):
    """Destroys the player instance and all its resources."""
    assert self._as_parameter_
    zzub_player_destroy(self)

  def initialize(self, samplesPerSecond):
    """Inititializes the player.
    Enumerates plugins, initializes the mixer component, MIDI driver and the default storage document.
    After creating the player instance, but before initialize:
      - Set up and initialize the audio driver
      - Configure specific plugincollections, f.ex set custom VST directories
      - Add event handlers to process plugin enumeration events"""
    assert self._as_parameter_
    return zzub_player_initialize(self, samplesPerSecond)

  def remote_connect(self, host, port):
    """Connects to an armserve server."""
    assert self._as_parameter_
    return zzub_player_remote_connect(self, host, port)

  def remote_disconnect(self):
    """Disconnects from an active armserve connection."""
    assert self._as_parameter_
    zzub_player_remote_disconnect(self)

  def remote_open(self, project, password):
    """Loads a project from the connected armserve server.
    Multiple clients can connect to the same project and keep their editors in sync."""
    assert self._as_parameter_
    return zzub_player_remote_open(self, project, password)

  def remote_create(self, project, password):
    """Saves the current project on the connected armserve server.
    Fails if the password is wrong or project name already exists."""
    assert self._as_parameter_
    return zzub_player_remote_create(self, project, password)

  def remote_delete(self, project, password):
    """Deletes a project from the connected armserve server."""
    assert self._as_parameter_
    return zzub_player_remote_delete(self, project, password)

  def get_remote_client_count(self):
    """Returns the number of clients on the connected armserve server."""
    assert self._as_parameter_
    return zzub_player_get_remote_client_count(self)

  def is_remote_connected(self):
    """Returns true when the armserve server connection is active."""
    assert self._as_parameter_
    return zzub_player_is_remote_connected(self)

  def load_armz(self, fileName, mode, plugingroup):
    """Load an ARMZ project from disk.
    The filename can be a waveless uncompressed database, or a zipped .armz.
    mode=0: clear+load  mode=1: import, optionally into a plugin group"""
    assert self._as_parameter_
    return zzub_player_load_armz(self, fileName, mode, plugingroup)

  def save_armz(self, fileName, plugins, plugincount, plugingroup):
    """Saves current project to disk.
    If the filename has extension .armdb it is saved as waveless database.
    Unless the plugins array is empty, only data associated with the specified plugins is saved.
    If a plugins array is specified and a plugin group is given, plugins will be moved relative to the group parameter.
    Load warnings and error messages can be retreived with get_validation_errors()."""
    assert self._as_parameter_
    return zzub_player_save_armz(self, fileName, plugins, plugincount, plugingroup)

  def load_bmx(self, datastream, flags, x, y):
    """Imports a BMX from memory or file.
    Load warnings and error messages can be retreived with get_validation_errors()."""
    assert self._as_parameter_
    return zzub_player_load_bmx(self, datastream, flags, x, y)

  def load_module(self, fileName):
    """Imports an oldschool tracker module from disk. Supports MOD, IT, S3M.
    Imported songs use the built-in modplug plugin."""
    assert self._as_parameter_
    return zzub_player_load_module(self, fileName)

  def get_validation_errors(self):
    """Returns an iterator for all plugin validation errors.
    The caller must destroy the iterator."""
    assert self._as_parameter_
    return ValidationErrorIterator._new_from_handle(zzub_player_get_validation_errors(self))

  def get_state(self):
    """Returns one of the values in the state enumeration."""
    assert self._as_parameter_
    return zzub_player_get_state(self)

  def set_state(self, state, stoprow):
    """Sets the player state. Takes one of the values in the state enumeration as parameter."""
    assert self._as_parameter_
    zzub_player_set_state(self, state, stoprow)

  def get_pluginloader_count(self):
    """Returns number of plugin loaders."""
    assert self._as_parameter_
    return zzub_player_get_pluginloader_count(self)

  def get_pluginloader(self, index):
    """Returns a zzub_pluginloader_t handle by index."""
    assert self._as_parameter_
    return Pluginloader._new_from_handle(zzub_player_get_pluginloader(self, index))

  def get_pluginloader_by_name(self, name):
    """Finds a zzub_pluginloader_t handle by uri."""
    assert self._as_parameter_
    return Pluginloader._new_from_handle(zzub_player_get_pluginloader_by_name(self, name))

  def get_plugin_count(self):
    """Returns number of plugins in the current song."""
    assert self._as_parameter_
    return zzub_player_get_plugin_count(self)

  @staticmethod
  def add_midimapping(plugin, group, track, param, channel, controller):
    return Midimapping._new_from_handle(zzub_player_add_midimapping(plugin, group, track, param, channel, controller))

  @staticmethod
  def remove_midimapping(plugin, group, track, param):
    return zzub_player_remove_midimapping(plugin, group, track, param)

  def get_plugin_by_name(self, name):
    """Returns the plugin object given the plugins name."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_player_get_plugin_by_name(self, name))

  def get_plugin_by_id(self, id):
    """Returns the plugin object given the plugin id. See also zzub_plugin_get_id()."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_player_get_plugin_by_id(self, id))

  def get_plugin(self, index):
    """Returns the plugin object given the plugins index in the graph."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_player_get_plugin(self, index))

  def get_plugin_iterator(self):
    """Returns an iterator for all plugins. Faster than get_plugin_count()/get_plugin()."""
    assert self._as_parameter_
    return PluginIterator._new_from_handle(zzub_player_get_plugin_iterator(self))

  def get_pattern_iterator(self):
    """Returns an iterator for all patterns. Faster than get_pattern_count()/get_pattern()."""
    assert self._as_parameter_
    return PatternIterator._new_from_handle(zzub_player_get_pattern_iterator(self))

  def get_pattern_by_id(self, id):
    """Returns a pattern by its id."""
    assert self._as_parameter_
    return Pattern._new_from_handle(zzub_player_get_pattern_by_id(self, id))

  def get_pattern_by_index(self, index):
    """Returns a pattern by its index, oldest first."""
    assert self._as_parameter_
    return Pattern._new_from_handle(zzub_player_get_pattern_by_index(self, index))

  def get_pattern_event_by_id(self, id):
    """Returns a pattern event by its id."""
    assert self._as_parameter_
    return PatternEvent._new_from_handle(zzub_player_get_pattern_event_by_id(self, id))

  def get_new_pattern_name(self, format, description):
    """Returns a suggested name for a new pattern"""
    assert self._as_parameter_
    return zzub_player_get_new_pattern_name(self, format, description)

  def get_pattern_count(self):
    """Returns total count of patterns."""
    assert self._as_parameter_
    return zzub_player_get_pattern_count(self)

  def get_pattern_by_name(self, name):
    """Returns the index of the pattern with the given name"""
    assert self._as_parameter_
    return Pattern._new_from_handle(zzub_player_get_pattern_by_name(self, name))

  def get_pattern_format_count(self):
    """Returns the number of pattern formats."""
    assert self._as_parameter_
    return zzub_player_get_pattern_format_count(self)

  def get_new_pattern_format_name(self, description):
    """Generates a name for a new pattern format."""
    assert self._as_parameter_
    return zzub_player_get_new_pattern_format_name(self, description)

  def get_pattern_format_by_name(self, name):
    """Returns a pattern format by name."""
    assert self._as_parameter_
    return PatternFormat._new_from_handle(zzub_player_get_pattern_format_by_name(self, name))

  def get_pattern_format_by_index(self, index):
    """Returns a pattern format by its index."""
    assert self._as_parameter_
    return PatternFormat._new_from_handle(zzub_player_get_pattern_format_by_index(self, index))

  def get_pattern_format_by_id(self, id):
    """Returns a pattern format by its id."""
    assert self._as_parameter_
    return PatternFormat._new_from_handle(zzub_player_get_pattern_format_by_id(self, id))

  def get_pattern_format_iterator(self):
    """Returns an iterator for all pattern formats. Faster than get_pattern_format_count()/get_pattern_format_by_index()."""
    assert self._as_parameter_
    return PatternFormatIterator._new_from_handle(zzub_player_get_pattern_format_iterator(self))

  def work_stereo(self, inbuffers, inchannels, outchannels, numsamples):
    """For silent processing. ."""
    assert self._as_parameter_
    outbuffers = (c_float*outchannels*numsamples)()
    zzub_player_work_stereo(self, inbuffers, outbuffers, inchannels, outchannels, numsamples)
    return [v for v in outbuffers]

  def clear(self):
    """Resets everything and clears the current project."""
    assert self._as_parameter_
    zzub_player_clear(self)

  def get_wave_count(self):
    """Returns the number of waves in the wavetable (hardcoded to 200)."""
    assert self._as_parameter_
    return zzub_player_get_wave_count(self)

  def get_wave(self, index):
    """Returns the wave by index in the range 0-199."""
    assert self._as_parameter_
    return Wave._new_from_handle(zzub_player_get_wave(self, index))

  def add_callback(self, callback, tag):
    """Adds a function that receives events."""
    assert self._as_parameter_
    # wrap callback
    cb_callback = callback._from_function(callback)
    zzub_player_add_callback(self, cb_callback._function_handle, tag)

  def remove_callback(self, callback, tag):
    """Removes a function that receives events."""
    assert self._as_parameter_
    # wrap callback
    cb_callback = callback._from_function(callback)
    zzub_player_remove_callback(self, cb_callback._function_handle, tag)

  def handle_events(self):
    """Process player events.
    Intended to be called by the host in a timer or on idle processing to receive events about parameter changes etc."""
    assert self._as_parameter_
    zzub_player_handle_events(self)

  def get_midimapping(self, index):
    """Returns the MIDI mapping by index."""
    assert self._as_parameter_
    return Midimapping._new_from_handle(zzub_player_get_midimapping(self, index))

  def get_midimapping_count(self):
    """Returns the number of MIDI mappings."""
    assert self._as_parameter_
    return zzub_player_get_midimapping_count(self)

  def get_automation(self):
    """Returns true if parameters are currently being recorded into patterns."""
    assert self._as_parameter_
    return zzub_player_get_automation(self)

  def set_automation(self, enable):
    """Set to true to begin recording parameter changes into patterns.
    Recording will reset on song stop."""
    assert self._as_parameter_
    zzub_player_set_automation(self, enable)

  def get_midi_transport(self):
    """Returns true if the engine handles play/stop/seek MIDI messages on any open MIDI input devices."""
    assert self._as_parameter_
    return zzub_player_get_midi_transport(self)

  def set_midi_transport(self, enable):
    """Set to true to handle MIDI play/stop/seek messages on any open MIDI input devices."""
    assert self._as_parameter_
    zzub_player_set_midi_transport(self, enable)

  def get_infotext(self):
    """Returns the song comment."""
    assert self._as_parameter_
    return zzub_player_get_infotext(self)

  def set_infotext(self, text):
    """Sets the song comment."""
    assert self._as_parameter_
    zzub_player_set_infotext(self, text)

  def set_midi_plugin(self, plugin):
    """Sets the plugin to receive MIDI data if the plugin's internal MIDI channel is set to the special channel 17 (\"Play if selected\")."""
    assert self._as_parameter_
    zzub_player_set_midi_plugin(self, plugin)

  def get_midi_plugin(self):
    """Returns the current MIDI plugin."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_player_get_midi_plugin(self))

  def get_midi_lock(self):
    """Returns true if the mixer has midi lock (will ignore set_midi_plugin()-calls)"""
    assert self._as_parameter_
    return zzub_player_get_midi_lock(self)

  def set_midi_lock(self, state):
    """Enables/disables midi locks, which will ignore/enable set_midi_plugin-calls"""
    assert self._as_parameter_
    zzub_player_set_midi_lock(self, state)

  def get_new_plugin_name(self, uri):
    """Generates a new plugin name that can be used in a call to create_plugin()."""
    assert self._as_parameter_
    return zzub_player_get_new_plugin_name(self, uri)

  def reset_keyjazz(self):
    """Resets all internal keyjazz buffers."""
    assert self._as_parameter_
    zzub_player_reset_keyjazz(self)

  def create_plugin(self, input, dataSize, instanceName, loader, group = None):
    """Create a new plugin
    If there was a problem during plugin creation, such as having an older version in the cache, warnings and error messages can be retreived with get_validation_errors()."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_player_create_plugin(self, input, dataSize, instanceName, loader, group))

  def create_pattern(self, format, description, rows):
    """Create a new pattern"""
    assert self._as_parameter_
    return Pattern._new_from_handle(zzub_player_create_pattern(self, format, description, rows))

  def clone_pattern(self, pattern, description):
    """Clone a pattern"""
    assert self._as_parameter_
    return Pattern._new_from_handle(zzub_player_clone_pattern(self, pattern, description))

  def create_pattern_format(self, description):
    """Create a new pattern description"""
    assert self._as_parameter_
    return PatternFormat._new_from_handle(zzub_player_create_pattern_format(self, description))

  def clone_pattern_format(self, format, description):
    """Clone a pattern format"""
    assert self._as_parameter_
    return PatternFormat._new_from_handle(zzub_player_clone_pattern_format(self, format, description))

  def undo(self):
    """Rolls back all editing operations one step. Each step is defined with a call to zzub_player_history_commit()."""
    assert self._as_parameter_
    zzub_player_undo(self)

  def redo(self):
    """Redoes all editing operations since last call to zzub_player_history_commit()."""
    assert self._as_parameter_
    zzub_player_redo(self)

  def history_enable(self, state):
    """Enable/disable undo/redo recording. Returns the previous state."""
    assert self._as_parameter_
    return zzub_player_history_enable(self, state)

  def history_begin(self, userdata):
    """Sets user data for events sent by the engine. User data is valid until it is cleared in history_commit()."""
    assert self._as_parameter_
    zzub_player_history_begin(self, userdata)

  def history_commit(self, redo_id, undo_id, description):
    """Commits the last operations to the mixer and undo buffer and marks a new undo step."""
    assert self._as_parameter_
    zzub_player_history_commit(self, redo_id, undo_id, description)

  def history_get_uncomitted_operations(self):
    """Returns the count of uncomitted operations."""
    assert self._as_parameter_
    return zzub_player_history_get_uncomitted_operations(self)

  def history_reset(self):
    """Clears the undo buffer and frees all associated resources."""
    assert self._as_parameter_
    zzub_player_history_reset(self)

  def history_get_size(self):
    """Returns the size of the undo buffer."""
    assert self._as_parameter_
    return zzub_player_history_get_size(self)

  def history_get_position(self):
    """Returns the current position in the undo buffer."""
    assert self._as_parameter_
    return zzub_player_history_get_position(self)

  def history_get_description(self, position):
    """Returns the description of an operation in the undo buffer."""
    assert self._as_parameter_
    return zzub_player_history_get_description(self, position)

  def set_host_info(self, id, version, host_ptr):
    """Set versioned, host-specific data. Plugins can retreive a pointer to this information with _host->get_host_info().
    Use and/or dependence on the host's version is regarded as bad practise and should not be used in new code."""
    assert self._as_parameter_
    zzub_player_set_host_info(self, id, version, host_ptr)

  def invoke_event(self, data, immediate):
    """Invokes event handlers for an internal event."""
    assert self._as_parameter_
    return zzub_player_invoke_event(self, data, immediate)

  def set_order_length(self, length):
    """Sets the order list length."""
    assert self._as_parameter_
    zzub_player_set_order_length(self, length)

  def get_order_length(self):
    """Returns the order list length."""
    assert self._as_parameter_
    return zzub_player_get_order_length(self)

  def set_order_pattern(self, index, pattern):
    """Sets a pattern in the order list by index."""
    assert self._as_parameter_
    zzub_player_set_order_pattern(self, index, pattern)

  def get_order_pattern(self, index):
    """Returns a pattern from the order list by index."""
    assert self._as_parameter_
    return Pattern._new_from_handle(zzub_player_get_order_pattern(self, index))

  def get_order_iterator(self):
    """Returns an iterator over all patterns in the orderlist. The iterator might contain NULLs for blank entries in the order list."""
    assert self._as_parameter_
    return PatternIterator._new_from_handle(zzub_player_get_order_iterator(self))

  def get_order_loop_start(self):
    """Returns the order list loop start index."""
    assert self._as_parameter_
    return zzub_player_get_order_loop_start(self)

  def set_order_loop_start(self, pos):
    """Sets the order list loop start index."""
    assert self._as_parameter_
    zzub_player_set_order_loop_start(self, pos)

  def get_order_loop_end(self):
    """Returns the order list loop end index."""
    assert self._as_parameter_
    return zzub_player_get_order_loop_end(self)

  def set_order_loop_end(self, pos):
    """Sets the order list loop end index."""
    assert self._as_parameter_
    zzub_player_set_order_loop_end(self, pos)

  def get_order_loop_enabled(self):
    """Returns true if order list looping is enabled."""
    assert self._as_parameter_
    return zzub_player_get_order_loop_enabled(self)

  def set_order_loop_enabled(self, enable):
    """Sets whether order list looping is enabled."""
    assert self._as_parameter_
    zzub_player_set_order_loop_enabled(self, enable)

  def set_queue_order_index(self, pos):
    """Sets the order list queue index.
    The order list will skip to the queue index when the current pattern ends."""
    assert self._as_parameter_
    zzub_player_set_queue_order_index(self, pos)

  def get_queue_order_index(self):
    """Returns the current queue index."""
    assert self._as_parameter_
    return zzub_player_get_queue_order_index(self)

  def timeshift_order(self, fromindex, timeshift):
    """Timeshifts the orderlist play position. Does not actually change the order list contents.
    Adds a cookie on the undo buffer which adjusts the currently playing order list index accordingly, handling any race issues in the audio thread."""
    assert self._as_parameter_
    zzub_player_timeshift_order(self, fromindex, timeshift)

  def get_position_order(self):
    """Returns the current order list position."""
    assert self._as_parameter_
    return zzub_player_get_position_order(self)

  def get_position_row(self):
    """Returns the current pattern position in the current order list pattern."""
    assert self._as_parameter_
    return zzub_player_get_position_row(self)

  def get_position_samples(self):
    """Returns the number of samples played since playback started."""
    assert self._as_parameter_
    return zzub_player_get_position_samples(self)

  def set_position(self, orderindex, tick):
    """Sets the song position at an order list index and a pattern row."""
    assert self._as_parameter_
    zzub_player_set_position(self, orderindex, tick)

  def adjust_position_order(self, orderindex):
    """Redundant?"""
    assert self._as_parameter_
    zzub_player_adjust_position_order(self, orderindex)

  def get_bpm(self):
    """Returns the global sequence BPM."""
    assert self._as_parameter_
    return zzub_player_get_bpm(self)

  def get_tpb(self):
    """Returns the global sequence TBP."""
    assert self._as_parameter_
    return zzub_player_get_tpb(self)

  def get_swing(self):
    """Returns the global sequence swing amount."""
    assert self._as_parameter_
    return zzub_player_get_swing(self)

  def set_bpm(self, bpm):
    """Sets the global sequence BPM."""
    assert self._as_parameter_
    zzub_player_set_bpm(self, bpm)

  def set_tpb(self, tpb):
    """Sets the global sequence TPB."""
    assert self._as_parameter_
    zzub_player_set_tpb(self, tpb)

  def set_swing(self, swing):
    """Sets the global sequence swing."""
    assert self._as_parameter_
    zzub_player_set_swing(self, swing)

  def set_swing_ticks(self, swing_ticks):
    """Sets the global sequence ticks per swing."""
    assert self._as_parameter_
    zzub_player_set_swing_ticks(self, swing_ticks)

  def get_timesource_count(self):
    """Returns the number of available time sources in the project."""
    assert self._as_parameter_
    return zzub_player_get_timesource_count(self)

  def get_timesource_plugin(self, index):
    """Returns the plugin for a time source."""
    assert self._as_parameter_
    return Plugin._new_from_handle(zzub_player_get_timesource_plugin(self, index))

  def get_timesource_group(self, index):
    """Returns the plugin parameter group for a time source."""
    assert self._as_parameter_
    return zzub_player_get_timesource_group(self, index)

  def get_timesource_track(self, index):
    """Returns the plugin parameter track for a time source."""
    assert self._as_parameter_
    return zzub_player_get_timesource_track(self, index)

  def play_pattern(self, pat, row, stoprow):
    """Plays a single pattern.
    This merely starts playback for a single pattern and does not affect the global song state or counters."""
    assert self._as_parameter_
    zzub_player_play_pattern(self, pat, row, stoprow)

  def get_machineview_offset_x(self):
    """Rendering hint; all plugins should be offset by this amount when rendered."""
    assert self._as_parameter_
    return zzub_player_get_machineview_offset_x(self)

  def get_machineview_offset_y(self):
    assert self._as_parameter_
    return zzub_player_get_machineview_offset_y(self)

  def set_machineview_offset(self, x, y):
    """Rendering hint; sets how much all plugins should be offset in the machine view."""
    assert self._as_parameter_
    zzub_player_set_machineview_offset(self, x, y)

  def get_thread_count(self):
    """Returns the current number of mixer worker threads. Returns 1 by default."""
    assert self._as_parameter_
    return zzub_player_get_thread_count(self)

  def set_thread_count(self, threads):
    """Changes the number of mixer worker threads. The threads argument must be greater than or equal to 1.
    When the thread count is 1, the engine processes one-by-one plugin sequentially.
    When the thread count is > 1, the engine distributes plugin processing across the requested number of threads."""
    assert self._as_parameter_
    zzub_player_set_thread_count(self, threads)

  def get_peaks(self):
    """Returns max peak values for all channels from the most recent frame sent to the audio driver."""
    assert self._as_parameter_
    peaks = (c_float*peakcount)()
    peakcount = (c_int)()
    zzub_player_get_peaks(self, peaks, peakcount)
    return [v for v in peaks], peakcount.value

  def get_waveimporter_count(self):
    """Returns the number of registrered wave importers."""
    assert self._as_parameter_
    return zzub_player_get_waveimporter_count(self)

  def get_waveimporter_format_ext_count(self, index):
    """Returns the number of supported file extensions on a wave importer."""
    assert self._as_parameter_
    return zzub_player_get_waveimporter_format_ext_count(self, index)

  def get_waveimporter_format_ext(self, index, extindex):
    """Returns a supported file extension."""
    assert self._as_parameter_
    return zzub_player_get_waveimporter_format_ext(self, index, extindex)

  def get_waveimporter_format_is_container(self, index):
    """Returns true if the importers get_instrument_count() can return more than 1."""
    assert self._as_parameter_
    return zzub_player_get_waveimporter_format_is_container(self, index)

  def get_waveimporter_format_type(self, index):
    """Returns type of importer. 0: single sample, 1: collection of samples, 2: single instrument, 3: collection of instruments"""
    assert self._as_parameter_
    return zzub_player_get_waveimporter_format_type(self, index)

  def create_waveimporter(self, index):
    """Creates a waveimporter instance."""
    assert self._as_parameter_
    return WaveImporter._new_from_handle(zzub_player_create_waveimporter(self, index))

  def create_waveimporter_by_file(self, filename):
    """Convenience function for creating a waveimporter from a filename."""
    assert self._as_parameter_
    return WaveImporter._new_from_handle(zzub_player_create_waveimporter_by_file(self, filename))

  def create_plugin_group(self, parent, name):
    """Creates a new plugin group."""
    assert self._as_parameter_
    return PluginGroup._new_from_handle(zzub_player_create_plugin_group(self, parent, name))

  def get_plugin_group_by_id(self, id):
    """Returns a plugin group based on its identifier."""
    assert self._as_parameter_
    return PluginGroup._new_from_handle(zzub_player_get_plugin_group_by_id(self, id))

  def get_plugin_group_iterator(self, parent):
    """Returns an iterator object for enumerating all plugin groups contained inside a plugin group. Use parent = NULL for the root."""
    assert self._as_parameter_
    return PluginGroupIterator._new_from_handle(zzub_player_get_plugin_group_iterator(self, parent))

zzub_player_t._wrapper_ = Player

class PluginGroup(object):
  """A container for plugins."""
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

  def destroy(self):
    """Deletes and disconnects the plugin group and moves all plugins to the parent group."""
    assert self._as_parameter_
    zzub_plugin_group_destroy(self)

  def get_id(self):
    """Returns an identifier for the plugin group."""
    assert self._as_parameter_
    return zzub_plugin_group_get_id(self)

  def get_name(self):
    """Returns the name of the plugin group."""
    assert self._as_parameter_
    return zzub_plugin_group_get_name(self)

  def set_name(self, name):
    """Sets the name of the plugin group."""
    assert self._as_parameter_
    zzub_plugin_group_set_name(self, name)

  def get_parent(self):
    """Returns the parent plugin group, or NULL if the parent is the root."""
    assert self._as_parameter_
    return PluginGroup._new_from_handle(zzub_plugin_group_get_parent(self))

  def set_parent(self, newparent):
    """Moves the plugin group to another plugin group.
    Pass NULL to move the plugin group to the root."""
    assert self._as_parameter_
    zzub_plugin_group_set_parent(self, newparent)

  def get_position_x(self):
    """Returns the x position of the plugin group."""
    assert self._as_parameter_
    return zzub_plugin_group_get_position_x(self)

  def get_position_y(self):
    """Returns the y position of the plugin group."""
    assert self._as_parameter_
    return zzub_plugin_group_get_position_y(self)

  def set_position(self, x, y):
    """Sets the position of the plugin group."""
    assert self._as_parameter_
    zzub_plugin_group_set_position(self, x, y)

  def get_plugins(self):
    """Returns an iterator for plugins contained directly in this plugin group."""
    assert self._as_parameter_
    return PluginIterator._new_from_handle(zzub_plugin_group_get_plugins(self))

zzub_plugin_group_t._wrapper_ = PluginGroup

class PluginGroupIterator(object):
  """Helper iterator for a range of groups."""
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

  def next(self):
    """Iterates to the next item."""
    assert self._as_parameter_
    zzub_plugin_group_iterator_next(self)

  def valid(self):
    """Returns true if the iterator is valid and it is safe to call current()"""
    assert self._as_parameter_
    return zzub_plugin_group_iterator_valid(self)

  def current(self):
    """Returns the current item."""
    assert self._as_parameter_
    return PluginGroup._new_from_handle(zzub_plugin_group_iterator_current(self))

  def destroy(self):
    """Destroys the iterator."""
    assert self._as_parameter_
    zzub_plugin_group_iterator_destroy(self)

zzub_plugin_group_iterator_t._wrapper_ = PluginGroupIterator

class WaveImporter(object):
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

  def open(self, filename, strm):
    assert self._as_parameter_
    return zzub_wave_importer_open(self, filename, strm)

  def destroy(self):
    assert self._as_parameter_
    zzub_wave_importer_destroy(self)

  def get_instrument_count(self):
    assert self._as_parameter_
    return zzub_wave_importer_get_instrument_count(self)

  def get_instrument_name(self, index):
    assert self._as_parameter_
    return zzub_wave_importer_get_instrument_name(self, index)

  def get_instrument_sample_count(self, index):
    assert self._as_parameter_
    return zzub_wave_importer_get_instrument_sample_count(self, index)

  def get_instrument_sample_info(self, index, sample, namelen):
    assert self._as_parameter_
    name = (c_char*namelen)()
    samplecount = (c_int)()
    channels = (c_int)()
    format = (c_int)()
    samplerate = (c_int)()
    zzub_wave_importer_get_instrument_sample_info(self, index, sample, name, namelen, samplecount, channels, format, samplerate)
    return name.value, samplecount.value, channels.value, format.value, samplerate.value

  def load_instrument(self, index, dest):
    assert self._as_parameter_
    return zzub_wave_importer_load_instrument(self, index, dest)

  def load_instrument_sample(self, index, sample, dest):
    assert self._as_parameter_
    return zzub_wave_importer_load_instrument_sample(self, index, sample, dest)

zzub_wave_importer_t._wrapper_ = WaveImporter

class callback(object):
  _function = None
  _function_handle = None
  _hash = 0

  def __init__(self, function):
    self._function = function
    self._function_handle = zzub_callback_t(self.wrapper_function)
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

  def wrapper_function(self, player, plugin, data, tag):
    self._function(Player._new_from_handle(player), Plugin._new_from_handle(plugin), EventData._new_from_handle(data), tag)

callback._instances_ = {}

