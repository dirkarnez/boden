#include <bdn/init.h>
#include <bdn/win32/ViewCore.h>

#include <bdn/win32/Win32UiProvider.h>

namespace bdn
{
namespace win32
{

ViewCore::ViewCore(	View* pOuterView,
					const String& className,
					const String& name,
					DWORD style,
					DWORD exStyle,
					int x,
					int y,
					int width,
					int height )
	: Win32Window(	className,
					name,
					style,
					exStyle,
					ViewCore::getViewHwnd( pOuterView->getParentView() ),
					x,
					y,
					width,
					height )
{
	_pOuterViewWeak = pOuterView;

	_uiScaleFactor = 0;

	View* pParentView = pOuterView->getParentView();
	if(pParentView==nullptr)
	{
		// we are a top level window without a parent. In that case the constructor
		// of the derived class will initialize the ui scale factor.
	}
	else
	{
		// we inherit the scale factor from our parent view

		P<ViewCore> pParentCore = cast<ViewCore>(pParentView->getViewCore());

		setUiScaleFactor( pParentCore->getUiScaleFactor() );
	}
}



void ViewCore::setUiScaleFactor(double factor)
{
	if(factor != _uiScaleFactor)
	{
		_uiScaleFactor = factor;

		// we must now update our font
		updateFont();

		// and we must also notify our child views.
		std::list< P<View> > childViews;
		_pOuterViewWeak->getChildViews(childViews);

		for( const P<View>& pChildView: childViews)
		{
			P<ViewCore> pCore = cast<ViewCore>( pChildView->getViewCore() );
			if(pCore!=nullptr)
				pCore->setUiScaleFactor(factor);
		}
	}	
}


void ViewCore::updateFont()
{
	_pFont = Win32UiProvider::get()->getFontForView( _pOuterViewWeak, _uiScaleFactor);

	::SendMessage( getHwnd(), WM_SETFONT, (WPARAM)_pFont->getHandle(), FALSE);
}


void	ViewCore::setVisible(const bool& visible)
{
	::ShowWindow( getHwnd(), visible ? SW_SHOW : SW_HIDE);		
}


void ViewCore::setMargin(const UiMargin& margin)
{
	// do nothing. It only influences the parent layout
}

void ViewCore::setPadding(const UiMargin& padding)
{
	// do nothing. We handle it on the fly when our preferred size is calculated.
}

void ViewCore::setBounds(const Rect& bounds)
{
	::SetWindowPos( getHwnd(), NULL, (int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, SWP_NOACTIVATE | SWP_NOZORDER);
}


void ViewCore::updateOrderAmongSiblings()
{
	HWND ourHwnd = getHwnd();
	if(ourHwnd!=NULL)
	{
		View* pParentView = _pOuterViewWeak->getParentView();

		if(pParentView!=nullptr)
		{		
			View* pPrevSibling = pParentView->findPreviousChildView( _pOuterViewWeak );

			if(pPrevSibling==nullptr)
			{
				// we are the first child
				::SetWindowPos(ourHwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
			else
			{
				HWND prevSiblingHwnd = getViewHwnd(pPrevSibling);
						
				if(prevSiblingHwnd!=NULL)
					::SetWindowPos(ourHwnd, prevSiblingHwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}			
		}
	}
}

bool ViewCore::tryChangeParentView(View* pNewParentView)
{
	HWND ourHwnd = getHwnd();
	if(ourHwnd==NULL)
	{
		// Our win32 window is already destroyed. Return false.
		return false;
	}
	else
	{
		HWND currentParentHwnd = ::GetParent( ourHwnd );

		if( getViewHwnd(pNewParentView)==currentParentHwnd )
		{
			// the underlying win32 window is the same.
			// Note that this case is quite common, since many View containers
			// are actually light objects that do not have their own backend
			// Win32 window. So often the Win32 window of some higher ancestor is
			// shared by many windows.

			// This also happens when a child view's ordering position is changed inside
			// the parent. So we have to check if we need to change something there.
			updateOrderAmongSiblings();

			return true;
		}
		else
		{
			// we cannot move a window to a new underlying win32 window.
			return false;
		}
	}
}


	
void ViewCore::handleParentMessage(MessageContext& context, HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	// do nothing by default.
}


void ViewCore::handleMessage(MessageContext& context, HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	ViewCore* pChildCore = findChildCoreForMessage(message, wParam, lParam);
	if(pChildCore!=nullptr)
		pChildCore->handleParentMessage(context, windowHandle, message, wParam, lParam);

	if(message==WM_SIZE)
	{
		// whenever our size changes it means that we have to update our layout
		_pOuterViewWeak->needLayout();
	}
	
	Win32Window::handleMessage(context, windowHandle, message, wParam, lParam);
}


P<ViewCore> ViewCore::findChildCoreForMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND childHwnd = NULL;

	if(message==WM_COMMAND)
		childHwnd = (HWND)lParam;

	if(childHwnd!=NULL)
		return cast<ViewCore>( getObjectFromHwnd(childHwnd) );
	else	
		return nullptr;
}


int ViewCore::uiLengthToPixels(const UiLength& uiLength) const
{
	return Win32UiProvider::get()->uiLengthToPixels( uiLength, _uiScaleFactor );
}

Margin ViewCore::uiMarginToPixelMargin(const UiMargin& margin) const
{
	return Win32UiProvider::get()->uiMarginToPixelMargin( margin, _uiScaleFactor );
}


}
}

