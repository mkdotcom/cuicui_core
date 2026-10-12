#include "central.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)


#ifdef CUICUI_UWP
namespace cui {
extern task<void> UwpPickAnImageAndUpload(std::string);
} // namespace cui
#endif

#ifdef CUICUI_IOS
#import "ios/cuicui_ios_host.h" // the app's root view controller: pickers, camera, status bar
#endif


namespace cui {
cCentral central;


void cCentral::OpenOsAppSettingsPage()
{
#if defined(CUICUI_ANDROID)
	JNICall_Void_("OpenOsAppSettingsPage");
#elif defined(CUICUI_UWP)
	// https://docs.microsoft.com/fr-fr/windows/uwp/launch-resume/launch-settings-app
	Uri ^u = ref new Uri(StdStringToUwpString("ms-settings:appsfeatures-app"));
	Windows::System::Launcher::LaunchUriAsync(u);
#elif defined(CUICUI_WEB)
	// not available
#elif defined(CUICUI_IOS)
	if (UIApplicationOpenSettingsURLString != NULL)
	{
		NSURL *url = [NSURL URLWithString : UIApplicationOpenSettingsURLString];

        [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:^(BOOL success) {
            if (success)
                CUICUI_NOP
            else
                CUICUI_NOP
        }];
	}
	else
	{
		// probably can't happen
	}
	//https://stackoverflow.com/questions/23824054/how-to-open-settings-programmatically-like-in-facebook-app
	//https://developer.apple.com/documentation/uikit/uiapplicationopensettingsurlstring?language=objc
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif

}

void cCentral::StopApp()
{
#if defined(CUICUI_ANDROID)
	JNICall_Void_("cui_Exit");
#elif defined(CUICUI_UWP)
	auto c = Windows::UI::Xaml::Application::Current;
	c->Exit();
#elif defined(CUICUI_WEB)
	EM_ASM_({ window.close(); });
#elif defined(CUICUI_IOS)
	LogWarning("Exit() not available on IOS");
	// NOP: I'm not sure we need this on iOS; we'll see when we implement "share"
#else
	missing
#endif
}

#ifdef CUICUI_ANDROID
void cCentral::AndroidSimpleToast(const std::string &message, bool shortDuration)
{
	AssertOnUiThread();

	// method 1
	//JNICall_Void_StringBool("cuiCentralSimpleToast", message, shortDuration);

    // method 2 : crashes if called in a GestureTap (TODO_57487)
	JniCall cc(globalCuiJniEnv, "io/github/mkdotcom/cuicui/CuicuiActivity", "cuiCentralSimpleToast");
	cc.str(message);
	cc.b(shortDuration);
	cc.CallVoidMethod(globalCuiJniObj);
	cc.CleanUp();
}
#endif

#if defined(CUICUI_UWP)	
task<void> GetTextFromClipboard_UWP_ResultReceiver(std::function<void(std::string)> resultFunc)
{
	using namespace Windows::ApplicationModel::DataTransfer;
	auto dataPackage = Clipboard::GetContent();
	auto ret = co_await dataPackage->GetTextAsync();
	
	resultFunc(UwpStringToStdString(ret));
}
#endif

void cCentral::GetTextFromClipboard(std::function<void(std::string)> resultFunc)
{
#if defined(CUICUI_ANDROID)
    std::string str = JNICall_String_Int("cuiCentralGetTextFromClipboard",0);
    resultFunc(str);

#elif defined(CUICUI_UWP)	
	GetTextFromClipboard_UWP_ResultReceiver(resultFunc);
#elif defined(CUICUI_WEB)
	LogWarning("cCentral::GetTextFromClipboard is not yet available on WEB cuicui."); // TODO web 
#elif defined(CUICUI_IOS)
    assert(false); //missing
#elif defined(CUICUI_MACOS)
	missing
#else
	non_implemented_platform
#endif
}

void cCentral::SendTextToClipboard(const std::string& str)
{
#if defined(CUICUI_ANDROID)
	JniCall cc(globalCuiJniEnv, "io/github/mkdotcom/cuicui/CuicuiActivity", "cuiCentralSendTextToClipboard");	
	cc.str(str);
	cc.CallVoidMethod(globalCuiJniObj);
	cc.CleanUp();

#elif defined(CUICUI_UWP)
	using namespace Windows::ApplicationModel::DataTransfer;
	auto dataPackage = ref new DataPackage();
	dataPackage->RequestedOperation = DataPackageOperation::Copy;
	dataPackage->SetText(StdStringToUwpString(str));
	Clipboard::SetContent(dataPackage);
#elif defined(CUICUI_WEB)
	LogWarning("cCentral::SendTextToClipboard is not yet available on WEB cuicui."); // TODO web 
#elif defined(CUICUI_IOS)
    assert(false); //missing
#elif defined(CUICUI_MACOS)
	missing
#else
	non_implemented_platform
#endif
}

std::string cCentral::Private_GetAssetsFolderPath()
{
    AssertOnUiThread();

#if defined(CUICUI_ANDROID)
    std::string d = JNICall_String_String("GetSpecialDirectory","assets");
    Log("cCentral::GetAssetsFolderPath() -> " + d);
    return d;
#elif defined(CUICUI_UWP)
    auto ss = Windows::ApplicationModel::Package::Current;
    auto ss1 = ss->InstalledLocation;
    auto ss2 = ss1->Path;
    std::string ss3 =  UwpStringToStdString(ss2) + "\\mkw_assets\\";
    return ss3;
#elif defined(CUICUI_WEB)
    return "not available on web (yet)"; // TODO
#elif defined(CUICUI_IOS)
    NSString *path = [[NSBundle mainBundle] bundlePath];
    std::string p = NSStringToStdString(path);
    
    if (NSProcessInfo.processInfo.isiOSAppOnMac == false && NSProcessInfo.processInfo.isMacCatalystApp == true)
    {
        // here it's for catalyst only (not for "designed for ipad" app)
        NSLog(@"checking platform: mac catalyst");
        return p + "/Contents/Resources/assets/"; // mac catalyst
    }
    else
    {
        NSLog(@"checking platform: ios");
        return p + "/assets/"; // ios
    }
#elif defined(CUICUI_MACOS)
    assert(false);
#else
    missing
#endif
    return "";
}


std::string cCentral::Private_GetTempFolderPath()
{
	AssertOnUiThread();

#if defined(CUICUI_ANDROID)
	std::string d = JNICall_String_String("GetSpecialDirectory","temp");
	Log("cCentral::GetTempFolderPath() -> " + d);
	return d;
#elif defined(CUICUI_UWP)
	auto a = Windows::Storage::ApplicationData::Current;
	auto ss = a->TemporaryFolder;	
	auto ss2 = ss->Path;
	std::string ss3 = UwpStringToStdString(ss2) + "\\";
	return ss3;
#elif defined(CUICUI_WEB)
	return "not available on web (yet)"; // TODO
#elif defined(CUICUI_IOS)
	NSString *s = NSTemporaryDirectory(); // note that it can be cleaned by the OS very quickly (not so much on ios but very often on mac catalyst)
    return NSStringToStdString(s);
#elif defined(CUICUI_MACOS)
	assert(false);
#else
	missing
#endif
	return "";
}

std::string cCentral::Private_GetUserDocumentFolderPath()
{
    AssertOnUiThread();

#if defined(CUICUI_ANDROID)
    std::string d = JNICall_String_String("GetSpecialDirectory","document");
    Log("cCentral::GetUserDocumentFolderPath() -> " + d);
    return d;
#elif defined(CUICUI_UWP)
    assert(false); // nyi
    auto a = Windows::Storage::ApplicationData::Current;
    auto ss = a->TemporaryFolder;
    auto ss2 = ss->Path;
    std::string ss3 = UwpStringToStdString(ss2) + "\\";
    return ss3;
#elif defined(CUICUI_WEB)
    return "not available on web (yet)"; // TODO
#elif defined(CUICUI_IOS)
    NSString *s = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES) objectAtIndex:0];
    return NSStringToStdString(s);
#elif defined(CUICUI_MACOS)
    assert(false);
#else
    missing
#endif
    return "";
}


bool cCentral::DeviceHasCamera()
{
	// TODO: fix this code to really check for a camera on the device

	// fake code:
#if defined CUICUI_ANDROID || defined CUICUI_IOS
	return true;
#else
	return false;
#endif
}

void cCentral::OpenCamera()
{
	if (DeviceHasCamera() == false) return;

#ifdef CUICUI_ANDROID
    JNICall_Void_("cuiOpenCamera");
	return ;
#endif
#ifdef CUICUI_IOS
    UIViewController *vc = master->uiView.window.rootViewController;
    UIViewController<CuiHostViewController> *v = (UIViewController<CuiHostViewController>*)vc;
    [v openCamera];
	return ; // should return "false" if app doesn't have the right, or device have no camera...  (I guess it will be asycn)
#endif
    // other platforms: not available
	return ;
}

void cCentral::OpenFilePicker(const std::string &filter, std::function<void()> onDisplayed)
{
	AssertOnUiThread();

	assert(filter == "" || filter == "images");

#if defined(CUICUI_ANDROID)
	JNICall_Void_IntString("OpenFilePicker", -1, filter);
	if (onDisplayed) onDisplayed(); // temporary
#elif defined(WINAPI_FAMILY)				
	UwpPickAnImageAndUpload(filter); 
	if (onDisplayed) onDisplayed(); // temporary
#elif defined(__EMSCRIPTEN__)				
	EM_ASM_(
		{
			var input = document.getElementById("webFilePickerInputElement");
			if (input == null) {
				Module.print("error 771");
				return;
			}
			var filter = UTF8ToString($0);
			if (filter == "images")
				input.accept = "image/*";
			else
				input.accept = "";
			input.click();
		}, filter.c_str());	
	if (onDisplayed) onDisplayed(); // temporary
#elif defined(CUICUI_IOS)
	UIViewController *vc = master->uiView.window.rootViewController;
	UIViewController<CuiHostViewController> *v = (UIViewController<CuiHostViewController>*)vc;
    if (filter=="")
        [v openPickerDocuments];
    else
        [v openPickerImages];
    if (onDisplayed) onDisplayed(); // temporary
#else
	missing 
#endif
}

void cCentral::MessageBoxOkCancel(const std::string& message, std::function<void()> onOk, std::function<void()> onCancel)
{
	RunOnUiThread([this, message, onOk, onCancel]()
	{
#if defined(CUICUI_ANDROID)		
		JNICall_Void_String("ShowSimpleMessageBoxYesNo", message);
		androidMessageBoxOkCancel_onOk = onOk;
		androidMessageBoxOkCancel_onCancel = onCancel;
#elif defined(CUICUI_UWP)

		using namespace Windows::UI::Popups;
		using namespace Windows::UI::Xaml;
		using namespace Windows::UI::Xaml::Controls;
		using namespace Windows::UI::Xaml::Navigation;

		MessageDialog ^msg = ref new MessageDialog{ StdStringToUwpString(message) };
						
		UICommand^ continueCommand = ref new UICommand(
			"Ok",
			ref new UICommandInvokedHandler([this, onOk](IUICommand ^command)
			{
				if (onOk) onOk();
			}));
		UICommand^ upgradeCommand = ref new UICommand(
			"Cancel",
			ref new UICommandInvokedHandler([this, onCancel](IUICommand ^command)
			{
				if (onCancel) onCancel();
			}));
			
		msg->Commands->Append(continueCommand);
		msg->Commands->Append(upgradeCommand);

		// Set the command that will be invoked by default
		msg->DefaultCommandIndex = 0;

		// Set the command to be invoked when escape is pressed
		msg->CancelCommandIndex = 1;

		msg->ShowAsync();
#elif defined(CUICUI_WEB)
		int ret = EM_ASM_INT({ 
			var str = UTF8ToString($0);
		if (confirm(str))
			return 1; // ok
		else
			return 0; // cancel
			}, message.c_str());
		if (ret == 1 && onOk != nullptr)
			onOk();
		if (ret == 0 && onCancel != nullptr)
			onCancel();
#elif defined(CUICUI_IOS)
		// Note: Using a timer here because in some cases (after selecting a image in the gallery popup for example)
		//       the messagebox won't show. The timer fix this.
        timerManager.Create(0.01, [message, onOk](Timer *t)
        {
			NSString* s = StdStringToNSString(message);
			UIAlertController* alert = [UIAlertController alertControllerWithTitle : @"Message"
				message : s
				preferredStyle : UIAlertControllerStyleAlert];

			UIAlertAction* defaultAction = [UIAlertAction actionWithTitle : @"OK" style : UIAlertActionStyleDefault
				handler : ^ (UIAlertAction * action)
                                            {
                                             RunOnUiThread([onOk]()
    			{
                  onOk();
					});
                                            }];

            // cancel not handled on ios for now!
            
			[alert addAction : defaultAction] ;

			UIViewController* vc = master->uiView.window.rootViewController;
			[vc presentViewController : alert animated : YES completion : nil] ;
        });
#elif defined(CUICUI_MACOS)
		assert(false);
#else
		missing
#endif
	});
}

void cCentral::MessageBox(const std::string &message, std::function<void()> onOk)
{
	RunOnUiThread([this, message, onOk]()  
	{
#if defined(CUICUI_ANDROID)	
		JNICall_Void_String("ShowSimpleMessageBox", message);
#elif defined(CUICUI_UWP)
		Windows::UI::Popups::MessageDialog msg{ StdStringToUwpString(message) };
		msg.ShowAsync();
#elif defined(CUICUI_WEB)
		EM_ASM_({
			var str = UTF8ToString($0);
			alert(str);
			}, message.c_str());
#elif defined(CUICUI_IOS)
		// Note: Using a timer here because in some cases (after selecting a image in the gallery popup for example)
		//       the messagebox won't show. The timer fix this.
		timerManager.Create(0.01, [message, onOk](Timer *t)
        {
			NSString* s = StdStringToNSString(message);
			UIAlertController* alert = [UIAlertController alertControllerWithTitle : @"Message"
				message : s
				preferredStyle : UIAlertControllerStyleAlert];

			UIAlertAction* defaultAction = [UIAlertAction actionWithTitle : @"OK" style : UIAlertActionStyleDefault
				handler : ^ (UIAlertAction * action)
                {
				 RunOnUiThread([onOk]()
    			{
                  onOk();
					});
                }];

			[alert addAction : defaultAction];

			UIViewController* vc = master->uiView.window.rootViewController;
			[vc presentViewController : alert animated : YES completion : nil] ;
		});
#elif defined(CUICUI_MACOS)
		assert(false);
#else
		missing
#endif
	});
}

void cCentral::SetMouseCursor(const std::string type)
{	
	AssertOnUiThread();

	assert(type == "default" || type == "hand" || type == "resize");

#if defined(CUICUI_UWP)
	CoreWindow ^window = CoreWindow::GetForCurrentThread();
	if (type == "default")
		window->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::Arrow, 0);
	if (type == "hand")
		window->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::Hand, 0);
	if (type == "resize")
		window->PointerCursor = ref new Windows::UI::Core::CoreCursor(Windows::UI::Core::CoreCursorType::SizeAll, 0);

#endif
#if defined(CUICUI_WEB)
	if (type == "hand")
		EM_ASM_({
				document.body.style.cursor = 'pointer';
			});
	if (type == "default")
		EM_ASM_({
				document.body.style.cursor = 'auto';
			});

	if (type == "resize")
		EM_ASM_({
				document.body.style.cursor = 'ns-resize'; // not tested
			});
#endif

	// other platforms: no mouse cursor
}

double cCentral::GetScreenWidthInEm()
{
	if (master == NULL) return -666;
	double v = currentScreenWidth.asDouble() / GetEmSize(1);
	return v;
}

double cCentral::GetScreenHeightInEm()
{
	if (master == NULL) return -666;
	double v = currentScreenHeight.asDouble() / GetEmSize(1);
	return v;
}

bool cCentral::IsMouseDevice()
{
	return !IsTouchDevice();
}

bool cCentral::IsTouchDevice() // warning: must be checked every time because it can change while the app is running (I plug in a mouse for example; or I switch to "mobile simulation" mode in chrome/firefox)
{
	// TODO: will probably need to ask the os rather than using the defines
	// --> very later: actually maybe we should know it at touch/click time? annoying because the ui potentially changes depending on touch or mouse (example: auto-hiding image in an app)
#if defined(CUICUI_ANDROID)
	return true; // not necessarily true (tablet with mouse?)
#endif
#if defined(CUICUI_IOS)
	return true;
#endif
#if defined(CUICUI_WEB)
	// TODO: return true if mobile mode
	//
#endif
	return false;
}


void cCentral::OpenExternalApp(const std::string &uri)
{
	AssertOnUiThread();

#if defined(CUICUI_ANDROID)
	JNICall_Void_String("OpenExternalApp", uri);
#elif defined(CUICUI_UWP)
	Uri ^u = ref new Uri(StdStringToUwpString(uri));
	Windows::System::Launcher::LaunchUriAsync(u);
#elif defined(__EMSCRIPTEN__)	
	EM_ASM_({
		var str = UTF8ToString($0);
	console.log("openapp: " + str);
	window.open(str, '_blank');
		}, uri.c_str());
#elif defined(CUICUI_IOS)
	NSString *customURL = StdStringToNSString(uri);
	customURL = [customURL stringByTrimmingCharactersInSet :
	[NSCharacterSet whitespaceAndNewlineCharacterSet]];

	if ([[UIApplication sharedApplication] canOpenURL:[NSURL URLWithString : customURL]])
	{
        [[UIApplication sharedApplication] openURL:[NSURL URLWithString : customURL]
                                           options:@{}
                                           completionHandler:^(BOOL success) {
                   if (success)
                       CUICUI_NOP
                   else
                       CUICUI_NOP
               }];
	}
	else
	{
		Log("IOS OpenExternalApp error 84839");
		//return false (TODO?)
	}
#else
	missing
#endif
}

void cCentral::ShowVirtualKeyboard(bool show) // Has no effect on UWP/WEB/MacOS
{
	assert(show == false); // true is not handled (but it will pop up when edit text is focused)

#ifdef CUICUI_ANDROID
	JNICall_Void_("hideSoftKeyboard");
#endif
#ifdef CUICUI_IOS
	// TODO :
	// [textField resignFirstResponder]; only solution? if yes we need to keep the last focused ti
	// (AND beware of the lifetime of that ti)
	Log("IOS TODO show hide keyboard");
#endif
}

void cCentral::SetAppTitle(const std::string & title) 
{
#ifdef CUICUI_WEB
	EM_ASM_({
		var msg = UTF8ToString($0);
	document.title = msg;
		}, title.c_str());
#endif
#ifdef CUICUI_UWP
	Windows::UI::ViewManagement::ApplicationView ^appView = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
	appView->Title = StdStringToUwpString(title);
#endif
	// macos?
}

bool cCentral::IsScreenPortrait()
{
#ifdef CUICUI_ANDROID	
	bool isInPortrait = JNICall_Bool_("cui_isDeviceInPortrait");
	return isInPortrait;
#else
	double emWidth = currentScreenWidth.asDouble();
	double emHeight = currentScreenHeight.asDouble();
	if (emWidth > emHeight) return false;
	return true;
#endif
}
bool cCentral::IsScreenLandscape()
{
	return !IsScreenPortrait();
}

void cCentral::Init()
{
	// we do that here because some platforms need to be on UI thread to retreive these folders, it's better to do it here so user don't need to use RunOnUiThread 
	folderTemp = Private_GetTempFolderPath();
	folderAssets = Private_GetAssetsFolderPath();
    folderDocuments = Private_GetUserDocumentFolderPath();

	LogD("DEBUG: temp folder: " + folderTemp);
	LogD("DEBUG: assets folder: " + folderAssets);

	inited = true;
	SetupDefaultFont();
}

void cCentral::SetupDefaultFont()
{
	assert(inited);
	Log("SetupDefaultFont start...");
	cFont *f = new cFont();
	defaultFont = f;


	// NOTE: we measure "█" ( U+2588 -> 'FULL BLOCK') which is a "full height character", so we can get the height of a line
	// BUT!! on web we use a smiley to compute the height, because u+2588 is actually not "full height" on web (PROBABLY depends on the font)
	// There is probably a better way to do that (the font can probably give us this height directly) -> CLEANUP

#ifdef CUICUI_IOS
	f->native = [UIFont preferredFontForTextStyle : UIFontTextStyleBody];
	// TODO: ios: we must listen to the "dynamic font changed" event, update this, and do a full refresh ?

	f->sizeInPoints = [f->native pointSize];

	double d = [f->native lineHeight];
	defaultEmSize.setFromDouble(d);
#elif defined(CUICUI_UWP) 
	// CLEANUP: isn't there a better way to do this? (a bit like we do on ios)

	Size s2((float)10000, (float)1000);
	TextBlock ^temp = ref new TextBlock(); // TODO: make it a global that we use every time?
	temp->Text = "█"; // U+2588 -> 'FULL BLOCK' 
	temp->TextWrapping = TextWrapping::NoWrap;

	f->native = temp->FontFamily;
	f->fontSizeInPixels = temp->FontSize;

	temp->Measure(s2); 
	defaultEmSize.setFromDouble(temp->DesiredSize.Height);
	delete temp;
#elif defined(CUICUI_WEB)
	/*unsigned int retVal = EM_ASM_INT({
		// none of this seems to work...
			var theSpan = document.createElement('span');
			theSpan.innerHtml = "█";
			var fs = theSpan.style["font-size"];
			console.log("fs=" + fs);
			var ff = window.getComputedStyle(document.createElement('div')).fontFamily;
			console.log("ff=" + ff);
			var fs = window.getComputedStyle(document.createElement('div')).fontSize;
			console.log("fs=" + ff);
			return fs;
	});*/

	// CLEANUP: isn't there a better way to do this? (a bit like we do on ios)
	unsigned int retVal = EM_ASM_INT({
		var result = computeTextHeight("😁", 1000, 0);
		return result;
		});
	int computedWidth = (retVal >> 16);
	int computedHeight = (retVal & 0x0000ffff);	

	f->fontSizeInPixels = 16;  // TODO: always the default in browser ? I'd be surprised
	defaultEmSize.setFromDouble(computedHeight);
	// NOTE: f->fontSizeInPixels and computedHeight ARE NOT identical
#elif defined(CUICUI_ANDROID)
	// CLEANUP: isn't there a better way to do this? (a bit like we do on ios)

	// possible java code to get the current font size:
	//TextView tmp = new TextView(this);
	//df = tmp.getTextSize();

	int v = JNICall_Int_Void("cui_GetDefaultFontSizeInPixels");
	f->fontSizeInPixels = v;

	v = JNICall_Int_Void("cui_GetDefaultFontLineHeight");
	defaultEmSize.setFromDouble(v);
#else
	missing
#endif            

		Log("SetupDefaultFont done.");
	return;
}

#ifdef CUICUI_IOS
void cCentral::IosSpecialSettings(cColor color, UIStatusBarStyle statusBarStyle)
{
    assert(master!=nullptr); // you must call this method after master view is created

    UIViewController *vc = [UIApplication sharedApplication].keyWindow.rootViewController;
    UIViewController<CuiHostViewController> *v = (UIViewController<CuiHostViewController>*)vc;
    [v setCurrentStatusBarStyle:statusBarStyle];
    UIView *uiView = v.view;
    assert(uiView!=nullptr); // app not ready yet?
    [uiView setBackgroundColor : [UIColor colorWithRed : (float)color.cr / 255.0f green : (float)color.cg / 255.0f blue : (float)color.cb / 255.0f alpha : color.alpha]];
    [uiView setOpaque:YES];
}
#endif

void cCentral::PreloadAllImagesAssets()
{
	CUICUI_NYI
}

void cCentral::SetAppIconBadgeNumber(int number) // UWP ONLY // 0=remove badge; if number is >99 badge will display "99+" 
{
	// TODO: check why there's an exception with the windows tablet simulator
#ifdef CUICUI_UWP
	try
	{
		if (number < 0) number = 0; 

		using namespace Windows::UI::Notifications;
		using namespace Windows::Data::Xml::Dom;

		// Get the blank badge XML payload for a badge number
		XmlDocument ^badgeXml = BadgeUpdateManager::GetTemplateContent(BadgeTemplateType::BadgeNumber);

		// Set the value of the badge in the XML to our number
		XmlElement ^badgeElement = (XmlElement^)badgeXml->SelectSingleNode("/badge");
		badgeElement->SetAttribute("value", StdStringToUwpString(std::to_string(number)));

		// Create the badge notification
		BadgeNotification ^badge = ref new BadgeNotification(badgeXml);

		// Create the badge updater for the application
		BadgeUpdater ^badgeUpdater = BadgeUpdateManager::CreateBadgeUpdaterForApplication();

		// And update the badge
		badgeUpdater->Update(badge);
	}
	catch (Exception ^ex)
	{
		return;
	}
#endif
#ifdef CUICUI_MACOS
	// todo
#endif
}

} // namespace cui