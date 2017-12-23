#pragma once

struct message_item;
typedef std::vector<message_item> message;
struct message_client;

struct armserve_client : armstrong::storage::documentlistener {

	int request_id;
	bool is_exec_request;
	message_client* client;
	armstrong::storage::document* document;
	std::stringstream query;
	std::vector<std::pair<std::string, std::string> > requested_queries;

	armserve_client();

	// documentlistener
	virtual void update_document(armstrong::storage::document_event_data* e);

	bool connect(const std::string& host, const std::string& port);
	bool open(const std::string& project_name, const std::string& password);
	bool create(const std::string& project_name, const std::string& password);
	bool remove(const std::string& project_name, const std::string& password);
	bool status(int* session_count, int* document_count, std::string* current_project, int* current_session_count);
	void disconnect();
	void handle_messages();

private:
	void process_messages(int until_call_id, boost::shared_ptr<message>* result);
	void request_exec(int request_call_id, const std::string& description, const std::string& query);
	bool exec(const std::string& description, const std::string& query);

};
