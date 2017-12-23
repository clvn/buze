#pragma once

const unsigned int WEN_SELECTIONCHANGED = 1;
const unsigned int WEN_LOOPCHANGED = 2;
const unsigned int WEN_LOOPTRACK = 3;
const unsigned int WEN_MOUSEMOVE = 4;
const unsigned int WEN_ZOOMCHANGED = 5;

enum WaveEditorDragMode {
	WaveEditorSelect,
	WaveEditorZoom
};

// MUST-HAVE wave tools: Mix with clipboard

enum MouseMoveWhat {
	MoveNothing,
	MoveLoopStart,
	MoveLoopEnd,
	MoveSelection,
	MovePosition,
};

enum WaveEditorGridMode {
	WaveEditorSamples,
	WaveEditorTicks,
	WaveEditorSeconds,
	WaveEditorWord,
};

#if !defined(__ZZUBTYPES_H) 
enum zzub_wave_buffer_type {
    zzub_wave_buffer_type_si16	= 0,    // signed int 16bit
    zzub_wave_buffer_type_f32	= 1,    // float 32bit
    zzub_wave_buffer_type_si32	= 2,    // signed int 32bit
    zzub_wave_buffer_type_si24	= 3,    // signed int 24bit
};

enum wave_buffer_type {
	wave_buffer_type_si16	= zzub_wave_buffer_type_si16,    // signed int 16bit
	wave_buffer_type_f32	= zzub_wave_buffer_type_f32,    // float 32bit
	wave_buffer_type_si32	= zzub_wave_buffer_type_si32,    // signed int 32bit
	wave_buffer_type_si24	= zzub_wave_buffer_type_si24,    // signed int 24bit
};
#endif

class CWaveEditorWaveProvider {
public:
	virtual void GetSamplesDigest(int channel, int start, int end, std::vector<float>& mindigest, std::vector<float>& maxdigest, std::vector<float>& ampdigest, int digestsize) = 0;
};

struct EDITWAVEINFO {
	zzub_wave_buffer_type type;
	long samples;
	long beginLoop, endLoop;
	bool looping;
	float samplesPerSec;
	float samplesPerTick;
	bool stereo;
	CWaveEditorWaveProvider* waveProvider;
	std::vector<int> slices;
};

class CWaveEditorCtrl : public CWindowImpl<CWaveEditorCtrl> {
public:
	CPen slicePen;
	CPen selectedPen;
	CPen gridPen;
	CPen wavePen;
	CPen loopPen;
	CPen loopDisabledPen;
	CPen cursorPen;
	CPen timelineTickPen;
	CPen timelineBorderPen;
	CFont timelineFont;
	MouseMoveWhat moveType;
	WaveEditorGridMode gridType;

	double paintDelta;	// was originally a float, but didnt work too well on small scales

	double beginDisplaySample;
	long numDisplaySamples;
	long beginSelectSample, endSelectSample;
	long firstSelectSample, lastSelectSample;
	long mouseSamplePosition;
	long cursorSamplePosition;

	std::vector<float> mindigestL;
	std::vector<float> maxdigestL;
	std::vector<float> ampdigestL;

	std::vector<float> mindigestR;
	std::vector<float> maxdigestR;
	std::vector<float> ampdigestR;
	EDITWAVEINFO waveInfo;

	DECLARE_WND_CLASS("WaveEditorCtrl")

	BEGIN_MSG_MAP(CWaveEditorCtrl)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)

		COMMAND_ID_HANDLER(ID_WAVE_SET_LOOPPOINTS_FROM_SELECTION, OnSetLooppointsFromSelection)
		COMMAND_ID_HANDLER(ID_VIEW_PROPERTIES, OnViewProperties)

		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	CWaveEditorCtrl(void);
	~CWaveEditorCtrl(void);

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetLooppointsFromSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/); 
	LRESULT OnViewProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void SortSelection();
	void SetZoomDisplay(double beginSample, long endSample, bool notify = true);
	void SetLoopPoints(long beginLoop, long endLoop);
	void ClearSelection();
	void SetSelection(long beginSel, long endSel);
	void SetEditWave(EDITWAVEINFO* pewi, bool reset);
	bool GetLoopHandle(int type, RECT* rc);
	bool GetSliceHandle(int index, RECT* rc);
	bool GetSliceRect(int index, RECT* rc);
	void GetSliceOffsets(int index, int* sliceStart, int* sliceEnd);
	void UpdateAlign();
	void UpdateDigest();
	void SetCursorPosition(long position);

	float GetSamplesPerUnit(WaveEditorGridMode mode);
	float GetDisplaySamplesPerUnit(float samplesPerUnit);
	std::string FormatSampleUnit(float units, WaveEditorGridMode mode);

	void ZoomShowSelection();
	void ZoomShowAll();
	void ZoomIn();
	void ZoomOut();

	void UpdateScrollbars();
	void PaintSampleChannel(CDC& dc, RECT rc, int channel);
	void PaintTimeline(CDC& dc, RECT rc);

};
