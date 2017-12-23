#include <windows.h>
#include "resource.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <buze/buzesdk.h>
#include "Mixdown.h"

// ---------------------------------------------------------------------------------------------------------------
// MIXDOWN HELPER
// ---------------------------------------------------------------------------------------------------------------

mixdownhelper::mixdownhelper(CApplication* _app, zzub_player_t* _player, zzub_plugin_t* _sourceplugin) {
	application = _app;
	player = _player;
	recordplugin = _sourceplugin;
	recorder = 0;
}

bool mixdownhelper::init_file_recorder(std::string filename) {
	zzub_pluginloader_t* floader = zzub_player_get_pluginloader_by_name(player, "@zzub.org/recorder/file;2");
	if (floader == 0) return false;
	
	recorder = zzub_player_create_plugin(player, 0, 0, "frecorder", floader, 0);
	if (recorder == 0) return false;

	//zzub_plugin_configure(recorder, "wavefilepath", filename.c_str());
	zzub_plugin_set_stream_source(recorder, filename.c_str());
	zzub_plugin_set_parameter_value(recorder, 1, 0, 0, 1, 0); // enable
	zzub_plugin_set_parameter_value(recorder, 1, 0, 1, 0, 0); // automatic mode, sync to play/stop
	zzub_plugin_set_parameter_value(recorder, 1, 0, 2, 0, 0); // format 0 = 16 bit
	zzub_plugin_tick(recorder, 0);

	return true;
}

bool mixdownhelper::init_wave_recorder(zzub_wavelevel_t* level) {
	zzub_wave_t* wave = zzub_wavelevel_get_wave(level);
	assert(wave != 0);
	zzub_wave_clear(wave);

	zzub_player_history_commit(player, 0, 0, "Clear wave before mixdown");

	zzub_pluginloader_t* wtloader = zzub_player_get_pluginloader_by_name(player, "@zzub.org/recorder/wavetable;2");
	if (!wtloader) return false;

	recorder = zzub_player_create_plugin(player, 0, 0, "wrecorder", wtloader, 0);
	if (recorder == 0) return false;

	int index = zzub_wave_get_index(wave);
	zzub_plugin_set_parameter_value(recorder, 1, 0, 0, 1, 0);	// enable
	zzub_plugin_set_parameter_value(recorder, 1, 0, 1, 0, 0); // automatic mode, sync to play/stop
	zzub_plugin_set_parameter_value(recorder, 1, 0, 2, 0, 0); // format 0 = 16 bit
	zzub_plugin_set_parameter_value(recorder, 1, 0, 3, index+1, 0); // wave index
	zzub_plugin_tick(recorder, 0);

	return true;
}

bool mixdownhelper::mixdown(int samplerate, int orderindex, int startrow, int stoprow) {
	zzub_player_set_state(player, zzub_player_state_muted, -1);

	zzub_connection_t* mixconn = zzub_plugin_create_audio_connection(recorder, recordplugin, -1, -1, -1, -1);
	if (mixconn == 0) {
		// Cant connect recorder
		return false;
	}

	if (is_orderlist_infinite())
		return false;

	//int samplerate = zzub_audiodriver_get_samplerate(_Module.driver);
	int loop_enabled = zzub_player_get_order_loop_enabled(player);

	buze_application_enable_silent_driver(application, 1);
	//_Module.setSilentAudioDriver(samplerate);

	zzub_player_set_order_loop_enabled(player, 0);
	zzub_player_set_position(player, orderindex, startrow);
	zzub_player_set_state(player, zzub_player_state_playing, stoprow);

	zzub_player_history_commit(player, 0, 0, "Pre Mix Down");

	int numsamples = 256;
	zzub_player_work_stereo(player, 0, 0, 0, 0, numsamples); // work once before looping to process audio events 
	while (zzub_player_get_state(player) == zzub_player_state_playing) {
		zzub_player_work_stereo(player, 0, 0, 0, 0, numsamples);
	}

	zzub_player_set_order_loop_enabled(player, loop_enabled);
	zzub_connection_destroy(mixconn);
	zzub_player_history_commit(player, 0, 0, "Post Mix Down");

	buze_application_enable_silent_driver(application, 0);

	return true;
}

void mixdownhelper::uninit() {
	if (recorder) zzub_plugin_destroy(recorder);
	recorder = 0;
}

bool mixdownhelper::is_orderlist_infinite() {
	// abort if any of the patterns in the orderlist are looping
	bool is_looping = false;
	zzub_pattern_iterator_t* patit = zzub_player_get_order_iterator(player);
	while (zzub_pattern_iterator_valid(patit)) {
		zzub_pattern_t* pat = zzub_pattern_iterator_current(patit);
		if (pat != 0 && zzub_pattern_get_loop_enabled(pat) != 0) {
			is_looping = true;
			break;
		}
		zzub_pattern_iterator_next(patit);
	}
	zzub_pattern_iterator_destroy(patit);
	return is_looping;
}
