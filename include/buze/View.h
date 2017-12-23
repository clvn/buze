#pragma once
// ---------------------------------------------------------------------------------------------------------------
// VIEW
// ---------------------------------------------------------------------------------------------------------------

class CView;
class CDocument;
class CApplication;
struct buze_event_data;

class CEventHandler {
public:
	~CEventHandler() {}
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) = 0;
};

class CViewInfo : public CEventHandler {
public:
	const char* uri;
	const char* label;
	const char* tooltip;
	int place; 
	int side;
	bool serializable;
	bool allowfloat;
	bool defaultview;
	int defaultfloatwidth;
	int defaultfloatheight;

	~CViewInfo() {}
	virtual void Attach() = 0; // for hooking up global menus, accelerators
	virtual void Detach() = 0;
	virtual CView* CreateView(HWND hWndParent, void* pCreateData) = 0;
	virtual void Destroy() = 0;
};

enum MachineParameterViewMode {
	parametermode_default,
	parametermode_internal_only,
	parametermode_custom_only,
	parametermode_both
};

class CViewFrame;

// The plugin DLL export "buze_create_viewlibrary"() returns a CViewLibrary.
// Initialize registers one or more CViewInfos with the host. Afterwards the 
// host discards the CViewLibrary immediately by calling Destroy().
class CViewLibrary {
public:
	enum {
		version = 6
	};
	virtual ~CViewLibrary() {}
	virtual void Initialize(CViewFrame* host) = 0;
	virtual void Destroy() = 0;
	virtual int GetVersion() = 0;
};

class CView : public CEventHandler {
public:
	virtual ~CView() {}
	virtual void UpdateTimer(int count) = 0;
	virtual HWND GetHwnd() = 0;
	virtual void GetClientSize(RECT* rc) = 0;
	virtual void GetHelpText(char* text, int* len) = 0;
	virtual bool DoesKeyjazz() = 0;
};

