#include "stdafx.h"
#include <iostream>
#include "resource.h"
#include "ToolbarWindow.h"
#include "ToolbarBands.h"
#include "PythonView.h"

using std::endl;
using std::cout;

extern CPythonView* g_pythonView; // used by redirection

static PyObject* redirection_stdoutredirect(PyObject *self, PyObject *args)
{
    const char *string;
    if (!PyArg_ParseTuple(args, "s", &string))
        return NULL;

	if (g_pythonView) {
		g_pythonView->textOutput.SetReadOnly(false);
		g_pythonView->textOutput.AppendText(strlen(string), string); 
		g_pythonView->textOutput.SetReadOnly(true);
	}
	cout << string;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef redirection_methods[] = {
    {"stdoutredirect", redirection_stdoutredirect, METH_VARARGS,
        "stdout redirection helper"},
    {NULL, NULL, 0, NULL}
};

class CPythonViewLibrary : public CViewLibrary {
public:
	virtual void Initialize(CViewFrame* host) {
		_Module.m_hostModule = buze_application_get_host_module(buze_main_frame_get_application(host));
		buze_main_frame_register_window_factory(host, new CPythonViewInfo(host));
	}

	virtual void Destroy() {
		delete this;
	}

	virtual int GetVersion() {
		return CViewLibrary::version;
	}
};

extern "C" CViewLibrary* buze_create_viewlibrary() {
	return new CPythonViewLibrary();
}

//
// Factory
//

CPythonViewInfo::CPythonViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CPythonView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Python";
	tooltip = "Python";
	place = 1; //DockSplitTab::placeMAINPANE;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = true;
	allowfloat = false;
	defaultview = true;
	glb = 0;
}

CView* CPythonViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CPythonView* view = new CPythonView(mainframe, this);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

static bool showExitHandlerError = true;

extern "C" void Py_Initialize_ExitHandler() {
	if (showExitHandlerError)
		MessageBox(GetForegroundWindow(), 
			"There was a seriously error initializing the Python environment. "
			"Please make sure the Python directory is set up correctly or delete Gear/Views/pythonview.dll.\r\n\r\n"
			"Python will now terminate the program. Sorry!"
			, "Python Error", MB_ICONERROR|MB_OK);
}

void CPythonViewInfo::Attach() {
	buze_document_add_view(document, this);

	show_eventcode = buze_main_frame_register_event(mainframe);

	// global accelerators - these generate global document events caught in OnUpdate
	WORD ID_SHOW_SCRIPT = buze_main_frame_register_accelerator_event(mainframe, "view_python", "", show_eventcode);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//buze_main_frame_register_accelerator(mainframe, "pythonview", "help", ID_HELP);
	buze_main_frame_register_accelerator(mainframe, "pythonview", "save", "s ctrl", ID_FILE_SAVE);
	buze_main_frame_register_accelerator(mainframe, "pythonview", "execute", "f5", ID_SCRIPT_EXECUTE);
	buze_main_frame_register_accelerator(mainframe, "pythonview", "edit_undo", "z ctrl", ID_EDIT_UNDO);
	buze_main_frame_register_accelerator(mainframe, "pythonview", "edit_redo", "y ctrl", ID_EDIT_REDO);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_SCRIPT, "Python");

	// initialize python engine
	g_pythonView = 0;
	char* path = "./Python;./Python/DLLs;./Python/lib;./Python/lib/site-packages";

	const char* redirectscript =
		"import redirection\n"
		"import sys\n"
		"class StdoutCatcher:\n"
		"    def write(self, stuff):\n"
		"        redirection.stdoutredirect(stuff)\n"
		"sys.stdout = StdoutCatcher()\n"
		"sys.stderr = StdoutCatcher()";

	Py_SetPythonHome("./Python");
	
	// set up atexit-handler to tell the user what happened in case Py_Initialize fails and exit()s
	atexit(Py_Initialize_ExitHandler);
	
	Py_InitializeEx(0);

	showExitHandlerError = false; // tell atexit-handler it dont needs to show error anymore

	PySys_SetPath(path);
	Py_InitModule("redirection", redirection_methods);

	glb = PyDict_New();
	PyDict_SetItemString(glb, "__builtins__", PyEval_GetBuiltins ());

	PyRun_SimpleString(redirectscript);

	PyObject* ctypes_module = PyImport_ImportModule("ctypes");
	PyObject* armstrong_module = PyImport_ImportModule("armstrong");
	PyObject* buze_module = PyImport_ImportModule("buze");
	if (ctypes_module != 0 && armstrong_module != 0 && buze_module != 0) {
		PyObject* ctypes_main_frame_t = PyObject_GetAttr(buze_module, PyString_FromString("buze_main_frame_t"));
		PyObject* ctypes_POINTER = PyObject_GetAttr(ctypes_module, PyString_FromString("POINTER"));
		PyObject* pointerargs = Py_BuildValue("(O)", ctypes_main_frame_t);
		PyObject* ctypes_main_frame_t_ptr = PyObject_CallObject(ctypes_POINTER, pointerargs);
		PyObject* ctypes_main_frame_from_address = PyObject_GetAttr(ctypes_main_frame_t_ptr, PyString_FromString("from_address"));
		PyObject* ctypes_main_frame_from_address_arg = Py_BuildValue("(l)", (long int)&mainframe);
		PyObject* instance_mainframe = PyObject_CallObject(ctypes_main_frame_from_address, ctypes_main_frame_from_address_arg);
		PyObject* classes_mainframe = PyObject_GetAttr(buze_module, PyString_FromString("MainFrame"));
		PyObject* classes_mainframe_args = Py_BuildValue("(O)", instance_mainframe);
		PyObject* maininst = PyObject_CallObject(classes_mainframe, classes_mainframe_args);

		assert(maininst);

		PyDict_SetItemString (glb, "mainframe", maininst);
	}
}

void CPythonViewInfo::Detach() {

	Py_DECREF(glb);
	Py_Finalize();

	buze_document_remove_view(document, this);
}

void CPythonViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CView* view;
	if (lHint == show_eventcode) {
		view = buze_main_frame_get_view(mainframe, "PythonView", 0);
		if (view) {
			buze_main_frame_set_focus_to(mainframe, view);
		} else
			buze_main_frame_open_view(mainframe, "PythonView", "Python", 0, -1, -1);
	}
}
