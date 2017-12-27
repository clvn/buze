#include "stdafx.h"
#include <iomanip>
#include <strstream>
#include <limits>
#include <fstream>
#include "resource.h"
#include "utils.h"
#include "GraphCtrl.h"

static const int MAX_MACHINE_FONTS = 10;

// TODO:
//  - connection amp tooltip

// ---------------------------------------------------------------------------------------------------------------
// OPTIONS
// ---------------------------------------------------------------------------------------------------------------

//static const int ID_MACHINE_UPDATE_TIMER = 1000;
static const float MAX_MACHINE_SCALE = 4.0f;
static const float MIN_MACHINE_SCALE = 0.3f;
static const int machineMovementPixel = 1;
static const int machineMovementStep = 7;
static const float machine_width = 100.0f;
static const float machine_height = 50.0f;
static const float machine_width_minimized = 12.0f;
static const float machine_height_minimized = 12.0f;
static const float led_width = 8.0f;
static const float led_height = 8.0f;
static const int led_width_minimum = 3;
static const int led_height_minimum = 3;

LRESULT CGraphCtrl::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	//m_hWnd = hWnd;

	LRESULT lRes = DefWindowProc();

	draggingMachine = 0;
	mbuttonState = false;
	useSkins = false;
	moveType = MachineViewMoveNothing;
	view_offset_x = 0.0;
	view_offset_y = 0.0;
	selectedConnectionValid = false;
	typepoint.x = typepoint.y = -1;

	SetRect(&prevDragRect, -1, -1, -1, -1);
	prevConnectPos.x = prevConnectPos.y = -1;
	moveCurrentPoint.x = moveCurrentPoint.y = 0;
	moveFromPoint.x = moveFromPoint.y = 0;
	dirtyOffscreenBitmap = true;
	dirtyMachines = dirtySelectionRectangle = dirtyStatus = dirtyVolumeSlider = dirtyConnecting = dirtyConnectTargetMenu = false;
	connectTargetMachine = 0;
	initialVolume = initialPan = 1.0f;
	showConnectionText = false;

	offscreenDC.CreateCompatibleDC(0);

	for (int i = 0; i < MAX_MACHINE_FONTS; i++) {
		machineFont[i].CreateFont((i + 2) * 4, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "");
	}

	UpdateFromTheme();
	connectTargetMenu.SetPaintWindow(m_hWnd);

	volumetip.Create(m_hWnd, rcDefault, 0, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP);
	volumetip.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	GetClientRect(&clientSize); //for CMachineView::OnSize's -- if (!scale_machine_points)
	volumeinfo = boost::shared_ptr<CToolInfo>(new CToolInfo(TTF_TRACK | TTF_ABSOLUTE, m_hWnd, 0, &clientSize));
	volumetip.AddTool(volumeinfo.get());

	searchbox.Create(*this, rcDefault, _T(""), WS_CHILD | WS_VISIBLE | CBS_SIMPLE | CBS_AUTOHSCROLL, 0, (UINT)0/*ID_SEARCHBOX*/);
	searchbox.Hide();
	// parent adds items to search for and calls the indexer

	return lRes;
}

LRESULT CGraphCtrl::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	// must call DestroyWindow on the tooltip in OnDestroy to prevent leaking gdi font handles
	volumetip.DestroyWindow();
	return 0;
}

LRESULT CGraphCtrl::OnForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

void CGraphCtrl::PaintConnectionTargetMenu(CDC& dc) {
	if (connectTargetMachine == 0) return;
	if (connectTargetMenu.IsEmpty()) return;
	connectTargetMenu.PaintMenu(dc);
}

void CGraphCtrl::PaintNode(CDC& dc, CGraphNode* plugin, bool ledOnly) {

	int halfWidth = (int)((GetMachineWidth(plugin) / 2) * scale);
	int halfHeight = (int)((GetMachineHeight(plugin) / 2) * scale);

	RECT mRect;
	GetMachineRect(plugin, &mRect);
	
	// draw outer box
	CPenHandle oldPen = dc.SelectPen(borderPen);
	CBrushHandle oldBrush = dc.SelectBrush(GetMachineBrush(plugin));
	
	//zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(plugin);
	//PLUGININFO& pinfo = document->getPluginData(plugin);
	
	//if (useSkins && pinfo.skin && pinfo.skin->skin && !zzub_plugin_get_minimize(plugin)) {
	//	dc.StretchBlt(mRect.left, mRect.top, halfWidth*2, halfHeight*2, pinfo.skin->skinDC, 0, 0, pinfo.skin->width, pinfo.skin->height, SRCCOPY);
	//} else {
		dc.Rectangle(mRect.left, mRect.top, mRect.right, mRect.bottom);
	//}
	
	// draw machine label 
	// always draw text if skins are enabled, since big skins may overwrite the text
	int fontIndex = (int)((scale / MAX_MACHINE_SCALE) * ((float)MAX_MACHINE_FONTS - 0.5f));
	CFont prevFont = dc.SelectFont(machineFont[fontIndex]);

	UINT prevAlign = dc.SetTextAlign(TA_CENTER);
	int prevBkMode = dc.SetBkMode(TRANSPARENT);

	std::string label = plugin->name;//zzub_plugin_get_name(plugin);
	//if (player->solo_plugin != -1 && player->solo_plugin != plugin)
	//	label = "["+label+"]";

	if (plugin->muted)
		label = "("+label+")";
	//if (zzub_player_get_midi_lock(player) && plugin == zzub_player_get_midi_plugin(player))
	//	label = "*"+label;

	int adjustY = plugin->minimized ? (int)-GetFontHeight() : 0;

	//if (useSkins && pinfo.skin && pinfo.skin->skin && !zzub_plugin_get_minimize(plugin)) {
	//	dc.SetTextColor(pinfo.skin->textColor);
	//} else {
		dc.SetTextColor(machineTextColor);
	//}
	dc.TextOut(mRect.left + halfWidth, mRect.top + halfHeight - (int)(GetFontHeight() / 2) + adjustY, label.c_str(), (int)label.length());

	dc.SelectFont(prevFont);
	dc.SetTextAlign(prevAlign);
	dc.SetBkMode(prevBkMode);

	if (!ledOnly) {
/*		connection* masterConnection=player->getMaster()->getConnection(plugin, zzub::connection_type_audio);
		// hvis denne går til master, så tegner vi connection-panning-fixing:
		if (masterConnection) {
			// draw panning box inside normal box
			dc.SelectBrush(getMachinePanningBrush(plugin));
			dc.Rectangle(mRect.left+2, mRect.bottom-(10.0*scale), mRect.right-2, mRect.bottom-2);
		
			// and the little pan-slider:
			RECT pRect;
			dc.SelectBrush(panHandleBrush);
			getPanSliderRect(masterConnection, &pRect);
			dc.Rectangle(&pRect);
		}
*/
		if (IsSelectedMachine(plugin->id)) {
			RECT fRect = mRect;
			InflateRect(&fRect, 2, 2);
			dc.SetTextColor(0);
			dc.DrawFocusRect(&fRect);
			InflateRect(&fRect, 1, 1);
			dc.DrawFocusRect(&fRect);
		}
	}

	dc.SelectPen(oldPen);
	dc.SelectBrush(oldBrush);
}

CGraphNode* CGraphCtrl::GetNode(int id) {
	for (std::vector<boost::shared_ptr<CGraphNode> >::iterator i = nodes.begin(); i != nodes.end(); ++i) {
		if ((*i)->id == id) return i->get();
	}
	return 0;
}

CGraphEdge* CGraphCtrl::GetEdge(int to_id, int from_id, edgetype type) {
	for (std::vector<boost::shared_ptr<CGraphEdge> >::iterator i = edges.begin(); i != edges.end(); ++i) {
		CGraphEdge* edge = i->get();
		if (edge->from_node_id == from_id && edge->to_node_id == to_id && edge->type == type)
			return edge;
	}
	return 0;
}

CGraphEdge* CGraphCtrl::GetSelectedEdge(edgetype type) {
	if (!selectedConnectionValid)
		return 0;
	return GetEdge(selectedConnection.to_node_id, selectedConnection.from_node_id, type);
}

CGraphNodePair* CGraphCtrl::GetSelectedEdge() {
	if (!selectedConnectionValid)
		return 0;
	return &selectedConnection;
}


void CGraphCtrl::PaintNodes(CDC& dc) {
	CPenHandle oldPen = dc.SelectPen(audioLinePen);

	for (std::vector<boost::shared_ptr<CGraphEdge> >::iterator i = edges.begin(); i != edges.end(); ++i) {
		CGraphEdge* edge = i->get();
		
		// render connections for this machine
		CGraphNode* from_plugin = GetNode(edge->from_node_id);
		CGraphNode* to_plugin = GetNode(edge->to_node_id);

		if (GetMachineHideIncomingConnections(edge->to_node_id) && !IsSelectedMachine(edge->to_node_id)) continue;

		POINT mPoint;
		GetMachinePoint(to_plugin, &mPoint);

		int type = edge->type;
		int amp = (int)(edge->amp * 0x4000);

		CPen* pen = &borderPen;
		if (edge->is_indirect) {
			dc.SelectPen(hiddenLinePen);
			pen = &hiddenLinePen;
		} else
		if (type == edge_audio) {
			dc.SelectPen(audioLinePen);
			pen = &audioLinePen;
		} else
		if (type == edge_event) {
			dc.SelectPen(eventLinePen);
			pen = &eventLinePen;
		} else
		if (type == edge_note) {
			dc.SelectPen(eventLinePen);
			pen = &eventLinePen;
		} else
		if (type == edge_midi) {
			dc.SelectPen(midiLinePen);
			pen = &midiLinePen;
		} else {
			continue;
		}

		dc.MoveTo(mPoint.x, mPoint.y);
		POINT sPoint;
		GetMachinePoint(from_plugin, &sPoint);
		dc.LineTo(sPoint.x, sPoint.y);

		std::string description = "";
		if (showConnectionText) {
			description = edge->description;
		}

		if (selectedConnectionValid && GetSelectedMachines() == 0 && selectedConnection.from_node_id == edge->from_node_id && selectedConnection.to_node_id == edge->to_node_id) {
			pen = &selectedBorderPen;
		}

		PaintDirectionTriangle(dc, mPoint, sPoint, amp, pen, description);

/*		std::vector<zzub_plugin_t*>& hiddenplugs = hiddenConnections[plugin];
		for (int j = 0; j < (int)hiddenplugs.size(); j++) {
			zzub_plugin_t* from_plugin = hiddenplugs[j];
			dc.SelectPen(hiddenLinePen); // TODO: make a better guesstimate what type it is?
			dc.MoveTo(mPoint.x, mPoint.y);
			POINT sPoint;
			getMachinePoint(from_plugin, &sPoint);
			dc.LineTo(sPoint.x, sPoint.y);
		}*/
	}

	dc.SelectPen(oldPen);

	for (std::vector<boost::shared_ptr<CGraphNode> >::iterator i = nodes.begin(); i != nodes.end(); ++i) {
		PaintNode(dc, i->get(), false);
	}
}


LRESULT CGraphCtrl::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	CPaintDC screenDC(m_hWnd);

	RECT rcClip;
	screenDC.GetClipBox(&rcClip);

	RECT rcClient;
	GetClientRect(&rcClient);

	if (dirtyOffscreenBitmap) {
		if (offscreenBitmap.m_hBitmap)
			offscreenBitmap.DeleteObject();

		offscreenBitmap.CreateCompatibleBitmap(screenDC, rcClient.right, rcClient.bottom);
		offscreenDC.SelectBitmap(offscreenBitmap);

		dirtyMachines = true;
		dirtyOffscreenBitmap = false;

	}

	if (dirtyBackground)
		dirtyMachines = true;

	if (dirtyMachines) {
		if (dirtyBackground)
			offscreenDC.FillSolidRect(rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, backgroundColor); else
			offscreenDC.FillSolidRect(rcClip.left, rcClip.top, rcClip.right - rcClip.left, rcClip.bottom - rcClip.top, backgroundColor);
		PaintNodes(offscreenDC);

		SetRect(&prevDragRect, -1, -1, -1, -1);
		dirtyStatus = true;
		if (moveType == MachineViewMoveConnectMachines) { 
			// make sure the connection menu stays visible if the mv repaints while connecting
			dirtyConnectTargetMenu = true;
			dirtyConnecting = true;
			prevConnectPos.x = prevConnectPos.y = -1;
		} else
		if (moveType == MachineViewMoveSelectRectangle) {
			dirtySelectionRectangle = true;
		} else
		if (moveType == MachineViewMoveVolumeSlider) {
			dirtyVolumeSlider = true;
		}
	}

	if (dirtyStatus)
		PaintMachineStatus(offscreenDC);

	if (dirtyVolumeSlider)
		PaintVolume(offscreenDC);

	if (dirtySelectionRectangle)
		PaintSelectionRect(offscreenDC);

	if (dirtyConnectTargetMenu)
		PaintConnectionTargetMenu(offscreenDC);

	if (dirtyConnecting)
		PaintConnectionLine(offscreenDC);

	dirtyBackground =
	dirtyMachines =
	dirtySelectionRectangle =
	dirtyStatus =
	dirtyVolumeSlider =
	dirtyConnecting =
	dirtyConnectTargetMenu =
		false;

	screenDC.BitBlt(
		rcClip.left, rcClip.top, rcClip.right - rcClip.left, rcClip.bottom - rcClip.top,
		offscreenDC, rcClip.left, rcClip.top, SRCCOPY
	);

	return DefWindowProc();
}

LRESULT CGraphCtrl::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return 1;
}

void CGraphCtrl::PaintMachineStatus(CDC& dc) {
	CPenHandle oldPen = dc.SelectPen(borderPen);

	for (std::vector<boost::shared_ptr<CGraphNode> >::iterator i = nodes.begin(); i != nodes.end(); ++i) {
		CGraphNode* plugin = i->get();
		RECT rc;
		GetStatusRect(plugin, &rc);

		/*if (useSkins && pinfo.skin && pinfo.skin->led && !zzub_plugin_get_minimize(plugin)) {
			if (newState) {
				dc.StretchBlt(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, pinfo.skin->skinLedDC, 0, 0, pinfo.skin->ledWidth, pinfo.skin->ledHeight, SRCCOPY);
			} else {
				// draw a rectangle from the machine skin where the led is drawn
				drawMachine(dc, plugin, true);
			}
		} else {*/
		CBrushHandle prevBrush = dc.SelectBrush(GetMachineLedBrush(plugin));
		dc.Rectangle(&rc);
		dc.SelectBrush(prevBrush);
	}

	dc.SelectPen(oldPen);
}

void CGraphCtrl::PaintVolume(CDC& dc) {
	if (moveType != MachineViewMoveVolumeSlider) return ;
	RECT vsr;
	GetVolumeSliderRect(selectedConnection, &vsr);

	
	CGraphEdge* audioconn = GetSelectedEdge(edge_audio);
	assert(audioconn != 0);

	PaintVolumeSlider(dc, vsr, audioconn);
}

void CGraphCtrl::PaintSelectionRect(CDC& dc) {
	RECT rc;
	GetDragRect(&rc);

	SIZE sz = { 2, 2 };

	if (prevDragRect.left==-1) {
		dc.DrawDragRect(&rc, sz, 0, sz);
	} else {
		dc.DrawDragRect(&rc, sz, &prevDragRect, sz);
	}

	prevDragRect = rc;
}

void CGraphCtrl::PaintConnectionLine(CDC& dc) {
	if (draggingMachine != 0 && moveType == MachineViewMoveConnectMachines) {
		POINT pt;
		GetMachinePoint(draggingMachine, &pt);

		CPenHandle oldPen = dc.SelectPen(audioLinePen);
		int prevROP = dc.SetROP2(R2_NOT);

		if (prevConnectPos.x != -1) {
			dc.MoveTo(pt.x, pt.y, 0);
			dc.LineTo(prevConnectPos.x, prevConnectPos.y);
		}

		dc.MoveTo(pt.x, pt.y, 0);
		dc.LineTo(connectPos.x, connectPos.y);

		dc.SetROP2(prevROP);
		dc.SelectPen(oldPen);

		prevConnectPos = connectPos;
	}
}

LRESULT CGraphCtrl::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	int d = (signed short)HIWORD(wParam);

	if (IsCtrlDown()) {
		if (d < 0) {
			if (scale < MAX_MACHINE_SCALE) {
				scale += 0.1f;
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_SCALE_CHANGE), 0);
				InvalidateMachines();
			}
		} else {
			if (scale > MIN_MACHINE_SCALE) {
				scale -= 0.1f;
				GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_SCALE_CHANGE), 0);
				InvalidateMachines();
			}
		}
		CalcFontHeight();
	}
	return 0;
}

LRESULT CGraphCtrl::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	// if we're inside one of our heroic machines, lets set capture and move
	draggingMachine = 0;

	if (dirtyBackground || dirtyMachines || dirtyOffscreenBitmap) return 0;

	POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	typepoint = pt;
	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_CLICK), 0);

	// copy selection since we clear it below, and we want to restore it if this was only a double click
	selectedMachinesCopy = selectedMachines;

	if (MouseDownMachine(pt, wParam)) return 0;
	if (MouseDownConnection(pt, wParam)) return 0;
	if (MouseDownBackground(pt, wParam)) return 0;
	return 0;
}

LRESULT CGraphCtrl::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	//int defaultAmp = configuration->getDefaultAmp();

	POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	typepoint = pt;
	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_CLICK), 0);

	if (searchbox.GetWindowLong(GWL_STYLE) & WS_VISIBLE) {
		searchbox.Hide();
		SetFocus();
	}

	if (moveType == MachineViewMoveConnectMachines) {
		ReleaseCapture();
		if ((connectTargetMachine != 0) && (!connectTargetMenu.IsEmpty())) {
			int wID = connectTargetMenu.HitTestID(pt.x, pt.y, true);
			connectTargetMenu.m_parent.SendMessage(WM_COMMAND, MAKELONG(wID, 0), 0);
		}

		draggingMachine = 0;
		prevConnectPos.x = prevConnectPos.y = -1;
		connectTargetMachine = 0;
		dirtyMachines = true;
		Invalidate(FALSE);
	} else
	if (moveType == MachineViewMovePanSlider) {
		ReleaseCapture();
		draggingMachine = 0;
		InvalidateMachines();
	} else
	if (moveType == MachineViewMoveSelectedMachines) {
		//RECT rc;
		//GetClientScale(&rc);
		//float dx = ((float)pt.x - beginDragPt.x) / (float)rc.right;
		//float dy = ((float)pt.y - beginDragPt.y) / (float)rc.bottom;

		//float mdx = dx * 2 / scale;
		//float mdy = dy * 2 / scale;
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_NODE_MOVE), 0);

		ReleaseCapture();
		draggingMachine = 0;
		moveCurrentPoint.x = moveCurrentPoint.y = 0;
		moveFromPoint.x = moveFromPoint.y = 0;
		beginDragPt.x = beginDragPt.y = 0;
	} else
	if (moveType == MachineViewMoveAllMachines) {
		RECT rc;
		GetClientScale(&rc);
		int dx = pt.x - beginDragPt.x;
		int dy = pt.y - beginDragPt.y;
		view_offset_x += (double)dx / scale / (double)rc.right;
		view_offset_y += (double)dy / scale / (double)rc.bottom;

		ReleaseCapture();
		draggingMachine = 0;
		moveCurrentPoint.x = moveCurrentPoint.y = 0;
		moveFromPoint.x = moveFromPoint.y = 0;
		beginDragPt.x = beginDragPt.y = 0;
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_VIEW_MOVE), 0);
	} else
	if (moveType == MachineViewMoveVolumeSlider) {
		ReleaseCapture();
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_CONN_AMP_SET), 0);
		moveFromPoint.x = moveFromPoint.y = 0;
		volumetip.TrackActivate(volumeinfo.get(), FALSE);
		InvalidateMachines();
	} else
	if (moveType == MachineViewMoveSelectRectangle) {
		ReleaseCapture();
		SetRect(&prevDragRect, -1, -1, -1, -1);
		dirtyBackground = true;
		dirtySelectionRectangle = false;
		Invalidate(FALSE);
	} else 
	if (moveType == MachineViewMoveNothing) {
		CGraphNodePair conn;
		if (GetConnectionAtPt(pt, &conn)) {
			GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_CONN_CLICK), 0);
		}
	} else {
		assert(false);
	}

	moveType = MachineViewMoveNothing;
	return 0;
}


LRESULT CGraphCtrl::OnLButtonDblClick(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	if (dirtyBackground || dirtyMachines || dirtyOffscreenBitmap) return 0;

	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

	RECT mRect;
	CGraphNode* propMachine = GetMachineAtPt(pt, &mRect);

	if (propMachine != 0) {
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_NODE_DBLCLICK), 0);
	}

	// double clicked a machine, restore selected machines
	ClearSelectedMachines();
	selectedMachines = selectedMachinesCopy;
	//if (selectedMachinesCopy.size()) zzub_player_set_midi_plugin(player, selectedMachinesCopy.back());
	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_NODE_SELECT), 0);
	InvalidateMachines(false);

	return 0;
}

LRESULT CGraphCtrl::OnMButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {

	if (dirtyBackground || dirtyMachines || dirtyOffscreenBitmap) return 0;

	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	
	CGraphNodePair conn;
	if (GetConnectionAtPt(pt, &conn)) {
		SelectConnection(conn);
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_CONN_DISCONNECT), 0);
	} else {
		mbuttonState = true;
	}
	
	return 0;
}

LRESULT CGraphCtrl::OnMButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	mbuttonState = false;
	return 0;
}

LRESULT CGraphCtrl::OnRButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	typepoint = pt;
	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_CLICK), 0);
	bHandled = FALSE;
	return 0;
}

LRESULT CGraphCtrl::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	if ((pt.x == mousemove_previous.x) && (pt.y == mousemove_previous.y)) return 0;
	mousemove_previous = pt;

	if (moveType == MachineViewMoveConnectMachines) {
		MouseMoveConnectMachines(pt, wParam);
	} else
	if (moveType==MachineViewMovePanSlider) {
		InvalidateMachines();
	} else
	if (moveType == MachineViewMoveSelectedMachines) {
		MouseMoveSelectedMachines(pt, wParam);
	} else
	if (moveType == MachineViewMoveVolumeSlider) {
		MouseMoveVolumeSlider(pt, wParam);
	} else
	if (moveType == MachineViewMoveAllMachines) {
		MouseMoveAllMachines(pt, wParam);
	} else 
	if (moveType==MachineViewMoveSelectRectangle) {
		MouseMoveSelectRectangle(pt, wParam);
	}

	return 0;

}

LRESULT CGraphCtrl::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	if (GetSelectedMachines() == 0) {
		if ((wParam >= 'a' && wParam <= 'z') || (wParam >= 'A' && wParam <= 'Z')) {
			TCHAR key[2] = { (TCHAR)wParam, 0 };
			//RECT rc;

			//GetClientScale(&rc);
			//POINT pt = graphCtrl.typepoint;
			//createMachinePosX = (((float)pt.x / (float)rc.right) - 0.5f) * 2.0f / graphCtrl.scale;
			//createMachinePosY = (((float)pt.y / (float)rc.bottom) - 0.5f) * 2.0f / graphCtrl.scale;

			searchbox.Show(GetParent(), typepoint, key);
			return 0;
		}
	}
	return OnForwardMessage(uMsg, wParam, lParam, bHandled);
	//return DefWindowProc();
}

void CGraphCtrl::MouseMoveVolumeSlider(POINT pt, WPARAM wParam) {
	moveCurrentPoint = pt;

	float newVolume = initialVolume + (float)(moveFromPoint.y - pt.y) / 54;
	newVolume = std::min(2.0f, std::max(0.0f, newVolume));

	CGraphEdge* audioconn = GetSelectedEdge(edge_audio);
	audioconn->amp = newVolume;

	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_CONN_AMP_TRACK), 0);

	UpdateConnectionToolTip();
	InvalidateConnectionVolume();
}

void CGraphCtrl::UpdateFromTheme() {
	if (ampBgBrush) ampBgBrush.DeleteObject();
	if (ampHandleBrush) ampHandleBrush.DeleteObject();
	if (arrowHighBrush) arrowHighBrush.DeleteObject();
	if (arrowBrush) arrowBrush.DeleteObject();
	if (arrowLowBrush) arrowLowBrush.DeleteObject();
	// background -- no deleteobject needed
	if (effectBgBrush) effectBgBrush.DeleteObject();
	if (effectLedOffBrush) effectLedOffBrush.DeleteObject();
	if (effectLedOnBrush) effectLedOnBrush.DeleteObject();
	if (effectMuteBrush) effectMuteBrush.DeleteObject();
	if (effectPanBrush) effectPanBrush.DeleteObject();
	if (generatorBgBrush) generatorBgBrush.DeleteObject();
	if (generatorLedOffBrush) generatorLedOffBrush.DeleteObject();
	if (generatorLedOnBrush) generatorLedOnBrush.DeleteObject();
	if (generatorMuteBrush) generatorMuteBrush.DeleteObject();
	if (generatorPanBrush) generatorPanBrush.DeleteObject();
	if (controllerBgBrush) controllerBgBrush.DeleteObject();
	if (controllerLedOffBrush) controllerLedOffBrush.DeleteObject();
	if (controllerLedOnBrush) controllerLedOnBrush.DeleteObject();
	if (controllerMuteBrush) controllerMuteBrush.DeleteObject();
	if (containerBgBrush) containerBgBrush.DeleteObject();
	if (containerLedOffBrush) containerLedOffBrush.DeleteObject();
	if (containerLedOnBrush) containerLedOnBrush.DeleteObject();
	if (containerMuteBrush) containerMuteBrush.DeleteObject();
	if (borderPen) borderPen.DeleteObject();
	// machine text -- no deleteobject needed
	if (masterBgBrush) masterBgBrush.DeleteObject();
	if (masterLedOffBrush) masterLedOffBrush.DeleteObject();
	if (masterLedOnBrush) masterLedOnBrush.DeleteObject();
	if (panHandleBrush) panHandleBrush.DeleteObject();
	if (audioLinePen) audioLinePen.DeleteObject();
	if (midiLinePen) midiLinePen.DeleteObject();
	if (eventLinePen) eventLinePen.DeleteObject();

	ampBgBrush.CreateSolidBrush(themecolors[amp_bg]);
	ampHandleBrush.CreateSolidBrush(themecolors[amp_handle]);
	arrowHighBrush.CreateSolidBrush(themecolors[arrow_high]);
	arrowBrush.CreateSolidBrush(themecolors[arrow]);
	arrowLowBrush.CreateSolidBrush(themecolors[arrow_low]);
	backgroundColor = themecolors[background];
	effectBgBrush.CreateSolidBrush(themecolors[effect_bg]);
	effectLedOffBrush.CreateSolidBrush(themecolors[effect_led_off]);
	effectLedOnBrush.CreateSolidBrush(themecolors[effect_led_on]);
	effectMuteBrush.CreateSolidBrush(themecolors[effect_mute]);
	effectPanBrush.CreateSolidBrush(themecolors[effect_pan]);
	generatorBgBrush.CreateSolidBrush(themecolors[generator_bg]);
	generatorLedOffBrush.CreateSolidBrush(themecolors[generator_led_off]);
	generatorLedOnBrush.CreateSolidBrush(themecolors[generator_led_on]);
	generatorMuteBrush.CreateSolidBrush(themecolors[generator_mute]);
	generatorPanBrush.CreateSolidBrush(themecolors[generator_pan]);
	controllerBgBrush.CreateSolidBrush(themecolors[controller_bg]);
	controllerLedOffBrush.CreateSolidBrush(themecolors[controller_led_off]);
	controllerLedOnBrush.CreateSolidBrush(themecolors[controller_led_on]);
	controllerMuteBrush.CreateSolidBrush(themecolors[controller_mute]);
	containerBgBrush.CreateSolidBrush(themecolors[container_bg]);
	containerLedOffBrush.CreateSolidBrush(themecolors[container_led_off]);
	containerLedOnBrush.CreateSolidBrush(themecolors[container_led_on]);
	containerMuteBrush.CreateSolidBrush(themecolors[container_mute]);
	borderPen.CreatePen(PS_SOLID, 1, themecolors[border]);
	selectedBorderPen.CreatePen(PS_SOLID, 2, themecolors[border]);
	machineTextColor = themecolors[text];
	masterBgBrush.CreateSolidBrush(themecolors[master_bg]);
	masterLedOffBrush.CreateSolidBrush(themecolors[master_led_off]);
	masterLedOnBrush.CreateSolidBrush(themecolors[master_led_on]);
	panHandleBrush.CreateSolidBrush(themecolors[generator_pan]);
	audioLinePen.CreatePen(PS_SOLID, 1, themecolors[audio_line]);
	midiLinePen.CreatePen(PS_SOLID, 1, themecolors[midi_line]);
	eventLinePen.CreatePen(PS_SOLID, 1, themecolors[event_line]);
	hiddenLinePen.CreatePen(PS_DOT, 1, themecolors[audio_line]);
	dirtyBackground = true;
	Invalidate(FALSE);
}

void CGraphCtrl::GetClientScale(RECT* rc) {
	if (scale_machine_points) {
		GetClientRect(rc);
	} else {
		rc->left = 0;
		rc->top = 0;
		rc->right = 1000;
		rc->bottom = 1000;
	}
}


float CGraphCtrl::GetMachineWidth(CGraphNode* plugin) {
	if (plugin->minimized) {
		return machine_width_minimized;
	} else {
		return machine_width;
	}
}

float CGraphCtrl::GetMachineHeight(CGraphNode* plugin) {
	if (plugin->minimized) {
		return machine_height_minimized;
	} else {
		return machine_height;
	}
}

void CGraphCtrl::GetMachinePoint(CGraphNode* machine, POINT* pt) {
	RECT rc;
	GetClientScale(&rc);

	float ox = machine->position.x;
	float oy = machine->position.y;

	pt->x = int(((((ox * scale) + 1.0f) / 2.0) * (float)rc.right) + 0.5);
	pt->y = int(((((oy * scale) + 1.0f) / 2.0) * (float)rc.bottom) + 0.5);

	if (false
		|| (moveType == MachineViewMoveSelectedMachines && IsSelectedMachine(machine->id))
		|| (moveType == MachineViewMoveAllMachines)
	) {
		pt->x += moveCurrentPoint.x - moveFromPoint.x;
		pt->y += moveCurrentPoint.y - moveFromPoint.y;
	}

  	pt->x += int((view_offset_x * scale * (double)rc.right) + 0.5);
  	pt->y += int((view_offset_y * scale * (double)rc.bottom) + 0.5);
}


void CGraphCtrl::GetMachineLeftTop(CGraphNode* machine, POINT* pt) {
	GetMachinePoint(machine, pt);

	pt->x -= (LONG)((GetMachineWidth(machine) / 2) * scale);
	pt->y -= (LONG)((GetMachineHeight(machine) / 2) * scale);
}

void CGraphCtrl::GetStatusRect(CGraphNode* plugin, RECT* rc) {
	POINT pt;
	GetMachineLeftTop(plugin, &pt);

	/*zzub_pluginloader_t* loader = zzub_plugin_get_pluginloader(plugin);
	MachineSkin* skin = mainFrame->getSkin(zzub_pluginloader_get_uri(loader));
	if (skin && skin->led && !zzub_plugin_get_minimize(plugin)) {
		BITMAP bm;
		::GetObject(skin->led, sizeof(bm), &bm);

		SetRect(rc, 
			(double)mRect.left + (double)skin->ledPosition.x * scale, 
			(double)mRect.top + (double)skin->ledPosition.y * scale, 
			(double)mRect.left + (double)bm.bmWidth * scale,
			(double)mRect.top + (double)bm.bmHeight * scale);
	} else {*/

	int left = (int)(pt.x + 2.0f * scale);
	int top = (int)(pt.y + 2.0f * scale);
	int right = (int)(pt.x + (2.0f + led_width) * scale);
	int bottom = (int)(pt.y + (2.0f + led_height) * scale);

	int width = std::max(right - left, led_width_minimum);
	int height = std::max(bottom - top, led_height_minimum);

	left = std::max(left, (int)pt.x + 2);
	top = std::max(top, (int)pt.y + 2);
	right = left + width;
	bottom = top + height;

	SetRect(rc, left, top, right, bottom);
}

void CGraphCtrl::GetMachineRect(CGraphNode* machine, RECT* outputRect) {
	POINT pt;
	GetMachineLeftTop(machine, &pt);

	int left = pt.x;
	int top = pt.y; 
	int right;
	int bottom;

	if (machine->minimized) {
		// dirty hack to make the minimized machine's LED's look symmetrical
		// the lack of symmetry is due to floating point -> int casting issues
		RECT rcStatus;
		GetStatusRect(machine, &rcStatus);
		int diff_x = rcStatus.left - left;
		int diff_y = rcStatus.top - top;
		right = rcStatus.right + diff_x;
		bottom = rcStatus.bottom + diff_y;
	} else {
		right = left + (int)(GetMachineWidth(machine) * scale);
		bottom = top + (int)(GetMachineHeight(machine) * scale);
	}

	SetRect(outputRect, left, top, right, bottom);
}

float CGraphCtrl::GetFontHeight() {
	return (scale+1)*8;	
}

CBrush& CGraphCtrl::GetMachineBrush(CGraphNode* machine) {
	if (machine->type == node_master) {
		return masterBgBrush;
	} else
	if (machine->type == node_controller) {
		if (machine->muted || machine->bypassed)
			return controllerMuteBrush;
		else
			return controllerBgBrush;
	} else
	if (machine->type == node_effect) {
		if (machine->muted || machine->bypassed)
			return effectMuteBrush;
		else
			return effectBgBrush;
	} else
	if (machine->type == node_generator) {
		if (machine->muted || machine->bypassed)
			return generatorMuteBrush;
		else
			return generatorBgBrush;
	} else
	if (machine->type == node_container) {
		if (machine->muted || machine->bypassed)
			return containerMuteBrush;
		else
			return containerBgBrush;
	} else
	// fallback to generator
		return generatorBgBrush;
}

CBrush& CGraphCtrl::GetMachineLedBrush(CGraphNode* machine) {
	if (machine->type == node_master) {
		if (machine->led)
			return masterLedOnBrush;
		else
			return masterLedOffBrush;
	} else
	if (machine->type == node_controller) {
		if (machine->led)
			return controllerLedOnBrush;
		else
			return controllerLedOffBrush;
	} else
	if (machine->type == node_effect) {
		if (machine->led)
			return effectLedOnBrush;
		else
			return effectLedOffBrush;
	} else {
		// fallback to generator
		//if (machine->type == node_generator) {
		if (machine->led)
			return generatorLedOnBrush;
		else
			return generatorLedOffBrush;
	}
	return generatorLedOffBrush;
}

void CGraphCtrl::PaintVolumeSlider(CDC& dc, RECT vsr, CGraphEdge* audioconn) {

	CBrushHandle oldBrush = dc.SelectBrush(ampBgBrush);

	int hx = vsr.left;
	int hy = vsr.top;
	dc.Rectangle(hx, hy, hx+32, hy+128);

//	int vx = ((volmax - volmin) - value) / ((volmax - volmin) / (128 - 20));
	int vx = 108 - (int)(audioconn->amp * 54);

	dc.SelectBrush(ampHandleBrush);
	dc.Rectangle(hx, hy + vx, hx + 32, hy + vx + 20);
	dc.SelectBrush(oldBrush);
}

void CGraphCtrl::PaintDirectionTriangle(CDC& dc, POINT const& mPoint, POINT const& sPoint, int amp, const CPen* pen, std::string const& text) {
	int mx = mPoint.x;
	int my = mPoint.y;
	int sx = sPoint.x;
	int sy = sPoint.y;

	// 1) finn vinkelen på linjen
	// kryssprodukt på linje og 'opp'

	int lx = sx - mx;
	int ly = sy - my;

	float len = (float)sqrt((float)( lx*lx + ly*ly));

	float ux = (float)lx / len;
	float uy = (float)ly / len;

	float rx = 0*ux - 1*uy;
	float ry = 1*ux + 0*uy;

	float hx = (mx+lx/2 + ux*8*scale);
	float hy = (my+ly/2 + uy*8*scale);

	float fx = (hx + rx*-10*scale);
	float fy = (hy + ry*-10*scale);
	dc.MoveTo((int)fx, (int)fy);
	POINT pts[3];
	pts[0].x = (LONG)(hx + rx*-10*scale);
	pts[0].y = (LONG)(hy + ry*-10*scale);

	pts[1].x = (LONG)(hx+rx*10*scale);
	pts[1].y = (LONG)(hy+ry*10*scale);

	pts[2].x = (LONG)(hx+ux*-16*scale);
	pts[2].y = (LONG)(hy+uy*-16*scale);

	// Colorize based on amp

	LOGBRUSH b;
	arrowHighBrush.GetLogBrush(b);
	COLORREF hicol = b.lbColor;
	arrowBrush.GetLogBrush(b);
	COLORREF midcol = b.lbColor;
	arrowLowBrush.GetLogBrush(b);
	COLORREF locol = b.lbColor;

	float percentage = float(amp) / 0x4000;

	CBrush percBrush;
	COLORREF col;
	if (percentage <= 1.0f)
		col = AveragePercentRGB(locol, midcol, percentage);
	else
		col = AveragePercentRGB(midcol, hicol, (percentage - 1.0f));
	percBrush.CreateSolidBrush(col);

	CPenHandle oldPen = dc.SelectPen(*pen); // use the appropriate pen for this arrow type
	CBrushHandle oldBrush = dc.SelectBrush(percBrush);

	dc.Polygon(pts, 3);

	// draw arrow label
	{
		// hx,hy er altså midten av tekstboksen vår
		hx = mx + lx / 2.0f;
		hy = my + ly / 2.0f;
		POINT tp;
		tp.x = (LONG)(hx + ux * 2);
		tp.y = (LONG)(hy + ux * 2);

		int fontIndex = (int)((scale / MAX_MACHINE_SCALE) * ((float)MAX_MACHINE_FONTS - 0.5f));
		CFont prevFont = dc.SelectFont(machineFont[fontIndex]);
		UINT prevAlign = dc.SetTextAlign(TA_LEFT);
		int prevBkMode = dc.SetBkMode(TRANSPARENT);

		int offset_x = (int)scale * 24;
		int offset_y = -8;// * scale;

		dc.SetTextColor(machineTextColor);
		//dc.TextOut(tp.x+offset_x, tp.y+offset_y, text.c_str());
		CRect rcText(tp.x+offset_x, tp.y+offset_y, tp.x+offset_x+400, tp.y+offset_y+100);
		dc.DrawText(text.c_str(), (int)text.length(), &rcText, DT_LEFT|DT_NOCLIP|DT_NOPREFIX);

		dc.SetBkMode(prevBkMode);
		dc.SetTextAlign(prevAlign);
		dc.SelectFont(prevFont);
	}

	dc.SelectPen(oldPen);
	dc.SelectBrush(oldBrush);
}

void CGraphCtrl::SetTheme(themecolor index, COLORREF color) {
	themecolors[index] = color;
}

CGraphNode* CGraphCtrl::AddNode(int id, const std::string& name, float x, float y, nodetype type) {
	CGraphNode* node = new CGraphNode();
	node->id = id;
	node->bypassed = false;
	node->muted = false;
	node->minimized = false;
	node->position.x = x;
	node->position.y = y;
	node->type = type;
	node->name = name;
	node->led = false;
	nodes.push_back(boost::shared_ptr<CGraphNode>(node));

	dirtyMachines = true;
	Invalidate(FALSE);
	return node;
}

CGraphEdge* CGraphCtrl::AddEdge(int id, int to_id, int from_id, edgetype type, float amp, const std::string& description) {
	CGraphEdge* edge = new CGraphEdge();
	edge->id = id;
	edge->type = type;
	edge->to_node_id = to_id;
	edge->from_node_id = from_id;
	edge->to_user_id = to_id;
	edge->from_user_id = from_id;
	edge->amp = amp;
	edge->is_indirect = false;
	edge->description = description;
	edges.push_back(boost::shared_ptr<CGraphEdge>(edge));

	dirtyMachines = true;
	Invalidate(FALSE);
	return edge;
}


// ---------------------------------------------------------------------------------------------------------------
// XXX
// ---------------------------------------------------------------------------------------------------------------

LRESULT CGraphCtrl::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	dirtyOffscreenBitmap = true;
	Invalidate(FALSE);
	return 0;
}

void CGraphCtrl::CalcFontHeight() {
	CFontHandle font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	CFontHandle prevfont = offscreenDC.SelectFont(font);

	SIZE sz;
	offscreenDC.GetTextExtent("X", 1, &sz);
	fontHeight = sz.cy;

	offscreenDC.SelectFont(prevfont);
}

void CGraphCtrl::InvalidateMachines(bool background) {
	if (background) dirtyBackground = true;
	dirtyMachines = true;
	Invalidate(FALSE);
}

void CGraphCtrl::InvalidateConnectionVolume() {
	dirtyVolumeSlider = true;

	RECT vsr;
	GetVolumeSliderRect(selectedConnection, &vsr);
	InvalidateRect(&vsr, FALSE);
}

void CGraphCtrl::InvalidateSelectionRectangle() {
	dirtySelectionRectangle = true;
	RECT rcSel;
	GetDragRect(&rcSel);
	if (prevDragRect.left != -1) UnionRect(&rcSel, &rcSel, &prevDragRect);
	InvalidateRect(&rcSel, FALSE);
}

void CGraphCtrl::GetVolumeSliderRect(POINT const& mPoint, POINT const& sPoint, RECT* vsr) {
	int mx = mPoint.x;
	int my = mPoint.y;
	int sx = sPoint.x;
	int sy = sPoint.y;

	int lx = sx-mx;
	int ly = sy-my;

	double len = (double)sqrt((double)( lx*lx + ly*ly));
	double ux = (double)lx/len;
	double uy = (double)ly/len;

	int hx = (int)(mx + lx / 2 + ux * 8);
	int hy = (int)(my + ly / 2 + uy * 8);

	hx -= 16;	// ??
	hy -= 64;	// ??

	vsr->left = hx;
	vsr->right = hx+32;
	vsr->top = hy;
	vsr->bottom = hy+128;
}

void CGraphCtrl::GetVolumeSliderRect(CGraphNodePair const& conn, RECT* vsr) {
	POINT mPoint, sPoint;
	GetMachinePoint(GetNode(conn.to_node_id), &mPoint);
	GetMachinePoint(GetNode(conn.from_node_id), &sPoint);
	GetVolumeSliderRect(mPoint, sPoint, vsr);
}

bool CGraphCtrl::MouseDownMachine(POINT pt, WPARAM wParam) {
	RECT mRect;
	
	// test if we're clicking inside a machine
	draggingMachine = GetMachineAtPt(pt, &mRect);
	if (draggingMachine == 0) return false;

	// clicking in small mute rect?
	RECT rc;
	GetStatusRect(draggingMachine, &rc);
	
	if (!draggingMachine->minimized && PtInRect(&rc, pt)) {
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_NODE_MUTE), 0);
		return true;
	}
	// ctrl-click = toggle machine selection
	// single click = select only clicked machine
	if ((wParam & MK_CONTROL) != 0) {
		if (IsSelectedMachine(draggingMachine->id)) {
			UnselectMachine(draggingMachine);
		} else {
			SelectMachine(draggingMachine);
		}
		moveType = MachineViewMoveNothing;
	} else {
		if (!IsSelectedMachine(draggingMachine->id))
			ClearSelectedMachines();
		SelectMachine(draggingMachine);
		moveType = MachineViewMoveSelectedMachines;
		moveCurrentPoint = pt;
		moveFromPoint = pt;
	}
	
	{
		bool isConnecting = ((wParam&MK_SHIFT) != 0) || mbuttonState ;
		if (isConnecting) {
			connectPos = pt;
			moveType = MachineViewMoveConnectMachines;

			dirtyBackground = true;
			prevConnectPos.x = prevConnectPos.y = -1;
			dirtyConnecting = true;
			Invalidate(FALSE);
		}
	}

	beginDragPt = pt;

	SetCapture();
	return true;
	
}

bool CGraphCtrl::MouseDownConnection(POINT pt, WPARAM wParam) {

	CGraphNodePair conn;
	if (!GetConnectionAtPt(pt, &conn)) {
		// de-select connection
		selectedConnectionValid = false;
		return false;
	}

	// test if we're adjusting connection volume
	SelectConnection(conn);
	CGraphEdge* audioconn = GetSelectedEdge(edge_audio);

	if (audioconn != 0) {
		initialVolume = audioconn->amp;

		moveType = MachineViewMoveVolumeSlider;
		moveFromPoint = pt;
		moveCurrentPoint = pt;
		InvalidateConnectionVolume();

		GetClientRect(&volumeinfo->rect);
		UpdateConnectionToolTip();
		volumetip.TrackActivate(volumeinfo.get(), TRUE);

		SetCapture();
	}

	return true;

}

bool CGraphCtrl::MouseDownBackground(POINT pt, WPARAM wParam) {
	
	if ((wParam&MK_CONTROL) != 0) {
		// we are pressing somewhere in the bkacground and holding down ctrl
		moveType = MachineViewMoveAllMachines;
		moveFromPoint = pt;
		moveCurrentPoint = pt;
		SetCapture();
	} else {
		// we are clking somewhere in the background, and not holding down ctrl
		// so we begin a selection rectangle
		machinesOutsideDragRectOriginallySelected.clear();
		if ((wParam&MK_SHIFT) != 0) {
			// if shift er nede, så skal vi beholde selectionen
			for (int i = 0; i < GetSelectedMachines(); i++) {
				machinesOutsideDragRectOriginallySelected.push_back(GetSelectedMachine(i));
			}
		} else {
			ClearSelectedMachines();
		}

		SetRect(&prevDragRect, -1, -1, -1, -1);
		connectPos = pt; // track movement in connectPos
		moveType = MachineViewMoveSelectRectangle;
		SetCapture();
	}

	beginDragPt = pt;

	return true;
}


void CGraphCtrl::SelectConnection(CGraphNodePair const& connpair) {
	selectedConnection = connpair;
	selectedConnectionValid = true;
	InvalidateMachines(false);
}

void CGraphCtrl::SelectConnection(int to_node_id, int from_node_id) {
	selectedConnection.to_node_id = to_node_id;
	selectedConnection.from_node_id = from_node_id;
	selectedConnectionValid = true;
	InvalidateMachines(false);
}

void CGraphCtrl::UnselectConnection() {
	selectedConnectionValid = false;
	InvalidateMachines(false);
}


// ---------------------------------------------------------------------------------------------------------------
// SELECTION
// ---------------------------------------------------------------------------------------------------------------

void CGraphCtrl::SelectMachine(CGraphNode* machine) {
	if (machine == 0) return ;

	for (unsigned int i = 0; i < selectedMachines.size(); i++) {
		if (selectedMachines[i] == machine->id) return;
	}
	selectedMachines.push_back(machine->id);

	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_NODE_SELECT), 0);

	InvalidateMachines(false);
}

void CGraphCtrl::UnselectMachine(CGraphNode* machine) {
	std::vector<int>::iterator i = find(selectedMachines.begin(), selectedMachines.end(), machine->id);
	if (i != selectedMachines.end()) {
		selectedMachines.erase(i);
	}

	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_NODE_SELECT), 0);

	InvalidateMachines(false);
}

bool CGraphCtrl::IsSelectedMachine(int node_id) {
	for (size_t i = 0; i < selectedMachines.size(); i++) {
		if (selectedMachines[i] == node_id) return true;
	}
	return false;
}

int CGraphCtrl::GetSelectedMachines() {
	return (int)selectedMachines.size();
}

CGraphNode* CGraphCtrl::GetSelectedMachine(int index) {
	if (index < 0 || index >= (int)selectedMachines.size()) return 0;
	return GetNode(selectedMachines[index]);
}

void CGraphCtrl::ClearSelectedMachines() {
	selectedMachines.clear();
	GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_NODE_SELECT), 0);
	InvalidateMachines(false);
}

bool CGraphCtrl::GetConnectionAtPt(POINT const& pt, CGraphNodePair* connpair) {
	for (std::vector<boost::shared_ptr<CGraphEdge> >::iterator i = edges.begin(); i != edges.end(); ++i) {
		CGraphEdge* edge = i->get();

		CGraphNode* from_plugin = GetNode(edge->from_node_id);
		CGraphNode* to_plugin = GetNode(edge->to_node_id);

		if (IsSelectedMachine(edge->from_node_id) && GetMachineHideIncomingConnections(edge->from_node_id)) continue;

		RECT cRect;
		GetConnectionRect(CGraphNodePair(from_plugin, to_plugin), &cRect);
		if (PtInRect(&cRect, pt)) {
			connpair->from_node_id = edge->from_node_id;
			connpair->to_node_id = edge->to_node_id;
			return true;
		}
	}

	return false;
}

CGraphNode* CGraphCtrl::GetMachineAtPt(POINT const& pt, RECT* machineRect) {
	for (std::vector<boost::shared_ptr<CGraphNode> >::reverse_iterator i = nodes.rbegin(); i != nodes.rend(); ++i) {
		CGraphNode* plugin = i->get();

		RECT mRect;
		GetMachineRect(plugin, &mRect);

		if (PtInRect(&mRect, pt)) {
			if (machineRect) *machineRect = mRect;
			return plugin;
		}
	}
	return 0;
}

bool CGraphCtrl::GetConnectionRect(CGraphNodePair const& conn, RECT* rc) {
	POINT mPoint, sPoint;
	GetMachinePoint(GetNode(conn.to_node_id), &mPoint);
	GetMachinePoint(GetNode(conn.from_node_id), &sPoint);

	int mx = mPoint.x;
	int my = mPoint.y;
	int sx = sPoint.x;
	int sy = sPoint.y;

	// 1) finn vinkelen på linjen
	// kryssprodukt på linje og 'opp'

	int lx = sx-mx;
	int ly = sy-my;

	double len = (double)sqrt((double)( lx*lx + ly*ly));

	double ux = (double)lx/len;
	double uy = (double)ly/len;

	double rx = 0*ux - 1*uy;
	double ry = 1*ux + 0*uy;

	int hx = mx+lx/2;
	int hy = my+ly/2;

	rc->left = (LONG)min(hx-scale*10, hx+scale*10);
	rc->top = (LONG)min(hy-scale*10, hy+scale*10);
	rc->right = (LONG)max(hx-scale*10, hx+scale*10);
	rc->bottom = (LONG)max(hy-scale*10, hy+scale*10);
	return true;
}

bool CGraphCtrl::GetConnectionTextRect(CGraphNodePair const& conn, RECT* rc) {
	RECT cRect;
	GetConnectionRect(conn, &cRect);
	// these numbers aren't exact, just tried to cover it.
	// still getting trails now and then.
	rc->left = cRect.right;// + (scale*24);
	rc->top = cRect.top;// - 8;
	rc->right = cRect.right + (int)(scale*125);
	rc->bottom = cRect.bottom;
	return true;
}

void CGraphCtrl::GetMachineInvalidRect(CGraphNode* machine, RECT* outputRect) {
	if (machine->minimized) {
		// dirty hax to try to cover the area of the minimized machine + its text
		// we just assume the text is as wide as a regular unminimized machine
		POINT pt;
		GetMachinePoint(machine, &pt);
		RECT rc;
		GetMachineRect(machine, &rc);

		int halfWidth = (int)((machine_width / 2) * scale);

		outputRect->left = pt.x - halfWidth;
		outputRect->top = pt.y - (int)(GetFontHeight() * 2);
		outputRect->right = pt.x + halfWidth;
		outputRect->bottom = rc.bottom;
	} else {
		GetMachineRect(machine, outputRect);
	}
}

void CGraphCtrl::MouseMoveSelectedMachines(POINT pt, WPARAM wParam) {
	RECT rcInvalidate = {
		0x10000000, 0x10000000, 
		-0x10000000, -0x10000000, 
	};

	// move the selected machines, and find the smallest area on screen that needs to be repainted
	for (int i = 0; i<GetSelectedMachines(); ++i) {
		RECT selrc;
		CGraphNode* selplugin = GetSelectedMachine(i);
		GetMachineInvalidRect(selplugin, &selrc);
		InflateRect(&selrc, 3, 3);
		UnionRect(&rcInvalidate, &rcInvalidate, &selrc);
	}

	moveCurrentPoint = pt;

	for (int i = 0; i < GetSelectedMachines(); ++i) {
		RECT selrc;
		CGraphNode* selplugin = GetSelectedMachine(i);

		// make sure the invalidation rect covers all machines we are connected to
		for (std::vector<boost::shared_ptr<CGraphEdge> >::iterator j = edges.begin(); j != edges.end(); ++j) {
			CGraphNode* to_plugin = GetNode((*j)->to_node_id);
			CGraphNode* from_plugin = GetNode((*j)->from_node_id);

			if (to_plugin == selplugin) {
				GetMachineInvalidRect(from_plugin, &selrc);
				InflateRect(&selrc, 3, 3);
				UnionRect(&rcInvalidate, &rcInvalidate, &selrc);

				if (showConnectionText) {
					RECT connrc;
					GetConnectionTextRect(CGraphNodePair(from_plugin, selplugin), &connrc);
					UnionRect(&rcInvalidate, &rcInvalidate, &connrc);
				}
			} else if (from_plugin == selplugin) {

				GetMachineInvalidRect(to_plugin, &selrc);
				InflateRect(&selrc, 3, 3);
				UnionRect(&rcInvalidate, &rcInvalidate, &selrc);

				if (showConnectionText) {
					RECT connrc;
					GetConnectionTextRect(CGraphNodePair(selplugin, to_plugin), &connrc);
					UnionRect(&rcInvalidate, &rcInvalidate, &connrc);
				}
			}
		}
	}

	dirtyMachines = true;
	InvalidateRect(&rcInvalidate, FALSE);
}

void CGraphCtrl::MouseMoveAllMachines(POINT pt, WPARAM wParam) {
	InvalidateMachines(false);
	moveCurrentPoint = pt;
}

void CGraphCtrl::MouseMoveSelectRectangle(POINT pt, WPARAM wParam) {
	// track movement in connectPos
	connectPos = pt;
	RECT rcSel;
	GetDragRect(&rcSel);

	// see how many selected machines there are inside this rect and see if its
	// a different number than last we checked, we may need to update which machines are selected...
	std::vector<CGraphNode*> prevMachines = machinesInDragRect;
	int numSelMachines = (int)machinesInDragRect.size();
	GetMachinesInDragRect(&rcSel);

	if (machinesInDragRect.size()!=numSelMachines) {

		SetRect(&prevDragRect, -1, -1, -1, -1);
		std::vector<CGraphNode*> selMachines;
		for (size_t i = 0; i < machinesOutsideDragRectOriginallySelected.size(); i++) {
			selMachines.push_back(machinesOutsideDragRectOriginallySelected[i]);
		}
		for (size_t i = 0; i < machinesInDragRect.size(); i++) {
			selMachines.push_back(machinesInDragRect[i]);
		}

		for (size_t i = 0; i < prevMachines.size(); i++) {
			CGraphNode* prevMachine = prevMachines[i];
			std::vector<CGraphNode*>::iterator j = find(selMachines.begin(), selMachines.end(), prevMachine);
			if (j == selMachines.end()) {
				UnselectMachine(prevMachine);
				continue;
			} else 
			if (!IsSelectedMachine(prevMachine->id)) {
				SelectMachine(prevMachine);
			}
			selMachines.erase(j);
		}
		for (size_t i = 0; i < selMachines.size(); i++) {
			SelectMachine(selMachines[i]);
		}

		dirtyMachines = true;

		// dont invalidate last dragrect because its cleared already
		dirtyBackground = true;
		dirtySelectionRectangle = true;
		Invalidate(FALSE);
	} else {
		InvalidateSelectionRectangle();
	}
}

void CGraphCtrl::MouseMoveConnectMachines(POINT pt, WPARAM wParam) {
	if (((wParam&MK_SHIFT) != 0) || mbuttonState ) {
		connectPos = pt;
	} else {
		// shift or mbutton was released, stop connection action
		// because, if we say there is connecting going on, and draggingMachine=0, there will be crashes
		draggingMachine = 0;
		moveType = MachineViewMoveNothing;	
		prevConnectPos.x = prevConnectPos.y = -1;
		ReleaseCapture();
		dirtyBackground = true;
		Invalidate(FALSE);
		return;
	}

	// if we are over a possible target machine, we should check for div stuff

	CGraphNode* targetPlugin = 0;
	if ((connectTargetMachine != 0) && (!connectTargetMenu.IsEmpty())) {
		if (connectTargetMenu.HitTestID(connectPos.x, connectPos.y, true)) 
			targetPlugin = connectTargetMachine;
	}

	if (targetPlugin == 0) {
		RECT rcTarget;
		targetPlugin = GetMachineAtPt(connectPos, &rcTarget);
		if (targetPlugin == draggingMachine)
			targetPlugin = 0;
	}

	if ((targetPlugin != 0) && (connectTargetMachine != targetPlugin)) {
		connectTargetMachine = targetPlugin;

		// parent must handle GN_CONNECT_CONTEXT and do connectTargetMenu.SetMenu(graphCtrl.m_hWnd, hMenu);
		GetParent().SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), GN_CONNECT_CONTEXT), 0);

		dirtyConnectTargetMenu = true;
		dirtyBackground = true;
		prevConnectPos.x = prevConnectPos.y = -1;
		dirtyConnecting = true;
		Invalidate(FALSE);
		return;
	} else
	if ((targetPlugin == 0) && (connectTargetMachine != 0)) {
		connectTargetMachine = 0;
		dirtyBackground = true;
		prevConnectPos.x = prevConnectPos.y = -1;
		dirtyConnecting = true;
		Invalidate(FALSE);
		return;
	}

	if ((targetPlugin != 0) && (!connectTargetMenu.IsEmpty())) {
		if (connectTargetMenu.OnMouseMove(connectPos.x, connectPos.y)) {
			dirtyBackground = true;
			dirtyConnectTargetMenu = true;
			prevConnectPos.x = prevConnectPos.y = -1;
		}
	}

	// invalidate only the area covered by the line

	dirtyConnecting = true;

	RECT mRect;
	GetMachineRect(draggingMachine, &mRect);
	POINT mp = { mRect.left + (mRect.right - mRect.left) / 2, mRect.top + (mRect.bottom - mRect.top) / 2 };

	RECT rcCurrent = {
		std::min(mp.x, connectPos.x), std::min(mp.y, connectPos.y), 
		std::max(mp.x, connectPos.x), std::max(mp.y, connectPos.y), 
	};
	RECT rcPrev = {
		std::min(mp.x, prevConnectPos.x), std::min(mp.y, prevConnectPos.y), 
		std::max(mp.x, prevConnectPos.x), std::max(mp.y, prevConnectPos.y), 
	};
	UnionRect(&rcCurrent, &rcCurrent, &rcPrev);
	InflateRect(&rcCurrent, 1, 1);
	InvalidateRect(&rcCurrent, FALSE);
}

void CGraphCtrl::GetMachinesInDragRect(RECT* rcSel) {
	machinesInDragRect.clear();
	for (std::vector<boost::shared_ptr<CGraphNode> >::iterator i = nodes.begin(); i != nodes.end(); ++i) {
		CGraphNode* plugin = i->get();

		RECT rcMachine, rcDest;
		GetMachineRect(plugin, &rcMachine);
		
		if (IntersectRect(&rcDest, rcSel, &rcMachine)) {
			machinesInDragRect.push_back(plugin);
		}
	}
}

void CGraphCtrl::GetDragRect(RECT* rc) {
	int minx = min(beginDragPt.x, connectPos.x);
	int miny = min(beginDragPt.y, connectPos.y);
	int maxx = max(beginDragPt.x, connectPos.x);
	int maxy = max(beginDragPt.y, connectPos.y);

	SetRect(rc, minx, miny, maxx, maxy);
}

bool CGraphCtrl::GetMachineHideIncomingConnections(int node_id) {
	std::map<int, bool>::iterator i = hideIncoming.find(node_id);
	if (i == hideIncoming.end()) return false;
	return i->second;
}

void CGraphCtrl::SetMachineHideIncomingConnections(int node_id, bool state) {
	hideIncoming[node_id] = state;
}

float linear_to_dB(float val, float limit) { 
	if(val <= 0)
		return limit;
	return(20.0f * log10(val)); 
}

void CGraphCtrl::UpdateConnectionToolTip() {
	int newVolume = 0;//getVolumeSliderVolume(moveCurrentPoint);

	CRect rcConn;
	GetVolumeSliderRect(selectedConnection, &rcConn);

	CPoint center = rcConn.CenterPoint();
	ClientToScreen(&center);

	CGraphEdge* audioconn = GetSelectedEdge(edge_audio);
	assert(audioconn != 0);
	float normalizedvolume = audioconn->amp;
	char volumestr[32];
	std::stringstream volumestrm;
	volumestrm << std::fixed << std::setprecision(1) << (normalizedvolume * 100) << "% (" << std::fixed << std::setprecision(2) << linear_to_dB(normalizedvolume, -60) << " dB)";
	strcpy(volumestr, volumestrm.str().c_str());

	volumeinfo->lpszText = volumestr;
	volumetip.SetToolInfo(volumeinfo.get());
	volumetip.TrackPosition(center.x + 32, center.y);
}
