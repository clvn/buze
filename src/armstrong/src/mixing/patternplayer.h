#pragma once

namespace zzub {

struct patternplugin;

struct patternplayer {
	zzub::plugin* _plugin;
	zzub::mixer* _mixer;
	zzub::master_info _master_info;
	patternplayer* parent;
	int pattern_id;
	int sample_position;
	int next_pattern_row;
	int pattern_row;
	int subtick_position;
	double subtick_frac_remainder;
	double last_next_pattern_samples;
	int next_pattern_samples;
	int seek_row;
	int stop_row;
	bool playing;
	int plugin_group;
	int plugin_track;
	bool is_preview_pattern;
	int transpose;
	int subtick_counter;
	double tick_position;
	bool has_interpolators;
	int reset_interpolator_position;
	int played_rows;

	patternplayer();

	void reset_next_position();
	void reset_position(int row = 0);
	void update(bool offline);
	void advance(int numsamples);
	int advance_pattern(pattern const* pat, int pos, bool offline);
	void set_speed(int bpm, int tpb, float swing, int swing_ticks);
	void get_swing(double* samples_per_row_target, double* samples_per_row_before_swing, double* samples_per_row_after_swing, int* beat_rows);
	void set_pattern(int id, int _transpose, bool preview);
	pattern* get_pattern();
	double get_pattern_resolution();
	void set_parameter(int formatid, int pluginid, int group, int track, int column, int value, bool offline, const pattern_events_by_time::iterator& eventit);
	void process_sequence(bool offline);
	void apply_sequence_size(int& min_chunk_size);
	void stop();
	void play(int stoprow = -1);
	bool is_parent(patternplayer* pp);
	void clear_children(bool is_root);
	void update_interpolator(metaplugin* mpl, int formatid, interpolator* interpol, pattern_events_by_time::iterator eventit);
	void reset_interpolators();
	int reset_interpolator(metaplugin* mpl, int formatid, int group, int track, int column, pattern_events_by_time::iterator eventit);
	double get_samplecount_for_rows(int rows, pattern* playpattern);
	double get_current_samples_per_row(int* next_swing_rows);
};

}
