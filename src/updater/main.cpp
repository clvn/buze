#include <windows.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <set>
#include <boost/lexical_cast.hpp>

using std::cout;
using std::endl;

typedef void (*downloadcallback)(void* userdata, int read, int total);

bool download_stream(LPCTSTR url, std::ostream& strm, downloadcallback callback = 0, void* userdata = 0);
bool unpack_zip_to_dir(const char* arc, const char* dir);
bool verify_signature(const std::string& hexsig, std::string filename);

// convert string like x.y.z into byte components of a 32 bit unsigned int
// version numbers can be compared. uses lower 24 bits only, assume subversions can be max 255.
unsigned int parse_version(std::string version) {
	std::string::size_type d0 = version.find_first_of('.');
	if (d0 == std::string::npos) return 0;
	
	std::string::size_type d1 = version.find_first_of('.', d0 + 1);
	if (d1 == std::string::npos) return 0;

	std::string s0 = version.substr(0, d0);
	std::string s1 = version.substr(d0 + 1, d1 - d0 - 1);
	std::string s2 = version.substr(d1 + 1);

	unsigned char n0, n1, n2;
	try {
		n0 = boost::lexical_cast<int, std::string>(s0);
		n1 = boost::lexical_cast<int, std::string>(s1);
		n2 = boost::lexical_cast<int, std::string>(s2);
	} catch (boost::bad_lexical_cast& e) {
		return 0;
	}

	return (unsigned int)(n0 << 16) | (unsigned int)(n1 << 8) | (unsigned int)n2;
}

void kill_buze() {
	HWND hLastWnd = 0;
	for (;;) {
		// TODO: "MainFrame" is a really bad class name!
		HWND hBuzeWnd = FindWindow("MainFrame", NULL);
		if (hBuzeWnd == NULL) break;
		if (hBuzeWnd == hLastWnd) break; // fail-safe mechanism in case theres another "MainFrame" which doesn't terminate on WM_USER+3
		SendMessage(hBuzeWnd, WM_USER + 3, 0, 0);
		hLastWnd = hBuzeWnd;
	}
}

std::string get_temp_filename(const std::string& basename) {

	char path[MAX_PATH];
	GetTempPath(MAX_PATH, path);

	std::stringstream strm;
	strm << path << "\\" << basename;
	return strm.str();
}

std::string get_filename_path(std::string fullpath) {
	std::string::size_type ls = fullpath.find_last_of("\\/");
	if (ls == std::string::npos) return "";
	return fullpath.substr(0, ls);
}

bool spawn_installer(std::string selfname, std::string userversion, std::string exedirectory) {

	// copy self to temp exe
	std::string tempexe = get_temp_filename("updater.exe");
	DeleteFile(tempexe.c_str());
	CopyFile(selfname.c_str(), tempexe.c_str(), FALSE);

	std::stringstream params;
	params << "\"" << tempexe << "\" " << userversion << " \"" << exedirectory << "\"";

	char argstr[32*1024];
	strcpy(argstr, params.str().c_str());

	// start temp exe with cmline args to run in install mode
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));

	cout << argstr << endl;

	BOOL result = CreateProcess(0, argstr, 0, 0, FALSE, CREATE_NEW_CONSOLE, 0, 0, &si, &pi);
	return result != 0;
}

std::string get_full_path(LPCTSTR path) {
	char buffer[MAX_PATH];
	LPTSTR filepart;
	GetFullPathName(path, MAX_PATH, buffer, &filepart);
	return buffer;
}

void download_callback(void* userdata, int readbytes, int total) {
	cout << ".";
	//cout << (readbytes / 1024) << "k of " << (total / 1024) << "k bytes" << endl;
}

// http://stackoverflow.com/questions/1570217/mfc-open-folder-dialog/1573047#1573047
#include <objbase.h>
#include <shlobj.h>

TCHAR szInitialDir[MAX_PATH];

// Set the initial path of the folder browser
int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    if (uMsg == BFFM_INITIALIZED) {
        SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)szInitialDir);
    }
    return 0;
}

std::string get_folder_name(const std::string& startpath, const char* title) {
	BROWSEINFO bi;
    ZeroMemory(&bi,   sizeof(bi)); 
    TCHAR   szDisplayName[MAX_PATH];
    szDisplayName[0]    =   '\0';

    bi.hwndOwner        =   NULL; 
    bi.pidlRoot         =   NULL; 
    bi.pszDisplayName   =   szDisplayName; 
    bi.lpszTitle        =   title;
    bi.ulFlags          =   BIF_NEWDIALOGSTYLE|BIF_RETURNONLYFSDIRS;
    bi.lParam           =   NULL; 
    bi.iImage           =   0;

	if (!startpath.empty()) {
		bi.lpfn = BrowseCallbackProc;
		strcpy(szInitialDir, startpath.c_str());
	}

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (NULL == pidl) return "";

	TCHAR szPathName[MAX_PATH];
    BOOL bRet = SHGetPathFromIDList(pidl,szPathName);
	LPMALLOC pMalloc;
    HRESULT HR = SHGetMalloc(&pMalloc);
    pMalloc->Free(pidl);
    pMalloc->Release();
	
	if (FALSE == bRet)
		return "";

	return szPathName;
}

int main(int argc, char** argv) {

	CoInitialize(0);
	// this program runs in two modes:
	//  - bootstrap, checks version, copies and launches itself in installer mode
	//  - installer, downloads and installs the zips

	std::string selfname = get_full_path(argv[0]);
	std::string userversion;
	std::string userdirectory;
	std::string exedirectory = get_filename_path(selfname);

	if (exedirectory.empty()) {
		cout << "ERROR: Invalid command line arguments" << endl;
		return 12;
	}

	bool bootstrap = true;

	if (argc == 1) {
		std::ifstream vfile("version.txt", std::ios::in);
		if (vfile) {
			vfile >> userversion;
			vfile.close();
			bootstrap = true;
		} else {
			// show a directory-picker and let the user choose a directory where to download and install buze
			userversion = "0.0.1";
			userdirectory = get_folder_name(exedirectory, "Please select a folder to install Buzé:");
			if (userdirectory.empty()) {
				//cout << "ERROR: Missing version.txt" << endl;
				return 17;
			}
			bootstrap = false;
		}
	} else if (argc == 3) {
		userversion = argv[1];
		userdirectory = argv[2];
		bootstrap = false;
	} else {
		cout << "Usage: updater.exe" << endl << endl;
		cout << "updater.exe must be placed in the Buze program directory." << endl;
		return 100;
	}

	// 1. get current version (cmdline arg)
	// 2. get server version and signature (http request)
	// 3. ask user to shut down buze and download/apply update
	// copy oiurself to temp, so we can update the updater as well when unzippering
	// 4. shut down buze
	// 5. download zip, show progress, 
	// 6. check signature, unpack zip
	// 7. restart buze.exe


	unsigned int userversionnumber = parse_version(userversion);
	if (userversionnumber == 0) {
		cout << "ERROR: Invalid user version" << endl;
		return 1;
	}

	cout << "Checking server version...";
	std::stringstream strm;
	download_stream("http://www.batman.no/buze/updater.txt", strm);
	cout << endl;

	std::string serverversion, signature, serverfile1;
	strm >> serverversion;
	strm >> signature;
	strm >> serverfile1;

	unsigned int serverversionnumber = parse_version(serverversion);
	if (serverversionnumber == 0) {
		if (bootstrap) {
			cout << "ERROR: Invalid server data" << endl;
		} else {
			MessageBox(GetForegroundWindow(), "ERROR: Invalid server data", "Buzé Update Failure", MB_OK);
		}
		return 2;
	}

	cout << "User version:   " << std::setw(6) << std::setfill('0') << std::hex << userversionnumber << endl;
	cout << "Server version: " << std::setw(6) << std::setfill('0') << std::hex << serverversionnumber << endl;

	cout << std::dec << endl;

	if (serverversionnumber > userversionnumber) {

		if (bootstrap) {

			// TODO: verify there is a buze.exe in the exedirectory? or allow installing from scratch

			if (IDYES == MessageBox(GetForegroundWindow(), "There is a newer version of Buzé available on the server.\r\n\r\nDo you want to download and install it?", "Buzé Auto Update", MB_YESNO)) {

				bool result = spawn_installer(selfname, userversion, exedirectory);
				if (result) {
					cout << "Bootstrapper successful. Installer started" << endl;
					return 0;
				} else {
					cout << "ERROR: Bootstrapper could not spawn installer." << endl;
					return 11;
				}
			} else {
				return 10;
			}
		} else {

			cout << "Stopping Buze...";
			kill_buze();
			cout << endl;

			std::string tempfile = get_temp_filename("buzeupdate.dat");
			
			std::ofstream f(tempfile.c_str(), std::ios::binary|std::ios::out);
			
			if (!f) {
				MessageBox(GetForegroundWindow(), "ERROR: Unable to create temporary file", "Buzé Update Failure", MB_OK);
				return 1;
			}

			cout << "Downloading...";
			bool dlresult = download_stream(serverfile1.c_str(), f, &download_callback);
			f.close();
			cout << endl;

			if (!dlresult) {
				MessageBox(GetForegroundWindow(), "ERROR: Failed downloading update", "Buzé Update Failure", MB_OK);
				return 16;
			}

			cout << "Verifying signature...";
			if (!verify_signature(signature, tempfile)) {
				MessageBox(GetForegroundWindow(), "ERROR: Could not verify signature of update", "Buzé Update Failure", MB_OK);
				return 20;
			}
			cout << endl;

			cout << "Unzipping to " << userdirectory << "...";
			if (!unpack_zip_to_dir(tempfile.c_str(), userdirectory.c_str())) {
				MessageBox(GetForegroundWindow(), "ERROR: Failed unzip", "Buzé Update Failure", MB_OK);
				return 15;
			}
			cout << endl;

			DeleteFile(tempfile.c_str());
		}
	}

	return 0;

}
