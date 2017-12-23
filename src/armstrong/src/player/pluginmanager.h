#pragma once
#include <map>

namespace armstrong {

namespace frontend {

struct player;

struct pluginmanager : zzub::pluginfactory {
	dummycollection dummy_plugins;
	std::vector<zzub::plugincollection*> builtinplugins;
	std::vector<zzub::info*> plugin_infos;
	std::map<std::string, zzub::info*> uri_infos;
	std::vector<const zzub::parameter*> internal_parameters; // common parameters for all plugins group 0

	pluginmanager();
	void initialize_plugin_libraries();
	void initialize_plugin_library(zzub::plugincollection* coll);
	void clear_dummy_plugins();
	void find_known_columns(zzub::info* _info);
	void create_virtual_parameters(zzub::info* _info);
	std::string plugin_get_new_name(const std::string& uri);
	zzub::info* plugin_get_info(const std::string& uri);
	zzub::plugincollection* get_collection(const std::string& uri);
	void add_collection(zzub::plugincollection* coll);

	// zzub::pluginfactory
	void register_info(zzub::info* _info);

private:
	zzub::parameter& add_internal_parameter();
	zzub::parameter& add_virtual_parameter(zzub::info* _info);
};

}
}
