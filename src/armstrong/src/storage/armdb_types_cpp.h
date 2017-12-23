// (automatically generated)

extern "C" void song_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<songdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void patternformat_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<patternformatdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void plugingroup_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<plugingroupdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void plugininfo_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<plugininfodata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void wave_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<wavedata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void pattern_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<patterndata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void attributeinfo_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<attributeinfodata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void parameterinfo_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<parameterinfodata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void plugin_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<plugindata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void envelope_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<envelopedata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void wavelevel_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<waveleveldata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void patternorder_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<patternorderdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void attribute_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<attributedata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void connection_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<connectiondata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void midimapping_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<midimappingdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void patternformattrack_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<patternformattrackdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void pluginparameter_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<pluginparameterdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void envelopepoint_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<envelopepointdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void slice_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<slicedata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void eventconnectionbinding_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<eventconnectionbindingdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void patternevent_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<patterneventdata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

extern "C" void patternformatcolumn_notify_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	bool result = dbgenpp::table_notify_callback<patternformatcolumndata, document_event_data>(ctx, row);
	sqlite3_result_int(ctx, result?1:0);
}

void create_tables(std::ostream& query, const char* prefix) {
	query << "create table " << prefix << "song (id integer primary key, version integer, title varchar(0), comment blob, loopbegin integer, loopend integer, loopenabled integer, samplerate integer, bpm integer, tpb integer, swing real, swingticks integer, machineview_x real, machineview_y real);" << endl;
	query << "create table " << prefix << "patternformat (id integer primary key, song_id integer references song(id), name varchar(64), scroller_width integer);" << endl;
	query << "create index " << prefix << "patternformat_song_id_index ON patternformat(song_id);" << endl;
	query << "create table " << prefix << "plugingroup (id integer primary key, song_id integer references song(id), name varchar(256), parent_plugingroup_id integer references plugingroup(id), position_x real, position_y real);" << endl;
	query << "create index " << prefix << "plugingroup_song_id_index ON plugingroup(song_id);" << endl;
	query << "create index " << prefix << "plugingroup_plugingroup_id_index ON plugingroup(parent_plugingroup_id);" << endl;
	query << "create table " << prefix << "plugininfo (id integer primary key, song_id integer references song(id), flags integer, uri varchar(64), name varchar(64), short_name varchar(64), author varchar(64), mintracks integer, maxtracks integer, input_count integer, output_count integer);" << endl;
	query << "create index " << prefix << "plugininfo_song_id_index ON plugininfo(song_id);" << endl;
	query << "create table " << prefix << "wave (id integer primary key, song_id integer references song(id), name varchar(64), filename varchar(64), flags integer, volume real);" << endl;
	query << "create index " << prefix << "wave_song_id_index ON wave(song_id);" << endl;
	query << "create table " << prefix << "pattern (id integer primary key, song_id integer references song(id), name varchar(64), length integer, resolution integer, beginloop integer, endloop integer, loopenabled integer, display_resolution integer, display_verydark_row integer, display_dark_row integer, patternformat_id integer references patternformat(id), replay_row integer);" << endl;
	query << "create index " << prefix << "pattern_song_id_index ON pattern(song_id);" << endl;
	query << "create index " << prefix << "pattern_patternformat_id_index ON pattern(patternformat_id);" << endl;
	query << "create table " << prefix << "attributeinfo (id integer primary key, plugininfo_id integer references plugininfo(id), attrindex integer, name varchar(64), minvalue integer, maxvalue integer, defaultvalue integer);" << endl;
	query << "create index " << prefix << "attributeinfo_plugininfo_id_index ON attributeinfo(plugininfo_id);" << endl;
	query << "create table " << prefix << "parameterinfo (id integer primary key, plugininfo_id integer references plugininfo(id), paramgroup integer, paramcolumn integer, name varchar(64), description varchar(128), flags integer, type integer, minvalue integer, maxvalue integer, novalue integer, defaultvalue integer);" << endl;
	query << "create index " << prefix << "parameterinfo_plugininfo_id_index ON parameterinfo(plugininfo_id);" << endl;
	query << "create table " << prefix << "plugin (id integer primary key, flags integer, song_id integer references song(id), name varchar(64), data blob, trackcount integer, x real, y real, streamsource varchar(64), is_muted integer, is_bypassed integer, is_solo integer, is_minimized integer, plugininfo_id integer references plugininfo(id), timesource_plugin_id integer, timesource_group integer, timesource_track integer, latency integer, plugingroup_id integer references plugingroup(id));" << endl;
	query << "create index " << prefix << "plugin_song_id_index ON plugin(song_id);" << endl;
	query << "create index " << prefix << "plugin_plugininfo_id_index ON plugin(plugininfo_id);" << endl;
	query << "create index " << prefix << "plugin_plugingroup_id_index ON plugin(plugingroup_id);" << endl;
	query << "create table " << prefix << "envelope (id integer primary key, wave_id integer references wave(id), attack integer, decay integer, sustain integer, release integer, subdivision integer, flags integer, disabled integer);" << endl;
	query << "create index " << prefix << "envelope_wave_id_index ON envelope(wave_id);" << endl;
	query << "create table " << prefix << "wavelevel (id integer primary key, wave_id integer references wave(id), basenote integer, samplerate integer, samplecount integer, beginloop integer, endloop integer, format integer, filename varchar(64));" << endl;
	query << "create index " << prefix << "wavelevel_wave_id_index ON wavelevel(wave_id);" << endl;
	query << "create table " << prefix << "patternorder (id integer primary key, song_id integer references song(id), pattern_id integer references pattern(id));" << endl;
	query << "create index " << prefix << "patternorder_song_id_index ON patternorder(song_id);" << endl;
	query << "create index " << prefix << "patternorder_pattern_id_index ON patternorder(pattern_id);" << endl;
	query << "create table " << prefix << "attribute (id integer primary key, plugin_id integer references plugin(id), attrindex integer, value integer);" << endl;
	query << "create index " << prefix << "attribute_plugin_id_index ON attribute(plugin_id);" << endl;
	query << "create table " << prefix << "connection (id integer primary key, plugin_id integer references plugin(id), from_plugin_id integer, to_plugin_id integer, type integer, first_input integer, first_output integer, input_count integer, output_count integer, mididevice varchar(512));" << endl;
	query << "create index " << prefix << "connection_plugin_id_index ON connection(plugin_id);" << endl;
	query << "create table " << prefix << "midimapping (id integer primary key, plugin_id integer references plugin(id), paramgroup integer, paramtrack integer, paramcolumn integer, midichannel integer, midicontroller integer);" << endl;
	query << "create index " << prefix << "midimapping_plugin_id_index ON midimapping(plugin_id);" << endl;
	query << "create table " << prefix << "patternformattrack (id integer primary key, patternformat_id integer references patternformat(id), plugin_id integer references plugin(id), paramgroup integer, paramtrack integer, label varchar(256), is_muted integer);" << endl;
	query << "create index " << prefix << "patternformattrack_patternformat_id_index ON patternformattrack(patternformat_id);" << endl;
	query << "create index " << prefix << "patternformattrack_plugin_id_index ON patternformattrack(plugin_id);" << endl;
	query << "create table " << prefix << "pluginparameter (id integer primary key, plugin_id integer references plugin(id), parameterinfo_id integer references parameterinfo(id), paramtrack integer, value integer, interpolator integer);" << endl;
	query << "create index " << prefix << "pluginparameter_plugin_id_index ON pluginparameter(plugin_id);" << endl;
	query << "create index " << prefix << "pluginparameter_parameterinfo_id_index ON pluginparameter(parameterinfo_id);" << endl;
	query << "create table " << prefix << "envelopepoint (id integer primary key, envelope_id integer references envelope(id), x integer, y integer, flags integer);" << endl;
	query << "create index " << prefix << "envelopepoint_envelope_id_index ON envelopepoint(envelope_id);" << endl;
	query << "create table " << prefix << "slice (id integer primary key, wavelevel_id integer references wavelevel(id), sampleoffset integer);" << endl;
	query << "create index " << prefix << "slice_wavelevel_id_index ON slice(wavelevel_id);" << endl;
	query << "create table " << prefix << "eventconnectionbinding (id integer primary key, connection_id integer references connection(id), sourceindex integer, targetparamgroup integer, targetparamtrack integer, targetparamcolumn integer);" << endl;
	query << "create index " << prefix << "eventconnectionbinding_connection_id_index ON eventconnectionbinding(connection_id);" << endl;
	query << "create table " << prefix << "patternevent (id integer primary key, pattern_id integer references pattern(id), time integer, pluginparameter_id integer references pluginparameter(id), value integer, meta integer);" << endl;
	query << "create index " << prefix << "patternevent_pattern_id_index ON patternevent(pattern_id);" << endl;
	query << "create index " << prefix << "patternevent_pluginparameter_id_index ON patternevent(pluginparameter_id);" << endl;
	query << "create table " << prefix << "patternformatcolumn (id integer primary key, patternformat_id integer references patternformat(id), pluginparameter_id integer references pluginparameter(id), mode integer, is_collapsed integer, idx integer);" << endl;
	query << "create index " << prefix << "patternformatcolumn_patternformat_id_index ON patternformatcolumn(patternformat_id);" << endl;
	query << "create index " << prefix << "patternformatcolumn_pluginparameter_id_index ON patternformatcolumn(pluginparameter_id);" << endl;
}

void create_callbacks(sqlite3* db, void* self) {
	sqlite3_create_function(db, "song_notify_callback", -1, SQLITE_ANY, self, song_notify_callback, 0, 0);
	sqlite3_create_function(db, "patternformat_notify_callback", -1, SQLITE_ANY, self, patternformat_notify_callback, 0, 0);
	sqlite3_create_function(db, "plugingroup_notify_callback", -1, SQLITE_ANY, self, plugingroup_notify_callback, 0, 0);
	sqlite3_create_function(db, "plugininfo_notify_callback", -1, SQLITE_ANY, self, plugininfo_notify_callback, 0, 0);
	sqlite3_create_function(db, "wave_notify_callback", -1, SQLITE_ANY, self, wave_notify_callback, 0, 0);
	sqlite3_create_function(db, "pattern_notify_callback", -1, SQLITE_ANY, self, pattern_notify_callback, 0, 0);
	sqlite3_create_function(db, "attributeinfo_notify_callback", -1, SQLITE_ANY, self, attributeinfo_notify_callback, 0, 0);
	sqlite3_create_function(db, "parameterinfo_notify_callback", -1, SQLITE_ANY, self, parameterinfo_notify_callback, 0, 0);
	sqlite3_create_function(db, "plugin_notify_callback", -1, SQLITE_ANY, self, plugin_notify_callback, 0, 0);
	sqlite3_create_function(db, "envelope_notify_callback", -1, SQLITE_ANY, self, envelope_notify_callback, 0, 0);
	sqlite3_create_function(db, "wavelevel_notify_callback", -1, SQLITE_ANY, self, wavelevel_notify_callback, 0, 0);
	sqlite3_create_function(db, "patternorder_notify_callback", -1, SQLITE_ANY, self, patternorder_notify_callback, 0, 0);
	sqlite3_create_function(db, "attribute_notify_callback", -1, SQLITE_ANY, self, attribute_notify_callback, 0, 0);
	sqlite3_create_function(db, "connection_notify_callback", -1, SQLITE_ANY, self, connection_notify_callback, 0, 0);
	sqlite3_create_function(db, "midimapping_notify_callback", -1, SQLITE_ANY, self, midimapping_notify_callback, 0, 0);
	sqlite3_create_function(db, "patternformattrack_notify_callback", -1, SQLITE_ANY, self, patternformattrack_notify_callback, 0, 0);
	sqlite3_create_function(db, "pluginparameter_notify_callback", -1, SQLITE_ANY, self, pluginparameter_notify_callback, 0, 0);
	sqlite3_create_function(db, "envelopepoint_notify_callback", -1, SQLITE_ANY, self, envelopepoint_notify_callback, 0, 0);
	sqlite3_create_function(db, "slice_notify_callback", -1, SQLITE_ANY, self, slice_notify_callback, 0, 0);
	sqlite3_create_function(db, "eventconnectionbinding_notify_callback", -1, SQLITE_ANY, self, eventconnectionbinding_notify_callback, 0, 0);
	sqlite3_create_function(db, "patternevent_notify_callback", -1, SQLITE_ANY, self, patternevent_notify_callback, 0, 0);
	sqlite3_create_function(db, "patternformatcolumn_notify_callback", -1, SQLITE_ANY, self, patternformatcolumn_notify_callback, 0, 0);
}

void create_triggers(sqlite3* db, std::ostream& query) {
	// song:
	query << "create temp trigger song_insert_notify_trigger after insert on song begin" << endl;
	query << "select undoredo_add_query('delete from song where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where song_notify_callback(0, new.id, new.version, new.title, new.comment, new.loopbegin, new.loopend, new.loopenabled, new.samplerate, new.bpm, new.tpb, new.swing, new.swingticks, new.machineview_x, new.machineview_y) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger song_delete_notify_trigger before delete on song begin" << endl;
	query << "delete from patternformat where song_id = old.id;" << endl;
	query << "delete from plugingroup where song_id = old.id;" << endl;
	query << "delete from plugininfo where song_id = old.id;" << endl;
	query << "delete from wave where song_id = old.id;" << endl;
	query << "delete from pattern where song_id = old.id;" << endl;
	query << "delete from plugin where song_id = old.id;" << endl;
	query << "delete from patternorder where song_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into song values('||quote(old.id)||', '||quote(old.version)||', '||quote(old.title)||', '||quote(old.comment)||', '||quote(old.loopbegin)||', '||quote(old.loopend)||', '||quote(old.loopenabled)||', '||quote(old.samplerate)||', '||quote(old.bpm)||', '||quote(old.tpb)||', '||quote(old.swing)||', '||quote(old.swingticks)||', '||quote(old.machineview_x)||', '||quote(old.machineview_y)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger song_after_delete_trigger after delete on song begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where song_notify_callback(1, old.id, old.version, old.title, old.comment, old.loopbegin, old.loopend, old.loopenabled, old.samplerate, old.bpm, old.tpb, old.swing, old.swingticks, old.machineview_x, old.machineview_y) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger song_update_notify_trigger after update on song begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where song_notify_callback(2, new.id, new.version, new.title, new.comment, new.loopbegin, new.loopend, new.loopenabled, new.samplerate, new.bpm, new.tpb, new.swing, new.swingticks, new.machineview_x, new.machineview_y, old.id, old.version, old.title, old.comment, old.loopbegin, old.loopend, old.loopenabled, old.samplerate, old.bpm, old.tpb, old.swing, old.swingticks, old.machineview_x, old.machineview_y) = 0;" << endl;
	query << "select undoredo_add_query('update song set id = '||quote(old.id)||', version = '||quote(old.version)||', title = '||quote(old.title)||', comment = '||quote(old.comment)||', loopbegin = '||quote(old.loopbegin)||', loopend = '||quote(old.loopend)||', loopenabled = '||quote(old.loopenabled)||', samplerate = '||quote(old.samplerate)||', bpm = '||quote(old.bpm)||', tpb = '||quote(old.tpb)||', swing = '||quote(old.swing)||', swingticks = '||quote(old.swingticks)||', machineview_x = '||quote(old.machineview_x)||', machineview_y = '||quote(old.machineview_y)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// patternformat:
	query << "create temp trigger patternformat_insert_notify_trigger after insert on patternformat begin" << endl;
	query << "select undoredo_add_query('delete from patternformat where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where patternformat_notify_callback(0, new.id, new.song_id, new.name, new.scroller_width) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternformat_delete_notify_trigger before delete on patternformat begin" << endl;
	query << "delete from pattern where patternformat_id = old.id;" << endl;
	query << "delete from patternformattrack where patternformat_id = old.id;" << endl;
	query << "delete from patternformatcolumn where patternformat_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into patternformat values('||quote(old.id)||', '||quote(old.song_id)||', '||quote(old.name)||', '||quote(old.scroller_width)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternformat_after_delete_trigger after delete on patternformat begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where patternformat_notify_callback(1, old.id, old.song_id, old.name, old.scroller_width) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternformat_update_notify_trigger after update on patternformat begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where patternformat_notify_callback(2, new.id, new.song_id, new.name, new.scroller_width, old.id, old.song_id, old.name, old.scroller_width) = 0;" << endl;
	query << "select undoredo_add_query('update patternformat set id = '||quote(old.id)||', song_id = '||quote(old.song_id)||', name = '||quote(old.name)||', scroller_width = '||quote(old.scroller_width)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// plugingroup:
	query << "create temp trigger plugingroup_insert_notify_trigger after insert on plugingroup begin" << endl;
	query << "select undoredo_add_query('delete from plugingroup where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where plugingroup_notify_callback(0, new.id, new.song_id, new.name, new.parent_plugingroup_id, new.position_x, new.position_y) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugingroup_delete_notify_trigger before delete on plugingroup begin" << endl;
	query << "delete from plugingroup where parent_plugingroup_id = old.id;" << endl;
	query << "delete from plugin where plugingroup_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into plugingroup values('||quote(old.id)||', '||quote(old.song_id)||', '||quote(old.name)||', '||quote(old.parent_plugingroup_id)||', '||quote(old.position_x)||', '||quote(old.position_y)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugingroup_after_delete_trigger after delete on plugingroup begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where plugingroup_notify_callback(1, old.id, old.song_id, old.name, old.parent_plugingroup_id, old.position_x, old.position_y) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugingroup_update_notify_trigger after update on plugingroup begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where plugingroup_notify_callback(2, new.id, new.song_id, new.name, new.parent_plugingroup_id, new.position_x, new.position_y, old.id, old.song_id, old.name, old.parent_plugingroup_id, old.position_x, old.position_y) = 0;" << endl;
	query << "select undoredo_add_query('update plugingroup set id = '||quote(old.id)||', song_id = '||quote(old.song_id)||', name = '||quote(old.name)||', parent_plugingroup_id = '||quote(old.parent_plugingroup_id)||', position_x = '||quote(old.position_x)||', position_y = '||quote(old.position_y)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// plugininfo:
	query << "create temp trigger plugininfo_insert_notify_trigger after insert on plugininfo begin" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where plugininfo_notify_callback(0, new.id, new.song_id, new.flags, new.uri, new.name, new.short_name, new.author, new.mintracks, new.maxtracks, new.input_count, new.output_count) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugininfo_delete_notify_trigger before delete on plugininfo begin" << endl;
	query << "delete from attributeinfo where plugininfo_id = old.id;" << endl;
	query << "delete from parameterinfo where plugininfo_id = old.id;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugininfo_after_delete_trigger after delete on plugininfo begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where plugininfo_notify_callback(1, old.id, old.song_id, old.flags, old.uri, old.name, old.short_name, old.author, old.mintracks, old.maxtracks, old.input_count, old.output_count) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugininfo_update_notify_trigger after update on plugininfo begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where plugininfo_notify_callback(2, new.id, new.song_id, new.flags, new.uri, new.name, new.short_name, new.author, new.mintracks, new.maxtracks, new.input_count, new.output_count, old.id, old.song_id, old.flags, old.uri, old.name, old.short_name, old.author, old.mintracks, old.maxtracks, old.input_count, old.output_count) = 0;" << endl;
	query << "end;" << endl;

	// wave:
	query << "create temp trigger wave_insert_notify_trigger after insert on wave begin" << endl;
	query << "select undoredo_add_query('delete from wave where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where wave_notify_callback(0, new.id, new.song_id, new.name, new.filename, new.flags, new.volume) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger wave_delete_notify_trigger before delete on wave begin" << endl;
	query << "delete from envelope where wave_id = old.id;" << endl;
	query << "delete from wavelevel where wave_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into wave values('||quote(old.id)||', '||quote(old.song_id)||', '||quote(old.name)||', '||quote(old.filename)||', '||quote(old.flags)||', '||quote(old.volume)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger wave_after_delete_trigger after delete on wave begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where wave_notify_callback(1, old.id, old.song_id, old.name, old.filename, old.flags, old.volume) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger wave_update_notify_trigger after update on wave begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where wave_notify_callback(2, new.id, new.song_id, new.name, new.filename, new.flags, new.volume, old.id, old.song_id, old.name, old.filename, old.flags, old.volume) = 0;" << endl;
	query << "select undoredo_add_query('update wave set id = '||quote(old.id)||', song_id = '||quote(old.song_id)||', name = '||quote(old.name)||', filename = '||quote(old.filename)||', flags = '||quote(old.flags)||', volume = '||quote(old.volume)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// pattern:
	query << "create temp trigger pattern_insert_notify_trigger after insert on pattern begin" << endl;
	query << "select undoredo_add_query('delete from pattern where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where pattern_notify_callback(0, new.id, new.song_id, new.name, new.length, new.resolution, new.beginloop, new.endloop, new.loopenabled, new.display_resolution, new.display_verydark_row, new.display_dark_row, new.patternformat_id, new.replay_row) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger pattern_delete_notify_trigger before delete on pattern begin" << endl;
	query << "select raise(abort, 'before delete failed from callback constraint') where pattern_notify_callback(11, old.id, old.song_id, old.name, old.length, old.resolution, old.beginloop, old.endloop, old.loopenabled, old.display_resolution, old.display_verydark_row, old.display_dark_row, old.patternformat_id, old.replay_row) = 0;" << endl;
	query << "delete from patternorder where pattern_id = old.id;" << endl;
	query << "delete from patternevent where pattern_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into pattern values('||quote(old.id)||', '||quote(old.song_id)||', '||quote(old.name)||', '||quote(old.length)||', '||quote(old.resolution)||', '||quote(old.beginloop)||', '||quote(old.endloop)||', '||quote(old.loopenabled)||', '||quote(old.display_resolution)||', '||quote(old.display_verydark_row)||', '||quote(old.display_dark_row)||', '||quote(old.patternformat_id)||', '||quote(old.replay_row)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger pattern_after_delete_trigger after delete on pattern begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where pattern_notify_callback(1, old.id, old.song_id, old.name, old.length, old.resolution, old.beginloop, old.endloop, old.loopenabled, old.display_resolution, old.display_verydark_row, old.display_dark_row, old.patternformat_id, old.replay_row) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger pattern_before_update_notify_trigger before update on pattern begin" << endl;
	query << "select raise(abort, 'before update failed from callback constraint') where pattern_notify_callback(12, new.id, new.song_id, new.name, new.length, new.resolution, new.beginloop, new.endloop, new.loopenabled, new.display_resolution, new.display_verydark_row, new.display_dark_row, new.patternformat_id, new.replay_row) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger pattern_update_notify_trigger after update on pattern begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where pattern_notify_callback(2, new.id, new.song_id, new.name, new.length, new.resolution, new.beginloop, new.endloop, new.loopenabled, new.display_resolution, new.display_verydark_row, new.display_dark_row, new.patternformat_id, new.replay_row, old.id, old.song_id, old.name, old.length, old.resolution, old.beginloop, old.endloop, old.loopenabled, old.display_resolution, old.display_verydark_row, old.display_dark_row, old.patternformat_id, old.replay_row) = 0;" << endl;
	query << "select undoredo_add_query('update pattern set id = '||quote(old.id)||', song_id = '||quote(old.song_id)||', name = '||quote(old.name)||', length = '||quote(old.length)||', resolution = '||quote(old.resolution)||', beginloop = '||quote(old.beginloop)||', endloop = '||quote(old.endloop)||', loopenabled = '||quote(old.loopenabled)||', display_resolution = '||quote(old.display_resolution)||', display_verydark_row = '||quote(old.display_verydark_row)||', display_dark_row = '||quote(old.display_dark_row)||', patternformat_id = '||quote(old.patternformat_id)||', replay_row = '||quote(old.replay_row)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// attributeinfo:
	query << "create temp trigger attributeinfo_insert_notify_trigger after insert on attributeinfo begin" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where attributeinfo_notify_callback(0, new.id, new.plugininfo_id, new.attrindex, new.name, new.minvalue, new.maxvalue, new.defaultvalue) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger attributeinfo_after_delete_trigger after delete on attributeinfo begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where attributeinfo_notify_callback(1, old.id, old.plugininfo_id, old.attrindex, old.name, old.minvalue, old.maxvalue, old.defaultvalue) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger attributeinfo_update_notify_trigger after update on attributeinfo begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where attributeinfo_notify_callback(2, new.id, new.plugininfo_id, new.attrindex, new.name, new.minvalue, new.maxvalue, new.defaultvalue, old.id, old.plugininfo_id, old.attrindex, old.name, old.minvalue, old.maxvalue, old.defaultvalue) = 0;" << endl;
	query << "end;" << endl;

	// parameterinfo:
	query << "create temp trigger parameterinfo_insert_notify_trigger after insert on parameterinfo begin" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where parameterinfo_notify_callback(0, new.id, new.plugininfo_id, new.paramgroup, new.paramcolumn, new.name, new.description, new.flags, new.type, new.minvalue, new.maxvalue, new.novalue, new.defaultvalue) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger parameterinfo_delete_notify_trigger before delete on parameterinfo begin" << endl;
	query << "delete from pluginparameter where parameterinfo_id = old.id;" << endl;
	query << "end;" << endl;

	query << "create temp trigger parameterinfo_after_delete_trigger after delete on parameterinfo begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where parameterinfo_notify_callback(1, old.id, old.plugininfo_id, old.paramgroup, old.paramcolumn, old.name, old.description, old.flags, old.type, old.minvalue, old.maxvalue, old.novalue, old.defaultvalue) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger parameterinfo_update_notify_trigger after update on parameterinfo begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where parameterinfo_notify_callback(2, new.id, new.plugininfo_id, new.paramgroup, new.paramcolumn, new.name, new.description, new.flags, new.type, new.minvalue, new.maxvalue, new.novalue, new.defaultvalue, old.id, old.plugininfo_id, old.paramgroup, old.paramcolumn, old.name, old.description, old.flags, old.type, old.minvalue, old.maxvalue, old.novalue, old.defaultvalue) = 0;" << endl;
	query << "end;" << endl;

	// plugin:
	query << "create temp trigger plugin_before_insert_trigger before insert on plugin begin" << endl;
	query << "select raise(abort, 'before insert failed from callback constraint') where plugin_notify_callback(10, new.id, new.flags, new.song_id, new.name, new.data, new.trackcount, new.x, new.y, new.streamsource, new.is_muted, new.is_bypassed, new.is_solo, new.is_minimized, new.plugininfo_id, new.timesource_plugin_id, new.timesource_group, new.timesource_track, new.latency, new.plugingroup_id) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugin_insert_notify_trigger after insert on plugin begin" << endl;
	query << "select undoredo_add_query('delete from plugin where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where plugin_notify_callback(0, new.id, new.flags, new.song_id, new.name, new.data, new.trackcount, new.x, new.y, new.streamsource, new.is_muted, new.is_bypassed, new.is_solo, new.is_minimized, new.plugininfo_id, new.timesource_plugin_id, new.timesource_group, new.timesource_track, new.latency, new.plugingroup_id) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugin_delete_notify_trigger before delete on plugin begin" << endl;
	query << "select raise(abort, 'before delete failed from callback constraint') where plugin_notify_callback(11, old.id, old.flags, old.song_id, old.name, old.data, old.trackcount, old.x, old.y, old.streamsource, old.is_muted, old.is_bypassed, old.is_solo, old.is_minimized, old.plugininfo_id, old.timesource_plugin_id, old.timesource_group, old.timesource_track, old.latency, old.plugingroup_id) = 0;" << endl;
	query << "delete from attribute where plugin_id = old.id;" << endl;
	query << "delete from connection where plugin_id = old.id;" << endl;
	query << "delete from midimapping where plugin_id = old.id;" << endl;
	query << "delete from patternformattrack where plugin_id = old.id;" << endl;
	query << "delete from pluginparameter where plugin_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into plugin values('||quote(old.id)||', '||quote(old.flags)||', '||quote(old.song_id)||', '||quote(old.name)||', '||quote(old.data)||', '||quote(old.trackcount)||', '||quote(old.x)||', '||quote(old.y)||', '||quote(old.streamsource)||', '||quote(old.is_muted)||', '||quote(old.is_bypassed)||', '||quote(old.is_solo)||', '||quote(old.is_minimized)||', '||quote(old.plugininfo_id)||', '||quote(old.timesource_plugin_id)||', '||quote(old.timesource_group)||', '||quote(old.timesource_track)||', '||quote(old.latency)||', '||quote(old.plugingroup_id)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugin_after_delete_trigger after delete on plugin begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where plugin_notify_callback(1, old.id, old.flags, old.song_id, old.name, old.data, old.trackcount, old.x, old.y, old.streamsource, old.is_muted, old.is_bypassed, old.is_solo, old.is_minimized, old.plugininfo_id, old.timesource_plugin_id, old.timesource_group, old.timesource_track, old.latency, old.plugingroup_id) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugin_before_update_notify_trigger before update on plugin begin" << endl;
	query << "select raise(abort, 'before update failed from callback constraint') where plugin_notify_callback(12, new.id, new.flags, new.song_id, new.name, new.data, new.trackcount, new.x, new.y, new.streamsource, new.is_muted, new.is_bypassed, new.is_solo, new.is_minimized, new.plugininfo_id, new.timesource_plugin_id, new.timesource_group, new.timesource_track, new.latency, new.plugingroup_id) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger plugin_update_notify_trigger after update on plugin begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where plugin_notify_callback(2, new.id, new.flags, new.song_id, new.name, new.data, new.trackcount, new.x, new.y, new.streamsource, new.is_muted, new.is_bypassed, new.is_solo, new.is_minimized, new.plugininfo_id, new.timesource_plugin_id, new.timesource_group, new.timesource_track, new.latency, new.plugingroup_id, old.id, old.flags, old.song_id, old.name, old.data, old.trackcount, old.x, old.y, old.streamsource, old.is_muted, old.is_bypassed, old.is_solo, old.is_minimized, old.plugininfo_id, old.timesource_plugin_id, old.timesource_group, old.timesource_track, old.latency, old.plugingroup_id) = 0;" << endl;
	query << "select undoredo_add_query('update plugin set id = '||quote(old.id)||', flags = '||quote(old.flags)||', song_id = '||quote(old.song_id)||', name = '||quote(old.name)||', data = '||quote(old.data)||', trackcount = '||quote(old.trackcount)||', x = '||quote(old.x)||', y = '||quote(old.y)||', streamsource = '||quote(old.streamsource)||', is_muted = '||quote(old.is_muted)||', is_bypassed = '||quote(old.is_bypassed)||', is_solo = '||quote(old.is_solo)||', is_minimized = '||quote(old.is_minimized)||', plugininfo_id = '||quote(old.plugininfo_id)||', timesource_plugin_id = '||quote(old.timesource_plugin_id)||', timesource_group = '||quote(old.timesource_group)||', timesource_track = '||quote(old.timesource_track)||', latency = '||quote(old.latency)||', plugingroup_id = '||quote(old.plugingroup_id)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// envelope:
	query << "create temp trigger envelope_insert_notify_trigger after insert on envelope begin" << endl;
	query << "select undoredo_add_query('delete from envelope where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where envelope_notify_callback(0, new.id, new.wave_id, new.attack, new.decay, new.sustain, new.release, new.subdivision, new.flags, new.disabled) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger envelope_delete_notify_trigger before delete on envelope begin" << endl;
	query << "delete from envelopepoint where envelope_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into envelope values('||quote(old.id)||', '||quote(old.wave_id)||', '||quote(old.attack)||', '||quote(old.decay)||', '||quote(old.sustain)||', '||quote(old.release)||', '||quote(old.subdivision)||', '||quote(old.flags)||', '||quote(old.disabled)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger envelope_after_delete_trigger after delete on envelope begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where envelope_notify_callback(1, old.id, old.wave_id, old.attack, old.decay, old.sustain, old.release, old.subdivision, old.flags, old.disabled) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger envelope_update_notify_trigger after update on envelope begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where envelope_notify_callback(2, new.id, new.wave_id, new.attack, new.decay, new.sustain, new.release, new.subdivision, new.flags, new.disabled, old.id, old.wave_id, old.attack, old.decay, old.sustain, old.release, old.subdivision, old.flags, old.disabled) = 0;" << endl;
	query << "select undoredo_add_query('update envelope set id = '||quote(old.id)||', wave_id = '||quote(old.wave_id)||', attack = '||quote(old.attack)||', decay = '||quote(old.decay)||', sustain = '||quote(old.sustain)||', release = '||quote(old.release)||', subdivision = '||quote(old.subdivision)||', flags = '||quote(old.flags)||', disabled = '||quote(old.disabled)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// wavelevel:
	query << "create temp trigger wavelevel_insert_notify_trigger after insert on wavelevel begin" << endl;
	query << "select undoredo_add_query('delete from wavelevel where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where wavelevel_notify_callback(0, new.id, new.wave_id, new.basenote, new.samplerate, new.samplecount, new.beginloop, new.endloop, new.format, new.filename) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger wavelevel_delete_notify_trigger before delete on wavelevel begin" << endl;
	query << "select raise(abort, 'before delete failed from callback constraint') where wavelevel_notify_callback(11, old.id, old.wave_id, old.basenote, old.samplerate, old.samplecount, old.beginloop, old.endloop, old.format, old.filename) = 0;" << endl;
	query << "delete from slice where wavelevel_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into wavelevel values('||quote(old.id)||', '||quote(old.wave_id)||', '||quote(old.basenote)||', '||quote(old.samplerate)||', '||quote(old.samplecount)||', '||quote(old.beginloop)||', '||quote(old.endloop)||', '||quote(old.format)||', '||quote(old.filename)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger wavelevel_after_delete_trigger after delete on wavelevel begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where wavelevel_notify_callback(1, old.id, old.wave_id, old.basenote, old.samplerate, old.samplecount, old.beginloop, old.endloop, old.format, old.filename) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger wavelevel_update_notify_trigger after update on wavelevel begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where wavelevel_notify_callback(2, new.id, new.wave_id, new.basenote, new.samplerate, new.samplecount, new.beginloop, new.endloop, new.format, new.filename, old.id, old.wave_id, old.basenote, old.samplerate, old.samplecount, old.beginloop, old.endloop, old.format, old.filename) = 0;" << endl;
	query << "select undoredo_add_query('update wavelevel set id = '||quote(old.id)||', wave_id = '||quote(old.wave_id)||', basenote = '||quote(old.basenote)||', samplerate = '||quote(old.samplerate)||', samplecount = '||quote(old.samplecount)||', beginloop = '||quote(old.beginloop)||', endloop = '||quote(old.endloop)||', format = '||quote(old.format)||', filename = '||quote(old.filename)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// patternorder:
	query << "create temp trigger patternorder_insert_notify_trigger after insert on patternorder begin" << endl;
	query << "select undoredo_add_query('delete from patternorder where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where patternorder_notify_callback(0, new.id, new.song_id, new.pattern_id) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternorder_delete_notify_trigger before delete on patternorder begin" << endl;
	query << "select undoredo_add_query('insert into patternorder values('||quote(old.id)||', '||quote(old.song_id)||', '||quote(old.pattern_id)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternorder_after_delete_trigger after delete on patternorder begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where patternorder_notify_callback(1, old.id, old.song_id, old.pattern_id) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternorder_update_notify_trigger after update on patternorder begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where patternorder_notify_callback(2, new.id, new.song_id, new.pattern_id, old.id, old.song_id, old.pattern_id) = 0;" << endl;
	query << "select undoredo_add_query('update patternorder set id = '||quote(old.id)||', song_id = '||quote(old.song_id)||', pattern_id = '||quote(old.pattern_id)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// attribute:
	query << "create temp trigger attribute_insert_notify_trigger after insert on attribute begin" << endl;
	query << "select undoredo_add_query('delete from attribute where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where attribute_notify_callback(0, new.id, new.plugin_id, new.attrindex, new.value) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger attribute_delete_notify_trigger before delete on attribute begin" << endl;
	query << "select undoredo_add_query('insert into attribute values('||quote(old.id)||', '||quote(old.plugin_id)||', '||quote(old.attrindex)||', '||quote(old.value)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger attribute_after_delete_trigger after delete on attribute begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where attribute_notify_callback(1, old.id, old.plugin_id, old.attrindex, old.value) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger attribute_update_notify_trigger after update on attribute begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where attribute_notify_callback(2, new.id, new.plugin_id, new.attrindex, new.value, old.id, old.plugin_id, old.attrindex, old.value) = 0;" << endl;
	query << "select undoredo_add_query('update attribute set id = '||quote(old.id)||', plugin_id = '||quote(old.plugin_id)||', attrindex = '||quote(old.attrindex)||', value = '||quote(old.value)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// connection:
	query << "create temp trigger connection_insert_notify_trigger after insert on connection begin" << endl;
	query << "select undoredo_add_query('delete from connection where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where connection_notify_callback(0, new.id, new.plugin_id, new.from_plugin_id, new.to_plugin_id, new.type, new.first_input, new.first_output, new.input_count, new.output_count, new.mididevice) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger connection_delete_notify_trigger before delete on connection begin" << endl;
	query << "delete from eventconnectionbinding where connection_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into connection values('||quote(old.id)||', '||quote(old.plugin_id)||', '||quote(old.from_plugin_id)||', '||quote(old.to_plugin_id)||', '||quote(old.type)||', '||quote(old.first_input)||', '||quote(old.first_output)||', '||quote(old.input_count)||', '||quote(old.output_count)||', '||quote(old.mididevice)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger connection_after_delete_trigger after delete on connection begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where connection_notify_callback(1, old.id, old.plugin_id, old.from_plugin_id, old.to_plugin_id, old.type, old.first_input, old.first_output, old.input_count, old.output_count, old.mididevice) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger connection_update_notify_trigger after update on connection begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where connection_notify_callback(2, new.id, new.plugin_id, new.from_plugin_id, new.to_plugin_id, new.type, new.first_input, new.first_output, new.input_count, new.output_count, new.mididevice, old.id, old.plugin_id, old.from_plugin_id, old.to_plugin_id, old.type, old.first_input, old.first_output, old.input_count, old.output_count, old.mididevice) = 0;" << endl;
	query << "select undoredo_add_query('update connection set id = '||quote(old.id)||', plugin_id = '||quote(old.plugin_id)||', from_plugin_id = '||quote(old.from_plugin_id)||', to_plugin_id = '||quote(old.to_plugin_id)||', type = '||quote(old.type)||', first_input = '||quote(old.first_input)||', first_output = '||quote(old.first_output)||', input_count = '||quote(old.input_count)||', output_count = '||quote(old.output_count)||', mididevice = '||quote(old.mididevice)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// midimapping:
	query << "create temp trigger midimapping_insert_notify_trigger after insert on midimapping begin" << endl;
	query << "select undoredo_add_query('delete from midimapping where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where midimapping_notify_callback(0, new.id, new.plugin_id, new.paramgroup, new.paramtrack, new.paramcolumn, new.midichannel, new.midicontroller) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger midimapping_delete_notify_trigger before delete on midimapping begin" << endl;
	query << "select undoredo_add_query('insert into midimapping values('||quote(old.id)||', '||quote(old.plugin_id)||', '||quote(old.paramgroup)||', '||quote(old.paramtrack)||', '||quote(old.paramcolumn)||', '||quote(old.midichannel)||', '||quote(old.midicontroller)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger midimapping_after_delete_trigger after delete on midimapping begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where midimapping_notify_callback(1, old.id, old.plugin_id, old.paramgroup, old.paramtrack, old.paramcolumn, old.midichannel, old.midicontroller) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger midimapping_update_notify_trigger after update on midimapping begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where midimapping_notify_callback(2, new.id, new.plugin_id, new.paramgroup, new.paramtrack, new.paramcolumn, new.midichannel, new.midicontroller, old.id, old.plugin_id, old.paramgroup, old.paramtrack, old.paramcolumn, old.midichannel, old.midicontroller) = 0;" << endl;
	query << "select undoredo_add_query('update midimapping set id = '||quote(old.id)||', plugin_id = '||quote(old.plugin_id)||', paramgroup = '||quote(old.paramgroup)||', paramtrack = '||quote(old.paramtrack)||', paramcolumn = '||quote(old.paramcolumn)||', midichannel = '||quote(old.midichannel)||', midicontroller = '||quote(old.midicontroller)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// patternformattrack:
	query << "create temp trigger patternformattrack_insert_notify_trigger after insert on patternformattrack begin" << endl;
	query << "select undoredo_add_query('delete from patternformattrack where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where patternformattrack_notify_callback(0, new.id, new.patternformat_id, new.plugin_id, new.paramgroup, new.paramtrack, new.label, new.is_muted) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternformattrack_delete_notify_trigger before delete on patternformattrack begin" << endl;
	query << "select undoredo_add_query('insert into patternformattrack values('||quote(old.id)||', '||quote(old.patternformat_id)||', '||quote(old.plugin_id)||', '||quote(old.paramgroup)||', '||quote(old.paramtrack)||', '||quote(old.label)||', '||quote(old.is_muted)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternformattrack_after_delete_trigger after delete on patternformattrack begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where patternformattrack_notify_callback(1, old.id, old.patternformat_id, old.plugin_id, old.paramgroup, old.paramtrack, old.label, old.is_muted) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternformattrack_update_notify_trigger after update on patternformattrack begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where patternformattrack_notify_callback(2, new.id, new.patternformat_id, new.plugin_id, new.paramgroup, new.paramtrack, new.label, new.is_muted, old.id, old.patternformat_id, old.plugin_id, old.paramgroup, old.paramtrack, old.label, old.is_muted) = 0;" << endl;
	query << "select undoredo_add_query('update patternformattrack set id = '||quote(old.id)||', patternformat_id = '||quote(old.patternformat_id)||', plugin_id = '||quote(old.plugin_id)||', paramgroup = '||quote(old.paramgroup)||', paramtrack = '||quote(old.paramtrack)||', label = '||quote(old.label)||', is_muted = '||quote(old.is_muted)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// pluginparameter:
	query << "create temp trigger pluginparameter_insert_notify_trigger after insert on pluginparameter begin" << endl;
	query << "select undoredo_add_query('delete from pluginparameter where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where pluginparameter_notify_callback(0, new.id, new.plugin_id, new.parameterinfo_id, new.paramtrack, new.value, new.interpolator) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger pluginparameter_delete_notify_trigger before delete on pluginparameter begin" << endl;
	query << "delete from patternevent where pluginparameter_id = old.id;" << endl;
	query << "delete from patternformatcolumn where pluginparameter_id = old.id;" << endl;
	query << "select undoredo_add_query('insert into pluginparameter values('||quote(old.id)||', '||quote(old.plugin_id)||', '||quote(old.parameterinfo_id)||', '||quote(old.paramtrack)||', '||quote(old.value)||', '||quote(old.interpolator)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger pluginparameter_after_delete_trigger after delete on pluginparameter begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where pluginparameter_notify_callback(1, old.id, old.plugin_id, old.parameterinfo_id, old.paramtrack, old.value, old.interpolator) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger pluginparameter_update_notify_trigger after update on pluginparameter begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where pluginparameter_notify_callback(2, new.id, new.plugin_id, new.parameterinfo_id, new.paramtrack, new.value, new.interpolator, old.id, old.plugin_id, old.parameterinfo_id, old.paramtrack, old.value, old.interpolator) = 0;" << endl;
	query << "select undoredo_add_query('update pluginparameter set id = '||quote(old.id)||', plugin_id = '||quote(old.plugin_id)||', parameterinfo_id = '||quote(old.parameterinfo_id)||', paramtrack = '||quote(old.paramtrack)||', value = '||quote(old.value)||', interpolator = '||quote(old.interpolator)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// envelopepoint:
	query << "create temp trigger envelopepoint_insert_notify_trigger after insert on envelopepoint begin" << endl;
	query << "select undoredo_add_query('delete from envelopepoint where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where envelopepoint_notify_callback(0, new.id, new.envelope_id, new.x, new.y, new.flags) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger envelopepoint_delete_notify_trigger before delete on envelopepoint begin" << endl;
	query << "select undoredo_add_query('insert into envelopepoint values('||quote(old.id)||', '||quote(old.envelope_id)||', '||quote(old.x)||', '||quote(old.y)||', '||quote(old.flags)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger envelopepoint_after_delete_trigger after delete on envelopepoint begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where envelopepoint_notify_callback(1, old.id, old.envelope_id, old.x, old.y, old.flags) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger envelopepoint_update_notify_trigger after update on envelopepoint begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where envelopepoint_notify_callback(2, new.id, new.envelope_id, new.x, new.y, new.flags, old.id, old.envelope_id, old.x, old.y, old.flags) = 0;" << endl;
	query << "select undoredo_add_query('update envelopepoint set id = '||quote(old.id)||', envelope_id = '||quote(old.envelope_id)||', x = '||quote(old.x)||', y = '||quote(old.y)||', flags = '||quote(old.flags)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// slice:
	query << "create temp trigger slice_insert_notify_trigger after insert on slice begin" << endl;
	query << "select undoredo_add_query('delete from slice where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where slice_notify_callback(0, new.id, new.wavelevel_id, new.sampleoffset) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger slice_delete_notify_trigger before delete on slice begin" << endl;
	query << "select undoredo_add_query('insert into slice values('||quote(old.id)||', '||quote(old.wavelevel_id)||', '||quote(old.sampleoffset)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger slice_after_delete_trigger after delete on slice begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where slice_notify_callback(1, old.id, old.wavelevel_id, old.sampleoffset) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger slice_update_notify_trigger after update on slice begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where slice_notify_callback(2, new.id, new.wavelevel_id, new.sampleoffset, old.id, old.wavelevel_id, old.sampleoffset) = 0;" << endl;
	query << "select undoredo_add_query('update slice set id = '||quote(old.id)||', wavelevel_id = '||quote(old.wavelevel_id)||', sampleoffset = '||quote(old.sampleoffset)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// eventconnectionbinding:
	query << "create temp trigger eventconnectionbinding_insert_notify_trigger after insert on eventconnectionbinding begin" << endl;
	query << "select undoredo_add_query('delete from eventconnectionbinding where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where eventconnectionbinding_notify_callback(0, new.id, new.connection_id, new.sourceindex, new.targetparamgroup, new.targetparamtrack, new.targetparamcolumn) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger eventconnectionbinding_delete_notify_trigger before delete on eventconnectionbinding begin" << endl;
	query << "select undoredo_add_query('insert into eventconnectionbinding values('||quote(old.id)||', '||quote(old.connection_id)||', '||quote(old.sourceindex)||', '||quote(old.targetparamgroup)||', '||quote(old.targetparamtrack)||', '||quote(old.targetparamcolumn)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger eventconnectionbinding_after_delete_trigger after delete on eventconnectionbinding begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where eventconnectionbinding_notify_callback(1, old.id, old.connection_id, old.sourceindex, old.targetparamgroup, old.targetparamtrack, old.targetparamcolumn) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger eventconnectionbinding_update_notify_trigger after update on eventconnectionbinding begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where eventconnectionbinding_notify_callback(2, new.id, new.connection_id, new.sourceindex, new.targetparamgroup, new.targetparamtrack, new.targetparamcolumn, old.id, old.connection_id, old.sourceindex, old.targetparamgroup, old.targetparamtrack, old.targetparamcolumn) = 0;" << endl;
	query << "select undoredo_add_query('update eventconnectionbinding set id = '||quote(old.id)||', connection_id = '||quote(old.connection_id)||', sourceindex = '||quote(old.sourceindex)||', targetparamgroup = '||quote(old.targetparamgroup)||', targetparamtrack = '||quote(old.targetparamtrack)||', targetparamcolumn = '||quote(old.targetparamcolumn)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// patternevent:
	query << "create temp trigger patternevent_before_insert_trigger before insert on patternevent begin" << endl;
	query << "select raise(abort, 'before insert failed from callback constraint') where patternevent_notify_callback(10, new.id, new.pattern_id, new.time, new.pluginparameter_id, new.value, new.meta) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternevent_insert_notify_trigger after insert on patternevent begin" << endl;
	query << "select undoredo_add_query('delete from patternevent where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where patternevent_notify_callback(0, new.id, new.pattern_id, new.time, new.pluginparameter_id, new.value, new.meta) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternevent_delete_notify_trigger before delete on patternevent begin" << endl;
	query << "select undoredo_add_query('insert into patternevent values('||quote(old.id)||', '||quote(old.pattern_id)||', '||quote(old.time)||', '||quote(old.pluginparameter_id)||', '||quote(old.value)||', '||quote(old.meta)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternevent_after_delete_trigger after delete on patternevent begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where patternevent_notify_callback(1, old.id, old.pattern_id, old.time, old.pluginparameter_id, old.value, old.meta) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternevent_update_notify_trigger after update on patternevent begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where patternevent_notify_callback(2, new.id, new.pattern_id, new.time, new.pluginparameter_id, new.value, new.meta, old.id, old.pattern_id, old.time, old.pluginparameter_id, old.value, old.meta) = 0;" << endl;
	query << "select undoredo_add_query('update patternevent set id = '||quote(old.id)||', pattern_id = '||quote(old.pattern_id)||', time = '||quote(old.time)||', pluginparameter_id = '||quote(old.pluginparameter_id)||', value = '||quote(old.value)||', meta = '||quote(old.meta)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	// patternformatcolumn:
	query << "create temp trigger patternformatcolumn_insert_notify_trigger after insert on patternformatcolumn begin" << endl;
	query << "select undoredo_add_query('delete from patternformatcolumn where id = '||quote(new.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "select raise(abort, 'after insert failed from callback constraint') where patternformatcolumn_notify_callback(0, new.id, new.patternformat_id, new.pluginparameter_id, new.mode, new.is_collapsed, new.idx) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternformatcolumn_delete_notify_trigger before delete on patternformatcolumn begin" << endl;
	query << "select raise(abort, 'before delete failed from callback constraint') where patternformatcolumn_notify_callback(11, old.id, old.patternformat_id, old.pluginparameter_id, old.mode, old.is_collapsed, old.idx) = 0;" << endl;
	query << "select undoredo_add_query('insert into patternformatcolumn values('||quote(old.id)||', '||quote(old.patternformat_id)||', '||quote(old.pluginparameter_id)||', '||quote(old.mode)||', '||quote(old.is_collapsed)||', '||quote(old.idx)||');') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternformatcolumn_after_delete_trigger after delete on patternformatcolumn begin" << endl;
	query << "select raise(abort, 'after delete failed from callback constraint') where patternformatcolumn_notify_callback(1, old.id, old.patternformat_id, old.pluginparameter_id, old.mode, old.is_collapsed, old.idx) = 0;" << endl;
	query << "end;" << endl;

	query << "create temp trigger patternformatcolumn_update_notify_trigger after update on patternformatcolumn begin" << endl;
	query << "select raise(abort, 'after update failed from callback constraint') where patternformatcolumn_notify_callback(2, new.id, new.patternformat_id, new.pluginparameter_id, new.mode, new.is_collapsed, new.idx, old.id, old.patternformat_id, old.pluginparameter_id, old.mode, old.is_collapsed, old.idx) = 0;" << endl;
	query << "select undoredo_add_query('update patternformatcolumn set id = '||quote(old.id)||', patternformat_id = '||quote(old.patternformat_id)||', pluginparameter_id = '||quote(old.pluginparameter_id)||', mode = '||quote(old.mode)||', is_collapsed = '||quote(old.is_collapsed)||', idx = '||quote(old.idx)||' where id = '||quote(old.id)||';') where undoredo_enabled_callback() = 1;" << endl;
	query << "end;" << endl;

}

