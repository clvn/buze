#include <cstring>
#include "library.h"
#include "player.h"
#include "dummy.h"
#include "archive.h"

using namespace armstrong::frontend;
using std::vector;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

extern "C" {

// PLuginloader methods

const char *zzub_pluginloader_get_loader_name(zzub_pluginloader_t* loader)
{
	return loader->uri.c_str();
}

const char *zzub_pluginloader_get_name(zzub_pluginloader_t* loader) {
	return loader->name.c_str();
}

const char *zzub_pluginloader_get_short_name(zzub_pluginloader_t* loader) {
	return loader->short_name.c_str();
}

int zzub_pluginloader_get_parameter_count(zzub_pluginloader_t* loader, int group) {
	switch (group) {
		case 0: // internals
			return (int)loader->internal_parameters->size();
		case 1: // globals
			return (int)loader->global_parameters.size();
		case 2: // track params
			return (int)loader->track_parameters.size();
		case 3: // controller params
			return (int)loader->controller_parameters.size();
		case 4: // virtual parameters
			return (int)loader->virtual_parameters.size();
		default:
			return 0;
	}
}

zzub_parameter_t *zzub_pluginloader_get_parameter(zzub_pluginloader_t* loader, int group, int index) {
	switch (group) {
		case 0: // internals
			return (zzub_parameter_t*)(*loader->internal_parameters)[index];
		case 1: // globals
			return (zzub_parameter_t*)loader->global_parameters[index];
		case 2: // track params
			return (zzub_parameter_t*)loader->track_parameters[index];
		case 3: // controller params
			return (zzub_parameter_t*)loader->controller_parameters[index];
		case 4: // virtual parameters
			return (zzub_parameter_t*)loader->virtual_parameters[index];
		default:
			return 0;
	}
}

int zzub_pluginloader_get_flags(zzub_pluginloader_t* loader) {
	return loader->flags;
}

const char *zzub_pluginloader_get_uri(zzub_pluginloader_t* loader) {
	return loader->uri.c_str();
}

const char *zzub_pluginloader_get_author(zzub_pluginloader_t* loader) {
	return loader->author.c_str();
}

int zzub_pluginloader_get_attribute_count(zzub_pluginloader_t* loader) {
	return (int)loader->attributes.size();
}

zzub_attribute_t *zzub_pluginloader_get_attribute(zzub_pluginloader_t* loader, int index) {
	return (zzub_attribute_t *)loader->attributes[index];
}

int zzub_pluginloader_get_instrument_list(zzub_pluginloader_t* loader, char* result, int maxbytes) {
	if (loader->plugin_lib == 0) return 0;

	vector<char> outputBytes;
	mem_outstream outf(outputBytes);
	loader->plugin_lib->get_instrument_list(&outf);
	int size = outputBytes.size();
	if (size > 0)
	{
		if (size > maxbytes) size = maxbytes;
		memcpy(result, &outputBytes.front(), size);
	}
	result[size] = 0;
	return size;
}

int zzub_pluginloader_get_tracks_min(zzub_pluginloader_t* loader) {
	return loader->min_tracks;
}

int zzub_pluginloader_get_tracks_max(zzub_pluginloader_t* loader) {
	return loader->max_tracks;
}

int zzub_pluginloader_get_stream_format_count(zzub_pluginloader_t* loader) {
	return (int)loader->supported_stream_extensions.size();
}

const char* zzub_pluginloader_get_stream_format_ext(zzub_pluginloader_t* loader, int index) {
	return loader->supported_stream_extensions[index].c_str();
}

int zzub_pluginloader_get_output_channel_count(zzub_pluginloader_t* loader) {
	return loader->outputs;
}

int zzub_pluginloader_get_input_channel_count(zzub_pluginloader_t* loader) {
	return loader->inputs;
}

const char *zzub_pluginloader_get_plugin_file(zzub_pluginloader_t* loader) {
	return loader->pluginfile.c_str();
}

zzub_plugincollection_t* zzub_pluginloader_get_plugincollection(zzub_pluginloader_t* loader) {
	return loader->collection;
}

// parameter methods

int zzub_parameter_get_type(zzub_parameter_t* param) {
	return param->type;
}

const char *zzub_parameter_get_name(zzub_parameter_t* param) {
	return param->name.c_str();
}

const char *zzub_parameter_get_description(zzub_parameter_t* param) {
	return param->description.c_str();
}

int zzub_parameter_get_value_min(zzub_parameter_t* param) {
	return param->value_min;
}

int zzub_parameter_get_value_max(zzub_parameter_t* param) {
	return param->value_max;
}

int zzub_parameter_get_value_none(zzub_parameter_t* param) {
	return param->value_none;
}

int zzub_parameter_get_value_default(zzub_parameter_t* param) {
	return param->value_default;
}

int zzub_parameter_get_flags(zzub_parameter_t* param) {
	return param->flags;
}

// attribute methods

const char *zzub_attribute_get_name(zzub_attribute_t *attrib) {
	return attrib->name.c_str();
}

int zzub_attribute_get_value_min(zzub_attribute_t *attrib) {
	return attrib->value_min;
}

int zzub_attribute_get_value_max(zzub_attribute_t *attrib) {
	return attrib->value_max;
}

int zzub_attribute_get_value_default(zzub_attribute_t *attrib) {
	return attrib->value_default;
}

}
