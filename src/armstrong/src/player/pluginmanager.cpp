#include <algorithm>
#include <vector>
#include "player.h"
#include "mixing/columninfo.h"

#if defined(POSIX)
#define strcmpi strcasecmp
#endif

namespace armstrong {

namespace frontend {

template <typename INFO>
struct find_info_by_uri : public std::unary_function<INFO*, bool> {
	std::string uri;
	find_info_by_uri(const std::string& u) {
		uri = u;
	}
	bool operator()(INFO* info) {
		return strcmpi(uri.c_str(), info->uri.c_str()) == 0;
	}
};

struct find_plugincollection_by_uri : public std::unary_function<zzub::plugincollection*, bool> {
	std::string uri;
	find_plugincollection_by_uri(const std::string& u) {
		uri = u;
	}
	bool operator()(zzub::plugincollection* coll) {
		const char* collectionuri = coll->get_uri();
		if (!collectionuri) return false;
		return strcmpi(uri.c_str(), collectionuri) == 0;
	}
};

pluginmanager::pluginmanager() {
	
	add_internal_parameter()
		.set_switch()
		.set_name("Hard Mute")
		.set_description("Hard Mute plugin")
		.set_value_default(0)
		.set_state_flag();

	add_internal_parameter()
		.set_switch()
		.set_name("Hard Bypass")
		.set_description("Hard Bypass plugin")
		.set_value_default(0)
		.set_state_flag();
	
	add_internal_parameter()
		.set_switch()
		.set_name("Soft Mute")
		.set_description("Soft Mute plugin");

	add_internal_parameter()
		.set_switch()
		.set_name("Soft Bypass")
		.set_description("Soft Bypass plugin");
/*
	add_internal_parameter()
		.set_byte()
		.set_name("Meta")
		.set_description("Metaparameter")
		.set_value_min(0)
		.set_value_max(0)
		.set_value_none(0)
		.set_value_default(0)
		.set_flags(zzub_parameter_flag_compound);*/

}

zzub::info* pluginmanager::plugin_get_info(const std::string& uri) {
	// dummy plugins override original plugins
	{
		std::vector<dummyinfo*>::iterator i = find_if(dummy_plugins.dummyinfos.begin(), dummy_plugins.dummyinfos.end(), find_info_by_uri<dummyinfo>(uri));
		if (i != dummy_plugins.dummyinfos.end()) return *i;
	}

	{
		std::map<std::string, zzub::info*>::iterator i = uri_infos.find(uri);
		if (i != uri_infos.end()) return i->second;
	}
	
	return 0;
}

zzub::plugincollection* pluginmanager::get_collection(const std::string& uri) {
	std::vector<zzub::plugincollection*>::iterator i = find_if(builtinplugins.begin(), builtinplugins.end(), find_plugincollection_by_uri(uri));
	if (i != builtinplugins.end()) return (*i);
	
	return 0;
}

void pluginmanager::add_collection(zzub::plugincollection* coll) {
	builtinplugins.push_back(coll);
}

void pluginmanager::register_info(zzub::info* _info) {
	assert(!_info->short_name.empty());
	assert(!_info->name.empty());
	assert(!_info->uri.empty());
	//assert(_info->flags != 0); // jeskola notematrix has 0 flags
	assert(_info->collection != 0);

	_info->internal_parameters = &internal_parameters;
	find_known_columns(_info);
	create_virtual_parameters(_info);
	plugin_infos.push_back(_info);
	uri_infos[_info->uri] = _info;
}


void pluginmanager::clear_dummy_plugins() {
	dummy_plugins.destroy();
}

void pluginmanager::find_known_columns(zzub::info* _info) {

	if (_info->note_group != -1) return ;

	// scan for note/velocity/wave columns
	if (get_note_info(_info, _info->note_group, _info->note_column)) {
		if (!get_velocity_info(_info, _info->note_group, _info->velocity_column)) {
			_info->velocity_column = -1;
		}
		if (!get_wave_info(_info, _info->note_group, _info->wave_column)) {
			_info->wave_column = -1;
		}
	} else {
		_info->note_group = -1;
		_info->note_column = -1;
		_info->velocity_column = -1;
		_info->wave_column = -1;
	}
}

void pluginmanager::create_virtual_parameters(zzub::info* _info) {
// TODO: virtual parameters are disabled. please bump the armz version if this is ever uncommented
	if (_info->note_group != -1)
		add_virtual_parameter(_info)
			.set_meta()
			.set_meta_note()
			.set_name("Note Meta")
			.set_description("Add this to a pattern format if the user interface supports note metaparameters.");

/*	if (_info->flags & zzub_plugin_flag_stream)
		add_virtual_parameter(_info)
			.set_meta()
			.set_meta_wave()
			.set_name("Wave Meta")
			.set_description("Add this to a pattern format if the user interface supports wave metaparameters.");
*/
}

zzub::parameter& pluginmanager::add_virtual_parameter(zzub::info* _info) {
	zzub::parameter *param = new zzub::parameter();
	_info->virtual_parameters.push_back(param);
	return *param;
}

zzub::parameter& pluginmanager::add_internal_parameter() {
	zzub::parameter *param = new zzub::parameter();
	internal_parameters.push_back(param);
	return *param;
}

}
}
