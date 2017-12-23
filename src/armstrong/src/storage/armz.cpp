#include <fstream>
#include <sstream>
#include <vector>
#include "document.h"
#include "armz.h"
#include "unzip.h"
#include "zip.h"

#if defined(_WIN32)
#define unlink _unlink
#endif

namespace armstrong {

namespace storage {

armzwriter::armzwriter() {
	f = 0;
}

bool armzwriter::open(std::string filename) {
    f = zipOpen(filename.c_str(), APPEND_STATUS_CREATE);
    return f != 0;
}

void armzwriter::close() {
	assert(f != 0);
	zipClose(f, 0);
	f = 0;
}

bool armzwriter::add_song(armstrong::storage::document* song, const std::vector<int>& plugins, int fromplugingroup_id) {

	assert(f != 0);

	std::string tempfile = song->get_unique_session_filename("song", 0, "db");
	if (!song->save(tempfile, plugins, fromplugingroup_id)) {
		//zipClose(f, 0);
		return false;
	}

	if (!add_file("song.armdb", tempfile)) {
		unlink(tempfile.c_str());
		//zipClose(f, 0);
		return false;
	}
	unlink(tempfile.c_str());

	if (plugins.size() == 0) {
		armstrong::storage::song songdata(song);
		song->get_song(songdata);
		int wavecount = songdata.get_wave_count();
		for (int i = 0; i < wavecount; i++) {
			armstrong::storage::wave wavedata(song);
			songdata.get_wave_by_index(i, wavedata);
			int wavelevelcount = wavedata.get_wavelevel_count();
			for (int j = 0; j < wavelevelcount; j++) {
				armstrong::storage::wavelevel waveleveldata(song);
				wavedata.get_wavelevel_by_index(j, waveleveldata);
				
				std::string rawfile = song->wavelevel_get_filename(waveleveldata.id);
				std::stringstream strm;
				strm << "wavelevel-" << waveleveldata.id << ".raw";
				add_file(strm.str(), rawfile);
			}
		}
	}

	return true;
}

bool armzwriter::add_file(std::string zipname, std::string srcfile) {

    FILE* fin = fopen(srcfile.c_str(), "rb");
    if (fin == NULL) {
        return false;
    }

	if (ZIP_OK != zipOpenNewFileInZip(f, zipname.c_str(), 0, 0, 0, 0, 0, 0, Z_DEFLATED, Z_DEFAULT_COMPRESSION)) {
		fclose(fin);
        return false;
	}

	const int size_buf = 16384;
	char buf[size_buf];

	int err, size_read;
    do {
        err = ZIP_OK;
        size_read = (int)fread(buf,1,size_buf,fin);
        if (size_read < size_buf && feof(fin) == 0) 
			err = ZIP_ERRNO;

        if (size_read > 0) 
			err = zipWriteInFileInZip(f, buf, size_read);
    } while ((err == ZIP_OK) && (size_read>0));

	fclose(fin);
    zipCloseFileInZip(f);
	return err == ZIP_OK;
}

int peek(const std::string& filename, char* ptr, int size) {
	std::ifstream fs;
	fs.open(filename.c_str(), std::ios::in | std::ios::binary);
	if (!fs) return 0;

	fs.read(ptr, size);
	size = (int)fs.tellg();
	fs.close();

	return size;
}

armzreader::armzreader() {
	f = 0;
}

bool armzreader::open(std::string openfilename) {
	// probe header for binary magic
	// if its an sqlite database, set f to 0 and filename to the filename
	// if its a zip file, set f to the zip file handle, and blank the filename
	// ie, null f and blank filename is an error (returns false)
	const int header_size = 15;
	char header[header_size];
	if (peek(openfilename, header, header_size) != header_size) return false;

	if (header[0] == 'S' && header[1] == 'Q' && header[2] == 'L' && header[3] == 'i' && header[4] == 't' && header[5] == 'e') {
		f = 0;
		filename = openfilename;
		return true;
	} else
	if (header[0] == 'P' && header[1] == 'K') {
		filename = "";
		f = unzOpen(openfilename.c_str());
		return f != 0;
	} else
		return false;
}

void armzreader::load_wavelevels(armstrong::storage::document* song, std::map<int, int>* wavelevelmappings) {
	armstrong::storage::song songdata(song);
	song->get_song(songdata);
	int wavecount = songdata.get_wave_count();
	for (int i = 0; i < wavecount; i++) {
		armstrong::storage::wave wavedata(song);
		songdata.get_wave_by_index(i, wavedata);
		int wavelevelcount = wavedata.get_wavelevel_count();
		for (int j = 0; j < wavelevelcount; j++) {
			armstrong::storage::wavelevel waveleveldata(song);
			wavedata.get_wavelevel_by_index(j, waveleveldata);
			
			std::stringstream strm;
			if (wavelevelmappings) {
				std::map<int, int>::iterator i = wavelevelmappings->find(waveleveldata.id);
				if (i == wavelevelmappings->end())
					continue;
				strm << "wavelevel-" << i->second << ".raw";
			} else {
				strm << "wavelevel-" << waveleveldata.id << ".raw";
			}

			std::string rawfile = song->wavelevel_get_filename(waveleveldata.id);
			unpack_file(strm.str(), rawfile);
			
			// send some kind of notification so the mixer can flush:
			armstrong::storage::document_event_data ev;
			ev.type = armstrong::storage::event_type_insert_samples;
			ev.id = waveleveldata.id;
			song->notify_listeners(&ev);

		}
	}

}

bool armzreader::load(armstrong::storage::document* song) {

	if (!f && !filename.empty()) {
		return song->load(filename);
	} else
	if (f) {

		std::string tempfile = song->get_unique_session_filename("song", 0, "db");
		unpack_file("song.armdb", tempfile);

		song->load(tempfile);
		unlink(tempfile.c_str());

		load_wavelevels(song);

		return true;
	}
	return false;
}

bool armzreader::import(armstrong::storage::document* song, int toplugingroup_id) {
	if (!f && !filename.empty()) {
		return song->import(filename, toplugingroup_id, false);
	} else
	if (f) {

		std::string tempfile = song->get_unique_session_filename("song", 0, "db");
		unpack_file("song.armdb", tempfile);

		std::map<int, int> wavelevelmappings;
		song->import(tempfile, toplugingroup_id, true, &wavelevelmappings);
		unlink(tempfile.c_str());

		load_wavelevels(song, &wavelevelmappings);
		return true;
	}
	return false;
}

void armzreader::close() {
	if (f != 0) {
		assert(filename.empty());
		unzClose(f);
		f = 0;
	} else {
		assert(!filename.empty());
		filename = "";
	}
}

#define CASESENSITIVITY (0)

bool armzreader::unpack_file(std::string zipname, std::string destfile) {
	assert(f != 0);
	if (unzLocateFile(f, zipname.c_str(), CASESENSITIVITY) != UNZ_OK) return false;

	unz_file_info file_info;
	char fileNameInZip[32768];

	const int size_buf = 16384;
	char buf[size_buf];
	int err = unzGetCurrentFileInfo(f, &file_info, fileNameInZip, 32768, NULL, 0, NULL, 0);
	if (err != UNZ_OK) return false;

	if (UNZ_OK != unzOpenCurrentFile(f)) return false;

	FILE* fout = fopen(destfile.c_str(), "wb");
	do {
		err = unzReadCurrentFile(f, buf, size_buf);
		if (err < 0) {
            //printf("error %d with zipfile in unzReadCurrentFile\n",err);
            break;
        }
        if (err > 0)
            if (fwrite(buf, err, 1, fout) != 1) {
                //printf("error in writing extracted file\n");
                err = UNZ_ERRNO;
                break;
            }
    } while (err > 0);
    fclose(fout);

    //int err = unzReadCurrentFile(f, charBuffer, size);

    unzCloseCurrentFile(f);
	return err == UNZ_OK;
}


}

}
