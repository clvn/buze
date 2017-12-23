#ifndef TUNINGCOLLECTION_H
#define TUNINGCOLLECTION_H

#include "tuning.h"
#include <vector>
#include <string>

class CTuningCollection;

namespace CTuningS11n
{
	void ReadTuning(istream& iStrm, CTuningCollection& Tc, const size_t);
};


//=====================
class CTuningCollection //To contain tuning objects.
//=====================
{
	friend class CTuningstreamer;
public:

//BEGIN TYPEDEFS
	typedef uint16 EDITMASK;
	//If changing this, see whether serialization should be 
	//modified as well.

	typedef vector<CTuning*> TUNINGVECTOR;
	typedef TUNINGVECTOR::iterator TITER; //Tuning ITERator.
	typedef TUNINGVECTOR::const_iterator CTITER;

	typedef uint16 SERIALIZATION_VERSION;

	typedef bool SERIALIZATION_RETURN_TYPE;

//END TYPEDEFS

//BEGIN PUBLIC STATIC CONSTS
public:
	enum 
	{
		EM_ADD = 1, //true <~> allowed
		EM_REMOVE = 2,
		EM_ALLOWALL = 0xffff,
		EM_CONST = 0,

	    s_SerializationVersion = 3,

		SERIALIZATION_SUCCESS = false,
		SERIALIZATION_FAILURE = true
	};

	static const TCHAR s_FileExtension[4];
	static const size_t s_nMaxTuningCount = 255;

//END PUBLIC STATIC CONSTS

//BEGIN INTERFACE:
public:
	CTuningCollection(const string& name = "");
	~CTuningCollection();
	
	//Note: Given pointer is deleted by CTuningCollection
	//at some point.
	bool AddTuning(CTuning* const pT);
	//bool AddTuning(istream& inStrm) {return AddTuning(inStrm, false);}
	
	bool Remove(const size_t i);
	bool Remove(const CTuning*);

	bool CanEdit(const EDITMASK& em) const {return (m_EditMask & em) != 0;}

	void SetConstStatus(const EDITMASK& em) {m_EditMask = em;}

	const EDITMASK& GetEditMask() const {return m_EditMask;}

	string GetEditMaskString() const;

	CTuning& GetTuning(size_t i) {return *m_Tunings.at(i);}
	const CTuning& GetTuning(size_t i) const {return *m_Tunings.at(i);}
	CTuning* GetTuning(const string& name);
	const CTuning* GetTuning(const string& name) const;

	size_t GetNumTunings() const {return m_Tunings.size();}

	const string& GetName() const {return m_Name;}

	void SetSavefilePath(LPCTSTR psz) {m_SavefilePath = psz;}
	const string& GetSaveFilePath() const {return m_SavefilePath;}

	//string GetVersionString() const {return Stringify(s_SerializationVersion);}

	size_t GetNameLengthMax() const {return 256;}

	//Serialization/unserialisation
	//bool Serialize(ostream&) const;
	//bool Serialize() const;
	//bool Deserialize(istream&);
	//bool Deserialize();

	//Transfer tuning pT from pTCsrc to pTCdest
	static bool TransferTuning(CTuningCollection* pTCsrc, CTuningCollection* pTCdest, CTuning* pT);
	

//END INTERFACE
	

//BEGIN: DATA MEMBERS
private:
	//BEGIN: SERIALIZABLE DATA MEMBERS
	TUNINGVECTOR m_Tunings; //The actual tuningobjects are stored as deletable pointers here.
	string m_Name;
	EDITMASK m_EditMask;
	//END: SERIALIZABLE DATA MEMBERS

	//BEGIN: NONSERIALIZABLE DATA MEMBERS
	TUNINGVECTOR m_DeletedTunings; //See Remove()-method for explanation of this.
	string m_SavefilePath;
	//END: NONSERIALIZABLE DATA MEMBERS
	
//END: DATA MEMBERS

	//friend void CTuningS11n::ReadTuning(istream& iStrm, CTuningCollection& Tc, const size_t);

//BEGIN PRIVATE METHODS
private:
	CTuning* FindTuning(const string& name) const;
	size_t FindTuning(const CTuning* const) const;

	//bool AddTuning(istream& inStrm, const bool ignoreEditmask);

	bool Remove(TITER removable, bool moveToTrashBin = true);

	//Hiding default operators because default meaning might not work right.
	CTuningCollection& operator=(const CTuningCollection&) {}
	CTuningCollection(const CTuningCollection&) {}

	//bool DeserializeOLD(istream&, bool& loadingSuccessful);

//END PRIVATE METHODS.
};


#endif
