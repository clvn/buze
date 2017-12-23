#include "stdafx.h"
#include "resource.h"
#include "PresetDialog.h"
#include "zzub/zzub.h"
#include "PresetManager.h"
#include "utils.h"

CPresetDialog::CPresetDialog(PresetManager* prs, zzub_player_t* _player, zzub_plugin_t* _plugin) {
	presets = prs;
	player = _player;
	plugin = _plugin;
}


LRESULT CPresetDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	commentBox.Attach(GetDlgItem(IDC_PRESET_COMMENT_EDIT));
	deleteButton.Attach(GetDlgItem(IDC_PRESET_DELETE_BUTTON));
	presetList.Attach(GetDlgItem(IDC_PRESET_LIST));
	nameBox.Attach(GetDlgItem(IDC_PRESET_NAME));

	for (size_t i = 0; i < presets->getPresetCount(); i++) {
		PresetInfo& pi = presets->getPreset(i);
		presetList.InsertString(i, pi.name.c_str());
	}
	
	return TRUE;
}

LRESULT CPresetDialog::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	EndDialog(wID);
	return 0;
}

void CPresetDialog::updatePresetInfo(PresetInfo* pi) {

	pi->tracks = zzub_plugin_get_track_count(plugin, 2);

	char pc[4096];
	commentBox.GetWindowText(pc, 4096);
	pi->comment = pc;
	
	zzub_archive_t* archive = zzub_archive_create_memory();
	zzub_output_t* outf = zzub_archive_get_output(archive, "");
	zzub_plugin_save(plugin, outf);

	zzub_input_t* inf = zzub_archive_get_input(archive, "");
	int input_size = zzub_input_size(inf);
	if (inf && input_size) {
		pi->savedata.resize(input_size);
		zzub_input_read(inf, &pi->savedata.front(), input_size);
	} else
		pi->savedata.resize(0);

	zzub_archive_destroy(archive);

	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(plugin);

	int valueIndex = 0;
	for (int k = 1; k < 3; ++k) {
		for (int j = 0; j < zzub_plugin_get_track_count(plugin, k); j++) {
			for (int i = 0; i < zzub_pluginloader_get_parameter_count(info, k); i++) {
				zzub_parameter_t* param = zzub_plugin_get_parameter(plugin, k, j, i);
				if ((zzub_parameter_get_flags(param) & zzub_parameter_flag_state) == 0) continue;

				pi->values[valueIndex] = zzub_plugin_get_parameter_value(plugin, k, j, i);
				pi->parameters++;
				valueIndex++;
			}
		}
	}

}

LRESULT CPresetDialog::OnBnClickedOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	char pc[4096];
	nameBox.GetWindowText(pc, 4096);

	// overwrite existing preset?
	int preset_index = presets->findPreset(pc);

	if (preset_index == -1) {
		// no - create a new
		PresetInfo pi;
		pi.name = pc;
		updatePresetInfo(&pi);
		presets->add(pi);
	} else {
		// yes - pls
		PresetInfo& pi = presets->getPreset(preset_index);
		updatePresetInfo(&pi);
	}

	EndDialog(wID);

	return 0;
}

LRESULT CPresetDialog::OnPresetChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = presetList.GetCurSel();

	if (sel != -1) {
		PresetInfo& pi = presets->getPreset(sel);
		nameBox.SetWindowText(pi.name.c_str());
		commentBox.SetWindowText(pi.comment.c_str());
	}

	return 0;
}

LRESULT CPresetDialog::OnPresetDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int sel = presetList.GetCurSel();

	if (sel != -1) {
		presets->remove(sel);
		EndDialog(IDOK);
	}

	return 0;
}
