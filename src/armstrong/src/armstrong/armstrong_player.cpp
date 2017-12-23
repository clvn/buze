#include <cstring>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include "library.h"
#include "player.h"
#include "mixing/mixer.h"
#include <map>
#include "bmxreader.h"
#include "storage/armz.h"
#include "archive.h"
#include "zipio.h"

using std::vector;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

namespace {
	std::string get_host_path() {
#if defined(_WIN32)
		HINSTANCE hInstance = GetModuleHandle(0);
		char path[MAX_PATH + 32] = { 0 };
		::GetModuleFileName(hInstance, path, MAX_PATH);
#else
		char path[PATH_MAX] = {0};
		readlink("/proc/self/exe", path, PATH_MAX);
#endif
		boost::filesystem::path exepath(path);
		return exepath.parent_path().string();
	}

	std::string get_user_path() {
#if defined(_WIN32)
		char* path = getenv("APPDATA");
		if (path == 0) {
			return "";
		}
		boost::filesystem::path userpath = boost::filesystem::path(path) / boost::filesystem::path("Armstrong");
		return userpath.string();
#else
		boost::filesystem::path userpath = boost::filesystem::path("~/.armstrong");
		return userpath.string();
#endif
	}
}

extern "C" {

zzub_player_t* zzub_player_create(const char* hostpath, const char* userpath, const char* temppath) {
	armstrong::frontend::player* p = new armstrong::frontend::player();
	if (hostpath != 0) {
		p->hostpath = hostpath;
	} else {
		p->hostpath = get_host_path();
	}

	if (userpath != 0) {
		p->userpath = userpath;
	} else {
		p->userpath = get_user_path();
	}

	if (temppath != 0) {
		p->temppath = temppath;
	} else {
		p->temppath = boost::filesystem::temp_directory_path().string();
	}
	return p;
}

void zzub_player_destroy(zzub_player_t* player) {
	delete player;
}

zzub_plugin_t* zzub_player_create_plugin(zzub_player_t* player, zzub_input_t* input, int dataSize, const char* instanceName, zzub_pluginloader_t* loader, zzub_plugin_group_t* plugingroup) {
	armstrong::storage::plugin plugdata(player->doc.get());
	int plugingroup_id = plugingroup != 0 ? plugingroup->dataplugingroup->id : 0;
	if (!player->create_plugin(loader, input, dataSize, instanceName, plugingroup_id, plugdata))
		return 0;
	return player->plugins[plugdata.id].get();
}

zzub_pattern_t* zzub_player_create_pattern(zzub_player_t* player, zzub_pattern_format_t* format, const char* description, int length) {
	const char* name = zzub_player_get_new_pattern_name(player, format, description);

	armstrong::storage::patterndata patdata;
	player->songdata->create_pattern(format->dataformat->id, name, length, patdata);
	return player->patterns[patdata.id].get();
}

zzub_pattern_t* zzub_player_clone_pattern(zzub_player_t* player, zzub_pattern_t* pattern, const char* description) {
	zzub_pattern_format_t* format = zzub_pattern_get_format(pattern);
	int length = zzub_pattern_get_row_count(pattern);
	int reso = zzub_pattern_get_resolution(pattern);
	int displayreso = zzub_pattern_get_display_resolution(pattern);
	int verydark, dark;
	zzub_pattern_get_display_beat_rows(pattern, &verydark, &dark);
	int loop_start = zzub_pattern_get_loop_start(pattern);
	int loop_end = zzub_pattern_get_loop_end(pattern);
	int loop_enabled = zzub_pattern_get_loop_enabled(pattern);

	zzub_pattern_t* newpat = zzub_player_create_pattern(player, format, description, length);
	zzub_pattern_set_resolution(newpat, reso);
	zzub_pattern_set_display_resolution(newpat, displayreso);
	zzub_pattern_set_display_beat_rows(newpat, verydark, dark);
	zzub_pattern_set_loop_start(newpat, loop_start);
	zzub_pattern_set_loop_end(newpat, loop_end);
	zzub_pattern_set_loop_enabled(newpat, loop_enabled);

	zzub_pattern_event_iterator_t* evit = zzub_pattern_get_event_iterator(pattern, 0, -1, -1, -1);
	while (zzub_pattern_event_iterator_valid(evit)) {
		zzub_pattern_event_t* ev = zzub_pattern_event_iterator_current(evit);

		int plugin_id = zzub_pattern_event_get_pluginid(ev);
		int group = zzub_pattern_event_get_group(ev);
		int track = zzub_pattern_event_get_track(ev);
		int column = zzub_pattern_event_get_column(ev);
		int time = zzub_pattern_event_get_time(ev);
		int value = zzub_pattern_event_get_value(ev);
		int meta = zzub_pattern_event_get_meta(ev);
		zzub_plugin_t* plugin = zzub_player_get_plugin_by_id(player, plugin_id);
		zzub_pattern_set_value(newpat, time, plugin, group, track, column, value, meta);

		zzub_pattern_event_iterator_next(evit);
	}
	zzub_pattern_event_iterator_destroy(evit);

	return newpat;
}

zzub_pattern_format_t* zzub_player_create_pattern_format(zzub_player_t* player, const char* description) {
	const char* name = zzub_player_get_new_pattern_format_name(player, description);

	armstrong::storage::patternformatdata fmtdata;
	player->songdata->create_pattern_format(name, fmtdata);
	return player->patternformats[fmtdata.id].get();
}

zzub_pattern_format_t* zzub_player_clone_pattern_format(zzub_player_t* player, zzub_pattern_format_t* format, const char* description) {
	zzub_pattern_format_t* newformat = zzub_player_create_pattern_format(player, description);
	if (!newformat) return 0;

	int screenidx = 0;
	zzub_pattern_format_column_iterator_t* colit = zzub_pattern_format_get_iterator(format);
	while (zzub_pattern_format_column_iterator_valid(colit)) {
		zzub_pattern_format_column_t* fmt_col = zzub_pattern_format_column_iterator_current(colit);
		zzub_plugin_t* plugin = zzub_pattern_format_column_get_plugin(fmt_col);
		int group = zzub_pattern_format_column_get_group(fmt_col);
		int track = zzub_pattern_format_column_get_track(fmt_col);
		int column = zzub_pattern_format_column_get_column(fmt_col);

		zzub_pattern_format_add_column(newformat, plugin, group, track, column, screenidx);
		zzub_pattern_format_column_iterator_next(colit);
		++screenidx;
	}
	zzub_pattern_format_column_iterator_destroy(colit);

	return newformat;
}

zzub_pluginloader_t* zzub_player_get_pluginloader_by_name(zzub_player_t* player, const char* name) {
	if (!name) return 0;
	return player->plugmgr.plugin_get_info(name);
}

void zzub_player_work_stereo(zzub_player_t* player, const float** inbuffers, float** outbuffers, int inchannels, int outchannels, int numsamples) {
	// these need to be in the zidl in order to specify array arguments:
	inchannels ;
	outchannels ;
	player->work_stereo((float**)inbuffers, outbuffers, numsamples);
}

zzub_midimapping_t* zzub_player_add_midimapping(zzub_plugin_t* plugin, int group, int track, int param, int channel, int controller) {
	plugin->dataplugin->add_midimapping(group, track, param, channel, controller);
	return 0;
}

int zzub_player_remove_midimapping(zzub_plugin_t* plugin, int group, int track, int param) {
	plugin->dataplugin->remove_midimapping(group, track, param);
	return 0;
}

zzub_midimapping_t* zzub_player_get_midimapping(zzub_player_t* player, int index) {
	armstrong::storage::midimappingdata data;
	player->songdata->get_midimapping_by_index(index, data);

	return player->midimappings[data.id].get();
}

int zzub_player_get_midimapping_count(zzub_player_t* player) {
	return player->songdata->get_midimapping_count();
}



/***

	Player methods

***/

int zzub_player_initialize(zzub_player_t* player, int samplesPerSec) {
	player->initialize();
	return 0;
}

int zzub_player_remote_connect(zzub_player_t* player, const char* host, const char* port) {
	bool success = player->armclient.connect(host, port);
	return success ? 0: -1;
}

void zzub_player_remote_disconnect(zzub_player_t* player) {
	player->armclient.disconnect();
}

int zzub_player_remote_open(zzub_player_t* player, const char* project, const char* password) {
	bool success = player->armclient.open(project, password);

	if (!success) {
		zzub_player_clear(player);
		return -1;
	}
	zzub_player_history_commit(player, 0, 0, "Open Remote");
	zzub_player_history_reset(player);
	return 0;
}

int zzub_player_remote_create(zzub_player_t* player, const char* project, const char* password) {
	bool success = player->armclient.create(project, password);
	return success ? 0 : -1;
}

int zzub_player_remote_delete(zzub_player_t* player, const char* project, const char* password) {
	bool result = player->armclient.remove(project, password);
	return result ? 0 : -1;
}

int zzub_player_get_remote_client_count(zzub_player_t* player) {
	return 0;
}

int zzub_player_is_remote_connected(zzub_player_t* player) {
	return  player->armclient.client != 0 ? 1 : 0;
}

void zzub_player_undo(zzub_player_t* player) {
	player->doc->undo();
}

void zzub_player_redo(zzub_player_t* player) {
	player->doc->redo();
}

void zzub_player_history_begin(zzub_player_t* player, void *userdata) {
	player->event_userdata = userdata;
}

void zzub_player_history_commit(zzub_player_t* player, int redo_id, int undo_id, const char* description) {
	player->doc->barrier(redo_id, undo_id, description);
}

int zzub_player_history_get_uncomitted_operations(zzub_player_t* player) {
	// the more exact version of this is: return player->doc->get_barrier_item_count();
	// but its hijacked for other purposes at the moment, as there is just a single 
	// case where uncomitted operations should be expected: while recording parameter
	// changes.
	return player->automated_pattern ? 1 : 0;
}

void zzub_player_history_reset(zzub_player_t* player) {
	player->doc->clear_history();
}

int zzub_player_history_get_size(zzub_player_t* player) {
	return player->doc->get_history_length();
}

int zzub_player_history_get_position(zzub_player_t* player) {
	return player->doc->get_history_position();
}

const char* zzub_player_history_get_description(zzub_player_t* player, int position) {
	static std::string desc;
	desc = player->doc->get_history_description(position);
	return desc.c_str();
}

int zzub_player_history_enable(zzub_player_t* player, int state) {
	assert(state == 0 || state == 1);
	int laststate = player->doc->undoredo_enabled ? 1 : 0;
	player->doc->undoredo_enabled = state;
	return laststate;
}


int zzub_player_get_pluginloader_count(zzub_player_t* player) {
	return (int)player->plugmgr.plugin_infos.size();
}

zzub_pluginloader_t *zzub_player_get_pluginloader(zzub_player_t* player, int index) {
	return player->plugmgr.plugin_infos[index];
}

const int bmx_flag_ignore_patterns = 1;
const int bmx_flag_ignore_sequences = 2;
const int bmx_flag_ignore_waves = 4;

int zzub_player_load_bmx(zzub_player_t* player, zzub_input_t* datastream, int flags, float x, float y) {
	zzub::BuzzReader f(datastream);

	if (flags & bmx_flag_ignore_patterns) f.ignorePatterns = true;
	if (flags & bmx_flag_ignore_sequences) f.ignoreSequences = true;
	if (flags & bmx_flag_ignore_waves) f.ignoreWaves = true;
	f.offsetX = x;
	f.offsetY = y;

	bool result = f.readPlayer(player);
		
	//if (maxLen > 0) {
	//	string messageText = f.lastError + f.lastWarning;
	//	strncpy(messages, messageText.c_str(), maxLen - 1);
	//}
	if (!result) {
		cerr << "Errors:" << endl << f.lastError << endl << endl;
		cerr << "Warnings:" << endl << f.lastWarning << endl;

		return -1;
	}

	return 0;
}

int zzub_player_load_mod(zzub_player_t* player, const char* filename);

int zzub_player_load_module(zzub_player_t* player, const char* fileName) {
	return zzub_player_load_mod(player, fileName);
}

int zzub_player_load_armz(zzub_player_t* player, const char* fileName, int mode, zzub_plugin_group_t* plugingroup) {
	armstrong::storage::armzreader f;
	if (!f.open(fileName)) return -1;
	if (mode == 0) {
		bool result = f.load(player->doc.get());
		if (result) {
			// if the load succeeded but f.f is null, an unzipped sqlite db was loaded and thus no handle to the unzFile
			if (f.f) {
				// send event, views can listen to the player_load event and read from files in the zip via the archive/stream interfaces
				zip_archive arc;
				arc.unzf = f.f;

				zzub_event_data_t data;
				data.type = zzub_event_type_player_load;
				data.player_load.userdata = &arc;
				player->invoke_player_event(data);
			}
		}
		f.close();
		return result ? 0 : -1;
	} else
	if (mode == 1) {
		int plugingroup_id = plugingroup != 0 ? plugingroup->dataplugingroup->id : 0;
		bool result = f.import(player->doc.get(), plugingroup_id);
		f.close();
		return result ? 0 : -1;
	}
	return -1;
}

int zzub_player_save_armz(zzub_player_t* player, const char* fileName, const zzub_plugin_t** plugins, int plugincount, zzub_plugin_group_t* plugingroup) {

	// armzwriter has no access to the mixer, so first make sure the 
	// userplugins' private state is saved to the db in memory

	zzub_player_history_enable(player, 0);

	player->songdata->update(); // save tpb/bpm if it was changed in the audio thread

	armstrong::storage::tableiterator pit;
	player->songdata->get_plugins(&pit);
	while (!pit.eof()) {
		zzub_plugin_t* p = zzub_player_get_plugin_by_id(player, pit.id());
		zzub_plugin_save(p, 0);
		pit.next();
	}
	pit.destroy();

	zzub_player_history_commit(player, 0, 0, "Save Plugin States");
	zzub_player_history_enable(player, 1);

	armstrong::storage::armzwriter f;
	std::vector<int> pluginids(plugincount);
	for (int i = 0; i < plugincount; i++)
		pluginids[i] = zzub_plugin_get_id((zzub_plugin_t*)plugins[i]);

	// 
	player->doc->clear_unused_plugininfos();

	int plugingroup_id = plugingroup != 0 ? plugingroup->dataplugingroup->id : 0;

	if (!f.open(fileName)) return -1;

	bool result = f.add_song(player->doc.get(), pluginids, plugingroup_id);

	if (result) {
		// send event, views can listen to the player_save event and write to files in the zip via the archive/stream interfaces
		zip_archive arc;
		arc.zipf = f.f;

		zzub_event_data_t data;
		data.type = zzub_event_type_player_save;
		data.player_save.userdata = &arc;
		player->invoke_player_event(data);
	}
	f.close();
	return result ? 0 : -1;
}

int zzub_player_get_state(zzub_player_t* player) {
	return (int)player->mix->state;
}

void zzub_player_set_state(zzub_player_t* player, int state, int stoprow) {
	player->mix->set_state((zzub::player_state)state, stoprow);
}

int zzub_player_get_plugin_count(zzub_player_t* player) {

	return player->songdata->get_plugin_count();
}

const char* zzub_player_get_new_plugin_name(zzub_player_t* player, const char* uri) {
	const zzub::info* info = player->plugmgr.plugin_get_info(uri);
	assert(info);

	std::string newname;
	for (int i = 0; i < 9999; i++) {
		std::stringstream strm;
		if (i == 0)
			strm << info->short_name; 
		else 
			strm << info->short_name << (i+1);

		zzub_plugin_t* plug = player->get_plugin_by_name(strm.str());
		if (!plug) {
			newname = strm.str();
			break;
		}
	}
	static char name[256];
	strncpy(name, newname.c_str(), 256);
	return name;
}

zzub_plugin_t* zzub_player_get_plugin_by_name(zzub_player_t* player, const char* name) {

	armstrong::storage::plugindata plugdata;
	bool result = player->songdata->get_plugin_by_name(name, plugdata);
	if (!result) return 0;
	return player->plugins[plugdata.id].get();

}

zzub_plugin_t* zzub_player_get_plugin_by_id(zzub_player_t* player, int id) {
	if (id >= player->plugins.size()) return 0;
	return player->plugins[id].get();
}

zzub_plugin_group_t* zzub_player_get_plugin_group_by_id(zzub_player_t* player, int id) {
	if (id >= player->plugingroups.size()) return 0;
	return player->plugingroups[id].get();
}

zzub_plugin_iterator_t *zzub_player_get_plugin_iterator(zzub_player_t *player) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();

	player->songdata->get_plugins(it);

	armstrong::frontend::pluginiterator* frontendit = new armstrong::frontend::pluginiterator();
	frontendit->owner = player;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
}

zzub_pattern_format_t* zzub_player_get_pattern_format_by_id(zzub_player_t* player, int id) {
	if (id >= player->patternformats.size()) return 0;
	return player->patternformats[id].get();
}

zzub_pattern_t* zzub_player_get_pattern_by_id(zzub_player_t* player, int id) {
	if (id >= player->patterns.size()) return 0;
	return player->patterns[id].get();
}

zzub_pattern_t* zzub_player_get_pattern_by_index(zzub_player_t* player, int index) {
	armstrong::storage::patterndata patterndata;
	bool result = player->songdata->get_pattern_by_index(index, patterndata);
	assert(result);
	return player->patterns[patterndata.id].get();
}

zzub_pattern_event_t* zzub_player_get_pattern_event_by_id(zzub_player_t* player, int id) {
	return player->patternevents[id].get();
}

int zzub_player_get_pattern_count(zzub_player_t* player) {
	return player->songdata->get_pattern_count();
}

int zzub_player_get_pattern_format_count(zzub_player_t* player) {
	return player->songdata->get_pattern_format_count();
}

zzub_pattern_t* zzub_player_get_pattern_by_name(zzub_player_t* player, const char* name) {
	armstrong::storage::pattern patt(player->doc.get());
	bool result = player->songdata->get_pattern_by_name(name, patt);
	if (!result) return 0;

	return player->patterns[patt.id].get();
}

const char* zzub_player_get_new_pattern_name(zzub_player_t* player, zzub_pattern_format_t* format, const char* description) {
	std::stringstream strm;
	std::string pattern_name;

	if (description) {
		char const* check_description = description;
		for (; isdigit(*check_description) || isspace(*check_description); ++check_description);
		if (strcmp(check_description, "") == 0) {
			description = zzub_pattern_format_get_name(format);
		}
	}

	if (description)
		for (; isdigit(*description) || isspace(*description); ++description);

	for (int i = 0; i < 9999; ++i) {
		if (i < 100)
			strm << std::setw(2) << std::setfill('0') << i; else
		if (i < 1000)
			strm << std::setw(3) << std::setfill('0') << i; else
		if (i < 10000)
			strm << std::setw(4) << std::setfill('0') << i; else
		if (i < 100000)
			strm << std::setw(5) << std::setfill('0') << i;

		if (description)
			pattern_name = strm.str() + " " + description;
		else
			pattern_name = strm.str();

		zzub_pattern_t* p = zzub_player_get_pattern_by_name(player, const_cast<char*>(pattern_name.c_str()));
		if (p != 0) {
			strm.str("");
			pattern_name = "";
			continue;
		} else {
			break;
		}
	}

	static char name[256];
	strncpy(name, pattern_name.c_str(), 256);
	return name;
}

zzub_pattern_format_t* zzub_player_get_pattern_format_by_name(zzub_player_t* player, const char* name) {
	armstrong::storage::patternformatdata formatdata;
	bool result = player->songdata->get_patternformat_by_name(name, formatdata);
	if (!result) return 0;

	return player->patternformats[formatdata.id].get();
}

const char* zzub_player_get_new_pattern_format_name(zzub_player_t* player, const char* description) {
	std::string s;

	if (description == 0) {
		s = "Format";
	} else {
		s = description;

		{	// rtrim
			std::string::difference_type dt;
			std::string::reverse_iterator it;
			for (it = s.rbegin(); it != s.rend(); ++it)
				if (!isspace((unsigned char)*it) && !isdigit((unsigned char)*it))
					break;
			dt = s.rend() - it;
			s.erase(s.begin() + dt, s.end());
		}
	}

	string newname;
	std::stringstream strm;
	//int start_idx = (description == 0) ? 1 : 0;
	for (int i = 1; i < 9999; ++i) {
		strm.str("");

		if (i == 1)
			strm << s;
		else
			strm << s << i;

		zzub_pattern_format_t* format = zzub_player_get_pattern_format_by_name(player, strm.str().c_str());
		if (!format) {
			newname = strm.str();
			break;
		}
	}

	static char name[256];
	strncpy(name, newname.c_str(), 256);
	return name;
}

zzub_pattern_format_t* zzub_player_get_pattern_format_by_index(zzub_player_t* player, int index) {
	armstrong::storage::patternformatdata formatdata;
	bool result = player->songdata->get_patternformat_by_index(index, formatdata);
	assert(result);
	return player->patternformats[formatdata.id].get();
}

zzub_plugin_t* zzub_player_get_plugin(zzub_player_t* player, int index) {
	armstrong::storage::plugindata plugdata;
	bool result = player->songdata->get_plugin_by_index(index, plugdata);
	assert(result);
	return player->plugins[plugdata.id].get();
}

void zzub_player_clear(zzub_player_t* player) {
	player->armclient.disconnect();
	player->reset();
}

int zzub_player_get_position_samples(zzub_player_t* player) {
	return player->mix->song_position;
}

int zzub_player_get_position_row(zzub_player_t* player) {
	return player->mix->rootplayer->pattern_row;
}

int zzub_player_get_position_order(zzub_player_t* player) {
	return player->mix->orderlist_position;
}

void zzub_player_set_position(zzub_player_t* player, int orderindex, int pos) {
	player->mix->set_play_position(orderindex, pos);
}

void zzub_player_set_order_loop_enabled(zzub_player_t* player, int enable) {
	player->songdata->loopenabled = enable != 0 ? 1 : 0;
	player->songdata->update();
}

int zzub_player_get_order_loop_enabled(zzub_player_t* player) {
	return player->songdata->loopenabled != 0 ? 1 : 0;
}

int zzub_player_get_wave_count(zzub_player_t* player) {
	return player->songdata->get_wave_count();
}

zzub_wave_t* zzub_player_get_wave(zzub_player_t* player, int index) {
	armstrong::storage::wavedata data;
	player->songdata->get_wave_by_index(index, data);

	return player->waves[data.id].get();
}

void zzub_player_add_callback(zzub_player_t* player, zzub_callback_t callback, void* tag) {
	armstrong::frontend::player::callbackpair cb(callback, tag);
	player->callbacks.push_back(cb);
}

void zzub_player_remove_callback(zzub_player_t* player, zzub_callback_t callback, void* tag) {
	armstrong::frontend::player::callbackpair cb(callback, tag);
	vector<armstrong::frontend::player::callbackpair>::iterator i = find(player->callbacks.begin(), player->callbacks.end(), cb);
	if (i != player->callbacks.end())
		player->callbacks.erase(i);
}

void zzub_player_handle_events(zzub_player_t* player) {
	player->armclient.handle_messages();
	player->mix->process_user_event_queue();
}

int zzub_player_get_automation(zzub_player_t* player) {
	return (int)player->mix->is_recording_parameters;
}

void zzub_player_set_automation(zzub_player_t* player, int enable) {
	player->mix->is_recording_parameters = enable!=0?true:false;
}

int zzub_player_get_midi_transport(zzub_player_t* player) {
	return player->mix->is_syncing_midi_transport;
}

void zzub_player_set_midi_transport(zzub_player_t* player, int enable) {
	player->mix->is_syncing_midi_transport = enable!=0?true:false;
}

void zzub_player_reset_keyjazz(zzub_player_t* player) {
	player->mix->reset_keyjazz();
}

const char *zzub_player_get_infotext(zzub_player_t* player) {
	if (player->songdata->comment.size() == 0) return "";
	return (const char*)&player->songdata->comment.front();
}

void zzub_player_set_infotext(zzub_player_t* player, const char* text) {
	int len = strlen(text);
	player->songdata->comment.resize(len + 1);
	memcpy(&player->songdata->comment.front(), text, len + 1);
	player->songdata->update();
}

float zzub_player_get_bpm(zzub_player_t* player) {
	return player->songdata->bpm;
}

int zzub_player_get_tpb(zzub_player_t* player) {
	return player->songdata->tpb;
}

float zzub_player_get_swing(zzub_player_t* player) {
	return player->songdata->swing;
}

void zzub_player_set_bpm(zzub_player_t* player, float bpm) {
	player->songdata->bpm = bpm;
	player->songdata->update();
}

void zzub_player_set_tpb(zzub_player_t* player, int tpb) {
	player->songdata->tpb = tpb;
	player->songdata->update();
}

void zzub_player_set_swing(zzub_player_t* player, float swing) {
	player->songdata->swing = swing;
	player->songdata->update();
}

void zzub_player_set_swing_ticks(zzub_player_t* player, int swing_ticks) {
	player->songdata->swingticks = swing_ticks;
	player->songdata->update();
}

void zzub_player_set_midi_plugin(zzub_player_t* player, zzub_plugin_t* plugin) {
	if (!player->midi_lock) {
		if (plugin == 0) {
			player->mix->set_midi_plugin(-1);
		} else {
			player->mix->set_midi_plugin(plugin->dataplugin->id);
		}
	}
}

zzub_plugin_t* zzub_player_get_midi_plugin(zzub_player_t* player) {
	if (player->mix->midi_plugin == -1) return 0;
	return zzub_player_get_plugin_by_id(player, player->mix->midi_plugin);
}

void zzub_player_set_host_info(zzub_player_t* player, int id, int version, void *host_ptr) {
	player->mix->hostinfo.id = id;
	player->mix->hostinfo.version = version;
	player->mix->hostinfo.host_ptr = host_ptr;
}

/*bool*/ int zzub_player_get_midi_lock(zzub_player_t *player) {
	return player->midi_lock;
}

void zzub_player_set_midi_lock(zzub_player_t *player, int state) {
	player->midi_lock = (state != 0);
}

int zzub_player_invoke_event(zzub_player_t* player, zzub_event_data_t *data, int immediate) {
	return player->invoke_player_event(*data)?0:-1;
}

// midimapping functions

int zzub_midimapping_get_plugin(zzub_midimapping_t* mapping) {
	return (int)mapping->datamidimapping->plugin_id;
}

int zzub_midimapping_get_group(zzub_midimapping_t* mapping) {
	return (int)mapping->datamidimapping->paramgroup;
}

int zzub_midimapping_get_track(zzub_midimapping_t* mapping) {
	return (int)mapping->datamidimapping->paramtrack;
}

int zzub_midimapping_get_column(zzub_midimapping_t* mapping) {
	return (int)mapping->datamidimapping->paramcolumn;
}

int zzub_midimapping_get_channel(zzub_midimapping_t* mapping) {
	return (int)mapping->datamidimapping->midichannel;
}

int zzub_midimapping_get_controller(zzub_midimapping_t* mapping) {
	return (int)mapping->datamidimapping->midicontroller;
}

// iterator retreival

zzub_pattern_iterator_t* zzub_player_get_pattern_iterator(zzub_player_t* player) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();

	player->songdata->get_patterns(it);

	armstrong::frontend::patterniterator* frontendit = new armstrong::frontend::patterniterator();
	frontendit->owner = player;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
}

zzub_pattern_format_iterator_t* zzub_player_get_pattern_format_iterator(zzub_player_t* player) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();

	player->songdata->get_patternformats(it);

	armstrong::frontend::patternformatiterator* frontendit = new armstrong::frontend::patternformatiterator();
	frontendit->owner = player;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
}



/***

	Order list methods

***/

void zzub_player_set_order_length(zzub_player_t* player, int length) {
	player->songdata->set_order_length(length);
}

int zzub_player_get_order_length(zzub_player_t* player) {
	return player->songdata->get_order_length();
}

void zzub_player_set_order_pattern(zzub_player_t* player, int index, zzub_pattern_t* pattern) {
	player->songdata->set_order_pattern(index, pattern ? pattern->datapattern->id : 0);
}

zzub_pattern_t* zzub_player_get_order_pattern(zzub_player_t* player, int index) {
	int patternid = player->songdata->get_order_pattern_id(index);
	if (patternid == 0) return 0;
	return player->patterns[patternid].get();
}

zzub_pattern_iterator_t *zzub_player_get_order_iterator(zzub_player_t* player) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();

	player->songdata->get_order_patterns(it);

	armstrong::frontend::patterniterator* frontendit = new armstrong::frontend::patterniterator();
	frontendit->owner = player;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
}

int zzub_player_get_order_loop_start(zzub_player_t* player) {
	return player->songdata->loopbegin;
}

void zzub_player_set_order_loop_start(zzub_player_t* player, int pos) {
	player->songdata->loopbegin = pos;
	player->songdata->update();
}

int zzub_player_get_order_loop_end(zzub_player_t* player) {
	return player->songdata->loopend;
}

void zzub_player_set_order_loop_end(zzub_player_t* player, int pos) {
	player->songdata->loopend = pos;
	player->songdata->update();
}

void zzub_player_set_queue_order_index(zzub_player_t* player, int pos) {
	player->order_queue_index = pos;
	player->mix->set_queue_index(pos);
}

int zzub_player_get_queue_order_index(zzub_player_t* player) {
	return player->order_queue_index; 

	// NOTE: this may not yet be updated in the audio thread after a set_queue_index
	//return player->mix->queue_index;
}

void zzub_player_adjust_position_order(zzub_player_t* player, int pos) {
	player->mix->set_order_position(pos);
}

void zzub_player_timeshift_order(zzub_player_t* player, int fromindex, int timeshift) {
	if (player->order_queue_index >= fromindex) {
		if (player->order_queue_index + timeshift < fromindex)
			player->order_queue_index = fromindex;
		else
			player->order_queue_index += timeshift;
	}

	player->songdata->timeshift_order(fromindex, timeshift);
}


/***

	Time source methods

***/

int zzub_player_get_timesource_count(zzub_player_t* player) {
	return player->mix->patternplayers.next().size();
}

zzub_plugin_t* zzub_player_get_timesource_plugin(zzub_player_t* player, int index) {
	zzub::plugin* plugin = player->mix->patternplayers.next()[index]->_plugin;
	if (!plugin) return 0;
	return player->plugins[plugin->_id].get();
}

int zzub_player_get_timesource_group(zzub_player_t* player, int index) {
	return player->mix->patternplayers.next()[index]->plugin_group;
}

int zzub_player_get_timesource_track(zzub_player_t* player, int index) {
	return player->mix->patternplayers.next()[index]->plugin_track;
}

void zzub_player_play_pattern(zzub_player_t* player, zzub_pattern_t* pat, int row, int stoprow) {
	player->mix->play_pattern(pat->datapattern->id, row, -1);
}

/***

	Machine view offset

***/

double zzub_player_get_machineview_offset_x(zzub_player_t* player) {
	return player->songdata->machineview_x;
}

double zzub_player_get_machineview_offset_y(zzub_player_t* player) {
	return player->songdata->machineview_y;
}

void zzub_player_set_machineview_offset(zzub_player_t* player, double x, double y) {
	player->songdata->machineview_x = x;
	player->songdata->machineview_y = y;
	player->songdata->update();
}

/***

	Multithreading

***/

void zzub_player_set_thread_count(zzub_player_t* player, int threads) {
	assert(threads >= 1);
	player->mix->set_thread_count(threads);
}

int zzub_player_get_thread_count(zzub_player_t* player) {
	int workerthreads = player->mix->threadworkers.next().size();
	if (workerthreads == 0) return 1;
	return workerthreads;
}

void zzub_player_get_peaks(zzub_player_t *player, float *peaks, int *peakcount) {
	for (int i = 0; i < player->work_out_channel_count; i++) {
		peaks[i] = player->last_max_peak[player->work_out_first_channel + i];
	}
	*peakcount = player->work_out_channel_count;
}

zzub_plugin_group_t* zzub_player_create_plugin_group(zzub_player_t* player, zzub_plugin_group_t* parent, const char* name) {
	int parent_id = (parent != 0) ? parent->dataplugingroup->id : 0;
	armstrong::storage::plugingroupdata data;
	if (!player->songdata->create_plugingroup(parent_id, name, data))
		return 0;

	return player->plugingroups[data.id].get();
}

zzub_plugin_group_iterator_t* zzub_player_get_plugin_group_iterator(zzub_player_t* player, zzub_plugin_group_t* parent) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();

	int parent_id = (parent != 0) ? parent->dataplugingroup->id : 0;
	player->songdata->get_plugingroups(parent_id, it);

	armstrong::frontend::plugingroupiterator* frontendit = new armstrong::frontend::plugingroupiterator();
	frontendit->owner = player;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
	
}

/***

	Plugin parameter validation

***/

zzub_validation_error_iterator_t* zzub_player_get_validation_errors(zzub_player_t* player) {
	armstrong::frontend::validationerroriterator* iterator = new armstrong::frontend::validationerroriterator();
	iterator->i = player->load_errors.begin();
	iterator->owner = player;
	return iterator;
}

/* class PluginIterator */

/** \brief Iterate to the next item. */
void zzub_validation_error_iterator_next(zzub_validation_error_iterator_t *iterator) {
	iterator->i++;
}

/** \brief Returns true if the iterator is valid and it is safe to call current() */
int zzub_validation_error_iterator_valid(zzub_validation_error_iterator_t *iterator) {
	return iterator->i != iterator->owner->load_errors.end() ? true : false;
}

/** \brief Returns the current item. */
zzub_validation_error_t *zzub_validation_error_iterator_current(zzub_validation_error_iterator_t *iterator) {
	return &*iterator->i;
}

/** \brief Resets the iterator. */
void zzub_validation_error_iterator_reset(zzub_validation_error_iterator_t *iterator) {
	iterator->i = iterator->owner->load_errors.begin();
}

/** \brief Resets the iterator. */
void zzub_validation_error_iterator_destroy(zzub_validation_error_iterator_t *iterator) {
	delete iterator;
}



int zzub_validation_error_get_type(zzub_validation_error_t *validationerror) {
	return validationerror->type;
}

int zzub_validation_error_get_group(zzub_validation_error_t *validationerror) {
	return validationerror->group;
}

int zzub_validation_error_get_column(zzub_validation_error_t *validationerror) {
	return validationerror->column;
}

int zzub_validation_error_get_found_value(zzub_validation_error_t *validationerror) {
	return validationerror->found_value;
}

int zzub_validation_error_get_expected_value(zzub_validation_error_t *validationerror) {
	return validationerror->expected_value;
}

const char *zzub_validation_error_get_parameter_name(zzub_validation_error_t *validationerror) {
	return validationerror->original_parameter_name.c_str();
}

const char* zzub_validation_error_get_plugin_name(zzub_validation_error_t *validationerror) {
	return validationerror->original_plugin_name.c_str();
}

zzub_pluginloader_t *zzub_validation_error_get_pluginloader(zzub_validation_error_t *validationerror) {
	return validationerror->info;
}

// Plugincollection

zzub_plugincollection_t *zzub_plugincollection_get_by_uri(zzub_player_t* player, const char *uri) {
	return player->plugmgr.get_collection(uri);
}

void zzub_plugincollection_configure(zzub_plugincollection_t* collection, const char* key, const char* value) {
	collection->configure(key, value);
}

const char* zzub_plugincollection_get_name(zzub_plugincollection_t* collection) {
	return collection->get_name();
}

/* class Layer */

void zzub_plugin_group_destroy(zzub_plugin_group_t* plugingroup) {
	plugingroup->dataplugingroup->destroy();
}

int zzub_plugin_group_get_id(zzub_plugin_group_t* plugingroup) {
	return plugingroup->dataplugingroup->id;
}

const char* zzub_plugin_group_get_name(zzub_plugin_group_t* plugingroup) {
	return plugingroup->dataplugingroup->name.c_str();
}

void zzub_plugin_group_set_name(zzub_plugin_group_t* plugingroup, const char* name) {
	plugingroup->dataplugingroup->name = name;
	plugingroup->dataplugingroup->update();
}

zzub_plugin_group_t* zzub_plugin_group_get_parent(zzub_plugin_group_t* plugingroup) {
	return plugingroup->owner->plugingroups[plugingroup->dataplugingroup->parent_plugingroup_id].get();
}

void zzub_plugin_group_set_parent(zzub_plugin_group_t* plugingroup, zzub_plugin_group_t* newparent) {
	assert(plugingroup != newparent); // TODO: prevent cyclic groups?
	plugingroup->dataplugingroup->parent_plugingroup_id = newparent != 0 ? newparent->dataplugingroup->id : 0;
	plugingroup->dataplugingroup->update();
}

float zzub_plugin_group_get_position_x(zzub_plugin_group_t* plugingroup) {
	return plugingroup->dataplugingroup->position_x;
}

float zzub_plugin_group_get_position_y(zzub_plugin_group_t* plugingroup) {
	return plugingroup->dataplugingroup->position_y;
}

void zzub_plugin_group_set_position(zzub_plugin_group_t* plugingroup, float x, float y) {
	plugingroup->dataplugingroup->position_x = x;
	plugingroup->dataplugingroup->position_y = y;
	plugingroup->dataplugingroup->update();
}

zzub_plugin_iterator_t* zzub_plugin_group_get_plugins(zzub_plugin_group_t* plugingroup) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();

	plugingroup->dataplugingroup->get_plugins(it);

	armstrong::frontend::pluginiterator* frontendit = new armstrong::frontend::pluginiterator();
	frontendit->owner = plugingroup->owner;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
}


/* class LayerIterator */

/** \brief Iterate to the next item. */
void zzub_plugin_group_iterator_next(zzub_plugin_group_iterator_t *plugingroupiterator) {
	plugingroupiterator->recordset->next();
}

/** \brief Returns true if the iterator is valid and it is safe to call current() */
int zzub_plugin_group_iterator_valid(zzub_plugin_group_iterator_t *plugingroupiterator) {
	return plugingroupiterator->recordset->eof() ? 0 : 1;
}

/** \brief Returns the current item. */
zzub_plugin_group_t *zzub_plugin_group_iterator_current(zzub_plugin_group_iterator_t *plugingroupiterator) {
	static armstrong::storage::plugingroupdata pev;
	return plugingroupiterator->owner->plugingroups[plugingroupiterator->recordset->id()].get();
}

/** \brief Destroys the iterator. */
void zzub_plugin_group_iterator_destroy(zzub_plugin_group_iterator_t *plugingroupiterator) {
	plugingroupiterator->recordset->destroy();
	//delete patterneventiterator->recordset;
	delete plugingroupiterator;
}

}
