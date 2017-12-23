#ifndef __SEGui_struct_base__
#define __SEGui_struct_base__

#include "se_datatypes.h"
#include <windows.h>

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
	#define SEMODCALLBACK __cdecl
#else
	#define SEMODCALLBACK
#endif

//---------------------------------------------------------------------------------------------
// misc def's
//---------------------------------------------------------------------------------------------

struct SEGUI_struct_base;
typedef	long (SEMODCALLBACK *seGuiCallback)(SEGUI_struct_base *effect, long opcode, long index,
		long value, void *ptr, float opt);

// the 'magic number' that identifies a SynthEdit module (spells SEPL)
//#define SepMagic 0x5345504C

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
struct SEGUI_struct_base
{
	long magic;			// magic number
	long version;
	long (SEMODCALLBACK *dispatcher)(SEGUI_struct_base *effect, long opCode, long index, long value,
		void *ptr, float opt);

	void *resvd1;		// reserved for host use, must be 0
	void *object;		// for class access 
	void *user;			// user access
	char future[16];	// pls zero
};

//---------------------------------------------------------------------------------------------
// Plugin Module opCodes (GUI)
//---------------------------------------------------------------------------------------------

enum
{
	seGuiInitialise = 0,		// initialise
	seGuiClose,			// exit, release all memory and other resources!
	seGuiPaint,
	seGuiLButtonDown,
	seGuiLButtonUp,
	seGuiMouseMove,
	seGuiOnModuleMessage,			// OnModuleMsg test
	seGuiOnGuiPlugValueChange,
	seGuiOnWindowOpen,
	seGuiOnWindowClose,
	seGuiOnIdle,
	seGuiOnNewConnection,
	seGuiOnDisconnect,
};

//---------------------------------------------------------------------------------------------
// Host opCodes (GUI)
//---------------------------------------------------------------------------------------------

enum
{
	seGuiHostRequestRepaint = 0,	//
	seGuiHostGetHandle,			//
	seGuiHostSendStringToAudio,			// SendStringToAudio test
	seGuiHostSetWindowSize,
	seGuiHostSetWindowSizeable,
	seGuiHostGetTotalPinCount,
	seGuiHostPlugSetValText,
	seGuiHostPlugGetValText,
	seGuiHostAddGuiPlug,
	seGuiRegisterPatchParameter,		// Obsolete, use IO_PATCH_STORE or IO_UI_COMMUNICATION_DUAL flags instead. Will crash module on destruction (mayby need Unregister opcode to fix this)
	seGuiHostGetFontInfo,
	seGuiHostSetWindowType,				// pass 1 to provide your GuiModule with a 'real' HWND (else SE draws your module on the parent window)
	seGuiHostGetWindowHandle,
		/* example code... (wi is a SEWndInfo pointer )
			HWND h = (HWND) CallHost( seGuiHostGetWindowHandle, wi->context_handle );
		*/
	seGuiHostSetWindowFlags,
	seGuiHostPlugGetVal,
	seGuiHostPlugSetVal,
	seGuiHostPlugSetExtraData, // sets enum list or file extension (depending on datatype)
		/* example code...
			// pass pin number and new list
			CallHost( seGuiHostPlugSetExtraData, 4, 0, "moose,cat,dog", 0 );
		*/
	seGuiHostPlugGetExtraData, // gets enum list or file extension (depending on datatype). Easier to use SeGuiPin::getExtraData()
		/* example code...
			int string_length = CallHost( seGuiHostPlugGetExtraData, getIndex(), 0, 0);

			// Destination is UNICODE (two-byte) character string
			wchar_t *dest = new wchar_t[string_length];

			CallHost( seGuiHostPlugGetExtraData, PN_ENUM_OUT, string_length, &dest);

			// to convert to ascii
			char *ascii_text = new char[string_length];
			WideCharToMultiByte(CP_ACP, 0, dest, -1, ascii_text, MAX_STRING_LENGTH, NULL, NULL);

			// clean up
			delete [] dest;
			delete [] ascii_text;
		*/
	seGuiHostSetCapture,	// see SEGUI_base::SetCapture(...)
	seGuiHostReleaseCapture,
	seGuiHostGetCapture,
	seGuiHostCallVstHost, // pass se_call_vst_host_params structure in ptr
	seGuiHostSetIdle,		// pass 1 to receive regular calls to OnIdle(), pass zero to cancel
	seGuiHostGetModuleFilename, // returns full module path
		/* example code...
			const int MAX_STRING_LENGTH = 300;

			// Destination is UNICODE (two-byte) character string
			unsigned short dest[MAX_STRING_LENGTH];

			CallHost( seGuiHostGetModuleFilename, 0, MAX_STRING_LENGTH, &dest);

			// to convert to ascii
			char ascii_text[MAX_STRING_LENGTH];
			WideCharToMultiByte(CP_ACP, 0, dest, -1, ascii_text, MAX_STRING_LENGTH, NULL, NULL);
		*/
	seGuiHostResolveFilename, // returns full module path
		/* example code...
			const int MAX_STRING_LENGTH = 300;

			// Destination is UNICODE (two-byte) character string
			unsigned short dest[MAX_STRING_LENGTH];

			// convert filename to UNICODE
			MultiByteToWideChar(CP_ACP, 0, "test.wav", -1, (LPWSTR) &dest, MAX_STRING_LENGTH );

			// query full filename (SE concatenates default path for that type of file, depending on file extension)
			CallHost( seGuiHostResolveFilename, 0, MAX_STRING_LENGTH, &dest);

			// to convert to ascii (optional)
			char ascii_text[MAX_STRING_LENGTH];
			WideCharToMultiByte(CP_ACP, 0, dest, -1, ascii_text, MAX_STRING_LENGTH, NULL, NULL);
		*/
		seGuiHostGetHostType, // return code 0 =unsuported, 1=module is running in SynthEdit, 2= Module is in a VST plugin (made with SE)
		seGuiHostRemoveGuiPlug,
		seGuiHostGetParentContext, // Get 'handle' of parent window.  This is an SE handle, not an HWND.  Use seGuiHostGetWindowHandle to convert.
		seGuiHostMapWindowPoints, // map a point on one window to the co-ordinate system of a 2nd window
		/*
			// Example: getting parent HWND, and your position relative to it
			long parent_context = wi->context_handle;
			HWND h = 0;
			while( h == 0 )
			{
				parent_context = CallHost(seGuiHostGetParentContext, parent_context );
				h = (HWND) CallHost( seGuiHostGetWindowHandle, parent_context );
			}

			sepoint offset(0,0);
			CallHost(seGuiHostMapWindowPoints, wi->context_handle, parent_context, &offset, 0 );
		*/
		seGuiHostMapClientPointToScreen, // maps a point on your gui to the system screen (absolute co-ords)
		/*
			// Example: converting a point on your GUI to an absolute co-ordinate. Useful for pop-up menus
			sepoint offset(0,0);
			CallHost(seGuiHostMapClientPointToScreen, wi->context_handle, 0, &offset, 0 );
		*/
		seGuiHostInvalidateRect, // invlalidate (cause redraw) of any SE window
		/*
			RECT n;
			n.top = 0;
			n.bottom = 1;
			n.left = 2;
			n.right = 20;
			CallHost( seGuiHostInvalidateRect, wi->context_handle, 0, &n, 0 );
		*/
		seGuiHostIsGraphInitialsed, // test if pin updates are due to file loading, or from user.
		seGuiHostPlugIsConnected,		// Test if pin connected to annother module.
};

enum SEHostWindowFlags {HWF_RESIZEABLE=1, HWF_NO_CUSTOM_GFX_ON_STRUCTURE = 2};

// painting info
struct sepoint : public POINT
{
	sepoint(int px=0, int py=0){x=px;y=py;};
	sepoint(sepoint &other){x=other.x;y=other.y;};
};

struct SEWndInfo
{
	int width;
	int height;
	long context_handle;
};

struct SeFontInfo
{
	int size;
	int color;
	int color_background;
	int flags;  //alignment etc
	int font_height;
//	char category[20];
//	char facename[50];
	char future [100];
};

#if PRAGMA_ALIGN_SUPPORTED || __MWERKS__
	#pragma options align=reset
#elif defined(WIN32) || defined(__FLAT__)
	#pragma pack(pop)
#elif defined CBUILDER
	#pragma -a-
#endif

#endif	// __SEMod_struct_base_gui__
