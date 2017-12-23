
struct session :
	boost::enable_shared_from_this<session> 
{
	boost::asio::ip::tcp::socket socket;
	boost::array<char, 8192> readbuffer;
	boost::array<char, 8192> writebuffer;
	message_reader reader;
	message_writer writer;

	typedef boost::function<void(boost::shared_ptr<session> client, boost::shared_ptr<message> msg)> messagehandler;
	messagehandler write_callback;
	messagehandler read_callback;

	typedef boost::function<void(boost::shared_ptr<session>, const boost::system::error_code& e)> errorhandler;
	errorhandler error_callback;

	std::queue<boost::shared_ptr<message> > write_queue;

	session(boost::asio::io_service& io_service) : socket(io_service) {
	}

	void start() {
		socket.async_read_some(
			boost::asio::buffer(readbuffer),
			boost::bind(&session::handle_read, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void handle_read(const boost::system::error_code& e, std::size_t bytes_transferred) {
		if (!e) {
			size_t bytes_read = 0;
			while (bytes_read < bytes_transferred) {
				size_t bytes = reader.read(readbuffer.data() + bytes_read, bytes_transferred - bytes_read);
				bytes_read += bytes;
				if (reader.eof()) {
					// got a complete message
					read_callback(shared_from_this(), reader.items);
					reader.reset();
				} else if (reader.err()) {
					// message protocol syntax error
					error_callback(shared_from_this(), boost::system::posix_error::make_error_code(boost::system::posix_error::bad_message));
					reader.reset();
					return ;
				}
			}

			socket.async_read_some(
				boost::asio::buffer(readbuffer),
				boost::bind(&session::handle_read, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		} else {
			// read error
			error_callback(shared_from_this(), e);
		}
	}

	void send(boost::shared_ptr<message> msg) {
		if (write_queue.empty()) {
			write_queue.push(msg);
			writer.data(msg);
			write();
		} else {
			write_queue.push(msg);
		}
	}

	void write() {
		std::size_t size = writer.write(writebuffer.c_array(), writebuffer.size());
		
		boost::asio::async_write(socket, boost::asio::buffer(writebuffer, size), boost::bind(&session::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void handle_write(const boost::system::error_code& e, std::size_t bytes_transferred) {
		if (!e) {
			if (writer.eof()) {
				// sent message
			} else if (writer.err()) {
				// protocol serialization error
				error_callback(shared_from_this(), boost::system::posix_error::make_error_code(boost::system::posix_error::bad_message));
				return ;
			} else {
				write();
				return ;
			}

			write_queue.pop();
			if (!write_queue.empty()) {
				writer.data(write_queue.front());
				write();
			}
		}  else {
			// write error
			error_callback(shared_from_this(), e);
		}
	}

	void create_return_message(int return_call_id, int status_code, const std::string& status_text, boost::shared_ptr<message>* result) {
		boost::shared_ptr<message> m = boost::shared_ptr<message>(new message());
		m->push_back(message_item(return_call_id));
		m->push_back(message_item("RET"));
		m->push_back(message_item(status_code));
		m->push_back(message_item(status_text));
		*result = m;
	}

	// check the message starts with an integer followed by a string 
	bool verify_message(boost::shared_ptr<message> msg, int* call_id, std::string* command) {
		if (msg->size() < 2) {
			return false;
		}

		if ((*msg)[0].type != 0) {
			return false;
		}

		if ((*msg)[1].type != 1) {
			return false;
		}

		*call_id = (*msg)[0].value;
		*command = std::string((*msg)[1].bytes.begin(), (*msg)[1].bytes.end());
		return true;
	}

	bool verify_message_parameter(boost::shared_ptr<message> msg, size_t index, int* result) {
		if (msg->size() <= index) {
			return false;
		}

		if ((*msg)[index].type != 0) {
			return false;
		}

		*result = (*msg)[index].value;
		return true;
	}

	bool verify_message_parameter(boost::shared_ptr<message> msg, size_t index, std::string* result) {
		if (msg->size() <= index) {
			return false;
		}

		if ((*msg)[index].type != 1) {
			return false;
		}

		if (result != 0) {
			*result = std::string((*msg)[index].bytes.begin(), (*msg)[index].bytes.end());
		}
		return true;
	}

	bool parse_return_message(boost::shared_ptr<message> msg, int* status_code, std::string* status_text) {
		if (!verify_message_parameter(msg, 2, status_code)) {
			return false;
		}

		if (!verify_message_parameter(msg, 3, status_text)) {
			return false;
		}
		return true;
	}

	void send_response(int return_call_id, int status_code, const std::string& status_text) {
		boost::shared_ptr<message> m;
		create_return_message(return_call_id, status_code, status_text, &m);
		send(m);
	}

	void post_response(int return_call_id, int status_code, const std::string& status_text) {
		boost::shared_ptr<message> m;
		create_return_message(return_call_id, status_code, status_text, &m);
		post(m);
	}

	void post(boost::shared_ptr<message> m) {
		socket.get_io_service().post(boost::bind(&session::send, shared_from_this(), m));
		//io_service.post(boost::bind(&message_client::handle_post, this, m));
	}


	void disconnect() {
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket.close();
	}

};
