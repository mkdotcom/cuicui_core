#pragma once

#include "../cuicui.h"


// is it normal that it's in Nws ?

// WARNING: IOS only ! 
// WARNING: DOESN'T WORK :)

namespace cui {
namespace native
{
	class wActivityIndicator : public wEmpty
	{
    public: //protected:
#ifdef CUICUI_IOS
        UIActivityIndicatorView *uiActivityIndicatorView;
#endif

	public:
		wActivityIndicator(const cColor &color);

		~wActivityIndicator()
		{
		}

	public: //protected:
		virtual bool ComputeMinSize(double *w, double *h);		

	};
}


} // namespace cui