#pragma once

/*

generic plugin enumeration and cache helper for armstrong plugin wrappers

the wrappers plugincollection must implement:
		create_plugin_infos_from_file - called when an uncached dll is encountered for the first time
		init_plugin_infos - called when a dll is initialized (loaded from cache, or enumerated)
		fill_plugin_infos - called when a dll wasnt found in the cache
		unregister_plugin_infos - called when a dll didnt initialize properly

the wrappers info contructor takes exactly two parameters: filename and collection

an instance of plugincache scans for plugins in a single directory, optionally recursively in subdirectories

the plugincache file is loaded/saved in the specified directory, containing all recursively scanned plugins

the crash apis works like:
	- before initializing a plugin, a "lockfile" is created for the plugin and deleted after init
	- if the plugin crashes, the host will also crash, leaving the lockfile on disk. 
	- the user must restart the host to restart scanning.
	- when a "lockfile" is encountered during rescan, its plugin is ignored and added to an internal blacklist

*/

template <typename PLUGINCOLLECTION, typename PLUGININFO>
struct plugincache {
	std::string version;
	std::string cachekey;

	std::map<std::string, std::vector<PLUGININFO*> > cached_info;
	std::map<std::string, std::vector<PLUGININFO*> > registered_infos;
	std::vector<std::string> crashed_plugins;

	plugincache(std::string _cachekey, std::string _version) {
		cachekey = _cachekey;
		version = _version;
	}

	virtual ~plugincache() {}

	inline std::string get_version() {
		return cachekey + "-cache-version-" + version;
	}

	bool get_plugin_infos_from_cache(std::string path, std::vector<PLUGININFO*>* infos) {
		std::map<std::string, std::vector<PLUGININFO*> >::iterator i = cached_info.find(path);
		if (i == cached_info.end())
			return false;
		else {
			infos->insert(infos->end(), i->second.begin(), i->second.end());
			return true;
		}
	}

	void register_info(const std::string& path, std::vector<PLUGININFO*>& i) {
		assert(registered_infos.find(path) == registered_infos.end()); // shouldnt register same file twice
		cached_info[path] = i; // re-write if it already exists
		registered_infos[path] = i;
	}

	void unregister_info(const std::string& path) {
		std::map<std::string, std::vector<PLUGININFO*> >::iterator i = cached_info.find(path);
		if (i != cached_info.end()) cached_info.erase(i);

		i = registered_infos.find(path);
		if (i != registered_infos.end()) registered_infos.erase(i);
	}

	void register_crashed(const std::string& path) {
		crashed_plugins.push_back(path);
	}

	bool is_crashed_plugin(const std::string& path) {
		std::vector<std::string>::iterator i = std::find(crashed_plugins.begin(), crashed_plugins.end(), path);
		return i != crashed_plugins.end();
	}

	//
	// enumeration
	//

	void enumerate_directory_with_cache(PLUGINCOLLECTION* coll, zzub::pluginfactory* factory, const std::string& rootpath, const std::string& cachepath, bool recursive) {
		load_cache(coll, rootpath, cachepath);

		enumerate_directory(coll, factory, rootpath, "", recursive);

		save_cache(rootpath, cachepath);
	}

	void enumerate_directory(PLUGINCOLLECTION* coll, zzub::pluginfactory* factory, const std::string& rootpath, const std::string& path, bool recursive) {

		using std::cout;
		using std::endl;

		std::string pluginpath = rootpath + path;
		std::string searchpath = rootpath + path + "*.*";

		cout << cachekey << "-cache: searching folder " << pluginpath;
		if (recursive) cout << " recursively";
		cout << " ..." << endl;

		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(searchpath.c_str(), &fd);

		while (hFind != INVALID_HANDLE_VALUE) {
			
			if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				char* ext = strrchr(fd.cFileName, '.');
				if (ext != 0 && stricmp(ext, ".dll") == 0) {
					std::string relpath = path + fd.cFileName;
					std::string fullpath = rootpath + relpath;
					add_plugin_dll(coll, factory, fullpath, relpath);
				}
			} else if (recursive) {
				// if not ./.. and recursive then goto 10
				if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
					std::string relpath = path + (std::string)"\\" + fd.cFileName;
					enumerate_directory(coll, factory, rootpath, relpath + "\\", true);
				}
			}
			if (!FindNextFile(hFind, &fd)) break;
		}
		FindClose(hFind);
	}

	void delete_infos(std::vector<PLUGININFO*>& infos) {
		for (std::vector<PLUGININFO*>::iterator k = infos.begin(); k != infos.end(); ++k) {
			delete *k;
		}
	}

	bool add_plugin_dll(PLUGINCOLLECTION* coll, zzub::pluginfactory* factory, const std::string& fullpath, const std::string& relpath) {
		std::vector<PLUGININFO*> infos;
		if (get_plugin_infos_from_cache(relpath, &infos)) {
			coll->init_plugin_infos(infos, true);
			register_info(relpath, infos);
		} else {
			coll->create_plugin_infos_from_file(fullpath, &infos);
			coll->init_plugin_infos(infos, false);
			register_info(relpath, infos);

			if (!coll->fill_plugin_infos(infos)) {
				coll->unregister_plugin_infos(infos);
				unregister_info(relpath);
				delete_infos(infos);
				return false;
			}
		}

		for (std::vector<PLUGININFO*>::iterator i = infos.begin(); i != infos.end(); ++i) {
			factory->register_info(*i);
		}
		return true;
	}

	//
	// (de)serialization
	//

	bool load_cache(PLUGINCOLLECTION* coll, const std::string& pluginpath, const std::string& cachepath) {
		using std::cout;
		using std::cerr;
		using std::endl;

		//std::string pluginPath = "./Gear/Vst/";
		// load cache from %APPDATA%\Buze\PluginCache\uniquely_encoded_full_plugin_path.dat
		std::string cachefile = cachepath + cachekey + "-" + urlencode(pluginpath) + ".dat";
		//std::string cacheFile = path + cachekey + ".dat";

		cerr << "plugincache: loading cache " << cachefile << endl;
		std::ifstream f;
		f.open(cachefile.c_str());
		if (!f.good()) return false;
		
		std::string version;
		std::getline(f, version);

		if (version != get_version()) return false;

		std::string dllcountstring;
		std::getline(f, dllcountstring);
		int dllcount = atoi(dllcountstring.c_str());

		//while (!f.eof() && !f.fail()) {
		for (int j = 0; j < dllcount; j++) {
			std::string dll_name, uri, name, author;
			std::string flagstring, timestring;
			int flags, min_tracks, max_tracks, inputs, outputs;
			std::string short_name;
			std::getline(f, dll_name);
			std::getline(f, timestring);

			std::string dllplugincountstring;
			std::getline(f, dllplugincountstring);
			int dllplugincount = atoi(dllplugincountstring.c_str());

			std::vector<PLUGININFO*> infos;
			for (int k = 0; k < dllplugincount; k++) {

				std::getline(f, uri);
				std::getline(f, name);
				std::getline(f, flagstring);
				flags = atoi(flagstring.c_str());
				std::getline(f, short_name);
				std::getline(f, flagstring);
				min_tracks = atoi(flagstring.c_str());
				std::getline(f, flagstring);
				max_tracks = atoi(flagstring.c_str());
				std::getline(f, author);

				std::getline(f, flagstring);
				outputs = atoi(flagstring.c_str());
				std::getline(f, flagstring);
				inputs = atoi(flagstring.c_str());

				if (name.empty() || short_name.empty() || f.eof() || f.fail()) break; // eof

				//cout << "Read cached plugin: " << name << ", flags=" << flags << ", short=" << short_name << endl;
				PLUGININFO* i = new PLUGININFO(coll, pluginpath + dll_name);
				i->uri = uri;
				i->name = urldecode(name);
				i->flags = flags;
				i->short_name = urldecode(short_name);
				i->min_tracks = min_tracks;
				i->max_tracks = max_tracks;
				i->author = urldecode(author);
				i->outputs = outputs;
				i->inputs = inputs;

				std::getline(f, flagstring);
				int numparams = atoi(flagstring.c_str());
				for (int j = 0; j < numparams; j++) {
					zzub::parameter* param = load_parameter_cache(f);
					i->global_parameters.push_back(param);
				}

				std::getline(f, flagstring);
				numparams = atoi(flagstring.c_str());
				for (int j = 0; j < numparams; j++) {
					zzub::parameter* param = load_parameter_cache(f);
					i->track_parameters.push_back(param);
				}

				std::getline(f, flagstring);
				numparams = atoi(flagstring.c_str());
				for (int j = 0; j < numparams; j++) {
					zzub::attribute* attr = load_attribute_cache(f);
					i->attributes.push_back(attr);
				}

				infos.push_back(i);

			}

			long cachetime = atol(timestring.c_str());
			long plugintime = filetime(pluginpath + dll_name);
			if (/*uri == "" ||*/ plugintime == 0 || cachetime != plugintime) {
				cout << "plugintime wot " << plugintime << " (from " << (pluginpath + dll_name) << ") vs " << cachetime << " (from " << timestring << ")" << endl;
				delete_infos(infos);
				continue; // dont load from cache when the timestamp changed
			}

			cached_info[dll_name] = infos;
		}

		std::string crashcountstring;
		std::getline(f, crashcountstring);
		int crashcount = atoi(crashcountstring.c_str());
		for (int i = 0; i < crashcount; i++) {
			std::string crashname;
			std::getline(f, crashname);
			crashed_plugins.push_back(crashname);
		}

		return true;
	}

	zzub::attribute* load_attribute_cache(std::istream& f) {
		zzub::attribute* attr = new zzub::attribute();
		std::string name, tempstr;

		std::getline(f, attr->name);
		attr->name = urldecode(attr->name);

		std::getline(f, tempstr);
		attr->value_min = atoi(tempstr.c_str());

		std::getline(f, tempstr);
		attr->value_max = atoi(tempstr.c_str());

		std::getline(f, tempstr);
		attr->value_default = atoi(tempstr.c_str());

		return attr;
	}

	zzub::parameter* load_parameter_cache(std::istream& f) {

		zzub::parameter* param = new zzub::parameter();
		std::string name, description, tempstr;

		std::getline(f, param->name);
		std::getline(f, param->description);

		param->name = urldecode(param->name);
		param->description = urldecode(param->description);

		std::getline(f, tempstr);
		param->flags = atoi(tempstr.c_str());

		std::getline(f, tempstr);
		param->type = (zzub_parameter_type)atoi(tempstr.c_str());

		std::getline(f, tempstr);
		param->value_min = atoi(tempstr.c_str());

		std::getline(f, tempstr);
		param->value_max = atoi(tempstr.c_str());

		std::getline(f, tempstr);
		param->value_none = atoi(tempstr.c_str());

		std::getline(f, tempstr);
		param->value_default = atoi(tempstr.c_str());

		return param;
	}

	bool save_cache(const std::string& pluginpath, const std::string& cachepath) {
		using std::cout;
		using std::cerr;
		using std::endl;

		// save cache in userpath\PluginCache\uniquely_encoded_full_plugin_path.dat
		std::string cachefile = cachepath + cachekey + "-" + urlencode(pluginpath) + ".dat";
		// TODO: create cache directory if not exists?
		//std::string cachefile = path + cachekey + ".dat";

		std::ofstream f;
		f.open(cachefile.c_str());
		if (!f.good()) return false;

		f << get_version() << endl;

		f << (int)registered_infos.size() << endl;

		{
			std::map<std::string, std::vector<PLUGININFO*> >::const_iterator i;
			//std::vector<char> bytes(1024);
			for (i = registered_infos.begin(); i != registered_infos.end(); ++i) {
				f << i->first << endl;
				f << filetime(pluginpath + i->first) << endl;

				f << (int)i->second.size() << endl;

				for (int j = 0; j < (int)i->second.size(); j++) {
					PLUGININFO& info = *i->second[j];

					f << info.uri << endl;
					f << urlencode(info.name) << endl;
					f << info.flags << endl;
					f << urlencode(info.short_name) << endl;
					f << info.min_tracks << endl;
					f << info.max_tracks << endl;
					f << urlencode(info.author) << endl;
					f << info.outputs << endl;
					f << info.inputs << endl;

					f << (int)info.global_parameters.size() << endl;
					for (std::vector<const zzub::parameter*>::const_iterator k = info.global_parameters.begin(); k != info.global_parameters.end(); ++k) {
						save_parameter_cache(f, *k);
					}
					f << (int)info.track_parameters.size() << endl;
					for (std::vector<const zzub::parameter*>::const_iterator k = info.track_parameters.begin(); k != info.track_parameters.end(); ++k) {
						save_parameter_cache(f, *k);
					}

					f << (int)info.attributes.size() << endl;
					for (std::vector<const zzub::attribute*>::const_iterator k = info.attributes.begin(); k != info.attributes.end(); ++k) {
						save_attribute_cache(f, *k);
					}
				}
			}
		}

		f << (int)crashed_plugins.size() << endl;
		{
			for (std::vector<std::string>::iterator i = crashed_plugins.begin(); i != crashed_plugins.end(); ++i) {
				f << *i << endl;
			}
		}

		f.close();

		return true;
	}

	void save_attribute_cache(std::ostream& f, const zzub::attribute* attr) {
		using std::endl;
		f << urlencode(attr->name) << endl;
		f << attr->value_min << endl;
		f << attr->value_max << endl;
		f << attr->value_default << endl;
	}

	void save_parameter_cache(std::ostream& f, const zzub::parameter* param) {
		using std::endl;
		f << urlencode(param->name) << endl;
		f << urlencode(param->description) << endl;
		f << param->flags << endl;
		f << param->type << endl;
		f << param->value_min << endl;
		f << param->value_max << endl;
		f << param->value_none << endl;
		f << param->value_default << endl;
	}


	//
	// locking / crash support 
	// if a file is unexpectedly locked, it likely crashed during last scan
	//

	void lock_plugin(const std::string& pluginfile) {
		std::string cachefile = pluginfile + ".locked";
		std::ofstream f;
		f.open(cachefile.c_str());
		if (!f.good()) return ;

		f << endl;
		f.close();
	}

	void unlock_plugin(const std::string& pluginfile) {
		std::string cachefile = pluginfile + ".locked";
		_unlink(cachefile.c_str());
	}

	bool is_locked_plugin(const std::string& pluginfile) {
		std::string cachefile = pluginfile + ".locked";
		if (GetFileAttributes(cachefile.c_str()) == INVALID_FILE_ATTRIBUTES) return false;
		return true;
	}

	void unlock_crashed_plugins() {
		for (std::vector<string>::iterator i = crashed_plugins.begin(); i != crashed_plugins.end(); ++i) 
			unlock_plugin(*i);
	}

	//
	// utility functions
	//

	long filetime(const std::string& filename) {
		FILETIME ftCreate, ftAccess, ftWrite;
		LARGE_INTEGER li;
		    
		HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) 
			return 0;

		if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) {
			CloseHandle(hFile);
			return 0;
		}

		CloseHandle(hFile);    

		li.LowPart = ftWrite.dwLowDateTime;
		li.HighPart = ftWrite.dwHighDateTime;

		return (long)(li.QuadPart / 10000 / 1000);
	}

	// http://dlib.net/dlib/server/server_http_1.h.html :
    unsigned char to_hex( unsigned char x ) const {
        return x + (x > 9 ? ('A'-10) : '0');
    }

    const std::string urlencode( const std::string& s ) const  {
        std::ostringstream os;

        for ( std::string::const_iterator ci = s.begin(); ci != s.end(); ++ci ) {
            if ( (*ci >= 'a' && *ci <= 'z') ||
                    (*ci >= 'A' && *ci <= 'Z') ||
                    (*ci >= '0' && *ci <= '9') ) { // allowed
                os << *ci;
            } else if ( *ci == ' ') {
                os << '+';
            } else {
                os << '%' << to_hex(*ci >> 4) << to_hex(*ci % 16);
            }
        }

        return os.str();
    }

    unsigned char from_hex (unsigned char ch) const {
        if (ch <= '9' && ch >= '0')
            ch -= '0';
        else if (ch <= 'f' && ch >= 'a')
            ch -= 'a' - 10;
        else if (ch <= 'F' && ch >= 'A')
            ch -= 'A' - 10;
        else 
            ch = 0;
        return ch;
    }

    const std::string urldecode (const std::string& str) const {
        using namespace std;
        string result;
        string::size_type i;
        for (i = 0; i < str.size(); ++i) {
            if (str[i] == '+') {
                result += ' ';
            } else if (str[i] == '%' && str.size() > i+2) {
                const unsigned char ch1 = from_hex(str[i+1]);
                const unsigned char ch2 = from_hex(str[i+2]);
                const unsigned char ch = (ch1 << 4) | ch2;
                result += ch;
                i += 2;
            } else {
                result += str[i];
            }
        }
        return result;
    }

};
