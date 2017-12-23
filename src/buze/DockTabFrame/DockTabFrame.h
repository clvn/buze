#pragma once

#ifndef __DOCKTAB_FRAME_H__
#define __DOCKTAB_FRAME_H__

/*********************************************************************
DockSplitTab::Frame class implementation

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
**********************************************************************/

#include "Winuser.h"
#include "DockTabSplitPane.h"

namespace DockSplitTab {
	
	// where client windows can be placed
	enum FramePlace {
		  placeUNKNOWN
		, placeMAINPANE
		, placeDOCKPANE
		, placeFLOATFRAME
	};
	
	// dock side enum
	enum DockSide {
		  dockUNKNOWN   = -1
		, dockBOTTOM    =  0
		, dockTOP       =  1
		, dockLEFT      =  2
		, dockRIGHT     =  3
	};
	
	// communication interface between float frames, dock panes and their owners ( class Frame)
	class FloatDockFrameListener: public CallBackListener {
	public:
		
		virtual CRect getDockSliderRect( int dockSide) = 0;
		virtual void dropDockSlider( int dockSide, int offset) = 0;
		
		virtual void floatDockPane( int dockSide) = 0;
		
		virtual void dockFloatFrame( HWND floatWnd) = 0;
		
		virtual void closeFloatFrame( HWND floatWnd) = 0;
		
	}; // __interface FloatDockFrameListener
	
	// All valuable attributes for a client view
	class ClientView {
	public:
		
		enum State {
			  stateNull    = 0 // ClientView can be in Main Pane only
			, stateDockable = 1 // ClientView can be either in a dock pane or in a float frame
			, stateFloating = 2 // ClientView can be in a float frame only
		};
		
		ClientView()
			: wnd( NULL)
			, object( NULL)
			, owner(NULL)
			, imageIndex( -1)
			, floatRect( rectNULL)
			, dockSide( dockUNKNOWN)
			, lastPlace( placeUNKNOWN)
			, state( stateNull)
			, allowFloat(true)
			, viewId(0)
		{}
		
		ClientView( const TCHAR* caption, HWND hWnd, const TCHAR* toolTip = NULL, int imageIndex = -1)
			: wnd( hWnd)
			, imageIndex( imageIndex)
			, caption( caption)
			, toolTip( toolTip)
			, dockSide( dockUNKNOWN)
			, owner(NULL)
			, lastPlace( placeUNKNOWN)
			, state( stateNull)
			, allowFloat(true)
			, viewId(0)
		{}
		
		State getState() const {
			
			return this->state;
		}
		
		HWND wnd;
		void* object; // Added by Megz
		int viewId; // Added by clvn
		_CSTRING_NS::CString caption;
		_CSTRING_NS::CString toolTip;
		int          imageIndex;
		CRect        floatRect;
		DockSide     dockSide;
		FramePlace   lastPlace;

		bool allowFloat;
		SplitPane* getOwner() { return owner; }
	private:
		State      state;
		SplitPane* owner;
		
		friend class Frame;
	}; // class ClientView
	
	// Communication interface between DockSplitTab::Frame and its owner
	class FrameListener {
	public:
		virtual void clientActivated( ClientView* clientView) = 0;
		virtual void clientDetached( ClientView* clientView) = 0;
		virtual void clientChangedPlace( ClientView* clientView, DockSplitTab::FramePlace place, DockSplitTab::DockSide side) = 0;
	};
	
	// 
	class Frame
		: public CWindowImplBaseT< CWindow, CControlWinTraits>
		, public FloatDockFrameListener
	{
		typedef CWindow TBase;
		typedef CControlWinTraits TWinTraits;
		typedef Frame ParentClass;
	
        friend class DockTabSerializer;
	private:
	
	// internal message events for Frame View
	static const int eventDropParcel      = WM_USER+1;
	static const int eventCloseFloatFrame = WM_USER+2;
	static const int eventCloseDockPane   = WM_USER+3;
	static const int eventCloseClientView = WM_USER+4;
	
	//----------------- protected classes
	protected:
		
		// TODO: refactor the code to eliminate this class
		class DragContext
			: public RectTracker {
		
		protected:
			CRect  startRect;
			CPoint startPoint;
			CPoint trackPoint;
			
		public:
			
			// constructor/destructor
			DragContext( HWND trackWindow)
				: RectTracker( trackWindow)
				, startRect( rectNULL)
				, startPoint( pointNULL)
				, trackPoint( pointNULL)
			{}
			
			DragContext( LPPOINT point, LPRECT rect)
				: RectTracker()
				, startRect( rect)
				, startPoint( *point)
				, trackPoint( pointNULL)
			{}
			
			// public interface for DragContext class
			void keepTrack( POINT point) {
				
				CSize size( CPoint( point) - this->trackPoint);
				if ( this->trackPoint == pointNULL) {
					
					this->trackPoint = this->startPoint;
					this->trackRect  = this->startRect;
				}
				this->trackPoint += size;
				this->trackRect  += size;
				return;
			}
			
			LPPOINT getTrackPoint() {
				
				return &trackPoint;
			}
			
			void reset( POINT point, LPRECT rect) {
				
				this->startPoint = point;
				this->startRect  = rect;
				this->trackPoint = pointNULL;
				this->trackRect  = rectNULL;
				return;
			}
			
			void setStartRect( LPRECT startRect) {
				this->startRect = startRect;
			}
			
			void setStartPoint( POINT startPoint) {
				this->startPoint = startPoint;
			}
			
			CRect getStartRect() const {
				
				return this->startRect;
			}
			
			CPoint getStartPoint() const {
				
				return this->startPoint;
			}
			
			// Overridables
			void drawTrackRectangle( HWND hWnd, LPRECT rect, bool clear = true) {
				
				RectTracker::drawTrackRectangle( hWnd, rect, clear);
				return;
			}
			
			void drawTrackRectangle( POINT point, bool clear = true) {
				
				// check if it's not the same track pane as we've got in the previous call
				if ( this->trackPoint == CPoint( point))
					return;
					
				// draw a new track rectangle
				CRect rect;
				if ( this->trackPoint == pointNULL) {
					
					this->trackPoint = this->startPoint;
					rect = this->startRect;
				} else
					rect = this->trackRect;
				CSize size( CPoint( point) - this->trackPoint);
				this->trackPoint += size;
				rect += size;
				RectTracker::drawTrackRectangle( NULL, rect, clear);
				return;
			}
			
			void clearTrackRectangle( bool clear = true) {
				
				RectTracker::clearTrackRectangle( clear);
				if ( clear)
					this->trackPoint = pointNULL;
				return;
			}
			
		}; // class DragContext
		
		// encapsulates common data and methods for split pane containers
		// used by FloatFrame class and DockPane class.
		class SplitPaneWrapper {
		protected:
			
			SplitPane* splitPane;
		public:
			
			// Constructor/destructor
			SplitPaneWrapper()
				: splitPane( NULL)
			{}
			
			~SplitPaneWrapper() {
				
				if ( splitPane) {
					
					if ( NULL != this->splitPane->m_hWnd)
						this->splitPane->DestroyWindow();
					delete this->splitPane;
				}
			}
			
			SplitPaneWrapper( SplitPane* pane)
				: splitPane( pane)
			{}
			
			SplitPane* getPane( ) const {
				
				return this->splitPane;
			}
			
			void attachPane( SplitPane* pane) {
				
				ATLASSERT( NULL == this->splitPane);
				ATLASSERT( NULL != pane);
				ATLASSERT( pane->IsWindow());
				
				this->splitPane = pane;
			}
			
			SplitPane* detachPane( ) {
				
				SplitPane* pane = this->splitPane;
				this->splitPane    = NULL;
				return pane;
			}
			
			int getClientViewCount() {
				
				return this->splitPane->getClientViewCount();
			}
			
			bool detachClientView( HWND clientViewWnd) {
				
				return this->splitPane->detachClientView( clientViewWnd);
			}
			
			HWND detachFocusedClientView() {
				
				HWND result = this->splitPane->focusedClientView();
				this->splitPane->detachClientView( result);
				return result;
			}
			
			bool append( const TCHAR* caption, HWND hWnd, const TCHAR* toolTip = NULL, int imageIndex = -1) {
				
				ATLASSERT( NULL != hWnd && ::IsWindow( hWnd));
				ATLASSERT( NULL != this->splitPane);
				
				return this->splitPane->append( caption, hWnd, toolTip, imageIndex);
			}
			
			bool setFocusTo( HWND clientView) {
				
				return NULL != this->splitPane && this->splitPane->setFocusTo( clientView);
			}
			
			bool confirmClosing() {
				
				return ( NULL != this->splitPane && 0 == this->splitPane->SendMessage( WM_CLOSE));
			}
		}; // class SplitPaneWrapper
		
		// contains floating client views
		class FloatFrame
			: public CFrameWindowImpl<FloatFrame>
			, public SplitPaneWrapper
		{
		// protected members
		protected:
			FloatDockFrameListener* cbListener;
			
		public:
			
			// Constructor/destructor
			FloatFrame( FloatDockFrameListener* cpListener = NULL)
				: SplitPaneWrapper()
				, cbListener( cpListener)
			{}
			
			~FloatFrame() {
				
				if ( NULL != this->m_hWnd)
					this->DestroyWindow();
			}
			
			static DWORD getFloatFrameStyle() {
				return WS_POPUPWINDOW
					| WS_VISIBLE
					| WS_CLIPSIBLINGS
					| WS_CLIPCHILDREN
					| WS_DLGFRAME
					| WS_THICKFRAME
					| WS_OVERLAPPED;
			}

			static DWORD getFloatFrameStyleEx() {
				return WS_EX_LEFT
					| WS_EX_LTRREADING
					| WS_EX_RIGHTSCROLLBAR
					| WS_EX_NOPARENTNOTIFY
					| WS_EX_TOOLWINDOW
					| WS_EX_WINDOWEDGE;
			}

			// public interface
			void create( HWND parent, LPRECT rect = NULL) {
				
				this->Create( parent
							, rect
#ifdef DEBUG
							, "Float Frame Name"
#else
							, NULL
#endif
							, getFloatFrameStyle()
							, getFloatFrameStyleEx()
							);

				return;
			}
			
			HWND focusedClientView() const {
				
				return this->splitPane->focusedClientView();
			}
			
			SplitPane* detachPane( ) {
				
				SplitPane* pane = SplitPaneWrapper::detachPane();
				this->m_hWndClient = NULL;
				return pane;
			}
			
			void attachPane( SplitPane* pane) {
				
				SplitPaneWrapper::attachPane( pane);
				
				this->m_hWndClient = pane->m_hWnd;
				this->splitPane->SetParent( this->m_hWnd);
				//this->UpdateLayout( FALSE);
				//this->splitPane->ShowWindow( SW_SHOW);
			}
			
			void updateLayout() {
				
				if ( !this->splitPane)
					return;
				CRect rect;
				this->GetClientRect( rect);
				this->splitPane->SetWindowPos( HWND_TOP, rect, SWP_SHOWWINDOW);
				return;
			}
			
			// Event handlers
			DECLARE_FRAME_WND_CLASS( "DockSplitTab::FloatFrame", 0)
			
			BEGIN_MSG_MAP( FloatFrame)
				
				MESSAGE_HANDLER( WM_CLOSE,  OnClose)
				MESSAGE_HANDLER( WM_SIZE,   OnSize)
				MESSAGE_HANDLER( WM_ERASEBKGND,       OnEraseBackground)
				
				MESSAGE_HANDLER( WM_CONTEXTMENU,     OnContextMenu)
				MESSAGE_HANDLER( WM_NCLBUTTONDOWN,   OnNCLButtonDown)
				//commented out by anders
				//MESSAGE_HANDLER( WM_NCLBUTTONDBLCLK, OnNCLButtonDoubleClick)
				MESSAGE_HANDLER( WM_DESTROY,         OnDestroy)
				FORWARD_NOTIFICATIONS()

			END_MSG_MAP();
			
			LRESULT OnNCLButtonDoubleClick( UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
				
				HWND parent = ::GetParent( this->m_hWnd);
				switch ( wParam) {
				case HTCAPTION:
					if ( NULL != this->cbListener)
						this->cbListener->dockFloatFrame( this->m_hWnd);
					return 1;
				}
				return 0;
			}
			
			LRESULT OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
				
				this->m_hWndClient = NULL;
				this->m_hWnd       = NULL;
				return 1;
			}
			
			LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				if ( SplitPaneWrapper::confirmClosing())
					this->cbListener->closeFloatFrame( this->m_hWnd);
				return 0;
			}
			
			LRESULT OnSize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				if ( this->m_hWndClient == 0)
					return this->DefWindowProc( uMsg, wParam, lParam);
				
				WORD width  = LOWORD( lParam);
				WORD height = HIWORD( lParam);
				::SetWindowPos( this->m_hWndClient, NULL, 0, 0, width, height, SWP_NOZORDER);
				return 1;
			}
		
			LRESULT OnEraseBackground( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				return 1;
			}

			LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				HWND clentViewWnd = this->splitPane->focusedClientView();
				::SendMessage( clentViewWnd, WM_CONTEXTMENU, wParam, lParam);
				return 0;
			}
			
			LRESULT OnNCLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
				
				UINT hitTest = (UINT)this->DefWindowProc( WM_NCHITTEST, 0, lParam);
				if ( hitTest != HTCAPTION || NULL == this->cbListener)
					return this->DefWindowProc( uMsg, wParam, lParam);
				
				if ( ::GetActiveWindow() != this->m_hWnd)
					this->SetActiveWindow();
				
				HWND clientViewWnd = this->splitPane->focusedClientView();
				if ( NULL != clientViewWnd)
					this->splitPane->setFocusTo( clientViewWnd);
				
				CPoint hitPoint = CPoint( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				this->ClientToScreen( &hitPoint);
				this->cbListener->trackDragAndDrop( this->m_hWnd, hitPoint);
				return 1;
			}
		}; // class FloatFrame
		
		// contains docked client views
		class DockPane
			: public CWindowImplBaseT< CWindow, CControlWinTraits>
			, public SplitPaneWrapper
		{
		protected:
			
			FloatDockFrameListener* cbListener;
			
			DockSide dockSide;
			
			CString captionText;
			CRect   captionRect;
			CRect   sliderRect;
			CRect   closeButtonRect;
			CRect   splitPaneRect;
			
			bool closeButtonPressed;
			bool mouseOverCloseButton;
			bool activeState;
			bool trackMouseLeave;
			
			static int instanceCount;
			static CBrush systemFrameBrush;
			static CBrush activeCaptionBrush;
			static CBrush inactiveCaptionBrush;
			static CBrush activeBorderBrush;
			static CBrush inactiveBorderBrush;
			
			static COLORREF activeCaptionTextColor;
			static COLORREF inactiveCaptionTextColor;
			
			static int   systemCxFrame;
			static int   systemCyFrame;
			static int   systemCaptionHeight;
			static CFont systemCaptionFont;
			
			static HCURSOR verCursor;
			static HCURSOR horCursor;
			
		protected:
			
			void activate() {
				if ( !this->activeState)
					this->setActive( true);
			}
			
			void deleteStaticObjects() {
				
				ATLASSERT( NULL != this->systemFrameBrush.m_hBrush);
				ATLASSERT( NULL != this->activeCaptionBrush.m_hBrush);
				ATLASSERT( NULL != this->inactiveCaptionBrush.m_hBrush);
				ATLASSERT( NULL != this->activeBorderBrush.m_hBrush);
				ATLASSERT( NULL != this->inactiveBorderBrush.m_hBrush);
				
				DockPane::systemFrameBrush.DeleteObject();
				DockPane::activeCaptionBrush.DeleteObject();
				DockPane::inactiveCaptionBrush.DeleteObject();
				DockPane::activeBorderBrush.DeleteObject();
				DockPane::inactiveBorderBrush.DeleteObject();
				
				DockPane::systemCaptionFont.DeleteObject();
				return;
			}
			
			void createStaticObjects() {
				
				DockPane::systemCaptionHeight = ::GetSystemMetrics( SM_CYCAPTION);
				DockPane::systemCxFrame       = ::GetSystemMetrics( SM_CXFRAME);
				DockPane::systemCyFrame       = ::GetSystemMetrics( SM_CYFRAME);
				
				DockPane::activeCaptionTextColor   = ::GetSysColor( COLOR_CAPTIONTEXT);
				DockPane::inactiveCaptionTextColor = ::GetSysColor( COLOR_INACTIVECAPTIONTEXT);
				
				DockPane::systemFrameBrush.CreateSolidBrush( ::GetSysColor( COLOR_WINDOWFRAME));
				DockPane::activeCaptionBrush.CreateSolidBrush( ::GetSysColor( COLOR_ACTIVECAPTION));
				DockPane::inactiveCaptionBrush.CreateSolidBrush( ::GetSysColor( COLOR_INACTIVECAPTION));
				DockPane::activeBorderBrush.CreateSolidBrush( ::GetSysColor( COLOR_ACTIVEBORDER));
				DockPane::inactiveBorderBrush.CreateSolidBrush( ::GetSysColor( COLOR_INACTIVEBORDER));
				
				LOGFONT logFont = {0};
				::SystemParametersInfo( SPI_GETICONTITLELOGFONT, sizeof(logFont), &logFont, 0);
				DockPane::systemCaptionFont.CreateFontIndirect( &logFont);
				return;
			}
			
			void trackSlider( CPoint hitPoint) {
				
				CWindow parentWnd( this->GetParent());
				
				CRect sliderRect( rectNULL);
				if ( NULL != this->cbListener) {
					
					sliderRect = this->cbListener->getDockSliderRect( this->dockSide);
					parentWnd.ScreenToClient( sliderRect);
				}
				
				CRect dockPaneClientRect;
				this->GetClientRect( dockPaneClientRect);
				
				CRect   parentClientRect;
				parentWnd.GetClientRect( parentClientRect);
				
				CRect trackRect = this->sliderRect;
				
				this->ClientToScreen( trackRect);
				
				parentWnd.ScreenToClient( trackRect);
				
				CPoint      prevPoint = hitPoint;
				parentWnd.ScreenToClient( &prevPoint);
				this->ClientToScreen( &prevPoint);
				
				RectTracker tracker;
				bool dragging = true;
				
				LPCTSTR sizeCursorId;
				if ( dockBOTTOM == this->dockSide || dockTOP == this->dockSide) {
					
					trackRect.top   += 2;
					trackRect.bottom = trackRect.top + 3;
					sizeCursorId     = IDC_SIZENS;
				} else {
					
					trackRect.left += 2;
					trackRect.right = trackRect.left + 3;
					sizeCursorId    = IDC_SIZEWE;
				}
				
				if ( !this->activeState && dockPaneClientRect.PtInRect( hitPoint)) {
					
					this->SetFocus();
					this->activeState = true;
				}
				
				::SetCapture( this->m_hWnd);
				MSG msg;
				while( ( ::GetCapture()==this->m_hWnd) && ( ::GetMessage( &msg, NULL, 0, 0))) {
					
					switch( msg.message) {
					case WM_MOUSEMOVE:
						
						hitPoint = CPoint( GET_X_LPARAM( msg.lParam), GET_Y_LPARAM( msg.lParam));
						if ( dragging) {
							
							this->ClientToScreen( &hitPoint);
							parentWnd.ScreenToClient( &hitPoint);
							
							CRect rect = trackRect;
							if ( dockBOTTOM == this->dockSide || dockTOP == this->dockSide) {
								
								int offset = hitPoint.y - prevPoint.y;
								rect.top    += offset;
								rect.bottom += offset;
							} else {
								
								int offset = hitPoint.x - prevPoint.x;
								rect.left  += offset;
								rect.right += offset;
							}
							CRect intersect;
							intersect.IntersectRect( parentClientRect, rect);
							if ( !intersect.IsRectEmpty()) {
								
								intersect.IntersectRect( sliderRect, rect);
								if ( intersect == rect) {
									
									trackRect = rect;
									tracker.drawTrackRectangle( parentWnd.m_hWnd, trackRect);
									prevPoint = hitPoint;
								}
							}
							
						} else
							if ( !( 0 == msg.wParam && this->sliderRect.PtInRect( hitPoint)))
								::ReleaseCapture();
						break;
						
					case WM_LBUTTONUP:
						
						if ( dragging && CRect( tracker.getTrackRect()).TopLeft() != pointNULL) {
							
							// drop slider to the new position
							CRect rect = this->sliderRect;
							this->ClientToScreen( rect);
							parentWnd.ScreenToClient( rect);
							int offset;
							switch ( this->dockSide) {
							case dockTOP:
								offset = tracker.getTrackRect()->top - rect.top - 2;
								break;
							case dockBOTTOM:
								offset = rect.top - tracker.getTrackRect()->top + 2;
								break;
							case dockLEFT:
								offset = tracker.getTrackRect()->left - rect.left - 2;
								break;
							case dockRIGHT:
								offset = rect.left - tracker.getTrackRect()->left + 2;
								break;
							default:
								offset = 0;
							}
							if ( ( 2 < offset || offset < -2) && NULL != this->cbListener)
								this->cbListener->dropDockSlider( this->dockSide, offset);
						}
						
					case WM_MBUTTONDOWN:
					case WM_RBUTTONDOWN:
				#if (_WIN32_WINNT >= 0x0500)
					case WM_XBUTTONDOWN:
				#endif
						if ( !this->activeState && dockPaneClientRect.PtInRect( hitPoint))
							this->activate();
						
					case WM_KEYDOWN:
					case WM_SYSKEYDOWN:
						::ReleaseCapture();
					default:
						::TranslateMessage(&msg);
						::DispatchMessage( &msg);
					}
				}
				
				// tidy up
				tracker.clearTrackRectangle();
			} // void trackSlider
			
		public:
			
			DockPane( FloatDockFrameListener* cpListener = NULL)
				: SplitPaneWrapper()
				, cbListener( cpListener)
				, dockSide( dockUNKNOWN)
				, closeButtonPressed( false)
				, mouseOverCloseButton( false)
				, activeState( false)
				, trackMouseLeave( false)
			{
				if( DockPane::verCursor == NULL) {
					
					::EnterCriticalSection( &_Module.m_csStaticDataInit);
					if ( DockPane::verCursor == NULL)
						DockPane::verCursor = ::LoadCursor( NULL, IDC_SIZEWE);
					::LeaveCriticalSection(&_Module.m_csStaticDataInit);
				}
				
				if( DockPane::horCursor == NULL) {
					
					::EnterCriticalSection( &_Module.m_csStaticDataInit);
					if ( DockPane::horCursor == NULL)
						DockPane::horCursor = ::LoadCursor( NULL, IDC_SIZENS);
					::LeaveCriticalSection(&_Module.m_csStaticDataInit);
				}
			}
			
			~DockPane() {
				
				if ( NULL != this->m_hWnd)
					this->DestroyWindow();
			}
			
			void create( HWND parentWnd, DockSide dockSide) {
				
				ATLASSERT( ::IsWindow( parentWnd));
				ATLASSERT( dockUNKNOWN != dockSide);
				
				this->dockSide = dockSide;
				
				ATOM atom = this->GetWndClassInfo().Register( &m_pfnSuperWindowProc);
				this->Create( parentWnd, rectNULL, NULL
				            , WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS //| WS_CAPTION | WS_OVERLAPPED
				            , 0
				            , (UINT) 0
				            , atom
				            , NULL
				            );
			}
			
			void setCaption(_CSTRING_NS::CString const& caption) {
				
				this->captionText = caption;
				this->InvalidateRect( this->captionRect);
				return;
			}
			
			HWND focusedClientView() const {
				
				return this->splitPane->focusedClientView();
			}
			
			_CSTRING_NS::CString getCaption() const {
				
				return this->captionText;
			}
			
			DockSide getDockSide() const {
				return this->dockSide;
			}
			
			bool isActive() const {
				return this->activeState;
			}
			
			void setActive( bool active) {
				
				this->activeState = active;
				this->InvalidateRect( this->captionRect);
				return;
			}
			
			// Event handlers
			DECLARE_WND_CLASS( "DockSplitTab::DockPane")
			
			BEGIN_MSG_MAP( DockPane)
				MESSAGE_HANDLER( WM_CREATE, OnCreate)
				MESSAGE_HANDLER( WM_CLOSE,  OnClose)
				MESSAGE_HANDLER( WM_SIZE,   OnSize)
				
				MESSAGE_HANDLER( WM_ERASEBKGND,     OnEraseBackground)
				MESSAGE_HANDLER( WM_CONTEXTMENU,    OnContextMenu)
				MESSAGE_HANDLER( WM_DESTROY,        OnDestroy)
				MESSAGE_HANDLER( WM_SETFOCUS,       OnSetFocus)
				MESSAGE_HANDLER( WM_KILLFOCUS,      OnKillFocus)
				MESSAGE_HANDLER( WM_SYSCOLORCHANGE, OnSysColorChange);
				MESSAGE_HANDLER( WM_PAINT,          OnPaint);
				MESSAGE_HANDLER( WM_MOUSEACTIVATE,  OnMouseActivate);
				MESSAGE_HANDLER( WM_MOUSEMOVE,      OnMouseMove);
				MESSAGE_HANDLER( WM_MOUSELEAVE,     OnMouseLeave);
				//commented out by anders
				//MESSAGE_HANDLER( WM_LBUTTONDBLCLK,  OnLButtonDoubleClick);
				MESSAGE_HANDLER( WM_LBUTTONDOWN,    OnLButtonDown);
				MESSAGE_HANDLER( WM_LBUTTONUP,      OnLButtonUp);
				MESSAGE_HANDLER( WM_CAPTURECHANGED, OnCaptureChanged);
				MESSAGE_HANDLER( WM_SETCURSOR,      OnSetCursor);
				FORWARD_NOTIFICATIONS()
			END_MSG_MAP();
			
			LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				LRESULT result = this->DefWindowProc( uMsg, wParam, lParam);
				if ( MA_ACTIVATE == result || MA_ACTIVATEANDEAT == result) {
					
					this->activate();
					//this->SetFocus(); <--- this would be enough if it was similar to a FloatFrame or SplitPane....
					//this->splitPane->SetFocus();
					
					HWND clientViewWnd = this->splitPane->focusedClientView();
					if ( NULL != clientViewWnd)
						this->splitPane->setFocusTo( clientViewWnd);
					
				}
				return result;
			}
			
			LRESULT OnCaptureChanged( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				if ( this->closeButtonPressed) {
					
					this->closeButtonPressed = false;
					this->InvalidateRect( this->closeButtonRect);
				}
				return 1;
			}
			
			LRESULT OnLButtonDoubleClick( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				CPoint hitPoint( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				
				if (    NULL != this->cbListener
				     && !this->closeButtonRect.PtInRect( hitPoint)
				     &&  this->captionRect.PtInRect( hitPoint)
				   )
				   this->cbListener->floatDockPane( this->dockSide);
				return 1;
			}
			
			LRESULT OnLButtonUp( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				CPoint hitPoint( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				
				if ( ::GetCapture() == this->m_hWnd) {
					
					if ( this->closeButtonRect.PtInRect( hitPoint)) {
						
						if ( this->closeButtonPressed) {
							
							this->closeButtonPressed = false;
							this->InvalidateRect( this->closeButtonRect);
						}
						
						this->SendMessage( WM_CLOSE);
					}
					::ReleaseCapture();
				}
				return 1;
			}
			
			LRESULT OnLButtonDown( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				CPoint hitPoint( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				
				HWND clientViewWnd = this->splitPane->focusedClientView();
				//if ( NULL != clientViewWnd)
				//	this->splitPane->setFocusTo( clientViewWnd);
				
				if ( this->closeButtonRect.PtInRect( hitPoint)) {
					
					// close dock pane button pressed
					this->SetCapture();
					if ( !this->closeButtonPressed) {
						
						this->closeButtonPressed = true;
						this->InvalidateRect( this->closeButtonRect);
					}
				} else if ( this->captionRect.PtInRect( hitPoint)) {
					
					// let's start dragging dock
					// TODO: here are forced non-draggable docks, maybe we want draggable some time?
					/*
					ATLASSERT( NULL != this->cbListener);
					this->ClientToScreen( &hitPoint);
					this->cbListener->trackDragAndDrop( this->m_hWnd, hitPoint, true);
					*/
					
				} else if ( this->sliderRect.PtInRect( hitPoint)) {
					
					// let's start dragging the slider
					this->trackSlider( hitPoint);					
				}
				return 1;
			}
			
			LRESULT OnMouseLeave( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				if ( this->mouseOverCloseButton || this->closeButtonPressed) {
					
					this->mouseOverCloseButton = this->closeButtonPressed = false;
					this->InvalidateRect( this->closeButtonRect);
				}
				this->trackMouseLeave = false;
				return 1;
			}
			
			LRESULT OnSetCursor( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
				
				if( (HWND)wParam == this->m_hWnd && LOWORD(lParam) == HTCLIENT) {
					
					DWORD hitPosition = ::GetMessagePos();
					CPoint hitPoint = CPoint( GET_X_LPARAM( hitPosition), GET_Y_LPARAM( hitPosition));
					this->ScreenToClient( &hitPoint);
					if( this->sliderRect.PtInRect( hitPoint))
						return 1;
				}
				
				bHandled = FALSE;
				return 0;
			}
			
			LRESULT OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				CPoint hitPoint( GET_X_LPARAM( lParam), GET_Y_LPARAM( lParam));
				
				if ( 0 == wParam && this->sliderRect.PtInRect( hitPoint)) {
					
					// the mouse is on slider rectangle and no keys are pressed
					::SetCursor( dockTOP == this->dockSide || dockBOTTOM == this->dockSide
					           ? DockPane::horCursor
					           : DockPane::verCursor
					           );
					
				} else if ( !this->mouseOverCloseButton && this->closeButtonRect.PtInRect( hitPoint)) {
					
					// the mouse is over close button. let's animate it.
					this->mouseOverCloseButton = true;
					this->InvalidateRect( this->closeButtonRect);
					if ( !this->trackMouseLeave) {
						
						TRACKMOUSEEVENT trackMouseEvent;
						memset( &trackMouseEvent, 0, sizeof( trackMouseEvent));
						trackMouseEvent.cbSize    = sizeof(TRACKMOUSEEVENT);
						trackMouseEvent.dwFlags   = TME_LEAVE;
						trackMouseEvent.hwndTrack = this->m_hWnd;
						if( _TrackMouseEvent( &trackMouseEvent))
							this->trackMouseLeave = true;
					}
					
				}
				
				if ( this->mouseOverCloseButton && !this->closeButtonRect.PtInRect( hitPoint)) {
					
					// finish close button animation
					this->mouseOverCloseButton = false;
					this->InvalidateRect( this->closeButtonRect);
				}
				return 1;
			}
			
			LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				LRESULT result = 1;
				
				result = this->DefWindowProc( uMsg, wParam, lParam);
				this->activate();
				
				if ( NULL == this->splitPane)
					return result;
				
				HWND clientViewWnd = this->splitPane->focusedClientView();
				::SetFocus( clientViewWnd? clientViewWnd: this->splitPane->m_hWnd);
				return result;
			}
			
			LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				this->activeState = false;
				this->InvalidateRect( this->captionRect, FALSE);
				return 1;
			}
			
			LRESULT OnSysColorChange( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
				
				DockPane::deleteStaticObjects();
				DockPane::createStaticObjects();
				return 1;
			}
			
			LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
				
#ifdef DEBUG
				switch ( this->dockSide) {
				case dockBOTTOM:
					this->captionText = _T("Bottom dock pane.");
					break;
				case dockTOP:
					this->captionText = _T("Top dock pane.");
					break;
				case dockLEFT:
					this->captionText = _T("Left dock pane.");
					break;
				case dockRIGHT:
					this->captionText = _T("Right dock pane.");
					break;
				}
#endif			
				if ( 0 == DockPane::instanceCount)
					this->createStaticObjects();
				
				DockPane::instanceCount++;
				return 0;
			}
			
			LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				if ( SplitPaneWrapper::confirmClosing())
					::PostMessage( this->GetParent(), eventCloseDockPane, static_cast<WPARAM>( this->dockSide), 0);
				return 0;
			}
			
			
			LRESULT OnDestroy( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
				
				if ( NULL != this->splitPane) {
					
					this->splitPane->DestroyWindow();
					this->splitPane->m_hWnd = NULL;
					delete this->splitPane;
					this->splitPane = NULL;
				}
				
				DockPane::instanceCount--;
				if ( 0 == DockPane::instanceCount)
					this->deleteStaticObjects();
				
				this->m_hWnd = NULL;
				return 1;
			}
			
			LRESULT OnEraseBackground( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				return 0;
			}
			
			LRESULT OnSize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				WORD cx = LOWORD( lParam);
				WORD cy = HIWORD( lParam);
				WORD dx = 0;
				WORD dy = 0;
				
				switch( this->dockSide) {
				case dockTOP:
					
					this->captionRect   = CRect( 0,  0, cx, systemCaptionHeight);          dy  = systemCaptionHeight;
					this->splitPaneRect = CRect( 0, dy, cx, cy - this->systemCyFrame * 2); dy += this->splitPaneRect.Height();
					this->sliderRect    = CRect( 0, dy, cx, cy);
					break;
					
				case dockBOTTOM:
					
					this->sliderRect    = CRect( 0,  0, cx, this->systemCyFrame * 2);  dy = this->systemCyFrame * 2;
					this->captionRect   = CRect( 0, dy, cx, dy + systemCaptionHeight); dy += this->systemCaptionHeight;
					this->splitPaneRect = CRect( 0, dy, cx, cy);
					break;
					
				case dockLEFT:
					
					dx = this->systemCxFrame * 2;
					dy = systemCaptionHeight;
					this->captionRect   = CRect(       0,  0, cx - dx, dy);
					this->splitPaneRect = CRect(       0, dy, cx - dx, cy);
					this->sliderRect    = CRect( cx - dx,  0,      cx, cy);
					break;
					
				case dockRIGHT:
					
					dx = this->systemCxFrame * 2;
					dy = systemCaptionHeight;
					this->sliderRect    = CRect(  0,  0, dx, cy);
					this->captionRect   = CRect( dx,  0, cx, dy);
					this->splitPaneRect = CRect( dx, dy, cx, cy);
					break;
				default:
					break;
				}
				
				// calculate close button rectangle
				this->closeButtonRect = CRect( this->captionRect.right  - 2 - systemCaptionHeight
				                             , this->captionRect.top    + 2
				                             , this->captionRect.right  - 2 - 4
											 , this->captionRect.bottom - 2
				                             );
				
				// set up splitPane position
				if ( NULL != this->splitPane)
					this->splitPane->SetWindowPos( NULL, this->splitPaneRect, SWP_NOZORDER | SWP_SHOWWINDOW);
				return 1;
			}
			
			LRESULT OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				CRect updateRect;
				this->GetUpdateRect( updateRect, FALSE);
				if ( updateRect.IsRectEmpty())
					return 1;
				
				COLORREF captionTextColor;
				CBrush* captionBrush;
				CBrush* borderBrush;
				
				if ( this->activeState) {
					
					captionTextColor = this->activeCaptionTextColor;
					captionBrush     = &this->activeCaptionBrush;
					borderBrush      = &this->activeBorderBrush;
				} else {
					
					captionTextColor = this->inactiveCaptionTextColor;
					captionBrush     = &this->inactiveCaptionBrush;
					borderBrush      = &this->inactiveBorderBrush;
				}
				
				// create device context
				CPaintDC paintDC( this->m_hWnd);
				
				// draw caption control
				CRect rect;
				rect.IntersectRect( this->captionRect, updateRect);
				if ( !rect.IsRectEmpty()) {
					
					rect = this->captionRect;
					
					CBrushHandle oldBrush = paintDC.SelectBrush( *borderBrush);
					
					// draw caption frame
					paintDC.FrameRect( rect, this->systemFrameBrush.m_hBrush);
					
					// draw caption background
					paintDC.SelectBrush( *captionBrush);
					rect.InflateRect( -1, -1);
					paintDC.PatBlt( rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
					
					// draw caption text
					paintDC.SetBkMode( TRANSPARENT);
					
					_CSTRING_NS::CString str = this->getCaption();
					CFontHandle oldFont = paintDC.SelectFont( DockPane::systemCaptionFont);
					
					COLORREF oldTextColor = paintDC.SetTextColor( captionTextColor);
					
					rect.top   +=  2;
					rect.left  +=  5;
					rect.right -= 50;
					LPSTR strbuffer = str.GetBuffer(0);
					paintDC.DrawText( strbuffer, str.GetLength()
									, rect
									, DT_INTERNAL | DT_LEFT | DT_MODIFYSTRING | DT_END_ELLIPSIS | DT_NOPREFIX
									);
					str.ReleaseBuffer();
					
					// draw close button
					CPen buttonPen;
					buttonPen.CreatePen( PS_SOLID, 1, captionTextColor);
					CPenHandle oldPen = paintDC.SelectPen( buttonPen);
					
					rect = this->closeButtonRect;
					const int sp = 4;
					//paintDC.MoveTo( rect.left  + sp - 1, rect.top    + sp);
					//paintDC.LineTo( rect.right - sp - 1, rect.bottom - sp);
					paintDC.MoveTo( rect.left  + sp,     rect.top    + sp);
					paintDC.LineTo( rect.right - sp,     rect.bottom - sp);
					
					//paintDC.MoveTo( rect.left  + sp - 1, rect.bottom - sp - 1);
					//paintDC.LineTo( rect.right - sp - 1, rect.top    + sp - 1 );
					paintDC.MoveTo( rect.left  + sp,     rect.bottom - sp - 1);
					paintDC.LineTo( rect.right - sp,     rect.top    + sp - 1);
					
					paintDC.SelectPen( oldPen);
					//buttonPen.DeleteObject();
					
					CBrush buttonBrush;
					buttonBrush.CreateSolidBrush( captionTextColor);
					paintDC.SelectBrush( *borderBrush);
					
					if ( this->closeButtonPressed)
						
						paintDC.DrawEdge( this->closeButtonRect, BDR_SUNKENOUTER, BF_RECT);
					
					else if ( this->mouseOverCloseButton)
						
						paintDC.DrawEdge( this->closeButtonRect, BDR_RAISEDINNER, BF_RECT);
					
					//buttonBrush.DeleteObject();
					
					// tidy up
					paintDC.SelectFont( oldFont);
					paintDC.SelectBrush( oldBrush);
					paintDC.SetTextColor( oldTextColor);
					//paintDC.SelectFont( oldFont);
				}
				
				// draw slider
				rect.IntersectRect( this->sliderRect, updateRect);
				if ( !rect.IsRectEmpty()) {
					
					paintDC.FillRect( this->sliderRect, COLOR_3DFACE);
					//if ( ( this->GetExStyle() & WS_EX_CLIENTEDGE) != 0)
					paintDC.DrawEdge( this->sliderRect
					                , EDGE_RAISED
					                , ( dockLEFT == this->dockSide || dockRIGHT == this->dockSide)
					                ? (BF_LEFT | BF_RIGHT)
					                : (BF_TOP | BF_BOTTOM)
					                );
				}
				return 1;
			}
			
			LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
				
				HWND clentViewWnd = this->splitPane->focusedClientView();
				::SendMessage( clentViewWnd, WM_CONTEXTMENU, wParam, lParam);
				return 0;
			}
			
		}; // class DockPane
		
		union ClientViewStateSet {
			
			ClientViewStateSet()
				: value(0)
			{}
			
			struct {
				unsigned null:     1;
				unsigned dockable: 1;
				unsigned floating: 1;
			};
			unsigned value;
			
			bool isEmpty() {
				return 0 == this->value;
			}
			bool isNull() {
				return this->null && !( this->dockable || this->floating);
			}
			bool isDockable() {
				return this->dockable && !( this->floating || this->null);
			}
			bool isFloating() {
				return this->floating && !( this->dockable || this->null);
			}
		}; // union ClientViewStateSet
		
		// stores drag and drop context
		class FloatFrameDragContext
			: public DragContext {
		private:
			
			CRect ghostRect;
			CRect ghostDrawRect;
			HWND  ghostWnd;
		public:
			
			ClientViewStateSet stateSet;
			
			FramePlace sourcePlace;
			FramePlace targetPlace;
			DockSide   sourceDock;
			DockSide   targetDock;
			
			HWND sourcePaneWnd;
			HWND targetPaneWnd;
			HWND clientViewWnd;
			
			int  dockWidth;
			int  dockSize;
			
			FloatFrameDragContext( HWND trackWnd)
				: DragContext( trackWnd)
				, ghostRect( rectNULL)
				, ghostDrawRect( rectNULL)
				, ghostWnd( NULL)
				, sourcePlace( placeUNKNOWN)
				, targetPlace( placeUNKNOWN)
				, sourceDock( dockUNKNOWN)
				, targetDock( dockUNKNOWN)
				, sourcePaneWnd( NULL)
				, targetPaneWnd( NULL)
				, clientViewWnd( NULL)
				, dockWidth( 0)
				, dockSize( 0)
			{}
			
			~FloatFrameDragContext() {
				
				if ( ghostRect.EqualRect( &rectNULL)) {
					DragContext::clearTrackRectangle( true);
				} else
					DragContext::drawTrackRectangle( this->ghostWnd, this->ghostDrawRect, false);
			}
			
			CRect getGhostRect() const {
				
				return this->ghostRect;
			}
			
			void clearTrackRectangle( bool clear = true) {
				
				if ( ghostRect.EqualRect( &rectNULL))
					DragContext::clearTrackRectangle( clear);
			}
			
			void drawTrackRectangle( POINT point, bool clear = true) {
				
				if ( !this->ghostRect.EqualRect( &rectNULL)) {
					
					CRect rect = this->trackRect;
					DragContext::drawTrackRectangle( this->ghostWnd, this->ghostDrawRect, false);
					this->trackRect = rect;
					this->ghostRect = rectNULL;
					this->ghostWnd  = NULL;
					DragContext::drawTrackRectangle( point, false);
				} else
					DragContext::drawTrackRectangle( point, clear);
				
				ATLASSERT( this->ghostRect != this->trackRect);
				return;
			}
			
			void drawTrackRectangle( LPRECT ghostRect, HWND ghostWnd) {
				
				ATLASSERT( this->ghostRect != this->trackRect);
				if ( this->ghostRect != ghostRect) {
					
					CRect rect = this->trackRect; // save this->trackRect from changing in drawTrackRectangle
					
					// clear the previous ghost rectangle
					if ( !(    this->ghostRect.EqualRect( &rectNULL)
					        || ( this->ghostRect.EqualRect( ghostRect) && this->ghostWnd == ghostWnd)
					      )
					   )
						RectTracker::drawTrackRectangle( this->ghostWnd, this->ghostDrawRect, false);
					
					// draw a new one
					this->ghostWnd  = ghostWnd;
					this->ghostDrawRect = this->ghostRect = ghostRect;
					if ( NULL != this->ghostWnd)
						CWindow( this->ghostWnd).ScreenToClient( this->ghostDrawRect);
					RectTracker::drawTrackRectangle( this->ghostWnd, this->ghostDrawRect, false);
					this->trackRect = rect;
				}
				
				return;
			}
			
		};
		
		// the result of drag and drop operations
		class DropParcel {
		public:
			
			DropParcel()
				: sourcePlace( placeUNKNOWN)
				, targetPlace( placeUNKNOWN)
				, sourceDock( dockUNKNOWN)
				, targetDock( dockUNKNOWN)
				, clientViewWnd( NULL)
				, dockWidth( 0)
				, dockSize( 0)
				, sourcePaneWnd( NULL)
				, targetPaneWnd( NULL)
				, targetRect(0, 0, 0, 0)
			{}
			
			FramePlace sourcePlace;
			FramePlace targetPlace;
			DockSide         sourceDock;
			DockSide         targetDock;
			CRect            targetRect;
			int              dockWidth;
			int              dockSize;
			
			HWND sourcePaneWnd;
			HWND targetPaneWnd;
			HWND clientViewWnd;
		};
		
		// layout implementation for Frame class
		class LayoutManager {
		private:
			
			// struct DockLayout stores information about data
			struct DockLayout {
				
				DockLayout()
					: size( 0)
					, visible( false)
					, place(0)
				{}
				
				int       size;
				bool      visible;
				
				union {
					unsigned place;
					struct {
						unsigned left:  1; // 1 means it covers left corner
						                   // when you look inside of the main pane rectangle
						                   // and there is a visible dock pane on the left
						                   // 0 means it doesn't do that
						unsigned right: 1; // 1 means it covers right corner
						                   // when you look inside of the main pane rectangle
						                   // and there is a visible dock pane on the right
						                   // 0 means it doesn't do that
					};
				};
			}; // 
			
			static const int mainPaneMinCX = 24;
			static const int mainPaneMinCY = 24;
			
			CSize      mainPaneSize;
			DockLayout dockLayouts[4];
			
		public:
			
			// aditional parameters for setDock and calcDockRect methods,
			// which defines the dock pane covering of right and left corners 
			// in case of neighbour dock pane existing
			static const int coveredLeft  = 1; // dock pane covers left corner too if there is the left neighbour dock pane
			static const int coveredRight = 2; // dock pane covers right corner too if there is the left neighbour dock pane
			static const int coveredBoth  = 3; // dock pane covers includes all bitmasks ( dockLeft | dockRight)
			
			LayoutManager()
				: mainPaneSize( mainPaneMinCX, mainPaneMinCY)
			{
                // anders added default dock widths
                dockLayouts[dockLEFT].size = dockLayouts[dockRIGHT].size = dockLayouts[dockBOTTOM].size = dockLayouts[dockTOP].size = 200;
            }
			
			void getMainPaneRect( const LPRECT rect) const {
				
				ATLASSERT( NULL != rect);
				
				memset( rect, 0, sizeof(*rect));
				
				if ( this->dockLayouts[ dockLEFT].visible)
					rect->left = this->dockLayouts[ dockLEFT].size;
				
				if ( this->dockLayouts[ dockTOP].visible)
					rect->top = this->dockLayouts[ dockTOP].size;
				
				rect->right  = rect->left + this->mainPaneSize.cx;
				rect->bottom = rect->top  + this->mainPaneSize.cy;
				return;
			}
			
			int getDockWidth( DockSide dockSide) const {
				
				return this->dockLayouts[ dockSide].place;
			}
			
			int getDockSize( DockSide dockSide) const {
				
				return this->dockLayouts[ dockSide].size;
			}
			
			int getDockPlace( DockSide dockSide) const {
				
				int result = 0;
				
				const DockLayout& dockLayout = this->dockLayouts[ dockSide];
				
				if ( dockLayout.right) result |= coveredRight;
				if ( dockLayout.left)  result |= coveredLeft;
				
				return result;
			}
			
			bool getDockRect( DockSide dockSide, const LPRECT rect) const {
				
				ATLASSERT( NULL != rect);
				
				const DockLayout& dockLayout = this->dockLayouts[ dockSide];
				if ( !dockLayout.visible)
					return false;
				
				switch ( dockSide) {
				case dockTOP:
					
					rect->top    = 0;
					rect->bottom = dockLayout.size;
					rect->left   = this->dockLayouts[ dockLEFT].visible && !dockLayout.right
					             ? this->dockLayouts[ dockLEFT].size
					             : 0
					             ;
					rect->right  = this->mainPaneSize.cx
					             + ( this->dockLayouts[ dockLEFT].visible
					               ? this->dockLayouts[ dockLEFT].size
					               : 0
					               )
					             + ( this->dockLayouts[ dockRIGHT].visible && dockLayout.left
					               ? this->dockLayouts[ dockRIGHT].size
					               : 0
								   )
					             ;
					return true;
					
				case dockBOTTOM:
					
					rect->top    = this->mainPaneSize.cy
					             + ( this->dockLayouts[ dockTOP].visible? this->dockLayouts[ dockTOP].size: 0)
					             ;
					rect->bottom = rect->top + dockLayout.size;
					rect->left   = this->dockLayouts[ dockLEFT].visible && !dockLayout.left
					             ? this->dockLayouts[ dockLEFT].size
					             : 0
					             ;
					rect->right  = this->mainPaneSize.cx
					             + ( this->dockLayouts[ dockLEFT].visible
					               ? this->dockLayouts[ dockLEFT].size
					               : 0
					               )
					             + ( this->dockLayouts[ dockRIGHT].visible && dockLayout.right
					               ? this->dockLayouts[ dockRIGHT].size
					               : 0
								   )
					             ;
					return true;
					
				case dockLEFT:
					
					rect->top    = this->dockLayouts[ dockTOP].visible && !dockLayout.left
					             ? this->dockLayouts[ dockTOP].size
					             : 0
					             ;
					rect->bottom = this->mainPaneSize.cy
					             + ( this->dockLayouts[ dockTOP].visible
					               ? this->dockLayouts[ dockTOP].size
					               : 0
								   )
					             + ( this->dockLayouts[ dockBOTTOM].visible && dockLayout.right
					               ? this->dockLayouts[ dockBOTTOM].size
					               : 0
								   )
					             ;
					rect->left   = 0;
					rect->right  = dockLayout.size;
					return true;
					
				case dockRIGHT:
					
					rect->top    = this->dockLayouts[ dockTOP].visible && !dockLayout.right
					             ? this->dockLayouts[ dockTOP].size
					             : 0
					             ;
					rect->bottom = this->mainPaneSize.cy
					             + ( this->dockLayouts[ dockTOP].visible
					               ? this->dockLayouts[ dockTOP].size
					               : 0
								   )
					             + ( this->dockLayouts[ dockBOTTOM].visible && dockLayout.left
					               ? this->dockLayouts[ dockBOTTOM].size
					               : 0
								   )
					             ;
					rect->left   = this->mainPaneSize.cx
					             + ( this->dockLayouts[ dockLEFT].visible? this->dockLayouts[ dockLEFT].size: 0)
					             ;
					rect->right  = rect->left + dockLayout.size;
					return true;
					
				}
				
				return false;
			}
			
			bool calcDockSize( DockSide dockSide, int coveredCorners, int& size) const {
				
				CRect rect;
				bool result = this->calcDockRect( dockSide, coveredCorners, 0, rect);
				if ( result) {
					switch ( dockSide) {
					case dockTOP:
					case dockBOTTOM:
						size = rect.Height();
						break;
					case dockLEFT:
					case dockRIGHT:
						size = rect.Width();
						break;
					default:
						return false;
					}
				}
				return result;
			}
			
			bool calcDockRect( DockSide dockSide, int coveredCorners, int size, const LPRECT rect) const {
				
				ATLASSERT( NULL != rect);
				
				const DockLayout& dockLayout = this->dockLayouts[ dockSide];
				if ( dockLayout.visible)
					return false;
				
				int fullSize;
				
				switch ( dockSide) {
				case dockTOP:
					
					fullSize     = this->mainPaneSize.cy;
					if ( this->dockLayouts[ dockBOTTOM].visible)
						fullSize += this->dockLayouts[ dockBOTTOM].size;
					fullSize     /= 4;
					if ( size > fullSize || size == 0)
						size = fullSize;
					
					rect->top    = 0;
					rect->bottom = size;
					rect->left   = this->dockLayouts[ dockLEFT].visible && !( coveredRight & coveredCorners)
					             ? this->dockLayouts[ dockLEFT].size
					             : 0
					             ;
					rect->right  = ( this->dockLayouts[ dockLEFT].visible
					               ? this->dockLayouts[ dockLEFT].size
					               : 0
					               )
					             + this->mainPaneSize.cx
					             + ( this->dockLayouts[ dockRIGHT].visible && ( coveredLeft & coveredCorners)
					               ? this->dockLayouts[ dockRIGHT].size
					               : 0
								   )
					             ;
					return true;
					
				case dockBOTTOM:
					
					fullSize     = this->mainPaneSize.cy;
					if ( this->dockLayouts[ dockTOP].visible)
						fullSize += this->dockLayouts[ dockTOP].size;
					if ( size > fullSize / 4 || size == 0)
						size = fullSize / 4;
					
					rect->top    = fullSize - size;
					rect->bottom = fullSize;
					rect->left   = this->dockLayouts[ dockLEFT].visible && !( coveredLeft & coveredCorners)
					             ? this->dockLayouts[ dockLEFT].size
					             : 0
					             ;
					rect->right  = ( this->dockLayouts[ dockLEFT].visible
					               ? this->dockLayouts[ dockLEFT].size
					               : 0
								   )
					             + this->mainPaneSize.cx
					             + ( this->dockLayouts[ dockRIGHT].visible && ( coveredRight & coveredCorners)
					               ? this->dockLayouts[ dockRIGHT].size
					               : 0
								   )
					             ;
					return true;
					
				case dockLEFT:
					
					fullSize     = this->mainPaneSize.cx;
					if ( this->dockLayouts[ dockRIGHT].visible)
						fullSize += this->dockLayouts[ dockRIGHT].size;
					fullSize     /= 4;
					if ( size > fullSize || size == 0)
						size = fullSize;
					
					rect->top    = this->dockLayouts[ dockTOP].visible && !( coveredLeft & coveredCorners)
					             ? this->dockLayouts[ dockTOP].size
					             : 0
					             ;
					rect->bottom = ( this->dockLayouts[ dockTOP].visible
					               ? this->dockLayouts[ dockTOP].size
					               : 0
								   )
					             + this->mainPaneSize.cy
					             + ( this->dockLayouts[ dockBOTTOM].visible && ( coveredRight & coveredCorners)
					               ? this->dockLayouts[ dockBOTTOM].size
					               : 0
								   )
					             ;
					rect->left   = 0;
					rect->right  = size;
					return true;
					
				case dockRIGHT:
					
					fullSize     = this->mainPaneSize.cx;
					if ( this->dockLayouts[ dockLEFT].visible)
						fullSize += this->dockLayouts[ dockLEFT].size;
					if ( size > fullSize / 4 || size == 0)
						size = fullSize / 4;
					
					rect->top    = this->dockLayouts[ dockTOP].visible && !( coveredRight & coveredCorners)
					             ? this->dockLayouts[ dockTOP].size
					             : 0
					             ;
					rect->bottom = ( this->dockLayouts[ dockTOP].visible
					               ? this->dockLayouts[ dockTOP].size
					               : 0
								   )
					             + this->mainPaneSize.cy
					             + ( this->dockLayouts[ dockBOTTOM].visible && ( coveredLeft & coveredCorners)
					               ? this->dockLayouts[ dockBOTTOM].size
					               : 0
								   )
					             ;
					rect->left   = fullSize - size;
					rect->right  = fullSize;
					return true;
				default:
					return false;
				}
			}
			
			bool isDockVisible( DockSide dockSide) const {
				
				return this->dockLayouts[ dockSide].visible;
			}
			
			bool setDock( DockSide dockSide, int coveredCorners, int size) {
				
				ATLASSERT( size > 0);
				
				switch ( dockSide) {
				case dockTOP:
				case dockBOTTOM:
					
					if ( this->mainPaneSize.cy > LayoutManager::mainPaneMinCY) {
						
						if ( size > this->mainPaneSize.cy - LayoutManager::mainPaneMinCY)
							return false;
						this->mainPaneSize.cy -= size;
					}
					break;
				case dockLEFT:
				case dockRIGHT:
					
					if ( this->mainPaneSize.cx > LayoutManager::mainPaneMinCX) {
						
						if ( size > this->mainPaneSize.cx - LayoutManager::mainPaneMinCX)
							return false;
						this->mainPaneSize.cx -= size;
					}
					break;
				default:
					return false;
				}
				
				DockLayout* dockLayoutLeft;
				DockLayout* dockLayoutRight;
				switch ( dockSide) {
				case dockTOP:
					
					dockLayoutLeft  = &this->dockLayouts[ dockRIGHT];
					dockLayoutRight = &this->dockLayouts[ dockLEFT];
					break;
					
				case dockBOTTOM:
					
					dockLayoutLeft  = &this->dockLayouts[ dockLEFT];
					dockLayoutRight = &this->dockLayouts[ dockRIGHT];
					break;
					
				case dockLEFT:
					
					dockLayoutLeft  = &this->dockLayouts[ dockTOP];
					dockLayoutRight = &this->dockLayouts[ dockBOTTOM];
					break;
					
				case dockRIGHT:
					
					dockLayoutLeft  = &this->dockLayouts[ dockBOTTOM];
					dockLayoutRight = &this->dockLayouts[ dockTOP];
					break;
					
				}
				
				DockLayout& dockLayout = this->dockLayouts[ dockSide];
				dockLayout.visible = true;
				dockLayout.place   = 0;
				
				if ( ( coveredLeft & coveredCorners) && dockLayoutLeft->visible) {
					dockLayout.left       = 1;
					dockLayoutLeft->right = 0;
				} else
					dockLayoutLeft->right = 1;
				
				if ( ( coveredRight & coveredCorners) && dockLayoutRight->visible) {
					dockLayout.right     = 1;
					dockLayoutRight->left = 0;
				} else
					dockLayoutRight->left = 1;
				
				this->dockLayouts[ dockSide].size = size;
				this->dockLayouts[ dockSide].visible = true;
				return false;
			}
			
			bool unsetDock( DockSide dockSide) {
				
				this->dockLayouts[ dockSide].visible = false;
				return true;
			}
			
			bool showDock( DockSide dockSide, bool show) {
				
				DockLayout& dockLayout = this->dockLayouts[ dockSide];
				
				// adjust main pane size
				if ( show) {
					
					if ( 0 == dockLayout.size)
						return false;
					
					int offset = dockLayout.size;
					switch ( dockSide) {
					case dockTOP:
					case dockBOTTOM:
						if ( this->mainPaneSize.cy - offset > mainPaneMinCY)
							this->mainPaneSize.cy -= offset;
						break;
					case dockLEFT:
					case dockRIGHT:
						if ( this->mainPaneSize.cx - offset > mainPaneMinCX)
							this->mainPaneSize.cx -= offset;
						break;
					}
				} else {
					
					switch ( dockSide) {
					case dockTOP:
					case dockBOTTOM:
						this->mainPaneSize.cy += dockLayout.size;
						break;
					case dockLEFT:
					case dockRIGHT:
						this->mainPaneSize.cx += dockLayout.size;
						break;
					}
				}
				
				dockLayout.visible = show;
				return true;
			}
			
			bool resizeDock( DockSide dockSide, int size) {
				
				DockLayout& dockLayout = this->dockLayouts[ dockSide];
				if ( !dockLayout.visible)
					return false;
				
				int offset = dockLayout.size - size;
				
				// calculate offset and check if it fits main pane size
				// adjust main pane size
				switch ( dockSide) {
				case dockTOP:
				case dockBOTTOM:
					
					if ( this->mainPaneSize.cy + offset < mainPaneMinCY || dockLayout.size - offset < mainPaneMinCY)
						return false;
					this->mainPaneSize.cy += offset;
					break;
					
				case dockLEFT:
				case dockRIGHT:
					
					if ( this->mainPaneSize.cx + offset < mainPaneMinCX || dockLayout.size - offset < mainPaneMinCX)
						return false;
					this->mainPaneSize.cx += offset;
					break;
					
				default:
					return false;
				}
				
				// adjust dock pane size
				dockLayout.size -= offset;
				return true;
			}
			
			bool resize( int cx, int cy) {
				
				// calculate the current size
				CSize size( 0, 0);
				if ( this->dockLayouts[ dockTOP].visible)    size.cy += this->dockLayouts[ dockTOP].size;
				if ( this->dockLayouts[ dockBOTTOM].visible) size.cy += this->dockLayouts[ dockBOTTOM].size;
				if ( this->dockLayouts[ dockLEFT].visible)   size.cx += this->dockLayouts[ dockLEFT].size;
				if ( this->dockLayouts[ dockRIGHT].visible)  size.cx += this->dockLayouts[ dockRIGHT].size;
				
				size.cx += this->mainPaneSize.cx;
				size.cy += this->mainPaneSize.cy;
				
				size.cx = cx - size.cx;
				size.cy = cy - size.cy;
				
				// adjust main pane size
				if ( this->mainPaneSize.cx + size.cx > LayoutManager::mainPaneMinCX)
					this->mainPaneSize.cx += size.cx;
				
				if ( this->mainPaneSize.cy + size.cy > LayoutManager::mainPaneMinCY)
					this->mainPaneSize.cy += size.cy;
				return true;
			}
		}; // class LayoutManager
		
	//----------------- protected members
	protected:
		
		// Context for drag and drop actions
		FloatFrameDragContext* dragContext;
		
		SplitPane* currentPane; // the current split pane. It must be mainPane, a docked split pane or a float one.
		SplitPane* lastPane;    // the previously used pane. It must be mainPane or a docked split pane.
		SplitPane  mainPane;    // main split pane
		HIMAGELIST imageList;   // Image List
		
		// Counter and container for drop parcels
		long                         dropParcelCount; // current drop dropParcel tag for the next drop parcel
		CAtlMap< long, DropParcel*>  dropParcels;     // request queue for drop operation
		
		// Containers for all split panes, client windows, float frames and dock panes
		CAtlMap< HWND, SplitPane*>   splitPanes;
		CAtlMap< HWND, ClientView*> clientViews;
		CAtlMap< HWND, FloatFrame*> floatFrames;
		
		DockPane* dockPanes[4];       // four dock pane container with DockSide type of index
		CRect     dockToFloatRect[4]; // float rectangles for dock panes
		
		LayoutManager layout;
		
		FrameListener* cbListener;
	//----------------- protected interface
	protected:
		
		// set the current split pane function
		// helper function to activate either mainPane or one of the dock panes 
		// when the focus to this window is set.
		void setCurrentPane( SplitPane* splitPane = NULL) {
			
			if ( splitPane != this->currentPane) {
				
				if ( NULL == splitPane) {
					
					// restoring the last pane
					splitPane = NULL == this->lastPane? &this->mainPane: this->lastPane;
					this->lastPane = NULL;
					
				} else if ( this->currentPane == &this->mainPane)
					
					this->lastPane = this->currentPane;
				else
					for ( int i=0; i<4; i++) {
						
						DockPane* dockPane = this->dockPanes[i];
						if ( NULL != dockPane && dockPane->getPane() == this->currentPane)
							this->lastPane = this->currentPane;
					}
				
				this->currentPane = splitPane;
			}
			for ( int i=0; i<4; i++) {
				
				DockPane* dockPane = this->dockPanes[i];
				if ( NULL != dockPane)
					dockPane->setActive( dockPane->getPane() == this->currentPane);
			}
		}
		
		// drop parcel queue functions
		void postParcel( long parcelTag) {
			
			this->PostMessage( eventDropParcel, static_cast<WPARAM>( parcelTag));
		}
		
		long addDropParcel( DropParcel* dropParcel) {
			
			this->dropParcelCount++;
			this->dropParcels[this->dropParcelCount] = dropParcel;
			return this->dropParcelCount;
		}
		
		DropParcel* getDropParcel( long parcelTag) const {
			
			const CAtlMap< long, DropParcel*>::CPair* parcelPair = this->dropParcels.Lookup( parcelTag);
			if ( NULL == parcelPair)
				return NULL;
			
			return parcelPair->m_value;
		}
		
		void removeDropParcel( long parcelTag) {
			
			this->dropParcels.RemoveKey( parcelTag);
		}
		
		// look up helper functions
		ClientView* lookUpClientView( HWND clientViewWnd) const {
			
			const CAtlMap< HWND, ClientView*>::CPair* pair = this->clientViews.Lookup( clientViewWnd);
			if ( NULL == pair)
				return NULL;
			
			return pair->m_value;
		}
		
		SplitPane* lookUpClientSplitPane( HWND clientViewWnd) const {
			
			const CAtlMap< HWND, ClientView*>::CPair* pair = this->clientViews.Lookup( clientViewWnd);
			if ( NULL == pair)
				return NULL;
			
			return pair->m_value->owner;
		}
		
		ClientViewStateSet getPaneStateSet( SplitPane* splitPane) {
			
			/*using ClientView;::stateNull;
			using ClientView::stateDockable;
			using ClientView::stateFloating;*/
			
			ClientViewStateSet stateSet;
			POSITION position = this->clientViews.GetStartPosition();
			if ( NULL != position)
				do {
					ClientView* clientView = this->clientViews.GetNextValue( position);
					if ( clientView->owner == splitPane)
						switch ( clientView->state) {
						case ClientView::stateNull:     stateSet.null     |= 1; break;
						case ClientView::stateDockable: stateSet.dockable |= 1; break;
						case ClientView::stateFloating: stateSet.floating |= 1; break;
						}
				} while ( NULL != position);
			return stateSet;
		}
		
		SplitPane* lookUpSplitPane( HWND splitPaneWnd) const {
			
			const CAtlMap< HWND, SplitPane*>::CPair* pair = this->splitPanes.Lookup( splitPaneWnd);
			if ( NULL == pair)
				return NULL;
			
			return pair->m_value;
		}
		
		FloatFrame* lookUpFloatFrame( HWND floatFrameWnd) const {
			
			const CAtlMap< HWND, FloatFrame*>::CPair* pair = this->floatFrames.Lookup( floatFrameWnd);
			if ( NULL == pair)
				return NULL;
			
			return pair->m_value;
		}
		
		FloatFrame* lookUpFloatFrameSplitPaneWnd( HWND splitPaneWnd) const {
			
			HWND floatFrameWnd = ::GetParent( splitPaneWnd);
			if ( NULL == floatFrameWnd)
				return NULL;
			
			const CAtlMap< HWND, FloatFrame*>::CPair* pair = this->floatFrames.Lookup( floatFrameWnd);
			if ( NULL == pair)
				return NULL;
			
			return pair->m_value;
		}
		
		DockPane* createDockPane( DockSide dockSide, SplitPane* splitPane = NULL) {
			
			int dockSize = this->layout.getDockSize( dockSide);
			int dockWidth = this->layout.getDockWidth( dockSide);
			if ( 0 == dockSize)
				this->layout.calcDockSize( dockSide, dockWidth, dockSize);
			
			return this->createDockPane( dockSide, dockWidth, dockSize, splitPane);
		}

		DockPane* createDockPane( DockSide dockSide, int dockWidth, int dockSize, SplitPane* splitPane = NULL) {
			
			ATLASSERT( NULL == this->dockPanes[ dockSide]);
			
			DockPane* dockPane = new DockPane( this);
			dockPane->create( this->m_hWnd, dockSide);
			
			if ( NULL == splitPane) {
				
				splitPane = new SplitPane( this);
				splitPane->create( dockPane->m_hWnd);
				this->splitPanes[ splitPane->m_hWnd] = splitPane;
			} else
				splitPane->SetParent( dockPane->m_hWnd);
			
			dockPane->attachPane( splitPane);
			this->dockPanes[ dockSide] = dockPane;
			this->layout.setDock( dockSide, dockWidth, dockSize);
			this->updateLayout();
			return dockPane;
		}
		
		void removeDockPane( DockSide dockSide) {
			
			DockPane* dockPane = this->dockPanes[ dockSide];
			this->showDockPane( dockSide, false);
			this->dockPanes[ dockSide] = NULL;
			this->updateLayout();
			delete dockPane;
		}
		
		void removeFloatFrame( HWND floatFrameWnd) {
			
			FloatFrame* floatFrame = this->lookUpFloatFrame( floatFrameWnd);
			this->removeFloatFrame( floatFrame);
		}
		
		void removeFloatFrame( FloatFrame* floatFrame) {
			
			this->floatFrames.RemoveKey( floatFrame->m_hWnd);
			delete floatFrame;
		}
		
		FloatFrame* createFloatFrame( LPRECT rect, SplitPane* splitPane = NULL) {
			
			FloatFrame* floatFrame = new FloatFrame( this);
			floatFrame->create( this->m_hWnd, rect);
			
			if ( NULL == splitPane) {
				
				splitPane = new SplitPane( this);
				splitPane->create( floatFrame->m_hWnd);
				this->splitPanes[ splitPane->m_hWnd] = splitPane;
			} else
				splitPane->SetParent( floatFrame->m_hWnd);
			
			floatFrame->attachPane( splitPane);
			this->floatFrames[ floatFrame->m_hWnd] = floatFrame;
			
			return floatFrame;
		}
		
		// hit test function
		enum HitTest {
			  htUnknown
			, htTop
			, htBottom
			, htLeft
			, htRight
		};
		
		inline HitTest hitTest( LPPOINT point) {
			
			ATLASSERT( NULL != point);
			
			CRect rect;
			this->GetClientRect( rect);
			this->ClientToScreen( rect);
			if ( rect.Height() <= 30 || rect.Width() <= 30)
				return htUnknown;
			
			if      ( point->y - rect.top    <= 15) return htTop;
			else if ( rect.bottom - point->y <= 15) return htBottom;
			else if ( point->x - rect.left   <= 15) return htLeft;
			else if ( rect.right - point->x  <= 15) return htRight;
			else
                return htUnknown;
		}
		
		void detachClientViews( SplitPane* splitPane, FramePlace lastPlace, DockSide dockSide = dockUNKNOWN) {
			
			HWND thisParentWnd = this->GetParent();
			POSITION position = this->clientViews.GetStartPosition();
			if ( NULL != position)
				do {
					ClientView* clientView = this->clientViews.GetNextValue( position);
					if ( clientView->owner == splitPane) {
						
						clientView->owner     = NULL;
						clientView->lastPlace = lastPlace;
						if ( placeDOCKPANE == lastPlace)
							clientView->dockSide = dockSide;
						
						CWindow clientViewWindow( clientView->wnd);
						if ( clientViewWindow.IsWindow()) {
							
							CRect floatRect;
							clientViewWindow.GetClientRect( floatRect);
							clientViewWindow.ClientToScreen( floatRect);
							clientView->floatRect = floatRect;
							clientViewWindow.ShowWindow( SW_HIDE);
							clientViewWindow.SetParent( thisParentWnd);
						}
						this->clientViews.RemoveKey( clientView->wnd);
						this->cbListener->clientDetached( clientView);
					}
				} while ( NULL != position);
			
			if ( &this->mainPane != splitPane)
				this->splitPanes.RemoveKey( splitPane->m_hWnd);
			return;
		}
	public:
		
		// Constructor/destructor
		
		Frame( FrameListener* cbListener)
			: dragContext( NULL)
			, currentPane( NULL)
			, lastPane( NULL)
			, mainPane( this, true)
			, dropParcelCount( 0)
			, imageList( NULL)
			, cbListener( cbListener)
		{
			memset( this->dockPanes, 0, sizeof(this->dockPanes));
			
			for ( int i=0; i<4; i++)
				this->dockToFloatRect[i] = rectNULL;
		}
		
		~Frame() {
			
			if ( this->dragContext)
				delete dragContext;
			
			POSITION position;
			
			if ( NULL != ( position = this->floatFrames.GetStartPosition()))
				do
					delete this->floatFrames.GetNextValue( position);
				while ( NULL != position);
			this->floatFrames.RemoveAll();
			
			if ( NULL != ( position = this->dropParcels.GetStartPosition()))
				do
					delete this->dropParcels.GetNextValue( position);
				while ( NULL != position);
			this->dropParcels.RemoveAll();
			
			for ( int i=0; i<4; i++)
				if ( NULL != this->dockPanes[i])
					delete this->dockPanes[i];
			
			if ( this->mainPane.m_hWnd)
				this->mainPane.DestroyWindow();
			
		}
		
		static DWORD getFloatFrameStyle() {
			return FloatFrame::getFloatFrameStyle();
		}
		static DWORD getFloatFrameStyleEx() {
			return FloatFrame::getFloatFrameStyleEx();
		}

		// public interface
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
		
		// sets the keyboard focus to the specified client view window
		bool setFocusTo( HWND clientView) {
			
			ATLASSERT( ::IsWindow( clientView));
			
			// check if client window in a float frame
			POSITION position = this->floatFrames.GetStartPosition();
			if ( NULL != position)
				do {
					FloatFrame* floatFrame = this->floatFrames.GetNextValue( position);
					if ( floatFrame->setFocusTo( clientView))
                        return true;
				} while ( NULL != position);
			
			// check if client window in a dock pane
			for ( int i=0; i<4; i++) {
				
				DockPane* dockPane = this->dockPanes[i];
				if ( NULL != dockPane && dockPane->setFocusTo( clientView))
					return true;
			}
			
			// check if client window in main pane
			return this->mainPane.setFocusTo( clientView);
		}
		
		// retrieves the client view window that receives the keyboard focus
		HWND focusedClientView() const {
			
			return NULL == this->currentPane? NULL: this->currentPane->focusedClientView();
		}

		bool isFloatClient(HWND clientViewWnd) {
			ClientView* clientView = lookUpClientView(clientViewWnd);
			POSITION position = this->floatFrames.GetStartPosition();
			if ( NULL != position)
				do {
					
					FloatFrame* floatFrame = this->floatFrames.GetNextValue( position);
					if ( floatFrame->getPane() == clientView->owner) {
						
						return true;
					}
				} while ( NULL != position );

			return false;
		}
		
		// toggles client view state and move the client view to an appropriate pane
		bool toggleClientViewState( HWND clientViewWnd, ClientView::State state, bool allowDockingFloatFrame) {
			
/*			using ClientView::stateDockable;
			using ClientView::stateFloating;
			using ClientView::stateNull;
	*/		
			ATLASSERT( ClientView::stateNull != state);
			
			ClientView* clientView = this->lookUpClientView( clientViewWnd);
			if ( NULL == clientView)
				return false;
			
			DropParcel* dropParcel    = new DropParcel();
			dropParcel->clientViewWnd = clientViewWnd;
			if ( clientView->owner == &this->mainPane) {
				
				dropParcel->sourcePlace = placeMAINPANE;
				dropParcel->sourcePaneWnd = clientView->owner->m_hWnd;
			} else {
				
				for ( int i=0; i<4; i++) {
					
					DockPane* dockPane = this->dockPanes[i];
					if ( NULL == dockPane)
						continue;
					if ( dockPane->getPane() == clientView->owner) {
						
						dropParcel->sourcePlace   = placeDOCKPANE;
						dropParcel->sourceDock    = static_cast< DockSide>( i);
						dropParcel->sourcePaneWnd = dockPane->m_hWnd;
						break;
					}
				}
				
				if ( placeUNKNOWN == dropParcel->sourcePlace) {
					
					POSITION position = this->floatFrames.GetStartPosition();
					if ( NULL != position)
						do {
							
							FloatFrame* floatFrame = this->floatFrames.GetNextValue( position);
							if ( floatFrame->getPane() == clientView->owner) {
								
								dropParcel->sourcePlace = placeFLOATFRAME;
								dropParcel->sourcePaneWnd = floatFrame->m_hWnd;
								break;
							}
						} while ( NULL != position );
						
					if ( placeUNKNOWN == dropParcel->sourcePlace) {
						
						delete dropParcel;
						return false;
					}
				}
			}
			
			if (    dockUNKNOWN   == clientView->dockSide
			            && placeDOCKPANE != dropParcel->sourcePlace
			            && ClientView::stateDockable == state
			          ) {
				
				// there is no information what dock side is going to be.
				delete dropParcel;
				return false;
			} else if (    placeFLOATFRAME == dropParcel->sourcePlace
			     && (    ( ClientView::stateFloating == state && ClientView::stateFloating != clientView->state)
			          || ( ClientView::stateDockable == state && ClientView::stateFloating == clientView->state)
			        )
			   ) {
				
				delete dropParcel;
				dropParcel = NULL;
				
			} else if (    (    placeFLOATFRAME == dropParcel->sourcePlace
			                 || placeMAINPANE   == dropParcel->sourcePlace
			               )
			            && dockUNKNOWN != clientView->dockSide
			            && ClientView::stateDockable == state
			            && ClientView::stateDockable != clientView->state
			          ) {
				// if the client view state is going to be dockable
				// and it wasn't before
				// and the client view either in a float frame or in Main Pane
				// and last time it was in a dock pane
				// then dock the client view where it was last time
				dropParcel->targetPlace = placeDOCKPANE;
				dropParcel->targetDock  = clientView->dockSide;
				dropParcel->dockSize  = this->layout.getDockSize( clientView->dockSide);
				dropParcel->dockWidth = this->layout.getDockWidth( clientView->dockSide);
				if ( 0 == clientView->dockSide)
					this->layout.calcDockSize( clientView->dockSide, dropParcel->dockWidth, dropParcel->dockSize);
				if ( NULL != this->dockPanes[clientView->dockSide])
					dropParcel->targetPaneWnd = this->dockPanes[clientView->dockSide]->m_hWnd;
				
			} else if (    ClientView::stateFloating == state
			            && ClientView::stateFloating != clientView->state
			            && ( placeMAINPANE == dropParcel->sourcePlace || placeDOCKPANE == dropParcel->sourcePlace)
			          ){
				
				// if the client view state is gonna be floating
				// and it wasn't before
				// and the client view either in a dock pane or in Main Pane
				// then float the client view 
				dropParcel->targetPlace = placeFLOATFRAME;
				
			} else {
				
				dropParcel->targetPlace   = placeMAINPANE;
				dropParcel->targetPaneWnd = this->mainPane.m_hWnd;
			}
			
			switch ( state) {
			case ClientView::stateDockable:
				clientView->state = ( ClientView::stateDockable == clientView->state)? ClientView::stateNull: ClientView::stateDockable;
				break;
			case ClientView::stateFloating:
				// anders avbryter floating her
				if (allowDockingFloatFrame)
					clientView->state = ( ClientView::stateFloating == clientView->state)? ClientView::stateNull: ClientView::stateDockable; else
					clientView->state = ( ClientView::stateFloating == clientView->state)? ClientView::stateNull: ClientView::stateFloating;
				break;
			}
			
			if ( NULL != dropParcel)
				this->postParcel( this->addDropParcel( dropParcel));
			return true;
		}

		SplitPane* getSplitPane( HWND splitPaneWnd) const {
			
			return this->lookUpSplitPane( splitPaneWnd);
		}

        SplitPane* getMainPane() { return &mainPane; }


		HWND getFloatFrame( HWND clientViewWnd) const {
			ClientView* clientView = lookUpClientView(clientViewWnd);
			POSITION position = this->floatFrames.GetStartPosition();
			if ( NULL != position)
				do {
					
					FloatFrame* floatFrame = this->floatFrames.GetNextValue( position);
					if ( floatFrame->getPane() == clientView->owner) {
						
						return floatFrame->m_hWnd;
					}
				} while ( NULL != position );

			return NULL;
		}

		// retrieves client view properties by client view window handle
		ClientView* getClientView( HWND clientViewWnd) const {
			
			return this->lookUpClientView( clientViewWnd);
		}
		
		// adds a new client view
		bool addClientView( ClientView* clientView) {
			
			ATLASSERT( ::IsWindow( clientView->wnd));
			ATLASSERT( NULL == this->lookUpClientView( clientView->wnd));
			
			this->setCurrentPane( &this->mainPane);
			
			this->clientViews[ clientView->wnd] = clientView;
			clientView->owner = this->currentPane;
			return this->currentPane->append( clientView->caption
			                                , clientView->wnd
			                                , clientView->toolTip
			                                , clientView->imageIndex
			                                );
		}

        // adds a new (manually created and placed) clientview
        bool registerClientView(ClientView* clientView, SplitPane* owner, bool dockable) {
			this->clientViews[ clientView->wnd] = clientView;
			clientView->owner = owner;
            clientView->state = dockable ? ClientView::stateDockable : ClientView::stateNull;
			this->setCurrentPane( owner);

            return true;
        }

		// adds a new client view and attach it to the specified dock pane
		bool dockClientView( DockSide dockSide, ClientView* clientView) {
			
			ATLASSERT( ::IsWindow( clientView->wnd));
			ATLASSERT( dockUNKNOWN != dockSide);
			
			SplitPane* splitPane;
			DockPane* dockPane = this->dockPanes[ dockSide];
			if ( NULL == dockPane)
				dockPane = this->createDockPane( dockSide);
			
			splitPane = dockPane->getPane();
			
			this->setCurrentPane( splitPane);
			
			this->clientViews[ clientView->wnd] = clientView;
			clientView->owner = this->currentPane;
			clientView->state = ClientView::stateDockable;
			return this->currentPane->append( clientView->caption
			                                , clientView->wnd
			                                , clientView->toolTip
			                                , clientView->imageIndex
			                                );
		}
		
		// shows/hides the specified dock pane
		bool showDockPane( DockSide dockSide, bool show) {
			
			if ( show) {
				
				if ( NULL == this->dockPanes[ dockSide])
					return false;
				if ( this->layout.isDockVisible( dockSide))
					return true;
			} else {
				
				if ( NULL == this->dockPanes[ dockSide])
					return true;
			}
			
			this->layout.showDock( dockSide, show);
			this->updateLayout();
			return true;
		}
		
		// retrieves the visibility state of the specified dock pane
		bool dockPaneVisible( DockSide dockSide) const {
			
			return this->layout.isDockVisible( dockSide);
		}
		
		// retrieves the existence state of the specified dock pane
		bool dockPaneExists( DockSide dockSide) const {
			
			return NULL != this->dockPanes[ dockSide];
		}
		
		// float frame iterator functions
		POSITION floatFrameStart() const {
			
			return this->floatFrames.GetStartPosition();
		}
		HWND floatFrameNext( POSITION& position) const {
			
			FloatFrame* floatFrame = this->floatFrames.GetNextValue( position);
			if ( NULL == floatFrame)
				return 0L;
			
			return floatFrame->m_hWnd;
		}
		
		// adds a new client view and attach it to a new float frame with the specified location
		HWND floatClientView( LPRECT rect, ClientView* clientView) {
			
			ATLASSERT( ::IsWindow( clientView->wnd));
			
			CRect floatRect( rect);
			if (floatRect.left != CW_USEDEFAULT) { // megz.restoresizefix
				DWORD dwStyle = FloatFrame::getFloatFrameStyle();
				DWORD dwStyleEx = FloatFrame::getFloatFrameStyleEx();
				AdjustWindowRectEx(&floatRect, dwStyle, FALSE, dwStyleEx);
				floatRect.bottom += 24;  // tab height: TODO: should calculate tab height based on the font size calculations in TabControl::create()
			}

			FloatFrame* floatFrame = this->createFloatFrame( floatRect);
			if ( NULL == floatFrame)
				return NULL;
			
			this->setCurrentPane( floatFrame->getPane());
			this->clientViews[ clientView->wnd] = clientView;
			clientView->owner = this->currentPane;
			this->currentPane->append( clientView->caption
			                         , clientView->wnd
			                         , clientView->toolTip
			                         , clientView->imageIndex
			                         );
			clientView->state = clientView->allowFloat?ClientView::stateDockable:ClientView::stateFloating;
			floatFrame->UpdateLayout(FALSE); // anders moved from FloatFrame::attachPane to prevent premature redrawing
			return floatFrame->m_hWnd;
		}
		
		// retrieves the visibility state of the specified float frame
		bool showFloatFrame( HWND floatFrameWnd, bool show) {
			
			ATLASSERT( ::IsWindow( floatFrameWnd));
			
			FloatFrame* floatFrame = this->lookUpFloatFrame( floatFrameWnd);
			ATLASSERT( NULL != floatFrame);
			
			return TRUE == floatFrame->ShowWindow( show? SW_SHOW: SW_HIDE);
		}
		
		// check if a float frame exists
		bool checkFloatFrame( HWND floatFrameWnd) const {
			
			return NULL != this->lookUpFloatFrame( floatFrameWnd);
		}
		
		// check if a float frame is visible
		bool floatFrameVisible( HWND floatFrameWnd) const {
			
			FloatFrame* floatFrame = this->lookUpFloatFrame( floatFrameWnd);
			if ( NULL == floatFrame)
				return false;
			
			return TRUE == floatFrame->IsWindowVisible();
		}
		
		// reattach the client view. the function uses the last location object propertes
		void attachClientView( ClientView* clientView) {
			
			ATLASSERT( ::IsWindow( clientView->wnd));
			ATLASSERT( NULL == this->lookUpClientView( clientView->wnd));
			
			switch ( clientView->lastPlace) {
			case placeDOCKPANE:
				
				ATLASSERT( dockUNKNOWN != clientView->dockSide);
				this->dockClientView( clientView->dockSide, clientView);
				break;
			case placeFLOATFRAME:
				
				ATLASSERT( !clientView->floatRect.IsRectEmpty());
				this->floatClientView( clientView->floatRect, clientView);
				break;
			default:
				
				this->addClientView( clientView);
				break;
			}
			return;
		}
		
		// detaches the client view window 
		// This method changes the parent window of the client view window to the Frame's parent.
		ClientView* detachClientView( HWND clientViewWnd) {
			
			ATLASSERT( ::IsWindow( clientViewWnd));
			
			ClientView* clientView = this->lookUpClientView( clientViewWnd);
			if ( NULL == clientView)
				return NULL;
			
			CWindow clientViewWindow( clientViewWnd);
			clientViewWindow.ShowWindow( SW_HIDE);
			int clientCount = 0;
			SplitPane* splitPane = NULL;
			
			// check if client window in main pane
			if ( &this->mainPane == clientView->owner) {
				splitPane = clientView->owner;
				clientView->lastPlace = placeMAINPANE;
			} else {
				
				// check if client window in a float frame
				POSITION position = this->floatFrames.GetStartPosition();
				if ( NULL != position)
					do {
						
						FloatFrame* floatFrame = this->floatFrames.GetNextValue( position);
						if ( floatFrame->getPane() == clientView->owner) {
							
							clientViewWindow.GetClientRect( clientView->floatRect);
							clientViewWindow.ClientToScreen( clientView->floatRect);
							splitPane = clientView->owner;
							if ( 1 == ( clientCount = splitPane->getClientViewCount())) {
								
								splitPane->ShowWindow( SW_HIDE);
								splitPane->SetParent( this->m_hWnd);
								floatFrame->detachPane();
								this->removeFloatFrame( floatFrame);
								clientView->lastPlace = placeFLOATFRAME;
							}
							break;
						}
					} while ( NULL != position);
				
				// check if client window in a dock pane
				if ( NULL == splitPane) {
					
					for ( int i=0; i<4; i++) {
						
						DockPane* dockPane = this->dockPanes[i];
						if ( NULL != dockPane && dockPane->getPane() == clientView->owner) {
							
							clientView->lastPlace = placeDOCKPANE;
							clientView->dockSide  = static_cast<DockSide>(i);
							splitPane = clientView->owner;
							if ( 1 == ( clientCount = splitPane->getClientViewCount())) {
								
								splitPane->ShowWindow( SW_HIDE);
								splitPane->SetParent( this->m_hWnd);
								dockPane->detachPane();
								this->removeDockPane( static_cast<DockSide>(i));
							}
							break;
						}
					}
				}
			}
			
			
			// remove the client window from anywhere
			clientViewWindow.SetParent( this->m_hWnd);
			
			this->clientViews.RemoveKey( clientViewWnd);
			
			splitPane->detachClientView( clientViewWnd);
			if ( 1 == clientCount) {
				
				if ( this->currentPane == splitPane)
					this->setCurrentPane();
				this->splitPanes.RemoveKey( splitPane->m_hWnd);
				delete splitPane;

                this->lastPane = 0;
			}
			
			clientView->owner = NULL;
			return clientView;
		}
		
		// retrieves the visibility state of the specified client view window handle
		bool clientViewVisible( HWND clientViewWnd) {
			
			ClientView* clientView = this->lookUpClientView( clientViewWnd);
			if ( NULL == clientView)
				return false;
			
			return clientView->owner->focusedClientView() == clientViewWnd;
		}
		
		// checks if client view window is attached.
		bool clientViewAttached( HWND clientViewWnd) {
			
			ClientView* clientView = this->lookUpClientView( clientViewWnd);
			return NULL != clientView;
		}
		
		// updates layout
		void updateLayout() {
			
			for ( int i=0; i<4; i++) {
				
				DockPane* dockPane = this->dockPanes[i];
				if ( NULL != dockPane && NULL!=dockPane->m_hWnd) {	// anders was here, crashed on exit without checking for hWnd was null
					
					if ( this->layout.isDockVisible( static_cast< DockSide>( i))) {
						CRect rect;
						this->layout.getDockRect( static_cast< DockSide>( i), rect);
						dockPane->SetWindowPos( NULL, rect, SWP_NOZORDER | SWP_SHOWWINDOW);
					} else
						dockPane->SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_HIDEWINDOW);
				}
			}
			
			CRect rect;
			this->layout.getMainPaneRect( rect);
			this->mainPane.SetWindowPos( NULL, rect, SWP_NOZORDER | SWP_SHOWWINDOW);
			this->Invalidate();
			return;
		}
		
		// sets and gets Image List
		void setImageList( HIMAGELIST imgList) {
			
			this->imageList = imgList;
			POSITION position = this->splitPanes.GetStartPosition();
			if ( NULL != position)
				do
					this->splitPanes.GetNextValue( position)->setImageList( imgList);
				while ( NULL != position);
			return;
		}
		
		HIMAGELIST getImageList() const {
			return this->imageList;
		}
		
		// FloatDockFrameListener interface
		CRect getDockSliderRect( int dockSide) {
			
			CRect result;
			CRect mainPaneRect;
			this->mainPane.GetClientRect( mainPaneRect);
			this->mainPane.ClientToScreen( mainPaneRect);
			
			ATLASSERT( this->dockPanes[ dockSide]);
			
			switch ( dockSide) {
			case dockTOP:
				
				this->dockPanes[ dockTOP]->GetClientRect( result);
				this->dockPanes[ dockTOP]->ClientToScreen( result);
				result.top    += 10;
				result.bottom += mainPaneRect.Height() - 10;
				break;
				
			case dockBOTTOM:
				
				this->dockPanes[ dockBOTTOM]->GetClientRect( result);
				this->dockPanes[ dockBOTTOM]->ClientToScreen( result);
				result.bottom -= 10;
				result.top    -= mainPaneRect.Height() - 10;
				break;
				
			case dockLEFT:
				
				this->dockPanes[ dockLEFT]->GetClientRect( result);
				this->dockPanes[ dockLEFT]->ClientToScreen( result);
				result.left  += 10;
				result.right += mainPaneRect.Width() - 10;
				break;
				
			case dockRIGHT:
				
				this->dockPanes[ dockRIGHT]->GetClientRect( result);
				this->dockPanes[ dockRIGHT]->ClientToScreen( result);
				result.right -= 10;
				result.left  -= mainPaneRect.Width() - 10;
				break;
				
			}
			
			return result;
		}
		
		void dropDockSlider( int dockSide, int offset) {
			
			int size = this->layout.getDockSize( static_cast< DockSide>( dockSide)) + offset;
			this->layout.resizeDock( static_cast< DockSide>( dockSide), size);
			this->updateLayout();
			return;
		}
		
		void floatDockPane( int dockSide) {
			
			ATLASSERT( this->dockPaneVisible( static_cast< DockSide>( dockSide)));
			
			DockPane* dockPane = this->dockPanes[dockSide];
			DropParcel* dropParcel    = new DropParcel();
			dropParcel->sourcePlace    = placeDOCKPANE;
			dropParcel->sourceDock    = static_cast< DockSide>( dockSide);
			dropParcel->sourcePaneWnd = dockPane->m_hWnd;
			dropParcel->targetPlace    = placeFLOATFRAME;
			this->postParcel( this->addDropParcel( dropParcel));
		}
		
		void dockFloatFrame( HWND floatFrameWnd) {
			
			FloatFrame* floatFrame;
			if ( NULL != ( floatFrame = this->lookUpFloatFrame( floatFrameWnd))) {
				
				DropParcel* dropParcel = new DropParcel();
				dropParcel->sourcePlace = placeFLOATFRAME;
				dropParcel->sourcePaneWnd = floatFrameWnd;
				this->postParcel( this->addDropParcel( dropParcel));
			}
			return;
		} // void dock( HWND floatFrameWnd)
		
		void closeFloatFrame( HWND floatWnd) {
			this->PostMessage( eventCloseFloatFrame, 0, reinterpret_cast<LPARAM>( floatWnd));
			return;
		}
		
		void dragStart( HWND sourceWnd, HWND clientViewWnd, long x, long y, DWORD keysPressed) {
			
			ATLASSERT( NULL == this->dragContext);
			ATLASSERT( ::IsWindow( sourceWnd));
			ATLASSERT( NULL == clientViewWnd || ::IsWindow( clientViewWnd));
			
			CPoint hitPoint( x, y);
			CRect trackRect;
			
			FloatFrame* floatFrame;
			DockPane*   dockPane;
			
			this->dragContext = new FloatFrameDragContext( NULL);
			
			SplitPane* splitPane = NULL;
			
			// check out if source wnd is a dock pane
            int i;
			for ( i=0; i<4; i++) {
				
				DockPane* dockPane = this->dockPanes[i];
				if ( NULL != dockPane && ( dockPane->m_hWnd == sourceWnd || dockPane->getPane()->m_hWnd == sourceWnd))
					break;
			}
			
			DockSide dockSide = i < 4? static_cast<DockSide>( i): dockUNKNOWN;
			if ( dockUNKNOWN != dockSide) {
				
				dockPane  = this->dockPanes[ dockSide];
				splitPane = dockPane->getPane();
				
				if ( NULL == clientViewWnd) {
				
					CRect dockPaneRect;
					dockPane->GetClientRect( dockPaneRect);
					dockPane->ClientToScreen( dockPaneRect);
					
					trackRect = this->dockToFloatRect[ dockSide];
					
					if ( trackRect.IsRectEmpty()) {
						
						trackRect = dockPaneRect;
					} else {
						
						
						trackRect.InflateRect( 4, 12); // TODO need to calculate it more accuratelly
						
						int offset = trackRect.top - dockPaneRect.top;
						trackRect.top    -= offset;
						trackRect.bottom -= offset;
						
						double coef = static_cast<double>( dockPaneRect.left - hitPoint.x)
									/ static_cast<double>( hitPoint.x - dockPaneRect.right)
									;
						coef  *= trackRect.Width() / (1 + coef);
						offset = trackRect.left - hitPoint.x + static_cast<int>( coef);
						trackRect.left  -= offset;
						trackRect.right -= offset;
					}
				}
				this->dragContext->sourcePlace   = placeDOCKPANE;
				this->dragContext->sourceDock    = dockSide;
				this->dragContext->sourcePaneWnd = dockPane->m_hWnd;
				
			// check out if the source wnd is a float frame
			} else if (    NULL != ( floatFrame = this->lookUpFloatFrame( sourceWnd))
			            || NULL != ( floatFrame = this->lookUpFloatFrameSplitPaneWnd( sourceWnd))
			          ) {
				
				if ( NULL == clientViewWnd)
					floatFrame->GetWindowRect( trackRect);
				
				this->dragContext->sourcePlace   = placeFLOATFRAME;
				this->dragContext->sourcePaneWnd = floatFrame->m_hWnd;
				splitPane = floatFrame->getPane();
				
			} else {
				
				if ( NULL == clientViewWnd) {
					this->mainPane.GetClientRect( trackRect);
					this->mainPane.ClientToScreen( trackRect);
				}
				
				this->dragContext->sourcePlace   = placeMAINPANE;
				this->dragContext->sourcePaneWnd = this->mainPane.m_hWnd;
				splitPane = &this->mainPane;
			}
			
			// calculate track rectangle for the client view window
			if ( NULL != clientViewWnd) {
				
				ClientView* clientView = this->lookUpClientView( clientViewWnd);
				
				CWindow clientViewWindow( clientViewWnd);
				CRect clientViewRect;
				clientViewWindow.GetClientRect( clientViewRect);
				clientViewWindow.ClientToScreen( clientViewRect);
				
				trackRect = NULL == clientView
				         || clientView->floatRect.IsRectEmpty()
				         || placeFLOATFRAME == this->dragContext->sourcePlace
				          ? clientViewRect
				          : clientView->floatRect
				          ;
				
				// adjust float rect to hitPoint
				int offset = clientView->owner->tabsOnTop()
				           ? trackRect.top - hitPoint.y
				           : trackRect.bottom - hitPoint.y
				           ;
				trackRect.top    -= offset;
				trackRect.bottom -= offset;
				
				if ( clientViewRect.left <= hitPoint.x && hitPoint.x <= clientViewRect.right) {
					
					double coef = static_cast<double>( clientViewRect.left - hitPoint.x)
								/ static_cast<double>( hitPoint.x - clientViewRect.right)
								;
					coef  *= trackRect.Width() / (1 + coef);
					offset = trackRect.left - hitPoint.x + static_cast<int>( coef);
					
				} else/* if ( hitPoint.x <= trackRect.left) {
					
					offset = trackRect.left - hitPoint.x;
					
				} else */
					
					offset = trackRect.left - hitPoint.x;
					
				trackRect.left  -= offset;
				trackRect.right -= offset;
			}
			
			this->dragContext->setStartPoint( hitPoint);
			this->dragContext->setStartRect( trackRect);
			this->dragContext->stateSet = this->getPaneStateSet( splitPane);
			this->dragContext->clientViewWnd = clientViewWnd;
			
		} // void dragStart( HWND sourceWnd, HWND clientViewWnd)
		
		void dragOver( HWND sourceWnd, HWND clientViewWnd, long x, long y, DWORD keysPressed) {
			
			ATLASSERT( NULL != this->dragContext);
			ATLASSERT( ::IsWindow( sourceWnd));
			ATLASSERT( NULL == clientViewWnd || ::IsWindow( clientViewWnd));
			ATLASSERT( ::IsWindow( this->dragContext->sourcePaneWnd));
			
			CPoint hitPoint( x, y);
			HWND  hitWnd = NULL;
			CRect hitRect;
			SplitPane* splitPane;
			POSITION position;
			
			// check if the cursor over the source float frame
			if ( placeFLOATFRAME == this->dragContext->sourcePlace) {
				
				if ( this->dragContext->getStartRect().PtInRect( hitPoint))
					// just track it
					goto dragOver_exit;
			}
			
			// check if a user holds CTRL key
			if (    (MK_CONTROL & keysPressed) == 0
//			     || ( this->dragContext->stateSet.isFloating()
				 || ( this->dragContext->stateSet.dockable && this->dragContext->stateSet.null)
			   )
					goto dragOver_exit; // noway to drop with floating state or CTRL key unpressed
			
			// check if the cursor over a float frame
			FloatFrame* floatFrame = NULL;
			FloatFrame* floatFrameIndex = NULL;
			if ( NULL != ( position = this->floatFrames.GetStartPosition()))
				do {
					
					floatFrameIndex = this->floatFrames.GetNextValue( position);
					
					if ( floatFrameIndex->m_hWnd == dragContext->sourcePaneWnd)
						continue;
					
					splitPane = floatFrameIndex->getPane();
					if ( NULL == splitPane)
						continue;
					
					splitPane->GetClientRect( hitRect);
					splitPane->ClientToScreen( hitRect);
					if ( hitRect.PtInRect( hitPoint) || dragContext->sourcePaneWnd == floatFrameIndex->m_hWnd) {
						
						floatFrame = floatFrameIndex;
						break;
					}
				} while ( NULL != position);
			
			if ( NULL != floatFrame) {
				
				// A float frame is found. Check if there are more float frames on top of it
				HWND nextFloatWnd = floatFrame->m_hWnd;
				while ( NULL != ( nextFloatWnd = ::GetNextWindow( nextFloatWnd, GW_HWNDPREV))) {
					
					if ( NULL == ( floatFrameIndex = this->lookUpFloatFrame( nextFloatWnd)))
						continue;
					
					if ( floatFrameIndex->m_hWnd == dragContext->sourcePaneWnd)
						continue;
					
					splitPane = floatFrameIndex->getPane();
					if ( NULL == splitPane)
						continue;
					
					CRect rect;
					splitPane->GetClientRect( rect);
					splitPane->ClientToScreen( rect);
					if ( rect.PtInRect( hitPoint)) {
						floatFrame = floatFrameIndex;
						hitRect = rect;
					}
				}
				
				this->dragContext->targetPlace    = placeFLOATFRAME;
				this->dragContext->targetPaneWnd = floatFrame->m_hWnd;
				hitWnd = floatFrame->getPane()->m_hWnd;
				goto dragOver_exit;
			}
			
			// check if the cursor over a dock pane
			bool hitInsideDockPane = false;
			if ( this->dragContext->stateSet.isDockable()) {
				
				int i;
				DockPane* dockPane     = NULL;
				for ( i=0; i<4; i++) {
					
					dockPane = this->dockPanes[i];
					if ( NULL != dockPane) {
						
						if ( i == this->dragContext->sourceDock) {
							CRect rect;
							dockPane->GetWindowRect( rect);
							hitInsideDockPane = TRUE == rect.PtInRect( hitPoint);
						}
						
						splitPane = dockPane->getPane();
						if ( NULL == splitPane)
							continue;
						
						splitPane->GetClientRect( hitRect);
						splitPane->ClientToScreen( hitRect);
						if ( hitRect.PtInRect( hitPoint))
							break;
					}
				}
				
				if ( i < 4 && i != this->dragContext->sourceDock) {
					
					this->dragContext->targetPlace    = placeDOCKPANE;
					this->dragContext->targetDock    = static_cast< DockSide>( i);
					this->dragContext->targetPaneWnd = dockPane->m_hWnd;
					hitWnd = splitPane->m_hWnd;
					goto dragOver_exit;
				}
			}
			
			// check if the cursor over main pane dock frame
			if ( !hitInsideDockPane) {
				
				const int dockingEdgeSize = 12;
				
				this->GetWindowRect( hitRect);
				hitRect.InflateRect( dockingEdgeSize, dockingEdgeSize);
				if ( hitRect.PtInRect( hitPoint)) {
					hitRect.InflateRect( -dockingEdgeSize*2, -dockingEdgeSize*2);
					if (    !hitRect.PtInRect( hitPoint)
					     && hitRect.Height() > 2*dockingEdgeSize && hitRect.Width() > 2*dockingEdgeSize
					   ) {
						
						if ( !this->dragContext->stateSet.isDockable())
							goto dragOver_exit; // noway to dock with non dockable state
						
						this->dragContext->dockWidth = LayoutManager::coveredBoth;
						if        ( NULL == this->dockPanes[ dockTOP]    && abs( hitPoint.y - hitRect.top)    <= 2*dockingEdgeSize) {
							
							// calculate top dock pane rectangle
							if ( hitPoint.y > hitRect.top - dockingEdgeSize) {
								
								if ( this->dockPaneVisible( dockRIGHT))
									this->dragContext->dockWidth -= LayoutManager::coveredLeft;
							
								if ( this->dockPaneVisible( dockLEFT))
									this->dragContext->dockWidth -= LayoutManager::coveredRight;
							}
							if ( this->layout.calcDockRect( dockTOP, this->dragContext->dockWidth, this->layout.getDockSize( dockTOP), hitRect)) {
								
								hitWnd = this->m_hWnd;
								this->dragContext->targetDock = dockTOP;
								this->dragContext->dockSize = hitRect.Height();
							}
						} else if ( NULL == this->dockPanes[ dockBOTTOM] && abs( hitPoint.y - hitRect.bottom) <= 2*dockingEdgeSize) {
							
							// calculate bottom dock pane rectangle
							if ( hitPoint.y < hitRect.bottom + dockingEdgeSize) {
								
								if ( this->dockPaneVisible( dockRIGHT))
									this->dragContext->dockWidth -= LayoutManager::coveredRight;
							
								if ( this->dockPaneVisible( dockLEFT))
									this->dragContext->dockWidth -= LayoutManager::coveredLeft;
							}
							if ( this->layout.calcDockRect( dockBOTTOM, this->dragContext->dockWidth, this->layout.getDockSize( dockBOTTOM), hitRect)) {
								hitWnd = this->m_hWnd;
								this->dragContext->targetDock = dockBOTTOM;
								this->dragContext->dockSize = hitRect.Height();
							}
						} else if ( NULL == this->dockPanes[ dockLEFT]   && abs( hitPoint.x - hitRect.left)   <= 2*dockingEdgeSize) {
							
							// calculate left dock pane rectangle
							if ( hitPoint.x > hitRect.left - dockingEdgeSize) {
								
								if ( this->dockPaneVisible( dockTOP))
									this->dragContext->dockWidth -= LayoutManager::coveredLeft;
								
								if ( this->dockPaneVisible( dockBOTTOM))
									this->dragContext->dockWidth -= LayoutManager::coveredRight;
							}
							if ( this->layout.calcDockRect( dockLEFT, this->dragContext->dockWidth, this->layout.getDockSize( dockLEFT), hitRect)) {
								
								hitWnd = this->m_hWnd;
								this->dragContext->targetDock = dockLEFT;
								this->dragContext->dockSize = hitRect.Width();
							}
						} else if ( NULL == this->dockPanes[ dockRIGHT]  && abs( hitPoint.x - hitRect.right)  <= 2*dockingEdgeSize) {
							
							// calculate right dock pane rectangle
							if ( hitPoint.x < hitRect.right + dockingEdgeSize) {
								
								if ( this->dockPaneVisible( dockTOP))
									this->dragContext->dockWidth -= LayoutManager::coveredRight;
								
								if ( this->dockPaneVisible( dockBOTTOM))
									this->dragContext->dockWidth -= LayoutManager::coveredLeft;
							}
							if ( this->layout.calcDockRect( dockRIGHT, this->dragContext->dockWidth, this->layout.getDockSize( dockRIGHT), hitRect)) {
								
								hitWnd = this->m_hWnd;
								this->dragContext->targetDock = dockRIGHT;
								this->dragContext->dockSize = hitRect.Width();
							}
						}
						
						if ( NULL != hitWnd) {
							
							this->ClientToScreen( hitRect);
							this->dragContext->targetPlace = placeDOCKPANE;
							goto dragOver_exit;
						}
					}
				}
			}
			
			// check if the cursor over main pane
			if ( this->mainPane.getClientViewRect( &hitPoint, hitRect)) {
				
				if ( !this->dragContext->stateSet.isNull())
					goto dragOver_exit; // noway to drop with no null state to the Main Pane

				hitWnd = this->mainPane.m_hWnd;
				this->dragContext->targetPlace   = placeMAINPANE;
				this->dragContext->targetPaneWnd = hitWnd;
				goto dragOver_exit;
			}
			
dragOver_exit:
			if ( NULL != hitWnd) {
				
				// draw docking rectangle
				this->dragContext->clearTrackRectangle( false);
				this->dragContext->keepTrack( hitPoint);
				if ( !this->dragContext->getGhostRect().EqualRect( hitRect))
					this->dragContext->drawTrackRectangle( hitRect, hitWnd);
			} else {
				// anders:
				// test if the dragged clientView allows floating
				// if not, do not allow drop
				bool allowFloat=true;
//				FramePlace place=placeUNKNOWN;
				if (clientViewWnd) {
					ClientView* cv=lookUpClientView(clientViewWnd);
					allowFloat=cv->allowFloat;
				}

				if (allowFloat) {
					// continue tracking
					this->dragContext->targetDock    = dockUNKNOWN;
					this->dragContext->targetPaneWnd = NULL;
					this->dragContext->targetPlace   = NULL == clientViewWnd? placeUNKNOWN: placeFLOATFRAME;
					this->dragContext->drawTrackRectangle( hitPoint);
				} else {
					//if (clientViewWnd)
					//	this->dragContext->targetPlace   = placeUNKNOWN;
				}
			}
			return;
		} // void dragOver( HWND sourceWnd, HWND clientViewWnd, long x, long y)
		
		void dragDrop( HWND sourceWnd, HWND clientViewWnd, long x, long y, DWORD keysPressed) {
			
			ATLASSERT( NULL != sourceWnd);
			ATLASSERT( NULL != this->dragContext);
			
			if ( placeDOCKPANE == this->dragContext->sourcePlace) {
				
				// if a dock pane goes to nowhere let it float
				if ( placeUNKNOWN == this->dragContext->targetPlace)
					
					this->dragContext->targetPlace = placeFLOATFRAME;
				else if (    placeDOCKPANE == this->dragContext->targetPlace
				          && this->dockToFloatRect[ this->dragContext->targetDock].EqualRect( &rectNULL)
				        )
					// if a dock pane changes a dock side it has to keep a float rectangle
					this->dockToFloatRect[ this->dragContext->targetDock] = this->dockToFloatRect[ this->dragContext->sourceDock];
			}
			
			if ( placeUNKNOWN == this->dragContext->targetPlace && placeFLOATFRAME == this->dragContext->sourcePlace) {
				
				CRect rect = this->dragContext->getTrackRect();
				this->dragContext->clearTrackRectangle();
				CWindow sourceWindow( sourceWnd);
				sourceWindow.SetWindowPos( NULL, rect, SWP_NOZORDER);
			} else {
				
				// create a drop dropParcel
				DropParcel* dropParcel = new DropParcel();
				
				dropParcel->sourceDock    = this->dragContext->sourceDock;
				dropParcel->sourcePlace   = this->dragContext->sourcePlace;
				dropParcel->sourcePaneWnd = this->dragContext->sourcePaneWnd;
				dropParcel->clientViewWnd = clientViewWnd;
				dropParcel->targetPlace   = this->dragContext->targetPlace;
				dropParcel->targetDock    = this->dragContext->targetDock;
				dropParcel->targetPaneWnd = this->dragContext->targetPaneWnd;
				dropParcel->targetRect    = placeFLOATFRAME == dropParcel->targetPlace
				                          ? this->dragContext->getTrackRect()
				                          : this->dragContext->getGhostRect()
				                          ;
				dropParcel->dockSize      = this->dragContext->dockSize;
				dropParcel->dockWidth     = this->dragContext->dockWidth;
				
				if ( dropParcel->targetRect.EqualRect( &rectNULL))
					dropParcel->targetRect = this->dragContext->getTrackRect();
				
				if ( placeMAINPANE == dropParcel->targetPlace)
					this->mainPane.setFocusTo( x, y);
				
				// shoot the drop dropParcel
				this->postParcel( this->addDropParcel( dropParcel));
			}
			
			// tidy up
			delete this->dragContext;
			this->dragContext = NULL;
		} // void dragDrop( HWND floatFrameWnd, HWND clientViewWnd, long x, long y)
		
		void dragCancel( HWND sourceWnd, HWND clientViewWnd) {
			
			ATLASSERT( NULL != this->dragContext);
			
			// tidy up
			delete this->dragContext;
			this->dragContext = NULL;
		} // void dragCancel( HWND sourceWnd, HWND clientViewWnd)
		
		void clientActivate(  HWND childWnd, HWND clientViewWnd) {
			SplitPane* foundSplitPane = ( NULL != clientViewWnd)
			                     ? this->lookUpClientSplitPane( clientViewWnd)
								 : this->lookUpSplitPane( childWnd)
			                     ;
			if ( NULL == foundSplitPane)
				return;
			
			ClientView* clientView = this->lookUpClientView( clientViewWnd);
			
			bool activated = false;
			// make active/inactive dock panes title bars
			for ( int i=0; i<4; i++) {
				
				DockPane* dockPane = this->dockPanes[i];
				if ( NULL != dockPane) {
					if (dockPane->getPane() == foundSplitPane) {
						
						activated = true;
						if ( NULL != clientView)
							dockPane->setCaption( clientView->caption);
						
						dockPane->setActive(true);
					} else {
						dockPane->setActive(false);
					}
				}
			}
			if (!activated) {
				// set float frame title bar
				POSITION position = this->floatFrames.GetStartPosition();
				if ( NULL != position)
					do {
						
						FloatFrame* floatFrame = this->floatFrames.GetNextValue( position);
						if ( floatFrame->getPane() == foundSplitPane) {
							
							activated = true;
							if ( NULL != clientView)
								floatFrame->SetWindowText( clientView->caption);
							break;
						}
					} while ( NULL != position);
			}
			
			// make active/inactive Pane tab bars
			{
				POSITION position = this->splitPanes.GetStartPosition();
				while (position != NULL) {
					SplitPane* splitPane = this->splitPanes.GetNextValue(position);
					if (splitPane != foundSplitPane) {
						splitPane->setActive(false);
					}
				}
				foundSplitPane->setActive(true);
			}
			
			this->setCurrentPane( foundSplitPane);
			if (clientView != 0)
				cbListener->clientActivated(clientView);
			return;
		} // void clientActivate(  HWND childWnd, HWND clientViewWnd)
		
		void clientDblClick(  HWND childWnd, HWND clientViewWnd) {
			
			// Got double mouse click from tab item
			ATLASSERT( ::IsWindow( clientViewWnd));
			
			if ( this->mainPane.IsChild( clientViewWnd)) {
				
				DropParcel* dropParcel = new DropParcel();
				dropParcel->sourcePlace    = placeMAINPANE;
				dropParcel->sourcePaneWnd  = this->mainPane.m_hWnd;
				dropParcel->clientViewWnd      = clientViewWnd;
				dropParcel->targetPlace    = placeFLOATFRAME;
				this->postParcel( this->addDropParcel( dropParcel));
				return;
			}
				
			for ( int i=0; i<4; i++) {
				
				DockSide dockSide  = static_cast< DockSide>( i);
				DockPane* dockPane = this->dockPanes[ dockSide];
				if (  NULL == dockPane || dockPane->getPane()->m_hWnd != childWnd)
					continue;
				
				DropParcel* dropParcel    = new DropParcel();
				dropParcel->sourcePlace   = placeDOCKPANE;
				dropParcel->sourceDock    = dockSide;
				dropParcel->sourcePaneWnd = dockPane->m_hWnd;
				dropParcel->clientViewWnd     = clientViewWnd;
				dropParcel->targetPlace   = placeFLOATFRAME;
				this->postParcel( this->addDropParcel( dropParcel));
				return;
			}
			return;
		}
		
		void clientCloseClick(  HWND childWnd, HWND clientViewWnd) {

// anders was here and swapped order of post/send message. we had a too early invalidation of the wnd, os all IsWindow-calls fail lateron
// now we swapped back ...!!!
			if ( 0 == ::SendMessage( clientViewWnd, WM_CLOSE, 0, 0))
				this->PostMessage( eventCloseClientView, 0, reinterpret_cast<LPARAM>( clientViewWnd));
//			if ( 0 != ::PostMessage( clientViewWnd, WM_CLOSE, 0, 0))
//				this->SendMessage( eventCloseClientView, 0, reinterpret_cast<LPARAM>( clientViewWnd));
			return;
		}
		
		// Event handlers
		DECLARE_WND_CLASS( "DockSplitTab::Frame")
		
		BEGIN_MSG_MAP( Frame)
			MESSAGE_HANDLER( WM_SIZE,             OnSize)
			MESSAGE_HANDLER( WM_CREATE,           OnCreate)
			MESSAGE_HANDLER( WM_ERASEBKGND,       OnEraseBackground)
			MESSAGE_HANDLER( WM_SETFOCUS,         OnSetFocus)
			
			MESSAGE_HANDLER( eventDropParcel,	   OnDropParcel)
			MESSAGE_HANDLER( eventCloseFloatFrame, OnCloseFloatFrame)
			MESSAGE_HANDLER( eventCloseDockPane,   OnCloseDockPane)
			MESSAGE_HANDLER( eventCloseClientView, OnCloseClientView)
			
			FORWARD_NOTIFICATIONS()
		END_MSG_MAP()
		
		LRESULT OnCloseClientView( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
			HWND clientViewWnd = reinterpret_cast<HWND>( lParam);
			ClientView* clientView = this->lookUpClientView( clientViewWnd);
			if ( NULL != clientView) {
				
				this->detachClientView( clientViewWnd);
				this->cbListener->clientDetached( clientView);
			}
			return 0;
		}
		
		LRESULT OnCloseDockPane( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
			DockSide  dockSide = static_cast<DockSide>(wParam);
			DockPane* dockPane = this->dockPanes[ dockSide];
			if( NULL == dockPane)
				return 0;
			
			SplitPane* splitPane = dockPane->detachPane();
			if ( NULL != splitPane) {
				
				this->detachClientViews( splitPane, placeDOCKPANE, dockSide);
				delete splitPane;
			}
			if ( this->currentPane == splitPane)
				this->setCurrentPane();
			this->removeDockPane( dockSide);
			return 0;
		}
		
		LRESULT OnCloseFloatFrame( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
			HWND floatWnd = reinterpret_cast<HWND>( lParam);
			FloatFrame* floatFrame = this->lookUpFloatFrame( floatWnd);
			if ( NULL == floatFrame)
				return 0;
			
			SplitPane* splitPane = floatFrame->detachPane();
			if ( NULL != splitPane) {
				
				this->detachClientViews( splitPane, placeFLOATFRAME);
				delete splitPane;
			}
			if ( this->currentPane == splitPane)
				this->setCurrentPane();
			this->removeFloatFrame( floatFrame);
			return 0;
		}
		
		LRESULT OnSetFocus( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
			ATLASSERT( NULL != this->currentPane);
			
			// check if need to focus to the last pane if the current pane is not a dock pane or the main pane.
			SplitPane* currentPane = this->currentPane;
			if ( currentPane != &this->mainPane) {
				int i;
				for ( i=0; i<4; i++) {
					DockPane* dockPane = this->dockPanes[i];
					if ( NULL != dockPane && dockPane->getPane() == currentPane) {
						break;
					}
				}
				if ( i>=4)
                    currentPane = this->lastPane ? this->lastPane : &this->mainPane; // the only purpose why we need to keep the last pane - anders added NULL test
			}
			
			this->setCurrentPane( currentPane);
			
			// we don't want to set focus to the client window in float frames
			POSITION position = this->floatFrames.GetStartPosition();
			if ( NULL != position)
				do {
					FloatFrame* floatFrame = this->floatFrames.GetNextValue( position);
					if ( floatFrame->getPane() == currentPane)
						return 0; // go away ...
				} while ( NULL != position);
			
			// set focus to the focused client window in the current pane
			HWND clientViewWnd = currentPane->focusedClientView();
			::SetFocus( NULL == clientViewWnd? currentPane->m_hWnd: clientViewWnd);
			return 0;
		}
		
		LRESULT OnDropParcel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
//			using ClientView::stateFloating;
//			using ClientView::stateDockable;
			
			DockPane*   targetDockPane;
			DockPane*   sourceDockPane;
			FloatFrame* targetFloatFrame;
			FloatFrame* sourceFloatFrame;
			SplitPane*  splitPane;
			
			long parcelTag = static_cast<long>( wParam);
			DropParcel* dropParcel = this->getDropParcel( parcelTag);


			// anders tok bort assert siden denne slo inn på en måte jeg oppfattet som feil
			if (dropParcel->sourcePaneWnd == dropParcel->targetPaneWnd) {
				this->removeDropParcel( parcelTag);
                delete dropParcel;  // added by anders
				return 1;
			}

			// anders added: when we are not dropping to anywhere: f.eks for non-floating views
			if (dropParcel->targetPaneWnd==0 && dropParcel->targetPlace==placeUNKNOWN) {
				this->removeDropParcel( parcelTag);
                delete dropParcel;  // added by anders
				return 1;
			}
//			ATLASSERT( dropParcel->sourcePaneWnd != dropParcel->targetPaneWnd);
			
			if ( NULL != dropParcel->clientViewWnd) {
				
				ClientView* clientView = this->lookUpClientView( dropParcel->clientViewWnd);
				ATLASSERT( NULL != clientView);

                // anders added: are we allowed to float?
                if (!clientView->allowFloat && dropParcel->targetPlace == placeFLOATFRAME) {
    				this->removeDropParcel( parcelTag);
                    delete dropParcel;
                    return 1;
                }
				
				CWindow clientViewWnd( clientView->wnd);
				CRect clientViewRect;
				clientViewWnd.GetClientRect( clientViewRect);
				clientViewWnd.ClientToScreen( clientViewRect);
				SplitPane* sourceSplitPane = NULL;
				
				// detach client window from the source pane
				switch ( dropParcel->sourcePlace) {
				case placeMAINPANE:
					
					// so source pane is main pane
					this->mainPane.detachClientView( dropParcel->clientViewWnd);
					break;
					
				case placeDOCKPANE:
					
					sourceDockPane = this->dockPanes[ dropParcel->sourceDock];
					ATLASSERT( NULL != sourceDockPane);
					sourceDockPane->detachClientView( dropParcel->clientViewWnd);
					if ( 0 == sourceDockPane->getClientViewCount()) {
						
						::ShowWindow( dropParcel->clientViewWnd, FALSE);
						::SetParent( dropParcel->clientViewWnd, this->m_hWnd);
						
						SplitPane* splitPane = sourceDockPane->getPane();
						this->splitPanes.RemoveKey( splitPane->m_hWnd);
						if ( this->currentPane == splitPane)
							this->currentPane = this->lastPane == splitPane? &this->mainPane: this->lastPane;
						
						this->removeDockPane( dropParcel->sourceDock);
					}
					clientView->dockSide = dropParcel->sourceDock;
					break;
					
				case placeFLOATFRAME:

					sourceFloatFrame = this->lookUpFloatFrame( dropParcel->sourcePaneWnd);
					ATLASSERT( NULL != sourceFloatFrame);
					
					clientView->floatRect = clientViewRect;
					sourceFloatFrame->detachClientView( dropParcel->clientViewWnd);
					
					if ( 0 == sourceFloatFrame->getClientViewCount()) {
						
						::ShowWindow( dropParcel->clientViewWnd, FALSE);
						::SetParent( dropParcel->clientViewWnd, this->m_hWnd);
						
						SplitPane* splitPane = sourceFloatFrame->getPane();
						this->splitPanes.RemoveKey( splitPane->m_hWnd);
						if ( this->currentPane == splitPane)
							this->currentPane = this->lastPane == splitPane? &this->mainPane: this->lastPane;
						
						FloatFrame* floatFrame = this->lookUpFloatFrame( dropParcel->sourcePaneWnd);
						if ( NULL != floatFrame)
							this->removeFloatFrame( floatFrame);
					}
					
					break;
					
				default:
					
					// wrong dropParcel
					ATLASSERT( FALSE);
				}
				
				// append client window to the target pane
				if ( NULL != dropParcel->targetPaneWnd)
					::SetFocus( dropParcel->targetPaneWnd);
				
				switch ( dropParcel->targetPlace) {
				case placeMAINPANE:
					
					clientView->owner = &this->mainPane;
					this->mainPane.append( clientView->caption
					                     , clientView->wnd
					                     , clientView->toolTip
					                     , clientView->imageIndex
					                     );
					this->setCurrentPane( &this->mainPane);
					this->cbListener->clientChangedPlace(clientView, placeMAINPANE, dockUNKNOWN);
					break;
					
				case placeDOCKPANE:
					
					// target pane is dock pane
					targetDockPane = this->dockPanes[ dropParcel->targetDock];
					if ( NULL == targetDockPane)
						targetDockPane = this->createDockPane( dropParcel->targetDock
						                                     , dropParcel->dockWidth
						                                     , dropParcel->dockSize
						                                     );
					splitPane = targetDockPane->getPane();
					clientView->owner = splitPane;
					targetDockPane->append( clientView->caption
					                      , clientView->wnd
					                      , clientView->toolTip
					                      , clientView->imageIndex
					                      );
					clientView->dockSide = dropParcel->targetDock;
					this->setCurrentPane( splitPane);
					this->cbListener->clientChangedPlace(clientView, placeDOCKPANE, dropParcel->targetDock);
					break;
					
				case placeFLOATFRAME:
					// target pane is float pane
					targetFloatFrame = this->lookUpFloatFrame( dropParcel->targetPaneWnd);
					if ( NULL == targetFloatFrame) {
						
						CRect frameClientRect;
						if ( !dropParcel->targetRect.IsRectEmpty())
							clientView->floatRect = dropParcel->targetRect;
						else if ( clientView->floatRect.IsRectEmpty())
							clientView->floatRect = clientViewRect;
						
						frameClientRect = clientView->floatRect;

						DWORD dwStyle = FloatFrame::getFloatFrameStyle();
						DWORD dwStyleEx = FloatFrame::getFloatFrameStyleEx();
						AdjustWindowRectEx(&frameClientRect, dwStyle, FALSE, dwStyleEx);
						frameClientRect.bottom += 24; // tab height: TODO: should calculate tab height based on the font size calculations in TabControl::create()

						targetFloatFrame = this->createFloatFrame( frameClientRect);
					}
					splitPane = targetFloatFrame->getPane();
					clientView->owner = splitPane;
					targetFloatFrame->append( clientView->caption
					                        , clientView->wnd
					                        , clientView->toolTip
					                        , clientView->imageIndex
					                        );
					this->setCurrentPane( splitPane);
					this->cbListener->clientChangedPlace(clientView, placeFLOATFRAME, dockUNKNOWN);
					targetFloatFrame->UpdateLayout(FALSE); // anders moved from FloatFrame::attachPane to prevent premature redrawing
					break;
					
				}
				
				this->currentPane->setFocusTo( clientView->wnd);
				
			} else {
				
				CRect splitPaneWindowRect = rectNULL;
				
				bool floatingViews = false;
				POSITION position;
				switch ( dropParcel->sourcePlace) {
				case placeDOCKPANE:
					
					// source base is a dock pane. Detach a split pane and remove it
					sourceDockPane = this->dockPanes[ dropParcel->sourceDock];
					ATLASSERT( NULL != sourceDockPane);
					
					splitPane = sourceDockPane->detachPane();
					splitPane->GetClientRect( splitPaneWindowRect);
					splitPane->ClientToScreen( splitPaneWindowRect);
					
					// save dock side for each client window
					if ( NULL != ( position = this->clientViews.GetStartPosition()))
						do {
							
							ClientView* clientView = this->clientViews.GetNextValue( position);
							if ( clientView->owner == splitPane)
								clientView->dockSide = dropParcel->sourceDock;
							if ( ClientView::stateFloating == clientView->state) {
								// there must not be a client view in a dock pane with floating state
								ATLASSERT( TRUE);
							}
						} while ( NULL != position);
					
					splitPane->ShowWindow( SW_HIDE);
					splitPane->SetParent( this->m_hWnd);
					this->removeDockPane( dropParcel->sourceDock);
					break;
					
				case placeFLOATFRAME:
					
					// source base is a float frame. Detach a split pane and remove it
					sourceFloatFrame = this->lookUpFloatFrame( dropParcel->sourcePaneWnd);
					
					ATLASSERT( NULL != sourceFloatFrame);
					
					splitPane = sourceFloatFrame->getPane();
					splitPane->GetClientRect( splitPaneWindowRect);
					splitPane->ClientToScreen( splitPaneWindowRect);
					
					// save float screen rectangle for each client window
					if ( NULL != ( position = this->clientViews.GetStartPosition())) {
						
						do {
							
							ClientView* clientView = this->clientViews.GetNextValue( position);
							if ( clientView->owner == splitPane) {
								
								if ( ClientView::stateFloating == clientView->state)
									floatingViews = true;
								else {
									
									::GetClientRect( clientView->wnd, clientView->floatRect);
									CWindow( clientView->wnd).ClientToScreen( clientView->floatRect);
								}
							}
						} while ( NULL != position);
						
						if ( !floatingViews || placeFLOATFRAME == dropParcel->targetPlace) {
							
							// if there is no client views with floating state then the float frame is going to be destroyed
							sourceFloatFrame->detachPane();
							splitPane->ShowWindow( SW_HIDE);
							splitPane->SetParent( this->m_hWnd);
							this->removeFloatFrame( sourceFloatFrame);
						}
					}
					break;
					
				default:
					
					// wrong dropParcel
					ATLASSERT( FALSE);
					splitPane = NULL;
				}
				
				// append client windows to the source pane
				HWND clientViewWnd = splitPane->focusedClientView();
				if ( NULL != dropParcel->targetPaneWnd)
					::SetFocus( dropParcel->targetPaneWnd);
				
				switch ( dropParcel->targetPlace) {
				case placeMAINPANE:
					
					// target base is main pane. move all client windows from the split pane to there
					if ( NULL != ( position = this->clientViews.GetStartPosition()))
						do {
							
							ClientView* clientView = this->clientViews.GetNextValue( position);
							if ( clientView->owner == splitPane) {
								
								splitPane->detachClientView( clientView->wnd);
								this->mainPane.append( clientView->caption
								                     , clientView->wnd
								                     , clientView->toolTip
								                     , clientView->imageIndex
								                     );
								clientView->owner = &this->mainPane;
								clientViewWnd = clientView->wnd;
								this->cbListener->clientChangedPlace(clientView, placeMAINPANE, dockUNKNOWN);
							}
						} while ( NULL != position);
					
					this->splitPanes.RemoveKey( splitPane->m_hWnd);
					if ( this->currentPane == splitPane)
						this->currentPane = this->lastPane == splitPane? &this->mainPane: this->lastPane;
					delete splitPane;
					break;
					
				case placeDOCKPANE:
					
					// target base is a dock pane. so attach the split pane to an existing or a new one
					ATLASSERT( dropParcel->targetDock != dropParcel->sourceDock);
					
					if ( placeFLOATFRAME == dropParcel->sourcePlace) 
						
						// Store float rectangle for the undocking case
						this->dockToFloatRect[ dropParcel->targetDock] = splitPaneWindowRect;
					
					targetDockPane = this->dockPanes[ dropParcel->targetDock];
					if ( NULL == targetDockPane) {
						
						// create a new dock pane and attach the split pane to it
						targetDockPane = this->createDockPane( dropParcel->targetDock
						                                     , dropParcel->dockWidth
						                                     , dropParcel->dockSize
						                                     , splitPane
						                                     );

						// notify all clientviews belonging to this split pane they place just changed place
						if ( NULL != ( position = this->clientViews.GetStartPosition()))
							do {
								ClientView* clientView = this->clientViews.GetNextValue( position);
								if ( clientView->owner == splitPane)
									this->cbListener->clientChangedPlace(clientView, placeDOCKPANE, dropParcel->targetDock);
							} while ( NULL != position);
					} else {
						
						// move all client windows from the split pane to an existing dock pane
						if ( NULL != ( position = this->clientViews.GetStartPosition()))
							do {
								
								ClientView* clientView = this->clientViews.GetNextValue( position);
								if ( clientView->owner == splitPane) {
									
									splitPane->detachClientView( clientView->wnd);
									targetDockPane->append( clientView->caption
									                      , clientView->wnd
									                      , clientView->toolTip
									                      , clientView->imageIndex
									                      );
									clientView->owner = targetDockPane->getPane();
									clientViewWnd = clientView->wnd;
									this->cbListener->clientChangedPlace(clientView, placeDOCKPANE, dropParcel->targetDock);
								}
							} while ( NULL != position);
						this->splitPanes.RemoveKey( splitPane->m_hWnd);
						if ( this->currentPane == splitPane)
							this->currentPane = this->lastPane == splitPane? &this->mainPane: this->lastPane;
						delete splitPane;
					}
					this->setCurrentPane( targetDockPane->getPane());
					targetDockPane->setFocusTo( clientViewWnd);
					break;
					
				case placeFLOATFRAME:
				{
					
					targetFloatFrame = this->lookUpFloatFrame( dropParcel->targetPaneWnd);
					if ( NULL == targetFloatFrame) {
						
						// calculate a size of new float frame as a float pane size union of all client windows
						CRect floatFrameRect;
						if ( dropParcel->targetRect.IsRectEmpty()) {
							
							if ( placeDOCKPANE == dropParcel->sourcePlace)
								floatFrameRect = this->dockToFloatRect[ dropParcel->sourceDock];
							
							if ( floatFrameRect == rectNULL)
								floatFrameRect = splitPaneWindowRect;
							
							//floatFrameRect.InflateRect( 4, 12); // TODO calculate more accuratelly
						} else
							floatFrameRect = dropParcel->targetRect;
						
						// create a new float pane and attach the split pane to it
						targetFloatFrame = this->createFloatFrame( floatFrameRect, splitPane);

						// notify all clientviews belonging to this split pane they place just changed place
						if ( NULL != ( position = this->clientViews.GetStartPosition()))
							do {
								ClientView* clientView = this->clientViews.GetNextValue( position);
								if ( clientView->owner == targetFloatFrame->getPane())
									this->cbListener->clientChangedPlace(clientView, placeFLOATFRAME, dockUNKNOWN);
							} while ( NULL != position);
					} else {
						
						// move all client windows from the split pane to existed float pane
						if ( NULL != ( position = this->clientViews.GetStartPosition()))
							do {
								
								ClientView* clientView = this->clientViews.GetNextValue( position);
								if ( clientView->owner == splitPane) {
									
									splitPane->detachClientView( clientView->wnd);
									targetFloatFrame->append( clientView->caption
									                        , clientView->wnd
									                        , clientView->toolTip
									                        , clientView->imageIndex
									                        );
									clientView->owner = targetFloatFrame->getPane();
									this->cbListener->clientChangedPlace(clientView, placeFLOATFRAME, dockUNKNOWN);
								}
							} while ( NULL != position);
						
						// tidy up
						this->splitPanes.RemoveKey( splitPane->m_hWnd);
						if ( this->currentPane == splitPane)
							this->currentPane = this->lastPane == splitPane? &this->mainPane: this->lastPane;
					}
					targetFloatFrame->setFocusTo( clientViewWnd);
					this->setCurrentPane( targetFloatFrame->getPane());
					break;
				}
				case placeUNKNOWN:
					
					// scatter client windows from float pane to dock panes and main pane
					CRect floatRect;
					if ( placeFLOATFRAME == dropParcel->sourcePlace)
						floatRect = splitPaneWindowRect;
					
					SplitPane* currentPane = NULL;
					if ( NULL != ( position = this->clientViews.GetStartPosition())) {
						
						bool floatingViews = false;
						do {
							
							ClientView* clientView = this->clientViews.GetNextValue( position);
							if ( clientView->owner == splitPane) {
								
								if ( ClientView::stateFloating == clientView->state)
									floatingViews = true;
								else {
									
									splitPane->detachClientView( clientView->wnd);
									if ( dockUNKNOWN != clientView->dockSide && ClientView::stateDockable == clientView->state) {
										
										SplitPane* splitPane;
										
										targetDockPane = this->dockPanes[ clientView->dockSide];
										if ( placeFLOATFRAME == dropParcel->sourcePlace)
											this->dockToFloatRect[ clientView->dockSide] = floatRect;
										
										if ( NULL == targetDockPane) {
											
											// create a dock pane
											int dockSize;
											int dockWidth;
											if ( 0 == dropParcel->dockSize) {
												dockSize  = this->layout.getDockSize( clientView->dockSide);
												dockWidth = this->layout.getDockWidth( clientView->dockSide);
												if ( 0 == dockSize)
													this->layout.calcDockSize( clientView->dockSide, dockWidth, dockSize);
											} else {
												dockSize  = dropParcel->dockSize;
												dockWidth = dropParcel->dockWidth;
											}
											targetDockPane = this->createDockPane( clientView->dockSide
																				, dockWidth
																				, dockSize
																				);
										}
										splitPane = targetDockPane->getPane();
										
										ATLASSERT( ::IsWindow( clientView->wnd));
										targetDockPane->append( clientView->caption
															, clientView->wnd
															, clientView->toolTip
															, clientView->imageIndex
															);
										clientView->owner = splitPane;
										if ( clientViewWnd == clientView->wnd)
											currentPane = splitPane;
										splitPane->setFocusTo( clientView->wnd);
									} else {
										
										ATLASSERT( ::IsWindow( clientView->wnd));
										this->mainPane.append( clientView->caption
															, clientView->wnd
															, clientView->toolTip
															, clientView->imageIndex
															);
										clientView->owner = &this->mainPane;
										if ( clientViewWnd == clientView->wnd)
											currentPane = &this->mainPane;
										this->mainPane.setFocusTo( clientView->wnd);
									}
								}
							}
						} while ( NULL != position);
						
						if ( !floatingViews) {
							
							if ( NULL != currentPane)
								this->setCurrentPane( currentPane);
							
							// tidy up
							this->splitPanes.RemoveKey( splitPane->m_hWnd);
							if ( this->currentPane == splitPane)
								this->currentPane = this->lastPane == splitPane? &this->mainPane: this->lastPane;
							delete splitPane;
							this->updateLayout();
						}
					}
					break;
					
				}
			}
			
			
			// tidy up
			this->removeDropParcel( parcelTag);
            delete dropParcel;  // added by anders
			return 1;
		} // LRESULT OnDropParcel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
		
		LRESULT OnCreate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
			this->mainPane.create( this->m_hWnd);
			this->currentPane = this->splitPanes[ this->mainPane.m_hWnd] = &this->mainPane;
			return 1;
		}
		
		LRESULT OnSize( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
			
			this->layout.resize( LOWORD( lParam), HIWORD( lParam));
			this->updateLayout();
			return 1;
		}
		
		LRESULT OnEraseBackground( UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
			
			return 1;
		}
	};

	
}; // namespace DockSplitTab

#endif // __DOCKTAB_FRAME_H__
