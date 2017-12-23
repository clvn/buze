#include <zzub/plugin.h>
#include "plugins.h"
#include "modplug.h"

// A plugin collection registers plugin infos and provides
// serialization services for plugin info, to allow
// loading of plugins from song data.
struct modplug_plugincollection : zzub::plugincollection {
	modplug_plugin_info _info;
	
	// Called by the host initially. The collection registers
	// plugins through the pluginfactory::register_info method.
	// The factory pointer remains valid and can be stored
	// for later reference.
	virtual void initialize(zzub::pluginfactory *factory);
	
	// Called by the host upon song loading. If the collection
	// can not provide a plugin info based on the uri or
	// the metainfo passed, it should return a null pointer.
	// This will usually only be called if the host does
	// not know about the uri already.
	virtual zzub::info *get_info(const char *uri, zzub::archive *arc) { return 0; }
	
	// Returns the uri of the collection to be identified,
	// return zero for no uri. Collections without uri can not be 
	// configured.
	virtual const char *get_uri() { return 0; }
	
	// Called by the host to set specific configuration options,
	// usually related to paths.
	virtual void configure(const char *key, const char *value) {}
	
	// Called by the host upon destruction. You should
	// delete the instance in this function
	//virtual void destroy() {}

	const char* get_name() { return "Modplug"; }

};

void modplug_plugincollection::initialize(zzub::pluginfactory *factory) {
	_info.collection = this;
	factory->register_info(&_info);
}

zzub::plugincollection* modplug_get_plugincollection() {
	return new modplug_plugincollection();
}
