#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include "resource.h"
#include "ToolbarWindow.h"
#include "ToolbarBands.h"
#include "utils.h"
#include "PythonView.h"
#include "SciLexer.h"

using std::endl;
using std::cout;

/*

NOTE:

the PYTHONPATH env variable seems to override Py_SetPythonHome and PySys_SetPath
when the interpreter is initialized. however, it seems PythonXX.dll reads
the envvar very early - ie in its DllMain. 
thus, to work smoothly, this view assumes the following from the host: 
    - clears the PYTHONPATH: SetEnvironmentVariable(_T("PYTHONPATH"), NULL);
    - loads pythonview.dll w/LoadLibrary

TODO:
	- clear output
	- highlight lines with errors
	- focus script dropdown hotkey
	- run script in a different thread (?)
		- break script
		- show progress/cancel button
	- global hotkeys mapped to scripts
	- handle armstrong and buze events in script

*/

CHostDllModule _Module;
CPythonView* g_pythonView; // used by redirection

bool write_file(std::string const& path, const char* text, int len) {
	FILE* file = fopen(path.c_str(), "wb");

	if (!file)
		return false;

	fwrite(text, 1, len, file);

	fclose(file);
	return true;
}

//
// View
//

CPythonView::CPythonView(buze_main_frame_t* m, CPythonViewInfo* _info) : CViewImpl(m) {
	info = _info;
}

CPythonView::~CPythonView() {
}

void CPythonView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

BOOL CPythonView::PreTranslateMessage(MSG* pMsg) {
	if (GetFocus() == m_hWnd || IsChild(GetFocus())) {
		HACCEL hAccel = (HACCEL)buze_main_frame_get_accelerators(mainframe, "pythonview");
		if (::TranslateAccelerator(m_hWnd, hAccel, pMsg))
			return TRUE;
	}

	return FALSE;
}

const char py_keywords[] = "and as assert break class continue def del elif "
	"else except exec False finally for from global if import in is lambda None "
	"not or pass print raise return True try while with yield";

LRESULT CPythonView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lres=DefWindowProc();
	int bWidth  = 90;
	int bHeight = 40;

	splitter.Create(*this, rcDefault, NULL, 0, 0);

	compileButton.Create(*this, rcDefault, "Test", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_NOTIFY, 0, ID_SCRIPTVIEW_COMPILEBUTTON);
	executeButton.Create(*this, rcDefault, "Execute", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_NOTIFY, 0, ID_SCRIPTVIEW_EXECUTEBUTTON);
	compileButton.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	executeButton.SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	
	scriptCombo.Create(*this, rcDefault, "", 0, 0, ID_SCRIPTVIEW_SCRIPTLIST);
	BindScriptList();

	textEditor.Create(splitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
	textEditor.SetCodePage(SC_CP_UTF8);
	textEditor.StyleSetFont(STYLE_DEFAULT, "Courier");
	textEditor.SetLexer(SCLEX_PYTHON);
	textEditor.SetKeyWords(0, py_keywords);
	textEditor.StyleSetFore(SCE_P_DEFAULT, RGB(0, 0, 0));
	textEditor.StyleSetFore(SCE_P_COMMENTLINE, RGB(0, 128, 0));
	textEditor.StyleSetFore(SCE_P_NUMBER, RGB(0, 0, 128));
	textEditor.StyleSetFore(SCE_P_STRING, RGB(0, 0, 128));
	textEditor.StyleSetFore(SCE_P_CHARACTER, RGB(0, 0, 128));
	textEditor.StyleSetFore(SCE_P_WORD, RGB(0, 0, 255));
	textEditor.StyleSetFore(SCE_P_WORD2, RGB(0, 128, 255));
	textEditor.StyleSetFore(SCE_P_CLASSNAME, RGB(128, 0, 255));
	textEditor.StyleSetFore(SCE_P_DEFNAME, RGB(128, 0, 255));
	textEditor.StyleSetFore(SCE_P_IDENTIFIER, RGB(0, 0, 0));
	textEditor.StyleSetFore(SCE_P_COMMENTBLOCK, RGB(0, 128, 0));

	textOutput.Create(splitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
	textOutput.SetCodePage(SC_CP_UTF8);
	textOutput.StyleSetFont(STYLE_DEFAULT, "Courier");
	textOutput.SetReadOnly(true);

	hWndButtonToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_SCRIPT, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	SIZE btbSize; SendMessage(hWndButtonToolBar, TB_GETMAXSIZE, 0, (LPARAM)&btbSize);
	insertToolbarBand(hWndButtonToolBar, "", btbSize.cx,-1, true, true);
	//insertToolbarBand(compileButton, "", BUTTONX,-1, true,true);
	insertToolbarBand(executeButton, "", BUTTONX,-1, true,true);
	insertToolbarBand(scriptCombo, "Script", 150, -1, true, true);

	buze_document_add_view(document, this);

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);

	buze_main_frame_viewstack_insert(mainframe, this); // true
	g_pythonView = this;

	return 0;
}

LRESULT CPythonView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	int width = LOWORD(lParam);
	int height = HIWORD(lParam);

	if (width == 0 || height == 0) return 0;
	
	int ss = splitter.GetSplitterPos();

	int toolbarHeight = getToolbarHeight();
	int textMargin = 5;
	splitter.MoveWindow(textMargin, toolbarHeight, width-textMargin, height-toolbarHeight);

	if (ss == -1) {
		splitter.SetSplitterPanes(textEditor, textOutput);
		splitter.SetSplitterExtendedStyle(SPLIT_PROPORTIONAL);
		splitter.SetSplitterPosPct(80);
	}
	
	return 0;
}

LRESULT CPythonView::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	g_pythonView = 0;

	CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);

	buze_document_remove_view(document, this);
	return 0;
}

void CPythonView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	switch (lHint) {
		case buze_event_type_update_post_open_document:
		case buze_event_type_update_new_document:
		case zzub_event_type_update_song:
			//edit.SetWindowText(zzub_player_get_infotext(player));
			break;
	}
}

void CPythonView::OnUndo() {
	textEditor.Undo();
}

void CPythonView::OnRedo() {
	textEditor.Redo();
}

void CPythonView::OnScriptExecute() {
	int len = textEditor.GetTextLength();
	char* buffer = new char[len + 1];
	textEditor.GetText(len + 1, buffer);

	ExecuteScriptText(buffer);

	delete[] buffer;
}

void CPythonView::OnScriptCompile() {
	MessageBox("TODO");
}

void CPythonView::OnScriptLoad(){
	printf("load script\n");
	std::string currentDir = buze_document_get_current_path(document);
	currentDir.append("\\Gear\\Scripts");

	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	char szCurDir[260];
	strcpy(szCurDir, currentDir.c_str());

    // open a file name
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = "All files\0*.*\0\0";
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	if (strlen(szCurDir) > 0)
    	ofn.lpstrInitialDir = szCurDir;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
	
	// Display the Open dialog box. 
	if (GetOpenFileName(&ofn)!=TRUE) return ;

	// load file if succesfull now lets read it
	LoadScript(ofn.lpstrFile);	
}

void CPythonView::OnScriptReload(){
	if (!currentScriptFile.empty())
		LoadScript(currentScriptFile);
}

void CPythonView::OnScriptSave() {
	ScriptSave(currentScriptFile);
}

void CPythonView::OnScriptSaveAs() {
	ScriptSave();
}

void CPythonView::OnScriptClear(){
	int confirm = MessageBox("Are you sure?", "Clear Text", MB_YESNOCANCEL|MB_ICONSTOP);
	if( confirm ) {
		textEditor.SetText("");
		currentScriptFile = "";
	}
}

void CPythonView::OnScriptReference() {
	MessageBox("TODO\n\nPlease refer to armstrong.py or buze.py for now");
}

void CPythonView::OnScripts() {
	//MessageBox("Please rightclick your mouse to access the scripts library");
	scriptCombo.SetFocus();
}

void CPythonView::ExecuteScriptText(const char* scripttextstr) {

	std::string scripttext = scripttextstr;
	scripttext.erase(std::remove(scripttext.begin(), scripttext.end(), '\r'), scripttext.end());

	g_pythonView = this;

	PyObject* pyc = Py_CompileString(scripttext.c_str(), "ScriptView.py", Py_file_input);

	if (pyc) {
		PyEval_EvalCode((PyCodeObject*)pyc, info->glb, info->glb);
		Py_DECREF(pyc);
	}

	if (PyErr_Occurred()) {
		PyErr_Print();
		PyErr_Clear();
	}
}

std::string CPythonView::GetScriptFileName() {
	int sel = scriptCombo.ctrl().GetCurSel();
	if (sel == -1) return "";
	TCHAR text[1024];
	scriptCombo.ctrl().GetLBText(sel, text);
	
	buze_application_t* application = buze_main_frame_get_application(mainframe);
	std::string scriptpath = buze_application_map_path(application, "Gear\\Scripts\\Python\\", buze_path_type_app_path);

	return scriptpath + text;
}

void CPythonView::OnSelChangeScript() {
	std::string scriptfile = GetScriptFileName();
	if (scriptfile.empty()) return ;

	LoadScript(scriptfile);
}

void CPythonView::BindScriptList() {
	// enum Gear/Scripts/Python/*.py
	buze_application_t* application = buze_main_frame_get_application(mainframe);
	std::string scriptpath = buze_application_map_path(application, "Gear\\Scripts\\Python", buze_path_type_app_path);

	std::string searchpath = scriptpath + "\\*.*";

	WIN32_FIND_DATA fd;
	HANDLE hFind=::FindFirstFile(searchpath.c_str(), &fd);
	while(hFind != INVALID_HANDLE_VALUE) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
				std::string relpath(searchpath + '\\' + fd.cFileName);
				// TODO: recursive scanning
				//scan_plugins(mainframe, rootpath, relpath + "\\");
			}
		} else {
			char* ext = strrchr(fd.cFileName, '.');
			if (ext != 0 && stricmp(ext, ".py") == 0) {
				scriptCombo.ctrl().AddString(fd.cFileName);
			}
		}
		if (!::FindNextFile(hFind, &fd)) break;
	}
	::FindClose(hFind);
}

void CPythonView::LoadScript(const std::string& filename) {

	std::string scriptText = read_file(filename);
	if (scriptText.empty()) return ;

	currentScriptFile = filename;
	textEditor.SetText(scriptText.c_str());
}

void CPythonView::ScriptSave( std::string scriptfile ){
	if (scriptfile.empty()) {
		OPENFILENAME ofn;       // common dialog box structure
		char szFile[260];       // buffer for file name
		strcpy(szFile, currentScriptFile.c_str());
		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All files\0*.*\0\0";
		ofn.lpstrDefExt = "py";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.lpstrInitialDir = "Gear\\Scripts\\";

		ofn.Flags = OFN_NOCHANGEDIR |OFN_OVERWRITEPROMPT|OFN_EXTENSIONDIFFERENT|OFN_NOREADONLYRETURN;
		// Display the Open dialog box. 
		if (GetSaveFileName(&ofn)!=TRUE) return;
		currentScriptFile = ofn.lpstrFile;
		scriptfile = currentScriptFile;
	}

	if (scriptfile.empty()) return;

	int len = textEditor.GetTextLength();
	char* buffer = new char[len + 1];
	textEditor.GetText(len + 1, buffer);
	write_file(scriptfile, buffer, len);
	delete[] buffer;

/*
	// save the file!
	std::ofstream myfile (currentScriptFile.c_str());
	if (myfile.is_open()){

		char content[ 200000 ] = {0}; // *FIXME* can the 20000 characters limit be prevented?
		textEditor.GetText( textEditor.GetTextLength()+1, content);
		myfile << content;
		myfile.close();
	}else MessageBox("could not save file");*/
}
