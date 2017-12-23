#define __BUZZ2ZZUB__
#define NO_ZZUB_MIXER_TYPE

namespace zzub {
	struct mixer;
};
typedef struct zzub::mixer zzub_mixer_t;

#define _ATL_NO_UUIDOF
#include <atlbase.h>
#include <atlcom.h>
#include <atlconv.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <MidiFilter.h>
#include <zzub/plugin.h>
#include "plugincollection.h"
#include "mfxplugin.h"

using std::cerr;
using std::endl;

zzub::plugincollection* mfx2zzub_get_plugincollection() {
	return new mfxplugincollection();
}

zzub::plugin* mfxplugininfo::create_plugin() {
	CComObjectNoLock<mfxplugin>* plugin = new CComObjectNoLock<mfxplugin>();
	plugin->AddRef();
	plugin->info = this;
	return plugin;
}

bool mfxplugincollection::add_plugin(zzub::pluginfactory *factory, CLSID pluginClsid, std::string name, std::string filename) {
	USES_CONVERSION;

	mfxplugininfo* info = new mfxplugininfo();
	info->collection = this;
	info->pluginFile = filename;
	info->pluginClsid = pluginClsid;
	info->name = name;
	info->short_name = name;
	info->flags = zzub_plugin_flag_has_midi_input | zzub_plugin_flag_has_midi_output;
	info->min_tracks = 0;
	info->max_tracks = 0;
	info->inputs = 0;
	info->outputs = 0;

	WCHAR guidstr[128];
	StringFromGUID2(info->pluginClsid, guidstr, 128);
	std::stringstream uristrm;
	uristrm << "@zzub.org/mfx2zzub/" << W2A(guidstr);

	info->uri = uristrm.str();
	plugins.push_back(info);
	factory->register_info(info);
	return true;
}

void mfxplugincollection::initialize(zzub::pluginfactory *factory) {
	CRegKey pluginRootKey;

	if (ERROR_SUCCESS != pluginRootKey.Open(HKEY_LOCAL_MACHINE, SZ_MIDI_FILTER_REGKEY, KEY_READ)) {
		cerr << "mfx2zzub: no registry entry for mfx plugins" << endl;
		return ;
	}

	int subkeyIndex = 0;
	for (;;subkeyIndex++) {
		DWORD keySize = 1024;
		TCHAR keyName[1024];
		FILETIME keyTime;
		if (ERROR_SUCCESS != pluginRootKey.EnumKey(subkeyIndex, keyName, &keySize, &keyTime))
			break;

		CLSID pluginClsid;
		if FAILED(CLSIDFromString(CComBSTR(keyName), &pluginClsid)) {
			continue; // incorrect clsid format
		}

		// get the dll name, and then we know all there is; ie it has no parameters and there is nothing fancy to alter the flags

		TCHAR friendlyName[1024];
		TCHAR inprocName[1024];
		{
			std::stringstream keyStrm;
			keyStrm << "CLSID\\" << keyName;

			CRegKey pluginClassKey;
			if (ERROR_SUCCESS != pluginClassKey.Open(HKEY_CLASSES_ROOT, keyStrm.str().c_str(), KEY_READ)) {
				continue;
			}

			keySize = 1024;
			if (ERROR_SUCCESS != pluginClassKey.QueryStringValue("", friendlyName, &keySize)) {
				pluginClassKey.Close();
				continue;
			}
			pluginClassKey.Close();
		}

		{
			std::stringstream keyStrm;
			keyStrm << "CLSID\\" << keyName << "\\InProcServer32";

			CRegKey inprocClassKey;
			if (ERROR_SUCCESS != inprocClassKey.Open(HKEY_CLASSES_ROOT, keyStrm.str().c_str(), KEY_READ)) {
				continue;
			}
			keySize = 1024;
			if (ERROR_SUCCESS != inprocClassKey.QueryStringValue("", inprocName, &keySize)) {
				inprocClassKey.Close();
				continue;
			}
			inprocClassKey.Close();
		}


		add_plugin(factory, pluginClsid, friendlyName, inprocName);

	}

	pluginRootKey.Close();
}

