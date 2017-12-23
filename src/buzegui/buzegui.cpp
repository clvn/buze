#include "stdafx.h"
#include "resource.h"
#include "ToolbarWindow.h"
#include "ToolbarBands.h"
#include "Configuration.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "BuzeConfiguration.h"

#include "AnalyzerView.h"
#include "CpuMeterView.h"
#include "CommentView.h"
#include "FileBrowserView.h"
#include "HelpView.h"
#include "HistoryView.h"
#include "PatternFormatView.h"
#include "PreferencesView.h"
#include "PropertyListView.h"
#include "MachineFolderView.h"

CHostDllModule _Module;

class CBuzeGuiLibrary : public CViewLibrary {
public:
	virtual void Initialize(CViewFrame* host) {

		_Module.m_hostModule = buze_application_get_host_module(buze_main_frame_get_application(host));

		buze_main_frame_register_window_factory(host, new CAnalyzerViewInfo(host));
		buze_main_frame_register_window_factory(host, new CCommentViewInfo(host));
		buze_main_frame_register_window_factory(host, new CCpuMeterViewInfo(host));
		buze_main_frame_register_window_factory(host, new CFileBrowserViewInfo(host));
		buze_main_frame_register_window_factory(host, new CHelpViewInfo(host));
		buze_main_frame_register_window_factory(host, new CHistoryViewInfo(host));
		buze_main_frame_register_window_factory(host, new CPatternFormatViewInfo(host));
		buze_main_frame_register_window_factory(host, new CPreferencesViewInfo(host));
		buze_main_frame_register_window_factory(host, new CPropertyListViewInfo(host));
		buze_main_frame_register_window_factory(host, new CMachineFolderViewInfo(host));
	}

	void Destroy() {
		delete this;
	}

	int GetVersion() {
		return CViewLibrary::version;
	}

};

extern "C" CViewLibrary* buze_create_viewlibrary() {
	return new CBuzeGuiLibrary();
}
