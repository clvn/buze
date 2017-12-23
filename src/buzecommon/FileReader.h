/*
Copyright (C) 2003-2007 Anders Ervik <calvin@countzero.no>

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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

class StreamWriter {
public:
	StreamWriter() { }
	virtual ~StreamWriter() { }

	void write(unsigned int i) {
		writeBytes(&i, sizeof(unsigned int));
	}

	void write(unsigned long i) {
		writeBytes(&i, sizeof(unsigned long));
	}

	void write(int i) {
		writeBytes(&i, sizeof(int));
	}

	void write(unsigned short i) {
		writeBytes(&i, sizeof(unsigned short));
	}

	void write(short i) {
		writeBytes(&i, sizeof(short));
	}

	void write(unsigned char i) {
		writeBytes(&i, sizeof(unsigned char));
	}

	void write(char i) {
		writeBytes(&i, sizeof(char));
	}

	void write(float i) {
		writeBytes(&i, sizeof(float));
	}

	void write(double i) {
		writeBytes(&i, sizeof(double));
	}

    void writeAsciiZ(std::string str) {
        writeBytes(str.c_str(), str.length()+1);
    }

    void writeString(std::string str) {
        writeBytes(str.c_str(), str.length());
    }

    virtual void close() { }

	virtual void writeBytes(const void* v, size_t size)=0;

};

class StreamReader {
public:
	StreamReader() { }
	virtual ~StreamReader() { }

	virtual int read(unsigned int* v) {
		return this->readBytes(v, sizeof(unsigned int));
	}

	virtual int read(int* v) {
		return this->readBytes(v, sizeof(int));
	}

	virtual int read(unsigned short* v) {
		return this->readBytes(v, sizeof(unsigned short));
	}

	virtual int read(unsigned char* v) {
		return this->readBytes(v, sizeof(unsigned char));
	}

	virtual int read(char* v) {
		return this->readBytes(v, sizeof(char));
	}

	virtual int read(unsigned long* v) {
		return this->readBytes(v, sizeof(unsigned long));
	}

	virtual int read(float* v) {
		return this->readBytes(v, sizeof(float));
	}

	std::string readAsciiZ() {
		using namespace std;
		string sb;
		while (!eof()) {
			char c;
			read(&c);
			if (c==0)
				break;
			sb+=c;
		}
		return sb;
	}

	std::string readLine() {
		using namespace std;
		string sb;
		char c=0;
		while (!eof()) {
			if (!read(&c)) break;
			if (c==0) break;
			if (c=='\r' || c=='\n') break;
			sb+=c;
		}
		if (c=='\r' && peek()=='\n') {
			read(&c);	// skip 2nd byte in crlf
		}
		return sb;
	}

	virtual bool eof()=0;
	virtual char peek()=0;
	virtual void seek(long pos, int mode=SEEK_SET)=0;
	virtual long position()=0;

	virtual int readBytes(void* v, size_t size)=0;
};


class FileWriter : public StreamWriter {
public:
	FILE* f;

    FileWriter() {
        f=0;
    }
	bool create(const char* fn) {
		f=fopen(fn, "w+b");//_O_BINARY|_O_RDWR|_O_CREAT|_O_TRUNC, _S_IREAD | _S_IWRITE );
		if (!f) return false;
		return true;
	}

	bool open(const char* fn) {
		f=fopen(fn, "rb");//_O_BINARY|_O_RDWR, _S_IREAD | _S_IWRITE );
		if (!f) return false;
		return true;
	}

	void close() {
		if (f)
			fclose(f);
	}

	void seek(long pos, int mode=SEEK_SET) {
		fseek(f, pos, mode);
	}

	long position() {
		return ftell(f);
	}

	void writeBytes(const void* p, size_t bytes) {
		fwrite(p, (unsigned int)bytes, 1, f);
	}


	void writeAsciiZ(const char* pc) {
		fwrite(pc, (unsigned int)strlen(pc), 1, f);
		char n=0;
		writeBytes(&n, 1);
	}

	void writeLine(std::string s) {
		fwrite(s.c_str(), (unsigned int)s.length(), 1, f);
		char n='\r';
		fwrite(&n, 1, 1, f);
		n='\n';
		fwrite(&n, 1, 1, f);
	}
};

class StringWriter : public StreamWriter {
	//std::string text;
	char* text;
	size_t bufferSize;
	size_t ofs;

public:
	StringWriter(bool isZeroTerminated=true) {
		text=0;
		bufferSize=0;
		ofs=0;
	}

	~StringWriter() {
		if (text)
			delete[] text;
	}

	int position() {
		return (int)ofs;
	}

	void grow() {
		int newSize=(int)((float)bufferSize*1.25f);
		if (newSize==0) newSize=128;
		char* newBuffer=new char[newSize];
		if (text) {
			memcpy(newBuffer, text, bufferSize);
			delete[] text;
		}
		bufferSize=newSize;
		text=newBuffer;
	}


	void writeBytes(const void* p, size_t bytes) {
		while (!text || ofs+bytes>=bufferSize) grow();	// not too cool with large buffers

		memcpy(text+ofs, p, bytes);
		ofs+=bytes;
	}

    void append(std::string str) {
        writeBytes(str.c_str(), str.length());
    }

	const char* getString() { return text; }

    void reset() {
        ofs=0;
    }
};

class StringReader : public StreamReader {
	size_t ofs;
	const char* text;
public:
	bool isZeroTerminated;

	StringReader(const char* text, bool isZeroTerminated=true) {
		this->text=text;
		ofs=0;
		this->isZeroTerminated=isZeroTerminated;
	}
	int readBytes(void* v, size_t size) {
		memcpy(v, text+ofs, size);
		ofs+=size;
		return (int)size;
	}
	bool eof() {
		if (isZeroTerminated && ofs>=strlen(text)) return true;
		return false;	// neverending buffer
	}
	char peek() {
		if (eof()) return -1;
		return text[ofs];
	}


	void seek(long pos, int mode=SEEK_SET) {
        switch (mode) {
            case SEEK_SET:
                ofs=pos;
                break;
            case SEEK_CUR:
                ofs+=pos;
                break;
            case SEEK_END:
                throw "no seek for you";
        }
	}

	long position() {
		return (long)ofs;
	}


};

class FileReader : public StreamReader {
public:
    FILE* f;

    FileReader() { 
        f=0; 
    }

	bool open(const char* fn) {
		f=fopen(fn, "rb");//_O_BINARY|_O_RDONLY, _S_IREAD | _S_IWRITE );
		if (!f) return false;
		return true;
	}

	int readBytes(void* v, size_t size) {
		int pos=position();
		fread(v, size, 1, f);
		return position()-pos;
	}

	char peek() {
		char c;
		if (!read(&c)) return 0;
		seek(-1, SEEK_CUR);
		return c;
	}

	bool eof() {
		return feof(f)!=0;
	}

	void close() {
		if (f)
			fclose(f);
	}

	void seek(long pos, int mode=SEEK_SET) {
		fseek(f, pos, mode);
	}

	long position() {
		return ftell(f);
	}


};

