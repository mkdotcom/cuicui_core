#pragma once


#include "platform.h"
#include "easing.h"


// this optim doesn't work, to fix (but it's not a big deal)
#define OPTIM_211_DISABLE  // not refreshing content of scrollview when app is resized only on the same axis (deprecated, to remove)(we'll redo this optim later)


/*
#if defined(CUICUI_ANDROID)
missing
#elif defined(CUICUI_UWP)
missing
#elif defined(CUICUI_WEB)
missing
#elif defined(CUICUI_IOS)
missing
#elif defined(CUICUI_MACOS)
missing
#else
non_implemented_platform
#endif

// note: when adding a new platform, you can use "replace all"  on "#else non_implemented_platform" to do it quickly (BUT this tag is not everywhere yet)
*/

#define  NEW_WIDGET(_type) (_type*)_type::Create

extern void InitMyApp();

namespace cui {
enum AXIS { X = 0, Y = 1 };
//enum class TYPE { INVALID=-1, EMPTY=0, ROWSCOLS=0 /*ROWS, COLUMNS,*/ /*STEXT,*/ /*MTEXT*/ }; // rename to CUICUI_CORE_LAYOUT_TYPE or something like that?
// TODO: rename EMPTY to AREA? or to STACK?  or to STANDARD ? an empty coretype is EVERYTHING except rows/columns
// NOTE: empty is used for everything (except rows/columns) actually (thanks to GetMinSize())
// STEXT/MTEXT: NATIVECONTROL? EMPTY?

enum class eCoreType
{
	wWidget, // keep? wCalendar uses this, if it used a wempty would it be the same? or not?
	RowColEmpty, // a wRows or a wColumns or a wEmpty
	Text,
	Image,
	TextInput,
	ScrollView,
    Custom, // a widget defined outside the core (e.g. in a module)
    WebView,
    Gradient
};

#ifdef CUICUI_IOS
#undef MIN // a MIN is already defined by some cocoa include
#endif

// various defines
#ifdef CUICUI_DEBUG
#define CUICUI_NOP { printf(""); } // this code does nothing but it's useful to be able to put a breakpoint on the "NOP"
#else
#define CUICUI_NOP {}
#endif
#define CUICUI_NYI assert(false); 


extern std::function<void(std::string, std::string)> onCppMessageBox;

class cCppMessageBoxManager // move to cCentral ? or not?
{
	// Info: You can use this to send simple messages to the cpp part of your ANDROID (java) or WEB (javascript) code.
	//       (It also work from UWP and IOS and MACOS but on these platforms you can simply call the c++)

	// Note: Sent messages are put on a queue until SetCallback() is called by the user, then the callback will be called for each enqueued message.
	//       After that all new messages are delivered instantly to the callback.

	// Note: The callback is always called on the UI thread.
	
	// WARNING: do NOT do long or blocking stuff in the callback, it would block the UI !

	// TODO: if the user never calls SetCallback() this will pile up, problematic? clean old messages if the list gets really too big?
	//         (once the app has started we could clean the list if SetCallback() hasn't been called ?)

protected:
	std::function<void(std::string, std::string)> onCppMessageBox = nullptr;
	std::mutex cuiCppMessageBox_mutex; // dev note: this may be called from another thread, so we need a mutex
	std::list<std::pair<std::string, std::string>> cuiCppMessageBox_list;

public:
	void SetCallback(std::function<void(std::string, std::string)> _onCppMessageBox);
	void Send(std::string _tag, std::string _value);

};
extern cCppMessageBoxManager cppMessageBoxManager; // TODO: make a singleton

#if defined(CUICUI_ANDROID)
extern void someJniFuncThatYouShouldCallOnceOnMainThread();
#endif

extern std::thread::id cuiUiThreadId; // in central?

void AssertOnUiThread();
void AssertNotOnUiThread();
void DebugPrintOnUiThread();
#ifdef CUICUI_DEBUG
std::string ThreadIdAsString(std::thread::id tid);
#endif

static void cui_assert(bool b)
{
#ifdef CUICUI_DEBUG
    assert(b);
#else
	// on some platform assert() stops the app, we don't want this behavior
    // nothing to do
#endif
}


} // namespace cui
#include "utils.h"

// size constants (these used to be macros)
namespace cui {
static const fill FILL(1);       // take all the available space
static const min MIN;            // wrap content
static const Length UNSET;       // no size set
static const sameAsX SAME_AS_X;  // same size as the X axis
static const pixels PIXELS;

// old names kept for compatibility
static const Length EM_NOT_SET;
static const fill EM_FILL(1);
static const min EM_AUTO;




void RunOnUiThread(std::function<void()> func);
// Logging: writes to the platform log (logcat on Android with the tag "cuicui", browser console on the
// Web, NSLog on iOS, Visual Studio output on UWP).
enum class LogLevel { Debug, Info, Warning, Error };
void LogWrite(LogLevel level, const std::string& str); // always written, whatever the build
void LogDebug(const std::string& str);   // only in debug builds (CUICUI_DEBUG)
void LogInfo(const std::string& str);
void LogWarning(const std::string& str);
void LogError(const std::string& str);
// short names (kept for compatibility)
void Log(const std::string& str);   // = LogDebug
void LogD(const std::string& str);  // = LogDebug
void LogR(const std::string& str);  // = LogInfo ("release": also written in release builds)
	

class TimeCounter
{
protected:
	std::chrono::time_point<std::chrono::system_clock> startTime;

public:
	TimeCounter()
	{
		startTime = std::chrono::system_clock::now();
	}
	void Log()
	{
		auto endTime = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = endTime - startTime;
		auto spentTimeInMilliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
		auto ms = spentTimeInMilliSeconds.count();
		cui::Log("TimeCounter: milliseconds spent: " + std::to_string(ms));
	}
};

} // namespace cui
#include "timer.h"
#include "local_storage.h"

/*
class BackButtonManager  // move to central?
{
protected:
	std::mutex _mutex;
	std::vector<std::function<void()>> list;

	// TODO: using the mutex blocks the "back" on images in an app (seen on ANDROID systematically, check other platforms)

	// TODO: I do pop/push manually, maybe better to go through PopListener and PushListener


public:

	BackButtonManager()
	{
		CUICUI_NOP
	}

	size_t GetPushedElementsCount()
	{
		//std::lock_guard<std::mutex> lock(_mutex);
		return list.size();
	}
	void PushListener(std::function<void()> f)
	{
		//std::lock_guard<std::mutex> lock(_mutex);
		list.push_back(f);
	}
	void PopListener()
	{
		//std::lock_guard<std::mutex> lock(_mutex);
		if (list.size() == 0) return;
		list.pop_back();
	}
	void PopAll()
	{
		list.clear();
	}
	void Call()
	{
		//std::lock_guard<std::mutex> lock(_mutex);
		if (list.size() == 0) return;
		list.back()();
	}
};
extern BackButtonManager backButtonManager_deprecated;*/





namespace cui {
#if  defined(__EMSCRIPTEN__)
	extern int htmlIdCount;
#endif



#if defined(WINAPI_FAMILY)
	
	// put in "defwidget.cpp"
	ref class cCuiElement_uwp // this is used to receive events from UWP controls (TODO: we should be able to fully replace this with lambdas!)
	{
		friend class wWidget;

	internal:
		wWidget * e;

	internal:
		cCuiElement_uwp(wWidget *_e)
		{
			e = _e;
		}

		void touchRectangle_PointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args);
		void touchRectangle_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args);
		void touchRectangle_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args);
		void touchRectangle_PointerExited(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args);
		void touchRectangle_PointerEntered(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args);
		// cancel missing? 
		// gesture may need it, see : https://docs.microsoft.com/en-us/uwp/api/windows.ui.input.gesturerecognizer

		void cic(Windows::UI::Xaml::Controls::ListViewBase^, Windows::UI::Xaml::Controls::ChoosingItemContainerEventArgs^);
		void ccc(Windows::UI::Xaml::Controls::ListViewBase^, Windows::UI::Xaml::Controls::ContainerContentChangingEventArgs^);

		void textinput_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs ^args);
		void textinput_PasswordChanged(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs ^args);
		void textinput_OnKeyUp(Platform::Object ^sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^args);

		// uncomment if needed, not tested (should try in winphone simulator?)
		//void scrollView_DirectManipulationStarted(Platform::Object^ sender, Platform::Object^ e);

		void ButtonClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args); // used by nativebutton

		void OnTransitionTick(Object^ sender, Object^ e);
		
		void OnDragOver(Platform::Object ^sender, DragEventArgs ^e);
		void OnDrop(Platform::Object ^sender, DragEventArgs ^e);

		void OnPasteOnTextInput(Platform::Object ^sender, Windows::UI::Xaml::Controls::TextControlPasteEventArgs ^args);

	};

#endif


	class wWidget;

	extern std::vector<wWidget*> RefreshUI_Private_sces;

	void DumpTree(AXIS axis, wWidget *w);

	bool CheckDefWidgetAlive(wWidget *w);

	void RefreshUI_Private(wWidget *defw); 
	void RefreshUI_Everything();
	void RefreshUI_Fast(wWidget *defw);

#if defined(CUICUI_IOS)
    class Transition;
} // namespace cui
@interface CuiTransitionEventClass : NSObject
@end
namespace cui {
#endif


	class AnimationStep
	{
	public:
		wWidget* self;
		double value;
		double deltaValue;
		bool isLast;
	};

	// move to central? meh
	class Transition //: public std::enable_shared_from_this<Transition> /*TODO : remove this inheritance if we get rid of shared_ptr*/
	{
		friend class AnimationManager;
		
	public:
		int guid;
		wWidget * widget;
		std::function<void(AnimationStep step)> callback;
		double valueStart, valueEnd;
		double prevValue;
		double duration; // seconds
		std::chrono::steady_clock::time_point startedAtThisGlobalTime; // seconds
		bool done = false;
		bool loop;
		easingFunction easingFunc = nullptr;




        
		// note: 'isLast' can be used to know when to start another transition for example

	public:
		Transition()
		{

		};
	public:
		~Transition()
		{
			Log("DebugInfo: Transition destructor.");

		}

	public:
		void RunFrame(std::chrono::steady_clock::time_point currentGlobalTime);

		// Note: a transition is attached to a widget, when the widget is destroyed the transition is destroyed
		// Note2: A transition that is not "loop=true" is destroyed when the transition is completed
		// we can still keep it in a cAnimFrame that would contain all the info the callback must receive
		


	private:
		void Start_deprecated();

	};

	// move to central? meh
	class AnimationManager
	{
	public: // TODO: should be protected
		std::list<Transition*> transitions;

	protected:		
		int count=0;

#ifdef CUICUI_IOS
        CuiTransitionEventClass *cuiTransitionEvent;
        CADisplayLink *displayLink;
#endif
#ifdef CUICUI_UWP
		Windows::Foundation::EventRegistrationToken dispatchTimer_registrationtoken;
		Windows::UI::Xaml::DispatcherTimer ^dispatchTimer = nullptr; // all transitions of a same widget share the same dispatchTimer( TODO:QUESTION: only one for the whole app?)
#endif

	public:
	

	public:
		Transition* create(wWidget *w, double _valueStart, double _valueEnd, double _duration, std::function<void(AnimationStep step)> _callback, bool _loop = false, easing_functions easing=EaseOutSine);

		void DeleteTransitionsOwnedByThisWidget(wWidget *w);
		void Run();

        AnimationManager()
        {
#if defined(CUICUI_IOS)
#ifdef CUICUI_IOS
            cuiTransitionEvent = [[CuiTransitionEventClass alloc] init];
#endif
#endif
        }
        
	protected:
        
		void StartEventLoop();
		void StopEventLoop();
	};
	extern AnimationManager animationManager;


    double GetEmSize(double v);
   

	class wWidget;
	class wEmpty;
	class wRows;
	class wColumns;
	extern std::list<wWidget*> touchables;
#ifdef CUICUI_DEBUG
	extern std::list<int> debugDeletedWidgets;
#endif

	class Children
	{
	public: //private:
		std::vector<wWidget*> list; // protected/private : TODO
	public:
		wWidget *self;

		Children& operator+=(wWidget* w);
	};

	void CreateUI_Recur(wWidget *w);


	extern int gDebugCountId;

	void GetAllWidgetRecursively(std::list<wWidget*> &list, wWidget *w);

    class cFont // note: will probably need a FontManager
				// cFont shouldn't contain any native at all (just the font description -> rename to cFontDescriptor)
			    // the user can then do FontManager.GetFontFromDescriptor(descriptor)
				// (and fontmanager keeps a cache to create each font of a same descriptor only once)
		// does this make sense on all platforms, including web?
    {
    public:
#ifdef CUICUI_IOS
        UIFont *native;
		double sizeInPoints;
#elif defined(CUICUI_UWP)
		FontFamily ^native;
		double fontSizeInPixels;
#elif defined(CUICUI_WEB)
		double fontSizeInPixels;
		std::string fontFamily = ""; // "" = default font in css (TODO: is that sure? isn't it rather "null" -> erase style? I don't know if that exists)
#elif defined(CUICUI_ANDROID)
		double fontSizeInPixels;
#else
        missing
#endif

    };
    

	class cFileReceived;

} // namespace cui
#include "central.h"
    
		

namespace cui {
	void ComputeNativeWidgetMinSize(wWidget *dw, double *w, double *h, bool subView); // Warning: on ANDROID some widgets won't be calculated correctly (exemple: wTimePicker; check how it's done)

#ifdef CUICUI_WEB
	void WEB_CuiAttachChildInNativeDom(int parentHtmlId, int childHtmlId);
#endif

	


} // namespace cui
#include "wBase.h"




namespace cui {
#ifdef CUICUI_UWP
void OnWindowSizeChanged(int width, int height, Canvas ^rootGrid);
#elif CUICUI_IOS
void OnWindowSizeChanged(int width, int height, UIView *parentView);
#else
void OnWindowSizeChanged(int width, int height);
#endif


class wEmpty : public wWidget
{
public:
	wEmpty() : wWidget()
	{		
		coreType = eCoreType::RowColEmpty;		
		Native();		
	}	

	virtual ~wEmpty()
	{
	}

	void SetChildVisibility(int idx, bool show)
	{
		if (idx < 0) return;
		if (idx >= children.size()) return;
		wWidget *w = GetChild(idx);
		if (w == nullptr) return;
		w->Visibility(show);
	}
	
public:
	static wEmpty *Create(Length xem, Length yem) { auto c = new wEmpty(); c->Xem(xem)->Yem(yem); return c; }
};

class wHGap : public wWidget 
{
public:
	wHGap(Length xem) : wWidget()
	{
		Xem(xem); 
		Yem(fill()); // important to fill() if user wants to BgColor() it
	}
public:
	static wHGap* Create() = delete;
	static wHGap* Create(double, double) = delete;
	static wHGap* Create(Length xem) { auto c = new wHGap(xem); return c; }
};

class wVGap : public wWidget 
{
public:
	wVGap() : wWidget()
	{
	}
	wVGap(Length verticalEm) : wWidget()
	{
		Xem(fill()); // important to fill() if user wants to BgColor() it
		Yem(verticalEm);
	}

public:	
	static wVGap* Create() = delete;
	static wVGap* Create(Length, Length) = delete;
	static wVGap* Create(Length yem) { auto c = new wVGap(); c->XemYem(fill(),yem); return c; }
};

class wColumns : public wWidget
{
public:
	wColumns() : wWidget()
	{		
		coreType = eCoreType::RowColEmpty;
		rowcolaxis = X;
	}
	wColumns* CenterAllChildrenVertically()
	{				
		centerChildrenInBadAxis = true;
		return this;
	}
	virtual ~wColumns()
	{
	}
		
public:
	static wColumns *Create(Length xem, Length yem) { auto c = new wColumns(); c->Xem(xem)->Yem(yem); return c; }
};

class wRows : public wWidget
{
public:	
	wRows() : wWidget()
	{
		
		coreType = eCoreType::RowColEmpty;
		rowcolaxis = Y;
	}
	wRows* CenterAllChildrenHorizontally()
	{
		centerChildrenInBadAxis = true;
		return this;
	}
	virtual ~wRows()
	{
	}
	
public:
	static wRows *Create(Length xem, Length yem) { auto c = new wRows(); c->Xem(xem)->Yem(yem); return c; }
};

} // namespace cui
#include "wScrollView.h"



//#define wListViewNative_ENABLED native list view not finished
namespace cui {
#ifdef wListViewNative_ENABLED
class wListView2 : public wWidget
{
public:
	float topPos, bottomPos;
#if defined(WINAPI_FAMILY)
	ScrollViewer ^sv;
#endif
#if defined(CUICUI_ANDROID)
	int scrollViewContentTheId;
#endif

public:
	wWidget * children;
	wListView2() : wWidget("listview2")
	{
		topPos = 0; 
		bottomPos = 0;

#if defined(WINAPI_FAMILY)
		sv = ref new ScrollViewer();
		sv->VerticalScrollBarVisibility = ScrollBarVisibility::Auto;
		b->Children->Append(sv);

#endif
	}
	void scrollPosChanged()
	{				
		// if bottomPos < scrollviewvisiblearea.bottom -> we add a new element (that we create or that we fetch from outside the scrollviewvisiblearea)
		// same at the top
		// items must be placed by hand 
	}
};

class wListView : public wWidget
{
public:
#if defined(WINAPI_FAMILY)
	ListView ^listView;
#endif
#if defined(CUICUI_ANDROID)
#endif



	class cItem
	{
	public:
		int uid;		

	public:
		cItem(int _uid) {
			uid = _uid;
		}
	};

	std::vector<cItem*> data;
	std::vector<wWidget*> items; // list of widgets added in cells of the native listview 


public:
    wListView() : wWidget("listview")
	{
		onSetupCell = nullptr;
		isNativeControl = true;

#if defined(WINAPI_FAMILY)
		listView = ref new ListView(); // is it the right control? we want virtualization
		listView->SelectionMode = Windows::UI::Xaml::Controls::ListViewSelectionMode::None; // disable selection mode
		listView->Width = 0; // very important! otherwise all cells are created from the start! (the real size is set in SetPosHW later)
		listView->Height = 0;// very important! otherwise all cells are created from the start! (the real size is set in SetPosHW later)
		b->Children->Append(listView);


		listView->ChoosingItemContainer += ref new Windows::Foundation::TypedEventHandler <Windows::UI::Xaml::Controls::ListViewBase^, Windows::UI::Xaml::Controls::ChoosingItemContainerEventArgs^>(hd, &cCuiElement_uwp::cic); 
		listView->ContainerContentChanging += ref new Windows::Foundation::TypedEventHandler <Windows::UI::Xaml::Controls::ListViewBase^, Windows::UI::Xaml::Controls::ContainerContentChangingEventArgs^>(hd, &cCuiElement_uwp::ccc);

#endif
#if defined(CUICUI_ANDROID)

		jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiListView_CreateAndAddToElement", "(I)I");
		/*jint v =*/ globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, theId);
#endif
#if defined(__EMSCRIPTEN__)
		
#endif
	}

	

	std::function<wWidget*(int cellId)> onSetupCell;

	std::function<wWidget*(cItem* item)> onCreateItem;
	std::function<void(wWidget *w, cItem* item)> onUpdateItem;
	
public: //private:
	/*wWidget* DoSetupCell_private(int cellId)
	{
		assert(onSetupCell != nullptr);

		wWidget *cell = onSetupCell(cellId);
		this->items.push_back(cell);
		
		// TODO: we do this in the "refreshui" of wListView, but it seems it must be done here too (at least on uwp), not really sure why
		if (this->w > 0)  // if not the case the lv doesn't have its real size yet (CLEANUP: not sure this is normal)
		{		
			RefreshUI_Recur(cell, this->w, 0);
			cell->SetPos_HW(cell->x, cell->y, cell->w, cell->h); 
		}

		return cell;
	}*/

public:
	template<typename T>
	void SetData(const std::vector<T> &_data)
	{		
#if defined(WINAPI_FAMILY)		


		Platform::Collections::Vector<int>^ vvv = ref new Platform::Collections::Vector<int>();

		for (cItem* d : _data)
		{
			vvv->Append(d->uid);
			data.push_back(d);
		}

		listView->ItemsSource = vvv;
		listView->SelectedItem = nullptr; // without that the first item in the listview has a "selection border"
#endif

		RefreshUI_Fast(this); // ok?
	}


};
#endif

} // namespace cui
#include "wImage.h"




// TODO: OnFileReceived should be called as a fast as possible with maybe just the url; and cFileReceived should have GetAsBase64(), GetAsRawBytes(), ...
// this will be better because the app get the control sooner and can display progress indicators, and maybe the app doesn't want base64
namespace cui {
class cFileReceived // in central? meh
{
public:
	std::string fileName;
	std::string base64Data; // note: could be a native string to reduce conversions?
	std::string mimeType;
	int imageOrientation = -1; // -1=unknown, will be set only on ANDROID ! (for now)
    std::string option; // may contain various information messages (android only for now?)
    bool valid=true;

public:
    virtual ~cFileReceived() // why virtual? this make the class polymorphic which is useful for use of std::dynamic_pointer_cast
	{
		Log("Debug info: a cFileReceived was deleted: shared_ptr did its work."); // TODO_68712
	}
	std::unique_ptr<std::string> GetAsRawBytes() // Note: it returns a std::string, but it's not a "string", it's a byte buffer!
	{
		auto v = std::make_unique<std::string>();
		
		size_t rawBytesSize = 0;
		unsigned char * rawBytes = base64_decode(this->base64Data.c_str(), this->base64Data.size(), &rawBytesSize);
		v->assign((char*)rawBytes, rawBytesSize);
		free(rawBytes);
		return v;		
	}
	void ConvertToJpgAndResize(int maximumWidth, int maximumHeight, int jpegCompression /*[0,100]*/)
	{
		assert(jpegCompression >= 0 && jpegCompression <= 100);
#ifdef CUICUI_ANDROID
		base64Data = JNICall_String_StringIntIntIntInt("cFileReceived_ResizeAndJpegize", base64Data, maximumWidth, maximumHeight, jpegCompression, imageOrientation);
        // TODO: return false? or does this set base64Data to "" ? return true/false anyway
#elif CUICUI_IOS
        // TODO : try /catch + returns true/false
        NSString *s = StdStringToNSString(base64Data);
        NSData *dataSource = [[NSData alloc]initWithBase64EncodedString:s options : 0];
        UIImage *chosenImage = [UIImage imageWithData : dataSource];
        assert(chosenImage!=nullptr);
        int biggest = std::max(chosenImage.size.width, chosenImage.size.height);
		int biggestRequested = std::max(maximumWidth, maximumHeight);
        float scaleValue = (float)biggestRequested/(float)biggest; // TODO: is this ok ? will it work ok in landscape and portrait? test well
        CGSize targetSize = CGSizeMake(chosenImage.size.width*scaleValue,chosenImage.size.height*scaleValue);
        UIImage *image = chosenImage;
        UIImage *resizedImage = nil;
        UIGraphicsBeginImageContext(targetSize);
        CGRect thumbnailRect = CGRectMake(0, 0, 0, 0);
        thumbnailRect.origin = CGPointMake(0.0,0.0);
        thumbnailRect.size.width  = targetSize.width;
        thumbnailRect.size.height = targetSize.height;
        [image drawInRect:thumbnailRect];
        resizedImage = UIGraphicsGetImageFromCurrentImageContext();
        
        UIGraphicsEndImageContext();
        
        // convert resizedImage to base64
        NSData *data = UIImageJPEGRepresentation(resizedImage, 85); // should be able to convert to base64 without  converting to jpg... (note: we also do it in viewcontroller.cpp, duplicate)
        NSString* base64String = [data base64EncodedStringWithOptions:0];
        base64Data = NSStringToStdString(base64String);
#elif CUICUI_UWP
		LogWarning("ConvertToJpgAndResize has no effect on UWP !"); // TODO UWP
#elif CUICUI_WEB
		LogWarning("ConvertToJpgAndResize has no effect on WEB !"); // TODO WEB
#else
		assert(false); // not available on this platform
#endif
	}
};

extern std::function<void()> onFileReceivedSoon; // this is called after the user selected an image in the imagepicker and before onFileReceived() is called
extern std::function<void(std::shared_ptr<cFileReceived> pr)> onFileReceived;

extern std::function<void()> onAppSizeChanged; // put in cCentral

#ifdef CUICUI_UWP
void UwpCommandLineArguments(Platform::String ^args); // called by the UWP host (MainPage::OnNavigatedTo) with the launch arguments
#endif

#ifdef CUICUI_WEB
extern std::function<void()> webOnPwaReadyToBeInstalled;
#endif



#if defined(CUICUI_WEB)
// TODO: we could do without this function by putting the code directly in test.cpp in the "fileinput_event" function
extern void WebOnFileInputSelection(std::string fileName, char *base64Data /*NOT null terminated!!*/, int base64DataSize, std::string mimeType);  // WARNING: do NOT delete/free "base64Data", it will be done by javascript side

#endif


#ifdef CUICUI_ANDROID
void AndroidOnFileInputSelection(std::string fileName, std::string base64Data, int dataSize, std::string mimeType, int imageOrientation, std::string option);
#endif

#ifdef CUICUI_IOS
} // namespace cui
#import <WebKit/WebKit.h>
namespace cui {
#endif

class wWebView : public wWidget
{
public: // protected
#if defined(CUICUI_UWP)
	WebView ^webView;
#endif
#ifdef CUICUI_IOS
    WKWebView *iosWebView;
#endif

	// WARNING: this widget is work in progress

	// Note: if content of the webview is bigger than the webview, there will be scrollbars

	// Warning: MIN is not supported
	//     UWP: one way to do it : http://jasonpoon.ca/resizing-webview-to-its-content/
	//     other platforms : ?

	// TODO: on android, if the wWebView is is a wScrollView it's not possible to scroll (the wScrollView catch the gestures...) 
	// TODO: android BgColor doesn't work (must call the webview's one via virtual BgColor())(but we removed the virtuals, didn't we?)(make a special BgColor?)(or override anyway and warn in the doc?)
public:
    wWebView() : wWidget()
    {
        coreType = eCoreType::WebView;
	
#if defined(CUICUI_ANDROID)
		JNICall_Int_Int("cuiWebView_CreateAndAddToElement", theId);
#elif defined(CUICUI_UWP)
		webView = ref new WebView();
		b = webView;
#elif defined(CUICUI_WEB)
		CUICUI_NOP
#elif defined(CUICUI_IOS)
        //TODO // assert(false); // TODO
        iosWebView = [[WKWebView alloc] init];
        [uiView addSubview: iosWebView];
#else
		missing
#endif
    }

	wWebView* Html(const std::string &html)
	{
#if defined(CUICUI_ANDROID)		
		JNICall_Void_IntString("cuiWebView_SetHtml", theId, html);
#elif defined(CUICUI_UWP)		
		webView->NavigateToString(StdStringToUwpString(html));		 
#elif defined(CUICUI_WEB)
		EM_ASM_(
		{				
			var elem = gu($0);			
			if (elem == null) { console.log("wWebView::Html() error 54");  return; }			
			var content = UTF8ToString($1);			
			elem.innerHTML = content;
			elem.style["overflow"] = "auto"; // this add scrollbars if content of the webview is bigger than the webview
		}, htmlId, html.c_str());
#elif defined(CUICUI_IOS)
        [iosWebView loadHTMLString:StdStringToNSString(html) baseURL:nil];
#else
		missing
#endif
		return this;
	}

    wWebView* Source(const std::string &source)
    {
#if defined(CUICUI_ANDROID)
        JNICall_Void_IntString("cuiWebView_SetSource", theId, source);
#elif defined(WINAPI_FAMILY)
		Uri ^u = ref new Uri(StdStringToUwpString(source));
		webView->Source =u;
#elif defined(__EMSCRIPTEN__)
		assert(false); // need to use an iframe?
#elif defined(CUICUI_IOS)
        assert(false); // TODO
#else
		missing
#endif
        return this;
    }
	
	static wWebView *Create(Length xem, Length yem) { auto c = new wWebView(); c->Xem(xem)->Yem(yem); assert(xem.IsMin()==false); assert(yem.IsMin()==false); return c; }
};


} // namespace cui
#include "wTextInput.h"
#include "wText.h"

   


namespace cui {
namespace native { // NativeWidgetSamples -> these are native controls samples

	// WARNING: it's a native control, so its width and height vary depending on the platform
	// PB: on UWP there is a big empty space at the right of the switch (TODO)
	class wNativeSwitch : public wWidget
	{
	public:
#if defined(WINAPI_FAMILY)
		ToggleSwitch ^sw;
#endif
	public:
		wNativeSwitch() : wWidget()
		{
#if defined(WINAPI_FAMILY)
			sw = ref new ToggleSwitch();
			sw->OnContent = nullptr; // hide "On" text at the right of the switch
			sw->OffContent = nullptr;  // hide "Off" text at the right of the switch
			b = sw;
#else
			assert(false); // NYI
#endif
		}
		std::function<void()> onClick;
		virtual bool ComputeMinSize(double *w, double *h)
		{
			ComputeNativeWidgetMinSize(this, w, h, false);
			return true;
		}

	};

	// WARNING: it's a native button, so its width and height vary depending on the platform	
	// WARNING: emRatio() is not applied to wNativeButton !
	class wNativeButton : public wWidget
	{
	public:
#if defined(WINAPI_FAMILY)
		TextBlock ^ title; // note: GetCalculatedWidth() and GetCalculatedHeight() needs the title of the button to be inside a TextBlock (or it always returns 0); source : https://stackoverflow.com/questions/39934712/desiredsize-of-a-button-and-radio-button-in-uwp-xaml
		Button ^bu;
#endif

	public:
		std::function<void()> onClick;

	public:
		wNativeButton() : wWidget()
		{				
			Yem(MIN); // must be MIN because the height of the button is controlled by the OS
			LockYem();

#if defined(CUICUI_UWP)
			title = ref new TextBlock();
			title->Text = "button"; 

			bu = ref new Button();
			bu->Content = title;
			bu->Click += ref new RoutedEventHandler(hd, &cCuiElement_uwp::ButtonClick);

			b = bu;
#else
			assert(false); // NYI
#endif
		}
		wNativeButton* Title(const std::string &str)
		{
			if (str.size() == 0) return this; // not allowed (why? uwp doesn't like it?)
#if defined(CUICUI_UWP)			
			title->Text = StdStringToUwpString(str);
#else
			assert(false); // NYI
#endif
			return this;
		}
		virtual bool ComputeMinSize(double *w, double *h)		
		{
			ComputeNativeWidgetMinSize(this, w, h, false);
			return true;
		}

		static wNativeButton *Create(Length xem) { auto c = new wNativeButton(); c->Xem(xem); return c; }
	};
}


double GetEmSize(double v);

extern wEmpty *master ; // it's the "root" wWidget that is placed on the native root View ; put it in central?

#ifdef CUICUI_DEBUG
static bool Debug_IsDeleted(wWidget *w)
{
	for (int i: debugDeletedWidgets)
		if (i == w->debugCountId)
		//if (i == w) // completely broken on web? don't compare pointers (pointers are maybe often reused on web)
			return true;
	return false;
}
#endif

#if defined(__EMSCRIPTEN__)


extern void WebOnTransitionEvent(int transitionPtr, int currentTimeInMilliseconds);



#endif


class wSafeZone : public wColumns // use this if you want your widget to stay away from navigation bar, status bar, insets/cutouts... You can have a view that use this (normal app page for example) and another view that doesn't (full screen image for exemple)
{
	// dev note: it's a wColumns because it suits android landscape with navigation bar (please don't change it to wRows)
	// dev note: wSafeZone correctly manages screen rotation which can make safe zone to change (usually on android: top&bottom in portrait, and top&right in landscape))
protected:
	wVGap* top, * bottom;
	wHGap* left, * right;

public:	
	// Note: you can simply use BgColor() to set all colors of the wSafeZone
	wSafeZone(/*optional parameters : */ cColor leftColor = cColor(), cColor topColor = cColor(), cColor rightColor = cColor(), cColor bottomColor = cColor());
	virtual wSafeZone* Content(std::function<void(Children & dc)> a_content);
};


static void ConvertCoordinates(wWidget *from, wWidget *to, double x, double y, double *outX, double *outY) // TODO: rename, it's not clear here (is it child to parent?)
{
	

	assert(CheckDefWidgetAlive(from));
	assert(CheckDefWidgetAlive(to));
	assert(from->IsThisWidgetOneOfMyParent(to)); 

	*outX = x;
	*outY = y;

	if (to == from)
	{		
		return;
	}
	wWidget *p = from;
	do
	{
		*outX += p->ax[X].pos.asDouble();
		*outY += p->ax[Y].pos.asDouble();

		if (p->coreType == eCoreType::ScrollView)
		{
			wScrollView *sv = (wScrollView*)p;
			if (sv->orientation == wScrollView::Orientation::Vertical)
				*outY -= sv->GetCurrentOffset().asDouble();

			if (sv->orientation == wScrollView::Orientation::Horizontal)
				*outX -= sv->GetCurrentOffset().asDouble();

			// we are at the top of a wScrollView
			p = p->parent;
		}
		else 
			p = p->parent;		
		
		assert(p != nullptr);		
		if (p == to) return;
	} while (1);
}





#if defined(CUICUI_ANDROID)


#ifdef listview_not_finished
static int AndroidListViewCreateOneCell(int cellId)
{
	// a cell is just a wEmpty, the real content will be created during the android "bind" of the cell

	return -1;
}


	static void AndroidListViewUpdateCell(int wListViewAndroidId, int cellId, int viewId) {
		std::string s = "update cell ";
		s += std::to_string(cellId);
		s += " which is using the view ";
		s += std::to_string(viewId);
		Log(s);

		wListView *listView = nullptr;
		{
			// PERF: not optimal
			std::list<wWidget *> list;
			GetAllWidgetRecursively(list,master); // PERF: not optimal
			for (wWidget *w : list) {
				std::string s = "looking for listview : ";
				s += std::to_string(wListViewAndroidId);
				s += " ; comparing with : ";
				s += std::to_string(w->theId);
				s += " / " + w->type;
				Cuicui:Log(s);
				if (w->theId == wListViewAndroidId) {
					listView = (wListView*)w;
					break;
				}
			}
			assert(listView!=nullptr);
		}



		//TODO: must clean "viewId" before putting something in it again:

		// NOTE (TODO?) : changing the params (size/bgcolor/...) of "e" here breaks everything
		// I don't know if it's "normal"
		// (however changing its children is ok)
		/*e->x=0; e->y=0; e->w=200; e->h=300;
        if (cellId%2==0) e->h = 400;
        e->SetPos_HW(e->x, e->y, e->w, e->h);*/

	}
#endif
#endif







class wPadding : public wRows
{
public:
	ems_or_pixels leftPadding, topPadding, rightPadding, bottomPadding;	

public:
	wPadding(ems_or_pixels _Padding) : wRows()
	{
		XemYem(fill());
		leftPadding = topPadding = rightPadding = bottomPadding = _Padding;		
		debugName = "wPadding";
	}
	wPadding(ems_or_pixels _horizontalPadding, ems_or_pixels _verticalPadding) : wRows()
	{
		XemYem(fill());
		leftPadding = rightPadding = _horizontalPadding;
		topPadding = bottomPadding = _verticalPadding;
		debugName = "wPadding";
	}
	wPadding(ems_or_pixels _leftPadding, ems_or_pixels _topPadding, ems_or_pixels _rightPadding, ems_or_pixels _bottomPadding) : wRows()
	{
		XemYem(fill());
		leftPadding = _leftPadding;
		topPadding = _topPadding;
		rightPadding = _rightPadding;
		bottomPadding = _bottomPadding;
		debugName = "wPadding";
	}	
	wPadding(const cRect& rectInPixels)
	{
		XemYem(fill());
		leftPadding = pixels(rectInPixels.left);
		topPadding = pixels(rectInPixels.top);
		rightPadding = pixels(rectInPixels.right);
		bottomPadding = pixels(rectInPixels.bottom);				
		debugName = "wPadding";	
	}
	
public:

	virtual wPadding* Content(std::function<void(Children &dc)> a_content)
	{			
		wRows::Content([this, a_content](Children& dc)
		{			
			dc += (new wVGap(topPadding));
			dc += (new wColumns())->XemYem(fill())->Content([this, a_content](Children& dc)
				{
					dc += new wHGap(leftPadding);
					dc += (new wEmpty())->XemYem(fill())->Content(a_content);
					dc += new wHGap(rightPadding);
				});
			dc += (new wVGap(bottomPadding));
		});
		return this;
	}

	// deprecated:
	static wPadding *Create(Length xem, Length yem, ems_or_pixels verticalAndHorizontalPadding) { auto c = new wPadding(verticalAndHorizontalPadding, verticalAndHorizontalPadding); c->Xem(xem)->Yem(yem); return c; }
	static wPadding *Create(Length xem, Length yem, ems_or_pixels verticalPaddingInEm, ems_or_pixels horizontalPaddingInEm) { auto c = new wPadding(verticalPaddingInEm, horizontalPaddingInEm); c->Xem(xem)->Yem(yem); return c; }
};


class wButtonImage : public wPadding // should be in nws ?
{
protected:
    wImage *im;
    std::function<void(wButtonImage* self)> onClick = nullptr;
    bool onClickEnabled = false;

public:
    wButtonImage(const std::string &source, ems_or_pixels padding = pixels(0), wImage::AutoResize autoResize = wImage::AutoResize::Contain) : wPadding(padding,padding)
    {
        Native();
        
        Content([this, autoResize, source](Children &dc)
        {
            im = wImage::Create(fill(),fill(), autoResize);
            im->Source(source);
            dc += im;
        });
        
        Clickable(1, [this](wWidget* self)
        {
            if (onClickEnabled == false) return;
            if (onClick != nullptr) onClick(this);
        });
    };


    void Enabled(bool b) // rename? same in wButton
    {
        onClickEnabled = b;
    }

    wButtonImage* Source(std::string str)
    {
        assert(str != "");
        im->Source(str);
        return this;
    }

    wButtonImage* OnClick(std::function<void(wButtonImage* self)> f)
    {
        onClickEnabled = true;
        onClick = f;
        return this;
    }
};




class wBorder : public wEmpty
{
public:
	enum class ePadding
	{
		None,
		Border,
		CornerRadius,
		CornerRadiusFull // default 
	};

protected:
	ems_or_pixels padding;

public:
	wBorder(Length xem, Length yem, const BorderStyle &borderArgs, ePadding _padding = ePadding::CornerRadiusFull) : wEmpty()
	{
		XemYem(xem, yem);
		DebugName("wBorder");
		Border(borderArgs);		

		if (_padding == ePadding::None)
			padding = pixels(0);
		if (_padding == ePadding::Border)
			padding = borderArgs.thicknessInPixels;
		if (_padding == ePadding::CornerRadius)
			padding = ems(borderArgs.cornerRadiusInEm.v / 2.0);
		if (_padding == ePadding::CornerRadiusFull)
			padding = borderArgs.cornerRadiusInEm;
	}
	
	virtual wBorder* Content(std::function<void(Children & dc)> a_content)
	{		
		wEmpty::Content([a_content, this](Children& dc)
		{
			dc += wPadding::Create(FILL, FILL, padding)->Content(a_content);
		});

		return this;
	}	
};


class wAlignRight : public wColumns
{
public:
	wAlignRight(wWidget *w) : wColumns()
	{
		this->Content([w](Children &dc)
		{
			wEmpty *e = new wEmpty();
			e->Xem(EM_FILL);
			e->Yem(EM_FILL);
			e->BgColor(0x993030);

			dc += e;
			dc += w;
		});
	}

	static wAlignRight *Create(Length xem, Length yem, wWidget *w) { auto c = new wAlignRight(w); c->Xem(xem)->Yem(yem); return c; }
};



class wGradient : public wEmpty
{
	// Note: you must call Apply() after setting up the gradient
	// Note: don't cal BgColor() on this widget or you will replace the gradient

public:

	class cStop 
	{
	public:
		cColor color;
		double offset; // [0.0,1.0]

	public:
		cStop(double _offset, cColor _color) {
			offset = _offset;
			color = _color;
		}
	};
	std::vector<cStop> stops;

#ifdef CUICUI_IOS
    CAGradientLayer *gradient=nullptr;
#endif
#ifdef CUICUI_UWP
	Canvas^ canvas;
#endif

public:
	wGradient() : wEmpty()
	{
        coreType = eCoreType::Gradient;
#ifdef CUICUI_UWP
		canvas = ref new Canvas();
		b = canvas;
#endif
	}

	void ClearGradient()
	{
		stops.clear();
	}
	void AddGradientStep(double offset, cColor color)
	{
		stops.push_back(cStop(offset, color));
	}
    void ApplyGradient();

public:
	static wGradient *Create(Length xem, Length yem)
	{
		auto c = new wGradient();
		c->Xem(xem)->Yem(yem);
		return c;
	}
};



	class wProgressIndicator : public wImage
	{
	protected:
		Transition *anim;

	public:
		wProgressIndicator() : wImage(wImage::AutoResize::Contain)
		{
			DebugName("wProgressIndicator");

			XemYem(ems(1), ems(1));
			Source("in_progress.png");

			anim = animationManager.create(this, 0, 3.1415*2.0, 3.0/*sec*/, [this](AnimationStep step)
			{
					this->Transform_Rotate(step.deltaValue);
			}, true);
		}

		void Pause()
		{
			// NYI
		}

		static wProgressIndicator *Create(Length xem, Length yem)
		{
			auto c = new wProgressIndicator();
			c->Xem(xem)->Yem(yem);
			return c;
		}
	};

	class wSwitch : public wEmpty
	{
		// Known issue: on UWP click only works on the dot, it's because a "touchable" bug of Cuicui on UWP		

		// TODO: add a OnGesture_Pan so use can "swipe" on or off

		// TODO_85214 on android it's perfect (on retina when zooming it's pixel perfect)	
		// however on UWP it's not ok (TODO); rounding problem?
		// not ok on WEB either
		// test ios
		// does this hide another problem in cuicui? on web I see that the padding of the capsule is not ok
		//  -> on web positions and sizes are all "int" yet the code seems to send them as float -> check this, maybe it's the problem

	public:
		double transitionDuration = 0.1; // in seconds
		bool on;
		wBorder* dot;
		wBorder* capsule;
		wPadding* capsuleInnerSpace;
		std::string storedTag;
		bool isAnimating = false;
		cColor colorWhenOff, colorWhenOn, borderColor = 0xAAAAAA;
		bool disabled = false;

	protected:
		std::function<void(wSwitch * self, bool newValue) > onChanged; // note: event are triggered at the end of the animation

	public:

		wSwitch* OnChanged(std::function<void(wSwitch * self, bool newValue)> f)
		{
			onChanged = f;
			return this;
		}

		wSwitch() : wEmpty()
		{
			debugName = "wSwitch";

			on = false;

			// the size of this control is fixed (but ChangeEmRatio() works if you need to zoom in/out)
			this->Xem(ems(2));
			this->Yem(ems(1));
			this->LockXem();
			this->LockYem();
			
			Touchable();
			OnGesture_Tap([this](wWidget* self, pixels posX, pixels posY)
				{
					if (disabled) return;
					if (isAnimating) return;
					isAnimating = true;
					if (on)
					{
						// becomes OFF	
						SetValue(false, false);
						animationManager.create(dot, 1, 0, transitionDuration, [this](AnimationStep step)
							{
								AssertOnUiThread();
								dot->hwPosOffsetX = pixels::CreateFromDouble(step.value * GetEmSize(1) * emRatio());
								if (step.isLast)
								{
									dot->BgColor(colorWhenOff);
									isAnimating = false;
									RefreshUI_Fast(this);

									RunOnUiThread([this]() {
									//timerManager.Create(0.020, [this](Timer* timer) { // what is this for? TODO
										AssertOnUiThread();
										if (onChanged != nullptr) onChanged(this, false);
										});
								}
								else
									RefreshUI_Fast(this); // note: RefreshUI_Fast is ok because we know this widget never change its size (BUT we would use offsethw+Sethwpos to be faster)							
							});
					}
					else
					{
						// becomes ON	
						SetValue(true, false);
						animationManager.create(dot, 0, 1, transitionDuration, [this](AnimationStep step)
							{
								AssertOnUiThread();
								dot->hwPosOffsetX = pixels::CreateFromDouble(step.value * GetEmSize(1) * emRatio());
								if (step.isLast)
								{
									dot->BgColor(colorWhenOn);
									isAnimating = false;
									RefreshUI_Fast(this);

									RunOnUiThread([this]() {
									//timerManager.Create(0.020, [this](Timer* timer) { // what is this for?
										AssertOnUiThread();
										if (onChanged != nullptr) onChanged(this, true);
										});
								}
								else
									RefreshUI_Fast(this); // note: RefreshUI_Fast is ok because we know this widget never change its size (BUT we would use offsethw+Sethwpos to be faster)							
							});
					}
				});

			Content([this](Children& dc)
				{
					BorderStyle borderArgs;
					borderArgs.SetAllCornersRadius(ems(0.5));
					borderArgs.color = borderColor;
					borderArgs.thicknessInPixels = pixels(1);

					dc += capsule = (new wBorder(FILL, FILL, borderArgs, wBorder::ePadding::Border))->Content([this](Children& dc)
						{
							dc += capsuleInnerSpace = (wPadding*)(new wPadding(pixels(1)))->XemYem(FILL, FILL)->Content([this](Children& dc)
								{
									BorderStyle borderArgs;
									borderArgs.SetAllCornersRadius(ems(0.5));
									borderArgs.thicknessInPixels = pixels(0);
									dc += dot = (wBorder*)(new wBorder(pixels(0), pixels(0), borderArgs, wBorder::ePadding::None))->BgColor(0xffffff); // we should do XemYem(SAME_AS_Y,FILL) but SAME_AS_Y doesn't exist
									// isn't there a wCircle ?																
								});

							capsuleInnerSpace->onSizeChanged = [this](wWidget* self, pixels width, pixels height)
							{
								// TODO_85214
								dot->ax[X].pos = pixels(0);
								dot->ax[X].size = height.AsDouble() - 2;
								dot->ax[Y].pos = pixels(0);
								dot->ax[Y].size = height.AsDouble() - 2;
								dot->SetPos_HW2();
							};
						});
				});

			DotColor(0xffffff, 0xaaaaaa);

			onSizeChanged = [this](wWidget* self, pixels width, pixels height)
			{
				SetValue(on, true); // this will update the pos of the dot
			};
		}

		wSwitch* Disabled(bool _disabled = true)
		{
			disabled = _disabled;
			return this;
		}

		wSwitch* BorderColor(cColor _borderColor)
		{
			BorderStyle borderArgs;
			borderArgs.SetAllCornersRadius(ems(0.5));
			borderArgs.color = borderColor;
			borderArgs.thicknessInPixels = pixels(1);
			capsule->Border(borderArgs);
			return this;
		}

		wSwitch* DotColor(cColor _colorWhenOn, cColor _colorWhenOff)
		{
			colorWhenOn = _colorWhenOn;
			colorWhenOff = _colorWhenOff;
			if (on)
				dot->BgColor(colorWhenOn);
			else
				dot->BgColor(colorWhenOff);
			return this;
		}

		// TODO: rename to WithLocalStorage() ? AutoStoreInLocalStorage()? other?
		wSwitch* Stored(const std::string& tag, bool defaultValue) // WARNING: when using this, the onChanged() callback is not triggered ! Use IsOn() later and do what you need to do (if you need to do something)
		{
			storedTag = tag;
			std::string v;
			LocalStorage::GetInstance()->Read(tag, &v);
			if (v == "") // no value yet
			{
				SetValue(defaultValue);
			}
			else
			{
				SetValue(v == "true" ? true : false);
			}
			return this;
		}

		bool IsOn()
		{
			return on;
		}

		wSwitch* SetValue(bool _on, bool updateDotPosition = true)
		{
			if (updateDotPosition == false && on == _on) return this;

			if (storedTag != "") LocalStorage::GetInstance()->Write(storedTag, _on ? "true" : "false");

			on = _on;

			dot->BgColor(on ? colorWhenOn : colorWhenOff);

			if (updateDotPosition)
			{
				if (on == false)
				{
					dot->hwPosOffsetX = pixels::CreateFromDouble(0);
					RefreshUI_Fast(this); // note: RefreshUI_Fast is ok because we know this widget never change its size	
				}
				else
				{
					dot->hwPosOffsetX = pixels::CreateFromDouble(GetEmSize(1) * emRatio());
					RefreshUI_Fast(this); // note: RefreshUI_Fast is ok because we know this widget never change its size		
				}
			}
			return this;
		}


	};


	class wSliderVertical : public wEmpty
	{
	public:
		wEmpty *dot = nullptr;
		wBorder *border = nullptr;
		wEmpty *colorFiller = nullptr;
		pixels sliderLength;
		pixels widgetWidth;		
		double currentValue = 0; // [0;1]
		pixels borderSpace;
		bool showColorFiller = true;
		cColor dotColor;

	public:
		std::function<void(wSliderVertical* self, double newValue)> onChanged = nullptr;
		std::function<void(wSliderVertical* self)> onPressed = nullptr;
		std::function<void(wSliderVertical* self)> onReleased = nullptr;

		void SetColors(cColor colorInside, cColor colorDot, cColor colorDotBorder)
		{
			BgColor(colorInside);
			

			dot->BgColor(colorDot);

            dot->border2_args.color = colorDotBorder;
			dot->Border(dot->border2_args);
		}

		void ShowColorFiller(bool b) 
		{
			showColorFiller = b;
			UpdateDotPos();
		}

		wSliderVertical(wWidget::BorderStyle &_borderArgs, ems_or_pixels _dotPadding) : wEmpty()
		{			
			this->Xem(ems(1.0)); 
			this->Yem(EM_NOT_SET);
			this->LockXem();
			
			Touchable(true);
			onPointerPressed = [this](wWidget *self, pixels x, pixels y)
			{
				pixels v = (y - borderSpace) / sliderLength;
				SetValue(v.asDouble());

				if (onPressed != nullptr) onPressed(this);
			};

			OnGesture_Pan([this](wWidget* self, cOnPanGestureArgs args)
			{
				double a = args.x.asDouble();
				double b = args.y.asDouble();

                if (args.state==cOnPanGestureArgs::eState::End)
                {
                    if (onReleased != nullptr) onReleased(this);
                    return;
                }
                
                pixels v = (pixels::CreateFromDouble(b) - borderSpace) / sliderLength;
                SetValue(v.asDouble());
            });

			onPointerReleased = [this](wWidget *self, pixels x, pixels y)
			{
				if (onReleased != nullptr) onReleased(this);
			};

			onSizeChanged = [this](wWidget *self, pixels width, pixels height)
			{
				borderSpace = width / 2.0;
				sliderLength = height - borderSpace * 2;
				widgetWidth = width;				
				UpdateDotPos();				
			};

			Content([this, _borderArgs, _dotPadding](Children &dc)
			{

				dc += border = (wBorder*)(new wBorder(ems(1), FILL, _borderArgs, wBorder::ePadding::Border))->BgColor(0x000000)->Content([this, _dotPadding](Children& dc)
				{
					dc += wPadding::Create(ems(1),FILL, _dotPadding)->Content([this](Children& dc)
					{
						auto e = wEmpty::Create(ems(1), FILL)->Content([this](Children& dc)
						{
							colorFiller = (wEmpty*)wEmpty::Create(ems(1), ems(0))->BgColor(0xE99910);
							dc += colorFiller;

                            double cheat = 1.0 - 0.2; // ok only if dotpadding = ems(0.1) ...
                            
							dot = (wEmpty*)wEmpty::Create(ems(cheat), ems(cheat))->BgColor(0xffffff); // default is white, use SetColors() to change it

							BorderStyle borderArgs;
							borderArgs.SetAllCornersRadius(ems(cheat/2.0)); // cheat/2 = radius of the dot: this way the dot will be perfectly round
							borderArgs.color = 0x000000; // default is black, use SetColors() to change it
							borderArgs.thicknessInPixels = pixels(1);
							dot->Border(borderArgs);

							dc += dot;
						});
						dc += e;
					});					
				});

				this->bgColorTarget = border; 
			});
		}

		

		void SetValue(double v) // [0.0;1.0]
		{
			if (v < 0) v = 0;
			if (v > 1) v = 1;
			currentValue = v;
			UpdateDotPos();
			if (onChanged != nullptr) onChanged(this, currentValue);		
		}

		double GetValue()
		{
			return currentValue;
		}

		void UpdateDotPos()
		{
			if (sliderLength.IsValid())
			{
				dot->hwPosOffsetY = sliderLength * currentValue;
				dot->SetPos_HW2();

				colorFiller->SetPos_HW(pixels::CreateZero(), pixels::CreateZero(), widgetWidth.asDouble(), dot->hwPosOffsetY.asDouble() + borderSpace.asDouble());
				if (showColorFiller)
					colorFiller->Opacity(0.5 + currentValue / 2);
				else
					colorFiller->Opacity(0);
			}
		}

	public:
		static wSliderVertical *Create(Length yem, wWidget::BorderStyle& _borderArgs, ems_or_pixels _dotPadding = ems(0.1))
		{
			auto c = new wSliderVertical(_borderArgs, _dotPadding);
			assert(yem.IsMin()==false);  /*min doesn't make sense on this widget*/
			c->Yem(yem);
			return c;
		}
	};

	class wSlider : public wEmpty // redo it the wverticalslider2 way, no?
	{
	protected:
		wBorder *dot;
		double currentValue; // [0.0,1.0]
		bool pressed;
		ems hgap; // em
		ems dotDiameter; // em
		pixels barWidth;
		pixels hgapPixels;
		wEmpty *theBar;

	public:
		std::function<void(wSlider* self, double newValue)> onChanged;

	public:
		wSlider();

		// note: this doesn't trigger the onChanged event; you can trigger it yourself by simply calling it : mySlider->onChanged(mySlider, value) (QUESTION: always follow this pattern? (or the opposite pattern?), to think about)
		void SetValue(double f); // [0.0,1.0]
	
		double GetValue()
		{
			return currentValue;
		}

	protected:
		void Apply(bool callOnChanged = true);
	

	public:
		static wSlider *Create(Length xem)
		{
			auto c = new wSlider();
			assert(xem.IsMin()==false);  /*min doesn't make sense on this widget*/			
			c->Xem(xem);
			return c;
		}
	};


	class wButton : public wPadding
	{
	protected:
		bool onClickEnabled = false;
		std::string text;
		wText* wtext;

	public:
		wButton(std::string _text="", ems_or_pixels horizontalPadding=pixels(0)) : wPadding(horizontalPadding,ems(0)) 
		{
			text = _text;
			XemYem(min(), ems(1.0));
			BorderStyle borderArgs;
			borderArgs.SetAllCornersRadius(ems(0.1));
			borderArgs.color = 0x000000;
			borderArgs.thicknessInPixels = pixels(1);
			Border(borderArgs);
			BgColor(0xaaaaaa);

			Content([this](Children& dc)
			{
				dc += wColumns::Create(FILL, FILL)->CenterAllChildrenVertically()->Content([this](Children& dc)
				{
					dc += wHGap::Create(fill());
					dc += wtext = (wText*)wText::Create(MIN, MIN)->SetText(text,0xff000000,true)->ChangeEmRatio(0.8);// ->BgColor(0xff0000); // TODO: if we set a bgcolor we see that the gap at the top and at the bottom is not the same (uwp only?)
					dc += wHGap::Create(fill());
				});
			});
		}

		wButton* SetText(const std::string& _text)
		{
			text = _text;
			wtext->SetText(text, 0xff000000, true);
			return this;
		}

		wButton* Enabled(bool b)
		{
			onClickEnabled = b; 
			return this;
		}

		wButton* OnClick(std::function<void(wButton * self)> f) 
		{
			this->Clickable(1, [this, f](wWidget* self) { f(this); });
			return this;
		}
        
    public:
        static wButton* Create(Length xem, Length yem, std::string _text, ems_or_pixels horizontalPadding)
        {
            auto c = new wButton(_text, horizontalPadding);
            c->Xem(xem)->Yem(yem);
            return c;
        }
	};



// in stdw ? a priori yes but it depends on backbutton manager, I don't know if that changes anything
class wStackPanel : public wEmpty
{
	// TODO: use Visibility() (except if the pushed one is not opaque? let the user choose, and by default we consider it's opaque?)
public:
	wStackPanel() : wEmpty()
	{
		DebugName("wStackPanel");
	}

	virtual wStackPanel* Content(std::function<void(Children &dc)> a_content)
	{
		// don't use this with wStackPanel
		// add stuff in the wStackPanel with Push()
		assert(false); 
		return this;
	};

	wWidget* GetPageFromDebugName(const std::string &aDebugName)
	{
		for (auto w : children)
		{
			if (w->debugName == aDebugName) return w;
		}
		return nullptr;
	}

	void PopAll()
	{
		if (children.size() == 0) return;
		while (children.size() > 0)
			children.back()->DeleteMe();
	}

	void PopAllUntilFirst()
	{
		if (children.size()<=1) return;
		while (children.size() > 1)
			children.back()->DeleteMe();
	}

	void Pop(bool refreshPreviousPanel=false) // if true, the panel that will be displayed is refreshed, only useful if things may have changed in this panel while it was "under" 
	{
		if (children.size() == 0) return;
		children.back()->DeleteMe();

		if (refreshPreviousPanel)
			if (children.size() != 0)
				RefreshUI_Fast(children.back());

	}

	// NOTE: if you give a "onNativeBackButtonPressed", you must call Pop() yourself in it (or not if you want to prevent the pop)
	void Push(wWidget *toAdd, std::function<void()> onPopped=nullptr, bool dontSetOpaqueToTouch=false)
	{
		if (toAdd->touchableAdded==false) // the user may have set a touchable/clickable, we don't want to remove it
            if (dontSetOpaqueToTouch==false)
                toAdd->SetOpaqueToTouch();  // default is new panel on the stackpanel won't let the touch events through


		this->AddChild_private(toAdd);


		if (parent != nullptr)
		{

			toAdd->Xem(FILL);
			toAdd->Yem(FILL);

#define WSTACKPANEL_OPTIM_TEST
#ifndef WSTACKPANEL_OPTIM_TEST
            // full refresh
            // RefreshUI_Fast(this); // note: RefreshUI_Fast is ok here because we know the size of "this" won't change
#else
            // optim 
			if (this->ax[X].size != -666 && this->ax[Y].size != -666)
			{
				toAdd->SetEm(X, this->GetEm(X));
				toAdd->SetEm(Y, this->GetEm(Y));
				toAdd->ax[X].size = this->ax[X].size;
				toAdd->ax[Y].size = this->ax[Y].size;
				toAdd->ax[X].pos = pixels(0);
				toAdd->ax[Y].pos = pixels(0);
				toAdd->Reset_setPosHw_prev_values();
				RefreshUI_Fast(toAdd); // refresh only the added view
			}
			else
				RefreshUI_Fast(this); // this will refresh all the panels
						
#endif
            
		}

		;
	}

	static wStackPanel *Create(Length xem, Length yem) { auto c = new wStackPanel(); c->Xem(xem)->Yem(yem); return c; }
};

// in stdw?
class wTabPanel : public wEmpty
{

public:
	wTabPanel() : wEmpty()
	{

	}

	void Add(wWidget *toAdd)
	{
		this->AddChild_private(toAdd);
		if (children.size() > 1)
			SetChildVisibility((int)children.size() - 1, false);
		if (parent != nullptr)
		{
			// TODO: is it normal to do this manually here ? shouldn't we just set it "FILL,FILL" ?
			toAdd->ax[X].pos = pixels(0);
			toAdd->ax[Y].pos = pixels(0);
			toAdd->ax[X].size = this->ax[X].size;
			toAdd->ax[Y].size = this->ax[Y].size;

			// isn't this enough?
			toAdd->Xem(FILL);
			toAdd->Yem(FILL);

			RefreshUI_Fast(this); // note: RefreshUI_Fast is ok here because we know the size of "this" won't change
		}
	}

	void Select(int idx) // TODO: shouldn't we refresh the tab that becomes visible? if the user did something on a non visible page while he was on a visible page?
	{
		assert(idx < children.size());
		int i = 0;
		for (wWidget *w : children) // hide the not selected views
		{
			if (i != idx)
            {
				SetChildVisibility(i, false);
            }
			i++;
		}
		i = 0;
		for (wWidget *w : children) // show the selected view (last to be sure refresh is ok; not sure it's useful though)
		{
			if (i == idx)
            {
				SetChildVisibility(i, true);
            }
			i++;
		}
	}


	static wTabPanel *Create(Length xem, Length yem) { auto c = new wTabPanel(); c->Xem(xem)->Yem(yem); return c; }
};






} // namespace cui