#define NOMINMAX
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <buze/buzesdk.h>
#include "Configuration.h"
#include "BuzeConfiguration.h"
#include "utils.h"

using namespace std;

static const char* REGISTRY_CONFIG_KEY = "Software\\zzub\\buze\\";

CBuzeConfiguration::CBuzeConfiguration() {
	configuration = 0;
}

CBuzeConfiguration::CBuzeConfiguration(buze_configuration_t* _configuration) {
	configuration = _configuration;
}

CBuzeConfiguration::~CBuzeConfiguration() {
}

void CBuzeConfiguration::operator=(buze_configuration_t* _configuration) {
	configuration = _configuration;
}

bool CBuzeConfiguration::getString(const char* section, const char* key, char* value) {
	return configuration->getConfigString(section, key, value);
}

bool CBuzeConfiguration::getNumber(const char* section, const char* key, DWORD* value) {
	return configuration->getConfigNumber(section, key, value);
}

bool CBuzeConfiguration::setString(const char* section, const char* key, const char* value) {
	return configuration->setConfigString(section, key, value);
}

bool CBuzeConfiguration::setNumber(const char* section, const char* key, int value) {
	return configuration->setConfigNumber(section, key, value);
}

bool CBuzeConfiguration::getAudioDriver(std::string& outDevice, std::string& inDevice, int& rate, int& bufferSize, int& outChannel) {
	char temp[1024];
	if (!getString("AudioDevice", "Name", temp))
		outDevice = "";
	else
		outDevice = temp;

	if (!getString("AudioDevice", "InName", temp))
		inDevice = "";
	else
		inDevice = temp;

	if (!getNumber("AudioDevice", "SampleRate", (DWORD*)&rate))
		rate = 44100;

	if (!getNumber("AudioDevice", "BufferSize", (DWORD*)&bufferSize))
		bufferSize = 512;

	if (!getNumber("Settings", "MasterOutChannel", (DWORD*)&outChannel))
		outChannel = 0;

	return true;
}

bool CBuzeConfiguration::setAudioDriver(std::string outDevice, std::string inDevice, int rate, int bufferSize, int outChannel) {
	if (!setString("AudioDevice", "Name", outDevice.c_str()))
		return false;

	if (!setString("AudioDevice", "InName", inDevice.c_str()))
		return false;

	if (!setNumber("AudioDevice", "SampleRate", rate))
		return false;

	if (!setNumber("AudioDevice", "BufferSize", bufferSize))
		return false;

	if (!setNumber("Settings", "MasterOutChannel", outChannel))
		return false;

	return true;
}

bool CBuzeConfiguration::getMidiInputs(std::vector<std::string>& names) {
	string midiInConfig = "";
	size_t pos = 0;
	char temp[1024];
	if (getString("Settings", "BuzeMidiInDevs", temp)) {
		midiInConfig = temp;
		while (pos < midiInConfig.size()) {
			string::size_type loc = midiInConfig.find( ',', pos );
			if (loc == string::npos)
				loc = midiInConfig.size();
			string devstr = midiInConfig.substr(pos, loc-pos);
			pos = loc+1;

			names.push_back(devstr);
		}
	} else
		return false;
	return true;
}

bool CBuzeConfiguration::setMidiInputs(std::vector<std::string> names) {
	string midiConfig = "";
	for (size_t i = 0; i < names.size(); i++) {
		midiConfig += names[i];
		if (i < (names.size() - 1))
			midiConfig += ",";
	}
	if (!setString("Settings", "BuzeMidiInDevs", midiConfig.c_str())) 
		return false;
	return true;
}

bool CBuzeConfiguration::getMidiOutputs(std::vector<std::string>& names) {
	string midiOutConfig = "";
	size_t pos = 0;
	char temp[1024];
	if (getString("Settings", "BuzeMidiOutDevs", temp)) {
		midiOutConfig = temp;
		while (pos < midiOutConfig.size()) {
			string::size_type loc = midiOutConfig.find( ',', pos );
			if (loc == string::npos)
				loc = midiOutConfig.size();
			string devstr = midiOutConfig.substr(pos, loc-pos);
			pos = loc+1;

			names.push_back(devstr);
		}
	}
	return true;
}

bool CBuzeConfiguration::setMidiOutputs(std::vector<std::string> names) {
	string midiConfig = "";
	for (size_t i=0; i<names.size(); i++) {
		midiConfig += names[i];
		if (i < (names.size() - 1))
			midiConfig += ",";
	}
	if (!setString("Settings", "BuzeMidiOutDevs", midiConfig.c_str())) 
		return false;
	return true;
}

int CBuzeConfiguration::getMixerThreads() {
	DWORD count = 1;
	getNumber("Settings", "MixerThreadCount", &count);
	return std::max(1UL, std::min(100UL, count));
}

void CBuzeConfiguration::setMixerThreads(int count) {
	count = std::max(1, std::min(100, count));
	setNumber("Settings", "MixerThreadCount", count);
}

bool CBuzeConfiguration::getParameterPopupFlag() {
	DWORD dw = 1;
	getNumber("Settings", "BuzePopupMachines", &dw);
	return dw != 0;
}

void CBuzeConfiguration::setParameterPopupFlag(bool flag) {
	setNumber("Settings", "BuzePopupMachines", (DWORD)flag);
}

std::string CBuzeConfiguration::getTheme() {
	char theme[1024];
	strcpy(theme, "<default>");
	getString("Settings", "Theme", theme);
	return theme;
}

void CBuzeConfiguration::setTheme(std::string theme) {
	setString("Settings", "Theme", theme.c_str());
}

std::string CBuzeConfiguration::getRecentSong(int index) {
	std::stringstream key;
	key << "RecentSong" + stringFromInt(index);
	char recent[1024];
	if (!getString("Settings", key.str().c_str(), recent))
		return "";
	return recent;
}

void CBuzeConfiguration::insertRecentSong(std::string fileName) {
	std::vector<std::string> recs;
	int i;

	for (i = 0; i < getMaxRecentSongs(); ++i) {
		std::stringstream key;
		key << "RecentSong" + stringFromInt(i);
		char str[1024] = {0};
		if (!getString("Settings", key.str().c_str(), str))
			break;
		if (str == fileName) {
			if(!i)
				return;
			continue;
		}
		if (strlen(str) > 0)
			recs.push_back(str);
	}

	recs.insert(recs.begin(), fileName);

	// resave all recent songs

	for(i = 0; i < getMaxRecentSongs() && i < (int)recs.size(); ++i) {
		std::stringstream key;
		key << "RecentSong" + stringFromInt(i);
		setString("Settings", key.str().c_str(), recs[i].c_str());
	}
}

int CBuzeConfiguration::getMaxRecentSongs() {
	DWORD dw = 10;
	getNumber("Settings", "MaxRecentSongs", &dw);
	if(dw > 100)
		dw = 100;
	return dw;
}

void CBuzeConfiguration::setMaxRecentSongs(int max) {
	setNumber("Settings", "MaxRecentSongs", max);
}

int CBuzeConfiguration::getDefaultAmp() {
	DWORD dw = 0x4000;
	getNumber("Settings", "DefaultAmp", &dw);
	if(dw > 0x4000)
		dw = 0x4000;
	return dw;
}

void CBuzeConfiguration::setDefaultAmp(int amp) {
	setNumber("Settings", "DefaultAmp", amp);
}


std::string CBuzeConfiguration::getExternalWaveEditor() {
	char cmd[1024];
	if (!getString("Settings", "ExternalWaveEditor", cmd))
		return "";
	return cmd;
}

void CBuzeConfiguration::setExternalWaveEditor(std::string cmd) {
	setString("Settings", "ExternalWaveEditor", cmd.c_str());
}

void CBuzeConfiguration::setMachineParameterVisibility(std::string uri, int group, int column, bool state) {
	std::stringstream strm;
	strm << "Machines\\" << uri << "\\ExtMachineInfo\\Parameter";
	strm << group << "-" << column;
	setNumber(strm.str().c_str(), "Hidden", state?1:0);
}

bool CBuzeConfiguration::getMachineParameterVisibility(std::string uri, int group, int column) {
	std::stringstream strm;
	strm << "Machines\\" << uri << "\\ExtMachineInfo\\Parameter";
	strm << group << "-" << column;
	DWORD dw = 0;
	
	getNumber(strm.str().c_str(), "Hidden", &dw);
	return dw != 0;
}

int CBuzeConfiguration::getNumSamplePaths() {
	DWORD count;
	if (!getNumber("Settings", "SamplePaths", &count))
		count = 0;
	return count;
}

std::string CBuzeConfiguration::getSamplePath(int index) {
	std::stringstream key;
	key << "SamplePath" + stringFromInt(index);
	char path[1024];
	if (!getString("Settings", key.str().c_str(), path))
		return "";
	return path;
}

void CBuzeConfiguration::removeSamplePath(int index) {
	int count = getNumSamplePaths();

	for (int i = index; i < count-1; i++) {
		string path = getSamplePath(i+1);
		std::stringstream key;
		key << "SamplePath" + stringFromInt(i);
		setString("Settings", key.str().c_str(), path.c_str());
	}

	setNumber("Settings", "SamplePaths", count-1);
}

void CBuzeConfiguration::addSamplePath(std::string path) {
	int count = getNumSamplePaths();
	std::stringstream key;
	key << "SamplePath" + stringFromInt(count);
	if (!setString("Settings", key.str().c_str(), path.c_str()))
		return;
	setNumber("Settings", "SamplePaths", count+1);
}

void CBuzeConfiguration::setMachineMidiInDisabled(std::string uri, bool state) {
	std::stringstream section;
	section << "Machines\\" << uri << "\\ExtMachineInfo";
	setNumber(section.str().c_str(), "DisableMidiIn", state?1:0);
}

bool CBuzeConfiguration::getMachineMidiInDisabled(std::string uri) {
	std::stringstream section;
	section << "Machines\\" << uri << "\\ExtMachineInfo";
	DWORD dw = 0;
	getNumber(section.str().c_str(), "DisableMidiIn", &dw);
	return dw != 0;
}

void CBuzeConfiguration::setStickySelections(bool state) {
	setNumber("Settings", "StickySelections", state?1:0);
}

bool CBuzeConfiguration::getStickySelections() {
	DWORD dw = 0;
	getNumber("Settings", "StickySelections", &dw);
	return dw != 0;
}

void CBuzeConfiguration::setColoredNotes(bool state) {
	setNumber("Settings", "ColoredNotes", state?1:0);
}

bool CBuzeConfiguration::getColoredNotes() {
	DWORD dw = 0;
	getNumber("Settings", "ColoredNotes", &dw);
	return dw != 0;
}

void CBuzeConfiguration::setShowAccelerators(bool state) {
	setNumber("Settings", "ShowAccelerators", state?1:0);
}

bool CBuzeConfiguration::getShowAccelerators() {
	DWORD dw = 1;
	getNumber("Settings", "ShowAccelerators", &dw);
	return dw != 0;
}

void CBuzeConfiguration::setOrderlistEnabled(bool state) {
	setNumber("Settings", "OrderlistEnabled", state?1:0);
}

bool CBuzeConfiguration::getOrderlistEnabled() {
	DWORD dw = 1;
	getNumber("Settings", "OrderlistEnabled", &dw);
	return dw != 0;
}

void CBuzeConfiguration::setGlobalPatternLength(int length) {
	setNumber("Settings", "PatternLength", length);
}

int CBuzeConfiguration::getGlobalPatternLength() {
	DWORD dw = 64;
	getNumber("Settings", "PatternLength", &dw);
	return dw;
}

void CBuzeConfiguration::setNotesAffectMode(int mode) {
	setNumber("Settings", "NotesAffectMode", mode);
}

int CBuzeConfiguration::getNotesAffectMode() {
	DWORD dw = 3;
	getNumber("Settings", "NotesAffectMode", &dw);
	return dw;
}

void CBuzeConfiguration::setPatternFollowMode(int mode) {
	setNumber("Settings", "PatternFollowMode", mode);
}

int CBuzeConfiguration::getPatternFollowMode() {
	DWORD dw = 0;
	getNumber("Settings", "PatternFollowMode", &dw);
	return dw;
}

void CBuzeConfiguration::setMachinePatternLength(std::string uri, int length) {
	std::stringstream section;
	section << "Machines\\" << uri << "\\ExtMachineInfo";
	setNumber(section.str().c_str(), "PatternLength", length);
}

int CBuzeConfiguration::getMachinePatternLength(std::string uri) {
	std::stringstream section;
	section << "Machines\\" << uri << "\\ExtMachineInfo";
	DWORD dw = getGlobalPatternLength();
	getNumber(section.str().c_str(), "PatternLength", &dw);
	return dw;
}

void CBuzeConfiguration::setMachineDefaultTracks(std::string uri, int length) {
	std::stringstream section;
	section << "Machines\\" << uri << "\\ExtMachineInfo";
	setNumber(section.str().c_str(), "DefaultTracks", length);
}

int CBuzeConfiguration::getMachineDefaultTracks(std::string uri, int deftracks) {
	std::stringstream section;
	section << "Machines\\" << uri << "\\ExtMachineInfo";
	DWORD dw = deftracks;
	getNumber(section.str().c_str(), "DefaultTracks", &dw);
	return dw;
}

void CBuzeConfiguration::setMachineSkinVisibility(bool state) {
	setNumber("Settings", "MachineSkins", state?1:0);
}

bool CBuzeConfiguration::getMachineSkinVisibility() {
	DWORD dw = 0;
	getNumber("Settings", "MachineSkins", &dw);
	return dw?true:false;
}

void CBuzeConfiguration::setScaleByWindowSize(bool state) {
	setNumber("Settings", "ScaleByWindowSize", state?1:0);
}

bool CBuzeConfiguration::getScaleByWindowSize() {
	DWORD dw = 1;
	getNumber("Settings", "ScaleByWindowSize", &dw);
	return dw?true:false;
}

void CBuzeConfiguration::setMachinesMinimized(bool state) {
	setNumber("Settings", "MachinesMinimized", state?1:0);
}

bool CBuzeConfiguration::getMachinesMinimized() {
	DWORD dw = 0;
	getNumber("Settings", "MachinesMinimized", &dw);
	return dw?true:false;
}

void CBuzeConfiguration::setMachineScale(float sc) {
	std::string str;
	char pc[10];
	sprintf(pc, "%.6f", sc);
	str = pc;
	setString("Settings", "MachineScale", str.c_str());
}

float CBuzeConfiguration::getMachineScale() {
	char str[1024];
	if (!getString("Settings", "MachineScale", str))
		return 1.0;
	return (float)atof(str);
}

std::string CBuzeConfiguration::getNoteOffString() {
	char s[1024];
	if (!getString("Settings", "NoteOffString", s))
		return "off";

	if (strlen(s) == 0)
		return "off";

	if (strlen(s) > 3)
		s[3] = 0;
	return s;
}

void CBuzeConfiguration::setNoteOffString(std::string s) {
	setString("Settings", "NoteOffString", s.c_str());
}

std::string CBuzeConfiguration::getNoteCutString() {
	char s[1024];
	if (!getString("Settings", "NoteCutString", s))
		return "^^^";

	if (strlen(s) == 0)
		return "^^^";

	if (strlen(s) > 3)
		s[3] = 0;
	return s;
}

void CBuzeConfiguration::setNoteCutString(std::string s) {
	setString("Settings", "NoteCutString", s.c_str());
}

std::string CBuzeConfiguration::getBGNote() {
	char s[1024];
	if (!getString("Settings", "BGNote", s))
		return "---";

	std::string str = s;
	if (str.size() != 3)
		str.resize(3, '.');
	return str;
}

void CBuzeConfiguration::setBGNote(std::string s) {
	setString("Settings", "BGNote", s.c_str());
}

std::string CBuzeConfiguration::getBGByte() {
	char s[1024];
	if (!getString("Settings", "BGByte", s))
		return "..";

	std::string str = s;
	if (str.size() != 2)
		str.resize(2, '.');
	return str;
}

void CBuzeConfiguration::setBGByte(std::string s) {
	setString("Settings", "BGByte", s.c_str());
}

std::string CBuzeConfiguration::getBGSwitch() {
	char s[1024];
	if (!getString("Settings", "BGSwitch", s))
		return ".";

	std::string str = s;
	if (str.size() != 1)
		str.resize(1, '.');
	return str;
}

void CBuzeConfiguration::setBGSwitch(std::string s) {
	setString("Settings", "BGSwitch", s.c_str());
}

std::string CBuzeConfiguration::getBGWord() {
	char s[1024];
	if (!getString("Settings", "BGWord", s))
		return "....";

	std::string str = s;
	if (str.size() != 4)
		str.resize(4, '.');
	return str;
}

void CBuzeConfiguration::setBGWord(std::string s) {
	setString("Settings", "BGWord", s.c_str());
}

void CBuzeConfiguration::setVUDropSpeed(float sc) {
	char pc[10];
	sprintf(pc, "%.6f", sc);
	setString("Settings", "VUDropSpeed", pc);
}

float CBuzeConfiguration::getVUDropSpeed() {
	char str[1024];
	if (!getString("Settings", "VUDropSpeed", str))
		return 0.0;
	return (float)atof(str);
}

void CBuzeConfiguration::setVUTimerSpeed(int vv) {
	setNumber("Settings", "VUTimerSpeed", vv);
}

int CBuzeConfiguration::getVUTimerSpeed() {
	DWORD dw = 2;
	getNumber("Settings", "VUTimerSpeed", &dw);
	return dw;
}

void CBuzeConfiguration::setDefaultEntryMode(int i) {
	setNumber("Settings", "DefaultEntryMode", i);
}

int CBuzeConfiguration::getDefaultEntryMode() {
	DWORD dw = 1; // horizontal -- we're being rebellious ;)
	getNumber("Settings", "DefaultEntryMode", &dw);
	return dw;
}

void CBuzeConfiguration::setHorizontalScrollMode(int i) {
	setNumber("Settings", "HorizontalScrollMode", i);
}

int CBuzeConfiguration::getHorizontalScrollMode() {
	DWORD dw = 0;
	getNumber("Settings", "HorizontalScrollMode", &dw);
	return dw;
}

void CBuzeConfiguration::setVerticalScrollMode(int i) {
	setNumber("Settings", "VerticalScrollMode", i);
}

int CBuzeConfiguration::getVerticalScrollMode() {
	DWORD dw = 0;
	getNumber("Settings", "VerticalScrollMode", &dw);
	return dw;
}

void CBuzeConfiguration::setFormatPatternCreationMode(int i) {
	setNumber("Settings", "FormatPatternCreationMode", i);
}

int CBuzeConfiguration::getFormatPatternCreationMode() {
	DWORD dw = 0;
	getNumber("Settings", "FormatPatternCreationMode", &dw);
	return dw;
}

void CBuzeConfiguration::setPatternNamingMode(int i) {
	setNumber("Settings", "PatternNamingMode", i);
}

int CBuzeConfiguration::getPatternNamingMode() {
	DWORD dw = 0;
	getNumber("Settings", "PatternNamingMode", &dw);
	return dw;
}

void CBuzeConfiguration::setPatternRightClickMode(int i) {
	setNumber("Settings", "PatternRightClickMode", i);
}

int CBuzeConfiguration::getPatternRightClickMode() {
	DWORD dw = 0;
	getNumber("Settings", "PatternRightClickMode", &dw);
	return dw;
}

void CBuzeConfiguration::setSubrowNamingMode(int i) {
	setNumber("Settings", "SubrowNamingMode", i);
}

int CBuzeConfiguration::getSubrowNamingMode() {
	DWORD dw = 1;
	getNumber("Settings", "SubrowNamingMode", &dw);
	return dw;
}

void CBuzeConfiguration::setTriggerWidth(int x) {
	setNumber("Settings", "TriggerWidth", x);
}

int CBuzeConfiguration::getTriggerWidth() {
	DWORD dw = 8;
	getNumber("Settings", "TriggerWidth", &dw);
	return dw;
}

void CBuzeConfiguration::setDefaultScrollerWidth(int x) {
	setNumber("Settings", "DefaultScrollerWidth", x);
}

int CBuzeConfiguration::getDefaultScrollerWidth() {
	DWORD dw = 128;
	getNumber("Settings", "DefaultScrollerWidth", &dw);
	return dw;
}

void CBuzeConfiguration::setShowInfoPane(int x) {
	setNumber("Settings", "ShowInfoPane", x);
}

int CBuzeConfiguration::getShowInfoPane() {
	DWORD dw = 1;
	getNumber("Settings", "ShowInfoPane", &dw);
	return dw;
}

bool CBuzeConfiguration::getToolbarVisible(int id, bool defValue) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = defValue;
	getNumber(key, "Visible", &dw);
	return dw?true:false;
}

void CBuzeConfiguration::setToolbarVisible(int id, bool state) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = state?1:0;
	setNumber(key, "Visible", dw);
}

int CBuzeConfiguration::getToolbarWidth(int id, int defValue) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);

	DWORD dw = defValue;
	getNumber(key, "Width", &dw);
	return dw;
}

void CBuzeConfiguration::setToolbarWidth(int id, int width) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = width;
	setNumber(key, "Width", dw);
}

bool CBuzeConfiguration::getToolbarBreak(int id, bool defValue) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = defValue;
	getNumber(key, "Break", &dw);
	return dw?true:false;
}

void CBuzeConfiguration::setToolbarBreak(int id, bool state) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = state?1:0;
	setNumber(key, "Break", dw);
}

int CBuzeConfiguration::getToolbarPosition(int id, int defValue) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = defValue;
	getNumber(key, "Position", &dw);
	return dw;
}

void CBuzeConfiguration::setToolbarPosition(int id, int position) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = position;
	setNumber(key, "Position", dw);
}

void CBuzeConfiguration::setLockedToolbars(bool state) {
	setNumber("Settings", "LockedToolbars", (int)state);
}

bool CBuzeConfiguration::getLockedToolbars() {
	DWORD dw = 0;
	getNumber("Settings", "LockedToolbars", &dw);
	return dw?true:false;
}

void CBuzeConfiguration::setPatternStackMode(int mode) {
	setNumber("Settings", "PatternStackMode", mode);
}

int CBuzeConfiguration::getPatternStackMode() {
	DWORD dw = 0;
	getNumber("Settings", "PatternStackMode", &dw);
	return (int)dw;
}

void CBuzeConfiguration::setVstPaths(std::string s) {
	setString("Settings", "VstPaths", s.c_str());
}

std::string CBuzeConfiguration::getVstPaths() {
	char result[4096];
	if (!getString("Settings", "VstPaths", result))
		return "";
	return result;
}

void CBuzeConfiguration::setAdvancedDefaultDocument(bool state) {
	setNumber("Settings", "AdvancedDefaultDocument", (int)state);
}

bool CBuzeConfiguration::getAdvancedDefaultDocument() {
	DWORD dw = 0;
	getNumber("Settings", "AdvancedDefaultDocument", &dw);
	return dw?true:false;
}

void CBuzeConfiguration::setPatternPositionCachePatternFormat(bool state) {
	setNumber("Settings", "PatternPositionCachePatternFormat", (int)state);
}

bool CBuzeConfiguration::getPatternPositionCachePatternFormat() {
	DWORD dw = 0;
	getNumber("Settings", "PatternPositionCachePatternFormat", &dw);
	return dw?true:false;
}
