#include <cstring>
#include "library.h"
#include "player.h"

using std::vector;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

extern "C" {

void zzub_pattern_destroy(zzub_pattern_t* pattern) {
	pattern->datapattern->destroy();
}

const char* zzub_pattern_get_name(zzub_pattern_t* pattern) {
	std::string& str = pattern->datapattern->name;
	static char name[256];
	strncpy(name, str.c_str(), 256);
	return name;
}

void zzub_pattern_set_name(zzub_pattern_t* pattern, const char* name) {
	pattern->datapattern->name = name;
	pattern->datapattern->update();
}

int zzub_pattern_get_row_count(zzub_pattern_t* pattern) {
	return pattern->datapattern->length;
}

void zzub_pattern_set_row_count(zzub_pattern_t* pattern, int length) {
	pattern->datapattern->length = length;
	if (pattern->datapattern->beginloop >= length)
 		pattern->datapattern->beginloop = 0;
	if (pattern->datapattern->endloop > length)
		pattern->datapattern->endloop = length;
	if (pattern->datapattern->replay_row >= length)
		pattern->datapattern->replay_row = 0;
	pattern->datapattern->update();
}

// --- events ---

void zzub_pattern_set_value(zzub_pattern_t* pattern, int row, zzub_plugin_t* plugin, int group, int track, int column, int value, int meta) {
	assert(value >= 0);
	zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, group, track, column);
	if (value == param->value_none) {
		pattern->datapattern->remove_event_at(plugin->dataplugin->id, group, track, column, row);
	} else {
		pattern->datapattern->set_event_at(plugin->dataplugin->id, group, track, column, row, value, meta);
	}
}

int zzub_pattern_get_value(zzub_pattern_t* pattern, int row, int pluginid, int group, int track, int column, int* value, int* meta) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	return pattern->datapattern->get_event_at(pp->datapluginparameter->id, row, value, meta);
}

void zzub_pattern_insert_value(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int time, int value, int meta) {
	assert(value >= 0);
	pattern->datapattern->insert_value(pluginid, group, track, column, time, value, meta);
}

void zzub_pattern_delete_value(zzub_pattern_t* pattern, int id) {
	pattern->datapattern->delete_value(id);
}

void zzub_pattern_update_value(zzub_pattern_t* pattern, int id, int time, int value, int meta) {
	assert(value >= 0);
	pattern->datapattern->update_value(id, time, value, meta);
}

void zzub_pattern_update_value_full(zzub_pattern_t* pattern, int id, int pluginid, int group, int track, int column, int time, int value, int meta) {
	assert(value >= 0);
	pattern->datapattern->update_value(id, pluginid, group, track, column, time, value, meta);
}

// --- pattern mutations ---

void zzub_pattern_expand_pattern(zzub_pattern_t* pattern, int factor) {
	pattern->datapattern->expand_pattern(factor);
}

void zzub_pattern_compact_pattern(zzub_pattern_t* pattern, int factor) {
	pattern->datapattern->compact_pattern(factor);
}

// --- edit ops ---

void zzub_pattern_timeshift_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int timeshift) {

	zzub_pattern_format_t* patternformat = zzub_pattern_get_format(pattern);
	zzub_pattern_format_column_iterator_t* it = zzub_pattern_format_get_iterator(patternformat);

	while (zzub_pattern_format_column_iterator_valid(it)) {
		zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_column_iterator_current(it);
		zzub_plugin_t* plug = zzub_pattern_format_column_get_plugin(fmtcol);
		int fmtpluginid = zzub_plugin_get_id(plug);
		int fmtgroup = zzub_pattern_format_column_get_group(fmtcol);
		int fmttrack = zzub_pattern_format_column_get_track(fmtcol);
		int fmtcolumn = zzub_pattern_format_column_get_column(fmtcol);

		if ((pluginid == -1 && group == -1 && track == -1 && column == -1) ||
			(pluginid == fmtpluginid && group == -1 && track == -1 && column == -1) ||
			(pluginid == fmtpluginid && group == fmtgroup && track == -1 && column == -1) ||
			(pluginid == fmtpluginid && group == fmtgroup && track == fmttrack && column == -1) ||
			(pluginid == fmtpluginid && group == fmtgroup && track == fmttrack && column == fmtcolumn)
			) {

			armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(fmtpluginid, fmtgroup, fmttrack, fmtcolumn);
			assert(pp != 0);
			pattern->datapattern->timeshift_events(pp->datapluginparameter->id, fromtime, timeshift);

		}
		zzub_pattern_format_column_iterator_next(it);
	}

	zzub_pattern_format_column_iterator_destroy(it);
}

void zzub_pattern_delete_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length) {

	zzub_pattern_format_t* patternformat = zzub_pattern_get_format(pattern);
	zzub_pattern_format_column_iterator_t* it = zzub_pattern_format_get_iterator(patternformat);

	while (zzub_pattern_format_column_iterator_valid(it)) {
		zzub_pattern_format_column_t* fmtcol = zzub_pattern_format_column_iterator_current(it);
		zzub_plugin_t* plug = zzub_pattern_format_column_get_plugin(fmtcol);
		int fmtpluginid = zzub_plugin_get_id(plug);
		int fmtgroup = zzub_pattern_format_column_get_group(fmtcol);
		int fmttrack = zzub_pattern_format_column_get_track(fmtcol);
		int fmtcolumn = zzub_pattern_format_column_get_column(fmtcol);

		if ((pluginid == -1 && group == -1 && track == -1 && column == -1) ||
			(pluginid == fmtpluginid && group == -1 && track == -1 && column == -1) ||
			(pluginid == fmtpluginid && group == fmtgroup && track == -1 && column == -1) ||
			(pluginid == fmtpluginid && group == fmtgroup && track == fmttrack && column == -1) ||
			(pluginid == fmtpluginid && group == fmtgroup && track == fmttrack && column == fmtcolumn)
			) {

			armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(fmtpluginid, fmtgroup, fmttrack, fmtcolumn);
			assert(pp != 0);
			pattern->datapattern->delete_events(pp->datapluginparameter->id, fromtime, length);
		}
		zzub_pattern_format_column_iterator_next(it);
	}

	zzub_pattern_format_column_iterator_destroy(it);
}

void zzub_pattern_move_scale_events(zzub_pattern_t* pattern,
	int src_idx, int src_time,
	int dst_idx, int dst_time,
	int width, int length, int mode, int makecopy
) {
	pattern->datapattern->move_scale_events(
		src_idx, src_time,
		dst_idx, dst_time,
		width, length, mode, makecopy
	);
}

void zzub_pattern_paste_stream_events(zzub_pattern_t* pattern, int fromidx, int fromtime, int mode, char const* charbuf) {
	pattern->datapattern->paste_stream_events(fromidx, fromtime, mode, charbuf);
}

// --- selection mutations ---

void zzub_pattern_transpose_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int delta, int* holes, int holecount, int* metas, int metacount, int chromatic) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->transpose_events(pp->datapluginparameter->id, fromtime, length, delta, holes, holecount, metas, metacount, chromatic);
}

void zzub_pattern_randomize_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int skip) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->randomize_events(pp->datapluginparameter->id, fromtime, length, skip);
}

void zzub_pattern_randomize_range_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int from_val, int to_val, int skip) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->randomize_range_events(pp->datapluginparameter->id, fromtime, length, from_val, to_val, skip);
}

void zzub_pattern_randomize_from_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int skip) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->randomize_from_events(pp->datapluginparameter->id, fromtime, length, skip);
}

void zzub_pattern_humanize_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int deviation) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->humanize_events(pp->datapluginparameter->id, fromtime, length, deviation);
}

void zzub_pattern_shuffle_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->shuffle_events(pp->datapluginparameter->id, fromtime, length);
}

void zzub_pattern_interpolate_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int skip) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->interpolate_events(pp->datapluginparameter->id, fromtime, length, skip);
}

void zzub_pattern_gradiate_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int skip) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->gradiate_events(pp->datapluginparameter->id, fromtime, length, skip);
}

void zzub_pattern_smooth_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int strength) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->smooth_events(pp->datapluginparameter->id, fromtime, length, strength);
}

void zzub_pattern_reverse_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->reverse_events(pp->datapluginparameter->id, fromtime, length);
}

void zzub_pattern_compact_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int factor) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->compact_events(pp->datapluginparameter->id, fromtime, length, factor);
}

void zzub_pattern_expand_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int factor) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->expand_events(pp->datapluginparameter->id, fromtime, length, factor);
}

void zzub_pattern_thin_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int major) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->thin_events(pp->datapluginparameter->id, fromtime, length, major);
}

void zzub_pattern_repeat_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int major) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->repeat_events(pp->datapluginparameter->id, fromtime, length, major);
}

void zzub_pattern_echo_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int major) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->echo_events(pp->datapluginparameter->id, fromtime, length, major);
}

void zzub_pattern_unique_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->unique_events(pp->datapluginparameter->id, fromtime, length);
}

void zzub_pattern_scale_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, double min1, double max1, double min2, double max2) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->scale_events(pp->datapluginparameter->id, fromtime, length, min1, max1, min2, max2);
}

void zzub_pattern_fade_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, double from, double to) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->fade_events(pp->datapluginparameter->id, fromtime, length, from, to);
}

void zzub_pattern_curvemap_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int mode) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->curvemap_events(pp->datapluginparameter->id, fromtime, length, mode);
}

void zzub_pattern_invert_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->invert_events(pp->datapluginparameter->id, fromtime, length);
}

void zzub_pattern_rotate_rows_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int offset) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->rotate_rows_events(pp->datapluginparameter->id, fromtime, length, offset);
}

void zzub_pattern_rotate_vals_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int offset) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->rotate_vals_events(pp->datapluginparameter->id, fromtime, length, offset);
}

void zzub_pattern_rotate_dist_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int offset) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->rotate_dist_events(pp->datapluginparameter->id, fromtime, length, offset);
}

void zzub_pattern_set_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int value, int meta) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->set_events(pp->datapluginparameter->id, fromtime, length, value, meta);
}

void zzub_pattern_replace_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int from_value, int from_meta, int to_value, int to_meta) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->replace_events(pp->datapluginparameter->id, fromtime, length, from_value, from_meta, to_value, to_meta);
}

void zzub_pattern_remove_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int value, int meta) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->remove_events(pp->datapluginparameter->id, fromtime, length, value, meta);
}

void zzub_pattern_notelength_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int fromtime, int length, int maxlen, int mode, int off_value) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->notelength_events(pp->datapluginparameter->id, fromtime, length, maxlen, mode, off_value);
}

void zzub_pattern_volumes_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int note_column, int vol_column, int fromtime, int length, int mode) {
	armstrong::frontend::pluginparameter* ppnote = pattern->owner->get_pluginparameter(pluginid, group, track, note_column);
	assert(ppnote != 0);
	armstrong::frontend::pluginparameter* ppvol = pattern->owner->get_pluginparameter(pluginid, group, track, vol_column);
	assert(ppvol != 0);

	pattern->datapattern->volumes_events(ppnote->datapluginparameter->id, ppvol->datapluginparameter->id, fromtime, length, mode);
}

void zzub_pattern_swap_track_events(zzub_pattern_t* pattern, int left_idx, int right_idx, int fromtime, int length) {
	pattern->datapattern->swap_track_events(left_idx, right_idx, fromtime, length);
}

void zzub_pattern_swap_rows_events(zzub_pattern_t* pattern, int pluginid, int group, int track, int column, int top_row, int bottom_row) {
	armstrong::frontend::pluginparameter* pp = pattern->owner->get_pluginparameter(pluginid, group, track, column);
	assert(pp != 0);
	pattern->datapattern->swap_rows_events(pp->datapluginparameter->id, top_row, bottom_row);
}

void zzub_pattern_invert_chord_events(zzub_pattern_t* pattern, int left_idx, int right_idx, int fromtime, int length, int direction, int mode) {
	pattern->datapattern->invert_chord_events(left_idx, right_idx, fromtime, length, direction, mode);
}

// ---

int zzub_pattern_get_id(zzub_pattern_t* pattern) {
	return pattern->datapattern->id;
}

void zzub_pattern_get_bandwidth_digest(zzub_pattern_t* pattern, float *digest, int digestsize) {
	/*
	float row = 0;
	int rowcount = zzub_pattern_get_row_count(pattern);
	// rows per digest sample
	float rps = (float)rowcount / (float)digestsize;
	for (int i = 0; i < digestsize; ++i) {
		int total = 0;
		int count = 0;
		float rowend = std::min(row + rps, (float)rowcount);
		for (int r = (int)row; r < (int)rowend; r++) {
			size_t trackcount = pattern->getPatternTracks();
			for (size_t t = 0; t < trackcount; ++t) {
				patterntrack* track = pattern->getPatternTrack(t);
				size_t paramcount = track->getParams();
				for (size_t p = 0; p < paramcount; ++p) {
					total += 1;
					int value = track->getValue(r, p);
					const parameter *param = track->getParam(p);
					if (value != param->value_none)
						count += 1;
				}				
			}
		}
		if (total)
			digest[i] = float(count) / float(total);
		else
			digest[i] = 0.0f;
		row = rowend;
	}
	*/
}

zzub_pattern_format_t* zzub_pattern_get_format(zzub_pattern_t* pattern) {
	return pattern->owner->patternformats[pattern->datapattern->patternformat_id].get();
}

void zzub_pattern_set_format(zzub_pattern_t* pattern, zzub_pattern_format_t* format) {
	pattern->datapattern->patternformat_id = format->dataformat->id;
	pattern->datapattern->update();
}

int zzub_pattern_get_resolution(zzub_pattern_t* pattern) {
	return pattern->datapattern->resolution;
}

void zzub_pattern_set_resolution(zzub_pattern_t* pattern, int resolution) {
	pattern->datapattern->resolution = resolution;
	pattern->datapattern->update();
}

int zzub_pattern_get_display_resolution(zzub_pattern_t* pattern) {
	return pattern->datapattern->display_resolution;
}

void zzub_pattern_set_display_resolution(zzub_pattern_t* pattern, int resolution) {
	pattern->datapattern->display_resolution = resolution;
	pattern->datapattern->update();
}

void zzub_pattern_get_display_beat_rows(zzub_pattern_t* pattern, int* verydark, int* dark) {
	assert(verydark);
	assert(dark);
	*verydark = pattern->datapattern->display_verydark_row;
	*dark = pattern->datapattern->display_dark_row;
}

void zzub_pattern_set_display_beat_rows(zzub_pattern_t* pattern, int verydark, int dark) {
	pattern->datapattern->display_verydark_row = verydark;
	pattern->datapattern->display_dark_row = dark;
	pattern->datapattern->update();
}

int zzub_pattern_get_currently_playing_row(zzub_pattern_t* pattern) {
	// ask each pattern player if it knows what row this pattern is playing
	return pattern->owner->mix->get_currently_playing_row(pattern->datapattern->id);
}

int zzub_pattern_get_replay_row(zzub_pattern_t* pattern) {
	return pattern->datapattern->replay_row;
}

void zzub_pattern_set_replay_row(zzub_pattern_t* pattern, int replay_row) {
	pattern->datapattern->replay_row = replay_row;
	pattern->datapattern->update();
}

// class PatternEvent

int zzub_pattern_event_get_id(zzub_pattern_event_t* patternevent) {
	return patternevent->datapatternevent->id;
}

int zzub_pattern_event_get_pluginid(zzub_pattern_event_t* ev) {
	armstrong::frontend::player* player = ev->owner;
	armstrong::frontend::pluginparameter* param = player->pluginparameters[ev->datapatternevent->pluginparameter_id].get();
	assert(param != 0);
	return param->datapluginparameter->plugin_id;
}

int zzub_pattern_event_get_group(zzub_pattern_event_t* ev) {
	armstrong::frontend::player* player = ev->owner;
	armstrong::frontend::pluginparameter* param = player->pluginparameters[ev->datapatternevent->pluginparameter_id].get();
	assert(param != 0);
	armstrong::frontend::parameterinfo* paraminfo = player->parameterinfos[param->datapluginparameter->parameterinfo_id].get();
	assert(paraminfo != 0);
	return paraminfo->dataparameterinfo->paramgroup;
}

int zzub_pattern_event_get_track(zzub_pattern_event_t* ev) {
	armstrong::frontend::player* player = ev->owner;
	armstrong::frontend::pluginparameter* param = player->pluginparameters[ev->datapatternevent->pluginparameter_id].get();
	assert(param != 0);
	return param->datapluginparameter->paramtrack;
}

int zzub_pattern_event_get_column(zzub_pattern_event_t* ev) {
	armstrong::frontend::player* player = ev->owner;
	armstrong::frontend::pluginparameter* param = player->pluginparameters[ev->datapatternevent->pluginparameter_id].get();
	assert(param != 0);
	armstrong::frontend::parameterinfo* paraminfo = player->parameterinfos[param->datapluginparameter->parameterinfo_id].get();
	assert(paraminfo != 0);
	return paraminfo->dataparameterinfo->paramcolumn;
}

int zzub_pattern_event_get_time(zzub_pattern_event_t* patternevent) {
	return patternevent->datapatternevent->time;
}

int zzub_pattern_event_get_value(zzub_pattern_event_t* patternevent) {
	return patternevent->datapatternevent->value;
}

int zzub_pattern_event_get_meta(zzub_pattern_event_t* patternevent) {
	return patternevent->datapatternevent->meta;
}

int zzub_pattern_event_set_value(zzub_pattern_event_t* patternevent, int value) {
	patternevent->datapatternevent->value = value;
	patternevent->datapatternevent->update();
	return -1;
}

int zzub_pattern_event_set_meta(zzub_pattern_event_t* patternevent, int meta) {
	patternevent->datapatternevent->meta = meta;
	patternevent->datapatternevent->update();
	return -1;
}

int zzub_pattern_event_set_time(zzub_pattern_event_t* patternevent, int value) {
	patternevent->datapatternevent->time = value;
	patternevent->datapatternevent->update();
	return 0;
}

zzub_pattern_t* zzub_pattern_event_get_pattern(zzub_pattern_event_t* patternevent) {
	assert(patternevent->datapatternevent->pattern_id < (int)patternevent->owner->patterns.size());

	return patternevent->owner->patterns[patternevent->datapatternevent->pattern_id].get();
}

/* class PatternEventIterator */

zzub_pattern_event_iterator_t* zzub_pattern_get_event_iterator(zzub_pattern_t* pattern, zzub_plugin_t* plugin, int group, int track, int column) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();
	if (plugin == 0)
		pattern->datapattern->get_events(it);
	else
		pattern->datapattern->get_events(plugin->dataplugin->id, group, track, column, it);

	armstrong::frontend::patterneventiterator* frontendit = new armstrong::frontend::patterneventiterator();
	frontendit->owner = pattern->owner;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
}

zzub_pattern_event_iterator_t* zzub_pattern_get_event_unsorted_iterator(zzub_pattern_t* pattern, zzub_plugin_t* plugin, int group, int track, int column) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();
	if (plugin == 0)
		pattern->datapattern->get_events_unsorted(it);
	else
		pattern->datapattern->get_events_unsorted(plugin->dataplugin->id, group, track, column, it);

	armstrong::frontend::patterneventiterator* frontendit = new armstrong::frontend::patterneventiterator();
	frontendit->owner = pattern->owner;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
}

void zzub_pattern_event_iterator_next(zzub_pattern_event_iterator_t* patterneventiterator) {
	patterneventiterator->recordset->next();
}

int zzub_pattern_event_iterator_valid(zzub_pattern_event_iterator_t* patterneventiterator) {
	return patterneventiterator->recordset->eof() ? 0 : 1;
}

zzub_pattern_event_t* zzub_pattern_event_iterator_current(zzub_pattern_event_iterator_t* patterneventiterator) {
	static armstrong::storage::patterneventdata pev;
	return patterneventiterator->owner->patternevents[patterneventiterator->recordset->id()].get();
	//return &pev;
}

void zzub_pattern_event_iterator_destroy(zzub_pattern_event_iterator_t* patterneventiterator) {
	patterneventiterator->recordset->destroy();
	//delete patterneventiterator->recordset;
	delete patterneventiterator;
}

/* class PatternIterator */

void zzub_pattern_iterator_next(zzub_pattern_iterator_t* patterniterator) {
	patterniterator->recordset->next();
}

int zzub_pattern_iterator_valid(zzub_pattern_iterator_t* patterniterator) {
	return patterniterator->recordset->eof() ? 0 : 1;
}

zzub_pattern_t* zzub_pattern_iterator_current(zzub_pattern_iterator_t* patterniterator) {
	int id = patterniterator->recordset->id();
	if (id == 0) return 0; // id 0 = NULL
	return patterniterator->owner->patterns[id].get();
}

void zzub_pattern_iterator_destroy(zzub_pattern_iterator_t* patterniterator) {
	patterniterator->recordset->destroy();
	//delete patterniterator->recordset;
	delete patterniterator;
}

/* class PatternFormat */

// void zzub_pattern_format_add_column(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, int column) {
// 	armstrong::storage::patternformatcolumndata fmtcoldata;
// 	patternformat->owner->songdata->create_pattern_format_column(patternformat->dataformat->id, plugin->dataplugin->id, group, track, column, fmtcoldata);
// }

zzub_pattern_format_column_t* zzub_pattern_format_add_column(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, int column, int idx) {
	armstrong::frontend::player* player = patternformat->owner;

	armstrong::frontend::pluginparameter* param = player->get_pluginparameter(plugin->dataplugin->id, group, track, column);
	assert(param != 0);

	armstrong::storage::patternformatcolumndata fmtcoldata;
	patternformat->owner->songdata->create_pattern_format_column(patternformat->dataformat->id, param->datapluginparameter->id, idx, fmtcoldata);

	return patternformat->owner->patternformatcolumns[fmtcoldata.id].get();
}

void zzub_pattern_format_delete_column(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, int column) {
	armstrong::storage::patternformatcolumn fmtcoldata(patternformat->owner->doc.get());
	patternformat->dataformat->get_column(plugin->dataplugin->id, group, track, column, fmtcoldata);
	fmtcoldata.destroy();
}

zzub_pattern_format_column_iterator_t* zzub_pattern_format_get_iterator(zzub_pattern_format_t* patternformat) {
	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();
	patternformat->dataformat->get_columns(it);

	armstrong::frontend::patternformatcolumniterator* frontendit = new armstrong::frontend::patternformatcolumniterator();
	frontendit->owner = patternformat->owner;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;
}

const char* zzub_pattern_format_get_name(zzub_pattern_format_t* format) {
	static char name[256];
	strncpy(name, format->dataformat->name.c_str(), 256);
	return name;
}

void zzub_pattern_format_set_name(zzub_pattern_format_t* format, const char* name) {
	format->dataformat->name = name;
	format->dataformat->update();
}

int zzub_pattern_format_get_scroller_width(zzub_pattern_format_t* format) {
	return format->dataformat->scroller_width;
}

void zzub_pattern_format_set_scroller_width(zzub_pattern_format_t* format, int width) {
	format->dataformat->scroller_width = width;
	format->dataformat->update();
}

void zzub_pattern_format_destroy(zzub_pattern_format_t* format) {
	format->dataformat->destroy();
}

int zzub_pattern_format_get_id(zzub_pattern_format_t* format) {
	return format->dataformat->id;
}

void zzub_pattern_format_iterator_next(zzub_pattern_format_iterator_t* patternformatiterator) {
	patternformatiterator->recordset->next();
}

int zzub_pattern_format_iterator_valid(zzub_pattern_format_iterator_t* patternformatiterator) {
	return patternformatiterator->recordset->eof() ? 0 : 1;
}

zzub_pattern_format_t* zzub_pattern_format_iterator_current(zzub_pattern_format_iterator_t* patternformatiterator) {
	static armstrong::storage::patternformatdata pev;
	return patternformatiterator->owner->patternformats[patternformatiterator->recordset->id()].get();
}

void zzub_pattern_format_iterator_destroy(zzub_pattern_format_iterator_t* patternformatiterator) {
	patternformatiterator->recordset->destroy();
	//delete patternformatiterator->recordset;
	delete patternformatiterator;
}

void zzub_pattern_format_column_iterator_next(zzub_pattern_format_column_iterator_t* patternformatcolumniterator) {
	patternformatcolumniterator->recordset->next();
}

int zzub_pattern_format_column_iterator_valid(zzub_pattern_format_column_iterator_t* patternformatcolumniterator) {
	return patternformatcolumniterator->recordset->eof() ? 0 : 1;
}

zzub_pattern_format_column_t* zzub_pattern_format_column_iterator_current(zzub_pattern_format_column_iterator_t* patternformatcolumniterator) {
	static armstrong::storage::patternformatcolumndata pev;
	return patternformatcolumniterator->owner->patternformatcolumns[patternformatcolumniterator->recordset->id()].get();
}

void zzub_pattern_format_column_iterator_destroy(zzub_pattern_format_column_iterator_t* patternformatcolumniterator) {
	patternformatcolumniterator->recordset->destroy();
	//delete patternformatcolumniterator->recordset;
	delete patternformatcolumniterator;
}

zzub_plugin_t* zzub_pattern_format_column_get_plugin(zzub_pattern_format_column_t* patternformatcolumn) {
	armstrong::frontend::player* player = patternformatcolumn->owner;
	armstrong::frontend::pluginparameter* param =  player->pluginparameters[patternformatcolumn->datacolumn->pluginparameter_id].get();
	assert(param != 0);

	return player->plugins[param->datapluginparameter->plugin_id].get();
}

int zzub_pattern_format_column_get_group(zzub_pattern_format_column_t* patternformatcolumn) {
	armstrong::frontend::player* player = patternformatcolumn->owner;
	armstrong::frontend::pluginparameter* param =  player->pluginparameters[patternformatcolumn->datacolumn->pluginparameter_id].get();
	assert(param != 0);
	armstrong::frontend::parameterinfo* paraminfo = player->parameterinfos[param->datapluginparameter->parameterinfo_id].get();
	assert(paraminfo);
	return paraminfo->dataparameterinfo->paramgroup;
}

int zzub_pattern_format_column_get_track(zzub_pattern_format_column_t* patternformatcolumn) {
	armstrong::frontend::player* player = patternformatcolumn->owner;
	armstrong::frontend::pluginparameter* param =  player->pluginparameters[patternformatcolumn->datacolumn->pluginparameter_id].get();
	assert(param != 0);
	return param->datapluginparameter->paramtrack;
}

int zzub_pattern_format_column_get_column(zzub_pattern_format_column_t* patternformatcolumn) {
	armstrong::frontend::player* player = patternformatcolumn->owner;
	armstrong::frontend::pluginparameter* param =  player->pluginparameters[patternformatcolumn->datacolumn->pluginparameter_id].get();
	assert(param != 0);
	armstrong::frontend::parameterinfo* paraminfo = player->parameterinfos[param->datapluginparameter->parameterinfo_id].get();
	assert(paraminfo != 0);
	return paraminfo->dataparameterinfo->paramcolumn;
}

zzub_pattern_format_t* zzub_pattern_format_column_get_format(zzub_pattern_format_column_t* patternformatcolumn) {
	return patternformatcolumn->owner->patternformats[patternformatcolumn->datacolumn->patternformat_id].get();
}

int zzub_pattern_format_column_get_mode(zzub_pattern_format_column_t* patternformatcolumn) {
	return patternformatcolumn->datacolumn->mode;
}

void zzub_pattern_format_column_set_mode(zzub_pattern_format_column_t* patternformatcolumn, int mode) {
	patternformatcolumn->datacolumn->mode = mode;
	patternformatcolumn->datacolumn->update();
}

int zzub_pattern_format_column_get_collapsed(zzub_pattern_format_column_t* patternformatcolumn) {
	return patternformatcolumn->datacolumn->is_collapsed;
}

void zzub_pattern_format_column_set_collapsed(zzub_pattern_format_column_t* patternformatcolumn, int is_collapsed) {
	patternformatcolumn->datacolumn->is_collapsed = is_collapsed;
	patternformatcolumn->datacolumn->update();
}

int zzub_pattern_format_column_get_index(zzub_pattern_format_column_t* patternformatcolumn) {
	return patternformatcolumn->datacolumn->idx;
}

void zzub_pattern_format_column_set_index(zzub_pattern_format_column_t* patternformatcolumn, int idx) {
	patternformatcolumn->datacolumn->idx = idx;
	patternformatcolumn->datacolumn->update();
}

void zzub_pattern_format_set_track_name(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, const char *name) {
	armstrong::storage::patternformattrack trackdata(patternformat->owner->doc.get());
	if (patternformat->dataformat->get_track(plugin->dataplugin->id, group, track, trackdata)) {
		trackdata.label = name;
		trackdata.update();
	} else {
		patternformat->dataformat->create_track(plugin->dataplugin->id, group, track, name, 0, trackdata);
	}
}

const char* zzub_pattern_format_get_track_name(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track) {
	armstrong::storage::patternformattrack trackdata(patternformat->owner->doc.get());
	if (!patternformat->dataformat->get_track(plugin->dataplugin->id, group, track, trackdata))
		trackdata.label = "";
	static char name[256];
	strncpy(name, trackdata.label.c_str(), 256);
	return name;
}

void zzub_pattern_format_set_track_mute(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, int mute) {
	armstrong::storage::patternformattrack trackdata(patternformat->owner->doc.get());
	if (patternformat->dataformat->get_track(plugin->dataplugin->id, group, track, trackdata)) {
		trackdata.is_muted = mute;
		trackdata.update();
	} else {
		patternformat->dataformat->create_track(plugin->dataplugin->id, group, track, "", mute, trackdata);
	}
}

int zzub_pattern_format_get_track_mute(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track) {
	armstrong::storage::patternformattrack trackdata(patternformat->owner->doc.get());
	if (!patternformat->dataformat->get_track(plugin->dataplugin->id, group, track, trackdata))
		return 0;
	return trackdata.is_muted;
}

void zzub_pattern_format_add_column_filter(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, int column, zzub_pattern_format_t* filterformat) {
	assert(false);
	/*armstrong::storage::patternformatcolumn col(patternformat->owner->doc.get());
	patternformat->dataformat->get_column(plugin->dataplugin->id, group, track, column, col);
	col.add_filter(filterformat->dataformat->id);*/
}

void zzub_pattern_format_remove_column_filter(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, int column, zzub_pattern_format_t* filterformat) {
	assert(false);
	/*armstrong::storage::patternformatcolumn col(patternformat->owner->doc.get());
	patternformat->dataformat->get_column(plugin->dataplugin->id, group, track, column, col);
	col.remove_filter(filterformat->dataformat->id);*/
}

zzub_pattern_format_iterator_t* zzub_pattern_format_get_column_filters(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, int column) {
	assert(false);
	return 0;
/*	armstrong::storage::tableiterator* it = new armstrong::storage::tableiterator();

	armstrong::storage::patternformatcolumn col(patternformat->owner->doc.get());
	patternformat->dataformat->get_column(plugin->dataplugin->id, group, track, column, col);
	col.get_filters(it);

	armstrong::frontend::patternformatiterator* frontendit = new armstrong::frontend::patternformatiterator();
	frontendit->owner = patternformat->owner;
	frontendit->recordset = boost::shared_ptr<armstrong::storage::tableiterator>(it);
	return frontendit;*/
}

zzub_pattern_format_column_t* zzub_pattern_format_get_column(zzub_pattern_format_t* patternformat, zzub_plugin_t* plugin, int group, int track, int column) {
	armstrong::storage::patternformatcolumndata fmtcol;
	if (patternformat->dataformat->get_column(plugin->dataplugin->id, group, track, column, fmtcol))
		return patternformat->owner->patternformatcolumns[fmtcol.id].get();
	else
		return 0;
}

int zzub_pattern_get_loop_start(zzub_pattern_t* pattern) {
	return pattern->datapattern->beginloop;
}

void zzub_pattern_set_loop_start(zzub_pattern_t* pattern, int pos) {
	pattern->datapattern->beginloop = pos;
	pattern->datapattern->update();
}

int zzub_pattern_get_loop_end(zzub_pattern_t* pattern) {
	return pattern->datapattern->endloop;
}

void zzub_pattern_set_loop_end(zzub_pattern_t* pattern, int pos) {
	pattern->datapattern->endloop = pos;
	pattern->datapattern->update();
}

int zzub_pattern_get_loop_enabled(zzub_pattern_t* pattern) {
	return pattern->datapattern->loopenabled;
}

void zzub_pattern_set_loop_enabled(zzub_pattern_t* pattern, int enable) {
	pattern->datapattern->loopenabled = enable;
	pattern->datapattern->update();
}


/** \brief Edits notes independently of voice.
	mode 0 = move entire notes, mode 1 = move beginning of notes, mode 2 = move end of notes */
void zzub_pattern_move_and_transpose_notes(zzub_pattern_t *pattern, const zzub_pattern_event_t * *events, int numevents, int timeshift, int pitchshift, int mode) {
	
	// events can contain note parameter events, ignoring everything else. 
	// they can belong to different plugins, but pattern data for each plugin is processed separately.

	// 1. find note events and group by plugins, for each plugin:
	// 1.1 find max/min event range of selected notes
	// 1.2 find other notes which last through, or begin/end during the min/max range. these will remain unchanged during the conversion process, but we need to know where they are
	// 1.3 convert selected + other notes into notes+lengths
	// 1.4 move and transpose the selected notes according to parameters
	// 1.5 converted selected notes back to pattern data, where other notes are fixed in their originating tracks
	// 1.6 create pattern event inserts/updates/deletes for any differences
	assert(numevents > 0);

	std::vector<int> eventids(numevents);
	for (int i = 0; i < numevents; i++) {
		eventids[i] = events[i]->datapatternevent->id;
	}

	pattern->datapattern->move_and_transpose_notes(&eventids.front(), numevents, timeshift, pitchshift, mode);
}

/** \brief Inserts a voice-independent note. */
void zzub_pattern_insert_note(zzub_pattern_t *pattern, zzub_plugin_t *plugin, int time, int note, int length) {
	if (note == zzub_note_value_off || note == zzub_note_value_cut) return ;
	pattern->datapattern->insert_note(plugin->dataplugin->id, time, note, length);
}

/** \brief Updates a voice-independent note. */
void zzub_pattern_update_note(zzub_pattern_t *pattern, zzub_pattern_event_t *event, int time, int note, int length) {
	if (note == zzub_note_value_off || note == zzub_note_value_cut) return ;
	pattern->datapattern->update_note(event->datapatternevent->id, time, note, length);
}

}
