#include <algorithm>
using std::min;
using std::max;

#define NOMINMAX
#include <atlbase.h>
#include <atlwin.h>
#include <wtl/atlapp.h>
#include <wtl/atlctrls.h>
#include <vector>
#include <string>
#include <cassert>
#include "utils.h"
#include "SearchBox.h"

// http://support.microsoft.com/kb/174667 - how to subclass CListBox / CEdit
/*
namespace {

// taken from comment on http://www.codeproject.com/string/stringsplit.asp?df=100&forumid=2167&exp=0&select=1062827#xx1062827xx
template<typename _Cont>
void split(const std::tstring& str, _Cont& _container, const std::tstring& delim = _T(",")) {
    std::tstring::size_type lpos = 0;
    std::tstring::size_type pos = str.find_first_of(delim, lpos);
    while(lpos != std::tstring::npos) {
        _container.insert(_container.end(), str.substr(lpos,pos - lpos));

        lpos = ( pos == std::tstring::npos ) ? std::tstring::npos : pos + 1;
        pos = str.find_first_of(delim, lpos);
    }
}

// found the trims in one of the comments at http://www.codeproject.com/vcpp/stl/stdstringtrim.asp

std::tstring& trimleft( std::tstring& s )
{
   std::tstring::iterator it;

   for( it = s.begin(); it != s.end(); it++ )
      if( !isspace((unsigned char) *it ) )
         break;

   s.erase( s.begin(), it );
   return s;
}

std::tstring& trimright( std::tstring& s )
{
   std::tstring::difference_type dt;
   std::tstring::reverse_iterator it;

   for( it = s.rbegin(); it != s.rend(); it++ )
      if( !isspace((unsigned char) *it ) )
         break;

   dt = s.rend() - it;

   s.erase( s.begin() + dt, s.end() );
   return s;
}

std::tstring& trim( std::tstring& s )
{
   trimleft( s );
   trimright( s );
   return s;
}

std::tstring trim( const std::tstring& s )
{
   std::tstring t = s;
   return trim( t );
}

} // namespace

*/
CSearchBox::CSearchBox() : searchedit(this, 1) {
	selectedname = 0;
}

CSearchBox::~CSearchBox(void) {
}

LRESULT CSearchBox::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	LRESULT lRes = DefWindowProc();
	SetFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	searchedit.SubclassWindow(GetEditCtrl());

	IndexNames();
	return lRes;
}

LRESULT CSearchBox::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int sel;
	switch (wParam) {
		case VK_ESCAPE:
			selectedname = 0;
			GetParent().SetFocus();
			Hide();
			//GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), SBN_CANCEL), (LPARAM)m_hWnd); 
			return 0;
		case VK_RETURN:
			sel = GetCurSel();
			if (sel == -1) {
				selectedname = 0;
			} else {
				selectedname = (SEARCHNAME*)GetItemData(sel);
			}
			GetParent().SetFocus();
			Hide();
			// send ok-message to parent, she's supposed to call GetSelectedID
			targetWnd.SendMessage(WM_COMMAND, MAKEWPARAM(GetSelectedID(), 0), (LPARAM)m_hWnd); 
			//GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetSelectedID(), 0), (LPARAM)m_hWnd); 
			//GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), SBN_OK), (LPARAM)m_hWnd); 
			return 0;
	}
	return searchedit.DefWindowProc(); // handling WM_KEYDOWN of the subclassed edit control
}

LRESULT CSearchBox::OnEditUpdate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	TCHAR keywords[1024];
	GetWindowText(keywords, 1024);
	BindList(keywords);
	Invalidate();
	UpdateWindow();
	return 0;
}

LRESULT CSearchBox::OnDblClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled) {
	int sel = GetCurSel();
	if (sel == -1) {
		selectedname = 0;
	} else {
		selectedname = (SEARCHNAME*)GetItemData(sel);
	}
	GetParent().SetFocus();
	Hide();
	// send ok-message to parent, she's supposed to call GetSelectedID
	//GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetSelectedID(), 0), (LPARAM)m_hWnd); 
	targetWnd.SendMessage(WM_COMMAND, MAKEWPARAM(GetSelectedID(), 0), (LPARAM)m_hWnd); 
	return 0;
}

int CSearchBox::GetSelectedID() {
	if (selectedname == 0) return -1;
	return selectedname->id;
}

HWND CSearchBox::GetEditCtrl() {
	HWND hEditWnd = FindWindowEx(m_hWnd, 0, CEdit::GetWndClassName(), 0);
	assert(hEditWnd);
	return hEditWnd;
}

void CSearchBox::AddMenu(HMENU _menu, SearchBoxMenuItemCallback* callback, void* userdata) {
	std::vector<TCHAR> buffer(1024);
	CMenuHandle menu = _menu;
	int count = menu.GetMenuItemCount();
	for (int i = 0; i < count; i++) {
		MENUITEMINFO mii;
		ZeroMemory(&mii, sizeof(MENUITEMINFO));
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.cch = 1024;
		mii.dwTypeData = (LPTSTR)&buffer.front();
		mii.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_SUBMENU | MIIM_ID | MIIM_DATA; 

		menu.GetMenuItemInfo(i, TRUE, &mii);
		if (mii.hSubMenu) {
			AddMenu(mii.hSubMenu, callback, userdata);
		} else
		if (mii.fType == MFT_STRING || mii.fType == MFT_MENUBARBREAK) {
			// MFT_MENUBARBREAK = manual pagebreak at maxEntriesPerPage
			if (callback) {
				callback(*this, mii, userdata);
			} else {
				std::tstring name(buffer.begin(), buffer.begin() + mii.cch);
				AddString(name.c_str(), mii.wID);
			}
		}
	}
}

void CSearchBox::AddString(LPCTSTR name, int id) {
	names.push_back(SEARCHNAME(name, "", id));
}

void CSearchBox::AddString(LPCTSTR name, LPCTSTR text, int id) {
	names.push_back(SEARCHNAME(name, text, id));
}

void CSearchBox::IndexNames() {
	for (std::vector<SEARCHNAME>::iterator i = names.begin(); i != names.end(); ++i) {
		i->keywords.clear();
		{
			std::tstring keylabel = i->label;
			std::transform(keylabel.begin(), keylabel.end(), keylabel.begin(), tolower);
			split<std::vector<std::tstring> > (keylabel, i->keywords, _T(" "));
		}
		{
			std::tstring keylabel = i->text;
			std::transform(keylabel.begin(), keylabel.end(), keylabel.begin(), tolower);
			split<std::vector<std::tstring> > (keylabel, i->keywords, _T(" "));
		}
	}
}

void CSearchBox::Show(HWND hMessageTargetWnd, POINT typepoint, LPCTSTR text) {
	targetWnd = hMessageTargetWnd;
	MoveWindow(typepoint.x, typepoint.y, 150, 300);
	ShowWindow(SW_SHOW);
	SetFocus();
	BindList(text);
}

void CSearchBox::Hide() {
	ShowWindow(SW_HIDE);
}

void CSearchBox::BindList(LPCTSTR _keywords) {
	std::tstring keywords = _keywords;

	std::transform(keywords.begin(), keywords.end(), keywords.begin(), tolower);
	keywords = trim(keywords);

	std::vector<std::tstring> tokens;
	split<std::vector<std::tstring> > (keywords, tokens, _T(" "));

	SetRedraw(FALSE);
	ResetContent();

	std::vector<SEARCHNAME*> searchnames;
	for (std::vector<std::tstring>::iterator j = tokens.begin(); j != tokens.end(); ++j) {
		if (*j == _T("")) continue;
		SearchToken(*j, searchnames, (int)(j - tokens.begin()));
	}

	for (std::vector<SEARCHNAME*>::iterator k = searchnames.begin(); k != searchnames.end(); ++k) {
		int index = InsertString(-1, (*k)->label.c_str());
		SetItemData(index, (DWORD_PTR)*k);
	}
	SetRedraw(TRUE);

	SetWindowText(_keywords);
	int len = (int)_tcslen(_keywords);
	SetEditSel(len, len);

}

bool CSearchBox::SearchTokens(std::tstring token, std::vector<std::tstring>& keywords) {
	for (std::vector<std::tstring>::iterator j = keywords.begin(); j != keywords.end(); ++j) {
		if (*j == _T("")) continue;
		if (j->substr(0, std::min(j->length(), token.length())) == token) {
			return true;
		}
	}
	return false;
}

void CSearchBox::SearchToken(std::tstring token, std::vector<SEARCHNAME*>& result, int index) {
	if (index == 0) {
		// first token, search in all names
		for (std::vector<SEARCHNAME>::iterator i = names.begin(); i != names.end(); ++i) {
			if (SearchTokens(token, i->keywords))
				if (find(result.begin(), result.end(), &*i) == result.end()) {
					result.push_back(&*i);
				}
		}
	} else {
		// second token and beyond, search in results from previous tokens
		std::vector<SEARCHNAME*> temp;
		temp.swap(result);
		for (std::vector<SEARCHNAME*>::iterator i = temp.begin(); i != temp.end(); ++i) {
			if (SearchTokens(token, (*i)->keywords)) {
				if (find(result.begin(), result.end(), *i) == result.end()) {
					result.push_back(*i);
				}
			}
		}
	}
}
