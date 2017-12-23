#if !defined(_SeGuiPin_h_inc_)
#define _SeGuiPin_h_inc_

#include "SeSdk_String.h"

struct SEGUI_base;

class SeGuiPin
{
public:
	void Init(int p_index, SEGUI_base *p_module){m_index=p_index;m_module=p_module;};
	void setValueText(const SeSdkString &p_new_val );
	SeSdkString getValueText(void);
	int getValueInt(void); // int, bool, and list type values
	void setValueInt( int p_new_val );
	float getValueFloat(void);
	void setValueFloat( float p_new_val );
	SeSdkString2 getExtraData(void);
	SEGUI_base *getModule(){return m_module;};
	int getIndex(){return m_index;};
	bool IsConnected();

private:
	int m_index;
	SEGUI_base *m_module;
};

#endif
