#pragma once

namespace dbgenpp {

/*

	dbgenpp compile time library:

*/

// sqlite3_stmt* <- table entities
template <typename T>
struct bind_column {
	sqlite3_stmt* stmt;
	T& data;
	int count;
	bool is_nullable_key;
	bind_column(sqlite3_stmt* _stmt, T& _data):stmt(_stmt), data(_data), count(0) {}

	template <typename C>
	void operator()(const C&) {
		std::string keytable = C::keytable();
		is_nullable_key = (C::is_nullable && keytable != "");
		bind_column_value(count + 1, data.*C::member());
		count++;
	}

	void bind_column_value(int index, const int& value) {
		if (is_nullable_key && value == 0) {
			// interpret 0 as null foreign key
			sqlite3_bind_null(stmt, index);
		} else {
			sqlite3_bind_int(stmt, index, value);
		}
	}

	void bind_column_value(int index, const std::string& value) {
		sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_STATIC);
	}

	void bind_column_value(int index, const double& value) {
		sqlite3_bind_double(stmt, index, value);	
	}

	void bind_column_value(int index, const std::vector<unsigned char>& value) {
		if (value.size() > 0)
			sqlite3_bind_blob(stmt, index, &value.front(), (int)value.size(), SQLITE_STATIC);
	}
};

template <typename T>
void operator <<(sqlite3_stmt* stmt, T& data) {
	boost::mpl::for_each<typename T::column_members>(bind_column<T>(stmt, data));
};


// sqlite3_stmt* -> table entities
template <typename T>
struct column_value {
	sqlite3_stmt* stmt;
	T& data;
	int count;
	column_value(sqlite3_stmt* _stmt, T& _data):stmt(_stmt), data(_data), count(0) {}

	template <typename C>
	void operator()(const C&) {
		get_column_value(count, &(data.*C::member()));
		count++;
	}

	void get_column_value(int index, int* result) {
		*result = sqlite3_column_int(stmt, index);
	}

	void get_column_value(int index, std::string* result) {
		*result = (const char*)sqlite3_column_text(stmt, index);
	}

	void get_column_value(int index, double* result) {
		*result = (double)sqlite3_column_double(stmt, index);
	}

	void get_column_value(int index, std::vector<unsigned char>* result) {
		int len = sqlite3_column_bytes(stmt, index);
		unsigned char* ptr = (unsigned char*)sqlite3_column_blob(stmt, index);
		result->assign(ptr, ptr + len);
	}
};

template <typename T>
void operator >>(sqlite3_stmt* stmt, T& data) {
	boost::mpl::for_each<typename T::column_members>(column_value<T>(stmt, data));
};


// sqlite3_value** -> table entities
template <typename T>
struct param_value {
	sqlite3_value** row;
	T& data;
	int count;
	param_value(sqlite3_value** _row, T& _data):row(_row), data(_data), count(0) {}

	template <typename C>
	void operator()(const C&) {
		get_column_value(count + 1, &(data.*C::member()));
		count++;
	}

	void get_column_value(int index, int* result) {
		*result = sqlite3_value_int(row[index]);
	}

	void get_column_value(int index, std::string* result) {
		*result = (const char*)sqlite3_value_text(row[index]);
	}

	void get_column_value(int index, double* result) {
		*result = (double)sqlite3_value_double(row[index]);
	}

	void get_column_value(int index, std::vector<unsigned char>* result) {
		int len = sqlite3_value_bytes(row[index]);
		unsigned char* ptr = (unsigned char*)sqlite3_value_blob(row[index]);
		result->assign(ptr, ptr + len);
	}
};

template <typename T>
void operator >>(sqlite3_value** row, T& data) {
	boost::mpl::for_each<typename T::column_members>(param_value<T>(row, data));
};



template <typename T>
struct column_update_statement {
	std::ostream& strm;
	int count;

	column_update_statement(std::ostream& _strm):strm(_strm), count(0) {}

	template <typename C>
	void operator()(const C&) {
		if (count > 0) strm << ", ";
		strm << C::name() << " = ?";
		count++;
	}
};

template <typename T>
std::string get_update_statement() {
	std::stringstream strm;
	strm << "update " << T::table_traits::name() << " set ";
	boost::mpl::for_each<typename T::column_members>(column_update_statement<T>(strm));
	strm << " where id = ?1;";
	return strm.str();
}

template <typename T>
inline void sqlquote(std::ostream& strm, const T& s, bool is_nullable_key) {
	strm << s;
}

template <>
inline void sqlquote(std::ostream& strm, const int& s, bool is_nullable_key) {
	if (is_nullable_key && s == 0) {
		strm << "null";
	} else {
		strm << s;
	}
}

template <>
inline void sqlquote(std::ostream& strm, const std::string& s, bool is_nullable_key) {
	strm << "'";
	for (std::string::const_iterator i = s.begin(); i != s.end(); ++i) {
		if (*i == '\'')
			strm << "''";
		else
			strm << *i;
	}
	strm << "'";
}

template <>
inline void sqlquote(std::ostream& strm, const std::vector<unsigned char>& s, bool is_nullable_key) {
	strm << "X'";
	for (std::vector<unsigned char>::const_iterator i = s.begin(); i != s.end(); ++i) {
		strm << std::hex << std::setw(2) << std::setfill('0') << (int)*i;
	}
	strm << "'";
}

template <typename T>
struct column_update_statement_row {
	const T& row;
	std::ostream& strm;
	int count;

	column_update_statement_row(std::ostream& _strm, const T& _row):strm(_strm), row(_row), count(0) {}

	template <typename C>
	void operator()(const C&) {
		if (C::is_primary) return ;
		if (count > 0) strm << ", ";
		strm << C::name() << " = ";

		std::string keytable = C::keytable();
		bool is_nullable_key = (C::is_nullable && keytable != "");

		sqlquote(strm, row.*C::member(), is_nullable_key);
		count++;
	}
};

template <typename T>
std::string get_update_statement(const T& row) {
	std::stringstream strm;
	strm << "update " << T::table_traits::name() << " set ";
	boost::mpl::for_each<typename T::column_members>(column_update_statement_row<T>(strm, row));
	strm << " where id = " << row.id << ";"; // TODO: dont update primary keys, where on primary keys
	return strm.str();
}


template <typename T>
struct column_names {
	std::ostream& strm;
	int count;

	column_names(std::ostream& _strm):strm(_strm), count(0) {}

	template <typename C>
	void operator()(const C&) {
		if (count > 0) strm << ", ";
		strm << C::name();
		count++;
	}
};

template <typename T>
struct column_values {
	const T& row;
	std::ostream& strm;
	int count;

	column_values(std::ostream& _strm, const T& _row):strm(_strm), row(_row), count(0) {}

	template <typename C>
	void operator()(const C&) {
		if (count > 0) strm << ", ";
		std::string keytable = C::keytable();
		bool is_nullable_key = (C::is_nullable && keytable != "");
		sqlquote(strm, row.*C::member(), is_nullable_key);
		count++;
	}
};

template <typename T>
std::string get_insert_statement(const T& row) {
	std::stringstream strm;
	strm << "insert into " << T::table_traits::name() << " (";
	boost::mpl::for_each<typename T::column_members>(column_names<T>(strm));
	strm << ") values (";
	boost::mpl::for_each<typename T::column_members>(column_values<T>(strm, row));
	strm << ");";
	return strm.str();
}


/*

	dbgenpp run time library:

*/

template <typename EV>
struct event_source {
	virtual bool notify_listeners(EV* ev) = 0;
};

template <typename EV>
struct event_listener {
	virtual ~event_listener() { };
	virtual void update_document(EV* ev) = 0;
};

template <typename T, typename EV>
bool table_notify_callback(sqlite3_context* context, sqlite3_value** values) {
	event_source<EV>* self = (event_source<EV>*)sqlite3_user_data(context);
	assert(self != 0);

	int mode = sqlite3_value_int(values[0]);

	T newdata;
	T olddata;
	values >> newdata;

	EV ev;
	ev.newdata = newdata;
	ev.id = sqlite3_value_int(values[1]);

	switch (mode) {
		case 0:		
			ev.type = T::table_traits::after_insert();
			break;
		case 1:
			ev.type = T::table_traits::after_delete();
			break;
		case 2:
			ev.type = T::table_traits::after_update();
			&values[boost::mpl::size<typename T::column_members>::type::value] >> olddata;
			ev.olddata = olddata;
			break;
		case 10:
			ev.type = T::table_traits::before_insert();
			break;
		case 11:
			ev.type = T::table_traits::before_delete();
			break;
		case 12:
			ev.type = T::table_traits::before_update();
			break;
	}
	// return false to RAISE(ABORT) inside the trigger
	return self->notify_listeners(&ev);
}

}
