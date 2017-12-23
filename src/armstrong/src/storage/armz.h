typedef void* zipFile;
typedef void* unzFile;

namespace armstrong {

namespace storage {

struct armzwriter {
    zipFile f;

	armzwriter();
	bool open(std::string filename);

	bool add_song(armstrong::storage::document* song, const std::vector<int>& plugins = std::vector<int>(), int fromplugingroup_id = 0);
	bool add_file(std::string zipname, std::string srcfile);
	bool add_chunk(std::string zipname, char* data, int bytes);

	void close();
};

struct armzreader {
	unzFile f;
	std::string filename;

	armzreader();
	bool open(std::string filename);
	bool load(armstrong::storage::document* song);
	void load_wavelevels(armstrong::storage::document* song, std::map<int, int>* wavelevelmappings = 0);
	bool import(armstrong::storage::document* song, int toplugingroup_id);
	bool unpack_file(std::string zipname, std::string destfile);

	void close();
};

}

}
