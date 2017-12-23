#pragma once

#include "atlscintilla.h"
#include "WtlUtils.h"

static const int BUTTONX = 50;
static const int BUTTONY = 30;

class CPythonViewInfo : public CViewInfoImpl {
public:
	PyObject* glb;
	int show_eventcode;

	CPythonViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CPythonView :
	public CToolbarWindow<CPythonView>,
	public CViewImpl,
	public CMessageFilter
{
public:
	CPythonViewInfo* info;
	HWND hWndButtonToolBar;
	CButton compileButton;
	CButton executeButton;
	CComboListBand scriptCombo;
	CSplitterWindow splitter;
	CScintilla textEditor;
	CScintilla textOutput;
	std::string currentScriptFile;

	DECLARE_WND_CLASS("PythonView")

	BEGIN_MSG_MAP(CPythonView)
		CHAIN_MSG_MAP(CToolbarWindow<CPythonView>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CMD_HANDLER(ID_SCRIPTVIEW_EXECUTEBUTTON, BN_CLICKED, OnScriptExecute)
		CMD_HANDLER(ID_SCRIPTVIEW_COMPILEBUTTON, BN_CLICKED, OnScriptCompile)
		CMD_HANDLER(ID_SCRIPTVIEW_SCRIPTLIST, CBN_SELCHANGE, OnSelChangeScript)
		CMD_ID_HANDLER(ID_FILE_SAVE, OnScriptSave)
		CMD_ID_HANDLER(ID_FILE_SAVE_AS, OnScriptSaveAs)
		CMD_ID_HANDLER(ID_EDIT_UNDO, OnUndo)
		CMD_ID_HANDLER(ID_EDIT_REDO, OnRedo)
		CMD_ID_HANDLER(ID_SCRIPT_LOAD, OnScriptLoad)
		CMD_ID_HANDLER(ID_SCRIPT_EXECUTE, OnScriptExecute)
		CMD_ID_HANDLER(ID_SCRIPT_CLEAR, OnScriptClear)
		CMD_ID_HANDLER(ID_SCRIPT_RELOAD, OnScriptReload)
		CMD_ID_HANDLER(ID_SCRIPT_COMPILE, OnScriptCompile)
		CMD_ID_HANDLER(ID_SCRIPT_REFERENCE, OnScriptReference)
		CMD_ID_HANDLER(ID_SCRIPT_SCRIPTS, OnScripts)
	END_MSG_MAP()

	CPythonView(buze_main_frame_t* m, CPythonViewInfo* _info);
	~CPythonView();
	virtual void OnFinalMessage(HWND /*hWnd*/);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnSelChangeScript();

	// menu funcs
	void OnUndo();
	void OnRedo();
	void OnScriptSave();
	void OnScriptSaveAs();
	void OnScriptLoad();
	void OnScriptReload();
	void OnScriptExecute();
	void OnScriptCompile();
	void OnScriptReference();
	void OnScriptClear();
	void OnScripts();

	void ScriptSave( std::string file = "" );
	
	void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual HWND GetHwnd() {
		return m_hWnd;
	}
	int GetEditFlags();

	void BindScriptList();
	void LoadScript(const std::string& filename);
	void ExecuteScriptText(const char* scripttext);
	std::string GetScriptFileName();
};
