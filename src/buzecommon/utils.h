#pragma once

#include <string>
#include <sstream>
#include "PrefGlobals.h"

// ---------------------------------------------------------------------------------------------------------------
// IS_xxx FLAG MASKS
// ---------------------------------------------------------------------------------------------------------------

#define PLUGIN_FLAGS_MASK (zzub_plugin_flag_is_root|zzub_plugin_flag_has_audio_input|zzub_plugin_flag_has_audio_output|zzub_plugin_flag_has_event_output)
#define IS_ROOT_PLUGIN_FLAGS (zzub_plugin_flag_is_root|zzub_plugin_flag_has_audio_input)
#define IS_GENERATOR_PLUGIN_FLAGS (zzub_plugin_flag_has_audio_output)
#define IS_EFFECT_PLUGIN_FLAGS (zzub_plugin_flag_has_audio_input)
#define IS_CONTROLLER_PLUGIN_FLAGS (zzub_plugin_flag_has_event_output)

// ---------------------------------------------------------------------------------------------------------------
// DEBUGGING MACROS
// ---------------------------------------------------------------------------------------------------------------

#define PRN(x) { std::cerr << x << "  "; }
#define DBG(x) { std::cerr << #x << "=" << (x) << "  "; }
#define END() { std::cerr << std::endl; }

// ---------------------------------------------------------------------------------------------------------------
// LANGUAGE UTILS
// ---------------------------------------------------------------------------------------------------------------

template <class X, class Y, class Z>
inline X clamp(X value, Y minimum, Z maximum) {
	return value < minimum ? (X)minimum : value > maximum ? (X)maximum : value;
}

// ---------------------------------------------------------------------------------------------------------------
// STRING OPERATIONS
// ---------------------------------------------------------------------------------------------------------------

inline std::string stringFromInt(int i, int len = 0, char fillChar = ' ') {
	char pc[16];
	sprintf(pc, "%i", i);
	std::string s = pc;
	while (s.length() < (size_t)len)
		s = fillChar + s;
	return s;
}

inline std::string fillString(char c, int l) {
	std::string r = "";
	for (int i = 0; i < l; ++i)
		r += c;
	return r;
}

// ---

// taken from comment on
// http://www.codeproject.com/string/stringsplit.asp?df=100&forumid=2167&exp=0&select=1062827#xx1062827xx
template<typename _Cont>
inline void split(const std::string& str, _Cont& _container, const std::string& delim = ",") {
    std::string::size_type lpos = 0;
    std::string::size_type pos = str.find_first_of(delim, lpos);
    while(lpos != std::string::npos) {
        _container.insert(_container.end(), str.substr(lpos, pos - lpos));

        lpos = (pos == std::string::npos) ? std::string::npos : pos + 1;
        pos = str.find_first_of(delim, lpos);
    }
}
/*	split example:
		vector<string> subcommands;
		split<vector<string> > (commandString, subcommands, "\n");
*/

// ---

// found the trims in one of the comments at
// http://www.codeproject.com/vcpp/stl/stdstringtrim.asp

inline std::string& trimleft(std::string& s) {
	std::string::iterator it;

	for (it = s.begin(); it != s.end(); ++it)
		if (!isspace((unsigned char)*it))
			break;

	s.erase(s.begin(), it);
	return s;
}

inline std::string& trimright(std::string& s) {
	std::string::difference_type dt;
	std::string::reverse_iterator it;

	for (it = s.rbegin(); it != s.rend(); ++it)
		if (!isspace((unsigned char)*it))
			break;

	dt = s.rend() - it;

	s.erase(s.begin() + dt, s.end());
	return s;
}

inline std::string& trim(std::string& s) {
	trimleft(s);
	trimright(s);
	return s;
}

inline std::string trim(const std::string& s) {
	std::string t = s;
	return trim(t);
}

// ---------------------------------------------------------------------------------------------------------------
// CONVERSIONS
// ---------------------------------------------------------------------------------------------------------------

inline int intFromHex(std::string const& s) {
	int num;
	std::stringstream(s) >> std::hex >> num;
	return num;
}

inline std::string hexFromInt(int i, int len, char fillChar) {
	char pc[16];
	sprintf(pc, "%X", i);
	std::string s = pc;
	while (s.length() < (size_t)len)
		s = fillChar + s;

	return s;
}

inline std::string noteFromInt(unsigned char i) {
	if (i == 255) return noteOffStr;
	else if (i == 254) return noteCutStr;

	static const char* const notes[] = {
		"..", "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-", "..", "..", "..", ".."
	};
	char pc[16];
	int note = i & 0xF;
	int oct = (i & 0xF0) >> 4;

	sprintf(pc, "%x", oct);
	//ltoa(oct, pc, 16);

	return notes[note] + std::string(pc);
}

inline int midi_to_buzz_note(int value) {
	return ((value / 12) << 4) + (value % 12) + 1;
}

inline int buzz_to_midi_note(int value) {
	return 12 * (value >> 4) + (value & 0xf) - 1;
}

inline int transposeNote(int v, int delta) {
	// 1) convert to "12-base"
	// 2) transpose
	// 3) convert back to "16-base"
	int note = (v & 0xF) - 1;
	int oct = (v & 0xF0) >> 4;
	int midinote = note + (12 * oct);
	midinote += delta;
	note = (midinote % 12) + 1;
	oct = midinote / 12;
	return note + (oct << 4);
}

// ---------------------------------------------------------------------------------------------------------------
// CLIPBOARD
// ---------------------------------------------------------------------------------------------------------------

void CopyBinary(HWND ownerWnd, UINT format, const char* data, int size);
void CopyBinary(HWND ownerWnd, char* formatString, const char* data, int size);
BOOL ClipboardHasFormat(HWND hWnd, LPCTSTR szFormat);
BOOL ClipboardHasAudio(HWND hWnd);

// ---------------------------------------------------------------------------------------------------------------
// WIN32 HELPERS
// ---------------------------------------------------------------------------------------------------------------

/*namespace WTL {
	class CImageList;
};

// for loading IDB_FOOBAR with proper colors
bool CreateImageList(WTL::CImageList &il, UINT nIDResource, int cx, int nGrow, COLORREF crMask);
*/
const CHAR* PeekString(HINSTANCE hinstRes, int idString);

std::string CreateAccelTableString(HACCEL hAccel);

std::string read_file(std::string const& path);

inline bool IsShiftDown() {
	if (GetKeyState(VK_LSHIFT)<0 || GetKeyState(VK_RSHIFT)<0) return true;
	return false;
}

inline bool IsCtrlDown() {
	if (GetKeyState(VK_LCONTROL)<0 || GetKeyState(VK_RCONTROL)<0) return true;
	return false;
}

inline bool IsAltDown() {
	if (GetKeyState(VK_LMENU)<0 || GetKeyState(VK_RMENU)<0) return true;
	return false;
}

inline COLORREF AverageRGB(COLORREF l, COLORREF r, int fact) {
	int out_fact = fact - 1;
	return RGB(
		((GetRValue(l) * out_fact) + GetRValue(r)) / fact,
		((GetGValue(l) * out_fact) + GetGValue(r)) / fact,
		((GetBValue(l) * out_fact) + GetBValue(r)) / fact
	);
}

// inline COLORREF AveragePercentRGB(COLORREF l, COLORREF r, float percentage) {
// 	int Rrange = (GetRValue(r) > GetRValue(l)) ? (GetRValue(r) - GetRValue(l)) : (GetRValue(l) - GetRValue(r));
// 	int Grange = (GetGValue(r) > GetGValue(l)) ? (GetGValue(r) - GetGValue(l)) : (GetGValue(l) - GetGValue(r));
// 	int Brange = (GetBValue(r) > GetBValue(l)) ? (GetBValue(r) - GetBValue(l)) : (GetBValue(l) - GetBValue(r));
// 
// 	float Rcol = (GetRValue(r) > GetRValue(l)) ? (GetRValue(l) + (percentage * Rrange)) : (GetRValue(l) - (percentage * Rrange));
// 	float Gcol = (GetGValue(r) > GetGValue(l)) ? (GetGValue(l) + (percentage * Grange)) : (GetGValue(l) - (percentage * Grange));
// 	float Bcol = (GetBValue(r) > GetBValue(l)) ? (GetBValue(l) + (percentage * Brange)) : (GetBValue(l) - (percentage * Brange));
// 
// 	return RGB(int(Rcol), int(Gcol), int(Bcol));
// }

inline COLORREF AveragePercentRGB(COLORREF l, COLORREF r, float percent) {
	percent = clamp(percent, 0.0f, 1.0f);
	float inv_percent = 1.0f - percent;
	return RGB(
		(GetRValue(l) * inv_percent) + (GetRValue(r) * percent),
		(GetGValue(l) * inv_percent) + (GetGValue(r) * percent),
		(GetBValue(l) * inv_percent) + (GetBValue(r) * percent)
	);
}

inline COLORREF InvertRGB(COLORREF color) {
	return 0xFFFFFF & ~color;
}

// ---------------------------------------------------------------------------------------------------------------
// BUZELIB HELPERS
// ---------------------------------------------------------------------------------------------------------------

void formatTime(char* pc, const char* label, float sec);

std::string GetMixdownFileName(std::string waveFilePath, HWND hWnd);

// ---------------------------------------------------------------------------------------------------------------
// PROFILING
// ---------------------------------------------------------------------------------------------------------------

typedef unsigned __int64 U64;

inline U64 highres_clock() {
	LARGE_INTEGER t;
	if (!::QueryPerformanceCounter(&t))
		return 0;
	return (U64)t.QuadPart;
}

/*
	U64 mark = highres_clock();

	DoStuff();

	U64 done = highres_clock();
	std::cerr << "time: " << (done - mark) << std::endl;
*/


namespace std
{
    #ifdef _UNICODE
        typedef wstring tstring;
        typedef wstringstream tstringstream;
    #else
        typedef string tstring;
        typedef stringstream tstringstream;
    #endif
};
