#pragma once

// minimum size of the entire dsound secondary buffer. used with smaller 
// chunk sizes since dsound prefers locking smaller areas of larger buffers.
const int dsound_min_buffersize = 16384;

// minimum number of chunks in the entire dsound secondary buffer. used with 
// larger chunk sizes (>4k), where "dsound_min_buffersize" is insufficient.
const int dsound_min_chunks = 4;

struct ds_output_buffer {
	enum {
		// number of play positions used in overrun/underrun detection history
		num_diffs = 8
	};

	LPDIRECTSOUNDBUFFER buffer;
	HANDLE bufferevent;
	WAVEFORMATEX format;
	int buffersize;
	int buffersizebytes;
	int chunksize;
	int chunksizebytes;
	int playposition;
	int diff[num_diffs];
	int averagediff;
	int playlatency[dsound_min_chunks];
	int maxlatency;
	std::vector<float*> userbuffers;

	ds_output_buffer();
	bool create_output_buffer(LPDIRECTSOUND device, const WAVEFORMATEX& _format, int _chunksize);
	void destroy();
	bool update_play_position();
	int write_float(int deviceformat);

	inline bool is_overrun() {
		return averagediff > chunksizebytes;
	}

	inline bool is_underrun() {
		return averagediff <= -chunksizebytes;
	}

	inline void skip_chunk() {
		playposition = (playposition + chunksizebytes) % buffersizebytes;
	}

	inline int get_write_position() {
		return (playposition + maxlatency * chunksizebytes) % buffersizebytes;
	}

	inline int get_latency() {
		return maxlatency * chunksize;
	}

};


struct ds_input_buffer {

	LPDIRECTSOUNDCAPTUREBUFFER buffer;
	WAVEFORMATEX format;
	int buffersizebytes;
	int chunksize;
	int chunksizebytes;
	int readposition;
	int captureposition;
	HANDLE bufferevent;
	std::vector<float*> userbuffers;

	ds_input_buffer();
	bool create_input_buffer(LPDIRECTSOUNDCAPTURE device, WAVEFORMATEX format, int _buffersize, int _chunksize);
	void destroy();
	int update_capture_position();
	int get_captured_bytes();
	int read_float(int deviceformat);

	inline void skip_capture_chunk() {
		captureposition = (captureposition + chunksizebytes) % buffersizebytes;
	}

	inline int skip_read_chunk() {
		readposition = (readposition + chunksizebytes) % buffersizebytes;
		return chunksize;
	}

};
