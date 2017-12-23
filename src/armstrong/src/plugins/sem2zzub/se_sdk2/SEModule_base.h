#if !defined(_SEModule_base_h_inc_)
#define _SEModule_base_h_inc_

#include "SEMod_struct_base.h"	// "c" interface
#include <string.h>

class SEModule_base;
class SEPin;
struct SEPinProperties;
struct SEModuleProperties;

// carefull of clash with SE's use of this
//typedef void (SEModule_base::* process_func_ptr)(long start_pos, long sampleframes); // Pointer to sound processing member function

// Needs to be defined by the audio effect and is
// called to create the audio effect object instance.
//SEModule_base* createEffectInstance (seaudioMasterCallback seaudioMaster);

// handy macro to assign a function pointer
#define SET_PROCESS_FUNC(func)	setSubProcess( static_cast <process_function_ptr2> (&func) )

typedef void (SEModule_base::*ug_func)(void); // Pointer to member function

// macro to run a Module function at a later time, specified as a sampleframe
#define RUN_AT(sample_clock, func) RunDelayed( sample_clock, static_cast <ug_func> (func) )

class function_pointer
{
public:
	function_pointer( SEModule_base *p_ug, ug_func p_func ) : ug(p_ug), func(p_func){};
	void Run(void) {(ug->*(func))();};
	SEModule_base *ug;
	ug_func func;
};

template< typename member_pointer_type > class my_delegate
{
public:
	my_delegate(member_pointer_type p_native = 0){ pointer.native = p_native;};
	#if defined(MINGW) || defined(__GNUC__)
		#if __GXX_ABI_VERSION > 101
    		my_delegate(void *p_raw){  pointer.native = 0; pointer.raw_data[0] = p_raw;};
    		void *RawPointer(void) {return pointer.raw_data[0];};
		#else
    		my_delegate(void *p_raw){  pointer.native = 0; pointer.raw_data[1] = p_raw;};
    		void *RawPointer(void) {return pointer.raw_data[1];};
		#endif
	#else
		my_delegate(void *p_raw){ pointer.native = 0; pointer.raw_data[0] = p_raw;};
		void *RawPointer(void) {return pointer.raw_data[0];};
	#endif
	union {
		member_pointer_type native;
		void * raw_data[2];
	}pointer; //different compilers use different layouts. may be more than 32 bits
};

long dispatchEffectClass(SEMod_struct_base *e,
	long opCode, long index, long value, void *ptr, float opt);
void processClassReplacing(SEMod_struct_base *e, float **inputs, float **outputs, long sampleFrames);

class SEModule_base
{
friend long dispatchEffectClass(SEMod_struct_base *e, long opCode, long index, long value, void *ptr, float opt);
friend void processClassReplacing(SEMod_struct_base *e, float **inputs, float **outputs, long sampleFrames);

public:
	SEModule_base(seaudioMasterCallback2 audioMaster, void *p_resvd1);
	virtual ~SEModule_base();
	void AddEvent( unsigned long p_time_stamp, int p_event_type, int p_int_parm1 = 0, int p_int_parm2 = 0, void *p_ptr_parm1 = 0);
	void RunDelayed( unsigned int sample_clock, ug_func (func) );
	virtual void OnPlugStateChange( SEPin *pin ){};

	SEMod_struct_base2 *getAeffect() {return &cEffect;}

	// called from audio master
	void SE_CALLING_CONVENTION process_idle(long start_pos, long sampleframes){};
	void SE_CALLING_CONVENTION private_HandleEvent(SeEvent *e); // divert to virtual function
	virtual void HandleEvent(SeEvent *e);
	
	virtual long dispatcher(long opCode, long index, long value, void *ptr, float opt);
	virtual void open();
	virtual void close() {};

	virtual void OnInputStatusChange( int plug_index, state_type new_state){};
	virtual void setSampleRate(float opt){sampleRate = opt;};
	virtual void setBlockSize( long bs){blockSize=bs;};
	// inquiry
	virtual float getSampleRate() {return sampleRate;}
	virtual long getBlockSize() {return blockSize;}
	long CallHost(long opcode, long index = 0, long value = 0, void *ptr = 0, float opt = 0);

	SEPin * getPin(int index);
	unsigned long SampleClock(); //{return m_sample_clock;};
//	void SetSampleClock(unsigned long p_clock){m_sample_clock = p_clock;};
//	virtual bool getModuleProperties (SEModuleProperties* properties) {return false;};
	virtual bool getPinProperties (long index, SEPinProperties* properties) {return false;}
	bool getPinProperties_clean (long index, SEPinProperties* properties);
	virtual bool getName (char* name) {return false;}		// name max 32 char
	virtual bool getUniqueId (char* name) {return false;}		// id max 32 char
	void setSubProcess( process_function_ptr2 p_function );
	process_function_ptr2 getSubProcess(void);
	virtual void Resume();
	virtual void VoiceReset(int future){};
	virtual void OnMidiData( unsigned long p_clock, unsigned long midi_msg, short pin_id ){};
	virtual void OnGuiNotify( int p_user_msg_id, int p_size, void *p_data ){};
protected:	
	seaudioMasterCallback2 seaudioMaster;
	SEMod_struct_base2 cEffect;
	float sampleRate;
	long blockSize;
	SEPin * m_pins;
private:
	my_delegate<process_function_ptr2> m_process_function_pointer;
};

#endif
