#pragma once

enum message_status {
	message_status_ok = 0,
	message_status_protocol_error = 1000,
	message_status_bad_message_format = 1002,
	message_status_bad_arguments = 1003,
	message_status_bad_command = 1004,
	message_status_not_found = 2000,
	message_status_already_exists = 2001,
	message_status_rejected = 3000,
	message_status_not_implemented = 9000,
};

enum message_item_type {
	message_item_type_int32 = 0,
	message_item_type_string = 1
};

struct message_item {
	message_item_type type;
	int value;
	std::vector<char> bytes;

	message_item(int i) : type(message_item_type_int32), value(i) {
	}

	message_item(const std::string& s) : type(message_item_type_string), bytes(s.begin(), s.end()) {
	}

	message_item(const std::vector<char>& s) : type(message_item_type_string), bytes(s.begin(), s.end()) {
	}
};

typedef std::vector<message_item> message;

inline std::size_t get_message_byte_count(const message& items) {
	std::size_t result = 4; // item count
	for (message::const_iterator i = items.begin(); i != items.end(); ++i) {
		result += 4; // type
		switch (i->type) {
			case message_item_type_int32:
				result += 4; // int
				break;
			case message_item_type_string:
				result += 4; // string length
				result += i->bytes.size(); // string
				break;
			default:
				assert(false);
				break;
		}
	}
	return result;
}

struct message_reader {

	enum parserstate {
		state_sequence_count,
		state_sequence_type,
		state_item_int,
		state_item_string,
		state_end,
		state_error
	};

	parserstate state;
	binary_instream_int int_parser;
	binary_instream_string string_parser;
	
	int item_count;
	boost::shared_ptr<message> items;

	message_reader() {
		state = state_sequence_count;
		items = boost::shared_ptr<message>(new message());
	}

	void reset() {
		state = state_sequence_count;
		item_count = 0;
		items = boost::shared_ptr<message>(new message());
		int_parser.reset();
		string_parser.reset();
	}

	inline bool eof() {
		return state == state_end;
	}

	inline bool err() {
		return state == state_error;
	}

	std::size_t read(char* buffer, std::size_t bytes) {
		int bytes_read = 0;
		while (bytes > 0 && !eof()) {
			read(*buffer);
			if (err()) {
				return bytes_read;
			}
			bytes_read++;
			buffer++;
			bytes--;
		}
		return bytes_read;
	}

	void read(char c) {
		switch (state) {
			case state_sequence_count:
				int_parser.read(c);
				if (int_parser.eof()) {
					item_count = int_parser.value.i;
					items->reserve(item_count);
					if (item_count == 0) {
						state = state_end;
					} else {
						state = state_sequence_type;
						int_parser.reset();
					}
				} else if (int_parser.err()) {
					state = state_error;
				}
				break;
			case state_sequence_type:
				int_parser.read(c);
				if (int_parser.eof()) {
					switch (int_parser.value.i) {
						case message_item_type_int32:
							state = state_item_int;
							int_parser.reset();
							break;
						case message_item_type_string:
							state = state_item_string;
							string_parser.reset();
							break;
					}
				} else if (int_parser.err()) {
					state = state_error;
				}
				break;
			case state_item_int:
				int_parser.read(c);
				if (int_parser.eof()) {
					items->push_back(message_item(int_parser.value.i));
					if (items->size() == item_count) {
						state = state_end;
					} else {
						state = state_sequence_type;
						int_parser.reset();
					}
				} else if (int_parser.err()) {
					state = state_error;
				}
				break;
			case state_item_string:
				string_parser.read(c);
				if (string_parser.eof()) {
					items->push_back(message_item(std::vector<char>()));
					items->back().bytes.swap(string_parser.bytes);
					if (items->size() == item_count) {
						state = state_end;
						break;
					}

					state = state_sequence_type;
					string_parser.reset();
					int_parser.reset();
				} else if (string_parser.err()) {
					state = state_error;
				}
				break;
		}
	}

};


struct message_writer {
	enum parserstate {
		state_sequence_count,
		state_sequence_type,
		state_item_int,
		state_item_string,
		state_end,
		state_error
	};

	parserstate state;
	binary_outstream_int int_writer;
	binary_outstream_string string_writer;
	boost::shared_ptr<message> msg;
	int item_index;
	message_item* item;

	message_writer() {
		state = state_error;
	}

	void data(boost::shared_ptr<message> m) {
		msg = m;
		int_writer.data((int)m->size());
		state = state_sequence_count;
	}

	inline bool eof() {
		return state == state_end;
	}

	inline bool err() {
		return state == state_error;
	}

	std::size_t write(char* buffer, std::size_t max_bytes) {
		std::size_t written_bytes = 0;
		while (!eof() && !err() && written_bytes < max_bytes) {
			write(buffer);
			if (err()) {
				// message writer error
				break;
			}
			written_bytes++;
			buffer++;
		}
		return written_bytes;
	}

	void write(char* c) {
		switch (state) {
			case state_sequence_count:
				int_writer.write(c);
				if (int_writer.eof()) {
					if (msg->empty()) {
						state = state_end;
					} else {
						state = state_sequence_type;
						item_index = 0;
						item = &(*msg)[item_index];
						int_writer.data(item->type);
					}
				} else if (int_writer.err()) {
					state = state_error;
				}
				break;
			case state_sequence_type:
				int_writer.write(c);
				if (int_writer.eof()) {
					switch (item->type) {
						case message_item_type_int32:
							state = state_item_int;
							int_writer.data(item->value);
							break;
						case message_item_type_string:
							state = state_item_string;
							string_writer.data(item->bytes);
							break;
						default:
							state = state_error;
							break;
					}
				} else if (int_writer.err()) {
					state = state_error;
				}
				break;
			case state_item_int:
				int_writer.write(c);
				if (int_writer.eof()) {
					item_index++;
					if (item_index == msg->size()) {
						state = state_end;
					} else {
						item = &(*msg)[item_index];
						int_writer.data(item->type);
						state = state_sequence_type;
					}
				} else if (int_writer.err()) {
					state = state_error;
				}
				break;
			case state_item_string:
				string_writer.write(c);
				if (string_writer.eof()) {
					item_index++;
					if (item_index == msg->size()) {
						state = state_end;
					} else {
						item = &(*msg)[item_index];
						int_writer.data(item->type);
						state = state_sequence_type;
					}
				} else if (string_writer.err()) {
					state = state_error;
				}
				break;
			case state_error:
				break;
			default:
				state = state_error;
				break;
		}
	}
};




