#pragma once

#include "SearchBox.h"
#include "ModelessMenu.h"

// sent as WM_COMMANDs to the parent:
const int GN_NODE_MOVE = 1;
const int GN_VIEW_MOVE = 2;
const int GN_NODE_MUTE = 3;
const int GN_CONN_AMP_TRACK = 4;
const int GN_CONN_AMP_SET = 5;
const int GN_NODE_SELECT = 6;
const int GN_SCALE_CHANGE = 7;
const int GN_CONNECT_CONTEXT = 8;
const int GN_NODE_DBLCLICK = 9;
const int GN_CONN_CLICK = 10;
const int GN_CONN_DISCONNECT = 11;
const int GN_CLICK = 12;

enum themecolor {
	background,
	amp_bg,
	amp_handle,
	arrow_high,
	arrow,
	arrow_low,
	effect_bg,
	effect_led_off,
	effect_led_on,
	effect_pan,
	effect_mute,
	generator_bg,
	generator_led_off,
	generator_led_on,
	generator_mute,
	generator_pan,
	controller_bg,
	controller_led_off,
	controller_led_on,
	controller_mute,
	container_bg,
	container_led_off,
	container_led_on,
	container_mute,
	border,
	master_bg,
	master_led_off,
	master_led_on,
	pen_handle,
	audio_line,
	midi_line,
	event_line,
	note_line,
	text,

	max_theme_colors
};

enum nodetype {
	node_master,
	node_generator,
	node_effect,
	node_controller,
	node_container,
};

enum edgetype {
	edge_audio,
	edge_midi,
	edge_event,
	edge_note,
	edge_hidden
};

enum MachineViewMouseMoveType {
	MachineViewMoveNothing,
	MachineViewMoveVolumeSlider,
	MachineViewMovePanSlider,
	MachineViewMoveConnectMachines,
	MachineViewMoveSelectedMachines,
	MachineViewMoveAllMachines,
	MachineViewMoveSelectRectangle,
};

class CGraphNode {
public:
	int id;
	nodetype type;
	std::string name;
	POINTF position;
	bool muted;
	bool bypassed;
	bool minimized;
	bool led;
};

class CGraphEdge {
public:
	int id;
	edgetype type;
	int from_node_id;
	int to_node_id;
	float amp;
	int from_user_id;
	int to_user_id;
	bool is_indirect;
	std::string description;
};

struct CGraphNodePair {
	CGraphNodePair(CGraphNode* from_node, CGraphNode* to_node)
		: from_node_id(from_node->id), to_node_id(to_node->id) {}
	CGraphNodePair()
		: from_node_id(0), to_node_id(0) {}
	int from_node_id;
	int to_node_id;
};

class CGraphCtrl : public CWindowImpl<CGraphCtrl> {
public:
	float scale;
	POINT connectPos, prevConnectPos, beginDragPt;
	RECT prevDragRect;
	float initialVolume, initialPan;	// connection volume and panning before sliding or inserting starts
	CFont machineFont[10];	// pre allocated fonts for changing font sizes
	bool mbuttonState;
	MachineViewMouseMoveType moveType;
	POINT moveFromPoint, moveCurrentPoint;
	//float pxSizeX, pxSizeY; // measures that helds value of 1px's size. @see calcPixelSize()
	POINT mousemove_previous;
	COLORREF themecolors[max_theme_colors];

	COLORREF backgroundColor;
	COLORREF machineTextColor;

	CBrush masterBgBrush;
	CBrush masterLedOnBrush;
	CBrush masterLedOffBrush;

	CBrush generatorBgBrush;
	CBrush generatorLedOnBrush;
	CBrush generatorLedOffBrush;
	CBrush generatorPanBrush;
	CBrush generatorMuteBrush;

	CBrush effectBgBrush;
	CBrush effectLedOnBrush;
	CBrush effectLedOffBrush;
	CBrush effectPanBrush;
	CBrush effectMuteBrush;

	CBrush controllerBgBrush;
	CBrush controllerLedOnBrush;
	CBrush controllerLedOffBrush;
	CBrush controllerMuteBrush;

	CBrush containerBgBrush;
	CBrush containerLedOnBrush;
	CBrush containerLedOffBrush;
	CBrush containerMuteBrush;

	CPen audioLinePen, midiLinePen, eventLinePen, noteLinePen, hiddenLinePen;
	CPen borderPen, selectedBorderPen;

	CBrush panHandleBrush;
	CBrush arrowHighBrush;
	CBrush arrowBrush;
	CBrush arrowLowBrush;
	CBrush ampBgBrush;
	CBrush ampHandleBrush;

	bool dirtyBackground, dirtyMachines, dirtyStatus, dirtySelectionRectangle, dirtyVolumeSlider, dirtyConnecting, dirtyConnectTargetMenu;
	bool dirtyOffscreenBitmap;//, dirtyVisibleMachines;
	CDC offscreenDC;
	CBitmap offscreenBitmap;
	bool showConnectionText;

	CGraphNode* connectTargetMachine;
	CGraphNode* draggingMachine;
	CModelessMenu connectTargetMenu;
	std::list<float> machinesListX, machinesListY; // used for aligning machines. @see getNearestMachineLocation(..);

	std::vector<int> selectedMachinesCopy;	// restore selection if a double click follows a click
	std::vector<int> selectedMachines;
	std::vector<CGraphNode*> machinesInDragRect;
	std::vector<CGraphNode*> machinesOutsideDragRectOriginallySelected;
//	zzub_plugin_t* lastMidiMachine;	// copy of midi plugin when unfocused
private:
	CGraphNodePair selectedConnection;
	bool selectedConnectionValid;
public:
	bool useSkins;
	std::vector<boost::shared_ptr<CGraphNode> > nodes;
	std::vector<boost::shared_ptr<CGraphEdge> > edges;

	double view_offset_x;
	double view_offset_y;
	bool scale_machine_points;
	int fontHeight;
	CToolTipCtrl volumetip; 
	boost::shared_ptr<CToolInfo> volumeinfo;
	//std::map<int, bool> ledStates;
	std::map<int, bool> hideIncoming;
	POINT typepoint;
	CRect clientSize;
	CSearchBox searchbox;

	BEGIN_MSG_MAP(CGraphCtrl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnForwardMessage)
		MESSAGE_HANDLER(WM_CHAR, OnChar)
		MESSAGE_HANDLER(WM_KEYDOWN, OnForwardMessage)
		MESSAGE_HANDLER(WM_KEYUP, OnForwardMessage)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMButtonDown)
		MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)

		REFLECT_NOTIFICATIONS_EX() // for the search box
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDblClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnForwardMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	CGraphNode* AddNode(int id, const std::string& name, float x, float y, nodetype type);
	CGraphEdge* AddEdge(int id, int to_id, int from_id, edgetype type, float amp, const std::string& description);
	CGraphNode* GetNode(int id);
	CGraphEdge* GetEdge(int to_id, int from_id, edgetype type);
	CGraphEdge* GetSelectedEdge(edgetype type);
	CGraphNodePair* GetSelectedEdge();
	void PaintNode(CDC& dc, CGraphNode* plugin, bool ledOnly);
	void PaintNodes(CDC& dc);
	void PaintVolumeSlider(CDC& dc, RECT vsr, CGraphEdge* audioconn);
	void PaintMachineStatus(CDC& dc);
	void PaintVolume(CDC& dc);
	void PaintSelectionRect(CDC& dc);
	void PaintConnectionLine(CDC& dc);
	void PaintDirectionTriangle(CDC& dc, POINT const& mPoint, POINT const& sPoint, int amp, const CPen* pen, std::string const& text);
	void PaintConnectionTargetMenu(CDC& dc);
	CBrush& GetMachineBrush(CGraphNode* machine);
	CBrush& GetMachinePanningBrush(CGraphNode* machine);
	CBrush& GetMachineLedBrush(CGraphNode* machine);
	void InvalidateMachines(bool background = true);
	void InvalidateConnectionVolume();
	void InvalidateSelectionRectangle();
	float GetFontHeight();
	void GetMachinePoint(CGraphNode* machine, POINT* pt);
	float GetMachineWidth(CGraphNode* machine);
	float GetMachineHeight(CGraphNode* machine);
	void GetMachineRect(CGraphNode* machine, RECT* outputRect);
	void GetClientScale(RECT* rc);
	void GetMachineLeftTop(CGraphNode* machine, POINT* pt);
	void GetStatusRect(CGraphNode* plugin, RECT* rc);
	CGraphNode* GetMachineAtPt(POINT const& pt, RECT* machineRect=0);
	bool GetConnectionAtPt(POINT const& pt, CGraphNodePair* conn);
	bool GetConnectionRect(CGraphNodePair const& conn, RECT* rc);
	bool GetConnectionTextRect(CGraphNodePair const& conn, RECT* rc);
	void GetVolumeSliderRect(POINT const& mPoint, POINT const& sPoint, RECT* vsr);
	void GetVolumeSliderRect(CGraphNodePair const& conn, RECT* vsr);
	void GetMachineInvalidRect(CGraphNode* machine, RECT* outputRect);
	void GetMachinesInDragRect(RECT* rcSel);
	void GetDragRect(RECT* rc);
	void CalcFontHeight();
	void MouseMoveConnectMachines(POINT pt, WPARAM wParam);
	void MouseMoveSelectedMachines(POINT pt, WPARAM wParam);
	void MouseMoveVolumeSlider(POINT pt, WPARAM wParam);
	void MouseMoveAllMachines(POINT pt, WPARAM wParam);
	void MouseMoveSelectRectangle(POINT pt, WPARAM wParam);
	bool MouseDownMachine(POINT pt, WPARAM wParam);
	bool MouseDownConnection(POINT pt, WPARAM wParam);
	bool MouseDownBackground(POINT pt, WPARAM wParam);
	void SelectMachine(CGraphNode* machine);
	void UnselectMachine(CGraphNode* machine);
	bool IsSelectedMachine(int node_id);
	int GetSelectedMachines();
	CGraphNode* GetSelectedMachine(int index);
	void ClearSelectedMachines();
	void SelectConnection(CGraphNodePair const& conn);
	void SelectConnection(int to_node_id, int from_node_id);
	void UnselectConnection();
	bool GetMachineHideIncomingConnections(int node_id);
	void SetMachineHideIncomingConnections(int node_id, bool state);
	void SetTheme(themecolor index, COLORREF color);
	void UpdateFromTheme();
	void UpdateConnectionToolTip();
};
