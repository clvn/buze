#pragma once

struct document;
struct pattern;
struct patternevent;
struct plugin;
struct connection;

struct tableiterator {
	sqlite3_stmt* stmt;
	int prepareresult;
	int stepresult;

	tableiterator();
	tableiterator(sqlite3* db, std::string const& query);
	~tableiterator();
	bool prepare(sqlite3* db, std::string const& query);
	bool next();
	bool eof() const;
	void reset();
	void destroy();
	int id();
};

struct plugin : plugindata {
	document* owner;

	plugin(document* _owner);
	bool update();
	void destroy();
	int get_parameter_value(int group, int track, int column);
	void set_parameter_value(int group, int track, int column, int value);
	bool add_input(int connplugin_id, const plugin& fromplugin, int type, int first_input, int input_count, int first_output, int output_count, std::string midi_device, connectiondata& result);
	bool add_audio_input(int connplugin_id, const plugin& fromplugin, int first_input, int input_count, int first_output, int output_count, connectiondata& result);
	bool add_midi_input(int connplugin_id, const plugin& fromplugin, std::string mididevice, connectiondata& result);
	bool add_event_input(int connplugin_id, const plugin& fromplugin, connectiondata& result);
	bool add_note_input(int connplugin_id, const plugin& fromplugin, connectiondata& result);
	int get_input_connection_count();
	bool get_input_connection_by_type(const plugin& fromplugin, int type, connectiondata& result);
	bool get_input_connection_by_index(int index, connectiondata& result);
	bool get_input_connection_plugin_by_type(const plugin& fromplugin, int type, plugindata& result);
	int get_output_connection_count();
	bool get_output_connection_by_type(const plugin& fromplugin, int type, connectiondata& result);
	bool get_output_connection_by_index(int index, connectiondata& result);

	void add_midimapping(int group, int track, int param, int channel, int controller);
	void remove_midimapping(int group, int track, int param);

	void set_attribute(int index, int value);
	int get_attribute(int index);

	bool get_connection(connectiondata& result);
};

struct pluginparameter : pluginparameterdata {
	document* owner;

	pluginparameter(document* _owner);
	bool update();
};

struct patternevent : patterneventdata {
	document* owner;

	patternevent(document* _owner);
	bool update();
	patternevent& operator=(const patterneventdata& rhs) {
		if (this != &rhs) {
			patterneventdata::operator=(rhs);
		}
		return *this;
	}
};

struct pattern : patterndata {
	document* owner;

	pattern(document* _owner);
	bool update();
	void destroy();

	void set_event_at(int pluginid, int group, int track, int column, int time, int value, int meta);
	//int get_event_at(int pluginid, int group, int track, int column, int time, int* value, int* meta);
	int get_event_at(int pluginparameter_id, int time, int* value, int* meta);
	bool remove_event_at(int pluginid, int group, int track, int column, int time);
	//int get_event_count(int group, int track, int column);
	int get_event_count();
	int get_event_by_index(int group, int track, int column, int index, int* value, int* meta);
	int get_time_by_index(int group, int track, int column, int index);
	void get_events(int pluginid, int group, int track, int column, tableiterator* it);
	void get_events(tableiterator* it);
	void get_events_unsorted(int pluginid, int group, int track, int column, tableiterator* it);
	void get_events_unsorted(tableiterator* it);
	void insert_value(int pluginid, int group, int track, int column, int time, int value, int meta);
	void insert_value(int pluginparameterid, int time, int value, int meta);
	void delete_value(int id_);
	void update_value(int id_, int time, int value, int meta);
	void update_value(int id_, int pluginid, int group, int track, int column, int time, int value, int meta);
	bool get_column_info(int pluginparameter_id, int* type, int* flags, int* minvalue, int* maxvalue, int* novalue);

	// pattern transforms
	void cleanup_pattern();
	void compact_pattern(int factor);
	void expand_pattern(int factor);
	// edit op transforms
	void timeshift_events(int pluginparameter_id, int fromtime, int timeshift);
	void delete_events(int pluginparameter_id, int fromtime, int length);
	void move_scale_events(int src_idx, int src_time, int dst_idx, int dst_time, int width, int length, int mode, int makecopy);
	void paste_stream_events(int fromidx, int fromtime, int mode, char const* charbuf);
	// selection transforms
	void transpose_events(int pluginparameter_id, int fromtime, int length, int delta, int* holes, int holecount, int* metas, int metacount, int chromatic);
	void randomize_events(int pluginparameter_id, int fromtime, int length, int skip);
	void randomize_range_events(int pluginparameter_id, int fromtime, int length, int from_val, int to_val, int skip);
	void randomize_from_events(int pluginparameter_id, int fromtime, int length, int skip);
	void humanize_events(int pluginparameter_id, int fromtime, int length, int deviation);
	void shuffle_events(int pluginparameter_id, int fromtime, int length);
	void interpolate_events(int pluginparameter_id, int fromtime, int length, int skip);
	void gradiate_events(int pluginparameter_id, int fromtime, int length, int skip);
	void smooth_events(int pluginparameter_id, int fromtime, int length, int strength);
	void reverse_events(int pluginparameter_id, int fromtime, int length);
	void compact_events(int pluginparameter_id, int fromtime, int length, int factor);
	void expand_events(int pluginparameter_id, int fromtime, int length, int factor);
	void thin_events(int pluginparameter_id, int fromtime, int length, int major);
	void repeat_events(int pluginparameter_id, int fromtime, int length, int major);
	void echo_events(int pluginparameter_id, int fromtime, int length, int major);
	void unique_events(int pluginparameter_id, int fromtime, int length);
	void scale_events(int pluginparameter_id, int fromtime, int length, double min1, double max1, double min2, double max2);
	void fade_events(int pluginparameter_id, int fromtime, int length, double from, double to);
	void curvemap_events(int pluginparameter_id, int fromtime, int length, int mode);
	void invert_events(int pluginparameter_id, int fromtime, int length);
	void rotate_rows_events(int pluginparameter_id, int fromtime, int length, int offset);
	void rotate_vals_events(int pluginparameter_id, int fromtime, int length, int offset);
	void rotate_dist_events(int pluginparameter_id, int fromtime, int length, int offset);
	void set_events(int pluginparameter_id, int fromtime, int length, int value, int meta);
	void replace_events(int pluginparameter_id, int fromtime, int length, int from_value, int from_meta, int to_value, int to_meta);
	void remove_events(int pluginparameter_id, int fromtime, int length, int value, int meta);
	void notelength_events(int pluginparameter_id, int fromtime, int length, int desired_len, int mode, int off_value);
	void volumes_events(int note_pluginparameter_id, int vol_pluginparameter_id, int fromtime, int length, int mode);
	void swap_track_events(int left_idx, int right_idx, int fromtime, int length);
	void swap_rows_events(int pluginparameter_id, int top_row, int bottom_row);
	void invert_chord_events(int left_idx, int right_idx, int fromtime, int length, int direction, int mode);
	// note transforms - works on notes in voice-independent manner, for pianorolls etc
	void move_and_transpose_notes(int* eventids, int numevents, int timeshift, int pitchshift, int mode);
	void insert_note(int pluginid, int time, int note, int length);
	void update_note(int id, int time, int note, int length);
};

struct patternformat : patternformatdata {
	document* owner;
	patternformat(document* _owner);
	bool update();
	void destroy();
	void get_columns(tableiterator* it);
	bool get_column(int pluginid, int group, int track, int column, patternformatcolumndata& result);
	bool get_track(int pluginid, int group, int track, patternformattrackdata& result);
	bool create_track(int pluginid, int group, int track, std::string const& label, int mute, patternformattrackdata& result);
};

struct patternformatcolumn : patternformatcolumndata {
	document* owner;
	patternformatcolumn(document* _owner);
	void destroy();
	bool update();
	/*void add_filter(int patternformat_id);
	void remove_filter(int patternformat_id);
	void get_filters(tableiterator* it);*/
};

struct patternformattrack : patternformattrackdata {
	document* owner;
	patternformattrack(document* _owner);
	void destroy();
	bool update();
};

struct connection : connectiondata {
	document* owner;

	connection(document* _owner);
	bool update();
	void destroy();
	int get_input_connection_index();
	int get_output_connection_index();
	void set_midi_device(const char* name);
	void add_event_binding(int sourceparam, int targetgroup, int targettrack, int targetparam);
	void delete_event_binding(int sourceparam, int targetgroup, int targettrack, int targetparam);
	int get_event_binding_count();
	void get_event_bindings(tableiterator* it);
	void set_channels(int first_input, int first_output, int inputs, int outputs);
};

struct midimapping : midimappingdata {
	document* owner;

	midimapping(document* _owner);
	bool update();
	void destroy();
};

struct wave : wavedata {
	document* owner;

	wave(document* _owner);
	bool get_wavelevel_by_index(int id, waveleveldata& result);
	int get_wavelevel_count();
	int get_wave_index();
	int get_envelope_count();
	void add_envelope();
	void get_envelopes(tableiterator* it);
	void get_envelope_by_index(int index, envelopedata& result);
	void set_stereo(bool state);
	bool update();
	void clear();
	void destroy();
};

struct wavelevel : waveleveldata {
	struct chunk {
		std::vector<boost::shared_array<float> > buffer;
		int numsamples;
	};
	document* owner;

	wavelevel(document* _owner);
	void insert_sample_range(const void* buffer, int offset, int length, int format, int channels);
	void insert_chunks(std::vector<chunk>& chunks, int offset);
	void replace_sample_range(const void* buffer, int offset, int length, int format, int channels);
	void delete_sample_range(int offset, int length);
	int get_sample_count();
	int get_bytes_per_sample();
	int get_slices_count();
	void set_slices(const std::vector<int>& slices);
	void get_slices(std::vector<int>& slices);
	bool update();
	void destroy();
};

struct envelope : envelopedata {
	document* owner;

	envelope(document* _owner);
	int get_envelope_point_count();
	void get_envelope_points(tableiterator* it);
	void insert_point(int x, int y, int flags, envelopepointdata& result);
	void get_point(int index, envelopepointdata& result);

	bool update();
	void destroy();
};

struct envelopepoint : envelopepointdata {
	document* owner;

	envelopepoint(document* _owner);
	bool update();
	void destroy();
};

struct song : songdata {
	document* owner;

	song(document* _owner);

	bool update();
	void destroy();
	bool create_attributeinfo(int plugininfo_id, int index, std::string name, int minvalue, int maxvalue, int defaultvalue, attributeinfodata& result);
	bool create_plugininfo(int flags, std::string uri, std::string name, std::string short_name, std::string author, int mintrack, int maxtracks, int inputs, int outputs, plugininfodata& result);
	bool create_parameterinfo(int plugininfo_id, int group, int track, int column, std::string name, std::string description, int flags, int type, int minvalue, int maxvalue, int novalue, int defaultvalue, parameterinfodata& result);
	bool create_plugin(int plugininfo_id, std::vector<char>& bytes, std::string name, int tracks, int flags, int plugingroup_id, plugindata& result);
	bool create_pattern(int format_id, const char* name, int length, patterndata& result);
	bool create_pattern_format(std::string name, patternformatdata& result);
	bool create_pattern_format_column(int format_id, int pluginparameter_id, int idx, patternformatcolumndata& result);
	//bool create_pattern_format_column(int format_id, int plugin_id, int group, int track, int column, int idx, patternformatcolumndata& result);
	bool create_wave(int flags, wavedata& result);
	bool create_wavelevel(int wave_id, int format, int samplerate, waveleveldata& result);
	bool create_midimapping(int plugin_id, int group, int track, int column, int midichannel, int midicc);
	bool create_plugingroup(int parent_id, const std::string& name, plugingroupdata& result);
	int get_plugin_count();
	int get_sequence_count();
	int get_pattern_count();
	int get_pattern_format_count();
	int get_wave_count();
	int get_midimapping_count();
	void get_plugins(tableiterator* it);
	void get_connections(tableiterator* it);
	void get_patterns(tableiterator* it);
	void get_patternformats(tableiterator* it);
	void get_midimappings(tableiterator* it);
	void get_parameterinfos(int plugininfo_id, int group, tableiterator* it);
	void get_attributeinfos(int plugininfo_id, tableiterator* it);
	void get_plugingroups(tableiterator* it);
	void get_plugingroups(int parent_plugingroup_id, tableiterator* it);
	int get_pattern_format_column_default_idx(int format_id, int pluginparameter_id);
	//int get_pattern_format_column_default_idx(int format_id, int plugin_id, int group, int track, int column);

	bool get_plugin_by_id(int id, plugindata& result);
	bool get_plugin_by_name(std::string name, plugindata& result);
	bool get_plugin_by_index(int index, plugindata& result);
	bool get_plugininfo_by_id(int id, plugininfodata& result);
	bool get_plugininfo_by_plugin_id(int id, plugininfodata& result);
	bool get_plugininfo_by_uri(std::string uri, plugininfodata& result);
	bool get_parameterinfo_by_id(int id, parameterinfodata& result);
	bool get_parameterinfo(int pluginid, int group, int track, int column, parameterinfodata& result);
	bool get_attributeinfo_by_id(int id, attributeinfodata& result);
	bool get_attribute_by_id(int id, attributedata& result);
	bool get_pattern_by_id(int id, patterndata& result);
	bool get_pattern_by_name(std::string name, patterndata& result);
	bool get_pattern_by_index(int index, patterndata& result);
	bool get_patternevent_by_id(int id, patterneventdata& result);
	bool get_patternformat_by_id(int id, patternformatdata& result);
	bool get_patternformat_by_name(std::string name, patternformatdata& result);
	bool get_patternformat_by_index(int index, patternformatdata& result);
	bool get_patternformatcolumn_by_id(int id, patternformatcolumndata& result);
	bool get_patternformatcolumn_by_filter_id(int id, patternformatcolumndata& result);
	//bool get_patternformatcolumnfilter_by_id(int id, patternformatcolumnfilterdata& result);
	bool get_patternformattrack_by_id(int id, patternformattrackdata& result);
	bool get_connection_by_id(int id, connectiondata& result);
	bool get_eventconnectionbinding_by_id(int id, eventconnectionbindingdata& result);
	bool get_pluginparameter_by_id(int id, pluginparameterdata& result);
	bool get_patternorder_by_id(int id, patternorderdata& result);
	bool get_wave_by_id(int id, wavedata& result);
	bool get_wave_by_index(int index, wavedata& result);
	bool get_wave_by_wavelevel_id(int wavelevel_id, wavedata& result);
	bool get_wavelevel_by_id(int id, waveleveldata& result);
	bool get_midimapping_by_id(int id, midimappingdata& result);
	bool get_midimapping_by_index(int index, midimappingdata& result);
	bool get_envelope_by_id(int id, envelopedata& result);
	bool get_plugingroup_by_id(int id, plugingroupdata& result);

	int get_order_length();
	void set_order_length(int length);
	void set_order_pattern(int index, int pattern_id);
	int get_order_pattern_id(int index);
	void get_order_patterns(tableiterator* it);
	void timeshift_order(int index, int timeshift);
};

struct plugingroup : plugingroupdata {
	document* owner;

	plugingroup(document* _owner);

	bool update();
	void destroy();
	void get_plugins(tableiterator* it);
	plugingroup& operator=(const plugingroupdata& rhs) {
		if (this != &rhs) {
			plugingroupdata::operator=(rhs);
		}
		return *this;
	}
};
