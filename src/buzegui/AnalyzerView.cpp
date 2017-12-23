#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "AnalyzerView.h"
#include "Fft.h"

const int MAX_FFT_SIZE = 1024;	// must be square of 2 and dividable by 256 (MAXBUFFERSAMPLES)

using namespace std;

// 
// Factory
//

CAnalyzerViewInfo::CAnalyzerViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CAnalyzerView::GetWndClassInfo().m_wc.lpszClassName;
	label = "Analyzer";
	tooltip = "Analyzer";
	place = 1; //DockSplitTab::placeMAINPANE;
	side = -1; //DockSplitTab::dockUNKNOWN;
	serializable = true;
	allowfloat = true;
	defaultview = false;
}

CView* CAnalyzerViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CAnalyzerView* view = new CAnalyzerView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CAnalyzerViewInfo::Attach() {
	buze_document_add_view(document, this);

	// global accelerators - these generate global document events caught in OnUpdate
	WORD ID_SHOW_ANALYZER = buze_main_frame_register_accelerator_event(mainframe, "view_analyzer", "y ctrl shift", buze_event_type_show_analyzer);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_ANALYZER, "Analyzer");

}

void CAnalyzerViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CAnalyzerViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CView* view;
	switch (lHint) {
		case buze_event_type_show_analyzer:
			view = buze_main_frame_get_view(mainframe, "AnalyzerView", 0);
			if (view) {
				buze_main_frame_set_focus_to(mainframe, view);
			} else
				buze_main_frame_open_view(mainframe, "AnalyzerView", "Analyzer", 0, -1, -1);
			break;
	}
}

//
// View
//

CAnalyzerView::CAnalyzerView(buze_main_frame_t* m)
	:CViewImpl(m)
{
	_xrecord = 0;
	bitmapBits = 0;
	bufferScreenWidth = 0;
	mode = 0;
	visplugin = 0;
	visinfo = 0;
	sourceplugin = 0;
	sourceconnection = 0;
	//setBufferSize(MAX_FFT_SIZE*2);

	buze_application_t* app = buze_main_frame_get_application(mainframe);
	zzub_audiodriver_t* driver = (zzub_audiodriver_t*)buze_application_get_audio_driver(app);

	fft = new Fft(MAX_FFT_SIZE, zzub_audiodriver_get_samplerate(driver));
	InitializeCriticalSection(&queueCritical);
}

CAnalyzerView::~CAnalyzerView(void) {
	DeleteCriticalSection(&queueCritical);
	delete fft;
}

void CAnalyzerView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

LRESULT CAnalyzerView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	buze_main_frame_add_timer_handler(mainframe, this);
	buze_document_add_view(document, this);
	buze_main_frame_viewstack_insert(mainframe, this);
	visinfo = zzub_player_get_pluginloader_by_name(player, "@zzub.org/recorder/visualizer");
	CreateVisualizerPlugin();
	return 0;
}

LRESULT CAnalyzerView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	if (visplugin) DestroyVisualizerPlugin();
	buze_main_frame_remove_timer_handler(mainframe, this);
	buze_document_remove_view(document, this);
	return 0;
}


LRESULT CAnalyzerView::OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	WORD cx = LOWORD(lParam);
	WORD cy = HIWORD(lParam);

	if (background.m_hBitmap)
		background.DeleteObject();

	BITMAPINFO bi;
	memset(&bi, 0, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = cx;
	bi.bmiHeader.biHeight = cy;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biPlanes = 1;

	bufferScreenWidth = cx;
	CDC dcCompat = CreateCompatibleDC(0);
	background.CreateDIBSection(dcCompat, &bi, DIB_RGB_COLORS, (void**)&bitmapBits, 0, 0);
	return 0;
}

LRESULT CAnalyzerView::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return TRUE;
}

LRESULT CAnalyzerView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	if (!background.m_hBitmap) return 0;

	if (mode == 0)
		DrawFft(); else
	if (mode == 1)
		DrawOscilloscope();

	CPaintDC dc(m_hWnd);
	HFONT guifont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	HFONT prevfont = dc.SelectFont(guifont);
	CDC dcImage;
	if (dcImage.CreateCompatibleDC(dc.m_hDC)) {
		CSize size;
		if (background.GetSize(size)) {
			HBITMAP hBmpOld = dcImage.SelectBitmap(background);
			dc.BitBlt(0, 0, size.cx, size.cy, dcImage, 0, 0, SRCCOPY);
			if (sourceplugin != 0) {
				const char* pluginName = zzub_plugin_get_name(sourceplugin);
				dc.TextOut(0, 0, pluginName);
			} else
				dc.TextOut(0, 0, "<no source plugin selected>");
			dcImage.SelectBitmap(hBmpOld);
		}
	}
	dc.SelectFont(prevfont);

	return 0;
}

LRESULT CAnalyzerView::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// set up context menu that sends ID_ANALYZER_MODE_FIRST ID_ANALYZER_MODE_FIRST+1 etc
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if (pt.x == -1 && pt.y == -1) {
		pt.x = pt.y = 0;
		ClientToScreen(&pt);
	}
	CMenu menu; 
	menu.CreatePopupMenu();
	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING|(mode==0?MF_CHECKED:0), (UINT_PTR)ID_ANALYZER_MODE_FIRST + 0, "Simple");
	menu.InsertMenu(-1, MF_BYPOSITION|MF_STRING|(mode==1?MF_CHECKED:0), (UINT_PTR)ID_ANALYZER_MODE_FIRST + 1, "Oscilloscope");

	// select sourceplugin from context = all plugins w/plugin_flag_has_audio_output

	CMenu pluginMenu;
	pluginMenu.CreatePopupMenu();
	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
	int index = 0;
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		int flags = zzub_plugin_get_flags(plugin);
		if (flags & zzub_plugin_flag_has_audio_output) {
			const char* pluginName = zzub_plugin_get_name(plugin);
			bool issource = plugin == sourceplugin;
			pluginMenu.InsertMenu(-1, MF_BYPOSITION|MF_STRING|(issource?MF_CHECKED:0), (UINT_PTR)ID_ANALYZER_SOURCE_FIRST + index, pluginName);
			index++;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);

	menu.InsertMenu(-1, MF_BYPOSITION|MF_POPUP, (UINT_PTR)pluginMenu.m_hMenu, "Source");

	//ClientToScreen(&pt);

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd, 0);

	return 0;
}

LRESULT CAnalyzerView::OnAnalyzerSetMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	RECT rect;
	GetClientRect(&rect);
	int _width = rect.right - rect.left;
	int _height = rect.bottom - rect.top;
	memset(bitmapBits, 0, _height * _width * sizeof(bitmapBits[0]));

	WORD type = wID - ID_ANALYZER_MODE_FIRST;
	switch (type) {
		case 0:
			mode = 0;
			break;
		case 1:
			mode = 1;
			break;
	}
	return 0;
}

LRESULT CAnalyzerView::OnAnalyzerSetSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	WORD sourceindex = wID - ID_ANALYZER_SOURCE_FIRST;

	zzub_plugin_iterator_t* plugit = zzub_player_get_plugin_iterator(player);
	int index = 0;
	while (zzub_plugin_iterator_valid(plugit)) {
		zzub_plugin_t* plugin = zzub_plugin_iterator_current(plugit);
		int flags = zzub_plugin_get_flags(plugin);
		if (flags & zzub_plugin_flag_has_audio_output) {
			if (sourceindex == index) {
				if (visplugin) DestroyVisualizerPlugin();
				sourceplugin = plugin;
				CreateVisualizerPlugin();
				break;
			}
			index++;
		}
		zzub_plugin_iterator_next(plugit);
	}
	zzub_plugin_iterator_destroy(plugit);
	return 0;
}


void CAnalyzerView::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	// check if source plugin was deleted
	// or .. pre-save, post-save song, etc
	zzub_event_data_t* zzubData = (zzub_event_data_t*)pHint;

	switch (lHint) {
		// the "before_delete_plugin"-event was added primarily to disconnect the sourceplugin un-undoably
		case zzub_event_type_before_delete_plugin:
			if (zzubData->delete_plugin.plugin == sourceplugin) {
				if (sourceconnection) {
					int historystate = zzub_player_history_enable(player, 0);
					zzub_connection_destroy(sourceconnection);
					zzub_player_history_enable(player, historystate);
					sourceconnection = 0;
				}
			}
			break;
		case zzub_event_type_delete_plugin:
			if (zzubData->delete_plugin.plugin == sourceplugin)
				sourceplugin = 0;
			break;
		case buze_event_type_update_pre_save_document:
		case buze_event_type_update_pre_clear_document:
		case buze_event_type_update_pre_open_document:
			DestroyVisualizerPlugin();
			break;
		case buze_event_type_update_post_save_document:
		case buze_event_type_update_post_clear_document:
		case buze_event_type_update_post_open_document:
			CreateVisualizerPlugin();
			break;
	}
}

void CAnalyzerView::UpdateTimer(int count) {

	if (!visplugin) return ;

	float l[4096], r[4096];
	float* buffers[] = { l, r };
	int numsamples = 4096;
	int size = zzub_plugin_get_encoder_digest(visplugin, 0, buffers, numsamples);
	if (size > 0) {
		WriteBuffer(buffers, size);
	}
}

void CAnalyzerView::DrawFft() {
	RECT rect;
	GetClientRect(&rect);
	int _width = rect.right - rect.left;
	int _height = rect.bottom - rect.top;

	int pts = fft->Points() / 2;
	float scaleHeight = (float)_height / (float)pts;///(float)fft.Points()/2;
	double frqd = (double)pts / (double)_height;

	EnterCriticalSection(&queueCritical);
	deque<vector<int> > copyQueue = fftQueue;
	fftQueue.clear();
	LeaveCriticalSection(&queueCritical);

	while (!copyQueue.empty()) {
		if (_xrecord >= _width) _xrecord=0;

		vector<int>& points = copyQueue.front();

		fft->Transform(points);

		double frq = 0;
		for (int i = 0; i < _height; i++) {

			int f = floor(frq);
			double n;
			double d = modf(frq, &n);

			double ints = fft->GetIntensity(f);
			if (f>=fft->Points()) f = fft->Points()-2;
			double ints2 = fft->GetIntensity(f+1);

			int s = abs((ints + (ints2 - ints)* d )) / 16;
			if (s<4) s = 0;
			if (s>254) s = 254;

			bitmapBits[_xrecord + i *_width] = RGB(s, s, s);

			frq += frqd;
		}

		_xrecord++;
		copyQueue.pop_front();
	}
}

static float fftfunc(float x)
{
	if(x == 0.0) return 0.0;

	x = log10(x) * 40.0 - 40;

	return x;
}

void CAnalyzerView::DrawOscilloscope()
{
	int i, len;
	RECT rect;
	GetClientRect(&rect);
	int _width = rect.right - rect.left;
	int _height = rect.bottom - rect.top;

	EnterCriticalSection(&queueCritical);
	deque<vector<int> > copyQueue = fftQueue;
	fftQueue.clear();
	LeaveCriticalSection(&queueCritical);

	if (!copyQueue.empty()) {
		vector<int>& points = copyQueue.front();

		memset(bitmapBits, 0, _height * _width * sizeof(bitmapBits[0]));

		len = points.size();
		if(len > _width) len = _width;
		if(len > 512) len = 512;

		int y = points[0] * 100 / 32768 + 200;
		if(y < 0) y = 0;
		else if(y >= _height) y = _height - 1;
		int prevy = y;

		for(i = 0; i < len; i++) {
			y = points[i] * 100 / 32768 + 200;
			if(y < 0) y = 0;
			else if(y >= _height) y = _height - 1;

			int sy;

			/* draw line: skip first pixel unless ydiff = 0 */

			if(y < prevy) {
				prevy--;

				for(int j = y; j <= prevy; j++) {
					bitmapBits[i + j * _width] = RGB(64, 255, 64);
				}
			} else if(y > prevy) {
				prevy++;

				for(int j = prevy; j <= y; j++) {
					bitmapBits[i + j * _width] = RGB(64, 255, 64);
				}
			} else { /* y = prevy */
				bitmapBits[i + y * _width] = RGB(64, 255, 64);
			}

			prevy = y;
		}
	}

	/* draw fft */

	int pts = fft->Points() / 2 + 1;
	float scaleHeight = (float)_height / (float)pts;///(float)fft.Points()/2;
	double frqd = (double)pts / (double)_height;

	if (!copyQueue.empty()) {

		vector<int>& points = copyQueue.front();
		fft->Transform(points);

		if(pts > _width)
			pts = _width;

		int y = fftfunc(fft->GetIntensity(0));
		if(y < 0) y = 0;
		else if(y >= _height) y = _height - 1;
		int prevy = y;

		for (int i = 0; i < pts; i++) {

			y = fftfunc(fft->GetIntensity(i));
			if(y < 0) y = 0;
			else if(y >= _height) y = _height - 1;

			int sy;

			/* draw line: skip first pixel unless ydiff = 0 */

			if(y < prevy) {
				prevy--;

				for(int j = y; j <= prevy; j++) {
					bitmapBits[i + j * _width] = RGB(64, 255, 64);
				}
			} else if(y > prevy) {
				prevy++;

				for(int j = prevy; j <= y; j++) {
					bitmapBits[i + j * _width] = RGB(64, 255, 64);
				}
			} else { /* y = prevy */
				bitmapBits[i + y * _width] = RGB(64, 255, 64);
			}

			prevy = y;
		}
		copyQueue.pop_front();
	}

	while (!copyQueue.empty()) {
		vector<int>& points = copyQueue.front();
		fft->Transform(points);

		copyQueue.pop_front();
	}
}

void CAnalyzerView::WriteBuffer(float** buffer, size_t numSamples) {

	int numffts = numSamples / MAX_FFT_SIZE;

	EnterCriticalSection(&queueCritical);
	for (int j = 0; j < numffts; j++) {
		vector<int> v(MAX_FFT_SIZE);

		for (unsigned int i = 0; i < MAX_FFT_SIZE; i++) {
			float s = buffer[0][j * MAX_FFT_SIZE + i];
			if (s > 1.0) s = 1.0; else
			if (s < -1.0) s = -1.0;
			v[i] = (int)(s*0x8000);
		}
		fftQueue.push_back(v);
	}

	while (fftQueue.size() > 10) {
		fftQueue.pop_front();
	}
	LeaveCriticalSection(&queueCritical);

	Invalidate(FALSE);
}


void CAnalyzerView::CreateVisualizerPlugin() {
	assert(visinfo);
	assert(visplugin == 0);
	int historystate = zzub_player_history_enable(player, 0);
	visplugin = zzub_player_create_plugin(player, 0, 0, "Vis", visinfo, 0);
	assert(visplugin != 0);

	// hide from all views
	buze_document_set_plugin_non_song(document, visplugin, true);

	if (sourceplugin == 0)
		sourceplugin = zzub_player_get_plugin_by_name(player, "Master");

	if (sourceplugin != 0) {
		int conninputs = zzub_plugin_get_output_channel_count(sourceplugin);
		int connoutputs = zzub_plugin_get_input_channel_count(visplugin);
		sourceconnection = zzub_plugin_create_audio_connection(visplugin, sourceplugin, 0, conninputs, 0, connoutputs);
		assert(sourceconnection != 0);
	}

	zzub_player_history_commit(player, 0, 0, "Create Visualizer Plugin");
	zzub_player_history_enable(player, historystate);

}

void CAnalyzerView::DestroyVisualizerPlugin() {
	assert(visinfo);
	assert(visplugin != 0);
	int historystate = zzub_player_history_enable(player, 0);
	zzub_plugin_destroy(visplugin);
	zzub_player_history_commit(player, 0, 0, "Delete Visualizer Plugin");
	zzub_player_history_enable(player, historystate);
	visplugin = 0;
	sourceconnection = 0;
}
