#include "SEModule_base.h"
#include "SEPin.h"
#include "SEMod_struct.h"
#include <assert.h>
#include <malloc.h>

long dispatchEffectClass (SEMod_struct_base2 *e, long opCode, long index, long value, void *ptr, float opt)
{
	return ((SEModule_base *)(e->object))->dispatcher(opCode, index, value, ptr, opt);
}

SEModule_base::SEModule_base(seaudioMasterCallback2 p_seaudioMaster, void *p_resvd1) :
m_pins(0)
{
	seaudioMaster = p_seaudioMaster;

	memset(&cEffect, 0, sizeof(cEffect));
	cEffect.magic = SepMagic;

	m_process_function_pointer.pointer.native = &SEModule_base::process_idle;
	cEffect.sub_process_ptr = (long) m_process_function_pointer.RawPointer();

	event_function_ptr temp2 = (event_function_ptr) &SEModule_base::private_HandleEvent;
//	cEffect.event_handler_ptr = *reinterpret_cast<long*>( &temp2 );
	
	//test
	int * t = reinterpret_cast<int*>( &temp2 );
	/* old, don't work on newer gcc
#if defined(MINGW) || defined(__GNUC__)
	cEffect.event_handler_ptr = t[1];
#else
	cEffect.event_handler_ptr = t[0];
#endif
*/
	// new. extract pointer-to-member address
	if( t[0] != 0 && t[0] != 0xffff0000) // GNU c++ (old) had ffff0000 in first dword
	{
		cEffect.event_handler_ptr = t[0]; // MS and newer gcc (V 3.3.1 was first I noticed)
	}
	else
	{
		cEffect.event_handler_ptr = t[1]; // older gcc
	}

	cEffect.dispatcher = &dispatchEffectClass;
	cEffect.resvd1 = p_resvd1;
	cEffect.object = this;
	cEffect.version = 1;

	sampleRate = 44100.f;
	blockSize = 1024L;
}

SEModule_base::~SEModule_base()
{
	delete [] m_pins;
}

long SEModule_base::dispatcher(long opCode, long index, long value, void *ptr, float opt)
{
	long v = 0;
	switch(opCode)
	{
		case seffOpen:				open();												break;
		case seffClose:
			close();
			delete this;
			v = 1;
			break;
		case seffSetSampleRate:		setSampleRate(opt);									break;
		case seffSetBlockSize:		setBlockSize(value);								break;
		case seffGetPinProperties:
			v = getPinProperties (index, (SEPinProperties*)ptr) ? 1 : 0;

			// check for illegal flag combinations
			// 'Dual' Input plugs must be private (GuiModule can set the value, but User must not be able to)
//			assert( (((SEPinProperties*)ptr)->flags & IO_UI_COMMUNICATION_DUAL) == 0 || (((SEPinProperties*)ptr)->flags & IO_PRIVATE) != 0 || ((SEPinProperties*)ptr)->direction == DR_OUT );

			// 'Patch Store' Input plugs must be private, or GuiModule
			assert( (((SEPinProperties*)ptr)->flags & IO_PATCH_STORE) == 0 || (((SEPinProperties*)ptr)->flags & IO_PRIVATE) != 0 || ((SEPinProperties*)ptr)->direction == DR_OUT || ( ( (SEPinProperties*)ptr)->flags & IO_UI_COMMUNICATION) != 0 );
			break;
/* obsolete
		case seffGetModuleProperties:
			v = getModuleProperties ( (SEModuleProperties*)ptr) ? 1 : 0;
			break;
//		case seffInputStatusChange:
//			OnInputStatusChange(index, (state_type) value);
//			v = 0;
//			break;
		case seffGetEffectName:
			v = getName ((char *)ptr) ? 1 : 0;
			break;
		case seffGetUniqueId:
			v = getUniqueId ((char *)ptr) ? 1 : 0;
			break;
*/
		case seffAddEvent:
			{
				assert(false); // not used in SDK2
/*
				SeEvent *e = (SeEvent *)ptr;

				SeEvent *ne = new SeEvent(e->time_stamp, e->event_type, e->int_parm1, e->int_parm2, e->ptr_parm1 );
				// can't directly use object allocated by host, make a copy
				AddEvent( ne );
*/
			}
			break;
		case seffResume:
			// index = 0 - Module initiated sleep
			// index = 1 - Voice initiated sleep. Indicates new note.
			Resume();
			if( index > 0 )
			{
				VoiceReset(index);
			}
			break;
		case seffIsEventListEmpty: // not used in SDK2
			assert(false);
//			return events == 0 ? 1 : 0;
			break;
/* retired
		case seffGetSdkVersion:
			return SDK_VERSION;
			break;
*/
		case seffGuiNotify:
			OnGuiNotify(value, index, ptr);
			break;
		case seffQueryDebugInfo:
			{
				static int info[4];
				info[0] = 1; // version number
				info[1] = sizeof(process_function_ptr2);
				info[2] = (int) &m_process_function_pointer.pointer;
#if defined( __GXX_ABI_VERSION )
				info[3] = __GXX_ABI_VERSION;
#else
				info[3] = 0;
#endif
				return (long) info;
			}
			break;
	}
	return v;
}

SEPin * SEModule_base::getPin(int index)
{
	assert( m_pins != 0 );
	return m_pins + index;
}

long SEModule_base::CallHost(long opcode, long index, long value, void *ptr, float opt)
{
	if (seaudioMaster)
		return seaudioMaster (&cEffect, opcode, index, value, ptr, opt);

	return 0;
}

unsigned long SEModule_base::SampleClock()
{
	return CallHost(seaudioMasterGetSampleClock);
}

void SEModule_base::open()
{
	// set sampleclock
//	SetSampleClock( CallHost(seaudioMasterGetSampleClock, 0, 0, 0 ) );

	// get actual number of pins used (may be more or less if auto-duplicating plugs used)
	int actual_plug_count = CallHost(seaudioMasterGetTotalPinCount);

	if( actual_plug_count > 0 )
	{
		m_pins = new SEPin[actual_plug_count];
	}

	// Set up standard plugs
	int plug_description_index = 0;
	int i = 0;
	SEPinProperties properties;

	while( getPinProperties_clean(plug_description_index, &properties) && i < actual_plug_count )
	{
		if( (properties.flags & IO_UI_COMMUNICATION) == 0 || (properties.flags & IO_UI_DUAL_FLAG) != 0 ) // skip GUI plugs
		{
			m_pins[i].Init(this, i, properties.datatype, properties.variable_address );
			i++;
		}
		plug_description_index++;
	}

	// now set up any additional 'autoduplicate' plugs
	// Assumed they are the last plug described in getPinProperties

	// Get the properites of last pin
	getPinProperties_clean(plug_description_index-1, &properties);

	if( (properties.flags & IO_UI_COMMUNICATION) == 0 || (properties.flags & IO_UI_DUAL_FLAG) != 0 ) // skip GUI plugs
	{
		if( (properties.flags & IO_AUTODUPLICATE) != 0 )
		{
			while( i < actual_plug_count )
			{
				m_pins[i].Init(this, i, properties.datatype, 0);
				i++;
			}
		}
	}
}

// gets a pins properties, after clearing the structure (prevents garbage getting in)
bool SEModule_base::getPinProperties_clean (long index, SEPinProperties* properties)
{
	memset( properties, 0, sizeof(SEPinProperties) ); // clear structure
	return getPinProperties(index, properties);
}

/*
void SE_CALLING_CONVENTION SEModule_base::processReplacing(long buffer_offset, long sampleFrames)
{
	assert( sampleFrames > 0 );

	unsigned long current_sample_clock = SampleClock();
	unsigned long end_time = current_sample_clock + sampleFrames;

	for(;;)
	{
		if( events == 0 ) // fast version, when no events on list.
		{
			assert(sampleFrames > 0 );
			(this->*(process_function))( buffer_offset, sampleFrames );
			SetSampleClock( end_time );
			return;
		}

		unsigned long delta_time = sampleFrames;

		SeEvent *next_event = events;
		
		assert(next_event->time_stamp >= current_sample_clock );
		
		if( next_event->time_stamp < end_time ) // will happen in this block
		{
			delta_time = next_event->time_stamp - current_sample_clock;
			// no, sub_process needs to know if event pending
			// events.RemoveHead();
		}

		if( delta_time > 0 )
		{
			//assert(delta_time > 0 ); // it's unsigned silly
			(this->*(process_function))( buffer_offset, delta_time );
			sampleFrames -= delta_time;
			current_sample_clock += delta_time;
			SetSampleClock( current_sample_clock );

			if( sampleFrames == 0 ) // done
			{
				return;
			}
			buffer_offset += delta_time;
		}

		SeEvent *e = events;
		events = events->next;
		HandleEvent(e);
		delete e;
	}
}
*/
void SEModule_base::HandleEvent(SeEvent *e)
{
	assert( e->time_stamp == SampleClock() );
	switch( e->event_type )
	{
		case UET_STAT_CHANGE:
			{
				getPin((int) e->ptr_parm1)->OnStatusUpdate( (state_type) e->int_parm1 );	
			}
			break;
/*
		case UET_RUN_FUNCTION: // buggy ( dll can't allocate mem and attach to event, causes crash when SE trys to free it)
			{
				// ptr_parm1 points to a ug_func pointer (allocated)
				function_pointer *fp = (function_pointer*) e->ptr_parm1;
				fp->Run();

				// TODO!!!!would be better to perform deletion in event detructor
				// will prevent mem leaks on events that are deleted without being used (due to power-off situation)
				delete fp;
				e->ptr_parm1 = NULL;
			}
			break;
*/
		case UET_RUN_FUNCTION2:
			{
//				ug_func func = 0; // important to initialise (code below is a hack)
//				*( (int*) &func) = *( (int*) &(e->ptr_parm1));
//				(this->*(func))();
				my_delegate<ug_func> temp_delegate(e->ptr_parm1);
//				temp_delegate.pointer.native = 0;
//				temp_delegate.pointer.raw_pointer = e->ptr_parm1;
				(this->*(temp_delegate.pointer.native))();
			}
			break;
/*	case UET_UI _NOTIFY2:
			OnGui Notify( e->int_parm1, (void *) e->int_parm2 );
			free( (void *) e->int_parm2 ); // free memory block
			break;*/
	case UET_PROG_CHANGE: // do nothing
		break;
	case UET_MIDI:
		OnMidiData( e->time_stamp, e->int_parm1, e->int_parm2 );
		break;
	default:
		;
//		assert( false ); // un-handled event
	};
}

void SEModule_base::RunDelayed( unsigned int sample_clock, ug_func (func) )
{
	my_delegate<ug_func> temp_delegate(func);
//	temp_delegate.pointer.native = func;
/*
	// NO, can't allocate events here (in dll)
//	function_pointer *fp = new function_pointer( this, func );
	void *function_address;
	*( (int*) &(function_address)) = *( (int*) &func); // copy first 32 bits of function pointer
*/
	AddEvent( sample_clock, UET_RUN_FUNCTION2, 0, 0, temp_delegate.RawPointer());
}

// insert event sorted.  Pass pointer to tempory event structure(event data will be copied by SE)
void SEModule_base::AddEvent(unsigned long p_time_stamp, int p_event_type, int p_int_parm1, int p_int_parm2, void *p_ptr_parm1)
{
	assert( p_time_stamp >= SampleClock() );

	SeEvent temp( p_time_stamp, p_event_type , p_int_parm1, p_int_parm2, p_ptr_parm1);
	CallHost(seaudioMasterAddEvent,0,0,&temp);

//	delete p_event;

/*
	unsigned long time_stamp = p_event->time_stamp;

	SeEvent *e = events;
	SeEvent *prev = 0;
	while(true)
	{
		if( e == 0 || e->time_stamp > time_stamp ) // events with same time must be added in same order received (for stat changes to work properly)
		{
			p_event->next = e;
			if( prev )
			{
				prev->next = p_event;
			}
			else
			{
				events = p_event;
			}
			return;
		}
		prev = e;
		e = e->next;
	}*/
}

void SEModule_base::Resume() // from either sleep or suspend
{
/*
	SetSampleClock( CallHost(seaudioMasterGetSampleClock, 0, 0, 0 ) );

	// update the time on any pending events
	// this applies to resume from suspend only
	SeEvent *e = events;
	while(e)
	{
		if( e->time_stamp < SampleClock() )
		{
			e->time_stamp = SampleClock();
		}
		e = e->next;
	}
*/
}

void SEModule_base::setSubProcess( process_function_ptr2 p_function )
{
	if( m_process_function_pointer.pointer.native != p_function )
	{
		m_process_function_pointer.pointer.native = p_function;

		cEffect.sub_process_ptr = (long)m_process_function_pointer.RawPointer();

		CallHost(seaudioMasterSetProcessFunction,0,0,0);
	}
}

process_function_ptr2 SEModule_base::getSubProcess(void)
{
	return m_process_function_pointer.pointer.native;
}

void SE_CALLING_CONVENTION SEModule_base::private_HandleEvent(SeEvent *e)
{
	HandleEvent(e);
} // divert to virtual function
