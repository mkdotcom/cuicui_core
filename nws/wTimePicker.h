#pragma once

#include "../cuicui.h"


// is it normal that it's in Nws ? rather yes (it's not a real "native cuicui" that the user must never touch)

#ifdef CUICUI_IOS
namespace cui {
namespace native {
    class wTimePicker;
}
} // namespace cui
@interface cIosDelegate : UIView
-(void)setP:(cui::native::wCalendar*)a_ptr;
@end
#endif

namespace cui {
namespace native
{
    // Note: user can select hours and minutes only

	class wTimePicker : public wEmpty
	{
	public: //protected:
#ifdef CUICUI_ANDROID
#endif

	public:
#ifdef CUICUI_UWP									
		TimePicker ^cp;
#endif
#ifdef CUICUI_IOS
        UIDatePicker *uiDatePicker;
        cIosDelegate *iosDelegate;
#endif

	public:
		wTimePicker(int hours, int minutes);

		~wTimePicker()
		{
		}

	public:
		wTimePicker* OnChanged(std::function<void(int hours, int minutes)> a_onChanged)
		{
			onChanged = a_onChanged;
			return this;
		}

	public: //because of android it can't be protected:
		std::function<void(int hours, int minutes)> onChanged = nullptr;

    public:

	public: //protected:
		virtual bool ComputeMinSize(double *w, double *h) override;


	};
}


} // namespace cui