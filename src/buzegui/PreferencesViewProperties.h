#pragma once

class CViewFrame;
class CConfiguration;

class CPropertyView {
public:
	virtual void BindProvider() = 0;
};

class CPreferencePropertyProvider : public CPropertyProvider {
public:
	bool dirty;
	CPropertyView* view;
	CPreferencePropertyProvider(CPropertyView* view) { dirty = false; this->view = view; }
	virtual void doDataExchange(bool save) = 0;
	virtual bool providerMessage(int id) { return false; }
	virtual void deleteProperty(int index) { }
};

/***

	CAudioDriverProvider

***/

class CAudioDriverProvider : public CPreferencePropertyProvider {
public:
	CViewFrame* mainFrame;
	CBuzeConfiguration configuration;
	zzub_audiodriver_t* driver;
	int outDeviceIndex, inDeviceIndex;
	int rateIndex, bufferSizeIndex, outChannelIndex;
	int mixerThreadCount;

	std::vector<std::string> out_names;
	std::vector<std::string> in_names;
	std::vector<int> buffersizes;
	std::vector<int> samplerates;
	std::vector<zzub_device_info_t*> out_devices;
	std::vector<zzub_device_info_t*> in_devices;

	CAudioDriverProvider(CPropertyView* view, CViewFrame* mainFrame);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);
	virtual void doDataExchange(bool save);
	virtual bool providerMessage(int id);

	void enumerateDevices();
	void updateOutputCaps();

	int getAudioDriver();
	bool setAudioDriver(int);
	std::vector<std::string> getAudioOutputDevices();

	int getAudioInputDriver();
	bool setAudioInputDriver(int);
	std::vector<std::string> getAudioInputDevices();

	int getMixingRate();
	bool setMixingRate(int);
	std::vector<std::string> getMixingRates();

	int getLatency();
	bool setLatency(int);
	std::vector<std::string> getLatencies();

	int getMasterOutputChannel();
	bool setMasterOutputChannel(int);
	std::vector<std::string> getAllOutputChannels();
	std::vector<std::string> getAvailableOutputChannels();

	int getFirstOutputChannel();
	bool setFirstOutputChannel(int);

	int getLastOutputChannel();
	bool setLastOutputChannel(int);

	int getMixerThreads();
	bool setMixerThreads(int);
};


/***

	CMidiConfigProvider

***/

class CMidiConfigProvider;

class CMidiDeviceProperty {
public:
	CMidiConfigProvider* provider;
	int index;
	bool enabled;

	bool setValue(BOOL value);
	BOOL getValue();
};

class CMidiConfigProvider : public CPreferencePropertyProvider {
public:
	CViewFrame* mainFrame;
	CBuzeConfiguration configuration;
	zzub_audiodriver_t* driver;
	std::vector<CMidiDeviceProperty*> out_devices;
	std::vector<CMidiDeviceProperty*> in_devices;

	CMidiConfigProvider(CPropertyView* view, CViewFrame* mainFrame);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);

	virtual void doDataExchange(bool save);
};


/***

	CThemeConfigProvider

***/

class CThemeConfigProvider;

class CThemeColorProperty {
public:
	CThemeConfigProvider* provider;
	std::string name;
	COLORREF value;

	bool setValue(COLORREF value);
	COLORREF getValue();
};

class CThemeConfigProvider : public CPreferencePropertyProvider {
public:
	CViewFrame* mainFrame;
	int themeIndex;
	std::string currentTheme;

	CThemeConfigProvider(CPropertyView* view, CViewFrame* mainFrame);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);

	void fillThemes(std::string prefix);

	int getTheme();
	bool setTheme(int);
	std::vector<std::string> getThemes();
	virtual void doDataExchange(bool save);
	virtual void deleteProperty(int index);
};


/***

	CGuiConfigProvider

***/

class CGuiConfigProvider : public CPreferencePropertyProvider
{
  public:

	CViewFrame* mainFrame;
	CBuzeConfiguration configuration;

	int
		VUDropSpeedIndex,
		VUTimerSpeedIndex,
		defaultEntryModeIndex,
		zoomIndex,
		fontIndex,
		fontSizeIndex,
		patternLength,
		triggerWidth,
		formatPatternCreationMode,
		horizontalScrollModeIndex,
		verticalScrollModeIndex,
		defaultScrollerWidth,
		patternNamingModeIndex,
		patternRightClickModeIndex,
		subrowNamingModeIndex
	;

	bool
		showAccelerators,
		machineSkins,
		paramPopup,
		stickySelection,
		scaleByWindowSize,
		coloredNotes,
		machinesMinimized,
		defaultDocumentAdvanced,
		patternFormatPositionCache
	;

	std::string
		waveEditor,
		noteOffStr,
		noteCutStr,
		bg_byte,
		bg_word,
		bg_switch,
		bg_note
	;

	CGuiConfigProvider(CPropertyView* view, CViewFrame* mainFrame);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);
	virtual void doDataExchange(bool save);

	std::string getGui() { return "..."; }
	bool setGui(std::string color) {}

	int getVUDropSpeed();
	bool setVUDropSpeed(int);
	std::vector<std::string> getVUDropSpeeds();

	int getVUTimerSpeed();
	bool setVUTimerSpeed(int i);
	std::vector<std::string> getVUTimerSpeeds();

	int getDefaultEntryMode();
	bool setDefaultEntryMode(int);
	std::vector<std::string> getDefaultEntryModes();

	int getHorizontalScrollMode();
	bool setHorizontalScrollMode(int);
	std::vector<std::string> getHorizontalScrollModes();
	int getVerticalScrollMode();
	bool setVerticalScrollMode(int);
	std::vector<std::string> getVerticalScrollModes();

	int getZoom();
	bool setZoom(int);
	std::vector<std::string> getZooms();

	BOOL getMachineSkins();
	bool setMachineSkins(BOOL value);

	BOOL getScaleByWindowSize();
	bool setScaleByWindowSize(BOOL value);

	int getFont();
	bool setFont(int);
	std::vector<std::string> getFonts();

	HDC enumFontDC;
	std::vector<std::string> fixedWidthFonts;
	std::vector<std::vector<int> > fixedWidthSizes;
	void enumFixedWidthFonts();
	void enumFixedWidthSizes(std::string name);
	static int CALLBACK FontNameProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam);
	static int CALLBACK FontSizeProc(ENUMLOGFONTEX* lpelfe, TEXTMETRIC *ptm, int FontType, LPARAM lParam);

	int getFontSize();
	bool setFontSize(int);
	std::vector<std::string> getFontSizes();

	BOOL getShowAccelerators();
	bool setShowAccelerators(BOOL value);

	int getPatternLength();
	bool setPatternLength(int);

	BOOL getStickySelection();
	bool setStickySelection(BOOL value);

	BOOL getParameterPopup();
	bool setParameterPopup(BOOL value);

	std::string getExternalWaveEditor();
	bool setExternalWaveEditor(std::string);

	bool setNoteOffString(std::string s);
	std::string getNoteOffString();
	bool setNoteCutString(std::string s);
	std::string getNoteCutString();
	bool setBGNote(std::string s);
	std::string getBGNote();
	bool setBGByte(std::string s);
	std::string getBGByte();
	bool setBGSwitch(std::string s);
	std::string getBGSwitch();
	bool setBGWord(std::string s);
	std::string getBGWord();

	int getTriggerWidth();
	bool setTriggerWidth(int value);
	
	BOOL getColoredNotes();
	bool setColoredNotes(BOOL value);

	int getFormatPatternCreationMode();
	bool setFormatPatternCreationMode(int i);
	std::vector<std::string> getFormatPatternCreationModes();

	int getDefaultScrollerWidth();
	bool setDefaultScrollerWidth(int value);

	int getPatternNamingMode();
	bool setPatternNamingMode(int i);
	std::vector<std::string> getPatternNamingModes();

	int getPatternRightClickMode();
	bool setPatternRightClickMode(int i);
	std::vector<std::string> getPatternRightClickModes();

	BOOL getMachinesMinimized();
	bool setMachinesMinimized(BOOL value);

	int getSubrowNamingMode();
	bool setSubrowNamingMode(int i);
	std::vector<std::string> getSubrowNamingModes();

	BOOL getDefaultDocumentAdvanced();
	bool setDefaultDocumentAdvanced(BOOL value);

	BOOL getPatternFormatPositionCache();
	bool setPatternFormatPositionCache(BOOL value);

};


/***

	CThemeConfigProvider

***/

class CKeyboardConfigProvider;

class CKeyboardProperty {
public:
	CKeyboardConfigProvider* provider;
	ACCEL* accelData;
	int accelIndex;
	std::string name;

	bool setValue(std::string value);
	std::string getValue();
};

class CKeyboardConfigProvider : public CPreferencePropertyProvider {
public:
	CViewFrame* mainFrame;

	CKeyboardConfigProvider(CPropertyView* view, CViewFrame* mainFrame);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);
	virtual void doDataExchange(bool save) {}

	std::string getCommandText(WORD cmd);
};


class CPluginConfigProvider : public CPreferencePropertyProvider {
public:
	std::string vstPaths;
	CViewFrame* mainFrame;
	CBuzeConfiguration configuration;

	CPluginConfigProvider(CPropertyView* view, CViewFrame* mainFrame);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);
	virtual void doDataExchange(bool save);

	bool setVstPath(std::string s);
	std::string getVstPath();
};
