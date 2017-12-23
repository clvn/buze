#include <algorithm>
#include <list>
#include <stack>
#include <iostream>
#include <vector>
#include <cstring>
#include "player.h"
#include "mixing/mixer.h"
#include "mixing/connections.h"
#include "archive.h"

namespace armstrong {

namespace frontend {

//
// dummyplugin
//

struct dummyplugin : zzub::plugin {
	const zzub::info* info;
	std::vector<char> globalbytes;
	std::vector<char> trackbytes;
	std::vector<char> controllerbytes;
	std::vector<int> attrbytes;
	std::vector<char> data;

	dummyplugin(const zzub::info* _info) {
		info = _info;
		globalbytes.resize(1024);
		trackbytes.resize(1024*16);
		controllerbytes.resize(1024);
		attrbytes.resize(128);
		global_values = &globalbytes.front();
		track_values = &trackbytes.front();
		controller_values = &controllerbytes.front();
		attributes = &attrbytes.front();
	}

	~dummyplugin() {
	}

	void init(zzub::archive* arc) {
		// we assume player::create_plugin gives us an archive which contains the
		// data bytes only (and not the remainder of e.g the .bmx)
		if (!arc) return ;
		zzub::instream* inf = arc->get_instream("");
		data.resize(inf->size());
		inf->read(&data.front(), (int)data.size());
	}

	void load(zzub::archive* arc) {
		if (!arc) return ;
		zzub::instream* inf = arc->get_instream("");
		if (!inf->size()) return ;
		data.resize(inf->size());
		inf->read(&data.front(), (int)data.size());
	}

	void save(zzub::archive* arc) {
		if (!arc) return ;
		if (data.empty()) return ;
		zzub::outstream* outf = arc->get_outstream("");
		outf->write(&data.front(), (int)data.size());
	}

	int get_interval_size() {
		return _master_info->samples_per_tick;
	}
};

//
// dummycollection
//

dummycollection::~dummycollection() {}

const char* dummycollection::get_name() { 
	return "Dummy"; 
}

void dummycollection::destroy() {
	// override plugincollection::destroy()'s default "delete this"-behavior: 
	// the dummycollection instance cant be deleted
	
	for (std::vector<dummyinfo*>::iterator i = dummyinfos.begin(); i != dummyinfos.end(); ++i) {
		dummyinfo* info = *i;
		delete info;
	}
	dummyinfos.clear();
}

dummyinfo* dummycollection::register_dummy(armstrong::frontend::player* player, dummyinfo* info) {
	info->collection = this;
	info->internal_parameters = &player->plugmgr.internal_parameters;
	player->plugmgr.find_known_columns(info);
	player->plugmgr.create_virtual_parameters(info);

	dummyinfos.push_back(info);
	return info;
}

// dummyinfo inherits from zzub::info which frees its parameters in the destructor and thus owns its parameters. must be cloned in case a copy is needed. the caller must free the returned dummyinfo.
dummyinfo* dummycollection::clone(player* owner, dummyinfo* source) {
	dummyinfo* result = new dummyinfo();
	result->version = source->version;
	result->flags = source->flags;
	result->uri = source->uri;
	result->name = source->name;
	result->short_name = source->short_name;
	result->author = source->author;
	result->min_tracks = source->min_tracks;
	result->max_tracks = source->max_tracks;
	result->inputs = source->inputs;
	result->outputs = source->outputs;
	result->commands = source->commands;
	result->pluginfile = source->pluginfile;
	result->note_group = source->note_group;
	result->note_column = source->note_column;
	result->wave_column = source->wave_column;
	result->velocity_column = source->velocity_column;
	result->instanceShortName = source->instanceShortName;
	result->supported_stream_extensions = source->supported_stream_extensions;
	result->supported_import_extensions = source->supported_import_extensions;

	// internal_parameters are owned by the engine and copied all right already

	result->global_parameters.clear();
	for (std::vector<const zzub::parameter*>::iterator i = source->global_parameters.begin(); i != source->global_parameters.end(); ++i) {
		result->add_global_parameter() = **i;
	}

	result->track_parameters.clear();
	for (std::vector<const zzub::parameter*>::iterator i = source->track_parameters.begin(); i != source->track_parameters.end(); ++i) {
		result->add_track_parameter() = **i;
	}

	result->controller_parameters.clear();
	for (std::vector<const zzub::parameter*>::iterator i = source->controller_parameters.begin(); i != source->controller_parameters.end(); ++i) {
		result->add_controller_parameter() = **i;
	}

	result->virtual_parameters.clear();
	owner->plugmgr.create_virtual_parameters(result);

	result->attributes.clear();
	for (std::vector<const zzub::attribute*>::iterator i = source->attributes.begin(); i != source->attributes.end(); ++i) {
		result->add_attribute() = **i;
	}

	register_dummy(owner, result);
	return result;

}


//
// dummyinfo
//

dummyinfo::dummyinfo() { }

zzub::plugin* dummyinfo::create_plugin() {
	return new dummyplugin(this);
}

bool dummyinfo::store_info(zzub::archive *arc) const {
	return false;
}

// loads info from the db to perform a plugin validation. this should only be used for testing.
void dummyinfo::load_from_db(player* owner, int plugininfo_id) {

	armstrong::storage::plugininfodata pluginfo;
	bool success = owner->songdata->get_plugininfo_by_id(plugininfo_id, pluginfo);
	assert(success);

	flags = pluginfo.flags;
	name = pluginfo.name;
	author = pluginfo.author;
	short_name = pluginfo.short_name;
	uri = pluginfo.uri;
	min_tracks = pluginfo.mintracks;
	max_tracks = pluginfo.maxtracks;
	outputs = pluginfo.output_count;
	inputs = pluginfo.input_count;

	armstrong::storage::tableiterator globalparams;
	owner->songdata->get_parameterinfos(pluginfo.id, 1, &globalparams);
	while (!globalparams.eof()) {
		armstrong::storage::parameterinfodata paraminfo;
		owner->songdata->get_parameterinfo_by_id(globalparams.id(), paraminfo);
		add_global_parameter()
			.set_name(paraminfo.name.c_str())
			.set_description(paraminfo.description.c_str())
			.set_flags(paraminfo.flags)
			.set_type((zzub_parameter_type)paraminfo.type)
			.set_value_min(paraminfo.minvalue)
			.set_value_max(paraminfo.maxvalue)
			.set_value_default(paraminfo.defaultvalue)
			.set_value_none(paraminfo.novalue)
		;
		globalparams.next();
	}
	globalparams.destroy();

	armstrong::storage::tableiterator trackparams;
	owner->songdata->get_parameterinfos(pluginfo.id, 2, &trackparams);
	while (!trackparams.eof()) {
		armstrong::storage::parameterinfodata paraminfo;
		owner->songdata->get_parameterinfo_by_id(trackparams.id(), paraminfo);
		add_track_parameter()
			.set_name(paraminfo.name.c_str())
			.set_description(paraminfo.description.c_str())
			.set_flags(paraminfo.flags)
			.set_type((zzub_parameter_type)paraminfo.type)
			.set_value_min(paraminfo.minvalue)
			.set_value_max(paraminfo.maxvalue)
			.set_value_default(paraminfo.defaultvalue)
			.set_value_none(paraminfo.novalue)
		;
		trackparams.next();
	}
	trackparams.destroy();


	armstrong::storage::tableiterator controllerparams;
	owner->songdata->get_parameterinfos(pluginfo.id, 3, &controllerparams);
	while (!controllerparams.eof()) {
		armstrong::storage::parameterinfodata paraminfo;
		owner->songdata->get_parameterinfo_by_id(controllerparams.id(), paraminfo);
		add_controller_parameter()
			.set_name(paraminfo.name.c_str())
			.set_description(paraminfo.description.c_str())
			.set_flags(paraminfo.flags)
			.set_type((zzub_parameter_type)paraminfo.type)
			.set_value_min(paraminfo.minvalue)
			.set_value_max(paraminfo.maxvalue)
			.set_value_default(paraminfo.defaultvalue)
			.set_value_none(paraminfo.novalue)
		;
		controllerparams.next();
	}
	controllerparams.destroy();

	armstrong::storage::tableiterator attrs;
	owner->songdata->get_attributeinfos(pluginfo.id, &attrs);
	while (!attrs.eof()) {
		armstrong::storage::attributeinfodata attrinfo;
		owner->songdata->get_attributeinfo_by_id(attrs.id(), attrinfo);
		add_attribute()
			.set_name(attrinfo.name.c_str())
			.set_value_min(attrinfo.minvalue)
			.set_value_max(attrinfo.maxvalue)
			.set_value_default(attrinfo.defaultvalue)
		;
		attrs.next();
	}
	attrs.destroy();
}

// inserts the plugin info to the db so that it can be loaded in player::on_insert_plugin(). 
// before this point, it must be registered with the player->plugmgr.register_dummy_plugin() in order to get the internal parameters
int dummyinfo::save_to_db(player* owner) {

	armstrong::storage::plugininfodata result;
	owner->songdata->create_plugininfo(flags, uri, name, short_name, author, min_tracks, max_tracks, inputs, outputs, result);

	for (int i = 0; i < (int)internal_parameters->size(); i++) {
		armstrong::storage::parameterinfodata resultparam;
		owner->songdata->create_parameterinfo(result.id, 0, 0, i, 
			(*internal_parameters)[i]->name, (*internal_parameters)[i]->description, (*internal_parameters)[i]->flags, 
			(*internal_parameters)[i]->type, (*internal_parameters)[i]->value_min, (*internal_parameters)[i]->value_max, 
			(*internal_parameters)[i]->value_none, (*internal_parameters)[i]->value_default, resultparam);
	}

	for (int i = 0; i < (int)global_parameters.size(); i++) {
		armstrong::storage::parameterinfodata resultparam;
		owner->songdata->create_parameterinfo(result.id, 1, 0, i, 
			global_parameters[i]->name, global_parameters[i]->description, global_parameters[i]->flags, 
			global_parameters[i]->type, global_parameters[i]->value_min, global_parameters[i]->value_max, 
			global_parameters[i]->value_none, global_parameters[i]->value_default, resultparam);
	}

	for (int i = 0; i < (int)track_parameters.size(); i++) {
		armstrong::storage::parameterinfodata resultparam;
		owner->songdata->create_parameterinfo(result.id, 2, 0, i, 
			track_parameters[i]->name, track_parameters[i]->description, track_parameters[i]->flags, 
			track_parameters[i]->type, track_parameters[i]->value_min, track_parameters[i]->value_max, 
			track_parameters[i]->value_none, track_parameters[i]->value_default, resultparam);
	}

	for (int i = 0; i < (int)controller_parameters.size(); i++) {
		armstrong::storage::parameterinfodata resultparam;
		owner->songdata->create_parameterinfo(result.id, 3, 0, i, 
			controller_parameters[i]->name, controller_parameters[i]->description, controller_parameters[i]->flags, 
			controller_parameters[i]->type, controller_parameters[i]->value_min, controller_parameters[i]->value_max, 
			controller_parameters[i]->value_none, controller_parameters[i]->value_default, resultparam);
	}

	for (int i = 0; i < (int)virtual_parameters.size(); i++) {
		armstrong::storage::parameterinfodata resultparam;
		owner->songdata->create_parameterinfo(result.id, 4, 0, i, 
			virtual_parameters[i]->name, virtual_parameters[i]->description, virtual_parameters[i]->flags, 
			virtual_parameters[i]->type, virtual_parameters[i]->value_min, virtual_parameters[i]->value_max, 
			virtual_parameters[i]->value_none, virtual_parameters[i]->value_default, resultparam);
	}

	for (int i = 0; i < (int)attributes.size(); i++) {
		armstrong::storage::attributeinfodata resultattr;
		owner->songdata->create_attributeinfo(result.id, i, attributes[i]->name, attributes[i]->value_min, attributes[i]->value_max, attributes[i]->value_default, resultattr);
	}

	return result.id;

}

// tests the plugin for most obvious problems
bool dummyinfo::test_plugin(zzub_pluginloader_t* loader, std::vector<validationerror>& result) {
	int numoutputs = zzub_pluginloader_get_output_channel_count(loader);
	if (outputs != numoutputs) {
		validationerror err;
		err.type = zzub_validation_error_type_plugin_outputs_mismatch;
		err.description = "Error: Output channel count mismatch.";
		err.info = loader;
		err.group = 1;
		err.found_value = outputs;
		err.expected_value = numoutputs;
		err.original_plugin_name = instanceShortName;
		result.push_back(err);
		return false;
	}

	int numinputs = zzub_pluginloader_get_input_channel_count(loader);
	if (inputs != numinputs) {
		validationerror err;
		err.type = zzub_validation_error_type_plugin_inputs_mismatch;
		err.description = "Error: Input channel count mismatch.";
		err.info = loader;
		err.group = 1;
		err.found_value = inputs;
		err.expected_value = numinputs;
		err.original_plugin_name = instanceShortName;
		result.push_back(err);
		return false;
	}

	int numglobalparams = zzub_pluginloader_get_parameter_count(loader, 1);
	if (global_parameters.size() != numglobalparams) {
		validationerror err;
		err.type = zzub_validation_error_type_parameter_count_mismatch;
		err.description = "Error: PARA global parameter count mismatch.";
		err.info = loader;
		err.group = 1;
		err.found_value = global_parameters.size();
		err.expected_value = numglobalparams;
		err.original_plugin_name = instanceShortName;
		result.push_back(err);
		return false;
	}

	int numtrackparams = zzub_pluginloader_get_parameter_count(loader, 2);
	if (track_parameters.size() != numtrackparams) {
		validationerror err;
		err.type = zzub_validation_error_type_parameter_count_mismatch;
		err.description = "Error: PARA track parameter count mismatch.";
		err.info = loader;
		err.group = 2;
		err.found_value = track_parameters.size();
		err.expected_value = numtrackparams;
		err.original_plugin_name = instanceShortName;
		result.push_back(err);
		return false;
	}

	int numcontrollerparams = zzub_pluginloader_get_parameter_count(loader, 3);
	if (controller_parameters.size() != numcontrollerparams) {
		validationerror err;
		err.type = zzub_validation_error_type_parameter_count_mismatch;
		err.description = "Error: PARA controller parameter count mismatch.";
		err.info = loader;
		err.group = 3;
		err.found_value = controller_parameters.size();
		err.expected_value = numcontrollerparams;
		err.original_plugin_name = instanceShortName;
		result.push_back(err);
		return false;
	}

	bool success = true;
	success &= test_group(loader, 1, global_parameters, result);
	success &= test_group(loader, 2, track_parameters, result);
	success &= test_group(loader, 3, controller_parameters, result);
	return success;
}

// creates validation error messages when there are parameter differences. returns true for no or permissible differences.
bool dummyinfo::test_group(zzub_pluginloader_t* loader, int group, std::vector<const zzub::parameter*>& parameters, std::vector<validationerror>& result) {
	bool success = true;
	for (size_t i = 0; i < parameters.size(); i++) {
		zzub_parameter_t* found_param = zzub_pluginloader_get_parameter(loader, group, i);
		assert(found_param);
		const zzub::parameter& expected_param = *parameters[i];
		
		success &= test_parameter_value("type", group, i, loader, 
			zzub_parameter_get_name(found_param),
			zzub_parameter_get_type(found_param), 
			expected_param.name,
			expected_param.type,
			zzub_validation_error_type_parameter_type_mismatch,
			result);

		success &= test_parameter_value("value_min", group, i, loader, 
			zzub_parameter_get_name(found_param),
			zzub_parameter_get_value_min(found_param), 
			expected_param.name,
			expected_param.value_min,
			zzub_validation_error_type_parameter_value_min_mismatch,
			result);

		success &= test_parameter_value("value_max", group, i, loader, 
			zzub_parameter_get_name(found_param),
			zzub_parameter_get_value_max(found_param), 
			expected_param.name,
			expected_param.value_max,
			zzub_validation_error_type_parameter_value_max_mismatch,
			result);

		test_parameter_value("value_default", group, i, loader, 
			zzub_parameter_get_name(found_param),
			zzub_parameter_get_value_default(found_param), 
			expected_param.name,
			expected_param.value_default,
			zzub_validation_error_type_parameter_value_default_mismatch,
			result);

		test_parameter_value("value_none", group, i, loader, 
			zzub_parameter_get_name(found_param),
			zzub_parameter_get_value_none(found_param), 
			expected_param.name,
			expected_param.value_none,
			zzub_validation_error_type_parameter_value_none_mismatch,
			result);

		// validate all but the velocity flag: it is added dynamically in buzz2zzub 
		// based on the parameter description which is only present in the runtime 
		// plugin info, not in the bmx format
		int flagmask = ~zzub_parameter_flag_velocity_index;

		test_parameter_value("flags", group, i, loader, 
			zzub_parameter_get_name(found_param),
			(zzub_parameter_get_flags(found_param) & flagmask), 
			expected_param.name,
			(expected_param.flags & flagmask),
			zzub_validation_error_type_parameter_flags_mismatch,
			result);
	}
	return success;
}

bool dummyinfo::test_parameter_value(std::string name, int group, int column, zzub_pluginloader_t* loader, const std::string& found_name, int found_value, const std::string& expected_name, int expected_value, int errortype, std::vector<validationerror>& result) {
	if (found_value != expected_value) {
		std::stringstream strm;
		strm << "Error: Parameter '" << name << "' mismatch. Expected (" << expected_name << ") " << expected_value << ". Found (" << found_name << ") " << found_value << ".";
		validationerror err;
		err.type = errortype;
		err.description = strm.str();
		err.info = loader;
		err.group = group;
		err.column = column;
		err.expected_value = expected_value;
		err.found_value = found_value;
		err.original_plugin_name = instanceShortName;
		err.original_parameter_name = expected_name;
		result.push_back(err);
		return false;
	}
	return true;
}

}
}
