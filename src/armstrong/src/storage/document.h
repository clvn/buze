#pragma once
#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30
#include <boost/shared_array.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>
#include <sstream>
#include <list>
#include <map>
#include <iomanip>

struct sqlite3;
struct sqlite3_stmt;

#include "database.h"
#include "dbgenpp.h"

namespace armstrong {
namespace storage {
	
#include "armdb_types.h"
#include "armdb_impl.h"

typedef dbgenpp::event_listener<document_event_data> documentlistener;

struct document : dbgenpp::event_source<document_event_data> {
	sqlite3 *db;

	std::string temppath;
	static int instance_count;
	std::pair<int, int> unique_session_id;

	typedef std::vector<documentlistener*>::iterator listeneriterator;
	std::vector<documentlistener*> listeners;

	int last_history_id;
	int current_history_id;
	bool undoredo_enabled;
	bool listener_enabled;
	typedef std::pair<int, std::string> undoblobtype;
	std::vector<undoblobtype> undoblobs;
	std::list<std::string> undoquery;

	document();

	bool open(std::string dbfile);
	bool load(std::string dbfile);
	bool save(std::string dbfile, const std::vector<int>& plugins = std::vector<int>(), int plugingroup_id = 0);
	bool import(std::string dbfile, int plugingroup_id, bool with_waves, std::map<int, int>* wavelevelmappings = 0);
	void close();
	void clear();
	int get_db_version(std::string prefix);
	bool check_db_upgrade(std::string prefix, std::ostream& query);

	//void create_tables(std::ostream& query, const char* prefix);	// generated in documentgenimpl.h
	void create_clone(std::ostream& query, const std::string& toprefix, const std::string& fromprefix, const std::vector<int>& plugins, int fromplugingroup_root_id, int toplugingroup_root_id);	// generated in documentgenimpl.h
	void create_import(std::ostream& query, const std::string& toprefix, const std::string& fromprefix);
	//void create_triggers(std::ostream& query);	// generated in documentgenimpl.h
	//void create_callbacks();
	void create_sql_functions();
	bool create_history_tables();

	bool exec_noret(std::string const& query);
	bool exec_scalar(const std::string& query, int* result);

	void register_listener(documentlistener* v);
	void unregister_listener(documentlistener* v);
	bool notify_listeners(document_event_data*);

	bool on_before_insert_plugin(document_event_data*);
	bool on_before_insert_patternevent(document_event_data*);
	bool on_before_delete_plugin(document_event_data*);
	bool on_before_delete_pattern(document_event_data*);
	bool on_before_delete_wavelevel(document_event_data*);
	bool on_before_delete_patternformatcolumn(document_event_data*);
	bool on_after_update_plugin(document_event_data*);
	bool on_after_update_pattern(document_event_data*);

	void history_step();
	void barrier(int redodata, int undodata, std::string const& description);
	void undo();
	void redo();
	int get_history_length();
	int get_history_position();
	std::string get_history_description(int index);
	void clear_history();
	int get_barrier_item_count();
	void add_undo_query(const std::string& query);

	bool create_song(songdata& result);
	bool get_song(songdata& result);

	std::string get_unique_session_filename(std::string prefix, int id, std::string ext);

	std::string wavelevel_get_filename(int id);
	void wavelevel_insert_bytes(int id, int offset, int length, const unsigned char* buffer);
	void wavelevel_delete_bytes(int id, int offset, int length);
	void wavelevel_replace_bytes(int id, int offset, int length, const unsigned char* buffer);
	void wavelevel_delete_file(int id);
	int wavelevel_get_byte_count(int id);
	void wavelevel_get_samples(int id, int minlength, int* length, unsigned char** result, bool legacyheader); // user must delete[] result!
	void wavelevel_set_samples(int id, int length, void* result);

	std::string create_undo_file(int length, const unsigned char* buffer);
	void finalize_undo_file(std::string, int* length, unsigned char** buffer);
	void finalize_undo_files();

	// pre-prepared statements :P
	void create_prepared_statements();
	sqlite::statement prep_pattern_insert_value;
	sqlite::statement prep_pattern_delete_value;
	sqlite::statement prep_pattern_update_value;
	sqlite::statement prep_pattern_update_value_param;

	void ensure_parameter_exists(const std::string& prefix, int pluginid, int group, int track, int column, int defaultvalue);
	void ensure_attribute_exists(const std::string& prefix, int pluginid, int index, int defaultvalue);
	void clear_unused_plugininfos(); // clean up unused plugininfo + parameterinfo + attributeinfo
};

}
}
