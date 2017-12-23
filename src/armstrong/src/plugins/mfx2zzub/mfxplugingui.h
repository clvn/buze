#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlctrlx.h>

class CMfxPluginGui
	: public CWindowImpl<CMfxPluginGui>
{
public:
	//CTabView tabView;
	std::vector<CComPtr<IPropertyPage> > propPages;
	std::vector<PROPPAGEINFO> propPageInfos;
	mfxplugin* userplugin;

	DECLARE_WND_CLASS("CMfxContainer")

	BEGIN_MSG_MAP(CMfxPluginGui)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP()

	CMfxPluginGui() {}
	~CMfxPluginGui(void) {}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	bool CreatePropertyPages();
	void UpdateSize(int* outwidth, int* outheight);

};

