#include "stdafx.h"
#include <wtl/atlframe.h>
#include <wtl/atlctrls.h>
#include <wtl/atldlgs.h>
#include <wtl/atlctrlw.h>

#include "DockTabFrame.h"

int DockSplitTab::Frame::DockPane::instanceCount = 0;
CBrush DockSplitTab::Frame::DockPane::systemFrameBrush;
CBrush DockSplitTab::Frame::DockPane::activeCaptionBrush;
CBrush DockSplitTab::Frame::DockPane::inactiveCaptionBrush;
CBrush DockSplitTab::Frame::DockPane::activeBorderBrush;
CBrush DockSplitTab::Frame::DockPane::inactiveBorderBrush;
		
COLORREF DockSplitTab::Frame::DockPane::activeCaptionTextColor;
COLORREF DockSplitTab::Frame::DockPane::inactiveCaptionTextColor;
		
int   DockSplitTab::Frame::DockPane::systemCxFrame;
int   DockSplitTab::Frame::DockPane::systemCyFrame;
int   DockSplitTab::Frame::DockPane::systemCaptionHeight;
CFont DockSplitTab::Frame::DockPane::systemCaptionFont;
		
HCURSOR DockSplitTab::Frame::DockPane::verCursor;
HCURSOR DockSplitTab::Frame::DockPane::horCursor;
