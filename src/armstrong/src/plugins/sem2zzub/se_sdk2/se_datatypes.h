#if !defined(_se_datatypes_h_inc_)
#define _se_datatypes_h_inc_

// handy macro to pack a 4 char message id into an int
// e.g. int id = id_to_int('d','u','c','k')
#define id_to_int(c1,c2,c3,c4) ((c1) + ((c2) << 8 ) + ((c3) << 16 ) + ((c4) << 24 ))

//enum EDirection{DR_IN, DR_OUT, DR_CONTAINER_IO, DR_PARAMETER, DR_FEATURE=DR_IN,DR_CNTRL=DR_OUT }; // plug direction
enum EDirection{DR_IN, DR_OUT, DR_UNUSED, DR_PARAMETER, DR_FEATURE=DR_OUT,DR_CNTRL=DR_IN }; // plug direction
enum EPlugDataType { DT_ENUM, DT_TEXT, DT_MIDI2, DT_MIDI = DT_MIDI2, DT_DOUBLE, DT_BOOL, DT_FSAMPLE, DT_FLOAT, DT_VST_PARAM, DT_INT, DT_INT64, DT_BLOB, DT_NONE=-1 };  //plug datatype

// ST_STOP, ST_ONE_OFF obsolete. Use ST_STATIC or ST_RUN.
enum state_type { ST_STOP, ST_ONE_OFF, ST_STATIC=1, ST_RUN }; // Order is important for comparissons

enum ug_event_type {
	  UET_STAT_CHANGE
	, UET_SUSPEND
	, UET_MIDI			// obsolete, used only by SDK2. Use UET_EVENT_MIDI.
	, UET_OBSOLETE // UET_RUN_FUNCTION // old, use UET_RUN_FUNCTION2 instead
	, UET_IO_FUNC
	, UET_UI_NOTIFY
//	, UET_UI_NOTIFY2 // newer method using fifo
	, UET_PROG_CHANGE
	, UET_NOTE_ON
	, UET_NOTE_OFF
	, UET_NOTE_MUTE
	, UET_PITCH_BEND
	, UET_AFTERTOUCH
	, UET_START_PORTAMENTO
	, UET_WS_TABLE_CHANGE
	, UET_DELAYED_GATE
	, UET_PARAM_AUTOMATION
	, UET_NOTE_OFF_HELD
	, UET_HELD_NOTES_OFF
	, UET_NULL			// do nothing
	, UET_GENERIC1		// MODULES CAN USE FOR WHATEVER
	, UET_UNUSED		// obsolete - UET_SET_OUTPUT_PIN
	, UET_RUN_FUNCTION2
	, UET_SERVICE_GUI_QUE
	, UET_DEACTIVATE_VOICE
	, UET_RETUNE

	// new. must match events in MP_API.h
	, UET_EVENT_SETPIN = 100
	, UET_EVENT_STREAMING_START
	, UET_EVENT_STREAMING_STOP
	, UET_EVENT_MIDI
	, UET_GRAPH_START
};

typedef float FSAMPLE;

// posible plug Flags values
// is this an 'active' input (ie you can't combine voices into it) eg filter cutoff
// or input to non-linear process (clipper, distortion etc)
// obsolete, use IO_LINEAR_INPUT instead (it's the oposit though)
#define IO_POLYPHONIC_ACTIVE	1
// midi channel selection etc should should ignore patch changes
#define IO_IGNORE_PATCH_CHANGE	2
// auto-rename on new connection
#define IO_RENAME				4
// plugs which are automaticly duplicated (like a container's 'spare' plug)
#define IO_AUTODUPLICATE		8
#define IO_FILENAME				16
// ALLOW USER TO SET THE VALUE OF THIS OUTPUt eg on 'constant value' ug
#define IO_SETABLE_OUTPUT		32
// plugs which can be duplicated/deleted by CUG
#define IO_CUSTOMISABLE			64
// plugs which handle multiple inputs, must belong to an Adder ug
#define IO_ADDER				128
// plugs which are private or obsolete, but are enabled on load if connected somewhere
#define IO_HIDE_PIN				256
#define IO_PRIVATE				IO_HIDE_PIN // obsolete use IO_HIDE_PIN
#define IO_DISABLE_IF_POS		IO_HIDE_PIN
// set this if this input can handle more that one polyphonic voice
#define IO_LINEAR_INPUT			512
#define IO_UI_COMMUNICATION		1024
#define IO_AUTO_ENUM			0x800
#define IO_HIDE_WHEN_LOCKED		0x1000
// DON'T EXPOSE AS PLUG (TO SAVE SPACE)
#define IO_PARAMETER_SCREEN_ONLY	0x2000
#define IO_DONT_CHECK_ENUM		0x4000
// don't use IO_UI_DUAL_FLAG by itself, use IO_UI_COMMUNICATION_DUAL
#define IO_UI_DUAL_FLAG			0x8000
// obsolete, use IO_PATCH_STORE instead
#define IO_UI_COMMUNICATION_DUAL (IO_UI_DUAL_FLAG | IO_UI_COMMUNICATION)
// Patch store is similar to dual but for DSP output plugs that appear as input plugs on UI (Output paramters) could consolodate?
#define IO_PATCH_STORE			0x10000
// Private parameter (not exposed to user of VST plugin)
#define IO_PAR_PRIVATE			0x20000
// minimised (not exposed on structure view (only properties window)
#define IO_MINIMISED			0x40000
#define IO_CONTAINER_PLUG		0x80000
#define IO_OLD_STYLE_GUI_PLUG	0x100000
#define IO_HOST_CONTROL			0x200000
#define IO_PAR_POLYPHONIC		0x400000
// (bit 24 of 32)
#define IO_PAR_POLYPHONIC_GATE	    0x800000

// sdk3 parameter values not saved in patch (output parameters).
#define IO_PARAMETER_PERSISTANT  	0x1000000

// When first connected, copy pin default, metadata etc to module's first parameter.
#define IO_AUTOCONFIGURE_PARAMETER	0x2000000

// Use Text pin like BLOB, ignore locale when translating to UTF-16 to preserve binary data.
#define IO_BINARY_DATA			    0x4000000

#endif
