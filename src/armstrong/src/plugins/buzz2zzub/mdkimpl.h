/*
	This file is part of the Buzé base Buzz-library. 
	
	Please refer to LICENSE.TXT for details regarding usage.
*/

#pragma once

// Stuff adapted from the Buzz machines SDK

#define MDK_VERSION		1

class CMachine;

class CInput
{
public:
	CInput(char const *n, bool st) : Name(n), Stereo(st) {}

public:
	std::string Name;
	bool Stereo;
};

typedef std::list<CInput> InputList;
typedef std::list<CInput>::iterator InputListIterator;

class CMDKImplementation
{
public:
	virtual ~CMDKImplementation() { }

private:
	friend class CMDKMachineInterface;
	friend class CMDKMachineInterfaceEx;

	virtual void AddInput(char const *macname, bool stereo);	// denne er formideltligvis riktig plassert
	virtual void DeleteInput(char const *macname);
	
	// setmode var her som renameinput er nå. renameinput var utenfor alt. men dette virker rimelig, siden setmode er en intern funksjon
	virtual void RenameInput(char const *macoldname, char const *macnewname);	// denne er formideltligvis riktig plassert

	virtual void SetInputChannels(char const *macname, bool stereo);	// nå skal denne også være riktig

	// byttet save med input
	// og save med setmode
	// og setmode med setinputchannels

	// workmonotostereo byttes med save først, så bytter wmts og w plass
	virtual void Input(float *psamples, int numsamples, float amp);	// riktig
	virtual bool Work(float *psamples, int numsamples, int const mode); // riktig
	
	// disse to bytett plass:
	virtual bool WorkMonoToStereo(float *pin, float *pout, int numsamples, int const mode); // riktig
	virtual void Init(CMachineDataInput * const pi); 	// riktig plassert

	// nå er save på riktig plass (save byttet med savemode, før byttet med setoup)
	virtual void Save(CMachineDataOutput * const po);	// riktig plassert
	virtual void SetOutputMode(bool stereo); 	// riktig plassert

	// denne lå egentlig etter renameinput


	virtual void SetMode();

public:
	CMDKMachineInterface *pmi;

	InputList Inputs;
	InputListIterator InputIterator;

	int HaveInput;
	int numChannels;
	int MachineWantsChannels;

	CMachine* ThisMachine;
	
	float Buffer[2*MAX_BUFFER_LENGTH];

};

