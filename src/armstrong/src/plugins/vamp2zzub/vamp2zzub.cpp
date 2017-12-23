#define NOMINMAX
#define _USE_MATH_DEFINES
#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginLoader.h>
#include <zzub/plugin.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <cmath>

// http://www.vamp-plugins.org/

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::RealTime;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;
using std::cerr;
using std::endl;
//#define HOST_VERSION "1.4"  

bool is_usable_output(Plugin::OutputDescriptor &od) {
	return (od.sampleType == Plugin::OutputDescriptor::VariableSampleRate && od.hasFixedBinCount && od.binCount == 0);
}

struct vampplugininfo;

struct vampplugininfo : zzub::info {

	PluginLoader::PluginKey key;
	int vampoutput;
	Plugin::ParameterList vampparameters;

	vampplugininfo() { }

	zzub::plugin* create_plugin();

	void get_internal_parameter_info(Plugin::ParameterDescriptor &pd, zzub_parameter_type* type, int* minvalue, int* maxvalue, int* novalue) {
		float paramrange = pd.maxValue - pd.minValue;
		if (pd.isQuantized) {
			*minvalue = 0;
			*maxvalue = (int)paramrange;
			if (paramrange <= 255) {
				*type = zzub_parameter_type_byte;
				*novalue = 255;
			} else
			if (paramrange <= 65535) {
				*type = zzub_parameter_type_word;
				*novalue = 65535;
			} else {
				// out of range parameter!
				*type = zzub_parameter_type_word;
				*novalue = 65535;
			}
		} else {
			*type = zzub_parameter_type_byte;
			*minvalue = 0;
			*maxvalue = 254;
			*novalue = 255;
		}
	}

	int get_internal_parameter_value(float value, Plugin::ParameterDescriptor &pd) {
		float negValue = -pd.minValue;

		if (pd.isQuantized) {
			return (value / (float)pd.quantizeStep) + negValue;
		} else {
			float paramrange = pd.maxValue - pd.minValue;
			float unitdefault = (negValue + value) / paramrange;
			unitdefault = std::max(std::min(unitdefault, 1.0f), 0.0f);
			return unitdefault * 254.0f;
		}
	}

	float get_external_parameter_value(int value, Plugin::ParameterDescriptor &pd) {
		float negValue = -pd.minValue;
		if (pd.isQuantized) {
			return (value * pd.quantizeStep) - negValue;
		} else {
			float paramrange = pd.maxValue - pd.minValue;
			float unitvalue = (float)value / 254.0f;
			return ((unitvalue * paramrange) - negValue);
		}
	}

};

struct vampplugin : zzub::plugin {
	char gvals[256];
	vampplugininfo* plugininfo;

	vampplugin(vampplugininfo* info) {
		plugininfo = info;
		global_values = &gvals;
		track_values = 0;
	}

	virtual void init(zzub::archive*) {
	}

	virtual void process_events() {
	}

	virtual bool process_offline(float **pin, float **pout, std::vector<int>& slices, int *numsamples, int *channels, int *samplerate, int beginloop, int endloop) { 
		if (!pin && !pout) {
			return true;
		}

		PluginLoader *loader = PluginLoader::getInstance();
		std::string path = loader->getLibraryPathForPlugin(plugininfo->key);
		Plugin* plugin = loader->loadPlugin(plugininfo->key, *samplerate, PluginLoader::ADAPT_ALL_SAFE); 

		int blockSize = plugin->getPreferredBlockSize();
		int stepSize = plugin->getPreferredStepSize();

		if (blockSize == 0) {
			blockSize = 1024;
		}
		if (stepSize == 0) {
			if (plugin->getInputDomain() == Plugin::FrequencyDomain) {
				stepSize = blockSize/2;
			} else {
				stepSize = blockSize;
			}
		} else if (stepSize > blockSize) {
			cerr << "WARNING: stepSize " << stepSize << " > blockSize " << blockSize << ", resetting blockSize to ";
			if (plugin->getInputDomain() == Plugin::FrequencyDomain) {
				blockSize = stepSize * 2;
			} else {
				blockSize = stepSize;
			}
			cerr << blockSize << endl;
		} 

		if (!plugin->initialise(*channels, stepSize, blockSize)) {
			cerr << "ERROR: Plugin initialise (channels = " << *channels
				<< ", stepSize = " << stepSize << ", blockSize = "
				<< blockSize << ") failed." << endl;
			return false;
		} 

		int offset = 0;
		assert(plugininfo->vampparameters.size() == plugininfo->global_parameters.size());
		for (int i = 0; i < plugininfo->vampparameters.size(); i++) {
			const zzub::parameter& p(*plugininfo->global_parameters[i]);
			Plugin::ParameterDescriptor &pd(plugininfo->vampparameters[i]);

			int rawvalue = 0;
			int bytesize = p.get_bytesize();
			memcpy(&rawvalue, &gvals[offset], bytesize); // is this endian safe? see metaplugin::transfer_parameter_track_row

			if (rawvalue != p.value_none) {
				float paramvalue = plugininfo->get_external_parameter_value(rawvalue, pd);
				plugin->setParameter(pd.identifier, paramvalue);
			}

			offset += bytesize;
		}

		RealTime rt;
		slices.clear();
		float **plugbuf = new float*[*channels]; 
		for (int c = 0; c < *channels; ++c) {
			plugbuf[c] = new float[blockSize + 2]; 
			memset(plugbuf[c], 0, blockSize * sizeof(float));
		}

		int fullBlockSize = blockSize;
		for (int i = 0; i < *numsamples; i += stepSize) { 

			if (i + stepSize > *numsamples) 
				stepSize = *numsamples - i;
			if (i + blockSize > *numsamples) 
				blockSize = *numsamples - i;

			for (int c = 0; c < *channels; ++c) {
				int copySize = std::max(blockSize, stepSize);
				memcpy(plugbuf[c], &pin[c][i], copySize * sizeof(float));
				memcpy(&pout[c][i], &pin[c][i], stepSize * sizeof(float));

				memset(&plugbuf[c][copySize], 0, (fullBlockSize - copySize) * sizeof(float));
			}

			rt = RealTime::frame2RealTime(i, *samplerate); 

			// append feature timestamps to wavelevel slice list
			{
				Plugin::FeatureSet features = plugin->process(plugbuf, rt);
				Plugin::FeatureList& featureList(features[plugininfo->vampoutput]);
				process_features(*samplerate, featureList, slices);
			}
		}

		// append remaining feature timestamps to wavelevel slice list
		{
			Plugin::FeatureSet features = plugin->getRemainingFeatures();
			Plugin::FeatureList& featureList(features[plugininfo->vampoutput]);
			process_features(*samplerate, featureList, slices);
		}

		for (int c = 0; c < *channels; ++c) delete[] plugbuf[c]; 
		delete[] plugbuf;

		delete plugin;

		return true; 
	}

	void process_features(int samplerate, Plugin::FeatureList& featureList, std::vector<int>& slices) {
		for (unsigned int j = 0; j < featureList.size(); ++j) { 
			Plugin::Feature& f(featureList[j]);
			if (f.hasTimestamp)
				slices.push_back(RealTime::realTime2Frame(f.timestamp, samplerate));
		}
	}

	const char* describe_value(int param, int value) {
		static char str[1014];
		std::stringstream strm;
		Plugin::ParameterDescriptor& pd(plugininfo->vampparameters[param]);

		float extvalue = plugininfo->get_external_parameter_value(value, pd);

		strm.str("");
		if (pd.isQuantized) {
			if (extvalue >= 0 && extvalue < pd.valueNames.size()) {
				strm << pd.valueNames[extvalue];
			} else {
				strm << (int)extvalue;
				if (!pd.unit.empty())
					strm << " " << pd.unit;
			}
		} else {
			strm << std::fixed << std::setprecision(1) << extvalue;
			if (!pd.unit.empty())
				strm << " " << pd.unit;
		}

		strcpy(str, strm.str().c_str());
		return str;
	}

};

zzub::plugin* vampplugininfo::create_plugin() {
	return new vampplugin(this);
}


struct vampplugincollection : zzub::plugincollection {

	virtual void initialize(zzub::pluginfactory * factory) {
		if (!factory) return;
		PluginLoader *loader = PluginLoader::getInstance();

		std::vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
		for (size_t i = 0; i < plugins.size(); ++i) {
			PluginLoader::PluginKey key = plugins[i];
			std::string path = loader->getLibraryPathForPlugin(plugins[i]);
			Plugin *plugin = loader->loadPlugin(key, 48000); 

			Plugin::OutputList outputs = plugin->getOutputDescriptors(); 

			bool usable_plugin = false;
			for (size_t j = 0; j < outputs.size(); ++j) {
				Plugin::OutputDescriptor &od(outputs[j]);
				if (is_usable_output(od)) {
					add_plugin(key, plugin, j, od, factory);
					break;
				}
			}
			delete plugin;
		}

	}

	void add_plugin(PluginLoader::PluginKey key, Plugin *plugin, int output, Plugin::OutputDescriptor &od, zzub::pluginfactory* factory) {
		std::stringstream strm;
		strm << "@zzub.org/vamp2zzub/" << key << "/" << output;

		vampplugininfo* plugininfo = new vampplugininfo();
		plugininfo->collection = this;
		plugininfo->flags = zzub_plugin_flag_is_offline;
		plugininfo->name = plugin->getName();
		plugininfo->short_name = plugin->getName();
		plugininfo->uri = strm.str();
		plugininfo->author = plugin->getMaker();
		plugininfo->min_tracks = 0;
		plugininfo->max_tracks = 0;
		plugininfo->outputs = 0;
		plugininfo->inputs = 0;

		plugininfo->key = key;
		plugininfo->vampoutput = output;
		plugininfo->vampparameters = plugin->getParameterDescriptors();
		for (size_t j = 0; j < plugininfo->vampparameters.size(); ++j) {
			Plugin::ParameterDescriptor &pd(plugininfo->vampparameters[j]);

			zzub::parameter& p = plugininfo->add_global_parameter()
				.set_name(pd.name.c_str())
				.set_description(pd.description.c_str())
				.set_value_default(plugininfo->get_internal_parameter_value(pd.defaultValue, pd))
				.set_state_flag();
			
			plugininfo->get_internal_parameter_info(pd, &p.type, &p.value_min, &p.value_max, &p.value_none);
			//cout << "  Parameter " << j+1 << ": \"" << pd.name << "\" (" << pd.identifier << ")" << endl;
		}

		factory->register_info(plugininfo);
	}

	virtual const char* get_name() {
		return "VAMP";
	}

};

zzub::plugincollection* vamp2zzub_get_plugincollection() {
	return new vampplugincollection();
}

/*
int main() {

	PluginLoader *loader = PluginLoader::getInstance();

	vector<PluginLoader::PluginKey> plugins = loader->listPlugins();
	//typedef multimap<string, PluginLoader::PluginKey> LibraryMap;
	//LibraryMap libraryMap;

	for (size_t i = 0; i < plugins.size(); ++i) {
		PluginLoader::PluginKey key = plugins[i];
		string path = loader->getLibraryPathForPlugin(plugins[i]);

		Plugin *plugin = loader->loadPlugin(key, 48000); 

		Plugin::OutputList outputs = plugin->getOutputDescriptors(); 


		bool usable_plugin = false;
		for (size_t j = 0; j < outputs.size(); ++j) {
			Plugin::OutputDescriptor &od(outputs[j]);
			if (is_usable_output(od)) {
				usable_plugin = true;
				break;
			}
		}

		if (!usable_plugin) continue;

		cout << plugins[i] << ", " << path << endl;

		Plugin::ParameterList params = plugin->getParameterDescriptors();
		for (size_t j = 0; j < params.size(); ++j) {
			Plugin::ParameterDescriptor &pd(params[j]);
			cout << "  Parameter " << j+1 << ": \"" << pd.name << "\" (" << pd.identifier << ")" << endl;
		}


		for (size_t j = 0; j < outputs.size(); ++j) {
			Plugin::OutputDescriptor &od(outputs[j]);
			if (is_usable_output(od)) {
				cout << "  Output " << j+1 << ": \"" << od.name << "\" (" << od.identifier << ")" << endl;
			}
		}
	

		delete plugin; 
	} 
	
}
*/