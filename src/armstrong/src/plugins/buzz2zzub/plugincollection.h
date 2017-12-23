#pragma once

#include "../plugincache.h"

namespace buzz2zzub {

const int OSCTABSIZE = (2048+1024+512+256+128+64+32+16+8+4)*sizeof(short);

extern short oscTables[8][OSCTABSIZE];

typedef CMachineInfo const *(__cdecl *GET_INFO)();
typedef CMachineInterface *(__cdecl *CREATE_MACHINE)();

struct plugin;
struct buzzplugincollection;

struct buzzplugininfo : zzub::info {
	std::string basefilename;
	int origFlags;
	HINSTANCE hDllInstance;
	bool exePlugin;
	GET_INFO GetInfo;
	CREATE_MACHINE CreateMachine;
	bool lockAddInput, lockSetTracks, useSequencerHack;
	CMachineManager* machines;
	bool has_filled, has_filled_parameters;
	int create_count;

	buzzplugininfo(zzub::plugincollection* coll, std::string dllpath);
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *arc) const;
	bool load_buzz_dll();
	void unload_buzz_dll();
	void fill_machineinfo(const CMachineInfo *buzzinfo);
	void fill_machineinfo_parameters(const CMachineInfo *buzzinfo);
};

struct buzzplugincollection : zzub::plugincollection {
	std::string hostpath;
	std::string userpath;

	CMachineManager machines;
	std::vector<buzzplugininfo*> buzzplugins;
	plugincache<buzzplugincollection, buzzplugininfo> effects;
	plugincache<buzzplugincollection, buzzplugininfo> generators;

	buzzplugincollection();
	~buzzplugincollection();

	// Called by the host initially. The collection registers
	// plugins through the pluginfactory::register_info method.
	// The factory pointer remains valid and can be stored
	// for later reference.
	virtual void initialize(zzub::pluginfactory *factory);

	// Called by the host upon song loading. If the collection
	// can not provide a plugin info based on the uri or
	// the metainfo passed, it should return a null pointer.
	virtual zzub::info *get_info(const char *uri, zzub::archive *arc) { return 0; }
	
	// Called by the host upon destruction. You should
	// delete the instance in this function
	virtual void destroy() { delete this; }

	// Returns the uri of the collection to be identified,
	// return zero for no uri. Collections without uri can not be 
	// configured.
	virtual const char *get_uri() { return 0; }
	
	// Called by the host to set specific configuration options,
	// usually related to paths.
	virtual void configure(const char *key, const char *value);

	virtual const char* get_name() {
		return "Buzz";
	}

	void load_config();
	void enumerate_plugins(std::string pluginpath, std::vector<buzzplugininfo*>& result_plugins);

	// exe plugins
	void load_exe_plugins(zzub::pluginfactory* factory);
	bool prepatch_buzzexe(unsigned char* data, int size);
	int scan_buzzexe_plugins_start(unsigned char* codeBase, int size);
	int scan_buzzexe_plugins_next(unsigned char* codeBase, int size, int offset);

	// called by plugincache
	void create_plugin_infos_from_file(const std::string& fullpath, std::vector<buzzplugininfo*>* infos);
	void init_plugin_infos(std::vector<buzzplugininfo*>& infos, bool from_cache);
	bool fill_plugin_infos(std::vector<buzzplugininfo*>& infos);
	void unregister_plugin_infos(std::vector<buzzplugininfo*>& info);

};

}
