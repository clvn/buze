#pragma once

struct group_input_plugin_info : zzub::info {

	group_input_plugin_info() {
		this->flags = 
			zzub_plugin_flag_has_audio_input |	// accepts audio input connections
			zzub_plugin_flag_has_audio_output | // sends output output
			zzub_plugin_flag_has_group_input;  // is a group input plugin

		this->name = "Group Input";
		this->short_name = "GroupIn";
		this->author = "n/a";
		this->uri = "@zzub.org/group/input";
		this->inputs = 2;
		this->outputs = 2;
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};


struct group_output_plugin_info : zzub::info {

	group_output_plugin_info() {
		this->flags = 
			zzub_plugin_flag_has_audio_input |	// accepts audio input connections
			zzub_plugin_flag_has_audio_output | // sends output output
			zzub_plugin_flag_has_group_output;  // is a layer input plugin

		this->name = "Group Output";
		this->short_name = "GroupOut";
		this->author = "n/a";
		this->uri = "@zzub.org/group/output";
		this->inputs = 2;
		this->outputs = 2;
	}
	
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *) const { return false; }
};
