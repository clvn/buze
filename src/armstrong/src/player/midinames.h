#pragma once

struct _midiouts : zzub::outstream {
	std::vector<std::string> names;

	void clear() {
		names.clear();
	}
	virtual int write(void *buffer, int size) {
		char* pcbuf = (char*)buffer;
		names.push_back(std::string(pcbuf, pcbuf+size-1));
		return size;
	}
	virtual long position() { return 0; }
	virtual void seek(long, int) { }
};
