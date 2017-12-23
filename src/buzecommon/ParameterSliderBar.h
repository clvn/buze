#pragma once

const int SM_CHANGE = 1001; // tell parent about parameter change

struct VALUENMHDR : NMHDR {
	int value;
};

class CParameterSliderBar : public CWindowImpl<CParameterSliderBar> {
public:
	enum {
		sliderHeight = 16,
		labelWidth = 100,
		valueWidth = 80,
	};

	std::string m_name;
	std::string m_valueName;
	POINT m_ptDragStart;
	RECT m_rcDragStart;
	int m_dragStartValue;
	bool m_dragging;
	bool m_selected;
	bool m_shaded;
	int m_value;
	int m_minValue, m_maxValue;

	int m_pluginid, m_group, m_track, m_column; // user-data

	DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW|CS_VREDRAW|CS_PARENTDC, NULL)

	BEGIN_MSG_MAP(CParameterSliderBar)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	END_MSG_MAP()

	CParameterSliderBar(void);
	~CParameterSliderBar(void);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	int GetValue();
	void SetValue(int value, const std::string& valueName);
	void NotifyChange(int value);
	void SetName(const std::string& name);
	void SetMinMax(int minval, int maxval);
	void SetUserData(int pluginid, int group, int track, int column);
	int GetMin();
	int GetMax();

	void SetSelected(bool state);
	int GetKnobWidth();

	bool GetKnobRect(RECT* rc);
	void GetLabelRect(RECT* rc);
	void GetSliderRect(RECT* rc);
	void GetValueRect(RECT* rc);
};
