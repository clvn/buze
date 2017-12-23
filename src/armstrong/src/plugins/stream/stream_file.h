#pragma once

struct stream_machine_info_file : stream_machine_info {
	stream_machine_info_file();
	virtual zzub::plugin* create_plugin();
	virtual bool store_info(zzub::archive *data) const { return false; }
};


