///\file \brief A COPY of the revision 5713 of the psycle plugin interface.
///\see http://psycle.svn.sourceforge.net/viewvc/psycle/trunk/psycle-plugins/src/psycle/plugins/plugin_interface.hpp
///\todo To keep this file up-to-date, use a svn:externals property to https://psycle.svn.sourceforge.net/svnroot/psycle/trunk/psycle-plugins/src/psycle/plugins

///\interface psycle's native plugin interface api

#pragma once

// *** Note ***
// Because this file may be used outside of the psycle project itself,
// we should not introduce any dependency by including
// anything that is not part of the c++ standard library.

#include <cstdio> // This is NOT part of the interface. It would be better if plugins that want it included it themselves.

namespace psycle
{
	namespace plugin_interface
	{
		///\todo use #include <cstdint> for that!
		typedef /* std::uint8_t  */ unsigned char      uint8;
		typedef /* std::uint16_t */ unsigned short int uint16;
		typedef /* std::uint32_t */ unsigned       int uint32;

		/// machine interface version
		int const MI_VERSION = 11;

		/// max number of pattern tracks
		int const MAX_TRACKS = 64;

		/// max number of samples (per channel) that the Work function may ask to return
		int const MAX_BUFFER_LENGTH = 256;

		///\name note values
		///\{
			/// value of B-9. NOTE: C-0 is note number 0!
			int const NOTE_MAX = 119;
			/// value of the "off" note
			int const NOTE_NO = 120;
			/// empty value
			int const NOTE_OFF = 255;
		///\}

		/// the pi constant.
		/// note: this is also defined in <psycle/helpers/math/pi.hpp> but we want no dependency here
		double const pi = 
			#if defined M_PI // on some systems, #include <cmath> defines M_PI but this is not standard
				M_PI
			#else
				3.14159265358979323846
			#endif
		;

		/*////////////////////////////////////////////////////////////////////////*/

		/// class to define the modificable parameters of the machine
		class CMachineParameter
		{
			public:
				/// Short name: "Cutoff"
				char const *Name;
				/// Longer description: "Cutoff Frequency (0-7f)"
				char const *Description;
				/// >= 0
				int MinValue;
				/// <= 65535
				int MaxValue;
				/// flags.
				int Flags;
				/// default value for params that have MPF_STATE flag set
				int DefValue;
		};

		///\name CMachineParameter flags
		///\{
			/// shows a line with background
			int const MPF_NULL = 0;
			/// shows a line with the text in a centered label
			int const MPF_LABEL = 1;
			/// shows a tweakable knob and text
			int const MPF_STATE = 2;
		///\}

		/*////////////////////////////////////////////////////////////////////////*/

		/// class defining the machine properties
		class CMachineInfo
		{
		public:
			int Version;
			int Flags;
			int numParameters;
			CMachineParameter const **Parameters;
			/// "Rambo Delay"
			char const *Name;
			/// "Delay"
			char const *ShortName;
			/// "John Rambo"
			char const *Author;
			/// "About"
			char const *Command;
			/// number of columns
			int numCols;
		};

		///\name CMachineInfo flags
		///\{
			int const EFFECT = 0;
			///\todo: unused
			int const SEQUENCER = 1;
			int const GENERATOR = 3;
			int const CUSTOM_GUI = 16;
		///\}

		/*////////////////////////////////////////////////////////////////////////*/

		/// callback functions to let plugins communicate with the host.
		class CFxCallback
		{
			public:
				virtual void MessBox(char* ptxt,char*caption,unsigned int type){}
				virtual int CallbackFunc(int cbkID,int par1,int par2,int par3){return 0;}
				/// unused slot kept for binary compatibility for (old) closed-source plugins on msvc++ on mswindows.
				virtual float * unused0(int, int){return 0;}
				/// unused slot kept for binary compatibility for (old) closed-source plugins on msvc++ on mswindows.
				virtual float * unused1(int, int){return 0;}
				virtual int GetTickLength(){return 2048;}
				virtual int GetSamplingRate(){return 44100;}
				virtual int GetBPM(){return 125;}
				virtual int GetTPB(){return 4;}
				// Don't get fooled by the above return values.
				// You get a pointer to a subclass of this one that returns the correct ones.
				virtual inline ~CFxCallback() throw() {}
		};

		/*////////////////////////////////////////////////////////////////////////*/
		
		/// base machine class
		class CMachineInterface
		{
			public:
				virtual inline ~CMachineInterface() {}
				///\todo doc
				virtual void Init() {}
				///\todo doc
				virtual void SequencerTick() {}
				///\todo doc
				virtual void ParameterTweak(int par, int val) {}

				/// Work function
				virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks) {}

				///\todo doc
				virtual void Stop() {}

				///\name Export / Import
				///\{
					///\todo doc
					virtual void PutData(void * pData) {}
					///\todo doc
					virtual void GetData(void * pData) {}
					///\todo doc
					virtual int GetDataSize() { return 0; }
				///\}

				///\todo doc
				virtual void Command() {}
				///\todo doc. not used (yet?)
				virtual void MuteTrack(int const i) {}
				///\todo doc. not used (yet?)
				virtual bool IsTrackMuted(int const i) const { return false; }
				///\todo doc. not used (yet?)
				virtual void MidiNote(int const channel, int const value, int const velocity) {}
				///\todo doc. not used (yet?)
				virtual void Event(uint32 const data) {}
				///\todo doc
				virtual bool DescribeValue(char* txt,int const param, int const value) { return false; }
				///\todo doc. not used (prolly never)
				virtual bool PlayWave(int const wave, int const note, float const volume) { return false; }
				///\todo doc
				virtual void SeqTick(int channel, int note, int ins, int cmd, int val) {}
				///\todo doc. not used (prolly never)
				virtual void StopWave() {}

			public:
				/// initialize these members in the constructor
				int *Vals;

				/// callback.
				/// This member is initialized by the engine right after it calls CreateMachine().
				/// Don't touch it in the constructor.
				CFxCallback * pCB;
		};

		/*////////////////////////////////////////////////////////////////////////*/
		namespace symbols
		{
			// spelling INSTANCIATOR -> INSTANTIATOR
			#define PSYCLE__PLUGIN__INSTANCIATOR(typename, info) PSYCLE__PLUGIN__INSTANTIATOR(typename, info)
			#define PSYCLE__PLUGIN__INSTANTIATOR(typename, info) \
				extern "C" \
				{ \
					PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT \
					psycle::plugin_interface::CMachineInfo const * const \
					PSYCLE__PLUGIN__CALLING_CONVENTION \
					PSYCLE__PLUGIN__SYMBOL_NAME__GET_INFO() { return &info; } \
					\
					PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT \
					psycle::plugin_interface::CMachineInterface * \
					PSYCLE__PLUGIN__CALLING_CONVENTION \
					PSYCLE__PLUGIN__SYMBOL_NAME__CREATE_MACHINE() { return new typename; } \
					\
					PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT \
					void \
					PSYCLE__PLUGIN__CALLING_CONVENTION \
					PSYCLE__PLUGIN__SYMBOL_NAME__DELETE_MACHINE(::CMachineInterface & plugin) { delete &plugin; } \
				}

			#define PSYCLE__PLUGIN__SYMBOL_NAME__GET_INFO GetInfo
			#define PSYCLE__PLUGIN__SYMBOL_NAME__CREATE_MACHINE CreateMachine
			#define PSYCLE__PLUGIN__SYMBOL_NAME__DELETE_MACHINE DeleteMachine
			
			/// we don't use universalis/diversalis here because we want no dependency
			#if !defined _WIN64 && !defined _WIN32 && !defined __CYGWIN__ && !defined __MSYS__ && !defined _UWIN
				#define PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT
				#define PSYCLE__PLUGIN__CALLING_CONVENTION
			#elif defined __GNUG__
				#define PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT __attribute__((dllexport))
				#define PSYCLE__PLUGIN__CALLING_CONVENTION __attribute__((__cdecl__))
			#elif defined _MSC_VER
				#define PSYCLE__PLUGIN__DYNAMIC_LINK__EXPORT __declspec(dllexport)
				#define PSYCLE__PLUGIN__CALLING_CONVENTION __cdecl
			#else
				#error please add definition for your compiler
			#endif

			#define PSYCLE__PLUGIN__DETAIL__STRINGIZED(x) PSYCLE__PLUGIN__DETAIL__STRINGIZED__NO_EXPANSION(x)
			#define PSYCLE__PLUGIN__DETAIL__STRINGIZED__NO_EXPANSION(x) #x
		
			const char get_info_function_name[] =
				PSYCLE__PLUGIN__DETAIL__STRINGIZED(PSYCLE__PLUGIN__SYMBOL_NAME__GET_INFO);
			typedef
				psycle::plugin_interface::CMachineInfo const * const
				(PSYCLE__PLUGIN__CALLING_CONVENTION * get_info_function)
				();
			
			const char create_machine_function_name[] =
				PSYCLE__PLUGIN__DETAIL__STRINGIZED(PSYCLE__PLUGIN__SYMBOL_NAME__CREATE_MACHINE);
			typedef
				psycle::plugin_interface::CMachineInterface *
				(PSYCLE__PLUGIN__CALLING_CONVENTION * create_machine_function)
				();

			const char delete_machine_function_name[] =
				PSYCLE__PLUGIN__DETAIL__STRINGIZED(PSYCLE__PLUGIN__SYMBOL_NAME__DELETE_MACHINE);
			typedef
				void
				(PSYCLE__PLUGIN__CALLING_CONVENTION * delete_machine_function)
				(psycle::plugin_interface::CMachineInterface &);
		}
	}
}

// for plugins that aren't namespace-aware
using psycle::plugin_interface::MI_VERSION;
using psycle::plugin_interface::MAX_TRACKS;
using psycle::plugin_interface::NOTE_MAX;
using psycle::plugin_interface::NOTE_NO;
using psycle::plugin_interface::NOTE_OFF;
using psycle::plugin_interface::MAX_BUFFER_LENGTH;
using psycle::plugin_interface::CMachineInfo;
using psycle::plugin_interface::GENERATOR;
using psycle::plugin_interface::EFFECT;
using psycle::plugin_interface::SEQUENCER;
using psycle::plugin_interface::CMachineInterface;
using psycle::plugin_interface::CMachineParameter;
using psycle::plugin_interface::MPF_LABEL;
using psycle::plugin_interface::MPF_STATE;
using psycle::plugin_interface::CFxCallback;
using psycle::plugin_interface::uint8; // deprecated anyway
using psycle::plugin_interface::uint16; // deprecated anyway
using psycle::plugin_interface::uint32; // deprecated anyway
