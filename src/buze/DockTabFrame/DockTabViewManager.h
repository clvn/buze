#pragma once

namespace DockSplitTab {

template <class MainFrameT, class ViewT>
class DockTabClientViewSerializer
{
  public:

	MainFrameT* mainFrame;
	ViewT* object;
	DockTabClientViewSerializer(MainFrameT* mainFrame) : mainFrame(mainFrame), object(0) {}
	std::string serialize();
	ViewT* deserialize(std::string const& data);
};

class DockTabClientFactory
{
  public:

	virtual std::string serializeClientWindow(HWND hWndClient) = 0;
	virtual DockSplitTab::ClientView* deserializeClientWindow(HWND hWndParent, std::string const& caption, int view_id) = 0;
	virtual DockSplitTab::ClientView* createClientWindow(HWND hWndParent, std::string const& caption = "", std::string const& toolTip = "", int imageIndex = -1, int view_id = 0) = 0;
	virtual DockSplitTab::ClientView* recycleClientWindow(ClientView* clientView, HWND hWndParent, int view_id) = 0; // screenset switching
	virtual HWND insertClient(DockSplitTab::ClientView* outerView, RECT* rect = 0, DockSplitTab::FramePlace framePlace = placeUNKNOWN, DockSplitTab::DockSide dockSide = dockUNKNOWN) = 0;

	virtual size_t getViewCount() = 0;
	virtual DockSplitTab::FramePlace getPlace() = 0;
	virtual DockSplitTab::DockSide getDockSide() = 0;
	virtual bool getFloatRects(std::map<int, RECT>& namedRects) = 0;

	virtual void setPlace(DockSplitTab::FramePlace) = 0;
	virtual void setSide(DockSplitTab::DockSide) = 0;
	virtual void setFloatRect(int view_id, const RECT& rc) = 0;
};

// implemented by views
template <class ViewT>
class DynamicDockTabViewFactory {
public:
	virtual ~DynamicDockTabViewFactory() {}
	virtual ViewT* CreateView(HWND hWndParent, void* lpParam) = 0;
	virtual std::string GetClassName() = 0;
	virtual std::string GetLabel() = 0;
	virtual std::string GetToolTip() = 0;
	virtual DockSplitTab::FramePlace GetPlace() = 0;
	virtual DockSplitTab::DockSide GetSide() = 0;
	virtual bool GetSerializable() = 0;
	virtual bool GetAllowFloat() = 0;
	virtual bool GetDefaultView() = 0;
	virtual int GetDefaultFloatWidth() = 0;
	virtual int GetDefaultFloatHeight() = 0;
};

template <class MainFrameT, class ViewT>
class DockTabClientFactoryImpl : public DockTabClientFactory {
protected:
	MainFrameT* frame;
	std::vector<ViewT*> views;
public:
	DockSplitTab::FramePlace framePlace;
	DockSplitTab::DockSide dockSide;
    bool allowFloat, serializable;
	std::map<int, RECT> captionSizes; // view_id->RECT
	std::string label, toolTip;
	std::string className;

	virtual HWND ViewGetHWND(ViewT* view) = 0;
	virtual LRESULT ViewCloseWindow(ViewT* view) = 0;
	//virtual void ViewDelete(ViewT* view) = 0;

    virtual HWND insertClient(DockSplitTab::ClientView* outerView, RECT* rect = 0, DockSplitTab::FramePlace place = placeUNKNOWN, DockSplitTab::DockSide side = dockUNKNOWN) {
        if (place == placeUNKNOWN) {
			place = framePlace;
			side = dockSide;
        }

		RECT* prcDefault = rect;
		RECT rcDefault;
		if (place == placeFLOATFRAME) {
			std::map<int, RECT>::iterator i = captionSizes.find(outerView->viewId);
			if (i == captionSizes.end()) {
				// try first specified rect, 
				// if there was none, use the first existing similar window size
				// if there was none, use the frame size
				if (rect) {
					prcDefault = rect;
				}
				else if (captionSizes.size()) {
					prcDefault = &captionSizes.begin()->second;
				}
				else {
					GetClientRect(frame->m_hWnd, prcDefault);
				}
			} else {
				rcDefault = i->second;
				prcDefault = &rcDefault;
			}
		}

		switch (place) {
			case placeMAINPANE:
				frame->frame.addClientView(outerView);
				break;
			case placeDOCKPANE:
				frame->frame.dockClientView(side, outerView);
				break;
			case placeFLOATFRAME:
				return frame->frame.floatClientView(prcDefault, outerView);
		}
		return 0;
	}

	// screenset switching
	virtual DockSplitTab::ClientView* recycleClientWindow(ClientView* clientView, HWND hWndParent, int view_id) {
		if (!clientView) return 0;

		CRect rcFrame;
		frame->GetClientRect(&rcFrame);

		CWindow clientWnd(clientView->wnd);
		clientWnd.SetParent(hWndParent);
		clientWnd.ResizeClient(rcFrame.Width(), rcFrame.Height());

		frame->recycleClientWindow(clientView); // equivalent of frame->createClientWindow but for recycled views

		clientView->allowFloat = allowFloat;
		clientView->viewId = view_id;

		// serializedData was unused here

		ViewT* view = (ViewT*)(clientView->object);
		views.push_back(view);
		return clientView;
	}

	/*virtual bool hasFloatRect(const std::string& name) {
		std::map<std::string, RECT>::iterator i = captionSizes.find(name.c_str());
		return i != captionSizes.end();
	}*/

	virtual bool getFloatRects(std::map<int, RECT>& namedRects) {
		namedRects = captionSizes;
		return true;
	}

	virtual size_t getViewCount() {
		return views.size();
	}

	HWND getViewHWND(size_t index) {
		if (index >= views.size()) return 0;
		return ViewGetHWND(views[index]);
	}

	ViewT* getView(size_t index) {
        if (index >= views.size()) return 0;
		return views[index];
	}

	virtual DockSplitTab::FramePlace getPlace() {
		return framePlace;
	}

	virtual DockSplitTab::DockSide getDockSide() {
		return dockSide;
	}

	virtual void setPlace(DockSplitTab::FramePlace place) {
		framePlace = place;
	}

	virtual void setSide(DockSplitTab::DockSide side) {
		dockSide = side;
	}

	virtual void setFloatRect(int view_id, const RECT& rc) {
		captionSizes.insert(std::pair<int, RECT>(view_id, rc));
	}


	void closeAll(bool destroy = false) {
        for (size_t i = 0; i < views.size(); ++i) {
			ViewT* view = views[i];
			HWND viewWnd = ViewGetHWND(view);

            if (destroy) {
                if (0 == ViewCloseWindow(view)) {
                    ClientView* clientView = frame->frame.getClientView(viewWnd);
                    assert(clientView);
                    frame->frame.detachClientView(viewWnd);

                    // is it a bug that detachClientView never does cbListener->clientDetached?
                    frame->clientDetached(clientView); // MainFrame causes destroy() then deletes the ClientView
                    --i; // because destroy() removes it from the vector
                }
            } else {
                frame->clientViewHide(viewWnd);
            }
        }
	}

    bool destroy(ClientView* clientView) {
        ViewT* view = lookupView(clientView);
        if (view == 0) return false;

        std::vector<ViewT*>::iterator i = find(views.begin(), views.end(), view);
        if (i != views.end())
            views.erase(i);

		// detachClientView updates lastPlace and floatRect
		// NOTE: when closing an entire pane, detachClientViews is called instead, which doesnt?
        //frame->frame.detachClientView(clientView->wnd);

        DestroyWindow(view->GetHwnd());
		//delete view; //ViewDelete(view);
        // the clientView is deleted in CMainFrame::clientDetached

		saveCaptionSizes(clientView);

        return true;
    }

	// screenset switching
	void detachAllForRecycler(std::vector<ClientView*>& current_views) {
		for (size_t i = 0; i < views.size(); ++i) {
			ViewT* view = views[i];
			HWND viewWnd = ViewGetHWND(view);

			ClientView* clientView = frame->frame.getClientView(viewWnd);
			assert(clientView);

			frame->frame.detachClientView(viewWnd);

			current_views.push_back(clientView);
		}

		views.clear();
	}

	// screenset switching
    void destroyUnrecycled(std::vector<ClientView*>& current_views) {
		for (std::vector<ClientView*>::iterator i = current_views.begin(); i != current_views.end();) {
			ClientView* clientView = *i;

			char clientView_className[1024];
			GetClassName(clientView->wnd, &clientView_className[0], array_size(clientView_className));

			bool removed = false;
			if (className == clientView_className) {
				ViewT* view = (ViewT*)(clientView->object);
				if (0 == ViewCloseWindow(view)) {
					frame->clientDetachedForRecycle(clientView);
					//delete view; //ViewDelete(view);
					DestroyWindow(view->GetHwnd()); // view deletes itself now
					saveCaptionSizes(clientView);
					delete clientView;
					removed = true;
				}
			}

			if (removed)
				i = current_views.erase(i);
			else
				++i;
		}
    }

	ViewT* lookupView(DockSplitTab::ClientView* clientView) {
		for (size_t i = 0; i < views.size(); ++i) {
			HWND viewWnd = ViewGetHWND(views[i]);
			if (viewWnd == clientView->wnd) {
				return views[i];
			}
		}
		return 0;
	}

	ViewT* lookupView(HWND hWnd) {
		for (size_t i = 0; i < views.size(); ++i) {
			HWND viewWnd = ViewGetHWND(views[i]);
			if (viewWnd == hWnd) {
				return views[i];
			}
		}
		return 0;
	}

    void saveCaptionSizes(ClientView* clientView) {
		if (clientView->lastPlace == placeFLOATFRAME) {
			RECT rc = clientView->floatRect;

			// megz.restoresizefix
			//rc.top += 7;
			//rc.left += 3;
			//rc.right -= 3;
			//rc.bottom += 1;

			captionSizes[clientView->viewId] = rc;
			framePlace = placeFLOATFRAME;
		} else {
			framePlace = clientView->lastPlace;
			dockSide = clientView->dockSide;
		}
	}
};

template <class MainFrameT, class ViewT>
class DynamicDockTabViewManager : public DockTabClientFactoryImpl<MainFrameT, ViewT> {
public:
	bool defaultView;
	int defaultFloatWidth;
	int defaultFloatHeight;
	DynamicDockTabViewFactory<ViewT>* factory;

	DynamicDockTabViewManager(MainFrameT* mainframe, DynamicDockTabViewFactory<ViewT>* viewInfo) {
		frame = mainframe;
		factory = viewInfo;

		allowFloat = factory->GetAllowFloat();
		dockSide = factory->GetSide();
		framePlace = factory->GetPlace();
		label = factory->GetLabel();
		toolTip = factory->GetToolTip();
		className = factory->GetClassName();
		serializable = factory->GetSerializable();
		defaultView = factory->GetDefaultView();
		defaultFloatWidth = factory->GetDefaultFloatWidth();
		defaultFloatHeight = factory->GetDefaultFloatHeight();
        std::cerr << "DynamicDockTabViewManager registered WndClass: " << className << std::endl;
	}

	~DynamicDockTabViewManager() {
		delete factory;
	}

	virtual HWND ViewGetHWND(ViewT* view);
	virtual LRESULT ViewCloseWindow(ViewT* view);

    virtual std::string serializeClientWindow(HWND hWndClient) {
		return "";
    }

    virtual DockSplitTab::ClientView* deserializeClientWindow(HWND hWndParent, std::string const& caption, int view_id) {
        ViewT* view = factory->CreateView(hWndParent, (void*)view_id);

		HWND viewWnd = view->GetHwnd(); //view->Create(hWndParent, CWindow::rcDefault, caption.c_str(), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
		//HWND viewWnd = ViewCreateWindow(view, hWndParent, CWindow::rcDefault, caption, 0);

        DockSplitTab::ClientView* clientView = frame->createClientWindow(viewWnd, caption, "", -1);
        if (!clientView) return 0;

        clientView->allowFloat = allowFloat;
        clientView->object = view; // for recycling
		clientView->viewId = view_id;
		views.push_back(view);
        return clientView;
    }


    virtual DockSplitTab::ClientView* createClientWindow(HWND hWndParent, std::string const& caption = "", std::string const& tip = "", int imageIndex = -1,int view_id = 0) {
        ViewT* view = factory->CreateView(hWndParent, (void*)view_id);

		HWND viewWnd = view->GetHwnd(); //view->Create(hWndParent, CWindow::rcDefault, caption.c_str(), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
		//HWND viewWnd = ViewCreateWindow(view, hWndParent, CWindow::rcDefault, caption, 0);

        DockSplitTab::ClientView* clientView = frame->createClientWindow(viewWnd, (caption=="")?label:caption, (tip=="")?toolTip:tip, imageIndex);
        if (!clientView) return 0;

        clientView->allowFloat = allowFloat;
        clientView->object = view; // for recycling
		clientView->viewId = view_id;
		views.push_back(view);
        return clientView;
    }

};

template <class MainFrameT, class ViewT>
class DockTabViewManager : public DockTabClientFactoryImpl<MainFrameT, ViewT>
{
  public:

	//MainFrameT* frame;

	DockTabViewManager(MainFrameT* frame, DockSplitTab::FramePlace place, DockSplitTab::DockSide side, std::string const& label, std::string const& toolTip = label, bool allowFloat = true) {
		this->frame = frame;
		this->framePlace = place;
		this->dockSide = side;
		this->label = label;
		this->toolTip = toolTip;
        this->allowFloat = allowFloat;
        className = ViewT::GetWndClassInfo().m_wc.lpszClassName;
        std::cerr << "DockTabViewManager registered WndClass: " << className << std::endl;
	}

	virtual HWND ViewGetHWND(ViewT* view);
	virtual LRESULT ViewCloseWindow(ViewT* view);
	virtual HWND ViewCreateWindow(ViewT* view, HWND hWndParent, RECT rect, std::string const& caption, void* userdata);

    virtual std::string serializeClientWindow(HWND hWndClient) {
        DockTabClientViewSerializer<MainFrameT, ViewT> serializer(frame);
        serializer.object = lookupView(hWndClient);
        return serializer.serialize();
    }

    virtual DockSplitTab::ClientView* deserializeClientWindow(HWND hWndParent, std::string const& caption, int view_id) {
        DockTabClientViewSerializer<MainFrameT, ViewT> serializer(frame);
        ViewT* view = serializer.deserialize("");

		//RECT rcFrame;
		//frame->GetClientRect(&rcFrame);
		HWND viewWnd = ViewCreateWindow(view, hWndParent, CWindow::rcDefault, caption, (void*)view_id);

        DockSplitTab::ClientView* clientView = frame->createClientWindow(viewWnd, caption, "", -1);
        if (!clientView) return 0;

        clientView->allowFloat = allowFloat;

        clientView->object = view; // for recycling
		clientView->viewId = view_id;

		views.push_back(view);

        return clientView;
    }

    virtual DockSplitTab::ClientView* createClientWindow(HWND hWndParent, std::string const& caption = "", std::string const& tip = "", int imageIndex = -1, int view_id = 0) {
		DockTabClientViewSerializer<MainFrameT, ViewT> serializer(frame);
		ViewT* view = serializer.deserialize("");

		//RECT rcFrame;
		//frame->GetClientRect(&rcFrame);
		HWND viewWnd = ViewCreateWindow(view, hWndParent, CWindow::rcDefault, caption, (void*)view_id);
		assert(viewWnd);

        DockSplitTab::ClientView* clientView = frame->createClientWindow(viewWnd, (caption=="")?label:caption, (tip=="")?toolTip:tip, imageIndex);
        if (!clientView) return 0;

        clientView->allowFloat = allowFloat;

        clientView->object = view; // for recycling
		clientView->viewId = view_id;

		views.push_back(view);

        return clientView;
    }


};

} // END namespace DockSplitTab
