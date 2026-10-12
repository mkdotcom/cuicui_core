#include "wTimePicker.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)

#include "../3rdParty/date.h"

#ifdef CUICUI_ANDROID
namespace cui {
extern "C"
void
JNICALL
cuijni_wTimePickerOnDateChanged(
	JNIEnv *env,
	jobject obj/* this */,
	jlong widgetPtr,
	int h,
	int m)
{
	wWidget *_w = (wWidget*)widgetPtr; 
	if (!CheckDefWidgetAlive(_w)) return;
	native::wTimePicker *wi = (native::wTimePicker*)_w;
	if (wi->onChanged != nullptr) wi->onChanged(h,m);
}
} // namespace cui
#endif

#ifdef CUICUI_WEB
namespace cui {
extern "C" {
	// called by the <input type="time"> created in the constructor; value is "HH:MM" (24h), or "" when cleared
	void EMSCRIPTEN_KEEPALIVE wTimePickerWebOnChanged(int wTimePickerPtr, char *value)
	{
		native::wTimePicker *wi = (native::wTimePicker*)wTimePickerPtr;
		if (!CheckDefWidgetAlive(wi)) return;
		std::string v(value);
		if (v.size() < 5 || v[2] != ':') return; // empty or invalid: ignore
		int hours = std::atoi(v.substr(0, 2).c_str());
		int minutes = std::atoi(v.substr(3, 2).c_str());
		if (wi->onChanged != nullptr) wi->onChanged(hours, minutes);
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
native::wTimePicker::wTimePicker(int hours, int minutes) : wEmpty()
{
	//Native(); // useful? beware, it can disturb android's measure, retest carefully
	            // with or without it the measure is incorrect

	this->Xem(MIN)->Yem(MIN);
	this->LockXem();
	this->LockYem(); 
	DebugName("wTimePicker");

#if defined(CUICUI_ANDROID)
	JniCall cc(globalCuiJniEnv, "io/github/mkdotcom/cuicui/CuicuiActivity", "cuiTimePicker_CreateAndAddToElement");
	cc.i(theId);
	cc.i(hours);
	cc.i(minutes);
	cc.b(true); // 24 hours mode
	cc.CallIntMethod(globalCuiJniObj);
	cc.CleanUp();

	// NOTE_58715:
	// time picker on android can have variable width, we fix it here (as wTimePicker has fixed size on others platforms)
	// TODO: MAYBE we should make wTimePicker with variable width on all platforms? i think UWP works like that too, we could check web/ios
    LockXem(false);
    pixels w = pixels::CreateFromDouble( central.GetScreenWidthInEm() * (GetEmSize(1) * 0.65)); // 65% of screen width seems to be fine for everyone
    Xem(w);
    LockXem(true);

#elif defined(CUICUI_UWP)
	cp = ref new TimePicker();
	cp->SelectedTimeChanged += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::TimePicker^, Windows::UI::Xaml::Controls::TimePickerSelectedValueChangedEventArgs^>(
															   [this](Windows::UI::Xaml::Controls::TimePicker^ sender, Windows::UI::Xaml::Controls::TimePickerSelectedValueChangedEventArgs^args)
	{		
		auto timespan = args->NewTime->Value;
		long long totalSeconds = (timespan.Duration) / (long long)10000000;

		int hours, minutes;
		hours = (int)((totalSeconds / 3600));
		minutes = (int)((totalSeconds % 3600)) / 60;

		if (onChanged != nullptr) onChanged(hours, minutes);
	});

	TimeSpan ts;
	long long v = (long long)hours * 3600 + minutes * 60;
	ts.Duration = (v * 10000000); // Duration is in "100-nanosecond" units!!
	cp->Time = ts;
	
	b = ref new Grid(); // we need this to be able to BgColor()
	((Grid^)b)->Children->Append(cp);	
	

#elif defined(CUICUI_WEB)
	EM_ASM_(
		{
			var num = $0;
			var elem = getUnattachedElementsFromId(num.toString());
			if (elem == null)
			{
				Module.print("ERROR wTimePicker -> elem not found : " + num.toString());
				return;
			}

			elem.style["pointer-events"] = "auto";

			var x = document.createElement('INPUT');
			x.setAttribute("type", "time");
			var pad = function(n) { return (n < 10 ? "0" : "") + n; };
			x.value = pad($2) + ":" + pad($3);
			x.oninput = function() { Module.ccall('wTimePickerWebOnChanged', 'void', ['number', 'string'], [$1, this.value]); };

			elem.appendChild(x);
		}, htmlId, this, hours, minutes);
#elif defined(CUICUI_IOS)
	#error "native::wTimePicker is not implemented on iOS yet (the code below is a copy of wCalendar)"
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

bool native::wTimePicker::ComputeMinSize(double * w, double * h)
{
#ifdef CUICUI_UWP
	//ComputeNativeWidgetMinSize(this, w, h); // doesn't work on uwp, when the app's window resize, the calendar also resizes and get very big and ugly

	// temporary size:
	*w = GetEmSize(13);
	*h = GetEmSize(2);
#else
	ComputeNativeWidgetMinSize(this, w, h, false);

	//try commenting the line above (it should work but not sure)

#ifdef CUICUI_ANDROID
	// it's done elsewhere now
#endif

	#endif
	return true;
}

} // namespace cui