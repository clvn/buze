#pragma once

#include <map>
#include <boost/shared_array.hpp>

struct PARAMS {
	LADSPA_Data min;
	LADSPA_Data max;
	LADSPA_PortRangeHintDescriptor hint;
};


class CLadspaCollection;

class CLadspaInfo : public zzub::info {
public:
	int lad_uid;
	int lad_index;
	const LADSPA_Descriptor *lad_descriptor;
	std::vector<PARAMS> lad_parameters;
	std::vector<PARAMS> lad_outparameters;
	CLadspaCollection* owner;

	CLadspaInfo(CLadspaCollection* coll, const std::string& fullpath);
	virtual ~CLadspaInfo();
	zzub::plugin *create_plugin(void);
	bool store_info(zzub::archive *arc) const;
	void fill_plugin_info();
	void fill_parameter_info();
	float get_default_parameter_value(int idx, LADSPA_Data *df = 0, LADSPA_Data *mi = 0, LADSPA_Data *mx = 0);
	static void ReplaceChar(char *str, const char toReplace, const char replacedBy);
};

typedef std::pair<HMODULE, int> moduleref_t;

class CLadspaCollection : public zzub::plugincollection {
public:
	std::string hostpath;
	std::string userpath;
	std::map<std::string, moduleref_t> dllrefs;
	plugincache<CLadspaCollection, CLadspaInfo> cachedplugins;
	zzub::pluginfactory *factory;

	CLadspaCollection();
	virtual ~CLadspaCollection();
	void initialize(zzub::pluginfactory *factory);
	void destroy(void);
	void configure(const char *key, const char *value);
	const char* get_name() { return "LADSPA"; }

	// called by plugincache
	void create_plugin_infos_from_file(const std::string& fullpath, std::vector<CLadspaInfo*>* infos);
	void init_plugin_infos(std::vector<CLadspaInfo*>& infos, bool from_cache);
	bool fill_plugin_infos(std::vector<CLadspaInfo*>& infos);
	void unregister_plugin_infos(std::vector<CLadspaInfo*>& info);

	HMODULE load_library(const std::string& dllname);
	bool free_library(const std::string& dllname);
	void free_libraries();
};

//*****************************************************************************************************************************************************

class CLadspa : public zzub::plugin {
	friend CLadspaInfo;
	friend class CLadspaCollection;

	struct SParam {				
		unsigned short newParam;
		float inertia;
		float curParam;
	};

public:

	enum {
		eVersion = 1,
		eMinTracks = 0,
		eMaxTracks = 0,
		eNumGlobalParams = 0,
		eNumTrackParams = 0,
		eNumParams = eNumGlobalParams + eNumTrackParams,
		eNumAttributes = 0,
		eMinParam = 0x0000,
		eMaxParam = 0x8000,		
		eMaxInertia = 32000,
		eDefaultInertia = 1000,
		eInertiaTick = 1000
	};
	
//**** plugin *****************************************************************************************************************************************

	void init(zzub::archive *arc);
	void save(zzub::archive *arc);
	void destroy(void);	
	void process_events(void);
	bool process_stereo(float **pin, float **pout, int numsamples, int mode);
	virtual bool process_offline(float **pin, float **pout, int *numsamples, int *channels, int *samplerate) { return false; }
	void stop(void);
	void command(int index);
	const char * describe_value(int param,int value);

//**** CLadspa ****************************************************************************************************************************************

private:
	
	CLadspa(CLadspaInfo *_info);
	virtual ~CLadspa();	
	void set_parameter(int idx, float val);
	float get_parameter(int idx, float val);

	__forceinline bool IsDenormal(const float &val) const {		
		return ( *((unsigned int *)&val) & 0x7f800000 ) == 0;
	};

private:	
	LADSPA_Handle handle;
	CLadspaInfo *info;
	bool ladspaInitialized;
	std::vector<short> params;
	std::vector<float> pluginparams;
	std::vector<float> pluginoutparams;
	std::vector<boost::shared_array<float> > inbuffers;
	std::vector<boost::shared_array<float> > outbuffers;
	unsigned short inertia;	
	std::vector<SParam> paramInertia;
};

