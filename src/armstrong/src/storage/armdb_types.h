#pragma once

// (automatically generated)

enum {
	event_type_before_insert_song, 
	event_type_insert_song, 
	event_type_before_delete_song, 
	event_type_delete_song, 
	event_type_before_update_song, 
	event_type_update_song, 
	event_type_before_insert_patternformat, 
	event_type_insert_patternformat, 
	event_type_before_delete_patternformat, 
	event_type_delete_patternformat, 
	event_type_before_update_patternformat, 
	event_type_update_patternformat, 
	event_type_before_insert_plugingroup, 
	event_type_insert_plugingroup, 
	event_type_before_delete_plugingroup, 
	event_type_delete_plugingroup, 
	event_type_before_update_plugingroup, 
	event_type_update_plugingroup, 
	event_type_before_insert_plugininfo, 
	event_type_insert_plugininfo, 
	event_type_before_delete_plugininfo, 
	event_type_delete_plugininfo, 
	event_type_before_update_plugininfo, 
	event_type_update_plugininfo, 
	event_type_before_insert_wave, 
	event_type_insert_wave, 
	event_type_before_delete_wave, 
	event_type_delete_wave, 
	event_type_before_update_wave, 
	event_type_update_wave, 
	event_type_before_insert_pattern, 
	event_type_insert_pattern, 
	event_type_before_delete_pattern, 
	event_type_delete_pattern, 
	event_type_before_update_pattern, 
	event_type_update_pattern, 
	event_type_before_insert_attributeinfo, 
	event_type_insert_attributeinfo, 
	event_type_before_delete_attributeinfo, 
	event_type_delete_attributeinfo, 
	event_type_before_update_attributeinfo, 
	event_type_update_attributeinfo, 
	event_type_before_insert_parameterinfo, 
	event_type_insert_parameterinfo, 
	event_type_before_delete_parameterinfo, 
	event_type_delete_parameterinfo, 
	event_type_before_update_parameterinfo, 
	event_type_update_parameterinfo, 
	event_type_before_insert_plugin, 
	event_type_insert_plugin, 
	event_type_before_delete_plugin, 
	event_type_delete_plugin, 
	event_type_before_update_plugin, 
	event_type_update_plugin, 
	event_type_before_insert_envelope, 
	event_type_insert_envelope, 
	event_type_before_delete_envelope, 
	event_type_delete_envelope, 
	event_type_before_update_envelope, 
	event_type_update_envelope, 
	event_type_before_insert_wavelevel, 
	event_type_insert_wavelevel, 
	event_type_before_delete_wavelevel, 
	event_type_delete_wavelevel, 
	event_type_before_update_wavelevel, 
	event_type_update_wavelevel, 
	event_type_before_insert_patternorder, 
	event_type_insert_patternorder, 
	event_type_before_delete_patternorder, 
	event_type_delete_patternorder, 
	event_type_before_update_patternorder, 
	event_type_update_patternorder, 
	event_type_before_insert_attribute, 
	event_type_insert_attribute, 
	event_type_before_delete_attribute, 
	event_type_delete_attribute, 
	event_type_before_update_attribute, 
	event_type_update_attribute, 
	event_type_before_insert_connection, 
	event_type_insert_connection, 
	event_type_before_delete_connection, 
	event_type_delete_connection, 
	event_type_before_update_connection, 
	event_type_update_connection, 
	event_type_before_insert_midimapping, 
	event_type_insert_midimapping, 
	event_type_before_delete_midimapping, 
	event_type_delete_midimapping, 
	event_type_before_update_midimapping, 
	event_type_update_midimapping, 
	event_type_before_insert_patternformattrack, 
	event_type_insert_patternformattrack, 
	event_type_before_delete_patternformattrack, 
	event_type_delete_patternformattrack, 
	event_type_before_update_patternformattrack, 
	event_type_update_patternformattrack, 
	event_type_before_insert_pluginparameter, 
	event_type_insert_pluginparameter, 
	event_type_before_delete_pluginparameter, 
	event_type_delete_pluginparameter, 
	event_type_before_update_pluginparameter, 
	event_type_update_pluginparameter, 
	event_type_before_insert_envelopepoint, 
	event_type_insert_envelopepoint, 
	event_type_before_delete_envelopepoint, 
	event_type_delete_envelopepoint, 
	event_type_before_update_envelopepoint, 
	event_type_update_envelopepoint, 
	event_type_before_insert_slice, 
	event_type_insert_slice, 
	event_type_before_delete_slice, 
	event_type_delete_slice, 
	event_type_before_update_slice, 
	event_type_update_slice, 
	event_type_before_insert_eventconnectionbinding, 
	event_type_insert_eventconnectionbinding, 
	event_type_before_delete_eventconnectionbinding, 
	event_type_delete_eventconnectionbinding, 
	event_type_before_update_eventconnectionbinding, 
	event_type_update_eventconnectionbinding, 
	event_type_before_insert_patternevent, 
	event_type_insert_patternevent, 
	event_type_before_delete_patternevent, 
	event_type_delete_patternevent, 
	event_type_before_update_patternevent, 
	event_type_update_patternevent, 
	event_type_before_insert_patternformatcolumn, 
	event_type_insert_patternformatcolumn, 
	event_type_before_delete_patternformatcolumn, 
	event_type_delete_patternformatcolumn, 
	event_type_before_update_patternformatcolumn, 
	event_type_update_patternformatcolumn, 

	event_type_barrier, 
	event_type_delete_samples, 
	event_type_ensure_plugin_parameters, 
	event_type_insert_samples, 
	event_type_orderlist_timeshift, 
};

struct songdata {
	struct table_traits {
		static const char* name() { return "song"; }
		static int before_insert() { return event_type_before_insert_song; }
		static int after_insert() { return event_type_insert_song; }
		static int before_update() { return event_type_before_update_song; }
		static int after_update() { return event_type_update_song; }
		static int before_delete() { return event_type_before_delete_song; }
		static int after_delete() { return event_type_delete_song; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type songdata::*member() { return &songdata::id; };
	};
	struct _version {
		static const char* name() { return "version"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::version; };
	};
	struct _title {
		static const char* name() { return "title"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::title; };
	};
	struct _comment {
		static const char* name() { return "comment"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::vector<unsigned char> type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::comment; };
	};
	struct _loopbegin {
		static const char* name() { return "loopbegin"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::loopbegin; };
	};
	struct _loopend {
		static const char* name() { return "loopend"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::loopend; };
	};
	struct _loopenabled {
		static const char* name() { return "loopenabled"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::loopenabled; };
	};
	struct _samplerate {
		static const char* name() { return "samplerate"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::samplerate; };
	};
	struct _bpm {
		static const char* name() { return "bpm"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::bpm; };
	};
	struct _tpb {
		static const char* name() { return "tpb"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::tpb; };
	};
	struct _swing {
		static const char* name() { return "swing"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef double type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::swing; };
	};
	struct _swingticks {
		static const char* name() { return "swingticks"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::swingticks; };
	};
	struct _machineview_x {
		static const char* name() { return "machineview_x"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef double type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::machineview_x; };
	};
	struct _machineview_y {
		static const char* name() { return "machineview_y"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef double type;
		enum { is_primary = false, is_nullable = true };
		static type songdata::*member() { return &songdata::machineview_y; };
	};
	typedef boost::mpl::vector<
		_id, 
		_version, 
		_title, 
		_comment, 
		_loopbegin, 
		_loopend, 
		_loopenabled, 
		_samplerate, 
		_bpm, 
		_tpb, 
		_swing, 
		_swingticks, 
		_machineview_x, 
		_machineview_y
	> column_members;

	_id::type id;
	_version::type version;
	_title::type title;
	_comment::type comment;
	_loopbegin::type loopbegin;
	_loopend::type loopend;
	_loopenabled::type loopenabled;
	_samplerate::type samplerate;
	_bpm::type bpm;
	_tpb::type tpb;
	_swing::type swing;
	_swingticks::type swingticks;
	_machineview_x::type machineview_x;
	_machineview_y::type machineview_y;
};

struct patternformatdata {
	struct table_traits {
		static const char* name() { return "patternformat"; }
		static int before_insert() { return event_type_before_insert_patternformat; }
		static int after_insert() { return event_type_insert_patternformat; }
		static int before_update() { return event_type_before_update_patternformat; }
		static int after_update() { return event_type_update_patternformat; }
		static int before_delete() { return event_type_before_delete_patternformat; }
		static int after_delete() { return event_type_delete_patternformat; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type patternformatdata::*member() { return &patternformatdata::id; };
	};
	struct _song_id {
		static const char* name() { return "song_id"; }
		static const char* keytable() { return "song"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patternformatdata::*member() { return &patternformatdata::song_id; };
	};
	struct _name {
		static const char* name() { return "name"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type patternformatdata::*member() { return &patternformatdata::name; };
	};
	struct _scroller_width {
		static const char* name() { return "scroller_width"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patternformatdata::*member() { return &patternformatdata::scroller_width; };
	};
	typedef boost::mpl::vector<
		_id, 
		_song_id, 
		_name, 
		_scroller_width
	> column_members;

	_id::type id;
	_song_id::type song_id;
	_name::type name;
	_scroller_width::type scroller_width;
};

struct plugingroupdata {
	struct table_traits {
		static const char* name() { return "plugingroup"; }
		static int before_insert() { return event_type_before_insert_plugingroup; }
		static int after_insert() { return event_type_insert_plugingroup; }
		static int before_update() { return event_type_before_update_plugingroup; }
		static int after_update() { return event_type_update_plugingroup; }
		static int before_delete() { return event_type_before_delete_plugingroup; }
		static int after_delete() { return event_type_delete_plugingroup; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type plugingroupdata::*member() { return &plugingroupdata::id; };
	};
	struct _song_id {
		static const char* name() { return "song_id"; }
		static const char* keytable() { return "song"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type plugingroupdata::*member() { return &plugingroupdata::song_id; };
	};
	struct _name {
		static const char* name() { return "name"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type plugingroupdata::*member() { return &plugingroupdata::name; };
	};
	struct _parent_plugingroup_id {
		static const char* name() { return "parent_plugingroup_id"; }
		static const char* keytable() { return "plugingroup"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugingroupdata::*member() { return &plugingroupdata::parent_plugingroup_id; };
	};
	struct _position_x {
		static const char* name() { return "position_x"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef double type;
		enum { is_primary = false, is_nullable = true };
		static type plugingroupdata::*member() { return &plugingroupdata::position_x; };
	};
	struct _position_y {
		static const char* name() { return "position_y"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef double type;
		enum { is_primary = false, is_nullable = true };
		static type plugingroupdata::*member() { return &plugingroupdata::position_y; };
	};
	typedef boost::mpl::vector<
		_id, 
		_song_id, 
		_name, 
		_parent_plugingroup_id, 
		_position_x, 
		_position_y
	> column_members;

	_id::type id;
	_song_id::type song_id;
	_name::type name;
	_parent_plugingroup_id::type parent_plugingroup_id;
	_position_x::type position_x;
	_position_y::type position_y;
};

struct plugininfodata {
	struct table_traits {
		static const char* name() { return "plugininfo"; }
		static int before_insert() { return event_type_before_insert_plugininfo; }
		static int after_insert() { return event_type_insert_plugininfo; }
		static int before_update() { return event_type_before_update_plugininfo; }
		static int after_update() { return event_type_update_plugininfo; }
		static int before_delete() { return event_type_before_delete_plugininfo; }
		static int after_delete() { return event_type_delete_plugininfo; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type plugininfodata::*member() { return &plugininfodata::id; };
	};
	struct _song_id {
		static const char* name() { return "song_id"; }
		static const char* keytable() { return "song"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type plugininfodata::*member() { return &plugininfodata::song_id; };
	};
	struct _flags {
		static const char* name() { return "flags"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugininfodata::*member() { return &plugininfodata::flags; };
	};
	struct _uri {
		static const char* name() { return "uri"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type plugininfodata::*member() { return &plugininfodata::uri; };
	};
	struct _name {
		static const char* name() { return "name"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type plugininfodata::*member() { return &plugininfodata::name; };
	};
	struct _short_name {
		static const char* name() { return "short_name"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type plugininfodata::*member() { return &plugininfodata::short_name; };
	};
	struct _author {
		static const char* name() { return "author"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type plugininfodata::*member() { return &plugininfodata::author; };
	};
	struct _mintracks {
		static const char* name() { return "mintracks"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugininfodata::*member() { return &plugininfodata::mintracks; };
	};
	struct _maxtracks {
		static const char* name() { return "maxtracks"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugininfodata::*member() { return &plugininfodata::maxtracks; };
	};
	struct _input_count {
		static const char* name() { return "input_count"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugininfodata::*member() { return &plugininfodata::input_count; };
	};
	struct _output_count {
		static const char* name() { return "output_count"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugininfodata::*member() { return &plugininfodata::output_count; };
	};
	typedef boost::mpl::vector<
		_id, 
		_song_id, 
		_flags, 
		_uri, 
		_name, 
		_short_name, 
		_author, 
		_mintracks, 
		_maxtracks, 
		_input_count, 
		_output_count
	> column_members;

	_id::type id;
	_song_id::type song_id;
	_flags::type flags;
	_uri::type uri;
	_name::type name;
	_short_name::type short_name;
	_author::type author;
	_mintracks::type mintracks;
	_maxtracks::type maxtracks;
	_input_count::type input_count;
	_output_count::type output_count;
};

struct wavedata {
	struct table_traits {
		static const char* name() { return "wave"; }
		static int before_insert() { return event_type_before_insert_wave; }
		static int after_insert() { return event_type_insert_wave; }
		static int before_update() { return event_type_before_update_wave; }
		static int after_update() { return event_type_update_wave; }
		static int before_delete() { return event_type_before_delete_wave; }
		static int after_delete() { return event_type_delete_wave; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type wavedata::*member() { return &wavedata::id; };
	};
	struct _song_id {
		static const char* name() { return "song_id"; }
		static const char* keytable() { return "song"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type wavedata::*member() { return &wavedata::song_id; };
	};
	struct _name {
		static const char* name() { return "name"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type wavedata::*member() { return &wavedata::name; };
	};
	struct _filename {
		static const char* name() { return "filename"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type wavedata::*member() { return &wavedata::filename; };
	};
	struct _flags {
		static const char* name() { return "flags"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type wavedata::*member() { return &wavedata::flags; };
	};
	struct _volume {
		static const char* name() { return "volume"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef double type;
		enum { is_primary = false, is_nullable = true };
		static type wavedata::*member() { return &wavedata::volume; };
	};
	typedef boost::mpl::vector<
		_id, 
		_song_id, 
		_name, 
		_filename, 
		_flags, 
		_volume
	> column_members;

	_id::type id;
	_song_id::type song_id;
	_name::type name;
	_filename::type filename;
	_flags::type flags;
	_volume::type volume;
};

struct patterndata {
	struct table_traits {
		static const char* name() { return "pattern"; }
		static int before_insert() { return event_type_before_insert_pattern; }
		static int after_insert() { return event_type_insert_pattern; }
		static int before_update() { return event_type_before_update_pattern; }
		static int after_update() { return event_type_update_pattern; }
		static int before_delete() { return event_type_before_delete_pattern; }
		static int after_delete() { return event_type_delete_pattern; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type patterndata::*member() { return &patterndata::id; };
	};
	struct _song_id {
		static const char* name() { return "song_id"; }
		static const char* keytable() { return "song"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patterndata::*member() { return &patterndata::song_id; };
	};
	struct _name {
		static const char* name() { return "name"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::name; };
	};
	struct _length {
		static const char* name() { return "length"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::length; };
	};
	struct _resolution {
		static const char* name() { return "resolution"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::resolution; };
	};
	struct _beginloop {
		static const char* name() { return "beginloop"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::beginloop; };
	};
	struct _endloop {
		static const char* name() { return "endloop"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::endloop; };
	};
	struct _loopenabled {
		static const char* name() { return "loopenabled"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::loopenabled; };
	};
	struct _display_resolution {
		static const char* name() { return "display_resolution"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::display_resolution; };
	};
	struct _display_verydark_row {
		static const char* name() { return "display_verydark_row"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::display_verydark_row; };
	};
	struct _display_dark_row {
		static const char* name() { return "display_dark_row"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::display_dark_row; };
	};
	struct _patternformat_id {
		static const char* name() { return "patternformat_id"; }
		static const char* keytable() { return "patternformat"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patterndata::*member() { return &patterndata::patternformat_id; };
	};
	struct _replay_row {
		static const char* name() { return "replay_row"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterndata::*member() { return &patterndata::replay_row; };
	};
	typedef boost::mpl::vector<
		_id, 
		_song_id, 
		_name, 
		_length, 
		_resolution, 
		_beginloop, 
		_endloop, 
		_loopenabled, 
		_display_resolution, 
		_display_verydark_row, 
		_display_dark_row, 
		_patternformat_id, 
		_replay_row
	> column_members;

	_id::type id;
	_song_id::type song_id;
	_name::type name;
	_length::type length;
	_resolution::type resolution;
	_beginloop::type beginloop;
	_endloop::type endloop;
	_loopenabled::type loopenabled;
	_display_resolution::type display_resolution;
	_display_verydark_row::type display_verydark_row;
	_display_dark_row::type display_dark_row;
	_patternformat_id::type patternformat_id;
	_replay_row::type replay_row;
};

struct attributeinfodata {
	struct table_traits {
		static const char* name() { return "attributeinfo"; }
		static int before_insert() { return event_type_before_insert_attributeinfo; }
		static int after_insert() { return event_type_insert_attributeinfo; }
		static int before_update() { return event_type_before_update_attributeinfo; }
		static int after_update() { return event_type_update_attributeinfo; }
		static int before_delete() { return event_type_before_delete_attributeinfo; }
		static int after_delete() { return event_type_delete_attributeinfo; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type attributeinfodata::*member() { return &attributeinfodata::id; };
	};
	struct _plugininfo_id {
		static const char* name() { return "plugininfo_id"; }
		static const char* keytable() { return "plugininfo"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type attributeinfodata::*member() { return &attributeinfodata::plugininfo_id; };
	};
	struct _attrindex {
		static const char* name() { return "attrindex"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type attributeinfodata::*member() { return &attributeinfodata::attrindex; };
	};
	struct _name {
		static const char* name() { return "name"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type attributeinfodata::*member() { return &attributeinfodata::name; };
	};
	struct _minvalue {
		static const char* name() { return "minvalue"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type attributeinfodata::*member() { return &attributeinfodata::minvalue; };
	};
	struct _maxvalue {
		static const char* name() { return "maxvalue"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type attributeinfodata::*member() { return &attributeinfodata::maxvalue; };
	};
	struct _defaultvalue {
		static const char* name() { return "defaultvalue"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type attributeinfodata::*member() { return &attributeinfodata::defaultvalue; };
	};
	typedef boost::mpl::vector<
		_id, 
		_plugininfo_id, 
		_attrindex, 
		_name, 
		_minvalue, 
		_maxvalue, 
		_defaultvalue
	> column_members;

	_id::type id;
	_plugininfo_id::type plugininfo_id;
	_attrindex::type attrindex;
	_name::type name;
	_minvalue::type minvalue;
	_maxvalue::type maxvalue;
	_defaultvalue::type defaultvalue;
};

struct parameterinfodata {
	struct table_traits {
		static const char* name() { return "parameterinfo"; }
		static int before_insert() { return event_type_before_insert_parameterinfo; }
		static int after_insert() { return event_type_insert_parameterinfo; }
		static int before_update() { return event_type_before_update_parameterinfo; }
		static int after_update() { return event_type_update_parameterinfo; }
		static int before_delete() { return event_type_before_delete_parameterinfo; }
		static int after_delete() { return event_type_delete_parameterinfo; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type parameterinfodata::*member() { return &parameterinfodata::id; };
	};
	struct _plugininfo_id {
		static const char* name() { return "plugininfo_id"; }
		static const char* keytable() { return "plugininfo"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type parameterinfodata::*member() { return &parameterinfodata::plugininfo_id; };
	};
	struct _paramgroup {
		static const char* name() { return "paramgroup"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::paramgroup; };
	};
	struct _paramcolumn {
		static const char* name() { return "paramcolumn"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::paramcolumn; };
	};
	struct _name {
		static const char* name() { return "name"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::name; };
	};
	struct _description {
		static const char* name() { return "description"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::description; };
	};
	struct _flags {
		static const char* name() { return "flags"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::flags; };
	};
	struct _type {
		static const char* name() { return "type"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::type; };
	};
	struct _minvalue {
		static const char* name() { return "minvalue"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::minvalue; };
	};
	struct _maxvalue {
		static const char* name() { return "maxvalue"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::maxvalue; };
	};
	struct _novalue {
		static const char* name() { return "novalue"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::novalue; };
	};
	struct _defaultvalue {
		static const char* name() { return "defaultvalue"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type parameterinfodata::*member() { return &parameterinfodata::defaultvalue; };
	};
	typedef boost::mpl::vector<
		_id, 
		_plugininfo_id, 
		_paramgroup, 
		_paramcolumn, 
		_name, 
		_description, 
		_flags, 
		_type, 
		_minvalue, 
		_maxvalue, 
		_novalue, 
		_defaultvalue
	> column_members;

	_id::type id;
	_plugininfo_id::type plugininfo_id;
	_paramgroup::type paramgroup;
	_paramcolumn::type paramcolumn;
	_name::type name;
	_description::type description;
	_flags::type flags;
	_type::type type;
	_minvalue::type minvalue;
	_maxvalue::type maxvalue;
	_novalue::type novalue;
	_defaultvalue::type defaultvalue;
};

struct plugindata {
	struct table_traits {
		static const char* name() { return "plugin"; }
		static int before_insert() { return event_type_before_insert_plugin; }
		static int after_insert() { return event_type_insert_plugin; }
		static int before_update() { return event_type_before_update_plugin; }
		static int after_update() { return event_type_update_plugin; }
		static int before_delete() { return event_type_before_delete_plugin; }
		static int after_delete() { return event_type_delete_plugin; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type plugindata::*member() { return &plugindata::id; };
	};
	struct _flags {
		static const char* name() { return "flags"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::flags; };
	};
	struct _song_id {
		static const char* name() { return "song_id"; }
		static const char* keytable() { return "song"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type plugindata::*member() { return &plugindata::song_id; };
	};
	struct _name {
		static const char* name() { return "name"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::name; };
	};
	struct _data {
		static const char* name() { return "data"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::vector<unsigned char> type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::data; };
	};
	struct _trackcount {
		static const char* name() { return "trackcount"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::trackcount; };
	};
	struct _x {
		static const char* name() { return "x"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef double type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::x; };
	};
	struct _y {
		static const char* name() { return "y"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef double type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::y; };
	};
	struct _streamsource {
		static const char* name() { return "streamsource"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::streamsource; };
	};
	struct _is_muted {
		static const char* name() { return "is_muted"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::is_muted; };
	};
	struct _is_bypassed {
		static const char* name() { return "is_bypassed"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::is_bypassed; };
	};
	struct _is_solo {
		static const char* name() { return "is_solo"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::is_solo; };
	};
	struct _is_minimized {
		static const char* name() { return "is_minimized"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::is_minimized; };
	};
	struct _plugininfo_id {
		static const char* name() { return "plugininfo_id"; }
		static const char* keytable() { return "plugininfo"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::plugininfo_id; };
	};
	struct _timesource_plugin_id {
		static const char* name() { return "timesource_plugin_id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::timesource_plugin_id; };
	};
	struct _timesource_group {
		static const char* name() { return "timesource_group"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::timesource_group; };
	};
	struct _timesource_track {
		static const char* name() { return "timesource_track"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::timesource_track; };
	};
	struct _latency {
		static const char* name() { return "latency"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::latency; };
	};
	struct _plugingroup_id {
		static const char* name() { return "plugingroup_id"; }
		static const char* keytable() { return "plugingroup"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type plugindata::*member() { return &plugindata::plugingroup_id; };
	};
	typedef boost::mpl::vector<
		_id, 
		_flags, 
		_song_id, 
		_name, 
		_data, 
		_trackcount, 
		_x, 
		_y, 
		_streamsource, 
		_is_muted, 
		_is_bypassed, 
		_is_solo, 
		_is_minimized, 
		_plugininfo_id, 
		_timesource_plugin_id, 
		_timesource_group, 
		_timesource_track, 
		_latency, 
		_plugingroup_id
	> column_members;

	_id::type id;
	_flags::type flags;
	_song_id::type song_id;
	_name::type name;
	_data::type data;
	_trackcount::type trackcount;
	_x::type x;
	_y::type y;
	_streamsource::type streamsource;
	_is_muted::type is_muted;
	_is_bypassed::type is_bypassed;
	_is_solo::type is_solo;
	_is_minimized::type is_minimized;
	_plugininfo_id::type plugininfo_id;
	_timesource_plugin_id::type timesource_plugin_id;
	_timesource_group::type timesource_group;
	_timesource_track::type timesource_track;
	_latency::type latency;
	_plugingroup_id::type plugingroup_id;
};

struct envelopedata {
	struct table_traits {
		static const char* name() { return "envelope"; }
		static int before_insert() { return event_type_before_insert_envelope; }
		static int after_insert() { return event_type_insert_envelope; }
		static int before_update() { return event_type_before_update_envelope; }
		static int after_update() { return event_type_update_envelope; }
		static int before_delete() { return event_type_before_delete_envelope; }
		static int after_delete() { return event_type_delete_envelope; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type envelopedata::*member() { return &envelopedata::id; };
	};
	struct _wave_id {
		static const char* name() { return "wave_id"; }
		static const char* keytable() { return "wave"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type envelopedata::*member() { return &envelopedata::wave_id; };
	};
	struct _attack {
		static const char* name() { return "attack"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopedata::*member() { return &envelopedata::attack; };
	};
	struct _decay {
		static const char* name() { return "decay"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopedata::*member() { return &envelopedata::decay; };
	};
	struct _sustain {
		static const char* name() { return "sustain"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopedata::*member() { return &envelopedata::sustain; };
	};
	struct _release {
		static const char* name() { return "release"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopedata::*member() { return &envelopedata::release; };
	};
	struct _subdivision {
		static const char* name() { return "subdivision"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopedata::*member() { return &envelopedata::subdivision; };
	};
	struct _flags {
		static const char* name() { return "flags"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopedata::*member() { return &envelopedata::flags; };
	};
	struct _disabled {
		static const char* name() { return "disabled"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopedata::*member() { return &envelopedata::disabled; };
	};
	typedef boost::mpl::vector<
		_id, 
		_wave_id, 
		_attack, 
		_decay, 
		_sustain, 
		_release, 
		_subdivision, 
		_flags, 
		_disabled
	> column_members;

	_id::type id;
	_wave_id::type wave_id;
	_attack::type attack;
	_decay::type decay;
	_sustain::type sustain;
	_release::type release;
	_subdivision::type subdivision;
	_flags::type flags;
	_disabled::type disabled;
};

struct waveleveldata {
	struct table_traits {
		static const char* name() { return "wavelevel"; }
		static int before_insert() { return event_type_before_insert_wavelevel; }
		static int after_insert() { return event_type_insert_wavelevel; }
		static int before_update() { return event_type_before_update_wavelevel; }
		static int after_update() { return event_type_update_wavelevel; }
		static int before_delete() { return event_type_before_delete_wavelevel; }
		static int after_delete() { return event_type_delete_wavelevel; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type waveleveldata::*member() { return &waveleveldata::id; };
	};
	struct _wave_id {
		static const char* name() { return "wave_id"; }
		static const char* keytable() { return "wave"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type waveleveldata::*member() { return &waveleveldata::wave_id; };
	};
	struct _basenote {
		static const char* name() { return "basenote"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type waveleveldata::*member() { return &waveleveldata::basenote; };
	};
	struct _samplerate {
		static const char* name() { return "samplerate"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type waveleveldata::*member() { return &waveleveldata::samplerate; };
	};
	struct _samplecount {
		static const char* name() { return "samplecount"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type waveleveldata::*member() { return &waveleveldata::samplecount; };
	};
	struct _beginloop {
		static const char* name() { return "beginloop"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type waveleveldata::*member() { return &waveleveldata::beginloop; };
	};
	struct _endloop {
		static const char* name() { return "endloop"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type waveleveldata::*member() { return &waveleveldata::endloop; };
	};
	struct _format {
		static const char* name() { return "format"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type waveleveldata::*member() { return &waveleveldata::format; };
	};
	struct _filename {
		static const char* name() { return "filename"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type waveleveldata::*member() { return &waveleveldata::filename; };
	};
	typedef boost::mpl::vector<
		_id, 
		_wave_id, 
		_basenote, 
		_samplerate, 
		_samplecount, 
		_beginloop, 
		_endloop, 
		_format, 
		_filename
	> column_members;

	_id::type id;
	_wave_id::type wave_id;
	_basenote::type basenote;
	_samplerate::type samplerate;
	_samplecount::type samplecount;
	_beginloop::type beginloop;
	_endloop::type endloop;
	_format::type format;
	_filename::type filename;
};

struct patternorderdata {
	struct table_traits {
		static const char* name() { return "patternorder"; }
		static int before_insert() { return event_type_before_insert_patternorder; }
		static int after_insert() { return event_type_insert_patternorder; }
		static int before_update() { return event_type_before_update_patternorder; }
		static int after_update() { return event_type_update_patternorder; }
		static int before_delete() { return event_type_before_delete_patternorder; }
		static int after_delete() { return event_type_delete_patternorder; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type patternorderdata::*member() { return &patternorderdata::id; };
	};
	struct _song_id {
		static const char* name() { return "song_id"; }
		static const char* keytable() { return "song"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patternorderdata::*member() { return &patternorderdata::song_id; };
	};
	struct _pattern_id {
		static const char* name() { return "pattern_id"; }
		static const char* keytable() { return "pattern"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patternorderdata::*member() { return &patternorderdata::pattern_id; };
	};
	typedef boost::mpl::vector<
		_id, 
		_song_id, 
		_pattern_id
	> column_members;

	_id::type id;
	_song_id::type song_id;
	_pattern_id::type pattern_id;
};

struct attributedata {
	struct table_traits {
		static const char* name() { return "attribute"; }
		static int before_insert() { return event_type_before_insert_attribute; }
		static int after_insert() { return event_type_insert_attribute; }
		static int before_update() { return event_type_before_update_attribute; }
		static int after_update() { return event_type_update_attribute; }
		static int before_delete() { return event_type_before_delete_attribute; }
		static int after_delete() { return event_type_delete_attribute; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type attributedata::*member() { return &attributedata::id; };
	};
	struct _plugin_id {
		static const char* name() { return "plugin_id"; }
		static const char* keytable() { return "plugin"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type attributedata::*member() { return &attributedata::plugin_id; };
	};
	struct _attrindex {
		static const char* name() { return "attrindex"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type attributedata::*member() { return &attributedata::attrindex; };
	};
	struct _value {
		static const char* name() { return "value"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type attributedata::*member() { return &attributedata::value; };
	};
	typedef boost::mpl::vector<
		_id, 
		_plugin_id, 
		_attrindex, 
		_value
	> column_members;

	_id::type id;
	_plugin_id::type plugin_id;
	_attrindex::type attrindex;
	_value::type value;
};

struct connectiondata {
	struct table_traits {
		static const char* name() { return "connection"; }
		static int before_insert() { return event_type_before_insert_connection; }
		static int after_insert() { return event_type_insert_connection; }
		static int before_update() { return event_type_before_update_connection; }
		static int after_update() { return event_type_update_connection; }
		static int before_delete() { return event_type_before_delete_connection; }
		static int after_delete() { return event_type_delete_connection; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type connectiondata::*member() { return &connectiondata::id; };
	};
	struct _plugin_id {
		static const char* name() { return "plugin_id"; }
		static const char* keytable() { return "plugin"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type connectiondata::*member() { return &connectiondata::plugin_id; };
	};
	struct _from_plugin_id {
		static const char* name() { return "from_plugin_id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type connectiondata::*member() { return &connectiondata::from_plugin_id; };
	};
	struct _to_plugin_id {
		static const char* name() { return "to_plugin_id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type connectiondata::*member() { return &connectiondata::to_plugin_id; };
	};
	struct _type {
		static const char* name() { return "type"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type connectiondata::*member() { return &connectiondata::type; };
	};
	struct _first_input {
		static const char* name() { return "first_input"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type connectiondata::*member() { return &connectiondata::first_input; };
	};
	struct _first_output {
		static const char* name() { return "first_output"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type connectiondata::*member() { return &connectiondata::first_output; };
	};
	struct _input_count {
		static const char* name() { return "input_count"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type connectiondata::*member() { return &connectiondata::input_count; };
	};
	struct _output_count {
		static const char* name() { return "output_count"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type connectiondata::*member() { return &connectiondata::output_count; };
	};
	struct _mididevice {
		static const char* name() { return "mididevice"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type connectiondata::*member() { return &connectiondata::mididevice; };
	};
	typedef boost::mpl::vector<
		_id, 
		_plugin_id, 
		_from_plugin_id, 
		_to_plugin_id, 
		_type, 
		_first_input, 
		_first_output, 
		_input_count, 
		_output_count, 
		_mididevice
	> column_members;

	_id::type id;
	_plugin_id::type plugin_id;
	_from_plugin_id::type from_plugin_id;
	_to_plugin_id::type to_plugin_id;
	_type::type type;
	_first_input::type first_input;
	_first_output::type first_output;
	_input_count::type input_count;
	_output_count::type output_count;
	_mididevice::type mididevice;
};

struct midimappingdata {
	struct table_traits {
		static const char* name() { return "midimapping"; }
		static int before_insert() { return event_type_before_insert_midimapping; }
		static int after_insert() { return event_type_insert_midimapping; }
		static int before_update() { return event_type_before_update_midimapping; }
		static int after_update() { return event_type_update_midimapping; }
		static int before_delete() { return event_type_before_delete_midimapping; }
		static int after_delete() { return event_type_delete_midimapping; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type midimappingdata::*member() { return &midimappingdata::id; };
	};
	struct _plugin_id {
		static const char* name() { return "plugin_id"; }
		static const char* keytable() { return "plugin"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type midimappingdata::*member() { return &midimappingdata::plugin_id; };
	};
	struct _paramgroup {
		static const char* name() { return "paramgroup"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type midimappingdata::*member() { return &midimappingdata::paramgroup; };
	};
	struct _paramtrack {
		static const char* name() { return "paramtrack"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type midimappingdata::*member() { return &midimappingdata::paramtrack; };
	};
	struct _paramcolumn {
		static const char* name() { return "paramcolumn"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type midimappingdata::*member() { return &midimappingdata::paramcolumn; };
	};
	struct _midichannel {
		static const char* name() { return "midichannel"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type midimappingdata::*member() { return &midimappingdata::midichannel; };
	};
	struct _midicontroller {
		static const char* name() { return "midicontroller"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type midimappingdata::*member() { return &midimappingdata::midicontroller; };
	};
	typedef boost::mpl::vector<
		_id, 
		_plugin_id, 
		_paramgroup, 
		_paramtrack, 
		_paramcolumn, 
		_midichannel, 
		_midicontroller
	> column_members;

	_id::type id;
	_plugin_id::type plugin_id;
	_paramgroup::type paramgroup;
	_paramtrack::type paramtrack;
	_paramcolumn::type paramcolumn;
	_midichannel::type midichannel;
	_midicontroller::type midicontroller;
};

struct patternformattrackdata {
	struct table_traits {
		static const char* name() { return "patternformattrack"; }
		static int before_insert() { return event_type_before_insert_patternformattrack; }
		static int after_insert() { return event_type_insert_patternformattrack; }
		static int before_update() { return event_type_before_update_patternformattrack; }
		static int after_update() { return event_type_update_patternformattrack; }
		static int before_delete() { return event_type_before_delete_patternformattrack; }
		static int after_delete() { return event_type_delete_patternformattrack; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type patternformattrackdata::*member() { return &patternformattrackdata::id; };
	};
	struct _patternformat_id {
		static const char* name() { return "patternformat_id"; }
		static const char* keytable() { return "patternformat"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patternformattrackdata::*member() { return &patternformattrackdata::patternformat_id; };
	};
	struct _plugin_id {
		static const char* name() { return "plugin_id"; }
		static const char* keytable() { return "plugin"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patternformattrackdata::*member() { return &patternformattrackdata::plugin_id; };
	};
	struct _paramgroup {
		static const char* name() { return "paramgroup"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patternformattrackdata::*member() { return &patternformattrackdata::paramgroup; };
	};
	struct _paramtrack {
		static const char* name() { return "paramtrack"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patternformattrackdata::*member() { return &patternformattrackdata::paramtrack; };
	};
	struct _label {
		static const char* name() { return "label"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef std::string type;
		enum { is_primary = false, is_nullable = true };
		static type patternformattrackdata::*member() { return &patternformattrackdata::label; };
	};
	struct _is_muted {
		static const char* name() { return "is_muted"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patternformattrackdata::*member() { return &patternformattrackdata::is_muted; };
	};
	typedef boost::mpl::vector<
		_id, 
		_patternformat_id, 
		_plugin_id, 
		_paramgroup, 
		_paramtrack, 
		_label, 
		_is_muted
	> column_members;

	_id::type id;
	_patternformat_id::type patternformat_id;
	_plugin_id::type plugin_id;
	_paramgroup::type paramgroup;
	_paramtrack::type paramtrack;
	_label::type label;
	_is_muted::type is_muted;
};

struct pluginparameterdata {
	struct table_traits {
		static const char* name() { return "pluginparameter"; }
		static int before_insert() { return event_type_before_insert_pluginparameter; }
		static int after_insert() { return event_type_insert_pluginparameter; }
		static int before_update() { return event_type_before_update_pluginparameter; }
		static int after_update() { return event_type_update_pluginparameter; }
		static int before_delete() { return event_type_before_delete_pluginparameter; }
		static int after_delete() { return event_type_delete_pluginparameter; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type pluginparameterdata::*member() { return &pluginparameterdata::id; };
	};
	struct _plugin_id {
		static const char* name() { return "plugin_id"; }
		static const char* keytable() { return "plugin"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type pluginparameterdata::*member() { return &pluginparameterdata::plugin_id; };
	};
	struct _parameterinfo_id {
		static const char* name() { return "parameterinfo_id"; }
		static const char* keytable() { return "parameterinfo"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type pluginparameterdata::*member() { return &pluginparameterdata::parameterinfo_id; };
	};
	struct _paramtrack {
		static const char* name() { return "paramtrack"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type pluginparameterdata::*member() { return &pluginparameterdata::paramtrack; };
	};
	struct _value {
		static const char* name() { return "value"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type pluginparameterdata::*member() { return &pluginparameterdata::value; };
	};
	struct _interpolator {
		static const char* name() { return "interpolator"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type pluginparameterdata::*member() { return &pluginparameterdata::interpolator; };
	};
	typedef boost::mpl::vector<
		_id, 
		_plugin_id, 
		_parameterinfo_id, 
		_paramtrack, 
		_value, 
		_interpolator
	> column_members;

	_id::type id;
	_plugin_id::type plugin_id;
	_parameterinfo_id::type parameterinfo_id;
	_paramtrack::type paramtrack;
	_value::type value;
	_interpolator::type interpolator;
};

struct envelopepointdata {
	struct table_traits {
		static const char* name() { return "envelopepoint"; }
		static int before_insert() { return event_type_before_insert_envelopepoint; }
		static int after_insert() { return event_type_insert_envelopepoint; }
		static int before_update() { return event_type_before_update_envelopepoint; }
		static int after_update() { return event_type_update_envelopepoint; }
		static int before_delete() { return event_type_before_delete_envelopepoint; }
		static int after_delete() { return event_type_delete_envelopepoint; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type envelopepointdata::*member() { return &envelopepointdata::id; };
	};
	struct _envelope_id {
		static const char* name() { return "envelope_id"; }
		static const char* keytable() { return "envelope"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type envelopepointdata::*member() { return &envelopepointdata::envelope_id; };
	};
	struct _x {
		static const char* name() { return "x"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopepointdata::*member() { return &envelopepointdata::x; };
	};
	struct _y {
		static const char* name() { return "y"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopepointdata::*member() { return &envelopepointdata::y; };
	};
	struct _flags {
		static const char* name() { return "flags"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type envelopepointdata::*member() { return &envelopepointdata::flags; };
	};
	typedef boost::mpl::vector<
		_id, 
		_envelope_id, 
		_x, 
		_y, 
		_flags
	> column_members;

	_id::type id;
	_envelope_id::type envelope_id;
	_x::type x;
	_y::type y;
	_flags::type flags;
};

struct slicedata {
	struct table_traits {
		static const char* name() { return "slice"; }
		static int before_insert() { return event_type_before_insert_slice; }
		static int after_insert() { return event_type_insert_slice; }
		static int before_update() { return event_type_before_update_slice; }
		static int after_update() { return event_type_update_slice; }
		static int before_delete() { return event_type_before_delete_slice; }
		static int after_delete() { return event_type_delete_slice; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type slicedata::*member() { return &slicedata::id; };
	};
	struct _wavelevel_id {
		static const char* name() { return "wavelevel_id"; }
		static const char* keytable() { return "wavelevel"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type slicedata::*member() { return &slicedata::wavelevel_id; };
	};
	struct _sampleoffset {
		static const char* name() { return "sampleoffset"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type slicedata::*member() { return &slicedata::sampleoffset; };
	};
	typedef boost::mpl::vector<
		_id, 
		_wavelevel_id, 
		_sampleoffset
	> column_members;

	_id::type id;
	_wavelevel_id::type wavelevel_id;
	_sampleoffset::type sampleoffset;
};

struct eventconnectionbindingdata {
	struct table_traits {
		static const char* name() { return "eventconnectionbinding"; }
		static int before_insert() { return event_type_before_insert_eventconnectionbinding; }
		static int after_insert() { return event_type_insert_eventconnectionbinding; }
		static int before_update() { return event_type_before_update_eventconnectionbinding; }
		static int after_update() { return event_type_update_eventconnectionbinding; }
		static int before_delete() { return event_type_before_delete_eventconnectionbinding; }
		static int after_delete() { return event_type_delete_eventconnectionbinding; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type eventconnectionbindingdata::*member() { return &eventconnectionbindingdata::id; };
	};
	struct _connection_id {
		static const char* name() { return "connection_id"; }
		static const char* keytable() { return "connection"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type eventconnectionbindingdata::*member() { return &eventconnectionbindingdata::connection_id; };
	};
	struct _sourceindex {
		static const char* name() { return "sourceindex"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type eventconnectionbindingdata::*member() { return &eventconnectionbindingdata::sourceindex; };
	};
	struct _targetparamgroup {
		static const char* name() { return "targetparamgroup"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type eventconnectionbindingdata::*member() { return &eventconnectionbindingdata::targetparamgroup; };
	};
	struct _targetparamtrack {
		static const char* name() { return "targetparamtrack"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type eventconnectionbindingdata::*member() { return &eventconnectionbindingdata::targetparamtrack; };
	};
	struct _targetparamcolumn {
		static const char* name() { return "targetparamcolumn"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type eventconnectionbindingdata::*member() { return &eventconnectionbindingdata::targetparamcolumn; };
	};
	typedef boost::mpl::vector<
		_id, 
		_connection_id, 
		_sourceindex, 
		_targetparamgroup, 
		_targetparamtrack, 
		_targetparamcolumn
	> column_members;

	_id::type id;
	_connection_id::type connection_id;
	_sourceindex::type sourceindex;
	_targetparamgroup::type targetparamgroup;
	_targetparamtrack::type targetparamtrack;
	_targetparamcolumn::type targetparamcolumn;
};

struct patterneventdata {
	struct table_traits {
		static const char* name() { return "patternevent"; }
		static int before_insert() { return event_type_before_insert_patternevent; }
		static int after_insert() { return event_type_insert_patternevent; }
		static int before_update() { return event_type_before_update_patternevent; }
		static int after_update() { return event_type_update_patternevent; }
		static int before_delete() { return event_type_before_delete_patternevent; }
		static int after_delete() { return event_type_delete_patternevent; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type patterneventdata::*member() { return &patterneventdata::id; };
	};
	struct _pattern_id {
		static const char* name() { return "pattern_id"; }
		static const char* keytable() { return "pattern"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patterneventdata::*member() { return &patterneventdata::pattern_id; };
	};
	struct _time {
		static const char* name() { return "time"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterneventdata::*member() { return &patterneventdata::time; };
	};
	struct _pluginparameter_id {
		static const char* name() { return "pluginparameter_id"; }
		static const char* keytable() { return "pluginparameter"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patterneventdata::*member() { return &patterneventdata::pluginparameter_id; };
	};
	struct _value {
		static const char* name() { return "value"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterneventdata::*member() { return &patterneventdata::value; };
	};
	struct _meta {
		static const char* name() { return "meta"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patterneventdata::*member() { return &patterneventdata::meta; };
	};
	typedef boost::mpl::vector<
		_id, 
		_pattern_id, 
		_time, 
		_pluginparameter_id, 
		_value, 
		_meta
	> column_members;

	_id::type id;
	_pattern_id::type pattern_id;
	_time::type time;
	_pluginparameter_id::type pluginparameter_id;
	_value::type value;
	_meta::type meta;
};

struct patternformatcolumndata {
	struct table_traits {
		static const char* name() { return "patternformatcolumn"; }
		static int before_insert() { return event_type_before_insert_patternformatcolumn; }
		static int after_insert() { return event_type_insert_patternformatcolumn; }
		static int before_update() { return event_type_before_update_patternformatcolumn; }
		static int after_update() { return event_type_update_patternformatcolumn; }
		static int before_delete() { return event_type_before_delete_patternformatcolumn; }
		static int after_delete() { return event_type_delete_patternformatcolumn; }
	};
	struct _id {
		static const char* name() { return "id"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = true, is_nullable = false };
		static type patternformatcolumndata::*member() { return &patternformatcolumndata::id; };
	};
	struct _patternformat_id {
		static const char* name() { return "patternformat_id"; }
		static const char* keytable() { return "patternformat"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patternformatcolumndata::*member() { return &patternformatcolumndata::patternformat_id; };
	};
	struct _pluginparameter_id {
		static const char* name() { return "pluginparameter_id"; }
		static const char* keytable() { return "pluginparameter"; }
		static const char* keyname() { return "id"; }
		typedef int type;
		enum { is_primary = false, is_nullable = false };
		static type patternformatcolumndata::*member() { return &patternformatcolumndata::pluginparameter_id; };
	};
	struct _mode {
		static const char* name() { return "mode"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patternformatcolumndata::*member() { return &patternformatcolumndata::mode; };
	};
	struct _is_collapsed {
		static const char* name() { return "is_collapsed"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patternformatcolumndata::*member() { return &patternformatcolumndata::is_collapsed; };
	};
	struct _idx {
		static const char* name() { return "idx"; }
		static const char* keytable() { return ""; }
		static const char* keyname() { return ""; }
		typedef int type;
		enum { is_primary = false, is_nullable = true };
		static type patternformatcolumndata::*member() { return &patternformatcolumndata::idx; };
	};
	typedef boost::mpl::vector<
		_id, 
		_patternformat_id, 
		_pluginparameter_id, 
		_mode, 
		_is_collapsed, 
		_idx
	> column_members;

	_id::type id;
	_patternformat_id::type patternformat_id;
	_pluginparameter_id::type pluginparameter_id;
	_mode::type mode;
	_is_collapsed::type is_collapsed;
	_idx::type idx;
};

typedef boost::mpl::vector<
	songdata, patternformatdata, plugingroupdata, plugininfodata, wavedata, patterndata, attributeinfodata, parameterinfodata, plugindata, envelopedata, waveleveldata, patternorderdata, attributedata, connectiondata, midimappingdata, patternformattrackdata, pluginparameterdata, envelopepointdata, slicedata, eventconnectionbindingdata, patterneventdata, patternformatcolumndata
> database_tables;

struct barrierdata {
	int type;
	std::string description;
};

struct orderlist_timeshiftdata {
	int song_id;
	int index;
	int timeshift;
};

struct tableunion {
	union {
		songdata* song;
		patternformatdata* patternformat;
		plugingroupdata* plugingroup;
		plugininfodata* plugininfo;
		wavedata* wave;
		patterndata* pattern;
		attributeinfodata* attributeinfo;
		parameterinfodata* parameterinfo;
		plugindata* plugin;
		envelopedata* envelope;
		waveleveldata* wavelevel;
		patternorderdata* patternorder;
		attributedata* attribute;
		connectiondata* connection;
		midimappingdata* midimapping;
		patternformattrackdata* patternformattrack;
		pluginparameterdata* pluginparameter;
		envelopepointdata* envelopepoint;
		slicedata* slice;
		eventconnectionbindingdata* eventconnectionbinding;
		patterneventdata* patternevent;
		patternformatcolumndata* patternformatcolumn;
		barrierdata* barrier;
		orderlist_timeshiftdata* orderlist_timeshift;
	};

	void operator=(songdata& data) {
		song = &data;
	}

	void operator=(patternformatdata& data) {
		patternformat = &data;
	}

	void operator=(plugingroupdata& data) {
		plugingroup = &data;
	}

	void operator=(plugininfodata& data) {
		plugininfo = &data;
	}

	void operator=(wavedata& data) {
		wave = &data;
	}

	void operator=(patterndata& data) {
		pattern = &data;
	}

	void operator=(attributeinfodata& data) {
		attributeinfo = &data;
	}

	void operator=(parameterinfodata& data) {
		parameterinfo = &data;
	}

	void operator=(plugindata& data) {
		plugin = &data;
	}

	void operator=(envelopedata& data) {
		envelope = &data;
	}

	void operator=(waveleveldata& data) {
		wavelevel = &data;
	}

	void operator=(patternorderdata& data) {
		patternorder = &data;
	}

	void operator=(attributedata& data) {
		attribute = &data;
	}

	void operator=(connectiondata& data) {
		connection = &data;
	}

	void operator=(midimappingdata& data) {
		midimapping = &data;
	}

	void operator=(patternformattrackdata& data) {
		patternformattrack = &data;
	}

	void operator=(pluginparameterdata& data) {
		pluginparameter = &data;
	}

	void operator=(envelopepointdata& data) {
		envelopepoint = &data;
	}

	void operator=(slicedata& data) {
		slice = &data;
	}

	void operator=(eventconnectionbindingdata& data) {
		eventconnectionbinding = &data;
	}

	void operator=(patterneventdata& data) {
		patternevent = &data;
	}

	void operator=(patternformatcolumndata& data) {
		patternformatcolumn = &data;
	}
};

struct document_event_data {
	int type;
	int id;
	tableunion newdata;
	tableunion olddata;
};

