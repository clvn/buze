#pragma once

#include "pugxml.h"

namespace DockSplitTab {

class DockTabClientFactory;

class DockTabSerializer
{
    std::map<std::string, DockTabClientFactory*> factories;
    pug::xml_parser xml;
    std::map<void*, DockSplitTab::ClientView*> clientViewPaths;
    std::map<void*, DockSplitTab::SplitPane*> splitPanePaths;
    ClientView* lastClientView;

// serializer:

    template <class T>
    int serializeSplitter(DockSplitTab::Frame* frame, pug::xml_node& parent, DockSplitTab::SplitPane* pane, T* splitter) {
        int counter = 0;
        parent.append_attribute("size", (long)splitter->GetSplitterPos());
        int singlePane = splitter->GetSinglePaneMode();
        if (singlePane != SPLIT_PANE_NONE) {
            pug::xml_node singlepane = parent.append_child(node_element);
            singlepane.name("pane");
            HWND singlePaneWnd = splitter->GetSplitterPane(singlePane);
            counter += serializeSplitterPaneViews(frame, singlepane, pane, singlePaneWnd);
        } else {
            pug::xml_node pane1 = parent.append_child(node_element);
            pane1.name("pane");
            pug::xml_node pane2 = parent.append_child(node_element);
            pane2.name("pane");
            HWND leftTopWnd = splitter->GetSplitterPane(0);
            HWND rightBottomWnd = splitter->GetSplitterPane(1);
            RECT rclt, rcrb;
            splitter->GetSplitterPaneRect(0, &rclt);
            splitter->GetSplitterPaneRect(1, &rcrb);
            pane1.append_attribute("width", rclt.right - rclt.left);
            pane1.append_attribute("height", rclt.bottom - rclt.top);
            pane2.append_attribute("width", rcrb.right - rcrb.left);
            pane2.append_attribute("height", rcrb.bottom - rcrb.top);
            counter += serializeSplitterPaneViews(frame, pane1, pane, leftTopWnd);
            counter += serializeSplitterPaneViews(frame, pane2, pane, rightBottomWnd);
        }

        return counter;
    }

    void serializeClient(DockSplitTab::Frame* frame, pug::xml_node& parent, HWND clientWnd);
    int serializeSplitterPaneViews(DockSplitTab::Frame* frame, pug::xml_node& parent, DockSplitTab::SplitPane* pane, HWND hWnd);
    bool splitterPaneRootFilter(DockSplitTab::SplitPane* pane, HWND paneWnd, std::map<HWND, bool>& allSplitters);

    template <class T>
    void splitterRootFilter(DockSplitTab::SplitPane* pane, T* splitter, std::map<HWND, bool>& allSplitters) {
        int singlePane = splitter->GetSinglePaneMode();
        if (singlePane != SPLIT_PANE_NONE) {
            HWND singlePaneWnd = splitter->GetSplitterPane(singlePane);
            splitterPaneRootFilter(pane, singlePaneWnd, allSplitters);
        } else {
            HWND leftTopWnd = splitter->GetSplitterPane(0);
            HWND rightBottomWnd = splitter->GetSplitterPane(1);
            splitterPaneRootFilter(pane, leftTopWnd, allSplitters);
            splitterPaneRootFilter(pane, rightBottomWnd, allSplitters);
        }

    }

    HWND findRootSplitter(DockSplitTab::SplitPane* pane);
    int serializeSplitPane(DockSplitTab::Frame* frame, pug::xml_node& parent, DockSplitTab::SplitPane* pane);
	int serializeSplitPane(DockSplitTab::Frame* frame, pug::xml_node& parent, DockSplitTab::SplitPane* pane, RECT *rc);

// deserializer

    // pass 1: set up docls
    void parseDocks(DockSplitTab::Frame* frame, pug::xml_node& gui);

    // pass 2: create client views
    DockSplitTab::ClientView* parseCreateClient(DockSplitTab::Frame* frame, DockSplitTab::SplitPane* owner, pug::xml_node& gui, std::vector<ClientView*>& current_views);
    void parseFlatDock(DockSplitTab::Frame* frame, pug::xml_node& gui, std::vector<ClientView*>& current_views);
    void parseFlatMain(DockSplitTab::Frame* frame, pug::xml_node& gui, std::vector<ClientView*>& current_views);
    void parseFlatFloat(DockSplitTab::Frame* frame, pug::xml_node& gui, std::vector<ClientView*>& current_views);
    HWND parseFlatPane(DockSplitTab::Frame* frame, DockSplitTab::SplitPane*& owner, DockSplitTab::SplitPane*& parent, pug::xml_node& gui, bool& floatFirstClient, pug::xml_node& floatNode, bool dockable, std::vector<ClientView*>& current_views);
    HWND parseFlatSplitter(DockSplitTab::Frame* frame, DockSplitTab::SplitPane*& owner, DockSplitTab::SplitPane*& parent, pug::xml_node gui, bool& floatFirstClient, pug::xml_node& floatNode, bool dockable, std::vector<ClientView*>& current_views);

    // pass 3: create splits
    void parseSplitDock(DockSplitTab::Frame* frame, pug::xml_node& gui);
    void parseSplitFloat(DockSplitTab::Frame* frame, pug::xml_node& gui);
    void parseSplitMain(DockSplitTab::Frame* frame, pug::xml_node& gui);
    void parseSplitPane(DockSplitTab::Frame* frame, DockSplitTab::SplitPane* owner, pug::xml_node& gui);
    void parseSplitSplitter(DockSplitTab::Frame* frame, DockSplitTab::SplitPane* owner, pug::xml_node& gui, DockSplitTab::SplitPane::TargetArea area);

// interface
  public:

    DockTabSerializer();

	bool serializeScreenset(DockSplitTab::Frame* frame, pug::xml_parser& screensets_parser, int screenset);
	bool deserializeScreenset(DockSplitTab::Frame* frame, pug::xml_parser& screensets_parser, int screenset, std::vector<ClientView*>& current_views);

    void registerFactory(std::string const& className, DockSplitTab::DockTabClientFactory* factory);
    DockTabClientFactory* getClassFactory(std::string const& className);
};

}
