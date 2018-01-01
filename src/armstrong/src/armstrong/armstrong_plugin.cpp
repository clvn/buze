#include <cstring>
#include <cstdio>
#define snprintf _snprintf
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <complex>
#include "library.h"
#include "player.h"
#include "archive.h"
#include "midinames.h"
#include "mixing/mixer.h"
#include "mixing/connections.h"

using namespace armstrong::frontend;
using std::vector;
using std::string;

extern "C" {

int zzub_plugin_destroy(zzub_plugin_t *plugin) {
	// save in order for the plugin to load last state on undo
	std::vector<zzub::newplugindata>::iterator newplug = std::find_if(plugin->owner->mix->new_plugins.begin(), plugin->owner->mix->new_plugins.end(), zzub::find_new_plugin(plugin->dataplugin->id));
	// if the plugin is in the new_plugins-vector, it's not initialized yet, and shouldn't be saved
	if (newplug == plugin->owner->mix->new_plugins.end()) {
		zzub_plugin_save(plugin, 0);
	}
	plugin->dataplugin->destroy();
	return 0;
}

/** \brief Load plugin state. */
int zzub_plugin_load(zzub_plugin_t *plugin, zzub_input_t *input) {

	mem_archive* arc = new mem_archive();
	zzub::outstream* outs = arc->get_outstream("");

	//vector<char> bytes(input->size());
	plugin->dataplugin->data.resize(input->size());
	input->read(&plugin->dataplugin->data.front(), (int)plugin->dataplugin->data.size());
	outs->write(&plugin->dataplugin->data.front(), (int)plugin->dataplugin->data.size());

	plugin->userplugin->load(arc);

	delete arc;

	plugin->dataplugin->update();
	return -1;
}

/** \brief Save plugin state. */
int zzub_plugin_save(zzub_plugin_t *plugin, zzub_output_t *ouput) {

	// disable db events while saving; this updates the db with values from the mixer which don't need to be sent back
	bool listener_state = plugin->owner->doc->listener_enabled;
	plugin->owner->doc->listener_enabled = false;

	// update plugin parameters in the storage. because, often times sliders/parameters have 
	// changed but are not persisted in the db. such as parameter changes during playback, or
	// song tempo changes -> sequence plugin parameter changes.
	for (std::vector<boost::shared_ptr<pluginparameter> >::iterator i = plugin->owner->pluginparameters.begin(); i != plugin->owner->pluginparameters.end(); ++i) {
		pluginparameter* pp = i->get();
		if (pp != 0 && pp->datapluginparameter->plugin_id == plugin->dataplugin->id) {
			parameterinfo* param = plugin->owner->parameterinfos[pp->datapluginparameter->parameterinfo_id].get();
			if (param->dataparameterinfo->flags & zzub_parameter_flag_state) pp->datapluginparameter->update();
		}
	}

	// save the plugins custom chunk
	mem_archive* arc = new mem_archive();
	plugin->userplugin->save(arc);

	zzub::instream* ins = arc->get_instream("");
	if (ins && ins->size()) {
		plugin->dataplugin->data.resize(ins->size());
		//vector<char> bytes(ins->size());
		ins->read(&plugin->dataplugin->data.front(), (int)plugin->dataplugin->data.size());
		if (ouput != 0)
			ouput->write(&plugin->dataplugin->data.front(), (int)plugin->dataplugin->data.size());
	}

	delete arc;

	bool success = plugin->dataplugin->update();
	assert(success);

	plugin->owner->doc->listener_enabled = listener_state;
	return -1;
}

void zzub_plugin_command(zzub_plugin_t *plugin, int i) {
	plugin->userplugin->command(i);
}

int zzub_plugin_set_name(zzub_plugin_t *plugin, const char* name) {
	plugin->dataplugin->name = name;
	plugin->dataplugin->update();
	return 0;
}

const char* zzub_plugin_get_name(zzub_plugin_t *plugin) {
	static char name[256];
	strncpy(name, plugin->dataplugin->name.c_str(), 256);
	return name;
}

void zzub_plugin_configure(zzub_plugin_t *plugin, const char *key, const char *value) {
	plugin->userplugin->configure(key, value);
}

int zzub_plugin_get_id(zzub_plugin_t* plugin) {
	return plugin->dataplugin->id;
}

int zzub_plugin_get_commands(zzub_plugin_t *plugin, char* commands, int maxlen) {

	strncpy(commands, plugin->loader->commands.c_str(), maxlen);
	return strlen(commands);
}

int zzub_plugin_get_sub_commands(zzub_plugin_t *plugin, int i, char* commands, int maxlen) {
// if a command string starts with the char '\', it has subcommands
// unexpectedly, this returns a \n-separated string (like getCommands())
// some machines need to be ticked before calling getSubCommands (not yet supported)

	vector<char> bytes;
	mem_outstream outm(bytes);
	zzub::outstream* outf = &outm;

	plugin->userplugin->get_sub_menu(i, outf);
	outf->write((char)0);	// terminate array

	// create a new \n-separated string and return it instead, means both getCommands() and getSubCommands() return similar formatted strings
	const char* firstp = &bytes.front();
	string ret = "";

	while (*firstp) {
		if (ret.length() > 0)
			ret += "\n";
		ret += firstp;
		firstp += strlen(firstp)+1;
	}

	if (commands != strncpy(commands, ret.c_str(), maxlen)) {
		// too many bytes, clear string pls
		strcat(commands, "");
	}
	return strlen(commands);
}

int zzub_plugin_get_midi_output_device_count(zzub_plugin_t *plugin) {

	static _midiouts midiouts;
	midiouts.clear();
	plugin->userplugin->get_midi_output_names(&midiouts);
	return midiouts.names.size();
}

const char* zzub_plugin_get_midi_output_device(zzub_plugin_t *plugin, int index) {

	static _midiouts midiouts;
	midiouts.clear();
	plugin->userplugin->get_midi_output_names(&midiouts);
	return midiouts.names[index].c_str();
}

int zzub_plugin_get_envelope_count(zzub_plugin_t *plugin) {

	const zzub::envelope_info** infos = plugin->userplugin->get_envelope_infos();
	if (!infos) return 0;

	int count = 0;
	while (*infos) { count++; infos++; }
	return count;
}

const zzub::envelope_info* get_envelope_info(zzub_plugin_t *plugin, int index) {

	const zzub::envelope_info** infos = plugin->userplugin->get_envelope_infos();
	if (!infos) return 0;

	int count = 0;
	while (*infos && count < index) { count++; infos++; }
	return *infos;
}

int zzub_plugin_get_envelope_flags(zzub_plugin_t *plugin, int index) {
	const zzub::envelope_info* info = get_envelope_info(plugin, index);
	return info->flags;
}

const char* zzub_plugin_get_envelope_name(zzub_plugin_t *plugin, int index) {
	const zzub::envelope_info* info = get_envelope_info(plugin, index);
	return info->name;
}

const char* zzub_plugin_get_stream_source(zzub_plugin_t *plugin) {
	return plugin->dataplugin->streamsource.c_str();
}

void zzub_plugin_set_stream_source(zzub_plugin_t *plugin, const char* resource) {
	plugin->dataplugin->streamsource = resource;
	plugin->dataplugin->update();
	//plugin->_player->plugin_set_stream_source(plugin->dataplugin->id, resource);
}

int zzub_plugin_get_flags(zzub_plugin_t *plugin) {

	return plugin->loader->flags | plugin->dataplugin->flags;
}

zzub_pluginloader_t *zzub_plugin_get_pluginloader(zzub_plugin_t *plugin) {
	assert(plugin != 0);
	return plugin->loader;
}

int zzub_plugin_get_parameter_count(zzub_plugin_t *plugin, int group, int track) {
	return zzub_pluginloader_get_parameter_count(plugin->loader, group);
}

zzub_parameter_t* zzub_plugin_get_parameter(zzub_plugin_t *plugin, int group, int track, int column) {
	return zzub_pluginloader_get_parameter(plugin->loader, group, column);
}

int zzub_plugin_get_parameter_value(zzub_plugin_t *plugin, int group, int track, int column) {

	armstrong::frontend::pluginparameter* plugparam = plugin->owner->get_pluginparameter(plugin->dataplugin->id, group, track, column);
	if (plugparam != 0) 
		return plugparam->datapluginparameter->value;

	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, track, column);
	if (param->flags & zzub_parameter_flag_state)
		return param->value_default; else
		return param->value_none;
}

void zzub_plugin_set_parameter_value(zzub_plugin_t *plugin, int group, int track, int column, int value, int record) {
	assert(group == 0 || group == 1 || group == 2);
	plugin->dataplugin->set_parameter_value(group, track, column, value);
}

void zzub_plugin_set_parameter_value_direct(zzub_plugin_t *plugin, int group, int track, int column, int value, int record) {
	plugin->owner->mix->alter_parameter(plugin->dataplugin->id, group, track, column, value, record);
}

float zzub_plugin_get_position_x(zzub_plugin_t *plugin) {
	return (float)plugin->dataplugin->x;
}

float zzub_plugin_get_position_y(zzub_plugin_t *plugin) {
	return (float)plugin->dataplugin->y;
}

void zzub_plugin_set_position(zzub_plugin_t *plugin, float x, float y) {
	plugin->dataplugin->x = x;
	plugin->dataplugin->y = y;
	plugin->dataplugin->update();
}

void zzub_plugin_set_position_direct(zzub_plugin_t *plugin, float x, float y) {
	plugin->dataplugin->x = x;
	plugin->dataplugin->y = y;
	plugin->dataplugin->update();
}

int zzub_plugin_get_input_connection_count(zzub_plugin_t *plugin) {
	return plugin->dataplugin->get_input_connection_count();
}

zzub_connection_t* zzub_plugin_get_input_connection(zzub_plugin_t *plugin, int index) {
	armstrong::storage::connection conn(plugin->owner->doc.get());
	if (!plugin->dataplugin->get_input_connection_by_index(index, conn)) return 0;
	return plugin->owner->connections[conn.id].get();
}

zzub_connection_t* zzub_plugin_get_input_connection_by_type(zzub_plugin_t *to_plugin, zzub_plugin_t* from_plugin, int type) {
	armstrong::storage::connection conn(to_plugin->owner->doc.get());
	if (!to_plugin->dataplugin->get_input_connection_by_type(*from_plugin->dataplugin, type, conn)) return 0;
	return to_plugin->owner->connections[conn.id].get();
}

int zzub_plugin_get_output_connection_count(zzub_plugin_t *plugin) {
	return plugin->dataplugin->get_output_connection_count();
}

zzub_connection_t* zzub_plugin_get_output_connection_by_type(zzub_plugin_t *to_plugin, zzub_plugin_t* from_plugin, int type) {
	armstrong::storage::connection conn(to_plugin->owner->doc.get());
	if (!to_plugin->dataplugin->get_output_connection_by_type(*from_plugin->dataplugin, type, conn)) return 0;
	return to_plugin->owner->connections[conn.id].get();
}

zzub_connection_t* zzub_plugin_get_output_connection(zzub_plugin_t* plugin, int index) {
	armstrong::storage::connection conn(plugin->owner->doc.get());
	if (!plugin->dataplugin->get_output_connection_by_index(index, conn)) return 0;
	return plugin->owner->connections[conn.id].get();
}

float zzub_plugin_get_last_peak(zzub_plugin_t *plugin, int channel) {
	if (plugin->dataplugin->id >= (int)plugin->owner->mix->plugins.next().size() || plugin->owner->mix->plugins.next()[plugin->dataplugin->id].get() == 0) {
		return 0.0f;
	}
	return plugin->owner->mix->plugins.next()[plugin->dataplugin->id]->audiodata->last_work_max[channel];
}

zzub_plugin_t* create_connection_plugin(zzub_player_t* player, zzub_plugin_group_t* plugingroup, std::string uri) {
	zzub_plugin_t* connplug;
	zzub::info* conninfo;
	
	conninfo = zzub_player_get_pluginloader_by_name(player, uri.c_str());
	assert(conninfo != 0);

	const char* name = zzub_player_get_new_plugin_name(player, uri.c_str());
	connplug = zzub_player_create_plugin(player, 0, 0, name, conninfo, plugingroup);
	assert(connplug != 0);
	return connplug;
}

bool can_connect(zzub_plugin_t* to_plugin, int to_req_flags, zzub_plugin_t *from_plugin, int from_req_flags, int type, bool allow_cyclic) {
	if (to_plugin == from_plugin) return false; // cannot connect to self
	
	zzub_connection_t* audioconn = zzub_plugin_get_input_connection_by_type(to_plugin, from_plugin, zzub_connection_type_audio);
	if (audioconn != 0) return false; // already exists

	if (type == zzub_connection_type_note) {
		zzub::metaplugin* mpl = to_plugin->owner->mix->plugins.next()[to_plugin->dataplugin->id].get();
		if (mpl->info->note_group == -1) return false;
	}

	if (!allow_cyclic) {
		// TODO: test against current graph if this connection would cause a feedback loop
	}

	int to_flags = zzub_plugin_get_flags(to_plugin);
	int from_flags = zzub_plugin_get_flags(from_plugin);
	
	bool can_to = (to_flags & to_req_flags) == to_req_flags;
	bool can_from = (from_flags & from_req_flags) == from_req_flags;

	return can_to && can_from;
}

zzub_connection_t *zzub_plugin_create_audio_connection(zzub_plugin_t *plugin, zzub_plugin_t *from_plugin, int first_input, int input_count, int first_output, int output_count) {
	if (!can_connect(plugin, zzub_plugin_flag_has_audio_input, from_plugin, zzub_plugin_flag_has_audio_output, zzub_connection_type_audio, true))
		return false;

	zzub_plugin_group_t* plugingroup = zzub_plugin_get_plugin_group(plugin);
	zzub_plugin_t* connplug = create_connection_plugin(plugin->owner, plugingroup, "@zzub.org/connection/audio");
	armstrong::storage::connection conndata(plugin->dataplugin->owner);

	if (first_input == -1)
		first_input = 0;
	if (input_count == -1)
		input_count = plugin->loader->inputs;
	if (first_output == -1)
		first_output = 0;
	if (output_count == -1)
		output_count = from_plugin->loader->outputs;

	bool result = plugin->dataplugin->add_audio_input(connplug->dataplugin->id, *from_plugin->dataplugin, 
		first_input, input_count, first_output, output_count, conndata);
	assert(result);

	zzub_plugin_set_track_count(connplug, from_plugin->loader->outputs);
	return plugin->owner->connections[conndata.id].get();
}

zzub_connection_t *zzub_plugin_create_midi_connection(zzub_plugin_t *plugin, zzub_plugin_t *from_plugin, const char *midi_device) {
	if (!can_connect(plugin, zzub_plugin_flag_has_midi_input, from_plugin, zzub_plugin_flag_has_midi_output, zzub_connection_type_midi, false))
		return false;

	zzub_plugin_group_t* plugingroup = zzub_plugin_get_plugin_group(plugin);
	zzub_plugin_t* connplug = create_connection_plugin(plugin->owner, plugingroup, "@zzub.org/connection/midi");
	armstrong::storage::connection conndata(plugin->dataplugin->owner);
	plugin->dataplugin->add_midi_input(connplug->dataplugin->id, *from_plugin->dataplugin, 
		midi_device, conndata);
	return plugin->owner->connections[conndata.id].get();
}

zzub_connection_t *zzub_plugin_create_event_connection(zzub_plugin_t *plugin, zzub_plugin_t *from_plugin) {
	if (!can_connect(plugin, 0, from_plugin, zzub_plugin_flag_has_event_output, zzub_connection_type_event, false))
		return false;

	zzub_plugin_group_t* plugingroup = zzub_plugin_get_plugin_group(plugin);
	zzub_plugin_t* connplug = create_connection_plugin(plugin->owner, plugingroup, "@zzub.org/connection/event");
	armstrong::storage::connection conndata(plugin->dataplugin->owner);
	plugin->dataplugin->add_event_input(connplug->dataplugin->id, *from_plugin->dataplugin, conndata);
	return plugin->owner->connections[conndata.id].get();
}

zzub_connection_t *zzub_plugin_create_note_connection(zzub_plugin_t *plugin, zzub_plugin_t *from_plugin) {
	if (!can_connect(plugin, 0, from_plugin, zzub_plugin_flag_has_note_output, zzub_connection_type_note, false))
		return false;

	zzub_plugin_group_t* plugingroup = zzub_plugin_get_plugin_group(plugin);
	zzub_plugin_t* connplug = create_connection_plugin(plugin->owner, plugingroup, "@zzub.org/connection/note");
	armstrong::storage::connection conndata(plugin->dataplugin->owner);
	plugin->dataplugin->add_note_input(connplug->dataplugin->id, *from_plugin->dataplugin, conndata);
	return plugin->owner->connections[conndata.id].get();
}

int zzub_plugin_get_track_count(zzub_plugin_t *plugin, int group) {
	switch (group) {
		case 0:
		case 1:
		case 3:
		case 4:
			return 1;
		case 2:
			return plugin->dataplugin->trackcount;
		default:
			assert(false);
			return 0;
	}
}

void zzub_plugin_set_track_count(zzub_plugin_t *plugin, int tracks) {
	plugin->dataplugin->trackcount = tracks;
	plugin->dataplugin->update();
    
	plugin->owner->ensure_pluginparameters_exist(plugin->dataplugin->id);
}

// TODO: get rid of this function! the only place it was used in buze is fixed! still one left here
int zzub_plugin_pattern_to_linear_no_connections(zzub_plugin_t *plugin, int group, int track, int column, int* index) {
	const zzub::info* info = plugin->loader;

	switch (group) {
		case 0:
			return 6000 + column;
		case 1:
			*index = column;
			return 1;
		case 2:
			*index = info->global_parameters.size() + track * info->track_parameters.size() + column;
			return 1;
		case 3:
			return 0;
		default:
			assert(false);
			return 0;
	}
	//return player->plugin_pattern_to_linear(plugin, group, track, column, *index);
}

int getNoValue(const zzub::parameter* para) {
	switch (para->type) {
		case zzub_parameter_type_switch:
			return zzub_switch_value_none;
		case zzub_parameter_type_note:
			return zzub_note_value_none;
		default:
			return para->value_none;
	}
}

float linear_to_dB(float val) { 
	return(20.0f * log10(val)); 
}

const char* zzub_plugin_describe_value(zzub_plugin_t *plugin, int group, int column, int value) {
	if (group == 0 || group == 3 || group == 4)
		return "";

	// TODO: query the plugin by group, track, column, and let the buzz2zzub deal with this directly
	int index = -1;
	zzub_plugin_pattern_to_linear_no_connections(plugin, group, 0, column, &index);

	zzub::metaplugin* k = plugin->owner->mix->plugins.next()[plugin->dataplugin->id].get();
	const zzub::parameter* para = k->get_parameter_info(group, 0, column);
	if (index != -1) {
		if (value != getNoValue(para)) {	// infector crashen when trying to describe novalues (and out-of-range-values)
			return k->plugin->describe_value(index, value);
		}
	}
	return 0;
}

int zzub_plugin_get_mute(zzub_plugin_t *plugin) {
	return zzub_plugin_get_parameter_value(plugin, 0, 0, 0);
}

void zzub_plugin_set_mute(zzub_plugin_t *plugin, int muted) {
	zzub_plugin_set_parameter_value(plugin, 0, 0, 0, muted ? 1 : 0, false);
}

int zzub_plugin_get_bypass(zzub_plugin_t *plugin) {
	return zzub_plugin_get_parameter_value(plugin, 0, 0, 1);
}

void zzub_plugin_set_bypass(zzub_plugin_t *plugin, int muted) {
	zzub_plugin_set_parameter_value(plugin, 0, 0, 1, muted ? 1 : 0, false);
}

int zzub_plugin_get_minimize(zzub_plugin_t *plugin) {
	return plugin->dataplugin->is_minimized ? 1 : 0;
}

void zzub_plugin_set_minimize(zzub_plugin_t *plugin, int minimized) {
	plugin->dataplugin->is_minimized = minimized != 0 ? true : false;
	plugin->dataplugin->update();
}

double zzub_plugin_get_last_cpu_load(zzub_plugin_t *plugin) {
	if (plugin->dataplugin->id >= (int)plugin->owner->mix->plugins.next().size() || plugin->owner->mix->plugins.next()[plugin->dataplugin->id].get() == 0) return 0.0f;
	return plugin->owner->mix->plugins.next()[plugin->dataplugin->id].get()->audiodata->cpu_load;
}

int zzub_plugin_get_last_audio_result(zzub_plugin_t *plugin) {
	if (plugin->dataplugin->id >= (int)plugin->owner->mix->plugins.next().size() || plugin->owner->mix->plugins.next()[plugin->dataplugin->id].get() == 0) return 0;
	return plugin->owner->mix->plugins.next()[plugin->dataplugin->id].get()->audiodata->last_work_audio_result?1:0;
}

int zzub_plugin_get_last_midi_result(zzub_plugin_t *plugin) {
	if (plugin->dataplugin->id >= (int)plugin->owner->mix->plugins.next().size() || plugin->owner->mix->plugins.next()[plugin->dataplugin->id].get() == 0) return 0;
	return plugin->owner->mix->plugins.next()[plugin->dataplugin->id].get()->audiodata->last_work_midi_result?1:0;
}

void zzub_plugin_tick(zzub_plugin_t *plugin, int immediate) {
	if (immediate == 0) {
		// mode 0 -> immediate_mode = false -> commit_event
		plugin->owner->mix->process_events(plugin->dataplugin->id, false);
	} else
	if (immediate == 1) {
		// mode 1 -> immediate_mode = true -> audio_event
		plugin->owner->mix->process_events(plugin->dataplugin->id, true);
	} else
	if (immediate == 2) {
		// mode 2 -> write value to mixer right away, only allowed on a plugin after create_plugin() and before barrier()
		zzub::metaplugin* k = plugin->owner->mix->plugins.next()[plugin->dataplugin->id].get();
		plugin->owner->mix->on_process_events(k);
	}
}

int zzub_plugin_get_attribute_value(zzub_plugin_t *plugin, int index) {
	return plugin->dataplugin->get_attribute(index);
}

void zzub_plugin_set_attribute_value(zzub_plugin_t *plugin, int index, int value) {
	plugin->dataplugin->set_attribute(index, value);
}

void zzub_plugin_play_midi_note(zzub_plugin_t *plugin, int note, int prevNote, int velocity) {
	plugin->owner->mix->play_plugin_note(plugin->dataplugin->id, note, prevNote, velocity);
}

int zzub_plugin_get_pattern_column_count(zzub_plugin_t *plugin) {
	const zzub::info* info = plugin->loader;

	return info->global_parameters.size() + info->track_parameters.size() * plugin->dataplugin->trackcount;
}

int zzub_plugin_set_instrument(zzub_plugin_t *plugin, const char *name) {
	return plugin->userplugin->set_instrument(name) ? 0 : -1;
}

int zzub_plugin_set_midi_connection_device(zzub_plugin_t *to_plugin, zzub_plugin_t* from_plugin, const char* name) {

	armstrong::storage::connection conn(to_plugin->dataplugin->owner);
	bool result = to_plugin->dataplugin->get_input_connection_by_type(*from_plugin->dataplugin, zzub_connection_type_midi, conn);
	assert(result);

	conn.set_midi_device(name);
	return 0;
}

zzub_connection_t* zzub_plugin_get_connection(zzub_plugin_t* plugin) {
	armstrong::storage::connectiondata conndata;
	if (plugin->dataplugin->get_connection(conndata)) {
		return plugin->owner->connections[conndata.id].get();
	}
	return 0;
}

zzub_plugin_t *zzub_plugin_get_timesource_plugin(zzub_plugin_t *plugin) {
	return zzub_player_get_plugin_by_id(plugin->owner, plugin->dataplugin->timesource_plugin_id);
}

void zzub_plugin_set_timesource(zzub_plugin_t *plugin, zzub_plugin_t *timesource, int group, int track) {
	if (timesource != 0) {
		assert((timesource->loader->flags & zzub_plugin_flag_is_sequence) != 0);
		plugin->dataplugin->timesource_plugin_id = timesource->dataplugin->id; 
		plugin->dataplugin->timesource_group = group;
		plugin->dataplugin->timesource_track = track;
	} else {
		plugin->dataplugin->timesource_plugin_id = -1;
		plugin->dataplugin->timesource_group = -1;
		plugin->dataplugin->timesource_track = -1;
	}
	plugin->dataplugin->update();
}

int zzub_plugin_get_timesource_group(zzub_plugin_t *plugin) {
	return plugin->dataplugin->timesource_group;
}

int zzub_plugin_get_timesource_track(zzub_plugin_t *plugin) {
	return plugin->dataplugin->timesource_track;
}

int zzub_plugin_get_input_channel_count(zzub_plugin_t* plugin) {
	int channelcount = plugin->userplugin->get_input_channel_count();
	if (channelcount == -1) return plugin->loader->inputs;
	return channelcount;
}

int zzub_plugin_get_output_channel_count(zzub_plugin_t* plugin) {
	int channelcount = plugin->userplugin->get_output_channel_count();
	if (channelcount == -1) return plugin->loader->outputs;
	return channelcount;
}

const char* zzub_plugin_get_input_channel_name(zzub_plugin_t* plugin, int index) {
	return plugin->userplugin->get_input_channel_name(index);
}

const char* zzub_plugin_get_output_channel_name(zzub_plugin_t* plugin, int index) {
	return plugin->userplugin->get_output_channel_name(index);
}

int zzub_plugin_get_encoder_digest(zzub_plugin_t* plugin, int type, float** buffers, int numsamples) { 
	return plugin->userplugin->get_encoder_digest(type, buffers, numsamples); 
}

/** \brief Returns the parameter interpolation mode.
	Mode 0 = absolute, 1 = inertial, 2 = linear */
int zzub_plugin_get_parameter_interpolator(zzub_plugin_t *plugin, int group, int track, int column) {

	pluginparameter* param = plugin->owner->get_pluginparameter(plugin->dataplugin->id, group, track, column);
	return param->datapluginparameter->interpolator;
}

/** \brief Sets the parameter interpolation mode.
	Mode 0 = absolute, 1 = inertial, 2 = linear */
void zzub_plugin_set_parameter_interpolator(zzub_plugin_t *plugin, int group, int track, int column, int mode) {
	assert(mode == 0 || mode == 1 || mode == 2);
	pluginparameter* param = plugin->owner->get_pluginparameter(plugin->dataplugin->id, group, track, column);
	param->datapluginparameter->interpolator = mode;
	param->datapluginparameter->update();
}

int zzub_plugin_has_embedded_gui(zzub_plugin_t *plugin) {
	return plugin->userplugin->has_embedded_gui() ? 1 : 0;
}

int zzub_plugin_create_embedded_gui(zzub_plugin_t *plugin, void *hwnd) {
	return plugin->userplugin->create_embedded_gui(hwnd) ? 1 : 0;
}

void zzub_plugin_resize_embedded_gui(zzub_plugin_t *plugin, void *hwnd, int* outwidth, int* outheight) {
	plugin->userplugin->resize_embedded_gui(hwnd, outwidth, outheight);
}

void zzub_plugin_set_latency(zzub_plugin_t *plugin, int latency) {
	plugin->dataplugin->latency = latency;
	plugin->dataplugin->update();
}

int zzub_plugin_get_latency(zzub_plugin_t *plugin) {
	return plugin->dataplugin->latency;
}

int zzub_plugin_get_latency_actual(zzub_plugin_t *plugin) {
	// TODO: get from metaplugin if dataplugin says -1
	return plugin->dataplugin->latency;
}

zzub_plugin_group_t* zzub_plugin_get_plugin_group(zzub_plugin_t* plugin) {
	if (plugin->dataplugin->plugingroup_id < (int)plugin->owner->plugingroups.size())
		return plugin->owner->plugingroups[plugin->dataplugin->plugingroup_id].get();
	else
		return 0;
}

void zzub_plugin_set_plugin_group(zzub_plugin_t* plugin, zzub_plugin_group_t* plugingroup) {

	if ((plugin->loader->flags & zzub_plugin_flag_is_connection) == 0) {
		// not a connection plugin: also set plugingroup on input connection plugins to prevent confusion later when saving
		for (int i = 0; i < zzub_plugin_get_input_connection_count(plugin); i++) {
			zzub_connection_t* conn = zzub_plugin_get_input_connection(plugin, i);
			zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(conn);
			zzub_plugin_set_plugin_group(connplug, plugingroup);
		}
	}

	int plugingroup_id = (plugingroup != 0) ? plugingroup->dataplugingroup->id : 0;
	plugin->dataplugin->plugingroup_id = plugingroup_id;
	plugin->dataplugin->update();
}

/* class PluginIterator */

/** \brief Iterate to the next item. */
void zzub_plugin_iterator_next(zzub_plugin_iterator_t *pluginiterator) {
	pluginiterator->recordset->next();
}

/** \brief Returns true if the iterator is valid and it is safe to call current() */
int zzub_plugin_iterator_valid(zzub_plugin_iterator_t *pluginiterator) {
	return pluginiterator->recordset->eof() ? 0 : 1;
}

/** \brief Returns the current item. */
zzub_plugin_t *zzub_plugin_iterator_current(zzub_plugin_iterator_t *pluginiterator) {
	static armstrong::storage::plugindata pev;
	return pluginiterator->owner->plugins[pluginiterator->recordset->id()].get();
}

/** \brief Destroys the iterator. */
void zzub_plugin_iterator_destroy(zzub_plugin_iterator_t *pluginiterator) {
	pluginiterator->recordset->destroy();
	//delete patterneventiterator->recordset;
	delete pluginiterator;
}

/* class Connection */

void zzub_connection_add_event_connection_binding(zzub_connection_t *connection, int sourceparam, int targetgroup, int targettrack, int targetparam) {
	connection->dataconn->add_event_binding(sourceparam, targetgroup, targettrack, targetparam);
}

void zzub_connection_remove_event_connection_binding(zzub_connection_t *connection, int sourceparam, int targetgroup, int targettrack, int targetparam) {
	connection->dataconn->delete_event_binding(sourceparam, targetgroup, targettrack, targetparam);
}

int zzub_connection_get_event_binding_count(zzub_connection_t *connection) {
	return connection->dataconn->get_event_binding_count();
}

zzub_connection_binding_iterator_t* zzub_connection_get_event_binding_iterator(zzub_connection_t *connection) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();

	connection->dataconn->get_event_bindings(it);

	armstrong::frontend::eventconnectionbindingiterator* frontendit = new armstrong::frontend::eventconnectionbindingiterator();
	frontendit->owner = connection->owner;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
}

int zzub_connection_get_first_input(zzub_connection_t *connection) {
	return connection->dataconn->first_input;
}

void zzub_connection_set_first_input(zzub_connection_t *connection, int value) {
	connection->dataconn->first_input = value;
	connection->dataconn->update();
}

int zzub_connection_get_input_count(zzub_connection_t *connection) {
	return connection->dataconn->input_count;
}

void zzub_connection_set_input_count(zzub_connection_t *connection, int value) {
	connection->dataconn->input_count = value;
	connection->dataconn->update();
}

int zzub_connection_get_first_output(zzub_connection_t *connection) {
	return connection->dataconn->first_output;
}

void zzub_connection_set_first_output(zzub_connection_t *connection, int value) {
	connection->dataconn->first_output = value;
	connection->dataconn->update();
}

int zzub_connection_get_output_count(zzub_connection_t *connection) {
	return connection->dataconn->output_count;
}

void zzub_connection_set_output_count(zzub_connection_t *connection, int value) {
	connection->dataconn->output_count = value;
	connection->dataconn->update();

	zzub_plugin_set_track_count(connection->owner->plugins[connection->dataconn->plugin_id].get(), value);
}

void zzub_connection_set_midi_device(zzub_connection_t *connection, const char *midi_device) {
	connection->dataconn->mididevice = midi_device;
	connection->dataconn->update();
}

const char *zzub_connection_get_midi_device(zzub_connection_t *connection) {
	return connection->dataconn->mididevice.c_str();
}

/* class ConnectionBinding */

/** \brief Returns the owner connection of this event binding. */
zzub_connection_t *zzub_connection_binding_get_connection(zzub_connection_binding_t *connectionbinding) {
	int connid = connectionbinding->databinding->connection_id;
	return connectionbinding->owner->connections[connid].get();
}

/** \brief Returns the parameter in group 3 on the source plugin being mapped. */
int zzub_connection_binding_get_source_column(zzub_connection_binding_t *connectionbinding) {
	return connectionbinding->databinding->sourceindex;
}

/** \brief Returns the target parameter group. */
int zzub_connection_binding_get_target_group(zzub_connection_binding_t *connectionbinding) {
	return connectionbinding->databinding->targetparamgroup;
}

/** \brief Returns the target parameter track. */
int zzub_connection_binding_get_target_track(zzub_connection_binding_t *connectionbinding) {
	return connectionbinding->databinding->targetparamtrack;
}

/** \brief Returns the target parameter column. */
int zzub_connection_binding_get_target_column(zzub_connection_binding_t *connectionbinding) {
	return connectionbinding->databinding->targetparamcolumn;
}

/** \brief Iterates to the next item. */
void zzub_connection_binding_iterator_next(zzub_connection_binding_iterator_t *iterator) {
	iterator->recordset->next();
}

/** \brief Returns true if the iterator is valid and it is safe to call current() */
int zzub_connection_binding_iterator_valid(zzub_connection_binding_iterator_t *iterator) {
	return iterator->recordset->eof() ? 0 : 1;
}

/** \brief Returns the current item. */
zzub_connection_binding_t *zzub_connection_binding_iterator_current(zzub_connection_binding_iterator_t *iterator) {
	//static armstrong::storage::eventconnectionbinding pev;
	return iterator->owner->eventconnectionbindings[iterator->recordset->id()].get();
}

/** \brief Resets the iterator. */
void zzub_connection_binding_iterator_reset(zzub_connection_binding_iterator_t *iterator) {
	iterator->recordset->reset();
}

/** \brief Destroys the iterator. */
void zzub_connection_binding_iterator_destroy(zzub_connection_binding_iterator_t *iterator) {
	iterator->recordset->destroy();
	delete iterator;
}

void zzub_connection_destroy(zzub_connection_t* connection) {
	// connection trigger deletes connection plugin
	connection->dataconn->destroy();
}

zzub_plugin_t* zzub_connection_get_from_plugin(zzub_connection_t* connection) {
	return connection->owner->plugins[connection->dataconn->from_plugin_id].get();
}

zzub_plugin_t* zzub_connection_get_to_plugin(zzub_connection_t* connection) {
	return connection->owner->plugins[connection->dataconn->to_plugin_id].get();
}

zzub_plugin_t* zzub_connection_get_connection_plugin(zzub_connection_t* connection) {
	return connection->owner->plugins[connection->dataconn->plugin_id].get();
}

int zzub_connection_get_type(zzub_connection_t* connection) {
	return connection->dataconn->type;
}

}
