struct message_client {

	boost::thread* client_thread;
	boost::asio::io_service io_service;
	boost::shared_ptr<session> s;
	boost::lockfree::spsc_queue<boost::shared_ptr<message>, boost::lockfree::capacity<2048> > message_queue;
	boost::system::error_code last_error;

	message_client() {
		client_thread = 0;
		s = boost::shared_ptr<session>(new session(io_service));
	}

	void handle_message(boost::shared_ptr<session> client, boost::shared_ptr<message> msg) {
		// add to the message queue, is polled from the user thread
		message_queue.push(msg);
	}

	void handle_error(boost::shared_ptr<session> client, const boost::system::error_code& e) {
		s->disconnect();
		io_service.stop();
		last_error = e;
	}

	void client_proc() {
		s->read_callback = session::messagehandler(boost::bind(&message_client::handle_message, this, _1, _2));
		s->error_callback = session::errorhandler(boost::bind(&message_client::handle_error, this, _1, _2));
		s->start();
		io_service.run();
	}

	bool connect(const std::string& server, const std::string& port) {
		if (client_thread != 0) return false;

		boost::asio::ip::tcp::resolver r(io_service);
		boost::asio::ip::tcp::resolver::query query(server, port);

		boost::asio::ip::tcp::resolver::iterator endpoint_iterator = r.resolve(query);
		boost::asio::ip::tcp::resolver::iterator end;
	
		last_error = boost::asio::error::host_not_found;
		while (last_error && endpoint_iterator != end) {
    		s->socket.close();
    		s->socket.connect(*endpoint_iterator++, last_error);
		}

		if (last_error) {
			return false;
		}

		client_thread = new boost::thread(boost::bind(&message_client::client_proc, this));
		return true;
	}

	void disconnect() {
		io_service.post(boost::bind(&session::disconnect, s));
		io_service.stop();

		client_thread->join();
		delete client_thread;
		client_thread = 0;
	}

	void post(boost::shared_ptr<message> m) {
		io_service.post(boost::bind(&session::send, s, m));
		//io_service.post(boost::bind(&message_client::handle_post, this, m));
	}
};


