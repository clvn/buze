#pragma once
/*

Super-simple search-as-you-type combobox.

Requires REFLECT_NOTIFICATIONS_EX() in the parents message map.

The combobox sends SBN_OK and SBN_CANCEL commands to the parent upon Enter or Esc:
	COMMAND_HANDLER(IDC_SEARCHBOX, SBN_OK, OnOK)
	COMMAND_HANDLER(IDC_SEARCHBOX, SBN_CANCEL, OnCancel)

Call Create() with styles CBS_SIMPLE | CBS_AUTOHSCROLL.

Call AddString() and/or AddMenu() to register names and associated ids.

Call IndexNames() when done adding strings to create the internal search index.

Example usages:

OnChar:
	if ((wParam >= 'a' && wParam <= 'z') || (wParam >= 'A' && wParam <= 'Z')) {
		TCHAR key[2] = { wParam, 0 };
		searchbox.Show(typepoint, key);
	}

OnLButtonUp:
	POINT pt = { (signed short)LOWORD(lParam), (signed short)HIWORD(lParam) };
	typepoint = pt;
	if (searchbox == GetFocus()) {
		searchbox.Hide();
		SetFocus();
	}

OnOK:
	id = searchbox.GetSelectedID();

*/

#define SBN_OK		128
#define SBN_CANCEL	129

struct SEARCHNAME {
	std::tstring label;
	std::tstring text;
	int id;
	std::vector<std::tstring> keywords;

	SEARCHNAME(std::tstring _label = _T(""), std::tstring _text = _T(""), int _id = -1) {
		label = _label;
		text = _text;
		id = _id;
	}
};

class CSearchBox;

typedef void SearchBoxMenuItemCallback(CSearchBox& self, MENUITEMINFO& mii, void* userdata);

class CSearchBox
	: public CWindowImpl<CSearchBox, CComboBox>
{
public:
	CWindow targetWnd;
	std::vector<SEARCHNAME> names;
	SEARCHNAME* selectedname;

	// subclass the search combo's edit control to forward WM_KEYDOWN on wine
	CContainedWindowT<CEdit> searchedit;

	DECLARE_WND_SUPERCLASS(_T("SearchBox"), CComboBox::GetWndClassName())

	BEGIN_MSG_MAP(CSearchBox)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)

		REFLECTED_COMMAND_CODE_HANDLER(CBN_EDITCHANGE, OnEditUpdate)
		REFLECTED_COMMAND_CODE_HANDLER(CBN_DBLCLK, OnDblClick)
	ALT_MSG_MAP(1)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	END_MSG_MAP()

	CSearchBox();
	~CSearchBox(void);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnEditUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled);
	LRESULT OnDblClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled);

	void Show(HWND hMessageTargetWnd, POINT pt, LPCTSTR text);
	void Hide();

	void AddMenu(HMENU _menu, SearchBoxMenuItemCallback* callback, void* userdata);
	void AddString(LPCTSTR name, int id);
	void AddString(LPCTSTR name, LPCTSTR text, int id);
	int GetSelectedID();
	void IndexNames();
	HWND GetEditCtrl();

private:
	void BindList(LPCTSTR keywords);
	void SearchToken(std::tstring token, std::vector<SEARCHNAME*>& result, int index);
	bool SearchTokens(std::tstring token, std::vector<std::tstring>& keywords);
};
