#pragma once

class CViewInfoImpl : public CViewInfo {
public:
	buze_document_t* document;
	buze_main_frame_t* mainframe;
	zzub_player_t* player;

	CViewInfoImpl(buze_main_frame_t* m) {
		mainframe = m;
		document = buze_main_frame_get_document(m);
		player = buze_document_get_player(document);
	}

	virtual void Attach() {}
	virtual void Detach() {}
	virtual void OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {}
	virtual void Destroy() {
		delete this;
	}
};

class CViewImpl : public CView {
public:
	buze_document_t* document;
	buze_main_frame_t* mainframe;
	zzub_player_t* player;

	CViewImpl(buze_main_frame_t* m) {
		mainframe = m;
		document = buze_main_frame_get_document(m);
		player = buze_document_get_player(document);
	}

	virtual void UpdateTimer(int count) {
		// override to handle timer
	}

	virtual HWND GetHwnd() {
		assert(false); // im lazy!implement in each new view or something
		return 0;
	}

	void GetClientSize(RECT* rc) {
		SetRect(rc, 0, 0, 0, 0);
	}

	// null text returns length, assume caller allocates at least len + null terminator for text
	void GetHelpText(char* text, int* len) {
		*len = 0;
		if (text)
			text[0] = 0;
	}

	// override if the view can inherit keydowns from other views
	bool DoesKeyjazz() {
		return false;
	}

};
