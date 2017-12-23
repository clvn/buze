// SEPin.cpp: implementation of the SEPin class.
//
//////////////////////////////////////////////////////////////////////

#include "SEPin.h"
#include "SEModule_base.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEPin::SEPin()
{
}

void SEPin::Init( SEModule_base *module, int pin_idx, EPlugDataType datatype, void *variable_ptr )
{
	m_module = module;
	m_pin_idx = pin_idx;
	m_datatype = datatype;
	m_variable_ptr = variable_ptr;
	m_status = ST_STOP;

	// auto-duplicate pins hold their own pointer to the audio buffer
	if( m_variable_ptr == 0 )
	{
		switch(m_datatype)
		{
		case DT_FSAMPLE:
			m_autoduplicate_plug_var.float_ptr = (float*) m_module->CallHost(seaudioMasterGetPinVarAddress, m_pin_idx); 
			m_variable_ptr = &m_autoduplicate_plug_var.float_ptr;
			break;
		case DT_ENUM:
			m_autoduplicate_plug_var.enum_value = 0; 
			m_variable_ptr = &m_autoduplicate_plug_var.enum_value;
			break;
		case DT_BOOL:
			m_autoduplicate_plug_var.bool_value = false; 
			m_variable_ptr = &m_autoduplicate_plug_var.bool_value;
			break;
		case DT_FLOAT:
			m_autoduplicate_plug_var.float_value = 0.f; 
			m_variable_ptr = &m_autoduplicate_plug_var.float_value;
			break;
		case DT_INT:
			m_autoduplicate_plug_var.int_value = 0; 
			m_variable_ptr = &m_autoduplicate_plug_var.int_value;
			break;
/* don't work
		case DT_TEXT:
//			m_autoduplicate_plug_var.text_ptr = 0; 
			m_autoduplicate_plug_var.text_ptr = (char**) m_module->CallHost(seaudioMasterGetPinVarAddress, m_pin_idx); 
			m_variable_ptr = &m_autoduplicate_plug_var.text_ptr;
			break;
*/
		};
	}
	else
	{
		m_autoduplicate_plug_var.float_ptr = 0;
	}
}

SEPin::~SEPin()
{
}

void SEPin::TransmitStatusChange(unsigned long sampleclock, state_type new_state)
{
	m_module->CallHost(seaudioMasterSetPinStatus, m_pin_idx, new_state, (void *) sampleclock );
}

void SEPin::TransmitMIDI(unsigned long sampleclock, unsigned long msg)
{
	// MIDI data must allways be timestamped at or after the current 'clock'.
	assert(sampleclock >= m_module->SampleClock() );

	m_module->CallHost(seaudioMasterSendMIDI, m_pin_idx, msg, (void *) sampleclock );
}

void SEPin::OnStatusUpdate(state_type p_status )
{
	m_status = p_status;
/* done in host now
	switch( getDataType() )
	{
	case DT_ENUM:
		{
			*((short *)m_variable_ptr) = (short) p_new_value;
		}
		break;
	case DT_BOOL:
		{
			*((bool *)m_variable_ptr) = p_new_value != 0;
		}
		break;
	case DT_FLOAT:
		{
			*((float *)m_variable_ptr) =  *((float*)&p_new_value);
		}
		break;
	case DT_TEXT: // ask host to update the text value
		{
			getModule()->CallHost(seaudioMasterGetPinInputText, m_pin_idx, p_new_value, 0  );
		}
		break;
	}
*/
	getModule()->OnPlugStateChange( this );

	if( p_status == ST_ONE_OFF ) // one-offs need re-set once processed
	{
		m_status = ST_STOP;
	}
}

bool SEPin::IsConnected()
{
	return m_module->CallHost(seaudioMasterIsPinConnected, m_pin_idx, 0, 0 ) != 0;
}

float SEPin::getValue(void) // for audio plugs only
{
	assert( m_datatype == DT_FSAMPLE );
	unsigned int block_pos = m_module->CallHost(seaudioMasterGetBlockStartClock, m_pin_idx, 0, 0 );
	unsigned int sampleclock = m_module->SampleClock();
	float *buffer = *(float**)m_variable_ptr;
	return *( buffer + sampleclock - block_pos );
}
