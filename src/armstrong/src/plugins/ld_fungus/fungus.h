#pragma once

struct fungus_machine_info : zzub::info {
	fungus_machine_info();
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *data) const;
};

extern fungus_machine_info fungus_info;

zzub::plugincollection *fungus_get_plugincollection();

