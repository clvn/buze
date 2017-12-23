#pragma once

namespace zzub {
	struct info;
	struct archive;
	struct instream;
	struct outstream;
	struct attribute;
	struct parameter;
	struct plugincollection;
};

namespace armstrong {

namespace frontend {
	struct player;
	struct plugin;
	struct pluginiterator;
	struct pattern;
	struct patterniterator;
	struct patternevent;
	struct patterneventiterator;
	struct patternformat;
	struct patternformatiterator;
	struct patternformatcolumn;
	struct patternformatcolumniterator;
	struct connection;
	struct audiodriver;
	struct midimapping;
	struct wave;
	struct wavelevel;
	struct envelope;
	struct validationerror;
	struct validationerroriterator;
	struct eventconnectionbinding;
	struct eventconnectionbindingiterator;
	struct plugingroup;
	struct plugingroupiterator;
	struct audiodevice;
	struct audiodeviceiterator;
}

}

#define NO_ZZUB_AUDIODRIVER_TYPE
typedef armstrong::frontend::audiodriver zzub_audiodriver_t;

#define NO_ZZUB_DEVICE_INFO_TYPE
typedef armstrong::frontend::audiodevice zzub_device_info_t;

#define NO_ZZUB_DEVICE_INFO_ITERATOR_TYPE
typedef armstrong::frontend::audiodeviceiterator zzub_device_info_iterator_t;

#define NO_ZZUB_PLAYER_TYPE
typedef armstrong::frontend::player zzub_player_t;

#define NO_ZZUB_PLUGIN_TYPE
typedef armstrong::frontend::plugin zzub_plugin_t;

#define NO_ZZUB_PLUGIN_ITERATOR_TYPE
typedef armstrong::frontend::pluginiterator zzub_plugin_iterator_t;

#define NO_ZZUB_CONNECTION_TYPE
typedef armstrong::frontend::connection zzub_connection_t;

#define NO_ZZUB_PATTERN_TYPE
typedef armstrong::frontend::pattern zzub_pattern_t;

#define NO_ZZUB_PATTERN_ITERATOR_TYPE
typedef armstrong::frontend::patterniterator zzub_pattern_iterator_t;

#define NO_ZZUB_PATTERN_EVENT_TYPE
typedef armstrong::frontend::patternevent zzub_pattern_event_t;

#define NO_ZZUB_PATTERN_FORMAT_TYPE
typedef armstrong::frontend::patternformat zzub_pattern_format_t;

#define NO_ZZUB_PATTERN_FORMAT_ITERATOR_TYPE
typedef armstrong::frontend::patternformatiterator zzub_pattern_format_iterator_t;

#define NO_ZZUB_PATTERN_FORMAT_COLUMN_TYPE
typedef armstrong::frontend::patternformatcolumn zzub_pattern_format_column_t;

#define NO_ZZUB_PATTERN_FORMAT_COLUMN_ITERATOR_TYPE
typedef armstrong::frontend::patternformatcolumniterator zzub_pattern_format_column_iterator_t;

#define NO_ZZUB_PATTERN_EVENT_ITERATOR_TYPE
typedef armstrong::frontend::patterneventiterator zzub_pattern_event_iterator_t;

#define NO_ZZUB_PLUGINLOADER_TYPE
typedef zzub::info zzub_pluginloader_t;

#define NO_ZZUB_ARCHIVE_TYPE
typedef zzub::archive zzub_archive_t;

#define NO_ZZUB_INPUT_TYPE
typedef zzub::instream zzub_input_t;

#define NO_ZZUB_OUTPUT_TYPE
typedef zzub::outstream zzub_output_t;

#define NO_ZZUB_ATTRIBUTE_TYPE
typedef zzub::attribute zzub_attribute_t;

#define NO_ZZUB_PARAMETER_TYPE
typedef const zzub::parameter zzub_parameter_t;

#define NO_ZZUB_MIDIMAPPING_TYPE
typedef armstrong::frontend::midimapping zzub_midimapping_t;

#define NO_ZZUB_PLUGINCOLLECTION_TYPE
typedef zzub::plugincollection zzub_plugincollection_t;

#define NO_ZZUB_WAVE_TYPE
typedef armstrong::frontend::wave zzub_wave_t;

#define NO_ZZUB_WAVELEVEL_TYPE
typedef armstrong::frontend::wavelevel zzub_wavelevel_t;

#define NO_ZZUB_ENVELOPE_TYPE
typedef armstrong::frontend::envelope zzub_envelope_t;

#define NO_ZZUB_VALIDATION_ERROR_ITERATOR_TYPE
typedef armstrong::frontend::validationerroriterator zzub_validation_error_iterator_t;

#define NO_ZZUB_VALIDATION_ERROR_TYPE
typedef armstrong::frontend::validationerror zzub_validation_error_t;

#define NO_ZZUB_CONNECTION_BINDING_TYPE
typedef armstrong::frontend::eventconnectionbinding zzub_connection_binding_t;

#define NO_ZZUB_CONNECTION_BINDING_ITERATOR_TYPE
typedef armstrong::frontend::eventconnectionbindingiterator zzub_connection_binding_iterator_t;

#define NO_ZZUB_PLUGIN_GROUP_TYPE
typedef armstrong::frontend::plugingroup zzub_plugin_group_t;

#define NO_ZZUB_PLUGIN_GROUP_ITERATOR_TYPE
typedef armstrong::frontend::plugingroupiterator zzub_plugin_group_iterator_t;
