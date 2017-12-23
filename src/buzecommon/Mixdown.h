#pragma once

class CApplication;

struct mixdownhelper {
	CApplication* application;
	zzub_player_t* player;
	zzub_plugin_t* recordplugin;
	zzub_plugin_t* recorder;

	mixdownhelper(CApplication* _app, zzub_player_t* _player, zzub_plugin_t* _sourceplugin);
	bool init_file_recorder(std::string filename);
	bool init_wave_recorder(zzub_wavelevel_t* level);
	bool mixdown(int samplerate, int orderindex = 0, int startrow = 0, int stoprow = -1);
	void uninit();
	bool is_orderlist_infinite();
};
