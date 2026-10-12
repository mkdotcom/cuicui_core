#include "wCalendar.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)

#include "../3rdParty/date.h" 


#ifdef CUICUI_UWP
// source : https://cyberspock.com/2015/10/02/some-time_point-to-from-filetime-conversions/
namespace cui {
std::chrono::system_clock::time_point FileTime2TimePoint(const FILETIME& ft)
{
	SYSTEMTIME st = { 0 };
	if (!FileTimeToSystemTime(&ft, &st)) {
		return std::chrono::system_clock::time_point((std::chrono::system_clock::time_point::min)());
	}

	// number of seconds 
	ULARGE_INTEGER ull;
	ull.LowPart = ft.dwLowDateTime;
	ull.HighPart = ft.dwHighDateTime;

	time_t secs = ull.QuadPart / 10000000ULL - 11644473600ULL;
	std::chrono::milliseconds ms((ull.QuadPart / 10000ULL) % 1000);

	auto tp = std::chrono::system_clock::from_time_t(secs);
	tp += ms;
	return tp;
}
} // namespace cui
#endif

#ifdef CUICUI_ANDROID
namespace cui {
extern "C"
void
JNICALL
cuijni_wDatePickerOnDateChanged(
	JNIEnv *env,
	jobject obj/* this */,
	jlong widgetPtr,
	int y,
	int m,
	int d)
{
	wWidget *_w = (wWidget*)widgetPtr; 
	if (!CheckDefWidgetAlive(_w)) return;

	native::wCalendar *wi = (native::wCalendar*)_w;
	std::chrono::system_clock::time_point tp =
		date::sys_days(date::year(y)
			/ date::month(m + 1)
			/ date::day(d));

	wi->tp = tp;	
	if (wi->onChanged != nullptr) wi->onChanged();
}
} // namespace cui
#endif

#ifdef CUICUI_WEB
namespace cui {
extern "C" {
	void EMSCRIPTEN_KEEPALIVE wCalendarWebOnChanged(int wCalendarPtr, char *result)
	{
		std::string newText(result);

		native::wCalendar *ti = (native::wCalendar*)wCalendarPtr;
		if (!CheckDefWidgetAlive(ti)) return;
		Log(newText);
		if (newText.size() != 10)
		{
			std::chrono::system_clock::time_point tp;
			if (ti->onChanged != nullptr) ti->onChanged();
		}
		std::string year = newText.substr(0, 4);
		std::string month = newText.substr(5, 2);
		std::string day = newText.substr(8, 2);
		int y = std::atoi(year.c_str());
		int m = std::atoi(month.c_str());
		int d = std::atoi(day.c_str());

		ti->tp = date::sys_days(date::year(y)
				/ date::month(m)
				/ date::day(d));
		if (ti->onChanged != nullptr) ti->onChanged();
	}
}
} // namespace cui
#endif

#ifdef CUICUI_IOS
@implementation cIosDelegate {
    native::wCalendar *ptr;
}
-(void)setP:(native::wCalendar*)a_ptr
{
    ptr=a_ptr;
}
- (void)dateIsChanged:(id)sender{    
    NSDate *nsd = ptr->uiDatePicker.date;
    
    NSDateComponents *components = [[NSCalendar currentCalendar] components:NSDayCalendarUnit | NSMonthCalendarUnit | NSYearCalendarUnit fromDate : nsd];
    
    int year = [components year];
    int month = [components month];
    int day = [components day];
    
    ptr->tp =
    date::sys_days(date::year(year)
                   / date::month(month)
                   / date::day(day));
    if (ptr->onChanged!=nullptr) ptr->onChanged();
}
@end
#endif

namespace cui {
std::chrono::system_clock::time_point native::wCalendar::GetSelectedDate()
{
    return tp;
}


native::wCalendar::wCalendar() : wEmpty()
{
	this->Xem(MIN)->Yem(MIN);
	this->LockXem();
	this->LockYem();
	DebugName("wCalendar");

	tp = std::chrono::system_clock::now();

#if defined(CUICUI_ANDROID)
	JNICall_Int_Int("cuiDatePicker_CreateAndAddToElement", theId);
#elif defined(CUICUI_UWP)
	cp = ref new CalendarView();
	Windows::Globalization::Calendar^ c = ref new Windows::Globalization::Calendar;	
	c->SetToNow();
	cp->SelectedDates->Append(c->GetDateTime());
	cp->SelectedDatesChanged += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::CalendarView ^, Windows::UI::Xaml::Controls::CalendarViewSelectedDatesChangedEventArgs ^>([this](Windows::UI::Xaml::Controls::CalendarView ^sender, Windows::UI::Xaml::Controls::CalendarViewSelectedDatesChangedEventArgs ^args)
	{		
		auto s = cp->SelectedDates;
		if (s->Size == 0)
		{
			assert(false); // not handled
			std::chrono::system_clock::time_point tp;
			if (onChanged != nullptr) onChanged();
		}
		Windows::Foundation::DateTime dt = s->GetAt(0);
		tp = FileTime2TimePoint((const FILETIME&)dt.UniversalTime);

		if (onChanged != nullptr) onChanged();
	});

	b = cp;
	

#elif defined(CUICUI_WEB)	
	EM_ASM_(
		{
			var num = $0;
			var elem = getUnattachedElementsFromId(num.toString());
			if (elem == null)
			{
				Module.print("ERROR wCalendar 961 -> elem not found : " + num.toString());
				return;
			}

			elem.style["pointer-events"] = "auto";

			var x = document.createElement('INPUT');
			x.oninput = function() { Module.ccall('wCalendarWebOnChanged', 'void',['number', 'string'],[$1, this.value]); };			
			x.setAttribute("type", "date");		

			x.valueAsDate = new Date();

			elem.appendChild(x);
}, htmlId, this);
#elif defined(CUICUI_IOS)
    uiDatePicker = [[UIDatePicker alloc] initWithFrame:CGRectZero];
    [uiView addSubview:uiDatePicker];
    
    [uiDatePicker setDatePickerMode:UIDatePickerModeDate];
    
    iosDelegate = [[cIosDelegate alloc] init];
    [iosDelegate setP:this];   	
    
    [uiDatePicker addTarget:iosDelegate action:@selector(dateIsChanged:) forControlEvents:UIControlEventValueChanged];
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif
	

	
}


bool native::wCalendar::ComputeMinSize(double * w, double * h)
{	
#ifdef CUICUI_UWP
	//ComputeNativeWidgetMinSize(this, w, h); // doesn't work, when the app's window resize, the calendar also resizes and get very big and ugly

	// this is a pretty fine size:
	* w = GetEmSize(16);
	*h = GetEmSize(16);
#else
	ComputeNativeWidgetMinSize(this, w, h, false); // doesn't work, when the app's window resize, the calendar also resizes and get very big and ugly
#endif
	
	return true;
}

} // namespace cui