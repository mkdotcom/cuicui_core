#pragma once

#ifdef CUICUI_IOS
namespace cui {
class wTextInput;
} // namespace cui
@interface CuiUITextFieldEventClass : NSObject <UITextFieldDelegate>
-(void)setTi : (cui::wTextInput*)_ti;

@end
#endif

// note: simplest possible textinput; if you need padding, clearbutton, or others things -> you can create your own wMyTextInput by inheriting this one
namespace cui {
class wTextInput : public wWidget
{
public:
	std::string currentText;
	bool enabled = true;
#if defined(WINAPI_FAMILY)
	TextBox ^textBox;
	PasswordBox ^passwordBox;
#endif
#if defined(CUICUI_IOS)
	UITextField *uiTextField = nullptr;
	CuiUITextFieldEventClass *theCuiUITextFieldEventClass=nullptr;
	UITextView *uiTextViewMultiline = nullptr;
#endif

	enum class eType //better to never change the ids (for example WEB adn ANDROID uses the ids)
	{
		Text=0,
		MultilineText=1,
		Password=2,
		Invalid=-1
	};

	eType type = eType::Invalid;
	std::function<void(std::string text)> onTextChanged = nullptr;
	std::function<void()> onEnterKeyPressed = nullptr;

    
    // WARNING! this is a "SINGLE LINE" TextInput
    // if you paste multiline text in it, the "return to lines" will be removed
    // this behavior is confirmed on all platforms:
    // [X] ios
    // [X] web
    // [X] android (note: textinput shows everything on a single line, though it actually doesn't remove the "return to lines")
    // [X] uwp
    // [ ] macos
	

public:
	wTextInput(eType a_type = eType::Text);
	wTextInput* TextColor(cColor color);
	void EnablePasteFile(); // a "pasted" (ctrl+v) file (or direct data if the user put a piece of image in the clipboard, for example with "snapin" app on windows) will be received by the app in the onFileReceived event	
	int GetCursorPos();
	void SetFocus();
	void SetCursorPos(int pos);
	void InsertText(int pos, const std::string &text);	
	void UpdateRealFontSize(); // DUPLICATE_5671		
	wTextInput* SetText(const std::string &str);
	void Clear() { SetText(""); }
	std::string GetText() { return currentText; }
	wTextInput* PlaceholderText(const std::string &str);
	wTextInput* Enabled(bool b); // true: the user can interact with the wTextInput; default is true 
								 // NOTE: this may unset the focus on some platforms, you should call SetFocus() after Enabled(true) if you want to be sure the focus stays
								 // On some platform (android, maybe ios) loosing focus will auto hide the virtual keyboard
	bool Enabled() { return enabled;  }
	bool isPasswordBox() { return type == eType::Password; } // deprecated, will be removed soon
	wTextInput* DisableReturnKey(); // if eType is MultilineText, user can press the "return" key to go to next line, this method disable this (note: user will still be able to paste multiline texts) 

	virtual bool ComputeMinSize(double *w, double *h);


	/*#if defined(__EMSCRIPTEN__) test to have the function callable from web in the class (doesn't work)
		static __cdecl static int test(int widgetPtr)
		{
			auto self = (wTextInput*)widgetPtr;
			if (self->onEnterKeyPressed != nullptr) self->onEnterKeyPressed();
		}
	#endif*/
		
public:
	static wTextInput *Create(Length xem, eType a_type = eType::Text) { auto c = new wTextInput(a_type); assert(xem.IsMin()==false); c->Xem(xem)->Yem(EM_AUTO); return c; }
};



} // namespace cui