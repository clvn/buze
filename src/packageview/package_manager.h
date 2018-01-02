#pragma once

#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include "../../armstrong/src/minizip/unzip.h"
#include "crypto.h"

#define CASESENSITIVITY (0)

typedef void(*downloadcallback)(void* userdata, int read, int total);
bool download_stream(LPCTSTR url, std::ostream& strm, LPDWORD statusCode, downloadcallback callback, void* userdata);

enum package_file_target {
	buzz_generators,
	buzz_effects,
	buzz_gear,
	vst,
	buze_views,
	program_directory,
	user_directory,
};

enum package_platform {
	windows_x86,
	windows_x64
};

struct package_file {
	package_file_target filetype;
	std::string source_filename;
	std::string target_filename;
};

struct package {
	std::string url;
	std::string hash;
	std::string id;
	std::string version;
	std::string archive;
	package_platform platform;
	std::vector<package_file> files;
};

struct package_manager {
	std::string program_directory;
	std::string user_directory;
	std::string download_directory;
	std::vector<package> installed;

	void read_installed(const std::string& filename) {
		Json::Reader reader;
		Json::Value root;
		std::ifstream f(filename);
		bool parsingSuccessful = reader.parse(f, root);
		f.close();
		if (!parsingSuccessful)
			return;

		if (!root.isArray())
			return;

		for (Json::ValueIterator i = root.begin(); i != root.end(); ++i) {
			Json::Value group = *i;
			if (!group.isObject())
				continue;
			package result;
			if (!read_package(group, result))
				continue;
			installed.push_back(result);
		}
	}

	void write_installed(const std::string& filename) {
		Json::Value root(Json::ValueType::arrayValue);

		for (std::vector<package>::iterator i = installed.begin(); i != installed.end(); ++i) {
			package& group = *i;

			Json::Value result_files(Json::ValueType::arrayValue);
			for (std::vector<package_file>::iterator j = group.files.begin(); j != group.files.end(); ++j) {
				package_file& file = *j;
				Json::Value result_file(Json::ValueType::objectValue);
				result_file["source_filename"] = file.source_filename;
				result_file["target_filename"] = file.target_filename;
				result_file["filetype"] = format_package_file(file.filetype);
				result_files.append(result_file);
			}

			Json::Value result_group(Json::ValueType::objectValue);
			result_group["id"] = group.id;
			result_group["platform"] = format_platform(group.platform);
			result_group["version"] = group.version;
			result_group["archive"] = group.archive;
			result_group["hash"] = group.hash;
			result_group["url"] = group.url;
			result_group["files"] = result_files;

			root.append(result_group);
		}

		std::ofstream f(filename, std::ios::out | std::ios::trunc);
		Json::StreamWriterBuilder factory;
		Json::StreamWriter* writer = factory.newStreamWriter();
		writer->write(root, &f);
		f.close();
	}

	std::string format_package_file(package_file_target t) {
		switch (t) {
		case package_file_target::buzz_gear:
			return "buzz_gear";
		case package_file_target::buzz_generators:
			return "buzz_generators";
		case package_file_target::buzz_effects:
			return "buzz_effects";
		case package_file_target::program_directory:
			return "program_directory";
		case package_file_target::user_directory:
			return "user_directory";
		}
		assert(false);
		return "unknown";
	}

	std::string format_platform(package_platform p) {
		switch (p) {
		case package_platform::windows_x86:
			return "x86-windows";
		case package_platform::windows_x64:
			return "x64-windows";
		}
		assert(false);
		return "unknown";
	}

	void scan(const std::string& filename, const std::function<void(package&)>& callback) {
		Json::Reader reader;
		Json::Value root;
		std::ifstream f(filename);
		if (!f)
			return;

		bool parsingSuccessful = reader.parse(f, root);
		f.close();
		if (!parsingSuccessful)
			return;

		if (!root.isArray())
			return;
		
		for (Json::ValueIterator i = root.begin(); i != root.end(); ++i) {
			Json::Value packageObject = *i;
			if (!packageObject.isObject())
				return;
			package result;
			if (!read_package(packageObject, result))
				continue;

			callback(result);
		}
	}

	bool read_package(const Json::Value& packageObject, package& result) {

		result.url = packageObject["url"].asString();
		result.hash = packageObject["hash"].asString();
		result.id = packageObject["id"].asString();
		result.version = packageObject["version"].asString();

		Json::Value archive = packageObject["archive"];
		if (archive.isNull())
			result.archive = "zip";
		else
			result.archive = archive.asString();

		std::string platform = packageObject["platform"].asString();
		if (platform == "x86-windows")
			result.platform = package_platform::windows_x86;
		else if (platform == "x64-windows")
			result.platform = package_platform::windows_x64;
		else
			return false;

		const Json::Value files = packageObject["files"];
		if (files.isArray()) {
			for (Json::ValueConstIterator i = files.begin(); i != files.end(); ++i) {
				const Json::Value& file = *i;
				if (!file.isObject())
					return false;

				package_file result_file;
				if (!read_package_file(file, result_file))
					return false;

				result.files.push_back(result_file);
			}
		}
		return true;
	}


	bool read_package_file(const Json::Value& file, package_file& result_file) {
		result_file.source_filename = file["source_filename"].asString();
		result_file.target_filename = file["target_filename"].asString();
		std::string filetype = file["filetype"].asString();
		if (filetype == "buzz_generators")
			result_file.filetype = package_file_target::buzz_generators;
		else if (filetype == "buzz_effects")
			result_file.filetype = package_file_target::buzz_effects;
		else if (filetype == "program_directory")
			result_file.filetype = package_file_target::program_directory;
		else if (filetype == "user_directory")
			result_file.filetype = package_file_target::user_directory;
		else
			return false;

		return true;
	}

	bool install(const package& pkg, std::ostream& log) {
		boost::filesystem::path filepath = boost::filesystem::path(download_directory) / 
			boost::filesystem::path(urlencode(pkg.url) + "." + pkg.archive);

		if (!boost::filesystem::is_directory(download_directory)) {
			if (!boost::filesystem::create_directories(filepath.parent_path())) {
				log << "ERROR: " << pkg.id << ": Cannot create directory " << filepath.parent_path().string() << std::endl;
				return false;
			}
		}

		std::string filename  = filepath.string();
		if (!download_package(filename, pkg)) {
			log << "ERROR: " << pkg.id << ": Cannot download " << pkg.url << std::endl;
			return false;
		}

		if (!verify_hash(filename, pkg, log)) {
			return false;
		}

		if (pkg.archive == "zip") {
			if (!unzip_package_files(filename, pkg, log)) {
				return false;
			}
			installed.push_back(pkg);
		}
		else if (pkg.archive == "exe") {
			// Trust the hash in the json and launch!
			ShellExecute(NULL, NULL, filename.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
	}

	bool verify_hash(const std::string& filename, const package& pkg, std::ostream& log) {
		HCRYPTPROV hProv;

		BOOL bResult = CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
		if (!bResult) {
			log << "ERROR: " << pkg.id << ": Cannot create crypto provider" << std::endl;
			return false;
		}

		std::vector<unsigned char> downloadHash;
		HCRYPTHASH hHash = create_hash_from_file(hProv, filename.c_str());
		get_hash_bytes(hHash, downloadHash);

		std::vector<unsigned char> expectedHash;
		parse_hexstring(pkg.hash, expectedHash);

		if (downloadHash != expectedHash) {
			log << "ERROR: " << pkg.id << ": Cannot verify hash, was [ ";
			print_hexstream(log, downloadHash);
			log << "] " << std::endl;
			return false;
		}

		return true;
	}

	void uninstall(const package& pkg) {
		auto orig = find_package(pkg);
		if (orig == installed.end())
			return;

		for (std::vector<package_file>::const_iterator i = pkg.files.begin(); i != pkg.files.end(); ++i) {
			const package_file& pkg_file = *i;

			std::string target_file = get_target_filename(pkg_file);
			boost::filesystem::remove(target_file);
		}

		installed.erase(orig);
	}

	std::string get_target_filename(const package_file& pkg_file) const {
		
		boost::filesystem::path target_directory;
		switch (pkg_file.filetype) {
		case package_file_target::buzz_generators:
			target_directory = (boost::filesystem::path(program_directory) / boost::filesystem::path("Gear\\Generators")).string();
			break;
		case package_file_target::buzz_effects:
			target_directory = (boost::filesystem::path(program_directory) / boost::filesystem::path("Gear\\Effects")).string();
			break;
		case package_file_target::program_directory:
			target_directory = boost::filesystem::path(program_directory).string();
			break;
		case package_file_target::user_directory:
			target_directory = boost::filesystem::path(user_directory).string();
			break;
		default:
			assert(false);
			break;
		}

		return (target_directory / pkg_file.target_filename).string();
	}

	bool unzip_package_files(const std::string filename, const package& pkg, std::ostream& log) const {
		unzFile f = unzOpen(filename.c_str());

		if (!f) {
			log << "WARNING: " << pkg.id << ": Cannot locate zip file " << filename << std::endl;
			return false;
		}

		const int size_buf = 16384;
		char buf[size_buf];

		int err;
		for (std::vector<package_file>::const_iterator i = pkg.files.begin(); i != pkg.files.end(); ++i) {
			const package_file& pkg_file = *i;
			if (unzLocateFile(f, pkg_file.source_filename.c_str(), CASESENSITIVITY) != UNZ_OK) {
				log << "WARNING: " << pkg.id << ": Cannot locate source file " << pkg_file.source_filename << std::endl;
				continue;
			}

			if (UNZ_OK != unzOpenCurrentFile(f)) {
				log << "WARNING: " << pkg.id << ": Cannot open source file in zip " << pkg_file.source_filename << std::endl;
				continue;
			}

			std::string unpacked_filename = get_target_filename(pkg_file);

			std::string unpacked_directory = boost::filesystem::path(unpacked_filename).parent_path().string();
			boost::filesystem::create_directories(unpacked_directory);

			std::ofstream fout(unpacked_filename.c_str(), std::ios::binary | std::ios::out);

			if (fout) {
				do {
					err = unzReadCurrentFile(f, buf, size_buf);
					if (err < 0) {
						break;
					}
					if (err > 0) {
						fout.write(buf, err);
					}
				} while (err > 0);

				fout.close();
			}

			unzCloseCurrentFile(f);
		}

		unzClose(f);
		return true;
	}

	bool download_package(const std::string& filename, const package& pkg) {

		if (boost::filesystem::exists(filename))
			return true;

		std::ofstream f(filename.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);

		if (!f)
			return false;

		DWORD statusCode;
		bool success = download_stream(pkg.url.c_str(), f, &statusCode, 0, 0);

		f.close();

		if (!success || statusCode < 200 || statusCode > 299) {
			boost::filesystem::remove(filename);
			return false;
		}

		return true;
	}

	std::vector<package>::const_iterator find_package(const package& self) const {
		for (std::vector<package>::const_iterator i = installed.begin(); i != installed.end(); ++i) {
			const package& install = *i;
			if (install.id == self.id && install.version == self.version && install.platform == self.platform) {
				return i;
			}
		}
		return installed.end();
	}

	// http://dlib.net/dlib/server/server_http_1.h.html :
	static unsigned char to_hex(unsigned char x) {
		return x + (x > 9 ? ('A' - 10) : '0');
	}

	static const std::string urlencode(const std::string& s) {
		std::ostringstream os;

		for (std::string::const_iterator ci = s.begin(); ci != s.end(); ++ci) {
			if ((*ci >= 'a' && *ci <= 'z') ||
				(*ci >= 'A' && *ci <= 'Z') ||
				(*ci >= '0' && *ci <= '9')) { // allowed
				os << *ci;
			}
			else if (*ci == ' ') {
				os << '+';
			}
			else {
				os << '%' << to_hex(*ci >> 4) << to_hex(*ci % 16);
			}
		}

		return os.str();
	}
};
