// --- include ATL headers first ---
#include <atlbase.h>
#include <wtl/atlapp.h>
#include <atlwin.h>
#include <wtl/atlctrls.h>
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include "utils.h"

// ---------------------------------------------------------------------------------------------------------------
// CLIPBOARD
// ---------------------------------------------------------------------------------------------------------------

void CopyBinary(HWND ownerWnd, char* formatString, const char* data, int size) {
	UINT format = RegisterClipboardFormat(formatString);
	CopyBinary(ownerWnd, format, data, size);
}

void CopyBinary(HWND ownerWnd, UINT format, const char* data, int size) {
	if(OpenClipboard(ownerWnd)) {
		//allocate some global memory
		HGLOBAL clipbuffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, size);
		char* buffer = (char*)GlobalLock(clipbuffer);

		//put the data into that memory
		memcpy(buffer, data, size);

		//Put it on the clipboard
		GlobalUnlock(clipbuffer);
		SetClipboardData(format,clipbuffer);
		CloseClipboard();
	}
}

BOOL ClipboardHasAudio(HWND hWnd) {
	if (!OpenClipboard(hWnd)) return false;

	HANDLE hData = GetClipboardData(CF_WAVE);
	CloseClipboard();
	return hData != 0;
}

BOOL ClipboardHasFormat(HWND hWnd, LPCTSTR szFormat) {
	BOOL result = FALSE;
	UINT format = RegisterClipboardFormat(szFormat);
	if (format != 0 && OpenClipboard(hWnd)) {
		HANDLE hData = GetClipboardData(format);
		if (hData != 0) result = TRUE;
		CloseClipboard();
	}
	return result;
}

// ---------------------------------------------------------------------------------------------------------------
// WIN32 HELPERS
// ---------------------------------------------------------------------------------------------------------------

bool CreateImageList(CImageList &il, UINT nIDResource, int cx, int nGrow, COLORREF crMask) {
	CBitmap bitmap;
	bitmap.LoadBitmap(nIDResource);
	BITMAP bm;
	bitmap.GetBitmap(&bm);

	UINT flags = 0;
	switch (bm.bmBitsPixel) {
		case 4: flags = ILC_COLOR4; break;
		case 8: flags = ILC_COLOR8; break;
		case 16: flags = ILC_COLOR16; break;
		case 24: flags = ILC_COLOR24; break;
		case 32: flags = ILC_COLOR32; break;
		default: flags = ILC_COLOR4; break;
	} 

	if (!il.Create(cx, bm.bmHeight, flags|ILC_MASK, 0, nGrow)) {
		return false;
	}

	il.Add(bitmap,crMask);
	return true;
}

const CHAR* PeekString(HINSTANCE hinstRes, int idString) {
	const int max_string_len = 16*1024;
	static CHAR pszString[max_string_len];

	HRSRC hrsrc = FindResource(hinstRes, MAKEINTRESOURCE(idString), RT_RCDATA);
	if (hrsrc != NULL) {
		HGLOBAL hglob = LoadResource(hinstRes, hrsrc);
		if (hglob != NULL) {
			const CHAR* pszText = (const CHAR *) LockResource(hglob);
			DWORD len = SizeofResource(hinstRes, hrsrc);
			assert(len < max_string_len);
			if (len > max_string_len)
				len = max_string_len;
			strncpy(pszString, pszText, max_string_len);
			pszString[len] = 0;
		}
	}

	return pszString;
}

std::string read_file(std::string const& path) {
	FILE* file = fopen(path.c_str(), "rb");

	if (!file)
		return "";

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	std::string text;
	char* buffer = new char[size + 1];
	buffer[size] = 0;

	if (fread(buffer, 1, size, file) == (unsigned long)size)
		text = buffer;

	fclose(file);
	delete[] buffer;

	return text;
}

std::string get_command_text(WORD cmd) {
	std::stringstream strm;
	//static char buffer[1024];
	std::vector<char> buffer(1024);
	AtlLoadString(cmd, &buffer.front(), 1024);
	std::vector<char>::iterator i = find(buffer.begin(), buffer.end(), '\n');
	if (i != buffer.end()) *i = '\0';
	strm << &buffer.front();
	return strm.str();
}

std::string get_key_name(ACCEL& accel) {
	std::stringstream strm;
	if (accel.fVirt & FCONTROL)
		strm << "Ctrl+";

	if (accel.fVirt & FSHIFT)
		strm << "Shift+";

	if (accel.fVirt & FALT)
		strm << "Alt+";

	if (accel.fVirt & FVIRTKEY) {
		// key is a virtual key code, VK_* etc
		static char buffer[256];
		UINT scanCode = MapVirtualKey(accel.key, 0) << 16;
		//UINT scanCode = accelData[accelIndex].key << 16;
		GetKeyNameText(scanCode, buffer, 256);
		strm << buffer;
	} else {
		// key is a char code
		strm << (char)accel.key;
	}

	return strm.str();
}

int get_sortable_modifier(BYTE fVirt) {
	int modifiervalue = 0;
	if (fVirt & FSHIFT)
		modifiervalue |= 1;
	if (fVirt & FCONTROL)
		modifiervalue |= 2;
	if (fVirt & FALT)
		modifiervalue |= 4;
	return modifiervalue;
}

UINT get_sortable_value(const ACCEL& a) {
	if (a.fVirt & FVIRTKEY) {
		UINT charcode = MapVirtualKey(a.key, 2);
		if (charcode == 0)
			charcode = a.key + 256;
		return charcode * 8 + get_sortable_modifier(a.fVirt);
	} else
		return a.key * 8;
}

bool keysorter(const ACCEL& a, const ACCEL& b) {
	return get_sortable_value(a) < get_sortable_value(b);
}

std::string CreateAccelTableString(HACCEL hAccel) {
	using namespace std;
	std::stringstream result;

	int numAccels = CopyAcceleratorTable(hAccel, 0, 0);
	std::vector<ACCEL> accels(numAccels);
	CopyAcceleratorTable(hAccel, &accels.front(), numAccels);
	std::sort(accels.begin(), accels.end(), keysorter);

	for (int i = 0; i < numAccels; i++) {
		std::string commandtext = get_command_text(accels[i].cmd);
		if (commandtext.empty())
			commandtext = "(missing description)";
		result << "    " << setw(36) << left << get_key_name(accels[i]) << commandtext << "\r\n";
	}

	return result.str();
}

// ---------------------------------------------------------------------------------------------------------------
// BUZELIB HELPERS
// ---------------------------------------------------------------------------------------------------------------

void formatTime(char* pc, const char* label, float sec) {
	int hours = sec / 60.0f / 60.0f;
	sec -= hours * 60.0f * 60.0f;

	int minutes = sec / 60.0f;
	sec -= minutes * 60.0f;

	int seconds = sec;
	sec -= seconds;

	sprintf(pc, "%s %02i:%02i:%02i:%1i", label, hours, minutes, seconds, (int)(sec * 10.0f));
}

std::string GetMixdownFileName(std::string waveFilePath, HWND hWnd) {
	char szFile[260]; // buffer for file name
	strcpy(szFile, waveFilePath.c_str());
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Waveforms (*.wav)\0*.wav\0All files\0*.*\0\0";
	ofn.lpstrDefExt="wav";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_NOCHANGEDIR |OFN_OVERWRITEPROMPT|OFN_EXTENSIONDIFFERENT|OFN_NOREADONLYRETURN;
	ofn.hwndOwner = hWnd;

	if (::GetSaveFileName(&ofn) == TRUE) {
		return ofn.lpstrFile;
		// send a parameter change event for the wave parameter, guis may want to update something
		//_host->control_change(_host->get_metaplugin(), 1, 0, 0, 0, false, true);
	} else {
		return "";
	}
}
