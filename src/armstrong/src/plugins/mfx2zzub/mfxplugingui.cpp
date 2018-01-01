#define _ATL_NO_UUIDOF
#define NOMINMAX
#include <algorithm>

using std::min;
using std::max;

#include <atlbase.h>
#include <wtl/atlapp.h>
#include <atlcom.h>
#include <cassert>
#include <vector>
#include <MidiFilter.h>
#include <zzub/plugin.h>
#include "plugincollection.h"
#include "mfxplugin.h"
#include "mfxplugingui.h"

LRESULT CMfxPluginGui::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
	userplugin = (mfxplugin*)pcs->lpCreateParams;

	LRESULT lres = DefWindowProc();

	//tabView.Create(m_hWnd, rcDefault, 0, WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN |TCS_TABS, 0, (HMENU)0);

	CreatePropertyPages();

	return lres;
}

LRESULT CMfxPluginGui::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	propPages.clear();
	userplugin = 0;
	return DefWindowProc();
}

LRESULT CMfxPluginGui::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);
	
	UpdateSize(&x, &y);
	return 0;
}

LRESULT CMfxPluginGui::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 1;
}


void CMfxPluginGui::UpdateSize(int* outwidth, int* outheight) {

	assert(outwidth);
	assert(outheight);

	// resize ourself if a fixed size in any direction was given

	PROPPAGEINFO& pageInfo = propPageInfos[0];
	//propPages[0]->GetPageInfo(&pageInfo);

	RECT rc = {0};
	rc.right = std::max((LONG)*outwidth, pageInfo.size.cx);
	rc.bottom = std::max((LONG)*outheight, pageInfo.size.cy);

	//MoveWindow(&rc, false);
	SetWindowPos(0, 0, 0, rc.right, rc.bottom, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOOWNERZORDER);

	propPages[0]->Move(&rc);

	if (outwidth) *outwidth = rc.right;
	if (outheight) *outheight = rc.bottom;

}

bool CMfxPluginGui::CreatePropertyPages() {

	assert(propPages.empty());

	CComPtr<ISpecifyPropertyPages> eventGui;
	if FAILED(userplugin->eventFilter.QueryInterface(&eventGui)) {
		return false;
	}

	CAUUID pages;
	if FAILED(eventGui->GetPages(&pages)) {
		return false;
	}
	
	// TODO: i cannot seem to find out why OleCreatePropertyFrame
	CComHeapPtr<GUID> elems(pages.pElems); // call CoTaskMemFree when going out of scope as per GetPages specification

	for (int i = 0; i < (int)pages.cElems; i++) {

		GUID propertyPageClsid = pages.pElems[i];

		CComPtr<IPropertyPage> propPage;
		if FAILED(propPage.CoCreateInstance(propertyPageClsid, 0, CLSCTX_INPROC))
			continue;

		propPage->SetPageSite(userplugin);

		CComPtr<IUnknown> filterUnknown(userplugin->eventFilter);
		propPage->SetObjects(1, &filterUnknown.p);
		
		propPages.push_back(propPage);

		PROPPAGEINFO pageInfo;
		propPage->GetPageInfo(&pageInfo);
		propPageInfos.push_back(pageInfo);
	}

	if (propPages.empty()) return false;

	//RECT rc;
	//GetClientRect(&rc);
	//SetRect(&rc, 0, 0, 300, 300);
	propPages[0]->Activate(m_hWnd, &rcDefault, FALSE);

	propPages[0]->Show(SW_SHOW);
	return true;
}
