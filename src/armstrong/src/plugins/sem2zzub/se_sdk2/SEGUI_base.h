#if !defined(_SEGUI_base_h_inc_)
#define _SEGUI_base_h_inc_

#include "SEGUI_struct_base.h"	// "c" interface
#include "SeSdk_String.h"
#include "SeGui_Pin.h"
#include <vector>

using namespace std;

// Needs to be defined by the audio effect and is
// called to create the audio effect object instance.
struct SEGUI_base;

long dispatchEffectClass(SEGUI_struct_base *e,long opCode, long index, long value, void *ptr, float opt);

struct SEGUI_base
{
friend long dispatchEffectClass(SEGUI_struct_base *e, long opCode, long index, long value, void *ptr, float opt);
public:
	long magic;
	SEGUI_base(seGuiCallback audioMaster, void *p_resvd1);
	virtual ~SEGUI_base();

	SEGUI_struct_base *getAeffect() {return &cEffect;}

	// called from audio master
	virtual long dispatcher(long opCode, long index, long value, void *ptr, float opt);
	virtual void Initialise(bool loaded_from_file);
	virtual void close() {};
	virtual void paint(HDC hDC, SEWndInfo *wi){};
	virtual void OnLButtonDown( SEWndInfo *wi, UINT nFlags, sepoint point ){};
	virtual void OnLButtonUp( SEWndInfo *wi, UINT nFlags, sepoint point ){};
	virtual void OnMouseMove( SEWndInfo *wi, UINT nFlags, sepoint point ){};
	virtual void OnWindowOpen(SEWndInfo *wi){};
	virtual void OnWindowClose(SEWndInfo *wi){};
	virtual bool OnIdle(void){return false;};
	virtual void OnNewConnection( int p_pin_idx ){};
	virtual void OnDisconnect( int p_pin_idx ){};
	void SetCapture(SEWndInfo *wi);
	void ReleaseCapture(SEWndInfo *wi);
	bool GetCapture(SEWndInfo *wi);
	long CallHost(long opcode, long index = 0, long value = 0, void *ptr = 0, float opt = 0);
	virtual void OnModuleMsg( int p_user_msg_id, int p_length, void * p_data ){};
	SeGuiPin * getPin(int index);//{ return &m_pins[index];};
	virtual void OnGuiPinValueChange(SeGuiPin *p_pin){};
	void AddGuiPlug(EPlugDataType p_datatype, EDirection p_direction, const char *p_name );
protected:	
	seGuiCallback seaudioMaster;
	SEGUI_struct_base cEffect;
private:
	void SetupPins(void);
//	vector<SeGuiPin> m_pins;
};

#endif
