#include "stdafx.h"
#include "resource.h"
#include <shlobj.h>
#include <cfloat>
#include <cmath>
#include <limits>
#include <boost/range/size.hpp>
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "PropertyList/PropertyList.h"
#include "ThemeManager.h"
#include "utils.h"
#include "Configuration.h"
#include "BuzeConfiguration.h"
#include "Properties.h"
#include "PreferencesViewProperties.h"

template <typename T>
int find_index_with_nearest_value(const std::vector<T>& vec, const T& value) {
	T diff = std::numeric_limits<T>::max();
	T index = -1;
	for (std::vector<T>::const_iterator i = vec.begin(); i != vec.end(); ++i) {
		if (*i == value) return (int)std::distance(vec.begin(), i);
		T valuediff = abs(*i - value);
		if (valuediff < diff) {
			diff = valuediff;
			index = (int)std::distance(vec.begin(), i);
		}
	}
	return index;
}

/***

	CAudioDriverProvider

***/

CAudioDriverProvider::CAudioDriverProvider(CPropertyView* view, CViewFrame* mainFrm):CPreferencePropertyProvider(view) {
	mainFrame = mainFrm;
	
	buze_document_t* document = buze_main_frame_get_document(mainFrame);
	configuration = buze_document_get_configuration(document);

	buze_application_t* app = buze_main_frame_get_application(mainFrame);
	driver = (zzub_audiodriver_t*)buze_application_get_audio_driver(app);

	outDeviceIndex = 0;
	inDeviceIndex = 0;
	rateIndex = 0;
	bufferSizeIndex = 0;
	outChannelIndex = 0;

	createProperties();
}

void CAudioDriverProvider::createProperties() {

	destroyProperties();
//	mainFrame->configuration->getAudioDriver(outDevice, inDevice, rate, size, outch);

	enumerateDevices();

	properties.push_back(new CategoryPropertyInfo("Mixer"));
	properties.push_back(new IntPropertyInfo<CAudioDriverProvider>(this, "Mixer Threads", &CAudioDriverProvider::getMixerThreads, &CAudioDriverProvider::setMixerThreads));

	properties.push_back(new CategoryPropertyInfo("Audio Output"));
	properties.push_back(new StringListPropertyInfo<CAudioDriverProvider>(this, "Output Device", &CAudioDriverProvider::getAudioDriver, &CAudioDriverProvider::setAudioDriver, &CAudioDriverProvider::getAudioOutputDevices));
	properties.push_back(new StringListPropertyInfo<CAudioDriverProvider>(this, "Mixing Rate", &CAudioDriverProvider::getMixingRate, &CAudioDriverProvider::setMixingRate, &CAudioDriverProvider::getMixingRates));
	properties.push_back(new StringListPropertyInfo<CAudioDriverProvider>(this, "Latency", &CAudioDriverProvider::getLatency, &CAudioDriverProvider::setLatency, &CAudioDriverProvider::getLatencies));
	//properties.push_back(new StringListPropertyInfo<CAudioDriverProvider>(this, "First Output Channel", &CAudioDriverProvider::getFirstOutputChannel, &CAudioDriverProvider::setFirstOutputChannel, &CAudioDriverProvider::getAllOutputChannels));
	//properties.push_back(new StringListPropertyInfo<CAudioDriverProvider>(this, "Last Output Channel", &CAudioDriverProvider::getLastOutputChannel, &CAudioDriverProvider::setLastOutputChannel, &CAudioDriverProvider::getAvailableOutputChannels));
	properties.push_back(new StringListPropertyInfo<CAudioDriverProvider>(this, "Master Output Channel", &CAudioDriverProvider::getMasterOutputChannel, &CAudioDriverProvider::setMasterOutputChannel, &CAudioDriverProvider::getAvailableOutputChannels));
	properties.push_back(new CategoryPropertyInfo("Audio Input"));
	properties.push_back(new StringListPropertyInfo<CAudioDriverProvider>(this, "Input Device", &CAudioDriverProvider::getAudioInputDriver, &CAudioDriverProvider::setAudioInputDriver, &CAudioDriverProvider::getAudioInputDevices));
	//properties.push_back(new StringListPropertyInfo<CAudioDriverProvider>(this, "First Input Channel", &CAudioDriverProvider::getFirstOutputChannel, &CAudioDriverProvider::setFirstOutputChannel, &CAudioDriverProvider::getAllOutputChannels));
	//properties.push_back(new StringListPropertyInfo<CAudioDriverProvider>(this, "Last Input Channel", &CAudioDriverProvider::getLastOutputChannel, &CAudioDriverProvider::setLastOutputChannel, &CAudioDriverProvider::getAvailableOutputChannels));

}

int CAudioDriverProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CAudioDriverProvider::getProperty(int index) {
	return properties[index];
}

bool CAudioDriverProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);
	return false;
}

void CAudioDriverProvider::enumerateDevices() {

	out_devices.clear();
	out_names.clear();

	{
		zzub_device_info_iterator_t* it = zzub_audiodriver_get_output_iterator(driver);

		while (zzub_device_info_iterator_valid(it)) {
			zzub_device_info_t* info = zzub_device_info_iterator_current(it);

			out_names.push_back(zzub_device_info_get_name(info));
			out_devices.push_back(info);

			zzub_device_info_iterator_next(it);
		}
		zzub_device_info_iterator_destroy(it);
	}

}

bool CAudioDriverProvider::providerMessage(int id) {
	if (id != 0) return false;
	enumerateDevices();
	updateOutputCaps();
	doDataExchange(false);
	view->BindProvider();
	return true;
}


void CAudioDriverProvider::updateOutputCaps() {
	buffersizes.resize(32);
	int count = zzub_device_info_get_supported_buffersizes(out_devices[outDeviceIndex], &buffersizes.front(), (int)buffersizes.size());
	buffersizes.resize(count);

	samplerates.resize(32);
	count = zzub_device_info_get_supported_samplerates(out_devices[outDeviceIndex], (int*)&samplerates.front(), (int)samplerates.size());
	samplerates.resize(count);

	in_names.clear();
	in_devices.clear();
	zzub_device_info_t* outdevice = out_devices[outDeviceIndex];
	zzub_device_info_iterator_t* it = zzub_audiodriver_get_input_iterator_for_output(driver, outdevice);
	while (zzub_device_info_iterator_valid(it)) {
		zzub_device_info_t* indevice = zzub_device_info_iterator_current(it);
		
		in_names.push_back(zzub_device_info_get_name(indevice));
		in_devices.push_back(indevice);

		zzub_device_info_iterator_next(it);
	}

	zzub_device_info_iterator_destroy(it);
}

// load/save settings via persistence mechanism and apply on components
void CAudioDriverProvider::doDataExchange(bool save) {
	if (!save) {

		zzub_device_info_t* outdevice = zzub_audiodriver_get_current_device(driver, 0);
		zzub_device_info_t* indevice = zzub_audiodriver_get_current_device(driver, 1);

		// get driver values
		int rate = zzub_audiodriver_get_samplerate(driver);
		int size = zzub_audiodriver_get_buffersize(driver);
		int outch = zzub_audiodriver_get_master_channel(driver);
		std::string outname = (outdevice) ? zzub_device_info_get_name(outdevice) : "";
		std::string inname = (indevice) ? zzub_device_info_get_name(indevice) : "";

		std::vector<std::string>::iterator i = find(out_names.begin(), out_names.end(), outname);
		if (i == out_names.end())
			outDeviceIndex = 0; else
			outDeviceIndex = (int)std::distance(out_names.begin(), i);

		updateOutputCaps();

		rateIndex = std::max(find_index_with_nearest_value(samplerates, rate), 0);
		bufferSizeIndex = std::max(find_index_with_nearest_value(buffersizes, size), 0);

		outChannelIndex = outch;

		i = find(in_names.begin(), in_names.end(), inname);
		if (i == in_names.end())
			inDeviceIndex = 0; else
			inDeviceIndex = (int)std::distance(in_names.begin(), i) + 1;

		// get stored config settings and set dirty flag if anythings different from the live settings
		int oldrate, oldsize, oldoutch;
		std::string oldoutname, oldinname;
		configuration->getAudioDriver(oldoutname, oldinname, oldrate, oldsize, oldoutch);
		dirty = (oldoutname != outname || oldinname != inname || oldrate != rate || oldsize != size || oldoutch != outch);

		mixerThreadCount = configuration->getMixerThreads();
	} else {

		std::string outDevice = out_names[outDeviceIndex];
		std::string inDevice = (inDeviceIndex > 0) ? in_names[inDeviceIndex - 1] : "";
		int rate = samplerates.empty() ? 44100 : samplerates[rateIndex];
		int size = buffersizes.empty() ? 1024 : buffersizes[bufferSizeIndex];

		// just save mixerthreads, setAudioDriver() updates the player from the config:
		configuration->setMixerThreads(mixerThreadCount);

		buze_application_t* app = buze_main_frame_get_application(mainFrame);
		if (buze_application_create_audio_device(app, outDevice.c_str(), inDevice.c_str(), rate, size, outChannelIndex, true)) {
			dirty = false;
		}

	}
}

int CAudioDriverProvider::getAudioDriver() {
	return outDeviceIndex;
}

bool CAudioDriverProvider::setAudioDriver(int index) {
	
	if (outDeviceIndex == index) return false;

	dirty = true;

	int rate = samplerates.empty() ? 44100 : samplerates[rateIndex];
	int size = buffersizes.empty() ? 1024 : buffersizes[bufferSizeIndex];

	updateOutputCaps();

	outDeviceIndex = index;
	updateOutputCaps();

	inDeviceIndex = 0;
	rateIndex = std::max(find_index_with_nearest_value(samplerates, rate), 0);
	bufferSizeIndex = std::max(find_index_with_nearest_value(buffersizes, size), 0);

	int device_out_channels = zzub_device_info_get_supported_output_channels(out_devices[outDeviceIndex]);
	if (outChannelIndex >= device_out_channels / 2)
		outChannelIndex = device_out_channels / 2 - 1;
		
	view->BindProvider();

	return true;
}

std::vector<std::string> CAudioDriverProvider::getAudioOutputDevices() {
	return out_names;
}

int CAudioDriverProvider::getAudioInputDriver() {
	return inDeviceIndex;
}

bool CAudioDriverProvider::setAudioInputDriver(int index) {
	dirty = true;
	inDeviceIndex = index;
	return true;
}

std::vector<std::string> CAudioDriverProvider::getAudioInputDevices() {
	std::vector<std::string> result;
	result.push_back("<none>");
	if (out_devices.size() == 0) return result;

	result.insert(result.end(), in_names.begin(), in_names.end());
	return result;
}

int CAudioDriverProvider::getMixingRate() {
	return rateIndex;
}

bool CAudioDriverProvider::setMixingRate(int index) {
	rateIndex = index;

	dirty = true;
	view->BindProvider();
	return true;
}

std::vector<std::string> CAudioDriverProvider::getMixingRates() {
	std::vector<std::string> result;
	if (out_devices.size() == 0) {
		result.push_back("<no driver>");
		return result;
	}

	for (size_t i = 0; i < samplerates.size(); i++) {
		result.push_back(stringFromInt(samplerates[i]) + " Hz");
	}

	if (result.empty()) result.push_back("<default>");
	return result;
}

int CAudioDriverProvider::getLatency() {
	return bufferSizeIndex;
}

bool CAudioDriverProvider::setLatency(int index) {

	dirty = true;
	bufferSizeIndex = index;
	return true;
}

std::vector<std::string> CAudioDriverProvider::getLatencies() {
	std::vector<std::string> result;

	if (out_devices.size() == 0) {
		result.push_back("<no driver>");
		return result;
	}

	if (samplerates.size() > 0) {
		int rate = samplerates[rateIndex];

		char pc[64];
		for (size_t i = 0; i < (int)buffersizes.size(); i++) {
			int buffersize = buffersizes[i];
			float latency = ((float)buffersize / rate) * 1000.0;

			if (!_isnan(latency) && _finite(latency)) {
				sprintf(pc, "%.2f ms (%i)", latency, buffersize);
			} else {
				sprintf(pc, "%i samples", buffersize);
			}

			result.push_back(pc);
		}
	}

	if (result.empty()) result.push_back("<default>");
	return result;
}


int CAudioDriverProvider::getMasterOutputChannel() {
	return outChannelIndex;
}

bool CAudioDriverProvider::setMasterOutputChannel(int index) {

	dirty = true;
	outChannelIndex = index;
	return true;
}

std::vector<std::string> CAudioDriverProvider::getAllOutputChannels() {
	std::vector<std::string> result;

	if (out_devices.size() == 0) {
		result.push_back("<no driver>");
		return result;
	}

	char pc[32];
	for (int i = 0; i < zzub_device_info_get_supported_output_channels(out_devices[outDeviceIndex]) / 2; i++) {
		sprintf(pc, "Channel %i/%i", i*2, (i*2)+1);
		result.push_back(pc);
	}

	return result;
}

std::vector<std::string> CAudioDriverProvider::getAvailableOutputChannels() {
	// TODO: check the result of of getFirstOutput and cut off the beginning accordingly
	return getAllOutputChannels();
}

int CAudioDriverProvider::getFirstOutputChannel() {
	return 0;
}

bool CAudioDriverProvider::setFirstOutputChannel(int) {
	// TODO: Update dropdowns that use for getAvailableOutputChannels
	return true;
}

int CAudioDriverProvider::getLastOutputChannel() {
	return 0;
}

bool CAudioDriverProvider::setLastOutputChannel(int) {
	return true;
}

int CAudioDriverProvider::getMixerThreads() {
	return mixerThreadCount;
}

bool CAudioDriverProvider::setMixerThreads(int count) {
	if (count < 1 || count > 100) return false;
	dirty = true;
	mixerThreadCount = count;
	return true;
}


/***

	CMidiConfigProvider

***/


CMidiConfigProvider::CMidiConfigProvider(CPropertyView* view, CViewFrame* mainFrame):CPreferencePropertyProvider(view) {

	this->mainFrame = mainFrame;
	
	buze_document_t* document = buze_main_frame_get_document(mainFrame);
	configuration = buze_document_get_configuration(document);

	buze_application_t* app = buze_main_frame_get_application(mainFrame);
	driver = (zzub_audiodriver_t*)buze_application_get_audio_driver(app);

	createProperties();
}

void CMidiConfigProvider::createProperties() {

	destroyProperties();

	std::vector<std::string> midiNames;
	configuration->getMidiOutputs(midiNames);

	properties.push_back(new CategoryPropertyInfo("Enable MIDI Output Devices"));

	for (size_t i = 0; i < zzub_mididriver_get_count(buze_main_frame_get_player(mainFrame)); i++) {
		if (zzub_mididriver_is_output(buze_main_frame_get_player(mainFrame), i)) {
			const char* name = zzub_mididriver_get_name(buze_main_frame_get_player(mainFrame), i);
			CMidiDeviceProperty* midiDevice = new CMidiDeviceProperty();
			midiDevice->provider = this;
			std::vector<std::string>::iterator ii = std::find(midiNames.begin(), midiNames.end(), name);
			midiDevice->enabled = ii != midiNames.end();
			midiDevice->index = i;
			properties.push_back(new BoolPropertyInfo<CMidiDeviceProperty>(midiDevice, name, &CMidiDeviceProperty::getValue, &CMidiDeviceProperty::setValue));
			out_devices.push_back(midiDevice);
		}
	}

	midiNames.clear();
	configuration->getMidiInputs(midiNames);

	properties.push_back(new CategoryPropertyInfo("Enable MIDI Input Devices"));

	for (size_t i = 0; i < zzub_mididriver_get_count(buze_main_frame_get_player(mainFrame)); i++) {
		if (zzub_mididriver_is_input(buze_main_frame_get_player(mainFrame), i)) {
			const char* name = zzub_mididriver_get_name(buze_main_frame_get_player(mainFrame), i);
			CMidiDeviceProperty* midiDevice = new CMidiDeviceProperty();
			midiDevice->provider = this;
			std::vector<std::string>::iterator ii = std::find(midiNames.begin(), midiNames.end(), name);
			midiDevice->enabled = ii != midiNames.end();
			midiDevice->index = i;
			properties.push_back(new BoolPropertyInfo<CMidiDeviceProperty>(midiDevice, name, &CMidiDeviceProperty::getValue, &CMidiDeviceProperty::setValue));
			in_devices.push_back(midiDevice);
		}
	}

}


int CMidiConfigProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CMidiConfigProvider::getProperty(int index) {
	return properties[index];
}

bool CMidiConfigProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);
	return false;
}

void CMidiConfigProvider::doDataExchange(bool save) {
	if (save) {
		std::vector<std::string> midiNames;
		for (size_t i = 0; i < in_devices.size(); i++) {
			if (in_devices[i]->enabled)
				midiNames.push_back(zzub_mididriver_get_name(buze_main_frame_get_player(mainFrame), in_devices[i]->index));
		}

		configuration->setMidiInputs(midiNames);


		midiNames.clear();
		for (size_t i = 0; i < out_devices.size(); i++) {
			if (out_devices[i]->enabled)
				midiNames.push_back(zzub_mididriver_get_name(buze_main_frame_get_player(mainFrame), out_devices[i]->index));
		}

		configuration->setMidiOutputs(midiNames);

		zzub_mididriver_close_all(buze_main_frame_get_player(mainFrame));
		buze_application_t* app = buze_main_frame_get_application(mainFrame);
		buze_application_open_midi_devices_from_config(app);
		//_Module.openMidiDevices();
		dirty = false;
	}
	
}

bool CMidiDeviceProperty::setValue(BOOL value) {
	enabled = value!=0?true:false;
	provider->dirty = true;
	return true;
}

BOOL CMidiDeviceProperty::getValue() {
	return enabled;
}



/***

	CThemeConfigProvider

***/



CThemeConfigProvider::CThemeConfigProvider(CPropertyView* view, CViewFrame* mainFrame):CPreferencePropertyProvider(view) {
	this->mainFrame = mainFrame;
	createProperties();
}

void CThemeConfigProvider::createProperties() {
	destroyProperties();
/*
	currentTheme = _Module.themes->currentTheme;

	properties.push_back(new CategoryPropertyInfo("Selected Theme"));
	properties.push_back(new StringListPropertyInfo<CThemeConfigProvider>(this, "Theme", &CThemeConfigProvider::getTheme, &CThemeConfigProvider::setTheme, &CThemeConfigProvider::getThemes));

	properties.push_back(new CategoryPropertyInfo("Machine View Colors"));
	fillThemes("MV");

	properties.push_back(new CategoryPropertyInfo("Pattern Editor Colors"));
	fillThemes("PE");

	properties.push_back(new CategoryPropertyInfo("SA Colors"));
	fillThemes("SA");

	properties.push_back(new CategoryPropertyInfo("Sequence Editor Colors"));
	fillThemes("SE");*/
}

void CThemeConfigProvider::fillThemes(std::string prefix) {

	for (size_t i = 0; i < ThemeManager::defaultThemeCount; i++) {
		CThemeColorProperty* ctp = new CThemeColorProperty();
		std::string name = ThemeManager::defaultTheme[i].name;
		if (name.find(prefix) != 0) continue;
		ctp->provider = this;
		ctp->name = name;
		
		properties.push_back(new ColorPropertyInfo<CThemeColorProperty>(ctp, name.c_str(), &CThemeColorProperty::getValue, &CThemeColorProperty::setValue));
//		properties.push_back(new StringPropertyInfo<void*, CThemeColorProperty>(ctp, name.c_str(), &CThemeColorProperty::getValue, &CThemeColorProperty::setValue));
	}
}


int CThemeConfigProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CThemeConfigProvider::getProperty(int index) {
	return properties[index];
}

bool CThemeConfigProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);
	return false;
}

void CThemeConfigProvider::doDataExchange(bool save) {
	if (!save) {
		std::vector<std::string> themes = getThemes();
		std::vector<std::string>::iterator i = find(themes.begin(), themes.end(), currentTheme);
		if (i == themes.end()) 
			themeIndex = 0; else
			themeIndex = (int)(i - themes.begin());
	} else {
		MessageBox(0, "CThemeConfigProvider::doDataExchange(true) Not implemented", "Not implemented", MB_OK);
		// save current theme!
		// we maybe also call this when the user switches theme after a change was made
	}
}

void CThemeConfigProvider::deleteProperty(int index) {
	MessageBox(0, "Are you sure you want to delete the theme '$themeName'?", "Delete theme?", MB_YESNO);
}

int CThemeConfigProvider::getTheme() {
	return themeIndex;
}

bool CThemeConfigProvider::setTheme(int index) {
	if (index == themeIndex) return true;

	if (IDYES == MessageBox(GetForegroundWindow()/*mainFrame->m_hWnd*/, "Save modified theme?", "Save theme?", MB_YESNO)) {
		doDataExchange(true);
	}
/*
	themeIndex = index;
	std::vector<std::string> themes = getThemes();
	currentTheme = themes[index];
	_Module.themes->loadTheme(currentTheme);

	view->BindProvider();*/
	return true;
}

std::vector<std::string> CThemeConfigProvider::getThemes() {
	std::vector<std::string> temp;
	return temp;
	//return _Module.themes->themes;
}


bool CThemeColorProperty::setValue(COLORREF value) {

	provider->dirty = true;
	return true;
}

COLORREF CThemeColorProperty::getValue() {
	COLORREF color; // = _Module.themes->getThemeColor(name);
	return color;
}


/***

	CGuiConfigProvider

***/

CGuiConfigProvider::CGuiConfigProvider(CPropertyView* view, CViewFrame* mainFrame):CPreferencePropertyProvider(view) {
	this->mainFrame = mainFrame;
	
	buze_document_t* document = buze_main_frame_get_document(mainFrame);
	configuration = buze_document_get_configuration(document);

	enumFixedWidthFonts();
	createProperties();
}

void CGuiConfigProvider::createProperties() {
	destroyProperties();
	properties.push_back(new CategoryPropertyInfo("Globals"));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "VU Meter Speed", &CGuiConfigProvider::getVUDropSpeed, &CGuiConfigProvider::setVUDropSpeed, &CGuiConfigProvider::getVUDropSpeeds));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "VU Update Rate", &CGuiConfigProvider::getVUTimerSpeed, &CGuiConfigProvider::setVUTimerSpeed, &CGuiConfigProvider::getVUTimerSpeeds));
	properties.push_back(new BoolPropertyInfo<CGuiConfigProvider>(this, "Show Accelerators", &CGuiConfigProvider::getShowAccelerators, &CGuiConfigProvider::setShowAccelerators));
	properties.push_back(new BoolPropertyInfo<CGuiConfigProvider>(this, "Create Default Pattern Player", &CGuiConfigProvider::getDefaultDocumentAdvanced, &CGuiConfigProvider::setDefaultDocumentAdvanced));

	properties.push_back(new CategoryPropertyInfo("Machine View"));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Default Zoom", &CGuiConfigProvider::getZoom, &CGuiConfigProvider::setZoom, &CGuiConfigProvider::getZooms));
	properties.push_back(new BoolPropertyInfo<CGuiConfigProvider>(this, "Disable Machine Skins", &CGuiConfigProvider::getMachineSkins, &CGuiConfigProvider::setMachineSkins));
	properties.push_back(new BoolPropertyInfo<CGuiConfigProvider>(this, "Scale by Window Size", &CGuiConfigProvider::getScaleByWindowSize, &CGuiConfigProvider::setScaleByWindowSize));
	properties.push_back(new BoolPropertyInfo<CGuiConfigProvider>(this, "Machines Default Minimized", &CGuiConfigProvider::getMachinesMinimized, &CGuiConfigProvider::setMachinesMinimized));

	properties.push_back(new CategoryPropertyInfo("Pattern View"));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Font", &CGuiConfigProvider::getFont, &CGuiConfigProvider::setFont, &CGuiConfigProvider::getFonts));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Font Size", &CGuiConfigProvider::getFontSize, &CGuiConfigProvider::setFontSize, &CGuiConfigProvider::getFontSizes));
	properties.push_back(new IntPropertyInfo<CGuiConfigProvider>(this, "Default Pattern Length", &CGuiConfigProvider::getPatternLength, &CGuiConfigProvider::setPatternLength));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Default Value Entry Mode", &CGuiConfigProvider::getDefaultEntryMode, &CGuiConfigProvider::setDefaultEntryMode, &CGuiConfigProvider::getDefaultEntryModes));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Horizontal Scroll Mode", &CGuiConfigProvider::getHorizontalScrollMode, &CGuiConfigProvider::setHorizontalScrollMode, &CGuiConfigProvider::getHorizontalScrollModes));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Vertical Scroll Mode", &CGuiConfigProvider::getVerticalScrollMode, &CGuiConfigProvider::setVerticalScrollMode, &CGuiConfigProvider::getVerticalScrollModes));
	properties.push_back(new BoolPropertyInfo<CGuiConfigProvider>(this, "Sticky Selection", &CGuiConfigProvider::getStickySelection, &CGuiConfigProvider::setStickySelection));
	properties.push_back(new StringPropertyInfo<CGuiConfigProvider>(this, "Note-Off String", &CGuiConfigProvider::getNoteOffString, &CGuiConfigProvider::setNoteOffString));
	properties.push_back(new StringPropertyInfo<CGuiConfigProvider>(this, "Note-Cut String", &CGuiConfigProvider::getNoteCutString, &CGuiConfigProvider::setNoteCutString));
	properties.push_back(new StringPropertyInfo<CGuiConfigProvider>(this, "Background for a Note", &CGuiConfigProvider::getBGNote, &CGuiConfigProvider::setBGNote));
	properties.push_back(new StringPropertyInfo<CGuiConfigProvider>(this, "Background for a Byte", &CGuiConfigProvider::getBGByte, &CGuiConfigProvider::setBGByte));
	properties.push_back(new StringPropertyInfo<CGuiConfigProvider>(this, "Background for a Switch", &CGuiConfigProvider::getBGSwitch, &CGuiConfigProvider::setBGSwitch));
	properties.push_back(new StringPropertyInfo<CGuiConfigProvider>(this, "Background for a Word", &CGuiConfigProvider::getBGWord, &CGuiConfigProvider::setBGWord));
	properties.push_back(new IntPropertyInfo<CGuiConfigProvider>(this, "Trigger Column Width", &CGuiConfigProvider::getTriggerWidth, &CGuiConfigProvider::setTriggerWidth));
	properties.push_back(new BoolPropertyInfo<CGuiConfigProvider>(this, "Colored Notes", &CGuiConfigProvider::getColoredNotes, &CGuiConfigProvider::setColoredNotes));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Format/Pattern Creation Mode", &CGuiConfigProvider::getFormatPatternCreationMode, &CGuiConfigProvider::setFormatPatternCreationMode, &CGuiConfigProvider::getFormatPatternCreationModes));
	properties.push_back(new IntPropertyInfo<CGuiConfigProvider>(this, "Default Scroller Width", &CGuiConfigProvider::getDefaultScrollerWidth, &CGuiConfigProvider::setDefaultScrollerWidth));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Pattern Naming Mode", &CGuiConfigProvider::getPatternNamingMode, &CGuiConfigProvider::setPatternNamingMode, &CGuiConfigProvider::getPatternNamingModes));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Right Click Mode", &CGuiConfigProvider::getPatternRightClickMode, &CGuiConfigProvider::setPatternRightClickMode, &CGuiConfigProvider::getPatternRightClickModes));
	properties.push_back(new StringListPropertyInfo<CGuiConfigProvider>(this, "Subrow Naming Mode", &CGuiConfigProvider::getSubrowNamingMode, &CGuiConfigProvider::setSubrowNamingMode, &CGuiConfigProvider::getSubrowNamingModes));
	properties.push_back(new BoolPropertyInfo<CGuiConfigProvider>(this, "Remember Cursor/Scroll By Format", &CGuiConfigProvider::getPatternFormatPositionCache, &CGuiConfigProvider::setPatternFormatPositionCache));

//	properties.push_back(new CategoryPropertyInfo("Parameter View"));
//	properties.push_back(new BoolPropertyInfo<CGuiConfigProvider>(this, "Always Open In Popup", &CGuiConfigProvider::getParameterPopup, &CGuiConfigProvider::setParameterPopup));

	properties.push_back(new CategoryPropertyInfo("Wavetable"));
	properties.push_back(new StringPropertyInfo<CGuiConfigProvider>(this, "Default Wave Editor", &CGuiConfigProvider::getExternalWaveEditor, &CGuiConfigProvider::setExternalWaveEditor));

	fontIndex = -1;
	fontSizeIndex = -1;
	zoomIndex = -1;
	VUDropSpeedIndex = -1;
	VUTimerSpeedIndex = -1;
	defaultEntryModeIndex = -1;
	verticalScrollModeIndex = -1;
	horizontalScrollModeIndex = -1;
	patternNamingModeIndex = -1;
	patternRightClickModeIndex = -1;
	subrowNamingModeIndex = -1;
}

int CGuiConfigProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CGuiConfigProvider::getProperty(int index) {
	return properties[index];
}

bool CGuiConfigProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);
	return false;
}

void CGuiConfigProvider::doDataExchange(bool save) {
	if (!save) {
		float vs = configuration->getVUDropSpeed() * 100;
		VUDropSpeedIndex = (int)vs;

		VUTimerSpeedIndex = configuration->getVUTimerSpeed();

		float scale = (configuration->getMachineScale() - 0.1f) * 10.0f;
		zoomIndex = (int)floor(scale + 0.5f);
		machineSkins = configuration->getMachineSkinVisibility();
		scaleByWindowSize = configuration->getScaleByWindowSize();
		machinesMinimized = configuration->getMachinesMinimized();
		stickySelection = configuration->getStickySelections();

		std::string fontName = buze_configuration_get_fixed_width_font(configuration.configuration);
		std::vector<std::string> fonts = getFonts();
		std::vector<std::string>::iterator i = find(fonts.begin(), fonts.end(), fontName);
		if (i == fonts.end()) 
			fontIndex = 0; else
			fontIndex = (int)(i - fonts.begin());

		int pointsize = 9;
		configuration->getNumber("Settings", "FixedWidthFontSize", (DWORD*)&pointsize);
		std::string ptstr = stringFromInt(pointsize);
		std::vector<std::string> sizes = getFontSizes();
		i = find(sizes.begin(), sizes.end(), ptstr);
		if (i == sizes.end()) 
			fontSizeIndex = 0; else
			fontSizeIndex = (int)(i - sizes.begin());

		showAccelerators = configuration->getShowAccelerators();

		patternLength = configuration->getGlobalPatternLength();

		paramPopup = configuration->getParameterPopupFlag()?TRUE:FALSE;
		waveEditor = configuration->getExternalWaveEditor();

		bg_byte = configuration->getBGByte();
		bg_word = configuration->getBGWord();
		bg_switch = configuration->getBGSwitch();
		bg_note = configuration->getBGNote();
		noteOffStr = configuration->getNoteOffString();
		noteCutStr = configuration->getNoteCutString();

		defaultEntryModeIndex = configuration->getDefaultEntryMode();

		horizontalScrollModeIndex = configuration->getHorizontalScrollMode();
		verticalScrollModeIndex = configuration->getVerticalScrollMode();

		triggerWidth = configuration->getTriggerWidth();

		coloredNotes = configuration->getColoredNotes();

		formatPatternCreationMode = configuration->getFormatPatternCreationMode();

		defaultScrollerWidth = configuration->getDefaultScrollerWidth();

		patternNamingModeIndex = configuration->getPatternNamingMode();

		patternRightClickModeIndex = configuration->getPatternRightClickMode();

		subrowNamingModeIndex = configuration->getSubrowNamingMode();

		defaultDocumentAdvanced = configuration->getAdvancedDefaultDocument();

		patternFormatPositionCache = configuration->getPatternPositionCachePatternFormat();
		dirty = false;
	} else {
		configuration->setVUDropSpeed(float(VUDropSpeedIndex) / 100);
		configuration->setVUTimerSpeed(VUTimerSpeedIndex);
		configuration->setMachineScale( (double)zoomIndex / 10 + 0.1);
		configuration->setMachineSkinVisibility(machineSkins);
		configuration->setScaleByWindowSize(scaleByWindowSize);
		configuration->setMachinesMinimized(machinesMinimized);
		configuration->setStickySelections(stickySelection);

		if (fontIndex < fixedWidthSizes.size() && fontSizeIndex < fixedWidthSizes[fontIndex].size()) {
			std::vector<std::string> fonts = getFonts();
			configuration->setString("Settings", "FixedWidthFont", fonts[fontIndex].c_str());

			int pointsize = fixedWidthSizes[fontIndex][fontSizeIndex];
			configuration->setNumber("Settings", "FixedWidthFontSize", (DWORD)pointsize);
		}

		configuration->setShowAccelerators(showAccelerators);

		configuration->setGlobalPatternLength(patternLength);

		configuration->setParameterPopupFlag(paramPopup);
		configuration->setExternalWaveEditor(waveEditor);

		configuration->setNoteOffString(noteOffStr);
		configuration->setNoteCutString(noteCutStr);
		configuration->setBGByte(bg_byte);
		configuration->setBGNote(bg_note);
		configuration->setBGWord(bg_word);
		configuration->setBGSwitch(bg_switch);
		
		configuration->setDefaultEntryMode(defaultEntryModeIndex);

		configuration->setHorizontalScrollMode(horizontalScrollModeIndex);
		configuration->setVerticalScrollMode(verticalScrollModeIndex);
		
		configuration->setTriggerWidth(triggerWidth);
		
		configuration->setColoredNotes(coloredNotes);
		
		configuration->setFormatPatternCreationMode(formatPatternCreationMode);

		configuration->setDefaultScrollerWidth(defaultScrollerWidth);

		configuration->setPatternNamingMode(patternNamingModeIndex);

		configuration->setPatternRightClickMode(patternRightClickModeIndex);

		configuration->setSubrowNamingMode(subrowNamingModeIndex);
		configuration->setAdvancedDefaultDocument(defaultDocumentAdvanced);

		configuration->setPatternPositionCachePatternFormat(patternFormatPositionCache);

		buze_document_notify_views(buze_main_frame_get_document(mainFrame), 0, buze_event_type_update_settings, 0);
		dirty = false;

	}
}

int CGuiConfigProvider::getVUDropSpeed() {
	return VUDropSpeedIndex;
}

bool CGuiConfigProvider::setVUDropSpeed(int i) {
	dirty = true;
	VUDropSpeedIndex = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getVUDropSpeeds() {
	std::vector<std::string> result;

	result.push_back("Disabled");
	result.push_back("Slowest");
	result.push_back("Slower");
	result.push_back("Slow");
	result.push_back("Steady");
	result.push_back("Fast");
	result.push_back("Faster");
	result.push_back("Fastest");

	return result;
}

int CGuiConfigProvider::getVUTimerSpeed() {
	return VUTimerSpeedIndex;
}

bool CGuiConfigProvider::setVUTimerSpeed(int i) {
	dirty = true;
	VUTimerSpeedIndex = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getVUTimerSpeeds() {
	std::vector<std::string> result;

	result.push_back("10ms");
	result.push_back("20ms");
	result.push_back("40ms");
	result.push_back("80ms");

	return result;
}

int CGuiConfigProvider::getDefaultEntryMode() {
	return defaultEntryModeIndex;
}

bool CGuiConfigProvider::setDefaultEntryMode(int i) {
	dirty = true;
	defaultEntryModeIndex = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getDefaultEntryModes() {
	std::vector<std::string> result;

	result.push_back("Vertical");
	result.push_back("Horizontal");

	return result;
}

int CGuiConfigProvider::getHorizontalScrollMode() {
	return horizontalScrollModeIndex;
}

bool CGuiConfigProvider::setHorizontalScrollMode(int i) {
	dirty = true;
	horizontalScrollModeIndex = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getHorizontalScrollModes() {
	std::vector<std::string> result;

	result.push_back("Regular");
	result.push_back("Center on Column");

	return result;
}

int CGuiConfigProvider::getVerticalScrollMode() {
	return verticalScrollModeIndex;
}

bool CGuiConfigProvider::setVerticalScrollMode(int i) {
	dirty = true;
	verticalScrollModeIndex = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getVerticalScrollModes() {
	std::vector<std::string> result;

	result.push_back("Regular");
	result.push_back("Center on Row");

	return result;
}

int CGuiConfigProvider::getFormatPatternCreationMode() {
	return formatPatternCreationMode;
}

bool CGuiConfigProvider::setFormatPatternCreationMode(int i) {
	dirty = true;
	formatPatternCreationMode = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getFormatPatternCreationModes() {
	std::vector<std::string> result;

	result.push_back("No Formats/Patterns");
	result.push_back("Gen Format");
	result.push_back("Gen Format+Pattern");
	result.push_back("Gen+FX Formats");
	result.push_back("Gen+FX Formats+Patterns");
	result.push_back("Gen Format+Pattern, FX Format");

	return result;
}

int CGuiConfigProvider::getPatternNamingMode() {
	return patternNamingModeIndex;
}

bool CGuiConfigProvider::setPatternNamingMode(int i) {
	dirty = true;
	patternNamingModeIndex = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getPatternNamingModes() {
	std::vector<std::string> result;

	result.push_back("00");
	result.push_back("00 xxx");

	return result;
}

int CGuiConfigProvider::getPatternRightClickMode() {
	return patternRightClickModeIndex;
}

bool CGuiConfigProvider::setPatternRightClickMode(int i) {
	dirty = true;
	patternRightClickModeIndex = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getPatternRightClickModes() {
	std::vector<std::string> result;

	result.push_back("Pattern Operations");
	result.push_back("Edit Operations");

	return result;
}

int CGuiConfigProvider::getSubrowNamingMode() {
	return subrowNamingModeIndex;
}

bool CGuiConfigProvider::setSubrowNamingMode(int i) {
	dirty = true;
	subrowNamingModeIndex = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getSubrowNamingModes() {
	std::vector<std::string> result;

	result.push_back("Fractions");
	result.push_back("Beats");

	return result;
}

int CGuiConfigProvider::getZoom() {
	return zoomIndex;
}

bool CGuiConfigProvider::setZoom(int i) {
	dirty = true;
	zoomIndex = i;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getZooms() {
	std::vector<std::string> result;
	char pc[32];
	for (int i = 0; i < 20; i++) {
		float zoom = 0.1f + (float)i * 0.1f;
		sprintf(pc, "%i%%", (int)floor(zoom * 100 + 0.5));
		result.push_back(pc);
	}

	return result;
}

BOOL CGuiConfigProvider::getMachineSkins() {
	// note we return the opposite result
	return machineSkins?FALSE:TRUE;
}

bool CGuiConfigProvider::setMachineSkins(BOOL value) {
	dirty = true;
	// note we set the opposite value
	machineSkins = value?false:true;
	return true;
}

BOOL CGuiConfigProvider::getScaleByWindowSize() {
	return scaleByWindowSize?TRUE:FALSE;
}

bool CGuiConfigProvider::setScaleByWindowSize(BOOL value) {
	dirty = true;
	scaleByWindowSize = value?true:false;
	return true;
}

BOOL CGuiConfigProvider::getMachinesMinimized() {
	return machinesMinimized?TRUE:FALSE;
}

bool CGuiConfigProvider::setMachinesMinimized(BOOL value) {
	dirty = true;
	machinesMinimized = value?true:false;
	return true;
}

int CGuiConfigProvider::getFont() {
	return fontIndex;
}

bool CGuiConfigProvider::setFont(int index) {
	dirty = true;
	fontIndex = index;
	fontSizeIndex = 0;
	view->BindProvider();
	return true;
}

// http://www.catch22.net/tuts/enumfont.asp

int CGuiConfigProvider::FontNameProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, int FontType, LPARAM lParam) {
	CGuiConfigProvider* provider = (CGuiConfigProvider*)lParam;

	if (lpelfe->elfLogFont.lfPitchAndFamily & FIXED_PITCH) {
		//printf("hello %s\n", lpelfe->elfFullName);
		int  logsize = lpntme->ntmTm.tmHeight - lpntme->ntmTm.tmInternalLeading;
		long pointsize  = MulDiv(logsize, 72, GetDeviceCaps(provider->enumFontDC, LOGPIXELSY));

		std::string name = (const char*)lpelfe->elfFullName;

		//printf("vi har en fontsize på %i med %s (%i chars)\n", pointsize, name.c_str(), name.length());

		std::vector<std::string>::iterator fi = find(provider->fixedWidthFonts.begin(), provider->fixedWidthFonts.end(), name);
		if (fi != provider->fixedWidthFonts.end()) {
			// hvis truetype, skip her, ellers legg til vår size
			return 1;
		} else  {
			provider->fixedWidthFonts.push_back(name);
			provider->fixedWidthSizes.push_back(std::vector<int>());
			// hvis truetype, så legg inn anne fra sizearrayen
		}

		// enum font sizes

		provider->enumFixedWidthSizes(name);
	}

	return 1;
}


int CGuiConfigProvider::FontSizeProc(ENUMLOGFONTEX* lpelfe, TEXTMETRIC *ptm, int FontType, LPARAM lParam) {
	static int truetypesize[] = { 8, 9, 10, 11, 12, 14, 16, 18, 20,  22, 24, 26, 28, 36, 48, 72 };
	static int truetypesizes = boost::range_detail::array_size(truetypesize);

	CGuiConfigProvider* provider = (CGuiConfigProvider*)lParam;
	int i;

	if (FontType != TRUETYPE_FONTTYPE) {
		//printf("found size for non-truetype\n");
		int  logsize = ptm->tmHeight - ptm->tmInternalLeading;
		long pointsize  = MulDiv(logsize, 72, GetDeviceCaps(provider->enumFontDC, LOGPIXELSY));

		std::vector<int>::iterator fi = find(provider->fixedWidthSizes.back().begin(), provider->fixedWidthSizes.back().end(), pointsize);
		if (fi != provider->fixedWidthSizes.back().end()) return 1;

		provider->fixedWidthSizes.back().push_back(pointsize);
		return 1;   
	} else {
		//printf("found sizes for truetype\n");
		for (i = 0; i < truetypesizes; i++) {
			provider->fixedWidthSizes.back().push_back(truetypesize[i]);
		}
		return 0;
	}
	return 1;
}

void CGuiConfigProvider::enumFixedWidthFonts() {
	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));

	logfont.lfCharSet = DEFAULT_CHARSET;
	logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
	lstrcpy(logfont.lfFaceName, "\0");

	enumFontDC = GetDC(0);

	EnumFontFamiliesEx(enumFontDC, &logfont, (FONTENUMPROC)FontNameProc, (LPARAM)this, 0);

	ReleaseDC(0, enumFontDC);
}

void CGuiConfigProvider::enumFixedWidthSizes(std::string name) {
	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));

	logfont.lfCharSet = DEFAULT_CHARSET;
	logfont.lfPitchAndFamily = 0;//FIXED_PITCH | FF_DONTCARE;
	lstrcpyn(logfont.lfFaceName, name.c_str(), LF_FACESIZE);

	HDC hdc = GetDC(0);

	//printf("enumerating font sizes for %s\n", name.c_str());
	EnumFontFamiliesEx(hdc, &logfont, (FONTENUMPROC)FontSizeProc, (LPARAM)this, 0);

	ReleaseDC(0, hdc);

	for (std::vector<std::vector<int> >::iterator i = fixedWidthSizes.begin(); i != fixedWidthSizes.end(); ++i) {
		std::sort(i->begin(), i->end());
	}
}

std::vector<std::string> CGuiConfigProvider::getFonts() {

	if (fixedWidthFonts.size() == 0) fixedWidthFonts.push_back("");
	return fixedWidthFonts;
}

int CGuiConfigProvider::getFontSize() {
	return fontSizeIndex;
}

bool CGuiConfigProvider::setFontSize(int index) {
	dirty = true;
	fontSizeIndex = index;
	return true;
}

std::vector<std::string> CGuiConfigProvider::getFontSizes() {
	int font = getFont();
	std::vector<std::string> result;

	if (font == -1) {
		return result;
	}

	for (size_t i = 0; i < fixedWidthSizes[font].size(); i++) {
		result.push_back(stringFromInt(fixedWidthSizes[font][i]));
	}
	return result;
}

BOOL CGuiConfigProvider::getShowAccelerators() {
	return showAccelerators;
}

bool CGuiConfigProvider::setShowAccelerators(BOOL value) {
	dirty = true;
	showAccelerators = value != 0;
	return true;
}

int CGuiConfigProvider::getPatternLength() {
	return patternLength;
}

bool CGuiConfigProvider::setPatternLength(int value) {
	dirty = true;
	patternLength = value;
	return true;
}

BOOL CGuiConfigProvider::getStickySelection() {
	return stickySelection?TRUE:FALSE;
}

bool CGuiConfigProvider::setStickySelection(BOOL value) {
	dirty = true;
	stickySelection = value?true:false;
	return true;
}

BOOL CGuiConfigProvider::getParameterPopup() {
	return paramPopup?TRUE:FALSE;
}

bool CGuiConfigProvider::setParameterPopup(BOOL value) {
	dirty = true;
	paramPopup = value?true:false;
	return true;
}

std::string CGuiConfigProvider::getExternalWaveEditor() {
	return waveEditor;
}

bool CGuiConfigProvider::setExternalWaveEditor(std::string value) {
	dirty = true;
	waveEditor = value;
	return true;
}

std::string CGuiConfigProvider::getNoteOffString() {
	return noteOffStr;
}

bool CGuiConfigProvider::setNoteOffString(std::string value) {
	dirty = true;
	noteOffStr = value;
	return true;
}

std::string CGuiConfigProvider::getNoteCutString() {
	return noteCutStr;
}

bool CGuiConfigProvider::setNoteCutString(std::string value) {
	dirty = true;
	noteCutStr = value;
	return true;
}

std::string CGuiConfigProvider::getBGNote() {
	return bg_note;
}

bool CGuiConfigProvider::setBGNote(std::string value) {
	dirty = true;
	bg_note = value;
	return true;
}

std::string CGuiConfigProvider::getBGByte() {
	return bg_byte;
}

bool CGuiConfigProvider::setBGByte(std::string value) {
	dirty = true;
	bg_byte = value;
	return true;
}

std::string CGuiConfigProvider::getBGSwitch() {
	return bg_switch;
}

bool CGuiConfigProvider::setBGSwitch(std::string value) {
	dirty = true;
	bg_switch = value;
	return true;
}

std::string CGuiConfigProvider::getBGWord() {
	return bg_word;
}

bool CGuiConfigProvider::setBGWord(std::string value) {
	dirty = true;
	bg_word = value;
	return true;
}

int CGuiConfigProvider::getTriggerWidth() {
	return triggerWidth;
}

bool CGuiConfigProvider::setTriggerWidth(int value) {
	dirty = true;
	triggerWidth = value;
	return true;
}

BOOL CGuiConfigProvider::getColoredNotes() {
	return coloredNotes;
}

bool CGuiConfigProvider::setColoredNotes(BOOL value) {
	dirty = true;
	coloredNotes = value != 0;
	return true;
}

int CGuiConfigProvider::getDefaultScrollerWidth() {
	return defaultScrollerWidth;
}

bool CGuiConfigProvider::setDefaultScrollerWidth(int value) {
	dirty = true;
	defaultScrollerWidth = value;
	return true;
}

BOOL CGuiConfigProvider::getDefaultDocumentAdvanced() {
	return defaultDocumentAdvanced?TRUE:FALSE;
}

bool CGuiConfigProvider::setDefaultDocumentAdvanced(BOOL value) {
	dirty = true;
	defaultDocumentAdvanced = value != 0;
	return true;
}

BOOL CGuiConfigProvider::getPatternFormatPositionCache() {
	return patternFormatPositionCache ? TRUE : FALSE;
}

bool CGuiConfigProvider::setPatternFormatPositionCache(BOOL value) {
	dirty = true;
	patternFormatPositionCache = value != 0;
	return true;
}



/***

	CKeyboardConfigProvider

***/

bool CKeyboardProperty::setValue(std::string value) {
	provider->dirty = true;
	return true;
}

std::string CKeyboardProperty::getValue() {
	std::stringstream strm;
	if (accelData[accelIndex].fVirt & FCONTROL)
		strm << "Ctrl + ";

	if (accelData[accelIndex].fVirt & FSHIFT)
		strm << "Shift + ";

	if (accelData[accelIndex].fVirt & FALT)
		strm << "Alt + ";

	if (accelData[accelIndex].fVirt & FVIRTKEY) {
		// key is a virtual key code, VK_* etc
		static char buffer[256];
		UINT scanCode = MapVirtualKey(accelData[accelIndex].key, 0) << 16;
		//UINT scanCode = accelData[accelIndex].key << 16;
		GetKeyNameText(scanCode, buffer, 256);
		strm << buffer;
	} else {
		// key is a char code
		strm << (char)accelData[accelIndex].key;
	}

	return strm.str();
}


CKeyboardConfigProvider::CKeyboardConfigProvider(CPropertyView* view, CViewFrame* mainFrame):CPreferencePropertyProvider(view) {
	this->mainFrame = mainFrame;
	createProperties();
}

void CKeyboardConfigProvider::createProperties() {
	destroyProperties();

	assert(false);
	/*properties.push_back(new CategoryPropertyInfo("Main Frame"));

	HACCEL accel = AtlLoadAccelerators(IDR_MAINFRAME);
	int numAccels = CopyAcceleratorTable(accel, 0, 0);
	ACCEL* accelData = new ACCEL[numAccels];
	CopyAcceleratorTable(accel, accelData, numAccels);

	for (int i = 0; i < numAccels; i++) {
		CKeyboardProperty* kbp = new CKeyboardProperty();
		
		kbp->name = getCommandText(accelData[i].cmd);
		kbp->accelData = accelData;
		kbp->accelIndex = i;
		

		properties.push_back(new StringPropertyInfo<CKeyboardProperty>(kbp, kbp->name.c_str(), &CKeyboardProperty::getValue, &CKeyboardProperty::setValue));
	}
*/

}

std::string CKeyboardConfigProvider::getCommandText(WORD cmd) {
	std::stringstream strm;
	//static char buffer[1024];
	std::vector<char> buffer(1024);
	AtlLoadString(cmd, &buffer.front(), 1024);
	std::vector<char>::iterator i = find(buffer.begin(), buffer.end(), '\n');
	if (i != buffer.end()) *i = '\0';
	strm << &buffer.front();
	return strm.str();
}

int CKeyboardConfigProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CKeyboardConfigProvider::getProperty(int index) {
	return properties[index];
}

bool CKeyboardConfigProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);
	return false;
}


CPluginConfigProvider::CPluginConfigProvider(CPropertyView* view, CViewFrame* mainFrame):CPreferencePropertyProvider(view) {
	this->mainFrame = mainFrame;
	
	buze_document_t* document = buze_main_frame_get_document(mainFrame);
	configuration = buze_document_get_configuration(document);

	createProperties();
}


void CPluginConfigProvider::createProperties() {
	destroyProperties();
	properties.push_back(new CategoryPropertyInfo("VST Plugin Wrapper"));
	properties.push_back(new StringPropertyInfo<CPluginConfigProvider>(this, "VST Plugin Directories (semicolon delimited paths, needs restart)", &CPluginConfigProvider::getVstPath, &CPluginConfigProvider::setVstPath));
}

int CPluginConfigProvider::getProperties() {
	return (int)properties.size();
}

PropertyInfoBase* CPluginConfigProvider::getProperty(int index) {
	return properties[index];
}

bool CPluginConfigProvider::checkUpdate(LPARAM lHint, LPVOID pHint) {
	assert(pHint != 0);
	return false;
}

void CPluginConfigProvider::doDataExchange(bool save) {
	if (!save) {
		vstPaths = configuration->getVstPaths();
		dirty = false;
	} else {
		configuration->setVstPaths(vstPaths);

		buze_document_notify_views(buze_main_frame_get_document(mainFrame), 0, buze_event_type_update_settings, 0);
		dirty = false;
	}
}

bool CPluginConfigProvider::setVstPath(std::string s) {
	vstPaths = s;
	dirty = true;
	return true;
}

std::string CPluginConfigProvider::getVstPath() {
	return vstPaths;
}
