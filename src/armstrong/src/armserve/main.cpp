#include <string>
#include <vector>
#include <queue>
#include <iostream>
#define _WIN32_WINNT 0x0501
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "include/binstream.h"
#include "include/message.h"
#include "include/session.h"
#include "../storage/document.h"
#include "../mixing/convertsample.cpp"

using std::cout;
using std::cerr;
using std::wcout;
using std::endl;

struct serverdocument {
	std::string name;
	std::string password;
	armstrong::storage::document document;
	std::vector<boost::shared_ptr<session> > clients;
};

struct storage_server :
	armstrong::storage::documentlistener 
{
	boost::asio::ip::tcp::acceptor acceptor;
	std::vector<boost::shared_ptr<serverdocument> > documents;
	std::vector<boost::shared_ptr<session> > sessions;
	std::string datapath;

	storage_server(boost::asio::io_service& io_service, const boost::asio::ip::tcp::endpoint& endpoint, const std::string& userpath) : acceptor(io_service, endpoint) {
		datapath = userpath;
		start_accept();
	}

	bool create_project(const std::string& name, const std::string& password, const std::vector<char>& bytes) {
		boost::filesystem::path projectpath = boost::filesystem::path(datapath) / boost::filesystem::path(name);
		if (boost::filesystem::exists(projectpath)) {
			return false;
		}

		boost::filesystem::create_directories(projectpath);

		{
			boost::filesystem::path pwpath = projectpath / "password.txt";
			std::ofstream f(pwpath.string(), std::ios::out | std::ios::binary);
			f.write(&password.front(), password.length());
			f.close();
		}

		{
			boost::filesystem::path dbpath = projectpath / "song.armdb";
			std::ofstream f(dbpath.string(), std::ios::out | std::ios::binary);
			f.write(&bytes.front(), bytes.size());
			f.close();
		}

		return true;
	}

	boost::shared_ptr<serverdocument> get_project(const std::string& name) {
		for (std::vector<boost::shared_ptr<serverdocument> >::iterator i = documents.begin(); i != documents.end(); ++i) {
			if (name == (*i)->name) {
				return *i;
			}
		}
		return 0;
	}

	boost::shared_ptr<serverdocument> get_project_by_client(boost::shared_ptr<session> client) {
		for (std::vector<boost::shared_ptr<serverdocument> >::iterator i = documents.begin(); i != documents.end(); ++i) {
			std::vector<boost::shared_ptr<session> >::iterator clientit =  std::find((*i)->clients.begin(), (*i)->clients.end(), client);
			if (clientit != (*i)->clients.end()) {
				return *i;
			}
		}
		return 0;
	}

	boost::shared_ptr<serverdocument> open_project(const std::string& name, const std::string& openpassword) {
		boost::shared_ptr<serverdocument> d = get_project(name);
		if (d != 0) {
			if (d->password != openpassword) {
				return 0;
			}
			return d;
		}

		// try to load password
		boost::filesystem::path pwpath = boost::filesystem::path(datapath) / boost::filesystem::path(name) / "password.txt";
		std::ifstream f(pwpath.string(), std::ios::in | std::ios::binary);
		if (!f) {
			return 0;
		}
		std::string password((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		f.close();

		if (password != openpassword) {
			return 0;
		}

		d = boost::shared_ptr<serverdocument>(new serverdocument());
		d->name = name;
		d->password = password;
		d->document.temppath = boost::filesystem::temp_directory_path().string();
		d->document.undoredo_enabled = false;
		if (!d->document.open(":memory:")) {
			cout << "init error" << endl;
			d.reset();
			return 0;
		}
		d->document.exec_noret("PRAGMA foreign_keys = ON;");

		boost::filesystem::path dbpath = boost::filesystem::path(datapath) / boost::filesystem::path(name) / "song.armdb";
		if (!d->document.load(dbpath.string())) {
			cout << "coudlnt open startup document" << endl;
			d.reset();
			return 0;
		}

		d->document.register_listener(this);

		documents.push_back(d);
		return d;
	}

	static bool is_valid_name_char(int c) {
		return isalnum(c) || c == '_';
	}

	bool is_valid_name(const std::string &str) {
		return std::find_if_not(str.begin(), str.end(), is_valid_name_char) == str.end();
	}

	void handle_message(boost::shared_ptr<session> client, boost::shared_ptr<message> items) {

		int request_call_id;
		std::string command;
		if (!client->verify_message(items, &request_call_id, &command)) {
			client->send_response(0, message_status_protocol_error, "Bad message format");
			return ;
		}

		if (command == "OPEN") {
			std::string name;
			std::string password;

			if (!client->verify_message_parameter(items, 2, &name) || name.empty() || !is_valid_name(name)) {
				client->send_response(request_call_id, message_status_bad_arguments, "Bad OPEN arguments");
				return ;
			}

			if (!client->verify_message_parameter(items, 3, &password)) {
				client->send_response(request_call_id, message_status_bad_arguments, "Bad OPEN arguments");
				return ;
			}

			request_open(client, request_call_id, name, password);
		} else if (command == "CREATE") {
			std::string name;
			std::string password;

			if (!client->verify_message_parameter(items, 2, &name) || name.empty() || !is_valid_name(name)) {
				client->send_response(request_call_id, message_status_bad_arguments, "Bad CREATE arguments");
				return ;
			}

			if (!client->verify_message_parameter(items, 3, &password)) {
				client->send_response(request_call_id, message_status_bad_arguments, "Bad CREATE arguments");
				return ;
			}

			if (!client->verify_message_parameter(items, 4, (std::string*)0)) {
				client->send_response(request_call_id, message_status_bad_arguments, "Bad CREATE arguments");
				return ;
			}

			request_create(client, request_call_id, name, password, (*items)[4].bytes);
		} else if (command == "DELETE") {
			std::string name;
			std::string password;

			if (!client->verify_message_parameter(items, 2, &name) || name.empty() || !is_valid_name(name)) {
				client->send_response(request_call_id, message_status_bad_arguments, "Bad CREATE arguments");
				return ;
			}

			if (!client->verify_message_parameter(items, 3, &password)) {
				client->send_response(request_call_id, message_status_bad_arguments, "Bad CREATE arguments");
				return ;
			}

			client->send_response(request_call_id, message_status_not_implemented, "Not implemented");
		} else if (command == "EXEC") {
			std::string description;
			std::string query;
			if (!client->verify_message_parameter(items, 2, &description)) {
				client->send_response(request_call_id, message_status_bad_arguments, "Bad EXEC arguments");
				return ;
			}
			if (!client->verify_message_parameter(items, 3, &query)) {
				client->send_response(request_call_id, message_status_bad_arguments, "Bad EXEC arguments");
				return ;
			}
			request_exec(client, request_call_id, description, query);
		} else if (command == "WAVELEVEL") {
			client->send_response(request_call_id, message_status_not_implemented, "Not implemented");
		} else if (command == "STATUS") {
			boost::shared_ptr<message> response;
			client->create_return_message(request_call_id, message_status_ok, "OK", &response);
			response->push_back(message_item((int)sessions.size()));
			response->push_back(message_item((int)documents.size()));
			boost::shared_ptr<serverdocument> d = get_project_by_client(client);
			if (d != 0) {
				response->push_back(message_item(d->name));
				response->push_back(message_item((int)d->clients.size()));
			} else {
				response->push_back(message_item(""));
				response->push_back(message_item(0));
			}
			client->send(response);
		} else if (command == "RET") {
			// handle when needed
		} else {
			cerr << "unknown command " << command;
			client->send_response(request_call_id, message_status_bad_command, "Unknown command");
		}
	}

	void handle_error(boost::shared_ptr<session> client, const boost::system::error_code& e) {
		cerr << "error: " << e.message() << endl;
		client->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		client->socket.close();
		std::vector<boost::shared_ptr<session> >::iterator i = std::find(sessions.begin(), sessions.end(), client);
		if (i != sessions.end()) {
			sessions.erase(i);
			cerr << "server removed client" << endl;
		}

		set_active_client_project(client, 0);
	}

	void update_document(armstrong::storage::document_event_data* e) {
		//cout << "server document event " << e->type << endl;
	}

	void request_create(boost::shared_ptr<session> client, int request_call_id, const std::string& name, const std::string& password, const std::vector<char>& bytes) {
		if (!create_project(name, password, bytes)) {
			client->post_response(request_call_id, message_status_already_exists, "Cannot create project");
			return ;
		}
		client->post_response(request_call_id, message_status_ok, "OK");
	}

	void set_active_client_project(boost::shared_ptr<session> client, boost::shared_ptr<serverdocument> d) {
		boost::shared_ptr<serverdocument> old_d = get_project_by_client(client);
		if (old_d != 0) {
			old_d->clients.erase(std::remove(old_d->clients.begin(), old_d->clients.end(), client), old_d->clients.end());
		}

		if (d != 0) {
			d->clients.push_back(client);
		}

		if (old_d != 0 && old_d->clients.empty()) {
			// in-memory: save after last client disconnects
			// disk-based: just close and leave as is
			cout << "saving and closing document" << endl;
			boost::filesystem::path projectpath = boost::filesystem::path(datapath) / boost::filesystem::path(old_d->name) / boost::filesystem::path("song.armdb");
			boost::filesystem::remove(projectpath);
			old_d->document.save(projectpath.string());
			old_d->document.close();
			documents.erase(std::remove(documents.begin(), documents.end(), old_d), documents.end());
		}
	}

	void request_open(boost::shared_ptr<session> client, int request_call_id, const std::string& name, const std::string& password) {
		cout << "request open project '" << name << "', password '" << password << "'" << endl;

		boost::shared_ptr<serverdocument> d = open_project(name, password);
		if (d == 0) {
			client->post_response(request_call_id, message_status_not_found, "Project not found or wrong password");
			return ;
		}

		// remove client from previous project, unload previous project if last reference
		set_active_client_project(client, d);

		boost::filesystem::path tempfile = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
		d->document.save(tempfile.string());

		boost::shared_ptr<message> response;
		client->create_return_message(request_call_id, message_status_ok, "OK", &response);
		response->push_back(message_item(std::vector<char>()));

		message_item& item = response->back();

		std::ifstream f(tempfile.string(), std::ios::in | std::ios::binary);
		item.bytes.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		f.close();
		boost::filesystem::remove(tempfile);

		client->send(response);
	}


	void reqexec(boost::shared_ptr<session> client, const std::string& description, const std::string& query) {
		boost::shared_ptr<message> request = boost::shared_ptr<message>(new message());
		request->push_back(message_item(0));
		request->push_back(message_item("REQEXEC"));
		request->push_back(message_item(description));
		request->push_back(message_item(query));
		client->send(request);
	}

	void request_exec(boost::shared_ptr<session> client, int request_call_id, const std::string& description, std::string& query) {
		// TODO: can parse the query with EXPLAIN and fail if the client tries to DROP etc
		boost::shared_ptr<serverdocument> d = get_project_by_client(client);
		if (d == 0) {
			cout << "EXEC failed - no open project" << endl;
			client->send_response(request_call_id, message_status_not_found, "EXEC failed - no open project");
			return ;
		}

		// run query on local db 
		d->document.exec_noret("begin;");
		cout << "executing server sql: " << query << endl;
		bool result = d->document.exec_noret(query);
		d->document.exec_noret("commit;");

		if (!result) {
			client->send_response(request_call_id, message_status_rejected, "EXEC failure");
			return ;
		}

		//boost::filesystem::remove("tempstate.armdb");
		//document->save("tempstate.armdb");

		for (std::vector<boost::shared_ptr<session> >::iterator i = d->clients.begin(); i != d->clients.end(); ++i) {
			if (*i != client) {
				reqexec(*i, description, query);
			}
		}

		// return immediately before clients reply
		client->send_response(request_call_id, message_status_ok, "OK");
	}

	void start_accept() {
		boost::shared_ptr<session> s = boost::shared_ptr<session>(new session(acceptor.get_io_service()));
		s->read_callback = session::messagehandler(boost::bind(&storage_server::handle_message, this, _1, _2));
		s->error_callback = session::errorhandler(boost::bind(&storage_server::handle_error, this, _1, _2));

		acceptor.async_accept(s->socket, boost::bind(&storage_server::handle_accept, this, s, boost::asio::placeholders::error));
	}

	void handle_accept(boost::shared_ptr<session> s, const boost::system::error_code& error) {
		if (!error) {
			sessions.push_back(s);
			s->start(); 
			start_accept();
		}
	}
};

std::string get_user_path() {
#if defined(_WIN32)
	char* path = getenv("APPDATA");
	if (path == 0) {
		return "";
	}
	boost::filesystem::path userpath = boost::filesystem::path(path) / boost::filesystem::path("Armserve");
	return userpath.string();
#else
	boost::filesystem::path userpath = boost::filesystem::path("~/.armserve");
	return userpath.string();
#endif
}


int main() {
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 8834);
	std::string userpath = get_user_path();
	storage_server s(io_service, endpoint, userpath);
	io_service.run();
	return 0;
}
