#pragma once

struct CUpdateData;

#include "PropertyList/PropertyList.h"

inline bool isVariantNumber(const VARIANT& v) {
	switch (v.vt) {
		case VT_I1:
		case VT_I2:
		case VT_I4:
		case VT_I8:
		case VT_UI1:
		case VT_UI2:
		case VT_UI4:
		case VT_UI8:
		case VT_INT:
			return true;
		default:
			return false;
	}
}

class PropertyInfoBase {
protected:
	VARIANT value;
public:
	PropertyInfoBase() { VariantInit(&value); }
	virtual ~PropertyInfoBase() { VariantClear(&value); }
	//const char* name;
	std::string name;
	std::string description;

	virtual HPROPERTY create() = 0;
	virtual VARIANT& get() = 0;
	virtual bool set(VARIANT v) = 0;
};

class CPropertyProvider {
public:
	CView* hReturnView;
	std::vector<PropertyInfoBase*> properties;
	std::string name;
	virtual ~CPropertyProvider();
	virtual void createProperties() = 0;
	virtual void destroyProperties();
	virtual int getProperties() = 0;
	virtual PropertyInfoBase* getProperty(int index) = 0;
	virtual bool checkUpdate(LPARAM lHint, LPVOID pHint) = 0;
};


/***

	ObjectPropertyInfo

	... and misc other properties for the properties view

***/

template <class P, typename valuetype>
class ObjectPropertyInfo : public PropertyInfoBase {
public:
	P* frame;

	ObjectPropertyInfo(P* frame, const char* n, valuetype (P::*getter)(), bool (P::*setter)(valuetype), std::string description = "") {
		this->frame = frame;
		this->name = n;
		this->getter = getter;
		this->setter = setter;
		this->description = description;
	}
	valuetype (P::*getter)();
	bool (P::*setter)(valuetype);

	virtual VARIANT& get() = 0;
	virtual bool set(VARIANT v) = 0;
};

template <class P> 
class IntPropertyInfo : public ObjectPropertyInfo<P, int> {
public:
	IntPropertyInfo(P* frame, const char* n, int (P::*getter)(), bool (P::*setter)(int), std::string description = ""):ObjectPropertyInfo<P, int>(frame, n, getter, setter, description) { }
	virtual VARIANT& get() {
		VariantClear(&value);
		value.vt = VT_I4;
		value.intVal = (frame->*getter)();
		return value;
	}

	bool set(VARIANT v) {
		if (setter == 0) return false;
		if (isVariantNumber(v)) {
			VariantCopy(&value, &v);
			int i = v.intVal;
			return (frame->*setter)(i);
		}
		return false;
	}
	HPROPERTY create() {
		VARIANT& v = get();
		if (this->setter == 0) {
			char pc[16];
			itoa(v.intVal, pc, 10);
			return PropCreateReadOnlyItem(name.c_str(), pc);
		} else
			return PropCreateVariant(name.c_str(), v);
	}

};

template <class P> 
class StringPropertyInfo : public ObjectPropertyInfo<P, std::string> {
public:
	StringPropertyInfo(P* frame, const char* n, std::string (P::*getter)(), bool (P::*setter)(std::string), std::string description = ""):ObjectPropertyInfo<P, std::string>(frame, n, getter, setter, description) { 
	}
	virtual VARIANT& get() {
		// convert from std::string to VARIANT
		VariantClear(&value);

		std::string v = (frame->*getter)();
		size_t len = v.length();

		BSTR str = SysAllocStringLen(0, (UINT)len);
		if (str == 0) return value;

		mbstowcs(str, v.c_str(), len+1);

		value.vt = VT_BSTR;
		value.bstrVal = str;

		return value;
	}
	bool set(VARIANT v) {
		if (setter == 0) return false;
		if (v.vt == VT_BSTR) {
			VariantCopy(&value, &v);
			size_t len = wcslen(value.bstrVal);
			char* pc = new char[len+1];
			wcstombs(pc, value.bstrVal, len+1);
			std::string str = pc;
			delete[] pc;
			len = 0;
			return (frame->*setter)(str);
		}
		return false;
	}

	HPROPERTY create() {
		USES_CONVERSION;
		VARIANT& v = get();
		if (setter == 0 && v.vt == VT_BSTR) {
			return PropCreateReadOnlyItem(name.c_str(), W2T(v.bstrVal));
		} else
			return PropCreateVariant(name.c_str(), v);
	}
};

template <class P>
class FileNamePropertyInfo : public StringPropertyInfo<P> {
public:
	std::string defext;
	FileNamePropertyInfo(P* frame, const char* n, LPCTSTR lpszDefExt, std::string (P::*getter)(), bool (P::*setter)(std::string), std::string description = "" ):StringPropertyInfo<P>(frame, n, getter, setter, description) { 
		defext = lpszDefExt;
	}
	HPROPERTY create() {
		USES_CONVERSION;
		VARIANT& v = get();
		return PropCreateFileName(name.c_str(), W2CT(v.bstrVal));
	}

	bool set(VARIANT v) {
		USES_CONVERSION;

		if (setter == 0) return false;
		if (v.vt == VT_BSTR) {
			CFileDialog ofd(TRUE, defext.c_str(), W2T(v.bstrVal));
			if (ofd.DoModal()) {
				return (frame->*setter)(ofd.m_szFileName);
			}
		}
		return false;
	}
};

template <class P> 
class BoolPropertyInfo : public ObjectPropertyInfo<P, BOOL> {
public:
	BoolPropertyInfo(P* frame, const char* n, BOOL (P::*getter)(), bool (P::*setter)(BOOL), std::string description = ""):ObjectPropertyInfo<P, BOOL>(frame, n, getter, setter, description) { 
	}
	virtual VARIANT& get() {
		VariantClear(&value);
		BOOL v = (frame->*getter)();
		value.vt = VT_BOOL;
		value.boolVal = v;
		return value;
	}
	bool set(VARIANT v) {
		if (setter == 0) return false;
		if (v.vt == VT_BOOL) {
			VariantCopy(&value, &v);
			return (frame->*setter)(v.boolVal!=VARIANT_FALSE);
		}
		return false;
	}

	HPROPERTY create() {
		USES_CONVERSION ;
		VARIANT& v = get();
		HPROPERTY hprop = PropCreateCheckButton(name.c_str(), v.boolVal!=VARIANT_FALSE);
		return hprop;
	}
};

template <class P> 
class NotePropertyInfo : public ObjectPropertyInfo<P, int> {
public:
	HWND hWnd;
	CListBox listBox;

	NotePropertyInfo(HWND hWnd, P* frame, const char* n, int (P::*getter)(), bool (P::*setter)(int), std::string description = ""):ObjectPropertyInfo<P, int>(frame, n, getter, setter, description) { 
		this->hWnd = hWnd;
	}
	virtual VARIANT& get() {
		value.vt = VT_I4;
		value.intVal = (frame->*getter)();
		return value;
	}

	bool set(VARIANT v) {
		if (setter == 0) return false;
		// the value is the index of the note list
		if (isVariantNumber(v)) {
			VariantCopy(&value, &v);
			int i = linearNoteToBuzz(v.intVal); 
			return (frame->*setter)(i);
		}
		return false;
	}

	int linearNoteToBuzz(int v) {
		int note=(v%12)+1;
		int oct=v/12;
		return (note) + (oct<<4);
	}

	int buzzNoteToLinear(int v) {
		int note=(v&0xF)-1;
		int oct=(v&0xF0) >> 4;
		return note+12*oct;
	}

	HPROPERTY create() {
		VARIANT& v = get();
		std::vector<std::string> notes;
		for (int i=0; i<16*12; i++) {
			notes.push_back(noteFromInt(linearNoteToBuzz(i)));
		}
		LPCTSTR* data = new LPCTSTR[16*12 + 1];
		for (int i=0; i<16*12; i++) {
			data[i] = notes[i].c_str();
		}
		data[16*12] = 0;

		int linearNote = buzzNoteToLinear(v.intVal);
		HPROPERTY hProp = PropCreateList(name.c_str(), data, linearNote);
		delete[] data;
		return hProp;
	}

};

template <class P> 
class StringListPropertyInfo : public ObjectPropertyInfo<P, int> {
public:
	CListBox listBox;
	std::vector<std::string> (P::*itemsgetter)();

	StringListPropertyInfo(P* frame, const char* n, int (P::*getter)(), bool (P::*setter)(int), std::vector<std::string> (P::*itemsgetter)(), std::string description = ""):ObjectPropertyInfo<P, int>(frame, n, getter, setter, description) { 
		this->itemsgetter = itemsgetter;
	}
	virtual VARIANT& get() {
		value.vt = VT_I4;
		value.intVal = (frame->*getter)();
		return value;
	}

	bool set(VARIANT v) {
		if (setter == 0) return false;
		// the value is the index of the note list
		if (isVariantNumber(v)) {
			VariantCopy(&value, &v);
			int i = v.intVal; 
			return (frame->*setter)(i);
		}
		return false;
	}

	std::vector<std::string> items;

	HPROPERTY create() {
		VARIANT& v = get();
		items = (frame->*itemsgetter)();

		LPCTSTR* data = new LPCTSTR[items.size() + 1];
		for (size_t i = 0; i < items.size(); i++) {
			data[i] = items[i].c_str();
		}
		data[items.size()] = 0;

		int cursel = v.intVal;
		HPROPERTY hProp = PropCreateList(name.c_str(), data, cursel);
		delete[] data;
		return hProp;
	}

};

class CategoryPropertyInfo : public PropertyInfoBase {
public:
	CategoryPropertyInfo(std::string name) {
		this->name = name;
	}

	virtual HPROPERTY create() {
		return PropCreateCategory(name.c_str());
	}

	virtual VARIANT& get() {
		static VARIANT v;
		VariantInit(&v);
		return v;
	}

	virtual bool set(VARIANT v) {
		return false;
	}
};

template <class P> 
class ColorPropertyInfo : public ObjectPropertyInfo<P, COLORREF> {
public:
	HPROPERTY hProp;
	ColorPropertyInfo(P* frame, const char* n, COLORREF (P::*getter)(), bool (P::*setter)(COLORREF)):ObjectPropertyInfo<P, COLORREF>(frame, n, getter, setter) { }

	virtual VARIANT& get() {
		VariantClear(&value);
		value.vt = VT_UI4;
		value.uintVal = (frame->*getter)();
		return value;
	}

	bool set(VARIANT v) {
		if (setter == 0) return false;
		if (isVariantNumber(v)) {
			VariantCopy(&value, &v);
			unsigned int i = v.uintVal;
			return (frame->*setter)(i);
		}
		return false;
	}

	HPROPERTY create() {
		VARIANT& v = get();
		char pc[16];
		itoa(v.uintVal, pc, 10);
		
		hProp = PropCreateColor(name.c_str(), v.uintVal);
		return hProp;
	}
};
