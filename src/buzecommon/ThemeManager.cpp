#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <map>
#include <vector>
#include <string>
#include "cpputils.h"
#include "ThemeManager.h"
#include "utils.h"
#include "FileReader.h"

using std::map;
using std::vector;
using std::string;
using std::pair;

default_theme_type ThemeManager::defaultTheme[] = {
	{ "MV Amp BG", RGB(0xAA, 0xAA, 0xAA) },
	{ "MV Amp Handle", RGB(0x00, 0x00, 0x00) },
	{ "MV Arrow Volume High", RGB(0xFF, 0x00, 0x00) },
	{ "MV Arrow", RGB(0xC8, 0xC8, 0xC8) },
	{ "MV Arrow Volume Low", RGB(0x00, 0x00, 0x00) },
	{ "MV Background", RGB(0xDA, 0xD6, 0xC9) },
	{ "MV Effect", RGB(0xC7, 0xAD, 0xA9) },
	{ "MV Effect LED Off", RGB(0x64, 0x1E, 0x1E) },
	{ "MV Effect LED On", RGB(0xFF, 0x64, 0x64) },
	{ "MV Effect Mute", RGB(0x9F, 0x8A, 0x87) },
	{ "MV Effect Pan BG", RGB(0x92, 0x65, 0x5F) },
	{ "MV Generator", RGB(0xA9, 0xAE, 0xC7) },
	{ "MV Generator LED Off", RGB(0x28, 0x28, 0x8C) },
	{ "MV Generator LED On", RGB(0x64, 0x64, 0xFF) },
	{ "MV Generator Mute", RGB(0x87, 0x8B, 0x9F) },
	{ "MV Generator Pan BG", RGB(0x5F, 0x67, 0x92) },
	{ "MV Controller", RGB(0xA9, 0xC7, 0xAE) },
	{ "MV Controller LED Off", RGB(0x28, 0x8C, 0x28) },
	{ "MV Controller LED On", RGB(0x64, 0x64, 0xFF) },
	{ "MV Controller Mute", RGB(0x87, 0x9F, 0x8B) },
	{ "MV Container", RGB(0xd7, 0xD7, 0x9E) },
	{ "MV Container LED Off", RGB(0x8C, 0x8C, 0x28) },
	{ "MV Container LED On", RGB(0xFF, 0xFF, 0x64) },
	{ "MV Container Mute", RGB(0x9F, 0x9E, 0x8B) },
	{ "MV Machine Border", RGB(0x00, 0x00, 0x00) },
	{ "MV Machine Text", RGB(0x00, 0x00, 0x00) },
	{ "MV Master", RGB(0xC6, 0xBE, 0xAA) },
	{ "MV Master LED Off", RGB(0x59, 0x59, 0x22) },
	{ "MV Master LED On", RGB(0xE8, 0xE8, 0xC1) },
	{ "MV Pan Handle", RGB(0x00, 0x00, 0x00) },
	{ "MV Line", RGB(0x00, 0x00, 0x00) },
	{ "MV MIDI Line", RGB(0x00, 0x00, 0x80) },
	{ "MV Event Line", RGB(0x00, 0x80, 0x00) },
	{ "PE BG", RGB(0xDA, 0xD6, 0xC9) },
	{ "PE BG Dark", RGB(0xBD, 0xB5, 0x9F) }, 
	{ "PE BG Very Dark", RGB(0xAA, 0x9F, 0x86) }, //RGB(0x9F, 0x93, 0x73) }, 
	{ "PE Selection", RGB(0x00, 0x00, 0xC0) },
	{ "PE Cursor", RGB(0x00, 0x00, 0x00) },
	{ "PE Text Value", RGB(0x00, 0x00, 0x00) }, //RGB(0x30, 0x30, 0x21) }, 
	{ "PE Text Note", RGB(0x26, 0x00, 0x7d) },
	{ "PE Text Note Off", RGB(0x36, 0x0b, 0x8d) },
	{ "PE Text Trigger", RGB(0x50, 0x00, 0x40) },
	{ "PE Text Wave", RGB(0x40, 0x00, 0x30) },
	{ "PE Text Volume", RGB(0x00, 0x65, 0x22) },
	{ "PE Text Shade", RGB(0x89, 0x79, 0x79) },
	{ "PE Text Rows", RGB(0x95, 0x00, 0x00) },
	{ "PE Text Track", RGB(0x95, 0x00, 0x00) },
	{ "PE Text Track Muted", RGB(0xBF, 0xAF, 0xAF) }, //RGB(0xBF, 0x9F, 0x9F) }, 
	{ "PE Text Track Muted BG", RGB(0xCA/2, 0xC6/2, 0xB9/2) }, //RGB(0xCA, 0xC6, 0xB9) }, 
	{ "PE Loop Points", RGB(0x00, 0x00, 0x00) },
	{ "PE Loop Points Disabled", RGB(0x78, 0x78, 0x78) },
	{ "PE Playback Pos", RGB(0xFF, 0x00, 0x00) },
	{ "PE Divider", RGB(0x48, 0x48, 0x48) },
	{ "PE Hidden", RGB(0x48, 0x48, 0x48) },
	{ "PE Control", RGB(0x30, 0x30, 0x21) },
	{ "PE Trigger",  RGB(0xC6, 0xBE, 0xA9) },
	{ "PE Trigger Highlight", RGB(0xE2, 0xDE, 0xD4) },
	{ "PE Trigger Shadow", RGB(0x82, 0x7D, 0x6F) },
	{ "SA Amp BG", RGB(0x70, 0x80, 0x90) },
	{ "SA Amp Line", RGB(0x00, 0xC8, 0x00) },
	{ "SA Freq BG", RGB(0x00, 0x00, 0x00) },
	{ "SA Freq Line", RGB(0x00, 0xC8, 0x00) },
	{ "PE Text Note 1", RGB(0x67, 0x67, 0x00) },
	{ "PE Text Note 2", RGB(0xC0, 0x50, 0x00) },
	{ "PE Text Note 3", RGB(0x00, 0x55, 0x00) },
	{ "PE Text Note 4", RGB(0x50, 0x00, 0x00) },
	{ "PE Text Note 5", RGB(0x80, 0x00, 0x00) },
	{ "PE Text Note 6", RGB(0x00, 0x00, 0x80) },
	{ "PE Text Note 7", RGB(0x44, 0x00, 0x55) },
	{ "PE Text Note 8", RGB(0x80, 0x40, 0x00) },
	{ "PE Text Note 9", RGB(0x00, 0x63, 0x63) },
	{ "PE Text Note 10", RGB(0x60, 0x60, 0x60) },
	{ "PE Text Note 11", RGB(0x50, 0x40, 0x10) },
	{ "PE Text Note 12", RGB(0x00, 0x00, 0x00) },
};

const size_t ThemeManager::defaultThemeCount = array_size(ThemeManager::defaultTheme);

void ThemeManager::initialize() {
	themes.clear();
	themes.push_back("<default>");

	WIN32_FIND_DATA fd;
	string searchPath = "Themes/*.col";
	HANDLE hFind=FindFirstFile(searchPath.c_str(), &fd);
	while (hFind!=INVALID_HANDLE_VALUE) {
		if ( (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0) {
			string name = fd.cFileName;
			size_t ld = name.find_last_of('.');
			if (ld == string::npos) continue;
			//string ext = name.substr(ld+1);
			name = name.substr(0, ld);

			themes.push_back(name);
		}
		if (!FindNextFile(hFind, &fd)) break;
	}
	FindClose(hFind);
}

size_t ThemeManager::getThemes() {
	return themes.size();
}

std::string ThemeManager::getThemeName(size_t index) {
	return themes[index];
}

void ThemeManager::setDefaultOverrideColor(std::string const& dest, std::string const& src) {
	map<string, COLORREF>::iterator i;

	if (theme.find(dest) != theme.end())
		return;

	i = theme.find(src);

	if (i != theme.end())
		theme.insert(pair<string, COLORREF>(dest, i->second));
}

bool ThemeManager::loadTheme(std::string const& themeName) {
	currentTheme = themeName;
	theme.clear();

	FileReader reader;
	string fileName = "Themes/" + themeName + ".col";
	if (reader.open(fileName.c_str())) {

		while (!reader.eof()) {
			std::string line=trim(reader.readLine());
			if (line.length()==0) continue;
			if (line.at(0)=='#') continue;
			
			size_t lt = line.find_first_of('\t');
			if (lt == string::npos) continue;

			string name = trim(line.substr(0, lt));
			string colorstr = trim(line.substr(lt+1));
			if (colorstr.length() != 6) continue;

			COLORREF color = RGB(
				intFromHex(colorstr.substr(0, 2)),
				intFromHex(colorstr.substr(2, 2)),
				intFromHex(colorstr.substr(4, 2))
			);
			theme[name] = color;
		}
		reader.close();
	}

//#if 0 // Uncomment this so that legacy buzz themes look their best with buze's new theme values

	// Theme-based defaults
	setDefaultOverrideColor("PE Text Value",	"PE Text");
	setDefaultOverrideColor("PE Text Note",		"PE Text");
	setDefaultOverrideColor("PE Text Note Off",	"PE Text");
	setDefaultOverrideColor("PE Text Trigger",	"PE Text");
	setDefaultOverrideColor("PE Text Wave",		"PE Text");
	setDefaultOverrideColor("PE Text Volume",	"PE Text");
	setDefaultOverrideColor("PE Text Shade",	"PE Text");
	setDefaultOverrideColor("PE Text Rows",		"PE Text Note");
	setDefaultOverrideColor("PE Text Headers",	"PE Text Note");
	setDefaultOverrideColor("PE Divider",		"PE Text");
	setDefaultOverrideColor("PE Control",		"PE Text");

//#endif

	// Defaults

	for (unsigned int i = 0; i< defaultThemeCount; i++) {
		if (theme.find(defaultTheme[i].name) == theme.end())
			theme.insert(pair<string, COLORREF>(defaultTheme[i].name, defaultTheme[i].color));
	}

	return true;
}

bool ThemeManager::saveTheme(std::string const& themeName) {
	if (themeName == "<default>") return false;

	FileWriter writer;
	string fileName = "Themes/" + themeName + ".col";
	if (!writer.create(fileName.c_str())) return false;

	currentTheme = themeName;

	for (std::map<std::string, COLORREF>::iterator i = theme.begin(); i != theme.end(); ++i) {
		std::stringstream strm;
		strm << i->first << "\t"
			<< hexFromInt(GetRValue(i->second), 2, '0')
			<< hexFromInt(GetGValue(i->second), 2, '0')
			<< hexFromInt(GetBValue(i->second), 2, '0');
		writer.writeLine(strm.str());
	}

	writer.close();

	return true;
}

COLORREF ThemeManager::getThemeColor(std::string const& name) {
	map<string, COLORREF>::iterator i = theme.find(name);
	if ( i == theme.end()) return 0;
	return i->second;
}

bool ThemeManager::setThemeColor(std::string const& name, COLORREF value) {
	map<string, COLORREF>::iterator i = theme.find(name);
	if ( i == theme.end()) return false;
	i->second = value;
	return true;
}
