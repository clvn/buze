#ifndef __SEMod_struct_base__
#define __SEMod_struct_base__

#include "se_datatypes.h"

#if PRAGMA_ALIGN_SUPPORTED || __MWERKS__
	#pragma options align=mac68k
#elif defined(MINGW) || defined(__GNUC__)
    #pragma pack(push,8)
#elif defined CBUILDER
    #pragma -a8
#elif defined(WIN32) || defined(__FLAT__)
	#pragma pack(push)
	#pragma pack(8)
#endif

#if defined(WIN32) || defined(__FLAT__) || defined CBUILDER
	#define SE_CALLING_CONVENTION __cdecl
#else
	#define SE_CALLING_CONVENTION
#endif

// SDK version number ( 1000 = V1.000 )
#define SDK_VERSION 2240
/*
1000 First numbered version
1001 30-Dec-03 Added IO_DONT_CHECK_ENUM flag
2000 23-Jan-2004 Added GUI support
2010 10-Apr-2004 Several bug fixes
2020 15-Apr-2004 Support for seGuiHostSetWindowType, seGuiHostGetWindowHandle, seGuiOnWindowOpen, seGuiOnWindowClose
2030 17-Apr-2004 Support IO_AUTODUPLICATE on Text pins
2040 11-May-2004 SeGuiPin new members: getValueInt, setValueInt, getValueFloat, setValueFloat
2050 15-May-2004 Fixed bug in RUN_AT function
2060  4-Jun-2004 new RUN_AT code
2070  6-Jun-2004 Guipins re-inplemented as flyweights to handle AUTODUPLICATE adds/removes nicer
2080 11-Jun-2004 Fixed crash for modules with zero pins
2090  6-Aug-2004 Added seaudioMasterGetSeVersion opcode
2100 12-Aug-2004 Added extra parameter, p_user_msg_id, to OnModuleMsg and OnGuiNotify
2110 12-Aug-2004 flags now passed on mouse events 
2120 12-Aug-2004 new functions: SetCapture, ReleaseCapture, GetCapture 
2130  1-Nov-2004 new function: OnIdle  
2140 10-Nov-2004 updates for GCC 3.1   
2150 10-Nov-2004 added opcode seGuiHostGetHostType
2160 10-Nov-2004 DSP Module Pin initialisation now skips GUI-only pins correctly
2170 28-Dec-2004 new function GuiModule::OnNewConnection()
2180 20-Jan-2005 new function Module::VoiceReset() not fully implemented
2190 02-Mar-2005 new opcodes: seGuiHostGetParentContext, seGuiHostMapWindowPoints
2200 04-Apr-2005 seGuiOnGuiPlugValueChange p_pin bug fixed, new function SeGuiPin::getExtraData()
2210 04-Apr-2005 new function seGuiHostMapClientPointToScreen
2220 04-Apr-2005 new function seGuiHostIsGraphInitialsed
2230 04-Apr-2005 new flag IO_PAR_PRIVATE
2240 16-Jul-2009 new GUI opcode seGuiHostPlugIsConnected.
*/

//---------------------------------------------------------------------------------------------
// misc def's
//---------------------------------------------------------------------------------------------

typedef struct SEMod_struct_base SEMod_struct_base;
typedef	long (SE_CALLING_CONVENTION *seaudioMasterCallback)(SEMod_struct_base *effect, long opcode, long index,
		long value, void *ptr, float opt);

// new way
typedef struct SEMod_struct_base2 SEMod_struct_base2;
typedef	long (SE_CALLING_CONVENTION *seaudioMasterCallback2)(SEMod_struct_base2 *effect, long opcode, long index,
		long value, void *ptr, float opt);


// the 'magic number' that identifies a SynthEdit module (spells SEPL)
#define SepMagic 0x5345504C
#define SepMagic2 (SepMagic+1)

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
struct SeEvent;

typedef	void (SE_CALLING_CONVENTION *process_funtion_ptr)(SEMod_struct_base2 *effect, long buffer_offset, long sampleframes);


// SDK V1
struct SEMod_struct_base
{
	long magic;
	long (SE_CALLING_CONVENTION *dispatcher)(SEMod_struct_base *effect, long opCode, long index, long value,
		void *ptr, float opt);

	// these 3 not used
	void (SE_CALLING_CONVENTION *process)(SEMod_struct_base *effect, float **inputs, float **outputs, long sampleframes);
	void (SE_CALLING_CONVENTION *setParameter)(SEMod_struct_base *effect, long index, float parameter);
	float (SE_CALLING_CONVENTION *getParameter)(SEMod_struct_base *effect, long index);

	long flags;			// see constants
	void *resvd1;		// reserved for host use, must be 0
	long resvd2;		// reserved, must be 0
	void *object;		// for class access, MUST be 0 else!
	void *user;			// user access
	long version;		//
//	void (SE_CALLING_CONVENTION *process Replacing)(SEMod_struct_base *effect, long buffer_offset, long sampleframes);
	process_funtion_ptr processReplacing;
	char future[60];	// pls zero
};

class SEModule_base;

#if defined(MINGW) || defined(__GNUC__)
	typedef	void ( SEModule_base::*process_function_ptr2)(long, long);
	typedef	void ( SEModule_base::*event_function_ptr)(SeEvent *e);
	typedef	long ( *dispatcher_ptr_type)(SEMod_struct_base2 *effect, long opCode, long index, long value,void *ptr, float opt);
#else
	typedef	void (SE_CALLING_CONVENTION SEModule_base::*event_function_ptr)(SeEvent *e);
	typedef	long (SE_CALLING_CONVENTION *dispatcher_ptr_type)(SEMod_struct_base2 *effect, long opCode, long index, long value,void *ptr, float opt);
	typedef	void (SE_CALLING_CONVENTION SEModule_base::*process_function_ptr2)(long, long);
#endif

// SDK V2
struct SEMod_struct_base2
{
	long magic;
	long version;
	void *resvd1;		// reserved for host use, must be 0
	void *object;		// for class access, MUST be 0 else!
	void *user;			// user access
	dispatcher_ptr_type dispatcher;
	long sub_process_ptr;
	long event_handler_ptr;
	char future[16];	// pls zero
};


//---------------------------------------------------------------------------------------------
// Plugin Module opCodes
//---------------------------------------------------------------------------------------------

enum
{
	seffOpen = 0,		// initialise
	seffClose,			// exit, release all memory and other resources!

	// system

	seffSetSampleRate,	// in opt (float)
	seffSetBlockSize,	// in value
	seffGetUniqueId,
	seffGetEffectName,
	seffGetPinProperties,
	seffAddEvent,
	seffResume,		
	seffGetModuleProperties,
 	seffIsEventListEmpty,
 	seffGetSdkVersion,
 	seffGuiNotify,
 	seffQueryDebugInfo // allows to host to determin compiler settings etc
};

//---------------------------------------------------------------------------------------------
// Host opCodes
//---------------------------------------------------------------------------------------------

enum
{
	seaudioMasterSetPinStatus = 0,
	seaudioMasterIsPinConnected,	// 
	seaudioMasterGetPinInputText,	// gets pointer to plugs input string (DT_TEXT only)
									// loading
	seaudioMasterGetSampleClock,	// get current sampleclock at block start
									// 
	seaudioMasterSendMIDI,			// send short MIDI msg
	seaudioMasterGetInputPinCount,  // Total AUDIO ins
	seaudioMasterGetOutputPinCount, // total AUDIO outs
	seaudioMasterGetPinVarAddress,	// for audio pins only.
	seaudioMasterGetBlockStartClock,
	seaudioMasterGetTime,
	seaudioMasterSleepMode,
	seaudioMasterGetRegisteredName, //limited to 50 characters or less
		/* EXAMPLE CALLING CODE
			char name[50];
			CallHost( seaudioMasterGetRegisteredName , 0, 0, &name);
		*/
	seaudioMasterGetFirstClone,
	seaudioMasterGetNextClone,
		/* EXAMPLE CALLING CODE
			// iterate through all clones
			SEMod_struct_base2 *clone_struct;

			// get first one
			CallHost(seaudioMasterGetFirstClone,0,0,&clone_struct);

			while( clone_struct != 0 )
			{
				// convert host's clone pointer to a 'Module' object
				Module *clone = ((Module *)(clone_struct->object));

				// Access each clone here

				// step to next clone
				clone->CallHost(seaudioMasterGetNextClone,0,0,&clone_struct);
			}
		*/
	seaudioMasterGetTotalPinCount,	// Total pins of all types
	seaudioMasterCallVstHost,		// Call VST Host direct (see se_call_vst_host_params struct)
	seaudioMasterResolveFilename,	// get full path from a short filename, (int pin_idx, float max_characters, char *destination)
	seaudioMasterSendStringToGui,			// Reserved for Experimental use (by Jef)
	seaudioMasterGetModuleHandle,	// Reserved for Experimental use (by Jef)
	seaudioMasterAddEvent,			// pass SeEvent *, host will copy data from struct. Safe to discard after call.
	seaudioMasterCreateSharedLookup,
	seaudioMasterSetPinOutputText,	// sets plug's output string (DT_TEXT only)
	seaudioMasterSetProcessFunction,// sets the current sub_process() function
	seaudioMasterResolveFilename2,	// get full path from a short filename - UNICODE.  see also seGuiHostResolveFilename
		/* EXAMPLE CALLING CODE
			#include <windows .h>  //for WideCharToMultiByte

			// get the full path of an imbedded file when you only know it's short name
			const int MAX_FILENAME_LENGTH = 300;

			// Both source and destination are UNICODE (two-byte) character strings
			unsigned short *source = L"test.txt";
			unsigned short dest[MAX_FILENAME_LENGTH];

			CallHost( seaudioMasterResolveFilename2 , (long) source, MAX_FILENAME_LENGTH, &dest);

			// to convert to ascii (optional)
			char ascii_filename[MAX_FILENAME_LENGTH];
			WideCharToMultiByte(CP_ACP, 0, dest, -1, ascii_filename, MAX_FILENAME_LENGTH, NULL, NULL);
		*/
	seaudioMasterGetSeVersion, // returns SE version number times 100,000 ( e.g. 120000 is V 1.2 )
		/* EXAMPLE CALLING CODE
			int v = CallHost( seaudioMasterGetSeVersion , 0, 0, 0);
		*/
};

// fill in this structure when using opcode seaudioMasterCallVstHost
/*
	Download the VST SDK for a list of VST opcodes.
	http://www.steinberg.net/en/support/3rdparty/vst_sdk/download/

	Look in aeffect.h and aeffectx.h
*/
struct se_call_vst_host_params
{
	long opcode;
	long index;
	long value;
	void *ptr;
	float opt;
};

#if PRAGMA_ALIGN_SUPPORTED || __MWERKS__
	#pragma options align=reset
#elif defined(WIN32) || defined(__FLAT__)
	#pragma pack(pop)
#elif defined CBUILDER
	#pragma -a-
#endif

#endif	// __SEMod_struct_base__
