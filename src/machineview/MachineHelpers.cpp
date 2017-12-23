#include "stdafx.h"
#include <iomanip>
#include <strstream>
#include <limits>
#include <fstream>
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "Configuration.h"
#include "Properties.h"
#include "DragDropImpl.h"
#include "MachineDropTarget.h"
#include "utils.h"
#include "Keymaps.h"

#include "BuzeConfiguration.h"
#include "Mixdown.h"
#include "MachineView.h"
#include "MachineHelpers.h"

// ---------------------------------------------------------------------------------------------------------------
// HELPERS
// ---------------------------------------------------------------------------------------------------------------

void load_binary(std::string filename, char** image, int* size) {
	std::ifstream fs;
	fs.open(filename.c_str(), std::ios::in | std::ios::binary);
	if (!fs) return ;
	fs.seekg (0, std::ios::end);
	*size = (int)fs.tellg();
	*image = new char[*size];
	fs.seekg (0, std::ios::beg);
	fs.read(*image, *size);
	fs.close();
}

void save_binary(std::string filename, const char* image, int size) {
	std::ofstream fs;
	fs.open(filename.c_str(), std::ios::out | std::ios::binary);
	fs.write(image, size);
	fs.close();
}

/*void SetMenuItemData(CMenuHandle menu, int pos, void* data) {
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_DATA;
	mii.dwItemData = (DWORD)data;
	menu.SetMenuItemInfo(pos, TRUE, &mii);
}

void* FindMenuItemDataByID(CMenuHandle menu, int wID) {
	for (int i = 0; i < menu.GetMenuItemCount(); i++) {
		MENUITEMINFO mii;
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_ID | MIIM_DATA | MIIM_SUBMENU;
		mii.wID = 0;
		mii.dwItemData = 0;
		mii.hSubMenu = 0;
		menu.GetMenuItemInfo(i, TRUE, &mii);
		if (mii.wID == wID) {
			return (void*)mii.dwItemData;
		}
		if (mii.hSubMenu != 0) {
			void* result = FindMenuItemDataByID(mii.hSubMenu, wID);
			if (result != 0) return result;
		}
	}
	return 0;
}
*/
void zzub_audio_connection_set_amp(zzub_plugin_t* connplug, int amp, bool with_undo) {
	int tracks = zzub_plugin_get_track_count(connplug, 2);
	for (int i = 0; i < tracks; i++) {
		if (with_undo)
			zzub_plugin_set_parameter_value(connplug, 2, i, 0, amp, true);
		else
			zzub_plugin_set_parameter_value_direct(connplug, 2, i, 0, amp, true);
	}
}

int zzub_audio_connection_get_amp(zzub_plugin_t* connplug) {
	int tracks = zzub_plugin_get_track_count(connplug, 2);
	
	// during ordinary use, tracks should never be 0 here, but some plugins (pvst) might send us redraw messages while there are incomplete connections, and this was the simplest fix:
	if (tracks == 0) return 0;

	int totamp = 0;
	for (int i = 0; i < tracks; i++) {
		totamp += zzub_plugin_get_parameter_value(connplug, 2, i, 0);
	}
	return totamp / tracks;
}

zzub_plugin_t* zzub_plugin_group_get_input_plugin(zzub_plugin_group_t* layer) {
	zzub_plugin_t* result = 0;
	zzub_plugin_iterator_t* plugit = zzub_plugin_group_get_plugins(layer);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		if (zzub_plugin_get_flags(plugin) & zzub_plugin_flag_has_group_input) {
			result = plugin;
			break;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);
	return result;
}

zzub_plugin_t* zzub_plugin_group_get_output_plugin(zzub_plugin_group_t* layer) {
	zzub_plugin_t* result = 0;
	zzub_plugin_iterator_t* plugit = zzub_plugin_group_get_plugins(layer);
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		if (zzub_plugin_get_flags(plugin) & zzub_plugin_flag_has_group_output) {
			result = plugin;
			break;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);
	return result;
}

zzub_plugin_group_t* zzub_player_create_group_with_io(zzub_player_t* player, zzub_plugin_group_t* currentlayer, float x, float y) {

	zzub_plugin_group_t* plugingroup = zzub_player_create_plugin_group(player, currentlayer, "Group");
	zzub_plugin_group_set_position(plugingroup, x, y);

	zzub_pluginloader_t* groupininfo = zzub_player_get_pluginloader_by_name(player, "@zzub.org/group/input");
	assert(groupininfo != 0);
	zzub_plugin_t* groupin = zzub_player_create_plugin(player, 0, 0, "GroupIn", groupininfo, plugingroup);
	assert(groupin != 0);
	zzub_plugin_set_position(groupin, -0.75, -0.75);

	zzub_pluginloader_t* groupoutinfo = zzub_player_get_pluginloader_by_name(player, "@zzub.org/group/output");
	assert(groupoutinfo != 0);
	zzub_plugin_t* groupout = zzub_player_create_plugin(player, 0, 0, "GroupOut", groupoutinfo, plugingroup);
	assert(groupin != 0);
	zzub_plugin_set_position(groupout, 0.75, 0.75);

	return plugingroup;
}

zzub_plugin_group_t* zzub_player_get_plugin_group_by_index(zzub_player_t* player, zzub_plugin_group_t* currentlayer, int* counter, int groupindex) {
	zzub_plugin_group_t* plugingroup = 0;
	zzub_plugin_group_iterator_t* groupit = zzub_player_get_plugin_group_iterator(player, currentlayer);
	while (zzub_plugin_group_iterator_valid(groupit)) {
		plugingroup = zzub_plugin_group_iterator_current(groupit);

		if (*counter == groupindex)
			break;

		(*counter)++;

		plugingroup = zzub_player_get_plugin_group_by_index(player, plugingroup, counter, groupindex);
		if (plugingroup != 0)
			break;

		zzub_plugin_group_iterator_next(groupit);
	}
	zzub_plugin_group_iterator_destroy(groupit);

	return plugingroup;
}

zzub_plugin_group_t* zzub_player_get_plugin_group_by_index(zzub_player_t* player, int groupindex) {

	int counter = 1; // 0 is root and must be checked before calling this function
	return zzub_player_get_plugin_group_by_index(player, 0, &counter, groupindex);

}

void FindVisibleInputs(buze_document_t* document, zzub_plugin_t* hiddenplugin, std::vector<zzub_plugin_t*>& result) {

	// TODO: can this go infinity if there are hidden plugins in a feedback loop?

	for (int j = 0; j < zzub_plugin_get_input_connection_count(hiddenplugin); j++) {
		zzub_connection_t* conn = zzub_plugin_get_input_connection(hiddenplugin, j);
		zzub_plugin_t* from_plugin = zzub_connection_get_from_plugin(conn);
		if (buze_document_get_plugin_non_song(document, from_plugin)) {
			// either recurse upwards via from_plugin to find visible inputs ...
			FindVisibleInputs(document, from_plugin, result);
		} else {
			std::vector<zzub_plugin_t*>::iterator i = std::find(result.begin(), result.end(), from_plugin);
			if (i == result.end())
				result.push_back(from_plugin);
		}
	}
}

nodetype GetNodeType(zzub_plugin_t* machine) {
	if ((zzub_plugin_get_flags(machine) & zzub_plugin_flag_is_root) != 0) {
		return node_master;
	} else
	if ((zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) & IS_CONTROLLER_PLUGIN_FLAGS) {
		return node_controller;
	} else
	if ((zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) & IS_EFFECT_PLUGIN_FLAGS) {
		return node_effect;
	} else
	if ((zzub_plugin_get_flags(machine) & PLUGIN_FLAGS_MASK) == IS_GENERATOR_PLUGIN_FLAGS) {
		return node_generator;
	} else
	// fallback to generator
		return node_generator;
}

edgetype GetEdgeType(zzub_connection_type type) {
	switch (type) {
		case zzub_connection_type_audio:
			return edge_audio;
		case zzub_connection_type_midi:
			return edge_midi;
		case zzub_connection_type_event:
			return edge_event;
		case zzub_connection_type_note:
			return edge_note;
	}
	assert(false);
	return (edgetype)-1;
}

float get_scaled_amp(zzub_connection_t* conn) {
	if (zzub_connection_get_type(conn) != zzub_connection_type_audio) 
		return 1.0f;
	zzub_plugin_t* connplug = zzub_connection_get_connection_plugin(conn);
	zzub_parameter_t* volparam = zzub_plugin_get_parameter(connplug, 2, 0, 0);
	assert(volparam != 0);

	int value = zzub_audio_connection_get_amp(connplug);
	int volmax = zzub_parameter_get_value_max(volparam);
	int volmin = zzub_parameter_get_value_min(volparam);
	return (float)(value - volmin) / (float)(volmax - volmin) * 2; // multiply by 2 since conn amp goes to 200%
}

zzub_plugin_group_t* zzub_plugin_group_get_child_group(zzub_plugin_group_t* plugingroup, zzub_plugin_group_t* childgroup) {
	if (childgroup == 0) return 0;
	for (;;) {
		zzub_plugin_group_t* parent = zzub_plugin_group_get_parent(childgroup);
		if (parent == plugingroup) return childgroup;
		if (parent == 0) return 0;
		childgroup = parent;
	}
}

bool is_valid_parent(zzub_plugin_group_t* selectedgroup, zzub_plugin_group_t* parentgroup) {
	if (selectedgroup == parentgroup) return false;

	bool already_parent = zzub_plugin_group_get_parent(selectedgroup) == parentgroup;
	if (already_parent) return false;

	bool already_child = zzub_plugin_group_get_child_group(selectedgroup, parentgroup) != 0;
	if (already_child) return false;

	return true;
}

//extern void SetMenuItemData(CMenuHandle menu, int pos, void* data);

int zzub_plugin_get_note_group(zzub_plugin_t* plugin) {
	zzub_pluginloader_t* info = zzub_plugin_get_pluginloader(plugin);
	for (int i = 1; i <= 2; i++) {
		int paramcount = zzub_pluginloader_get_parameter_count(info, i);
		for (int j = 0; j < paramcount; j++) {
			zzub_parameter_t* param = zzub_pluginloader_get_parameter(info, i, j);
			int type = zzub_parameter_get_type(param);
			if (type == zzub_parameter_type_note) return i;
		}
	}
	return -1;
}

std::string GetOutputChannelName(zzub_plugin_t* plugin, int index) {
	const char* namestr = zzub_plugin_get_output_channel_name(plugin, index);

	if (namestr != 0)
		return namestr;

	std::stringstream strm;
	strm << "Out " << (index + 1);
	return strm.str();
}

std::string GetInputChannelName(zzub_plugin_t* plugin, int index) {
	const char* namestr = zzub_plugin_get_input_channel_name(plugin, index);

	if (namestr != 0)
		return namestr;

	std::stringstream strm;
	strm << "In " << (index + 1);
	return strm.str();
}
