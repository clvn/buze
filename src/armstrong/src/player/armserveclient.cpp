#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <fstream>
#define _WIN32_WINNT 0x0501
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/filesystem.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../armserve/include/binstream.h"
#include "../armserve/include/message.h"
#include "../armserve/include/session.h"
#include "../armserve/include/client.h"
#include "player.h"
#include "armserveclient.h"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;

armserve_client::armserve_client() {
	client = 0;
	request_id = std::numeric_limits<int>::max() / 2;
	is_exec_request = false;
}

void armserve_client::update_document(armstrong::storage::document_event_data* e) {
	using namespace armstrong::storage;

	if (client == 0 || is_exec_request) {
		return ;
	}

	// TODO: can encode statements as messages instead -> avoids bogus sql
	// but needs to support messages of arbitrary length w/o reallocations
	// and sending multiple messages
	switch (e->type) {
		case event_type_update_song:
			query << dbgenpp::get_update_statement(*e->newdata.song);
			break;
		case event_type_insert_plugininfo:
			query << dbgenpp::get_insert_statement(*e->newdata.plugininfo);
			break;
		case event_type_update_plugininfo:
			query << dbgenpp::get_update_statement(*e->newdata.plugininfo);
			break;
		case event_type_delete_plugininfo:
			query << "delete from plugininfo where id = " << e->newdata.plugininfo->id << ";";
			break;
		case event_type_insert_parameterinfo:
			query << dbgenpp::get_insert_statement(*e->newdata.parameterinfo);
			break;
		case event_type_update_parameterinfo:
			query << dbgenpp::get_update_statement(*e->newdata.parameterinfo);
			break;
		case event_type_delete_parameterinfo:
			query << "delete from parameterinfo where id = " << e->newdata.parameterinfo->id << ";";
			break;
		case event_type_insert_attributeinfo:
			query << dbgenpp::get_insert_statement(*e->newdata.attributeinfo);
			break;
		case event_type_update_attributeinfo:
			query << dbgenpp::get_update_statement(*e->newdata.attributeinfo);
			break;
		case event_type_delete_attributeinfo:
			query << "delete from attributeinfo where id = " << e->newdata.attributeinfo->id << ";";
			break;
		case event_type_insert_plugin:
			query << dbgenpp::get_insert_statement(*e->newdata.plugin);
			break;
		case event_type_update_plugin:
			query << dbgenpp::get_update_statement(*e->newdata.plugin);
			break;
		case event_type_delete_plugin:
			query << "delete from plugin where id = " << e->newdata.plugin->id << ";";
			break;
		case event_type_insert_plugingroup:
			query << dbgenpp::get_insert_statement(*e->newdata.plugingroup);
			break;
		case event_type_update_plugingroup:
			query << dbgenpp::get_update_statement(*e->newdata.plugingroup);
			break;
		case event_type_delete_plugingroup:
			query << "delete from plugingroup where id = " << e->newdata.plugingroup->id << ";";
			break;
		case event_type_insert_pluginparameter:
			query << dbgenpp::get_insert_statement(*e->newdata.pluginparameter);
			break;
		case event_type_update_pluginparameter:
			query << dbgenpp::get_update_statement(*e->newdata.pluginparameter);
			break;
		case event_type_delete_pluginparameter:
			query << "delete from pluginparameter where id = " << e->newdata.pluginparameter->id << ";";
			break;
		case event_type_insert_attribute:
			query << dbgenpp::get_insert_statement(*e->newdata.attribute);
			break;
		case event_type_update_attribute:
			query << dbgenpp::get_update_statement(*e->newdata.attribute);
			break;
		case event_type_delete_attribute:
			query << "delete from attribute where id = " << e->newdata.attribute->id << ";";
			break;
		case event_type_insert_patternformat:
			query << dbgenpp::get_insert_statement(*e->newdata.patternformat);
			break;
		case event_type_update_patternformat:
			query << dbgenpp::get_update_statement(*e->newdata.patternformat);
			break;
		case event_type_delete_patternformat:
			query << "delete from patternformat where id = " << e->newdata.patternformat->id << ";";
			break;
		case event_type_insert_patternorder:
			query << dbgenpp::get_insert_statement(*e->newdata.patternorder);
			break;
		case event_type_update_patternorder:
			query << dbgenpp::get_update_statement(*e->newdata.patternorder);
			break;
		case event_type_delete_patternorder:
			query << "delete from patternorder where id = " << e->newdata.patternorder->id << ";";
			break;
		case event_type_insert_connection:
			query << dbgenpp::get_insert_statement(*e->newdata.connection);
			break;
		case event_type_update_connection:
			query << dbgenpp::get_update_statement(*e->newdata.connection);
			break;
		case event_type_delete_connection:
			query << "delete from connection where id = " << e->newdata.connection->id << ";";
			break;
		case event_type_insert_pattern:
			query << dbgenpp::get_insert_statement(*e->newdata.pattern);
			break;
		case event_type_update_pattern:
			query << dbgenpp::get_update_statement(*e->newdata.pattern);
			break;
		case event_type_delete_pattern:
			query << "delete from pattern where id = " << e->newdata.pattern->id << ";";
			break;
		case event_type_insert_patternevent:
			query << dbgenpp::get_insert_statement(*e->newdata.patternevent);
			break;
		case event_type_update_patternevent:
			query << dbgenpp::get_update_statement(*e->newdata.patternevent);
			break;
		case event_type_delete_patternevent:
			query << "delete from patternevent where id = " << e->newdata.patternevent->id << ";";
			break;
		case event_type_insert_patternformatcolumn:
			query << dbgenpp::get_insert_statement(*e->newdata.patternformatcolumn);
			break;
		case event_type_update_patternformatcolumn:
			query << dbgenpp::get_update_statement(*e->newdata.patternformatcolumn);
			break;
		case event_type_delete_patternformatcolumn:
			query << "delete from patternformatcolumn where id = " << e->newdata.patternformatcolumn->id << ";";
			break;

		case event_type_insert_samples:
			// TODO: handle edited wavedata
			// does not separate between insert_samples/replace_samples/delete_samples.
			// currently the only option is to send the entire wavelevel everytime
			// also wave related tables arent synced
			// also: send wavedata with separate protocol, 
			// we can obtain a filename to the wave contents in %TEMP%, also undo blobs for later
			// so we need to transfer wave blobs to the server before next barrier()
			// likewise, need to receive wave blobs from the server when another client makes changes
			// how to deal with rejection: list of changed wavs + query + description in requested_queries
			break;
		case event_type_barrier:
			if (!query.str().empty()) {
				bool success = exec(e->newdata.barrier->description, query.str());
				is_exec_request = true;
				if (!success) {
					// do not requeue but undo if the server rejected our query
					document->undo();
				} else if (!requested_queries.empty()) {
					// the server sent us new queries to be executed before the query we just executed, so rollback and requeue
					document->undo();
					requested_queries.push_back(std::pair<std::string, std::string>(e->newdata.barrier->description, query.str()));
				}
				is_exec_request = false;
				query.str(""); query.clear();
			}
			break;
	}
}

bool armserve_client::connect(const std::string& host, const std::string& port) {
	if (client) return false;

	client = new message_client();
	if (!client->connect(host, port)) {
		delete client;
		client = 0;
		return false;
	}
	return true;
}

void armserve_client::disconnect() {
	if (!client) return;
	client->disconnect();
	process_messages(0, 0);
	requested_queries.clear();
}

void armserve_client::handle_messages() {
	process_messages(0, 0);

	// execute queued reqexecs, and assume rollbackadjustments were already done
	is_exec_request = true;
	for (std::vector<std::pair<std::string, std::string> >::iterator i = requested_queries.begin(); i != requested_queries.end(); ++i) {
		document->exec_noret(i->second);
		document->barrier(0, 0, i->first);
	}
	requested_queries.clear();
	is_exec_request = false;
}

void armserve_client::process_messages(int until_call_id, boost::shared_ptr<message>* result) {
	if (!client) return ;

	boost::shared_ptr<message> msg;
	while (1) {
		if (client->io_service.stopped()) {
			cout << client->last_error.message() << endl;
			delete client;
			client = 0;
			cout << "cleaning up disconnected client - ready to reconnect" << endl;
			break;
		}
		if (!client->message_queue.empty()) {
			client->message_queue.pop(msg);

			int request_call_id;
			std::string command;
			if (!client->s->verify_message(msg, &request_call_id, &command)) {
				client->s->post_response(0, message_status_bad_message_format, "Bad message format");
				continue;
			}

			if (command == "REQEXEC") {
				std::string description;
				std::string query;
				if (!client->s->verify_message_parameter(msg, 2, &description)) {
					client->s->post_response(request_call_id, message_status_bad_arguments, "Bad REQEXEC arguments");
					continue;
				} 
				if (!client->s->verify_message_parameter(msg, 3, &query)) {
					client->s->post_response(request_call_id, message_status_bad_arguments, "Bad REQEXEC arguments");
					continue;
				} 
				request_exec(request_call_id, description, query);
			} else if (command == "RET") {
				if (until_call_id != 0 && request_call_id == until_call_id) {
					*result = msg;
					break;
				}
			} else {
				client->s->post_response(request_call_id, message_status_bad_command, "Unknown client command");
			}
		} else {
			if (until_call_id == 0) {
				break;
			}
			boost::this_thread::sleep(boost::posix_time::milliseconds(50));
		}
	}
}

void armserve_client::request_exec(int request_call_id, const std::string& description, const std::string& query) {
	cout << "request_exec: " << query << endl;

	// reqexec is an async and authoritative request from the server.
	// if we are currently exec() another query locally, the server query should get priority
	// -> queue the query here, and process later

	requested_queries.push_back(std::pair<std::string, std::string>("Remote: " + description, query));

	client->s->post_response(request_call_id, message_status_ok, "OK");
}

// client -> server methods
bool armserve_client::create(const std::string& project_name, const std::string& password) {
	// save current document to a char vector, then send it to the server
	boost::filesystem::path tempfile = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	if (!document->save(tempfile.string())) {
		return false;
	}

	request_id++;

	boost::shared_ptr<message> request = boost::shared_ptr<message>(new message());
	request->push_back(message_item(request_id));
	request->push_back(message_item("CREATE"));
	request->push_back(message_item(project_name));
	request->push_back(message_item(password));
	request->push_back(message_item(std::vector<char>()));

	message_item& item = request->back();

	std::ifstream f(tempfile.string(), std::ios::in | std::ios::binary);
	item.bytes.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	f.close();
	boost::filesystem::remove(tempfile);

	client->s->post(request);

	boost::shared_ptr<message> response;
	process_messages(request_id, &response);

	int status_code;
	std::string status_text;
	if (!client->s->parse_return_message(response, &status_code, &status_text)) {
		return false;
	}

	if (status_code != message_status_ok) {
		return false;
	}

	return true;
}

bool armserve_client::remove(const std::string& project_name, const std::string& password) {
	request_id++;

	boost::shared_ptr<message> request = boost::shared_ptr<message>(new message());
	request->push_back(message_item(request_id));
	request->push_back(message_item("DELETE"));
	request->push_back(message_item(project_name));
	request->push_back(message_item(password));
	client->s->post(request);

	boost::shared_ptr<message> response;
	process_messages(request_id, &response);

	int status_code;
	std::string status_text;
	if (!client->s->parse_return_message(response, &status_code, &status_text)) {
		return false;
	}

	if (status_code != message_status_ok) {
		return false;
	}

	return true;
}

bool armserve_client::open(const std::string& project_name, const std::string& password) {
	request_id++;

	boost::shared_ptr<message> request = boost::shared_ptr<message>(new message());
	request->push_back(message_item(request_id));
	request->push_back(message_item("OPEN"));
	request->push_back(message_item(project_name));
	request->push_back(message_item(password));
	client->s->post(request);

	boost::shared_ptr<message> response;
	process_messages(request_id, &response);

	int status_code;
	std::string status_text;
	if (!client->s->parse_return_message(response, &status_code, &status_text)) {
		return false;
	}

	if (status_code != message_status_ok) {
		// response[3] is a string is the error message
		return false;
	}

	if (!client->s->verify_message_parameter(response, 4, (std::string*)0)) {
		return false;
	}

	std::vector<char>& bytes = (*response)[4].bytes;

	boost::filesystem::path tempfile = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	std::ofstream f(tempfile.string(), std::ios::out | std::ios::binary);
	f.write(&bytes.front(), bytes.size());
	f.close();
	
	is_exec_request = true;
	bool result = document->load(tempfile.string());
	is_exec_request = false;

	boost::filesystem::remove(tempfile);
	return result;
}

bool armserve_client::exec(const std::string& description, const std::string& query) {
	request_id++;

	boost::shared_ptr<message> request = boost::shared_ptr<message>(new message());
	request->push_back(message_item(request_id));
	request->push_back(message_item("EXEC"));
	request->push_back(message_item(description));
	request->push_back(message_item(query));
	client->s->post(request);

	boost::shared_ptr<message> response;
	process_messages(request_id, &response);

	int status_code;
	std::string status_text;
	if (!client->s->parse_return_message(response, &status_code, &status_text)) {
		return false;
	}

	if (status_code != message_status_ok) {
		cout << "Server error: " << status_text << endl;
		return false;
	}

	// elsewhere: if failure rollback+reqexec else if has_queued_reqexec rollback+reqexec+reapply
	return true;
}

bool armserve_client::status(int* session_count, int* document_count, std::string* current_project, int* current_session_count) {
	request_id++;

	boost::shared_ptr<message> request = boost::shared_ptr<message>(new message());
	request->push_back(message_item(request_id));
	request->push_back(message_item("STATUS"));
	client->s->post(request);

	boost::shared_ptr<message> response;
	process_messages(request_id, &response);

	int status_code;
	std::string status_text;
	if (!client->s->parse_return_message(response, &status_code, &status_text)) {
		return false;
	}

	if (status_code != message_status_ok) {
		cout << "Server error: " << status_text << endl;
		return false;
	}

	if (!client->s->verify_message_parameter(response, 2, session_count)) {
		return false;
	}

	if (!client->s->verify_message_parameter(response, 3, document_count)) {
		return false;
	}

	if (!client->s->verify_message_parameter(response, 4, current_project)) {
		return false;
	}

	if (!client->s->verify_message_parameter(response, 5, current_session_count)) {
		return false;
	}
	return true;
}
