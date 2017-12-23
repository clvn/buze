#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlframe.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <string>
#include "resource.h"
#include "DragDropImpl.h"
#include "MachineDropTarget.h"
#include "utils.h"
#include <iomanip>
#include <strstream>

/***

	CDropTarget

***/

CDropTarget::CDropTarget(CDropTargetWindow* dtw):CIDropTarget(dtw->hWndTarget) {
	dropTargetWindow = dtw;
}

bool CDropTarget::OnDragOver(const POINTL& pt, FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect) {
	return dropTargetWindow->OnDragOver(pt, pFmtEtc, medium, pdwEffect);
}

void CDropTarget::OnDragLeave() {
	dropTargetWindow->OnDragLeave();
}

bool CDropTarget::OnDrop(const POINTL& p, FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect) {
	return dropTargetWindow->OnDrop(p, pFmtEtc, medium, pdwEffect);
}

/***

	CMachineDropTargetWindow

***/

bool CMachineDropTargetWindow::OnDrop(const POINTL& p, FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect) {

	if (pFmtEtc->cfFormat == CF_TEXT && medium.tymed == TYMED_HGLOBAL) {
		// get machine name, create a machine instance, set x and y to mouse positions and make sure the machine view is redrawn
		char* pStr = (char*)GlobalLock(medium.hGlobal);
		if(pStr != NULL) {
			std::string machineName = pStr;
			std::string instrumentName = "";
			// if this is a machine+instrument, there is a pipe in the name, f.ex Polav PVST 1.1|AbsynthVST
			int fs=machineName.find_first_of('|');
			if (fs!=-1) {
				instrumentName=machineName.substr(fs+1);
				machineName=machineName.substr(0, fs);
			}
			POINT pt = { p.x, p.y};
			ScreenToClient(hWndTarget, &pt);
			RECT rc;
			OnDropMachine(machineName, instrumentName, pt.x, pt.y);
		}
		GlobalUnlock(medium.hGlobal);
	}
	return true;
}

CMachineDropTargetWindow::CMachineDropTargetWindow() {
	dropTarget = 0;
	hWndTarget = 0;
}

CMachineDropTargetWindow::~CMachineDropTargetWindow() {
	if (dropTarget) releaseDropTarget();
}

bool CMachineDropTargetWindow::createDropTarget(HWND hWnd) {
	if (dropTarget) releaseDropTarget();
	hWndTarget = hWnd;
	dropTarget = new CDropTarget(this);
	dropTarget->AddRef();

	HRESULT hr;
	if FAILED(hr = RegisterDragDrop(hWnd, dropTarget)) {
		dropTarget->Release();
		dropTarget = 0;
		hWndTarget = 0;
		return false;
	}
	dropTarget->Release();

	return true;
}

void CMachineDropTargetWindow::releaseDropTarget() {
	if (hWndTarget)
		RevokeDragDrop(hWndTarget);	// release dropTarget

	hWndTarget = 0;
	dropTarget = 0;
}

