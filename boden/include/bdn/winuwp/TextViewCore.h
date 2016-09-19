#ifndef BDN_WINUWP_TextViewCore_H_
#define BDN_WINUWP_TextViewCore_H_

#include <bdn/TextView.h>
#include <bdn/winuwp/ChildViewCore.h>

namespace bdn
{
namespace winuwp
{

class TextViewCore : public ChildViewCore, BDN_IMPLEMENTS ITextViewCore
{
private:
	static ::Windows::UI::Xaml::Controls::TextBlock^ _createTextBlock(TextView* pOuter)
	{
        BDN_WINUWP_TO_STDEXC_BEGIN;

		::Windows::UI::Xaml::Controls::TextBlock^ pTextBlock = ref new ::Windows::UI::Xaml::Controls::TextBlock();		

		pTextBlock->TextWrapping = ::Windows::UI::Xaml::TextWrapping::WrapWholeWords;
		pTextBlock->TextTrimming = ::Windows::UI::Xaml::TextTrimming::None;

		return pTextBlock;

        BDN_WINUWP_TO_STDEXC_END;
	}

public:

	TextViewCore(	TextView* pOuter)
		: ChildViewCore(pOuter, _createTextBlock(pOuter), ref new ViewCoreEventForwarder(this) )
	{
        BDN_WINUWP_TO_STDEXC_BEGIN;

		_pTextBlock = dynamic_cast< ::Windows::UI::Xaml::Controls::TextBlock^ >( getFrameworkElement() );

		setPadding( pOuter->padding() );
		setText( pOuter->text() );

        BDN_WINUWP_TO_STDEXC_END;
	}


	void setPadding(const Nullable<UiMargin>& pad) override
	{
        BDN_WINUWP_TO_STDEXC_BEGIN;

		// Apply the padding to the control, so that the content is positioned accordingly.
        UiMargin uiPadding;
        if(!pad.isNull())
            uiPadding = pad;

		Margin padding = UiProvider::get().uiMarginToDipMargin(uiPadding);

		_doSizingInfoUpdateOnNextLayout = true;		

		// UWP also uses DIPs => no conversion necessary

		_pTextBlock->Padding = ::Windows::UI::Xaml::Thickness(
			padding.left,
			padding.top,
			padding.right,
			padding.bottom );
        
        BDN_WINUWP_TO_STDEXC_END;
	}

	void setText(const String& text)
	{
        BDN_WINUWP_TO_STDEXC_BEGIN;

        // we cannot simply schedule a sizing info update here. The desired size of the control will still
		// be outdated when the sizing happens.
		// Instead we wait for the "layout updated" event that will happen soon after we set the
		// content. That is when we update our sizing info.
        _doSizingInfoUpdateOnNextLayout = true;		
        
		_pTextBlock->Text = ref new ::Platform::String( text.asWidePtr() );

        BDN_WINUWP_TO_STDEXC_END;
	}
	

protected:


	bool canAdjustWidthToAvailableSpace() const override
	{
		// text views can adjust the text wrapping to reduce their width.
		return true;
	}
	

	void _layoutUpdated() override
	{
		if(_doSizingInfoUpdateOnNextLayout)
		{
			_doSizingInfoUpdateOnNextLayout = false;

            P<View> pOuterView = getOuterViewIfStillAttached();
            if(pOuterView!=nullptr)
			    pOuterView->needSizingInfoUpdate();
		}
	}
    
	::Windows::UI::Xaml::Controls::TextBlock^ _pTextBlock;

	double      _doSizingInfoUpdateOnNextLayout = true;
	
};

}
}

#endif
