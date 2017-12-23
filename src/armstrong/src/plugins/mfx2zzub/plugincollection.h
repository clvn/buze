#pragma once

struct mfxplugin;

struct mfxplugininfo : zzub::info {
	std::string pluginFile;
	CLSID pluginClsid;

	zzub::plugin* create_plugin();
};

struct mfxplugincollection : zzub::plugincollection {
	std::vector<mfxplugininfo*> plugins;

	virtual void initialize(zzub::pluginfactory *factory);
	bool add_plugin(zzub::pluginfactory *factory, CLSID pluginClsid, std::string name, std::string filename);
	const char* get_name() { return "MFX"; }

};
