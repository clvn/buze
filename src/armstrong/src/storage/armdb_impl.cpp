#include <iostream>
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <fstream>
#include <cmath>
#include "database.h"
#include "document.h"
#include "../mixing/convertsample.h"

using namespace std;
using namespace dbgenpp;

namespace armstrong {
namespace storage {

//
// tableiterator
//

tableiterator::tableiterator()
:
	stmt(0),
	prepareresult(0),
	stepresult(SQLITE_DONE)
{}

tableiterator::tableiterator(sqlite3* db, string const& query)
:
	stmt(0),
	stepresult(SQLITE_DONE)
{
	prepareresult = sqlite3_prepare_v2(db, query.c_str(), (int)query.length(), &stmt, 0);

	if (prepareresult != 0) {
		cerr << "Error in query:" << endl;
		cerr << query << endl;
		cerr << sqlite3_errmsg(db) << endl;
		assert(false);
		return;
	}

	next();
}

tableiterator::~tableiterator() {
	if (stmt) sqlite3_finalize(stmt);
}

bool tableiterator::prepare(sqlite3* db, string const& query) {
	prepareresult = sqlite3_prepare_v2(db, query.c_str(), (int)query.length(), &stmt, 0);

	if (prepareresult != SQLITE_OK) {
		cerr << "Error in query:" << endl << query << endl;
		cerr << sqlite3_errmsg(db) << endl;
		assert(false);
		return false;
	}

	stepresult = SQLITE_DONE;
	next();
	return true;
}

bool tableiterator::next() {
	assert(stmt != 0);
	stepresult = sqlite3_step(stmt);
	return stepresult == SQLITE_ROW || stepresult == SQLITE_DONE;
}

bool tableiterator::eof() const {
	return stepresult == SQLITE_DONE;
}

void tableiterator::reset() {
	prepareresult = sqlite3_reset(stmt);
	assert(prepareresult == SQLITE_OK);
	stepresult = SQLITE_DONE;
	next();
}

void tableiterator::destroy() {
	sqlite3_finalize(stmt);
	stmt = 0;
	prepareresult = 0;
	stepresult = SQLITE_DONE;
}

int tableiterator::id() {
	return sqlite3_column_int(stmt, 0);
}

//
// utils
//

extern "C" void undoredo_enabled_callback(sqlite3_context* ctx, int, sqlite3_value** row) {
	document* self = (document*)sqlite3_user_data(ctx);
	sqlite3_result_int(ctx, self->undoredo_enabled?1:0);
}

extern "C" void undoredo_add_query(sqlite3_context* ctx, int, sqlite3_value** row) {
	document* self = (document*)sqlite3_user_data(ctx);
	std::string query = (const char*)sqlite3_value_text(row[0]);
	self->add_undo_query(query);
}

extern "C" void noteutil_buzz_note_transpose(sqlite3_context* ctx, int, sqlite3_value** row) {
	static const int midi_note_min = 0;
	static const int midi_note_max = 119;

	int value = sqlite3_value_int(row[0]);
	int delta = sqlite3_value_int(row[1]);
	if (value != 0 && value != 254 && value != 255) {
		int midinote = (12 * (value >> 4)) + ((value & 0xF) - 1);
		midinote += delta;
		midinote = std::max(std::min(midinote, midi_note_max), midi_note_min);
		int buzznote = ((midinote / 12) << 4) + ((midinote % 12) + 1);
		sqlite3_result_int(ctx, buzznote);
	} else {
		sqlite3_result_int(ctx, value);
	}
}

extern "C" void noteutil_buzz_note_base(sqlite3_context* ctx, int, sqlite3_value** row) {
	int value = sqlite3_value_int(row[0]);
	sqlite3_result_int(ctx, (value & 0xF) - 1);
}

extern "C" void noteutil_buzz_to_midi_note(sqlite3_context* ctx, int, sqlite3_value** row) {
	int value = sqlite3_value_int(row[0]);
	sqlite3_result_int(ctx, 12 * (value >> 4) + (value & 0xF) - 1);
}

extern "C" void noteutil_midi_to_buzz_note(sqlite3_context* ctx, int, sqlite3_value** row) {
	int value = sqlite3_value_int(row[0]);
	sqlite3_result_int(ctx, ((value / 12) << 4) + (value % 12) + 1);
}

extern "C" void orderlist_timeshift(sqlite3_context* ctx, int, sqlite3_value** row) {
	int song_id = sqlite3_value_int(row[0]);
	int index = sqlite3_value_int(row[1]);
	int timeshift = sqlite3_value_int(row[2]);

	document* self = (document*)sqlite3_user_data(ctx);

	std::stringstream query;
	// add a call to ourself with negated timeshift to the undo buffer
	query << "select orderlist_timeshift(" << song_id << ", " << index << ", " << -timeshift << ");";
	self->add_undo_query(query.str());

	document_event_data ev;
	orderlist_timeshiftdata data;
	data.song_id = song_id;
	data.index = index;
	data.timeshift = timeshift;

	ev.type = event_type_orderlist_timeshift;
	ev.id = song_id;
	ev.newdata.orderlist_timeshift = &data;
	self->notify_listeners(&ev);
	// TODO: generate event -> pickup in player -> add signal to mixer commit queue -> pickup in mixer audio thread -> make sure the orderlist position keeps pointing at the expected index after orderlist operations
	// NOTE: if 
}

extern "C" void ensure_plugin_parameters(sqlite3_context* ctx, int, sqlite3_value** row) {
	// TODO: should take an optional prefix parameter from the sql and pass it in the event args so this can be used outside of import scripts
	document* self = (document*)sqlite3_user_data(ctx);

	document_event_data ev;
	ev.type = event_type_ensure_plugin_parameters;
	self->notify_listeners(&ev);
}


namespace {
	template <class X, class Y, class Z>
	inline X clamp(X value, Y minimum, Z maximum) {
		return value < minimum ? (X)minimum : value > maximum ? (X)maximum : value;
	}
}

extern "C" void scale_param(sqlite3_context* ctx, int, sqlite3_value** row) {
	int x       = sqlite3_value_int(row[0]);
	int src_min = sqlite3_value_int(row[1]);
	int src_max = sqlite3_value_int(row[2]);
	int dst_min = sqlite3_value_int(row[3]);
	int dst_max = sqlite3_value_int(row[4]);
	double src_range0 = src_max - src_min;
	double dst_range0 = dst_max - dst_min;
	double x0         = (x - src_min) / src_range0;
	int scaled_x      = int((x0 * dst_range0) + 0.5) + dst_min;
	sqlite3_result_int(ctx, scaled_x);
}

extern "C" void scale_param_range(sqlite3_context* ctx, int, sqlite3_value** row) {
	int x          = sqlite3_value_int(row[0]);
	int param_min  = sqlite3_value_int(row[1]);
	int param_max  = sqlite3_value_int(row[2]);
	double in_min  = sqlite3_value_double(row[3]);
	double in_max  = sqlite3_value_double(row[4]);
	double out_min = sqlite3_value_double(row[5]);
	double out_max = sqlite3_value_double(row[6]);
	double param_range0 = param_max - param_min;
	double in_range0    = in_max - in_min;
	double out_range0   = out_max - out_min;
	double x0           = (x - param_min) / param_range0;
	int scaled_x        = int((((out_range0 * ((x0 - in_min) / in_range0)) + out_min) * param_range0) + 0.5) + param_min;
	sqlite3_result_int(ctx, clamp(scaled_x, param_min, param_max));
}

extern "C" void scale_param_point(sqlite3_context* ctx, int, sqlite3_value** row) {
	int x          = sqlite3_value_int(row[0]);
	int param_min  = sqlite3_value_int(row[1]);
	int param_max  = sqlite3_value_int(row[2]);
	int pt		   = sqlite3_value_int(row[3]);
	int pt_min     = sqlite3_value_int(row[4]);
	int pt_max     = sqlite3_value_int(row[5]);
	double out_min = sqlite3_value_double(row[6]);
	double out_max = sqlite3_value_double(row[7]);
	double param_range0 = param_max - param_min;
	double pt_range0    = pt_max - pt_min;
	double out_range0   = out_max - out_min;
	double x0           = (x - param_min) / param_range0;
	double pt0          = (pt - pt_min) / pt_range0;
	int scaled_x        = int((((x0 * (pt0 * out_range0)) + out_min) * param_range0) + 0.5) + param_min;
	sqlite3_result_int(ctx, clamp(scaled_x, param_min, param_max));
}

extern "C" void zzub_print(sqlite3_context* ctx, int, sqlite3_value** row) {
	const char* value = (const char*)sqlite3_value_text(row[0]);
	cout << "zzub_print: " << value << endl;
}

//
// song
//

song::song(document* _owner) {
	owner = _owner;
	id = 0;
	loopbegin = 0;
	loopend = 0;
	loopenabled = true;
}

bool song::update() {
	sqlite::statement stmt(owner->db, get_update_statement<songdata>());
	stmt.stmt << *this;
	return stmt.execute();
}

bool song::create_attributeinfo(int plugininfo_id, int index, std::string name, int minvalue, int maxvalue, int defaultvalue, attributeinfodata& result) {
	stringstream query;
	query << "insert into attributeinfo (plugininfo_id, attrindex, name, minvalue, maxvalue, defaultvalue) values (" 
		<< "?, ?, ?, ?, ?, ?);";

	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, plugininfo_id);
	sqlite3_bind_int(stmt.stmt, 2, index);
	sqlite3_bind_text(stmt.stmt, 3, name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt.stmt, 4, minvalue);
	sqlite3_bind_int(stmt.stmt, 5, maxvalue);
	sqlite3_bind_int(stmt.stmt, 6, defaultvalue);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.plugininfo_id = plugininfo_id;
	result.attrindex = index;
	result.name = name;
	result.minvalue = minvalue;
	result.maxvalue = maxvalue;
	result.defaultvalue = defaultvalue;

	return true;
}

bool song::create_plugininfo(int flags, std::string uri, std::string name, std::string short_name, std::string author, int mintracks, int maxtracks, int inputs, int outputs, plugininfodata& result) {
	assert(id != 0);

	stringstream query;
	query << "insert into plugininfo (song_id, flags, uri, name, short_name, author, mintracks, maxtracks, input_count, output_count) values (" 
		<< "?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, id);
	sqlite3_bind_int(stmt.stmt, 2, flags);
	sqlite3_bind_text(stmt.stmt, 3, uri.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt.stmt, 4, name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt.stmt, 5, short_name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt.stmt, 6, author.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt.stmt, 7, mintracks);
	sqlite3_bind_int(stmt.stmt, 8, maxtracks);
	sqlite3_bind_int(stmt.stmt, 9, inputs);
	sqlite3_bind_int(stmt.stmt, 10, outputs);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.song_id = id;
	result.flags = flags;
	result.uri = uri;
	result.name = name;
	result.short_name = name;
	result.author = author;
	result.mintracks = mintracks;
	result.maxtracks = maxtracks;
	result.input_count = inputs;
	result.output_count = outputs;
	return true;
}

bool song::create_parameterinfo(int plugininfo_id, int group, int track, int column, std::string name, std::string description, int flags, int type, int minvalue, int maxvalue, int novalue, int defaultvalue, parameterinfodata& result) {

	stringstream query;
	query << "insert into parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) values (" 
		<< "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, plugininfo_id);
	sqlite3_bind_int(stmt.stmt, 2, group);
	sqlite3_bind_int(stmt.stmt, 3, column);
	sqlite3_bind_text(stmt.stmt, 4, name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt.stmt, 5, description.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt.stmt, 6, flags);
	sqlite3_bind_int(stmt.stmt, 7, type);
	sqlite3_bind_int(stmt.stmt, 8, minvalue);
	sqlite3_bind_int(stmt.stmt, 9, maxvalue);
	sqlite3_bind_int(stmt.stmt, 10, novalue);
	sqlite3_bind_int(stmt.stmt, 11, defaultvalue);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.plugininfo_id = plugininfo_id;
	result.paramgroup = group;
	result.paramcolumn = column;
	result.name = name;
	result.description = description;
	result.flags = flags;
	result.type = type;
	result.minvalue = minvalue;
	result.maxvalue = maxvalue;
	result.novalue = novalue;
	result.defaultvalue = defaultvalue;

	return true;
}

bool song::create_plugin(int plugininfo_id, std::vector<char>& bytes, std::string name, int tracks, int flags, int plugingroup_id, plugindata& result) {
	assert(id != 0);

	stringstream query;
	query << "insert into plugin (song_id, name, flags, trackcount, plugininfo_id, x, y, streamsource, timesource_plugin_id, data, latency, plugingroup_id) values (" 
		<< "?, ?, ?, ?, ?, 0, 0, '', -1, ?, -1, ?);";

	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, id);
	sqlite3_bind_text(stmt.stmt, 2, name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt.stmt, 3, flags);
	sqlite3_bind_int(stmt.stmt, 4, tracks);
	sqlite3_bind_int(stmt.stmt, 5, plugininfo_id);
	if (bytes.size() > 0)
		sqlite3_bind_blob(stmt.stmt, 6, &bytes.front(), (int)bytes.size(), SQLITE_STATIC); 
	else
		sqlite3_bind_null(stmt.stmt, 6);
	if (plugingroup_id != 0)
		sqlite3_bind_int(stmt.stmt, 7, plugingroup_id);
	else
		sqlite3_bind_null(stmt.stmt, 7);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.data = std::vector<unsigned char>(bytes.begin(), bytes.end());
	result.plugininfo_id = plugininfo_id;
	result.flags = flags;
	result.x = 0.0f;;
	result.y = 0.0f;
	result.song_id = id;
	result.name = name;
	result.trackcount = tracks;
	result.timesource_plugin_id = -1;
	result.latency = -1;
	result.plugingroup_id = plugingroup_id;
	return true;
}

bool song::create_pattern(int format_id, const char* name, int length, patterndata& result) {
	assert(id != 0);

	stringstream query;
	query << "insert into pattern (song_id, patternformat_id, name, length, resolution, display_resolution, display_verydark_row, display_dark_row, beginloop, endloop, loopenabled, replay_row) values ("
		<< "?, ?, ?, ?, 1, 1, 16, 4, ?, ?, 0, 0);";
	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, id);
	sqlite3_bind_int(stmt.stmt, 2, format_id);
	sqlite3_bind_text(stmt.stmt, 3, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt.stmt, 4, length);
	sqlite3_bind_int(stmt.stmt, 5, 0);
	sqlite3_bind_int(stmt.stmt, 6, length);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.song_id = id;
	result.name = name;
	result.length = length;
	result.resolution = 1;
	result.display_resolution = 1;
	result.display_verydark_row = 16;
	result.display_dark_row = 4;
	result.beginloop = 0;
	result.endloop = length;
	result.loopenabled = false;
	result.replay_row = 0;
	return true;
}

bool song::create_pattern_format(std::string name, patternformatdata& result) {
	assert(id != 0);

	stringstream query;
	query << "insert into patternformat (song_id, name, scroller_width) values ("
		<< "?, ?, -1);";

	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, id);
	sqlite3_bind_text(stmt.stmt, 2, name.c_str(), -1, SQLITE_STATIC);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.song_id = id;
	result.name = name;
	result.scroller_width = -1;
	return true;
}

bool song::create_plugingroup(int parent_id, const std::string& name, plugingroupdata& result) {
	assert(id != 0);

	stringstream query;
	query << "insert into plugingroup (song_id, name, parent_plugingroup_id) values ("
		<< "?, ?, ?);";

	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, id);
	sqlite3_bind_text(stmt.stmt, 2, name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt.stmt, 3, parent_id);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.song_id = id;
	result.name = name;
	result.parent_plugingroup_id = parent_id;
	return true;
}

int song::get_pattern_format_column_default_idx(int format_id, int pluginparameter_id) {

	stringstream q;
	q << "select idx from patternformatcolumn c"
	  << " inner join pluginparameter pp on pp.id = c.pluginparameter_id"
	  << " inner join parameterinfo i on i.id = pp.parameterinfo_id"
	  << " inner join pluginparameter ppsrc on ppsrc.id = " << pluginparameter_id
	  << " inner join parameterinfo isrc on isrc.id = ppsrc.parameterinfo_id"
	  << " where c.patternformat_id = " << format_id
	  << "  and ("
	  << "       (pp.plugin_id = ppsrc.plugin_id and i.paramgroup = isrc.paramgroup and pp.paramtrack = ppsrc.paramtrack and i.paramcolumn > isrc.paramcolumn)"
	  << "    or (pp.plugin_id = ppsrc.plugin_id and i.paramgroup = isrc.paramgroup and pp.paramtrack > ppsrc.paramtrack)"
	  << "    or (pp.plugin_id = ppsrc.plugin_id and i.paramgroup > isrc.paramgroup)"
	  << "  )"
	  << " order by idx"
	  << " limit 1;";
	sqlite::statement stmt_getdefault_right(owner->db, q.str());

	int result;

	stmt_getdefault_right.execute();
	if (!stmt_getdefault_right.eof()) { // found a position
		result = sqlite3_column_int(stmt_getdefault_right.stmt, 0);
	} else { // add column at end of the plugin
		q.str("");
		q << "select idx from patternformatcolumn c"
		  << " inner join pluginparameter pp on pp.id = c.pluginparameter_id"
		  << " inner join pluginparameter ppsrc on ppsrc.id = " << pluginparameter_id
		  << " where c.patternformat_id = "<<format_id
		  << "  and pp.plugin_id = ppsrc.plugin_id"
		  << "  order by idx desc"
		  << "  limit 1;";
		sqlite::statement stmt_getdefault_left(owner->db, q.str());

		stmt_getdefault_left.execute();
		if (!stmt_getdefault_left.eof()) {
			result = 1 + sqlite3_column_int(stmt_getdefault_left.stmt, 0);
		} else { // add column at end of the format
			q.str("");
			q << "select count(*) from patternformatcolumn where patternformat_id = "<<format_id<<";";
			sqlite::statement stmt_getcount(owner->db, q.str());

			stmt_getcount.execute();
			if (!stmt_getcount.eof()) {
				result = sqlite3_column_int(stmt_getcount.stmt, 0);
			} else { //
				result = 0;
			}
		}
	}

	return result;
}

bool song::create_pattern_format_column(int format_id, int pluginparameter_id, int idx, patternformatcolumndata& result) {
	assert(id != 0);

	if (idx == -1)
		idx = get_pattern_format_column_default_idx(format_id, pluginparameter_id);

	stringstream query;
	query << "update patternformatcolumn set idx = idx + 1 where patternformat_id = "<<format_id<<" and idx >= "<<idx<<";";
	owner->exec_noret(query.str());

	query.str("");
	query << "insert into patternformatcolumn (patternformat_id, pluginparameter_id, mode, is_collapsed, idx)";
	query << "  values (?, ?, 0, 0, ?);";

	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, format_id);
	sqlite3_bind_int(stmt.stmt, 2, pluginparameter_id);
	sqlite3_bind_int(stmt.stmt, 3, idx);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.patternformat_id = format_id;
	result.pluginparameter_id = pluginparameter_id;
	result.mode = 0;
	result.is_collapsed = 0;
	result.idx = idx;
	return true;
}

bool song::create_wave(int flags, wavedata& result) {
	assert(id != 0);

	stringstream query;
	query << "insert into wave (song_id, flags, volume, name, filename) values ("
		<< "?, ?, 1.0, '', '');";

	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, id);
	sqlite3_bind_int(stmt.stmt, 2, flags);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.song_id = id;
	result.volume = 1.0f;
	result.flags = flags;
	return true;
}

bool song::create_wavelevel(int wave_id, int format, int samplerate, waveleveldata& result) {
	assert(id != 0);

	stringstream query;
	query << "insert into wavelevel (wave_id, basenote, samplerate, samplecount, beginloop, endloop, format, filename) values ("
		<< "?, 65, ?, 0, 0, 0, ?, '');";

	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, wave_id);
	sqlite3_bind_int(stmt.stmt, 2, samplerate);
	sqlite3_bind_int(stmt.stmt, 3, format);

	if (!stmt.execute()) return false;
	
	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.wave_id = wave_id;
	result.basenote = 65;
	result.beginloop = 0;
	result.endloop = 0;
	result.format = format;
	result.samplecount = 0;
	result.samplerate = samplerate;

	int temp = 0;
	owner->wavelevel_set_samples(result.id, 0, &temp);
	return true;
}

void song::get_plugins(tableiterator* result) {
	stringstream query;
	// retreive all plugins except those associated with a sequence
	query << "select id from plugin where song_id = " << id << ";"; // and id not in (select plugin_id from sequence)";
	result->prepare(owner->db, query.str());
}

void song::get_connections(tableiterator* result) {
	stringstream query;
	query << "select id from connection";
	result->prepare(owner->db, query.str());
}

void song::get_patterns(tableiterator* result) {
	stringstream query;
	query << "select id from pattern;";
	result->prepare(owner->db, query.str());
}

void song::get_patternformats(tableiterator* result) {
	stringstream query;
	query << "select id from patternformat;";
	result->prepare(owner->db, query.str());
}

void song::get_parameterinfos(int plugininfo_id, int group, tableiterator* result) {
	stringstream query;
	query << "select id from parameterinfo where plugininfo_id = " << plugininfo_id << " and paramgroup = " << group << " order by paramcolumn";
	result->prepare(owner->db, query.str());
}

void song::get_attributeinfos(int plugininfo_id, tableiterator* result) {
	stringstream query;
	query << "select * from attributeinfo where plugininfo_id = " << plugininfo_id << " order by id";
	result->prepare(owner->db, query.str());
}

int song::get_plugin_count() {
	stringstream q;
	q << "select count(*) from plugin where song_id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

int song::get_sequence_count() {
	assert(false);
	stringstream q;
	q << "select count(*) from sequence where plugin_id in (select id from plugin where song_id = " << id << ");";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

int song::get_pattern_count() {
	stringstream q;
	q << "select count(*) from pattern;";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

int song::get_pattern_format_count() {
	stringstream q;
	q << "select count(*) from patternformat;";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

int song::get_wave_count() {
	stringstream q;
	q << "select count(*) from wave where song_id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

bool song::get_plugin_by_id(int id, plugindata& result) {
	stringstream q;
	q << "select * from plugin where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_plugin_by_name(string name, plugindata& result) {
	stringstream q;
	q << "select * from plugin where song_id = ? and name = ?;";

	sqlite::statement stmt(owner->db, q.str());
	sqlite3_bind_int(stmt.stmt, 1, id);
	sqlite3_bind_text(stmt.stmt, 2, name.c_str(), -1, SQLITE_STATIC);

	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_plugin_by_index(int index, plugindata& result) {
	stringstream q;
	q << "select * from plugin where song_id = " << id << " limit 1 offset " << index;

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_plugininfo_by_plugin_id(int id, plugininfodata& result) {
	stringstream q;
	q << "select * from plugininfo where id = (select plugininfo_id from plugin where id = " << id << ");";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_plugininfo_by_id(int id, plugininfodata& result) {
	stringstream q;
	q << "select * from plugininfo where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_plugininfo_by_uri(std::string uri, plugininfodata& result) {
	stringstream q;
	q << "select * from plugininfo where uri = ?;";

	sqlite::statement stmt(owner->db, q.str());
	sqlite3_bind_text(stmt.stmt, 1, uri.c_str(), -1, SQLITE_STATIC);

	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_parameterinfo_by_id(int id, parameterinfodata& result) {
	stringstream q;
	q << "select * from parameterinfo where id = ?;";

	sqlite::statement stmt(owner->db, q.str());
	sqlite3_bind_int(stmt.stmt, 1, id);

	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_parameterinfo(int pluginid, int group, int /*track*/, int column, parameterinfodata& result) {
	stringstream q;
	q << "select * from parameterinfo where plugininfo_id = (select plugininfo_id from plugin where id = ?) and paramgroup = ? and paramtrack = ? and paramcolumn = ?;";

	sqlite::statement stmt(owner->db, q.str());
	sqlite3_bind_int(stmt.stmt, 1, pluginid);
	sqlite3_bind_int(stmt.stmt, 2, group);
	sqlite3_bind_int(stmt.stmt, 3, 0/*track*/);
	sqlite3_bind_int(stmt.stmt, 4, column);

	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_attributeinfo_by_id(int id, attributeinfodata& result) {
	stringstream q;
	q << "select * from attributeinfo where id = ?;";

	sqlite::statement stmt(owner->db, q.str());
	sqlite3_bind_int(stmt.stmt, 1, id);

	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_attribute_by_id(int id, attributedata& result) {
	stringstream q;
	q << "select * from attribute where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_pattern_by_index(int index, patterndata& result) {
	stringstream q;
	q << "select * from pattern limit 1 offset " << index;

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_patternformat_by_index(int index, patternformatdata& result) {
	stringstream q;
	q << "select * from patternformat limit 1 offset " << index;

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_patternorder_by_id(int id, patternorderdata& result) {
	stringstream q;
	q << "select * from patternorder where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

int song::get_midimapping_count() {
	stringstream q;
	q << "select count(*) from midimapping;";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

bool song::get_midimapping_by_id(int id, midimappingdata& result) {
	stringstream q;
	q << "select * from midimapping where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_midimapping_by_index(int pos, midimappingdata& result) {
	stringstream q;
	//q << "select * from sequence where plugin_id in (select id from plugin where song_id = " << id << ") and position = " << pos;
	q << "select * from midimapping limit 1 offset " << pos;

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_wave_by_id(int id, wavedata& result) {
	stringstream q;
	q << "select * from wave where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_wave_by_index(int pos, wavedata& result) {
	stringstream q;
	//q << "select * from sequence where plugin_id in (select id from plugin where song_id = " << id << ") and position = " << pos;
	q << "select * from wave limit 1 offset " << pos;

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_wave_by_wavelevel_id(int wavelevel_id, wavedata& result) {
	stringstream q;
	q << "select * from wave where id = (select wave_id from wavelevel where id = " << wavelevel_id << ");";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_wavelevel_by_id(int id, waveleveldata& result) {
	stringstream q;
	q << "select * from wavelevel where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_envelope_by_id(int id, envelopedata& result) {
	stringstream q;
	q << "select * from envelope where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_pattern_by_id(int id, patterndata& result) {
	stringstream q;
	q << "select * from pattern where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_pattern_by_name(std::string name, patterndata& result) {
	stringstream q;
	q << "select * from pattern where name = ? limit 1;";

	sqlite::statement stmt(owner->db, q.str());
	sqlite3_bind_text(stmt.stmt, 1, name.c_str(), -1, SQLITE_STATIC);

	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_patternevent_by_id(int id, patterneventdata& result) {
	stringstream q;
	q << "select * from patternevent where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_patternformat_by_id(int id, patternformatdata& result) {
	stringstream q;
	q << "select * from patternformat where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_patternformat_by_name(std::string name, patternformatdata& result) {
	stringstream q;
	q << "select * from patternformat where name = ? limit 1;";

	sqlite::statement stmt(owner->db, q.str());
	sqlite3_bind_text(stmt.stmt, 1, name.c_str(), -1, SQLITE_STATIC);

	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_patternformatcolumn_by_id(int id, patternformatcolumndata& result) {
	stringstream q;
	q << "select * from patternformatcolumn where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_patternformatcolumn_by_filter_id(int id, patternformatcolumndata& result) {
	stringstream q;
	q << "select * from patternformatcolumn where id = (select patternformatcolumn_id from patternformatcolumnfilter where id = " << id << ");";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

/*bool song::get_patternformatcolumnfilter_by_id(int id, patternformatcolumnfilterdata& result) {
	stringstream q;
	q << "select * from patternformatcolumnfilter where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}
*/
bool song::get_patternformattrack_by_id(int id, patternformattrackdata& result) {
	stringstream q;
	q << "select * from patternformattrack where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_connection_by_id(int id, connectiondata& result) {
	stringstream q;
	q << "select * from connection where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_eventconnectionbinding_by_id(int id, eventconnectionbindingdata& result) {
	stringstream q;
	q << "select * from eventconnectionbinding where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_pluginparameter_by_id(int id, pluginparameterdata& result) {
	stringstream q;
	q << "select * from pluginparameter where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool song::get_plugingroup_by_id(int id, plugingroupdata& result) {
	stringstream q;
	q << "select * from plugingroup where id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

void song::get_plugingroups(tableiterator* it) {
	stringstream query;
	// retreive all plugins except those associated with a sequence
	query << "select id from plugingroup where song_id = " << id << ";"; // and id not in (select plugin_id from sequence)";
	it->prepare(owner->db, query.str());
}

void song::get_plugingroups(int parent_plugingroup_id, tableiterator* it) {
	stringstream query;
	// retreive all plugins except those associated with a sequence
	query << "select id from plugingroup where song_id = " << id << " and ifnull(parent_plugingroup_id, 0) = " << parent_plugingroup_id << ";"; // and id not in (select plugin_id from sequence)";
	it->prepare(owner->db, query.str());
}

void song::destroy() {
	stringstream query;
	query << "delete from song where id = " << id;
	owner->exec_noret(query.str());
}

//
// plugin
//

plugin::plugin(document* _owner) {
	owner = _owner;
	x = y = 0.0f;
}

bool plugin::update() {
	sqlite::statement stmt(owner->db, get_update_statement<plugindata>());
	stmt.stmt << *this;
	return stmt.execute();
}

void plugin::destroy() {
	stringstream query;
	query << "delete from plugin where id = " << id << ";";
	owner->exec_noret(query.str());
}

int plugin::get_parameter_value(int group, int track, int column) {
	stringstream query;
	query << "select value from pluginparameter pp "
		<< "inner join parameterinfo i on pp.parameterinfo_id = i.id "
		<< "where pp.plugin_id = " << id << " and i.paramgroup = " << group << " and pp.paramtrack = " << track << " and i.paramcolumn = " << column;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return -1;
	return sqlite3_column_int(stmt.stmt, 0);
}

void plugin::set_parameter_value(int group, int track, int column, int value) {
	// TODO: insert or update only
	stringstream query;
	query << "select pp.id from pluginparameter pp "
		  << "inner join parameterinfo i on pp.parameterinfo_id = i.id "
		  << "where pp.plugin_id = " << id << " and i.paramgroup = " << group << " and pp.paramtrack = " << track << " and i.paramcolumn = " << column;

	int pluginparameter_id;
	if (owner->exec_scalar(query.str(), &pluginparameter_id)) {
		query.str("");
		query << "update pluginparameter set value = " << value << " where id = " << pluginparameter_id;
		owner->exec_noret(query.str());
	} else {
		query.str("");
		query << "select id from parameterinfo where plugininfo_id = " << plugininfo_id << " and paramgroup = " << group << " and paramcolumn = " << column;
		int parameterinfo_id;
		if (owner->exec_scalar(query.str(), &parameterinfo_id)) {
			query.str("");
			query << "insert into pluginparameter (plugin_id, parameterinfo_id, paramtrack, value) values (" << id << ", " << parameterinfo_id << ", " << track << ", " << value << ");";
			owner->exec_noret(query.str());
		} else {
			assert(false); // shouldnt get here
		}
	}

}

bool plugin::add_audio_input(int connplugin_id, const plugin& fromplugin, int first_input, int input_count, int first_output, int output_count, connectiondata& result) {
	// zzub_connection_type_event == 0
	return add_input(connplugin_id, fromplugin, 0, first_input, input_count, first_output, output_count, "", result);
}

bool plugin::add_midi_input(int connplugin_id, const plugin& fromplugin, std::string mididevice, connectiondata& result) {
	// zzub_connection_type_event == 2
	return add_input(connplugin_id, fromplugin, 2, 0, 0, 0, 0, mididevice, result);
}

bool plugin::add_event_input(int connplugin_id, const plugin& fromplugin, connectiondata& result) {
	// zzub_connection_type_event == 1
	return add_input(connplugin_id, fromplugin, 1, 0, 0, 0, 0, "", result);
}

bool plugin::add_note_input(int connplugin_id, const plugin& fromplugin, connectiondata& result) {
	// zzub_connection_type_event == 1
	return add_input(connplugin_id, fromplugin, 3, 0, 0, 0, 0, "", result);
}

bool plugin::add_input(int connplugin_id, const plugin& fromplugin, int type, int first_input, int input_count, int first_output, int output_count, std::string midi_device, connectiondata& result) {
	// TODO: check for duplicate and apply business rules for a connection here yes?
	stringstream query;
	query << "insert into connection (plugin_id, from_plugin_id, to_plugin_id, type, first_input, input_count, first_output, output_count, mididevice) values ("
		<< "?, ?, ?, ?, ?, ?, ?, ?, ?);";
	//	<< connplugin_id << ", " << fromplugin.id << ", " << id << ", " << type << ", '');";
	//if (!owner->exec_noret(query.str())) return false;
	sqlite::statement stmt(owner->db, query.str());
	sqlite3_bind_int(stmt.stmt, 1, connplugin_id);
	sqlite3_bind_int(stmt.stmt, 2, fromplugin.id);
	sqlite3_bind_int(stmt.stmt, 3, id);
	sqlite3_bind_int(stmt.stmt, 4, type);
	sqlite3_bind_int(stmt.stmt, 5, first_input);
	sqlite3_bind_int(stmt.stmt, 6, input_count);
	sqlite3_bind_int(stmt.stmt, 7, first_output);
	sqlite3_bind_int(stmt.stmt, 8, output_count);
	sqlite3_bind_text(stmt.stmt, 9, midi_device.c_str(), -1, SQLITE_STATIC);

	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.plugin_id = connplugin_id;
	result.from_plugin_id = fromplugin.id;
	result.to_plugin_id = id;
	result.type = type;
	result.first_input = first_input;
	result.input_count = input_count;
	result.first_output = first_output;
	result.output_count = output_count;
	result.mididevice = midi_device;
	return true;
}

int plugin::get_input_connection_count() {
	stringstream query;
	query << "select count(*) from connection where to_plugin_id = " << id << ";";
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

bool plugin::get_input_connection_by_type(const plugin& fromplugin, int type, connectiondata& result) {
	stringstream query;
	query << "select * from connection where to_plugin_id = " << id << " and from_plugin_id = " << fromplugin.id << " and type = " << type;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool plugin::get_input_connection_plugin_by_type(const plugin& fromplugin, int type, plugindata& result) {
	stringstream query;
	query << "select * from plugin where id = (select plugin_id from connection where to_plugin_id = " << id << " and from_plugin_id = " << fromplugin.id << " and type = " << type << ")";
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool plugin::get_input_connection_by_index(int index, connectiondata& result) {
	stringstream query;
	query << "select * from connection where to_plugin_id = " << id << " limit 1 offset " << index;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

int plugin::get_output_connection_count() {
	stringstream query;
	query << "select count(*) from connection where from_plugin_id = " << id << ";";
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

bool plugin::get_output_connection_by_type(const plugin& fromplugin, int type, connectiondata& result) {
	stringstream query;
	query << "select * from connection where to_plugin_id = " << fromplugin.id << " and from_plugin_id = " << id << " and type = " << type;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

bool plugin::get_output_connection_by_index(int index, connectiondata& result) {
	stringstream query;
	query << "select * from connection where from_plugin_id = " << id << " limit 1 offset " << index;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

void plugin::add_midimapping(int group, int track, int param, int channel, int controller) {
	stringstream query;
	query << "insert into midimapping (plugin_id, paramgroup, paramtrack, paramcolumn, midichannel, midicontroller) values (" << id << ", " << group << ", " << track << ", " << param << ", " << channel << ", " << controller << ");";
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return ;
}

void plugin::remove_midimapping(int group, int track, int param) {
	stringstream query;
	query << "delete from midimapping where plugin_id = " << id << " and paramgroup = " << group << " and paramtrack = " << track << " and paramcolumn = " << param << ";";
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return ;
}

void plugin::set_attribute(int index, int value) {
	stringstream query;
	query << "select id from attribute where plugin_id = " << id << " and attrindex = " << index << ";";
	sqlite::statement stmt(owner->db, query.str());
	query.str("");
	if (!stmt.execute() || stmt.eof()) {
		query << "insert into attribute (plugin_id, attrindex, value) values (" << id << ", " << index << ", " << value << ");";
		owner->exec_noret(query.str());
	} else {
		int attrid = sqlite3_column_int(stmt.stmt, 0);
		query << "update attribute set value = " << value << " where id = " << attrid;
		owner->exec_noret(query.str());
	}
}

int plugin::get_attribute(int index) {
	stringstream query;
	query << "select value from attribute where plugin_id = " << id << " and attrindex = " << index << ";";
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) {
		query.str("");
		query << "select defaultvalue from attributeinfo where plugininfo_id = (select plugininfo_id from plugin where id = " << id << ") and attrindex = " << index << ";";
		sqlite::statement defstmt(owner->db, query.str());
		if (!defstmt.execute() || defstmt.eof()) {
			assert(false); // cant find default value = bad
			return 0;
		} else
			return sqlite3_column_int(defstmt.stmt, 0);
	} else {
		return sqlite3_column_int(stmt.stmt, 0);
	}
}

bool plugin::get_connection(connectiondata& result) {
	stringstream query;
	query << "select * from connection where plugin_id = " << id;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return false;

	stmt.stmt >> result;
	return true;
}

//
// pluginparameter
//

pluginparameter::pluginparameter(document* _owner) {
	owner = _owner;
}

bool pluginparameter::update() {
	sqlite::statement stmt(owner->db, get_update_statement<pluginparameterdata>());
	stmt.stmt << *this;
	return stmt.execute();
}


//
// orderlist
//

int song::get_order_length() {
	stringstream query;
	query << "select count(*) from patternorder where song_id = " << id << ";";
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

void song::set_order_length(int length) {
	int lastlength = get_order_length();
	// insert n or delete n
	if (length < lastlength) {
		stringstream query;
		query << "insert into connidxtab (connid) select id from patternorder where song_id = " << id << ";";
		owner->exec_noret(query.str());

		query.str("");
		query << "delete from patternorder where id in (select connid from connidxtab limit -1 offset " << length << ");";
		owner->exec_noret(query.str());

		query.str("");
		query << "delete from connidxtab;";
		owner->exec_noret(query.str());
	} else 
	if (length > lastlength) {
		for (int i = 0; i < length - lastlength; i++) {
			stringstream query;
			query << "insert into patternorder (song_id, pattern_id) values (" << id << ", null);";
			owner->exec_noret(query.str());
		}
	}
}

int song::get_order_pattern_id(int index) {
	stringstream query;
	query << "select pattern_id from patternorder where song_id = " << id << " limit 1 offset " << index << ";";
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

void song::set_order_pattern(int index, int pattern_id) {
	// update at index
	stringstream query;
	query << "insert into connidxtab (connid) select id from patternorder where song_id = " << id << ";";
	owner->exec_noret(query.str());
	query.str("");

	query << "update patternorder set pattern_id = ";
	if (pattern_id != 0) {
		query << pattern_id;
	} else {
		query << "null";
	}
	query << " where id = (select connid from connidxtab limit 1 offset " << index << ");";
	owner->exec_noret(query.str());
	query.str("");

	query << "delete from connidxtab;";
	owner->exec_noret(query.str());
}

void song::get_order_patterns(tableiterator* it) {
	stringstream query;
	query << "select pattern_id from patternorder where song_id = " << id << " order by id;";
	//query << "select id from pattern p join sequenceorder s on p.id = s.pattern_id where s.sequence_id = " << id << ";";
	it->prepare(owner->db, query.str());
}

void song::timeshift_order(int index, int timeshift) {
	if (timeshift == 0) return ;

	// modify order looping points etc
	// support: don't shift loop start if it's the first one
	if (loopbegin > index) {
		if (loopbegin + timeshift < index)
			loopbegin = index;
		else
			loopbegin += timeshift;
	}

	if (loopend >= index) {
		if (loopend + timeshift < index)
			loopend = index;
		else
			loopend += timeshift;
	}
	update();

	// TODO: modify orderlist data here

	// this adds an undoable hint on the undo buffer which tells the mixer to 
	// synchronize the current orderlist playing position from the parameters.
	stringstream query;
	query << "select orderlist_timeshift(" << id << ", " << index << ", " << timeshift << ");";
	owner->exec_noret(query.str());
}

//
// connection
//

connection::connection(document* _owner) {
	owner = _owner;
}

bool connection::update() {
	sqlite::statement stmt(owner->db, get_update_statement<connectiondata>());
	stmt.stmt << *this;
	return stmt.execute();
}

void connection::destroy() {
	stringstream query;
	query << "delete from plugin where id = " << plugin_id << ";";
	//query << "delete from connection where id = " << id << ";";
	//query << "delete from connection where id = " << id << "; delete from plugin where id = " << plugin_id << ";";
	owner->exec_noret(query.str());
}

int connection::get_input_connection_index() {
	stringstream query;
	query << "insert into connidxtab (connid) select id from connection where to_plugin_id = " << to_plugin_id << ";";
	if (!owner->exec_noret(query.str())) return -1;
	query.str("");

	query << "select rowid from connidxtab where connid = " << id << endl;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) {
		// delete from connidxtab?
		return -1;
	}

	int index = sqlite3_column_int(stmt.stmt, 0) - 1;

	query.str("");
	query << "delete from connidxtab;";
	if (!owner->exec_noret(query.str())) return -1;

	return index;
}

int connection::get_output_connection_index() {
	stringstream query;
	query << "insert into connidxtab (connid) select id from connection where from_plugin_id = " << from_plugin_id << ";";
	if (!owner->exec_noret(query.str())) return -1;
	query.str("");

	query << "select rowid from connidxtab where connid = " << id << endl;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) {
		// delete from connidxtab?
		return -1;
	}

	int index = sqlite3_column_int(stmt.stmt, 0) - 1;

	query.str("");
	query << "delete from connidxtab;";
	if (!owner->exec_noret(query.str())) return -1;

	return index;
}

void connection::set_midi_device(const char* name) {
	assert(type == 2);

	mididevice = name;
	update();
}

void connection::set_channels(int _first_input, int _first_output, int inputs, int outputs) {
	assert(type == 0);
	first_input = _first_input;
	first_output = _first_output;
	input_count = inputs;
	output_count = outputs;
	update();
}

void connection::add_event_binding(int sourceparam, int targetgroup, int targettrack, int targetparam) {
	assert(type == 1);
	stringstream query;
	query << "insert into eventconnectionbinding (connection_id, sourceindex, targetparamgroup, targetparamtrack, targetparamcolumn) values (" << id << ", "
		<< sourceparam << ", " << targetgroup << ", " << targettrack << ", " << targetparam << ")";
	
	owner->exec_noret(query.str());
}

void connection::delete_event_binding(int sourceparam, int targetgroup, int targettrack, int targetparam) {
	assert(type == 1);
	stringstream query;
	query << "delete from eventconnectionbinding where connection_id = " << id << " and sourceindex = " << sourceparam << " and targetparamgroup = " << targetgroup << " and targetparamtrack = " << targettrack << " and targetparamcolumn = " << targetparam;
	owner->exec_noret(query.str());
}

int connection::get_event_binding_count() {
	stringstream query;
	query << "select count(*) from eventconnectionbinding where connection_id = " << id;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

void connection::get_event_bindings(tableiterator* it) {
	stringstream query;
	query << "select id from eventconnectionbinding where connection_id = " << id << " order by sourceindex, targetparamgroup, targetparamtrack, targetparamcolumn;";
	it->prepare(owner->db, query.str());
}


//
// pattern
//

pattern::pattern(document* _owner) {
	owner = _owner;
}

void pattern::destroy() {
	stringstream query;
	query << "delete from pattern where id = " << id;
	owner->exec_noret(query.str());
}

// pre-prepared statement
void pattern::insert_value(int pluginid, int group, int track, int column, int time, int value, int meta) {
	sqlite::statement* stmt = &owner->prep_pattern_insert_value;
	sqlite3_bind_int(stmt->stmt, 1, id);
	sqlite3_bind_int(stmt->stmt, 2, pluginid);
	sqlite3_bind_int(stmt->stmt, 3, group);
	sqlite3_bind_int(stmt->stmt, 4, track);
	sqlite3_bind_int(stmt->stmt, 5, column);
	sqlite3_bind_int(stmt->stmt, 6, time);
	sqlite3_bind_int(stmt->stmt, 7, value);
	sqlite3_bind_int(stmt->stmt, 8, meta);
	stmt->execute();
	sqlite3_reset(stmt->stmt);
}
// pre-prepared statement
void pattern::delete_value(int evid) {
	sqlite::statement* stmt = &owner->prep_pattern_delete_value;
	sqlite3_bind_int(stmt->stmt, 1, evid);
	stmt->execute();
	sqlite3_reset(stmt->stmt);
}
// pre-prepared statement
void pattern::update_value(int evid, int time, int value, int meta) {
	sqlite::statement* stmt = &owner->prep_pattern_update_value;
	sqlite3_bind_int(stmt->stmt, 1, evid);
	sqlite3_bind_int(stmt->stmt, 2, time);
	sqlite3_bind_int(stmt->stmt, 3, value);
	sqlite3_bind_int(stmt->stmt, 4, meta);
	stmt->execute();
	sqlite3_reset(stmt->stmt);
}
// pre-prepared statement
void pattern::update_value(int evid, int pluginid, int group, int track, int column, int time, int value, int meta) {
	sqlite::statement* stmt = &owner->prep_pattern_update_value_param;
	sqlite3_bind_int(stmt->stmt, 1, evid);
	sqlite3_bind_int(stmt->stmt, 2, pluginid);
	sqlite3_bind_int(stmt->stmt, 3, group);
	sqlite3_bind_int(stmt->stmt, 4, track);
	sqlite3_bind_int(stmt->stmt, 5, column);
	sqlite3_bind_int(stmt->stmt, 6, time);
	sqlite3_bind_int(stmt->stmt, 7, value);
	sqlite3_bind_int(stmt->stmt, 8, meta);
	stmt->execute();
	sqlite3_reset(stmt->stmt);
}
void pattern::set_event_at(int pluginid, int group, int track, int column, int time, int value, int meta) {
	stringstream query;
	//query << "select id from patternevent where pattern_id = " << id << " and plugin_id = " << pluginid << " and paramgroup = " << group << " and paramtrack = " << track << " and paramcolumn = " << column << " and time = " << time;
	query 
		<< "select e.id from patternevent e"
		<< " inner join pluginparameter pp on pp.id = e.pluginparameter_id"
		<< " inner join parameterinfo i on i.id = pp.parameterinfo_id"
		<< " where e.pattern_id = " << id << " and pp.plugin_id = " << pluginid << " and i.paramgroup = " << group << " and pp.paramtrack = " << track << " and i.paramcolumn = " << column << " and time = " << time;
	sqlite::statement stmt(owner->db, query.str());
	query.str("");
	if (!stmt.execute() || stmt.eof()) {
		query 
			<< "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta) "
			<< " select " << id << ", pp.id, " << time << ", " << value << ", " << meta << " from pluginparameter pp"
			<< " inner join parameterinfo i on i.id = pp.parameterinfo_id"
			<< " where pp.plugin_id = " << pluginid << " and i.paramgroup = " << group << " and pp.paramtrack = " << track << " and i.paramcolumn = " << column << ";";
		sqlite::statement insstmt(owner->db, query.str());
		insstmt.execute();
	} else {
		int eventid = sqlite3_column_int(stmt.stmt, 0);
		query << "update patternevent set value = " << value << ", meta = " << meta << " where id = " << eventid;
		sqlite::statement updstmt(owner->db, query.str());
		updstmt.execute();
	}
}

/*int pattern::get_event_at(int pluginid, int group, int track, int column, int time, int* value, int* meta) {
	stringstream query;
	query << "select value, meta, id from patternevent where pattern_id = " << id << " and plugin_id = " << pluginid << " and paramgroup = " << group << " and paramtrack = " << track << " and paramcolumn = " << column << " and time = " << time;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute()) return -1;
	if (stmt.eof()) return -1;
	*value = sqlite3_column_int(stmt.stmt, 0);
	*meta = sqlite3_column_int(stmt.stmt, 1);
	return sqlite3_column_int(stmt.stmt, 2);
}*/

int pattern::get_event_at(int pluginparameter_id, int time, int* value, int* meta) {
	stringstream query;
	query << "select value, meta, id from patternevent where pattern_id = " << id << " and pluginparameter_id = " << pluginparameter_id << " and time = " << time;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute()) return -1;
	if (stmt.eof()) return -1;
	*value = sqlite3_column_int(stmt.stmt, 0);
	*meta = sqlite3_column_int(stmt.stmt, 1);
	return sqlite3_column_int(stmt.stmt, 2);
}

int pattern::get_event_by_index(int group, int track, int column, int index, int* value, int* meta) {
	stringstream query;
	query << "select value, meta, id from patternevent where pattern_id = " << id << " and paramgroup = " << group << " and paramtrack = " << track << " and paramcolumn = " << column << " order by time limit 1 offset " << index;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute()) return -1;
	if (stmt.eof()) return -1;
    *value = sqlite3_column_int(stmt.stmt, 0);
    *meta = sqlite3_column_int(stmt.stmt, 1);
    return sqlite3_column_int(stmt.stmt, 2);
}

int pattern::get_time_by_index(int group, int track, int column, int index) {
	stringstream query;
	query << "select time from patternevent where pattern_id = " << id << " and paramgroup = " << group << " and paramtrack = " << track << " and paramcolumn = " << column << " order by time limit 1 offset " << index;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute()) return -1;
	if (stmt.eof()) return -1;
    return sqlite3_column_int(stmt.stmt, 0);
}

bool pattern::remove_event_at(int pluginid, int group, int track, int column, int time) {
	stringstream query;
	query 
		<< "delete from patternevent where pattern_id = " << id << " and "
		<< " pluginparameter_id = (select pp.id from pluginparameter pp"
		<< "  inner join parameterinfo i on i.id = pp.parameterinfo_id"
		<< "  where pp.plugin_id = " << pluginid << " and i.paramgroup = " << group << " and pp.paramtrack = " << track << " and i.paramcolumn = " << column
		<< " ) and time = " << time;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute()) return false;
	return true;
}

/*int pattern::get_event_count(int group, int track, int column) {
	stringstream query;
	query << "select count(*) from patternevent where pattern_id = " << id << " and paramgroup = " << group << " and paramtrack = " << track << " and paramcolumn = " << column;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}*/

int pattern::get_event_count() {
	stringstream query;
	query << "select count(*) from patternevent where pattern_id = " << id;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

void pattern::get_events(tableiterator* it) {
	stringstream query;
	query << "select id from patternevent where pattern_id = " << id << " order by time;";

	it->prepare(owner->db, query.str());
}

inline void print_where_patternevent(std::ostream& q, int pluginid, int group, int track, int column) {
	if (pluginid != -1) {
		q 
			<< " and pluginparameter_id in ("
			<< " select pp.id from pluginparameter pp"
			<< " inner join parameterinfo i on i.id = pp.parameterinfo_id"
			<< " where pp.plugin_id = " << pluginid;
		if (group != -1) {
			q << "  and i.paramgroup = "<<group;
			if (track != -1) {
				q << "  and pp.paramtrack = "<<track;
				if (column != -1) {
					q << "  and i.paramcolumn = "<<column;
				}
			}
		}
		q << ")";
	}
}

void pattern::get_events(int pluginid, int group, int track, int column, tableiterator* it) {
	stringstream query;
	query << "select id from patternevent where pattern_id = " << id;
	print_where_patternevent(query, pluginid, group, track, column);
	query << " order by time;";
	it->prepare(owner->db, query.str());
}

void pattern::get_events_unsorted(tableiterator* it) {
	stringstream query;
	query << "select id from patternevent where pattern_id = " << id << ";";

	it->prepare(owner->db, query.str());
}

void pattern::get_events_unsorted(int pluginid, int group, int track, int column, tableiterator* it) {
	stringstream query;
	query << "select id from patternevent where pattern_id = " << id;
	print_where_patternevent(query, pluginid, group, track, column);
	it->prepare(owner->db, query.str());
}

bool pattern::update() {
	sqlite::statement stmt(owner->db, get_update_statement<patterndata>());
	stmt.stmt << *this;
	return stmt.execute();
}


//
// patternevent
//

patternevent::patternevent(document* _owner) {
	owner = _owner;
}

bool patternevent::update() {
	sqlite::statement stmt(owner->db, get_update_statement<patterneventdata>());
	stmt.stmt << *this;
	return stmt.execute();
}

//
// patternformat
//

patternformat::patternformat(document* _owner) {
	owner = _owner;
}

bool patternformat::update() {
	sqlite::statement stmt(owner->db, get_update_statement<patternformatdata>());
	stmt.stmt << *this;
	return stmt.execute();
}

void patternformat::destroy() {
	stringstream query;
	query << "delete from patternformat where id = " << id;
	owner->exec_noret(query.str());
}

void patternformat::get_columns(tableiterator* it) {
	stringstream query;
	query << "select id from patternformatcolumn where patternformat_id = " << id << " order by idx;";//plugin_id, paramgroup, paramtrack, paramcolumn";

	it->prepare(owner->db, query.str());
}

bool patternformat::create_track(int pluginid, int group, int track, std::string const& label, int mute, patternformattrackdata& result) {
	stringstream query;
	query << "insert into patternformattrack (patternformat_id, plugin_id, paramgroup, paramtrack, label, is_muted) values ("
		<< id << ", " << pluginid << ", " << group << ", " << track << ", '" << label << "', " << mute << ");";

	owner->exec_noret(query.str());
	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.patternformat_id = id;
	result.plugin_id = pluginid;
	result.paramgroup = group;
	result.paramtrack = track;
	result.label = label;
	result.is_muted = mute;
	return true;
}

bool patternformat::get_track(int pluginid, int group, int track, patternformattrackdata& result) {
	stringstream query;
	query << "select * from patternformattrack where patternformat_id = " << id << " and plugin_id = " << pluginid << " and paramgroup = " << group << " and paramtrack = " << track;

	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return false; 
	stmt.stmt >> result;
	return true;
}

bool patternformat::get_column(int pluginid, int group, int track, int column, patternformatcolumndata& result) {
	stringstream query;
	query
		<< "select c.* from patternformatcolumn c"
		<< " inner join pluginparameter pp on pp.id = c.pluginparameter_id"
		<< " inner join parameterinfo i on i.id = pp.parameterinfo_id"
		<< " where c.patternformat_id = " << id << " and pp.plugin_id = " << pluginid << " and i.paramgroup = " << group << " and pp.paramtrack = " << track << " and i.paramcolumn = " << column << ";";

	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}
/*
void patternformatcolumn::get_filters(tableiterator* it) {
	stringstream query;
	query << "select id from patternformat where id in (select filterformat_id from patternformatcolumnfilter where patternformatcolumn_id = " << id << ");";

	it->prepare(owner->db, query.str());
}

void patternformatcolumn::add_filter(int patternformat_id) {
	stringstream query;
	query << "insert into patternformatcolumnfilter (patternformatcolumn_id, filterformat_id) values (" << id << ", " << patternformat_id << ");";
	owner->exec_noret(query.str());
}

void patternformatcolumn::remove_filter(int patternformat_id) {
	stringstream query;
	query << "delete from patternformatcolumnfilter where patternformatcolumn_id = " << id << " and filterformat_id = " << patternformat_id << ";";
	owner->exec_noret(query.str());
}*/

bool patternformatcolumn::update() {
	sqlite::statement stmt(owner->db, get_update_statement<patternformatcolumndata>());
	stmt.stmt << *this;
	return stmt.execute();
}

patternformatcolumn::patternformatcolumn(document* _owner) {
	owner = _owner;
}

void patternformatcolumn::destroy() {
	stringstream query;
	query << "delete from patternformatcolumn where id = " << id;
	owner->exec_noret(query.str());

	query.str("");
	query << "update patternformatcolumn set idx = idx - 1 where patternformat_id = "<<patternformat_id<<" and idx > " << idx << ";";
	owner->exec_noret(query.str());
}

patternformattrack::patternformattrack(document* _owner) {
	owner = _owner;
}

bool patternformattrack::update() {
	sqlite::statement stmt(owner->db, get_update_statement<patternformattrackdata>());
	stmt.stmt << *this;
	return stmt.execute();
}

//
// wave
//

midimapping::midimapping(document* _owner) {
	owner = _owner;
}

//
// wave
//

wave::wave(document* _owner) {
	owner = _owner;
}

bool wave::get_wavelevel_by_index(int index, waveleveldata& result) {
	stringstream q;
	q << "select * from wavelevel where wave_id = " << id << " limit 1 offset " << index;

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

int wave::get_wavelevel_count() {
	stringstream q;
	q << "select count(*) from wavelevel where wave_id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

int wave::get_wave_index() {
	stringstream q;
	q << "select count(*) from wave where id < " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

bool wave::update() {
	sqlite::statement stmt(owner->db, get_update_statement<wavedata>());
	stmt.stmt << *this;
	return stmt.execute();
}

void wave::clear() {
	stringstream query;
	query << "delete from wavelevel where wave_id = " << id << ";";
	owner->exec_noret(query.str());
	
	name = "";
	filename = "";
	flags = 0;
	volume = 1.0f;
	update();
}

void wave::destroy() {
	stringstream query;
	query << "delete from wave where id = " << id << ";";
	owner->exec_noret(query.str());
}

void wave::add_envelope() {
	stringstream query;
	query << "insert into envelope (wave_id, attack, decay, sustain, release, subdivision, flags, disabled) values (" << id << ", 0, 0, 0, 0, 0, 0, 1);";

	owner->exec_noret(query.str());

	armstrong::storage::envelope env(owner);
	armstrong::storage::envelopepointdata envpt;

	get_envelope_by_index(get_envelope_count() - 1, env);
	env.insert_point(0, 0, 0, envpt);
	env.insert_point(65535, 65535, 0, envpt);
}

int wave::get_envelope_count() {
	stringstream q;
	q << "select count(*) from envelope where wave_id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

void wave::get_envelopes(tableiterator* it) {
	stringstream query;
	query << "select id from envelope where wave_id = " << id << ";";

	it->prepare(owner->db, query.str());
}

void wave::get_envelope_by_index(int index, envelopedata& result) {
	stringstream query;
	query << "select * from envelope where wave_id = " << id << " limit 1 offset " << index << ";";

	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return ;
	stmt.stmt >> result;
	return ;
}

void wave::set_stereo(bool state) {
	flags = 
		(flags & ~(1 << 3)) | 
		((state) ? (1 << 3) : 0);

	// TODO: convert all wavelevels to current mode
	update();
}

//
// wavelevel
//

wavelevel::wavelevel(document* _owner) {
	owner = _owner;
}

bool wavelevel::update() {
	sqlite::statement stmt(owner->db, get_update_statement<waveleveldata>());
	stmt.stmt << *this;
	return stmt.execute();
}

extern "C" void wavelevel_insert_samples(sqlite3_context* ctx, int, sqlite3_value** row) {
	document* self = (document*)sqlite3_user_data(ctx);
	int wavelevel_id = sqlite3_value_int(row[0]);
	int offset = sqlite3_value_int(row[1]);
	int length = sqlite3_value_int(row[2]);

	const unsigned char* filename = sqlite3_value_text(row[3]);
	unsigned char* buffer = 0;
	int filesize = 0;
	self->finalize_undo_file((const char*)filename, &filesize, &buffer);
	self->wavelevel_insert_bytes(wavelevel_id, offset, length, buffer);
	delete[] buffer;
}

extern "C" void wavelevel_replace_samples(sqlite3_context* ctx, int, sqlite3_value** row) {
	document* self = (document*)sqlite3_user_data(ctx);
	int wavelevel_id = sqlite3_value_int(row[0]);
	int offset = sqlite3_value_int(row[1]);
	int length = sqlite3_value_int(row[2]);

	const unsigned char* filename = sqlite3_value_text(row[3]);
	unsigned char* buffer;
	int filesize;
	self->finalize_undo_file((const char*)filename, &filesize, &buffer);
	assert(filesize == length);
	self->wavelevel_replace_bytes(wavelevel_id, offset, length, buffer);
	delete[] buffer;
}

extern "C" void wavelevel_delete_samples(sqlite3_context* ctx, int, sqlite3_value** row) {
	document* self = (document*)sqlite3_user_data(ctx);
	int wavelevel_id = sqlite3_value_int(row[0]);
	int offset = sqlite3_value_int(row[1]);
	int length = sqlite3_value_int(row[2]);

	self->wavelevel_delete_bytes(wavelevel_id, offset, length);
}

extern "C" void wavelevel_delete_file(sqlite3_context* ctx, int, sqlite3_value** row) {
	document* self = (document*)sqlite3_user_data(ctx);
	int wavelevel_id = sqlite3_value_int(row[0]);

	self->wavelevel_delete_file(wavelevel_id);
}

void wavelevel::insert_chunks(std::vector<wavelevel::chunk>& chunks, int srcoffset) {
	armstrong::storage::song song(owner);
	owner->get_song(song);
	armstrong::storage::wave wave(owner);
	song.get_wave_by_wavelevel_id(id, wave);

	int srclength = 0;
	for (std::vector<chunk>::iterator i = chunks.begin(); i != chunks.end(); ++i)
		srclength += i->numsamples;

	int destchannels = (wave.flags & (1 << 3)) ? 2 : 1;
	int bytes_per_sample = destchannels * (int)sizeFromWaveFormat(format);
	int resultlength = srclength * bytes_per_sample;
	unsigned char* resultbuffer = new unsigned char[resultlength];
	assert(resultbuffer != 0);
	unsigned char* writebuffer = resultbuffer;

	for (std::vector<chunk>::iterator i = chunks.begin(); i != chunks.end(); ++i) {
		int srcbuffercount = (int)i->buffer.size();
		assert(srcbuffercount != 0);
		void* srcbuffer = i->buffer[0].get();
		CopySamples((void*)srcbuffer, writebuffer, i->numsamples, 1, format, 1, destchannels, 0, 0);
		if (destchannels == 2) {
			srcbuffer = i->buffer[1 % srcbuffercount].get();
			CopySamples((void*)srcbuffer, writebuffer, i->numsamples, 1, format, 1, destchannels, 0, 1);
		}
		writebuffer += bytes_per_sample * i->numsamples;
	}

	owner->wavelevel_insert_bytes(id, srcoffset * bytes_per_sample, resultlength, resultbuffer);

	delete[] resultbuffer;
	samplecount = samplecount + srclength;
	update();
}

void wavelevel::insert_sample_range(const void* srcbuffer, int srcoffset, int srclength, int srcformat, int srcchannels) {

	armstrong::storage::song song(owner);
	owner->get_song(song);
	armstrong::storage::wave wave(owner);
	song.get_wave_by_wavelevel_id(id, wave);

	int destchannels = (wave.flags & (1 << 3)) ? 2 : 1;
	int bytes_per_sample = destchannels * (int)sizeFromWaveFormat(format);
	int resultlength = srclength * bytes_per_sample;
	unsigned char* resultbuffer = new unsigned char[resultlength];
	assert(resultbuffer != 0);

	CopySamples((void*)srcbuffer, resultbuffer, srclength, srcformat, format, srcchannels, destchannels, 0, 0);
	if (destchannels == 2) {
		CopySamples((void*)srcbuffer, resultbuffer, srclength, srcformat, format, srcchannels, destchannels, srcchannels - 1, 1);
	}

	owner->wavelevel_insert_bytes(id, srcoffset * bytes_per_sample, resultlength, resultbuffer);

	delete[] resultbuffer;

	// adjust loop points to inserted sample range

	samplecount = samplecount + srclength;
	if (srcoffset < beginloop) {
		// inserting before the loop
		beginloop += srclength;
		endloop += srclength;
	} else if (srcoffset >= beginloop && srcoffset <= endloop) {
		// inserting inside the loop
		endloop += srclength;
	}
	update();

	// adjust slice positions to inserted sample range
	std::vector<int> slices;
	get_slices(slices);
	bool adjusted_slices = false;
	for (std::vector<int>::iterator i = slices.begin(); i != slices.end(); ++i) {
		if (*i >= srcoffset) {
			*i += srclength;
			adjusted_slices = true;
		}
	}
	if (adjusted_slices) set_slices(slices);

}

void wavelevel::replace_sample_range(const void* srcbuffer, int srcoffset, int srclength, int srcformat, int srcchannels) {

	armstrong::storage::song song(owner);
	owner->get_song(song);
	armstrong::storage::wave wave(owner);
	song.get_wave_by_wavelevel_id(id, wave);

	int destchannels = (wave.flags & (1 << 3)) ? 2 : 1;
	int bytes_per_sample = destchannels * (int)sizeFromWaveFormat(format);
	int resultlength = srclength * bytes_per_sample;

	unsigned char* resultbuffer = new unsigned char[resultlength];
	assert(resultbuffer != 0);

	CopySamples((void*)srcbuffer, resultbuffer, srclength, srcformat, format, srcchannels, destchannels, 0, 0);
	if (destchannels == 2) {
		CopySamples((void*)srcbuffer, resultbuffer, srclength, srcformat, format, srcchannels, destchannels, srcchannels - 1, 1);
	}

	owner->wavelevel_replace_bytes(id, srcoffset * bytes_per_sample, resultlength, resultbuffer);

	delete[] resultbuffer;
}

void wavelevel::delete_sample_range(int offset, int length) {
	// convert offset and length to bytes
	armstrong::storage::song song(owner);
	owner->get_song(song);
	armstrong::storage::wave wave(owner);
	song.get_wave_by_wavelevel_id(id, wave);

	int destchannels = (wave.flags & (1 << 3)) ? 2 : 1;
	int bytes_per_sample = destchannels * (int)sizeFromWaveFormat(format);
	int resultlength = length * bytes_per_sample;
	int resultoffset = offset * bytes_per_sample;

	owner->wavelevel_delete_bytes(id, resultoffset, resultlength);

	// adjust loop points
	samplecount = samplecount - length;
	if (offset < beginloop && offset + length <= beginloop) {
		// deleting before the loop
		beginloop -= length;
		endloop -= length;
	} else if (offset < beginloop && offset + length <= endloop) {
		// deleting before to inside the loop
		beginloop = offset;
		endloop -= length;
	}else if (offset < beginloop && offset + length > endloop) {
		// deleting the entire loop
		beginloop = offset;
		endloop = offset;
	} else if (offset >= beginloop && offset + length < endloop) {
		// deleting inside the loop
		endloop -= length;
	} else if (offset >= beginloop && offset + length >= endloop) {
		// deleting inside and beyond the loop
		endloop = offset;
	}
	update();

	// adjust slice positions
	std::vector<int> slices;
	get_slices(slices);

	bool adjusted_slices = false;
	for (std::vector<int>::iterator i = slices.begin(); i != slices.end(); ) {
		if (*i >= offset && *i < offset + length) {
			slices.erase(i);
			adjusted_slices = true;
		} else if (*i >= offset + length) {
			*i -= length;
			adjusted_slices = true;
			++i;
		} else
			++i;
	}
	if (adjusted_slices) set_slices(slices);
}

int wavelevel::get_sample_count() {
	int length = owner->wavelevel_get_byte_count(id);
	int bytes_per_sample = get_bytes_per_sample();

	int count = length / bytes_per_sample;
	return count;
	//assert(count == samplecount);
	//return samplecount;
}

int wavelevel::get_bytes_per_sample() {
	armstrong::storage::song song(owner);
	owner->get_song(song);
	armstrong::storage::wave wave(owner);
	song.get_wave_by_wavelevel_id(id, wave);

	int destchannels = (wave.flags & (1 << 3)) ? 2 : 1;
	return destchannels * (int)sizeFromWaveFormat(format);
}

void wavelevel::destroy() {
	stringstream query;
	query << "delete from wavelevel where id = " << id << ";";
	owner->exec_noret(query.str());
}

int wavelevel::get_slices_count() {
	stringstream q;
	q << "select count(*) from slice where wavelevel_id = " << id << ";";

	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return 0;
	return sqlite3_column_int(stmt.stmt, 0);
}

void wavelevel::set_slices(const std::vector<int>& slices) {
	// [X] approach 1: delete all existing slices and insert the new ones
	// [ ] approach 2: compare existing with new slices and insert/delete selectively
	// [ ] approach 3: different type of slice api using insert_slice()/delete_slice()
	stringstream query;
	query << "delete from slice where wavelevel_id = " << id << ";";

	for (std::vector<int>::const_iterator i = slices.begin(); i != slices.end(); ++i) {
		query << "insert into slice (wavelevel_id, sampleoffset) values (" << id << ", " << *i << ");";
	}

	owner->exec_noret(query.str());
}

void wavelevel::get_slices(std::vector<int>& slices) {
	stringstream query;
	query << "select id, sampleoffset from slice where wavelevel_id = " << id << " order by sampleoffset;";

	tableiterator i;
	i.prepare(owner->db, query.str());
	
	while (!i.eof()) {
		slices.push_back(sqlite3_column_int(i.stmt, 1));
		i.next();
	}
	i.destroy();
}


//
// envelope
//

envelope::envelope(document* _owner) {
	owner = _owner;
}

bool envelope::update() {
	sqlite::statement stmt(owner->db, get_update_statement<envelopedata>());
	stmt.stmt << *this;
	return stmt.execute();
}

void envelope::destroy() {
	stringstream query;
	query << "delete from envelope where id = " << id << ";";
	owner->exec_noret(query.str());
}

int envelope::get_envelope_point_count() {
	stringstream query;
	query << "select count(*) from envelopepoint where envelope_id = " << id << ";";
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return -1;
	return sqlite3_column_int(stmt.stmt, 0);
}

void envelope::get_envelope_points(tableiterator* it) {
	stringstream query;
	query << "select id from envelopepoint where envelope_id = " << id << ";";
	it->prepare(owner->db, query.str());
}

void envelope::insert_point(int x, int y, int flags, envelopepointdata& result) {
	stringstream query;
	query << "insert into envelopepoint (envelope_id, x, y, flags) values (" << id << ", " << x << ", " << y << ", " << flags << ");";
	owner->exec_noret(query.str());

	result.id = (int)sqlite3_last_insert_rowid(owner->db);
	result.envelope_id = id;
	result.x = 0;
	result.y = 0;
	result.flags = 0;
}

void envelope::get_point(int index, envelopepointdata& result) {
	stringstream query;
	query << "select * from envelopepoint where envelope_id = " << id << " order by id limit 1 offset " << index;
	sqlite::statement stmt(owner->db, query.str());
	if (!stmt.execute() || stmt.eof()) return ;
	stmt.stmt >> result;

}


//
// envelopepoint
//

envelopepoint::envelopepoint(document* _owner) {
	owner = _owner;
}

bool envelopepoint::update() {
	sqlite::statement stmt(owner->db, get_update_statement<envelopepointdata>());
	stmt.stmt << *this;
	return stmt.execute();
}

void envelopepoint::destroy() {
	stringstream query;
	query << "delete from envelopepoint where id = " << id << ";";
	owner->exec_noret(query.str());
}


//
// plugingroup
//

plugingroup::plugingroup(document* _owner) {
	owner = _owner;
}

bool plugingroup::update() {
	sqlite::statement stmt(owner->db, get_update_statement<plugingroupdata>());
	stmt.stmt << *this;
	return stmt.execute();
}

void plugingroup::destroy() {
	stringstream query;
	query << "delete from plugingroup where id = " << id << ";";
	owner->exec_noret(query.str());
}

void plugingroup::get_plugins(tableiterator* it) {
	stringstream query;
	// retreive all plugins except those associated with a sequence
	query << "select id from plugin where plugingroup_id = " << id << ";"; // and id not in (select plugin_id from sequence)";
	it->prepare(owner->db, query.str());
}


}
}
