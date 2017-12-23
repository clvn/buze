#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "Configuration.h"
#include "PropertyList/PropertyList.h"
#include "PropertyList/PropertyItem.h"
#include "PropertyList/PropertyItemEditors.h"
#include "PropertyList/PropertyItemImpl.h"
#include "Properties.h"
#include "BuzeConfiguration.h"
#include "utils.h"
#include "PropertyViewProperties.h"

using namespace std;

int bits_from_format(int format) {
	switch (format) {
		case zzub_wave_buffer_type_si16:
			return 16;
		case zzub_wave_buffer_type_si24:
			return 24;
		case zzub_wave_buffer_type_f32:
		case zzub_wave_buffer_type_si32:
			return 32;
		default:
			return 0;
	}
}

CPropertyProvider::~CPropertyProvider() {
	destroyProperties();
}

void CPropertyProvider::destroyProperties() {
	for (std::vector<PropertyInfoBase*>::iterator i = properties.begin(); i != properties.end(); ++i) {
		delete *i;
	}
	properties.clear();
}


/***

	CPatternPropertyProvider

***/

bool CTrackProperty::setLabel(std::string value) {
	zzub_pattern_format_set_track_name(format, plugin, group, track, value.c_str());
	zzub_player_history_commit(provider->player, 0, 0, "Set Track Name");
	return true;
}

std::string CTrackProperty::getLabel() {
	return zzub_pattern_format_get_track_name(format, plugin, group, track);
}

bool CTrackProperty::setMuted(BOOL value) {
	zzub_pattern_format_set_track_mute(format, plugin, group, track, value ? 1 : 0);
	zzub_player_history_commit(provider->player, 0, 0, "Toggle Track Mute");
	return true;
}

BOOL CTrackProperty::getMuted() {
	return zzub_pattern_format_get_track_mute(format, plugin, group, track) ? TRUE : FALSE;
}

CPatternPropertyProvider::CPatternPropertyProvider(CView* returnView, CDocument* doc, zzub_pattern_t* pattern) {
	this->hReturnView = returnView;
	this->document = doc;
	this->player = (zzub_player_t*)buze_document_get_player(document);
	this->pattern = pattern;
	name = "pattern";

	createProperties();
}

void CPatternPropertyProvider::createProperties() {

	destroyProperties();

	properties.push_back(new CategoryPropertyInfo("Pattern"));
	properties.push_back(new StringPropertyInfo<CPatternPropertyProvider>(this, "Name", &CPatternPropertyProvider::getPatternName, &CPatternPropertyProvider::setPatternName));
	properties.push_back(new IntPropertyInfo<CPatternPropertyProvider>(this, "Length", &CPatternPropertyProvider::getPatternLength, &CPatternPropertyProvider::setPatternLength));
	properties.push_back(new IntPropertyInfo<CPatternPropertyProvider>(this, "Resolution", &CPatternPropertyProvider::getResolution, &CPatternPropertyProvider::setResolution));
	properties.push_back(new BoolPropertyInfo<CPatternPropertyProvider>(this, "Looping", &CPatternPropertyProvider::getLoopEnabled, &CPatternPropertyProvider::setLoopEnabled));
	properties.push_back(new IntPropertyInfo<CPatternPropertyProvider>(this, "Begin Loop", &CPatternPropertyProvider::getBeginLoop, &CPatternPropertyProvider::setBeginLoop));
	properties.push_back(new IntPropertyInfo<CPatternPropertyProvider>(this, "End Loop", &CPatternPropertyProvider::getEndLoop, &CPatternPropertyProvider::setEndLoop));

	zzub_pattern_format_t* format = zzub_pattern_get_format(pattern);
	zzub_pattern_format_column_iterator_t* it = zzub_pattern_format_get_iterator(format);

	properties.push_back(new CategoryPropertyInfo("Tracks"));

	zzub_plugin_t* lastplugin = 0;
	int lastgroup = -1, lasttrack = -1;
	int index = 0;
	while (zzub_pattern_format_column_iterator_valid(it)) {
		zzub_pattern_format_column_t* col = zzub_pattern_format_column_iterator_current(it);
		zzub_plugin_t* plugin = zzub_pattern_format_column_get_plugin(col);
		int group = zzub_pattern_format_column_get_group(col);
		int track = zzub_pattern_format_column_get_track(col);

		if (lastplugin != plugin || lastgroup != group || lasttrack != track) {
			CTrackProperty* prop = new CTrackProperty();
			prop->provider = this;
			prop->format = format;
			prop->group = group;
			prop->track = track;
			prop->plugin = plugin;
			std::stringstream strm;
			strm << "Track " << (index + 1) << " Name";
			properties.push_back(new StringPropertyInfo<CTrackProperty>(prop, strm.str().c_str(), &CTrackProperty::getLabel, &CTrackProperty::setLabel));

			strm.str("");
			strm << "Track " << (index + 1) << " Mute";
			properties.push_back(new BoolPropertyInfo<CTrackProperty>(prop, strm.str().c_str(), &CTrackProperty::getMuted, &CTrackProperty::setMuted));

			index++;
		}

		lastplugin = plugin;
		lastgroup = group;
		lasttrack = track;

		zzub_pattern_format_column_iterator_next(it);
	}

	zzub_pattern_format_column_iterator_destroy(it);
//	properties.push_back(new BoolPropertyInfo<zzub::pattern, CPatternPropertyProvider>(this, "Repeat When Lengthening", &CPatternPropertyProvider::getPatternRepeatLength, &CPatternPropertyProvider::setPatternRepeatLength));
}

int CPatternPropertyProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CPatternPropertyProvider::getProperty(int index) {
	return properties[index];
}

bool CPatternPropertyProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);

	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
		case zzub_event_type_delete_pattern:
			return zzubData->delete_pattern.pattern == pattern;
		case zzub_event_type_update_pattern:
			return zzubData->update_pattern.pattern == pattern;
		case zzub_event_type_insert_patternformatcolumn:
			return zzub_pattern_format_column_get_format(zzubData->insert_pattern_format_column.patternformatcolumn) == zzub_pattern_get_format(pattern);
		case zzub_event_type_delete_patternformatcolumn:
			return zzub_pattern_format_column_get_format(zzubData->delete_pattern_format_column.patternformatcolumn) == zzub_pattern_get_format(pattern);
		case zzub_event_type_insert_patternformattrack:
		case zzub_event_type_update_patternformattrack:
		case zzub_event_type_delete_patternformattrack:
			return true; // armstrong event lacks data to identify this patternformattrack so just handle all for now
	}
	return false;
}

bool CPatternPropertyProvider::setPatternName(std::string name) {
	zzub_pattern_t* pat = zzub_player_get_pattern_by_name(player, name.c_str());
	if (pat != pattern && pat != 0) return false; // disallow dupes

	const char* patternname = zzub_pattern_get_name(pattern);
	if (name == patternname) return false;

	zzub_pattern_set_name(pattern, name.c_str());
	zzub_player_history_commit(player, 0, 0, "Rename Pattern");
	return true;
}

std::string CPatternPropertyProvider::getPatternName() {
	return zzub_pattern_get_name(pattern);
}

int CPatternPropertyProvider::getPatternLength() {
	return zzub_pattern_get_row_count(pattern);
}

bool CPatternPropertyProvider::setPatternLength(int rows) {
	zzub_pattern_set_row_count(pattern, rows);
	zzub_player_history_commit(player, 0, 0, "Resize Pattern");
	return true;
}

// flag to tell we  should repeat the existing pattern when lengthening 
BOOL CPatternPropertyProvider::getPatternRepeatLength() {
	return FALSE;
}

bool CPatternPropertyProvider::setPatternRepeatLength(BOOL state) {
	return false;
}

int CPatternPropertyProvider::getResolution() {
	return zzub_pattern_get_resolution(pattern);
}

bool CPatternPropertyProvider::setResolution(int resolution) {
	if (resolution < 1) return false;
	zzub_pattern_set_resolution(pattern, resolution);
	zzub_player_history_commit(player, 0, 0, "Set Pattern Resolution");
	return true;
}

BOOL CPatternPropertyProvider::getLoopEnabled() {
	return zzub_pattern_get_loop_enabled(pattern) != 0 ? TRUE : FALSE;
}

bool CPatternPropertyProvider::setLoopEnabled(BOOL state) {
	zzub_pattern_set_loop_enabled(pattern, state ? 1 : 0);
	zzub_player_history_commit(player, 0, 0, "Toggle Pattern Looping");
	return true;
}

bool CPatternPropertyProvider::setEndLoop(int pos) {
	zzub_pattern_set_loop_end(pattern, pos);
	zzub_player_history_commit(player, 0, 0, "Set Pattern Loop End");
	return true;
}

int CPatternPropertyProvider::getEndLoop() {
	return zzub_pattern_get_loop_end(pattern);
}

bool CPatternPropertyProvider::setBeginLoop(int pos) {
	zzub_pattern_set_loop_start(pattern, pos);
	zzub_player_history_commit(player, 0, 0, "Set Pattern Loop Start");
	return true;
}

int CPatternPropertyProvider::getBeginLoop() {
	return zzub_pattern_get_loop_start(pattern);
}








CPatternFormatPropertyProvider::CPatternFormatPropertyProvider(CView* returnView, CDocument* doc, zzub_pattern_format_t* patternformat) {
	this->hReturnView = returnView;
	this->document = doc;
	this->player = (zzub_player_t*)buze_document_get_player(document);
	this->patternformat = patternformat;
	name = "patternformat";

	createProperties();
}

void CPatternFormatPropertyProvider::createProperties() {
	destroyProperties();
	properties.push_back(new CategoryPropertyInfo("Patternformat"));
	properties.push_back(new StringPropertyInfo<CPatternFormatPropertyProvider>(this, "Name", &CPatternFormatPropertyProvider::getName, &CPatternFormatPropertyProvider::setName));
}

int CPatternFormatPropertyProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CPatternFormatPropertyProvider::getProperty(int index) {
	return properties[index];
}

bool CPatternFormatPropertyProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);

	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
		case zzub_event_type_update_patternformat:
			return zzubData->update_pattern_format.patternformat == patternformat;
		case zzub_event_type_delete_patternformat:
			return zzubData->delete_pattern_format.patternformat == patternformat;
		case zzub_event_type_insert_patternformatcolumn:
			return zzub_pattern_format_column_get_format(zzubData->insert_pattern_format_column.patternformatcolumn) == patternformat;
		case zzub_event_type_delete_patternformatcolumn:
			return zzub_pattern_format_column_get_format(zzubData->delete_pattern_format_column.patternformatcolumn) == patternformat;
	}
	return false;
}

bool CPatternFormatPropertyProvider::setName(std::string name) {
	zzub_pattern_format_set_name(patternformat, name.c_str());
	zzub_player_history_commit(player, 0, 0, "Rename Patternformat");
	return true;
}

std::string CPatternFormatPropertyProvider::getName() {
	return zzub_pattern_format_get_name(patternformat);
}

/***

	CAttributeProperty

***/

bool CAttributeProperty::setValue(int value) {
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	zzub_attribute_t* attr = zzub_pluginloader_get_attribute(loader, attributeIndex);
	value = std::min(value, zzub_attribute_get_value_max(attr));
	value = std::max(value, zzub_attribute_get_value_min(attr));
	zzub_plugin_set_attribute_value(machine, attributeIndex, value);
	zzub_player_history_commit(provider->player, 0, 0, "Set Plugin Attribute");
	return true;
}

int CAttributeProperty::getValue() {
	return zzub_plugin_get_attribute_value(machine, attributeIndex);
}

/***

	CMachinePropertyProvider

***/

CMachinePropertyProvider::CMachinePropertyProvider(CView* returnView, CDocument* doc, zzub_plugin_t* machine) {
	this->hReturnView = returnView;
	this->document = doc;
	this->player = (zzub_player_t*)buze_document_get_player(document);
	this->configuration = buze_document_get_configuration(document);
	this->machine = machine;
	name = "machine";
	createProperties();
}

void CMachinePropertyProvider::createProperties() {
	destroyProperties();

	int flags = zzub_plugin_get_flags(machine);
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	int mintracks = zzub_pluginloader_get_tracks_min(loader);
	int maxtracks = zzub_pluginloader_get_tracks_max(loader);

	std::stringstream trackstrm;
	trackstrm << "Number of tracks (" << mintracks << ".." << maxtracks << ")";

	std::stringstream deftrackstrm;
	deftrackstrm << "Set the default number of tracks (voices) when creating new plugins of this type (" << mintracks << ".." << maxtracks << ")";

	properties.push_back(new CategoryPropertyInfo("Machine"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Name", &CMachinePropertyProvider::getMachineName, &CMachinePropertyProvider::setMachineName, "Plugin name"));
	//properties.push_back(new BoolPropertyInfo<CMachinePropertyProvider>(this, "Disable MIDI-In", &CMachinePropertyProvider::getMachineMidiIn, &CMachinePropertyProvider::setMachineMidiIn));
	//properties.push_back(new IntPropertyInfo<CMachinePropertyProvider>(this, "Default Pattern Length", &CMachinePropertyProvider::getMachinePatternLength, &CMachinePropertyProvider::setMachinePatternLength));
	properties.push_back(new IntPropertyInfo<CMachinePropertyProvider>(this, "Tracks", &CMachinePropertyProvider::getMachineTracks, &CMachinePropertyProvider::setMachineTracks, trackstrm.str()));
	properties.push_back(new BoolPropertyInfo<CMachinePropertyProvider>(this, "Solo", &CMachinePropertyProvider::getMachineSolo, &CMachinePropertyProvider::setMachineSolo, "Toggle mute on all other plugins"));
	properties.push_back(new BoolPropertyInfo<CMachinePropertyProvider>(this, "Mute", &CMachinePropertyProvider::getMachineMuted, &CMachinePropertyProvider::setMachineMuted, "Toggle mute on this plugin"));
	properties.push_back(new BoolPropertyInfo<CMachinePropertyProvider>(this, "Bypass", &CMachinePropertyProvider::getMachineBypass, &CMachinePropertyProvider::setMachineBypass, "Toggle bypass on this plugin"));
	properties.push_back(new IntPropertyInfo<CMachinePropertyProvider>(this, "Latency", &CMachinePropertyProvider::getMachineLatency, &CMachinePropertyProvider::setMachineLatency, "Plugin latency compensation in samples. -1 = use plugin default"));

	if ((flags & zzub_plugin_flag_stream) != 0) {
		properties.push_back(new FileNamePropertyInfo<CMachinePropertyProvider>(this, "Stream Source", "*.*", &CMachinePropertyProvider::getStreamSource, &CMachinePropertyProvider::setStreamSource, "Stream file name/URI"));
	}

	properties.push_back(new CategoryPropertyInfo("Info"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Plugin Type", &CMachinePropertyProvider::getMachineType, 0, "Plugin collection name"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Plugin URI", &CMachinePropertyProvider::getMachineURI, 0, "Unique plugin identification"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Plugin Name", &CMachinePropertyProvider::getMachineFullName, 0, "Plugin type name"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Plugin File", &CMachinePropertyProvider::getMachineFileName, 0, "Plugin file"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Plugin Author", &CMachinePropertyProvider::getMachineAuthor, 0, "Plugin author"));
	properties.push_back(new IntPropertyInfo<CMachinePropertyProvider>(this, "Default Tracks", &CMachinePropertyProvider::getMachineDefaultTracks, &CMachinePropertyProvider::setMachineDefaultTracks, deftrackstrm.str()));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Has Audio Input", &CMachinePropertyProvider::getMachineAudioInput, 0, "The plugin accepts incoming audio wires"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Has Audio Output", &CMachinePropertyProvider::getMachineAudioOutput, 0, "The plugin supports outgoing audio wires"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Has MIDI Input", &CMachinePropertyProvider::getMachineMidiInput, 0, "The plugin accepts incoming MIDI wires"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Has MIDI Output", &CMachinePropertyProvider::getMachineMidiOutput, 0, "The plugin supports outgoing MIDI wires"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Has Event Output", &CMachinePropertyProvider::getMachineEventOutput, 0, "The plugin supports outgoing parameter event wires"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Has Interval", &CMachinePropertyProvider::getMachineInterval, 0, "The plugin generates events at intervals"));
	properties.push_back(new StringPropertyInfo<CMachinePropertyProvider>(this, "Has Sequence", &CMachinePropertyProvider::getMachineSequence, 0, "The plugin triggers sequences"));
	
	properties.push_back(new CategoryPropertyInfo("Attributes"));

	std::stringstream strm;
	for (int i = 0; i < zzub_pluginloader_get_attribute_count(loader); i++) {
		strm.str(""); strm.clear();
		zzub_attribute_t* attr = zzub_pluginloader_get_attribute(loader, i);
		CAttributeProperty* prop = new CAttributeProperty();
		prop->attributeIndex = i;
		prop->provider = this;
		prop->machine = machine;
		strm << zzub_attribute_get_name(attr) << ":\n" << "Min " << zzub_attribute_get_value_min(attr) << "\nMax: " << zzub_attribute_get_value_max(attr) << "\nDefault: " << zzub_attribute_get_value_default(attr);
		PropertyInfoBase* pi = new IntPropertyInfo<CAttributeProperty>(prop, zzub_attribute_get_name(attr), &CAttributeProperty::getValue, &CAttributeProperty::setValue, strm.str());
		properties.push_back(pi);
	}
}

int CMachinePropertyProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CMachinePropertyProvider::getProperty(int index) {
	return properties[index];
}

bool CMachinePropertyProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);

	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
		case zzub_event_type_update_plugin:
			return zzubData->update_plugin.plugin == machine;
		case zzub_event_type_delete_plugin:
			return zzubData->delete_plugin.plugin == machine;
	}
	return false;
}


// machine properties (working on selected machine)
bool CMachinePropertyProvider::setMachineName(std::string name) {
	zzub_plugin_set_name(machine, const_cast<char*>(name.c_str()));
	zzub_player_history_commit(player, 0, 0, "Rename Plugin");
	return true;
}

std::string CMachinePropertyProvider::getMachineName() {
	return zzub_plugin_get_name(machine);
}

std::string CMachinePropertyProvider::getMachineURI() {
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	return zzub_pluginloader_get_uri(loader);
}

std::string CMachinePropertyProvider::getMachineType() {
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	zzub_plugincollection_t* coll = zzub_pluginloader_get_plugincollection(loader);
	const char* collnamestr = zzub_plugincollection_get_name(coll);
	return collnamestr ? collnamestr : "(null)";
}

std::string CMachinePropertyProvider::getMachineFileName() {
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	return zzub_pluginloader_get_plugin_file(loader);
}

std::string CMachinePropertyProvider::getMachineFullName() {
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	return zzub_pluginloader_get_name(loader);
}

std::string CMachinePropertyProvider::getMachineAuthor() {
	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(machine);
	return zzub_pluginloader_get_author(loader);
}

std::string CMachinePropertyProvider::getMachineAudioInput() {
	int flags = zzub_plugin_get_flags(machine);
	return (flags & zzub_plugin_flag_has_audio_input) != 0 ? "Yes" : "No";
}

std::string CMachinePropertyProvider::getMachineAudioOutput() {
	int flags = zzub_plugin_get_flags(machine);
	return (flags & zzub_plugin_flag_has_audio_output) != 0 ? "Yes" : "No";
}

std::string CMachinePropertyProvider::getMachineMidiInput() {
	int flags = zzub_plugin_get_flags(machine);
	return (flags & zzub_plugin_flag_has_midi_input) != 0 ? "Yes" : "No";
}

std::string CMachinePropertyProvider::getMachineMidiOutput() {
	int flags = zzub_plugin_get_flags(machine);
	return (flags & zzub_plugin_flag_has_midi_output) != 0 ? "Yes" : "No";
}

std::string CMachinePropertyProvider::getMachineEventOutput() {
	int flags = zzub_plugin_get_flags(machine);
	return (flags & zzub_plugin_flag_has_event_output) != 0 ? "Yes" : "No";
}

std::string CMachinePropertyProvider::getMachineInterval() {
	int flags = zzub_plugin_get_flags(machine);
	return (flags & zzub_plugin_flag_is_interval) != 0 ? "Yes" : "No";
}

std::string CMachinePropertyProvider::getMachineSequence() {
	int flags = zzub_plugin_get_flags(machine);
	return (flags & zzub_plugin_flag_is_sequence) != 0 ? "Yes" : "No";
}



BOOL CMachinePropertyProvider::getMachineMidiIn() {
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	return configuration->getMachineMidiInDisabled(zzub_pluginloader_get_uri(info))?TRUE:FALSE;
}

bool CMachinePropertyProvider::setMachineMidiIn(BOOL state) {
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	configuration->setMachineMidiInDisabled(zzub_pluginloader_get_uri(info), state!=FALSE?true:false);
	return true;
}

bool CMachinePropertyProvider::setMachineTracks(int tracks) {

	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	int min_tracks = zzub_pluginloader_get_tracks_min(info);
	int max_tracks = zzub_pluginloader_get_tracks_max(info);
	if (tracks < min_tracks) tracks = min_tracks;
	if (tracks > max_tracks) tracks = max_tracks;

	zzub_plugin_set_track_count(machine, tracks);
	zzub_player_history_commit(player, 0, 0, "Set Plugin Tracks");
	return true;
}

int CMachinePropertyProvider::getMachineTracks() {
	int tracks = zzub_plugin_get_track_count(machine, 2);
	return tracks;
}

BOOL CMachinePropertyProvider::getMachineMuted() {
	return zzub_plugin_get_mute(machine) != 0;
}

bool CMachinePropertyProvider::setMachineMuted(BOOL state) {
	zzub_plugin_set_mute(machine, state != 0?1:0);
	zzub_player_history_commit(player, 0, 0, "Mute Plugin");
	return true;
}

BOOL CMachinePropertyProvider::getMachineSolo() {
	return machine == buze_document_get_solo_plugin(document) ? TRUE : FALSE;
}

bool CMachinePropertyProvider::setMachineSolo(BOOL state) {
	buze_document_set_solo_plugin(document, machine, state);
	zzub_player_history_commit(player, 0, 0, "Solo Plugin");
	return true;
}

BOOL CMachinePropertyProvider::getMachineBypass() {
	return zzub_plugin_get_bypass(machine) != 0;
}

bool CMachinePropertyProvider::setMachineBypass(BOOL state) {
	zzub_plugin_set_bypass(machine, state != 0?1:0);
	zzub_player_history_commit(player, 0, 0, "Bypass Plugin");
	return true;
}

bool CMachinePropertyProvider::setMachinePatternLength(int length) {
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	configuration->setMachinePatternLength(zzub_pluginloader_get_uri(info), length);
	return true;
}

int CMachinePropertyProvider::getMachinePatternLength() {
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	return configuration->getMachinePatternLength(zzub_pluginloader_get_uri(info));
}

bool CMachinePropertyProvider::setMachineDefaultTracks(int amount) {
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	configuration->setMachineDefaultTracks(zzub_pluginloader_get_uri(info), amount);
	return true;
}

int CMachinePropertyProvider::getMachineDefaultTracks() {
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(machine);
	int deftracks = zzub_pluginloader_get_tracks_min(info);
	return configuration->getMachineDefaultTracks(zzub_pluginloader_get_uri(info), deftracks);
}

bool CMachinePropertyProvider::setStreamSource(std::string name) {
	zzub_plugin_set_stream_source(machine, const_cast<char*>(name.c_str()));
	zzub_player_history_commit(player, 0, 0, "Set Plugin Stream Source");
	return true;
}

std::string CMachinePropertyProvider::getStreamSource() {
	return zzub_plugin_get_stream_source(machine);
}

bool CMachinePropertyProvider::setMachineLatency(int length) {
	zzub_plugin_set_latency(machine, length);
	zzub_player_history_commit(player, 0, 0, "Set Plugin Latency Compensation");
	return false;
}

int CMachinePropertyProvider::getMachineLatency() {
	return zzub_plugin_get_latency(machine);
}


/***

	CMachinePropertyProvider

***/

CGroupPropertyProvider::CGroupPropertyProvider(CView* returnView, CDocument* doc, zzub_plugin_group_t* group) {
	this->hReturnView = returnView;
	this->document = doc;
	this->player = (zzub_player_t*)buze_document_get_player(document);
	this->configuration = buze_document_get_configuration(document);
	this->plugingroup = group;
	name = "plugingroup";
	createProperties();
}

void CGroupPropertyProvider::createProperties() {
	destroyProperties();
	properties.push_back(new CategoryPropertyInfo("Plugin Group"));
	properties.push_back(new StringPropertyInfo<CGroupPropertyProvider>(this, "Name", &CGroupPropertyProvider::getName, &CGroupPropertyProvider::setName, "Plugin group name"));
}

int CGroupPropertyProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CGroupPropertyProvider::getProperty(int index) {
	return properties[index];
}

bool CGroupPropertyProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);

	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
		case zzub_event_type_update_plugin_group:
			return zzubData->update_plugin_group.group == plugingroup;
		case zzub_event_type_delete_plugin_group:
			return zzubData->delete_plugin_group.group == plugingroup;
	}
	return false;
}

bool CGroupPropertyProvider::setName(std::string name) {
	zzub_plugin_group_set_name(plugingroup, name.c_str());
	return true;
}

std::string CGroupPropertyProvider::getName() {
	return zzub_plugin_group_get_name(plugingroup);
}

/***

	CConnectionPropertyProvider

***/

CConnectionPropertyProvider::CConnectionPropertyProvider(CView* returnView, CDocument* doc, zzub_plugin_t* _to_plugin, zzub_plugin_t* _from_plugin) {
	this->hReturnView = returnView;
	this->document = doc;
	this->player = (zzub_player_t*)buze_document_get_player(document);
	from_plugin = _from_plugin;
	to_plugin = _to_plugin;
	createProperties();
}

CConnectionPropertyProvider::CConnectionPropertyProvider(CView* returnView, CDocument* doc, zzub_connection_t* conn) {
	this->hReturnView = returnView;
	this->document = doc;
	this->player = (zzub_player_t*)buze_document_get_player(document);
	from_plugin = zzub_connection_get_from_plugin(conn);
	to_plugin = zzub_connection_get_to_plugin(conn);
	createProperties();
}

void CConnectionPropertyProvider::createProperties() {

	destroyProperties();

	zzub_pluginloader_t* from_loader = zzub_plugin_get_pluginloader(from_plugin);
	zzub_pluginloader_t* to_loader = zzub_plugin_get_pluginloader(to_plugin);

	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	zzub_connection_t* midiconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_midi);
	zzub_connection_t* eventconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_event);

	properties.push_back(new CategoryPropertyInfo("Plugins"));
	properties.push_back(new StringPropertyInfo<CConnectionPropertyProvider>(this, "Output Plugin", &CConnectionPropertyProvider::getFromPluginName, 0, "Name of the plugin where the connection comes from"));
	properties.push_back(new StringPropertyInfo<CConnectionPropertyProvider>(this, "Input Plugin", &CConnectionPropertyProvider::getToPluginName, 0, "Name of the plugin where the connection goes to"));

	if (audioconn != 0) {
		properties.push_back(new CategoryPropertyInfo("Audio Connection"));

		{
			std::stringstream strm;
			strm << "Index of the first audio channel on '" << getFromPluginName() << "'.\n";
			strm << "Active outputs: '" << zzub_plugin_get_output_channel_count(from_plugin) << " (Max " << zzub_pluginloader_get_output_channel_count(from_loader) << ")";
			properties.push_back(new IntPropertyInfo<CConnectionPropertyProvider>(this, "First Output Channel", &CConnectionPropertyProvider::getFirstOutput, &CConnectionPropertyProvider::setFirstOutput, strm.str()));
		}
		{
			std::stringstream strm;
			strm << "Number of audio channels on '" << getFromPluginName() << "'.\n";
			strm << "Active outputs: '" << zzub_plugin_get_output_channel_count(from_plugin) << " (Max " << zzub_pluginloader_get_output_channel_count(from_loader) << ")";
			properties.push_back(new IntPropertyInfo<CConnectionPropertyProvider>(this, "Output Channels", &CConnectionPropertyProvider::getOutputs, &CConnectionPropertyProvider::setOutputs, strm.str()));
		}
		{
			std::stringstream strm;
			strm << "Index of the first audio channel on '" << getToPluginName() << "'.\n";
			strm << "Active inputs: '" << zzub_plugin_get_input_channel_count(to_plugin) << " (Max " << zzub_pluginloader_get_input_channel_count(to_loader) << ")";
			properties.push_back(new IntPropertyInfo<CConnectionPropertyProvider>(this, "First Input Channel", &CConnectionPropertyProvider::getFirstInput, &CConnectionPropertyProvider::setFirstInput, strm.str()));
		}
		{
			std::stringstream strm;
			strm << "Number of audio channels on '" << getToPluginName() << "'.\n";
			strm << "Active inputs: '" << zzub_plugin_get_input_channel_count(to_plugin) << " (Max " << zzub_pluginloader_get_input_channel_count(to_loader) << ")";
			properties.push_back(new IntPropertyInfo<CConnectionPropertyProvider>(this, "Input Channels", &CConnectionPropertyProvider::getInputs, &CConnectionPropertyProvider::setInputs, strm.str()));
		}

	}

	if (midiconn != 0) {
		properties.push_back(new CategoryPropertyInfo("MIDI Connection"));
		properties.push_back(new StringPropertyInfo<CConnectionPropertyProvider>(this, "MIDI Device", &CConnectionPropertyProvider::getMidiDevice, 0, "Selected MIDI device on the input plugin"));
	}

	//properties.push_back(new CategoryPropertyInfo("Event Connection"));
	//properties.push_back(new IntPropertyInfo<CConnectionPropertyProvider>(this, "", 0, 0));
}

int CConnectionPropertyProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CConnectionPropertyProvider::getProperty(int index) {
	return properties[index];
}

bool CConnectionPropertyProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	zzub_connection_t* midiconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_midi);
	zzub_connection_t* eventconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_event);

	zzub_plugin_t* audioplug = audioconn != 0 ? zzub_connection_get_connection_plugin(audioconn) : 0;
	zzub_plugin_t* midiplug = midiconn != 0 ? zzub_connection_get_connection_plugin(midiconn) : 0;
	zzub_plugin_t* eventplug = eventconn != 0 ? zzub_connection_get_connection_plugin(eventconn) : 0;
	// TODO ... the armstrong interface is a bit lacking here
	assert(pHint != 0);

	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
		case zzub_event_type_delete_plugin:
			if (from_plugin == zzubData->delete_plugin.plugin)
				return true;
			if (to_plugin == zzubData->delete_plugin.plugin)
				return true;
			if (audioplug && audioplug == zzubData->delete_plugin.plugin)
				return true;
			if (midiplug && midiplug == zzubData->delete_plugin.plugin)
				return true;
			if (eventplug && eventplug == zzubData->delete_plugin.plugin)
				return true;
			break;
		case zzub_event_type_update_connection:
			return false;
	}
	return false;
}

bool CConnectionPropertyProvider::setFirstInput(int chn) {
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (!audioconn) return false;
	int maxvalue = zzub_plugin_get_input_channel_count(to_plugin);
	chn = std::max(chn, 0);
	chn = std::min(chn, maxvalue);
	zzub_connection_set_first_input(audioconn, chn);
	zzub_player_history_commit(player, 0, 0, "Set Connection First Input");
	return true;
}

int CConnectionPropertyProvider::getFirstInput() {
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (!audioconn) return 0;
	return zzub_connection_get_first_input(audioconn);
}

bool CConnectionPropertyProvider::setInputs(int chn) {
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (!audioconn) return false;
	int maxvalue = zzub_plugin_get_input_channel_count(to_plugin);
	chn = std::max(chn, 1);
	chn = std::min(chn, maxvalue);
	zzub_connection_set_input_count(audioconn, chn);
	zzub_player_history_commit(player, 0, 0, "Set Connection Input Count");
	return true;
}

int CConnectionPropertyProvider::getInputs() {
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (!audioconn) return 0;
	return zzub_connection_get_input_count(audioconn);
}


bool CConnectionPropertyProvider::setFirstOutput(int chn) {
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (!audioconn) return false;
	int maxvalue = zzub_plugin_get_output_channel_count(from_plugin);
	chn = std::max(chn, 0);
	chn = std::min(chn, maxvalue);
	zzub_connection_set_first_output(audioconn, chn);
	zzub_player_history_commit(player, 0, 0, "Set Connection First Output");
	return true;
}

int CConnectionPropertyProvider::getFirstOutput() {
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (!audioconn) return 0;
	return zzub_connection_get_first_output(audioconn);
}

bool CConnectionPropertyProvider::setOutputs(int chn) {
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (!audioconn) return false;
	int maxvalue = zzub_plugin_get_output_channel_count(from_plugin);
	chn = std::max(chn, 1);
	chn = std::min(chn, maxvalue);
	zzub_connection_set_output_count(audioconn, chn);
	zzub_player_history_commit(player, 0, 0, "Set Connection Output Count");
	return true;
}

int CConnectionPropertyProvider::getOutputs() {
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (!audioconn) return 0;
	return zzub_connection_get_output_count(audioconn);
}

std::string CConnectionPropertyProvider::getMidiDevice() {
	zzub_connection_t* midiconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_midi);
	if (!midiconn) return 0;
	const char* name = zzub_connection_get_midi_device(midiconn);
	if (!name) return "";
	return name;
}

std::string CConnectionPropertyProvider::getFromPluginName() {
	return zzub_plugin_get_name(from_plugin);
}

std::string CConnectionPropertyProvider::getToPluginName() {
	return zzub_plugin_get_name(to_plugin);
}

/***

	CWavePropertyProvider

***/

CWavePropertyProvider::CWavePropertyProvider(CView* returnView, CDocument* doc, zzub_wave_t* wave) {
	this->hReturnView = returnView;
	this->document = doc;
	this->player = (zzub_player_t*)buze_document_get_player(document);
	this->wave = wave;
	name = "wave";

	createProperties();
}

void CWavePropertyProvider::createProperties() {
	destroyProperties();

	properties.push_back(new CategoryPropertyInfo("Info"));
	properties.push_back(new IntPropertyInfo<CWavePropertyProvider>(this, "Wavetable Index", &CWavePropertyProvider::getWaveIndex, 0));
	properties.push_back(new CategoryPropertyInfo("Wave"));
	properties.push_back(new StringPropertyInfo<CWavePropertyProvider>(this, "Filename", &CWavePropertyProvider::getWaveFileName, &CWavePropertyProvider::setWaveFileName));
	properties.push_back(new StringPropertyInfo<CWavePropertyProvider>(this, "Name", &CWavePropertyProvider::getWaveName, &CWavePropertyProvider::setWaveName));
	properties.push_back(new IntPropertyInfo<CWavePropertyProvider>(this, "Volume", &CWavePropertyProvider::getWaveVolume, &CWavePropertyProvider::setWaveVolume));
	properties.push_back(new BoolPropertyInfo<CWavePropertyProvider>(this, "Looping", &CWavePropertyProvider::getWaveLooping, &CWavePropertyProvider::setWaveLooping));
	properties.push_back(new BoolPropertyInfo<CWavePropertyProvider>(this, "Bi-directional", &CWavePropertyProvider::getWaveBidirLoop, &CWavePropertyProvider::setWaveBidirLoop));

}

int CWavePropertyProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CWavePropertyProvider::getProperty(int index) {
	return properties[index];
}

bool CWavePropertyProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);

	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
		case zzub_event_type_insert_wavelevel:
			return zzub_wavelevel_get_wave(zzubData->insert_wavelevel.wavelevel) == wave;
		case zzub_event_type_update_wave:
			return zzubData->update_wave.wave == wave;
	}
	return false;
}

int CWavePropertyProvider::getWaveIndex() {
	return zzub_wave_get_index(wave);
}

bool CWavePropertyProvider::setWaveVolume(int vol) {
	zzub_wave_set_volume(wave, (float)vol / 16384.0f);
	zzub_player_history_commit(player, 0, 0, "Set Wave Volume");
	return true;
}

int CWavePropertyProvider::getWaveVolume() {
	return (int)(zzub_wave_get_volume(wave) * 16384);
}

bool CWavePropertyProvider::setWaveLooping(BOOL state) {
	int flags = zzub_wave_get_flags(wave);
	if (state) {
		flags |= zzub_wave_flag_loop;
	} else {
		flags ^= flags & zzub_wave_flag_loop;
	}
	zzub_wave_set_flags(wave, flags);
	zzub_player_history_commit(player, 0, 0, "Toggle Wave Looping");
	return true;
}

BOOL CWavePropertyProvider::getWaveLooping() {
	int flags = zzub_wave_get_flags(wave);
	return ((flags & zzub_wave_flag_loop) != 0) ? TRUE : FALSE;
}

bool CWavePropertyProvider::setWaveBidirLoop(BOOL state) {
	int flags = zzub_wave_get_flags(wave);
	if (state) {
		flags |= zzub_wave_flag_pingpong;
	} else {
		flags ^= flags & zzub_wave_flag_pingpong;
	}
	zzub_wave_set_flags(wave, flags);
	zzub_player_history_commit(player, 0, 0, "Toggle Wave Bidirectional Looping");
	return true;
}

BOOL CWavePropertyProvider::getWaveBidirLoop() {
	int flags = zzub_wave_get_flags(wave);
	return ((flags & zzub_wave_flag_pingpong) != 0) ? TRUE : FALSE;
}

std::string CWavePropertyProvider::getWaveFileName() {
	return zzub_wave_get_path(wave);
}

bool CWavePropertyProvider::setWaveFileName(std::string name) {
	zzub_wave_set_path(wave, name.c_str());
	zzub_player_history_commit(player, 0, 0, "Set Wave Path");
	return true;
}

std::string CWavePropertyProvider::getWaveName() {
	return zzub_wave_get_name(wave);
}

bool CWavePropertyProvider::setWaveName(std::string name) {
	zzub_wave_set_name(wave, name.c_str());
	zzub_player_history_commit(player, 0, 0, "Set Wave Name");
	return true;
}


/***

	CWaveLevelPropertyProvider

***/

CWaveLevelPropertyProvider::CWaveLevelPropertyProvider(CView* hWnd, CDocument* doc, zzub_wave_t* wave, zzub_wavelevel_t* level) {
	this->hReturnView = hWnd;
	this->document = doc;
	this->player = (zzub_player_t*)buze_document_get_player(document);
	this->wave = wave;
	this->level = level;
	this->hWnd = hWnd->GetHwnd();
	name = "wavelevel";
	createProperties();
}

void CWaveLevelPropertyProvider::createProperties() {
	destroyProperties();

	properties.push_back(new CategoryPropertyInfo("Wave Level"));
	properties.push_back(new NotePropertyInfo<CWaveLevelPropertyProvider>(hWnd, this, "Base Note", &CWaveLevelPropertyProvider::getWaveLevelBaseNote, &CWaveLevelPropertyProvider::setWaveLevelBaseNote));
	properties.push_back(new IntPropertyInfo<CWaveLevelPropertyProvider>(this, "Samples", &CWaveLevelPropertyProvider::getWaveLevelSamples, &CWaveLevelPropertyProvider::setWaveLevelSamples));
	properties.push_back(new IntPropertyInfo<CWaveLevelPropertyProvider>(this, "Begin Loop", &CWaveLevelPropertyProvider::getWaveLevelBeginLoop, &CWaveLevelPropertyProvider::setWaveLevelBeginLoop));
	properties.push_back(new IntPropertyInfo<CWaveLevelPropertyProvider>(this, "End Loop", &CWaveLevelPropertyProvider::getWaveLevelEndLoop, &CWaveLevelPropertyProvider::setWaveLevelEndLoop));
	properties.push_back(new IntPropertyInfo<CWaveLevelPropertyProvider>(this, "Samplerate", &CWaveLevelPropertyProvider::getWaveLevelSampleRate, &CWaveLevelPropertyProvider::setWaveLevelSampleRate));
	properties.push_back(new IntPropertyInfo<CWaveLevelPropertyProvider>(this, "Bits", &CWaveLevelPropertyProvider::getWaveLevelBits, &CWaveLevelPropertyProvider::setWaveLevelBits));
	properties.push_back(new IntPropertyInfo<CWaveLevelPropertyProvider>(this, "Arithmetic", &CWaveLevelPropertyProvider::getWaveLevelType, &CWaveLevelPropertyProvider::setWaveLevelType));
}

int CWaveLevelPropertyProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CWaveLevelPropertyProvider::getProperty(int index) {
	return properties[index];
}

bool CWaveLevelPropertyProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);

	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
		case zzub_event_type_insert_wavelevel:
			return zzubData->insert_wavelevel.wavelevel == level;
		case zzub_event_type_delete_wavelevel:
			return zzubData->delete_wavelevel.wavelevel == level;
		case zzub_event_type_update_wavelevel:
			return zzubData->update_wavelevel.wavelevel == level;
		case zzub_event_type_update_wavelevel_samples:
			return zzubData->update_wavelevel_samples.wavelevel == level;
		case zzub_event_type_update_wave:
			return zzubData->update_wave.wave == wave;
	}
	return false;
}

bool CWaveLevelPropertyProvider::setWaveLevelBaseNote(int note) {
	zzub_wavelevel_set_root_note(level, note);
	zzub_player_history_commit(player, 0, 0, "Set Wavelevel Root Note");
	return true;
}

int CWaveLevelPropertyProvider::getWaveLevelBaseNote() {
	return zzub_wavelevel_get_root_note(level);
}

bool CWaveLevelPropertyProvider::setWaveLevelBits(int bits) {
	assert(false);
	return true;
}

int CWaveLevelPropertyProvider::getWaveLevelBits() {
	int format = zzub_wavelevel_get_format(level);
	return bits_from_format(format);
}

bool CWaveLevelPropertyProvider::setWaveLevelEndLoop(int bits) {
	zzub_wavelevel_set_loop_end(level, bits);
	zzub_player_history_commit(player, 0, 0, "Set Wavelevel Loop End");
	return true;
}

int CWaveLevelPropertyProvider::getWaveLevelEndLoop() {
	return zzub_wavelevel_get_loop_end(level);
}

bool CWaveLevelPropertyProvider::setWaveLevelBeginLoop(int bits) {
	zzub_wavelevel_set_loop_start(level, bits);
	zzub_player_history_commit(player, 0, 0, "Set Wavelevel Loop Start");
	return true;
}

int CWaveLevelPropertyProvider::getWaveLevelBeginLoop() {
	return zzub_wavelevel_get_loop_start(level);
}

int CWaveLevelPropertyProvider::getWaveLevelSamples() {
	return zzub_wavelevel_get_sample_count(level);
}

bool CWaveLevelPropertyProvider::setWaveLevelSamples(int samples) {
	zzub_wavelevel_set_sample_count(level, samples);
	zzub_player_history_commit(player, 0, 0, "Set Wavelevel Size");
	return true;
}

bool CWaveLevelPropertyProvider::setWaveLevelType(int bits) {
	assert(false);
	return true;
}

int CWaveLevelPropertyProvider::getWaveLevelType() {
	int format = zzub_wavelevel_get_format(level);
	// TODO: convert to something usable
	return format;
}

bool CWaveLevelPropertyProvider::setWaveLevelSampleRate(int rate) {
	zzub_wavelevel_set_samples_per_second(level, rate);
	zzub_player_history_commit(player, 0, 0, "Set Wavelevel Samplerate");
	return true;
}

int CWaveLevelPropertyProvider::getWaveLevelSampleRate() {
	return zzub_wavelevel_get_samples_per_second(level);
}
