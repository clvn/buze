/*
Copyright (C) 2003-2007 Anders Ervik <calvin@countzero.no>
Copyright (C) 2006-2007 Leonard Ritter

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <vector>
#include <list>
#include <map>
#include <cstdio>

namespace armstrong {

namespace frontend {

/*! \struct mem_outstream
	\brief Memory-based output stream
*/

/*! \struct mem_instream
	\brief Memory-based input stream
*/

/*! \struct mem_archive
	\brief Memory-based archive stream
*/

struct file_outstream : zzub::outstream {
	FILE* f;

	file_outstream() { f = 0; }

	~file_outstream() { close(); }

	bool create(const char* fn) {
		f = fopen(fn, "w+b");
		if (!f) return false;
		return true;
	}

	bool open(const char* fn) {
		f = fopen(fn, "rb");
		if (!f) return false;
		return true;
	}

	void close() {
		if (f) fclose(f);
		f = 0;
	}

	void seek(long pos, int mode=SEEK_SET) {
		fseek(f, pos, mode);
	}

	long position() {
		return ftell(f);
	}

	int write(void* p, int bytes) {
		return (int)fwrite(p, (unsigned int)bytes, 1, f);
	}

};

struct file_instream : zzub::instream {
	FILE* f;

	file_instream() { f = 0; }
	~file_instream() { close(); }

	bool open(const char* fn) {
		f = fopen(fn, "rb");
		if (!f) return false;
		return true;
	}

	int read(void* v, int size) {
		int pos = position();
		fread(v, size, 1, f);
		return position()-pos;
	}

	void close() {
		if (f) fclose(f);
		f = 0;
	}

	void seek(long pos, int mode=SEEK_SET) {
		fseek(f, pos, mode);
	}

	long position() {
		return ftell(f);
	}

	virtual long size() { 
		int prev = position();
		seek(0, SEEK_END);
		long pos = position();
		seek(prev, SEEK_SET);
		return pos;
	}


};

struct mem_outstream : zzub::outstream {
	int pos;
	std::vector<char> &buffer;
	
	mem_outstream(std::vector<char> &b) : buffer(b), pos(0) {}

	virtual int write(void *buffer, int size) {		
		char *charbuf = (char*)buffer;
		int ret = size;
		if (pos + size > (int)this->buffer.size()) this->buffer.resize(pos+size);
		while (size--) {
			//this->buffer.push_back(*charbuf++);
			this->buffer[pos++] = *charbuf++;
		}
		return ret;		
	}

	virtual long position() {
		return pos;
	}

	virtual void seek(long offset, int origin) {
		if (origin == SEEK_SET) {
			pos = offset;
		} else if (origin == SEEK_CUR) {
			pos += offset;
		} else if (origin == SEEK_END) {
			pos = (int)this->buffer.size() - offset;
		} else {
			assert(0);
		}
	}
};

struct mem_instream : zzub::instream {
	long pos;
	std::vector<char> &buffer;
	
	mem_instream(std::vector<char> &b) : buffer(b) { pos = 0; }
	
	virtual int read(void *buffer, int size) {
		if (pos + size > (int)this->buffer.size()) {
			std::cerr << "Tried to read beyond end of memory stream" << std::endl;
			size = (int)this->buffer.size() - pos;
		}
		memcpy(buffer, &this->buffer[pos], size);
		pos += size;
		return size;
	}

	virtual long position() {
		return pos;
	}
	
	virtual void seek(long offset, int origin) {
		if (origin == SEEK_SET) {
			pos = offset;
		} else if (origin == SEEK_CUR) {
			pos += offset;
		} else if (origin == SEEK_END) {
			pos = (int)this->buffer.size() - offset;
		} else {
			assert(0);
		}
	}

	virtual long size() { return (long)buffer.size(); }
};

struct mem_archive : zzub::archive {
	typedef std::map<std::string, std::vector<char> > buffermap;
	typedef std::pair<std::string, std::vector<char> > bufferpair;
	buffermap buffers;
	
	std::list<mem_outstream *> outstreams;
	std::list<mem_instream *> instreams;
	
	~mem_archive() {
		for (std::list<mem_outstream *>::iterator i = outstreams.begin(); i != outstreams.end(); ++i) {
			delete *i;
		}
		for (std::list<mem_instream *>::iterator i = instreams.begin(); i != instreams.end(); ++i) {
			delete *i;
		}
	}
	
	std::vector<char> &get_buffer(const char *path) {
		buffermap::iterator i = buffers.find(path);
		if(i == buffers.end()) {
			buffers.insert(bufferpair(path, std::vector<char>()));
			i = buffers.find(path);
      assert(i != buffers.end());
		}
		return i->second;
	}
	
	virtual zzub::outstream *get_outstream(const char *path) {
		buffers.insert(bufferpair(path, std::vector<char>()));
		buffermap::iterator i = buffers.find(path);
		assert(i != buffers.end());
		mem_outstream *mos = new mem_outstream(i->second);
		outstreams.push_back(mos);
		return mos;
	}
	virtual zzub::instream *get_instream(const char *path) {
		buffermap::iterator i = buffers.find(path);
		if (i == buffers.end())
			return 0;
		mem_instream *mis = new mem_instream(i->second);
		instreams.push_back(mis);
		return mis;
	}
};

} // namespace frontend
} // namespace armstrong
