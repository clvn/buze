#pragma once

class CViewFrame {
public:

	CDocument* document;
	zzub_player_t* player;
	CApplication* application;

	virtual ~CViewFrame() {}
	virtual bool CreateFrame(HWND hParentWnd) = 0;
	virtual HWND GetHwnd() = 0;
	virtual void RegisterViewInfo(CViewInfo* info) = 0;
	virtual void AddTimerHandler(CView* handler) = 0;
	virtual void RemoveTimerHandler(CView* handler) = 0;
	virtual void ViewStackInsert(HWND hViewWnd, bool keyboard_tabable) = 0;
	virtual HACCEL GetAccelerators(const char* viewname) = 0;
	virtual HWND GetFocusedClientView() = 0;
	virtual CView* GetFocusedView() = 0;
	virtual bool IsFloatView(HWND hViewWnd) = 0;
	virtual void SetFocusTo(HWND hWnd) = 0;
	virtual void CloseView(HWND clientViewWnd) = 0;
	virtual CView* GetView(const char* viewname, int view_id) = 0;
	virtual CView* OpenView(const char* viewname, const char* label, int view_id, int x = -1, int y = -1) = 0;
	virtual const char* GetOpenFileName() = 0;
	virtual const char* GetSaveFileName() = 0;
	virtual HMENU GetMachineMenuCreate() = 0;
	virtual HMENU GetMachineMenuInsertAfter() = 0;
	virtual HMENU GetMachineMenuInsertBefore() = 0;
	virtual HMENU GetMachineMenuReplace() = 0;
	virtual void AddMenuKeys(const char* viewname, HMENU hMenu) = 0;
	virtual int RegisterEvent() = 0;
	virtual WORD RegisterAcceleratorEvent(const char* name, const char* hotkey, int event) = 0;
	virtual void RegisterAccelerator(const char* viewname, const char* name, const char* hotkey, WORD id) = 0;
	virtual HMENU GetMainMenu() = 0;
	virtual void ShowMachineParameters(zzub_plugin_t* m, MachineParameterViewMode modehint, int x = -1, int y = -1) = 0;
	virtual void* GetKeyjazzMap() = 0;
	virtual const char* GetProgramName() = 0;
	virtual CView* GetViewByHwnd(HWND hWnd) = 0;
};