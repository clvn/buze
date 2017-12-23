#pragma once

struct vstplugininfo : zzub::info {
	std::vector<std::string> programNames;
	std::vector<VstParameterProperties> parameterProperties;

	vstplugininfo(zzub::plugincollection* coll, std::string _path);
	void load_program_names(AEffect* effect);
	void load_parameter_properties(AEffect* effect);
	AEffect* create_vst_effect(HMODULE* hPluginResult);
	bool check_can_do(int canDo, bool defaultValue = true);
	// fills in plugin info based on a vst plugin instance
	void fill_plugin_info(AEffect* effect);
	// creates a vst plugin instance, fills in plugin info and destroys the vst plugin instance
	bool fill_plugin_info();
	zzub::plugin* create_plugin();
	bool store_info(zzub::archive *) const;
};

struct vstplugincollection;

typedef plugincache<vstplugincollection, vstplugininfo> vstplugincache ;

struct vstplugincollection : zzub::plugincollection {

	// plugininfos must be shared across armstrong sessions in the same
	// process when exposed as a vst to prevent recursive enumeration.
	static std::map<std::string, vstplugincache*> plugincaches;
	std::string hostpath;
	std::string userpath;
	std::vector<std::string> vstpaths;

	static vstplugincache* get_plugincache(const std::string& path);

	virtual const char* get_uri();
	virtual const char* get_name();
	virtual void configure(const char *key, const char *value);
	virtual void initialize(zzub::pluginfactory * factory);
	virtual void destroy();

	void set_vst_paths();

	// called by plugincache
	void create_plugin_infos_from_file(const std::string& fullpath, std::vector<vstplugininfo*>* infos);
	void init_plugin_infos(std::vector<vstplugininfo*>& infos, bool from_cache);
	bool fill_plugin_infos(std::vector<vstplugininfo*>& infos);
	void unregister_plugin_infos(std::vector<vstplugininfo*>& info);

};
