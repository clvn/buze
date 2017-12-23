#pragma once

struct semplugininfo;

struct semplugin : zzub::plugin {
	static std::map<SEMod_struct_base2*, semplugin*> pluginmap;
	HMODULE hPlugin;
	SEMod_struct_base2* eff;
	std::vector<semparameter> semparams; // this is copied from the info, and updated with a fresh SEPinProperties::variable_address for this instance
	std::vector<semparameter> audioinputs;
	std::vector<semparameter> audiooutputs;
	std::vector<semparameter> midiinputs;
	std::vector<semparameter> midioutputs;
	char parameterbuffer[1024];

	semplugininfo* _info;
	semplugin(semplugininfo* info);
	virtual void init(zzub::archive* arc);
	virtual void destroy();
	virtual void process_events();
	virtual bool process_stereo(float** pin, float** pout, int numsamples, int mode);
	virtual const char* describe_value(int param, int value);
	virtual const char* get_output_channel_name(int i);
	virtual const char* get_input_channel_name(int i);

	bool load_dll();
	void update_pins(std::vector<semparameter>& semparams, bool is_param);
};
