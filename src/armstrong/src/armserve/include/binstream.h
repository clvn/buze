struct charint {
	union {
		int i;
		char c[4];
	};
};

struct binary_outstream_writer {
	std::vector<char> bytes;

	void write(int i) {
		charint b;
		b.i = i;
		bytes.push_back(b.c[0]);
		bytes.push_back(b.c[1]);
		bytes.push_back(b.c[2]);
		bytes.push_back(b.c[3]);
	}

	void write(const std::string& s) {
		write((int)s.length());
		bytes.insert(bytes.end(), s.begin(), s.end());
	}

	void write(const std::vector<char>& s) {
		write((int)s.size());
		bytes.insert(bytes.end(), s.begin(), s.end());
	}
};

struct binary_outstream_int {
	enum parserstate {
		state_0,
		state_1,
		state_2,
		state_3,
		state_end,
		state_error
	};

	parserstate state;
	charint value;

	binary_outstream_int() {
		state = state_error;
	}

	void data(int i) {
		value.i = i;
		state = state_0;
	}

	inline bool eof() {
		return state == state_end;
	}

	inline bool err() {
		return state == state_error;
	}

	void write(char* c) {
		switch (state) {
			case state_0:
				state = state_1;
				*c = value.c[0];
				break;
			case state_1:
				state = state_2;
				*c = value.c[1];
				break;
			case state_2:
				state = state_3;
				*c = value.c[2];
				break;
			case state_3:
				state = state_end;
				*c = value.c[3];
				break;
			case state_error:
				break;
			default:
				state = state_error;
				break;
		}
	}
};

struct binary_outstream_string {
	enum parserstate {
		state_string_length,
		state_string_bytes,
		state_end,
		state_error
	};

	parserstate state;
	binary_outstream_int int_writer;
	std::vector<char>* bytes;
	std::size_t write_bytes;

	binary_outstream_string() {
		state = state_error;
	}

	void data(std::vector<char>& s) {
		state = state_string_length;
		bytes = &s;
		write_bytes = 0;
		int_writer.data((int)s.size());
	}

	inline bool eof() {
		return state == state_end;
	}

	inline bool err() {
		return state == state_error;
	}

	void write(char* c) {
		switch (state) {
			case state_string_length:
				int_writer.write(c);
				if (int_writer.eof()) {
					if (bytes->empty()) {
						state = state_end;
					} else {
						state = state_string_bytes;
						write_bytes = 0;
					}
				} else if (int_writer.err()) {
					state = state_error;
				}
				break;
			case state_string_bytes:
				if (write_bytes >= bytes->size()) {
					state = state_error;
					return ;
				}

				*c = (*bytes)[write_bytes];
				write_bytes++;
				if (write_bytes == bytes->size()) {
					state = state_end;
					return ;
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

struct binary_instream_int {

	enum parserstate {
		state_0,
		state_1,
		state_2,
		state_3,
		state_end,
		state_error
	};

	parserstate state;
	charint value;

	binary_instream_int() {
		reset();
	}

	void reset() {
		state = state_0;
	}
	
	inline bool eof() {
		return state == state_end;
	}

	inline bool err() {
		return state == state_error;
	}

	void read(char c) {
		switch (state) {
			case state_0:
				value.c[0] = c;
				state = state_1;
				break;
			case state_1:
				value.c[1] = c;
				state = state_2;
				break;
			case state_2:
				value.c[2] = c;
				state = state_3;
				break;
			case state_3:
				value.c[3] = c;
				state = state_end;
				break;
			case state_error:
				break;
			default:
				state = state_error;
				break;
		}
	}

};

struct binary_instream_string {
	enum parserstate {
		state_string_length,
		state_string_bytes,
		state_end,
		state_error
	};

	parserstate state;
	binary_instream_int size_parser;
	int total_bytes;
	int read_bytes;
	std::vector<char> bytes;

	binary_instream_string() {
		reset();
	}

	void reset() {
		state = state_string_length;
		size_parser.reset();
		read_bytes = 0;
		total_bytes = 0;
	}
	
	inline bool eof() {
		return state == state_end;
	}

	inline bool err() {
		return state == state_error;
	}

	void read(char c) {
		switch (state) {
			case state_string_length:
				size_parser.read(c);
				if (size_parser.eof()) {
					state = state_string_bytes;
					total_bytes = size_parser.value.i;
					read_bytes = 0;
					bytes.resize(total_bytes);
					if (total_bytes == 0) {
						state = state_end;
						break;
					}
				} else if (size_parser.err()) {
					state = state_error;
				}
				break;
			case state_string_bytes:
				if (read_bytes > total_bytes) {
					state = state_error;
					break;
				}
				bytes[read_bytes] = c;
				read_bytes++;
				if (read_bytes == total_bytes) {
					state = state_end;
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
