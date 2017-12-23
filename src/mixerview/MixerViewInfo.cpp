#include "stdafx.h"
#include "resource.h"
#include "MixerView.h"

class CMixerViewLibrary : public CViewLibrary {
public:
	virtual void Initialize(CViewFrame* host) {
		_Module.m_hostModule = buze_application_get_host_module(buze_main_frame_get_application(host));
		buze_main_frame_register_window_factory(host, new CMixerViewInfo(host));
	}

	virtual void Destroy() {
		delete this;
	}

	virtual int GetVersion() {
		return CViewLibrary::version;
	}
};

extern "C" CViewLibrary* buze_create_viewlibrary() {
	return new CMixerViewLibrary();
}

//
// Factory
//

CMixerViewInfo::CMixerViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CMixerView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Mixer";
	tooltip = "Mixer";
	place = 1; //DockSplitTab::placeMAINPANE;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = true;
	allowfloat = true;
	defaultview = false;
}

CView* CMixerViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CMixerView* view = new CMixerView(mainframe, this);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CMixerViewInfo::Attach() {
	buze_document_add_view(document, this);

	show_eventcode = buze_main_frame_register_event(mainframe);
	// global accelerators - these generate global document events caught in OnUpdate
	WORD ID_SHOW_MIXER = buze_main_frame_register_accelerator_event(mainframe, "view_mixer", "f3", show_eventcode);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_MIXER, "Mixer");

}

void CMixerViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CMixerViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	zzub_event_data_t* zzubdata = (zzub_event_data_t*)pHint;
	CMixerView* view;

	if (lHint == show_eventcode) {
		view = (CMixerView*)buze_main_frame_get_view(mainframe, "MixerView", 0);
		if (view) {
			buze_main_frame_set_focus_to(mainframe, view);
		} else
			buze_main_frame_open_view(mainframe, "MixerView", "Mixer", 0, -1, -1);
		return ;
	} 

	switch (lHint) {
		case zzub_event_type_player_save:
			OnSaveState(zzubdata->player_save.userdata);
			break;
		case zzub_event_type_player_load:
			OnLoadState(zzubdata->player_save.userdata);
			break;
	}
}

void CMixerViewInfo::OnLoadState(zzub_archive_t* arc) {
	zzub_input_t* ins = zzub_archive_get_input(arc, "mixerview.dat");
	if (!ins) return ;
	unsigned int version = 0;
	zzub_input_read(ins, (char*)&version, sizeof(version));
	
	// version 1 contained a list of plugin ids, no longer supported
	// version 2 contains two strings, plugin and parameter filters
	if (version != 2) return ; 

	// parse pluginfilter
	unsigned int filterlength = 0;
	zzub_input_read(ins, (char*)&filterlength, sizeof(filterlength)); 

	if (filterlength >= 1024) return ;
	char filtertext[1024];
	memset(filtertext, 0, sizeof(filtertext));
	zzub_input_read(ins, filtertext, filterlength);

	plugin_filter = filtertext;

	// parse parameterfilter
	filterlength = 0;
	zzub_input_read(ins, (char*)&filterlength, sizeof(filterlength)); 

	if (filterlength >= 1024) return ;
	memset(filtertext, 0, sizeof(filtertext));
	zzub_input_read(ins, filtertext, filterlength);

	parameter_filter = filtertext;

	zzub_input_destroy(ins);
}

void CMixerViewInfo::OnSaveState(zzub_archive_t* arc) {
	zzub_output_t* outs = zzub_archive_get_output(arc, "mixerview.dat");
	unsigned int version = 2;
	zzub_output_write(outs, (const char*)&version, sizeof(version));

	// save plugin filter
	unsigned int filterlength = plugin_filter.size();
	zzub_output_write(outs, (const char*)&filterlength, sizeof(filterlength));
	zzub_output_write(outs, (const char*)plugin_filter.c_str(), filterlength);

	// save parameter filter
	filterlength = parameter_filter.size();
	zzub_output_write(outs, (const char*)&filterlength, sizeof(filterlength));
	zzub_output_write(outs, (const char*)parameter_filter.c_str(), filterlength);

	zzub_output_destroy(outs);
}
