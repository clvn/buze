#include "stdafx.h"
#include "BuzeConfiguration.h"
#include "Utils/utils.h"

using namespace std;

static const char* REGISTRY_CONFIG_KEY = "Software\\zzub\\buze\\";

CConfiguration::CConfiguration() {
}

CConfiguration::~CConfiguration() {
}

bool CConfiguration::getConfigString(std::string section, std::string key, std::string* value) {
	CRegKey reg;

	string regKey = (string)REGISTRY_CONFIG_KEY + (string)section;
	LONG err = reg.Open(HKEY_CURRENT_USER, regKey.c_str(), KEY_READ);
	if (err != ERROR_SUCCESS) return false;

	DWORD len = 1024;
	char pc[1024];

	err = reg.QueryStringValue(key.c_str(), pc, &len);
	if (err != ERROR_SUCCESS) return false;

	if (value)
		*value = pc;
	return true;
}

bool CConfiguration::getConfigNumber(std::string section, std::string key, DWORD* value) {
	CRegKey reg;

	string regKey = (string)REGISTRY_CONFIG_KEY + section;
	LONG err = reg.Open(HKEY_CURRENT_USER, regKey.c_str(), KEY_READ);
	if (err != ERROR_SUCCESS) return false;

	err = reg.QueryDWORDValue(key.c_str(), *value);
	if (err != ERROR_SUCCESS) return false;

	return true;
}

bool CConfiguration::setConfigString(std::string section, std::string key, std::string value) {
	CRegKey reg;

	string regKey = (string)REGISTRY_CONFIG_KEY + section;
	LONG err = reg.Create(HKEY_CURRENT_USER, regKey.c_str());
	if (err != ERROR_SUCCESS) return false;

	err = reg.SetStringValue(key.c_str(), value.c_str());
	if (err != ERROR_SUCCESS) return false;

	return true;
}

bool CConfiguration::setConfigNumber(std::string section, std::string key, int value) {
	CRegKey reg;

	string regKey = (string)REGISTRY_CONFIG_KEY + (string)section;
	LONG err = reg.Create(HKEY_CURRENT_USER, regKey.c_str());
	if (err != ERROR_SUCCESS) return false;

	err = reg.SetDWORDValue(key.c_str(), value);
	if (err != ERROR_SUCCESS) return false;

	return true;
}

bool CConfiguration::getAudioDriver(std::string& outDevice, std::string& inDevice, int& rate, int& bufferSize, int& outChannel) {
	if (!getConfigString("AudioDevice", "Name", &outDevice))
		outDevice = "";

	if (!getConfigString("AudioDevice", "InName", &inDevice))
		inDevice = "";

	if (!getConfigNumber("AudioDevice", "SampleRate", (DWORD*)&rate))
		rate = 44100;

	if (!getConfigNumber("AudioDevice", "BufferSize", (DWORD*)&bufferSize))
		bufferSize = 512;

	if (!getConfigNumber("Settings", "MasterOutChannel", (DWORD*)&outChannel))
		outChannel = 0;

	return true;
}

bool CConfiguration::setAudioDriver(std::string outDevice, std::string inDevice, int rate, int bufferSize, int outChannel) {
	if (!setConfigString("AudioDevice", "Name", outDevice))
		return false;

	if (!setConfigString("AudioDevice", "InName", inDevice))
		return false;

	if (!setConfigNumber("AudioDevice", "SampleRate", rate))
		return false;

	if (!setConfigNumber("AudioDevice", "BufferSize", bufferSize))
		return false;

	if (!setConfigNumber("Settings", "MasterOutChannel", outChannel))
		return false;

	return true;
}

bool CConfiguration::getMidiInputs(std::vector<std::string>& names) {
	string midiInConfig = "";
	size_t pos = 0;
	if (getConfigString("Settings", "BuzeMidiInDevs", &midiInConfig)) {
		
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

bool CConfiguration::setMidiInputs(std::vector<std::string> names) {
	string midiConfig = "";
	for (size_t i=0; i<names.size(); i++) {
		midiConfig += names[i];
		if (i < (names.size() - 1))
			midiConfig += ",";
	}
	if (!setConfigString("Settings", "BuzeMidiInDevs", midiConfig)) 
		return false;
	return true;
}

bool CConfiguration::getMidiOutputs(std::vector<std::string>& names) {
	string midiOutConfig = "";
	size_t pos = 0;
	if (getConfigString("Settings", "BuzeMidiOutDevs", &midiOutConfig)) {
		
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

bool CConfiguration::setMidiOutputs(std::vector<std::string> names) {
	string midiConfig = "";
	for (size_t i=0; i<names.size(); i++) {
		midiConfig += names[i];
		if (i < (names.size() - 1))
			midiConfig += ",";
	}
	if (!setConfigString("Settings", "BuzeMidiOutDevs", midiConfig)) 
		return false;
	return true;
}

int CConfiguration::getMixerThreads() {
	DWORD count = 1;
	getConfigNumber("Settings", "MixerThreadCount", &count);
	return std::max(1UL, std::min(100UL, count));
}

void CConfiguration::setMixerThreads(int count) {
	count = std::max(1, std::min(100, count));
	setConfigNumber("Settings", "MixerThreadCount", count);
}

bool CConfiguration::getParameterPopupFlag() {
	DWORD dw = 1;
	getConfigNumber("Settings", "BuzePopupMachines", &dw);
	return dw != 0;
}

void CConfiguration::setParameterPopupFlag(bool flag) {
	setConfigNumber("Settings", "BuzePopupMachines", (DWORD)flag);
}

std::string CConfiguration::getTheme() {
	string theme = "<default>";
	getConfigString("Settings", "Theme", &theme);
	return theme;
}

void CConfiguration::setTheme(std::string theme) {
	setConfigString("Settings", "Theme", theme);
}

std::string CConfiguration::getRecentSong(int index) {
	string recent;

	if (!getConfigString("Settings", "RecentSong" + stringFromInt(index), &recent))
		return "";
	return recent;
}

void CConfiguration::insertRecentSong(std::string fileName) {
	std::vector<std::string> recs;
	std::string str;
	int i;

	for (i = 0; i < getMaxRecentSongs(); ++i) {
		str.clear();
		getConfigString("Settings", "RecentSong" + stringFromInt(i), &(str));
		if (str == fileName) {
			if(!i)
				return;
			continue;
		}
		if (str.size())
			recs.push_back(str);
	}

	recs.insert(recs.begin(), fileName);

	// resave all recent songs

	for(i = 0; i < getMaxRecentSongs() && i < (int)recs.size(); ++i) {
		setConfigString("Settings", "RecentSong" + stringFromInt(i), recs[i]);
	}
}

int CConfiguration::getMaxRecentSongs() {
	DWORD dw = 10;
	getConfigNumber("Settings", "MaxRecentSongs", &dw);
	if(dw > 100)
		dw = 100;
	return dw;
}

void CConfiguration::setMaxRecentSongs(int max) {
	setConfigNumber("Settings", "MaxRecentSongs", max);
}

int CConfiguration::getDefaultAmp() {
	DWORD dw = 0x4000;
	getConfigNumber("Settings", "DefaultAmp", &dw);
	if(dw > 0x4000)
		dw = 0x4000;
	return dw;
}

void CConfiguration::setDefaultAmp(int amp) {
	setConfigNumber("Settings", "DefaultAmp", amp);
}


std::string CConfiguration::getExternalWaveEditor() {
	string cmd;
	if (!getConfigString("Settings", "ExternalWaveEditor", &cmd))
		return "";
	return cmd;
}

void CConfiguration::setExternalWaveEditor(std::string cmd) {
	setConfigString("Settings", "ExternalWaveEditor", cmd);
}

void CConfiguration::setMachineParameterVisibility(std::string uri, int group, int column, bool state) {
	std::stringstream strm;
	strm << group << "-" << column;
	setConfigNumber("Machines\\" + (string)uri + "\\ExtMachineInfo\\Parameter" + strm.str(), "Hidden", state?1:0);
}

bool CConfiguration::getMachineParameterVisibility(std::string uri, int group, int column) {
	std::stringstream strm;
	strm << group << "-" << column;
	DWORD dw = 0;
	getConfigNumber("Machines\\" + (string)uri + "\\ExtMachineInfo\\Parameter" + strm.str(), "Hidden", &dw);
	return dw != 0;
}

int CConfiguration::getNumSamplePaths() {
	DWORD count;
	if (!getConfigNumber("Settings", "SamplePaths", &count))
		count = 0;
	return count;
}

std::string CConfiguration::getSamplePath(int index) {
	std::string path;
	if (!getConfigString("Settings", "SamplePath" + stringFromInt(index), &path))
		path = "";
	return path;
}

void CConfiguration::removeSamplePath(int index) {
	int count = getNumSamplePaths();

	for (int i = index; i < count-1; i++) {
		string path = getSamplePath(i+1);
		setConfigString("Settings", "SamplePath" + stringFromInt(i), path);
	}

	setConfigNumber("Settings", "SamplePaths", count-1);
}

void CConfiguration::addSamplePath(std::string path) {
	int count = getNumSamplePaths();
	if (!setConfigString("Settings", "SamplePath" + stringFromInt(count), path))
		return;
	setConfigNumber("Settings", "SamplePaths", count+1);
}

void CConfiguration::setMachineMidiInDisabled(std::string uri, bool state) {
	setConfigNumber("Machines\\" + (string)uri + "\\ExtMachineInfo", "DisableMidiIn", state?1:0);
}

bool CConfiguration::getMachineMidiInDisabled(std::string uri) {
	DWORD dw = 0;
	getConfigNumber("Machines\\" + (string)uri + "\\ExtMachineInfo", "DisableMidiIn", &dw);
	return dw != 0;
}

void CConfiguration::setStickySelections(bool state) {
	setConfigNumber("Settings", "StickySelections", state?1:0);
}

bool CConfiguration::getStickySelections() {
	DWORD dw = 0;
	getConfigNumber("Settings", "StickySelections", &dw);
	return dw != 0;
}

void CConfiguration::setColoredNotes(bool state) {
	setConfigNumber("Settings", "ColoredNotes", state?1:0);
}

bool CConfiguration::getColoredNotes() {
	DWORD dw = 0;
	getConfigNumber("Settings", "ColoredNotes", &dw);
	return dw != 0;
}

void CConfiguration::setShowAccelerators(bool state) {
	setConfigNumber("Settings", "ShowAccelerators", state?1:0);
}

bool CConfiguration::getShowAccelerators() {
	DWORD dw = 1;
	getConfigNumber("Settings", "ShowAccelerators", &dw);
	return dw != 0;
}

void CConfiguration::setOrderlistEnabled(bool state) {
	setConfigNumber("Settings", "OrderlistEnabled", state?1:0);
}

bool CConfiguration::getOrderlistEnabled() {
	DWORD dw = 0;
	getConfigNumber("Settings", "OrderlistEnabled", &dw);
	return dw != 0;
}

void CConfiguration::setGlobalPatternLength(int length) {
	setConfigNumber("Settings", "PatternLength", length);
}

int CConfiguration::getGlobalPatternLength() {
	DWORD dw = 64;
	getConfigNumber("Settings", "PatternLength", &dw);
	return dw;
}

void CConfiguration::setNotesAffectMode(int mode) {
	setConfigNumber("Settings", "NotesAffectMode", mode);
}

int CConfiguration::getNotesAffectMode() {
	DWORD dw = 3;
	getConfigNumber("Settings", "NotesAffectMode", &dw);
	return dw;
}

void CConfiguration::setPatternFollowMode(int mode) {
	setConfigNumber("Settings", "PatternFollowMode", mode);
}

int CConfiguration::getPatternFollowMode() {
	DWORD dw = 0;
	getConfigNumber("Settings", "PatternFollowMode", &dw);
	return dw;
}

void CConfiguration::setMachinePatternLength(std::string uri, int length) {
	setConfigNumber("Machines\\" + (string)uri + "\\ExtMachineInfo", "PatternLength", length);
}

int CConfiguration::getMachinePatternLength(std::string uri) {
	DWORD dw = getGlobalPatternLength();
	getConfigNumber("Machines\\" + (string)uri + "\\ExtMachineInfo", "PatternLength", &dw);
	return dw;
}

void CConfiguration::setMachineDefaultTracks(std::string uri, int length) {
	setConfigNumber("Machines\\" + (string)uri + "\\ExtMachineInfo", "DefaultTracks", length);
}

int CConfiguration::getMachineDefaultTracks(std::string uri, int deftracks) {
	DWORD dw = deftracks;
	getConfigNumber("Machines\\" + (string)uri + "\\ExtMachineInfo", "DefaultTracks", &dw);
	return dw;
}

void CConfiguration::setMachineSkinVisibility(bool state) {
	setConfigNumber("Settings", "MachineSkins", state?1:0);
}

bool CConfiguration::getMachineSkinVisibility() {
	DWORD dw = 0;
	getConfigNumber("Settings", "MachineSkins", &dw);
	return dw?true:false;
}

void CConfiguration::setScaleByWindowSize(bool state) {
	setConfigNumber("Settings", "ScaleByWindowSize", state?1:0);
}

bool CConfiguration::getScaleByWindowSize() {
	DWORD dw = 1;
	getConfigNumber("Settings", "ScaleByWindowSize", &dw);
	return dw?true:false;
}

void CConfiguration::setMachinesMinimized(bool state) {
	setConfigNumber("Settings", "MachinesMinimized", state?1:0);
}

bool CConfiguration::getMachinesMinimized() {
	DWORD dw = 0;
	getConfigNumber("Settings", "MachinesMinimized", &dw);
	return dw?true:false;
}

void CConfiguration::setMachineScale(double sc) {
	std::string str;
	char pc[10];
	sprintf(pc, "%.6f", sc);
	str = pc;
	setConfigString("Settings", "MachineScale", str);
}

double CConfiguration::getMachineScale() {
	string str;
	if (!getConfigString("Settings", "MachineScale", &str))
		return 1.0;
	return atof(str.c_str());
}

std::string CConfiguration::getNoteOffString() {
	string s;
	if (!getConfigString("Settings", "NoteOffString", &s))
		return "off";

	if (!s.size())
		return "off";

	if (s.size() > 3)
		s.resize(3);
	return s;
}

void CConfiguration::setNoteOffString(std::string s) {
	setConfigString("Settings", "NoteOffString", s);
}

std::string CConfiguration::getNoteCutString() {
	string s;
	if (!getConfigString("Settings", "NoteCutString", &s))
		return "^^^";

	if (!s.size())
		return "^^^";

	if (s.size() > 3)
		s.resize(3);
	return s;
}

void CConfiguration::setNoteCutString(std::string s) {
	setConfigString("Settings", "NoteCutString", s);
}

std::string CConfiguration::getBGNote() {
	string s;
	if (!getConfigString("Settings", "BGNote", &s))
		return "---";

	if (s.size() != 3)
		s.resize(3, '.');
	return s;
}

void CConfiguration::setBGNote(std::string s) {
	setConfigString("Settings", "BGNote", s);
}

std::string CConfiguration::getBGByte() {
	string s;
	if (!getConfigString("Settings", "BGByte", &s))
		return "..";

	if (s.size() != 2)
		s.resize(2, '.');
	return s;
}

void CConfiguration::setBGByte(std::string s) {
	setConfigString("Settings", "BGByte", s);
}

std::string CConfiguration::getBGSwitch() {
	string s;
	if (!getConfigString("Settings", "BGSwitch", &s))
		return ".";

	if (s.size() != 1)
		s.resize(1, '.');
	return s;
}

void CConfiguration::setBGSwitch(std::string s) {
	setConfigString("Settings", "BGSwitch", s);
}

std::string CConfiguration::getBGWord() {
	string s;
	if (!getConfigString("Settings", "BGWord", &s))
		return "....";

	if (s.size() != 4)
		s.resize(4, '.');
	return s;
}

void CConfiguration::setBGWord(std::string s) {
	setConfigString("Settings", "BGWord", s);
}

void CConfiguration::setVUDropSpeed(float sc) {
	std::string str;
	char pc[10];
	sprintf(pc, "%.6f", sc);
	str = pc;
	setConfigString("Settings", "VUDropSpeed", str);
}

float CConfiguration::getVUDropSpeed() {
	string str;
	if (!getConfigString("Settings", "VUDropSpeed", &str))
		return 0.0;
	return atof(str.c_str());
}

void CConfiguration::setVUTimerSpeed(int vv) {
	setConfigNumber("Settings", "VUTimerSpeed", vv);
}

int CConfiguration::getVUTimerSpeed() {
	DWORD dw = 2;
	getConfigNumber("Settings", "VUTimerSpeed", &dw);
	return dw;
}

void CConfiguration::setDefaultEntryMode(int i) {
	setConfigNumber("Settings", "DefaultEntryMode", i);
}

int CConfiguration::getDefaultEntryMode() {
	DWORD dw = 1; // horizontal -- we're being rebellious ;)
	getConfigNumber("Settings", "DefaultEntryMode", &dw);
	return dw;
}

void CConfiguration::setHorizontalScrollMode(int i) {
	setConfigNumber("Settings", "HorizontalScrollMode", i);
}

int CConfiguration::getHorizontalScrollMode() {
	DWORD dw = 0;
	getConfigNumber("Settings", "HorizontalScrollMode", &dw);
	return dw;
}

void CConfiguration::setVerticalScrollMode(int i) {
	setConfigNumber("Settings", "VerticalScrollMode", i);
}

int CConfiguration::getVerticalScrollMode() {
	DWORD dw = 0;
	getConfigNumber("Settings", "VerticalScrollMode", &dw);
	return dw;
}

void CConfiguration::setFormatPatternCreationMode(int i) {
	setConfigNumber("Settings", "FormatPatternCreationMode", i);
}

int CConfiguration::getFormatPatternCreationMode() {
	DWORD dw = 2;
	getConfigNumber("Settings", "FormatPatternCreationMode", &dw);
	return dw;
}

void CConfiguration::setPatternNamingMode(int i) {
	setConfigNumber("Settings", "PatternNamingMode", i);
}

int CConfiguration::getPatternNamingMode() {
	DWORD dw = 0;
	getConfigNumber("Settings", "PatternNamingMode", &dw);
	return dw;
}

void CConfiguration::setPatternRightClickMode(int i) {
	setConfigNumber("Settings", "PatternRightClickMode", i);
}

int CConfiguration::getPatternRightClickMode() {
	DWORD dw = 0;
	getConfigNumber("Settings", "PatternRightClickMode", &dw);
	return dw;
}

void CConfiguration::setSubrowNamingMode(int i) {
	setConfigNumber("Settings", "SubrowNamingMode", i);
}

int CConfiguration::getSubrowNamingMode() {
	DWORD dw = 1;
	getConfigNumber("Settings", "SubrowNamingMode", &dw);
	return dw;
}

void CConfiguration::setTriggerWidth(int x) {
	setConfigNumber("Settings", "TriggerWidth", x);
}

int CConfiguration::getTriggerWidth() {
	DWORD dw = 8;
	getConfigNumber("Settings", "TriggerWidth", &dw);
	return dw;
}

void CConfiguration::setDefaultScrollerWidth(int x) {
	setConfigNumber("Settings", "DefaultScrollerWidth", x);
}

int CConfiguration::getDefaultScrollerWidth() {
	DWORD dw = 128;
	getConfigNumber("Settings", "DefaultScrollerWidth", &dw);
	return dw;
}

void CConfiguration::setShowInfoPane(int x) {
	setConfigNumber("Settings", "ShowInfoPane", x);
}

int CConfiguration::getShowInfoPane() {
	DWORD dw = 1;
	getConfigNumber("Settings", "ShowInfoPane", &dw);
	return dw;
}

bool CConfiguration::getToolbarVisible(int id, bool defValue) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = defValue;
	getConfigNumber(key, "Visible", &dw);
	return dw?true:false;
}

void CConfiguration::setToolbarVisible(int id, bool state) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = state?1:0;
	_Module.configuration->setConfigNumber(key, "Visible", dw);
}

int CConfiguration::getToolbarWidth(int id, int defValue) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);

	DWORD dw = defValue;
	getConfigNumber(key, "Width", &dw);
	return dw;
}

void CConfiguration::setToolbarWidth(int id, int width) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = width;
	_Module.configuration->setConfigNumber(key, "Width", dw);
}

bool CConfiguration::getToolbarBreak(int id, bool defValue) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = defValue;
	getConfigNumber(key, "Break", &dw);
	return dw?true:false;
}

void CConfiguration::setToolbarBreak(int id, bool state) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = state?1:0;
	_Module.configuration->setConfigNumber(key, "Break", dw);
}

int CConfiguration::getToolbarPosition(int id, int defValue) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = defValue;
	getConfigNumber(key, "Position", &dw);
	return dw;
}

void CConfiguration::setToolbarPosition(int id, int position) {
	char key[128];
	sprintf(key, "Settings\\Toolbars\\#%i", id);
	DWORD dw = position;
	_Module.configuration->setConfigNumber(key, "Position", dw);
}

void CConfiguration::setLockedToolbars(bool state) {
	setConfigNumber("Settings", "LockedToolbars", (int)state);
}

bool CConfiguration::getLockedToolbars() {
	DWORD dw = 0;
	getConfigNumber("Settings", "LockedToolbars", &dw);
	return dw?true:false;
}

void CConfiguration::setPatternStackMode(int mode) {
	setConfigNumber("Settings", "PatternStackMode", mode);
}

int CConfiguration::getPatternStackMode() {
	DWORD dw = 0;
	getConfigNumber("Settings", "PatternStackMode", &dw);
	return (int)dw;
}

void CConfiguration::setVstPaths(std::string s) {
	setConfigString("Settings", "VstPaths", s);
}

std::string CConfiguration::getVstPaths() {
	std::string result;
	if (!getConfigString("Settings", "VstPaths", &result))
		result = "";
	return result;
}
