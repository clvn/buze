#include "SEGUI_base.h"
#include "SEMod_struct_base.h"
#include <assert.h>
#include <string.h> // 4 memset

long dispatchEffectClass (SEGUI_struct_base *e, long opCode, long index, long value, void *ptr, float opt)
{
	return ((SEGUI_base *)e->object)->dispatcher(opCode, index, value, ptr, opt);
}

SEGUI_base::SEGUI_base(seGuiCallback seaudioMaster, void *p_resvd1)
{
	this->seaudioMaster = seaudioMaster;

	memset(&cEffect, 0, sizeof(cEffect));
	cEffect.magic = SepMagic2;
	cEffect.dispatcher = &dispatchEffectClass;
	cEffect.object = this;
	cEffect.resvd1 = p_resvd1;
	cEffect.version = 1;
}

SEGUI_base::~SEGUI_base()
{
}

long SEGUI_base::dispatcher(long opCode, long index, long value, void *ptr, float opt)
{
	long v = 0;
	switch(opCode)
	{
		case seGuiInitialise:
			Initialise(index == 1);
			break;
		case seGuiClose:
			{
				close();
				delete this;
				v = 1; // this object now deleted, must do nothing more.
			}
			break;
		case seGuiPaint:
			paint( (HDC) index, (SEWndInfo*) ptr );
			break;
		case seGuiLButtonDown:
		  {
		        sepoint temp(index,value);
// I don't know why fcn gcc don't compile this		        OnLButtonDown( (SEWndInfo*) ptr, *(int*) &opt ,sepoint(index, value) );
		        OnLButtonDown( (SEWndInfo*) ptr, *(int*) &opt , temp );
		  } 			
			break;
		case seGuiLButtonUp:
		  {
		        sepoint temp(index,value);
//			OnLButtonUp( (SEWndInfo*) ptr, *(int*) &opt ,sepoint(index,value) );
		        OnLButtonUp( (SEWndInfo*) ptr, *(int*) &opt , temp );
		  } 			
			break;
		case seGuiMouseMove:
		  {
		        sepoint temp(index,value);
//			OnMouseMove( (SEWndInfo*) ptr, *(int*) &opt ,sepoint(index,value) );
		        OnMouseMove( (SEWndInfo*) ptr, *(int*) &opt , temp );
		  } 			
			break;
		case seGuiOnModuleMessage:
			OnModuleMsg(value,index,ptr);
			break;
		case seGuiOnGuiPlugValueChange:
			{
				SeGuiPin pin;
				pin.Init(index,this);
				OnGuiPinValueChange(&pin);
			}
			break;
		case seGuiOnWindowOpen:
			OnWindowOpen( (SEWndInfo*) ptr );
			break;
		case seGuiOnWindowClose:
			OnWindowClose( (SEWndInfo*) ptr );
			break;
		case seGuiOnIdle:
			return OnIdle();
			break;
		case seGuiOnNewConnection:
			OnNewConnection(index);
			break;
		case seGuiOnDisconnect:
			OnDisconnect(index);
			break;
/*
		case seffSetSampleRate:		setSampleRate(opt);									break;
		case seffSetBlockSize:		setBlockSize(value);								break;
		case seffGe tPinProperties:
			v = getPin Properties (index, (SEPinProperties*)ptr) ? 1 : 0;
			break;
		case seffGet ModuleProperties:
			v = get ModuleProperties ( (SEModuleProperties*)ptr) ? 1 : 0;
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
		case seffAddEvent:
			{
				SeEvent *e = (SeEvent *)ptr;
				// can't directly use object allocated by host, make a copy
				AddEvent( new SeEvent(e->time_stamp, e->event_type, e->int_parm1, e->int_parm2, e->ptr_parm1 ));
			}
			break;
		case seffResume:
			Res ume();
			break;
		case seffIsEventListEmpty:
			return events == 0 ? 1 : 0;
			break;
		case seffGetSdkVersion:
			return SDK _VERSION;
			break;
//		case seffIdentify:			v = 'NvEf';											break;
*/
	}
	return v;
}

long SEGUI_base::CallHost(long opcode, long index, long value, void *ptr, float opt)
{
	assert (seaudioMaster);
	return seaudioMaster (&cEffect, opcode, index, value, ptr, opt);
}

void SEGUI_base::Initialise(bool loaded_from_file)
{
	SetupPins();
}

void SEGUI_base::AddGuiPlug(EPlugDataType p_datatype, EDirection p_direction, const char *p_name )
{
	CallHost(seGuiHostAddGuiPlug, p_datatype, p_direction, (void*)p_name );
	SetupPins();
}

void SEGUI_base::SetupPins(void)
{
	// commented out, may need reinstating if pins ever gain state
	/*
	// get actual number of pins used (may be more or less if auto-duplicating plugs used)
	int actual_plug_count = CallHost(seGuiHostGetTotalPinCount);

	m_pins.resize(actual_plug_count);

	for( int i = 0 ; i < actual_plug_count; i++ )
	{
		m_pins[i].Init(i,this);
	}
	*/
}

SeGuiPin * SEGUI_base::getPin(int index)
{
	// there are no pins.
	// pins currently hold no state, implement them as a flyweight (saves having to track pin add/remove, we're not notified of autoduplicate add/remove anyhow)
	static SeGuiPin pin;
	pin.Init(index,this);
	return &pin;
}

// capture mouse movement
void SEGUI_base::SetCapture(SEWndInfo *wi)
{
	CallHost(seGuiHostSetCapture, 0, 0, (void*)wi );
}

// release capture mouse movement
void SEGUI_base::ReleaseCapture(SEWndInfo *wi)
{
	CallHost(seGuiHostReleaseCapture, 0, 0, (void*)wi );
}

// query mouse capture state
bool SEGUI_base::GetCapture(SEWndInfo *wi)
{
	return 0 != CallHost(seGuiHostGetCapture, 0, 0, (void*)wi );
}
