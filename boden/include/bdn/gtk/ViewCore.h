#ifndef BDN_GTK_ViewCore_H_
#define BDN_GTK_ViewCore_H_

#include <bdn/IViewCore.h>
#include <bdn/View.h>
#include <bdn/gtk/UiProvider.h>
#include <bdn/gtk/util.h>

#include <gtk/gtk.h>

namespace bdn
{
namespace gtk
{

class ViewCore : public Base, BDN_IMPLEMENTS IViewCore
{
public:
    ViewCore(View* pOuterView, GtkWidget* pWidget)
    {
        _outerViewWeak = pOuterView;
        
        _pWidget = pWidget;
        
        setVisible( pOuterView->visible() );
        
        _addToParent();        
    }
    
        
	void setVisible(const bool& visible) override
    {
        gtk_widget_set_visible(_pWidget, visible ? TRUE : FALSE);
    }
        
	
	
	void setPadding(const Nullable<UiMargin>& padding) override
    {
    }

	
	void setBounds(const Rect& bounds) override
    {
        GtkAllocation alloc = rectToGtkRect(bounds, getGtkScaleFactor() );
        
        if(alloc.width<0)
            alloc.width = 0;
        if(alloc.height<0)
            alloc.height = 0;
        
        gtk_widget_set_size_request( _pWidget, alloc.width, alloc.height );
        
        P<View> pView = getOuterViewIfStillAttached();
        if(pView!=nullptr)
        {
            P<View> pParentView = pView->getParentView();
            if(pParentView!=nullptr)
            {
                P<ViewCore> pParentViewCore = cast<ViewCore>( pParentView->getViewCore() );
            
                pParentViewCore->_moveChildViewCore( this, alloc.x, alloc.y );            
            }
        }
    }


	
	int uiLengthToPixels(const UiLength& uiLength) const override
    {
        return UiProvider::get().uiLengthToPixelsForWidget( getGtkWidget(), uiLength);
    }
	

	Margin uiMarginToPixelMargin(const UiMargin& margin) const override
    {
        return UiProvider::get().uiMarginToPixelMarginForWidget( getGtkWidget(), margin);
    }

	

	Size calcPreferredSize(int availableWidth=-1, int availableHeight=-1) const override
    {
		GtkRequisition minSize;
        GtkRequisition naturalSize;
        
        // we must clear our "size request". Otherwise the current size will be
        // the basis for the preferred size.
        gint oldWidth;
        gint oldHeight;
        gtk_widget_get_size_request( _pWidget, &oldWidth, &oldHeight);
        
        // invisible widgets do not take up any size in the layout.
        // So if it is 
        gboolean oldVisible = gtk_widget_get_visible(_pWidget);
        if(oldVisible==FALSE)
            gtk_widget_set_visible(_pWidget, TRUE);
            
                             
        gtk_widget_set_size_request( _pWidget, 0, 0);
        
        if(availableWidth!=-1)
        {
            gint minHeight=0;
            gint naturalHeight=0;
            
            int forGtkWidth = availableWidth / getGtkScaleFactor();
            
            if(forGtkWidth<0)
                forGtkWidth = 0;
            
            gtk_widget_get_preferred_height_for_width( _pWidget, forGtkWidth, &minHeight, &naturalHeight );
            
            naturalSize.width = forGtkWidth;
            naturalSize.height = naturalHeight;
        }
        else if(availableHeight!=-1)
        {
            gint minWidth=0;
            gint naturalWidth=0;
            
            int forGtkHeight = availableHeight / getGtkScaleFactor();
            
            if(forGtkHeight<0)
                forGtkHeight = 0;
                        
            gtk_widget_get_preferred_width_for_height( _pWidget, forGtkHeight, &minWidth, &naturalWidth );
            
            naturalSize.width = naturalWidth;
            naturalSize.height = forGtkHeight;
        }
        else        
            gtk_widget_get_preferred_size (_pWidget, &minSize, &naturalSize );
            
            
        // restore the old visibility
        if(oldVisible==FALSE)
            gtk_widget_set_visible(_pWidget, oldVisible);
        
        // restore the old size
        gtk_widget_set_size_request( _pWidget, oldWidth, oldHeight);        
        
        Size size = gtkSizeToSize(naturalSize, getGtkScaleFactor() );
        

        Margin padding = _getPaddingPixels();
        size += padding;
        
        return size;
    }
	
 
	bool tryChangeParentView(View* pNewParent) override
    {
        _addToParent();
        
        return true;
    }
    
    
    GtkWidget*  getGtkWidget() const
    {
        return _pWidget;
    }
    
    
    P<const View> getOuterViewIfStillAttached() const
    {
        return _outerViewWeak.toStrong();
    }
    
    P<View> getOuterViewIfStillAttached()
    {
        return _outerViewWeak.toStrong();
    }
    
    virtual void _addChildViewCore(ViewCore* pChildCore)
    {
        throw ProgrammingError("ViewCore::_addChildViewCore called on view that does not support child views.");
    }
    
    virtual void _moveChildViewCore(ViewCore* pChildCore, int gtkX, int gtkY)
    {
        throw ProgrammingError("ViewCore::_moveChildViewCore called on view that does not support child views.");
    }
        
        
    double getGtkScaleFactor() const
    {
        return gtk_widget_get_scale_factor(_pWidget);
    }
    
    
protected:    
    virtual Margin getDefaultPaddingPixels() const
    {
        return Margin();
    }
    

private:

    Margin _getPaddingPixels() const
    {
        P<View> pView = getOuterViewIfStillAttached();
        Nullable<UiMargin> pad;
        if(pView!=nullptr)
            pad = pView->padding();

        if(pad.isNull())
            return getDefaultPaddingPixels();
        else            
            return uiMarginToPixelMargin( pad.get() );
    }

    

    void _addToParent()
    {
        P<View> pView = getOuterViewIfStillAttached();
        P<View> pParent;
        if(pView!=nullptr)
            pParent = pView->getParentView();
        if(pParent==nullptr)
        {
            // nothing to do. We are a top level window.            
        }
        else
        {        
            P<ViewCore> pParentCore = cast<ViewCore>( pParent->getViewCore() );
            if(pParentCore==nullptr)
            {
                // should never happen
                throw ProgrammingError("Error: constructed a gtk::ViewCore for a view whose parent does not have a core.");
            }
            
            pParentCore->_addChildViewCore( this );            
        }
    }
    


    GtkWidget*  _pWidget;
    WeakP<View> _outerViewWeak;
};



}
}

#endif
