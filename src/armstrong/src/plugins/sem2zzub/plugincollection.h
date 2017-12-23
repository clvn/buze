#pragma once

//extern "C" typedef int32_t (* MP_STDCALL MP_GETFACTORY)(gmpi::IMpUnknown** returnInterface);
extern "C" typedef int (*SE2_GETMODULEPROPERTIES)(int p_index, SEModuleProperties* p_properties);
extern "C" typedef void* (*SE2_MAKEMODULE)(int p_index, int p_type, seaudioMasterCallback2 seaudioMaster, void *p_resvd1);

struct semplugin;

struct semparameter {
	int pinindex;
	SEPinProperties prop;
	it_enum_list* range; // DT_ENUM
	int offset;
	float minvalue; // DT_FSAMPLE
	float maxvalue; // DT_FSAMPLE
	int channel; // DT_FSAMPLE index to i/o channel pin
	float defaultvalue;
	union {
		float floatvalue;
		bool boolvalue;
		int intvalue;
	};
};

struct semplugininfo : zzub::info {
	std::string pluginFile;
	int index;
	std::vector<semparameter> semparams;
	std::vector<semparameter> audioinputs;
	std::vector<semparameter> audiooutputs;
	std::vector<semparameter> midiinputs;
	std::vector<semparameter> midioutputs;

	zzub::plugin* create_plugin();
	void enum_parameters(SEMod_struct_base2* eff);
	int sem_to_internal_int(int value);
	int internal_to_sem_int(int value);
	int sem_to_internal_float(float value, semparameter* semparam);
	float internal_to_sem_float(int value, semparameter* semparam);
};

struct semplugincollection : zzub::plugincollection {
	std::vector<semplugininfo*> plugins;

	virtual void initialize(zzub::pluginfactory *factory);
	void scan_plugins(std::string const & rootpath, std::string path, zzub::pluginfactory* factory);
	bool add_plugin(zzub::pluginfactory *factory, std::string abspath, std::string relpath);
	std::string get_plugin_path();
	const char* get_name() { return "SynthEdit"; }
};
