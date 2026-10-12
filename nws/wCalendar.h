#pragma once

#include "../cuicui.h"


// is it normal that it's in Nws ? rather yes (it's not a real "native cuicui" that the user must never touch)

#ifdef CUICUI_IOS
namespace cui {
namespace native {
    class wCalendar;
}
} // namespace cui
@interface cIosDelegate : UIView
-(void)setP:(cui::native::wCalendar*)a_ptr;
@end
#endif

namespace cui {
namespace native
{
    // note: "today" is selected by default on all platforms
	class wCalendar : public wEmpty
	{
    public: //protected:
        std::chrono::system_clock::time_point tp;
#ifdef CUICUI_UWP									
		CalendarView ^cp;
#endif
#ifdef CUICUI_IOS
        UIDatePicker *uiDatePicker;
        cIosDelegate *iosDelegate;
#endif

	public:
		wCalendar();

		~wCalendar()
		{
		}


	public:
		std::function<void()> onChanged = nullptr;

    public:
        std::chrono::system_clock::time_point GetSelectedDate();
        
	public: //protected:
		virtual bool ComputeMinSize(double *w, double *h);		

	public:
		static wCalendar *Create() { auto c = new wCalendar();  return c; }
	};
}


} // namespace cui