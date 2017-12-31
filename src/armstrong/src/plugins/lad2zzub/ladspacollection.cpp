/* lad2zzub.cpp

   Copyright (C) 2007 Frank Potulski (polac@gmx.de)
   Copyright (C) 2012 Armstrong Developers

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
   USA. */

#include <windows.h>
#include <list>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <zzub/plugin.h>
#include "../plugincache.h"
#include "ladspa/ladspa.h"
#include "ladspacollection.h"

//*****************************************************************************************************************************************************

typedef const LADSPA_Descriptor *(__cdecl *fnladspa_descriptor)(unsigned long);

const LADSPA_Descriptor* ladspa_get_descriptor(HMODULE hm, unsigned long index) {
	fnladspa_descriptor ladspa_descriptor = (const LADSPA_Descriptor *(__cdecl *)(unsigned long)) GetProcAddress(hm,"ladspa_descriptor");
	if (!ladspa_descriptor) return 0;

	const LADSPA_Descriptor* descriptor ;
	try {
		descriptor = ladspa_descriptor(index);
		if (!descriptor) return 0;
	} catch (...) {
		return 0;
	}

	return descriptor;
}

//**** CLadspaCollection ******************************************************************************************************************************

CLadspaInfo::CLadspaInfo(CLadspaCollection* coll, const std::string& path) : info() {
	owner = coll;
	collection = coll;
	pluginfile = path;
	lad_descriptor = 0;
	lad_index = 0;
	//lad_num_desc = 0;
	lad_uid = 0;

	version = zzub_version;
	flags = zzub_plugin_flag_has_audio_input | zzub_plugin_flag_has_audio_output;

	min_tracks = CLadspa::eMinTracks;
	max_tracks = CLadspa::eMaxTracks;	

	name = "Polac Ladspa Loader";
	short_name = "PLAD";
	author = "Frank Potulski";	
	uri = "@xlutop.com/lad2zzub";
	commands = "&About...";
}

CLadspaInfo::~CLadspaInfo() {
}

void CLadspaInfo::ReplaceChar(char *str,const char toReplace,const char replacedBy) {
	if ( str && toReplace!=replacedBy ) {
		while (*str) {
			char &cmp = *str;
			if (cmp==toReplace) {
				cmp = replacedBy;
			}
			++str;
		}			
	}
}

zzub::plugin *CLadspaInfo::create_plugin(void) {
	return new CLadspa(this);
}
	
bool CLadspaInfo::store_info(zzub::archive *arc) const {
	return false;
}

float CLadspaInfo::get_default_parameter_value(int idx, float *df, float *mi, float *mx) {
	if (!lad_descriptor) return -1.f;
	if (idx < 0) return -1.f;

	const LADSPA_PortRangeHintDescriptor *range = &lad_descriptor->PortRangeHints[idx].HintDescriptor;
	const LADSPA_Data *lower = &lad_descriptor->PortRangeHints[idx].LowerBound;
	const LADSPA_Data *upper = &lad_descriptor->PortRangeHints[idx].UpperBound;

	float def = 1.f;
	float min = 0.f;
	float max = 10.f;
	float ret = -1.f;

	if ( LADSPA_IS_HINT_BOUNDED_BELOW(*range) ) {
		min = *lower;
	}

	if ( LADSPA_IS_HINT_BOUNDED_ABOVE(*range) ) {
		max = *upper;
	}
					
	if ( LADSPA_IS_HINT_HAS_DEFAULT(*range) ) {
		if ( LADSPA_IS_HINT_DEFAULT_MINIMUM(*range) ) {
			def = min;

			ret = 0.f;
		} else if ( LADSPA_IS_HINT_DEFAULT_LOW(*range) ) {
			if ( LADSPA_IS_HINT_LOGARITHMIC(*range) ) {
				def = expf( logf(min) * 0.75f + logf(max) * 0.25f );
			}
			else {
				def = ( min * 0.75f ) + ( max * 0.25f );
			}

			ret = 0.25f;
		} else if ( LADSPA_IS_HINT_DEFAULT_MIDDLE(*range) ) {
			if ( LADSPA_IS_HINT_LOGARITHMIC(*range) ) {
				def = expf( logf(min) * 0.5f + logf(max) * 0.5f );
			} else {
				def = ( min * 0.5f ) + ( max * 0.5f );
			}

			ret = 0.5f;
		} else if ( LADSPA_IS_HINT_DEFAULT_HIGH(*range) ) {
			if ( LADSPA_IS_HINT_LOGARITHMIC(*range) ) {
				def = expf( logf(min) * 0.25f + logf(max) * 0.75f );
			} else {
				def = ( min * 0.25f ) + ( max * 0.75f );
			}

			ret = 0.75f;
		} else if ( LADSPA_IS_HINT_DEFAULT_MAXIMUM(*range) ) {
			def = max;
			ret = 1.f;
		} else if ( LADSPA_IS_HINT_DEFAULT_0(*range) ) {
			def = 0.f;
		} else if ( LADSPA_IS_HINT_DEFAULT_1(*range) ) {
			def = 1.f;
		} else if ( LADSPA_IS_HINT_DEFAULT_100(*range) ) {
			def = 100.f;
		} else if ( LADSPA_IS_HINT_DEFAULT_440(*range) ) {
			def = 440.f;
		}
	}
	
	if (def > max)  {
		def = max;
	} else if (def < min) {
		def = min;
	}
	
	if (ret==-1.f) {
		if ( LADSPA_IS_HINT_LOGARITHMIC(*range) ) {
			ret = ( logf(def) - logf(min) ) / ( logf(max) - logf(min) );
		} else {
			ret = 0.f;
			
			float f = (max-min);

			if (f) {
				ret = (def-min) / f;
			}
		}
	}

	/*if ( LADSPA_IS_HINT_SAMPLE_RATE(*range) ) {
		def *= sampleRate;
		min *= sampleRate;
		max *= sampleRate;
	}*/	
	
	if ( LADSPA_IS_HINT_INTEGER(*range) ) {
		def = (float)(int)def;
	}

	if (df) *df=def;
	if (mi) *mi=min;
	if (mx) *mx=max;

	return ret;
}

// fill_parameter_info = populate special info members specific to the wrapper
void CLadspaInfo::fill_parameter_info() {
	if (lad_parameters.size() > 0) return ;

	for (int i = 0; i < (int)lad_descriptor->PortCount; i++) {
		if (lad_descriptor->PortDescriptors[i]&LADSPA_PORT_CONTROL) {
			if (lad_descriptor->PortDescriptors[i]&LADSPA_PORT_INPUT) {
				// normal parameter
				PARAMS ladparam;
				float defvalue = get_default_parameter_value(i, 0, &ladparam.min, &ladparam.max);
				ladparam.hint = lad_descriptor->PortRangeHints[i].HintDescriptor;
				lad_parameters.push_back(ladparam);
			} else if (lad_descriptor->PortDescriptors[i] & LADSPA_PORT_OUTPUT) {
				// peer parameter
				PARAMS ladparam;
				float defvalue = get_default_parameter_value(i, 0, &ladparam.min, &ladparam.max);
				ladparam.hint = lad_descriptor->PortRangeHints[i].HintDescriptor;
				lad_outparameters.push_back(ladparam);
			}
		}
	}
}


// fill_plugin_info = reproduce generic cached info struct
void CLadspaInfo::fill_plugin_info() {

	assert(lad_descriptor != 0);
	assert(lad_descriptor->run != 0);

	uri = "@xlutop.com/lad2zzub/";
	uri += lad_descriptor->Name;
	ReplaceChar((char *)uri.c_str(),' ','+');

	//lad_uid = pinfo.uniqueID;

	name = lad_descriptor->Name;
	short_name = lad_descriptor->Label;
	author = lad_descriptor->Maker;
	commands = "About ";
	commands += name;
	commands += "...";

	inputs = 0;
	outputs = 0;

	int numOutParams = 0; // peer parameters?
	int numInParams = 0; // normal parameters
	for (int i = 0; i < (int)lad_descriptor->PortCount; i++) {
		if (lad_descriptor->PortDescriptors[i]&LADSPA_PORT_CONTROL) {
			if (lad_descriptor->PortDescriptors[i]&LADSPA_PORT_INPUT) {

				float defvalue = get_default_parameter_value(i);

				zzub::parameter &global = add_global_parameter();

				global.set_word();

				global.set_name(lad_descriptor->PortNames[i]);
				global.set_description(lad_descriptor->PortNames[i]);

				global.set_flags(zzub_parameter_flag_state);

				global.set_value_min(CLadspa::eMinParam);
				global.set_value_max(CLadspa::eMaxParam);
				global.set_value_default(int( defvalue * CLadspa::eMaxParam ) );

				numInParams++;
			} else if (lad_descriptor->PortDescriptors[i]&LADSPA_PORT_OUTPUT) {
				// TODO: add_controller_parameter()
				numOutParams++;
			}
		}
		else if (lad_descriptor->PortDescriptors[i]&LADSPA_PORT_AUDIO) {
			// TODO: remember io names
			if (lad_descriptor->PortDescriptors[i]&LADSPA_PORT_INPUT) {
				inputs++;
			} else if (lad_descriptor->PortDescriptors[i]&LADSPA_PORT_OUTPUT) {
				outputs++;
			}
		}
	}

	zzub::parameter &inertia = add_global_parameter();
	inertia.set_word();
	inertia.set_name("Inertia");
	inertia.set_description("Inertia in ticks");
	inertia.set_flags(zzub_parameter_flag_state);
	inertia.set_value_min(0);
	inertia.set_value_max(CLadspa::eMaxInertia);
	inertia.set_value_default(CLadspa::eDefaultInertia);
}



CLadspaCollection::CLadspaCollection() : factory(0), cachedplugins("lad2zzub", "8") {
}

CLadspaCollection::~CLadspaCollection() {	
}

void CLadspaCollection::initialize(zzub::pluginfactory *f) {	
	factory = f;

	if (!factory) return;

	boost::filesystem::path pluginpath = boost::filesystem::path(hostpath) / boost::filesystem::path("Gear\\LADSPA");
	boost::filesystem::path cachepath = boost::filesystem::path(userpath) / boost::filesystem::path("PluginCache\\");
	boost::filesystem::create_directories(cachepath);
	cachedplugins.enumerate_directory_with_cache(this, f, pluginpath.string(), cachepath.string(), true);
	free_libraries();
}

void CLadspaCollection::destroy(void){	
	delete this;
}

void CLadspaCollection::configure(const char *key, const char *value) {
	std::string ck = key;
	// TODO: support semicolon delimited ladspapaths key
	if (ck == "hostpath" && value)
		hostpath = value;
	else if (ck == "userpath" && value)
		userpath = value;
}

HMODULE CLadspaCollection::load_library(const std::string& dllname) {
	std::map<std::string, moduleref_t>::iterator i = dllrefs.find(dllname);
	if (i != dllrefs.end()) {
		i->second.second++;
		return i->second.first;
	}
	HMODULE hm = LoadLibrary(dllname.c_str());
	dllrefs[dllname] = moduleref_t(hm, 1);
	return hm;
}

bool CLadspaCollection::free_library(const std::string& dllname) {
	std::map<std::string, moduleref_t>::iterator i = dllrefs.find(dllname);
	if (i == dllrefs.end()) return true;

	assert(i->second.second > 0);
	i->second.second--;
	if (i->second.second == 0) {
		FreeLibrary(i->second.first);
		dllrefs.erase(i);
		return true;
	}
	return false;
}

void CLadspaCollection::free_libraries() {
	for (std::map<std::string, moduleref_t>::iterator i = dllrefs.begin(); i != dllrefs.end(); ++i) {
		FreeLibrary(i->second.first);
	}
	dllrefs.clear();
}

// called by plugincache
void CLadspaCollection::create_plugin_infos_from_file(const std::string& fullpath, std::vector<CLadspaInfo*>* infos) {

	HMODULE hm = load_library(fullpath.c_str());
	if (!hm) return ;

	int numplugins = 0;
	for (;;) {
		const LADSPA_Descriptor* descriptor = ladspa_get_descriptor(hm, numplugins);
		if (descriptor == 0) break;

		CLadspaInfo* info = new CLadspaInfo(this, fullpath);
		infos->push_back(info);

		numplugins++;
	}

	// the module handle isnt freed until after enumeration, need it later during fill_plugin_infos()
}

void CLadspaCollection::init_plugin_infos(std::vector<CLadspaInfo*>& infos, bool from_cache) {
	// special stuff

	for (std::vector<CLadspaInfo*>::iterator i = infos.begin(); i != infos.end(); ++i) {
		int index = (int)std::distance(infos.begin(), i);
		(*i)->lad_index = index;
	}
}

bool CLadspaCollection::fill_plugin_infos(std::vector<CLadspaInfo*>& infos) {
	// wasnt loaded from cache
	for (std::vector<CLadspaInfo*>::iterator i = infos.begin(); i != infos.end(); ++i) {

		HMODULE hm = load_library((*i)->pluginfile.c_str());
		if (!hm) return false;

		int index = (int)std::distance(infos.begin(), i);

		const LADSPA_Descriptor* descriptor = ladspa_get_descriptor(hm, index);
		if (descriptor == 0) {
			free_library((*i)->pluginfile.c_str());
			return false;
		}

		(*i)->lad_descriptor = descriptor;

		// add parameters and stuff
		(*i)->fill_plugin_info();

		(*i)->lad_descriptor = 0;

		free_library((*i)->pluginfile.c_str());

	}
	return true;
}

void CLadspaCollection::unregister_plugin_infos(std::vector<CLadspaInfo*>& info) {
	// special stuff
}

zzub::plugincollection* lad2zzub_get_plugincollection() {
	return new CLadspaCollection();
}
