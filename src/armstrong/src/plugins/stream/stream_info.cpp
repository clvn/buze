#include <zzub/plugin.h>
#include "stream_info.h"

/***

	machine_info

***/

namespace {

const zzub::parameter *paraNote = 0;
const zzub::parameter *paraOffsetHigh = 0;
const zzub::parameter *paraOffsetLow = 0;
const zzub::parameter *paraLengthHigh = 0;
const zzub::parameter *paraLengthLow = 0;
	
const zzub::attribute *attrOffsetFromPlayPos = 0;

}

stream_machine_info::stream_machine_info() {
	this->flags = zzub_plugin_flag_plays_waves 
		| zzub_plugin_flag_has_audio_output
		| zzub_plugin_flag_stream;
	
	this->outputs = 2;
	this->inputs = 0;

	paraNote = &add_global_parameter()
		.set_note();

	// what about a 32bit parameter instead
	paraOffsetLow = &add_global_parameter()
		.set_word()
		.set_name("Offset Low")
		.set_description("32 bit Offset (Lower 16 bits)")
		.set_value_min(0)
		.set_value_max(0xFFFE)
		.set_value_none(0xFFFF)
		.set_value_default(0xFFFF);

	paraOffsetHigh = &add_global_parameter()
		.set_word()
		.set_name("Offset High")
		.set_description("32 bit Offset (Higher 16 bits)")
		.set_value_min(0)
		.set_value_max(0xFFFE)
		.set_value_none(0xFFFF)
		.set_value_default(0xFFFF);

	paraLengthLow = &add_global_parameter()
		.set_word()
		.set_name("Length Low")
		.set_description("32 bit Length (Lower 16 bits)")
		.set_value_min(0)
		.set_value_max(0xFFFE)
		.set_value_none(0xFFFF)
		.set_value_default(0xFFFF);

	paraLengthHigh = &add_global_parameter()
		.set_word()
		.set_name("Length High")
		.set_description("32 bit Length (Higher 16 bits)")
		.set_value_min(0)
		.set_value_max(0xFFFE)
		.set_value_none(0xFFFF)
		.set_value_default(0xFFFF);
		
	attrOffsetFromPlayPos = &add_attribute()
		.set_name("Offset from Song")
		.set_value_min(0)
		.set_value_max(1)
		.set_value_default(0);
}

