#include <zzub/signature.h>
#include <zzub/plugin.h>


#include "fungus.h"

struct introtoolsplugincollection : zzub::plugincollection {
	fungus_machine_info fungus_info;

	virtual void initialize(zzub::pluginfactory *factory) {
		fungus_info.collection = this;
		factory->register_info(&fungus_info);
	}
	
	virtual zzub::info *get_info(const char *uri, zzub::archive *data) { return 0; }
	virtual void destroy() { delete this; }
	// Returns the uri of the collection to be identified,
	// return zero for no uri. Collections without uri can not be 
	// configured.
	virtual const char *get_uri() { return 0; }
	
	// Called by the host to set specific configuration options,
	// usually related to paths.
	virtual void configure(const char *key, const char *value) {}
};

zzub::plugincollection *fungus_get_plugincollection() {
	return new introtoolsplugincollection();
}

//const char *zzub_get_signature() { return ZZUB_SIGNATURE; }
