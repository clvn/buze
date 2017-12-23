#pragma once

// virtual filesystems
//
// usage:
// root_directory root;
// root.factories.push_back(new zzub_directory_factory<false>(player));
// root.factories.push_back(new zzub_directory_factory<true>(player));
//  ... 
// root.get_files(entries); //  returns a list of all drives on the system
// root.get_directory("c:/test.s3m/2"); to retreve a listing of instruments in the s3m

#define NOMINMAX
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <zzub/zzub.h>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cassert>
#include <iomanip>
#include <boost/lexical_cast.hpp>

using std::cout;
using std::endl;

struct directory_entry {
	enum {
		type_directory = 0,
		type_file = 1,
		type_container = 2
	};

	int type;
	std::string name;
	std::string fullname;
	unsigned long size;
};

struct directory {
	virtual ~directory() {}

	virtual bool get_files(std::vector<directory_entry>& entries) = 0;
	virtual directory* get_directory(const std::string& path) = 0;
};

struct root_directory;

struct file_directory_base : directory {
	root_directory* root;
	std::string fullpath;
	virtual directory* get_directory(const std::string& path);
	directory* create_directory(const std::string& rootpath, const std::string& filename);
};

struct file_directory : file_directory_base {
	file_directory(root_directory* rootdir, const std::string& path);
	virtual bool get_files(std::vector<directory_entry>& entries);
};

struct directory_factory {
	std::vector<std::string> extensions;
	virtual ~directory_factory() {}
	virtual directory* create_directory(const std::string& rootpath, const std::string& filename) = 0;
	virtual bool is_container() = 0;
};

struct root_directory : file_directory_base {
	std::vector<directory_factory*> factories;

	root_directory();
	virtual bool get_files(std::vector<directory_entry>& entries);
	directory_factory* get_factory(std::string filename) ;
};


// 
// file_directory
//

file_directory::file_directory(root_directory* rootdir, const std::string& path) {
	root = rootdir;
	fullpath = path;
}

bool file_directory::get_files(std::vector<directory_entry>& entries) {
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile((fullpath + "\\*.*").c_str(), &fd);
	while (hFind != INVALID_HANDLE_VALUE) {
		
		bool ignorefile = false;
		if (0 != strcmp(fd.cFileName, ".") && 0 != strcmp(fd.cFileName, "..")) {
			directory_entry de;
			de.name = fd.cFileName;
			de.fullname = fullpath + "\\" + fd.cFileName;
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
				de.type = directory_entry::type_directory;
			} else {
				directory_factory* dirfactory = root->get_factory(de.name);
				if (dirfactory) {
					if (dirfactory->is_container())
						de.type = directory_entry::type_container;
					else
						de.type = directory_entry::type_file;
				} else
					ignorefile = true;
					//de.type = directory_entry::type_file;
				de.size = fd.nFileSizeLow;
			}

			if (!ignorefile)
				entries.push_back(de);
		}

		if (!FindNextFile(hFind, &fd)) break;
	}
	FindClose(hFind);
	return true;
}

// 
// file_directory_base
//

directory* file_directory_base::get_directory(const std::string& path) {
	std::string::size_type ns = path.find_first_of("/\\");
	if (ns != std::string::npos) {
		std::string dirname = path.substr(0, ns);
		std::string subpath = path.substr(ns + 1);
		directory* subdir = create_directory(fullpath, dirname);
		directory* result = subdir->get_directory(subpath);
		delete subdir;
		return result;
	} else
		return create_directory(fullpath, path);
}

directory* file_directory_base::create_directory(const std::string& rootpath, const std::string& filename) {

	// enumerate importer extensions  for module_directory
	// map to zip for zip_directory

	std::string fullpath;
	if (rootpath.empty())
		fullpath = filename;
	else
		fullpath = rootpath + "\\" + filename;

	directory_factory* factory = root->get_factory(filename);

	if (factory)
		return factory->create_directory(rootpath, filename);
	else
		return new file_directory(root, fullpath);
}


// 
// root_directory
//

root_directory::root_directory() {
	root = this;
}

bool root_directory::get_files(std::vector<directory_entry>& entries) {
	char drivestring[1024];
	DWORD result = GetLogicalDriveStrings(1024, drivestring);
	if (result == 0) return false;
	char* cd = drivestring;
	int count = 0;
	while (int len = (int)strlen(cd)) {
		cd[2] = 0; // loosen the '\'
		directory_entry de;
		de.name = cd;
		de.fullname = de.name;
		de.type = directory_entry::type_directory;
		cd += len + 1;
		if (de.name == "A:") continue;
		entries.push_back(de);
	}
	return true;
}

directory_factory* root_directory::get_factory(std::string filename) {
	size_t dp = filename.find_last_of('.');
	if (dp == std::string::npos) return 0;
	std::string ext = filename.substr(dp + 1);
	transform(ext.begin(), ext.end(), ext.begin(), (int(*)(int))std::tolower);
	std::vector<directory_factory*>::iterator i;
	for (i = factories.begin(); i != factories.end(); ++i) {
		std::vector<std::string>& exts = (*i)->extensions;
		std::vector<std::string>::iterator j = find(exts.begin(), exts.end(), ext);
		if (j != exts.end()) return *i;
	}

	return 0;

}


// zzub_directory - browse samplebanks on disk as virtual directories using armstrong waveimporters

// class for enumerating instruments in a container-format supported by armstrongs waveimporter
struct zzub_directory : directory {
	std::string fullpath;
	zzub_wave_importer_t* importer;

	zzub_directory(std::string path, zzub_wave_importer_t* _importer) {
		fullpath = path;
		importer = _importer;
	}

	virtual bool get_files(std::vector<directory_entry>& entries) {

		int count = zzub_wave_importer_get_instrument_count(importer);
		for (int i = 0; i < count; i++) {
			const char* name = zzub_wave_importer_get_instrument_name(importer, i);

			// name = e.g "000 grandpiano", "001 brightpiano" etc - 
			std::stringstream strm;
			strm << std::setw(3) << std::setfill('0') << i << " " << name; 

			directory_entry de;
			de.name = strm.str(); 
			de.fullname = fullpath + "\\" + boost::lexical_cast<std::string, int>(i); // fullname has index only

			int samplecount = zzub_wave_importer_get_instrument_sample_count(importer, i);
			//if (samplecount == 1) {
			// TODO: get the size of the file?
			//	de.type = directory_entry::type_file;
			//} else {
				de.type = directory_entry::type_container;
			//}
			de.size = 0;

			entries.push_back(de);
		}

		return true;
	}
/*
	int get_index(std::string filename) {
		std::string::size_type sl = filename.find_first_of(" ");
		if (sl == std::string::npos) return -1; // error
		return atoi(filename.substr(0, sl).c_str());
	}*/

	virtual directory* get_directory(const std::string& path) {
		// return something that enumerates the individual samples
		std::string::size_type ns = path.find_first_of("/\\");
		if (ns != std::string::npos) {
			std::string dirname = path.substr(0, ns);
			std::string subpath = path.substr(ns + 1);

			int index = boost::lexical_cast<int, std::string>(dirname);
			if (index == -1) return 0;

			directory* subdir = new zzub_instrument_directory(fullpath + "\\" + dirname, importer, index);
			directory* result = subdir->get_directory(subpath);
			delete subdir;
			return result;
		} else {
			int index = boost::lexical_cast<int, std::string>(path);
			if (index == -1) return 0;

			return new zzub_instrument_directory(fullpath + "\\" + path, importer, index);
		}
		return 0;
	}

	// inner class for enumerating waveimporter samples on an instrument
	struct zzub_instrument_directory : directory {

		std::string fullpath;
		zzub_wave_importer_t* importer;
		int index;

		zzub_instrument_directory(std::string path, zzub_wave_importer_t* _parent, int _index) {
			fullpath = path;
			importer = _parent;
			index = _index;
		}

		virtual bool get_files(std::vector<directory_entry>& entries) {
			int count = zzub_wave_importer_get_instrument_sample_count(importer, index);
			char name[256];
			for (int i = 0 ; i < count; i++) {
				int samplecount, channels, format, samplerate;
				zzub_wave_importer_get_instrument_sample_info(importer, index, i, name, 256, &samplecount, &channels, &format, &samplerate);
				std::stringstream strm;
				strm << std::setw(3) << std::setfill('0') << i << " " << name; 
				directory_entry de;
				de.name = strm.str();
				de.fullname = fullpath + "\\" + boost::lexical_cast<std::string, int>(i);
				de.type = directory_entry::type_file;
				de.size = samplecount;
				entries.push_back(de);
			}
			return true;
		}

		virtual directory* get_directory(const std::string& path) {
			assert(false); // shouldnt be any dirs here
			return 0;
		}

	};

};

template <bool is_virtual_container>
struct zzub_directory_factory : directory_factory {

	zzub_player_t* player;

	zzub_directory_factory(zzub_player_t* _player) {
		player = _player;
		for (int i = 0; i < zzub_player_get_waveimporter_count(player); i++) {
			bool is_format_container = zzub_player_get_waveimporter_format_is_container(player, i) != 0;

			if (is_format_container == is_container()) {
				int extcount = zzub_player_get_waveimporter_format_ext_count(player, i);
				for (int j = 0; j < extcount; j++) {
					const char* ext = zzub_player_get_waveimporter_format_ext(player, i, j);
					extensions.push_back(ext);
				}
			}
		}
	}

	directory* create_directory(const std::string& rootpath, const std::string& filename) {
		std::string fullpath = rootpath + "\\" + filename;
		zzub_wave_importer_t* importer = zzub_player_create_waveimporter_by_file(player, fullpath.c_str());
		if (importer == 0)
			return 0;

		return new zzub_directory(rootpath + "\\" + filename, importer);
	}

	bool is_container() {
		return is_virtual_container;
	}
};

/*

void dump(directory& d) {
	std::vector<directory_entry> entries;
	d.get_files(entries);

	for (std::vector<directory_entry>::iterator i = entries.begin(); i != entries.end(); ++i) {

		if (i->type == directory_entry::type_directory || i->type == directory_entry::type_container) {
			cout << endl;
			cout << i->fullname << endl;
			directory* dir = d.get_directory(i->name);
			if (dir) {
				dump(*dir);
				delete dir;
			} else {
				cout << "cant open directory " << i->name << endl;
			}
		} else {
			cout << i->name << " ";
		}
	}
}

int main() {

	zzub_player_t* player = zzub_player_create();

	root_directory root;
	root.factories.push_back(new zzub_directory_factory<false>(player));
	root.factories.push_back(new zzub_directory_factory<true>(player));
	//dump(root); // dump the contents of all drive letters on the system

	//directory* x = root.get_directory("C:\\code\\buze\\trunk\\buze\\src\\armstrong\\src\\libsf2");
	directory* x = root.get_directory("L:\\program files\\synthfont");

	dump(*x);

	return 0;
}
*/