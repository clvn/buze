#include <windows.h>
#include <wininet.h>
#include <tchar.h>
#include <ostream>
#include <iostream>

using std::cout;
using std::endl;

// http://www.google.no/codesearch/p?hl=en#EuengHJwXig/~mooij/httpleech.cpp&q=HttpOpenRequest%20lang:c%2B%2B&sa=N&cd=10&ct=rc

#define DOWNLOAD_CHUNK_SIZE  8192
#define DOWNLOAD_UPDATE_SIZE 262144

typedef void (*downloadcallback)(void* userdata, int read, int total);

bool download_stream(LPCTSTR url, std::ostream& strm, downloadcallback callback, void* userdata) {
	BOOL            Succes;
	CHAR            Buffer[DOWNLOAD_CHUNK_SIZE];
	DWORD           BufferSize = DOWNLOAD_CHUNK_SIZE;
	DWORD           NumBytesReceived;
	DWORD           NumBytes;
	DWORD           NumBytesRead;
	DWORD           NumBytesSinceLastUpdate = 0;
	HINTERNET       hURL;
	//DWORD           NumBytesWritten;

	HINTERNET hNet = InternetOpen(_T("TrayRSS 1.0"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	bool result;
	if (hNet != NULL) {

		hURL = InternetOpenUrl(hNet, url, NULL, 0, INTERNET_FLAG_HYPERLINK, 0);
		if (hURL != NULL) {

			Succes = InternetQueryDataAvailable ( hURL, &NumBytes, 0, 0 );
			if (Succes) {

				DWORD contentLength = 0;
				DWORD bufferSize = sizeof(DWORD);
				Succes = HttpQueryInfo(hURL, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &bufferSize, 0);

				//HANDLE hOut = CreateFile(outfilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				//if (hOut != INVALID_HANDLE_VALUE) {

					NumBytesReceived = 0;
					NumBytesRead = 1;

					while (NumBytesRead != 0) {
						Succes = InternetReadFile ( hURL, Buffer, DOWNLOAD_CHUNK_SIZE, &NumBytesRead );
						if (!Succes) return false;

						NumBytesReceived += NumBytesRead;
						//WriteFile(hOut, Buffer, NumBytesRead, &NumBytesWritten, 0);
						strm.write(Buffer, NumBytesRead);
						
						NumBytesSinceLastUpdate += NumBytesRead;

						/*if (NumBytesWritten != NumBytesRead) {
							// error writing file
							result = false;
							break;
						}*/

						if (callback != 0) {

							callback(userdata, NumBytesSinceLastUpdate, contentLength);
						}
					}

					//CloseHandle(hOut);
				//} else {
				//	// cant create file
				//	result = false;
				//}
			} else {
				// no data
				result = false;
			}
			InternetCloseHandle(hURL);
		} else {
			// invalid url
			result = false;
		}
		InternetCloseHandle(hNet);
	} else {
		// invalid internet
		result = false;
	}

	return true;
}
