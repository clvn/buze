#pragma once

class Fft;

class CAnalyzerViewInfo : public CViewInfoImpl {
public:
	CAnalyzerViewInfo(buze_main_frame_t* m);

	virtual void Attach();
	virtual void Detach();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual CView* CreateView(HWND hWndParent, void* pCreateData);
};

class CAnalyzerView 
	: public CWindowImpl<CAnalyzerView>
	, public CViewImpl
{
	zzub_plugin_t* visplugin;
	zzub_pluginloader_t* visinfo;
	zzub_plugin_t* sourceplugin;
	zzub_connection_t* sourceconnection;

	Fft* fft;
	int _xrecord;

	CRITICAL_SECTION queueCritical;
	std::deque<std::vector<int> > fftQueue;
	int bufferScreenWidth;
	unsigned int* bitmapBits;
	CBitmap background;
	int mode;

public:
	DECLARE_WND_CLASS("AnalyzerView")
//	DECLARE_WND_CLASS_EX("AnalyzerView", CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, BLACK)

	BEGIN_MSG_MAP(CAnalyzerView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)

		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)

		COMMAND_RANGE_HANDLER(ID_ANALYZER_MODE_FIRST, ID_ANALYZER_MODE_LAST, OnAnalyzerSetMode)
		COMMAND_RANGE_HANDLER(ID_ANALYZER_SOURCE_FIRST, ID_ANALYZER_SOURCE_LAST, OnAnalyzerSetSource)
	END_MSG_MAP()


	CAnalyzerView(buze_main_frame_t* frame);
	~CAnalyzerView(void);
	virtual void OnFinalMessage(HWND /*hWnd*/);

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAnalyzerSetMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAnalyzerSetSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint);
	virtual void UpdateTimer(int count);
	virtual HWND GetHwnd() {
		return m_hWnd;
	}

	void CreateVisualizerPlugin();
	void DestroyVisualizerPlugin();
	void WriteBuffer(float** samples, size_t numSamples);
	void DrawFft();
	void DrawOscilloscope();
};
