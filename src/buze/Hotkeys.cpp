#include "stdafx.h"
#include "resource.h"
#include "Hotkeys.h"

#include <fstream>
#include <string>
#include <boost/tokenizer.hpp>
#include "utils.h"

// ---------------------------------------------------------------------------------------------------------------
// VIRTUAL KEYS
// ---------------------------------------------------------------------------------------------------------------

static vkey_desc_t const vkey_descs[] =
{
	{ "backspace", VK_BACK },
	{ "tab", VK_TAB },
	{ "clear", VK_CLEAR },
	{ "enter", VK_RETURN },
	{ "pause", VK_PAUSE },
	{ "capslock", VK_CAPITAL },
	{ "escape", VK_ESCAPE },
	{ "space", VK_SPACE },
	{ "pageup", VK_PRIOR },
	{ "pagedown", VK_NEXT },
	{ "end", VK_END },
	{ "home", VK_HOME },
	{ "left", VK_LEFT },
	{ "up", VK_UP },
	{ "right", VK_RIGHT },
	{ "down", VK_DOWN },
	{ "select", VK_SELECT },
	{ "print", VK_PRINT },
	{ "execute", VK_EXECUTE },
	{ "printscreen", VK_SNAPSHOT },
	{ "insert", VK_INSERT },
	{ "delete", VK_DELETE },
	{ "help", VK_HELP },
	{ "numpad0", VK_NUMPAD0 },
	{ "numpad1", VK_NUMPAD1 },
	{ "numpad2", VK_NUMPAD2 },
	{ "numpad3", VK_NUMPAD3 },
	{ "numpad4", VK_NUMPAD4 },
	{ "numpad5", VK_NUMPAD5 },
	{ "numpad6", VK_NUMPAD6 },
	{ "numpad7", VK_NUMPAD7 },
	{ "numpad8", VK_NUMPAD8 },
	{ "numpad9", VK_NUMPAD9 },
	{ "multiply", VK_MULTIPLY },
	{ "add", VK_ADD },
	{ "separator", VK_SEPARATOR },
	{ "subtract", VK_SUBTRACT },
	{ "decimal", VK_DECIMAL },
	{ "divide", VK_DIVIDE },
	{ "f1", VK_F1 },
	{ "f2", VK_F2 },
	{ "f3", VK_F3 },
	{ "f4", VK_F4 },
	{ "f5", VK_F5 },
	{ "f6", VK_F6 },
	{ "f7", VK_F7 },
	{ "f8", VK_F8 },
	{ "f9", VK_F9 },
	{ "f10", VK_F10 },
	{ "f11", VK_F11 },
	{ "f12", VK_F12 },
	{ "f13", VK_F13 },
	{ "f14", VK_F14 },
	{ "f15", VK_F15 },
	{ "f16", VK_F16 },
	{ "numlock", VK_NUMLOCK },
	{ "scrolllock", VK_SCROLL },
	{ "oem_1", VK_OEM_1 },
	{ "oem_plus", VK_OEM_PLUS },
	{ "oem_comma", VK_OEM_COMMA },
	{ "oem_minus", VK_OEM_MINUS },
	{ "oem_period", VK_OEM_PERIOD },
	{ "oem_2", VK_OEM_2 },
	{ "oem_3", VK_OEM_3 },
	{ "oem_4", VK_OEM_4 },
	{ "oem_5", VK_OEM_5 },
	{ "oem_6", VK_OEM_6 },
	{ "oem_7", VK_OEM_7 },
	{ "oem_8", VK_OEM_8 },
};
static size_t const vkey_descs_count = array_size(vkey_descs);

// ---------------------------------------------------------------------------------------------------------------
// KEYJAZZ
// ---------------------------------------------------------------------------------------------------------------

struct keyjazz_desc_t {
	char const* name;
	int note_or_cmd;
};

static keyjazz_desc_t const keyjazz_descs[] =
{
	{ "C-0", 0  },
	{ "C#0", 1  },
	{ "D-0", 2  },
	{ "D#0", 3  },
	{ "E-0", 4  },
	{ "F-0", 5  },
	{ "F#0", 6  },
	{ "G-0", 7  },
	{ "G#0", 8  },
	{ "A-0", 9  },
	{ "A#0", 10 },
	{ "B-0", 11 },
	{ "C-1", 12 },
	{ "C#1", 13 },
	{ "D-1", 14 },
	{ "D#1", 15 },
	{ "E-1", 16 },
	{ "F-1", 17 },
	{ "F#1", 18 },
	{ "G-1", 19 },
	{ "G#1", 20 },
	{ "A-1", 21 },
	{ "A#1", 22 },
	{ "B-1", 23 },
	{ "C-2", 24 },
	{ "C#2", 25 },
	{ "D-2", 26 },
	{ "D#2", 27 },
	{ "E-2", 28 },
	{ "note_off",     255 },
	{ "note_cut",     254 },
	{ "jazz_lastnote", -2 },
	{ "jazz_track",    -3 },
	{ "jazz_row",      -4 },
	{ "octave_up",     -5 },
	{ "octave_down",   -6 },
};
static size_t const keyjazz_descs_count = array_size(keyjazz_descs);

// ---------------------------------------------------------------------------------------------------------------
// CHOTKEYS
// ---------------------------------------------------------------------------------------------------------------

CHotkeys::CHotkeys() {
}

CHotkeys::~CHotkeys() {
	DestroyAccelTables();
}

void CHotkeys::Reload() {
	view_accel_reverse_maps.clear();
	view_accel_maps.clear();
	keyjazz_key_map.clear();
	DestroyAccelTables();
	ParseJson();
}

HACCEL CHotkeys::GetAccelerators(const std::string& view) {
	std::map<std::string, HACCEL>::iterator i = accel_map.find(view);
	if (i == accel_map.end()) return 0;
	return i->second;
}

void CHotkeys::Initialize() {
	InitMaps();
	bool success = ParseJson();
	//if (success)
		//CreateAccelTables();
}

accel_default_t* GetOverrideAccel(CHotkeys* self, const std::string& view, const std::string& action) {
	std::map<std::string, std::vector<accel_default_t> >:: iterator viewit = self->accel_default_map.find(view);
	std::vector<std::string> hotkeyparts;
	if (viewit != self->accel_default_map.end()) {
		for (std::vector<accel_default_t>::iterator i = viewit->second.begin(); i != viewit->second.end(); ++i) {
			if (i->action == action)
				return &*i;
		}
	}
	return 0;
}

void CHotkeys::RegisterAcceleratorId(const std::string& view, const std::string& name, WORD id, const std::string& hotkey) {
	id_map_t& id_map = view_id_maps[view];
	id_map[name] = id;

	accel_vec_t& accel_vec = view_accel_maps[view];
	accel_vec.reserve(2048); // :( works, but not 100% - needs to reserve the ACCEL array because the reverse map stores pointers
	accel_reverse_map_t& accel_reverse_map = view_accel_reverse_maps[view];

	accel_default_t* defacc = GetOverrideAccel(this, view, name);
	std::vector<std::string> hotkeyparts;
	if (defacc != 0) {
		hotkeyparts = defacc->hotkeys ;
	} else {
		split(hotkey, hotkeyparts, ",");
	}

	for (std::vector<std::string>::iterator i = hotkeyparts.begin(); i != hotkeyparts.end(); ++i) {
		std::string hk = trim(*i);
		CreateStringAccel(view, name, hk, accel_vec, accel_reverse_map, id_map);
	}

}

void CHotkeys::InitMaps() {
	for (size_t i = 0; i < vkey_descs_count; ++i) {
		vkey_desc_t const& vkey_desc = vkey_descs[i];
		vkey_map[vkey_desc.name] = vkey_desc.vkey;
	}

	for (size_t i = 0; i < keyjazz_descs_count; ++i) {
		keyjazz_desc_t const& keyjazz_desc = keyjazz_descs[i];
		keyjazz_map[keyjazz_desc.name] = keyjazz_desc.note_or_cmd;
	}
}

std::string CHotkeys::GetPath() const {
	HMODULE module_handle = ::GetModuleHandle(0);
	if (!module_handle) return "";

	char path[MAX_PATH + 32] = { 0 };
	::GetModuleFileName(module_handle, path, MAX_PATH);
	std::size_t n = std::strlen(path);
	if (!n) return "";
	while (n--) {
		if (path[n]=='\\') {
			path[n] = 0;
			break;
		}
	}

	std::string s = path;
	s += "\\hotkeys.json";

	return s;
}

bool CHotkeys::ParseJson() {
	std::string path = GetPath();
	std::string input = read_file(path);

	Json::Reader reader;
	Json::Value root;

	bool parsingSuccessful = reader.parse(input, root);

	if (!parsingSuccessful) {
		std::cerr << "hotkeys: failed to parse file: " << path << std::endl;
		std::cerr << "hotkeys: parsing error: " << reader.getFormatedErrorMessages() << std::endl;
		std::stringstream ss; ss << path << "\n\n" << reader.getFormatedErrorMessages() << std::endl;
		::MessageBox(::GetForegroundWindow(), ss.str().c_str(), "hotkeys parse error!", MB_OK);
		return false;
	}

	bool success = true;
	for (Json::Value::iterator val_it = root.begin(); val_it != root.end(); ++val_it) {
		Json::Value& kv = *val_it;
		if (kv.type() == Json::objectValue)
			success &= ParseViewAccels(root, val_it.memberName());
	}
	success &= CreateKeyjazzKeyMap(root);
	return true;
}

bool CHotkeys::CreateKeyjazzKeyMap(Json::Value& root) {
	Json::Value v = root["keyjazz"];

	if (v.type() != Json::objectValue)
		return false;

	if (!v.empty()) {
		for (Json::Value::iterator val_it = v.begin(); val_it != v.end(); ++val_it) {
			Json::Value& kv = *val_it;

			if (kv.type() == Json::stringValue) {
				CreateKeyjazzKey(val_it.memberName(), kv.asString());
			} else
			if (kv.type() == Json::arrayValue) {
				for (Json::Value::iterator val_it2 = kv.begin(); val_it2 != kv.end(); ++val_it2) {
					Json::Value& kkv = *val_it2;
					if (kkv.type() == Json::stringValue) {
						CreateKeyjazzKey(val_it.memberName(), kkv.asString());
					} else {
						return false;
					}
				}
			} else {
				return false;
			}
		}
	}

	return true;
}

bool CHotkeys::CreateKeyjazzKey(std::string const& name, std::string const& s) {
	keyjazz_map_t::const_iterator keyjazz_it = keyjazz_map.find(name);
	if (keyjazz_it == keyjazz_map.end())
		return false;

	if (s == "") // skip
		return true;

	WORD keycode;

	if (s.length() == 1) {
		keycode = toupper(s[0]);
	} else {
		vkey_map_t::const_iterator vkey_it = vkey_map.find(s);
		if (vkey_it == vkey_map.end())
			return false;

		keycode = (*vkey_it).second;
	}

	keyjazz_key_map[keycode] = (*keyjazz_it).second;
	return true;
}

bool CHotkeys::ParseViewAccels(Json::Value& root, std::string const& view) {
	Json::Value v = root[view];

	if (v.type() != Json::objectValue)
		return false;

	if (!v.empty()) {
		for (Json::Value::iterator val_it = v.begin(); val_it != v.end(); ++val_it) {
			Json::Value& vk = *val_it;
			std::vector<accel_default_t>& defaults = accel_default_map[view];

			if (vk.type() == Json::stringValue) {
				accel_default_t accdef;
				accdef.action = val_it.memberName();
				accdef.hotkeys.push_back(vk.asString());
				defaults.push_back(accdef);
			} else
			if (vk.type() == Json::arrayValue) {
				accel_default_t accdef;
				accdef.action = val_it.memberName();
				for (Json::Value::iterator val2_it = vk.begin(); val2_it != vk.end(); ++val2_it) {
					Json::Value& vkk = *val2_it;

					if (vkk.type() == Json::stringValue) {
						accdef.hotkeys.push_back(vkk.asString());
					} else {
						return false;
					}
				}
				defaults.push_back(accdef);
			} else {
				return false;
			}
		}
	}

	return true;
}

void CHotkeys::ShowHotkeyError(std::string const& view, std::string const& name) {
	std::cerr << "hotkeys: error in accelerator: " << view << "." << name << std::endl;
	std::stringstream ss; ss << "error in accelerator: " << view << "." << name << std::endl;
	//::MessageBox(::GetForegroundWindow(), ss.str().c_str(), "hotkeys accelerator error!", MB_OK);
}

int CHotkeys::CreateStringAccel(std::string const& view, std::string const& name, std::string const& hotkey, accel_vec_t& accel_vec, accel_reverse_map_t& accel_reverse_map, id_map_t const& id_map) {
	ACCEL acc = { 0 };

	CreateAccelResult result = CreateAccel(name, hotkey, acc, id_map);

	if (result == create_ok) {
		accel_vec.push_back(acc);
		accel_reverse_map.insert(std::make_pair(acc.cmd, &accel_vec.back()));
	} else
	if (result == create_skip) {
		// skip
	} else {
		ShowHotkeyError(view, name);
	}

	return 0;///
}

CHotkeys::CreateAccelResult CHotkeys::CreateAccel(std::string const& name, std::string const& hotkey, ACCEL& acc, const id_map_t& id_map) {
	// id must be registered ahead .. at the moment we are just enumerating possible name/hotkey combos from the json
	id_map_t::const_iterator id_it = id_map.find(name);
	if (id_it == id_map.end())
		return create_error;
		
	typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	boost::char_separator<char> sep(" ");
	tokenizer tokens(hotkey, sep);

// 	if (tokens.size() == 0)
// 		return create_skip;

	acc.cmd = (*id_it).second;

	bool got_key = false;
	bool got_modifier = false;
	bool is_char = false;

	int tok_count = 0;

	for (tokenizer::iterator tok_it = tokens.begin(); tok_it != tokens.end(); ++tok_it) {
		std::string s = *tok_it;

		if (s == "shift") {
			acc.fVirt |= FSHIFT;
			got_modifier = true;
		} else
		if (s == "ctrl") {
			acc.fVirt |= FCONTROL;
			got_modifier = true;
		} else
		if (s == "alt") {
			acc.fVirt |= FALT;
			got_modifier = true;
		} else
		if (s.length() == 1) {
			if (got_key)
				return create_error;

			acc.key = s[0];
			got_key = true;
			is_char = true;
		} else {
			if (got_key)
				return create_error;

			vkey_map_t::const_iterator vkey_it = vkey_map.find(s);
			if (vkey_it == vkey_map.end())
				return create_error;

			acc.key = (*vkey_it).second;
			got_key = true;
			got_modifier = true;
		}

		++tok_count;
	}

	if (tok_count == 0)
		return create_skip;

	if (!got_key)
		return create_error;

	if (got_modifier || !is_char) ///|| !is_char
		acc.fVirt |= FVIRTKEY;

	if (got_modifier && is_char)
		acc.key = toupper(acc.key);

	return create_ok;
}

void CHotkeys::CreateAccelTables() {
	for (view_id_maps_t::iterator i = view_id_maps.begin(); i != view_id_maps.end(); ++i) {
		HACCEL hAccel;
		CreateAccelTable(i->first, hAccel);
	}
}

void CHotkeys::CreateAccelTable(std::string const& view, HACCEL& hAccel) {
	accel_vec_t& accel_vec = view_accel_maps[view];
	if (!accel_vec.empty()) {
		hAccel = ::CreateAcceleratorTable(&accel_vec.front(), accel_vec.size());
		accel_map[view] = hAccel;
	}
}

void CHotkeys::DestroyAccelTables() {
	for (std::map<std::string, HACCEL>::iterator i = accel_map.begin(); i != accel_map.end(); ++i) {
		::DestroyAcceleratorTable(i->second);
	}
}

// ---------------------------------------------------------------------------------------------------------------
// MENU UPDATING
// ---------------------------------------------------------------------------------------------------------------

// this helper from codeproject.com
// (c) 2004 Jörgen Sigvardsson <jorgen@profitab.com>
inline CString NameFromVKey(UINT nVK) {
	UINT nScanCode = ::MapVirtualKeyEx(nVK, 0, ::GetKeyboardLayout(GetCurrentThreadId()));

	switch (nVK) { // Keys which are "extended" (except for Return which is Numeric Enter as extended)
		case VK_INSERT:
		case VK_DELETE:
		case VK_HOME:
		case VK_END:
		case VK_NEXT:
		case VK_PRIOR:
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			nScanCode |= 0x100; // Add extended bit
	}

	// GetKeyNameText() expects the scan code to be on the same format as WM_KEYDOWN hence the left shift
	CString str;
	LPTSTR prb = str.GetBuffer(80);
	BOOL bResult = ::GetKeyNameText(nScanCode << 16, prb, 79);

	// these key names are capitalized and look a bit daft
	int len = lstrlen(prb);
	if (len > 1) {
		LPTSTR p2 = ::CharNext(prb);
		::CharLowerBuff(p2, len - (p2 - prb));
	}

	str.ReleaseBuffer();
	ATLASSERT(str.GetLength());
	return str; // internationalization ready, sweet!
}

inline CString NameFromAccel(ACCEL const& acc) {
	CString name;

	if (acc.fVirt & FCONTROL)
		name = "Ctrl+";
	if (acc.fVirt & FALT)
		name += "Alt+";
	if (acc.fVirt & FSHIFT)
		name += "Shift+";

	if (acc.fVirt & FVIRTKEY) {
		name += NameFromVKey(acc.key);
	} else {
		// key field is an ASCII key code.
#ifdef _UNICODE
		char    ca = (char)acc.key;
		wchar_t cu;

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, &ca, 1, &cu, 1);
		name += cu;
#else
		name += (char)acc.key;
#endif
	}

	ATLASSERT(name.GetLength());
	return name;
}

void CHotkeys::UpdateMenuKeys(std::string const& view, HMENU hMenu, bool show_accels) {
	accel_reverse_map_t const& accel_reverse_map = view_accel_reverse_maps[view];
	UpdateMenuKeys(accel_reverse_map, hMenu, show_accels);
}

void CHotkeys::UpdateMenuKeys(accel_reverse_map_t const& accel_reverse_map, HMENU hMenu, bool show_accels) {
	int nItems = ::GetMenuItemCount(hMenu);

	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_SUBMENU;

	TCHAR buf[512];
	CString name;

	for (int i = 0; i < nItems; ++i) {
		::GetMenuItemInfo(hMenu, i, TRUE, &mi); // by position

		if (mi.hSubMenu) {
			UpdateMenuKeys(accel_reverse_map, mi.hSubMenu, show_accels);
		} else
		if (mi.wID != 0) {
			// see if there's accelerator info in text
			ATLASSERT(!(buf[0] = 0));
			::GetMenuString(hMenu, i, buf, array_size(buf), MF_BYPOSITION);
			ATLASSERT(buf[0]);

			int len = lstrlen(buf);
			int k = len;

			while (k--)
				if (_T('\t') == buf[k])
					break;

			bool has_tab = (k > 0);
			bool changed = true;

			// is there any accelerator for this command nowadays?
			accel_reverse_map_t::const_iterator j = accel_reverse_map.find((WORD)mi.wID);
			if (!show_accels || j == accel_reverse_map.end()) {
				if (has_tab)
					buf[k] = 0; // remove old one
				else
					changed = 0;
			} else {
				if (!has_tab) {
					k = len;
					buf[k] = _T('\t');
				}
				++k;

				ACCEL const& acc = *j->second; //*((*j).second);
				name = NameFromAccel(acc);
				ATLASSERT(k + name.GetLength() < (int)array_size(buf));
				lstrcpy(buf + k, name);
			}

			if (changed) {
				ATLASSERT(lstrlen(buf));
				::ModifyMenu(hMenu, i, MF_BYPOSITION, mi.wID, buf);
				// $TSEK no need to update item enable/icon states? (see wtl's command bar atlctrlw.h line 2630)
			}
		}
	}
}

void CHotkeys::AddMenuKeys(std::string const& view, HMENU hMenu) {
	accel_reverse_map_t const& accel_reverse_map = view_accel_reverse_maps[view];
	AddMenuKeys(accel_reverse_map, hMenu);
}

void CHotkeys::AddMenuKeys(accel_reverse_map_t const& accel_reverse_map, HMENU hMenu) {
	int nItems = ::GetMenuItemCount(hMenu);

	CMenuItemInfo mi;
	mi.fMask = MIIM_ID | MIIM_SUBMENU;

	TCHAR buf[512];
	std::stringstream name;

	for (int i = 0; i < nItems; ++i) {
		::GetMenuItemInfo(hMenu, i, TRUE, &mi);

		if (mi.hSubMenu) {
			AddMenuKeys(accel_reverse_map, mi.hSubMenu);
		} else
		if (mi.wID != 0) {
			accel_reverse_map_t::const_iterator j = accel_reverse_map.find((WORD)mi.wID);
			if (j != accel_reverse_map.end()) {
				::GetMenuString(hMenu, i, buf, array_size(buf), MF_BYPOSITION);

				ACCEL const& acc = *((*j).second);

				name.str("");
				name << buf << _T('\t') << NameFromAccel(acc);

				::ModifyMenu(hMenu, i, MF_BYPOSITION, mi.wID, name.str().c_str());
			}
		}
	}
}
