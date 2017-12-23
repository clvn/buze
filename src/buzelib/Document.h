#pragma once

#include "zzub/zzub.h"
#include "MachineIndex.h"
#include "ThemeManager.h"

// ---------------------------------------------------------------------------------------------------------------
// DOCUMENT TYPES
// ---------------------------------------------------------------------------------------------------------------

struct PLUGININFO {
	bool nonSongPlugin;
	std::string lastPreset;
	int paramViewMode; // MachineParameterViewMode
};

struct KEYJAZZNOTE {
	zzub_plugin_t* plugin;
	int key;
	int note;
};

class CApplication;

// ---------------------------------------------------------------------------------------------------------------
// DOCUMENT
// ---------------------------------------------------------------------------------------------------------------

class CPatternView;
class CPropertyProvider;
class CConfiguration;
class CView;
class CEventHandler;

class CDocument
{
  public:

	std::map<zzub_plugin_t*, PLUGININFO> pluginData; // gui-specific data per plugin
	std::vector<CEventHandler*> views; // listeners that receive OnUpdate messages
	int lastSaveUndoPosition; // use to display "Are you sure you want to quit"-message

	std::string currentFileName; // titlebar filename
	std::string currentDirectory; // directory to save current file in
	std::string currentExtension; // extension of current file

	zzub_player_t* player;

	MachineIndex machineIndex; // index.txt wrapper

	CBuzeConfiguration configuration;
	ThemeManager* themes;
	CApplication* application;

	int octave;

	zzub_plugin_t* streamplayer;
	zzub_plugin_t* soloplugin;
	std::vector<KEYJAZZNOTE> keyjazznotes;

	std::map<std::string, std::string> stream_ext_uri_mappings;

	zzub_plugin_t* currentPlugin;
	zzub_wave_t* currentWave;
	zzub_wavelevel_t* currentWavelevel;
	zzub_pattern_t* currentPattern;
	zzub_pattern_format_t* currentPatternformat;
	zzub_connection_t* currentConnection;
	int currentOrderIndex;
	int currentOrderPatternRow;
	int currentPatternRow;

	// creation / destruction
	CDocument(CApplication* _app, zzub_player_t* _player, CConfiguration* config, ThemeManager* _themes);
	~CDocument();

	// views
	void updateAllViews(CView* pSender, int hint, void* param = 0);
	void addView(CEventHandler* view);
	void removeView(CEventHandler* view);

	// graph control
	zzub_plugin_t* createMachine(std::string const& uri, std::string const& instrumentName, float x, float y, zzub_plugin_group_t* plugingroup);
	zzub_pattern_format_t* createDefaultFormat(zzub_plugin_t* plugin, bool simple);
	void extendPatternFormat(zzub_pattern_format_t* format, zzub_plugin_t* plugin, bool simple);

	BOOL getMachineSolo(zzub_plugin_t*);
	void setMachineSolo(zzub_plugin_t* m, BOOL state);
	bool isDirty();

	// importing
	bool importSong(std::vector<char>& bytes, int flags, float x, float y, std::string* error_messages);
	bool importSong(std::string const& filename, int flags, float x, float y, std::string* error_messages);

	// ?
	bool createDefaultDocument();
	bool createDefaultDocumentSimple();
	bool createDefaultDocumentAdvanced();

	// machine properties (working on selected machine)
	BOOL getMachineNonSong(zzub_plugin_t* m);
	void setMachineNonSong(zzub_plugin_t* m, BOOL state);
	std::string getMachinePreset(zzub_plugin_t* plugin);
	void setMachinePreset(zzub_plugin_t* plugin, std::string const& name);

	// plugin data
	PLUGININFO& getPluginData(zzub_plugin_t* plugin);
	void removePluginData(zzub_plugin_t* plugin);

	// streams
	void enumerateStreamPlugins();
	std::string getStreamPluginUriForFile(std::string const& fileName);
	void createStreamPlayer(std::string const& uri);
	void deleteStreamPlayer();
	void playStream(int note, unsigned int offset, unsigned int length, std::string const& dataUrl, std::string const& pluginUri);

	// ?
	//bool loadSampleByFileName(const char *szFileName, int curWave);
	void setCurrentFile(std::string fileName);

	// keyjazz
	void playMachineNote(zzub_plugin_t* m, int note, int prevNote); // note = buzz note or NOTE_OFF, prevNote = note for which NOTE_OFF is set or -1
	void keyjazzKeyDown(zzub_plugin_t* plugin, int key, int note);
	KEYJAZZNOTE keyjazzKeyUp(int key);
	void keyjazzRelease(bool sendNoteOffs);

	// machine help
	std::string getMachineHelpFile(zzub_pluginloader_t* loader);

	// themes
	void clearSong();
	bool saveFile(std::string const& filename, bool withWaves);

	bool loadIndex();

	void preloadMachines();	// walks through index.txt and preloads those marked as such
	void preloadMachines(IndexItem* item, std::map<zzub_pluginloader_t*, std::vector<std::string> >& libs);	// walks through index.txt and preloads those marked as such
	void preloadMachine(MachineItem* machineItem, std::map<zzub_pluginloader_t*, std::vector<std::string> >& libs);
	void populateTemplateDirectory(MachineMenu* parent, std::string const& path);
	void populateUnsortedMachines(); // adds an entry to index.txt with machines not previously listed
};
