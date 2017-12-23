#ifndef __DOCKTAB_PANE_H__
#define __DOCKTAB_PANE_H__

#ifndef __ATLWIN_H__
	#error DockTabSplitPane.h requires atlwin.h to be included first
#endif

#ifndef __ATLSPLIT_H__
	#error DockTabSplitPane.h requires atlsplit.h to be included first
#endif

#ifndef __ATLCOLL_H__
	#error DockTabSplitPane.h requires atlcoll.h to be included first
#endif

/*********************************************************************
DockSplitTab::TabControl and 
DockSplitTab::TabPane classes implementation

Written by Igor Katrayev.
Copyright (c) 2003 Igor Katrayev.

This code may be used in compiled form in any way you desire. This
file may be redistributed unmodified by any means PROVIDING it is 
not sold for profit without the authors written consent, and 
providing that this notice and the authors name is included. 

This code is also based of work done by Bjarke Viksoe (bjarke@viksoe.dk)
and Daniel Bowen (dbowen@es.com).

This file is provided "as is" with no expressed or implied warranty.
The author accepts no liability if it causes any damage to you or your
computer whatsoever. It's free, so don't hassle me about it.

Beware of bugs.

HISTORY:

Dec 31,2004; Igor Katrayev:
came up with the first version

Jan 03,2004; Igor Katrayev:
[+] rewrote drag and drop code for switching tabs using CallBackListener::trackDragAndDrop function.
[+] renamed window class names for Pane and TabControl classes

**********************************************************************/

#include "CustomTabCtrl.h"
#include "DotNetTabCtrl.h" 

namespace DockSplitTab {
	
	class CallBackListener {
	public:
		
		// triggered when client view client view wnd has gained the keyboard focus
		virtual void clientActivate(  HWND childWnd, HWND clientViewWnd) = 0;
		// triggered when client view client view wnd got doble mouse click on the tab button
		virtual void clientDblClick(  HWND childWnd, HWND clientViewWnd) = 0;
		// triggered the close button was pushed for the client view client
		virtual void clientCloseClick(     HWND childWnd, HWND clientViewWnd) = 0;
		
		// drag and drop notifications
		virtual void dragStart(  HWND childWnd, HWND clientViewWnd, long x, long y, DWORD keysPressed) = 0;
		virtual void dragOver(   HWND childWnd, HWND clientViewWnd, long x, long y, DWORD keysPressed) = 0;
		virtual void dragDrop(   HWND childWnd, HWND clientViewWnd, long x, long y, DWORD keysPressed) = 0;
		virtual void dragCancel( HWND childWnd, HWND clientViewWnd) = 0;
		
		// performs the drag and drop tracking with drag and drop notifications
		void trackDragAndDrop( HWND hWnd, POINT startPoint, bool lockWindowUpdate = false, HWND clientViewWnd = NULL) {
			
			ATLASSERT( NULL != this);
			ATLASSERT( ::IsWindow( hWnd));
			
			///MEGZ: This here is the bastardly line that causes the whole thing to freeze up when clicking on tabs
			//if ( !::DragDetect( hWnd, startPoint))
			//	return;
				
			// tracker drawing conflicts with dock pane drawing
			// disable drawing in the window during drag and drop operations.
			if ( lockWindowUpdate)
				::LockWindowUpdate( hWnd);
			
			MSG msg;
			bool dragging = false;
			::SetCapture( hWnd);
			while( ( ::GetCapture()==hWnd) && ( ::GetMessage( &msg, NULL, 0, 0))) {
				
				CPoint hitPoint = CPoint( GET_X_LPARAM( msg.lParam), GET_Y_LPARAM( msg.lParam));
				::ClientToScreen( hWnd, &hitPoint);
				switch( msg.message) {
				case WM_MOUSEMOVE:
					
					if ( !dragging) {
						
						dragging = true;
						this->dragStart( hWnd, clientViewWnd, hitPoint.x, hitPoint.y, static_cast<DWORD>(msg.wParam));
					}
					this->dragOver( hWnd, clientViewWnd, hitPoint.x, hitPoint.y, static_cast<DWORD>(msg.wParam));
					break;
					
				case WM_LBUTTONUP:
					
					if ( dragging) {
						dragging = false;
						this->dragDrop( hWnd, clientViewWnd, hitPoint.x, hitPoint.y, static_cast<DWORD>(msg.wParam));
					}
					::ReleaseCapture();
					break;
					
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					
					switch ( msg.wParam) {
					case VK_CONTROL:
					case VK_SHIFT:
						break;
					default:
						if ( dragging) {
							dragging = false;
							this->dragCancel( hWnd, NULL);
						}
						::ReleaseCapture();
					}
				default:
					::TranslateMessage(&msg);
					::DispatchMessage( &msg);
				}
			}
			
			// tidy up
			if ( dragging)
				this->dragCancel( hWnd, clientViewWnd);
			
			if ( lockWindowUpdate)
				::LockWindowUpdate( NULL);
			return;
		}
		
	}; // interface CallBackListenerr
	
	class TabControlItem: public CCustomTabItem {
	public:
		
		TabControlItem()
			: CCustomTabItem()
			, clientViewWnd( NULL)
		{}
		
		HWND clientViewWnd;
	};
	
	class TabControl: public CDotNetTabCtrlImpl< TabControl, TabControlItem> {
	
	// protected class members
	protected:
		
		typedef CDotNetTabCtrlImpl< TabControl, TabControlItem> baseClass;
		CallBackListener* cbListener;
		int height;
		
	public:
		
		TabControl( CallBackListener* listener)
			: CDotNetTabCtrlImpl<TabControl, TabControlItem>()
			, cbListener( listener)
			, height( 0)
		{}
		
		// Methods
		void create( HWND parentWnd, bool tabOnTop = false) {
			
			
			DWORD style = TCS_FOCUSNEVER | WS_VISIBLE | WS_CHILD;//| TCS_FLATBUTTONS | TCS_BUTTONS
			
			style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CTCS_SCROLL | CTCS_CLOSEBUTTON | CTCS_FLATEDGE | CTCS_BOLDSELECTEDTAB | CTCS_TOOLTIPS;

			if ( !tabOnTop)
				style |= CTCS_BOTTOM;
			
			DWORD exStyle = TCS_EX_REGISTERDROP; //| TCS_EX_FLATSEPARATORS
			this->Create( parentWnd, rcDefault, NULL, style, exStyle);
			this->SetDlgCtrlID(0);
			
			// calculate tab height
			const int nNominalHeight = 24;
			const int nNominalFontLogicalUnits = 11;	// 8 point Tahoma with 96 DPI
			
			// Initialize nFontLogicalUnits to the typical case
			// appropriate for CDotNetTabCtrl
			LOGFONT lfIcon = { 0 };
			::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfIcon), &lfIcon, 0);
			int nFontLogicalUnits = -lfIcon.lfHeight;
			
			// Use the actual font of the tab control
			if( this->IsWindow()) {
				
				CFontHandle hFont = this->GetFont();
				if ( hFont != NULL) {
					
					CDC dc = this->GetDC();
					CFontHandle hFontOld = dc.SelectFont(hFont);
					TEXTMETRIC tm = {0};
					dc.GetTextMetrics(&tm);
					nFontLogicalUnits = tm.tmAscent;
					dc.SelectFont(hFontOld);
				}
			}
			
			this->height = nNominalHeight + ( ::MulDiv(nNominalHeight, nFontLogicalUnits, nNominalFontLogicalUnits) - nNominalHeight ) / 2;
		}
		
		int getHeight() {
			return this->height;
		}
		
		HWND getWND( int index) const {
			
			ATLASSERT( 0 <= index && index < this->GetItemCount());
			
			return this->GetItem( index)->clientViewWnd;
		}
		
		int getIndex( HWND win) const {
			
			for ( int i=0; i<this->GetItemCount(); i++)
				if ( this->GetItem( i)->clientViewWnd == win)
					return i;
			
			return -1;
		}
		
		bool removeTab( HWND clientViewWnd) {
			
			int index = this->getIndex( clientViewWnd);
			if ( index == -1)
				return false;
			
			return this->DeleteItem( index) == TRUE;
		}
		
		bool removeTab( int index) {
			
			ATLASSERT( 0 <= index && index < this->GetItemCount());
			
			return this->DeleteItem( index) == TRUE;
		}
		
		int getCurrentTab() const {
			
			return this->GetCurSel();
		}
		
		int setCurrentTab( int index) {
			
			ATLASSERT( 0 <= index && index < this->GetItemCount());
			
			int result = this->SetCurSel( index);
			if ( result > -1 && index != result)
				::ShowWindow( this->getWND( result), FALSE);
			return result;
		}
		
		// Message map handlers
		DECLARE_WND_CLASS(_T("DockSplitTab::TabControl"))  
		
		BEGIN_MSG_MAP( TabControl)
			
			MESSAGE_HANDLER( WM_CREATE,        OnCreate)
			///MESSAGE_HANDLER( WM_CONTEXTMENU,   OnContextMenu)///MEGZ flicker
			MESSAGE_HANDLER( WM_LBUTTONDOWN,   OnLButtonDown)
			
			CHAIN_MSG_MAP( baseClass)
		END_MSG_MAP()
		
		LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
			CPoint hitPoint( GET_X_LPARAM( lParam), GET_Y_LPARAM( lParam));
			this->ScreenToClient( &hitPoint);
			CTCHITTESTINFO hci;
			hci.pt.x = hitPoint.x;
			hci.pt.y = hitPoint.y;
			hci.flags = TCHT_ONITEM;
			int tabItemTarget = this->HitTest( &hci);
			if ( tabItemTarget > -1) {
				
				this->SetCurSel( tabItemTarget);
				::SendMessage( this->getWND( tabItemTarget)
							, WM_CONTEXTMENU
							, wParam
							, lParam
							);
			}
			return 0;
		}
		
		LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
			// process with the default handler. select a ñhosen item
			LRESULT result = 0;
			baseClass::ProcessWindowMessage( this->m_hWnd, uMsg, wParam, lParam, result);
			
			CPoint hitPoint( GET_X_LPARAM( lParam), GET_Y_LPARAM( lParam));
			
			CTCHITTESTINFO hci;
			hci.pt.x = hitPoint.x;
			hci.pt.y = hitPoint.y;
			hci.flags = TCHT_ONITEM;
			int tabItem = this->HitTest( &hci);
			
			if ( tabItem > -1) {
				
				this->ClientToScreen( &hitPoint);
				this->cbListener->trackDragAndDrop( this->m_hWnd, hitPoint, false, this->getWND( tabItem));
			}
			return result;
		}
		
		LRESULT OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
			return this->DefWindowProc( uMsg, wParam, lParam);
		}
		
		LRESULT OnEraseBackground( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			return 1;
		}
		
	};
	
	class ClientProperties {
	public:
		
		CString caption;
		CString toolTip;
		int     imageIndex;
	};
	
	// class Pane
	class Pane
		: public CWindowImpl<Pane, CWindow>
		, public CallBackListener
	{
	
	//----------------- protected classes
	protected:
		
		class DragContext {
		public:
			
			int     tabItem;
			CPoint  hitPoint;
			bool    draggingOut;
			HCURSOR oldCursor;
			HCURSOR cursorSizeWE;
			HCURSOR cursorSizeAll;///megz
			
			DragContext( int tabItem, LPPOINT hitPoint)
				: tabItem( tabItem)
				, hitPoint( *hitPoint)
				, draggingOut( false)
			{
				this->cursorSizeWE = ::LoadCursor( NULL, IDC_SIZEWE);
				this->cursorSizeAll = ::LoadCursor( NULL, IDC_SIZEALL);///megz
				this->oldCursor = ::GetCursor();
			}
			
			~DragContext() {
				
				if ( NULL != this->cursorSizeWE)  ::DestroyCursor( this->cursorSizeWE);
				if ( NULL != this->cursorSizeAll) ::DestroyCursor( this->cursorSizeAll);///megz
				if ( NULL != this->oldCursor)     ::DestroyCursor( this->oldCursor);
			}
		};
		
	//----------------- protected members
	protected:
		
		TabControl        tabs;
		CallBackListener* cbListener;
		bool              isTabsOnTop;
		DragContext*      dragContext;
		
	//----------------- protected interface
	protected:
		
		int insertTab( int index, const TCHAR* caption, HWND clientViewWnd, const TCHAR* toolTip = NULL, int imageIndex = -1) {
			
			ATLASSERT( ::IsWindow( clientViewWnd));
			
			::SetParent( clientViewWnd, this->m_hWnd);
			TabControlItem* item = this->tabs.CreateNewItem();
			item->clientViewWnd = clientViewWnd;
			item->SetText( caption);
			item->SetToolTip( toolTip);
			item->SetImageIndex( imageIndex);
			return this->tabs.InsertItem( index, item, true);
		}
		
		int getCurrentTab() const {
			
			return this->tabs.getCurrentTab();
		}
		
		int setCurrentTab( int index) {
			
			return this->tabs.setCurrentTab( index);
		}
		
		int getTabIndex( HWND win) const {
			
			return this->tabs.getIndex( win);
		}
		
		bool deleteTab( int index) {
			
			ATLASSERT( 0 <= index && index < this->tabs.GetItemCount());
			
			return this->tabs.DeleteItem( index) == TRUE;
		}
		
		HWND getTabWND( int index) const {
			
			ATLASSERT( 0 <= index && index < this->tabs.GetItemCount());
			
			return this->tabs.getWND( index);
		}
		
	//----------------- public interface
	public:
		
		// Constructor/destructor
		Pane( CallBackListener* listener, bool isTabsOnTop = true)
			: tabs( this)
			, cbListener( listener)
			, isTabsOnTop( isTabsOnTop)
			, dragContext( NULL)
		{
			ATLASSERT( listener);
		}
		
		~Pane() {
			
			if ( this->m_hWnd) {
				
				this->DestroyWindow();
				this->m_hWnd = NULL;
			}
		}
		
		// public interface

		void setTabText(HWND clientView, const char* text) {
			int tabIndex   = this->getTabIndex( clientView);
			TabControlItem* item = this->tabs.GetItem(tabIndex);
			item->SetText(text);

		}
		
		void setActive(bool active) {
			tabs.SetFocused(active);
		}
		
		void showCurrent() {
			
			int clientCount = this->clientCount();
			if ( clientCount > 0) {
				
				CRect tabPageRect;
				this->getTabPageRect( tabPageRect);
				::SetWindowPos( this->getTabWND( this->getCurrentTab())
				              , HWND_TOP
				              , tabPageRect.left
				              , tabPageRect.top
				              , tabPageRect.Width()
				              , tabPageRect.Height()
				              , SWP_SHOWWINDOW
				              );
			} else
				this->tabs.SetWindowPos( HWND_BOTTOM, 0,0,0,0, SWP_HIDEWINDOW);
		}
		
		void getTabRect( LPRECT tabRect) {
			
			ATLASSERT( NULL != tabRect);
			
			int tabHeight;
			if ( this->tabs.GetItemCount() > 0) {
				
				tabHeight = this->tabs.getHeight();
			} else
				tabHeight = 0;
			
			this->GetClientRect( tabRect);
			if ( this->isTabsOnTop)
				tabRect->bottom = tabHeight;
			else
				tabRect->top = tabRect->bottom - tabHeight;
		}
		
		void getTabPageRect( LPRECT tabPageRect) {
			
			ATLASSERT( NULL != tabPageRect);
			
			int tabHeight;
			if ( this->tabs.GetItemCount() > 0) {
				
				tabHeight = this->tabs.getHeight();
			} else
				tabHeight = 0;
			this->GetClientRect( tabPageRect);
			if ( this->isTabsOnTop)
				tabPageRect->top += tabHeight;
			else
				tabPageRect->bottom = tabPageRect->bottom - tabHeight;
		}
		
		bool isEmpty() {
			
			return this->clientCount() == 0;
		}
		
        HWND getClientView(int index) {
            return this->tabs.GetItem(index)->clientViewWnd;
        }
		int clientCount() {
			
			return this->tabs.GetItemCount();
		}
		
		bool setFocusTo( HWND clientViewWnd) {
			
			int index = this->getTabIndex( clientViewWnd);
			if ( -1 < index) {
				
				this->setCurrentTab( index);
				CRect clientRect;
				this->getTabPageRect( clientRect);
				CWindow win( clientViewWnd);
				win.SetWindowPos( HWND_TOP, clientRect, SWP_SHOWWINDOW);
				return true;
			}

			return false;
		}
		
		bool removeClientView( HWND childWnd) {
			
			int currentTab = this->getCurrentTab();
			int tabIndex   = this->getTabIndex( childWnd);
			
			if ( tabIndex == currentTab && this->clientCount() > 1)
				this->setCurrentTab( ( tabIndex == 0)? 1: tabIndex-1);
			
			bool result = this->tabs.removeTab( tabIndex);
			
			this->showCurrent();
			
			return result;
		};
		
		HWND moveCurrentTabTo( Pane* targetPane) {
			
			ATLASSERT( NULL != targetPane);
			
			int currentTab = this->getCurrentTab();
			
			TabControlItem* item = this->tabs.GetItem( currentTab);
			HWND result = item->clientViewWnd;
			int insertedTab = targetPane->clientCount();
			targetPane->insertTab( insertedTab
			                     , item->GetText()
			                     , item->clientViewWnd
			                     , item->GetToolTip()
			                     , item->GetImageIndex()
			                     );
			
			targetPane->setCurrentTab( insertedTab);
			this->tabs.DeleteItem( currentTab);
			this->showCurrent();
			
			return result;
		}
		
		void updateLayout() {
			
			if ( this->tabs.GetItemCount() > 0) {
				
				CRect rect;
				this->GetClientRect( &rect);
				WORD width  = rect.Width();
				WORD height = rect.Height();
				
				this->tabs.GetItemRect( 0, &rect);
				int tabHeight = this->tabs.getHeight();
				
				this->tabs.SetWindowPos( HWND_TOP
				                       , 0
				                       , ( this->isTabsOnTop)? 0: height - tabHeight
				                       , width
				                       , tabHeight
				                       , SWP_SHOWWINDOW
				                       );
				
				int currentTabIndex = this->getCurrentTab();
				if ( currentTabIndex > -1) {
					
					HWND hWnd = this->getTabWND( currentTabIndex);
					::SetWindowPos( hWnd
					              , HWND_TOP
					              , 0
								  , ( this->isTabsOnTop)? tabHeight: 0
					              , width
					              , height - tabHeight
					              , SWP_SHOWWINDOW
					              );
				}
			} else
				this->tabs.SetWindowPos( HWND_TOP, &rcDefault, SWP_SHOWWINDOW);
		}
		
		bool append( const TCHAR* caption, HWND hWnd, const TCHAR* toolTip = NULL, int imageIndex = -1) {
			
			int lastTabIndex = this->clientCount();
			
			this->insertTab( lastTabIndex, caption, hWnd, toolTip, imageIndex);
			return true;
		}
		
		bool get( HWND clientViewWnd, ClientProperties& clientProperties) {
			
			for ( int i=0; i<this->clientCount(); i++) {
				
				TabControlItem* item = this->tabs.GetItem( i);
				if ( clientViewWnd == item->clientViewWnd) {
					clientProperties.caption    = item->GetText();
					clientProperties.imageIndex = item->GetImageIndex();
					clientProperties.toolTip    = item->GetToolTip();
					return true;
				}
			}
			
			return false;
		}
		

		HWND focusedClientView() const {
			
			int index = this->getCurrentTab();
			return index > -1? this->tabs.getWND( index): NULL;
		}
		
		void setImageList( HIMAGELIST imageList) {
			
			::SendMessage( this->tabs.m_hWnd, TCM_SETIMAGELIST, 0, (LPARAM)imageList);
		}
		
		enum TabPaneHitTest {
			  TabPaneHitTest_Unknown = 0
			, TabPaneHitTest_TabArea = 1
			, TabPaneHitTest_Top     = 2
			, TabPaneHitTest_Right   = 3
			, TabPaneHitTest_Left    = 4
			, TabPaneHitTest_Bottom  = 5
		};
		
		TabPaneHitTest hitTest( LPPOINT point) {
			
			ATLASSERT( NULL != point);
			
			CPoint hitPoint( *point);
			this->ScreenToClient( &hitPoint);
			CRect rect;
			this->getTabRect( &rect);
			if ( rect.PtInRect( hitPoint))
				return TabPaneHitTest_TabArea;
			
			this->getTabPageRect( &rect);
			if ( !rect.PtInRect( hitPoint))
				return TabPaneHitTest_Unknown;
			
			TabPaneHitTest result = TabPaneHitTest_Unknown;
			long dsize  = 0;
			long top    = hitPoint.y - rect.top;
			long bottom = rect.bottom - hitPoint.y;
			if ( top < bottom) {
				result = TabPaneHitTest_Top;
				dsize  = top;
			} else {
				result = TabPaneHitTest_Bottom;
				dsize  = bottom;
			}
			long left  = hitPoint.x - rect.left;
			if ( left < dsize) {
				result = TabPaneHitTest_Left;
				dsize = left;
			}
			long right = rect.right - hitPoint.x;
			if ( right < dsize)
				result = TabPaneHitTest_Right;
			
			return result;
		}
		
		// CallBackListener inteface
		void clientActivate(  HWND childWnd, HWND clientViewWnd) {
			
			this->cbListener->clientActivate( this->m_hWnd, clientViewWnd);
			return;
		}
		
		void clientDblClick(  HWND childWnd, HWND clientViewWnd) {
			
			this->cbListener->clientDblClick( this->m_hWnd, clientViewWnd);
			return;
		}
		
		void clientCloseClick(  HWND childWnd, HWND clientViewWnd) {
			
			this->cbListener->clientCloseClick( this->m_hWnd, clientViewWnd);
			return;
		}
		
		void dragStart( HWND client, HWND clientViewWnd, long x, long y, DWORD keysPressed) {
			
			ATLASSERT( NULL == this->dragContext);
			
			CPoint hitPoint( x, y);
			this->tabs.ScreenToClient( &hitPoint);
			int tabItem = this->tabs.getIndex( clientViewWnd);
			this->dragContext = new DragContext( tabItem, &hitPoint);
			return;
		}
		
		void dragOver( HWND client, HWND clientViewWnd, long x, long y, DWORD keysPressed) {
			
			ATLASSERT( NULL != this->dragContext);
			
			CPoint hitPoint( x, y);
			this->tabs.ScreenToClient( &hitPoint);
			CTCHITTESTINFO hci;
			hci.pt.x = hitPoint.x;
			hci.pt.y = hitPoint.y;
			hci.flags = TCHT_ONITEM;
			int tabItemTarget = this->tabs.HitTest( &hci);

// 			::SetCursor( (    tabItemTarget == this->dragContext->tabItem
// 			               && this->dragContext->hitPoint != hitPoint
// 			               && ( 0 == ( MK_CONTROL & keysPressed))
// 			             )
// 			             ? this->dragContext->cursorSizeWE
// 			             : this->dragContext->oldCursor
// 			           );
			///megz
			if ( tabItemTarget == -1 && ( 0 == ( MK_CONTROL & keysPressed))) {
				::SetCursor( this->dragContext->cursorSizeAll);
			} else
			if ( this->dragContext->hitPoint != hitPoint && ( 0 == ( MK_CONTROL & keysPressed))) {
				::SetCursor( this->dragContext->cursorSizeWE);
			} else {
				::SetCursor( this->dragContext->oldCursor);
			}
			
			if ( tabItemTarget > -1 && tabItemTarget != this->dragContext->tabItem && ( 0 == ( MK_CONTROL & keysPressed))) {

				RECT rcItemDP;///megz
				this->tabs.GetItemRect(tabItemTarget, &rcItemDP);///megz
				int tab_halfwaypt = rcItemDP.left + ((rcItemDP.right - rcItemDP.left) / 2);///megz

				// tab shifting
				if ( this->dragContext->draggingOut) {
					this->dragContext->draggingOut = false;
					this->cbListener->dragCancel( this->m_hWnd, clientViewWnd);
				}
				
				bool shifted = false;///megz
				
				if ( tabItemTarget < this->dragContext->tabItem && hitPoint.x < tab_halfwaypt) {///megz
					shifted = true;///megz
					// shift the tab left
					for ( int i=this->dragContext->tabItem; i > tabItemTarget; i--)
						this->tabs.SwapItemPositions( i-1, i);
					
				} else if ( this->dragContext->tabItem < tabItemTarget && hitPoint.x > tab_halfwaypt) {///megz
					shifted = true;///megz
					// shift the tab right
					for ( int i=this->dragContext->tabItem; i < tabItemTarget; i++)
						this->tabs.SwapItemPositions( i, i+1);
				}
				
				if ( shifted) {///megz
					this->tabs.SetCurSel( tabItemTarget);
					this->dragContext->tabItem = tabItemTarget;
				}
			} else if ( tabItemTarget == -1) {
				
				// handle drag moving
				if ( !this->dragContext->draggingOut) {
					
					this->dragContext->draggingOut = true;
					if ( NULL != this->cbListener)
						this->cbListener->dragStart( this->m_hWnd, clientViewWnd, x, y, keysPressed);
				}
				if ( NULL != this->cbListener)
					this->cbListener->dragOver( this->m_hWnd, clientViewWnd, x, y, keysPressed);
			}
			this->dragContext->hitPoint = hitPoint;
			return;
		}
		
		void dragDrop( HWND client, HWND clientViewWnd, long x, long y, DWORD keysPressed) {
			
			ATLASSERT( NULL != this->dragContext);
			
			if ( this->dragContext->draggingOut)
				this->cbListener->dragDrop( this->m_hWnd, clientViewWnd, x, y, keysPressed);
			
			delete this->dragContext;
			this->dragContext = NULL;
			return;
		}
		
		void dragCancel( HWND client, HWND clientViewWnd) {
			
			ATLASSERT( NULL != this->dragContext);
			
			if ( this->dragContext->draggingOut)
				this->cbListener->dragCancel( this->m_hWnd, clientViewWnd);
			
			delete this->dragContext;
			this->dragContext = NULL;
			return;
		}
		
		// Overrideables
		BOOL DestroyWindow() throw() {
		
			ATLASSERT(::IsWindow(m_hWnd));
			
			::DestroyWindow( this->tabs.m_hWnd);
			::DestroyWindow( m_hWnd);
			this->tabs.m_hWnd = NULL;
			m_hWnd = NULL;
			return TRUE;
		}
		
		// Message Map
		DECLARE_WND_CLASS( "DockSplitTab::Pane")
		
		BEGIN_MSG_MAP(thisClass)
			MESSAGE_HANDLER( WM_CREATE,        OnCreate)
			MESSAGE_HANDLER( WM_ERASEBKGND,    OnEraseBackground)
			MESSAGE_HANDLER( WM_SETFOCUS,      OnSetFocus)
			MESSAGE_HANDLER( WM_SIZE,          OnSize)
			MESSAGE_HANDLER( WM_MOUSEACTIVATE, OnMouseActivate)
			
			NOTIFY_HANDLER( 0, CTCN_CLOSE,       OnTabClose);
			NOTIFY_HANDLER( 0, CTCN_SELCHANGING, OnTabSelChanging);
			NOTIFY_HANDLER( 0, CTCN_SELCHANGE,   OnTabSelChange);
			NOTIFY_HANDLER( 0, NM_DBLCLK,        OnTabDoubleClick);

			NOTIFY_ID_HANDLER( 0, OnTabNoForward);

			FORWARD_NOTIFICATIONS()
		END_MSG_MAP()
		
		LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
			
			LRESULT result = this->DefWindowProc( uMsg, wParam, lParam);
			if ( MA_ACTIVATE == result || MA_ACTIVATEANDEAT == result)
				this->SetFocus();
			return result;
		}

		LRESULT OnTabNoForward( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
			// anders: forwarding notifications for the tab control may lead to unexpected side effects
			return 0;
		}

		LRESULT OnTabClose( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
			
			LPNMCTCITEM notifyItem = reinterpret_cast<LPNMCTCITEM>(pnmh);
			if ( notifyItem->iItem > -1 && NULL != this->cbListener)
				this->cbListener->clientCloseClick( this->m_hWnd, this->tabs.getWND( notifyItem->iItem));
			return TRUE;
		}
		
		LRESULT OnTabDoubleClick( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
			
			LPNMCTCITEM notifyItem = reinterpret_cast<LPNMCTCITEM>(pnmh);
			if ( notifyItem->iItem > -1 && NULL != this->cbListener)
				this->cbListener->clientDblClick( this->m_hWnd, this->tabs.getWND( notifyItem->iItem));
			return TRUE;
		}
		
		LRESULT OnSetFocus( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
			
			int currentIndex = this->tabs.GetCurSel();
			if ( currentIndex > -1) {
				
				TabControlItem* item = this->tabs.GetItem( currentIndex);
				if ( NULL != ::SetFocus( item->clientViewWnd) && NULL != this->cbListener)
					this->cbListener->clientActivate( this->m_hWnd, item->clientViewWnd);
			} else if ( NULL != this->cbListener)
				this->cbListener->clientActivate( this->m_hWnd, NULL);
			return 0;
		}
		
		LRESULT OnTabSelChanging( int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/) {
			
			LPNMCTC2ITEMS nmct = reinterpret_cast< LPNMCTC2ITEMS>( pnmh);
			
			if ( nmct->iItem1 > -1) {
				HWND clientViewWnd = getTabWND( nmct->iItem1);
				if (nmct->showwindow) { //megz: bullshit fix
					::SetWindowPos( clientViewWnd , NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			return FALSE;
		}
		
		LRESULT OnTabSelChange( int idCtrl, LPNMHDR pnmh, BOOL& /*bHandled*/) {
		
			LPNMCTC2ITEMS nmct = reinterpret_cast< LPNMCTC2ITEMS>( pnmh);
			
			CRect rect;
			this->getTabPageRect( &rect);
			
			HWND clientViewWnd = this->getTabWND( this->getCurrentTab());
			if (nmct->showwindow) // megz: bullshit fix
				::SetWindowPos( clientViewWnd, HWND_TOP, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
			
			if ( NULL != ::SetFocus( clientViewWnd) && NULL != this->cbListener)
				this->cbListener->clientActivate( this->m_hWnd, clientViewWnd);
			
			return TRUE;
		}
		
		LRESULT OnCreate( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
			
			this->tabs.create( this->m_hWnd, this->isTabsOnTop);
			return 0;
		}
		
		LRESULT OnSize( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
			
			if ( this->tabs.GetItemCount() > 0) {
				
				WORD width  = LOWORD( lParam);
				WORD height = HIWORD( lParam);
				
				int tabHeight = this->tabs.getHeight();
				
				this->tabs.SetWindowPos( NULL
				                       , 0
				                       , ( this->isTabsOnTop)? 0: height - tabHeight
				                       , width
				                       , tabHeight
				                       , SWP_NOZORDER
				                       );
				
				int currentTabIndex = this->getCurrentTab();
				if ( currentTabIndex > -1) {
					
					HWND hWnd = this->getTabWND( currentTabIndex);
					::SetWindowPos( hWnd
					              , HWND_TOP
					              , 0
								  , ( this->isTabsOnTop)? tabHeight: 0
					              , width
					              , height - tabHeight
					              , SWP_SHOWWINDOW
					              );
				}
			} else
				this->tabs.SetWindowPos( HWND_TOP, &rcDefault, SWP_SHOWWINDOW);
			return 0;
		}
		
		LRESULT OnEraseBackground( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
			
			WTL::CDCHandle cdc = ( HDC) wParam;
			if ( this->tabs.GetItemCount() == 0) {
				
				CRect rect;
				this->GetClientRect( &rect);
				cdc.FillRect( rect, (HBRUSH) GetStockObject( DKGRAY_BRUSH));
			}
			return TRUE;
		}
		
	}; // class Pane
		
}; // namespace DockSplitTab

#endif // __DOCKTAB_PANE_H__