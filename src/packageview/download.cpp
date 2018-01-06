#include <windows.h>
#include <wininet.h>
#include <tchar.h>
#include <ostream>
#include <iostream>

#define DOWNLOAD_CHUNK_SIZE  8192
#define DOWNLOAD_UPDATE_SIZE 262144

typedef void (*downloadcallback)(void* userdata, int read, int total);

bool download_stream(LPCTSTR url, std::ostream& strm, LPDWORD statusCode, downloadcallback callback, void* userdata) {
	BOOL            Succes;
	CHAR            Buffer[DOWNLOAD_CHUNK_SIZE];
	DWORD           BufferSize = DOWNLOAD_CHUNK_SIZE;
	DWORD           NumBytesReceived;
	DWORD           NumBytes;
	DWORD           NumBytesRead;
	DWORD           NumBytesSinceLastUpdate = 0;
	HINTERNET       hURL;

	HINTERNET hNet = InternetOpen(_T("Mozilla/4.0 (compatible)"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
	bool result = false;
	if (hNet != NULL) {

		hURL = InternetOpenUrl(hNet, url, NULL, 0, INTERNET_FLAG_HYPERLINK|INTERNET_FLAG_NO_UI|INTERNET_FLAG_NO_COOKIES|INTERNET_FLAG_RELOAD|INTERNET_FLAG_DONT_CACHE, 0);
		if (hURL != NULL) {

			// No error checking, assume fails if Content-Length header is missing:
			DWORD contentLength = 0;
			DWORD bufferSize = sizeof(DWORD);
			HttpQueryInfo(hURL, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &bufferSize, 0);

			DWORD statusCodeSize = sizeof(DWORD);
			Succes = HttpQueryInfo(hURL, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, statusCode, &statusCodeSize, NULL);

			if (Succes) {

				result = true;
				NumBytesReceived = 0;
				NumBytesRead = 1;

				while (NumBytesRead != 0) {
					Succes = InternetQueryDataAvailable(hURL, &NumBytes, 0, 0);
					if (!Succes) {
						result = false;
						break;
					}

					Succes = InternetReadFile ( hURL, Buffer, DOWNLOAD_CHUNK_SIZE, &NumBytesRead );
					if (!Succes) {
						result = false;
						break;
					}

					NumBytesReceived += NumBytesRead;
					strm.write(Buffer, NumBytesRead);

					NumBytesSinceLastUpdate += NumBytesRead;

					if (callback != 0) {

						callback(userdata, NumBytesSinceLastUpdate, contentLength);
					}
				}
			}
			InternetCloseHandle(hURL);
		}
		InternetCloseHandle(hNet);
	}

	return result;
}
