#include <zzub/plugin.h>
#include <zzub/signature.h>
#include "plugins.h"

/***

	zzub entry points

***/

const char *zzub_get_signature() { 
	return ZZUB_SIGNATURE; 
}

zzub::plugincollection *zzub_get_plugincollection() {
	return new streamplugincollection();
}
