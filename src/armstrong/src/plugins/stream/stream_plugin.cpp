/*

TODO:

things we can do with a semi-native wave-player (ui-wise):

	- preview waves in wave table
	- preview waves on disk
	- set preview volume
	- play notes in wavetable
	- play selected range in wave editor
	- play back frozen machine audio
	- play mp3s and oggs and streams in songs (use new zzub api)
	- preview mp3s, oggs in wavetable and filebrowser
	- sync long waves with song position
	- play ranges of long waves synced with song position

*/

#if defined(_WIN32)
#include <windows.h>
#endif

#include <zzub/signature.h>

#include "stream_plugin.h"

int buzz_to_midi_note(int note) {
	return 12 * (note >> 4) + (note & 0xf) - 1;
}

const char* get_open_filename(const char* fileName, const char* filter) {

	static char szFile[260];       // buffer for file name
	strcpy(szFile, fileName);

#if defined(_WIN32)
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.lpstrDefExt="";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_NOCHANGEDIR |OFN_OVERWRITEPROMPT|OFN_EXTENSIONDIFFERENT|OFN_NOREADONLYRETURN;

	if (::GetOpenFileName(&ofn)==TRUE) {
		return szFile;
	}
#else
	printf("get_open_filename not implemented!");
#endif
	return 0;
}

float lognote(int freq) {
	return (logf(freq) - logf(440.0)) / logf(2.0) + 4.0;
}
