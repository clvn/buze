#pragma once

#include "Properties.h"

class CPatternPropertyProvider;

class CTrackProperty {
public:
	CPatternPropertyProvider* provider;
	zzub_pattern_format_t* format;
	zzub_plugin_t* plugin;
	int group, track;

	bool setLabel(std::string value);
	std::string getLabel();
	bool setMuted(BOOL value);
	BOOL getMuted();
};

class CPatternPropertyProvider : public CPropertyProvider {
public:
	CDocument* document;
	zzub_player_t* player;
	zzub_pattern_t* pattern;

	CPatternPropertyProvider(CView* returnView, CDocument* doc, zzub_pattern_t* pattern);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);

	// pattern properties (working on selected pattern)
	bool setPatternName(std::string name);
	std::string getPatternName();
	bool setPatternLength(int length);
	int getPatternLength();
	BOOL getPatternRepeatLength();
	bool setPatternRepeatLength(BOOL state);
	int getResolution();
	bool setResolution(int resolution);
	BOOL getLoopEnabled();
	bool setLoopEnabled(BOOL state);
	bool setEndLoop(int pos);
	int getEndLoop();
	bool setBeginLoop(int pos);
	int getBeginLoop();
};

class CPatternFormatPropertyProvider : public CPropertyProvider {
public:
	CDocument* document;
	zzub_player_t* player;
	zzub_pattern_format_t* patternformat;
	
	CPatternFormatPropertyProvider(CView* returnView, CDocument* doc, zzub_pattern_format_t* format);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);

	bool setName(std::string name);
	std::string getName();
};

class CMachinePropertyProvider;

class CAttributeProperty {
public:
	CMachinePropertyProvider* provider;
	zzub_plugin_t* machine;
	int attributeIndex;

	bool setValue(int value);
	int getValue();
};

class CMachinePropertyProvider : public CPropertyProvider {
public:
	CDocument* document;
	zzub_player_t* player;
	CBuzeConfiguration configuration;
	zzub_plugin_t* machine;

	CMachinePropertyProvider(CView* returnView, CDocument* doc, zzub_plugin_t* machine);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);

	// machine properties (working on selected machine)
	bool setMachineName(std::string name);
	std::string getMachineName();
	bool setMachineTracks(int tracks);
	int getMachineTracks();
	BOOL getMachineMuted();
	bool setMachineMuted(BOOL state);
	BOOL getMachineSolo();
	bool setMachineSolo(BOOL state);
	BOOL getMachineBypass();
	bool setMachineBypass(BOOL state);
	BOOL getMachineMidiIn();
	bool setMachineMidiIn(BOOL state);
	bool setStreamSource(std::string name);
	std::string getStreamSource();

	std::string getMachineURI();
	std::string getMachineType();
	std::string getMachineFileName();
	std::string getMachineFullName();
	std::string getMachineAuthor();
	std::string getMachineAudioInput();
	std::string getMachineAudioOutput();
	std::string getMachineMidiInput();
	std::string getMachineMidiOutput();
	std::string getMachineEventOutput();
	std::string getMachineInterval();
	std::string getMachineSequence();

	bool setMachinePatternLength(int length);
	int getMachinePatternLength();

	bool setMachineDefaultTracks(int length);
	int getMachineDefaultTracks();

	bool setMachineLatency(int length);
	int getMachineLatency();
};

class CGroupPropertyProvider : public CPropertyProvider {
public:
	CDocument* document;
	zzub_player_t* player;
	CBuzeConfiguration configuration;
	zzub_plugin_group_t* plugingroup;

	CGroupPropertyProvider(CView* returnView, CDocument* doc, zzub_plugin_group_t* group);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);

	bool setName(std::string name);
	std::string getName();
};

class CConnectionPropertyProvider : public CPropertyProvider {
public:
	CDocument* document;
	zzub_player_t* player;
	zzub_plugin_t* from_plugin;
	zzub_plugin_t* to_plugin;

	CConnectionPropertyProvider(CView* returnView, CDocument* doc, zzub_plugin_t* _to_plugin, zzub_plugin_t* _from_plugin);
	CConnectionPropertyProvider(CView* returnView, CDocument* doc, zzub_connection_t* _conn);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);

	bool setFirstInput(int chn);
	int getFirstInput();
	bool setInputs(int chn);
	int getInputs();

	bool setFirstOutput(int chn);
	int getFirstOutput();
	bool setOutputs(int chn);
	int getOutputs();

	std::string getMidiDevice();
	std::string getFromPluginName();
	std::string getToPluginName();
};

class CWavePropertyProvider : public CPropertyProvider {
public:
	CDocument* document;
	zzub_player_t* player;
	zzub_wave_t* wave;

	CWavePropertyProvider(CView* returnView, CDocument* doc, zzub_wave_t* wave);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);

	// wave properties
	int getWaveIndex();
	bool setWaveVolume(int vol);
	int getWaveVolume();
	bool setWaveLooping(BOOL );	// eller waveFlags?
	BOOL getWaveLooping();
	bool setWaveBidirLoop(BOOL );	// eller waveFlags?
	BOOL getWaveBidirLoop();
	std::string getWaveFileName();
	bool setWaveFileName(std::string name);
	std::string getWaveName();
	bool setWaveName(std::string name);
};

class CWaveLevelPropertyProvider : public CPropertyProvider {
public:
	CDocument* document;
	zzub_player_t* player;
	zzub_wave_t* wave;
	zzub_wavelevel_t* level;
	HWND hWnd;

	CWaveLevelPropertyProvider(CView* hWnd, CDocument* doc, zzub_wave_t* wave, zzub_wavelevel_t* level);
	virtual void createProperties();
	virtual int getProperties();
	virtual PropertyInfoBase* getProperty(int index);
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint);

	bool setWaveLevelBaseNote(int note);	// eller waveLevelProps?
	int getWaveLevelBaseNote();
	bool setWaveLevelBits(int bits);
	int getWaveLevelBits();
	bool setWaveLevelEndLoop(int bits);
	int getWaveLevelEndLoop();
	bool setWaveLevelBeginLoop(int bits);
	int getWaveLevelBeginLoop();
	int getWaveLevelSamples();
	bool setWaveLevelSamples(int samples);
	bool setWaveLevelType(int bits);
	int getWaveLevelType();
	bool setWaveLevelSampleRate(int rate);
	int getWaveLevelSampleRate();
};
