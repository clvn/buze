#pragma once

#ifndef __DOCKSPLITPANE_H__
#define __DOCKSPLITPANE_H__

#ifndef __cplusplus
	#error DockTabSplitPane.h requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLWIN_H__
	#error DockTabSplitPane.h requires atlwin.h to be included first
#endif

#ifndef __ATLAPP_H__
	#error DockTabSplitPane.h requires atlapp.h to be included first
#endif

#ifndef __ATLCOLL_H__
	#error DockTabSplitPane.h requires atlcoll.h to be included first
#endif

#ifndef __ATLMISC_H__
	#error DockTabSplitPane.h requires atlmisc.h to be included first
#endif

#ifndef __ATLSPLIT_H__
	#error DockTabSplitPane.h requires atlsplit.h to be included first
#endif

#ifndef __ATLGDI_H__
	#error DockTabSplitPane.h requires atlgdi.h to be included first
#endif

#ifndef __ATLCTRLS_H__
	#error DockTabSplitPane.h requires atlctrls.h to be included first
#endif

/*********************************************************************
DockSplitTab::SplitPane class implementation

Written by Igor Katrayev.
Copyright (c) 2003 Igor Katrayev.

This code may be used in compiled form in any way you desire. This
file may be redistributed unmodified by any means PROVIDING it is 
not sold for profit without the authors written consent, and 
providing that this notice and the authors name is included. 

This file is provided "as is" with no expressed or implied warranty.
The author accepts no liability if it causes any damage to you or your
computer whatsoever. It's free, so don't hassle me about it.

Beware of bugs.

HISTORY:

Dec 31,2004; Igor Katrayev:
came up with the first version

Jan 03,2004; Igor Katrayev:
[+] put CallBackListener interface calls in SplitPane::drag* functions
[+] added a new method - tab control on top ot on bottom of panes:
	bool SplitPane::tabOnTop();
[+] renamed window class names for Splitter class and SplitPane class

**********************************************************************/

#include "DockTabPane.h"

namespace DockSplitTab {

// destroy pane message
#define WM_USER_DESTROY_PANE WM_USER+1
	
	_declspec(selectany) POINT pointNULL = { -1, -1};
	_declspec(selectany) RECT  rectNULL  = { -1, -1, -1, -1};
	
	template <bool t_bVertical = true>
	class SplitterT: public CSplitterWindowImpl<SplitterT<t_bVertical> > {
	protected:
	public:
		SplitterT() : CSplitterWindowImpl<SplitterT<t_bVertical> >(t_bVertical) { }

		~SplitterT() {
			
			if ( this->m_hWnd) {
				
				::DestroyWindow( this->m_hWnd);
				this->m_hWnd = NULL;
			}
		}
		
		DECLARE_WND_CLASS( _T("DockSplitTab::Splitter"))
	};
	
	typedef SplitterT<true>  VSplitter;
	typedef SplitterT<false> HSplitter;
	
	
	class RectTracker {
	protected:
		HWND  trackWindow;
		CRect trackRect;
		/*
		static void drawRect( HDC hdc, LPRECT rect) {
			printf("drawRect\n");
			
			int width  = rect->right - rect->left;
			int height = rect->bottom - rect->top;
			const int size = 4;
			if ( width <= size || height <= size) {
				
				::PatBlt( hdc, rect->left, rect->top, width, height, PATINVERT);
			} else {
				
				::PatBlt( hdc, rect->left,       rect->top,         width-size,        size, PATINVERT);
				::PatBlt( hdc, rect->left+size,  rect->bottom-size, width-size,        size, PATINVERT);
				::PatBlt( hdc, rect->left,       rect->top+size,          size, height-size, PATINVERT);
				::PatBlt( hdc, rect->right-size, rect->top,               size, height-size, PATINVERT);
			}
		}
		*/
	public:
		
		RectTracker()
			: trackWindow( NULL)
			, trackRect( -1, -1, -1, -1)
		{}
		
		RectTracker( HWND trackWindow)
			: trackWindow( trackWindow)
			, trackRect( -1, -1, -1, -1)
		{}
		
		void emptyRect() {
			
			this->trackRect = rectNULL;
		}
		
		LPRECT getTrackRect() {
			
			return this->trackRect;
		}
		
		const HWND getTrackWindow() const {
			
			return this->trackWindow;
		}
		
		void drawTrackRectangle( HWND trackWindow, LPRECT rect, bool clear = true) {
			
			if ( this->trackRect.EqualRect( rect) && this->trackWindow == trackWindow)
				return;
			
			// draw a new track rectangle
			SIZE s = { 4, 4 };
			CWindowDC windowDC( trackWindow);
			CBrush brush = CDCHandle::GetHalftoneBrush();
			if ( NULL == brush.m_hBrush)
				return;
			
			RECT* rcLast = 0;
			HBRUSH bLast = 0;
			CBrushHandle brushOld = windowDC.SelectBrush( brush);
			if (clear) {
				windowDC.DrawDragRect(rect, s, &this->trackRect, s, brush, brush);

			} else {
				windowDC.DrawDragRect(rect, s, rcLast, s);
			}

			windowDC.SelectBrush(brushOld);
			
			this->trackRect   = rect;
			this->trackWindow = trackWindow;
		}
		
		void clearTrackRectangle( bool clear = true) {
			
			if ( this->trackRect.TopLeft() != pointNULL) {
				
				HWND wnd          = this->trackWindow;
				this->trackWindow = reinterpret_cast<HWND>( -1);
				this->drawTrackRectangle( wnd, this->trackRect, false);
				
				if ( clear) {
					
					this->trackRect = rectNULL;
					this->trackWindow = reinterpret_cast<HWND>( -1);
				} else
					this->trackWindow = wnd;
			}
		}
	};
	
	class SplitPane
		: public CWindowImplBaseT< CWindow, CControlWinTraits>
		, public CallBackListener

	{
	typedef CWindow TBase;
	typedef CControlWinTraits TWinTraits;
	typedef CWindowImplBaseT< CWindow, CControlWinTraits> ParentClass;
    friend class DockTabSerializer;
	
	//----------------- protected classes
	protected:
		
		class DragContext
			: public RectTracker {
		
		public:
			
			bool                 draggingOut;
			Pane::TabPaneHitTest tabPaneHitTest;
			
			// Constructor/destructor for DragContext class
			DragContext()
				: RectTracker()
				, tabPaneHitTest( Pane::TabPaneHitTest_Unknown)
				, draggingOut( false)
			{}
			
			~DragContext() {
				
				this->clearTrackRectangle();
			}
		};
		
	//----------------- protected members
	protected:
		
		WTL::CImageList imageList;
		
		CAtlMap< HWND, VSplitter*> verSplitterMap;
		
		CAtlMap< HWND, HSplitter*> horSplitterMap;
		
		CAtlMap< HWND, Pane*> paneMap;
		
		CAtlMap< HWND, Pane*> clientViewPaneMap;
		
		Pane* currentPane;
		HWND  rootWnd;
		
		DragContext* dragContext;
		CallBackListener* cbListener;
		bool tabOnTop;
		
	//----------------- protected interface
	protected:
		
		// Get a pane by a given point in screen coordinates
		Pane* getPane( long x, long y) {
			
			CPoint dropPoint( x, y);
			Pane* result = NULL;
			
			POSITION position = this->paneMap.GetStartPosition();
			if ( position)
				do {
					
					Pane* pane = this->paneMap.GetNextValue( position);
					CPoint pt = dropPoint;
					pane->ScreenToClient( &pt);
					CRect rect;
					pane->GetClientRect( &rect);
					if ( rect.PtInRect( pt)) {
						result = pane;
						break;
					}
				} while ( position);
			return result;
		} // Pane* getPane( long x, long y)
		
  		void moveCurrentTab( HWND sourcePane, HWND targetPane) {
            ATLASSERT( NULL != sourcePane && NULL != targetPane && targetPane != sourcePane);
			
            Pane* sp = lookUpPane(sourcePane);
            Pane* tp = lookUpPane(targetPane);

            if (sp == tp) return ;

            moveCurrentTab(sp, tp);
        }

		void moveCurrentTab( Pane* sourcePane, Pane* targetPane) {
			
			ATLASSERT( NULL != sourcePane && NULL != targetPane && targetPane != sourcePane);
			
			HWND clientWin = sourcePane->moveCurrentTabTo( targetPane);
			this->clientViewPaneMap[clientWin] = targetPane;
			
			HWND sourcePaneParentWnd = sourcePane->GetParent();
			targetPane->SetFocus();
			targetPane->updateLayout();
			
			if ( sourcePane->isEmpty() && sourcePaneParentWnd != this->m_hWnd) {
				// important:
				// destroy the source pane via the message queue becouse this call maybe done by a tab control
				// belonging to the source pane. we don't want to destroy it before it ends.
				this->PostMessage( WM_USER_DESTROY_PANE, 0, (LPARAM) sourcePane->m_hWnd);
			}
			return;
		} // moveCurrentTab( Pane* sourcePane, Pane* targetPane);
		
		HWND splitWithCurrentTab( Pane* sourcePane, Pane* targetPane, Pane::TabPaneHitTest where) {
			
			ATLASSERT( NULL != sourcePane);
			ATLASSERT( NULL != targetPane);
			ATLASSERT(    Pane::TabPaneHitTest_Right  == where
			           || Pane::TabPaneHitTest_Left   == where
			           || Pane::TabPaneHitTest_Top    == where
			           || Pane::TabPaneHitTest_Bottom == where
			         );
			
			VSplitter* verSplitter;
			HSplitter* horSplitter;
			CAtlMap< HWND, HSplitter*>::CPair* horSplitPair;
			CAtlMap< HWND, VSplitter*>::CPair* verSplitPair;
			CRect newPaneRect;
			CRect newSplitterRect;
			CRect sourcePaneRect;
			CRect targetPaneRect;
			
			HWND newSplitterWnd = NULL;
			HWND targetPaneParent = ::GetParent( targetPane->m_hWnd);
			
			CRect targetRect;
			targetPane->GetWindowRect( &targetRect);
			
			// create a new splitter
			bool isVertical;
			switch ( where) {
			case Pane::TabPaneHitTest_Right:
			case Pane::TabPaneHitTest_Left: {
					
					isVertical = true;
					verSplitter = new VSplitter();
					verSplitter->Create( targetPaneParent, &rectNULL, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE);
					this->verSplitterMap.SetAt( verSplitter->m_hWnd, verSplitter);
					
					newSplitterWnd = verSplitter->m_hWnd;
				}
				break;
				
			case Pane::TabPaneHitTest_Top:
			case Pane::TabPaneHitTest_Bottom: {
					
					isVertical = false;
					horSplitter = new HSplitter();
					horSplitter->Create( targetPaneParent, &rectNULL, NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE);
					this->horSplitterMap.SetAt( horSplitter->m_hWnd, horSplitter);
					
					newSplitterWnd = horSplitter->m_hWnd;
				}
				break;
			}
			
			// create a new pane and move the current tab from the target pane to the new one
			Pane* newPane = new Pane( this, this->tabOnTop);
			newPane->Create( newSplitterWnd);
			this->paneMap[ newPane->m_hWnd] = newPane;
			
			HWND clientWin = sourcePane->moveCurrentTabTo( newPane);
			this->clientViewPaneMap[ clientWin] = newPane;
			
			// assign the splitter as the parent window to the target pane
			targetPane->SetParent( newSplitterWnd);
			
			// work out with the splitter parent window
			if ( this->m_hWnd == targetPaneParent) {
				
				// the first splitter created
				this->rootWnd   = newSplitterWnd;
				newSplitterRect = CRect( 0, 0, targetRect.Width(), targetRect.Height());
				
			} else {
				
				// get a parent splitter for the target pane and reassign the pane to the new splitter created
				VSplitter* parentVerSplitter = NULL;
				HSplitter* parentHorSplitter = NULL;
				
				::SetParent( newSplitterWnd, targetPaneParent);
				if ( NULL != ( verSplitPair = this->verSplitterMap.Lookup( targetPaneParent))) {
					
					// it is a vertical splitter
					parentVerSplitter = verSplitPair->m_value;
					
					if ( parentVerSplitter->GetSplitterPane( SPLIT_PANE_LEFT) == targetPane->m_hWnd) {
						
						parentVerSplitter->SetSplitterPane( SPLIT_PANE_LEFT, newSplitterWnd, false);
						parentVerSplitter->GetSplitterPaneRect( SPLIT_PANE_LEFT, &newSplitterRect);
						
					} else if ( parentVerSplitter->GetSplitterPane( SPLIT_PANE_RIGHT) == targetPane->m_hWnd) {
						
						parentVerSplitter->SetSplitterPane( SPLIT_PANE_RIGHT, newSplitterWnd, false);
						parentVerSplitter->GetSplitterPaneRect( SPLIT_PANE_RIGHT, &newSplitterRect);
					}
					
				} else if ( NULL != ( horSplitPair = this->horSplitterMap.Lookup( targetPaneParent))) {
					
					// it is a horizontal splitter
					parentHorSplitter = horSplitPair->m_value;
					if ( parentHorSplitter->GetSplitterPane( SPLIT_PANE_LEFT) == targetPane->m_hWnd) {
						
						parentHorSplitter->SetSplitterPane( SPLIT_PANE_LEFT, newSplitterWnd, false);
						parentHorSplitter->GetSplitterPaneRect( SPLIT_PANE_LEFT, &newSplitterRect);
						
					} else if ( parentHorSplitter->GetSplitterPane( SPLIT_PANE_RIGHT) == targetPane->m_hWnd) {
						
						parentHorSplitter->SetSplitterPane( SPLIT_PANE_RIGHT, newSplitterWnd, false);
						parentHorSplitter->GetSplitterPaneRect( SPLIT_PANE_RIGHT, &newSplitterRect);
					}
					
				}
			}
			
			::SetWindowPos( newSplitterWnd, HWND_TOP, newSplitterRect.left, newSplitterRect.top, newSplitterRect.Width(), newSplitterRect.Height(), SWP_HIDEWINDOW);
			
			// set panes for the new splitter
			if ( isVertical) {
				
				int newPaneID;
				int targetPaneID;
				verSplitter->SetSplitterPos( -1, false);
				if ( where == Pane::TabPaneHitTest_Left) {
					
					targetPaneID = SPLIT_PANE_RIGHT;
					newPaneID    = SPLIT_PANE_LEFT;
					verSplitter->SetSplitterPanes( newPane->m_hWnd, targetPane->m_hWnd, false);
				} else {
					
					targetPaneID = SPLIT_PANE_LEFT;
					newPaneID    = SPLIT_PANE_RIGHT;
					verSplitter->SetSplitterPanes( targetPane->m_hWnd, newPane->m_hWnd, false);
				}
				verSplitter->GetSplitterPaneRect( targetPaneID, &targetPaneRect);
				verSplitter->GetSplitterPaneRect( newPaneID,    &newPaneRect);
				
			} else {
				
				int newPaneID;
				int targetPaneID;
				horSplitter->SetSplitterPos(-1, false);
				if ( where == Pane::TabPaneHitTest_Top) {
					
					targetPaneID = SPLIT_PANE_RIGHT;
					newPaneID    = SPLIT_PANE_LEFT;
					horSplitter->SetSplitterPanes( newPane->m_hWnd, targetPane->m_hWnd, false);
				} else {
					
					targetPaneID = SPLIT_PANE_LEFT;
					newPaneID    = SPLIT_PANE_RIGHT;
					horSplitter->SetSplitterPanes( targetPane->m_hWnd, newPane->m_hWnd, false);
				}
				horSplitter->GetSplitterPaneRect( targetPaneID, &targetPaneRect);
				horSplitter->GetSplitterPaneRect( newPaneID,    &newPaneRect);
			}
			
			// get the new splitter visible
			::SetWindowPos( newSplitterWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER);
			
			// Set new window position for the new pane, the target pane and the new splitter window
			newPane->SetWindowPos( HWND_TOP, newPaneRect.left, newPaneRect.top, newPaneRect.Width(), newPaneRect.Height(), SWP_SHOWWINDOW);
			targetPane->SetWindowPos( HWND_TOP, targetPaneRect.left, targetPaneRect.top, targetPaneRect.Width(), targetPaneRect.Height(), SWP_SHOWWINDOW);
			
			// Set focus to the new pane
			newPane->SetFocus();
			this->currentPane = newPane;
			
			// if there is no tab left in the source pane it needs to be destroyed
			HWND sourcePaneParentWnd = sourcePane->GetParent();
			if ( sourcePane->isEmpty() && sourcePaneParentWnd != this->m_hWnd) {
				
				// important:
				// destroy the source pane via the message queue becouse this call maybe done by a tab control
				// belonging to the source pane. we don't want to destroy it before it ends.
				this->PostMessage( WM_USER_DESTROY_PANE, 0, (LPARAM) sourcePane->m_hWnd);
			}

            return newSplitterWnd;
		} // splitWithCurrentTab( Pane* sourcePane, Pane* targetPane, Pane::TabPaneHitTest where)
		

		HSplitter* lookUpHSplitter( HWND splitterWnd) {
			
			CAtlMap< HWND, HSplitter*>::CPair* splitterPair = this->horSplitterMap.Lookup( splitterWnd);
			if ( NULL == splitterPair)
				return NULL;
			
			return splitterPair->m_value;
		} // HSplitter* lookUpHSplitter( HWND splitterWnd)
		
		VSplitter* lookUpVSplitter( HWND splitterWnd) {
			
			CAtlMap< HWND, VSplitter*>::CPair* splitterPair = this->verSplitterMap.Lookup( splitterWnd);
			if ( NULL == splitterPair)
				return NULL;
			
			return splitterPair->m_value;
		} // HSplitter* lookUpHSplitter( HWND splitterWnd)
		
		
		Pane* lookUpPane( HWND clientViewWnd) {
			
			CAtlMap< HWND, Pane*>::CPair* clientPanePair = this->clientViewPaneMap.Lookup( clientViewWnd);
			if ( NULL == clientPanePair)
				return NULL;
			
			return clientPanePair->m_value;
		} // Pane* lookUpPane( HWND clientViewWnd)
		
		Pane* lookUpPaneWnd( HWND paneWnd) {
			
			CAtlMap< HWND, Pane*>::CPair* clientPanePair = this->paneMap.Lookup( paneWnd);
			if ( NULL == clientPanePair)
				return NULL;
			
			return clientPanePair->m_value;
		} // Pane* lookUpPaneWnd( HWND paneWnd)
		
		Pane* getNextPane( HWND hWnd) {
			
			Pane* result = this->lookUpPaneWnd( hWnd);
			
			// check if hwnd is a Pane
			if ( NULL != result)
				return result;
			
			HWND splitter[2];
			memset( splitter, 0, sizeof(splitter));
			
			HWND childWnd = ::GetWindow( hWnd, GW_CHILD);
			int i = 0;
			while ( NULL != childWnd) {
				
				if ( NULL != ( result = this->lookUpPaneWnd( childWnd)))
					break;
				
				// TODO need to keep a beakpoint for debugging more time
				if ( NULL != this->lookUpHSplitter( childWnd) || NULL != this->lookUpHSplitter( childWnd))
					// dangerous code, but we know that there are no more than two splitters ;)
					splitter[i++] = childWnd;
				
				childWnd = ::GetWindow( hWnd, GW_HWNDNEXT);
			}
			
			if ( NULL == result) {
				
				if ( NULL != splitter[0])
					result = getNextPane( ::GetWindow( splitter[0], GW_CHILD));
				else if ( NULL != splitter[1])
					result = getNextPane( ::GetWindow( splitter[1], GW_CHILD));
			}
			
			return result;
		}
		
		bool setCurrentPane( HWND paneWnd) {
			
			Pane* pane = this->lookUpPaneWnd( paneWnd);
			if ( NULL != pane) {
				
				this->currentPane = pane;
				return true;
			} else
				return false;
		}
		
		void removeAll() {
			
			if ( NULL != this->dragContext)
				delete dragContext;
			
			POSITION position;
			if ( position = this->paneMap.GetStartPosition())
				do
					delete this->paneMap.GetNextValue( position);
				while ( position);
			this->paneMap.RemoveAll();
			
			if ( position = this->verSplitterMap.GetStartPosition())
				do
					delete this->verSplitterMap.GetNextValue( position);
				while ( position);
			this->verSplitterMap.RemoveAll();
			
			if ( position = this->horSplitterMap.GetStartPosition())
				do
					delete this->horSplitterMap.GetNextValue( position);
				while ( position);
			this->horSplitterMap.RemoveAll();
		}
		
		void updateLayout() {
			
			if ( NULL == this->rootWnd)
				return;
			
			CRect rect;
			this->GetClientRect( rect);
			::SetWindowPos( this->rootWnd, HWND_TOP, rect.left, rect.top, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
			return;
		}
		
	//----------------- public interface
	public:
		
		// Constructor/Destructor
		SplitPane( CallBackListener* cbListener = NULL, bool tabOnTop = false)
			: currentPane( NULL)
			, rootWnd( NULL)
			, dragContext( NULL)
			, cbListener( cbListener)
			, tabOnTop( tabOnTop)
		{}
		
		~SplitPane() {
			
			this->removeAll();
			
			if ( NULL != this->m_hWnd)
				this->DestroyWindow();
		}

		
		// CallBackListener interface
		void clientActivate(  HWND childWnd, HWND clientViewWnd) {
			
			this->setCurrentPane( childWnd);
			if ( NULL != this->cbListener)
				this->cbListener->clientActivate( this->m_hWnd, clientViewWnd);
			return;
		}
		
		void clientDblClick(  HWND childWnd, HWND clientViewWnd) {
			
			if ( NULL != this->cbListener)
				this->cbListener->clientDblClick( this->m_hWnd, clientViewWnd);
			return;
		}
		
		void clientCloseClick(  HWND childWnd, HWND clientViewWnd) {
			
			if ( NULL != this->cbListener)
				this->cbListener->clientCloseClick( this->m_hWnd, clientViewWnd);
			return;
		}
		
		void dragStart( HWND srcPane, HWND clientViewWnd, long x, long y, DWORD keysPressed) {
			
			ATLASSERT( NULL == this->dragContext);
			this->dragContext = new DragContext();
		}
		
		void dragOver( HWND srcPane, HWND clientViewWnd, long x, long y, DWORD keysPressed) {
			
			ATLASSERT( NULL != this->dragContext);
			
			CRect drawRect;
			bool dragIn = false;
			Pane* targetPane = this->getPane( x, y);
			if ( NULL != targetPane) {
				
				CPoint hitPoint( x, y);
				this->dragContext->tabPaneHitTest = targetPane->hitTest( &hitPoint);
				if ( this->dragContext->tabPaneHitTest != Pane::TabPaneHitTest_Unknown) {
					
					// determine a source pane
					Pane* sourcePane = this->lookUpPaneWnd( srcPane);
					if ( sourcePane != targetPane || sourcePane->clientCount() > 1) {
							
						// draw hit area only if there is more than one tab in the source tab
						// or if the target pane is not the same as the source tab.
						targetPane->getTabPageRect( drawRect);
						
						switch ( this->dragContext->tabPaneHitTest) {
						case Pane::TabPaneHitTest_Top:    drawRect.bottom /= 2;               break;
						case Pane::TabPaneHitTest_Right:  drawRect.left = drawRect.right / 2; break;
						case Pane::TabPaneHitTest_Left:   drawRect.right /= 2;                break;
						case Pane::TabPaneHitTest_Bottom: drawRect.top = drawRect.bottom / 2; break;
						}
						
						dragIn = 0 == ( MK_CONTROL & keysPressed);
					}
				}
			}
			
			if ( dragIn) {
				
				// Draw a track rectangle
				if ( this->dragContext->draggingOut) {
					this->dragContext->draggingOut = false;
					this->cbListener->dragCancel( this->m_hWnd, clientViewWnd);
				}
				this->dragContext->drawTrackRectangle( targetPane->m_hWnd, drawRect);
				
			} else {
				
				if ( !this->dragContext->draggingOut) {
					
					this->dragContext->clearTrackRectangle();
					this->dragContext->draggingOut = true;
					if ( NULL != this->cbListener)
						this->cbListener->dragStart( this->m_hWnd, clientViewWnd, x, y, keysPressed);
				}
				if ( NULL != this->cbListener)
					this->cbListener->dragOver( this->m_hWnd, clientViewWnd, x, y, keysPressed);
			}
		}
		
		void dragDrop( HWND srcPane, HWND clientViewWnd, long x, long y, DWORD keysPressed) {
			
			ATLASSERT( NULL != this->dragContext);
			
			if ( this->dragContext->draggingOut) {
				
				if ( NULL != this->cbListener)
					this->cbListener->dragDrop( this->m_hWnd, clientViewWnd, x, y, keysPressed);
			} else {
				
				Pane* sourcePane;
				Pane* targetPane;
				this->dragContext->clearTrackRectangle();
				
				switch( this->dragContext->tabPaneHitTest) {
				case Pane::TabPaneHitTest_Unknown:
					
					break;
					
				case Pane::TabPaneHitTest_TabArea:
					
					targetPane = this->getPane( x, y);
					sourcePane = this->paneMap.Lookup( srcPane)->m_value;
					if ( NULL != targetPane && NULL != sourcePane && targetPane != sourcePane)
						this->moveCurrentTab( sourcePane, targetPane);
					break;
					
				default:
					
					targetPane = this->getPane( x, y);
					sourcePane = this->paneMap.Lookup( srcPane)->m_value;
					if ( NULL != targetPane && NULL != sourcePane)
						this->splitWithCurrentTab( sourcePane, targetPane, this->dragContext->tabPaneHitTest);
				}
			}
			// Remove drag context
			delete this->dragContext;
			this->dragContext = NULL;
		}
		
		void dragCancel( HWND srcPane, HWND clientViewWnd) {
			
			ATLASSERT( NULL != this->dragContext);
			
			if ( this->dragContext->draggingOut && NULL != this->cbListener)
				this->cbListener->dragCancel( this->m_hWnd, clientViewWnd);
			
			delete this->dragContext;
			this->dragContext = NULL;
		}
		
		// public methods
		
		// creates a new Split Pane window with parentWnd and rect parameters
		HWND create( HWND hWndParent, ATL::_U_RECT rect = NULL) {
			
			if ( NULL == this->GetWndClassInfo().m_lpszOrigName)
				this->GetWndClassInfo().m_lpszOrigName = GetWndClassName();
			
			ATOM atom = this->GetWndClassInfo().Register( &m_pfnSuperWindowProc);
			
			int dwStyle = this->GetWndStyle( 0);
			int dwExStyle = this->GetWndExStyle(0);
			
			return CWindowImplBaseT< TBase, TWinTraits >::Create( hWndParent
			                                                    , rect, NULL
			                                                    , dwStyle
			                                                    , dwExStyle
																, ATL::_U_MENUorID( (UINT) 0)
			                                                    , atom
			                                                    , 0
			                                                    );
		}
		
		// tab controls on top ot on bottom of panes
		bool tabsOnTop() const {
			
			return this->tabOnTop;
		}
		
		enum TargetArea {
			  targetLeft
			, targetTop
			, targetRight
			, targetBottom
		};
		
		// splits source client view window (sourceWnd) with the target client view
		HWND splitClientView( HWND sourceWnd, HWND targetWnd, TargetArea targetArea) {
			
			ATLASSERT( ::IsWindow( sourceWnd));
			ATLASSERT( ::IsWindow( targetWnd));
			
			Pane* sourcePane = this->lookUpPane( sourceWnd);
			Pane* targetPane = this->lookUpPane( targetWnd);
			
			ATLASSERT( NULL != sourcePane);
			ATLASSERT( NULL != targetPane);
			
			Pane::TabPaneHitTest tabPaneHitTest;
			switch ( targetArea) {
			case targetLeft:   tabPaneHitTest = Pane::TabPaneHitTest_Left;   break;
			case targetTop:    tabPaneHitTest = Pane::TabPaneHitTest_Top;    break;
			case targetRight:  tabPaneHitTest = Pane::TabPaneHitTest_Right;  break;
			case targetBottom: tabPaneHitTest = Pane::TabPaneHitTest_Bottom; break;
			default:
				ATLASSERT( FALSE);
			}
			
			this->setFocusTo( sourceWnd);
			return this->splitWithCurrentTab( sourcePane, targetPane, tabPaneHitTest);
		}
		
		// move the client view window (sourceWnd) to the same tab pane where the specified client view window is located
		void moveClientView( HWND sourceWnd, HWND targetWnd) {
			
			ATLASSERT( ::IsWindow( sourceWnd));
			ATLASSERT( ::IsWindow( targetWnd));
			
			Pane* sourcePane = this->lookUpPane( sourceWnd);
			Pane* targetPane = this->lookUpPane( targetWnd);
			
			ATLASSERT( NULL != sourcePane);
			ATLASSERT( NULL != targetPane);
			
			this->setFocusTo( sourceWnd);

            // anders set moveCurrentTab instead of spli here - was this a bug?
            this->moveCurrentTab(sourcePane, targetPane);
//			this->splitWithCurrentTab( sourcePane, targetPane, Pane::TabPaneHitTest_TabArea);
		}
		
		// move all client view windows to the specified split pane.
		void moveClientViewsTo( SplitPane* targetPane) {
			
			POSITION position = this->clientViewPaneMap.GetStartPosition();
			if ( position) {
				do {
					Pane* pane;
					HWND clientViewWnd;
					this->clientViewPaneMap.GetNextAssoc( position, clientViewWnd, pane);
					
					ClientProperties clientProperties;
					if ( pane->get( clientViewWnd, clientProperties)) {
						
						pane->removeClientView( clientViewWnd);
						targetPane->append( clientProperties.caption, clientViewWnd, clientProperties.toolTip, clientProperties.imageIndex);
					}
				} while ( position);
				
				this->removeAll();
				this->currentPane = NULL;
				this->rootWnd     = NULL;
			}
			return;
		}
		
		// returns the client view window that receives the keyboard focus
		HWND focusedClientView() const {
			return this->currentPane->focusedClientView();
		}
		
		// sets the pane's tab bar to highlight when it has focus
		void setActive(bool active) {
			POSITION position = this->paneMap.GetStartPosition();
			while (position != NULL) {
				Pane* pane = this->paneMap.GetNextValue(position);
				if (pane == currentPane) {
					pane->setActive(active);
				} else {
					pane->setActive(false);
				}
			}
		}
		
		// sets the keyboard focus to a tab pane at the specified position
		bool setFocusTo( long x, long y) {
			
			CPoint hitPoint( x, y);
			POSITION position = this->paneMap.GetStartPosition();
			if ( position)
				do {
					Pane* pane = this->paneMap.GetNextValue( position);
					CRect rect;
					pane->GetWindowRect( rect);
					if ( rect.PtInRect( hitPoint)) {
						
						pane->SetFocus();
						return true;
					}
				} while (position);
			
			return false;
		}
		
		// sets the keyboard focus to the specified client view window
		bool setFocusTo( HWND clientViewWnd) {
			
			Pane* pane = this->lookUpPane( clientViewWnd);
			if ( NULL != pane && pane->setFocusTo( clientViewWnd)) {
				
				this->currentPane = pane;
				return true;
			}
			
			return false;
		}
		
		// adds the new client view window to Split Pane.
		// The new client view is added into the focused tab pane
		bool append( const TCHAR* caption, HWND hWnd, const TCHAR* toolTip = NULL, int imageIndex = -1) {
			
			ATLASSERT( NULL != hWnd);
			
			this->currentPane->append( caption, hWnd, toolTip, imageIndex);
			this->clientViewPaneMap[hWnd] = this->currentPane;
			this->currentPane->updateLayout();
			
			return true;
		}
		
		// detaches the client view window from Split Pane. 
		// This method changes a parent window of the client window view to the Split Pane parent one.
		bool detachClientView( HWND clientViewWnd) {
			
			ATLASSERT( true);
			Pane* pane = this->lookUpPane( clientViewWnd);
			if ( NULL == pane)
				return false;
			
			if ( !pane->removeClientView( clientViewWnd))
				return false;
			
			this->clientViewPaneMap.RemoveKey( clientViewWnd);
			
			::ShowWindow( clientViewWnd, FALSE);
			if ( pane->isEmpty() && this->rootWnd != pane->m_hWnd) {
				
				if ( pane->IsChild( clientViewWnd))
					::SetParent( clientViewWnd, this->m_hWnd);
				this->SendMessage( WM_USER_DESTROY_PANE, 0, (LPARAM) pane->m_hWnd);
			}
			return false;
		}
		
		// returns the number of client view windows in Split Pane
		int getClientViewCount() {
			return (int)this->clientViewPaneMap.GetCount();
		}
		
		// sets Image List
		void setImageList( HIMAGELIST imgList) {
			
			this->imageList = imgList;
			POSITION position = this->paneMap.GetStartPosition();
			if ( NULL != position)
				do
					this->paneMap.GetNextValue( position)->setImageList( imgList);
				while ( NULL != position);
		}
		
		// gets Image List
		HIMAGELIST getImageList() const {
			return this->imageList;
		}
		
		// returns the rectangle of tab pane, if any, is at a specified position
		bool getClientViewRect( LPPOINT point, CRect& rect) {
			
			ATLASSERT( point);
			CPoint hitPoint( *point);
			POSITION position = this->paneMap.GetStartPosition();
			if ( position)
				do {
					Pane* pane = this->paneMap.GetNextValue( position);
					if ( Pane::TabPaneHitTest_TabArea == pane->hitTest( &hitPoint)) {
						
						pane->getTabPageRect( rect);
						pane->ClientToScreen( rect);
						return true;
					}
				} while (position);
			
			return false;
		}
		
		// Overridables
		static LPCTSTR GetWndCaption() {
			return NULL;
		}
		
		// Event handlers
		DECLARE_WND_CLASS( "DockSplitTab::SplitPane")
		
		BEGIN_MSG_MAP( SplitPane)
			
			MESSAGE_HANDLER( WM_CREATE,        OnCreate)
			MESSAGE_HANDLER( WM_SIZE,          OnSize)
			MESSAGE_HANDLER( WM_ERASEBKGND,    OnEraseBackground)
			MESSAGE_HANDLER( WM_CLOSE,         OnClose)
			MESSAGE_HANDLER( WM_MOUSEACTIVATE, OnMouseActivate)
		
			MESSAGE_HANDLER( WM_USER_DESTROY_PANE, OnDestroyPane);
			FORWARD_NOTIFICATIONS()
		END_MSG_MAP()
		
		LRESULT OnMouseActivate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
			
			LRESULT result = this->DefWindowProc( uMsg, wParam, lParam);
			if ( MA_ACTIVATE == result || MA_ACTIVATEANDEAT == result)
				this->SetFocus();
			return result;
		}
		
		LRESULT OnClose( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
			
			POSITION position = this->clientViewPaneMap.GetStartPosition();
			if ( position)
				do {
					
					HWND clientViewWnd = this->clientViewPaneMap.GetNextKey( position);
					
					ATLASSERT( ::IsWindow( clientViewWnd)); // check if it's still window
					
					if ( 0 != ::SendMessage( clientViewWnd, WM_CLOSE, 0, 0))
						return 1;
				} while ( position);
			return 0;
		}
		
		LRESULT OnDestroyPane( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
			
			HWND srcPane = (HWND) lParam;

			// determine a source pane
			CAtlMap< HWND, Pane*>::CPair* sourcePanePair = this->paneMap.Lookup( srcPane);
			if ( NULL == sourcePanePair)
				return 1;
				
			Pane* sourcePane;
			sourcePane = sourcePanePair->m_value;
			if ( NULL == sourcePane)
				return 1;
			
			HWND sourcePaneParentWnd = sourcePane->GetParent();
			CAtlMap< HWND, HSplitter*>::CPair* horSplitPair;
			CAtlMap< HWND, VSplitter*>::CPair* verSplitPair;
			
			HWND neighbourWnd;
			CRect neighbourRect;
			if ( NULL != ( verSplitPair = this->verSplitterMap.Lookup( sourcePaneParentWnd))) {
				
				// it is a vertical splitter
				VSplitter* parentSplitter = verSplitPair->m_value;
				
				neighbourWnd = parentSplitter->GetSplitterPane( SPLIT_PANE_LEFT);
				if ( neighbourWnd == sourcePane->m_hWnd)
					neighbourWnd = parentSplitter->GetSplitterPane( SPLIT_PANE_RIGHT);
				
				HWND theParentWnd = ::GetParent( parentSplitter->m_hWnd);
				::SetParent( neighbourWnd, theParentWnd);
				if ( this->rootWnd == parentSplitter->m_hWnd)
					this->rootWnd = neighbourWnd;
				
				// find the parent of the parent splitter and replace the pane to the neighbour
				if ( theParentWnd == this->m_hWnd) {
					
					this->GetClientRect( &neighbourRect);
				
				} else if ( NULL != ( verSplitPair = this->verSplitterMap.Lookup( theParentWnd))) {
					
					VSplitter* theParent = verSplitPair->m_value;
					if ( theParent->GetSplitterPane( SPLIT_PANE_LEFT) == parentSplitter->m_hWnd) {
						
						theParent->SetSplitterPane( SPLIT_PANE_LEFT, neighbourWnd);
						theParent->GetSplitterPaneRect( SPLIT_PANE_LEFT, &neighbourRect);
					} else if ( theParent->GetSplitterPane( SPLIT_PANE_RIGHT) == parentSplitter->m_hWnd) {
						
						theParent->SetSplitterPane( SPLIT_PANE_RIGHT, neighbourWnd);
						theParent->GetSplitterPaneRect( SPLIT_PANE_RIGHT, &neighbourRect);
					}
				} else if ( NULL != ( horSplitPair = this->horSplitterMap.Lookup( theParentWnd))) {
					
					HSplitter* theParent = horSplitPair->m_value;
					if ( theParent->GetSplitterPane( SPLIT_PANE_LEFT) == parentSplitter->m_hWnd) {
						
						theParent->SetSplitterPane( SPLIT_PANE_LEFT, neighbourWnd);
						theParent->GetSplitterPaneRect( SPLIT_PANE_LEFT, &neighbourRect);
					} else if ( theParent->GetSplitterPane( SPLIT_PANE_RIGHT) == parentSplitter->m_hWnd) {
						
						theParent->SetSplitterPane( SPLIT_PANE_RIGHT, neighbourWnd);
						theParent->GetSplitterPaneRect( SPLIT_PANE_RIGHT, &neighbourRect);
					}
				}
				
				// delete the parent splitter
				this->verSplitterMap.RemoveKey( parentSplitter->m_hWnd);
				sourcePane->DestroyWindow();
				parentSplitter->DestroyWindow();
				delete parentSplitter;
				
			} else if ( NULL != ( horSplitPair = this->horSplitterMap.Lookup( sourcePaneParentWnd))) {
				
				// it is a horizontal splitter
				HSplitter* parentSplitter = horSplitPair->m_value;
				
				neighbourWnd = parentSplitter->GetSplitterPane( SPLIT_PANE_LEFT);
				if ( neighbourWnd == sourcePane->m_hWnd)
					neighbourWnd = parentSplitter->GetSplitterPane( SPLIT_PANE_RIGHT);
				
				HWND theParentWnd = ::GetParent( parentSplitter->m_hWnd);
				::SetParent( neighbourWnd, theParentWnd);
				if ( this->rootWnd == parentSplitter->m_hWnd)
					this->rootWnd = neighbourWnd;
				
				// find the parent of the parent splitter and replace the pane to the neighbour
				if ( theParentWnd == this->m_hWnd) {
					
					this->GetClientRect( &neighbourRect);
					
				} else if ( NULL != ( verSplitPair = this->verSplitterMap.Lookup( theParentWnd))) {
					
					VSplitter* theParent = verSplitPair->m_value;
					if ( theParent->GetSplitterPane( SPLIT_PANE_LEFT) == parentSplitter->m_hWnd) {
						
						theParent->SetSplitterPane( SPLIT_PANE_LEFT, neighbourWnd, false);
						theParent->GetSplitterPaneRect( SPLIT_PANE_LEFT, &neighbourRect);
						
					} else if ( theParent->GetSplitterPane( SPLIT_PANE_RIGHT) == parentSplitter->m_hWnd) {
						
						theParent->SetSplitterPane( SPLIT_PANE_RIGHT, neighbourWnd, false);
						theParent->GetSplitterPaneRect( SPLIT_PANE_RIGHT, &neighbourRect);
					}
				} else if ( NULL != ( horSplitPair = this->horSplitterMap.Lookup( theParentWnd))) {
					
					HSplitter* theParent = horSplitPair->m_value;
					if ( theParent->GetSplitterPane( SPLIT_PANE_LEFT) == parentSplitter->m_hWnd) {
						
						theParent->SetSplitterPane( SPLIT_PANE_LEFT, neighbourWnd, false);
						theParent->GetSplitterPaneRect( SPLIT_PANE_LEFT, &neighbourRect);
					} else if ( theParent->GetSplitterPane( SPLIT_PANE_RIGHT) == parentSplitter->m_hWnd) {
						
						theParent->SetSplitterPane( SPLIT_PANE_RIGHT, neighbourWnd, false);
						theParent->GetSplitterPaneRect( SPLIT_PANE_RIGHT, &neighbourRect);
					}
				}
				
				// delete the parent splitter
				this->horSplitterMap.RemoveKey( parentSplitter->m_hWnd);
				sourcePane->DestroyWindow();
				parentSplitter->DestroyWindow();
				delete parentSplitter;
				
			}
			
			
			this->paneMap.RemoveKey( srcPane);
			delete sourcePane; // 
			::SetWindowPos( neighbourWnd, HWND_TOP, neighbourRect.left, neighbourRect.top, neighbourRect.Width(), neighbourRect.Height(), SWP_SHOWWINDOW);
			
			if ( this->currentPane == sourcePane)
				this->currentPane = this->getNextPane( neighbourWnd);
			
			return 1;
		}
		
		LRESULT OnEraseBackground( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
			
			return 1;
		}
		
		LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/) {
			
			if ( NULL == this->rootWnd)
				return 0;
			
			WORD width  = LOWORD( lParam);
			WORD height = HIWORD( lParam);
			::SetWindowPos( this->rootWnd, NULL, 0, 0, width, height, SWP_NOZORDER);
			return 0;
		}
		
		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
			
			this->currentPane = new Pane( this, this->tabOnTop);
			this->currentPane->Create( this->m_hWnd);
			this->paneMap.SetAt( this->currentPane->m_hWnd, this->currentPane);
			this->rootWnd = this->currentPane->m_hWnd;
			return 0;
		}
		
	}; // class SplitPane: public WinContainer<SplitPane>{};
	
}; // namespace DockSplitTab

#endif // __DOCKSPLITPANE_H__
