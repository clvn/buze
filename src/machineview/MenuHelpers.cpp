#include "stdafx.h"
#include <iomanip>
#include <strstream>
#include <limits>
#include <fstream>
#include "resource.h"
#include <buze/buzesdk.h>
#include "MenuHelpers.h"
#include "utils.h"

extern bool is_valid_parent(zzub_plugin_group_t* selectedgroup, zzub_plugin_group_t* parentgroup);

void BindPluginGroupMenu(zzub_player_t* player, CMenuHandle& groupMenu, int* counter, int level, zzub_plugin_group_t* parentgroup, zzub_plugin_group_t* selectedgroup) {

	if (parentgroup == 0) {
		UINT itemFlags = MF_STRING;
		bool canMove = selectedgroup == 0 || is_valid_parent(selectedgroup, parentgroup);//zzub_plugin_group_get_parent(selectedgroup) != 0;
		if (!canMove) itemFlags |= MF_GRAYED;
		groupMenu.AppendMenu(itemFlags, (UINT_PTR)ID_MOVE_TO_GROUP_FIRST + *counter, "Root");
		(*counter)++;
	}

	//if ((zzub_plugin_group_get_child_group(selgroup, plugingroup) == 0) && plugingroup != selgroup)

	zzub_plugin_group_iterator_t* groupit = zzub_player_get_plugin_group_iterator(player, parentgroup);
	while (zzub_plugin_group_iterator_valid(groupit)) {
		zzub_plugin_group_t* plugingroup = zzub_plugin_group_iterator_current(groupit);

		const char* groupname = zzub_plugin_group_get_name(plugingroup);
		std::stringstream strm;
		for (int i = 0; i < level; i++)
			strm << "      ";
		strm << groupname;

		UINT itemFlags = MF_STRING;
		bool canMove = selectedgroup == 0 || is_valid_parent(selectedgroup, plugingroup);
		if (!canMove) itemFlags |= MF_GRAYED;

		groupMenu.AppendMenu(itemFlags, (UINT_PTR)ID_MOVE_TO_GROUP_FIRST + *counter, strm.str().c_str());
		(*counter)++;

		BindPluginGroupMenu(player, groupMenu, counter, level + 1, plugingroup, selectedgroup);

		zzub_plugin_group_iterator_next(groupit);
	}
	zzub_plugin_group_iterator_destroy(groupit);
}

void BindPluginGroupMenu(zzub_player_t* player, CMenu& menu, zzub_plugin_group_t* selectedgroup) {
	CMenuHandle groupMenu;
	groupMenu.CreatePopupMenu();
	int counter = 0;
	BindPluginGroupMenu(player, groupMenu, &counter, 0, 0, selectedgroup);

	menu.AppendMenu(MF_STRING, (UINT_PTR)ID_CREATE_GROUP_FROM_SELECTION, "Create Group From Selected Plugins");
	menu.AppendMenu(MF_POPUP, (UINT_PTR)groupMenu.m_hMenu, "Move Plugin(s) To Group");

}

void BindPluginTimingMenu(zzub_player_t* player, CMenu& menu, zzub_plugin_t* machine) {

	zzub_plugin_t* timesource;
	int timesourcegroup;
	int timesourcetrack;

	if (machine) {
		timesource = zzub_plugin_get_timesource_plugin(machine);
		timesourcegroup = zzub_plugin_get_timesource_group(machine);
		timesourcetrack = zzub_plugin_get_timesource_track(machine);
	} else {
		timesource = 0;
		timesourcegroup = -1;
		timesourcetrack = -1;
	}

	CMenuHandle timingMenu;
	timingMenu.CreatePopupMenu();

	for (int i = 0; i < zzub_player_get_timesource_count(player); i++) {
		zzub_plugin_t* plugin = zzub_player_get_timesource_plugin(player, i);
		if (plugin == 0) continue; // no sequence plugin
		int timegroup = zzub_player_get_timesource_group(player, i);
		int timetrack = zzub_player_get_timesource_track(player, i);

		int trackcount = zzub_plugin_get_track_count(plugin, timegroup);
		if (timetrack >= trackcount) continue;

		int checked = (timesource == plugin && timegroup == timesourcegroup && timetrack == timesourcetrack) ? MF_CHECKED : 0;
		std::stringstream strm;
		strm << zzub_plugin_get_name(plugin);
		if (timegroup == 1)
			strm << ": Global";
		else 
		if (timegroup == 2)
			strm << ": Track " << timetrack;
		else
			strm << ": (Unknown)";
		timingMenu.AppendMenu(MF_STRING|checked, (UINT_PTR)ID_MACHINE_TIMESOURCE_FIRST + i, strm.str().c_str());
	}

	menu.AppendMenu(MF_POPUP, (UINT_PTR)timingMenu.m_hMenu, "Time Source");

}


void BindPluginCommandsMenu(zzub_player_t* player, CMenu& menu, zzub_plugin_t* machine) {
	// TODO: find a similar way to returning commands and subcommands
	static std::vector<char> bytes(1024 * 64);
	zzub_plugin_get_commands(machine, &bytes.front(), (int)bytes.size());
	std::string cmd(&bytes.front());//machine->getCommands();

	if (cmd.length()) {
		menu.AppendMenu(MF_SEPARATOR);
		std::string commandString = cmd;

		std::vector<std::string> commands;
		split<std::vector<std::string> > (commandString, commands, "\n");

		int subgroup = 1;

		for (int i = 0; i < (int)commands.size(); i++) {
			// if command begins with /, we check for machineEx and call getsubmenu
			std::string cmdstr = commands[i];

			// på peer maskiner kræsjer denne snutten dersom sangen ikke har blitt spilt
			if (cmdstr.length() && cmdstr.at(0)=='/') {
				zzub_plugin_get_sub_commands(machine, i, &bytes.front(), (int)bytes.size());
				cmd = &bytes.front();//machine->getSubCommands(i);
				CMenuHandle subMenu;
				subMenu.CreatePopupMenu();
				if (cmd.length()) {
					commandString = cmd;

					std::vector<std::string> subcommands;
					split<std::vector<std::string> >(commandString, subcommands, "\n");
					for (size_t j = 0; j < subcommands.size(); j++) {
						subMenu.AppendMenu(MF_STRING, (UINT_PTR)(ID_MACHINECOMMANDS+subgroup*256+j), subcommands[j].c_str());
					}
				}

				subgroup++;
				cmdstr = cmdstr.substr(1);
				menu.AppendMenu(MF_POPUP, (UINT_PTR)subMenu.m_hMenu, cmdstr.c_str());
			} else {
				menu.AppendMenu(MF_STRING, (UINT_PTR)(ID_MACHINECOMMANDS+i), cmdstr.c_str());
			}
		}
	}
}
