#if defined(_WIN32)
#include <dinput.h>
#endif
#include <zzub/plugin.h>
#include "plugins.h"
#include "pattern.h"
#include "sequence.h"
#include "recorder.h"
#include "input.h"
#include "output.h"
#include "master.h"
#include "harmony.h"
#include "note.h"
#include "peer.h"
#include "offline.h"
#include "group.h"

// A plugin collection registers plugin infos and provides
// serialization services for plugin info, to allow
// loading of plugins from song data.
struct builtin_plugincollection : zzub::plugincollection {
	pattern_plugin_info pattern_info;
	sequence_plugin_info sequence_info;
	recorder_wavetable_plugin_info wavetable_info;
	recorder_file_plugin_info file_info;
	visualizer_plugin_info vis_info;
	output_plugin_info output_info;
	output16_plugin_info output16_info;
	output32_plugin_info output32_info;
	input_plugin_info input_info;
	master_plugin_info master_info;
	harmony_plugin_info harmony_info;
	note_plugin_info note_info;
	value_plugin_info value_info;
	lfo_plugin_info lfo_info;
	signal_value_plugin_info signal_info;
	adsr_value_plugin_info adsr_info;
	value_mapper_plugin_info valuemapper_info;
#if defined(_WIN32)
	joystick_value_plugin_info joystick_info;
#endif
	fade_plugin_info fade_info;
	reverse_plugin_info reverse_info;
	loopxfade_plugin_info loopxfade_info;
#if defined(_WIN32) && !defined(_WIN64)
	soundtouch_plugin_info soundtouch_info;
#endif
	midinotevel_value_plugin_info midinotevel_info;
	midictrl_value_plugin_info midictrl_info;
	group_input_plugin_info groupinput_info;
	group_output_plugin_info groupoutput_info;

	void register_info(zzub::pluginfactory *factory, zzub::info* info);

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

	virtual const char* get_name() {
		return "Core";
	}

};

void builtin_plugincollection::register_info(zzub::pluginfactory *factory, zzub::info* info) {
	info->collection = this;
	factory->register_info(info);
}

void builtin_plugincollection::initialize(zzub::pluginfactory *factory) {
	register_info(factory, &master_info);
	register_info(factory, &pattern_info);
	register_info(factory, &sequence_info);
	register_info(factory, &wavetable_info);
	register_info(factory, &file_info);
	register_info(factory, &vis_info);
	register_info(factory, &output_info);
	register_info(factory, &output16_info);
	register_info(factory, &output32_info);
	register_info(factory, &input_info);
	register_info(factory, &harmony_info);
	register_info(factory, &note_info);
	register_info(factory, &value_info);
	register_info(factory, &lfo_info);
	register_info(factory, &signal_info);
	register_info(factory, &adsr_info);
	register_info(factory, &valuemapper_info);
	register_info(factory, &midinotevel_info);
	register_info(factory, &midictrl_info);
#if defined(_WIN32)
	register_info(factory, &joystick_info);
#endif
	register_info(factory, &fade_info);
	register_info(factory, &reverse_info);
	register_info(factory, &loopxfade_info);
#if defined(_WIN32) && !defined(_WIN64)
	register_info(factory, &soundtouch_info);
#endif
	register_info(factory, &groupinput_info);
	register_info(factory, &groupoutput_info);
}

zzub::plugincollection* core_get_plugincollection() {
	return new builtin_plugincollection();
}
