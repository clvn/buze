#pragma once

// this is a combined trackbar/analyzer to change master volume
// could be made to work both horizontal and vertical, and then
// create a builtin master mixer-board to set vol and pan

#define VU_COLOR_LOW	0x00ff00	/* Green */
#define VU_COLOR_12DB	0x99CCFF	/* Yellow */
#define VU_COLOR_6DB	0x0000FF	/* Red */
#define VU_COLOR_BLANK	0x000000	/* Black */

#define VU_DB_LIMIT 36
#define VU_PEAK_DROPRATE 0.002f

const int VISUAL_BUFFER_SIZE = 1024;

// visual volume slider styles
const int VVSS_VSCROLL=1;
const int VVSS_HSCROLL=0;

// sent to parent via WM_NOTIFY
#define NM_SETVALUE			NM_FIRST - 1
#define NM_SETVALUEPREVIEW	NM_FIRST - 2

class CVisualVolumeSlider
	: public CWindowImpl<CVisualVolumeSlider>
{
public:
	enum drag_mode {
		drag_mode_none,
		drag_mode_move
	};
	drag_mode mode;
	int group, track, column;
	bool is_master;

	float maxR, maxL;
	float rPeak, lPeak;
	float rTop, lTop;
	float rDrop, lDrop;
	COLORREF lPeakColor;
	COLORREF rPeakColor;
	float dropRate;
	int timerMod;
	float percent12dB;
	float percent6dB;
	int direction;	// SB_VERT or SB_HORZ

	int counter;
	int min_value, max_value;
	int current_value;
	int drag_value;

	DECLARE_WND_CLASS("CVisualVolumeSlider")

	BEGIN_MSG_MAP(CVisualVolumeSlider)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
	END_MSG_MAP()

	CVisualVolumeSlider();
	~CVisualVolumeSlider(void);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);

	void SetValue(int value);
	void SetPeak(float maxL, float maxR);
	void SetDropRate(float rate);
	void SetTimerRate(int rate);
	void SetMinMax(int _min, int _max);
	void DrawItem(CDC& dc, RECT& rc, float* curPeak, float* curTop, float* curDrop, float* curMax, COLORREF* curPeakColor);
	void GetHandleRect(RECT* rc);
};
