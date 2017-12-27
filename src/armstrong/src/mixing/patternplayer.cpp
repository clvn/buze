#include <boost/shared_ptr.hpp>
#include <cmath>
#include <limits>
#include <vector>
#include <map>
#include <iostream>
#include "mixer.h"
#include "patternplayer.h"

#include <iostream>

using std::cout;
using std::endl;

namespace zzub {

//
// patternplayer
//

patternplayer::patternplayer() {
	_plugin = 0;
	_mixer = 0;
	pattern_id = -1;
	subtick_frac_remainder = 0.0f;
	next_pattern_samples = std::numeric_limits<int>::max();
	seek_row = -1;
	stop_row = -1;
	playing = false;
	parent = 0;
	is_preview_pattern = false;
	reset_interpolator_position = 0;
	reset_position();
	reset_interpolators();

	_master_info.beats_per_minute = 0;
	_master_info.ticks_per_beat = 0;
	_master_info.tick_position = 0;
	_master_info.tick_position_frac = 0.0f;
	_master_info.row_position = 0;
	_master_info.swing_amount = 0.5f;
	_master_info.swing_ticks = 4;
}

void patternplayer::reset_next_position() {
	next_pattern_samples = 0;
	next_pattern_row = 0;
}

void patternplayer::reset_position(int row) {
	reset_next_position();
	sample_position = 0;
	subtick_position = 0;
	pattern_row = row;
	subtick_counter = 0;
	tick_position = 0;
	played_rows = 0;
}

pattern* patternplayer::get_pattern() {
	if (pattern_id == -1) return 0;
	if (pattern_id <= 0 || pattern_id >= (int)_mixer->patterns.top().size()) return 0;
	return _mixer->patterns.top()[pattern_id].get();
}

double patternplayer::get_pattern_resolution() {
	pattern* p = get_pattern();
	if (p == 0) return 1.0f;
	return (double)p->resolution;
}

void patternplayer::stop() {
	playing = false;
}

void patternplayer::play(int stoprow) {
	playing = true;
	stop_row = stoprow;
}

namespace {	// duplicate from ccm.h and pattern.cpp

int midi_to_buzz_note(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}

int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}
}

inline bool equal_event(pattern_events_by_time::iterator& eventit, int pluginid, int group, int track, int column) {
	return eventit->pluginid == pluginid && eventit->group == group && eventit->track == track && eventit->column == column;
}

// find_previous_event() iterates eventid backwards until it finds the 
// previous pattern event for the requested parameter. 
bool find_previous_event(int pluginid, int group, int track, int column, pattern_events* events, pattern_events_by_time::iterator& eventit) {
	while (eventit != events->by_time.begin()) {
		eventit--;
		if (equal_event(eventit, pluginid, group, track, column)) {
			return true;
		}
	}
	eventit = events->by_time.end();
	return false;
}

// find_next_event() iterates eventid forwards until it finds the 
// next pattern event for the requested parameter. 
bool find_next_event(int pluginid, int group, int track, int column, pattern_events* events, pattern_events_by_time::iterator& eventit) {
	while (eventit != events->by_time.end()) {
		eventit++;
		if (eventit != events->by_time.end() && equal_event(eventit, pluginid, group, track, column)) {
			return true;
		}
	}
	return false;
}

// reset_interpolator() -> sets plugin interpolators from current pattern to the current pattern position
//   -> if an interpolator is already owned, it wont reset
// update_interpolator() -> updates plugin interpolators with (lookahead) values at the current pattern position

void patternplayer::reset_interpolators() {
	pattern* p = get_pattern();
	if (p == 0) return ;

	has_interpolators = false;

	patternformat* format = _mixer->patternformats.top()[p->formatid].get();
	for (std::vector<patternformatcolumn>::iterator i = format->columns.begin(); i != format->columns.end(); ++i) {
		zzub::metaplugin* mpl = _mixer->plugins.top()[i->pluginid].get();
		interpolator* interpol = &mpl->state_write.interpolators[i->group][i->track][i->column];
		interpol->reset(interpol->mode);
		if (interpol->needs_before + interpol->needs_after > 0) has_interpolators = true;
	}
	reset_interpolator_position = -1;
}

int patternplayer::reset_interpolator(metaplugin* mpl, int formatid, int group, int track, int column, pattern_events_by_time::iterator eventit) {

	mutetrackkeytype mtkt = { formatid, mpl->id, group, track };
	std::map<mutetrackkeytype, int>::iterator muteit = _mixer->mutestate.find(mtkt);
	if (muteit != _mixer->mutestate.end()) return -1;

	// fill interpol with needs_before, current and needs_after by iterating the eventit
	interpolator* interpol = &mpl->state_write.interpolators[group][track][column];

	if (interpol->owner != this && interpol->owner != 0) return -1;

	pattern_events* events = _mixer->patternevents.top()[pattern_id].get();

	interpol->time = pattern_row;
	interpol->tick_position = subtick_position;
	interpol->value_position = -1;
	interpol->owner = this;

	// eventit points at the first event after the current pattern position, 
	// or the last event in the pattern if thats on the current position:
	// scan backwards to find the current event for this column
	pattern_events_by_time::iterator currentit = eventit;
	if (currentit != events->by_time.end() && (currentit->time > pattern_row || !(equal_event(currentit, mpl->id, group, track, column) && currentit->time == pattern_row ) ) ) {
		find_previous_event(mpl->id, group, track, column, events, currentit);
	}

	pattern_event lastevent;
	if (currentit == events->by_time.end()) {
		lastevent.time = pattern_row - 1; // set to -1 so it doesnt trigger now
		lastevent.value = mpl->get_parameter(group, track, column);
	} else {
		lastevent = *currentit;
	}

	pattern_events_by_time::iterator beforeit = currentit;
	for (int i = 0; i < interpol->needs_before; i++) {
		if (beforeit != events->by_time.end() && find_previous_event(mpl->id, group, track, column, events, beforeit)) {
			interpol->set_value(-1 - i, beforeit->time, beforeit->value);
			lastevent = *beforeit;
		} else {
			interpol->set_value(-1 - i, lastevent.time, lastevent.value);
		}
	}

	if (currentit == events->by_time.end()) {
		lastevent.time = pattern_row - 1; // subtract -1 so it doesnt trigger imnmediately
		lastevent.value = mpl->get_parameter(group, track, column);
		interpol->set_value(0, lastevent.time, lastevent.value);
	} else {
		lastevent = *currentit;
		interpol->set_value(0, currentit->time, currentit->value);
	}

	pattern_events_by_time::iterator nextit = currentit;
	for (int i = 0; i < interpol->needs_after; i++) {
		if (nextit != events->by_time.end() && find_next_event(mpl->id, group, track, column, events, nextit)) {
			interpol->set_value(1 + i, nextit->time, nextit->value);
			lastevent = *nextit;
		} else {
			interpol->set_value(1 + i, lastevent.time, lastevent.value);
		}
	}

	if (interpol->needs_after + interpol->needs_before == 0) 
		return -1; // no lookahead/readbehind -> use default
	return 1;
}

void patternplayer::update_interpolator(metaplugin* mpl, int formatid, interpolator* interpol, pattern_events_by_time::iterator eventit) {

	// skip first update after reset, update is incremental - use the value_position instead?
	if (reset_interpolator_position == _mixer->work_position)
		return ;

	// check for interpolation ownership change, when two patterns refer the same column and can switch ownership
	if (interpol->owner != this) {
		interpol->owner = 0;
		reset_interpolator(mpl, formatid, eventit->group, eventit->track, eventit->column, eventit);
		return ;
	}

	// update interpol with the last value it needs, ie current or the last of needs_after
	pattern_event lastevent = *eventit;
	pattern_events* events = _mixer->patternevents.top()[pattern_id].get();

	assert(eventit != events->by_time.end());

	if (interpol->needs_after > 0) {
		pattern_events_by_time::iterator nextit = eventit;
		for (int i = 0; i < interpol->needs_after; i++) {
			find_next_event(nextit->pluginid, nextit->group, nextit->track, nextit->column, events, nextit);
			if (nextit != events->by_time.end()) {
				lastevent = *nextit;
			} else
				break;
		}
	}

	interpol->shift_set_value(this, lastevent.time, lastevent.value);
}

void patternplayer::set_parameter(int formatid, int pluginid, int group, int track, int column, int value, bool offline, const pattern_events_by_time::iterator& eventit) {	
	// lookup patternformat+pluginid+group+track to find if current track is muted
	mutetrackkeytype mtkt = { formatid, pluginid, group, track };
	std::map<mutetrackkeytype, int>::iterator muteit = _mixer->mutestate.find(mtkt);
	if (muteit != _mixer->mutestate.end()) return ;

	metaplugin* mp = _mixer->plugins.top()[pluginid].get();

	const parameter* param = mp->get_parameter_info(group, track, column);
	if (transpose != 0 && param->type == zzub_parameter_type_note && value != zzub_note_value_none && value != zzub_note_value_cut && value != zzub_note_value_off) {
		int midinote = buzz_to_midi_note(value) + transpose;
		midinote = std::min(std::max(midinote, 1), 1 + 8 * 12); // 8 octaves?
		value = midi_to_buzz_note(midinote);
	}

	if (mp->info->flags & zzub_plugin_flag_stream) {
		mp->set_parameter(group, track, column, value, false);
	} else
	if (mp->info->flags & zzub_plugin_flag_is_sequence) {
		mp->set_parameter(group, track, column, value, false);

		// make a relation between the triggered pattern player and ourself
		// dont play ourself, prevent recursive relations etc
		patternplayer* childplayer = mp->get_patternplayer(group, track);
		if (childplayer != 0 && childplayer != this && !is_parent(childplayer)) {
			childplayer->parent = this;
			childplayer->stop();
		}
	} else 
	if (!offline) {
		// cant interpolate notes, but can transpose:
		if (param->type == zzub_parameter_type_note) {
			mp->set_parameter(group, track, column, value, false);
		} else {
			interpolator* interpol = &mp->state_write.interpolators[group][track][column];
			update_interpolator(mp, formatid, interpol, eventit);
		}
	}
	return ;
}

void patternplayer::clear_children(bool is_root) {
	reset_position();
	if (!is_root) {
		parent = 0;
		stop();
	}
}

bool patternplayer::is_parent(patternplayer* pp) {
	if (parent == 0) return false;
	if (parent == pp) return true;
	return parent->is_parent(pp);
}

double patternplayer::get_current_samples_per_row(int* next_swing_rows) {

	int beat_rows;
	double samples_per_row_target;
	double samples_per_row_before_swing;
	double samples_per_row_after_swing;
	get_swing(&samples_per_row_target, &samples_per_row_before_swing, &samples_per_row_after_swing, &beat_rows);

	if (!playing) {
		*next_swing_rows = std::numeric_limits<int>::max();
		return samples_per_row_target;
	}

	int beat_offset = played_rows % beat_rows;
	int swingrows = beat_rows / 2;

	if (beat_offset < swingrows) {
		*next_swing_rows = swingrows - beat_offset;
		return samples_per_row_before_swing;
	} else if (beat_offset >= swingrows && beat_offset < swingrows + swingrows) {
		*next_swing_rows = swingrows + swingrows - beat_offset;
		return samples_per_row_after_swing;
	} else {
		*next_swing_rows = 1;
		return samples_per_row_target;
	}
}

// rows should be max "tpb"
double patternplayer::get_samplecount_for_rows(int rows, pattern* playpattern) {
	int beat_rows;
	double samples_per_row_target;
	double samples_per_row_before_swing;
	double samples_per_row_after_swing;
	get_swing(&samples_per_row_target, &samples_per_row_before_swing, &samples_per_row_after_swing, &beat_rows);

	if (!playing) {		
		return samples_per_row_target * rows;
	}

	int beat_offset = played_rows % beat_rows;

	int swingrows = beat_rows / 2;
	if (beat_offset < swingrows) {
		// currently before_swing
		int rows_before_swing = std::min(swingrows - beat_offset, rows);
		int rows_after_swing = std::min(rows - rows_before_swing, swingrows);
		int odd_rows = rows - rows_before_swing - rows_after_swing;
		assert((odd_rows == 0 || odd_rows == 1) && "before_swing");
		return 
			rows_before_swing * samples_per_row_before_swing + 
			rows_after_swing * samples_per_row_after_swing + 
			odd_rows * samples_per_row_target + 
			subtick_frac_remainder;
	} else if (beat_offset >= swingrows && beat_offset < swingrows + swingrows) {
		// currently after_swing
		int rows_after_swing = std::min(rows, swingrows);
		int odd_rows = rows - rows_after_swing;
		assert((odd_rows == 0 || odd_rows == 1) && "after_swing");
		return 
			rows_after_swing * samples_per_row_after_swing + 
			odd_rows * samples_per_row_target + 
			subtick_frac_remainder;
	} else {
		// currently at the last row in a beat with an odd number of rows (1, 3, 5 etc tpb or resolution)
		assert(rows == 1);
		return samples_per_row_target + subtick_frac_remainder;
	}

}

void patternplayer::update(bool offline) {
	pattern* playpattern = get_pattern();
	if (playpattern == 0) {
		// not patterns playing - suggest skipping as much as possible
		next_pattern_samples = std::numeric_limits<int>::max();
		return;
	}

	int next_swing_rows;
	double samples_per_pat_line = get_current_samples_per_row(&next_swing_rows);
	if ((double)subtick_position >= samples_per_pat_line + subtick_frac_remainder - 1) {
		subtick_position = 0;
		next_pattern_row = 0;
	}
	if (next_pattern_row == 0) {
		if (subtick_position != 0) {
			// this could happen right after an external seek or tempochange
			// finish the remaining samples of this subtick
			next_pattern_row = 1;
			next_pattern_samples = (int)((samples_per_pat_line - (double)subtick_position) + subtick_frac_remainder);
		} else {
			// reached user defined stop?
			if (stop_row != -1 && pattern_row >= stop_row) {
				stop();
				return;
			} else
			// reached end of pattern? (and not looping, or there is a queued order)
			if (pattern_row >= playpattern->rows && (!playpattern->loopenabled || _mixer->queue_index != -1)) {
				_mixer->end_of_pattern(this, offline); // let mixer decide what to do, usually set a new pattern
				return ;
			} else
			// reached end of loop?
			if (pattern_row >= playpattern->endloop && playpattern->endloop > playpattern->beginloop && playpattern->loopenabled) {
				// end of loop = resume from beginloop as a continuation of the current pattern, and pretend nothing happened
				// NOTE: this used to do a full seek but didnt quite work. this is much simpler and works:
				pattern_row = playpattern->beginloop;
				reset_interpolators();
			}

			assert(subtick_position == 0);
			next_pattern_row = advance_pattern(playpattern, pattern_row, offline);

			if (stop_row != -1 && pattern_row + next_pattern_row > stop_row) {
				// cut off at target position
				next_pattern_row = stop_row - pattern_row;
			} else
			if (seek_row != -1 && pattern_row + next_pattern_row > seek_row) {
				// cut off at target position
				next_pattern_row = seek_row - pattern_row;
			} else
			if (playpattern->endloop > 0 && pattern_row + next_pattern_row > playpattern->endloop && playpattern->loopenabled) {
				// cut off at end of loop
				next_pattern_row = playpattern->endloop - pattern_row;
			} else
			if (pattern_row + next_pattern_row > playpattern->rows) {
				// cut off at end of pattern
				next_pattern_row = playpattern->rows - pattern_row;
			}
			// cut off at the next half beat / row where swing tempo change occurs
			if (next_pattern_row > next_swing_rows) {
				next_pattern_row = next_swing_rows;
			}

			last_next_pattern_samples = get_samplecount_for_rows(next_pattern_row, playpattern);
			next_pattern_samples = (int)last_next_pattern_samples;
		}
	} else {
		// recalc next_pattern_samples with tempo changes
		next_pattern_samples = (int)get_samplecount_for_rows(next_pattern_row, playpattern);
	}
	assert(next_pattern_samples > 0);
}

void patternplayer::advance(int numsamples) {

	double resolution = get_pattern_resolution();
	pattern* p = get_pattern();

	if (!playing || p == 0) 
		return ;

	assert(numsamples <= next_pattern_samples);

	next_pattern_samples -= numsamples;
	sample_position += numsamples;
	subtick_position += numsamples;
	tick_position += numsamples;

	double n;

	// this loops usually runs once, some rare cases demand a second run due to precision issues
	for (; ;) {
		int next_swing_rows;
		double samples_per_pat_line = get_current_samples_per_row(&next_swing_rows);

		int rows = (int)((subtick_position + (1 - subtick_frac_remainder)) / (samples_per_pat_line));
		rows = std::min(rows, next_swing_rows);
		if (rows <= 0) 
			break;

		double advance_samples = samples_per_pat_line * (double)rows + subtick_frac_remainder;

		// super rare edge case which happens e.g when playing from start at 120 bpm, 8 tpb.
		// then trigger a pattern at 960 and play something in its row 4. row 4, and only row 4 is silent.
		// this detects the case and lets the engine process one more sample:
		if (next_pattern_samples > 0 && subtick_position - (int)advance_samples < 0)
			break;

 		subtick_position -= (int)advance_samples;

		// check extreme edge cases to keep precision errors under control
		if (next_pattern_samples == 0 && subtick_position != 0) {
			// check for precision errors which may occur depending on samplerate, bpm and tpb.
			if (subtick_position == -1) {
				// handle case where samples_per_pat_line * totalrows is nnn.99999998XXX, where rounding produces 1 extra sample. compare to last_next_pattern_samples
				subtick_position = 0;
				advance_samples = 0.0f; // reset fractions
			} else
			if (subtick_position == 1) {
				// handle case where the counted frac_remainder becomes 0.99999998X, where rounding removes a sample
				subtick_position = 0;
				advance_samples = 0.0f; // reset fractions
			}
			
			assert(subtick_position >= 0); // when subtick_position is <> 0 here, it should be equal to samples_per_pat_line+fracs and cause the loop to run again. compare to last_next_pattern_samples.
		}

		assert(subtick_position >= 0);

		subtick_frac_remainder = modf(advance_samples, &n);

		assert(rows <= next_pattern_row);
		next_pattern_row -= rows;
		pattern_row += rows;
		played_rows += rows;

		// make sure we know where we are in the regular tick
		subtick_counter += rows;
		subtick_counter %= (int)resolution;
		tick_position = samples_per_pat_line * (double)subtick_counter + (double)subtick_position + subtick_frac_remainder;

		assert(!playing || (next_pattern_samples == 0 && (subtick_position == 0 || subtick_position > (int)advance_samples)) || (next_pattern_samples != 0));
		assert(subtick_position >= 0);
	}

	// update interpolator times
	patternformat* format = _mixer->patternformats.top()[p->formatid].get();
	for (std::vector<patternformatcolumn>::iterator i = format->columns.begin(); i != format->columns.end(); ++i) {
		zzub::metaplugin* mpl = _mixer->plugins.top()[i->pluginid].get();
		interpolator* interpol = &mpl->state_write.interpolators[i->group][i->track][i->column];
		if (interpol->owner == this) {
			interpol->time = pattern_row;
			interpol->tick_position = subtick_position;
			assert(subtick_position >= 0);
		}
	}
}

//called from patternplayer::update
int patternplayer::advance_pattern(pattern const* pat, int time, bool offline) {
	pattern_events* events = _mixer->patternevents.top()[pattern_id].get();

	if (!pat || events->by_time.empty()) {
		if (offline)
			return pat->rows;// * pat->resolution;
		else
			return 1;//pat->resolution;
	}

	pattern_events_by_time::iterator i = events->by_time.lower_bound(time, less_by_time());

	// set all parameter changes at this time
	for (; i != events->by_time.end(); ++i) {
		if (i->time != time) break;

		set_parameter(pat->formatid, i->pluginid, i->group, i->track, i->column, i->value, offline, i);
	}

	// scan for the next trigger event when seeking, otherwise use the next event
	if (offline) {
		for (; i != events->by_time.end(); ++i) {
			metaplugin* mp = _mixer->plugins.top()[i->pluginid].get();
			if (mp->info->flags & zzub_plugin_flag_is_sequence) break;
			if (mp->info->flags & zzub_plugin_flag_stream) break;
			if (mp->info->flags & zzub_plugin_flag_has_event_output) break;
		}

		if (i != events->by_time.end())
			return i->time - time;

		return pat->rows;// * pat->resolution; // allow skipping up to end of pattern
	}

	// return number of subticks until next event.
	if (!has_interpolators && i != events->by_time.end())
		return i->time - time;

	// skip a full tick
	return 1;//pat->resolution;
}

void patternplayer::apply_sequence_size(int& min_sequence_chunk) {
	if (playing) {
		assert(next_pattern_samples > 0);
		if (next_pattern_samples > 0 && next_pattern_samples < min_sequence_chunk) {
			min_sequence_chunk = next_pattern_samples;
			assert(min_sequence_chunk > 0);
		}
	}
}

void patternplayer::process_sequence(bool offline) {
	if (playing) {
		double n;
		_master_info.tick_position = (int)tick_position;
		_master_info.tick_position_frac = (float)modf(tick_position, &n);
		_master_info.row_position = pattern_row;

		if (next_pattern_samples == 0) {
			update(offline);
		}
	}
}

void patternplayer::set_speed(int bpm, int tpb, float swing, int swing_ticks) {
	double n;
	double sps = (double)_master_info.samples_per_second;
	double tpm = (double)bpm * (double)tpb;
	double spt = (60.0f * sps) / tpm;
	
	if (bpm == _master_info.beats_per_minute && tpb == _master_info.ticks_per_beat && swing == _master_info.swing_amount && swing_ticks == _master_info.swing_ticks)
		return ;

	_master_info.samples_per_second = (int)sps;
	_master_info.beats_per_minute = bpm;
	_master_info.ticks_per_beat = tpb;
	_master_info.samples_per_tick = (int)spt;
	_master_info.samples_per_tick_frac = (float)modf(spt, &n);
	_master_info.ticks_per_second = (float)(sps / spt);
	_master_info.swing_amount = swing;
	_master_info.swing_ticks = swing_ticks;
	
	reset_next_position();

	_mixer->on_tempo_changed(this, _plugin ? _plugin->_id : 0, plugin_group, plugin_track);
}

// swingamont must be between 0..1 where 0.5 = no swing
void patternplayer::get_swing(double* samples_per_row_target, double* samples_per_row_before_swing, double* samples_per_row_after_swing, int* outbeat_rows) {
	int pattern_resolution = (int)get_pattern_resolution();
	int beat_rows = _master_info.ticks_per_beat * pattern_resolution;
	int swing_beat_rows = beat_rows - (beat_rows % 2);
	double sec_per_row = 60.0 / _master_info.beats_per_minute / beat_rows;
	double sec_per_swing_beat = sec_per_row * swing_beat_rows;
	double sec_before_swing = sec_per_swing_beat * _master_info.swing_amount;
	double bpm_before_swing = 30.0 / sec_before_swing;
	double sec_after_swing = sec_per_swing_beat * (1.0 - _master_info.swing_amount);
	double bpm_after_swing = 30.0 / sec_after_swing;

	*samples_per_row_target = (_master_info.samples_per_tick + _master_info.samples_per_tick_frac) / pattern_resolution;
	*samples_per_row_before_swing = (60.0 * _master_info.samples_per_second) / bpm_before_swing / swing_beat_rows;
	*samples_per_row_after_swing = (60.0 * _master_info.samples_per_second) / bpm_after_swing / swing_beat_rows;
	*outbeat_rows = _master_info.swing_ticks * pattern_resolution;
}

void patternplayer::set_pattern(int id, int _transpose, bool preview) {
	pattern_id = id;
	transpose = _transpose;
	is_preview_pattern = preview;
	
	// reset fractions to parent's fractions to keep fractions in sync
	if (parent) subtick_frac_remainder = parent->subtick_frac_remainder;
}

}
