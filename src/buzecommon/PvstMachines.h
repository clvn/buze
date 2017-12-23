#pragma once

// this is:
//	- wrap the getMenu() method in Machines.dll provided by Polacs VST wrappers
//  - re-implement the portions of Machines.dll that build the menu for buze

HMENU pvst_get_menu(int type, DWORD dwFirstCommand);
void pvst_show_hide_all(bool show);
