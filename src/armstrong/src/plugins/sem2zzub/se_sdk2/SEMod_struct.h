#if !defined(_SEMod_struct_h_inc_)
#define _SEMod_struct_h_inc_

#include "SEMod_struct_base.h"


#if defined IGNORE_OLD_SEEVENT

// new one for SDK3
struct SeEvent			// a generic timestamped event
{
public:
	SeEvent(unsigned long p_time_stamp, int p_event_type, int p_int_parm1, int p_int_parm2, void *p_ptr_parm1 ) : timeStamp(p_time_stamp), eventType(p_event_type), int_parm1(p_int_parm1), int_parm2(p_int_parm2), parm3(p_ptr_parm1), next(0){};
	unsigned long timeStamp;
	int eventType;
	int int_parm1;
	int int_parm2;
	void *parm3;
	SeEvent *next; // next in list (not used)
};

#else

struct SeEvent			// a generic timestamped event
{
public:
	SeEvent(unsigned long p_time_stamp, int p_event_type, int p_int_parm1, int p_int_parm2, void *p_ptr_parm1 ) : time_stamp(p_time_stamp), event_type(p_event_type), int_parm1(p_int_parm1), int_parm2(p_int_parm2), ptr_parm1(p_ptr_parm1), next(0){};
	unsigned long time_stamp;
	int event_type;
	int int_parm1;
	int int_parm2;
	void *ptr_parm1;
	SeEvent *next; // next in list (not used)
};

#endif

#ifndef __AEffect__

// VstTimeInfo as requested via seaudioMasterGetTime (getTimeInfo())
// refers to the current time slice. note the new slice is
// already started when processEvents() is called

struct VstTimeInfo
{
	double samplePos;			// current location
	double sampleRate;
	double nanoSeconds;			// system time
	double ppqPos;				// 1 ppq
	double tempo;				// in bpm
	double barStartPos;			// last bar start, in 1 ppq
	double cycleStartPos;		// 1 ppq
	double cycleEndPos;			// 1 ppq
	long timeSigNumerator;		// time signature
	long timeSigDenominator;
	long smpteOffset;
	long smpteFrameRate;		// 0:24, 1:25, 2:29.97, 3:30, 4:29.97 df, 5:30 df
	long samplesToNextClock;	// midi clock resolution (24 ppq), can be negative
	long flags;					// see below
};

enum
{
	kVstTransportChanged 		= 1,
	kVstTransportPlaying 		= 1 << 1,
	kVstTransportCycleActive	= 1 << 2,

	kVstAutomationWriting		= 1 << 6,
	kVstAutomationReading		= 1 << 7,

	// flags which indicate which of the fields in this VstTimeInfo
	//  are valid; samplePos and sampleRate are always valid
	kVstNanosValid  			= 1 << 8,
	kVstPpqPosValid 			= 1 << 9,
	kVstTempoValid				= 1 << 10,
	kVstBarsValid				= 1 << 11,
	kVstCyclePosValid			= 1 << 12,	// start and end
	kVstTimeSigValid 			= 1 << 13,
	kVstSmpteValid				= 1 << 14,
	kVstClockValid 				= 1 << 15
};
#endif

// module flags, indicate special abilities
#define UGF_POLYPHONIC_AGREGATOR	0x0040		// A ug that always combines voices
#define UGF_VOICE_MON_IGNORE		0x0002// DON'T WANT VOICE HELD OPEN BECAUSE A SCOPE IS CONNECTED

//read only
#define UGF_SUSPENDED				0x0080
#define UGF_OPEN					0x0100
#define UGF_CLONE					0x0800
#define UGF_SEND_TIMEINFO_TO_HOST	0x20000

// normally when a voice has faded out, SE shuts all that voice's modules off
// in very special cases you can prevent SE from shutting off your module
#define UGF_NEVER_SUSPEND			0x0200

// visible on control panel (applys to gui_flags member)
#if !defined( CF_CONTROL_VIEW )
	#define CF_CONTROL_VIEW	128
#endif
#if !defined( CF_STRUCTURE_VIEW )
	#define CF_STRUCTURE_VIEW	256
#endif

struct SEModuleProperties
{
	char *name;
	char *id;
	char *about;
	long flags;
	long gui_flags;
	int sdk_version;
};

struct SEPinProperties
{
	void * variable_address;
	EDirection direction;
	EPlugDataType datatype;
	char *name;
	char *default_value;
	char *datatype_extra;
	long flags;
	long spare;
};

#endif
