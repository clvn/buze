#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "CpuMeterView.h"

// 
// Factory
//

CCpuMeterViewInfo::CCpuMeterViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CCpuMeterView::GetWndClassInfo().m_wc.lpszClassName;
	label = "CPU Meter";
	tooltip = "Machine CPU usage";
	place = 2; //DockSplitTab::placeDOCKPANE;
	side = 2; //DockSplitTab::dockLEFT;
	serializable = true;
	allowfloat = true;
	defaultview = false;
}

CView* CCpuMeterViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CCpuMeterView* view = new CCpuMeterView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CCpuMeterViewInfo::Attach() {
	buze_document_add_view(document, this);

	// global accelerators - these generate global document events caught in OnUpdate
	WORD ID_SHOW_CPUMETER = buze_main_frame_register_accelerator_event(mainframe, "view_cpumeter", "c ctrl shift", buze_event_type_show_cpu_view);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_CPUMETER, "CPU Meter");
}

void CCpuMeterViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CCpuMeterViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CView* view;
	switch (lHint) {
		case buze_event_type_show_cpu_view:
			view = buze_main_frame_get_view(mainframe, "CpuMeterView", 0);
			if (view) {
				buze_main_frame_set_focus_to(mainframe, view);
			} else
				buze_main_frame_open_view(mainframe, "CpuMeterView", "CPU Meter", 0, -1, -1);
			break;
	}
}

//
// View
//

CCpuMeterView::CCpuMeterView(buze_main_frame_t* mainFrm)
	: CViewImpl(mainFrm) 
{
}

CCpuMeterView::~CCpuMeterView(void) {
}

void CCpuMeterView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CCpuMeterView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	DefWindowProc();

 	pluginList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | LVS_REPORT, WS_EX_CLIENTEDGE, IDC_CPUMETER_PLUGINLIST);
	pluginList.AddColumn("Plugin", 0);
	pluginList.AddColumn("Load", 1);
	pluginList.SetColumnWidth(0, 150);
	pluginList.SetColumnWidth(1, 80);

	int index = 0;
	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		if (zzub_plugin_get_flags(plugin) != zzub_plugin_flag_is_connection) {
			const char* name = zzub_plugin_get_name(plugin);
			pluginList.InsertItem(index, name);
			index++;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	details.Create(m_hWnd, rcDefault, "", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS);
	details.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	buze_main_frame_add_timer_handler(mainframe, this);
	buze_document_add_view(document, this);

	return 0;
}

LRESULT CCpuMeterView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	buze_document_remove_view(document, this);
	buze_main_frame_remove_timer_handler(mainframe, this);
	return 0;
}

LRESULT CCpuMeterView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD(lParam);
	WORD cy = HIWORD(lParam);

	pluginList.MoveWindow(0, 0, cx, cy-20);
	details.MoveWindow(0, cy-20, cx, cy);
	return 0;
}

void CCpuMeterView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;
	std::vector<char> bytes(1024);
	switch (lHint) {
		case zzub_event_type_update_plugin:
			// TODO: could have renamed a plugin here
			break;
		case zzub_event_type_insert_plugin:
		{
			if (zzub_plugin_get_flags(zzubData->insert_plugin.plugin) != zzub_plugin_flag_is_connection) {
				const char* name = zzub_plugin_get_name(zzubData->insert_plugin.plugin);
				pluginList.InsertItem(pluginList.GetItemCount(), name);
			}
			return ;
		}
		case zzub_event_type_delete_plugin:
		{
			zzub_plugin_t* plugin = zzubData->delete_plugin.plugin;
			if (zzub_plugin_get_flags(plugin) != zzub_plugin_flag_is_connection) {
				std::string name = zzub_plugin_get_name(plugin);
				for (size_t i = 0; i<pluginList.GetItemCount(); i++) {
					pluginList.GetItemText(i, 0, &bytes.front(), bytes.size());
					if (name == &bytes.front()) {
						pluginList.DeleteItem(i);
						break;
					}
				}
			}
			return ;
		}
	}
}

void CCpuMeterView::UpdateTimer(int count) {
	if ((count % 50) != 0) return; /// 500ms

	buze_application_t* app = buze_main_frame_get_application(mainframe);
	zzub_audiodriver_t* driver = (zzub_audiodriver_t*)buze_application_get_audio_driver(app);

	float samples_per_second = zzub_audiodriver_get_samplerate(driver);
	float audioload = zzub_audiodriver_get_cpu_load(driver) * 100;

	double sum = 0;

	pluginList.SetRedraw(FALSE);

	char bytes[256];
	int plugin_count = 0;
	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);

		if (zzub_plugin_get_flags(plugin) != zzub_plugin_flag_is_connection) {
			const char* name = zzub_plugin_get_name(plugin);
			pluginList.SetItem(plugin_count, 0, LVIF_TEXT, name, -1, 0, 0, 0);

			double cpu_load = zzub_plugin_get_last_cpu_load(plugin) * 100;
			sprintf(bytes, "%.2f%%", cpu_load);
			pluginList.SetItem(plugin_count, 1, LVIF_TEXT, bytes, -1, 0, 0, 0);
			sum += cpu_load;
			plugin_count++;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	pluginList.SetRedraw(TRUE);

	sprintf(bytes, "Total load: %.1f%% %i plugins: %.1f%%", audioload, plugin_count, sum);
	details.SetWindowText(bytes);
}
