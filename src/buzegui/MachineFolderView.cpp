#include "stdafx.h"
#include "resource.h"
#include <buze/buzesdk.h>
#include <buze/ViewImpl.h>
#include "MachineFolderView.h"
#include "DragDropImpl.h"
#include "MachineDropTarget.h"
#include "utils.h"

//#include "BuzeConfiguration.h"
// 
// Factory
//
DWORD WINAPI PopulateMachinesThread(LPVOID lpParam);

CMachineFolderViewInfo::CMachineFolderViewInfo(buze_main_frame_t* m) : CViewInfoImpl(m) {
	uri = CMachineFolderView::GetWndClassInfo().m_wc.lpszClassName;
	label = "All Machines";
	tooltip = "All machines";
	place = 2; //DockSplitTab::placeDOCKPANE;
	side = 2; //DockSplitTab::dockLEFT;
	serializable = true;
	allowfloat = false;
	defaultview = false;
}

CView* CMachineFolderViewInfo::CreateView(HWND hWndParent, void* pCreateData) {
	CMachineFolderView* view = new CMachineFolderView(mainframe);
	view->Create(hWndParent, CWindow::rcDefault, label, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)0, 0);
	return view;
}

void CMachineFolderViewInfo::Attach() {
	buze_document_add_view(document, this);

	// global accelerators - these generate global document events caught in OnUpdate
	WORD ID_SHOW_ALLMACHINES = buze_main_frame_register_accelerator_event(mainframe, "view_allmachines", "f3 shift", buze_event_type_show_all_machines);

	// local accelerators - these generate local WM_COMMAND messages caught in the message map
	//mainframe->RegisterAccelerator("patternformatview", "help", ID_HELP);

	CMenuHandle mainMenu = (HMENU)buze_main_frame_get_main_menu(mainframe);
	CMenuHandle viewMenu = mainMenu.GetSubMenu(2);
	viewMenu.InsertMenu(-1, MF_BYCOMMAND, (UINT_PTR)ID_SHOW_ALLMACHINES, "All Machines");

}

void CMachineFolderViewInfo::Detach() {
	buze_document_remove_view(document, this);
}

void CMachineFolderViewInfo::OnUpdate(CView* pSender, LPARAM lHint, LPVOID pHint) {
	buze_event_data* ev = (buze_event_data*)pHint;
	CMachineFolderView* view;
	switch (lHint) {
		case buze_event_type_show_all_machines:
			view = (CMachineFolderView*)buze_main_frame_get_view(mainframe, "MachineFolderView", 0);
			if (view) {
				buze_main_frame_set_focus_to(mainframe, view);
			} else
				view = (CMachineFolderView*)buze_main_frame_open_view(mainframe, "MachineFolderView", "All Machines", 0, -1, -1);
			break;
		case buze_event_type_update_index:
			view = (CMachineFolderView*)buze_main_frame_get_view(mainframe, "MachineFolderView", 0);
			if (view) {
				view->treeCtrl.DeleteAllItems();
				DWORD dwId;
				HANDLE hThread = CreateThread(0, 0, PopulateMachinesThread, view, 0, &dwId);
				assert(hThread != 0);
				CloseHandle(hThread);
			}
			break;
	}
}

//
// View
//

using namespace std;

LRESULT CEditableTreeViewCtrl::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	bHandled = FALSE;
	return GetParent().SendMessage(uMsg, wParam, lParam);
}

CMachineFolderView::CMachineFolderView(CViewFrame* mainFrm)
	:CViewImpl(mainFrm)
{
	hMachineFolderSignal = CreateEvent(0, FALSE, FALSE, "");
	assert(hMachineFolderSignal != 0);
}

CMachineFolderView::~CMachineFolderView(void) {
}

void CMachineFolderView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

bool WaitAndPump(HANDLE hHandle);/* {
	MSG msg;
	for (;;) {
		if (WaitForSingleObject(hHandle, 0) == WAIT_OBJECT_0) 
			return true;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) return false;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}*/

LRESULT CMachineFolderView::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	WaitAndPump(hMachineFolderSignal);
	CloseHandle(hMachineFolderSignal);
	hMachineFolderSignal = 0;

	//mainFrame->closeClientWindow(m_hWnd);
	return 0;
}

HTREEITEM g_hCurrentDragItem = 0;
//extern MachineMenu* g_effectMenu;
//extern MachineMenu* g_generatorMenu;
//extern MachineMenu* g_controllerMenu;

LRESULT CMachineFolderView::OnBeginDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {

	NMTREEVIEW* pnmtv = (NMTREEVIEW*)pnmh;

	g_hCurrentDragItem = pnmtv->itemNew.hItem;
	const char* machineFileName = (const char*)treeCtrl.GetItemData(g_hCurrentDragItem);
	if (machineFileName == 0) return 0;

	FORMATETC fmtetc = {0};
	fmtetc.cfFormat = CF_TEXT;
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	fmtetc.tymed = TYMED_HGLOBAL;

	STGMEDIUM medium = {0};
	medium.tymed = TYMED_HGLOBAL;
	const TCHAR* str = machineFileName;//item->fileName.c_str(); //OLE2T(bstr.m_str);

	medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE, strlen(str)+1); //for NULL
	TCHAR* pMem = (TCHAR*)GlobalLock(medium.hGlobal);
	strcpy(pMem,str);
	GlobalUnlock(medium.hGlobal);


	CIDropSource* pdsrc = new CIDropSource();
	CIDataObject* pdobj = new CIDataObject(pdsrc);

	pdsrc->AddRef();
	pdobj->AddRef();

	pdobj->SetData(&fmtetc,&medium,TRUE);

	CDragSourceHelper dragSrcHelper;
	dragSrcHelper.InitializeFromWindow(m_hWnd, pnmtv->ptDrag, pdobj);

	DWORD dwEffect;
	::DoDragDrop(pdobj, pdsrc, DROPEFFECT_COPY, &dwEffect);

	return 0;
}

int CMachineFolderView::BindMachineItems(buze_plugin_index_item_t* item, HTREEITEM parent) {

	DWORD dwEnabled = 0;
	//mainFrame->document->configuration->getConfigNumber("Settings", "IndexEditing", &dwEnabled);

	int machineCounter = 0;

	int subitemcount = buze_plugin_index_item_get_sub_item_count(item);
	for (int i = 0; i < subitemcount; i++) {
		buze_plugin_index_item_t* subitem = buze_plugin_index_item_get_sub_item(item, i);

		if (buze_plugin_index_item_is_hidden(subitem)) continue;
		int type = buze_plugin_index_item_get_type(subitem);
		if (type <= 1) {
			char* itemData = 0;
			if (type == 1) {
				std::string fileName = buze_plugin_index_item_get_filename(subitem);
				std::string instrumentName = buze_plugin_index_item_get_instrumentname(subitem);
				bool is_template = fileName == "@zzub.org/buze/template";
				
				if (!is_template) {
					zzub_pluginloader_t* loader = zzub_player_get_pluginloader_by_name(player, fileName.c_str());
					if (loader == 0) continue;
					int flags = zzub_pluginloader_get_flags(loader);
					if (flags & zzub_plugin_flag_is_connection) continue;
				}

				// NOTE: fullMachineName is separated by slash for instruments
				//string smn=pluginUri;
				
				int itemDataLen = fileName.length();
				if (instrumentName.length())
					itemDataLen += instrumentName.length()+1;

				itemData = new char[itemDataLen+1];	// TODO: delete later!
				if (fileName.length())
					strcpy(itemData, fileName.c_str()); else
					strcpy(itemData, "");
				if (instrumentName.length()) {
					strcat(itemData, "|");
					strcat(itemData, instrumentName.c_str());
				}
				machineCounter++;
			}

			std::string name = buze_plugin_index_item_get_name(subitem);
			HTREEITEM treeItem = treeCtrl.InsertItem(name.c_str(), parent, TVI_LAST);
			treeCtrl.SetItemData(treeItem, (DWORD_PTR)itemData);

			// ii->userData = treeItem; // TODO: not in use, used for dropping

			if (type == 0) {
				int bindCount = BindMachineItems(subitem, treeItem);
				// keep even machine menus because we might want to drop something into them

				if (!bindCount && dwEnabled!=0) {
					HTREEITEM placeholder = treeCtrl.InsertItem( "(drop on me)", treeItem, TVI_LAST);
					treeCtrl.SetItemData(treeItem, (DWORD_PTR)0);
				}
				machineCounter += bindCount;
			}
		}
	}
	return machineCounter;
}

DWORD WINAPI PopulateMachinesThread(LPVOID lpParam) {
	CMachineFolderView* view = (CMachineFolderView*)lpParam;
	view->LoadFromIndex();

	/* should we put a message loop + critical section in here? */
	// ... //

	SetEvent(view->hMachineFolderSignal);
	// her skal vi poste en melding om at treet er ferdigpopulært
	return 0;
}

void CMachineFolderView::LoadFromIndex() {
	treeCtrl.SetRedraw(FALSE);
	HTREEITEM hMachines = treeCtrl.InsertItem("Gear Index", TVI_ROOT, TVI_LAST);
	HTREEITEM hGeneratorItem = treeCtrl.InsertItem("Generators", TVI_ROOT, TVI_LAST);
	HTREEITEM hEffectItem = treeCtrl.InsertItem("Effects", TVI_ROOT, TVI_LAST);
	HTREEITEM hControllerItem = treeCtrl.InsertItem("Controllers", TVI_ROOT, TVI_LAST);

	buze_plugin_index_item_t* rootItem = buze_document_get_plugin_index_root(document);
	BindMachineItems(rootItem, hMachines);	

	treeCtrl.Expand(hMachines);

	for (int i = 0; i < zzub_player_get_pluginloader_count(player); i++) {
		zzub_pluginloader_t* loader = zzub_player_get_pluginloader(player, i);
		HTREEITEM hmachineItem;
		if ((zzub_pluginloader_get_flags(loader) & PLUGIN_FLAGS_MASK) & IS_CONTROLLER_PLUGIN_FLAGS)
			hmachineItem = treeCtrl.InsertItem(zzub_pluginloader_get_name(loader), hControllerItem, TVI_LAST);
		else if ((zzub_pluginloader_get_flags(loader) & PLUGIN_FLAGS_MASK) & IS_EFFECT_PLUGIN_FLAGS)
			hmachineItem = treeCtrl.InsertItem(zzub_pluginloader_get_name(loader), hEffectItem, TVI_LAST);
		else if ((zzub_pluginloader_get_flags(loader) & PLUGIN_FLAGS_MASK) == IS_GENERATOR_PLUGIN_FLAGS)
			hmachineItem = treeCtrl.InsertItem(zzub_pluginloader_get_name(loader), hGeneratorItem, TVI_LAST);
		else // fallback to Generator
			hmachineItem = treeCtrl.InsertItem(zzub_pluginloader_get_name(loader), hGeneratorItem, TVI_LAST);
		treeCtrl.SetItemData(hmachineItem, (DWORD_PTR)zzub_pluginloader_get_uri(loader));
	}
	treeCtrl.SetRedraw(TRUE);
}

LRESULT CMachineFolderView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	using namespace std;
	treeCtrl.Create(this->m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_HASBUTTONS | TVS_TRACKSELECT, WS_EX_CLIENTEDGE, IDC_MACHINETREE);

	DWORD dwId;
	HANDLE hThread = CreateThread(0, 0, PopulateMachinesThread, this, 0, &dwId);
	assert(hThread != 0);

	CloseHandle(hThread);

	if (!createDropTarget(*this)) return 0;
	FORMATETC ftetc = {0}; 
	ftetc.cfFormat = CF_TEXT; 
	ftetc.dwAspect = DVASPECT_CONTENT; 
	ftetc.lindex = -1; 
	ftetc.tymed = TYMED_HGLOBAL; 
	dropTarget->AddSuportedFormat(ftetc); 
	ftetc.cfFormat=CF_HDROP; 
	dropTarget->AddSuportedFormat(ftetc);

	return 0;
}


LRESULT CMachineFolderView::OnSetFocus( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	treeCtrl.SetFocus();
	return 0;
}

LRESULT CMachineFolderView::OnSize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	WORD cx = LOWORD( lParam);
	WORD cy = HIWORD( lParam);
	
	treeCtrl.SetWindowPos( HWND_TOP, 0, 0, cx, cy, SWP_SHOWWINDOW);
	return 0;
}

LRESULT CMachineFolderView::OnSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled) {
	return 0;
}


//LRESULT CMachineFolderView::OnViewProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
//	return 0;
//}

LRESULT CMachineFolderView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	return 0;
}

LRESULT CMachineFolderView::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	return 0;
}


// copyitem / copybranch by Zafir Anjum
// http://www.codeguru.com/cpp/controls/treeview/dragdrop/article.php/c695/

HTREEITEM CopyItem(CTreeViewCtrl& treeView, HTREEITEM hItem, HTREEITEM htiNewParent, HTREEITEM htiAfter = TVI_LAST) {
	TV_INSERTSTRUCT tvstruct;
	HTREEITEM hNewItem;

	// get information of the source item
	tvstruct.item.hItem = hItem;
	tvstruct.item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	treeView.GetItem(&tvstruct.item);  
	char itemText[1024];
	treeView.GetItemText( hItem, itemText, 1024 );

	tvstruct.item.cchTextMax = strlen(itemText);
	tvstruct.item.pszText = itemText;

	// Insert the item at proper location
	tvstruct.hParent = htiNewParent;
	tvstruct.hInsertAfter = htiAfter;
	tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	hNewItem = treeView.InsertItem(&tvstruct);

	// Now copy item data and item state.
	treeView.SetItemData( hNewItem, treeView.GetItemData( hItem ));
	treeView.SetItemState( hNewItem, treeView.GetItemState( hItem, TVIS_STATEIMAGEMASK ), TVIS_STATEIMAGEMASK );

	return hNewItem;
}

HTREEITEM CopyBranch(CTreeViewCtrl& treeView, HTREEITEM htiBranch, HTREEITEM htiNewParent, HTREEITEM htiAfter = TVI_LAST) {
	HTREEITEM hChild;

	HTREEITEM hNewItem = CopyItem(treeView, htiBranch, htiNewParent, htiAfter);
	hChild = treeView.GetChildItem(htiBranch);
	while (hChild != NULL) {
		// recursively transfer all the items
		CopyBranch(treeView, hChild, hNewItem);  
		hChild = treeView.GetNextSiblingItem( hChild );
	}
	return hNewItem;
}

CMenuHandle FindMenuItem(CMenuHandle menuItem, std::string text, int& pos) {
	char menuText[1024];

	pos = -1;
	for (UINT i = 0; i < menuItem.GetMenuItemCount(); i++) {
		menuItem.GetMenuString(i, menuText, 1024, MF_BYPOSITION);
		if (text == menuText) {
			pos = i;
			return menuItem;
		}
		CMenuHandle submenu = menuItem.GetSubMenu(i);
		if (submenu.m_hMenu == 0) continue;

		submenu = FindMenuItem(submenu, text, pos);
		if (submenu.m_hMenu != 0) return submenu;

	}
	return CMenuHandle();
}

void* GetMenuItemData(CMenuHandle menu, int pos) {
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_DATA;
	menu.GetMenuItemInfo(pos, TRUE, &mii);
	return (void*)mii.dwItemData;
}

CMenuHandle FindMenuItemByData(CMenuHandle menuItem, void* data, int& pos) {
	pos = -1;
	for (UINT i = 0; i < menuItem.GetMenuItemCount(); i++) {
		void* itemData = GetMenuItemData(menuItem, i);
		if (itemData == data) {
			pos = i;
			return menuItem;
		}
		CMenuHandle submenu = menuItem.GetSubMenu(i);
		if (submenu.m_hMenu == 0) continue;

		submenu = FindMenuItemByData(submenu, data, pos);
		if (submenu.m_hMenu != 0) return submenu;

	}
	return CMenuHandle();
}

int CountTreeItems(CTreeViewCtrl& tree, HTREEITEM hItem = NULL, BOOL recurse = TRUE) {
	int count = 0;
	if (hItem == NULL)
		hItem = tree.GetSelectedItem();

	if (tree.ItemHasChildren(hItem)) {
		hItem = tree.GetNextItem(hItem, TVGN_CHILD);
		while (hItem) {
			count++;
			if (recurse)
				count += CountTreeItems(tree, hItem, recurse);
			hItem = tree.GetNextItem(hItem, TVGN_NEXT);
		}
	}
	return count;
}

#include "MachineIndex.h"

bool CMachineFolderView::OnDropMachine(std::string uri, std::string instrumentName, int x, int y) {

	treeCtrl.RemoveInsertMark();

	DWORD dwEnabled = 0;
	//mainFrame->document->configuration->getConfigNumber("Settings", "IndexEditing", &dwEnabled);
	if (!dwEnabled) return true;
/*
	POINT pt = { x, y };
	UINT flags;
	HTREEITEM dropItem = treeCtrl.HitTest(pt, &flags);
	if (dropItem == 0) return true;

	// TODO: if dropped item is an expandable thing and its expanded, we want to add the dragItem as the first child here

	IndexItem* dropIndex = mainFrame->document->machineIndex.root.getItemByData(dropItem);
	IndexItem* dragIndex = mainFrame->document->machineIndex.root.getItemByData(g_hCurrentDragItem);

	if (dropIndex == dragIndex) return true;

	// are we dropping in a machine list?
	if (dropIndex && dropIndex->type == 0) {
		MachineMenu* mm = (MachineMenu*)dropIndex;
		// are we dropping on a preloaded machine?
		if (mm->preloadReplaced != 0) return true;
	} else
	if (dropIndex && dropIndex->type == 1) {
		if (dropIndex->parent->type == 0) {
			MachineMenu* mm = (MachineMenu*)dropIndex->parent;
			// are we dropping inside a preloaded machine?
			if (mm->preloadReplaced != 0) return true;
		}
	}

	HTREEITEM dropParent = treeCtrl.GetParentItem(dropItem);
	HTREEITEM dragParent = treeCtrl.GetParentItem(g_hCurrentDragItem);

	char dragItemText[1024];
	treeCtrl.GetItemText(g_hCurrentDragItem, dragItemText, 1024);
	char dropItemText[1024];
	treeCtrl.GetItemText(dropItem, dropItemText, 1024);

	// need parent of where we are dropping in the index, 
	// if there is none, disallow attempt to drop outside the index
	IndexItem* parentIndex = mainFrame->document->machineIndex.root.getItemByData(dropParent);
	if (!parentIndex) return true;

	treeCtrl.SetRedraw(FALSE);

	HTREEITEM dragCopy = CopyBranch(treeCtrl, g_hCurrentDragItem, dropParent, dropItem);
	if (dragIndex != 0) {
		treeCtrl.DeleteItem(g_hCurrentDragItem);
		int dragFromSiblings = CountTreeItems(treeCtrl, dragParent);
		if (dragFromSiblings == 0) {
			HTREEITEM placeholder = treeCtrl.InsertItem("(drop on me)", dragParent, TVI_LAST);
			treeCtrl.SetItemData(dragParent, (DWORD)0);
		}
	}

	bool created = false;
	if (dragIndex == 0) {
		// we dragged an item from out-of-index into the index, so we create a new item for it
		//std::vector<const zzub::info*>::iterator it = std::find_if(mainFrame->player->plugin_infos.begin(), mainFrame->player->plugin_infos.end(), find_info_by_uri(machineName));
		//zzub::pluginloader* loader = mainFrame->player->getMachineLoader(machineName);
		zzub_pluginloader_t* loader = zzub_player_get_pluginloader_by_name(mainFrame->player, uri.c_str());

		MachineItem* mi = new MachineItem();
		mi->label = zzub_pluginloader_get_name(loader);
		mi->fileName = zzub_pluginloader_get_name(loader);;
		mi->fullMachineName = uri;
		mi->instrumentName = instrumentName;
		dragIndex = mi;

		created = true;
	}
	dragIndex->userData = dragCopy;

	if (!created) dragIndex->parent->removeItem(dragIndex);

	if (dropIndex != 0) {
		dropIndex->parent->insertAfter(dragIndex, dropIndex);
	} else {
		if ((std::string)dropItemText == "(drop on me)") {
			treeCtrl.DeleteItem(dropItem);
		}
		parentIndex->append(dragIndex);
	}

	treeCtrl.SetRedraw(TRUE);

	IndexItem* generatorParent = g_generatorMenu->parent;
	IndexItem* effectParent = g_effectMenu->parent;
	IndexItem* controllerParent = g_controllerMenu->parent;

	// temporarily remove the Unsorted Generators and Unsorted Effects
	generatorParent->removeItem(g_generatorMenu);
	effectParent->removeItem(g_effectMenu);
	controllerParent->removeItem(g_controllerMenu);

	// we could also revert the PlaceGeneratorHere-thing but its not needed
	mainFrame->document->machineIndex.save(_Module.mapPath("Gear/index.txt").c_str());

	generatorParent->append(g_generatorMenu);
	effectParent->append(g_effectMenu);
	controllerParent->append(g_controllerMenu);

	g_hCurrentDragItem = 0;

	// rebuild right-click menu in machine view because we are now out of sync
	mainFrame->rebuildMachineMenus();
*/
	return true;
}

bool CMachineFolderView::OnDragOver(const POINTL& ptl, FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect) {
	POINT pt = { ptl.x, ptl.y };
	treeCtrl.ScreenToClient(&pt);
	UINT flags;
	HTREEITEM item = treeCtrl.HitTest(pt, &flags);
	if (item == 0) return true;
		
	treeCtrl.SetInsertMark(item, TRUE);

	return true;
}

void CMachineFolderView::OnDragLeave() {
	treeCtrl.RemoveInsertMark();
}
