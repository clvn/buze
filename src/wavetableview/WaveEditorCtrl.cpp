#include "stdafx.h"
#include "resource.h"
#include "utils.h"
#include "WaveEditorCtrl.h"

template <typename T>
inline bool compareWithEpsilon(const T& v1, const T& v2, const T &epsilon) {
	const T diff = v1 - v2;
	return diff < epsilon && diff > -epsilon;
}

void ClearWaveInfo(EDITWAVEINFO* pewi) {
	pewi->beginLoop = 0;
	pewi->endLoop = 0;
	pewi->looping = false;
	pewi->samples = 0;
	pewi->samplesPerSec = 0;
	pewi->samplesPerTick = 0;
	pewi->stereo = false;
	pewi->type = zzub_wave_buffer_type_si16;
	pewi->slices.clear();
	pewi->waveProvider = 0;
}

//
// Construction / Destruction
//

CWaveEditorCtrl::CWaveEditorCtrl() {
	paintDelta = 0.0f;
	beginSelectSample = -1;
	endSelectSample = 0;
	firstSelectSample = -1;
	lastSelectSample = 0;
	beginDisplaySample = -1;
	numDisplaySamples = 0;
	mouseSamplePosition = 0;
	cursorSamplePosition = 0;

	moveType = MoveNothing;
	gridType = WaveEditorSamples;

	ClearWaveInfo(&waveInfo);
	//memset(&waveInfo, 0, sizeof(EDITWAVEINFO));
}

CWaveEditorCtrl::~CWaveEditorCtrl(void) {
}

LRESULT CWaveEditorCtrl::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	wavePen.CreatePen(PS_SOLID, 1, 0xffffff);
	gridPen.CreatePen(PS_SOLID, 1, 0x888888);
	selectedPen.CreatePen(PS_SOLID, 1, 0xff0000);
	slicePen.CreatePen(PS_SOLID, 1, 0x0000ff);
	loopPen.CreatePen(PS_SOLID, 1, 0x00FF00);
	loopDisabledPen.CreatePen(PS_SOLID, 1, 0x008000);
	cursorPen.CreatePen(PS_SOLID, 1, 0xFF00FF);
	timelineTickPen.CreatePen(PS_SOLID, 1, (COLORREF)0x0);
	timelineBorderPen.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
	timelineFont.CreateFont(11, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, "");

	SetZoomDisplay(0,0, false);
	return 0;
}

LRESULT CWaveEditorCtrl::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}

//
// Sizing / focus
//

LRESULT CWaveEditorCtrl::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	UpdateDigest();
	UpdateScrollbars();
	return 0;
}

LRESULT CWaveEditorCtrl::OnFocus( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 0;
}

//
// Painting
///

LRESULT CWaveEditorCtrl::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	CPaintDC screenDC(m_hWnd);
	CMemDC dc(screenDC);

	RECT rc;
	GetClientRect(&rc);

	// paint sample channels
	int width = rc.right-rc.left;
	int height = (rc.bottom-rc.top) - 20; // reserve 20px for timeline

	int fullHeight = height;

	if (waveInfo.stereo) {
		height = (height - 4) / 2;
		RECT rcL = { 0, 0, width, height };
		RECT rcR = { 0, height + 4, width, height * 2 + 4 };
		PaintSampleChannel(dc, rcL, 0);
		PaintSampleChannel(dc, rcR, 1);
		// paint splitter
		RECT rcSplit = { 0, height, width, height + 4};
		dc.FillSolidRect(&rcSplit, GetSysColor(COLOR_3DFACE));
	} else {
		RECT rcL = { 0, 0, width, height };
		PaintSampleChannel(dc, rcL, 0);
	}


	// paint timeline
	CRect tlrc(0, fullHeight+1, width, fullHeight+20);
	PaintTimeline(dc, tlrc);

	// if looping points are set, draw them with protracker-type handles
	int beginLoopX = (float)(waveInfo.beginLoop - beginDisplaySample) / paintDelta;
	int endLoopX = (float)(waveInfo.endLoop - beginDisplaySample) / paintDelta - 1;

	if (waveInfo.looping)
		dc.SelectPen(loopPen);
	else
		dc.SelectPen(loopDisabledPen);

	
	dc.MoveTo(beginLoopX, fullHeight);
	dc.LineTo(beginLoopX, 0);
	dc.LineTo(beginLoopX+8, 0);
	dc.LineTo(beginLoopX+8, 8);
	dc.LineTo(beginLoopX, 8);

	dc.MoveTo(endLoopX, fullHeight);
	dc.LineTo(endLoopX, 0);
	dc.LineTo(endLoopX-8, 0);
	dc.LineTo(endLoopX-8, 8);
	dc.LineTo(endLoopX, 8);

	int cursorX = (float)(cursorSamplePosition - beginDisplaySample) / paintDelta;

	dc.SelectPen(cursorPen);

	dc.MoveTo(cursorX, 0);
	dc.LineTo(cursorX, fullHeight);

	return DefWindowProc();
}

float CWaveEditorCtrl::GetDisplaySamplesPerUnit(float samplesPerUnit) {
	RECT rc;
	GetClientRect(&rc);
 	return (float)numDisplaySamples / samplesPerUnit / (float)rc.right;
}

void CWaveEditorCtrl::PaintSampleChannel(CDC& dc, RECT rc, int channel) {

	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	int waveChannels = waveInfo.stereo?2:1;

	double sample = beginDisplaySample;
	double selectDelta = paintDelta>1?paintDelta:0;
	std::vector<float>& mindigest = channel==0?mindigestL:mindigestR;
	std::vector<float>& maxdigest = channel==0?maxdigestL:maxdigestR;
	int last_height = -1;

	dc.FillSolidRect(&rc, 0);
	int left = (beginSelectSample - beginDisplaySample) / paintDelta;
	int right = ((endSelectSample + 1) - beginDisplaySample) / paintDelta;
	dc.FillSolidRect(left, rc.top, right - left, rc.bottom, 0xFFFFFF);

	CPenHandle oldpen = dc.SelectPen(gridPen);
	int halfheight = height / 2;
	dc.MoveTo(0, rc.top + halfheight);
	dc.LineTo(rc.right, rc.top + halfheight);

	oldpen = dc.SelectPen(selectedPen);
	for (int i = 0; i < width; i++) {
		if (i >= mindigest.size()) break;

		int sampleOfs = (int)sample;

		float out_min, out_max;
		out_min = mindigest[i];
		out_max = maxdigest[i];

		out_min *= (float)height/2;
		out_max *= (float)height/2;

		if (sampleOfs >= beginSelectSample-selectDelta && sampleOfs <= endSelectSample-selectDelta)
			dc.SelectPen(selectedPen); else
			dc.SelectPen(wavePen);

		if(last_height < rc.top + out_min + halfheight - 1 || last_height > rc.top + out_min + halfheight + 1) {
			dc.MoveTo(i, last_height - 1);
			dc.LineTo(i, rc.top + out_min + halfheight);
		}

		last_height = rc.top + out_max + halfheight + 1;
		dc.MoveTo(i, rc.top + out_min + halfheight);
		dc.LineTo(i, last_height);

		sample += paintDelta;
	}

	dc.SelectPen(slicePen);
	for (int i = 0; i < waveInfo.slices.size(); i++) {
		int sliceOfs = waveInfo.slices[i];
		if (sliceOfs < beginDisplaySample) continue;
		if (sliceOfs > beginDisplaySample + numDisplaySamples) break;

		int sliceX = (sliceOfs - beginDisplaySample) / paintDelta;
		dc.MoveTo(sliceX, rc.top);
		dc.LineTo(sliceX, rc.bottom);
	}

	dc.SelectPen(oldpen);

}

void CWaveEditorCtrl::PaintTimeline(CDC& dc, RECT rc) {

	int width = rc.right - rc.left;
	int fullHeight = rc.top;

	dc.FillRect(&rc, COLOR_BTNFACE);
	
	CPenHandle oldpen = dc.SelectPen(timelineBorderPen);
	dc.MoveTo(0, rc.top);
	dc.LineTo(rc.right, rc.top);
//	dc.Draw3dRect(&rc, GetSysColor(COLOR_BTNHIGHLIGHT), GetSysColor(COLOR_BTNSHADOW));//0, height, width, height+20);

	float samplesPerUnit = GetSamplesPerUnit(gridType);
	float unit = GetDisplaySamplesPerUnit(samplesPerUnit);
	float firstTickValue = beginDisplaySample / samplesPerUnit;

	// assume min 30px per major tick
	float majorScale = unit*30.0f;
	
	// round majorunit up to nearest 10, 100, 1000, 10000 etc if scale is in samples, or 1,2,4,8,16,etc for ticks or, ms,sec,min for time
	if (majorScale < 10)
		majorScale = ceil(majorScale); else
	if (majorScale < 100)
		majorScale = ceil(majorScale/10)*10; else
	if (majorScale < 1000)
		majorScale = ceil(majorScale/100)*100; else
	if (majorScale < 10000)
		majorScale = ceil(majorScale/1000)*1000; else
	if (majorScale < 100000)
		majorScale = ceil(majorScale/10000)*10000; else
		majorScale = ceil(majorScale/100000)*100000;

	float roundAdjust = fmod(firstTickValue, majorScale);

	float firstTick = firstTickValue - roundAdjust;

	float tickX = -roundAdjust/unit;
	dc.SelectPen(timelineTickPen);
	UINT oldAlign = dc.SetTextAlign(TA_CENTER);

	CFontHandle oldfont = dc.SelectFont(timelineFont);
	int oldBkMode = dc.SetBkMode(TRANSPARENT);
	while (tickX < width) {
		dc.MoveTo(tickX, fullHeight+2);
		dc.LineTo(tickX, fullHeight+8);

		std::string marker_str = FormatSampleUnit(firstTick, gridType);
		dc.TextOut(tickX, fullHeight+8, marker_str.c_str(), marker_str.length());
		tickX += majorScale / unit;
		firstTick += majorScale;
	}

	dc.SetBkMode(oldBkMode);
	dc.SetTextAlign(oldAlign);
	dc.SelectPen(oldpen);
	dc.SelectFont(oldfont);
}

LRESULT CWaveEditorCtrl::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 1;
}

std::string CWaveEditorCtrl::FormatSampleUnit(float units, WaveEditorGridMode mode) {
	std::stringstream marker_strm;
	if (mode == WaveEditorWord)
		marker_strm << std::uppercase << std::setw(4) << std::hex << (unsigned int)units; else
	if (mode == WaveEditorSeconds)
		marker_strm << std::setprecision(2) << std::fixed << units / 1000.0f; else
		marker_strm << (unsigned int)units;
	return marker_strm.str();
}

float CWaveEditorCtrl::GetSamplesPerUnit(WaveEditorGridMode mode) {
	if (mode == WaveEditorSamples) {
		return 1.0f;
	} else
	if (mode == WaveEditorTicks) {
		return waveInfo.samplesPerTick;//view->mainFrame->player->masterInfo.samples_per_tick;
	} else
	if (mode == WaveEditorSeconds) {
		return waveInfo.samplesPerSec;//(float)wave->get_samples_per_sec(currentLevel) / 1000.0f;
	} else
	if (mode == WaveEditorWord) {
 		float num_samples = waveInfo.samples;//->get_sample_count(currentLevel);
		return num_samples / 65536;
 	}
	
	return 1.0f;
}

//
// Wave binding
//

void CWaveEditorCtrl::SetEditWave(EDITWAVEINFO* pewi, bool reset) {
	//memset(&waveInfo, 0, sizeof(EDITWAVEINFO));
	ClearWaveInfo(&waveInfo);

	if (reset || !pewi) {
		cursorSamplePosition = 0;
		ClearSelection();
	}
	if (!pewi) {
		// clear wave editor
		numDisplaySamples = 0;
		SetLoopPoints(0, 0);
		SetZoomDisplay(0, 0, false);
	} else {
		waveInfo = *pewi;
		// if waveeditor currently shows a zero sized sample, expand the visible range
		//if (numDisplaySamples == 0) numDisplaySamples = waveInfo.samples;

		SetLoopPoints(waveInfo.beginLoop, waveInfo.endLoop);
		if (reset)
			SetZoomDisplay(0, waveInfo.samples, false); else
			SetZoomDisplay(beginDisplaySample, std::min(numDisplaySamples, waveInfo.samples), false);

		// sanitize the cursor - f.ex undoing a paste operation won't adjust the cursor of itself
		if (cursorSamplePosition >= waveInfo.samples)
			cursorSamplePosition = 0;
	}
}

void CWaveEditorCtrl::UpdateDigest() {
	RECT rcClient;
	GetClientRect(&rcClient);

	mindigestL.clear();
	maxdigestL.clear();
	ampdigestL.clear();
	mindigestR.clear();
	maxdigestR.clear();
	ampdigestR.clear();

	if (waveInfo.waveProvider == 0 || numDisplaySamples == 0) return;

	waveInfo.waveProvider->GetSamplesDigest(0, beginDisplaySample, beginDisplaySample + numDisplaySamples, mindigestL, maxdigestL, ampdigestL, rcClient.right);
	if (waveInfo.stereo)
		waveInfo.waveProvider->GetSamplesDigest(1, beginDisplaySample, beginDisplaySample + numDisplaySamples, mindigestR, maxdigestR, ampdigestR, rcClient.right);
}

//
// Context menu
//

LRESULT CWaveEditorCtrl::OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	ClientToScreen(&pt);
	return SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(pt.x, pt.y));
}

LRESULT CWaveEditorCtrl::OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if (pt.x == -1 && pt.y == -1) {
		pt.x = pt.y = 0;
	}

	CMenu menu; 
	menu.CreatePopupMenu();
	menu.InsertMenu(-1, ((beginSelectSample == -1)?MF_GRAYED:0)|MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_EDIT_CUT, "Cut");
	menu.InsertMenu(-1, ((beginSelectSample == -1)?MF_GRAYED:0)|MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_EDIT_COPY, "Copy");
	menu.InsertMenu(-1, (!ClipboardHasAudio(m_hWnd)?MF_GRAYED:0)|MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_EDIT_PASTE, "Paste");
	menu.InsertMenu(-1, ((beginSelectSample == -1)?MF_GRAYED:0)|MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_EDIT_DELETE, "Delete");
	menu.InsertMenu(-1, ((beginSelectSample == -1)?MF_GRAYED:0)|MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVE_TRIM, "Trim"); // edit 29
	//menu.InsertMenu(-1, MF_BYPOSITION|MF_SEPARATOR);
	//menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVE_SILENCE, "Silence");
	//menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVE_FADEIN, "Fade In");
	//menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVE_FADEOUT, "Fade Out");
	//menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_WAVE_AMP, "Amp...");
	menu.InsertMenu(-1, MF_BYPOSITION|MF_SEPARATOR);
	menu.InsertMenu(-1, (!waveInfo.samples?MF_GRAYED:0)|MF_BYPOSITION|MF_STRING, (UINT_PTR)ID_VIEW_PROPERTIES, "Properties");
	menu.InsertMenu(-1, ((beginSelectSample == -1)?MF_GRAYED:0)|MF_STRING, (UINT_PTR)ID_WAVE_SET_LOOPPOINTS_FROM_SELECTION, "Loop Selection");

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	return 0;
}

//
// Mouse input
//

LRESULT CWaveEditorCtrl::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	// vi skal sette et startsample på når vi begynner selecte
	// fordi vi skal senere kunne scrolle viewet mens vi selecter, og da må vi vite mer enn pixel
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	// if pt is in the loop-handle, we're gonna set looping points and must notify parent window

	RECT rcBeginLoop, rcEndLoop;
	GetLoopHandle(0, &rcBeginLoop);
	GetLoopHandle(1, &rcEndLoop);

	// adjust for extreme cases, so we can click on the loop-pixel itself
	rcBeginLoop.left -= 1;
	rcEndLoop.right += 1;

	// TODO: getSliceHandle()
	if (PtInRect(&rcBeginLoop, pt)) {
		moveType = MoveLoopStart;
	} else
	if (PtInRect(&rcEndLoop, pt)) {
		moveType = MoveLoopEnd;
	} else {
		float selpos = beginDisplaySample + (float)pt.x * paintDelta;
		SetSelection(selpos, selpos);
		moveType = MoveSelection;
	}
	SetCapture();

	return 0;
}

LRESULT CWaveEditorCtrl::OnLButtonDoubleClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/){
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	for (int i = 0; i < waveInfo.slices.size() + 1; i++) {
		RECT rcSlice;
		GetSliceRect(i, &rcSlice);
		if (PtInRect(&rcSlice, pt)) {
			int selStart, selEnd;
			GetSliceOffsets(i, &selStart, &selEnd);
			SetSelection(selStart, selEnd);
			SetCursorPosition(selStart);
		}
	}
	return 0;
}

LRESULT CWaveEditorCtrl::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	ReleaseCapture();

	if (moveType == MoveLoopStart || moveType == MoveLoopEnd ) {
		// notify parent
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WEN_LOOPCHANGED), (LPARAM)m_hWnd);
	} else {

		if (moveType == MoveSelection) {
			if (beginSelectSample == endSelectSample) {
				int pos = beginSelectSample;
				ClearSelection();
				if (cursorSamplePosition == pos)
					SetCursorPosition(0);
				else
					SetCursorPosition(pos);
			}
		}
	}

	moveType = MoveNothing;

	return 0;
}

LRESULT CWaveEditorCtrl::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt={(short)LOWORD(lParam),(short)HIWORD(lParam)};
	RECT rc;
	GetClientRect(&rc);

	unsigned int notificationCode;

	if (moveType == MoveLoopStart) {
		waveInfo.beginLoop = beginDisplaySample + (double)pt.x * paintDelta;
		if (waveInfo.beginLoop < beginDisplaySample)
			waveInfo.beginLoop = beginDisplaySample;
		if (waveInfo.beginLoop > beginDisplaySample + numDisplaySamples)
			waveInfo.beginLoop = beginDisplaySample + numDisplaySamples;
		
		notificationCode = WEN_LOOPTRACK;
	} else
	if (moveType == MoveLoopEnd) {
		waveInfo.endLoop = beginDisplaySample + (double)(pt.x) * paintDelta;
		if (waveInfo.endLoop < 0)
			waveInfo.endLoop = 0;
		if (waveInfo.endLoop > waveInfo.samples)
			waveInfo.endLoop = waveInfo.samples;
		notificationCode = WEN_LOOPTRACK;
	} else
	if (moveType == MoveSelection) {
		SetSelection(firstSelectSample, beginDisplaySample + (float)pt.x * paintDelta);
		notificationCode = WEN_SELECTIONCHANGED;
	} else {
		notificationCode = WEN_MOUSEMOVE;
	}

	mouseSamplePosition = beginDisplaySample+(float)pt.x*paintDelta; 
	if (mouseSamplePosition < 0) mouseSamplePosition = 0;
	if (mouseSamplePosition >= waveInfo.samples) mouseSamplePosition = waveInfo.samples - 1;

	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), notificationCode), (LPARAM)m_hWnd);

	if (notificationCode != WEN_MOUSEMOVE) Invalidate(FALSE);

	return 0;
}

LRESULT CWaveEditorCtrl::OnKeyDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return 0;
}

//
// Zoom / scroll
//

LRESULT CWaveEditorCtrl::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

  	WORD req = LOWORD(wParam);

	double unit = paintDelta;
	if (unit < 1.0f) 
		unit = 1.0f;

	SCROLLINFO si;
	size_t maxSamples = waveInfo.samples;

	int halfPageSamples = ceil((float)numDisplaySamples / 2);

	switch (req) {
		case SB_LINERIGHT:
			SetZoomDisplay(beginDisplaySample + unit, numDisplaySamples);
			break;
		case SB_LINELEFT:
			SetZoomDisplay(beginDisplaySample - unit, numDisplaySamples);
			break;
		case SB_PAGERIGHT:
			SetZoomDisplay(beginDisplaySample + halfPageSamples, numDisplaySamples);
			break;
		case SB_PAGELEFT:
			SetZoomDisplay(beginDisplaySample - halfPageSamples, numDisplaySamples);
			break;
		case SB_THUMBTRACK:
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_TRACKPOS;
			GetScrollInfo(SB_HORZ, &si);

			SetZoomDisplay(si.nTrackPos, numDisplaySamples);
			break;
		default:
			return 0;
	}
	return 0;
}

void CWaveEditorCtrl::SetZoomDisplay(double beginSample, long numSamples, bool notify) {

	if (beginSample < 0) beginSample = 0;

	if (beginSample + numSamples > waveInfo.samples) {
		if (numSamples > waveInfo.samples) numSamples = waveInfo.samples;
		beginSample = waveInfo.samples - numSamples;
	}

	beginDisplaySample = beginSample;
	numDisplaySamples = numSamples;

	UpdateDigest();
	UpdateScrollbars();

	if (notify)
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WEN_ZOOMCHANGED), (LPARAM)m_hWnd);
	Invalidate(FALSE);
}

void CWaveEditorCtrl::ZoomShowSelection() {
	if (beginSelectSample != -1)
		SetZoomDisplay(beginSelectSample, endSelectSample - beginSelectSample + 1);
}

void CWaveEditorCtrl::ZoomShowAll() {
	// shift-a, zoom out and show all
	SetZoomDisplay(0, waveInfo.samples);
}

void CWaveEditorCtrl::ZoomIn() {
	// unexpand synlige samples med 25%
	size_t visibleSamples = numDisplaySamples;
	visibleSamples *= 0.25;
	SetZoomDisplay(beginDisplaySample + visibleSamples, numDisplaySamples - visibleSamples);
}

void CWaveEditorCtrl::ZoomOut() {
	// expand synlige samples med 25%
	long visibleSamples = numDisplaySamples;
	visibleSamples *= 0.25;
	long begin;
	if (visibleSamples > beginDisplaySample)
		begin = 0; else
		begin = beginDisplaySample - visibleSamples;
	SetZoomDisplay(begin, numDisplaySamples + visibleSamples);
}

void CWaveEditorCtrl::UpdateAlign() {
	RECT rcClient;
	GetClientRect(&rcClient);
	if (rcClient.right > 0) {
		paintDelta = (double)numDisplaySamples / (double)rcClient.right;

/*		if (paintDelta > 1) {
			double modval = fmod(beginDisplaySample, paintDelta);
			const double epsilon = 0.000002;
			if (!compareWithEpsilon(paintDelta, modval, epsilon) && !compareWithEpsilon(modval, (double)0.0f, epsilon))
				beginDisplaySample -= modval;
		}*/
	} else
		paintDelta = 0;
}

void CWaveEditorCtrl::UpdateScrollbars() {

	UpdateAlign();
	double samples = waveInfo.samples - 1;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPage = numDisplaySamples;
	si.nPos = ceil(beginDisplaySample);
	si.nMin = 0;
	si.nMax = samples;
	SetScrollInfo(SB_HORZ, &si);
}

//
// Selection
//

void CWaveEditorCtrl::ClearSelection() {
	beginSelectSample = -1;
	endSelectSample = -1;
	firstSelectSample = -1;
	lastSelectSample = -1;
	//beginDisplaySample = -1;
	SortSelection();
	if (waveInfo.waveProvider)
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WEN_SELECTIONCHANGED), (LPARAM)m_hWnd);

	Invalidate(FALSE);
}

void CWaveEditorCtrl::SetSelection(long beginSel, long endSel) {
	firstSelectSample = beginSel;
	lastSelectSample = endSel;

	if (firstSelectSample < 0) beginSelectSample = 0;
	if (firstSelectSample > beginDisplaySample + numDisplaySamples)
		firstSelectSample = beginDisplaySample + numDisplaySamples;
	SortSelection();
	if (waveInfo.waveProvider)
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WEN_SELECTIONCHANGED), (LPARAM)m_hWnd);
	Invalidate(FALSE);
}

void CWaveEditorCtrl::SortSelection() {
	beginSelectSample = std::min(firstSelectSample, lastSelectSample);
	endSelectSample = std::max(firstSelectSample, lastSelectSample);
	
	if (beginSelectSample == -1) return ;

	if (beginSelectSample < 0) beginSelectSample = 0;
	if (endSelectSample < 0) endSelectSample = 0;

	if (beginSelectSample >= waveInfo.samples)
		beginSelectSample = waveInfo.samples - 1;

	if (endSelectSample >= waveInfo.samples)
		endSelectSample = waveInfo.samples - 1;
}

//
// Loop points
//

bool CWaveEditorCtrl::GetLoopHandle(int type, RECT* rcOut) {

	RECT rc;
	GetClientRect(&rc);

	// sizes and positioning are determined by the constants defined on top of this page
	int width = rc.right-rc.left;

	int beginLoopX = (float)(waveInfo.beginLoop - beginDisplaySample) / paintDelta;
	int endLoopX = (float)(waveInfo.endLoop - beginDisplaySample) / paintDelta -0.5;

	if (type == 0)
		SetRect(rcOut, beginLoopX, 0, beginLoopX+8, 8); else
		SetRect(rcOut, endLoopX-8, 0, endLoopX, 8);
	return true;
}

LRESULT CWaveEditorCtrl::OnSetLooppointsFromSelection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/){
	
	waveInfo.beginLoop = beginSelectSample;
	waveInfo.endLoop = endSelectSample;

	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), WEN_LOOPCHANGED), (LPARAM)m_hWnd);
	return 0;
}

void CWaveEditorCtrl::SetLoopPoints(long beginLoop, long endLoop) {
	waveInfo.beginLoop = beginLoop;
	waveInfo.endLoop = endLoop;
	Invalidate(FALSE);
}

//
// Slice helpers
//

bool CWaveEditorCtrl::GetSliceHandle(int index, RECT* rcOut) {
	RECT rc;
	GetClientRect(&rc);

	int sliceOfs = waveInfo.slices[index];
	if (sliceOfs < beginDisplaySample) return false;
	if (sliceOfs > beginDisplaySample + numDisplaySamples) return false;

	int sliceX = (sliceOfs - beginDisplaySample) / paintDelta;
	SetRect(rcOut, sliceX - 2, 0, sliceX + 2, rc.bottom);
	return true;
}

void CWaveEditorCtrl::GetSliceOffsets(int index, int* sliceStart, int* sliceEnd) {

	if (index == 0) {
		*sliceStart = 0;
		if (index < (int)waveInfo.slices.size())
			*sliceEnd = waveInfo.slices[index];
		else
			*sliceEnd = waveInfo.samples;
	} else if (index == waveInfo.slices.size()) {
		*sliceStart = waveInfo.slices[index - 1];
		*sliceEnd = waveInfo.samples;
	} else {
		*sliceStart = waveInfo.slices[index - 1];
		*sliceEnd = waveInfo.slices[index];
	}
}

bool CWaveEditorCtrl::GetSliceRect(int index, RECT* rcOut) {
	RECT rc;
	GetClientRect(&rc);

	int sliceStart;
	int sliceEnd;
	GetSliceOffsets(index, &sliceStart, &sliceEnd);

	int sliceX1 = (sliceStart - beginDisplaySample) / paintDelta;
	int sliceX2 = (sliceEnd - beginDisplaySample) / paintDelta;
	SetRect(rcOut, sliceX1, 0, sliceX2, rc.bottom);
	return true;
}

// Misc

LRESULT CWaveEditorCtrl::OnViewProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	return GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(ID_VIEW_PROPERTIES, 0), (LPARAM)m_hWnd);
}

void CWaveEditorCtrl::SetCursorPosition(long position) {
	cursorSamplePosition = position;
	if (cursorSamplePosition < 0) cursorSamplePosition = 0;
	if (cursorSamplePosition >= waveInfo.samples) cursorSamplePosition = waveInfo.samples > 0 ? waveInfo.samples - 1 : 0;
	Invalidate(FALSE);
}
