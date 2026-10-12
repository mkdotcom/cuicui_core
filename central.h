#pragma once


#include "cuicui.h"

namespace cui {
class cCentral
{
public:
	enum class VirtualKeyboardScreenMode : unsigned char {
		Resize = 0, // default, the app resizes to avoid keyboard
		Pan = 1, // move all app up to avoid keyboard
		Custom = 2, // NYI !
		Nothing = 3, // 
	};


public:
	cFont *defaultFont = nullptr;
	pixels defaultEmSize;
	bool inited = false;
#ifdef CUICUI_ANDROID
	std::function<void()> androidMessageBoxOkCancel_onOk = nullptr;
	std::function<void()> androidMessageBoxOkCancel_onCancel = nullptr;
#endif
	std::shared_ptr<cFileReceived> initialPayload = nullptr; // TODO: we could do like that :when user do onFileReceived = []() {} then send it automaticaly the initialPayload
															 // so user doesn't have to handle "initpayload" at all
															 // but what if user need the payload in appstart() "synchronously" ?
	// put wWidget *master here?
	pixels currentScreenWidth;
	pixels currentScreenHeight;
	std::string folderTemp, folderAssets, folderDocuments;
    cRect safeAreaRect; // pixels

public: //protected:
    wWidget *iosPanGestureStarted=nullptr;
    
public: //protected:
    // TODO_81412: currently on android we hook the event so the standard volume won't display in any cuicui app ! We should capture the volumes keys events ONLY if user wants to get them
    // make a central::CaptureVolumesKeyEvents(std::function<>down, std::function>up>) that sends a "capturevolumeevents=true" on the android side; and onKeyDown() will return true or false accordingly
	std::function<void()> onVolumeDownKeyPressed=nullptr; // android only (for now)
	std::function<void()> onVolumeUpKeyPressed=nullptr; // android only (for now)

#ifdef CUICUI_ANDROID
	bool isCurrentlyScreenInPortrait = true; // TODO: should be undefined
#endif

protected:
	VirtualKeyboardScreenMode virtualKeyboardScreenMode = VirtualKeyboardScreenMode::Resize;

public:
	std::list<wWidget*> hardwareBackButtonLinked; // wWidget need access

public:
	cCentral()
	{
        safeAreaRect.Set(0,0,0,0);
	}
	~cCentral()
	{
		if (defaultFont != nullptr)
		{
			delete defaultFont;
			defaultFont = nullptr;
		}
	}
	void Init();
	bool Inited()
	{
		return inited;
	}

	enum class eAppState
	{
		Foreground=0, // ok: UWP, IOS, WEB, Android
		Background=1, // ok: UWP, IOS, WEB, Android
        Ending=2, // ok: IOS
				
		
		// Background: ---> do we always get this when the app terminates?
						   // TODO: to validate on every platform, then document it (apps rely on it to save their state)
												 // UWP : YES, confirmed
												 // Android: ?
												 // web: ?
												 // ios: ?
		
		// Note: background means "not active", on uwp/macos if another window has the focus, your app is "background" even if it's still visible

		// WARNING WEB:
			// the event will be raised if you change tab or minimize/maximize the browser
			// BUT if you are using your browser and that your app is in the active tab, if you go to another app and come back to the browser : the even WILL NOT be raised
			// TODO_we_appstate_issue_451: can this be fixed?

		// UWP: is "background" called when the app goes "suspended" ? TODO: I think so but it should be tested
		
		// TODO web: https://developer.mozilla.org/en-US/docs/Web/Events/visibilitychange
		
	};
	
    std::function<void(eAppState newState)> onAppStateChanged = nullptr; 

	// filter: "images" (images only) or "" (all files)
	// onDisplayed: will trigger when the picker is displayed; this is useful because on iOs the picker is very slow to appear (the first time): user can display a "please wait"
	void OpenFilePicker(const std::string &filter, std::function<void()> onDisplayed=nullptr); 

    void OpenCamera(); // ANDROID and IOS only	(for now)
	bool DeviceHasCamera(); // returns true on ANDROID and IOS; TODO: fix this code to really check for a camera on the device
    std::string GetDocumentFolderPath() { return folderDocuments; } // use this if you need to access the user document folder
	std::string GetTempFolderPath() { return folderTemp; } // use this if you need to access the temporary folder with fopen() or similar
    std::string GetAssetsFolderPath() { return folderAssets; } // use this if you need to access a file in your app assets folder through fopen() or similar; Note: don't use it with wImage
	void MessageBox(const std::string &message, std::function<void()> onOk = nullptr); // Very simple native messagebox displaying only a message (no title) and a "ok" button. Note: you probably should only use it for debug because behavior may differs on each platforms
	void MessageBoxOkCancel(const std::string& message, std::function<void()> onOk=nullptr, std::function<void()> onCancel = nullptr); // note: read notes about MessageBox()
	void StopApp(); // note: has no effect on iOS (yet?)
	void SetMouseCursor(const std::string type);
	void OpenExternalApp(const std::string &uri);
#if defined CUICUI_IOS
    void IosSpecialSettings(cColor insetsColor, UIStatusBarStyle statusBarStyle); // IOS only: insetsColor: bgcolor in the "safe area" (what is the real name of this zone?)
#endif
#if defined CUICUI_ANDROID
	void AndroidSimpleToast(const std::string &message, bool shortDuration=true); // ANDROID only
#endif
	bool IsScreenPortrait();
	bool IsScreenLandscape();
	void SendTextToClipboard(const std::string& str);
	void GetTextFromClipboard(std::function<void(std::string)> resultFunc); // WARNING: on android it works only if you call it from an user input action ! TODO: should return true/false, and a detailed error message maybe (if it 's possible to ask android if he will accept the call)(it throws an exception I presume if it agrees?)
	void OpenOsAppSettingsPage(); // Do nothing on WEB
	bool IsTouchDevice();
	bool IsMouseDevice();
	void PreloadAllImagesAssets(); // NYI
	void ShowVirtualKeyboard(bool show); // android/ios only, has no effect on UWP/WEB/MacOS
	void SetAppTitle(const std::string &title); // WEB and UWP only, has no effect on other platforms
												// Note_4814: on UWP it will be "title - yourappname"... Let's hope microsoft change this behaviour someday...
	void SetAppIconBadgeNumber(int number);
	void SetVirtualKeyboardScreenMode(VirtualKeyboardScreenMode mode) // ANDROID and IOS only, had no effect on other platforms, default is Resize
	{
#if defined(CUICUI_ANDROID) || defined(CUICUI_IOS)
		virtualKeyboardScreenMode = mode; 
#else
		CUICUI_NOP
#endif		
	}
	VirtualKeyboardScreenMode GetVirtualKeyboardScreenMode()
	{
#if defined(CUICUI_ANDROID) || defined(CUICUI_IOS)
		return virtualKeyboardScreenMode;
#else
		return VirtualKeyboardScreenMode::Nothing;
#endif
	}

	double /*ems*/ GetScreenWidthInEm();
	double /*ems*/ GetScreenHeightInEm();

protected:
	void SetupDefaultFont();
	std::string Private_GetTempFolderPath(); // use this if you need to access the temporary folder with fopen()
	std::string Private_GetAssetsFolderPath(); // use this if you need to access a file in you assets folder through fopen(); Note: don't use it with wImage
	std::string Private_GetUserDocumentFolderPath(); // use this if you need to access or store a file in the user documents folder
};

extern cCentral central;

} // namespace cui