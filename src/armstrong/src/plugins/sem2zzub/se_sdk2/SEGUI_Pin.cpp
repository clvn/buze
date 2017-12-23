#include "SeGui_Pin.h"
#include "SEGUI_base.h"

void SeGuiPin::setValueText(const SeSdkString &p_new_val )
{
	getModule()->CallHost(seGuiHostPlugSetValText, getIndex(), 0, (void *) p_new_val.data() );
}

SeSdkString SeGuiPin::getValueText(void)
{
	// warning, unstable over 2000 bytes  ( that's 1000 UNICODE characters )
	SeSdkString temp;
	temp.assign( (char *) getModule()->CallHost(seGuiHostPlugGetValText, getIndex(), 0, 0 ));
	return temp;
}

int SeGuiPin::getValueInt(void) // int, bool, and list type values
{
	int temp;
	getModule()->CallHost(seGuiHostPlugGetVal, getIndex(), 0, &temp );
	return temp;
}

float SeGuiPin::getValueFloat(void)
{
	float temp;
	getModule()->CallHost(seGuiHostPlugGetVal, getIndex(), 0, &temp );
	return temp;
}

void SeGuiPin::setValueFloat( float p_new_val )
{
	getModule()->CallHost(seGuiHostPlugSetVal, getIndex(), 0, 0, p_new_val );
}

void SeGuiPin::setValueInt( int p_new_val )
{
	getModule()->CallHost(seGuiHostPlugSetVal, getIndex(), p_new_val, 0 );
}

SeSdkString2 SeGuiPin::getExtraData(void)
{
	int string_length = getModule()->CallHost( seGuiHostPlugGetExtraData, getIndex(), 0, 0);
	wchar_t *temp = new wchar_t[string_length];

	getModule()->CallHost(seGuiHostPlugGetExtraData, getIndex(), string_length, temp );
	SeSdkString2 temp2(temp);
	delete [] temp;
	return temp2;
}

bool SeGuiPin::IsConnected()
{
	return 0 != getModule()->CallHost( seGuiHostPlugIsConnected, getIndex(), 0, 0 );
}

