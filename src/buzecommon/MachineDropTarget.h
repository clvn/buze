#pragma once

class CDropTargetWindow {
public:
	HWND hWndTarget;

	virtual ~CDropTargetWindow() { }
	virtual bool OnDrop(const POINTL& pt, FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect) = 0;
	virtual bool OnDragOver(const POINTL& pt, FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect) = 0;
	virtual void OnDragLeave() = 0;
};

class CDropTarget : public CIDropTarget {
	CDropTargetWindow* dropTargetWindow;
public:
	CDropTarget(CDropTargetWindow* dtw);
	virtual bool OnDrop(const POINTL& pt, FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect);
	virtual bool OnDragOver(const POINTL& pt, FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect);
	virtual void OnDragLeave();
};

class CMachineDropTargetWindow : public CDropTargetWindow {
public:
	CDropTarget* dropTarget;

	CMachineDropTargetWindow();
	~CMachineDropTargetWindow();
	bool createDropTarget(HWND hWndTarget);
	void releaseDropTarget();
	virtual bool OnDrop(const POINTL& pt, FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect);
	virtual bool OnDropMachine(std::string machineName, std::string instrumentName, int x, int y) = 0;
};
