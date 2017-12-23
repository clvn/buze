#include "stdafx.h"

#include "DockTabFrame.h"
#include "DockTabViewManager.h"
#include "DockTabSerializer.h"
#include "pugxml.h"
#include <boost/lexical_cast.hpp>

using namespace pug;

namespace DockSplitTab {

DockTabSerializer::DockTabSerializer() {
	lastClientView = 0;
}

void DockTabSerializer::serializeClient(DockSplitTab::Frame* frame, xml_node& parent, HWND clientWnd) {
	char lpClassName[1024];
	GetClassName(clientWnd, lpClassName, array_size(lpClassName));

	// get window class name for 
	DockTabClientFactory* factory = getClassFactory(lpClassName);
	if (factory == 0) {
		//MessageBox(frame->m_hWnd, "Unknown serializer window class factory", lpClassName, MB_OK);
		return;
	}

	parent.append_attribute("class", lpClassName);

	ClientView* clientView = frame->getClientView(clientWnd);
	parent.append_attribute("caption", clientView->caption);
	parent.append_attribute("view_id", (long)clientView->viewId);

	std::string data = factory->serializeClientWindow(clientWnd);
	if (data.length()) {
		xml_node node = parent.append_child(node_pcdata);
		node.value(data.c_str());
	}
}

void setActivePane(DockSplitTab::Frame* frame, DockSplitTab::Pane* pane, xml_node& parent) {
	HWND clientWnd = pane->focusedClientView();
	if (clientWnd != 0) {
		ClientView* clientView = frame->getClientView(clientWnd);
		parent.append_attribute("active", clientView->caption);
	}
}

int DockTabSerializer::serializeSplitterPaneViews(DockSplitTab::Frame* frame, xml_node& parent, SplitPane* pane, HWND hWnd) {
	HSplitter* leftTopHSplitter = pane->lookUpHSplitter(hWnd);
	VSplitter* leftTopVSplitter = pane->lookUpVSplitter(hWnd);

	Pane* panePane = pane->lookUpPane(hWnd);
	Pane* paneWndPane = pane->lookUpPaneWnd(hWnd);

	if (leftTopHSplitter) {
		xml_node hsplitter = parent.append_child(node_element);
		hsplitter.name("hsplitter");
		return serializeSplitter<HSplitter>(frame, hsplitter, pane, leftTopHSplitter);
	} else
	if (leftTopVSplitter) {
		xml_node vsplitter = parent.append_child(node_element);
		vsplitter.name("vsplitter");
		return serializeSplitter<VSplitter>(frame, vsplitter, pane, leftTopVSplitter);
	} else 
	if (paneWndPane) {
		setActivePane(frame, paneWndPane, parent);
		for (int i=0; i<paneWndPane->clientCount(); ++i) {
			HWND clientWnd = paneWndPane->getClientView(i);

			xml_node client = parent.append_child(node_element);
			client.name("client");
			serializeClient(frame, client, clientWnd);
		}

		return paneWndPane->clientCount();
	}
	return 0;
}

bool DockTabSerializer::splitterPaneRootFilter(SplitPane* pane, HWND paneWnd, std::map<HWND, bool>& allSplitters) {
	HSplitter* hsplit = pane->lookUpHSplitter(paneWnd);
	VSplitter* vsplit = pane->lookUpVSplitter(paneWnd);

	if (hsplit)
		splitterRootFilter<HSplitter>(pane, hsplit, allSplitters);
	if (vsplit)
		splitterRootFilter<VSplitter>(pane, vsplit, allSplitters);

	if (hsplit || vsplit) {
		allSplitters.erase(paneWnd);
		return true;
	}
	return false;
}

HWND DockTabSerializer::findRootSplitter(SplitPane* pane) {
	std::map<HWND, bool> allSplitters;

	// 1. finn alle splitters
	for (POSITION start = pane->verSplitterMap.GetStartPosition(); start != 0; pane->verSplitterMap.GetNext(start)) {
		HWND hWnd = pane->verSplitterMap.GetKeyAt(start);
		allSplitters[hWnd] = true;
	}

	for (POSITION start = pane->horSplitterMap.GetStartPosition(); start != 0; pane->horSplitterMap.GetNext(start)) {
		HWND hWnd = pane->horSplitterMap.GetKeyAt(start);
		allSplitters[hWnd] = true;
	}

	// 2. fjern splitters fra allSplitters som er referert i panes
	bool restart;
	do {
		restart = false;
		for (std::map<HWND, bool>::iterator i = allSplitters.begin(); i != allSplitters.end(); ++i) {
			if (i->second == false) continue;

			size_t size = allSplitters.size();
			HSplitter* leftTopHSplitter = pane->lookUpHSplitter(i->first);
			VSplitter* leftTopVSplitter = pane->lookUpVSplitter(i->first);
			if (leftTopHSplitter) {
				splitterRootFilter<HSplitter>(pane, leftTopHSplitter, allSplitters);
			}
			if (leftTopVSplitter) {
				splitterRootFilter<VSplitter>(pane, leftTopVSplitter, allSplitters);
			}

			if (size != allSplitters.size()) {
				restart = true;
				break;
			}
		}
	} while (restart);

	// 3. nå skal det være 0 eller 1 splitters igjen i allSplitters
	assert(allSplitters.size()<=1);

	if (allSplitters.size() != 1) return 0;

	return allSplitters.begin()->first;
}

int DockTabSerializer::serializeSplitPane(DockSplitTab::Frame* frame, xml_node& parent, SplitPane* pane, RECT *rc) {
	int counter = 0;

	parent.append_attribute("width", rc->right - rc->left);
	parent.append_attribute("height", rc->bottom - rc->top);

	HWND rootSplitterWnd = findRootSplitter(pane);
	if (rootSplitterWnd) {
		HSplitter* rootHSplitter = pane->lookUpHSplitter(rootSplitterWnd);
		VSplitter* rootVSplitter = pane->lookUpVSplitter(rootSplitterWnd);

		if (rootHSplitter) {
			xml_node hsplitter = parent.append_child(node_element);
			hsplitter.name("hsplitter");
			counter += serializeSplitter<HSplitter>(frame, hsplitter, pane, rootHSplitter);
		} else
		if (rootVSplitter) {
			xml_node vsplitter = parent.append_child(node_element);
			vsplitter.name("vsplitter");
			counter += serializeSplitter<VSplitter>(frame, vsplitter, pane, rootVSplitter);
		}
	} else {
		Pane* paneWndPane = pane->currentPane;
		setActivePane(frame, paneWndPane, parent);
		for (int i=0; i<paneWndPane->clientCount(); ++i) {
			HWND clientWnd = paneWndPane->getClientView(i);

			xml_node client = parent.append_child(node_element);
			client.name("client");
			serializeClient(frame, client, clientWnd);
		}
	}

	return counter;
}

int DockTabSerializer::serializeSplitPane(DockSplitTab::Frame* frame, xml_node& parent, SplitPane* pane) {
	int counter = 0;

	RECT rc;

	GetClientRect(pane->m_hWnd, &rc);
	return serializeSplitPane(frame, parent, pane, &rc);
}

bool DockTabSerializer::serializeScreenset(DockSplitTab::Frame* frame, xml_parser& screensets_parser, int screenset)
{
	const CAtlMap<HWND, SplitPane*>::CPair* pair;
	int wc = 0;

	std::stringstream out;
	out << screenset;
	std::string element_name = "gui_" + out.str();

	xml_node doc = screensets_parser.document();

	xml_node gui = doc.first_element_by_name(element_name);
	if (!gui.empty()) {
		xml_node::iterator i = doc.begin();
		while (i != doc.end()) {
			if (i->name() == element_name) break;
			++i;
		}
		doc.erase(i);
	}
	gui = doc.append_child(node_element);
	gui.name(element_name);
	gui.attribute("xmlns") = "http://www.zzub.org/buze/gui";

	RECT rc;
	HWND hMainFrmWnd = GetParent(frame->m_hWnd);
	GetWindowRect(hMainFrmWnd, &rc);

	gui.attribute("x") = rc.left;
	gui.attribute("y") = rc.top;
	gui.attribute("width") = rc.right - rc.left;
	gui.attribute("height") = rc.bottom - rc.top;

	if (IsZoomed(hMainFrmWnd)) {
		// TODO: her stod det noe intersting: http://www.blitzbasic.com/Community/posts.php?topic=42475
		gui.attribute("maximize") = true;
	}

	for (POSITION start = frame->splitPanes.GetStartPosition(); start != 0; ) {
		pair = frame->splitPanes.GetNext(start);
		SplitPane* pane = pair->m_value;
		Frame::DockPane* dockPane = 0;

		// is it a dock pane?
		for (int i = 0; i<4; ++i) {
			dockPane = frame->dockPanes[i];
			if (!dockPane) continue;
			if (dockPane->getPane() == pane) {
				break;
			}
			dockPane = 0;
		}

		if (dockPane != 0) {
			xml_node dock = gui.append_child(node_element);
			dock.name("dock");
			dock.append_attribute("side", (long)dockPane->getDockSide());
			dock.append_attribute("size", (long)frame->layout.getDockSize(dockPane->getDockSide()));
			dock.append_attribute("place", (long)frame->layout.getDockWidth(dockPane->getDockSide()));
			wc += serializeSplitPane(frame, dock, pane);
			continue;
		} 

		// is it a float frame?
		Frame::FloatFrame* floatFrame = frame->lookUpFloatFrameSplitPaneWnd(pane->m_hWnd);
		if (floatFrame != 0) {
			xml_node dock = gui.append_child(node_element);
			dock.name("float");
			RECT rc;
			floatFrame->GetWindowRect(&rc);
			dock.append_attribute("x", rc.left);
			dock.append_attribute("y", rc.top);
			wc += serializeSplitPane(frame, dock, pane, &rc);
			continue;
		}

		// is it the main frame?
		SplitPane* splitPane = frame->getMainPane();
		if (splitPane->m_hWnd == pane->m_hWnd) {
			xml_node main = gui.append_child(node_element);
			main.name("main");
			wc += serializeSplitPane(frame, main, pane);
			continue;
		}

		// if we get here something is wrong
	}

	xml_node classes = gui.append_child(node_element);
	classes.name("classes");
	for (std::map<std::string, DockTabClientFactory*>::iterator i = factories.begin(); i != factories.end(); ++i) {
		assert(i->second != 0);
		if (i->second == 0) continue;
		xml_node classNode = classes.append_child(node_element);
		classNode.name("class");
		classNode.append_attribute("name", i->first);
		classNode.append_attribute("place", (long)i->second->getPlace());
		classNode.append_attribute("side", (long)i->second->getDockSide());

		// save named rects
		std::map<int, RECT> namedRects;
		i->second->getFloatRects(namedRects);
		for (std::map<int, RECT>::iterator j = namedRects.begin(); j != namedRects.end(); ++j) {
			xml_node rectNode = classNode.append_child(node_element);
			rectNode.name("rect");
			rectNode.append_attribute("view_id", (long)j->first);
			rectNode.append_attribute("left", j->second.left);
			rectNode.append_attribute("right", j->second.right);
			rectNode.append_attribute("top", j->second.top);
			rectNode.append_attribute("bottom", j->second.bottom);
		}
	}

	return true;
}

DockSplitTab::ClientView* DockTabSerializer::parseCreateClient(DockSplitTab::Frame* frame, DockSplitTab::SplitPane* splitPane, pug::xml_node& client, std::vector<ClientView*>& current_views) {
	std::string className = client.attribute("class").value();

	DockTabClientFactory* factory = getClassFactory(className);
	if (!factory) {
		return 0;
	}

	//std::string data = "";
	//if (client.has_child_nodes())
	//	data = client.first_node(node_pcdata).value();

	int view_id = 0;
	try {
		if (client.has_attribute("view_id")) 
			view_id = boost::lexical_cast<int, std::string>((const char*)client.attribute("view_id"));
	} catch (boost::bad_lexical_cast& e) {}

	std::string caption = "";
	if (client.has_attribute("caption")) caption = (const char*)client.attribute("caption");

	bool got_recycled_view = false;
	for (std::vector<ClientView*>::iterator i = current_views.begin(); i != current_views.end(); ++i) {
		ClientView* clientView = *i;
		char clientView_className[1024];
		GetClassName(clientView->wnd, clientView_className, array_size(clientView_className));

		// screenset switching: transfer window over if it exists
		if ((className == clientView_className) && (caption == (const char*)clientView->caption) && view_id == clientView->viewId) {
			lastClientView = factory->recycleClientWindow(*i, frame->m_hWnd, view_id);
			got_recycled_view = true;
			current_views.erase(i);
			break;
		}
	}

	// create new window if we didn't recycle one
	if (!got_recycled_view) {
		lastClientView = factory->deserializeClientWindow(frame->m_hWnd, caption, view_id);
	}
	return lastClientView;
}

void DockTabSerializer::parseFlatDock(DockSplitTab::Frame* frame, xml_node& dock, std::vector<ClientView*>& current_views) {

	DockSide side = (DockSide)(long)dock.attribute("side");

	Frame::DockPane* dockPane = frame->dockPanes[side];
	if (!dockPane)
		dockPane = frame->createDockPane(side);

	SplitPane* splitPane = dockPane->getPane();
	// a splitpane contains a list of clients, or HSplitter or VSplitter
	xml_node floatNode; // not really in use here, used for parsing floating clients
	bool floatFirstClient = false;
	for (xml_node::iterator i = dock.begin(); i!= dock.end(); ++i) {
		std::string name = i->name();
		if (name == "hsplitter") {
			parseFlatSplitter(frame, splitPane, splitPane, *i, floatFirstClient, floatNode, true, current_views);
		} else
		if (name == "vsplitter") {
			parseFlatSplitter(frame, splitPane, splitPane, *i, floatFirstClient, floatNode, true, current_views);
		} else
		if (name == "client") {
			ClientView* clientView = parseCreateClient(frame, splitPane, *i, current_views);
			if (clientView == 0) {
				//MessageBox(frame->m_hWnd, "Cannot deserialize window class", i->attribute("class"), MB_OK);
				continue;
			}
			frame->registerClientView(clientView, splitPane, true);
			std::string caption = "";
			if (i->has_attribute("caption"))
				caption = (const char*)i->attribute("caption");
			splitPane->append(caption.c_str(), clientView->wnd);
			clientViewPaths[*i] = clientView;
		}
	}
}

HWND DockTabSerializer::parseFlatPane(DockSplitTab::Frame* frame, DockSplitTab::SplitPane*& owner, DockSplitTab::SplitPane*& parent, xml_node& pane, bool& floatFirstClient, xml_node& floatNode, bool dockable, std::vector<ClientView*>& current_views) {
	HWND lastPane = 0;
	for (xml_node::iterator i = pane.begin(); i!= pane.end(); ++i) {
		std::string name = i->name();
		if (name == "hsplitter") {
			lastPane = parseFlatSplitter(frame, owner, parent, *i, floatFirstClient, floatNode, dockable, current_views);
		} else
		if (name == "vsplitter") {
			lastPane = parseFlatSplitter(frame, owner, parent, *i, floatFirstClient, floatNode, dockable, current_views);
		} else
		if (name == "client") {
			ClientView* clientView = parseCreateClient(frame, owner, *i, current_views);
			if (clientView == 0) {
				//MessageBox(frame->m_hWnd, "Cannot deserialize window class", i->attribute("class"), MB_OK);
				continue;
			}
			std::string caption = "";
			if (i->has_attribute("caption"))
				caption = (const char*)i->attribute("caption");

			if (floatFirstClient) {
				floatFirstClient = false;
				RECT rcFloat;
				SetRect(&rcFloat, CW_USEDEFAULT, CW_USEDEFAULT, 200, 200 );

				if (floatNode.has_attribute("x"))
					rcFloat.left = (long)floatNode.attribute("x");
				if (floatNode.has_attribute("y"))
					rcFloat.top = (long)floatNode.attribute("y");

				if (floatNode.has_attribute("width"))
					rcFloat.right = rcFloat.left + (long)floatNode.attribute("width");
				if (floatNode.has_attribute("height"))
					rcFloat.bottom = rcFloat.top + (long)floatNode.attribute("height");

				///InflateRect(&rcFloat, -7, -27);
				// floatClientView does the opposite inside!
				if (rcFloat.left != CW_USEDEFAULT) { // megz.restoresizefix
					rcFloat.left += ::GetSystemMetrics(SM_CXFRAME);
					rcFloat.right -= ::GetSystemMetrics(SM_CXFRAME);
					rcFloat.top += ::GetSystemMetrics(SM_CYCAPTION);
					rcFloat.bottom -= (::GetSystemMetrics(SM_CYFRAME) + 24);
				}

				HWND floatWnd = frame->floatClientView(&rcFloat, clientView);
				Frame::FloatFrame* floatFrame = frame->lookUpFloatFrame(floatWnd);
				SplitPane* splitPane = floatFrame->getPane();
				owner = splitPane;
				parent = splitPane;
			} else {
				frame->registerClientView(clientView, owner, dockable);
				parent->append(caption.c_str(), clientView->wnd);
			}
			lastPane = clientView->wnd;
			clientViewPaths[*i] = clientView;
		}
	}
	return lastPane;
}

HWND DockTabSerializer::parseFlatSplitter(DockSplitTab::Frame* frame, DockSplitTab::SplitPane*& owner, DockSplitTab::SplitPane*& parent, xml_node splitter, bool& floatFirstClient, xml_node& floatNode, bool dockable, std::vector<ClientView*>& current_views) {
	for (xml_node::iterator i = splitter.children_begin(); i != splitter.children_end(); ++i) {
		if (i->has_name() && i->name() == (std::string)"pane")
			parseFlatPane(frame, owner, parent, *i, floatFirstClient, floatNode, dockable, current_views);
	}
	return 0;
}

void DockTabSerializer::parseFlatMain(DockSplitTab::Frame* frame, xml_node& main, std::vector<ClientView*>& current_views) {
	SplitPane* splitPane = frame->getMainPane();
	bool floatFirstClient = false;
	xml_node floatNode;
	parseFlatPane(frame, splitPane, splitPane, main, floatFirstClient, floatNode, false, current_views);
}

void DockTabSerializer::parseFlatFloat(DockSplitTab::Frame* frame, xml_node& flt, std::vector<ClientView*>& current_views) {
	bool floatFirstClient = true;
	SplitPane* parent = 0;
	SplitPane* owner = 0;
	// owner is set when first client is deserialized:
	parseFlatPane(frame, owner, parent, flt, floatFirstClient, flt, true, current_views);
	splitPanePaths[flt] = owner;
}

void DockTabSerializer::parseSplitMain(DockSplitTab::Frame* frame, xml_node& main) {
	SplitPane* mainPane = frame->getMainPane();
	parseSplitPane(frame, mainPane, main);
}

void DockTabSerializer::parseSplitFloat(DockSplitTab::Frame* frame, xml_node& flt) {
	SplitPane* floatPane = splitPanePaths[flt];
	parseSplitPane(frame, floatPane, flt);
}

void DockTabSerializer::parseSplitDock(DockSplitTab::Frame* frame, xml_node& dock) {
	DockSide side = (DockSide)(long)dock.attribute("side");

	Frame::DockPane* dockPane = frame->dockPanes[side];
	if (!dockPane) dockPane = frame->createDockPane(side);

	SplitPane* splitPane = dockPane->getPane();
	parseSplitPane(frame, splitPane, dock);
}

void DockTabSerializer::parseSplitPane(DockSplitTab::Frame* frame, DockSplitTab::SplitPane* owner, pug::xml_node& pane) {
	std::string activeCaption = pane.attribute("active").value();

	for (xml_node::iterator i = pane.begin(); i != pane.end(); ++i) {
		std::string name = i->name();
		if (name == "hsplitter") {
			parseSplitSplitter(frame, owner, *i, SplitPane::targetBottom);
		} else
		if (name == "vsplitter") {
			parseSplitSplitter(frame, owner, *i, SplitPane::targetRight);
		} else
		if (name == "client") {
			if (activeCaption == (std::string)i->attribute("caption").value()) {
				ClientView* cv = clientViewPaths[*i];
				if (cv == 0) continue;	// skip unopenable clients
				owner->setFocusTo(cv->wnd);
			}
		}
	}
}

void DockTabSerializer::parseSplitSplitter(DockSplitTab::Frame* frame, DockSplitTab::SplitPane* owner, pug::xml_node& hsplitter, DockSplitTab::SplitPane::TargetArea area) {
	std::vector<ClientView*> clients(2);
	long size = -1;

	if (hsplitter.has_attribute("size"))
		size = hsplitter.attribute("size");

	int index = 0;
	for (xml_node::iterator i = hsplitter.begin(); i != hsplitter.end(); ++i) {
		std::string name = i->name();
		if (name == "pane") {
			xml_node firstClient = i->first_element_by_name("client");
			ClientView* clientView = clientViewPaths[firstClient];
			// TODO: something can go wrong here if a client is missing
			if (firstClient.empty() || !clientView) continue;
			clients[index] = clientView;
			index++;
		}
	}

	// we currently support two pane-splitters only:
	assert(index == 2);

	// update splitter size and pane sizes
	HWND firstWnd = clients[0]->wnd;
	HWND secondWnd = clients[1]->wnd;

	HWND splitterWnd = owner->splitClientView(secondWnd, firstWnd, area);
	HSplitter* hs = owner->lookUpHSplitter(splitterWnd);
	VSplitter* vs = owner->lookUpVSplitter(splitterWnd);

	// we also need to update outer pane sizes
	Pane* pane1 = owner->lookUpPane(clients[0]->wnd);
	Pane* pane2 = owner->lookUpPane(clients[1]->wnd);

	xml_node& parent = hsplitter.parent();
	long parentWidth = parent.attribute("width");
	long parentHeight = parent.attribute("height");

	RECT rc1, rc2;

	RECT rcSplitter;
	SetRect(&rcSplitter, 0, 0, parentWidth-4, parentHeight-4);
	pane1->SetWindowPos(HWND_TOP, &rc1, SWP_NOMOVE|SWP_SHOWWINDOW);
	pane2->SetWindowPos(HWND_TOP, &rc2, SWP_NOMOVE|SWP_SHOWWINDOW);

	if (hs) {
		hs->SetSplitterRect(&rcSplitter, FALSE);
		hs->SetSplitterPos(size);
		hs->GetSplitterPaneRect(SPLIT_PANE_TOP, &rc1);
		hs->GetSplitterPaneRect(SPLIT_PANE_BOTTOM, &rc2);
	} else
	if (vs) {
		vs->SetSplitterRect(&rcSplitter, FALSE);
		vs->SetSplitterPos(size);
		vs->GetSplitterPaneRect(SPLIT_PANE_LEFT, &rc1);
		vs->GetSplitterPaneRect(SPLIT_PANE_RIGHT, &rc2);
	}

	// move clients that belong to other pane over from first pane
	index = 0;
	for (xml_node::iterator i = hsplitter.begin(); i != hsplitter.end(); ++i) {
		std::string name = i->name();
		if (name == "pane") {
			HWND testWnd = index == 0 ? firstWnd : secondWnd;
			for (xml_node::iterator j = i->begin(); j != i->end(); ++j) {
				std::string name = j->name();
				if (name == "client") {
					ClientView* cv = clientViewPaths[*j];
					assert(cv);
					if (cv->wnd != testWnd) {
						owner->moveCurrentTab(cv->wnd, testWnd);
					}
				}
			}
			++index;
		}
	}

	index = 0;
	for (xml_node::iterator i = hsplitter.begin(); i != hsplitter.end(); ++i) {
		std::string name = i->name();
		if (name == "pane") {
			parseSplitPane(frame, owner, *i);
		}
	}

	if (hs) hs->UpdateSplitterLayout();
	if (vs) vs->UpdateSplitterLayout();
}

void DockTabSerializer::parseDocks(DockSplitTab::Frame* frame, pug::xml_node& node) {
	for (xml_node::iterator i = node.begin(); i != node.end(); ++i) {
		std::string name = i->name();
		xml_node& dock = *i;
		if (name == "dock") {

			DockSide side = (DockSide)(long)dock.attribute("side");
			int coveredCorners = (long)dock.attribute("place");
			int size = (long)dock.attribute("size");
			if (!size) {
				if (side == dockLEFT || side == dockRIGHT)
					size = (long)dock.attribute("width"); else
				if (side == dockTOP || side == dockBOTTOM)
					size = (long)dock.attribute("height");

				if (!size)
					size = 200;
			}

			frame->layout.setDock(side, coveredCorners, size);

			continue;
		}
		parseDocks(frame, *i);
	}
}

bool DockTabSerializer::deserializeScreenset(DockSplitTab::Frame* frame, xml_parser& screensets_parser, int screenset, std::vector<ClientView*>& current_views)
{
	std::stringstream out;
	out << screenset;
	std::string element_name = "gui_" + out.str();

	xml_node gui = screensets_parser.document().first_element_by_name(element_name);

	RECT rc;

	// restore mainframe window state/size
	HWND hMainFrmWnd = GetParent(frame->m_hWnd);
	if (gui.has_attribute("maximize")) {
		ShowWindow(hMainFrmWnd, SW_MAXIMIZE);
	} else {
		GetWindowRect(hMainFrmWnd, &rc);
		if (gui.has_attribute("x"))
			rc.left = (long)gui.attribute("x");
		if (gui.has_attribute("y"))
			rc.top = (long)gui.attribute("y");
		if (gui.has_attribute("width"))
			rc.right = rc.left + (long)gui.attribute("width");
		if (gui.has_attribute("height"))
			rc.bottom = rc.top + (long)gui.attribute("height");
		MoveWindow(hMainFrmWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		ShowWindow(hMainFrmWnd, SW_NORMAL);
	}

	// set up docks -- no windows created yet
	parseDocks(frame, gui);

	// add all docked client views
	for (xml_node::iterator i = gui.begin(); i != gui.end(); ++i) {
		std::string name = i->name();
		if (name == "dock") 
			parseFlatDock(frame, *i, current_views); else
		if (name == "main") 
			parseFlatMain(frame, *i, current_views); else
		if (name == "float") 
			parseFlatFloat(frame, *i, current_views);
	}

	// update frame layout after docks are open and populated with clients, so the following splitting has correct sizing values to work with
	GetClientRect(frame->m_hWnd, &rc);
	BOOL bHandled = TRUE;
	frame->OnSize(0, 0, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top), bHandled);

	xml_node classes;
	// split docks
	for (xml_node::iterator i = gui.begin(); i != gui.end(); ++i) {
		std::string name = i->name();
		if (name == "dock") 
			parseSplitDock(frame, *i); else
		if (name == "main") 
			parseSplitMain(frame, *i); else
		if (name == "float") 
			parseSplitFloat(frame, *i);
		if (name == "classes")
			classes = *i;
	}

	// fetch defaults from classes
	for (xml_node::iterator classNode = classes.begin(); classNode != classes.end(); ++classNode) {
		std::map<std::string, DockTabClientFactory*>::iterator i = factories.find(classNode->attribute("name"));
		if (i == factories.end()) continue;
		i->second->setPlace((DockSplitTab::FramePlace)(long)classNode->attribute("place"));
		i->second->setSide((DockSplitTab::DockSide)(long)classNode->attribute("side"));

		for (xml_node::iterator rectNode = classNode->begin(); rectNode != classNode->end(); ++rectNode) {
			std::string rectName = rectNode->name();
			int view_id;
			try {
				view_id = boost::lexical_cast<int, std::string>(rectNode->attribute("view_id").value());
			} catch (boost::bad_lexical_cast& e) {
				// invalid view_id, assume the best
				view_id = 0;
			}
			if (rectName != "rect") continue;
			RECT rc;
			rc.left = rectNode->attribute("left");
			rc.right = rectNode->attribute("right");
			rc.top = rectNode->attribute("top");
			rc.bottom = rectNode->attribute("bottom");
			i->second->setFloatRect(view_id, rc);

		}
	}

	return true;
}

void DockTabSerializer::registerFactory(std::string const& className, DockSplitTab::DockTabClientFactory* factory) {
	factories[className] = factory;
}

DockSplitTab::DockTabClientFactory* DockTabSerializer::getClassFactory(std::string const& className) {
	std::map<std::string, DockTabClientFactory*>::iterator i = factories.find(className);
	if (i == factories.end()) return 0;
	return i->second;
}

}
