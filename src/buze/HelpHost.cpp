#include "stdafx.h"
#include "HelpHost.h"
#include <boost/algorithm/string.hpp>

/*

plan for manual:
	- must render to web (with default hotkeys) and in chm (live hotkeys)
	- manual must contain tables with automatically updated hotkey listings and help texts
	- the web template and chm xslt templates must have access to the hotkeys
	- input xml gets two new tags: 
		- <hotkey view="" name="">default mapping</hotkey> - unchanged into the final html
		- <hotkey view="" /> - expands to HTML table with <hotkey>-tags
	- CHelpHost attaches to the helpviewer
		- on DocumentComplete: parse the html-tree and replaces the <hotkey>-contents with 
		live data from hotkeys.json
	- alternately, instead of <hotkey>, transform to <span class="hotkey">
	- help texts must be extracted from the .rc's? also used for tooltips and text in help-view

=> templatetools.js, transformFile(), add hotkeys.json to xslproc?

*/

using std::cout;
using std::wcout;
using std::endl;

HRESULT __stdcall CHelpHost::OnNavigateComplete2(LPDISPATCH pDisp, VARIANT* URL) {
	cout << "OnNavigateComplete2" << endl;
	return S_OK;
}

HRESULT __stdcall CHelpHost::OnDownloadComplete() {
	cout << "OnDownloadComplete" << endl;
	return S_OK;
}

HRESULT __stdcall CHelpHost::OnDocumentComplete(LPDISPATCH pDisp, VARIANT* URL) {
	cout << "OnDocumentComplete" << endl;
	CComQIPtr<IWebBrowser2> browser(pDisp);
	if (browser != NULL) {
		CComPtr<IDispatch> documentDispatch;
		if SUCCEEDED(browser->get_Document(&documentDispatch)) {
			CComQIPtr<IHTMLDocument2> document(documentDispatch);
			if (document != NULL) {
				CComPtr<IHTMLElement> body;
				if SUCCEEDED(document->get_body(&body)) {
					BSTR bodyInnerHtml;
					body->get_innerHTML(&bodyInnerHtml);
					wcout << bodyInnerHtml << endl;

					std::wstring bodyString(bodyInnerHtml);
					// TODO: search&replace hotkey strings
					//"{patternview|properties}" -> hotkeys.json -> "Alt+Backspace"
					boost::replace_all(bodyString, L"{patternview|properties}", "Alt+Backspace");
					body->put_innerHTML(CComBSTR(bodyString.c_str()));
					SysFreeString(bodyInnerHtml);
				}
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CHelpHost::QueryService(const GUID& sid, const IID& iid, void** ppv) {
	if (sid == GUID_NULL && iid == DIID_DWebBrowserEvents2)
		return QueryInterface(iid, ppv);
	return E_NOINTERFACE;
}