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

using namespace std;

namespace armstrong {
namespace storage {

// ---------------------------------------------------------------------------------------------------------------
// WHOLE PATTERN TRANSFORMS
// ---------------------------------------------------------------------------------------------------------------

/*
	MUTATION		COLS
	--------		----
	Compact			*
	Expand			*
*/

// Removes data outside the pattern boundaries
void pattern::cleanup_pattern() {
	stringstream q;
	q << "delete from patternevent where pattern_id = "<<id<<" and (time < 0 or time >= "<<this->length<<");";
	owner->exec_noret(q.str());
}

// Divides pattern length and event times
void pattern::compact_pattern(int factor) {
	sqlite::transaction tr(owner->db);
	stringstream q;
	q << "delete from patternevent where pattern_id = " << id << " and time % " << factor << " <> 0;"
	  << "update patternevent set time = time / " << factor << " where pattern_id = "<< id <<";"
	  << "update pattern set length = length / " << factor << " where id = "<< id <<";";
	owner->exec_noret(q.str());
}

// Multiplies pattern length and event times
void pattern::expand_pattern(int factor) {
	sqlite::transaction tr(owner->db);
	stringstream q;
	q << "update pattern set length = length * " << factor << " where id = " << id << ";"
	  << "update patternevent set time = time * " << factor << " where pattern_id = " << id <<";";
	owner->exec_noret(q.str());
}

// ---------------------------------------------------------------------------------------------------------------
// BASIC EDITING TRANSFORMS
// ---------------------------------------------------------------------------------------------------------------

/*
	MUTATION		COLS
	--------		----
	Timeshift		*
	Delete			*
	MoveScale		*
*/


void pattern::timeshift_events(int pluginparameter_id, int fromtime, int timeshift) {
	sqlite::transaction tr(owner->db);

	stringstream q;
	q << "delete from patternevent where "
	  << "  pattern_id = " << id << " and pluginparameter_id = " << pluginparameter_id
	  << " and (time + " << timeshift << " < " << fromtime << " or time + " << timeshift << " >= " << this->length << ") and time >= " << fromtime << ";";

	q << "update patternevent set time = time + "<<timeshift<<" where time >= "<<fromtime
	  << "  and pattern_id = "<<id << " and pluginparameter_id = " << pluginparameter_id << ";";

	owner->exec_noret(q.str());

	//cleanup_pattern();
}

void pattern::delete_events(int pluginparameter_id, int fromtime, int length) {
	stringstream q;
	q << "delete from patternevent where"
	  << "  pattern_id = "<<id << " and pluginparameter_id = " << pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<(fromtime + length) << ";";
	owner->exec_noret(q.str());
}

/// Meta issue
// Used for drag'n'drop
namespace { enum {
	drag_replace = 0,
	drag_mix_over = 1,
	drag_mix_under = 2,
	drag_swap = 3,
};
struct PE_value {
	PE_value() {}
	PE_value(int time, int value, int meta) : time(time), value(value), meta(meta) {}
	int time, value, meta;
};
typedef std::vector<PE_value> value_vect_t;
}
void pattern::move_scale_events(
	int src_idx, int src_time,
	int dst_idx, int dst_time,
	int src_width, int length, int mode, int makecopy
) {
	sqlite::transaction tr(owner->db);

	std::stringstream q;

	if (mode == drag_swap) makecopy = false;

	// source
	int src_endtime = src_time + length;
	std::vector<int> src_ids;
	{	q.str("");
		q << "select id from patternformatcolumn where patternformat_id = "<<patternformat_id
		  << "  order by idx limit "<<src_idx<<", "<<src_width<<";";
		tableiterator src_it(owner->db, q.str());

		while (!src_it.eof()) {
			src_ids.push_back(src_it.id());
			src_it.next();
		}
	}

	// dest
	int dst_endtime = dst_time + length;
	std::vector<int> dst_ids;
	{	q.str("");
		q << "select id from patternformatcolumn where patternformat_id = "<<patternformat_id
		  << "  order by idx limit "<<dst_idx<<", "<<src_width<<";";
		tableiterator dst_it(owner->db, q.str());

		while (!dst_it.eof()) {
			dst_ids.push_back(dst_it.id());
			dst_it.next();
		}
	}
	int dst_width = (int)dst_ids.size();
	int dst_last_idx = dst_idx + dst_width - 1;
	int dst_offset = (dst_idx < 0) ? dst_idx : 0;

	// setup
	armstrong::storage::song song(owner);
	owner->get_song(song);

	// step 1 -- insert into temp from source, ?1 = pluginparameter_id
	q.str("");
	q << "select time, value, meta from patternevent where pattern_id = " << id
	  << "  and pluginparameter_id = ?1"
	  << "  and time >= " << src_time << " and time < " << src_endtime
	  << "  order by time;";
	sqlite::statement stmt_temp_insert(owner->db, q.str());
	value_vect_t temp;

	// step 2 -- delete from source, ?1 = pluginparameter_id, ?2 = starttime, ?3 = endtime
	q.str("");
	q << "delete from patternevent where"
	  << "  pattern_id = " << id << " and pluginparameter_id = ?1"
	  << "  and time >= ?2 and time < ?3;";
	sqlite::statement stmt_delete(owner->db, q.str());

	// step 3 -- insert into source from dest (swap drop)
	q.str("");
	if (mode == drag_swap) {
		int diff_time = src_time - dst_time;
		q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
		  << "  select "<<id<<", ?1, time + "<<diff_time<<", scale_param(value, ?3, ?4, ?5, ?6), meta from patternevent where"
		  << "    pattern_id = "<<id<<" and pluginparameter_id = ?2"
		  << "    and time >= "<<dst_time<<" and time < "<<dst_endtime<<";";
	}
	sqlite::statement stmt_swap(owner->db, q.str());

	// step 4 -- check for collisions
	q.str("");
	if (mode == drag_mix_over || mode == drag_mix_under) {
		q << "select id from patternevent where"
		  << "  pattern_id = "<<id<<" and pluginparameter_id = ?1"
		  << "  and time = ?2"
		  << "  limit 1;";
	}
	sqlite::statement stmt_checktime(owner->db, q.str());

	// step 5 -- insert
	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  values ("<<id<<", ?, ?, ?, ?);";
	sqlite::statement stmt_insert(owner->db, q.str());

	// step 5.5 -- update
	q.str("");
	if (mode == drag_mix_over) {
		q << "update patternevent set value = ?2, meta = ?3 where id = ?1;";
	}
	sqlite::statement stmt_update(owner->db, q.str());

	// ok
	bool direction = (dst_idx <= src_idx);
	int i = direction ? 0 : src_width - 1;
	for (;;) {
		bool do_dest = ((dst_idx + i) >= 0) && ((dst_idx + i) <= dst_last_idx);

		if (!do_dest) {
			// step 2 only -- delete from source
			if (!makecopy) {
				sqlite3_bind_int(stmt_delete.stmt, 1, src_ids[i]);
				sqlite3_bind_int(stmt_delete.stmt, 2, src_time);
				sqlite3_bind_int(stmt_delete.stmt, 3, src_endtime);
				stmt_delete.execute();
				sqlite3_reset(stmt_delete.stmt);
			}
		} else {
			armstrong::storage::patternformatcolumn src_col(owner);
			song.get_patternformatcolumn_by_id(src_ids[i], src_col);
			
			armstrong::storage::pluginparameterdata src_plugparam;
			song.get_pluginparameter_by_id(src_col.pluginparameter_id, src_plugparam);

			armstrong::storage::parameterinfodata src_param;
			song.get_parameterinfo_by_id(src_plugparam.parameterinfo_id, src_param);
			double src_range0 = src_param.maxvalue - src_param.minvalue;

			armstrong::storage::patternformatcolumn dst_col(owner);
			song.get_patternformatcolumn_by_id(dst_ids[i + dst_offset], dst_col);

			armstrong::storage::pluginparameterdata dst_plugparam;
			song.get_pluginparameter_by_id(dst_col.pluginparameter_id, dst_plugparam);

			armstrong::storage::parameterinfodata dst_param;
			song.get_parameterinfo_by_id(dst_plugparam.parameterinfo_id, dst_param);
			double dst_range0 = dst_param.maxvalue - dst_param.minvalue;

			// step 1 -- insert into temp from source
			sqlite3_bind_int(stmt_temp_insert.stmt, 1, src_col.pluginparameter_id);
			stmt_temp_insert.next();
			while (!stmt_temp_insert.eof()) {
				int time = sqlite3_column_int(stmt_temp_insert.stmt, 0);
				int value = sqlite3_column_int(stmt_temp_insert.stmt, 1);
				int meta = sqlite3_column_int(stmt_temp_insert.stmt, 2);
				temp.push_back(PE_value(time - src_time, value, meta));
				stmt_temp_insert.next();
			}
			sqlite3_reset(stmt_temp_insert.stmt);

			// step 2 -- delete from source
			if (!makecopy) {
				sqlite3_bind_int(stmt_delete.stmt, 1, src_col.pluginparameter_id);
				sqlite3_bind_int(stmt_delete.stmt, 2, src_time);
				sqlite3_bind_int(stmt_delete.stmt, 3, src_endtime);
				stmt_delete.execute();
				sqlite3_reset(stmt_delete.stmt);
			}

			// step 3 -- swap drop
			if (mode == drag_swap) {
				sqlite3_bind_int(stmt_swap.stmt, 1, src_col.pluginparameter_id);
				sqlite3_bind_int(stmt_swap.stmt, 2, dst_col.pluginparameter_id);
				sqlite3_bind_int(stmt_swap.stmt, 3, src_param.minvalue);
				sqlite3_bind_int(stmt_swap.stmt, 4, src_param.maxvalue);
				sqlite3_bind_int(stmt_swap.stmt, 5, dst_param.minvalue);
				sqlite3_bind_int(stmt_swap.stmt, 6, dst_param.maxvalue);
				stmt_swap.execute();
				sqlite3_reset(stmt_swap.stmt);
			}

			// step 4+5 -- collisions + update

			sqlite3_bind_int(stmt_insert.stmt, 1, dst_col.pluginparameter_id);

			if (mode == drag_replace || mode == drag_swap) {
				sqlite3_bind_int(stmt_delete.stmt, 1, dst_col.pluginparameter_id);
				sqlite3_bind_int(stmt_delete.stmt, 2, dst_time);
				sqlite3_bind_int(stmt_delete.stmt, 3, dst_endtime);
				stmt_delete.execute();
				sqlite3_reset(stmt_delete.stmt);
			} else
			if (mode == drag_mix_over || mode == drag_mix_under) {
				sqlite3_bind_int(stmt_checktime.stmt, 1, dst_col.pluginparameter_id);
			}

			for (value_vect_t::iterator j = temp.begin(); j != temp.end(); ++j) {
				int time = j->time + dst_time;
				if (time < 0) continue;
				if (time >= this->length) break;

				double value0 = double(j->value - src_param.minvalue) / src_range0;
				int value = int((value0 * dst_range0) + 0.5) + dst_param.minvalue;
				int meta = j->meta;

				if (mode == drag_replace || mode == drag_swap) {
					sqlite3_bind_int(stmt_insert.stmt, 2, time);
					sqlite3_bind_int(stmt_insert.stmt, 3, value);
					sqlite3_bind_int(stmt_insert.stmt, 4, meta);
					stmt_insert.execute();
					sqlite3_reset(stmt_insert.stmt);
				} else
				if (mode == drag_mix_over) {
					sqlite3_bind_int(stmt_checktime.stmt, 2, time);
					stmt_checktime.execute();
					if (stmt_checktime.eof()) {
						sqlite3_bind_int(stmt_insert.stmt, 2, time);
						sqlite3_bind_int(stmt_insert.stmt, 3, value);
						sqlite3_bind_int(stmt_insert.stmt, 4, meta);
						stmt_insert.execute();
						sqlite3_reset(stmt_insert.stmt);
					} else {
						int evid = sqlite3_column_int(stmt_checktime.stmt, 0);
						sqlite3_bind_int(stmt_update.stmt, 1, evid);
						sqlite3_bind_int(stmt_update.stmt, 2, value);
						sqlite3_bind_int(stmt_update.stmt, 3, meta);
						stmt_update.execute();
						sqlite3_reset(stmt_update.stmt);
					}
					sqlite3_reset(stmt_checktime.stmt);
				} else
				if (mode == drag_mix_under) {
					sqlite3_bind_int(stmt_checktime.stmt, 2, time);
					stmt_checktime.execute();
					if (stmt_checktime.eof()) {
						sqlite3_bind_int(stmt_insert.stmt, 2, time);
						sqlite3_bind_int(stmt_insert.stmt, 3, value);
						sqlite3_bind_int(stmt_insert.stmt, 4, meta);
						stmt_insert.execute();
						sqlite3_reset(stmt_insert.stmt);
					}
					sqlite3_reset(stmt_checktime.stmt);
				}
			}

			// step 6 -- cleanup temp
			temp.clear();
		}

		// iterate
		if (direction) {
			if (++i < src_width) continue;
		} else {
			if (--i >= 0) continue;
		}

		break;
	}
}

namespace { enum {
	paste_replace = 0,
	paste_mix_over = 1,
	paste_mix_under = 2,
}; }
void pattern::paste_stream_events(int fromidx, int fromtime, int mode, char const* charbuf) {
	sqlite::transaction tr(owner->db);

	std::stringstream q;
	q << "select c.id, c.pluginparameter_id, i.minvalue, i.maxvalue from patternformatcolumn c"
	  << "  inner join pluginparameter pp on pp.id = c.pluginparameter_id"
	  << "  inner join parameterinfo i on i.id = pp.parameterinfo_id"
	  << "  where c.patternformat_id = "<<patternformat_id<<" order by c.idx limit 1 offset ?1;";
	sqlite::statement stmt_getcol(owner->db, q.str());

	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  values ("<<id<<", ?, ?, ?, ?);";
	sqlite::statement stmt_insert(owner->db, q.str());

	q.str("");
	if (mode == paste_mix_over || mode == paste_mix_under) {
		q << "select id from patternevent where"
		  << "  pattern_id = "<<id<<" and pluginparameter_id = ?1"
		  << "  and time = ?2"
		  << "  limit 1;";
	 }
	sqlite::statement stmt_checktime(owner->db, q.str());

	q.str("");
	q << "update patternevent set value = ?2, meta = ?3 where id = ?1;";
	sqlite::statement stmt_update(owner->db, q.str());

	armstrong::storage::song song(owner);
	owner->get_song(song);

	std::stringstream strm(charbuf);

	int columns;
	strm >> columns;
	int length;
	strm >> length;

	for (int i = 0; i < columns; ++i) {
		sqlite3_bind_int(stmt_getcol.stmt, 1, fromidx + i);
		stmt_getcol.execute();
		if (stmt_getcol.eof()) {
			sqlite3_reset(stmt_getcol.stmt);
			return;
		}
		int col_id = sqlite3_column_int(stmt_getcol.stmt, 0);
		int pluginparameter_id = sqlite3_column_int(stmt_getcol.stmt, 1);
		int minvalue = sqlite3_column_int(stmt_getcol.stmt, 2);
		int maxvalue = sqlite3_column_int(stmt_getcol.stmt, 3);

		sqlite3_reset(stmt_getcol.stmt);

		float col_range0 = (float)(maxvalue - minvalue);

		sqlite3_bind_int(stmt_insert.stmt, 1, pluginparameter_id);

		if (mode == paste_replace) {
			delete_events(pluginparameter_id, fromtime, length);
		} else
		if (mode == paste_mix_over || mode == paste_mix_under) {
			sqlite3_bind_int(stmt_checktime.stmt, 1, pluginparameter_id);
		}

		int rows;
		strm >> rows;

		int time;
		float value0;
		int meta;

		for (int j = 0; j < rows; ++j) {
			strm >> time;
			strm >> value0;
			strm >> meta;

			int value = int((value0 * col_range0) + 0.5f) + minvalue;

			int dest_time = fromtime + time;
			if (dest_time >= this->length) continue;

			if (mode == paste_replace) {
				sqlite3_bind_int(stmt_insert.stmt, 2, dest_time);
				sqlite3_bind_int(stmt_insert.stmt, 3, value);
				sqlite3_bind_int(stmt_insert.stmt, 4, meta);
				stmt_insert.execute();
				sqlite3_reset(stmt_insert.stmt);
			} else
			if (mode == paste_mix_over) {
				sqlite3_bind_int(stmt_checktime.stmt, 2, dest_time);
				stmt_checktime.execute();
				if (stmt_checktime.eof()) {
					sqlite3_bind_int(stmt_insert.stmt, 2, dest_time);
					sqlite3_bind_int(stmt_insert.stmt, 3, value);
					sqlite3_bind_int(stmt_insert.stmt, 4, meta);
					stmt_insert.execute();
					sqlite3_reset(stmt_insert.stmt);
				} else {
					int evid = sqlite3_column_int(stmt_checktime.stmt, 0);
					sqlite3_bind_int (stmt_update.stmt, 1, evid);
					sqlite3_bind_int(stmt_update.stmt, 2, value);
					sqlite3_bind_int(stmt_update.stmt, 3, meta);
					stmt_update.execute();
					sqlite3_reset(stmt_update.stmt);
				}
				sqlite3_reset(stmt_checktime.stmt);
			} else
			if (mode == paste_mix_under) {
				sqlite3_bind_int(stmt_checktime.stmt, 2, dest_time);
				stmt_checktime.execute();
				if (stmt_checktime.eof()) {
					sqlite3_bind_int(stmt_insert.stmt, 2, dest_time);
					sqlite3_bind_int(stmt_insert.stmt, 3, value);
					sqlite3_bind_int(stmt_insert.stmt, 4, meta);
					stmt_insert.execute();
					sqlite3_reset(stmt_insert.stmt);
				}
				sqlite3_reset(stmt_checktime.stmt);
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------
// SELECTION TRANSFORMS
// ---------------------------------------------------------------------------------------------------------------

/*
	MUTATION		NOTE	VALS	VOLS	WAVE	TRIG	CHAR

	--- Useful On Multiple Columns -----------------------------
	Randomize		x		x		x
	Random Range	x		x		x
	Random From		x		x		x		x		x		x
	Shuffle			x		x		x		x		x		x
	Interpolate		x		x		x		x
	Gradiate		x		x		x		x

	--- Intended For Single Column -----------------------------
	All To First	x		x		x		x		x		x
	First To Last	x		x		x		x		x		x
	Remove First	x		x		x		x		x		x

	--- Always Works On Anything -------------------------------
	Track Swap		x		x		x		x		x		x
	Row Swap		x		x		x		x		x		x
	Reverse			x		x		x		x		x		x
	Compact			x		x		x		x		x		x
	Expand			x		x		x		x		x		x
	Thin			x		x		x		x		x		x
	Rotate Rows		x		x		x		x		x		x
	Replace Waves							x
	Notelengths		x
	Volumes							x

	--- Can Be Note Masked -------------------------------------
	Echo			x		x		x		x		x		x
	Repeat			x		x		x		x		x		x
	Unique			x		x		x		x		x		x
	Rotate Values	x		x		x		x		x		x
	Rotate Rhythms	x		x		x		x		x		x

	--- Can Be Volume Masked -----------------------------------
	Scale					x		x
	Fade					x		x
	Curvemap				x		x
	Invert					x		x
	Humanize				x		x
	Smooth					x		x
*/

bool pattern::get_column_info(int pluginparameter_id, int* type, int* flags, int* minvalue, int* maxvalue, int* novalue) {
	stringstream q;
	q << "select pi.type, pi.flags, pi.minvalue, pi.maxvalue, pi.novalue from parameterinfo pi"
	  << " inner join pluginparameter pp on pp.parameterinfo_id = pi.id"
	  << " where pp.id = " << pluginparameter_id;
	sqlite::statement stmt(owner->db, q.str());
	if (!stmt.execute()) return false;

	*type = sqlite3_column_int(stmt.stmt, 0);
	*flags = sqlite3_column_int(stmt.stmt, 1);
	*minvalue = sqlite3_column_int(stmt.stmt, 2);
	*maxvalue = sqlite3_column_int(stmt.stmt, 3);
	*novalue = sqlite3_column_int(stmt.stmt, 4);
	return true;
}

// Transposes by delta.
void pattern::transpose_events(
	int pluginparameter_id, int fromtime, int length,
	int delta, int* holes, int holecount, int* metas, int metacount, int chromatic
) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	// special handling for notes and minmax-checking
	stringstream q;

	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	if (type == 0 /*zzub_parameter_type_note*/) {
		q.str("");
		q << "insert into connidxtab (connid) values (?1);";
		sqlite::statement stmt_insertid(owner->db, q.str());

		for (int i = 0; i < holecount; ++i) {
			sqlite3_bind_int(stmt_insertid.stmt, 1, holes[i]);
			stmt_insertid.execute();
			sqlite3_reset(stmt_insertid.stmt);
		}

		if ((delta == 12) || (delta == -12)) {
			q.str("");
			q << "update patternevent set value = buzz_note_transpose(value, "<<delta<<") where"
			  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
			  << "  and time >= "<<fromtime<<" and time < "<<endtime
			  << "  and value != 255 and value != 254"
			  << "  and buzz_note_transpose(value, "<<delta<<") >= "<<minvalue
			  << "  and buzz_note_transpose(value, "<<delta<<") <= "<<maxvalue
			  << "  and not exists (select 1 from connidxtab where connid = patternevent.id limit 1);";
			owner->exec_noret(q.str());
		} else {
			if (metacount <= 1) return;

			q.str("");
			q << "insert into patternevent_temp (evid, value, meta) select id, buzz_note_base(value), meta from patternevent where"
			  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
			  << "  and time >= "<<fromtime<<" and time < "<<endtime
			  << "  and value != 255 and value != 254"
			  << "  and buzz_note_base(value) = ?1" << (chromatic ? "" : " and meta = ?2")
			  << "  and not exists (select 1 from connidxtab where connid = patternevent.id limit 1);";
			sqlite::statement stmt_tempinsert(owner->db, q.str());

			q.str("");
			q << "update patternevent set value = buzz_note_transpose(value, ?3), meta = ?4 where"
			  << "  id in (select evid from patternevent_temp where value = ?1" << (chromatic ? "" : " and meta = ?2") << ")"
			  << "  and buzz_note_transpose(value, ?3) >= "<<minvalue
			  << "  and buzz_note_transpose(value, ?3) <= "<<maxvalue;
			sqlite::statement stmt_update(owner->db, q.str());

			typedef std::vector<std::pair<int, int> > metavect_t;
			metavect_t metavect;

			for (int i = 0; i < 12; ++i) {
				if (metas[i] != -1) {
					int src_baseval = i;
					int src_meta = metas[i];

					sqlite3_bind_int(stmt_tempinsert.stmt, 1, src_baseval);
					sqlite3_bind_int(stmt_tempinsert.stmt, 2, src_meta);
					stmt_tempinsert.execute();
					sqlite3_reset(stmt_tempinsert.stmt);

					metavect.push_back(std::pair<int, int>(src_baseval, src_meta));
				}
			}

			for (metavect_t::iterator i = metavect.begin(); i != metavect.end(); ++i) {
				metavect_t::iterator j = i;

				int octjump = 0;

				if (delta == 0) {
					// this is valid -- causes a Re-key
				} else
				if (delta == +1) {
					++j;
					if (j == metavect.end()) {
						j = metavect.begin();
						octjump = +12;
					}
				} else
				if (delta == -1) {
					if (j == metavect.begin()) {
						j = metavect.end();
						octjump = -12;
					}
					--j;
				}

				int src_baseval = i->first;
				int src_meta = i->second;
				int dst_baseval = j->first;
				int dst_meta = j->second;
				int dst_diff = (dst_baseval - src_baseval) + octjump;

				sqlite3_bind_int(stmt_update.stmt, 1, src_baseval);
				sqlite3_bind_int(stmt_update.stmt, 2, src_meta);
				sqlite3_bind_int(stmt_update.stmt, 3, dst_diff);
				sqlite3_bind_int(stmt_update.stmt, 4, dst_meta);
				stmt_update.execute();
				sqlite3_reset(stmt_update.stmt);
			}

			owner->exec_noret("delete from patternevent_temp;");

		}

		owner->exec_noret("delete from connidxtab;");
	} else {
		q.str("");
		q << "update patternevent set value = max(min(value + "<<delta<<", " << maxvalue << "), " << minvalue << ") where"
		<< "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
		<< "  and time >= "<<fromtime<<" and time < "<<endtime;
		owner->exec_noret(q.str());
	}

}

///
namespace {
inline int midi_to_buzz_note(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}
inline int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}
static const int midi_note_min = 0;
static const int midi_note_max = 119;
static const int midi_range1 = midi_note_max - midi_note_min + 1;
}
///

// Randomizes values in range.
void pattern::randomize_events(int pluginparameter_id, int fromtime, int length, int skip) {

	sqlite::transaction tr(owner->db);

	delete_events(pluginparameter_id, fromtime, length);

	stringstream q;
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  values (" << id << ", " << pluginparameter_id << ", ?, ?, 0);";
	sqlite::statement insert_stmt(owner->db, q.str());

	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	int range1 = maxvalue - minvalue + 1;

	for (int i = 0; i < length; i += skip) {
		int value;
		if (type == 0 /*zzub_parameter_type_note*/)
			value = midi_to_buzz_note(midi_note_min + (rand() % midi_range1));
		else
			value = minvalue + (rand() % range1);

		sqlite3_bind_int(insert_stmt.stmt, 1, fromtime + i);
		sqlite3_bind_int(insert_stmt.stmt, 2, value);

		insert_stmt.execute();
		sqlite3_reset(insert_stmt.stmt);
	}

}

// Range of top value to bottom value.
void pattern::randomize_range_events(int pluginparameter_id, int fromtime, int length, int from_val, int to_val, int skip) {

	sqlite::transaction tr(owner->db);

	stringstream q;
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  values ("<<id<<", "<<pluginparameter_id<<", ?1, ?2, 0);";
	sqlite::statement stmt_insert(owner->db, q.str());

	delete_events(pluginparameter_id, fromtime, length);

	q.str("");

	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	int from_rand = std::min(from_val, to_val);
	int to_rand = std::max(from_val, to_val);

	if (type == 0 /*zzub_parameter_type_note*/) {
		from_rand = buzz_to_midi_note(from_rand);
		to_rand = buzz_to_midi_note(to_rand);
	}

	int range1 = to_rand - from_rand + 1;

	for (int i = 0; i < length; i += skip) {
		int value = from_rand + (rand() % range1);
		if (type == 0 /*zzub_parameter_type_note*/)
			value = midi_to_buzz_note(value);

		sqlite3_bind_int(stmt_insert.stmt, 1, fromtime + i);
		sqlite3_bind_int(stmt_insert.stmt, 2, value);
		stmt_insert.execute();
		sqlite3_reset(stmt_insert.stmt);
	}

}

// Randomize from the set of values already existing in the range
void pattern::randomize_from_events(int pluginparameter_id, int fromtime, int length, int skip) {
	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
	tableiterator tabit(owner->db, q.str());

	if (tabit.eof()) return;

	q.str("");
	q << "select value, meta from patternevent where id = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  values ("<<id<<", "<<pluginparameter_id<<", ?1, ?2, ?3);";
	sqlite::statement stmt_insert(owner->db, q.str());

	std::vector<std::pair<int, int> > vals;

	while (!tabit.eof()) {
		sqlite3_bind_int(stmt_getval.stmt, 1, tabit.id());
		stmt_getval.execute();
		int value = sqlite3_column_int(stmt_getval.stmt, 0);
		int meta = sqlite3_column_int(stmt_getval.stmt, 1);
		sqlite3_reset(stmt_getval.stmt);

		vals.push_back(std::pair<int, int>(value, meta));

		tabit.next();
	}

	if (vals.size() < 1) return;

	delete_events(pluginparameter_id, fromtime, length);

	int range1 = (int)vals.size();
	for (int i = 0; i < length; i += skip) {
		int validx = rand() % range1;
		int value = vals[validx].first;
		int meta = vals[validx].second;

		sqlite3_bind_int(stmt_insert.stmt, 1, fromtime + i);
		sqlite3_bind_int(stmt_insert.stmt, 2, value);
		sqlite3_bind_int(stmt_insert.stmt, 3, meta);
		stmt_insert.execute();
		sqlite3_reset(stmt_insert.stmt);
	}
}

// Like randomize, but with a deviation.
void pattern::humanize_events(int pluginparameter_id, int fromtime, int length, int deviation) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "update patternevent set value = min(max(value + (random() % ?1), ?2), ?3) where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = " << pluginparameter_id
	  << "  and time >= "<<fromtime<< " and time < "<<endtime<<";";
	sqlite::statement stmt(owner->db, q.str());

	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	int range1 = maxvalue - minvalue + 1;
	int deviance = max(2, range1 / deviation);

	sqlite3_bind_int(stmt.stmt, 1, deviance);
	sqlite3_bind_int(stmt.stmt, 2, minvalue);
	sqlite3_bind_int(stmt.stmt, 3, maxvalue);
	stmt.execute();

}

// Shuffles events while maintaining time positions.
void pattern::shuffle_events(int pluginparameter_id, int fromtime, int length) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id 
	  << "  and time >= "<<fromtime<<" and time < "<<endtime
	  << "  order by time;";
	tableiterator tabit(owner->db, q.str());

	if (tabit.eof()) return;

	q.str("");
	q << "select value, meta from patternevent where id = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "update patternevent set value = ?1, meta = ?2 where id = ?3;";
	sqlite::statement stmt_updateval(owner->db, q.str());

	typedef std::vector<std::pair<int, int> > vals_t;
	vals_t vals;
	typedef std::vector<int> ids_t;
	ids_t ids;

	while (!tabit.eof()) {
		int evid = tabit.id();

		sqlite3_bind_int(stmt_getval.stmt, 1, evid);
		stmt_getval.execute();
		int value = sqlite3_column_int(stmt_getval.stmt, 0);
		int meta = sqlite3_column_int(stmt_getval.stmt, 1);
		sqlite3_reset(stmt_getval.stmt);

		vals.push_back(std::pair<int, int>(value, meta));
		ids.push_back(evid);

		tabit.next();
	}

	std::random_shuffle(vals.begin(), vals.end());

	vals_t::iterator vals_it = vals.begin();
	ids_t::iterator ids_it = ids.begin();
	while (ids_it != ids.end()) {
		sqlite3_bind_int(stmt_updateval.stmt, 1, (*vals_it).first);
		sqlite3_bind_int(stmt_updateval.stmt, 2, (*vals_it).second);
		sqlite3_bind_int(stmt_updateval.stmt, 3, *ids_it);
		stmt_updateval.execute();
		sqlite3_reset(stmt_updateval.stmt);
		++vals_it;
		++ids_it;
	}

}

// Fills in between first and last, ignoring events in between.
// If there's no last value, works as a "fill" of the first value.
void pattern::interpolate_events(int pluginparameter_id, int fromtime, int length, int skip) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  values ("<<id<<", "<<pluginparameter_id << ", ?, ?, 0);";
	sqlite::statement stmt(owner->db, q.str());

	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	int totime = (endtime - 1) - (skip - 1);

	int startvalue;
	int startmeta;
	int start_id = get_event_at(pluginparameter_id, fromtime, &startvalue, &startmeta);
	int endvalue;
	int endmeta;
	int end_id = get_event_at(pluginparameter_id, totime, &endvalue, &endmeta);

	if (start_id != -1) {
		delete_events(pluginparameter_id, fromtime, length);

		if (type == 0 /*zzub_parameter_type_note*/) {
			startvalue = buzz_to_midi_note(startvalue);
			if (endvalue != -1) {
				endvalue = buzz_to_midi_note(endvalue);
			}
		}

		int steps = length / skip;
		float delta;
		if (end_id != -1)
			delta = (float)(endvalue - startvalue) / (steps - 1);
		else
			delta = 0;

		for (int i = 0; i < steps; i++) {
			int value;
			if (delta == 0)
				value = startvalue;
			else
			if (delta > 0)
				value = startvalue + (int)floor((delta * i) + 0.5);
			else
				value = startvalue + (int)ceil((delta * i) - 0.5);
			if (type == 0)
				value = midi_to_buzz_note(value);

			sqlite3_bind_int(stmt.stmt, 1, fromtime + i * skip);
			sqlite3_bind_int(stmt.stmt, 2, value);
			stmt.execute();
			sqlite3_reset(stmt.stmt);
		}
	}


}

// Like interpolate, but fills in between each existing value.
// Does not require first or last rows to exist.
void pattern::gradiate_events(int pluginparameter_id, int fromtime, int length, int skip) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "select time, value from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = ?"
	  << "  and time >= ? and time < "<<endtime
	  << "  order by time"
	  << "  limit 1;";
	sqlite::statement stmt_nexttime(owner->db, q.str());

	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  values ("<<id<<", " << pluginparameter_id << ", ?, ?, 0);";
	sqlite::statement stmt_insert(owner->db, q.str());

	// find first time-stamp
	sqlite3_bind_int(stmt_nexttime.stmt, 1, fromtime);

	if (!stmt_nexttime.execute() || stmt_nexttime.eof()) {
		// got no first time
	} else {
		int cur_t = sqlite3_column_int(stmt_nexttime.stmt, 0);
		int cur_v = sqlite3_column_int(stmt_nexttime.stmt, 1);
		sqlite3_reset(stmt_nexttime.stmt);

		int next_t = -1;
		int next_v = -1;

		do {
			sqlite3_bind_int(stmt_nexttime.stmt, 1, cur_t + 1);

			if (!stmt_nexttime.execute() || stmt_nexttime.eof()) {
				next_t = -1;
			} else {
				next_t = sqlite3_column_int(stmt_nexttime.stmt, 0);
				next_v = sqlite3_column_int(stmt_nexttime.stmt, 1);

				int len = next_t - cur_t;
				float delta = (float)(next_v - cur_v) / len; 

				for (int i = skip; i < len; i += skip) {
					int value;
					if (delta > 0)
						value = cur_v + (int)floor((delta * i) + 0.5);
					else
						value = cur_v + (int)ceil((delta * i) - 0.5);

					sqlite3_bind_int(stmt_insert.stmt, 1, cur_t + i);
					sqlite3_bind_int(stmt_insert.stmt, 2, value);
					stmt_insert.execute();
					sqlite3_reset(stmt_insert.stmt);
				}
			}
			sqlite3_reset(stmt_nexttime.stmt);

			cur_t = next_t;
			cur_v = next_v;
		} while(next_t != -1);
	}
	//sqlite3_reset(stmt_nexttime.stmt);


}

// Moves events closer to the next event.
void pattern::smooth_events(int pluginparameter_id, int fromtime, int length, int strength) {
}

// Reverses selection.
void pattern::reverse_events(int pluginparameter_id, int fromtime, int length) {

	int endtime = fromtime + length;

	stringstream q;
	q << "update patternevent set time = "<<(endtime - 1)<<" - (time - "<<fromtime<<") where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = " << pluginparameter_id << " and time >= "<<fromtime<<" and time < "<<endtime;
	q << ";";
	owner->exec_noret(q.str());
}

// Makes selection i.e. half size.
void pattern::compact_events(int pluginparameter_id, int fromtime, int length, int factor) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;

	q << "delete from patternevent where"
	  << "  (time - "<<fromtime<<") % "<<factor<<" <> 0 and time >= "<<fromtime<<" and time < "<<endtime
	  << "  and pattern_id = "<<id << " and pluginparameter_id = " << pluginparameter_id;
	q << ";";

	q << "update patternevent set time = "<<fromtime<<" + (time - "<<fromtime<<") / "<<factor<<" where"
	  << "  time >= "<<fromtime<<" and time < "<<endtime
	  << "  and pattern_id = "<<id << " and pluginparameter_id = " << pluginparameter_id;
	q << ";";

	owner->exec_noret(q.str());

}

// Makes selection i.e. double size.
void pattern::expand_events(int pluginparameter_id, int fromtime, int length, int factor) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	delete_events(pluginparameter_id, endtime, length * (factor - 1));

	stringstream q;
	q << "update patternevent set time = "<<fromtime<<" + (time - "<<fromtime<<") * "<<factor<<" where"
	  << "  time >= "<<fromtime<<" and time < "<<endtime
	  << "  and pattern_id = "<<id << " and pluginparameter_id = " << pluginparameter_id;
	q << ";";

	owner->exec_noret(q.str());

	cleanup_pattern();

}

// Removes every event that's not (time%major == 0).
void pattern::thin_events(int pluginparameter_id, int fromtime, int length, int major) {

	int endtime = fromtime + length;

	stringstream q;
	q << "delete from patternevent where"
	  << "  (time - "<<fromtime<<") % "<<major<<" <> 0 and time >= "<<fromtime<<" and time < "<<endtime
	  << "  and pattern_id = "<<id << " and pluginparameter_id = " << pluginparameter_id;
	q << ";";

	owner->exec_noret(q.str());

}

// REPEAT
void pattern::repeat_events(int pluginparameter_id, int fromtime, int length, int major) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime
	  << "  order by time;";
	tableiterator tabit(owner->db, q.str());

	q.str("");
	q << "select time, value, meta from patternevent where id = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "select time from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time > ?1 and time < "<<endtime
	  << "  order by time"
	  << "  limit 1;";
	sqlite::statement stmt_nexttime(owner->db, q.str());

	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  values ("<<id<<", "<<pluginparameter_id<<", ?1, ?2, ?3);";
	sqlite::statement stmt_insert(owner->db, q.str());

	while (!tabit.eof()) {
		sqlite3_bind_int(stmt_getval.stmt, 1, tabit.id());
		stmt_getval.execute();
		int time = sqlite3_column_int(stmt_getval.stmt, 0);
		int value = sqlite3_column_int(stmt_getval.stmt, 1);
		int meta = sqlite3_column_int(stmt_getval.stmt, 2);
		sqlite3_reset(stmt_getval.stmt);

		sqlite3_bind_int(stmt_nexttime.stmt, 1, time);
		stmt_nexttime.execute();
		int next_time;
		if (stmt_nexttime.eof())
			next_time = endtime;
		else
			next_time = sqlite3_column_int(stmt_nexttime.stmt, 0);
		sqlite3_reset(stmt_nexttime.stmt);

		bool past_next = false;
		while (!past_next) {
			time += major;

			if (time < next_time) {
				sqlite3_bind_int(stmt_insert.stmt, 1, time);
				sqlite3_bind_int(stmt_insert.stmt, 2, value);
				sqlite3_bind_int(stmt_insert.stmt, 3, meta);
				stmt_insert.execute();
				sqlite3_reset(stmt_insert.stmt);
			} else {
				past_next = true;
			}
		}

		tabit.next();
	}

}

// ECHO
void pattern::echo_events(int pluginparameter_id, int fromtime, int length, int major) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime
	  << "  order by time;";
	tableiterator tabit(owner->db, q.str());

	q.str("");
	q << "select time, value, meta from patternevent where id = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "select time from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time = ?1"
	  << "  limit 1;";
	sqlite::statement stmt_checktime(owner->db, q.str());

	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  values ("<<id<<", "<<pluginparameter_id<<", ?1, ?2, ?3);";
	sqlite::statement stmt_insert(owner->db, q.str());

	while (!tabit.eof()) {
		sqlite3_bind_int(stmt_getval.stmt, 1, tabit.id());
		stmt_getval.execute();
		int time = sqlite3_column_int(stmt_getval.stmt, 0);
		int value = sqlite3_column_int(stmt_getval.stmt, 1);
		int meta = sqlite3_column_int(stmt_getval.stmt, 2);
		sqlite3_reset(stmt_getval.stmt);

		int checktime = time + major;
		if (checktime >= endtime) break;

		sqlite3_bind_int(stmt_checktime.stmt, 1, checktime);
		stmt_checktime.execute();
		if (stmt_checktime.eof()) {
			sqlite3_bind_int(stmt_insert.stmt, 1, checktime);
			sqlite3_bind_int(stmt_insert.stmt, 2, value);
			sqlite3_bind_int(stmt_insert.stmt, 3, meta);
			stmt_insert.execute();
			sqlite3_reset(stmt_insert.stmt);
		}
		sqlite3_reset(stmt_checktime.stmt);

		tabit.next();
	}

}

// UNIQUE
void pattern::unique_events(int pluginparameter_id, int fromtime, int length) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime
	  << "  order by time;";
	tableiterator tabit(owner->db, q.str());

	q.str("");
	q << "select value, meta from patternevent where id = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "delete from patternevent where id = ?1;";
	sqlite::statement stmt_delete(owner->db, q.str());

	int prev_value = -1;
	int prev_meta = -1;
	while (!tabit.eof()) {
		int evid = tabit.id();

		sqlite3_bind_int(stmt_getval.stmt, 1, evid);
		stmt_getval.execute();
		int value = sqlite3_column_int(stmt_getval.stmt, 0);
		int meta = sqlite3_column_int(stmt_getval.stmt, 1);
		sqlite3_reset(stmt_getval.stmt);

		if (value == prev_value && meta == prev_meta) {
			sqlite3_bind_int(stmt_delete.stmt, 1, evid);
			stmt_delete.execute();
			sqlite3_reset(stmt_delete.stmt);
		}

		tabit.next();
		prev_value = value;
		prev_meta = meta;
	}
}

// Scales
// Used by: AMPLIFY
void pattern::scale_events(int pluginparameter_id, int fromtime, int length, double min1, double max1, double min2, double max2) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "update patternevent set value = scale_param_range(value, ?1, ?2, ?3, ?4, ?5, ?6) where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<< " and time < "<<endtime<<";";
	sqlite::statement stmt(owner->db, q.str());

	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	sqlite3_bind_int(stmt.stmt, 1, minvalue);
	sqlite3_bind_int(stmt.stmt, 2, maxvalue);
	sqlite3_bind_double(stmt.stmt, 3, min1);
	sqlite3_bind_double(stmt.stmt, 4, max1);
	sqlite3_bind_double(stmt.stmt, 5, min2);
	sqlite3_bind_double(stmt.stmt, 6, max2);

	stmt.execute();

}

// Fades
void pattern::fade_events(int pluginparameter_id, int fromtime, int length, double from, double to) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "update patternevent set value = scale_param_point(value, ?1, ?2, time, ?3, ?4, ?5, ?6) where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<< " and time < "<<endtime<<";";
	sqlite::statement stmt(owner->db, q.str());

	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	sqlite3_bind_int(stmt.stmt, 1, minvalue);
	sqlite3_bind_int(stmt.stmt, 2, maxvalue);
	sqlite3_bind_int(stmt.stmt, 3, fromtime);
	sqlite3_bind_int(stmt.stmt, 4, endtime);
	sqlite3_bind_double(stmt.stmt, 5, from);
	sqlite3_bind_double(stmt.stmt, 6, to);

	stmt.execute();
}

// Curves
#include <cmath>
namespace { static const double PI = 4.0 * std::atan(1.0); }
void pattern::curvemap_events(int pluginparameter_id, int fromtime, int length, int mode) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
	tableiterator tabit(owner->db, q.str());

	if (tabit.eof()) return;

	q.str("");
	q << "select value from patternevent where id = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "update patternevent set value = ?1 where id = ?2;";
	sqlite::statement stmt_updateval(owner->db, q.str());

	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	double range0 = maxvalue - minvalue;

	while (!tabit.eof()) {
		int evid = tabit.id();

		sqlite3_bind_int(stmt_getval.stmt, 1, evid);
		stmt_getval.execute();
		int value = sqlite3_column_int(stmt_getval.stmt, 0);
		sqlite3_reset(stmt_getval.stmt);

		double x = (value - minvalue) / range0;

		switch (mode) {
			case 0:
				x = std::sin(x * PI/2.0);
				break;
			case 1:
				x = std::asin(x) / PI*2.0;
				break;
			case 2:
				x = std::sin(x * PI/2.0 + PI*1.5) + 1.0;
				break;
			case 3:
				x = std::asin(x*2.0 - 1.0)/PI + 0.5;
				break;
			case 4:
				x = std::sin(x*PI - PI/2.0)/2.0 + 0.5;///
				break;
			case 5:
				x = std::asin(x - 1.0) / PI*2.0 + 1.0;
				break;
			case 6:
				x = -std::sqrt(1.0 - std::pow(x, 2.0)) + 1.0;
				break;
			case 7:
				x = std::sqrt(1.0 - std::pow(1.0-x, 2.0));///
				break;
		}

		value = int((x * range0) + 0.5) + minvalue;

		sqlite3_bind_int(stmt_updateval.stmt, 1, value);
		sqlite3_bind_int(stmt_updateval.stmt, 2, evid);
		stmt_updateval.execute();
		sqlite3_reset(stmt_updateval.stmt);

		tabit.next();
	}

}

// Sets events to (maxvalue - x).
void pattern::invert_events(int pluginparameter_id, int fromtime, int length) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "update patternevent set value = ((?1 - value) + ?2) where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime <<";";
	sqlite::statement stmt(owner->db, q.str());

	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	if (type != 0 /*zzub_parameter_type_note*/) {
		sqlite3_bind_int(stmt.stmt, 1, maxvalue);
		sqlite3_bind_int(stmt.stmt, 2, minvalue);
		stmt.execute();

		sqlite3_reset(stmt.stmt);
	}
}

// Moves all events down by offset, overflows back to the top. Works in reverse.
void pattern::rotate_rows_events(int pluginparameter_id, int fromtime, int length, int offset) {

	if (offset == 0) return;

	sqlite::transaction tr(owner->db);

	int begintime = fromtime;
	int endtime = fromtime + length;

	int intotemp_time_begin;
	int intotemp_time_end;
	int update_time_begin;
	int update_time_end;
	int fromtemp_time_begin;

	if (offset < 0) {
		intotemp_time_begin = begintime;
		intotemp_time_end = begintime + -offset;
		update_time_begin = intotemp_time_end;
		update_time_end = endtime;
		fromtemp_time_begin = endtime - -offset;
	} else {
		intotemp_time_begin = endtime - offset;
		intotemp_time_end = endtime;
		update_time_begin = begintime;
		update_time_end = intotemp_time_begin;
		fromtemp_time_begin = begintime;
	}

	// step 1 -- insert into temp from source
	stringstream q;
	q << "insert into patternevent_temp (time, value, meta) select time - "<<intotemp_time_begin<<", value, meta from patternevent where";
	q << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id;
	q << "  and time >= "<<intotemp_time_begin<<" and time < "<<intotemp_time_end<<";";
	owner->exec_noret(q.str());

	// step 2 -- delete
	q.str("");
	q << "delete from patternevent where";
	q << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id;
	q << "  and time >= "<<intotemp_time_begin<<" and time < "<<intotemp_time_end<<";";
	owner->exec_noret(q.str());

	// step 3 -- update
	q.str("");
	q << "update patternevent set time = time + "<<offset<<" where";
	q << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id;
	q << "  and time >= "<<update_time_begin<<" and time < "<<update_time_end<<";";
	owner->exec_noret(q.str());

	// step 4 -- insert into dest from temp
	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)";
	q << "  select "<<id<<", "<<pluginparameter_id<<", time + "<<fromtemp_time_begin<<", value, meta from patternevent_temp;";
	owner->exec_noret(q.str());

	// step 5 -- cleanup temp
	owner->exec_noret("delete from patternevent_temp");

}

// ROTATE VALUES
// Used by: ROTATE NOTES
void pattern::rotate_vals_events(int pluginparameter_id, int fromtime, int length, int offset) {

	if (offset == 0) return;

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime
	  << "  order by time;";
	tableiterator tabit(owner->db, q.str());

	if (tabit.eof()) return;

	q.str("");
	q << "select value, meta from patternevent where id = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "update patternevent set value = ?1, meta = ?2 where id = ?3;";
	sqlite::statement stmt_updateval(owner->db, q.str());

	typedef std::vector<std::pair<int, int> > vals_t;
	vals_t vals;
	typedef std::vector<int> ids_t;
	ids_t ids;

	while (!tabit.eof()) {
		int evid = tabit.id();

		sqlite3_bind_int(stmt_getval.stmt, 1, evid);
		stmt_getval.execute();
		int value = sqlite3_column_int(stmt_getval.stmt, 0);
		int meta = sqlite3_column_int(stmt_getval.stmt, 1);
		sqlite3_reset(stmt_getval.stmt);

		vals.push_back(std::pair<int, int>(value, meta));
		ids.push_back(evid);

		tabit.next();
	}

	if (vals.size() == 1) return;

	vals_t::iterator middle;
	if (offset < 0) {
		int rot = -offset % (int)vals.size();
		middle = vals.begin() + (size_t)(rot);
	} else
	if (offset > 0) {
		int rot = offset % (int)vals.size();
		middle = vals.end() - (size_t)(rot);
	}

	// "the element pointed by middle becomes the new first element."
	std::rotate(vals.begin(), middle, vals.end());

	vals_t::iterator vals_it = vals.begin();
	ids_t::iterator ids_it = ids.begin();
	while (ids_it != ids.end()) {
		sqlite3_bind_int(stmt_updateval.stmt, 1, (*vals_it).first);
		sqlite3_bind_int(stmt_updateval.stmt, 2, (*vals_it).second);
		sqlite3_bind_int(stmt_updateval.stmt, 3, *ids_it);
		stmt_updateval.execute();
		sqlite3_reset(stmt_updateval.stmt);
		++vals_it;
		++ids_it;
	}

}

// ROTATE DISTANCES
void pattern::rotate_dist_events(int pluginparameter_id, int fromtime, int length, int offset) {

	if (offset == 0) return;

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime
	  << "  order by time;";
	tableiterator tabit(owner->db, q.str());

	if (tabit.eof()) return;

	q.str("");
	q << "select time from patternevent where id = ?1;";
	sqlite::statement stmt_gettime(owner->db, q.str());

	q.str("");
	q << "select time from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time > ?1 and time < "<<endtime
	  << "  order by time"
	  << "  limit 1;";
	sqlite::statement stmt_nexttime(owner->db, q.str());

	q.str("");
	q << "update patternevent set time = ?1 where id = ?2;";
	sqlite::statement stmt_updatetime(owner->db, q.str());

	typedef std::vector<int> dists_t;
	dists_t dists;
	typedef std::vector<int> ids_t;
	ids_t ids;

	int first_time = -1;
	while (!tabit.eof()) {
		int evid = tabit.id();

		sqlite3_bind_int(stmt_gettime.stmt, 1, evid);
		stmt_gettime.execute();
		int time = sqlite3_column_int(stmt_gettime.stmt, 0);
		sqlite3_reset(stmt_gettime.stmt);

		sqlite3_bind_int(stmt_nexttime.stmt, 1, time);
		stmt_nexttime.execute();
		int dist;
		if (stmt_nexttime.eof()) {
			if (first_time == -1) return;
			dist = (endtime - time) + (first_time - fromtime);
		} else {
			int next_time = sqlite3_column_int(stmt_nexttime.stmt, 0);
			sqlite3_reset(stmt_nexttime.stmt);
			dist = next_time - time;
		}

		if (first_time == -1) first_time = time;
		dists.push_back(dist);
		ids.push_back(evid);

		tabit.next();
	}

	//if (dists.size() == 1) return;

	dists_t::iterator middle;
	if (offset < 0) {
		int rot = -offset % (int)dists.size();
		middle = dists.begin() + (size_t)(rot);
	} else
	if (offset > 0) {
		int rot = offset % (int)dists.size();
		middle = dists.end() - (size_t)(rot);
	}

	// "the element pointed by middle becomes the new first element."
	std::rotate(dists.begin(), middle, dists.end());

	dists_t::iterator dists_it = dists.begin();
	ids_t::iterator ids_it = ids.begin();
	int prev_time = first_time;
	int prev_dist = 0;
	while (ids_it != ids.end()) {
		int new_time = prev_time + prev_dist;
		if (new_time >= endtime) new_time = fromtime + (new_time - endtime);
		sqlite3_bind_int(stmt_updatetime.stmt, 1, new_time);
		sqlite3_bind_int(stmt_updatetime.stmt, 2, *ids_it);
		stmt_updatetime.execute();
		sqlite3_reset(stmt_updatetime.stmt);
		prev_time = new_time;
		prev_dist = *dists_it;
		++dists_it;
		++ids_it;
	}

}

// Used by: REPLACE WAVES, ALL TO FIRST, SET VOLS
void pattern::set_events(int pluginparameter_id, int fromtime, int length, int value, int meta) {

	int endtime = fromtime + length;

	stringstream q;
	q << "update patternevent set value = "<<value<<", meta = "<<meta<<" where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
	owner->exec_noret(q.str());

}

// Used by: FIRST TO LAST
void pattern::replace_events(int pluginparameter_id, int fromtime, int length, int from_value, int from_meta, int to_value, int to_meta) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;
	int type, flags, minvalue, maxvalue, novalue;
	get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

	if (to_value == novalue) {
		remove_events(pluginparameter_id, fromtime, length, from_value, from_meta);
	} else
	if (from_value == novalue) {
		stringstream q;
		q << "insert into patternevent (pattern_id, pluginparameter_id, time, value)"
		  << "  values ("<<id<<", "<<pluginparameter_id<<", ?1, "<<to_value<<");";
		sqlite::statement stmt_insert(owner->db, q.str());

		q.str("");
		q << "select 1 from patternevent where"
		  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
		  << "  and time = ?1"
		  << "  limit 1;";
		sqlite::statement stmt_check(owner->db, q.str());

		for (int i = fromtime; i < endtime; ++i) {
			sqlite3_bind_int(stmt_check.stmt, 1, i);
			stmt_check.execute();
			if (stmt_check.eof()) {
				sqlite3_bind_int(stmt_insert.stmt, 1, i);
				stmt_insert.execute();
				sqlite3_reset(stmt_insert.stmt);
			}
			sqlite3_reset(stmt_check.stmt);
		}
	} else {
		stringstream q;
		q << "update patternevent set value = "<<to_value<<", meta = "<<to_meta<<" where"
		  << "  value = "<<from_value<<" and meta = "<<from_meta
		  << "  and pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
		  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
		owner->exec_noret(q.str());
	}
}

// Used by: REMOVE FIRST
void pattern::remove_events(int pluginparameter_id, int fromtime, int length, int value, int meta) {

	int endtime = fromtime + length;

	stringstream q;
	q << "delete from patternevent where"
	  << "  value = "<<value<<" and meta = "<<meta
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
	owner->exec_noret(q.str());

}

// sets max notelengths
// mode 0 == absolute length
// mode 1 == maximum length
// mode 2 == minimum length
// mode 3 == remove noteoffs
// offval is the value used -- ie 255 or 254
namespace { enum {
	notelen_abs = 0,
	notelen_max = 1,
	notelen_min = 2,
	notelen_remove = 3,
}; }
void pattern::notelength_events(int pluginparameter_id, int fromtime, int length, int desired_len, int mode, int off_val) {

	if (desired_len < 1) return;

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;

	if (mode == notelen_abs || mode == notelen_remove) {
		q << "delete from patternevent where"
		  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
		  << "  and time >= "<<fromtime<<" and time < "<<endtime
		  << "  and (value = 255 or value = 254);";
		owner->exec_noret(q.str());
	}

	if (mode == notelen_remove) return;

	q.str("");
	q << "select time, value from patternevent where id = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value)"
	  << "  values ("<<id<<", "<<pluginparameter_id<<", ?1, ?2);";
	sqlite::statement stmt_insert(owner->db, q.str());

	q.str("");
	q << "delete from patternevent where id = ?1;";
	sqlite::statement stmt_delete(owner->db, q.str());

	q.str("");
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime
	  << "  order by time;";
	tableiterator tabit(owner->db, q.str());

	bool is_dupe_off = false;
	bool got_valid_off= false;
	int last_note_time = -1;
	bool at_end = tabit.eof();

	while (!at_end)
	{
		int evid = tabit.id();
		tabit.next();
		if (tabit.eof()) at_end = true;

		sqlite3_bind_int(stmt_getval.stmt, 1, evid);
		stmt_getval.execute();
		int time = sqlite3_column_int(stmt_getval.stmt, 0);
		int value = sqlite3_column_int(stmt_getval.stmt, 1);
		sqlite3_reset(stmt_getval.stmt);

		bool is_noteoff = (value == 255 || value == 254);

		if (is_noteoff) {
			int this_offset = time - last_note_time;

			if (false
				|| is_dupe_off
				|| last_note_time == -1
				|| ((mode == notelen_max) && (this_offset > desired_len))
				|| ((mode == notelen_min) && (this_offset < desired_len))
			) {
				sqlite3_bind_int(stmt_delete.stmt, 1, evid);
				stmt_delete.execute();
				sqlite3_reset(stmt_delete.stmt);
			} else {
				got_valid_off = true;
			}

			is_dupe_off = true;
		} else
		if (!is_noteoff) {
			if (!got_valid_off && last_note_time != -1) {
				int this_offset = time - last_note_time;

				if (false
					|| ((mode == notelen_max) && (this_offset > desired_len))
					|| ((mode == notelen_abs) && (this_offset > desired_len))
				) {
					int off_time = last_note_time + desired_len;
					if (off_time < endtime) {
						sqlite3_bind_int(stmt_insert.stmt, 1, off_time);
						sqlite3_bind_int(stmt_insert.stmt, 2, off_val);
						stmt_insert.execute();
						sqlite3_reset(stmt_insert.stmt);
					}
				}
			}

			is_dupe_off = false;
			got_valid_off = false;
			last_note_time = time;
		}

		if (at_end) { // special case for when we're at the end
			if (!got_valid_off && last_note_time != -1) {
				int this_offset = endtime - last_note_time;

				if (false
					|| ((mode == notelen_abs) && (this_offset > desired_len))
					|| ((mode == notelen_max) && (this_offset > desired_len))
				) {
					int off_time = last_note_time + desired_len;
					if (off_time < endtime) {
						sqlite3_bind_int(stmt_insert.stmt, 1, off_time);
						sqlite3_bind_int(stmt_insert.stmt, 2, off_val);
						stmt_insert.execute();
						sqlite3_reset(stmt_insert.stmt);
					}
				}
			}
		}
	}

}

// VOLUMES
namespace { enum {
	vol_insert = 0,
	vol_remove = 1,
	vol_allow = 2,
}; }
void pattern::volumes_events(int note_pluginparameter_id, int vol_pluginparameter_id, int fromtime, int length, int mode) {
	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	stringstream q;

	if (mode == vol_insert || mode == vol_remove) {
		q << "delete from patternevent where"
		  << "  pattern_id = "<<id<<" and pluginparameter_id = "<<vol_pluginparameter_id
		  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
		owner->exec_noret(q.str());
	}

	if (mode == vol_insert) {
		int type, flags, minvalue, maxvalue, novalue;
		get_column_info(vol_pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

		q.str("");
		q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
		  << "  values ("<<id<<", "<<vol_pluginparameter_id<<", ?1, "<<maxvalue<<", 0);";
		sqlite::statement stmt_insert(owner->db, q.str());

		q.str("");
		q << "select id from patternevent where"
		  << "  pattern_id = "<<id<<" and pluginparameter_id = " << note_pluginparameter_id
		  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
		tableiterator tabit(owner->db, q.str());

		q.str("");
		q << "select time, value from patternevent where id = ?1;";
		sqlite::statement stmt_getval(owner->db, q.str());

		while (!tabit.eof()) {
			sqlite3_bind_int(stmt_getval.stmt, 1, tabit.id());
			stmt_getval.execute();
			int time = sqlite3_column_int(stmt_getval.stmt, 0);
			int value = sqlite3_column_int(stmt_getval.stmt, 1);
			sqlite3_reset(stmt_getval.stmt);

			if (value != 255 && value != 254) {
				sqlite3_bind_int(stmt_insert.stmt, 1, time);
				stmt_insert.execute();
				sqlite3_reset(stmt_insert.stmt);
			}

			tabit.next();
		}
	} else
	if (mode == vol_allow) {
		q.str("");
		q << "delete from patternevent where id in"
		  << "  (select id from patternevent A where"
		  << "    A.pattern_id = "<<id<<" and A.pluginparameter_id = "<<vol_pluginparameter_id
		  << "    and A.time >= "<<fromtime<<" and A.time < "<<endtime
		  << "    and not exists (select 1 from patternevent B where"
		  << "      B.pattern_id = "<<id<<" and B.pluginparameter_id = "<<note_pluginparameter_id
		  << "      and B.time >= "<<fromtime<<" and B.time < "<<endtime
		  << "      and B.time = A.time"
		  << "      and B.value != 255 and B.value != 254"
		  << "    )"
		  << "  );";
		owner->exec_noret(q.str());
	}
}

// Swaps values in columns if they are found in both the leftmost and rightmost track.
void pattern::swap_track_events(int left_idx, int right_idx, int fromtime, int length) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	int left_plug;
	int left_group;
	int left_track;
	int left_index;
	int right_plug;
	int right_group;
	int right_track;
	int right_index;

	stringstream q;
	q << "select pp.plugin_id, i.paramgroup, pp.paramtrack, c.pluginparameter_id, c.idx from patternformatcolumn c"
	  << "  inner join pluginparameter pp on c.pluginparameter_id = pp.id"
	  << "  inner join parameterinfo i on pp.parameterinfo_id = i.id"
	  << "  where c.patternformat_id = "<<patternformat_id<<" order by c.idx limit 1 offset ?;";
	sqlite::statement stmt_check(owner->db, q.str());

	sqlite3_bind_int(stmt_check.stmt, 1, left_idx);
	if (!stmt_check.execute() || stmt_check.eof()) return;
	left_plug = sqlite3_column_int(stmt_check.stmt, 0);
	left_group = sqlite3_column_int(stmt_check.stmt, 1);
	left_track = sqlite3_column_int(stmt_check.stmt, 2);
	left_index = sqlite3_column_int(stmt_check.stmt, 4);
	sqlite3_reset(stmt_check.stmt);

	sqlite3_bind_int(stmt_check.stmt, 1, right_idx);
	if (!stmt_check.execute() || stmt_check.eof()) return;
	right_plug = sqlite3_column_int(stmt_check.stmt, 0);
	right_group = sqlite3_column_int(stmt_check.stmt, 1);
	right_track = sqlite3_column_int(stmt_check.stmt, 2);
	right_index = sqlite3_column_int(stmt_check.stmt, 4);
	sqlite3_reset(stmt_check.stmt);

	if (left_plug != right_plug) return;
	if (left_group != right_group) return;
	if (left_track == right_track) return;

	q.str("");
	q << "select 1 from patternformatcolumn where patternformat_id = "<<patternformat_id
	  << "  and idx >= "<<left_index<<" and idx <= "<<right_index
	  << "  and pluginparameter_id = ?1"
	  << "  limit 1;";
	sqlite::statement stmt_match(owner->db, q.str());

	q.str("");
	q << "insert into patternevent_temp (time, value, meta) select time, value, meta from patternevent where"
	  << "	pattern_id = "<<id<<" and pluginparameter_id = ?1"
	  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
	sqlite::statement stmt_temp_insert(owner->db, q.str());

	q.str("");
	q << "delete from patternevent where"
	  << "	pattern_id = "<<id<<" and pluginparameter_id = ?1"
	  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
	sqlite::statement stmt_left_delete(owner->db, q.str());

	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  select "<<id<<", ?1, time, value, meta from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = ?2"
	  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
	sqlite::statement stmt_left_insert(owner->db, q.str());

	q.str("");
	q << "delete from patternevent where"
	  << "	pattern_id = "<<id<<" and pluginparameter_id = ?1"
	  << "  and time >= "<<fromtime<<" and time < "<<endtime<<";";
	sqlite::statement stmt_right_delete(owner->db, q.str());

	q.str("");
	q << "insert into patternevent (pattern_id, pluginparameter_id, time, value, meta)"
	  << "  select "<<id<<", ?1, time, value, meta from patternevent_temp;";
	sqlite::statement stmt_right_insert(owner->db, q.str());

	armstrong::storage::song song(owner);
	owner->get_song(song);

	q.str("");
	q << "select c.id, c.pluginparameter_id, c2.pluginparameter_id from patternformatcolumn c, patternformatcolumn c2"
	  << "  inner join pluginparameter pp on c.pluginparameter_id = pp.id"
	  << "  inner join pluginparameter pp2 on c2.pluginparameter_id = pp2.id"
	  << "  where c.patternformat_id = "<<patternformat_id
	  << "  and c.idx >= "<<left_index<<" and c.idx <= "<<right_index<<" and pp.paramtrack = "<<left_track
	  << "  and c2.patternformat_id = "<<patternformat_id
	  << "  and c2.idx >= "<<left_index<<" and c2.idx <= "<<right_index<<" and pp2.paramtrack = "<<right_track<<";";
	tableiterator tabit(owner->db, q.str());

	while (!tabit.eof()) {
		//armstrong::storage::patternformatcolumn col(owner);
		//song.get_patternformatcolumn_by_id(tabit.id(), col);
		int left_pluginparameter_id = sqlite3_column_int(tabit.stmt, 1);
		int right_pluginparameter_id = sqlite3_column_int(tabit.stmt, 2);

		sqlite3_bind_int(stmt_match.stmt, 1, left_pluginparameter_id);
		if (!stmt_match.execute() || stmt_match.eof()) break;
		sqlite3_reset(stmt_match.stmt);

		sqlite3_bind_int(stmt_temp_insert.stmt, 1, left_pluginparameter_id);
		stmt_temp_insert.execute();
		sqlite3_reset(stmt_temp_insert.stmt);

		sqlite3_bind_int(stmt_left_delete.stmt, 1, left_pluginparameter_id);
		stmt_left_delete.execute();
		sqlite3_reset(stmt_left_delete.stmt);

		sqlite3_bind_int(stmt_left_insert.stmt, 1, left_pluginparameter_id);
		sqlite3_bind_int(stmt_left_insert.stmt, 2, right_pluginparameter_id);
		stmt_left_insert.execute();
		sqlite3_reset(stmt_left_insert.stmt);

		sqlite3_bind_int(stmt_right_delete.stmt, 1, right_pluginparameter_id);
		stmt_right_delete.execute();
		sqlite3_reset(stmt_right_delete.stmt);

		sqlite3_bind_int(stmt_right_insert.stmt, 1, right_pluginparameter_id);
		stmt_right_insert.execute();
		sqlite3_reset(stmt_right_insert.stmt);

		owner->exec_noret("delete from patternevent_temp;");

		tabit.next();
	}

}

void pattern::swap_rows_events(int pluginparameter_id, int top_row, int bottom_row) {
	sqlite::transaction tr(owner->db);

	stringstream q;	
	q << "select id from patternevent where"
	  << "  pattern_id = "<<id<<" and pluginparameter_id = " << pluginparameter_id
	  << "  and time = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "update patternevent set time = ?2 where id = ?1;";
	sqlite::statement stmt_updateval(owner->db, q.str());

	sqlite3_bind_int(stmt_getval.stmt, 1, top_row);
	stmt_getval.execute();
	int top_id = -1;
	if (!stmt_getval.eof()) {
		top_id = sqlite3_column_int(stmt_getval.stmt, 0);	
	}
	sqlite3_reset(stmt_getval.stmt);

	sqlite3_bind_int(stmt_getval.stmt, 1, bottom_row);
	stmt_getval.execute();
	int bottom_id = -1;
	if (!stmt_getval.eof()) {
		bottom_id = sqlite3_column_int(stmt_getval.stmt, 0);	
	}
	sqlite3_reset(stmt_getval.stmt);

	if (top_id != -1) {
		sqlite3_bind_int(stmt_updateval.stmt, 1, top_id);
		sqlite3_bind_int(stmt_updateval.stmt, 2, bottom_row);
		stmt_updateval.execute();
		sqlite3_reset(stmt_updateval.stmt);
	}

	if (bottom_id != -1) {
		sqlite3_bind_int(stmt_updateval.stmt, 1, bottom_id);
		sqlite3_bind_int(stmt_updateval.stmt, 2, top_row);
		stmt_updateval.execute();
	}
}

// Harmonic inversion of chord blocks
namespace {
	struct note_t {
		note_t() {}
		note_t(int id, int value, int meta)
		:	id(id), value(value), meta(meta)
		{}
		int id, value, meta;
	};
	typedef std::vector<note_t> got_notes_t;
	struct pat_pos {
		pat_pos() {}
		pat_pos(int colid, int pluginparameterid)
		:	colid(colid), pluginparameter_id(pluginparameterid)
		{}
		int colid, pluginparameter_id;
	};
	struct notesorter {
		bool operator()(note_t const& lhs, note_t const& rhs) {
			return lhs.value < rhs.value;
		}
	};
}
void pattern::invert_chord_events(int left_idx, int right_idx, int fromtime, int length, int direction, int mode) {

	sqlite::transaction tr(owner->db);

	int endtime = fromtime + length;

	int idxcount = (right_idx - left_idx) + 1;
	std::stringstream q;
	q << "select id, pluginparameter_id from patternformatcolumn where patternformat_id = "<<patternformat_id
	  << " order by idx limit " << idxcount << " offset " << left_idx;
	tableiterator colit(owner->db, q.str());

	std::vector<pat_pos> note_cols;

	while (!colit.eof()) {
		int col_id = colit.id();
		int pluginparameter_id = sqlite3_column_int(colit.stmt, 1);

		int type, flags, minvalue, maxvalue, novalue;
		get_column_info(pluginparameter_id, &type, &flags, &minvalue, &maxvalue, &novalue);

		if (type == 0)
			note_cols.push_back(pat_pos(col_id, pluginparameter_id));

		colit.next();
	}

	if (note_cols.size() < 2) return;

	pat_pos const& first_col = note_cols.front();

	q.str("");
	q << "select id, time, value, meta from patternevent where pattern_id = "<<id
	  << "  and pluginparameter_id = "<<first_col.pluginparameter_id
	  << "  and time >= "<<fromtime<<" and time < "<<endtime
	  << "  order by time;";
	sqlite::statement stmt_getfirstvals(owner->db, q.str());

	q.str("");
	q << "select id, value, meta from patternevent where pattern_id = "<<id
	  << "  and pluginparameter_id = ?2"
	  << "  and time = ?1;";
	sqlite::statement stmt_getval(owner->db, q.str());

	q.str("");
	q << "delete from patternevent where id = ?1;";
	sqlite::statement stmt_delete(owner->db, q.str());

	q.str("");
	q << "insert into patternevent (pattern_id, time, pluginparameter_id, value, meta)"
	  << "  values ("<<id<<", ?1, ?2, ?3, ?4);";
	sqlite::statement stmt_insert(owner->db, q.str());

	got_notes_t got_notes;

	stmt_getfirstvals.next();
	while (!stmt_getfirstvals.eof()) {
		got_notes.clear();

		int first_id = sqlite3_column_int(stmt_getfirstvals.stmt, 0);
		int first_time = sqlite3_column_int(stmt_getfirstvals.stmt, 1);
		int first_value = sqlite3_column_int(stmt_getfirstvals.stmt, 2);
		int first_meta = sqlite3_column_int(stmt_getfirstvals.stmt, 3);
		got_notes.push_back(note_t(first_id, first_value, first_meta));

		sqlite3_bind_int(stmt_getval.stmt, 1, first_time);
		for (int i = 1; i < (int)note_cols.size(); ++i) {
			sqlite3_bind_int(stmt_getval.stmt, 2, note_cols[i].pluginparameter_id);

			stmt_getval.execute();
			if (!stmt_getval.eof()) {
				int evid = sqlite3_column_int(stmt_getval.stmt, 0);
				int value = sqlite3_column_int(stmt_getval.stmt, 1);
				int meta = sqlite3_column_int(stmt_getval.stmt, 2);
				got_notes.push_back(note_t(evid, value, meta));
			}
			sqlite3_reset(stmt_getval.stmt);
		}

		if (got_notes.size() >= 2) {
			for (int i = 0; i < (int)got_notes.size(); ++i) {
				sqlite3_bind_int(stmt_delete.stmt, 1, got_notes[i].id);
				stmt_delete.execute();
				sqlite3_reset(stmt_delete.stmt);
			}

			std::sort(got_notes.begin(), got_notes.end(), notesorter());

			got_notes_t rot_notes = got_notes;
			got_notes_t::iterator rot_middle;
			if (direction)
				rot_middle = rot_notes.begin() + (size_t)(1);
			else
				rot_middle = rot_notes.end() - (size_t)(1);
			std::rotate(rot_notes.begin(), rot_middle, rot_notes.end());

			int from_idx;
			int to_idx;
			if (mode) {
				if (direction)
					from_idx = (int)got_notes.size() - 1;
				else
					from_idx = 0;
				to_idx = from_idx + 1;
			} else {
				from_idx = 0;
				to_idx = (int)got_notes.size();
			}

			for (int i = from_idx; i < to_idx; ++i) {
				int got_val = got_notes[i].value;
				int rot_val = rot_notes[i].value;
				int got_base = (got_val & 0xF) - 1;
				int rot_base = (rot_val & 0xF) - 1;
				int got_oct = (got_val >> 4);
				int new_oct;

				if (direction) {
					if (rot_base > got_base)
						new_oct = got_oct;
					else
						new_oct = got_oct + 1;
				} else {
					if (rot_base < got_base)
						new_oct = got_oct;
					else
						new_oct = got_oct - 1;
				}

				rot_notes[i].value = (new_oct << 4) + (rot_base + 1);
			}

			sqlite3_bind_int(stmt_insert.stmt, 1, first_time);
			for (int i = 0; i < (int)rot_notes.size(); ++i) {
				pat_pos const& col = note_cols[i];

				sqlite3_bind_int(stmt_insert.stmt, 2, col.pluginparameter_id);
				sqlite3_bind_int(stmt_insert.stmt, 3, rot_notes[i].value);
				sqlite3_bind_int(stmt_insert.stmt, 4, rot_notes[i].meta);

				stmt_insert.execute();
				sqlite3_reset(stmt_insert.stmt);
			}
		}

		stmt_getfirstvals.next();
	}

}

namespace {

const int note_value_off = 255;
const int note_value_cut = 254;
const int note_value_none = 0;
const int max_tracks = 128;

struct noteevent {
	int id, time, column, value;
};

struct note {
	int on_id;
	int off_id;
	int time;
	int value;
	int length;
	int meta;
	std::vector<noteevent> notedata; // additional patternevents from the same track as the note, such as wave, amp, tracker fx, etc.
};

struct notematrix {
	std::vector<note> notes;

	note* get_note(int on_id) {
		for (std::vector<note>::iterator i = notes.begin(); i != notes.end(); ++i) {
			if (i->on_id == on_id) return &*i;
		}
		return 0;
	}

	void add_note(int time, int value, int meta, int length) {
		note n;
		n.on_id = 0;
		n.off_id = 0;
		n.time = time;
		n.value = value;
		n.meta = meta;
		n.length = length;
		notes.push_back(n);
	}
};

struct notecolumn {
	int pluginid, group, track, column;
	std::vector<int> datacolumns; // additional columns in this pattern format
};

void create_notematrix(pattern& p, int pluginid, notematrix* result, std::vector<int>* redundantnoteoffs) {
	note tracks[max_tracks];
	memset(tracks, 0, sizeof(tracks));

	std::stringstream q;
	q << "select pe.id, pe.time, pe.value, pp.paramtrack, pe.meta, pi.type, pi.paramcolumn, pi.paramgroup from patternevent pe inner join pluginparameter pp on pe.pluginparameter_id = pp.id inner join parameterinfo pi on pp.parameterinfo_id = pi.id where pe.pattern_id = " << p.id << " and pp.plugin_id = " << pluginid << " order by pe.time, pi.type;";

	tableiterator events(p.owner->db, q.str());

	// determine which parameter group the note belongs, so values in other groups will be ignored
	// if there are notes in both global and track parameter groups, prefer the track group
	int note_group = -1;
	for (; !events.eof(); events.next()) {
		int type = sqlite3_column_int(events.stmt, 5);
		int group = sqlite3_column_int(events.stmt, 7);
		if (type == 0 && (group == 1 || group == 2) && group > note_group) {
			note_group = group;
		}
	}

	events.reset();

	for (; !events.eof(); events.next()) {

		int eventid = events.id();
		int time = sqlite3_column_int(events.stmt, 1);
		int value = sqlite3_column_int(events.stmt, 2);
		int track = sqlite3_column_int(events.stmt, 3);
		int meta = sqlite3_column_int(events.stmt, 4);
		int type = sqlite3_column_int(events.stmt, 5);
		int column = sqlite3_column_int(events.stmt, 6);
		int group = sqlite3_column_int(events.stmt, 7);

		if (group != note_group) {
			continue;
		}

		if (type == 0) { // note

			if (tracks[track].value != note_value_none) {
				// note off on this track
				if (value == note_value_off || value == note_value_cut)
					tracks[track].off_id = eventid;
				tracks[track].length = time - tracks[track].time;
				result->notes.push_back(tracks[track]);
				tracks[track].value = note_value_none;
				tracks[track].notedata.clear();
			} else
			if (redundantnoteoffs != 0 && value == note_value_off || value == note_value_cut) {
				redundantnoteoffs->push_back(eventid);
			}

			if (value != note_value_off && value != note_value_cut) {
				// note on this track
				tracks[track].on_id = eventid;
				tracks[track].off_id = 0;
				tracks[track].value = buzz_to_midi_note(value);
				tracks[track].time = time;
				tracks[track].length = 0;
				tracks[track].meta = meta;
			}
		} else { // non-note pattern data
			if (tracks[track].value != note_value_none) {
				noteevent ev;
				ev.id = eventid;
				ev.time = time - tracks[track].time; // relative from note start
				ev.column = column;
				ev.value = value;
				tracks[track].notedata.push_back(ev);
			} else {
				// pattern events but no note playing, collect for deletion:
				redundantnoteoffs->push_back(eventid);
			}
		}
	}
	events.destroy();

	// emit non-terminated notes with their end at end-of-pattern, or support -1 for infinite length?
	for (int i = 0; i < max_tracks; i++) {
		if (tracks[i].value != note_value_none) {
			tracks[i].length = -1;
			result->notes.push_back(tracks[i]);
		}
	}
}

int allocate_track(note** tracks, int time) {
	for (int i = 0; i < max_tracks; i++) {
		if (tracks[i] == 0 || tracks[i]->value == note_value_none || (tracks[i]->length != -1 && (tracks[i]->time + tracks[i]->length) <= time))
			return i;
	}
	return -1;
}

void update_event(pattern* result, int id, int pluginid, int group, int track, int column, int time, int value, int meta, bool isnote) {
	if (id == 0) {
		result->insert_value(pluginid, group, track, column, time, value, meta);
	} else 
	if ((isnote && value == note_value_none) || time >= result->length ) {
		// delete deleted notes and events past end of pattern
		result->delete_value(id);
	} else {
		result->update_value(id, pluginid, group, track, column, time, value, meta);
	}
}

void update_event_data(pattern* result, note* sourcenote, notecolumn& column) {
	for (std::vector<noteevent>::iterator i = sourcenote->notedata.begin(); i != sourcenote->notedata.end(); ++i) {

		// does the target column have a target column for sourcenote->data?
		// ie, if the note was moved from one track to another, its pattern format may not have all the columns as the original track
		bool column_in_format = false;
		for (std::vector<int>::iterator j = column.datacolumns.begin(); j != column.datacolumns.end(); ++j) {
			if (*j == i->column) {
				column_in_format = true;
				break;
			}
		}

		if (column_in_format) {
			update_event(result, i->id, column.pluginid, column.group, column.track, i->column, sourcenote->time + i->time, i->value, 0, false);
		} else {
			// target track does not have this column - send the event into eternal cyberspace
			result->delete_value(i->id);
		}
	}
}

void delete_event_data(pattern* result, note* sourcenote) {
	for (std::vector<noteevent>::iterator i = sourcenote->notedata.begin(); i != sourcenote->notedata.end(); ++i) {
		result->delete_value(i->id);
	}
}

// update/insert/delete in the db - ignores notes in voices > available notes columns
void create_pattern(notematrix& m, std::vector<notecolumn>& columns, pattern* result) {

	note* tracks[max_tracks];
	memset(tracks, 0, sizeof(tracks));

	for (std::vector<note>::iterator i = m.notes.begin(); i != m.notes.end(); ++i) {
		int track = allocate_track(tracks, i->time);
		if (track == -1 || track >= (int)columns.size()) {
			// not enough voices - delete note
			if (i->on_id != 0) {
				result->delete_value(i->on_id);
				delete_event_data(result, &*i);
			}
			continue;
		}
		if (tracks[track] && tracks[track]->value != note_value_none && (tracks[track]->time + tracks[track]->length) < i->time) {
			// terminate a previously playing note which is not being cut off
			int noteend = tracks[track]->time + tracks[track]->length;
			update_event(result, tracks[track]->off_id, columns[track].pluginid, columns[track].group, columns[track].track, columns[track].column, noteend, note_value_off, 0, true);
			tracks[track]->off_id = 0; // reset the off-id so we know which are left over at the end
		}
		int notevalue = i->value;
		if (notevalue != note_value_none) {
			notevalue = midi_to_buzz_note(notevalue);
			tracks[track] = &*i;
		}
		update_event(result, i->on_id, columns[track].pluginid, columns[track].group, columns[track].track, columns[track].column, i->time, notevalue, i->meta, true);
		update_event_data(result, &*i, columns[track]);
	}

	// emit note terminators
	for (int i = 0; i < max_tracks; i++) {
		if (i >= (int)columns.size()) continue;
		if (tracks[i] && tracks[i]->value != note_value_none && tracks[i]->length != -1) {
			// terminate a previously playing note
			int noteend = tracks[i]->time + tracks[i]->length;
			update_event(result, tracks[i]->off_id, columns[i].pluginid, columns[i].group, columns[i].track, columns[i].column, noteend, note_value_off, 0, true);
			tracks[i]->off_id = 0; // reset the off-id so we know which are left over at the end
		}
	}

	// delete leftover note-offs
	for (std::vector<note>::iterator i = m.notes.begin(); i != m.notes.end(); ++i) {
		if (i->off_id != 0) {
			result->delete_value(i->off_id);
		}
	}
}

bool notesorter(const note& a, const note& b) {
	if (a.time == b.time)
		return a.value < b.value;
	return a.time < b.time;
}

void get_distinct_plugins(pattern& p, int* eventids, int numevents, std::vector<int>* result) {
	std::stringstream q;
	q << "select distinct pp.plugin_id from patternevent pe inner join pluginparameter pp on pp.id = pe.pluginparameter_id where pe.id in (";
	for (int i = 0; i < numevents; i++) {
		if (i > 0)
			q << ", ";
		q << eventids[i];
	}
	q << ")";

	tableiterator ids(p.owner->db, q.str());

	while (!ids.eof()) {
		result->push_back(ids.id());
		ids.next();
	}
	ids.destroy();
}

void delete_events_by_id(pattern& p, std::vector<int>& eventids) {
	for (std::vector<int>::iterator i = eventids.begin(); i != eventids.end(); ++i) {
		p.delete_value(*i);
	}
}

void get_track_columns(pattern& p, int pluginid, int group, int track, notecolumn& notecol) {
	std::stringstream q;
	q << "select pi.paramcolumn from patternformatcolumn pc "
	  << " inner join patternformat pf on pf.id = pc.patternformat_id "
	  << " inner join pattern p on p.patternformat_id = pf.id "
	  << " inner join pluginparameter pp on pp.id = pc.pluginparameter_id "
	  << " inner join parameterinfo pi on pi.id = pp.parameterinfo_id "
	  << "where p.id = " << p.id << " and pp.plugin_id = " << pluginid << " and pi.paramgroup = " << group << " and pp.paramtrack = " << track << " and pi.type != 0 "
	  << "order by pi.paramcolumn";

	tableiterator eventcols(p.owner->db, q.str());

	while (!eventcols.eof()) {
		int col = sqlite3_column_int(eventcols.stmt, 0);
		notecol.datacolumns.push_back(col);
		eventcols.next();
	}
	eventcols.destroy();
}

void get_note_columns(pattern& p, int pluginid, std::vector<notecolumn>* result) {
	std::stringstream q;
	q << "select pi.paramgroup, pp.paramtrack, pi.paramcolumn from patternformatcolumn pc "
	  << " inner join patternformat pf on pf.id = pc.patternformat_id "
	  << " inner join pattern p on p.patternformat_id = pf.id "
	  << " inner join pluginparameter pp on pp.id = pc.pluginparameter_id "
	  << " inner join parameterinfo pi on pi.id = pp.parameterinfo_id "
	  << "where p.id = " << p.id << " and pp.plugin_id = " << pluginid << " and pi.type = 0 "
	  << "order by pp.paramtrack, pi.paramcolumn";
	
	tableiterator notecols(p.owner->db, q.str());

	while (!notecols.eof()) {
		notecolumn col;
		col.pluginid = pluginid;
		col.group = sqlite3_column_int(notecols.stmt, 0);
		col.track = sqlite3_column_int(notecols.stmt, 1);
		col.column = sqlite3_column_int(notecols.stmt, 2);
		
		get_track_columns(p, pluginid, col.group, col.track, col); // couldve rewritten the sql with grouping but .. .. doh

		result->push_back(col);
		notecols.next();
	}
	notecols.destroy();
}

int get_event_plugin_id(pattern& p, int id) {
	std::stringstream q;
	q << "select pp.plugin_id from patternevent pe inner join pluginparameter pp on pp.id = pe.pluginparameter_id where pe.id = " << id;
	int result;
	if (p.owner->exec_scalar(q.str(), &result))
		return result;
	return 0;
}

}

void pattern::move_and_transpose_notes(int* eventids, int numevents, int timeshift, int pitchshift, int mode) {

	// TODO: operate on the affected plugin/time range of a pattern only. currently updates the entire pattern

	// find distinct plugins refered by the events
	std::vector<int> pluginids;
	get_distinct_plugins(*this, eventids, numevents, &pluginids);

	// move+transpose events for each plugin separately
	for (std::vector<int>::iterator plugit = pluginids.begin(); plugit != pluginids.end(); ++plugit) {
		notematrix m;
		std::vector<int> noteoffs;
		create_notematrix(*this, *plugit, &m, &noteoffs);

		for (int i = 0; i < numevents; i++) {
			note* n = m.get_note(eventids[i]);
			if (n == 0) continue;

			if (mode == 1) {
				// move note start
				if (n->length - timeshift > 0) {
					n->time += timeshift;
					n->length -= timeshift;
				} else if (n->length - timeshift < 0) {
					// moved start after end
					int movetime = n->time + n->length;
					n->length = -(n->length - timeshift);
					n->time = movetime;
				}
			} else
			if (mode == 2) {
				// when changing the end of an infinite note, do it relative to the pattern end
				if (n->length == -1) n->length = this->length - n->time;

				// move note end
				if (n->length + timeshift > 0) {
					n->length += timeshift;
				} else if (n->length + timeshift < 0) {
					// moved end before start
					int movetime = n->time + n->length + timeshift;
					n->length = n->time - movetime;
					n->time = movetime;
				}
			} else {
				// mode 0 and fallback: move entire note
				n->time += timeshift;
			}

			// delete notes moved entirely before pattern start
			if (n->length != -1 && n->time + n->length < 0) {
				n->value = note_value_none;
				n->time = 0;
				n->length = 0;
			}

			// cut off notes moved beyond pattern start
			if (n->time < 0) {
				if (n->length != -1)
					n->length += n->time; // subtract 
				n->time = 0;
			}

			// delete notes moved beyond pattern end
			if (n->time >= length) {
				n->value = note_value_none;
				n->time = 0;
				n->length = 0;
			}

			// delete noteoffs moved beyond pattern end
			if (n->length != -1 && n->time + n->length >= length) {
				n->length = -1;
			}

			// make sure the note value is valid
			if (n->value != note_value_none) {
				n->value += pitchshift;
				if (n->value < 1)
					n->value = 1;
				if (n->value > 127)
					n->value = 127;
			}
		}

		// sort by time for optimal distribution
		std::sort(m.notes.begin(), m.notes.end(), notesorter);

		std::vector<notecolumn> columns;
		get_note_columns(*this, *plugit, &columns);
		delete_events_by_id(*this, noteoffs);
		create_pattern(m, columns, this);
	}
}

void pattern::insert_note(int pluginid, int time, int note, int length) {
	notematrix m;
	std::vector<int> noteoffs;
	create_notematrix(*this, pluginid, &m, &noteoffs);

	m.add_note(time, buzz_to_midi_note(note), 0, length);

	// sort by time for optimal distribution
	std::sort(m.notes.begin(), m.notes.end(), notesorter);

	std::vector<notecolumn> columns;
	get_note_columns(*this, pluginid, &columns);

	delete_events_by_id(*this, noteoffs);
	create_pattern(m, columns, this);
}


void pattern::update_note(int id, int time, int notevalue, int length) {
	int pluginid = get_event_plugin_id(*this, id);
	assert(pluginid != 0);

	notematrix m;
	std::vector<int> noteoffs;
	create_notematrix(*this, pluginid, &m, &noteoffs);

	note* n = m.get_note(id);
	assert(n != 0);
	if (notevalue != note_value_none) notevalue = buzz_to_midi_note(notevalue);
	n->value = notevalue;
	n->time = time;
	n->length = length;

	// sort by time for optimal distribution
	std::sort(m.notes.begin(), m.notes.end(), notesorter);

	std::vector<notecolumn> columns;
	get_note_columns(*this, pluginid, &columns);

	delete_events_by_id(*this, noteoffs);
	create_pattern(m, columns, this);
}

} // END namespace storage
} // END namespace armstrong
