#pragma once

// ---------------------------------------------------------------------------------------------------------------
// BASE
// ---------------------------------------------------------------------------------------------------------------
inline BOOL CenterDialogRelative(HWND hWndDialog, HWND hWndCenter/* = NULL*/) {
	CWindow dlgWnd(hWndDialog);
	ATLASSERT(::IsWindow(dlgWnd));

	if (hWndCenter == NULL) {
		hWndCenter = ::GetParent(dlgWnd);
	}

	RECT rcDlg;
	::GetWindowRect(dlgWnd, &rcDlg);

	RECT rcCenter;
	::GetWindowRect(hWndCenter, &rcCenter);

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	return ::SetWindowPos(dlgWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

template <class X>
class CConnectionDialog 
:
	public CDialogImpl<X>,
	public CMessageFilter
{
  public:
	zzub_connection_t* connection;

	BEGIN_MSG_MAP_EX(X)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_ACTIVATE(OnActivate)
		CMD_ID_HANDLER(IDCLOSE, OnClose)
	END_MSG_MAP()

	CWindow parentWnd;

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam) {
		parentWnd = (HWND)lInitParam;
		CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->AddMessageFilter(this);
		X* pX = static_cast<X*>(this);
		pX->Init();
		CenterDialogRelative(m_hWnd, parentWnd);
		return TRUE;
	}

	void OnClose() {
		DestroyWindow();
	}

	void OnDestroy() {
		CModuleMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->RemoveMessageFilter(this);
		SetMsgHandled(FALSE);
	}

	virtual void OnFinalMessage(HWND /*hWnd*/) {
		X* pX = static_cast<X*>(this);
		pX->Uninit();
	}

	BOOL PreTranslateMessage(MSG* pMsg) {
		if (::IsWindow(m_hWnd) && IsChild(pMsg->hwnd)) {
			if (pMsg->message == WM_KEYDOWN) {
				if (pMsg->wParam == VK_RETURN) {
					parentWnd.SendMessage(WM_COMMAND, MAKEWPARAM(X::IDD, IDOK), (LPARAM)m_hWnd);
					parentWnd.SetFocus(); // ->WM_ACTIVATE->WA_INACTIVE->IDCLOSE
					return TRUE;
				} else
				if (pMsg->wParam == VK_ESCAPE) {
					parentWnd.SetFocus(); // ->WM_ACTIVATE->WA_INACTIVE->IDCLOSE
					return TRUE;				
				}
			}
			return IsDialogMessage(pMsg);
		} else
			return FALSE;
	}

	void OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther) {
		if (nState == WA_INACTIVE) {
			PostMessage(WM_COMMAND, MAKEWPARAM(IDCLOSE, 0), 0);
		}
		SetMsgHandled(FALSE);
	}
};

class CEventConnectionDialog : public CConnectionDialog<CEventConnectionDialog> {

	struct parameterindex {
		int group, track, column;
	};

	struct parameterbinding {
		int sourceparam, group, track, column;
	};

public:

	enum { IDD = IDD_CONNECTION_EVENT };

	std::vector<parameterindex> targetIndices;
	std::vector<parameterbinding> bindings;

	CStatic m_sourcePlugin;
	CListBox m_sourceParams;
	CStatic m_targetPlugin;
	CListBox m_targetParams;

	BEGIN_MSG_MAP_EX(CEventConnectionDialog)
		CMD_HANDLER(IDC_CONNECTION_SOURCE_PARAM, LBN_SELCHANGE, OnSourceParamSelChange)
		CMD_HANDLER(IDC_CONNECTION_TARGET_PARAM, LBN_SELCHANGE, OnTargetParamSelChange)
		CHAIN_MSG_MAP(CConnectionDialog<CEventConnectionDialog>)
	END_MSG_MAP()

	void Init() {
		m_sourcePlugin.Attach(GetDlgItem(IDC_EVENTCONNECTION_SOURCE_PLUGIN));
		m_targetPlugin.Attach(GetDlgItem(IDC_EVENTCONNECTION_TARGET_PLUGIN));
		m_targetParams.Attach(GetDlgItem(IDC_CONNECTION_TARGET_PARAM));
		m_sourceParams.Attach(GetDlgItem(IDC_CONNECTION_SOURCE_PARAM));
	}

	void Uninit() {
		m_sourcePlugin.m_hWnd = 0;
		m_sourceParams.m_hWnd = 0;
		m_targetPlugin.m_hWnd = 0;
		m_targetParams.m_hWnd = 0;
		targetIndices.clear();
		bindings.clear();
		connection = 0;
	}

	void AddTargetParameter(int group, int track, int column, std::string name) {
		m_targetParams.AddString(name.c_str());

		parameterindex pai = { group, track, column };
		targetIndices.push_back(pai);
	}

	int GetTargetParameterIndex(int group, int track, int column) {
		for (std::vector<parameterindex>::iterator i = targetIndices.begin(); i != targetIndices.end(); ++i) {
			if (i->group == group && i->track == track && i->column == column)
				return (int)distance(targetIndices.begin(), i);
		}
		return -1;
	}

	void AddBinding(int sourceparam, int group, int track, int column) {
		parameterbinding binding = { sourceparam, group, track, column };
		bindings.push_back(binding);
	}

	int GetBindingCount() {
		return (int)bindings.size();
	}

	void GetBinding(int i, int* sourceparam, int* group, int* track, int* column) {
		*sourceparam = bindings[i].sourceparam;
		*group = bindings[i].group;
		*track = bindings[i].track;
		*column = bindings[i].column;
	}

	void OnTargetParamSelChange() {
		int sourceparam = m_sourceParams.GetCurSel();
		if (sourceparam == -1) return ;

		// re-set all bindings on targetparams for selected sourceparam
		std::vector<parameterbinding> result;

		for (std::vector<parameterbinding>::iterator i = bindings.begin(); i != bindings.end(); ++i) {
			if (i->sourceparam != sourceparam) {
				result.push_back(*i);
			}
		}

		int selcount = m_targetParams.GetSelCount();
		int* selitems = new int[selcount];
		m_targetParams.GetSelItems(selcount, selitems);
		for (int i = 0; i < selcount; i++) {
			const parameterindex& pai = targetIndices[selitems[i]];
			parameterbinding binding = { sourceparam, pai.group, pai.track, pai.column  };
			result.push_back(binding);
		}
		delete[] selitems;

		bindings = result;
	}

	void OnSourceParamSelChange() {

		for (int i = 0; i < m_targetParams.GetCount(); i++) {
			m_targetParams.SetSel(i, FALSE);
		}

		for (std::vector<parameterbinding>::iterator i = bindings.begin(); i != bindings.end(); ++i) {
			if (i->sourceparam == m_sourceParams.GetCurSel()) {
				int targetIndex = GetTargetParameterIndex(i->group, i->track, i->column);
				m_targetParams.SetSel(targetIndex, TRUE);
			}
		}
	}
};

inline void GetListBoxRanges(CListBox& list, int* inrangeindex, int* longestrangeindex, bool* wrapping, std::vector<std::pair<int, int> >* ranges) {
	int cursel = list.GetCaretIndex();

	int firstsel = -1;
	int selcount = 0;
	int longestcount = 0;

	*inrangeindex = -1;
	*longestrangeindex = -1;
	for (int i = 0; i < list.GetCount(); i++) {
		if (list.GetSel(i) != 0) {
			if (firstsel == -1) {
				firstsel = i;
				selcount = 1;
			} else {
				selcount++;
			}
			if (cursel == i) *inrangeindex = (int)ranges->size();
		} else if (firstsel != -1) {
			if (selcount > longestcount) {
				*longestrangeindex = ranges->size();
				longestcount = selcount;
			}
			ranges->push_back(std::pair<int, int>(firstsel, selcount));
			firstsel = -1;
		}
	}
	if (firstsel != -1) {
		if (selcount > longestcount) *longestrangeindex = ranges->size();
		*wrapping = !ranges->empty() && (*ranges)[0].first == 0;
		ranges->push_back(std::pair<int, int>(firstsel, selcount));
	}

}

class CAudioConnectionDialog : public CConnectionDialog<CAudioConnectionDialog>
{
  public:

	enum { IDD = IDD_CONNECTION_AUDIO };

	CStatic m_sourcePlugin;
	CListBox m_sourceChannels;
	CStatic m_targetPlugin;
	CListBox m_targetChannels;
	bool m_ownerSelChange;

	BEGIN_MSG_MAP_EX(CAudioConnectionDialog)
		CMD_HANDLER(IDC_AUDIO_SOURCE_CHANNEL, LBN_SELCHANGE, OnSourceChannelSelChange)
		CMD_HANDLER(IDC_AUDIO_TARGET_CHANNEL, LBN_SELCHANGE, OnTargetChannelSelChange)
		CHAIN_MSG_MAP(CConnectionDialog<CAudioConnectionDialog>)
	END_MSG_MAP()

	void Init() {
		m_sourcePlugin.Attach(GetDlgItem(IDC_AUDIO_SOURCE_PLUGIN));
		m_sourceChannels.Attach(GetDlgItem(IDC_AUDIO_SOURCE_CHANNEL));
		m_targetPlugin.Attach(GetDlgItem(IDC_AUDIO_TARGET_PLUGIN));
		m_targetChannels.Attach(GetDlgItem(IDC_AUDIO_TARGET_CHANNEL));
		m_ownerSelChange = false;
	}

	void Uninit() {
		m_sourcePlugin.m_hWnd = 0;
		m_sourceChannels.m_hWnd = 0;
		m_targetPlugin.m_hWnd = 0;
		m_targetChannels.m_hWnd = 0;
	}

	void GetChannelRange(const std::vector<std::pair<int, int> >& ranges, int inrange, int longestrange, bool wrapping, int maxrange, int* first_channel, int* channel_count) {
		if (ranges.empty()) {
			*first_channel = 0;
			*channel_count = 0;
		} else if (ranges.size() == 1) {
			*first_channel = ranges[0].first;
			*channel_count = ranges[0].second;
		} else {
			int lastrangeindex = ranges.size() - 1;
			int wraplen = wrapping ? ranges[0].second + ranges[lastrangeindex].second : 0;

			int resultrange;
			if (wrapping && inrange == 0)
				resultrange = lastrangeindex;
			else if (inrange != -1)
				resultrange = inrange;
			else if (wrapping && wraplen > longestrange)
				resultrange = lastrangeindex;
			else 
				resultrange = longestrange;

			*first_channel = ranges[resultrange].first;
			*channel_count = ranges[resultrange].second;
			if (wrapping)
				*channel_count += ranges[0].second;
		}
	}

	void GetChannelRange(CListBox& list, int* first_channel, int* channel_count) {
		std::vector<std::pair<int, int> > ranges;
		int inrangeindex = -1;
		int longestrangeindex = -1;
		bool wrapping = false;
		GetListBoxRanges(list, &inrangeindex, &longestrangeindex, &wrapping, &ranges);

		//int first_channel = 0;
		//int channel_count = 0;
		int maxcount = list.GetCount();
		GetChannelRange(ranges, inrangeindex, longestrangeindex, wrapping, maxcount, first_channel, channel_count);

	}

	void ChannelSelChange(CListBox& list) {
		if (m_ownerSelChange) return ;
/*
		std::vector<std::pair<int, int> > ranges;
		int inrangeindex = -1;
		int longestrangeindex = -1;
		bool wrapping = false;
		GetListBoxRanges(list, &inrangeindex, &longestrangeindex, &wrapping, &ranges);

		int first_channel = 0;
		int channel_count = 0;
		int maxcount = list.GetCount();
		GetChannelRange(ranges, inrangeindex, longestrangeindex, wrapping, maxcount, &first_channel, &channel_count);
*/
		int first_channel = 0;
		int channel_count = 0;
		GetChannelRange(list, &first_channel, &channel_count);

		m_ownerSelChange = true;
		SetChannelRange(list, first_channel, channel_count);
		m_ownerSelChange = false;
	}

	void OnSourceChannelSelChange() {
		ChannelSelChange(m_sourceChannels);
	}

	void OnTargetChannelSelChange() {
		ChannelSelChange(m_targetChannels);
	}

	void AddSourceChannel(std::string name) {
		m_sourceChannels.AddString(name.c_str());
	}

	void AddTargetChannel(std::string name) {
		m_targetChannels.AddString(name.c_str());
	}

	void SetChannelRange(int first_input_channel, int input_count, int first_output_channel, int output_count) {
		// TODO: wrapping
		//m_targetChannels.SelItemRange(TRUE, first_input_channel, first_input_channel + input_count - 1);

		SetChannelRange(m_targetChannels, first_input_channel, input_count);
		SetChannelRange(m_sourceChannels, first_output_channel, output_count);
	}

	void SetChannelRange(CListBox& list, int first_channel, int channel_count) {
		int cursel = list.GetCaretIndex();
		if (list.GetCount() > 1)
			list.SelItemRange(FALSE, 0, list.GetCount() - 1);
		else if (list.GetCount() == 1)
			list.SetSel(0, FALSE);

		if (first_channel + channel_count > list.GetCount()) {
			// select wrapping channels
			int begin_len = first_channel + channel_count - list.GetCount();
			int last_len = list.GetCount() - first_channel;

			if (begin_len > 1)
				list.SelItemRange(TRUE, 0, begin_len - 1);
			else if (begin_len == 1)
				list.SetSel(0, TRUE);

			if (last_len > 1)
				list.SelItemRange(TRUE, first_channel, first_channel + last_len - 1);
			else if (last_len == 1)
				list.SetSel(first_channel, TRUE);
		} else {
			// select channel range
			if (channel_count > 1)
				list.SelItemRange(TRUE, first_channel, first_channel + channel_count - 1);
			else if (channel_count == 1)
				list.SetSel(first_channel, TRUE);
		}

		list.SetCaretIndex(cursel);
	}

};
