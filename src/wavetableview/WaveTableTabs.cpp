#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "utils.h"
#include "Keymaps.h"
#include "Configuration.h"
#include "WaveTableView.h"
#include "WaveTableTabs.h"


/***

	CWaveTabEdit

***/

CWaveTabEdit::CWaveTabEdit(CWaveTableView* view) {
	this->view = view;
}

LRESULT CWaveTabEdit::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DefWindowProc();

	volumeSliderLabel.Create(m_hWnd, rcDefault, "Volume", WS_VISIBLE|WS_CHILD);
	volumeSliderLabel.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	DWORD trackBarStyle = WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_FIXEDLENGTH|TBS_BOTH;

	volumeSlider.Create(m_hWnd, rcDefault, 0, trackBarStyle, 0, IDC_WAVEVOLUMESLIDER);
	volumeSlider.SetPageSize(16);
	volumeSlider.SetTicFreq(1024);
	volumeSlider.SetRange(0, 0x7fff);	// converted to a float between from 0-2 (0-200%)
	volumeSlider.SetThumbLength(12);

	UpdateFromWavetable();

	return 0;
}

LRESULT CWaveTabEdit::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD( lParam);
	WORD cy = HIWORD( lParam);

	volumeSliderLabel.MoveWindow(0, 0, cx, 22);
	
	volumeSlider.MoveWindow(0, 22, cx, 22);

	return 0;
}

LRESULT CWaveTabEdit::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CWaveTabEdit::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CWaveTabEdit::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if (LOWORD(wParam) == TB_ENDTRACK) {
		zzub_wave_set_volume(view->currentWave, (float)volumeSlider.GetPos() / 0x7fff * 2);
		zzub_player_history_commit(view->player, 0, 0, "Set Wave Volume");
	} else if (LOWORD(wParam) == TB_THUMBTRACK) {
		// TODO: disable undo buffer, set volume, enable undo buffer
		// OR: wave_set_volume_direct() to bypass the storage and set the volume in the mixer only
		SetSliderValue((float)volumeSlider.GetPos() / 0x7fff * 2);
	}
	return 0;
}

LRESULT CWaveTabEdit::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle bgDC((HDC)wParam);
	RECT rcClient;
	GetClientRect(&rcClient);
	bgDC.FillRect(&rcClient, COLOR_3DFACE);
	return 1;
}

void CWaveTabEdit::SetSliderValue(float volume) { // 0..2
	volumeSlider.SetPos(volume / 2 * 0x7fff);

	std::stringstream strm;
	strm << "Volume (" << int(volume * 100) << "%)";
	volumeSliderLabel.SetWindowText(strm.str().c_str());
}

void CWaveTabEdit::UpdateFromWavetable() {
	if (view->currentWave == 0) return ;
	
	float volume = zzub_wave_get_volume(view->currentWave);
	SetSliderValue(volume);
}


/***
	CWaveTabEffects
***/

CWaveTabEffects::CWaveTabEffects(CWaveTableView* _view, zzub_player_t* _player)
	:effectParameters(_player)
{
	view = _view;
	player = _player;
	effectplugin = 0;
}

LRESULT CWaveTabEffects::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	LRESULT lRes = DefWindowProc();

	SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	effectsList.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | 
					WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | 
					LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 0, IDC_WAVETABLE_EFFECTSLIST);
	effectsList.AddColumn("Effect", 0);
	effectsList.SetColumnWidth(0, 150);

	effectParameters.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL);
	effectParameters.SetUndo(false);

	BindEffectsList();

	applyButton.Create(m_hWnd, rcDefault, "Apply", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, IDC_WAVETABLE_EFFECTBUTTON);
	applyButton.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	return lRes;
}

LRESULT CWaveTabEffects::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD( lParam);
	WORD cy = HIWORD( lParam);

	effectsList.MoveWindow(0, 0, 150, cy - 20);
	effectParameters.MoveWindow(150, 0, cx - 150, cy);
	applyButton.MoveWindow(0, cy - 20, 150, 20);
	return 0;
}

LRESULT CWaveTabEffects::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CWaveTabEffects::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CWaveTabEffects::OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CWaveTabEffects::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle bgDC((HDC)wParam);
	RECT rcClient;
	GetClientRect(&rcClient);
	rcClient.top = rcClient.bottom - 20;
	bgDC.FillRect(&rcClient, COLOR_3DFACE);
	return 1;
}

LRESULT CWaveTabEffects::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	// handle and ignore effect parameter slider context menu
	return 0;
}

LRESULT CWaveTabEffects::OnChangeEffect(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pnmh;
	if (pnmv->iItem == -1) return 0;	// applies to all items, we arent interested

	if (pnmv->uNewState & LVIS_SELECTED)
		BindParameters();
	return 0;
}

LRESULT CWaveTabEffects::OnApplyEffect(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	if (view->currentWavelevel == 0 || effectplugin == 0) return 0;

	long beginSelect, endSelect;
	
	if (view->waveEditorCtrl.beginSelectSample != -1) {
		beginSelect = min(view->waveEditorCtrl.beginSelectSample, view->waveEditorCtrl.endSelectSample);
		endSelect = max(view->waveEditorCtrl.beginSelectSample, view->waveEditorCtrl.endSelectSample);
	} else {
		beginSelect = 0;
		endSelect = view->waveEditorCtrl.waveInfo.samples - 1;
	}
	buze_application_t *a = buze_main_frame_get_application(view->mainframe);
	buze_application_show_wait_window(a);
	buze_application_set_wait_text(a, "processing sample..");

	int numSamples = (endSelect - beginSelect) + 1;
	zzub_wavelevel_process_sample_range_offline(view->currentWavelevel, beginSelect, numSamples, effectplugin);

	buze_application_hide_wait_window(a, this->view);

	zzub_player_history_commit(player, 0, 0, "Process Offline Wavelevel");

	GetParent().SetFocus(); // assume the "apply"-button is focused, change focus to prevent eating preview/accelerator events
	return 0;
}

void CWaveTabEffects::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;
	switch (lHint) {
		case zzub_event_type_delete_plugin:
			if (zzubData->delete_plugin.plugin == effectplugin) {
				effectplugin = 0;
			}
			break;
		case zzub_event_type_update_pluginparameter:
			if (zzubData->update_pluginparameter.plugin == effectplugin)
				effectParameters.SetParameter(zzubData->update_pluginparameter.plugin, zzubData->update_pluginparameter.group, zzubData->update_pluginparameter.track, zzubData->update_pluginparameter.param, zzubData->update_pluginparameter.value);
			break;
		case buze_event_type_update_pre_save_document:
		case buze_event_type_update_pre_clear_document:
		case buze_event_type_update_pre_open_document:
			if (effectplugin) 
				DestroyOfflinePlugin();
			break;
		case buze_event_type_update_post_save_document:
		case buze_event_type_update_post_clear_document:
		case buze_event_type_update_post_open_document:
			CreateOfflinePlugin();
			break;
	}
}

void CWaveTabEffects::BindEffectsList() {
	
	zzub_plugin_iterator_t* plugins = zzub_player_get_plugin_iterator(player);

	int index = 0;
	for (int i = 0; i < zzub_player_get_pluginloader_count(player); i++) {
		zzub_pluginloader_t* info = zzub_player_get_pluginloader(player, i);
		int flags = zzub_pluginloader_get_flags(info);
		if ((flags & zzub_plugin_flag_is_offline) == 0) continue;

		effectsList.InsertItem(index, zzub_pluginloader_get_name(info));
		effectsList.SetItemData(index, (DWORD_PTR)info);

		zzub_plugin_iterator_next(plugins);
	}

	zzub_plugin_iterator_destroy(plugins);
}

void CWaveTabEffects::BindParameters() {

	if (effectplugin) DestroyOfflinePlugin();
	CreateOfflinePlugin();

	std::vector<paramid> parameters;

	if (effectplugin) {
		zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(effectplugin);

		for (int k = 1; k < 3; ++k) {
			for (int j = 0; j < zzub_plugin_get_track_count(effectplugin, k); ++j) {
				for (int i = 0; i < zzub_pluginloader_get_parameter_count(info, k); ++i) {
					zzub_parameter_t* param = zzub_plugin_get_parameter(effectplugin, k, j, i);
					if ((zzub_parameter_get_flags(param) & zzub_parameter_flag_state) == 0) continue;

					paramid pp = { effectplugin, k, j, i };
					parameters.push_back(pp);
				}
			}
		}
	}
	effectParameters.SetParameters(parameters);
}


void CWaveTabEffects::CreateOfflinePlugin() {
	int index = effectsList.GetSelectedIndex();
	if (index < 0) {
		return ;
	}
	zzub_pluginloader_t* effectinfo = (zzub_pluginloader_t*)effectsList.GetItemData(index);

	assert(effectinfo);
	assert(effectplugin == 0);
	int historystate = zzub_player_history_enable(player, 0);
	effectplugin = zzub_player_create_plugin(player, 0, 0, "OfflineFx", effectinfo, 0);
	assert(effectplugin != 0);

	// hide from all views
	buze_document_set_plugin_non_song(view->document, effectplugin, TRUE);

	zzub_player_history_commit(player, 0, 0, "Create Offline Plugin");
	zzub_player_history_enable(player, historystate);

}

void CWaveTabEffects::DestroyOfflinePlugin() {
	std::vector<paramid> parameters;
	effectParameters.SetParameters(parameters);

	assert(effectplugin != 0);
	int historystate = zzub_player_history_enable(player, 0);
	zzub_plugin_destroy(effectplugin);
	zzub_player_history_commit(player, 0, 0, "Delete Offline Plugin");
	zzub_player_history_enable(player, historystate);
	effectplugin = 0;
}


/***
	CWaveTabSlice
***/

CWaveTabSlice::CWaveTabSlice(CWaveTableView* view) {
	this->view = view;
	noselect = false;
}

LRESULT CWaveTabSlice::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DefWindowProc();

	sliceSliderLabel.Create(m_hWnd, rcDefault, "To create slices, either use a loop recorder or an onset detector in the Effects tab.", WS_VISIBLE|WS_CHILD);
	sliceSliderLabel.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

/*	DWORD trackBarStyle = WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_FIXEDLENGTH|TBS_BOTH;

	sliceSlider.Create(m_hWnd, rcDefault, 0, trackBarStyle, 0, IDC_WAVETABLE_SLICESLIDER);
	sliceSlider.SetPageSize(16);
	sliceSlider.SetTicFreq(1024);
	sliceSlider.SetRange(0, 0x7fff);	// converted to a float between from 0-2 (0-200%)
	sliceSlider.SetThumbLength(12);
*/
	sliceList.Create(m_hWnd, rcDefault, "", WS_CHILD|WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 0, IDC_WAVETABLE_SLICELIST);
	sliceList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT,LVS_EX_FULLROWSELECT);
	sliceList.AddColumn("#", 0);
	sliceList.AddColumn("From", 1, -1, LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_RIGHT);
	sliceList.AddColumn("To", 2, -1, LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM, LVCFMT_RIGHT);
	sliceList.SetColumnWidth(0, 50);
	sliceList.SetColumnWidth(1, 100);
	sliceList.SetColumnWidth(2, 100);

	return 0;
}

LRESULT CWaveTabSlice::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD( lParam);
	WORD cy = HIWORD( lParam);

	sliceList.MoveWindow(0, 0, cx, cy - 44);
	sliceSliderLabel.MoveWindow(0, cy-42, cx, 18);
	//sliceSlider.MoveWindow(0, cy-24, cx, 22);

	return 0;
}

LRESULT CWaveTabSlice::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CWaveTabSlice::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CWaveTabSlice::OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	
	// things to do with slices:
	//  - convert slices to single wave hits
	//  - convert slices to wavelevels with notes mapped to each slice
	//  - copy slice / crop to a single slice / cut slice / delete slice / duplicate slice
	//  - paste as slice
	// ::MessageBox(m_hWnd, "Slice Context", "Slice", MB_OK);
	return 0;
}

LRESULT CWaveTabSlice::OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	// this is supposed to generate slices using the slider for "følsomhet"
	return 0;
}

bool get_slice(int wavelen, std::vector<int>& slices, int slice, int* beginSample, int* endSample) {
	if (slice == 0) {
		*beginSample = 0;
		*endSample = slices[0] - 1;
	} else 
	if (slice == slices.size()) {
		*beginSample = slices[slice - 1];
		*endSample = wavelen - 1;
	} else {
		*beginSample = slices[slice - 1];
		*endSample = slices[slice] - 1;
	}
	return true;
}

LRESULT CWaveTabSlice::OnSliceChange(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/) {
	if (wavelevel == 0) return 0;

	int selitem = sliceList.GetSelectedIndex();
	if (selitem == -1) return 0;

	int slicecount;
	zzub_wavelevel_get_slices(wavelevel, &slicecount, 0);
	
	std::vector<int> slices(slicecount);
	if (!slices.empty())
		zzub_wavelevel_get_slices(wavelevel, &slicecount, &slices.front());
	slices.push_back(zzub_wavelevel_get_sample_count(wavelevel));

	int wavelen = zzub_wavelevel_get_sample_count(wavelevel);

	int beginsample, endsample;
	get_slice(wavelen, slices, selitem, &beginsample, &endsample);

	// set wave editor selection to slice
	if (!noselect)
		view->waveEditorCtrl.SetSelection(beginsample, endsample);

	CWaveInfo* currentInfo = view->GetCurrentWaveInfo();
	currentInfo->slice = selitem;

	//float sens = (float)sliceSlider.GetPos() / 0x7fff;
	//zzub_wavelevel_generate_slices(wavelevel, sens);
	return 0;
}

LRESULT CWaveTabSlice::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	CDCHandle bgDC((HDC)wParam);
	RECT rcClient;
	GetClientRect(&rcClient);
	rcClient.top = rcClient.bottom - 44;
	bgDC.FillRect(&rcClient, COLOR_3DFACE);
	return 1;
}

void CWaveTabSlice::SetWaveLevel(zzub_wavelevel_t* _wavelevel) {
	wavelevel = _wavelevel;

	sliceList.DeleteAllItems();

	if (wavelevel == 0) return ;

	int slicecount;
	zzub_wavelevel_get_slices(wavelevel, &slicecount, 0);
	
	int* slices = new int[slicecount + 1];
	zzub_wavelevel_get_slices(wavelevel, &slicecount, slices);
	slices[slicecount]= zzub_wavelevel_get_sample_count(wavelevel);

	int lastofs = 0;
	for (int i = 0 ; i < slicecount + 1; i++) {
		std::stringstream strm;
		strm << (i + 1);
		sliceList.InsertItem(i, strm.str().c_str());
		strm.str("");
		
		strm << lastofs;
		sliceList.SetItemText(i, 1, strm.str().c_str());
		strm.str("");

		strm << slices[i];
		sliceList.SetItemText(i, 2, strm.str().c_str());

		lastofs = slices[i];
	}

	delete[] slices;

	noselect = true;
	CWaveInfo* currentInfo = view->GetCurrentWaveInfo();
	if (currentInfo->slice >= 0 && currentInfo->slice < sliceList.GetItemCount())
		sliceList.SelectItem(currentInfo->slice); else
		sliceList.SelectItem(0);

	noselect = false;
}

/***

	CWaveTabEnvelope

***/

CWaveTabEnvelope::CWaveTabEnvelope(CWaveTableView* view) { 
	this->view=view; 
	autoCreateEnvelopes = true;
}

LRESULT CWaveTabEnvelope::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DefWindowProc();

	disabledButton.Create(m_hWnd, rcDefault, "Disabled", WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|BS_CHECKBOX|BS_NOTIFY, 0, IDC_DISABLEDBUTTON);
	disabledButton.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	envelopeDropDown.Create(m_hWnd, rcDefault, "Envelopes", WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VSCROLL|CBS_DROPDOWNLIST, 0, IDC_ENVELOPEDROPDOWN);
	waveMachineDropDown.Create(m_hWnd, rcDefault, "Envelopes", WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VSCROLL|CBS_DROPDOWNLIST, 0, IDC_WAVEMACHINEDROPDOWN);

	envelopeDropDown.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));
	waveMachineDropDown.SetFont((HFONT) GetStockObject( DEFAULT_GUI_FONT ));

	envelopeBackground.Create(m_hWnd, rcDefault, "", WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);

	if (!envelopeCtrl.Create(m_hWnd, rcDefault, "ENVELOPE.EnvelopeCtrl.1", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, IDC_ENVELOPECTRL)) {
		//MessageBox("Cannot open envelope.ocx");
	}

	UpdateMachines();

	return 0;
}

LRESULT CWaveTabEnvelope::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}


LRESULT CWaveTabEnvelope::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD( lParam);
	WORD cy = HIWORD( lParam);

	envelopeBackground.MoveWindow(0, 0, cx, 22);
	if (envelopeCtrl.m_hWnd) envelopeCtrl.MoveWindow(0, 22, cx, cy-22);
	disabledButton.MoveWindow(cx-100, 0, 100, 20);
	waveMachineDropDown.MoveWindow(2, 0, 100, 20 + 200);
	envelopeDropDown.MoveWindow(102, 0, 100, 20 + 200);

	return 0;
}

LRESULT CWaveTabEnvelope::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CWaveTabEnvelope::OnToggleDisabled(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	// TODO: undo...
	int e = GetSelectedEnvelope();
	if (e == -1) return 0;

	zzub_envelope_t* env = zzub_wave_get_envelope(view->currentWave, e);
	if (env == 0) return 0;
	
	disabledButton.SetCheck(!disabledButton.GetCheck());
	zzub_envelope_enable(env, !disabledButton.GetCheck());
	//entry->disabled = disabledButton.GetCheck();

	return 0;
}

LRESULT CWaveTabEnvelope::OnEnvelopeChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	if (!autoCreateEnvelopes) return 0;

	ECINFO* eci = (ECINFO*)wParam;
	view->SetEnvelope(eci->numPoints, eci->sustainPoint, eci->points);
	zzub_player_history_commit(view->player, 0, 0, "Set Envelope");
	return 0;
}

void CWaveTabEnvelope::UpdateMachines() {
	waveMachineDropDown.ResetContent();
	waveMachineDropDown.InsertString(-1, "<select machine>");
	for (int i = 0; i < zzub_player_get_plugin_count(view->player); i++) {
		zzub_plugin_t* plugin = zzub_player_get_plugin(view->player, i);
		if (buze_document_get_plugin_non_song(view->document, plugin)) continue;
		if ((zzub_plugin_get_flags(plugin) & zzub_plugin_flag_plays_waves) == 0) continue;

		const char* name = zzub_plugin_get_name(plugin);
		waveMachineDropDown.InsertString(-1, name);
	}
	waveMachineDropDown.SetCurSel(0);

	UpdateMachineEnvelopes();
}

int CWaveTabEnvelope::GetSelectedEnvelope() {
	int e = envelopeDropDown.GetCurSel();
	if (e == -1) return -1;
	
	int num_env = zzub_wave_get_envelope_count(view->currentWave);
	if (e >= num_env) {
		if (!autoCreateEnvelopes) return -1;
		zzub_wave_set_envelope_count(view->currentWave, e + 1);
	}

	return e;
}

void CWaveTabEnvelope::UpdateMachineEnvelopes() {
	// get selected wave machine and update envelope dropdown

	int index = waveMachineDropDown.GetCurSel();
	if (index == -1) return;

	char pc[1024];
	waveMachineDropDown.GetLBText(index, pc);

	zzub_plugin_t* pd = zzub_player_get_plugin_by_name(view->player, pc);
	if (pd == 0) return ;

	envelopeDropDown.ResetContent();

	int envelope_count = zzub_plugin_get_envelope_count(pd);

	if (envelope_count == 0) {
		envelopeDropDown.InsertString(-1, "<no envelope>");
		envelopeDropDown.SetCurSel(0);
	}

	for (int i = 0; i < envelope_count; i++) {
		envelopeDropDown.InsertString(-1, zzub_plugin_get_envelope_name(pd, i));
	}
	envelopeDropDown.SetCurSel(0);
	UpdateEnvelope();

}

void CWaveTabEnvelope::UpdateEnvelope() {
	// SetEnvelope will invoke OnEnvelopeChanged, which in turn will call setEnvelope()
	// We need to tell setEnvelope to not write anything to zzub, since updateEnvelope should
	// be callable in a zzub-event.. thus autoCreateEnvelopes = false here...

	autoCreateEnvelopes = false;
	if (view->currentWave == 0) return ;

	int e = GetSelectedEnvelope();
	if (e == -1) {
		// no envelope - reset envelope controls to default
		disabledButton.SetCheck(TRUE);
		long defaultEnvelope[] = { 0, 0, 65535, 65535 };
		if (envelopeCtrl.m_hWnd) envelopeCtrl->SetEnvelope(2, -1, defaultEnvelope);
		autoCreateEnvelopes = true;
		return ;
	}

	zzub_envelope_t* env = zzub_wave_get_envelope(view->currentWave, e);
	bool enabled = zzub_envelope_is_enabled(env)?true:false;
	disabledButton.SetCheck(!enabled);

	int sustainPoint = -1;
	std::vector<long> points;
	for (size_t i = 0; i < zzub_envelope_get_point_count(env); i++) {
		
		unsigned short x;
		unsigned short y;
		char flag;
		zzub_envelope_get_point(env, i, &x, &y, &flag);
		if (flag & 1) sustainPoint = i;
		points.push_back(x);
		points.push_back(65535-y);
	}
	if (envelopeCtrl.m_hWnd) envelopeCtrl->SetEnvelope(zzub_envelope_get_point_count(env), sustainPoint, &points.front());

	autoCreateEnvelopes = true;
}

LRESULT CWaveTabEnvelope::OnSelChangeWaveMachine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	waveMachineDropDown.ShowDropDown(FALSE);
	UpdateMachineEnvelopes();
	return 0;
}

LRESULT CWaveTabEnvelope::OnSelChangeEnvelope(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	envelopeDropDown.ShowDropDown(FALSE);
	UpdateEnvelope();
	return 0;
}

