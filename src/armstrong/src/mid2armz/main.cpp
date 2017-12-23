#define _USE_MATH_DEFINES
#include <zzub/zzub.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include "midiparser.h"

/*
TODO conversion modes:

 - create 1 big pattern with everything in one place
 - create a pattern per midi track

 - always separate midi channels into separate miditrackers, y/n


*/


using std::cout;
using std::endl;

zzub_pattern_format_t* create_default_format(zzub_player_t* player, zzub_plugin_t* plugin) {
	assert(plugin != 0);

	zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(plugin);
	std::string uri = zzub_pluginloader_get_uri(loader);

	zzub_pattern_format_t* patfmt = zzub_player_create_pattern_format(player, 0);//todo, could use a more descriptive name

	int screenidx = 0;

	// global parameters
	for (int j = 0; j < zzub_pluginloader_get_parameter_count(loader, 1); ++j) {
		zzub_parameter_t* para = zzub_pluginloader_get_parameter(loader, 1, j);
		zzub_pattern_format_add_column(patfmt, plugin, 1, 0, j, screenidx);
		++screenidx;
	}

	// track parameters
	for (int i = 0; i < zzub_plugin_get_track_count(plugin, 2); ++i) {
		for (int j = 0; j < zzub_pluginloader_get_parameter_count(loader, 2); ++j) {
			zzub_parameter_t* para = zzub_pluginloader_get_parameter(loader, 2, j);
			zzub_pattern_format_add_column(patfmt, plugin, 2, i, j, screenidx);
			++screenidx;
		}
	}

	const char* pluginname = zzub_plugin_get_name(plugin);
	std::stringstream name;
	name << pluginname << " (Default Format)";
	zzub_pattern_format_set_name(patfmt, name.str().c_str());

	return patfmt;
}

struct midiimporter {
	midifile f;
	zzub_player_t* player;

	bool import(zzub_player_t* player, std::string filename);
	bool import_track(miditrack& track, zzub_plugin_t* miditracker, zzub_pattern_t* pat, int midichannel, int firsttrackindex);
};

bool midiimporter::import(zzub_player_t* player, std::string filename) {

	std::ifstream strm;
	strm.open(filename.c_str(), std::ios::binary | std::ios::in);
	if (!strm) return false;

	bool result = f.parse(strm);
	strm.close();
	if (!result) return false;

	int maxlength = 0;
	for (std::vector<miditrack>::iterator i = f.tracks.begin(); i != f.tracks.end(); ++i) {
		maxlength = std::max(maxlength, i->get_length());
	}

	zzub_pluginloader_t* miditrackerinfo = zzub_player_get_pluginloader_by_name(player, "@zzub.org/miditracker;1");
	zzub_pluginloader_t* seqplayerinfo = zzub_player_get_pluginloader_by_name(player, "@zzub.org/sequence/sequence");
	zzub_pluginloader_t* patplayerinfo = zzub_player_get_pluginloader_by_name(player, "@zzub.org/sequence/pattern");

	zzub_pattern_format_t* patfmt = zzub_player_create_pattern_format(player, 0);//todo, could use a more descriptive name
	zzub_pattern_t* seqpat = zzub_player_create_pattern(player, patfmt, "Sequence", maxlength);

	zzub_plugin_t* patplugin = zzub_player_create_plugin(player, 0, 0, "Pattern", patplayerinfo, 0);
	zzub_plugin_set_track_count(patplugin, (int)f.tracks.size());
	zzub_plugin_set_position(patplugin, 0.25f, -0.25f);

	zzub_plugin_t* seqplugin = zzub_player_create_plugin(player, 0, 0, "Sequence", seqplayerinfo, 0);
	zzub_plugin_set_position(seqplugin, 0.25f, 0.25f);

	zzub_player_set_order_length(player, 1);
	zzub_player_set_order_pattern(player, 0, seqpat);

	int max_track_count = zzub_pluginloader_get_tracks_max(miditrackerinfo);

	for (std::vector<miditrack>::iterator i = f.tracks.begin(); i != f.tracks.end(); ++i) {
		std::string name = i->name;
		int index = (int)(i - f.tracks.begin());
		zzub_pattern_format_add_column(patfmt, patplugin, 2, index, 0, index);

		if (name.empty()) {
			std::stringstream namestrm;
			namestrm << "Track " << (index + 1);
			name = namestrm.str();
		}

		zzub_plugin_t* miditracker = zzub_player_create_plugin(player, 0, 0, name.c_str(), miditrackerinfo, 0);

		// count needed voices per midi channel
		int maxvoices = 0;
		for (int j = 0; j < 16; j++) {
			maxvoices += i->get_channel_polyphony(j);
		}

		assert(maxvoices <= max_track_count);

		zzub_plugin_set_track_count(miditracker, maxvoices);
		float plugindegree = ((float)index / f.tracks.size()) * 2.0f * M_PI;
		zzub_plugin_set_position(miditracker, cos(plugindegree) * 0.9, sin(plugindegree) * 0.9);
		zzub_pattern_format_t* fmt = create_default_format(player, miditracker);

		int mult = i->get_time_multiplier(f.header.delta);
		zzub_pattern_t* trackpat = zzub_player_create_pattern(player, fmt, name.c_str(), i->get_length() / mult);

		int startvoice = 0;
		for (int j = 0; j < 16; j++) {
			import_track(*i, miditracker, trackpat, j, startvoice);
			startvoice += i->get_channel_polyphony(j);
		}

		int trackpatid = zzub_pattern_get_id(trackpat);
		int patpluginid = zzub_plugin_get_id(patplugin);
		zzub_pattern_insert_value(seqpat, patpluginid, 2, index, 0, 0, trackpatid, 0);

	}

	return true;
}

int midi_to_buzz_note(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}

bool midiimporter::import_track(miditrack& track, zzub_plugin_t* miditracker, zzub_pattern_t* pat, int midichannel, int firsttrackindex) {
	int mult = track.get_time_multiplier(f.header.delta);
	int pluginid = zzub_plugin_get_id(miditracker);
	int note;
	channelmanager trkmgr;
	int trkindex;
	int lasttime = 0;

	for (std::vector<midievent>::iterator i = track.events.begin(); i != track.events.end(); ++i) {
		int timestamp = i->timestamp / mult;
		if (timestamp - lasttime > 0) {
			lasttime = timestamp;
			trkmgr.clear_released();
		}
		if (i->channel != midichannel) continue;

		switch (i->command) {
			case 8:
				// note off
				note = midi_to_buzz_note(i->nv.note);
				trkindex = trkmgr.release_channel(note) + firsttrackindex;
				zzub_pattern_insert_value(pat, pluginid, 2, trkindex, 0, timestamp, zzub_note_value_off, 0);
				zzub_pattern_insert_value(pat, pluginid, 2, trkindex, 8, timestamp, i->channel + 1, 0);
				break;
			case 9:
				note = midi_to_buzz_note(i->nv.note);
				if (i->nv.velocity != 0) {
					// note on
					trkindex = trkmgr.allocate_channel(note) + firsttrackindex;
					zzub_pattern_insert_value(pat, pluginid, 2, trkindex, 0, timestamp, note, 0);
					zzub_pattern_insert_value(pat, pluginid, 2, trkindex, 1, timestamp, i->nv.velocity, 0);
					zzub_pattern_insert_value(pat, pluginid, 2, trkindex, 8, timestamp, i->channel + 1, 0);
				} else {
					// note off
					trkindex = trkmgr.release_channel(note) + firsttrackindex;
					zzub_pattern_insert_value(pat, pluginid, 2, trkindex, 0, timestamp, zzub_note_value_off, 0);
					zzub_pattern_insert_value(pat, pluginid, 2, trkindex, 8, timestamp, i->channel + 1, 0);
				}
				break;
			case 0xB:
				// cc 0 = bank select
				// cc 10 = pan
				// cc 7 = main volume
				cout << "control change, chn = " << i->channel << ", cc = " << i->nv.note << ", val = " << i->nv.velocity << endl;
				break;
			case 0xC: // program change
				cout << "program change " << i->program << endl;
				zzub_pattern_insert_value(pat, pluginid, 1, 0, 4, timestamp, i->program, 0);
				break;
			case 0xF:
				switch (i->metacommand) {
					case 0x51: // tempo
						cout << "unhandled temp = " << i->metacommand << endl;
						cout << "musec = " << i->t.musec << endl;
						cout << "bpm = " << (60000000/i->t.musec) << endl;
						break;
					case 0x3: // instrument name
					case 0x4: // track name
					case 0x58: // timesig
					default:
						cout << "unhandled meta = " << i->metacommand << endl;
						break;
				}
				break;
			default:
				cout << "unhandled command " << i->command << endl;
				break;
		}
	}
	return true;
}


int zzub_player_import_mid(zzub_player_t* player, const char* filename) {
	midiimporter midi;
	return midi.import(player, filename) ? 0 : -1;
}

int main(int argc, char **argv) {

	if (argc != 3) {
		printf("usage: mid2armz midfile armzfile\n\n");
		return 1;
	}
	const char* midfile = argv[1];
	const char* armzfile = argv[2];

	zzub_player_t* player = zzub_player_create(0, 0, 0);
	zzub_player_initialize(player, 0);

	if (zzub_player_import_mid(player, midfile) != 0) {
		printf("cant open mid");
		return 2;
	}
	zzub_player_history_commit(player, 0, 0, "OK");

	zzub_player_save_armz(player, armzfile, 0, 0, 0);

	zzub_player_destroy(player);

	return 0;
}
