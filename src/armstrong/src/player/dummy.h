#pragma once

namespace armstrong {

namespace frontend {

struct validationerror {
	int type;
	std::string original_plugin_name;
	std::string original_parameter_name;
	std::string description;
	zzub_pluginloader_t* info;
	int group, column, found_value, expected_value;
};

struct dummyinfo : zzub::info {
	std::string instanceShortName;

	dummyinfo();
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *arc) const;
	void load_from_db(player* owner, int plugininfo_id);
	int save_to_db(player* owner);
	bool test_plugin(zzub_pluginloader_t* loader, std::vector<validationerror>& result);
	bool test_group(zzub_pluginloader_t* loader, int group, std::vector<const zzub::parameter*>& parameters, std::vector<validationerror>& result);
	bool test_parameter_value(std::string name, int group, int column, zzub_pluginloader_t* loader, const std::string& found_name, int found_value, const std::string& expected_name, int expected_value, int errortype, std::vector<validationerror>& result);
};

struct dummycollection : zzub::plugincollection {
	std::vector<dummyinfo*> dummyinfos;

	virtual ~dummycollection();
	virtual const char* get_name();
	virtual void destroy();
	dummyinfo* register_dummy(player* owner, dummyinfo* info);
	dummyinfo* clone(player* owner, dummyinfo* source);
};

}
}
