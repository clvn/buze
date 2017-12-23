#pragma once

// NOTE: the configuration suffers some lazyness when the views were DLLized!
// buzelib.dll implements CBuzeConfigurationImpl which inherits from CBuzeConfiguration 
// and CConfiguration. this CConfiguration can be retreived by DLLs via the document 
// or application *_get_configuration()-apis. views using the old C++-syntax can
// emulate this using the CBuzeConfiguration with minor source code modifications.

// in the end all views should use the c api for configuration and this should be
// moved completely into the buzelib.dll - unavailable to views.

class CBuzeConfiguration {
  public:
	buze_configuration_t* configuration;

	CBuzeConfiguration();
	CBuzeConfiguration(buze_configuration_t* _configuration);
	~CBuzeConfiguration();

	// operator hacks: these are used to reduce the amount of work needed to get the configuration abi working across the dll boundary
	void operator=(buze_configuration_t* _configuration);
	CBuzeConfiguration* operator->() { return this; }

	virtual bool getString(const char* section, const char* key, char* value);
	virtual bool getNumber(const char* section, const char* key, DWORD* value);
	virtual bool setString(const char* section, const char* key, const char* value);
	virtual bool setNumber(const char* section, const char* key, int value);
	bool getAudioDriver(std::string& outDevice, std::string& inDevice, int& rate, int& bufferSize, int& outChannel);
	bool setAudioDriver(std::string outDevice, std::string inDevice, int rate, int bufferSize, int outChannel);

	bool getMidiInputs(std::vector<std::string>& names);
	bool setMidiInputs(std::vector<std::string> names);
	bool getMidiOutputs(std::vector<std::string>& names);
	bool setMidiOutputs(std::vector<std::string> names);

	int getMixerThreads();
	void setMixerThreads(int count);

	bool getParameterPopupFlag();
	void setParameterPopupFlag(bool flag);

	std::string getTheme();
	void setTheme(std::string theme);

	std::string getRecentSong(int index);
	void insertRecentSong(std::string fileName);
	int getMaxRecentSongs();
	void setMaxRecentSongs(int max);

	void setMachineParameterVisibility(std::string uri, int group, int column, bool state);
	bool getMachineParameterVisibility(std::string uri, int group, int column);

	int getNumSamplePaths();
	std::string getSamplePath(int index);
	void removeSamplePath(int index);
	void addSamplePath(std::string);

	void setMachineMidiInDisabled(std::string uri, bool state);
	bool getMachineMidiInDisabled(std::string uri);

	void setStickySelections(bool state);
	bool getStickySelections();

	void setShowAccelerators(bool state);
	bool getShowAccelerators();

	void setGlobalPatternLength(int length);
	int getGlobalPatternLength();

	void setMachinePatternLength(std::string uri, int length);
	int getMachinePatternLength(std::string uri);

	void setMachineDefaultTracks(std::string uri, int length);
	int getMachineDefaultTracks(std::string uri, int deftracks);

	void setMachineSkinVisibility(bool state);
	bool getMachineSkinVisibility();

	void setScaleByWindowSize(bool state);
	bool getScaleByWindowSize();

	void setExternalWaveEditor(std::string cmd);
	std::string getExternalWaveEditor();

	void setMachineScale(float sc);
	float getMachineScale();

	void setNoteOffString(std::string s);
	std::string getNoteOffString();
	void setNoteCutString(std::string s);
	std::string getNoteCutString();
	void setBGNote(std::string s);
	std::string getBGNote();
	void setBGByte(std::string s);
	std::string getBGByte();
	void setBGSwitch(std::string s);
	std::string getBGSwitch();
	void setBGWord(std::string s);
	std::string getBGWord();

	void setVUDropSpeed(float sc);
	float getVUDropSpeed();
	void setVUTimerSpeed(int vv);
	int getVUTimerSpeed();

	void setDefaultEntryMode(int i);
	int getDefaultEntryMode();

	void setHorizontalScrollMode(int i);
	int getHorizontalScrollMode();
	void setVerticalScrollMode(int i);
	int getVerticalScrollMode();

	void setTriggerWidth(int x);
	int getTriggerWidth();

	bool getToolbarVisible(int id, bool defValue);
	void setToolbarVisible(int id, bool state);
	int getToolbarWidth(int id, int defValue);
	void setToolbarWidth(int id, int width);
	bool getToolbarBreak(int id, bool defValue);
	void setToolbarBreak(int id, bool state);
	int getToolbarPosition(int id, int defValue);
	void setToolbarPosition(int id, int position);

	void setLockedToolbars(bool state);
	bool getLockedToolbars();

	int getDefaultAmp();
	void setDefaultAmp(int amp);

	void setFormatPatternCreationMode(int i);
	int getFormatPatternCreationMode();

	bool getColoredNotes();
	void setColoredNotes(bool state);	

	void setDefaultScrollerWidth(int x);
	int getDefaultScrollerWidth();

	void setShowInfoPane(int x);
	int getShowInfoPane();

	void setPatternNamingMode(int i);
	int getPatternNamingMode();

	void setPatternRightClickMode(int i);
	int getPatternRightClickMode();

	void setMachinesMinimized(bool state);
	bool getMachinesMinimized();

	void setSubrowNamingMode(int i);
	int getSubrowNamingMode();

	void setNotesAffectMode(int mode);
	int getNotesAffectMode();

	void setPatternFollowMode(int mode);
	int getPatternFollowMode();

	void setOrderlistEnabled(bool state);
	bool getOrderlistEnabled();

	void setPatternStackMode(int mode);
	int getPatternStackMode();

	void setVstPaths(std::string s);
	std::string getVstPaths();

	void setAdvancedDefaultDocument(bool state);
	bool getAdvancedDefaultDocument();

	void setPatternPositionCachePatternFormat(bool state);
	bool getPatternPositionCachePatternFormat();

};
