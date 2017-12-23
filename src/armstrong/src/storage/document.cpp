#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#elif defined(POSIX)
#include <sys/types.h>
#include <unistd.h>
#endif
#include <iostream>
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include "database.h"
#include "document.h"

#if !defined(_WIN32_)
#define _unlink unlink
#endif

// bump this whenever the database format changes and add an appropriate upgrade script
const int armstrong_document_version = 27;

const char* upgrade_scripts[] = { 
	/*
	  version 0->1:
		<Megz> that lets songs load.. but some stuff wont work
		<Megz> like trackswap, which uses index range to check "matches" between columns
		<Megz> the idx will get remade if you toggle the columns off+on in the pattern format editor
		<Megz> have to toggle every one though :P
	*/
	"alter table loaddb.patternformatcolumn add column idx integer;"
	"update loaddb.patternformatcolumn set idx = 0;"
,
	// version 1->2:
	"alter table loaddb.patternevent add column meta integer;"
	"update loaddb.patternevent set meta = 0;"
,
	// version 2->3:
	"alter table loaddb.connection add column first_input integer;"
	"alter table loaddb.connection add column input_count integer;"
	"alter table loaddb.connection add column first_output integer;"
	"alter table loaddb.connection add column output_count integer;"
	"alter table loaddb.connection add column mididevice varchar(64);"
	"update loaddb.connection set first_input = (select first_input from loaddb.audioconnection where connection_id = loaddb.connection.id) where type = 0;"
	"update loaddb.connection set input_count = (select input_count from loaddb.audioconnection where connection_id = loaddb.connection.id) where type = 0;"
	"update loaddb.connection set first_output = (select first_output from loaddb.audioconnection where connection_id = loaddb.connection.id) where type = 0;"
	"update loaddb.connection set output_count = (select output_count from loaddb.audioconnection where connection_id = loaddb.connection.id) where type = 0;"
	"update loaddb.connection set mididevice = '' where type != 2;"
	"update loaddb.connection set mididevice = (select mididevice from loaddb.midiconnection where connection_id = loaddb.connection.id) where type = 2;"
,
	// version 3->4:
	"alter table loaddb.patternformat add column scroller_width;"
	"update loaddb.patternformat set scroller_width = -1;"
,
	// version 4->5:
	"alter table loaddb.song add column bpm;"
	"alter table loaddb.song add column tpb;"
	"alter table loaddb.plugin add column latency;"
	"update loaddb.song set bpm = 126;"
	"update loaddb.song set tpb = 4;"
	"update loaddb.plugin set latency = -1;"
,
	// version 5->6:
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramtrack, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 2, 0, 1, 'Transpose', 'Transpose', 0, 0, 1, 156, 0, 0 from loaddb.plugininfo where uri = '@zzub.org/sequence/pattern';"
,
	// version 6->7:
	"delete from loaddb.parameterinfo where plugininfo_id = (select id from loaddb.plugininfo where name = 'Master') and paramgroup = 1 and paramtrack = 0 and paramcolumn = 1;"
	"delete from loaddb.parameterinfo where plugininfo_id = (select id from loaddb.plugininfo where name = 'Master') and paramgroup = 1 and paramtrack = 0 and paramcolumn = 2;"
	"delete from loaddb.pluginparameter where plugin_id = (select id from loaddb.plugin where name = 'Master') and paramgroup = 1 and paramtrack = 0 and paramcolumn = 1;"
	"delete from loaddb.pluginparameter where plugin_id = (select id from loaddb.plugin where name = 'Master') and paramgroup = 1 and paramtrack = 0 and paramcolumn = 2;"
	"delete from loaddb.patternformatcolumn where plugin_id = (select id from loaddb.plugin where name = 'Master') and paramgroup = 1 and paramtrack = 0 and paramcolumn = 1;"
	"delete from loaddb.patternformatcolumn where plugin_id = (select id from loaddb.plugin where name = 'Master') and paramgroup = 1 and paramtrack = 0 and paramcolumn = 2;"
	"delete from loaddb.patternevent where plugin_id = (select id from loaddb.plugin where name = 'Master') and paramgroup = 1 and paramtrack = 0 and paramcolumn = 1;"
	"delete from loaddb.patternevent where plugin_id = (select id from loaddb.plugin where name = 'Master') and paramgroup = 1 and paramtrack = 0 and paramcolumn = 2;"
	"alter table loaddb.song add column machineview_x real;"
	"alter table loaddb.song add column machineview_y real;"
	"update loaddb.song set machineview_x = 0.0;"
	"update loaddb.song set machineview_y = 0.0;"
,
	// version 7->8:
	"alter table loaddb.patternorder add column song_id int;"
	"update loaddb.patternorder set song_id = 1;"
,
	// version 8->9:
	"create table loaddb.slice (id integer primary key, wavelevel_id integer, sampleoffset integer);"
,
	// version 9->10:
	"delete from loaddb.parameterinfo where plugininfo_id in (select id from loaddb.plugininfo where uri='@zzub.org/connection/audio');"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) select id, 2, 0, 'Volume', 'Volume (0=0%, 4000=100%)', 2, 3, 0, 32768, 65535, 16384 from loaddb.plugininfo where uri='@zzub.org/connection/audio';"
	"alter table loaddb.pluginparameter add column parameterinfo_id int;"
	"alter table loaddb.patternevent add column pluginparameter_id int;"
	"alter table loaddb.patternformatcolumn add column pluginparameter_id int;"
	"update loaddb.pluginparameter set parameterinfo_id = ("
	" select i.id from loaddb.parameterinfo i"
	" inner join loaddb.plugin p on p.plugininfo_id = i.plugininfo_id and p.id = loaddb.pluginparameter.plugin_id"
	" where "
	"  loaddb.pluginparameter.paramgroup = i.paramgroup and "
	"  loaddb.pluginparameter.paramcolumn = i.paramcolumn"
	" );"
	"select ensure_plugin_parameters();"
	"update loaddb.patternevent set pluginparameter_id = ("
	" select pp.id from loaddb.pluginparameter pp"
	" inner join loaddb.parameterinfo i on pp.parameterinfo_id = i.id"
	" where "
	"  loaddb.patternevent.plugin_id = pp.plugin_id and"
	"  loaddb.patternevent.paramgroup = i.paramgroup and"
	"  loaddb.patternevent.paramtrack = pp.paramtrack and"
	"  loaddb.patternevent.paramcolumn = i.paramcolumn"
	" );"
	"update loaddb.patternformatcolumn set pluginparameter_id = ("
	" select pp.id from loaddb.pluginparameter pp"
	" inner join loaddb.parameterinfo i on pp.parameterinfo_id = i.id"
	" where "
	"  loaddb.patternformatcolumn.plugin_id = pp.plugin_id and"
	"  loaddb.patternformatcolumn.paramgroup = i.paramgroup and"
	"  loaddb.patternformatcolumn.paramtrack = pp.paramtrack and"
	"  loaddb.patternformatcolumn.paramcolumn = i.paramcolumn"
	" );"
,
	// version 10->11:
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 0, 0, 'Hard Mute', 'Hard Mute Plugin', 2, 1, 0, 1, 255, 0 from loaddb.plugininfo;"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 0, 1, 'Hard Bypass', 'Hard Bypass Plugin', 2, 1, 0, 1, 255, 0 from loaddb.plugininfo;"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 0, 2, 'Soft Mute', 'Soft Mute Plugin', 0, 1, 0, 1, 255, 255 from loaddb.plugininfo;"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 0, 3, 'Soft Bypass', 'Soft Bypass Plugin', 0, 1, 0, 1, 255, 255 from loaddb.plugininfo;"

	"insert into loaddb.pluginparameter (plugin_id, parameterinfo_id, paramtrack, value) "
	"	select p.id, i.id, 0, 0 from loaddb.plugin p inner join loaddb.parameterinfo i on i.plugininfo_id = p.plugininfo_id and i.paramgroup = 0 and i.paramcolumn = 0;"
	"insert into loaddb.pluginparameter (plugin_id, parameterinfo_id, paramtrack, value) "
	"	select p.id, i.id, 0, 0 from loaddb.plugin p inner join loaddb.parameterinfo i on i.plugininfo_id = p.plugininfo_id and i.paramgroup = 0 and i.paramcolumn = 1;"
	"insert into loaddb.pluginparameter (plugin_id, parameterinfo_id, paramtrack, value) "
	"	select p.id, i.id, 0, 255 from loaddb.plugin p inner join loaddb.parameterinfo i on i.plugininfo_id = p.plugininfo_id and i.paramgroup = 0 and i.paramcolumn = 2;"
	"insert into loaddb.pluginparameter (plugin_id, parameterinfo_id, paramtrack, value) "
	"	select p.id, i.id, 0, 255 from loaddb.plugin p inner join loaddb.parameterinfo i on i.plugininfo_id = p.plugininfo_id and i.paramgroup = 0 and i.paramcolumn = 3;"
,
	// version 11->12
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 4, 0, 'Note Meta', 'Note Metaparameter', 512, 4, 0, 0, 0, 0 from loaddb.plugininfo where id in (select plugininfo_id from loaddb.parameterinfo where type = 0);"
	"insert into loaddb.pluginparameter (plugin_id, parameterinfo_id, paramtrack, value) "
	"	select p.id, i.id, 0, 0 from loaddb.plugin p inner join loaddb.parameterinfo i on i.plugininfo_id = p.plugininfo_id and i.paramgroup = 4 and i.paramcolumn = 0;"
,
	// version 12->13:
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 2, 2, 'Offset', 'Offset', 0, 3, 0, 32768, 65535, 65535 from loaddb.plugininfo where uri = '@zzub.org/sequence/pattern';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 2, 3, 'Length', 'Length', 2, 3, 0, 32768, 65535, 0 from loaddb.plugininfo where uri = '@zzub.org/sequence/pattern';"
	"select ensure_plugin_parameters();"
,
	// version 13->14:
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 6, 'Seed', 'Random Number Generator Seed (used with type = random)', 2, 3, 0, 65534, 65535, 32767 from loaddb.plugininfo where uri = '@zzub.org/peer/lfo';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 3, 0, 'LfoOut', 'LFO Output', 0, 3, 0, 65534, 65535, 65535 from loaddb.plugininfo where uri = '@zzub.org/peer/lfo';"
	"update loaddb.parameterinfo set maxvalue = 4 where paramgroup = 1 and paramcolumn = 5 and plugininfo_id in (select id from loaddb.plugininfo where uri='@zzub.org/peer/lfo');"

	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 3, 0, 'LfoOut', 'LFO Output', 0, 3, 0, 100, 65535, 0 from loaddb.plugininfo where uri = '@trac.zeitherrschaft.org/aldrin/lunar/controller/lfo;1';"

	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 1, 'Seed', 'Random Number Generator Seed (used with rnd)', 2, 3, 0, 65534, 65535, 32767 from loaddb.plugininfo where uri = '@zzub.org/peer/wordvalue';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 2, 'Threshold', 'Skips n values for every outputted value', 2, 2, 0, 254, 255, 0 from loaddb.plugininfo where uri = '@zzub.org/peer/wordvalue';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 3, 0, 'ValueOut', 'Value Output', 0, 3, 0, 65534, 65535, 65535 from loaddb.plugininfo where uri = '@zzub.org/peer/wordvalue';"
	"update loaddb.parameterinfo set maxvalue = 6 where paramgroup = 2 and paramcolumn = 0 and plugininfo_id in (select id from loaddb.plugininfo where uri='@zzub.org/peer/wordvalue');"

	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 3, 0, 'Out', 'Output', 0, 3, 0, 65534, 65535, 65535 from loaddb.plugininfo where uri = '@zzub.org/peer/signal';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 3, 0, 'AdsrOut', 'ADSR Output', 0, 3, 0, 65534, 65535, 65535 from loaddb.plugininfo where uri = '@zzub.org/peer/adsr';"
,
	// version 14->15:
	"alter table loaddb.pluginparameter add column interpolator int;"
	"update loaddb.pluginparameter set interpolator = 0;"
,
	// version 15->16:
	"update loaddb.plugininfo set maxtracks = 64 where uri='@zzub.org/miditracker;1';"
,
	// version 16->17:
	"update loaddb.parameterinfo set flags = flags | 16 where plugininfo_id in (select id from loaddb.plugininfo where uri like '@zzub.org/buzz2zzub%') and paramgroup = 2 and (description like '%olume%' or description like '%elocity%') ;"
	"update loaddb.parameterinfo set flags = flags | 16 where plugininfo_id in (select id from loaddb.plugininfo where uri = '@zzub.org/miditracker;1') and paramgroup = 2 and paramcolumn = 1;"
	"update loaddb.parameterinfo set flags = flags | 16 where plugininfo_id in (select id from loaddb.plugininfo where uri = '@zzub.org/notegen') and paramgroup = 2 and paramcolumn = 2;"
,
	// version 17->18:
	"update loaddb.plugininfo set flags = flags | 32768 where uri = '@zzub.org/master';"
	"update loaddb.plugininfo set flags = flags | 32768 where uri = '@zzub.org/sequence/sequence';"
	"update loaddb.plugininfo set flags = flags | 32768 where uri = '@zzub.org/output16';"
,
	// version 18->19:
	"update loaddb.parameterinfo set maxvalue = 9 where paramgroup = 2 and paramcolumn = 0 and plugininfo_id in (select id from loaddb.plugininfo where uri='@zzub.org/peer/wordvalue');"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 3, 'Allow Min', 'Min value in allowed output range', 2, 3, 0, 65534, 65535, 0 from loaddb.plugininfo where uri = '@zzub.org/peer/wordvalue';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 4, 'Allow Max', 'Max value in allowed output range', 2, 3, 0, 65534, 65535, 65534 from loaddb.plugininfo where uri = '@zzub.org/peer/wordvalue';"
,
	// version 19->20:
	"update loaddb.plugininfo set flags = flags | 8388608 where uri='@zzub.org/notegen';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 15, 'Allow Min', 'Min allowed input note', 2, 2, 0, 119, 255, 0 from loaddb.plugininfo where uri = '@zzub.org/notegen';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 16, 'Allow Max', 'Max allowed input note', 2, 2, 0, 119, 255, 119 from loaddb.plugininfo where uri = '@zzub.org/notegen';"
,
	// version 20-21
	"create table loaddb.plugingroup (id integer primary key, song_id integer, name varchar(256), parent_plugingroup_id int, position_x real, position_y real);"
	"alter table loaddb.plugin add column plugingroup_id integer;"
	"update loaddb.plugin set plugingroup_id = null;"
,
	// version 21-22
	"update loaddb.parameterinfo set minvalue = 1 where paramgroup = 1 and paramcolumn = 0 and plugininfo_id in (select id from loaddb.plugininfo where uri='@zzub.org/sequence/sequence');"
	"update loaddb.parameterinfo set minvalue = 1 where paramgroup = 1 and paramcolumn = 1 and plugininfo_id in (select id from loaddb.plugininfo where uri='@zzub.org/sequence/sequence');"

	// this update belongs to a much earlier version, but wasnt discovered until it became critical when it could exist in newer versions as well
	"update loaddb.plugininfo set input_count = 0, output_count = 0 where uri = '@zzub.org/connection/audio';"

,
	// version 22-23:
	"update loaddb.plugininfo set flags = flags | 536870912 where uri = '@zzub.org/midiinput;1';"
	"update loaddb.plugininfo set flags = flags | 536870912 where uri = '@zzub.org/connection/midi';"
,
	// version 23->24
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 2, 'Swing', 'Swing Offset (1-99%, 0=sync to timesource, 50=no swing)', 2, 2, 0, 99, 255, 0 from loaddb.plugininfo where uri = '@zzub.org/sequence/pattern';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 2, 'Swing', 'Swing Offset (1-99%, 50=no swing)', 2, 2, 1, 99, 255, 50 from loaddb.plugininfo where uri = '@zzub.org/sequence/sequence';"
,
	// version 24->25
	"alter table loaddb.song add column swing real;"
	"update loaddb.song set swing = 0.5;"
,
	// version 25->26
	"update loaddb.patternorder set pattern_id = null where pattern_id = 0;"
,
	// version 26->27
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 3, 'Swing Ticks', 'Ticks Per Swing (1-32, 0=sync to timesource)', 2, 2, 0, 32, 255, 0 from loaddb.plugininfo where uri = '@zzub.org/sequence/pattern';"
	"insert into loaddb.parameterinfo (plugininfo_id, paramgroup, paramcolumn, name, description, flags, type, minvalue, maxvalue, novalue, defaultvalue) "
	"	select id, 1, 3, 'Swing Ticks', 'Ticks Per Swing (1-32)', 2, 2, 1, 32, 255, 4 from loaddb.plugininfo where uri = '@zzub.org/sequence/sequence';"
	"alter table loaddb.song add column swingticks int;"
	"update loaddb.song set swingticks = tpb;"

};

// max_upgrade_version must be equal to armstrong_document_version
const int max_upgrade_version = sizeof(upgrade_scripts) / sizeof(char*);

using namespace std;
using namespace dbgenpp;

/*

documentgenimpl.h has generated implementations of:
	- sql triggers for insert/update/delete on each table
	- the triggers write to the undolog and invoke a native callback
	- generated native callbacks call document::invoke_listeners()
	- sqlite3_column_* -> document data objects

the following special cases are hardcoded in the generated trigger actions:

before delete connection:
	- adjust track indices for group 0 in patterns and states:
	-> delete from patternevent where group = 0 and track = connindex
	-> update patternevent set track = track - 1 where group = 0 and track > connindex
	-> delete from pluginparameters .. same as above

before update plugin:
	- if trackcount, remove overflowing events from patterns:
	-> delete from patternevent where group = 2 and track > trackcount

before update pattern:
	- if pattern length changes
	-> delete from patternevent where time > pattern.length

inserting and removing sample data:
	- we dont use blobs for sample data, rather files are (will be) used

*/

extern "C" void wavelevel_insert_samples(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void wavelevel_replace_samples(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void wavelevel_delete_samples(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void wavelevel_delete_file(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void undoredo_enabled_callback(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void noteutil_buzz_note_transpose(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void noteutil_buzz_note_base(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void noteutil_buzz_to_midi_note(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void noteutil_midi_to_buzz_note(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void scale_param(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void scale_param_range(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void scale_param_point(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void undoredo_add_query(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void zzub_print(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void orderlist_timeshift(sqlite3_context* ctx, int, sqlite3_value** row);
extern "C" void ensure_plugin_parameters(sqlite3_context* ctx, int, sqlite3_value** row);

/*int get_next_table_id(sqlite3* db, std::string tablename) {
	sqlite::statement stmt(db, "select max(id) from " + tablename);
	if (!stmt.execute()) return -1;
	if (stmt.eof()) return -1;
	return sqlite3_column_int(stmt.stmt, 0) + 1;
}
*/
void save_binary(std::string filename, const unsigned char* image, int size) {
	std::ofstream fs;
	fs.open(filename.c_str(), std::ios::out | std::ios::binary);
	fs.write((const char*)image, size);
	fs.close();
}

void load_binary(std::string filename, unsigned char** image, int* size) {
	std::ifstream fs;
	fs.open(filename.c_str(), std::ios::in | std::ios::binary);
	if (!fs) return ;
	fs.seekg (0, ios::end);
	*size = (int)fs.tellg();
	*image = new unsigned char[*size];
	assert(*image != 0);
	fs.seekg (0, ios::beg);
	fs.read((char*)*image, *size);
	fs.close();
}

namespace armstrong {

namespace storage {

#include "armdb_types_cpp.h"

int document::instance_count = 0;

document::document() {
	db = 0;
	last_history_id = 0;
	current_history_id = 0;
	undoredo_enabled = true;
	listener_enabled = true;

	unique_session_id.second = instance_count++;
#if defined(POSIX)
	unique_session_id.first = getpid();
#elif defined(_WIN32)
	unique_session_id.first = GetCurrentProcessId();
#else
	unique_session_id.first = rand();
#endif
}

bool document::open(string dbfile) {
	char *errmsg = 0;

	int libversion = sqlite3_libversion_number();
	assert(libversion >= 3006018);

	int rc = sqlite3_open(dbfile.c_str(), &db);
	if (rc) {
		cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
		sqlite3_close(db);
		return false;
	}

//	exec_noret("PRAGMA default_cache_size = 50000;");
//	exec_noret("PRAGMA cache_size = 50000;");
	exec_noret("PRAGMA recursive_triggers = on;");

	create_callbacks(db, this);
	create_history_tables();

	stringstream query;
	create_tables(query, "");
	query << "create temp table patternevent_temp (evid integer, time integer, value integer, meta integer);" << endl;
	query << "create index patternevent_index on patternevent (pattern_id, pluginparameter_id, time);" << endl;
	//query << "create index patternformatcolumn_index on patternformatcolumn (patternformat_id, pluginparameter_id);" << endl;
	//query << "create index pluginparameter_index on pluginparameter (plugin_id, parameterinfo_id);" << endl;
	//query << "create index parameterinfo_index on parameterinfo (plugininfo_id);" << endl;
	//query << "create index plugin_index on plugin (plugininfo_id);" << endl;
	query << "create temp table connidxtab (connid integer);" << endl;
	create_triggers(db, query);

	exec_noret(query.str());
	create_sql_functions();
	create_prepared_statements();

//	document_event_data ev;
//	ev.type = event_type_open_document;
//	notify_listeners(&ev);

	return true;
}

void document::create_prepared_statements() {
	std::stringstream q;
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta) "
	  << " select ?1, pp.id, ?6, ?7, ?8 from pluginparameter pp"
	  << " inner join parameterinfo i on i.id = pp.parameterinfo_id"
	  << " where pp.plugin_id = ?2 and i.paramgroup = ?3 and pp.paramtrack = ?4 and i.paramcolumn = ?5;";
	prep_pattern_insert_value.prepare(db, q.str());

	q.str("");
	q << "delete from patternevent where id = ?;";
	prep_pattern_delete_value.prepare(db, q.str());

	q.str("");
	q << "update patternevent set time = ?2, value = ?3, meta = ?4 where id = ?1;";
	prep_pattern_update_value.prepare(db, q.str());

	q.str("");
	q << "update patternevent set time = ?6, value = ?7, meta = ?8, pluginparameter_id = ("
	  << " select pp.id from pluginparameter pp"
	  << " inner join parameterinfo i on i.id = pp.parameterinfo_id"
	  << " where pp.plugin_id = ?2 and i.paramgroup = ?3 and pp.paramtrack = ?4 and i.paramcolumn = ?5"
	  << ") where id = ?1;";
	prep_pattern_update_value_param.prepare(db, q.str());
}

void document::create_sql_functions() {
	sqlite3_create_function(db, "wavelevel_insert_samples", 4, SQLITE_ANY, this, ::wavelevel_insert_samples, 0, 0);
	sqlite3_create_function(db, "wavelevel_replace_samples", 4, SQLITE_ANY, this, ::wavelevel_replace_samples, 0, 0);
	sqlite3_create_function(db, "wavelevel_delete_samples", 3, SQLITE_ANY, this, ::wavelevel_delete_samples, 0, 0);
	sqlite3_create_function(db, "wavelevel_delete_file", 1, SQLITE_ANY, this, ::wavelevel_delete_file, 0, 0);
	sqlite3_create_function(db, "buzz_note_transpose", 2, SQLITE_ANY, this, noteutil_buzz_note_transpose, 0, 0);
	sqlite3_create_function(db, "buzz_note_base", 1, SQLITE_ANY, this, noteutil_buzz_note_base, 0, 0);
	sqlite3_create_function(db, "buzz_to_midi_note", 1, SQLITE_ANY, this, noteutil_buzz_to_midi_note, 0, 0);
	sqlite3_create_function(db, "midi_to_buzz_note", 1, SQLITE_ANY, this, noteutil_midi_to_buzz_note, 0, 0);
	sqlite3_create_function(db, "scale_param", 5, SQLITE_ANY, this, scale_param, 0, 0);
	sqlite3_create_function(db, "scale_param_range", 7, SQLITE_ANY, this, scale_param_range, 0, 0);
	sqlite3_create_function(db, "scale_param_point", 8, SQLITE_ANY, this, scale_param_point, 0, 0);
	sqlite3_create_function(db, "zzub_print", 1, SQLITE_ANY, this, zzub_print, 0, 0);
	sqlite3_create_function(db, "orderlist_timeshift", 3, SQLITE_ANY, this, orderlist_timeshift, 0, 0);
	sqlite3_create_function(db, "ensure_plugin_parameters", -1, SQLITE_ANY, this, ensure_plugin_parameters, 0, 0);
}

int document::get_db_version(std::string prefix) {
	sqlite::statement stmt(db, "select version from " + prefix + "song");
	if (!stmt.execute() || stmt.eof()) return -1;
	return sqlite3_column_int(stmt.stmt, 0);
}

bool document::check_db_upgrade(std::string prefix, std::ostream& query) {
	int version = get_db_version(prefix);
    
	// cant look up version, assume empty db with tables but no records
	if (version == -1) return true;

	if (version > armstrong_document_version) return false;	// trying to load a newer version
	// check version and abort if incompatible. add a series of upgrade scripts to the import query
	if (version < armstrong_document_version) {
		assert(max_upgrade_version == armstrong_document_version);
		for (int i = version; i < max_upgrade_version; i++) {
			cerr << "NOTE: upgrade from version " << i << " to " << (i+1) << endl;
			query << upgrade_scripts[i] << endl;
		}
		query << "update " << prefix << "song set version = " << armstrong_document_version << ";" << endl;
	}
	return true;
}

bool document::load(std::string dbfile) {
	stringstream query;

	query << "delete from song; attach database '" << dbfile << "' as loaddb; begin;";
	exec_noret(query.str());
	query.str(""); query.clear();

	if (!check_db_upgrade("loaddb.", query)) {
		query << "commit; detach database loaddb;";
		exec_noret(query.str());
		return false;
	}

	// TODO: 
	// - check db parameters match live parameters, 
	// - request user mappings (load from file/server, user ui to bind unknowns, save to server)
	// - run conversion scripts

	std::vector<int> plugins;
	create_clone(query, "", "loaddb.", plugins, 0, 0);
	
	exec_noret(query.str());
	query.str(""); query.clear();

	query << "commit; detach database loaddb;";
	exec_noret(query.str());
	return true;
}

void create_wave_clone(std::ostream& query, const std::string& toprefix, const std::string& fromprefix, bool remap);

// plugingroup_id = remaps group NULL to this
bool document::import(std::string dbfile, int plugingroup_id, bool with_waves, std::map<int, int>* wavelevelmappings) {
	stringstream query;

	// import should work like this:
	//     attach the db file to import as "loaddb"
	//     create a new in-memory song db as "tempdb"
	//     (the original song db is always "")
	//     clone original db into tempdb
	//     import loaddb into tempdb
	//     clone tempdb into original db
	//     detach tempdb and loaddb
	// -> all work takes place without adding extra fields in origdb, undoable etc
	
	query << "attach database '" << dbfile << "' as loaddb;";
	query << "attach database ':memory:' as tempdb; begin;";
	exec_noret(query.str());
	query.str(""); query.clear();

	if (!check_db_upgrade("loaddb.", query)) {
		query << "commit; detach database tempdb; detach database loaddb;";
		exec_noret(query.str());
		return false;
	}

	std::vector<int> plugins;
	create_tables(query, "tempdb.");
	create_clone(query, "tempdb.", "", plugins, 0, 0);
	create_import(query, "tempdb.", "loaddb.");
	create_clone(query, "", "tempdb.", plugins, 0, plugingroup_id);

	// remap pattern events for imported pattern ids, zzub_parameter_flag_pattern_index = 0x8
	query << "update patternevent set value = (select id from tempdb.pattern where value = orig_id) where "
		<< "pattern_id in (select id from tempdb.pattern where is_imported = 1) and "
		<< "value in (select orig_id from tempdb.pattern where is_imported = 1) and "
		<< "pluginparameter_id in ("
		<< "select pp.id from pluginparameter pp inner join parameterinfo pi on pi.id = pp.parameterinfo_id where (pi.flags & 8)"
		<< ");";

	// wave import modes: no waves, load+remap waves (third mode load+overwrite is not handled yet)
	//     no waves = do nothing
	//     remap = create_wave_clone(tempdb, loaddb, true) + create_wave_clone("", tempdb, false)
	//     overwrite = create_wave_clone("", loaddb, false)
	
	if (with_waves) {
		create_wave_clone(query, "tempdb.", "loaddb.", true);
		create_wave_clone(query, "", "tempdb.", false);

		// remap pattern events for imported wave ids, zzub_parameter_flag_wavetable_index = 1
		query << "update patternevent set value = (select id from tempdb.wave where value = orig_id) where "
			<< "pattern_id in (select id from tempdb.pattern where is_imported = 1) and "
			<< "value in (select orig_id from tempdb.wave where is_imported = 1) and "
			<< "pluginparameter_id in ("
			<< "select pp.id from pluginparameter pp inner join parameterinfo pi on pi.id = pp.parameterinfo_id where (pi.flags & 1)"
			<< ");";
	}

	exec_noret(query.str());
	query.str(""); query.clear();

	if (wavelevelmappings != 0) {
		// return wavelevel mappings so the outer importer can unpack to the correct filename
		sqlite::statement stmt(db, "select id, orig_id from tempdb.wavelevel where is_imported = 1;");
		if (stmt.execute()) {
			while (!stmt.eof()) {
				int id = sqlite3_column_int(stmt.stmt, 0);
				int orig_id = sqlite3_column_int(stmt.stmt, 1);
				wavelevelmappings->insert(std::pair<int, int>(id, orig_id));
				stmt.next();
			}
		}
	}

	query << "commit; detach database tempdb; detach database loaddb;";
	exec_noret(query.str());
	return true;
}

void document::clear() {
	assert(db != 0);
	stringstream query;
	query << "delete from song;";
	exec_noret(query.str());

	clear_history();
}

// plugingroup_id = saves with this group as the root, remaps to group NULL
bool document::save(std::string dbfile, const std::vector<int>& plugins, int plugingroup_id) {

	stringstream query;
	query << "attach database '" << dbfile << "' as savedb; begin;";

	exec_noret(query.str());
	query.str(""); query.clear();

	create_tables(query, "savedb.");
	create_clone(query, "savedb.", "", plugins, plugingroup_id, 0);

	exec_noret(query.str());
	query.str(""); query.clear();

	query << "commit; detach database savedb;";
	exec_noret(query.str());

	// now put the db file together with file blobs in a zip or something?
	return true;
}

void document::close() {
	assert(db != 0);
	sqlite3_close(db);
	db = 0;
}

void document::register_listener(documentlistener* v) {
	listeners.push_back(v);
}

void document::unregister_listener(documentlistener* v) {
	std::vector<documentlistener*>::iterator i = std::find(listeners.begin(), listeners.end(), v);
	if (i != listeners.end())
		listeners.erase(i);
}

bool document::notify_listeners(document_event_data* ev) {
	// handle events internally to enforce dynamic constraints and cascades. such as:
	//  - disallow inserting multiple singleton plugins
	//  - disallow inserting pattern events beyond the pattern length
	//  - delete pattern events and pattern format columns from deleted tracks
	//  - delete connection plugins when deleting plugins
	//  - delete pattern events when pattern resizes
	// etc... 
	// internal events can fail. return false causes a RAISE(ABORT) inside the trigger
	
	// before-events are sent to listeners before any internal processing
	// after-events are sent to listereners after internal processing
	
	if (!listener_enabled) return true;

	switch (ev->type) {
		case event_type_update_plugin:
			if (!on_after_update_plugin(ev)) return false;
			break;
		case event_type_update_pattern:
			if (!on_after_update_pattern(ev)) return false;
			break;
	}

	// notify listeners
	for (listeneriterator i = listeners.begin(); i != listeners.end(); ++i) {
		(*i)->update_document(ev);
	}

	switch (ev->type) {
		case event_type_before_insert_plugin:
			if (!on_before_insert_plugin(ev)) return false;
			break;
		case event_type_before_insert_patternevent:
			if (!on_before_insert_patternevent(ev)) return false;
			break;
		case event_type_before_delete_plugin:
			if (!on_before_delete_plugin(ev)) return false;
			break;
		case event_type_before_delete_pattern:
			if (!on_before_delete_pattern(ev)) return false;
			break;
		case event_type_before_delete_wavelevel:
			if (!on_before_delete_wavelevel(ev)) return false;
			break;
		case event_type_before_delete_patternformatcolumn:
			if (!on_before_delete_patternformatcolumn(ev)) return false;
			break;
	}

	return true;
}

bool document::on_before_insert_plugin(document_event_data* ev) {
	std::stringstream query;

	int count;
	// fail if theres a plugin with the same plugininfo_id and is_singleton
	query << "select count(*) from plugin p inner join plugininfo pi on pi.id = p.plugininfo_id where (pi.flags & 32768) and p.plugininfo_id = " << ev->newdata.plugin->plugininfo_id;
	if (!exec_scalar(query.str(), &count)) return false;

	// TODO: would the user like to know why we failed? ie notify if singleton?

	return count == 0;
}

bool document::on_before_insert_patternevent(document_event_data* ev) {

	std::stringstream query;
	int length;
	query << "select length from pattern where id = " << ev->newdata.patternevent->pattern_id;
	if (!exec_scalar(query.str(), &length)) return false;

	if (ev->newdata.patternevent->time >= length) {
		// TODO: error: time is greater than pattern length
		return false;
	}
	return true;
}

bool document::on_before_delete_plugin(document_event_data* ev) {
	std::stringstream query;
	query << "delete from plugin where id in (select plugin_id from connection where from_plugin_id = " << ev->newdata.plugin->id << ");" << endl;
	query << "delete from plugin where id in (select plugin_id from connection where to_plugin_id = " << ev->newdata.plugin->id << ");" << endl;

	// delete patternformats whose columns only belong to the deleted plugin:
	query 
		<< "delete from patternformat where (select count(*) from patternorder o inner join pattern p on p.id = o.pattern_id and p.patternformat_id = patternformat.id) = 0 and ("
		<< " select count(*) from patternformatcolumn c"
		<< " inner join pluginparameter pp on pp.id = c.pluginparameter_id"
		<< " where pp.plugin_id = " << ev->newdata.plugin->id << " and c.patternformat_id = patternformat.id) > 0 and ("
		<< " select count(*) from patternformatcolumn c"
		<< " inner join pluginparameter pp on pp.id = c.pluginparameter_id"
		<< " where pp.plugin_id != " << ev->newdata.plugin->id << " and c.patternformat_id = patternformat.id) = 0;" << endl;

	// update plugins whose timeinfos are set to this plugin
	query << "update plugin set timesource_plugin_id = -1 where timesource_plugin_id = " << ev->newdata.plugin->id << ";" << endl;

	exec_noret(query.str());
	return true;
}

bool document::on_before_delete_pattern(document_event_data* ev) {
	// instead of cascade deleting order entries, nullify the order entries first to keep the order length the same
	std::stringstream query;
	query << "update patternorder set pattern_id = null where pattern_id = " << ev->newdata.pattern->id << ";" << endl;
	exec_noret(query.str());
	return true;
}

bool document::on_before_delete_wavelevel(document_event_data* ev) {
	std::stringstream query;
	query << "select wavelevel_delete_samples(" << ev->newdata.wavelevel->id << ", 0, -1);" << endl;
	query << "select wavelevel_delete_file(" << ev->newdata.wavelevel->id << ");" << endl;
	exec_noret(query.str());
	return true;
}

bool document::on_before_delete_patternformatcolumn(document_event_data* ev) {
	std::stringstream query;
	query << "delete from patternevent where pattern_id in (select id from pattern where patternformat_id = " << ev->newdata.patternformatcolumn->patternformat_id << ") and pluginparameter_id = " << ev->newdata.patternformatcolumn->pluginparameter_id;
	cout << "on_before_delete_patternformatcolumn deletes superfluos patternevents" << endl;
	cout << query.str() << endl;
	exec_noret(query.str());
	return true;
}

bool document::on_after_update_plugin(document_event_data* ev) {
	std::stringstream query;

	if (ev->olddata.plugin->name != ev->newdata.plugin->name) {
		// rename patternformats with the same name as the plugin and only has columns from the same plugin
		query << "update patternformat set name = '" << ev->newdata.plugin->name << "' where name = '" << ev->olddata.plugin->name << "' and (select count(*) from patternorder o inner join pattern p on p.id = o.pattern_id and p.patternformat_id = patternformat.id) = 0 and ("
			<< " select count(*) from patternformatcolumn c"
			<< " inner join pluginparameter pp on pp.id = c.pluginparameter_id"
			<< " where pp.plugin_id = " << ev->newdata.plugin->id << " and c.patternformat_id = patternformat.id) > 0 and ("
			<< " select count(*) from patternformatcolumn c"
			<< " inner join pluginparameter pp on pp.id = c.pluginparameter_id"
			<< " where pp.plugin_id != " << ev->newdata.plugin->id << " and c.patternformat_id = patternformat.id) = 0;" << endl;
	}

	if (ev->olddata.plugin->trackcount > ev->newdata.plugin->trackcount) {
		query 
			<< "delete from patternevent where " << " pluginparameter_id in ("
			<< " select pp.id from pluginparameter pp"
			<< " inner join parameterinfo i on i.id = pp.parameterinfo_id"
			<< " where pp.plugin_id = " << ev->newdata.plugin->id << " and i.paramgroup = 2 and pp.paramtrack >= " << ev->newdata.plugin->trackcount << ");" << endl;
	}

	if (ev->olddata.plugin->trackcount != ev->newdata.plugin->trackcount) {
		query << "delete from pluginparameter where "
			<< " plugin_id = " << ev->newdata.plugin->id << " and paramtrack >= " << ev->newdata.plugin->trackcount
			<< " and parameterinfo_id in ("
			<< "  select i.id from parameterinfo i "
			<< "  inner join pluginparameter ipp on ipp.parameterinfo_id = i.id"
			<< "  where ipp.plugin_id = " << ev->newdata.plugin->id << " and i.paramgroup = 2"
			<< " )" << endl;
	}
	exec_noret(query.str());
	return true;
}

bool document::on_after_update_pattern(document_event_data* ev) {
	std::stringstream query;
	query << "delete from patternevent where " << ev->olddata.pattern->length << " != " << ev->newdata.pattern->length << " and pattern_id = " << ev->olddata.pattern->id << " and time >= " << ev->newdata.pattern->length << ";" << endl;
	exec_noret(query.str());
	return true;
}

void document::barrier(int redodata, int undodata, std::string const& description) {
	stringstream query;

	if (undoredo_enabled /*&& get_barrier_item_count() > 0*/) {

		query << "insert into undolog (stmt) values (\"";
		for (std::list<std::string>::iterator i = undoquery.begin(); i != undoquery.end(); ++i) {
			query << *i;
		}
		query << "\");";
		//query << "insert into undolog (stmt) values (\"" << undoquery.str() << "\");";
		exec_noret(query.str());
		query.str("");

		// if current_history_id < last_history -> delete remaining histories and undolog
		if (current_history_id < last_history_id) {
			query << "delete from undolog where history_id > " << current_history_id << ";";
			query << "delete from history where id > " << current_history_id << ";";
			exec_noret(query.str());

			query.str("");
		}

		// insert into history
		query << "insert into history (redodata, undodata, description) values(" << redodata << ", " << undodata << ", '" << description << "');";
		exec_noret(query.str());
		query.str("");

		last_history_id = current_history_id = (int)sqlite3_last_insert_rowid(db);
		
		query << "update undolog set history_id = " << current_history_id << " where history_id is null;";
		exec_noret(query.str());
	}

	undoquery.clear();

	// send some kind of notification so the mixer can flush:
	document_event_data ev;
	ev.type = event_type_barrier;
	ev.id = redodata;
	barrierdata data;
	data.description = description;
	data.type = 0;
	ev.newdata.barrier = &data;
	notify_listeners(&ev);
}

int document::get_barrier_item_count() {
	// NOTE: since after caching undo statements, this should always return 0
	stringstream query;
	sqlite::statement stmt(db, "select count(*) from undolog where history_id is null;");
	if (!stmt.execute() || stmt.eof()) return 0;

	return sqlite3_column_int(stmt.stmt, 0);
}

void document::add_undo_query(const std::string& query) {
	undoquery.insert(undoquery.begin(), query);
}

bool document::exec_noret(string const& query) {
	char *zErrMsg = 0;
	int rc = sqlite3_exec(db, query.c_str(), 0, 0, &zErrMsg);
	if (rc) {
		cerr << "Error in query:" << endl << query << endl;
		if (zErrMsg) {
			cerr << zErrMsg << endl;
		}
		return false;
	}
	return true;
}

bool document::exec_scalar(const std::string& query, int* result) {
	sqlite::statement stmt(db, query);
	if (!stmt.execute() || stmt.eof()) return false;
	*result = sqlite3_column_int(stmt.stmt, 0);
	return true;
}

void document::history_step() {
	// delete records from the undolog as they are played back
	// a play-back will generate a new undo-log entry, which will inherit the current history_id for redo

	stringstream query;
	query << "select * from undolog where history_id = " << current_history_id << " order by id desc;";

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, query.str().c_str(), (int)query.str().length(), &stmt, 0);
	if (rc != 0) {
		cerr << sqlite3_errmsg(db) << endl;
		return ;
	}

	query.str("");

	rc = sqlite3_step(stmt);
	while (rc == SQLITE_ROW) {
		int undo_id = sqlite3_column_int(stmt, 0);
		string undostmt = (const char*)sqlite3_column_text(stmt, 2);

		query << undostmt << endl;
		query << "delete from undolog where id = " << undo_id << ";" << endl;
		rc = sqlite3_step(stmt);
	}

	sqlite3_finalize(stmt);

	exec_noret(query.str());
	query.str("");

	query << "insert into undolog (history_id, stmt) values (" << current_history_id << ", \"";
	for (std::list<std::string>::iterator i = undoquery.begin(); i != undoquery.end(); ++i) {
		query << *i;
	}
	query << "\");";
	exec_noret(query.str());

	undoquery.clear();
}

void document::undo() {
	if (current_history_id < 1) return ;
	history_step();
	current_history_id--;

	document_event_data ev;
	ev.type = event_type_barrier;
	ev.id = 0;	// either history.undodata or history.redodata

	barrierdata data;
	data.type = 1; // 0 = first, 1 = undo, 2 = redo
	data.description = get_history_description(current_history_id);
	ev.newdata.barrier = &data;

	notify_listeners(&ev);
}

void document::redo() {
	if (current_history_id >= last_history_id) return ;
	current_history_id++;
	history_step();

	document_event_data ev;
	ev.type = event_type_barrier;
	ev.id = 0;	// either history.undodata or history.redodata

	barrierdata data;
	data.type = 2; // 0 = first, 1 = undo, 2 = redo
	data.description = get_history_description(current_history_id);
	ev.newdata.barrier = &data;

	notify_listeners(&ev);
}

int document::get_history_length() {
	return last_history_id;
}

int document::get_history_position() {
	return current_history_id;
}

std::string document::get_history_description(int index) {
	stringstream query;
	query << "select description from history where id = " << (index + 1) << ";";

	sqlite::statement stmt(db, query.str());
	if (!stmt.execute()) return "ERROR: Could not retreive description";
	if (stmt.eof()) return "ERROR: No description";

	return (char*)sqlite3_column_text(stmt.stmt, 0);
}

void document::clear_history() {
	finalize_undo_files();
	stringstream query;
	query << "delete from history; delete from undolog;" << endl;
	exec_noret(query.str());

	last_history_id = 0;
	current_history_id = 0;
	undoquery.clear();
}

bool document::create_history_tables() {
	const string query = 
		"create temp table undolog (id integer primary key, history_id integer, stmt varchar(512));"
		"create temp table history (id integer primary key, redodata int, undodata int, description varchar(64));";

	exec_noret(query);

	sqlite3_create_function(db, "undoredo_enabled_callback", 0, SQLITE_ANY, this, ::undoredo_enabled_callback, 0, 0);
	sqlite3_create_function(db, "undoredo_add_query", 1, SQLITE_ANY, this, ::undoredo_add_query, 0, 0);

	return true;
}

bool document::create_song(songdata& result) {

	stringstream query;
	query << "insert into song (title, loopbegin, loopend, loopenabled, samplerate, version, bpm, tpb, swing, swingticks) values ('', " << result.loopbegin << ", " << result.loopend << ", " << result.loopenabled << ", " << result.samplerate << ", " << armstrong_document_version << ", " << result.bpm << ", " << result.tpb << ", " << result.swing << ", " << result.swingticks << ")";
	sqlite::statement stmt(db, query.str());
	if (!stmt.execute()) return false;

	result.id = (int)sqlite3_last_insert_rowid(db);
	return true;
}

bool document::get_song(songdata& result) {
	sqlite::statement stmt(db, "select * from song limit 1");
	if (!stmt.execute() || stmt.eof()) return false;
	stmt.stmt >> result;
	return true;
}

std::string document::get_unique_session_filename(std::string prefix, int id, std::string ext) {
	stringstream strm;
	strm << prefix << "-" << unique_session_id.first << "-" << unique_session_id.second << "-" << id << "." << ext;
	boost::filesystem::path result = boost::filesystem::path(temppath) / boost::filesystem::path(strm.str());
	return result.string();
}

std::string document::wavelevel_get_filename(int id) {
	// use session-id embedded in the filename to prevent
	// file name collisions when running multiple instances
	return get_unique_session_filename("wavelevel", id, "raw");
}

void document::wavelevel_get_samples(int id, int minsize, int* length, unsigned char** result, bool legacyheader) { // user must delete[] result!
	std::string filename = wavelevel_get_filename(id);
	int legacysize = legacyheader ? 8 : 0;
	FILE* f = fopen(filename.c_str(), "rb");
	if (!f) {
		*length = minsize;
		*result = new unsigned char[minsize + legacysize];
		assert(*result != 0);
		memset(*result, 0, minsize + legacysize);
		return ;
	}
	
	// seek to end, find filesize, seek to start: (use stat? iostream?)
	fseek(f, 0, SEEK_END);
	*length = ftell(f);
	fseek(f, 0, SEEK_SET);

	*length = std::max(*length, minsize);

	unsigned char* buffer = new unsigned char[*length + legacysize];
	assert(buffer != 0);

	memset(buffer, 0, legacysize);
	fread(&buffer[legacysize], *length, 1, f);
	*result = buffer;

	fclose(f);
}

int document::wavelevel_get_byte_count(int id) { // user must delete[] result!
	std::string filename = wavelevel_get_filename(id);
	FILE* f = fopen(filename.c_str(), "rb");
	if (!f) return 0;
	
	fseek(f, 0, SEEK_END);
	int length = ftell(f);
	fclose(f);
	return length;
}

void document::wavelevel_set_samples(int id, int length, void* buffer) {
	std::string filename = wavelevel_get_filename(id);
	FILE* f = fopen(filename.c_str(), "wb");
	if (!f) {
		assert(false);
		return ;
	}

	fwrite(buffer, length, 1, f);

	fclose(f);
}

void document::wavelevel_delete_file(int id) {
	std::string filename = wavelevel_get_filename(id);
	_unlink(filename.c_str());
}

void document::wavelevel_insert_bytes(int id, int offset, int length, const unsigned char* buffer) {

	stringstream query;
	query << "select wavelevel_delete_samples(" << id << ", " << offset << ", " << length << ");";
	add_undo_query(query.str());

	// operate on file-on-disk, and generate an event when done
	// the mixer needs to update its wavedata from disk somehow later

	int prevlength = wavelevel_get_byte_count(id);
	unsigned char* resultbuffer;
	int resultsize;
	wavelevel_get_samples(id, prevlength + length, &resultsize, &resultbuffer, false);

	// move existing sampledata from offset to offset+length in the resultbuffer
	memmove(&resultbuffer[offset+length], &resultbuffer[offset], prevlength - offset);
	// write new sampledata at offset
	memcpy(&resultbuffer[offset], buffer, length);

	wavelevel_set_samples(id, prevlength + length, resultbuffer);
	delete[] resultbuffer;

	// send some kind of notification so the mixer can flush:
	document_event_data ev;
	ev.type = event_type_insert_samples;
	ev.id = id;
	notify_listeners(&ev);
}

std::string document::create_undo_file(int length, const unsigned char* buffer) {
	int next_id = (int)undoblobs.size(); // get_next_table_id(db, "undolog");
	std::string filename = get_unique_session_filename("undowave", next_id, "raw");
	undoblobs.push_back(undoblobtype(next_id, filename));
	save_binary(filename, buffer, length);
	return filename;
}

void document::finalize_undo_file(std::string filename, int* length, unsigned char** buffer) {
	if (buffer && length)
		load_binary(filename, buffer, length);
	_unlink((const char*)filename.c_str());
}

void document::finalize_undo_files() {
	for (std::vector<undoblobtype>::iterator i = undoblobs.begin(); i != undoblobs.end(); ++i) {
		_unlink(i->second.c_str());
	}
	undoblobs.clear();
}

void document::wavelevel_replace_bytes(int id, int offset, int length, const unsigned char* buffer) {

	stringstream query;
	unsigned char* resultbuffer;
	int resultlength;
	wavelevel_get_samples(id, 0, &resultlength, &resultbuffer, false);

	// save the undo wave data to file
	std::string undofile = create_undo_file(length, &resultbuffer[offset]);
	query << "select wavelevel_replace_samples(" << id << ", " << offset << ", " << length << ", '" << undofile << "');";
	add_undo_query(query.str());

	// operate on file-on-disk, and generate an event when done
	// the mixer needs to update its wavedata from disk somehow later

	int prevlength = wavelevel_get_byte_count(id);

	// write new sampledata at offset
	memcpy(&resultbuffer[offset], buffer, length);

	wavelevel_set_samples(id, prevlength, resultbuffer);
	delete[] resultbuffer;

	// send some kind of notification so the mixer can flush:
	document_event_data ev;
	ev.type = event_type_insert_samples;
	ev.id = id;
	notify_listeners(&ev);
}

void document::wavelevel_delete_bytes(int id, int offset, int length) { // length = -1: delete all
	stringstream query;
	unsigned char* resultbuffer;
	int resultlength;
	wavelevel_get_samples(id, 0, &resultlength, &resultbuffer, false);

	if (length == -1) length = resultlength;

	// save the undo wave data to file
	std::string undofile = create_undo_file(length, &resultbuffer[offset]);
	query << "select wavelevel_insert_samples(" << id << ", " << offset << ", " << length << ", '" << undofile << "');";
	add_undo_query(query.str());

	memcpy(&resultbuffer[offset], &resultbuffer[offset + length], resultlength - (offset + length));

	wavelevel_set_samples(id, resultlength - length, resultbuffer);

	delete[] resultbuffer;

	document_event_data ev;
	ev.type = event_type_delete_samples;
	ev.id = id;
	notify_listeners(&ev);

}

void document::ensure_parameter_exists(const std::string& prefix, int pluginid, int group, int track, int column, int defaultvalue) {
	stringstream query;
	query << "select pp.id from " << prefix << "pluginparameter pp "
		  << "inner join " << prefix << "parameterinfo i on pp.parameterinfo_id = i.id "
		  << "where pp.plugin_id = " << pluginid << " and i.paramgroup = " << group << " and pp.paramtrack = " << track << " and i.paramcolumn = " << column;
	
	int pluginparameter_id;
	if (exec_scalar(query.str(), &pluginparameter_id))
		return ; // exists! return!

	query.str("");
	query << "select id from " << prefix << "parameterinfo where plugininfo_id = (select plugininfo_id from " << prefix << "plugin where id = " << pluginid << ") and paramgroup = " << group << " and paramcolumn = " << column;
	int parameterinfo_id;
	if (exec_scalar(query.str(), &parameterinfo_id)) {
		// HACK: ensuring old version for "loaddb."-prefix:
		query.str("");
		if (prefix == "loaddb.") {
			query << "insert into " << prefix << "pluginparameter (plugin_id, parameterinfo_id, paramtrack, value) values (" << pluginid << ", " << parameterinfo_id << ", " << track << ", " << defaultvalue << ");";
		} else {
			query << "insert into " << prefix << "pluginparameter (plugin_id, parameterinfo_id, paramtrack, value, interpolator) values (" << pluginid << ", " << parameterinfo_id << ", " << track << ", " << defaultvalue << ", 0);";
		}
		exec_noret(query.str());
	} else {
		// shouldnt get here except during upgrades of older armdbs. in that case the parameter will be inserted for the later version
		assert(prefix == "loaddb."); 
	}
}

void document::ensure_attribute_exists(const std::string& prefix, int pluginid, int index, int defaultvalue) {
	stringstream query;
	query << "select pa.id from " << prefix << "attribute pa "
		  << "where pa.plugin_id = " << pluginid << " and pa.attrindex = " << index;
	
	int attribute_id;
	if (exec_scalar(query.str(), &attribute_id))
		return ; // exists! return!

	query.str("");
	query << "insert into " << prefix << "attribute (plugin_id, attrindex, value) values (" << pluginid << ", " << index << ", " << defaultvalue << ");";
	exec_noret(query.str());
}

void document::clear_unused_plugininfos() {
	stringstream query;
	query << "delete from plugininfo where (select count(*) from plugin where plugininfo_id = plugininfo.id) = 0";
	exec_noret(query.str());
}

// mpl::vector<>-based compiletime/runtime query generators:
//    create_clone - generates a sql query which clones (parts of) the db
//    create_import - generates a query that imports a database into another one

struct create_insert_column {
	std::ostream& strm;
	const std::string& toprefix;
	const std::string& fromprefix;
	bool with_primary;
	int count;

	create_insert_column(std::ostream& _strm, const std::string& _toprefix, const std::string& _fromprefix, bool _with_primary)
		: strm(_strm)
		, toprefix(_toprefix)
		, fromprefix(_fromprefix)
		, with_primary(_with_primary)
		, count(0)
	{}

	template <typename C>
	void operator()(const C&) {
		if (!with_primary && C::is_primary) return ;

		if (count > 0) strm << ", ";
		strm << C::name();
		count++;
	}
};

template <typename T>
struct create_clone_insert_select_column {
	std::ostream& strm;
	const std::string& toprefix;
	const std::string& fromprefix;
	int fromplugingroup_id, toplugingroup_id;
	int count;

	create_clone_insert_select_column(std::ostream& _strm, const std::string& _toprefix, const std::string& _fromprefix, int _toplugingroup_id, int _fromplugingroup_id)
		: strm(_strm)
		, toprefix(_toprefix)
		, fromprefix(_fromprefix)
		, toplugingroup_id(_toplugingroup_id)
		, fromplugingroup_id(_fromplugingroup_id)
		, count(0)
	{}

	template <typename C>
	void operator()(const C&) {
		std::string tablename = T::table_traits::name();
		std::string columnname = C::name();
		if (count > 0) strm << ", ";
		if (tablename == "plugingroup" && columnname == "parent_plugingroup_id") {
			strm << "case when ifnull(parent_plugingroup_id, 0)=" << fromplugingroup_id << " then ";
			if (toplugingroup_id != 0)
				strm << toplugingroup_id;
			else
				strm << "null";
			strm << " else parent_plugingroup_id end";
		} else
		if (tablename == "plugin" && columnname == "plugingroup_id") {
			// reassign the root plugingroup to the target plugingroup
			strm << "case when ifnull(plugingroup_id, 0)=" << fromplugingroup_id << " then ";
			if (toplugingroup_id != 0)
				strm << toplugingroup_id;
			else
				strm << "null";
			strm << " else plugingroup_id end";
		} else {
			strm << C::name();
		}
		count++;
	}
};

template <typename T>
struct create_clone_insert_where_column {
	std::ostream& strm;
	const std::string& toprefix;
	const std::string& fromprefix;
	int fromplugingroup_id, toplugingroup_id;
	int count;

	create_clone_insert_where_column(std::ostream& _strm, const std::string& _toprefix, const std::string& _fromprefix, int _toplugingroup_id, int _fromplugingroup_id)
		: strm(_strm)
		, toprefix(_toprefix)
		, fromprefix(_fromprefix)
		, toplugingroup_id(_toplugingroup_id)
		, fromplugingroup_id(_fromplugingroup_id)
		, count(0)
	{}

	template <typename C>
	void operator()(const C&) {
		std::string tablename = T::table_traits::name();
		std::string keytable = C::keytable();
		
		if (keytable == "") return;
		if (tablename == keytable) return;

		strm << " and ";

		if (tablename == "plugin" && keytable == "plugingroup") {
			strm << "(";
			strm << C::name();
			if (fromplugingroup_id != 0)
				strm << " = " << fromplugingroup_id; else
				strm << " is null";
			strm << " or ";
		}
		
		strm << C::name() << " in (select id from " << toprefix << C::keytable();
		
		if (tablename == "plugin" && keytable == "plugingroup") {
			strm << ")";
		}

		//if (tablename == "plugin" && keytable == "plugingroup") {
		//}
		strm << ")";
	}
};

template <typename T>
void create_clone_table_insert_query(std::ostream& strm, const std::string& tablename, const std::string& toprefix, const std::string& fromprefix, int toplugingroup_id, int fromplugingroup_id) {
	strm << "insert into " << toprefix << tablename << "(";
	boost::mpl::for_each<typename T::column_members>(create_insert_column(strm, toprefix, fromprefix, true));
	strm << ") select ";
	boost::mpl::for_each<typename T::column_members>(create_clone_insert_select_column<T>(strm, toprefix, fromprefix, toplugingroup_id, fromplugingroup_id));
	strm << " from " << fromprefix << tablename;

	strm << " where id > (select ifnull(max(id), 0) from " << toprefix << tablename << ")";
	boost::mpl::for_each<typename T::column_members>(create_clone_insert_where_column<T>(strm, toprefix, fromprefix, toplugingroup_id, fromplugingroup_id));

}

struct create_clone_table {
	std::ostream& strm;
	const std::string& toprefix;
	const std::string& fromprefix;
	const std::vector<int>& plugins;
	int fromplugingroup_id, toplugingroup_id;

	create_clone_table(std::ostream& _strm, const std::string& _toprefix, const std::string& _fromprefix, const std::vector<int>& _plugins, int _toplugingroup_id, int _fromplugingroup_id)
		: strm(_strm)
		, toprefix(_toprefix)
		, fromprefix(_fromprefix)
		, toplugingroup_id(_toplugingroup_id)
		, fromplugingroup_id(_fromplugingroup_id)
		, plugins(_plugins)
	{}

	template <typename T>
	void operator()(const T&) {
		// if plugins is non-empty(), do not:
		//    clone wave (which also prohibits wavelevels, envelopes, envpoints)
		//    clone patternformat (which also prohibits fmtcols, patterns, etc)
		std::string tablename = T::table_traits::name();
		if (!plugins.empty() && (tablename == "wave" || tablename == "patternformat"))
			return ;

		create_clone_table_insert_query<T>(strm, tablename, toprefix, fromprefix, toplugingroup_id, fromplugingroup_id);

		if (tablename == "plugininfo") {
			// only copy the plugininfos we need
			if (!plugins.empty()) {
				strm << " and id in (select plugininfo_id from " << fromprefix << "plugin where id in (";
				for (std::vector<int>::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
					if (i != plugins.begin())
						strm << ", ";
					strm << *i;
				}
				strm << "));";
			} else {
				strm << ";";
			}
		} else if (tablename == "plugin") {
			// only copy the plugins we need
			if (!plugins.empty()) {
				strm << " and id in (";
				for (std::vector<int>::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
					if (i != plugins.begin())
						strm << ", ";
					strm << *i;
				}
				strm << ");";
			} else {
				strm << ";";
			}
		} else if (tablename == "plugingroup") {
			// only copy the groups we need - except fromplugingroup_id
			if (!plugins.empty()) {
				strm << " and id in (select distinct plugingroup_id from " << fromprefix << "plugin where";
				if (fromplugingroup_id != 0)
					strm << " plugingroup_id != " << fromplugingroup_id << " and";
				strm << " id in (";
				for (std::vector<int>::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
					if (i != plugins.begin())
						strm << ", ";
					strm << *i;
				}
				strm << "));";
			} else {
				strm << ";";
			}
		} else {
			strm << ";";
		}

	}
};

void document::create_clone(std::ostream& query, const std::string& toprefix, const std::string& fromprefix, const std::vector<int>& plugins, int fromplugingroup_id, int toplugingroup_id) {
	boost::mpl::for_each<database_tables>(create_clone_table(query, toprefix, fromprefix, plugins, toplugingroup_id, fromplugingroup_id));
}

template <typename T>
struct create_import_insert_select_column {
	std::ostream& strm;
	const std::string& toprefix;
	const std::string& fromprefix;
	int count;

	create_import_insert_select_column(std::ostream& _strm, const std::string& _toprefix, const std::string& _fromprefix)
		: strm(_strm)
		, toprefix(_toprefix)
		, fromprefix(_fromprefix)
		, count(0)
	{}

	template <typename C>
	void operator()(const C&) {
		if (C::is_primary) return ;

		std::string tablename = T::table_traits::name();
		std::string columnname = C::name();

		if (count > 0) strm << ", ";
		strm << fromprefix << tablename << "." << columnname;
		count++;
	}
};

template <typename T>
struct create_import_update_key_column {
	std::ostream& strm;
	const std::string& toprefix;
	const std::string& fromprefix;
	int count;

	create_import_update_key_column(std::ostream& _strm, const std::string& _toprefix, const std::string& _fromprefix)
		: strm(_strm)
		, toprefix(_toprefix)
		, fromprefix(_fromprefix)
		, count(0)
	{}

	template <typename C>
	void operator()(const C&) {
		std::string tablename = T::table_traits::name();
		std::string columnname = C::name();
		std::string keytable = C::keytable();

		// connection from_plugin_id to_plugin_id arent keyed (for reasons related to events sent during deletion), but should at least be remapped like keys:
		if (tablename == "connection" && (columnname == "from_plugin_id" || columnname == "to_plugin_id")) {
			keytable = "plugin";
		}
		if (keytable.empty()) return;

		if (count > 0) strm << ", ";

		strm << columnname << " = ";
		{
			// general foreign key nullability:
			//if (C::is_nullable) {
			//	strm << "case when ifnull(" << toprefix << tablename << "." << columnname << ", 0) = 0 then null else ";
			//}
			if (tablename == "plugingroup" && columnname == "parent_plugingroup_id") {
				strm << "case when ifnull(" << toprefix << "plugingroup.parent_plugingroup_id, 0) = 0 then null else ";
			} else
			if (tablename == "plugin" && columnname == "plugingroup_id") {
				strm << "case when ifnull(" << toprefix << "plugin.plugingroup_id, 0) = 0 then null else ";
			}

			strm << "(select k.id from " << toprefix << keytable << " k where k.orig_id = " << toprefix << tablename << "." << columnname << ")";

			//if (C::is_nullable) {
			if (tablename == "plugingroup" && columnname == "parent_plugingroup_id") {
				strm << " end ";
			} else
			if (tablename == "plugin" && columnname == "plugingroup_id") {
				strm << " end ";
			}
		}

		count++;
	}
};

template <typename T>
void create_import_table_insert_query(std::ostream& strm, const std::string& tablename, const std::string& toprefix, const std::string& fromprefix) {
	strm << "insert into " << toprefix << tablename << " (";
	boost::mpl::for_each<typename T::column_members>(create_insert_column(strm, toprefix, fromprefix, false));
	strm << ", orig_id, is_imported) select ";
	boost::mpl::for_each<typename T::column_members>(create_import_insert_select_column<T>(strm, toprefix, fromprefix));
	strm << ", id, 1 from " << fromprefix << tablename;
}

struct create_import_table {
	std::ostream& strm;
	const std::string& toprefix;
	const std::string& fromprefix;

	create_import_table(std::ostream& _strm, const std::string& _toprefix, const std::string& _fromprefix)
		: strm(_strm)
		, toprefix(_toprefix)
		, fromprefix(_fromprefix)
	{}

	template <typename T>
	void operator()(const T&) {

		std::stringstream fromsingletonquery;
		fromsingletonquery << "select fpi.id from " << fromprefix << "plugininfo fpi inner join " << toprefix << "plugininfo tpi on fpi.uri = tpi.uri where (fpi.flags & 32768) != 0 and tpi.is_imported is null";
		std::stringstream tosingletonquery;
		tosingletonquery << "select tpi.id from " << toprefix << "plugininfo tpi inner join " << fromprefix << "plugininfo fpi on fpi.uri = tpi.uri where (tpi.flags & 32768) != 0 and tpi.is_imported is null";

		std::string tablename = T::table_traits::name();
		strm << "alter table " << toprefix << tablename << " add column orig_id;" << endl;
		strm << "alter table " << toprefix << tablename << " add column is_imported;" << endl;

		if (tablename == "wave" ||
			tablename == "wavelevel" ||
			tablename == "envelope" ||
			tablename == "envelopepoint")
				return ;

		if (tablename == "song") {
			// allow only one song
			strm << "update " << toprefix << "song set orig_id = id;" << endl; // this assumes there is just one song, and it has an id of 1!
		} else {

			if (tablename == "plugin") {
				// fixup master plugin orig_id, and also other singletons such as audio output, sequence
				strm << "update " << toprefix << "plugin set orig_id = " <<
					"(select fp.id from " << fromprefix << "plugin fp " << 
					" inner join " << fromprefix << "plugininfo fpi on fpi.id = fp.plugininfo_id and fpi.uri = tpi.uri " <<
					" inner join " << toprefix << "plugininfo tpi on tpi.id = " << toprefix << "plugin.plugininfo_id and (tpi.flags & 32768) != 0);" << endl;
			} else
			if (tablename == "plugininfo") {
				// update orig_id in plugininfos where uri is the same in src and target
				// the update query syntax requires a toprefix, make sure to use "main." where appropriate
				// http://www.sqlite.org/lang_attach.html: The database-names 'main' and 'temp' refer to the main database and the database used for temporary tables.
				assert(!toprefix.empty());
				strm << "update " << toprefix << "plugininfo set orig_id = (select id from " << fromprefix << "plugininfo where uri = " << toprefix << "plugininfo.uri);" << endl;
			} else
			if (tablename == "parameterinfo") {
				// update orig_id in parameterinfos where its plugininfo uri is the same in src and target
				assert(!toprefix.empty());
				strm << "update " << toprefix << "parameterinfo set orig_id = (select fp.id from " << fromprefix << "parameterinfo fp inner join " << toprefix << "plugininfo tp on fp.plugininfo_id = tp.orig_id where tp.id = " << toprefix << "parameterinfo.plugininfo_id and fp.paramgroup = " << toprefix << "parameterinfo.paramgroup and fp.paramcolumn = " << toprefix << "parameterinfo.paramcolumn);" << endl;
			} else
			if (tablename == "attributeinfo") {
				// update orig_id in attributeinfos where its plugininfo uri is the same in src and target
				assert(!toprefix.empty());
				strm << "update " << toprefix << "attributeinfo set orig_id = (select fa.id from " << fromprefix << "attributeinfo fa inner join " << toprefix << "plugininfo tp on fa.plugininfo_id = tp.orig_id where tp.id = " << toprefix << "attributeinfo.plugininfo_id and fa.attrindex = " << toprefix << "attributeinfo.attrindex);" << endl;
			}

			// to support self-referential fields, such as plugingroup.parent_plugingroup_id
			// the foreign keys must be updated in a separate step

			create_import_table_insert_query<T>(strm, tablename, toprefix, fromprefix);

			// allow only one of each singleton - dont import its plugin and plugininfo:
			// also dont import unused plugininfos (= check fromplugins.plugininfo_id and toplugins.orig_id)
			// also dont import unused or singleton parameterinfos/attributeinfos
			if (tablename == "plugin")
				strm << " where plugininfo_id not in (" << fromsingletonquery.str() << ");" << endl;
			else if (tablename == "plugininfo")
				strm << " where id not in (" << fromsingletonquery.str() << ") and id in (select plugininfo_id from " << fromprefix << "plugin where plugininfo_id not in (select orig_id from " << toprefix << "plugininfo where orig_id is not null));" << endl;
			else if (tablename == "parameterinfo" || tablename == "attributeinfo")
				strm << " where plugininfo_id not in (" << fromsingletonquery.str() << ") and plugininfo_id in (select orig_id from " << toprefix << "plugininfo where is_imported = 1);" << endl;
			else
				strm << ";" << endl;

			// pass 2: update foreign keys
			strm << "update " << toprefix << tablename << " set ";
			boost::mpl::for_each<typename T::column_members>(create_import_update_key_column<T>(strm, toprefix, fromprefix));
			strm << " where is_imported = 1;" << endl;
		}

	}
};

void document::create_import(std::ostream& query, const std::string& toprefix, const std::string& fromprefix) {
	boost::mpl::for_each<database_tables>(create_import_table(query, toprefix, fromprefix));
}

void create_wave_clone(std::ostream& query, const std::string& toprefix, const std::string& fromprefix, bool remap) {
	// dont need to delete replaced wavelevels, because "replace into wave" triggers cascade delete on wave -> wavelevel
	if (remap) {
		query << "create temp table allocatedwaves (wave_id integer);" << endl;
		query << "insert into allocatedwaves (wave_id) select id from " << fromprefix << "wave w where ";
		query << "(select count(*) from " << fromprefix << "wavelevel where wave_id = w.id) > 0;" << endl;

		query << "create temp table availablewaves (wave_id integer);" << endl;
		query << "insert into availablewaves (wave_id) select id from " << toprefix << "wave w where ";
		query << "(select count(*) from " << toprefix << "wavelevel where wave_id = w.id) = 0;" << endl;
	}

	query << "replace into " << toprefix << "wave (id, song_id, name, filename, flags, volume";
	if (remap) {
		query << ", orig_id, is_imported";
	}
	query << ") select dest.id, src.song_id, src.name, src.filename, src.flags, src.volume ";
	if (remap) {
		query << ", src.id, 1 ";
	}
	query << "from " << fromprefix << "wave src ";
	
	if (!remap) {
		query << "inner join " << toprefix << "wave dest on dest.id = src.id and src.is_imported = 1;";
	} else {
		query << "inner join allocatedwaves al on al.wave_id = src.id ";
		query << "inner join availablewaves av on av.rowid = al.rowid ";
		query << "inner join " << toprefix << "wave dest on dest.id = av.wave_id ";
		query << "where ";
		query << "(select count(*) from " << fromprefix << "wavelevel where wave_id = src.id) > 0;" << endl;
	}
	
	if (remap) {
		query << "drop table availablewaves; drop table allocatedwaves;" << endl;
		create_import_table_insert_query<waveleveldata>(query, waveleveldata::table_traits::name(), toprefix, fromprefix);
		query << " where wave_id in (select id from " << fromprefix << "wave w where (select count(*) from " << fromprefix << "wavelevel where wave_id = w.id) > 0);";

		// pass 2: update foreign keys
		query << "update " << toprefix << waveleveldata::table_traits::name() << " set ";
		boost::mpl::for_each<waveleveldata::column_members>(create_import_update_key_column<waveleveldata>(query, toprefix, fromprefix));
		query << " where is_imported = 1;" << endl;

	} else {
		create_clone_table_insert_query<waveleveldata>(query, waveleveldata::table_traits::name(), toprefix, fromprefix, 0, 0);
		query << ";" << endl;
	}
}

}
}
