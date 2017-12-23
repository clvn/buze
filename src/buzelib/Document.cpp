#include <algorithm>
using std::min;
using std::max;

#define NOMINMAX
#include <windows.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <map>
#include <cassert>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <Shlwapi.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <zzub/zzub.h>
#include <buze/View.h>
#include "resource.h"
#include "utils.h"
#include "Configuration.h"

class CApplication;

#define NO_BUZE_APPLICATION_TYPE
typedef CApplication buze_application_t;

#define NO_BUZE_CONFIGURATION_TYPE
typedef CConfiguration buze_configuration_t;

#include <buze/buze.h>
#include "BuzeConfiguration.h"
#include "ConfigurationImpl.h"
#include "Document.h"

using std::replace;
using std::cerr;

// ---------------------------------------------------------------------------------------------------------------
// GLOBAL HELPERS
// ---------------------------------------------------------------------------------------------------------------

std::string rewriteBuzzWrapperName(std::string const& uri) {
	if (uri.find("@zzub.org/buzz2zzub/") != 0) return uri;
	std::string out_uri = uri.substr(20);
	replace(out_uri.begin(), out_uri.end(), '+', ' ');
	return out_uri;
}

// from bmxreader.cpp:
std::string rewriteBuzzWrapperUri(std::string const& fileName) {
	std::string uri = "@zzub.org/buzz2zzub/" + fileName;
	replace(uri.begin(), uri.end(), ' ', '+');
	return uri;
}

int zzub_callback(zzub_player_t* player, zzub_plugin_t* machine, zzub_event_data_t* data, void* tag) {
	buze_document_t* self = (buze_document_t*)tag;
	buze_document_notify_views(self, 0, data->type, (void*)data);
	return -1; // -1 = false, did nothing
}

// ---------------------------------------------------------------------------------------------------------------
// CREATION / DESTRUCTION
// ---------------------------------------------------------------------------------------------------------------

CDocument::CDocument(CApplication* _app, zzub_player_t* _player, CConfiguration* config, ThemeManager* _themes) {
	application = _app;
	player = _player;
	configuration = config;
	themes = _themes;
	lastSaveUndoPosition = 0;
	currentFileName = "Untitled";
	currentExtension = "armz";
	octave = 4;
	streamplayer = 0;
	soloplugin = 0;
	currentPlugin = 0;
	currentPattern = 0;
	currentPatternformat = 0;
	currentConnection = 0;
	currentWave = 0;
	currentWavelevel = 0;
	currentOrderIndex = 0;
	currentOrderPatternRow = 0;
	currentPatternRow = 0;

	enumerateStreamPlugins();
	zzub_player_add_callback(player, &zzub_callback, this);
}

CDocument::~CDocument() {
	zzub_player_history_enable(player, 0);

	deleteStreamPlayer();

	zzub_player_history_commit(player, 0, 0, "");
	zzub_player_history_enable(player, 1);
	zzub_player_remove_callback(player, &zzub_callback, this);
}

// ---------------------------------------------------------------------------------------------------------------
// VIEWS
// ---------------------------------------------------------------------------------------------------------------

void CDocument::updateAllViews(CView* pSender, int lHint, void* pHint) {
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;
	buze_event_data_t* buzeData = (buze_event_data_t*)pHint;
	zzub_connection_t* conn;

	// global events:
	switch (lHint) {
		case buze_event_type_change_pattern_order:
			currentOrderIndex = buzeData->change_pattern_order.index;
			break;
		case buze_event_type_change_pattern_row:
			if (zzub_player_get_order_pattern(player, currentOrderIndex) == currentPattern)
				currentOrderPatternRow = buzeData->change_pattern_row.row;
			currentPatternRow = buzeData->change_pattern_row.row;
			break;
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_post_clear_document:
			currentOrderIndex = 0;
			currentOrderPatternRow = 0;
			currentPatternRow = 0;
			break;
		case buze_event_type_update_properties:
			switch (buzeData->show_properties.type) {
				case buze_property_type_pattern_format:
					currentPatternformat = buzeData->show_properties.pattern_format;
					break;
				case buze_property_type_connection:
					currentConnection = buzeData->show_properties.connection;
					break;
				case buze_property_type_plugin:
					currentPlugin = buzeData->show_properties.plugin;
					break;
				case buze_property_type_pattern:
					currentPattern = buzeData->show_properties.pattern;
					break;
				case buze_property_type_wave:
					currentWave = buzeData->show_properties.wave;
					break;
				case buze_property_type_wave_level:
					currentWavelevel = buzeData->show_properties.wavelevel;
					break;
			}
			break;
		case zzub_event_type_delete_plugin:
			// prevent sending note-offs to a deleted plugin:
			keyjazzRelease(false);
			removePluginData(zzubData->delete_plugin.plugin);
			if (zzubData->delete_plugin.plugin == currentPlugin)
				currentPlugin = 0;
			break;
		case zzub_event_type_delete_pattern:
			if (zzubData->delete_pattern.pattern == currentPattern)
				currentPattern = 0;
			break;
		case zzub_event_type_delete_patternformat:
			if (zzubData->delete_pattern_format.patternformat == currentPatternformat)
				currentPatternformat = 0;
			break;
		case zzub_event_type_delete_connection:
			conn = zzub_plugin_get_input_connection_by_type(zzubData->delete_connection.to_plugin, zzubData->delete_connection.from_plugin, zzubData->delete_connection.type);
			if (conn == currentConnection)
				currentConnection = 0;
			break;
		case zzub_event_type_delete_wave:
			if (zzubData->delete_wave.wave == currentWave)
				currentWave = 0;
			break;
		case zzub_event_type_delete_wavelevel:
			if (zzubData->delete_wavelevel.wavelevel == currentWavelevel)
				currentWavelevel = 0;
			break;
		//case zzub_event_type_delete_patternformatcolumn:
			// remove deleted pattern positions
			// moved to the pattern editor viewinfo
			//removePatternPosition(zzubData->delete_pattern_format_column.patternformatcolumn);
			//break;
		case zzub_event_type_insert_connection:
			setMachineNonSong(zzubData->insert_connection.connection_plugin, TRUE);
			break;
		case zzub_event_type_user_alert:
			if (zzubData->alert.type == zzub_alert_type_pattern_recursion) {
				MessageBox(GetForegroundWindow(), "Too much pattern recursion - stopped!", "Armstrong Alert", MB_OK);
			}
			break;
	}

	// iterate the event handlers in a manner where the event handlers can modify the array of event handlers:
	std::set<CEventHandler*> handlers;
	bool done = false;
	while (!done) {
		done = true;
		for (unsigned i = 0; i < views.size(); ++i) {
			std::set<CEventHandler*>::iterator it = handlers.find(views[i]);
			if (it == handlers.end()) {
				handlers.insert(views[i]);
				views[i]->OnUpdate(pSender, lHint, pHint);
				done = false;
				break;
			}
		}
	}

}

void CDocument::addView(CEventHandler* view) {
	views.push_back(view);
}

void CDocument::removeView(CEventHandler* view) {
	std::vector<CEventHandler*>::iterator i = std::find(views.begin(), views.end(), view);
	if (i == views.end()) return;
	views.erase(i);
}

// ---------------------------------------------------------------------------------------------------------------
// GRAPH CONTROL
// ---------------------------------------------------------------------------------------------------------------

extern void zzub_audio_connection_set_amp(zzub_plugin_t* connplug, int amp, bool with_undo);

zzub_pattern_format_t* CDocument::createDefaultFormat(zzub_plugin_t* plugin, bool simple) {
	assert(plugin != 0);

	const char* name = zzub_plugin_get_name(plugin);
	zzub_pattern_format_t* patfmt = zzub_player_create_pattern_format(player, name);

	extendPatternFormat(patfmt, plugin, simple);

	return patfmt;
}

namespace {

bool is_simple_parameter(zzub_parameter_t* para) {
	if (zzub_parameter_get_type(para) == zzub_parameter_type_note)
		return true;
	int flags = zzub_parameter_get_flags(para);
	if (flags & zzub_parameter_flag_wavetable_index)
		return true;
	if (flags & zzub_parameter_flag_velocity_index)
		return true;
	return false;
}

// TODO: should be armstrong method!!
int zzub_pattern_format_get_column_count(zzub_pattern_format_t* format) {
	zzub_pattern_format_column_iterator_t* colit = zzub_pattern_format_get_iterator(format);

	int counter = 0;
	while (zzub_pattern_format_column_iterator_valid(colit)) {
		counter++;
		zzub_pattern_format_column_iterator_next(colit);
	}

	zzub_pattern_format_column_iterator_destroy(colit);
	return counter;
}

}

// TODO: hasSimpleFormatParameters(plugin)

void CDocument::extendPatternFormat(zzub_pattern_format_t* format, zzub_plugin_t* plugin, bool simple) {
	assert(plugin != 0);
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(plugin);

	int screenidx = zzub_pattern_format_get_column_count(format);

	// global parameters
	for (int j = 0; j < zzub_pluginloader_get_parameter_count(loader, 1); ++j) {
		zzub_parameter_t* para = zzub_pluginloader_get_parameter(loader, 1, j);
		if (!simple || is_simple_parameter(para)) {
			bool hascol = zzub_pattern_format_get_column(format, plugin, 1, 0, j) != 0;
			if (!hascol) {
				zzub_pattern_format_add_column(format, plugin, 1, 0, j, screenidx);
				++screenidx;
			}
		}
	}

	// track parameters
	for (int i = 0; i < zzub_plugin_get_track_count(plugin, 2); ++i) {
		for (int j = 0; j < zzub_pluginloader_get_parameter_count(loader, 2); ++j) {
			zzub_parameter_t* para = zzub_pluginloader_get_parameter(loader, 2, j);
			if (!simple || is_simple_parameter(para)) {
				bool hascol = zzub_pattern_format_get_column(format, plugin, 2, i, j) != 0;
				if (!hascol) {
					zzub_pattern_format_add_column(format, plugin, 2, i, j, screenidx);
					++screenidx;
				}
			}
		}
	}
}

bool CDocument::createDefaultDocument() {
	if (configuration->getAdvancedDefaultDocument())
		return createDefaultDocumentAdvanced();
	else
		return createDefaultDocumentSimple();
}

bool CDocument::createDefaultDocumentSimple() {

	if (configuration->getMachinesMinimized()) {

		{	/// todo: this is a hack and should not be here.
			zzub_plugin_t* masterplug = zzub_player_get_plugin_by_name(player, "Master");
			if (masterplug != 0) {
				zzub_plugin_set_minimize(masterplug, true);
			}
		}
	}

	zzub_pattern_format_t* patfmt = zzub_player_create_pattern_format(player, "Default");

	char const* description = 0;
	if (configuration->getPatternNamingMode() == 1)
		description = zzub_pattern_format_get_name(patfmt);
	
	zzub_pattern_t* pat = zzub_player_create_pattern(player, patfmt, description, 16);

	zzub_pattern_set_loop_enabled(pat, 0);

	zzub_player_set_order_length(player, 1);
	zzub_player_set_order_pattern(player, 0, pat);

	return true;
}

bool CDocument::createDefaultDocumentAdvanced() {
	std::string sequri = "@zzub.org/sequence/sequence";
	std::string paturi = "@zzub.org/sequence/pattern";
	zzub_pluginloader_t* seqloader = zzub_player_get_pluginloader_by_name(player, sequri.c_str());
	assert(seqloader);
	zzub_pluginloader_t* patloader = zzub_player_get_pluginloader_by_name(player, paturi.c_str());
	assert(patloader);

	const char* newname = zzub_player_get_new_plugin_name(player, paturi.c_str());
	zzub_plugin_t* patplug = zzub_player_create_plugin(player, 0, 0, newname, patloader, 0);

	newname = zzub_player_get_new_plugin_name(player, sequri.c_str());
	zzub_plugin_t* seqplug = zzub_player_create_plugin(player, 0, 0, newname, seqloader, 0);

	if (configuration->getMachinesMinimized()) {
		zzub_plugin_set_minimize(patplug, true);
		zzub_plugin_set_minimize(seqplug, true);

		{	/// todo: this is a hack and should not be here.
			zzub_plugin_t* masterplug = zzub_player_get_plugin_by_name(player, "Master");
			if (masterplug != 0) {
				zzub_plugin_set_minimize(masterplug, true);
			}
		}
	}

	zzub_plugin_set_position(patplug, 0.2f, 0.2f);
	zzub_plugin_set_position(seqplug, 0.2f, -0.2f);

	zzub_pattern_format_t* patfmt = zzub_player_create_pattern_format(player, newname);

	int screenidx = 0;
	zzub_pattern_format_add_column(patfmt, patplug, 2, 0, 0, screenidx);
	++screenidx;

	char const* description = 0;
	if (configuration->getPatternNamingMode() == 1) {
		description = zzub_pattern_format_get_name(patfmt);
	}
	zzub_pattern_t* pat = zzub_player_create_pattern(player, patfmt, description, 8 * 1024);

	zzub_pattern_set_display_resolution(pat, 16);
	zzub_pattern_set_loop_enabled(pat, 1);
	//	zzub_sequence_t* seq = zzub_player_create_sequence(player, seqplug, pat);

	zzub_player_set_order_length(player, 1);
	zzub_player_set_order_pattern(player, 0, pat);

	//zzub_sequence_t* seq = zzub_player_create_sequence(player, plugin, type);
	//zzub_sequence_move(seq, index);
	return true;
}

void CDocument::setMachineSolo(zzub_plugin_t* m, BOOL state) {
	if (state != FALSE)
		soloplugin = m; else
		soloplugin = 0;

	for (int i = 0; i < zzub_player_get_plugin_count(player); ++i) {
		zzub_plugin_t* plugin = zzub_player_get_plugin(player, i);
		if (getMachineNonSong(plugin)) continue; // dont mute preview plugins when soloing other machines

		int flags = zzub_plugin_get_flags(plugin);
		bool is_generator = ((flags & zzub_plugin_flag_has_audio_output) != 0) && ((flags & zzub_plugin_flag_has_audio_input) == 0);
		if (state != FALSE) {
			if (is_generator && plugin != m) {
				zzub_plugin_set_mute(plugin, 1);
			} else
				if (is_generator && plugin == m) {
					zzub_plugin_set_mute(plugin, 0);
				}
		} else
			if (is_generator)
				zzub_plugin_set_mute(plugin, 0);

	}
}

BOOL CDocument::getMachineSolo(zzub_plugin_t* plugin) {
	return FALSE; // todo
}

// ---------------------------------------------------------------------------------------------------------------
// STREAM
// ---------------------------------------------------------------------------------------------------------------

void CDocument::enumerateStreamPlugins() {
	for (int i = 0; i < zzub_player_get_pluginloader_count(player); ++i) {
		zzub_pluginloader_t* info = zzub_player_get_pluginloader(player, i);
		if (zzub_pluginloader_get_flags(info) & zzub_plugin_flag_stream) {
			for (int j = 0; j < zzub_pluginloader_get_stream_format_count(info); ++j) {
				std::string ext = zzub_pluginloader_get_stream_format_ext(info, j);
				std::string uri = zzub_pluginloader_get_uri(info);
				std::map<std::string, std::string>::iterator k = stream_ext_uri_mappings.find(ext);
				if (k != stream_ext_uri_mappings.end()) {
					std::cerr << "Found another mapping for " << ext << "! Skipping " << uri << std::endl;
					continue;
				}
				stream_ext_uri_mappings[ext] = uri;
			}
		}
	}
}

std::string CDocument::getStreamPluginUriForFile(std::string const& fileName) {
	size_t ls = fileName.find_last_of('.');
	std::string ext = "";
	if (ls != -1)
		ext = fileName.substr(ls+1);

	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

	std::map<std::string, std::string>::iterator k = stream_ext_uri_mappings.find(ext);
	if (k == stream_ext_uri_mappings.end()) return "";

	return k->second;
}

void CDocument::createStreamPlayer(std::string const& uri) {
	// create a stream player plugin with no_undo flag to keep it out of the undo buffer
	assert(streamplayer == 0);

	zzub_pluginloader_t* loader = zzub_player_get_pluginloader_by_name(player, uri.c_str());
	if (loader == 0) {
		streamplayer = 0;
		printf("Cant can't find streamplayer plugin loader.\n");
		return ;
	}

	std::vector<char> bytes;
	streamplayer = zzub_player_create_plugin(player, 0, 0, "_PreviewPlugin", loader, 0);

	if (streamplayer == 0) {
		printf("Cant can't create streamplayer plugin instance.\n");
		return ;
	}

	zzub_plugin_create_audio_connection(zzub_player_get_plugin(player, 0), streamplayer, -1, -1, -1, -1);

	setMachineNonSong(streamplayer, TRUE);
}

void CDocument::deleteStreamPlayer() {
	// remove the stream player plugin (if it exists)
	if (streamplayer == 0) return ;

	zzub_plugin_destroy(streamplayer);

	streamplayer = 0;
}

void CDocument::playStream(int note, unsigned int offset, unsigned int length, std::string const& pluginUri, std::string const& dataUrl) {

	zzub_player_history_enable(player, 0);

	if (streamplayer != 0) {
		zzub_pluginloader_t* streaminfo = zzub_plugin_get_pluginloader(streamplayer);
		std::string prevuri = zzub_pluginloader_get_uri(streaminfo);
		if (prevuri != pluginUri)
			deleteStreamPlayer();
	}

	if (streamplayer == 0)
		createStreamPlayer(pluginUri);

	if (streamplayer != 0) {
		zzub_plugin_set_stream_source(streamplayer, dataUrl.c_str());
	}

	zzub_player_history_commit(player, 0, 0, "Play Stream");

	if (streamplayer != 0) {
		if (offset != 0) {
			zzub_plugin_set_parameter_value_direct(streamplayer, 1, 0, 1, offset & 0xFFFF, false);
			zzub_plugin_set_parameter_value_direct(streamplayer, 1, 0, 2, offset >> 16, false);
		}
		if (length != 0) {
			zzub_plugin_set_parameter_value_direct(streamplayer, 1, 0, 3, length & 0xFFFF, false);
			zzub_plugin_set_parameter_value_direct(streamplayer, 1, 0, 4, length >> 16, false);
		}
		playMachineNote(streamplayer, note, 0);
	}

	zzub_player_history_enable(player, 1);
}

// ---------------------------------------------------------------------------------------------------------------
// PLUGIN DATA
// ---------------------------------------------------------------------------------------------------------------

PLUGININFO& CDocument::getPluginData(zzub_plugin_t* plugin) {
	assert(plugin != 0);

	std::map<zzub_plugin_t*, PLUGININFO>::iterator i = pluginData.find(plugin);
	if (i != pluginData.end()) return i->second;

	PLUGININFO& info = pluginData[plugin] = PLUGININFO();

	info.nonSongPlugin = false;
	info.paramViewMode = 0;
	return info;
}

void CDocument::removePluginData(zzub_plugin_t* plugin) {
	std::map<zzub_plugin_t*, PLUGININFO>::iterator i = pluginData.find(plugin);
	if (i == pluginData.end()) return ;
	pluginData.erase(i);
}

// ---------------------------------------------------------------------------------------------------------------
// MACHINE PROPERTIES
// ---------------------------------------------------------------------------------------------------------------

BOOL CDocument::getMachineNonSong(zzub_plugin_t* m) {
	PLUGININFO& pi = getPluginData(m);
	return pi.nonSongPlugin;
}

void CDocument::setMachineNonSong(zzub_plugin_t* m, BOOL state) {
	PLUGININFO& pi = getPluginData(m);
	pi.nonSongPlugin = state != FALSE;
}

std::string CDocument::getMachinePreset(zzub_plugin_t* m) {
	PLUGININFO& pi = getPluginData(m);
	return pi.lastPreset;
}

void CDocument::setMachinePreset(zzub_plugin_t* m, std::string const& preset) {
	PLUGININFO& pi = getPluginData(m);
	pi.lastPreset = preset;
}

// ---------------------------------------------------------------------------------------------------------------
// IMPORTING
// ---------------------------------------------------------------------------------------------------------------

std::string get_parameter_group_name(int group) {
	switch (group) {
		case 0:
			return "Internal parameter";
		case 1:
			return "Global parameter";
		case 2:
			return "Track parameter";
		case 3:
			return "Controller parameter";
		default:
			assert(false);
			return "";
	}
}

std::string get_validation_error_text(zzub_validation_error_t* error) {
	zzub_validation_error_type type = (zzub_validation_error_type)zzub_validation_error_get_type(error);
	int group = zzub_validation_error_get_group(error);
	int column = zzub_validation_error_get_column(error);
	zzub_pluginloader_t* info = zzub_validation_error_get_pluginloader(error);
	std::string plugintype = info != 0 ? zzub_pluginloader_get_name(info) : "";
	std::string pluginname = zzub_validation_error_get_plugin_name(error);

	std::stringstream strm;
	if (!pluginname.empty() || !plugintype.empty()) {
		strm << pluginname;
		if (!pluginname.empty() && !plugintype.empty())
			strm << " (";

		strm << plugintype;
		if (!pluginname.empty() && !plugintype.empty())
			strm << ")";
	} else {
		strm << "<unknown plugin>";
	}

	switch (type) {
		case zzub_validation_error_type_plugin_not_found:
			strm << " not found. Plugin data not imported."; 
			return strm.str();
		case zzub_validation_error_type_plugin_not_found_using_dummy:
			strm << " not found. Replaced by a dummy plugin.";
			return strm.str();
		case zzub_validation_error_type_plugin_validation_failed_using_dummy:
			strm << " validation failed. Replaced by a dummy plugin.";
			return strm.str();
		case zzub_validation_error_type_plugin_inputs_mismatch:
		case zzub_validation_error_type_plugin_outputs_mismatch:
			strm << "IO channel count mismatch.";
			return strm.str();

		case zzub_validation_error_type_parameter_count_mismatch:
			strm << get_parameter_group_name(group) << " count mismatch.";
			return strm.str();
		case zzub_validation_error_type_parameter_flags_mismatch:
			strm << get_parameter_group_name(group) << " " << zzub_validation_error_get_parameter_name(error) << " flags mismatch.";
			return strm.str();
		case zzub_validation_error_type_parameter_type_mismatch:
			strm << get_parameter_group_name(group) << " " << zzub_validation_error_get_parameter_name(error) << " type mismatch.";
			return strm.str();
		case zzub_validation_error_type_parameter_value_default_mismatch:
			strm << get_parameter_group_name(group) << " " << zzub_validation_error_get_parameter_name(error) << " default value mismatch.";
			return strm.str();
		case zzub_validation_error_type_parameter_value_min_mismatch:
			strm << get_parameter_group_name(group) << " " << zzub_validation_error_get_parameter_name(error) << " minimum value mismatch.";
			return strm.str();
		case zzub_validation_error_type_parameter_value_max_mismatch:
			strm << get_parameter_group_name(group) << " " << zzub_validation_error_get_parameter_name(error) << " maximum value mismatch.";
			return strm.str();
		case zzub_validation_error_type_parameter_value_none_mismatch:
			strm << get_parameter_group_name(group) << " " << zzub_validation_error_get_parameter_name(error) << " novalue mismatch.";
			return strm.str();
		default:
			return "Unknown validation error.";
	}
}

void format_error(zzub_player_t* player, std::string* result) {
	
	zzub_validation_error_iterator_t* errors = zzub_player_get_validation_errors(player);
	std::stringstream strm;
	while (zzub_validation_error_iterator_valid(errors)) {
		zzub_validation_error_t* error = zzub_validation_error_iterator_current(errors);
		strm << get_validation_error_text(error) << std::endl;
		zzub_validation_error_iterator_next(errors);
	}
	zzub_validation_error_iterator_destroy(errors);
	*result = strm.str();
}

bool CDocument::importSong(std::string const& filename, int flags, float x, float y, std::string* error_messages) {
	size_t ls = filename.find_last_of('.');
	std::string ext = "";
	if (ls != std::string::npos) ext = filename.substr(ls+1);

	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

	zzub_validation_error_iterator_t* errors = 0;
	int errorcount = 0;
	bool success = false;
	if (ext == "mod" || ext == "xm" || ext == "it" || ext == "s3m") {
		return zzub_player_load_module(player, filename.c_str()) != -1 ? true : false;
	} else 
	if (ext == "bmx" || ext == "bmw") {
		zzub_input_t* inf = zzub_input_open_file(filename.c_str());
		if (inf) {
			success = zzub_player_load_bmx(player, inf, flags, x, y) != -1 ? true : false;
			zzub_input_destroy(inf);
			format_error(player, error_messages);
			return success;
		} else {
			*error_messages = "Cannot open file.";
			return false;
		}
	} else {
		success = zzub_player_load_armz(player, filename.c_str(), (flags&8)?1:0, 0) != -1 ? true : false;
		format_error(player, error_messages);
		return success;
	}
}

#if 0

bool CDocument::importSong(std::vector<char>& bytes, int flags, float x, float y, std::string* error_messages) {
	zzub_archive_t* arc = zzub_archive_create_memory();

	zzub_output_t* outf = zzub_archive_get_output(arc, "");
	zzub_output_write(outf, &bytes.front(), bytes.size());

	zzub_input_t* inf = zzub_archive_get_input(arc, "");

	std::vector<char> error_bytes(1024);
	bool result = zzub_player_load_bmx(player, inf, &error_bytes.front(), error_bytes.size(), flags, x, y) != -1 ? true : false;
    
	zzub_archive_destroy(arc);

	*error_messages = &error_bytes.front();
	return result;
/*	SongImport* redo = new SongImport();
	redo->init(bytes);
	if(!allowundo) {
		redo->document = this;
		redo->commit();
		redo->updateUI();
		delete redo;

		for (int j = 0; j < zzub_player_get_plugin_count(player); ++j) {
			// set the longest pattern length for sequencer highlight

			size_t maxlen = 0;
			for(size_t k = 0; k < zzub_plugin_get_pattern_count(player, i); ++k) {
				if (zzub_plugin_get_pattern_length(player, i, k) > maxlen) {
					maxlen = zzub_plugin_get_pattern_length(player, i, k);
				}
			}
			setMachineLongestPattern(j, maxlen);
		}

		return true;
	}

	int numMachines = zzub_player_get_plugin_count(player);
	GroupAction* undo = new GroupAction();

	// we need to load the song before we add the delete commands
	bool status = undoManager.insertAndCommit(redo, undo);

	for (int j = 0; j < zzub_player_get_plugin_count(player); ++j) {
		//plugin_vertex& machine = player->get_plugin(j);
		if(j >= numMachines) {
			MachineDelete* del = new MachineDelete();
			del->init(j);
			undo->actions.push_back(del);
		}

		// set the longest pattern length for sequencer highlight

		size_t maxlen = 0;
		for(size_t k = 0; k < zzub_plugin_get_pattern_count(j); ++k) {
			if (zzub_plugin_get_pattern_length(player, j, k) > maxlen) {
				maxlen = zzub_plugin_get_pattern_length(player, j, k);
			}
		}
		setMachineLongestPattern(j, maxlen);
	}

	return status;
	*/
	return false;
}

#endif

// a little helper function to load and name a sample; this gets used in about three separate places
// returns true on success; cries and returns false otherwise
/*bool CDocument::loadSampleByFileName(const char *szFileName, int curWave) {
	bool ret = true;

	zzub_wave_t* wave = zzub_player_get_wave(player, curWave);
	zzub_wave_clear(wave);
	zzub_wavelevel_t* wavelevel = zzub_wave_add_level(wave);

	if (zzub_wavelevel_load_sample(wavelevel, 0, TRUE, szFileName, 0) > 0) {
		std::string name = szFileName;
		
		size_t ls = name.find_last_of('\\');
		if (ls != std::string::npos) name = name.substr(ls + 1);
		
		ls = name.find_last_of('.');
		if (ls != std::string::npos) name = name.substr(0, ls);
		
		zzub_wave_set_name(wave, name.c_str());
		zzub_wave_set_path(wave, szFileName);
		zzub_wave_set_volume(wave, 1.0f);

		curWave++;
	} else {
		std::stringstream strm;
		strm << "Cannot open " << szFileName;
		MessageBox(0, strm.str().c_str(), "Error", MB_OK|MB_ICONWARNING);
		ret = false;
	}
	
	return ret;
}
*/
void CDocument::setCurrentFile(std::string fileName) {
	int ls = fileName.find_last_of("\\/");
	if (ls != -1) {
		currentDirectory = fileName.substr(0, ls);
		fileName = fileName.substr(ls+1);
	} else
		currentDirectory = "";

	int ld = fileName.find_last_of('.');
	if (ld != -1) {
		currentExtension = fileName.substr(ld+1);
		std::transform(currentExtension.begin(), currentExtension.end(), currentExtension.begin(), tolower);
		fileName = fileName.substr(0, ld);
	} else
		currentExtension = "";

	currentFileName = fileName;
}

// ---------------------------------------------------------------------------------------------------------------
// KEYJAZZ / MACHINE NOTES
// ---------------------------------------------------------------------------------------------------------------

void CDocument::playMachineNote(zzub_plugin_t* m, int note, int prevNote) {
	zzub_plugin_play_midi_note(m, note, prevNote, 0);
}

void CDocument::keyjazzKeyDown(zzub_plugin_t* plugin, int key, int note) {
	KEYJAZZNOTE kjn = { plugin, key, note };
	keyjazznotes.push_back(kjn);
}

KEYJAZZNOTE CDocument::keyjazzKeyUp(int key) {
	KEYJAZZNOTE kjn = { 0, -1, -1 };
	std::vector<KEYJAZZNOTE>::iterator i;
	for (i = keyjazznotes.begin(); i != keyjazznotes.end(); ++i) {
		if (i->key == key) {
			kjn = *i;
			keyjazznotes.erase(i);
			break;
		}
	}
	return kjn;
}

void CDocument::keyjazzRelease(bool sendNoteOffs) {
	if (sendNoteOffs) {
		std::vector<KEYJAZZNOTE>::iterator i;
		for (i = keyjazznotes.begin(); i != keyjazznotes.end(); ++i) {
			playMachineNote(i->plugin, zzub_note_value_off, i->note);
		}
	}
	keyjazznotes.clear();
}

// ---------------------------------------------------------------------------------------------------------------
// MACHINE HELP
// ---------------------------------------------------------------------------------------------------------------

std::string hexencode(const std::string& input) {
	std::ostringstream ssOut;
	ssOut << std::setbase(16);
	for(std::string::const_iterator i = input.begin(); i != input.end(); ++i) {
		if(isalnum((unsigned char)*i))
			ssOut << *i;
		else
			ssOut << '%' << std::setw(2) << std::setfill('0') << ((unsigned int)(unsigned char)*i);
	}
	return ssOut.str();
}

std::string findHelpExtension(std::string basePath, std::string const& name) {
	struct stat st;
	std::string temp;
	#define tryHelpFile(ext) temp = basePath + name + ext; if (stat(temp.c_str(), &st) == 0) return temp
	tryHelpFile(".html");
	tryHelpFile(".htm");
	tryHelpFile(".txt");
	tryHelpFile(".mht");
	tryHelpFile(".rtf");

	return "";
}

std::string CDocument::getMachineHelpFile(zzub_pluginloader_t* info) {
	std::string uri = zzub_pluginloader_get_uri(info);
	std::string pluginName = "";//player->getBuzzName(uri);
	std::string fileName;
	if (pluginName == "") {
		fileName = hexencode(uri);
		pluginName = uri;
	} else
		fileName = pluginName;

	std::string temp;
	#define tryHelpDirectory(dir) temp = findHelpExtension(dir, fileName); if (temp != "") return temp
	tryHelpDirectory("Gear\\Native\\");
	tryHelpDirectory("Gear\\Effects\\");
	tryHelpDirectory("Gear\\Generators\\");

	return "";
}

bool CDocument::isDirty() {
	return zzub_player_history_get_position(player) != lastSaveUndoPosition;
}


void CDocument::clearSong() {
	zzub_player_set_state(player, zzub_player_state_muted, -1);

	// close all song owned windows
	/*for (std::vector<DynamicDockTabViewManager<CMainFrame, CView>*>::iterator i = views.begin(); i != views.end(); ++i) {
		if (!(*i)->serializable)
			(*i)->closeAll(true);
	}
	this->customView.closeAll(true);*/

	updateAllViews(0, buze_event_type_update_pre_clear_document, 0);
	deleteStreamPlayer();

	zzub_player_clear(player);

	pluginData.clear();

	setCurrentFile("Untitled.armz");
	//setWindowTitle("");

	lastSaveUndoPosition = 0;

	zzub_player_set_state(player, zzub_player_state_stopped, -1);
	zzub_player_set_position(player, 0, 0);

	updateAllViews(0, buze_event_type_update_post_clear_document, 0);
}

bool CDocument::saveFile(std::string const& filename, bool withWaves) {
	char tempPath[MAX_PATH];
	char tempFile[MAX_PATH];
	GetTempPath(MAX_PATH, tempPath);
	GetTempFileName(tempPath, "3zE", 0, tempFile);

	zzub_player_history_enable(player, 0);
	deleteStreamPlayer();
	zzub_player_history_commit(player, 0, 0, "");
	zzub_player_history_enable(player, 1);

	updateAllViews(0, buze_event_type_update_pre_save_document, 0);

	bool state = false;
/*	string ext = "";
	size_t lp = filename.find_last_of('.');
	if (lp!=string::npos) {
		ext = filename.substr(lp+1);
		transform(ext.begin(), ext.end(), ext.begin(), tolower);
	}
*/
	// TODO: does this overwrite bmx or mod with armz?
	state = zzub_player_save_armz(player, tempFile, 0, 0, 0) == 0;

	if (state) {
		
		if (PathFileExists(filename.c_str()))
			state = DeleteFile(filename.c_str()) != 0;

		if (state) {
			state = MoveFile(tempFile, filename.c_str()) != FALSE;
		}

		DeleteFile(tempFile);
	}

	updateAllViews(0, buze_event_type_update_post_save_document, (void*)state);

	if (state) {
		// update window text
		setCurrentFile(filename);
		//setWindowTitle(document->currentFileName.c_str());
		lastSaveUndoPosition = zzub_player_history_get_position(player);
		//setMostRecent(filename);
		return true;
	} else {
		// TODO: error message in UpdatePostSaveDocument in mainframe?
		MessageBox(GetDesktopWindow(), "Cannot save file!", filename.c_str(), MB_OK);
		return false;
	}

	return 0;
}

void moveAndReplaceIndexItem(MachineIndex& index, std::string dest, std::string src) {
	MachineMenu* destItem = index.root.getMenuByName(dest);
	if (!destItem) {
		return ;
	}

	MachineMenu* srcItem = index.root.getMenuByName(src);
	if (!srcItem) {
		return ;
	}

	srcItem->parent->removeItem(srcItem);
	destItem->parent->replaceItem(destItem, srcItem);
}

bool CDocument::loadIndex() {
	machineIndex.root.clear();
	if (!machineIndex.open(buze_application_map_path(application, "index.txt", buze_path_type_user_path)))
		machineIndex.open(buze_application_map_path(application, "Gear/index.txt", buze_path_type_app_path));

	preloadMachines();
	populateUnsortedMachines();
	populateTemplateDirectory(0, buze_application_map_path(application, "Templates", buze_path_type_user_path));
	populateTemplateDirectory(0, buze_application_map_path(application, "Gear/Templates", buze_path_type_app_path));

	moveAndReplaceIndexItem(machineIndex, "PlaceGeneratorHere", "Generator");
	moveAndReplaceIndexItem(machineIndex, "PlaceEffectHere", "Effect");
	moveAndReplaceIndexItem(machineIndex, "PlaceTemplateHere", "Template");

	updateAllViews(0, buze_event_type_update_index, 0);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------
// MACHINE PRELOADING
// ---------------------------------------------------------------------------------------------------------------

void CDocument::preloadMachines() {
	// walks through index.txt and preloads those marked as such
	std::map<zzub_pluginloader_t*, std::vector<std::string> > instrumentLibraries;
	preloadMachines(&machineIndex.root, instrumentLibraries);
}

void CDocument::preloadMachines(IndexItem* item, std::map<zzub_pluginloader_t*, std::vector<std::string> >& libs) {
	// walks through index.txt and preloads those marked as such

	MachineItem* machineItem;

	for (size_t i = 0; i < item->items.size(); i++) {
		int type = item->items[i]->type;
		switch (type) {
			case 0:
				// MachineMenu
				preloadMachines(item->items[i], libs);
				break;
			case 1:
				machineItem=(MachineItem*)item->items[i];
				preloadMachine(machineItem, libs);
				// MachineItem
				break;
			case 2:
				// MachineSeparator
				break;
		}
	}
}

void CDocument::preloadMachine(MachineItem* machineItem, std::map<zzub_pluginloader_t*, std::vector<std::string> >& libs) {
	std::string pluginUri = machineItem->fileName;

	zzub_pluginloader_t* loader = zzub_player_get_pluginloader_by_name(player, pluginUri.c_str());

	// couldnt find loader directly, try to rewrite as buzz2zzub-uri
	if (loader == 0) {
		pluginUri = rewriteBuzzWrapperUri(pluginUri);
		loader = zzub_player_get_pluginloader_by_name(player, pluginUri.c_str());
		if (loader != 0) {
			// rewrite the wrapped uri directly in the index - kind of a hack
			machineItem->fileName = pluginUri;
		}
	}

	if (!loader || !machineItem->preload) return;

	// TODO: if machine was already preloaded, do something here (= keep the returned isntrument library and re-populate)
	if ((zzub_pluginloader_get_flags(loader) & zzub_plugin_flag_uses_lib_interface) != 0) {

		std::vector<std::string> instrumentNames;

		std::map<zzub_pluginloader_t*, std::vector<std::string> >::iterator it = libs.find(loader);
		if (it == libs.end()) {
			// 128k for retreiving list of vsts etc should be enough for anyone??
			std::vector<char> outputBytes(1024*128);
			int size = zzub_pluginloader_get_instrument_list(loader, &outputBytes.front(), outputBytes.size());

			const char* instr = &outputBytes.front();
			const char* pc = instr;
			while (int len = strlen(pc)) {
				instrumentNames.push_back(pc);
				pc+=len+1;
			}
			libs[loader] = instrumentNames;
		} else {
			instrumentNames = it->second;
		}

		//cout << "Found instruments: " << wr.getString() << endl;

		// replace MachineItem with a MachineMenu in the parent
		// these are for calling setInstrument() on the plugin when it is created
		MachineMenu* newMenu = new MachineMenu();
		newMenu->label = machineItem->label;
		newMenu->hide = machineItem->hide;
		newMenu->preloadReplaced = machineItem;
		for (size_t i = 0; i < instrumentNames.size(); i++) {
			MachineItem* item = new MachineItem();
			item->fileName = machineItem->fileName;
			item->label = instrumentNames[i];
			item->type = machineItem->type;
			item->hide = false;
			item->instrumentName = instrumentNames[i];
			item->fullMachineName = machineItem->fileName + "|" + item->instrumentName;
			item->preload = false;
			newMenu->append(item);
		}

		machineItem->parent->replaceItem(machineItem, newMenu);
	}
}

MachineMenu* g_effectMenu = 0;
MachineMenu* g_generatorMenu = 0;
MachineMenu* g_controllerMenu = 0;
MachineMenu* g_templateMenu = 0;
MachineMenu* g_midiEffectMenu = 0;
MachineMenu* g_midiGeneratorMenu = 0;
MachineMenu* g_otherMenu = 0;

void CDocument::populateTemplateDirectory(MachineMenu* parent, std::string const& path) {
	bool is_root = false;
	if (parent == 0) {
		if (g_templateMenu == 0) {
			g_templateMenu = new MachineMenu();
			g_templateMenu->label = "Template";
			is_root = true;
		}
		parent = g_templateMenu;
	}
	WIN32_FIND_DATA fd;
	std::string searchPath = path + "\\*.*";
	HANDLE hFind = FindFirstFile(searchPath.c_str(), &fd);
	while (hFind != INVALID_HANDLE_VALUE) {
		std::string name = fd.cFileName;
		if ( (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0) {
			size_t ld = name.find_last_of('.');
			std::string ext;
			if (ld != std::string::npos) ext = name.substr(ld + 1);
			transform(ext.begin(), ext.end(), ext.begin(), tolower);

			if (ext == "bmx" || ext == "bmw" || ext == "ccm" || ext == "armz") {
				MachineItem* tmp = new MachineItem();
				tmp->label = name.substr(0, ld);
				tmp->fileName = "@zzub.org/buze/template";
				tmp->instrumentName = path + "\\" + name;
				tmp->fullMachineName = tmp->fileName + "|" + tmp->instrumentName;
				std::cerr << tmp->fullMachineName << std::endl;
				parent->append(tmp);
			}
		} else {
			if (name != "." && name != "..") {
				MachineMenu* dir = new MachineMenu();
				dir->label = name;
				populateTemplateDirectory(dir, path + "\\" + name);
				parent->append(dir);
			}
		}
		if (!FindNextFile(hFind, &fd)) break;
	}
	FindClose(hFind);

	if (is_root) {
		std::string findName = "Template";
		MachineMenu* templateParent = machineIndex.root.getMenuByName(findName);
		if (templateParent)
			templateParent->append(g_templateMenu); else
			machineIndex.root.append(g_templateMenu);
	}
}

inline bool has_flags(int flags, int testflags) {
	return (flags & testflags) == testflags;
}

MachineMenu* get_unsorted_menu(int flags) {

	if (has_flags(flags, zzub_plugin_flag_has_event_output)) {
		return g_controllerMenu;
	} else if (has_flags(flags, zzub_plugin_flag_has_audio_output | zzub_plugin_flag_has_audio_input)) {
		return g_effectMenu;
	} else if (has_flags(flags, zzub_plugin_flag_has_audio_output)) {
		return g_generatorMenu;
	} else if (has_flags(flags, zzub_plugin_flag_has_midi_output | zzub_plugin_flag_has_midi_input)) {
		return g_midiEffectMenu;
	} else if (has_flags(flags, zzub_plugin_flag_has_midi_output)) {
		return g_midiGeneratorMenu;
	} else if (flags & (zzub_plugin_flag_has_note_output | zzub_plugin_flag_is_sequence)) {
		return g_controllerMenu;
	} else if (flags & (zzub_plugin_flag_has_audio_input | zzub_plugin_flag_has_midi_input ))
		return g_otherMenu;
	return 0;
}

bool pluginloader_name_sorter(zzub_pluginloader_t* a, zzub_pluginloader_t* b) {
	return stricmp(zzub_pluginloader_get_name(a), zzub_pluginloader_get_name(b)) < 0;
}

void CDocument::populateUnsortedMachines() {

	g_effectMenu = new MachineMenu();
	g_effectMenu->label = "Unsorted Effects";

	g_generatorMenu = new MachineMenu();
	g_generatorMenu->label = "Unsorted Generators";

	g_controllerMenu = new MachineMenu();
	g_controllerMenu->label = "Unsorted Controllers";

	g_midiEffectMenu = new MachineMenu();
	g_midiEffectMenu->label = "Unsorted MIDI Effects";

	g_midiGeneratorMenu = new MachineMenu();
	g_midiGeneratorMenu->label = "Unsorted MIDI Generators";
	
	g_otherMenu = new MachineMenu();
	g_otherMenu->label = "Unsorted Other Plugins";

	// get all loaders, sort alphabetaicly, then add to unsorted menu
	std::vector<zzub_pluginloader_t*> loaders;

	for (int i = 0; i < zzub_player_get_pluginloader_count(player); i++) {
		zzub_pluginloader_t* loader = zzub_player_get_pluginloader(player, i);
		loaders.push_back(loader);
	}

	std::sort(loaders.begin(), loaders.end(), pluginloader_name_sorter);

	for (int i = 0; i < (int)loaders.size(); i++) {
		zzub_pluginloader_t* loader = loaders[i];
		int flags = zzub_pluginloader_get_flags(loader);
		MachineMenu* machineMenu = get_unsorted_menu(flags);
		if (machineMenu == 0) continue;

		std::string pluginUri = zzub_pluginloader_get_uri(loader);
		MachineItem* mi = machineIndex.root.getMachineByName(pluginUri);
		// uri is already rewritten!
		/*if (!mi) {
			// couldnt find an entry in the index for this uri. if the uri is a buzz2zzub-uri we try to
			// rewrite it and look again
			std::string name = rewriteBuzzWrapperName(pluginUri);
			if (name != pluginUri)
				mi = machineIndex.root.getMachineByName(name);
		}*/

		if (!mi) {
			mi = new MachineItem();
			mi->label = zzub_pluginloader_get_name(loader);
			mi->fullMachineName = mi->fileName = pluginUri;
			machineMenu->append(mi);
		}
	}

	std::string findName;
	findName = "Generator";
	MachineMenu* generatorParent = machineIndex.root.getMenuByName(findName);
	if (generatorParent)
		generatorParent->append(g_generatorMenu); else
		machineIndex.root.append(g_generatorMenu);

	findName = "Effect";
	MachineMenu* effectParent = machineIndex.root.getMenuByName(findName);
	if (effectParent)
		effectParent->append(g_effectMenu); else
		machineIndex.root.append(g_effectMenu);

	findName = "Controller";
	MachineMenu* controllerParent = machineIndex.root.getMenuByName(findName);
	if (controllerParent)
		controllerParent->append(g_controllerMenu); else
		machineIndex.root.append(g_controllerMenu);

	machineIndex.root.append(g_midiGeneratorMenu);
	machineIndex.root.append(g_midiEffectMenu);
	machineIndex.root.append(g_otherMenu);
}

zzub_plugin_t* CDocument::createMachine(std::string const& uri, std::string const& instrumentName, float x, float y, zzub_plugin_group_t* plugingroup) {
	zzub_pluginloader_t* loader = zzub_player_get_pluginloader_by_name(player, uri.c_str());
	if (loader == 0) return 0;

	const char* name = zzub_player_get_new_plugin_name(player, uri.c_str());
	//std::string name(bytes.begin(), bytes.begin() + strlen(&bytes.front()));// = player->plugin_get_new_name(uri);

	zzub_plugin_t* plugin = zzub_player_create_plugin(player, 0, 0, name, loader, plugingroup);
	if (plugin == 0) return 0;
	
	//name = zzub_plugin_get_name(plugin);

	if (configuration->getMachinesMinimized()) {
		zzub_plugin_set_minimize(plugin, true);
	}

	zzub_plugin_set_position(plugin, x, y);

	// add more pattern tracks
	int defTracks = configuration->getMachineDefaultTracks(uri, zzub_pluginloader_get_tracks_min(loader));
	defTracks = std::min(defTracks, zzub_pluginloader_get_tracks_max(loader));
	defTracks = std::max(defTracks, zzub_pluginloader_get_tracks_min(loader));

	if (zzub_plugin_get_track_count(plugin, 2) != defTracks)
		zzub_plugin_set_track_count(plugin, defTracks);

	// add a default pattern format and pattern for non-effects
	int flags = zzub_pluginloader_get_flags(loader);

	// Format/Pattern Creation Modes
	{
		int mode = configuration->getFormatPatternCreationMode();
		// 0 == No Formats/Patterns
		// 1 == Gen Format
		// 2 == Gen Format+Pattern
		// 3 == Gen+FX Formats
		// 4 == Gen+FX Formats+Patterns
		// 5 == Gen Format+Pattern, FX Format

		if (mode != 0) {
			int createFlags = (flags & PLUGIN_FLAGS_MASK);

			bool create_format = false;
			bool create_pattern = false;

			if (createFlags & IS_CONTROLLER_PLUGIN_FLAGS) {
				// quickfix: treat as plugin
				createFlags = IS_EFFECT_PLUGIN_FLAGS;
			}
			/*else*/ if (createFlags & IS_EFFECT_PLUGIN_FLAGS) {
				if (mode == 3) create_format = true;
				if (mode == 4) create_format = true;
				if (mode == 5) create_format = true;
				// ---
				if (mode == 4) create_pattern = true;
			}
			else if (createFlags == IS_GENERATOR_PLUGIN_FLAGS) {
				create_format = true; // all modes besides 0 create a format for gens
				// ---
				if (mode == 2) create_pattern = true;
				if (mode == 4) create_pattern = true;
				if (mode == 5) create_pattern = true;
			}

			zzub_pattern_format_t* format;
			zzub_pattern_t* pattern;

			if (create_format) {
				format = createDefaultFormat(plugin, false);
			}

			if (create_pattern) {
				int rows = configuration->getMachinePatternLength(uri);

				char const* description = 0;
				if (configuration->getPatternNamingMode() == 1) {
					description = zzub_pattern_format_get_name(format);
				}
				pattern = zzub_player_create_pattern(player, format, description, rows);
			}
		}
	}

// 	// add a sequence track for generators
// 	if (((flags & zzub_plugin_flag_has_audio_output) || (flags & zzub_plugin_flag_has_midi_output) || (flags & zzub_plugin_flag_has_event_output)) && ((flags & zzub_plugin_flag_has_audio_input) == 0)) {
// 		int seq_count = zzub_player_get_sequence_track_count(player);
// 		createSequenceTrack(seq_count);
// 	//zzub_player_create_sequence(player, plugin, zzub_sequence_type_pattern);
// 	}

	if (instrumentName.length() > 0)
		zzub_plugin_set_instrument(plugin, instrumentName.c_str());

	return plugin;
}