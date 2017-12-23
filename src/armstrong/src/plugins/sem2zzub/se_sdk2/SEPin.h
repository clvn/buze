// SEPin.h: interface for the SEPin class.
//
//////////////////////////////////////////////////////////////////////
#if !defined(_SEPin_h_inc_)
#define _SEPin_h_inc_

#include "se_datatypes.h"

class SEModule_base;

typedef	union
{
	float *float_ptr;
	short enum_value;
	bool bool_value;
	float float_value;
	char **text_ptr;
	int int_value;
}AutoduplicatePlugData;

class SEPin  
{
public:
	bool IsConnected();
	SEPin();
	virtual ~SEPin();
	void Init( SEModule_base *module, int pin_idx, EPlugDataType datatype, void *variable_ptr );
	void OnStatusUpdate(state_type p_status );
	state_type getStatus(void){return m_status;};
	float getValue(void); // for audio plugs only
//	AutoduplicatePlugData getValueNonAudio(void){return m_autoduplicate_plug_var;};
	EPlugDataType getDataType(void){return m_datatype;};
	void TransmitStatusChange(unsigned long sampleclock, state_type new_state);
	void TransmitMIDI(unsigned long sampleclock, unsigned long msg);
	SEModule_base *getModule(void){return m_module;};
	int getPinID(){return m_pin_idx;};
	void *GetVariableAddress(){return m_variable_ptr;};
private:
	void *m_variable_ptr;
	SEModule_base *m_module;
	int m_pin_idx;
	state_type m_status;
	EPlugDataType m_datatype;
	AutoduplicatePlugData m_autoduplicate_plug_var; // Holds pointer to buffer (auto duplicate plugs only)
};

#endif
