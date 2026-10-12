#include "wActivityIndicator.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)


namespace cui {
native::wActivityIndicator::wActivityIndicator(const cColor &color) : wEmpty()
{
    this->Xem(min());
	this->Yem(min());
	this->LockXem();
	this->LockYem();
	DebugName("wActivityIndicator");

#if defined(CUICUI_ANDROID)
    cui_assert(false); // NYI
#elif defined(CUICUI_UWP)
    cui_assert(false); // NYI
#elif defined(CUICUI_WEB)
    cui_assert(false); // NYI
#elif defined(CUICUI_IOS)
    uiActivityIndicatorView = [[UIActivityIndicatorView alloc] initWithFrame:CGRectZero];
    uiView = (CuiUIView*)uiActivityIndicatorView;
    uiActivityIndicatorView.hidden = false;
    //[uiActivityIndicatorView setActivityIndicatorViewStyle:UIActivityIndicatorViewStyleLarge]; // ios 13 only I think
    [uiActivityIndicatorView setColor:color.ToUIColor()];
    [uiActivityIndicatorView startAnimating];
#elif defined(CUICUI_MACOS)
    cui_assert(false); // NYI
#else
    cui_assert(false); // NYI
#endif
}


bool native::wActivityIndicator::ComputeMinSize(double * w, double * h)
{	
	ComputeNativeWidgetMinSize(this, w, h, false); // would it work for this control?
    
	// TODO: I think it can have any size we want, no? we could forbid min

	return true;
}

} // namespace cui