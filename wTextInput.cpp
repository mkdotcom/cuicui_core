#include "cuicui.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include "wTextInput.h"


#ifdef CUICUI_UWP
extern Platform::Object^ GetAppStyle(const std::string &styleName);
#endif


#if defined(CUICUI_IOS)
// TODO: uitextview did change
@implementation CuiUITextFieldEventClass {
    wTextInput *ti; // TODO: nullptr
}
-(void)setTi:(wTextInput*)_ti {
    ti = _ti;
}
- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    [textField resignFirstResponder];
    return YES;
}
- (void)textFieldDidEndEditing:(UITextField *)textField {
    [textField resignFirstResponder];
}
- (void)textFieldDidChange:(UITextField *)textField {
    assert(ti!=nullptr); // you need to call "setTi"
    ti->currentText = NSStringToStdString(textField.text);
    if (ti->onTextChanged!=nullptr) ti->onTextChanged(ti->currentText);
}
@end
#endif


namespace cui {
wTextInput* wTextInput::Enabled(bool b)
{
	enabled = b;
#if defined(CUICUI_ANDROID)
	JNICall_Void_IntInt("wTextInput_Enabled", theId, b?1:0);
#elif defined(CUICUI_UWP)
	if (textBox != nullptr) textBox->IsEnabled = b;
	if (passwordBox != nullptr) passwordBox->IsEnabled = b;
#elif defined(CUICUI_WEB)	
	EM_ASM_({
		var textinput = gu($0.toString());
		if (textinput == null) return;
		textinput.disabled = $1;
	}, htmlId, !b);
#elif defined(CUICUI_IOS)
    if (uiTextField!=nullptr) uiTextField.enabled = b;
	if (uiTextViewMultiline!=nullptr) uiTextViewMultiline.editable = b;		
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif
	return this;
}

bool wTextInput::ComputeMinSize(double * w, double * h)
{
	assert(ax[X].realEm.IsMin()==false); // a wTextInput can't have Xem(MIN)

	// always this
	*w = -666;
	
	// depends of type:
	if (type == eType::MultilineText)
	{
		double requiredWidthInPixels = this->GetPixels(X);

#ifdef CUICUI_UWP		
		// TODO: isn't there a textbox->textheight ?
		// TODO_867645: same as wText: we must not do Setpos_hw because it breaks the cache (BUT it's not a big deal, there are rarely a lot of textinputs...)
		SetPos_HW(pixels(0), pixels(0), requiredWidthInPixels, 0); // doing this is required for Measure to be correct (Measure doesn't really works?)
														   // -> TODO: no risk of "flicker"? seems not
		Size s2((float)requiredWidthInPixels, (float)10000000);
		textBox->Measure(s2);				 // TODO/PERF: would not doing measure and using actualheight work?
		double desiredWidthInPixels = textBox->DesiredSize.Width;
		double desiredHeightInPixels = textBox->DesiredSize.Height;		
		*h = (desiredHeightInPixels);
#elif defined(CUICUI_WEB)		
		int retVal = EM_ASM_INT(
		{			
			var elem = gu($0);
			var h = elem.scrollHeight; // TODO_5457: ok when text is growing, not ok when shrinking... 
			return h;
		}, htmlId);
		*h = retVal;

#elif defined(CUICUI_ANDROID)
		pixels pzero(0);


		// PERF: do we really need to do this?
		SetPos_HW(pzero, pzero, requiredWidthInPixels, 0); // doing this is required for Measure to be correct (Measure doesn't really works?)
		int heightAndWidthInPixels = JNICall_Int_IntInt("cuiTextViewComputeWidthHeight", this->theId, requiredWidthInPixels);
		*w = (double)(heightAndWidthInPixels & 0xffff);
		*h = (double)(heightAndWidthInPixels >> 16);
#elif defined(CUICUI_IOS)
		SetPos_HW(pixels(0), pixels(0), requiredWidthInPixels, 0); // useful? TODO: try without

														   // source : https://stackoverflow.com/questions/50467/how-do-i-size-a-uitextview-to-its-content
        CGSize newSize;
        if (uiTextViewMultiline!=nullptr)
            newSize = [uiTextViewMultiline sizeThatFits : CGSizeMake(requiredWidthInPixels, CGFLOAT_MAX)];
        else
            newSize = [uiTextField sizeThatFits : CGSizeMake(requiredWidthInPixels, CGFLOAT_MAX)];

		*w = newSize.width;
		*h = newSize.height;
#else
		missing
#endif
	}
	else
	{
		*h = GetEmSize(1) * emRatio();
	}
	return true;
}

wTextInput* wTextInput::PlaceholderText(const std::string &str)
{
	if (type == eType::MultilineText)
	{
		assert(false); // PlaceholderText is not available for multilinetext
		// actually it works on android/uwp/web but not on ios (yet), it's locked for now then		
		return this;
	}

#if defined(WINAPI_FAMILY)
	auto str3 = StdStringToUwpString(str);
	if (isPasswordBox())
		passwordBox->PlaceholderText = str3;
	else
		textBox->PlaceholderText = str3;
#endif
#ifdef CUICUI_IOS
    if (uiTextField!=nullptr) [uiTextField setPlaceholder : StdStringToNSString(str)];
    if (uiTextViewMultiline!=nullptr) CUICUI_NYI
#endif
#if defined(CUICUI_ANDROID)
	JNICall_Void_IntString("cuiTextInput_SetPlaceholder", theId, str);
#endif
#if defined(__EMSCRIPTEN__)

	EM_ASM_({
		var num = $0;
		var elem = getUnattachedElementsFromId(num.toString());
		if (elem == null)
		{
			Module.print("wTextInput::SetPlaceholder() --> elem not found : " + num.toString());
			return;
		}
		var msg = UTF8ToString($1);
		elem.placeholder = msg;
	}, htmlId, str.c_str());
#endif
	return this;
}

wTextInput* wTextInput::SetText(const std::string &str)
{
	currentText = str;

#if defined(WINAPI_FAMILY)				
	if (isPasswordBox())
		passwordBox->Password = StdStringToUwpString(str);
	else
		textBox->Text = StdStringToUwpString(str);
#endif
#if defined(CUICUI_ANDROID)
	JNICall_Void_IntString("cuiTextInput_SetText", theId, str);
#endif
#if defined(CUICUI_IOS)
	if (uiTextField!=nullptr) [uiTextField setText : StdStringToNSString(str)];
	if (uiTextViewMultiline!=nullptr) [uiTextViewMultiline setText : StdStringToNSString(str)];
#endif
#if defined(__EMSCRIPTEN__)


	EM_ASM_({
		var num = $0;
	var elem = getUnattachedElementsFromId(num.toString());
	if (elem == null)
	{
		Module.print("wTextInput::SetText() --> elem not found : " + num.toString());
		return;
	}
	var msg = UTF8ToString($1);
	elem.value = msg;
		}, htmlId, str.c_str());
#endif
	return this;
}

wTextInput* wTextInput::DisableReturnKey()
{
	assert(type == eType::MultilineText);

#if defined(CUICUI_ANDROID)
	JNICall_Void_Int("wTextInput_Multiline_DisableReturnKey", theId);	
#elif defined(CUICUI_UWP)
	textBox->AcceptsReturn = false;
#elif defined(CUICUI_WEB)
	EM_ASM_({
		var elem = getUnattachedElementsFromId($0.toString());
		if (elem != null) elem.dataset.cuicuiNoReturn = "1";
	}, htmlId);
#elif defined(CUICUI_IOS)
    Log("TODO_NYI_43");
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif

	return this;
}

wTextInput::wTextInput(eType a_type) : wWidget()
{
	coreType = eCoreType::TextInput;

	type = a_type;
	currentText = "";
	onTextChanged = nullptr;

#if defined(CUICUI_IOS)

	if (type==eType::MultilineText) 
	{
		uiTextViewMultiline = [[UITextView alloc]init];
		[uiView addSubview : uiTextViewMultiline];
        uiTextViewMultiline.editable = YES;
        uiTextViewMultiline.userInteractionEnabled = YES;
        [uiTextViewMultiline setFont : central.defaultFont->native];
        
        [uiTextViewMultiline setBackgroundColor : [UIColor colorWithRed :0 green : 0 blue : 0 alpha : 0]];
        [uiTextViewMultiline setOpaque : NO];
	}
	else 
	{
		uiTextField = [[UITextField alloc]init];
		[uiView addSubview : uiTextField];

		if (type==eType::Password)
			[uiTextField setSecureTextEntry : YES];

		theCuiUITextFieldEventClass = [[CuiUITextFieldEventClass alloc] init];
		[theCuiUITextFieldEventClass setTi : this];
		[uiTextField addTarget : theCuiUITextFieldEventClass action : @selector(textFieldDidChange : ) forControlEvents:UIControlEventEditingChanged];
		[uiTextField setDelegate : theCuiUITextFieldEventClass];

		uiTextField.userInteractionEnabled = YES;

		[uiTextField setFont : central.defaultFont->native];
	}

    assert(uiView==nullptr);
    uiView = (CuiUIView*)uiTextField;


	// this doesnt work it seems ( https://stackoverflow.com/questions/1750256/making-view-resize-to-its-parent-when-added-with-addsubview )
	/*
	 //[uiView setClipsToBounds:YES]; // TODO: should always be YES for all defwidgets?
	 [uiView setAutoresizingMask:UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight];
	 [uiView setAutoresizesSubviews:YES];*/
#endif
#if defined(CUICUI_UWP)
	passwordBox = nullptr;
	textBox = nullptr;
	if (isPasswordBox())
	{
		passwordBox = ref new PasswordBox();

		passwordBox->Margin = 0;
		passwordBox->Padding = 0;
		passwordBox->BorderThickness = 0; // remove default border


		// Grid stretches the native editor to the widget's full hit-test area.
		b = ref new Grid();
		((Grid^)b)->Children->Append(passwordBox);
		
		hd = ref new cCuiElement_uwp(this);

		auto style = GetAppStyle("PasswordBoxStyleCui");
		passwordBox->Style = safe_cast<Windows::UI::Xaml::Style ^>(style);
		passwordBox->HorizontalAlignment = HorizontalAlignment::Stretch;
		passwordBox->VerticalAlignment = VerticalAlignment::Stretch;

		passwordBox->PasswordChanged += ref new RoutedEventHandler(hd, &cCuiElement_uwp::textinput_PasswordChanged);
		passwordBox->KeyUp += ref new KeyEventHandler(hd, &cCuiElement_uwp::textinput_OnKeyUp);
	}
	else
	{
		textBox = ref new TextBox();
		if (type == eType::MultilineText)
		{
			// multiline
			textBox->AcceptsReturn = true; 
			textBox->TextWrapping = TextWrapping::Wrap;		
			

			/*TODO_89671: wTextInput multiltine: on UWP there's a flicker, we should maybe do this in the UWP onTextChaning event?
			textBox->TextChanging += ref new Windows::UI::Xaml::Type([this](Platform::Object ^sender, Windows::UI::Xaml::SizeChangedEventArgs ^e)
			{
				CUICUI_NOP
			});*/
		}
		else
		{
			// single line
			textBox->AcceptsReturn = false; 
			textBox->TextWrapping = TextWrapping::NoWrap;
		}

		
		textBox->Margin = 0;
		textBox->Padding = 0;
		textBox->BorderThickness = 0; // remove default border

		// Canvas leaves the editor at its desired size, even when the widget is wider.
		b = ref new Grid();
		((Grid^)b)->Children->Append(textBox);

		hd = ref new cCuiElement_uwp(this);
		
		auto style = GetAppStyle("TextBoxStyleCui");
		textBox->Style = safe_cast<Windows::UI::Xaml::Style ^>(style);
		textBox->HorizontalAlignment = HorizontalAlignment::Stretch;
		textBox->VerticalAlignment = VerticalAlignment::Stretch;

		textBox->TextChanged += ref new TextChangedEventHandler(hd, &cCuiElement_uwp::textinput_TextChanged);
		textBox->KeyUp += ref new KeyEventHandler(hd, &cCuiElement_uwp::textinput_OnKeyUp);
	}




#endif
#if defined(CUICUI_ANDROID)
	JNICall_Int_IntInt("cuiTextInput_CreateAndAddToElement", theId, (int)type);
#endif
#if defined(__EMSCRIPTEN__)

	htmlId = htmlIdCount;
	htmlIdCount++;

	EM_ASM_(
		{
			var num = $0;

			var x;			
			
			

			if ($1 == 2) // password
			{
				x = document.createElement('INPUT');
				x.setAttribute("type", "password");
			}
			else
			{
				if ($1 == 0) // text singleline
				{
					x = document.createElement('INPUT');					
					x.setAttribute("type", "text");
				}
				if ($1 == 1) // text multiline
				{
					x = document.createElement('TEXTAREA');
					x.style["height"] = "100%"; // allows the textarea to have the height of the wBase <div> -> TODO_5457: on web type 4 lines (it autogrows) then press backspace to have fewer lines: it doesn't shrink
					x.style["resize"] = "none"; // hide the "resize thumb" css3
					x.style["overflow"] = "hidden"; // hide the scrollbar
				}
			}
			x.className = "b";
			x.oninput = function() { Module.ccall('ontextinput', 'number', ['number', 'string'], [$2, this.value]); };
			x.style["border-width"] = 0;
			x.style["outline"] = 0; // remove colored border when the textinput is selected (note: not sure it's the best way)
			x.style["padding"] = 0; // i think html adds a 1px padding; not sure
			x.style["font-family"] = "inherit";
			x.style["font-size"] = "inherit";
			x.style["width"] = "100%";
			x.style["background-color"] = "transparent";
			x.style["pointer-events"] = "auto";

			x.onkeypress = function(e)
			{
				if (!e) e = window.event;
				var keyCode = e.keyCode || e.which;
				if (keyCode == '13')
				{
					// multiline: Enter goes to the next line, unless DisableReturnKey() was called
					if (this.tagName == 'TEXTAREA' && this.dataset.cuicuiNoReturn != "1") return true;
					// Enter pressed								
					Module.ccall('wtextinput_keyup_event', 'number',['number'],[$2]);
					return false;
				}
			};

			addUnattachedElements(x, num.toString());
		}, htmlId, (int)type, this);
#endif
}

void wTextInput::UpdateRealFontSize() // DUPLICATE_5671
{
#if defined(CUICUI_UWP)

	if (isPasswordBox()==false)
	{
		double df = central.defaultFont->fontSizeInPixels;
		textBox->FontSize = df * emRatio();
	}
	else
	{
		double df = central.defaultFont->fontSizeInPixels;
		passwordBox->FontSize = df * emRatio();
	}
#endif
#if defined(CUICUI_IOS)
	UIFont *f = central.defaultFont->native;
	double s = central.defaultFont->sizeInPoints * emRatio();
	if (uiTextField!=nullptr) [uiTextField setFont : [UIFont fontWithDescriptor : [f fontDescriptor] size : s]];
    if (uiTextViewMultiline!=nullptr) [uiTextViewMultiline setFont : [UIFont fontWithDescriptor : [f fontDescriptor] size : s]];
#endif
#if defined(CUICUI_ANDROID)
	//jstring jstr = globalCuiJniEnv->NewString((unsigned short*)str.data(), str.size()*2); // ANDROID TODO: not sure it's correct

	JNICall_Void_IntDouble("cuiTextInput_SetFontSize", theId, emRatio()*central.defaultFont->fontSizeInPixels);
#endif
#if defined(__EMSCRIPTEN__) 	
	EM_ASM_(
		{
			var num = $0;
			var elem = getUnattachedElementsFromId(num.toString());
			if (elem == null)
			{
				Module.print("cText::UpdateRealFontSize() --> elem not found : " + num.toString());
				return;
			}

			var fontSizeInPixels = $1;
			elem.style["font-size"] = fontSizeInPixels + "px";
		}, htmlId, emRatio()*central.defaultFont->fontSizeInPixels);
#endif
}

void wTextInput::InsertText(int pos, const std::string &text)
{
	assert(pos >= 0);
	if (text.size() == 0) return;

	currentText.insert(pos, text);

	assert(isPasswordBox() == false); // not available on password box

#if defined(CUICUI_ANDROID)
	JNICall_Void_IntIntString("cuiTextInput_InsertText", theId, pos, text);
#elif defined(CUICUI_UWP)	
	std::wstring a = make_wstring(text);
	std::wstring w = textBox->Text->Data();
	w.insert(pos, a);
	Platform::String ^s = ref new Platform::String(w.c_str());
	textBox->Text = s;
#elif defined(CUICUI_WEB)

	EM_ASM_(
		{
			var txtToInsert = UTF8ToString($1);

			var textinput = gu($0);
			var t = textinput.value;
			var r = t.slice(0, $2) + txtToInsert + t.slice($2);
			textinput.value = r;
		}, htmlId, text.c_str(), pos);

#elif defined(CUICUI_IOS)
    if (uiTextViewMultiline!=nullptr)
        assert(false); // NYI ! TODO
    if (uiTextField!=nullptr)
    {
        UITextPosition *end = [uiTextField positionFromPosition : uiTextField.beginningOfDocument offset : pos];
        UITextRange *textRange = [uiTextField textRangeFromPosition : end toPosition : end];
        [uiTextField replaceRange : textRange withText : StdStringToNSString(text)];
    }
#else
	missing
#endif
}

void wTextInput::SetCursorPos(int pos)
{
	assert(isPasswordBox() == false); // not available on password box

#if defined(CUICUI_ANDROID)
	JNICall_Void_IntInt("cuiTextInput_SetCursorPos", theId, pos);
#elif defined(CUICUI_UWP)		
	textBox->Select(pos, 0);
#elif defined(CUICUI_WEB)
	EM_ASM_({
		var textinput = gu($0.toString());
		if (textinput == null) return;
		textinput.selectionStart = $1;
		}, htmlId, pos);
#elif defined(CUICUI_IOS)
    if (uiTextViewMultiline!=nullptr)
        CUICUI_NYI
    else {
        UITextPosition *pp = [uiTextField positionFromPosition : uiTextField.beginningOfDocument offset : pos];
        UITextRange *textRange = [uiTextField textRangeFromPosition : pp toPosition : pp];
        [uiTextField setSelectedTextRange : textRange];
    }
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif
}

void wTextInput::SetFocus()
{
#if defined(CUICUI_ANDROID)
	JNICall_Void_Int("cuiTextInput_SetFocus", theId);
#elif defined(CUICUI_UWP)
	if (isPasswordBox())
		passwordBox->Focus(FocusState::Programmatic);
	else
		textBox->Focus(FocusState::Programmatic);
#elif defined(CUICUI_WEB)
	EM_ASM_({
		var textinput = gu($0.toString());
		if (textinput == null) return;
		textinput.focus();
		}, htmlId);
#elif defined(CUICUI_IOS)
    if (uiTextViewMultiline!=nullptr)
        CUICUI_NYI
    else
        [uiTextField becomeFirstResponder];
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif
}

int wTextInput::GetCursorPos()
{
	assert(isPasswordBox() == false); // not available on password box

#if defined(CUICUI_ANDROID)
	int pos = JNICall_Int_Int("cuiTextInput_GetCursorPos", theId);
	if (pos < 0) pos = 0;
	return pos;
#elif defined(CUICUI_UWP)		
	int pos;
	pos = textBox->SelectionStart;
	if (pos < 0) pos = 0;
	return pos;
#elif defined(CUICUI_WEB)
	unsigned int retVal = EM_ASM_INT({
		var textinput = gu($0.toString());
		return textinput.selectionStart;
		}, htmlId);
	return retVal;
#elif defined(CUICUI_IOS)
    if (uiTextViewMultiline!=nullptr)
        CUICUI_NYI
        else {
	UITextRange *range = uiTextField.selectedTextRange;
	if (range == nil) return 0;
	NSInteger pos = [uiTextField offsetFromPosition : uiTextField.beginningOfDocument toPosition : range.start];
	if (pos < 0) return 0;
	return (int)pos;
        }
#else
	missing
#endif
    return 0;
}

void wTextInput::EnablePasteFile() // a "pasted" (ctrl+v) file will be received by the app in the onFileReceived event
{
	assert(isPasswordBox() == false); // not allowed on password box
#ifdef CUICUI_UWP		
	textBox->Paste += ref new Windows::UI::Xaml::Controls::TextControlPasteEventHandler(hd, &cCuiElement_uwp::OnPasteOnTextInput);
#elif defined (CUICUI_WEB)		
	EM_ASM_({
		var fileInput = gu($0);

		fileInput.addEventListener('paste', function(e)
		{
			console.log("paste on wTextInput");
			if (e.clipboardData && e.clipboardData.files && e.clipboardData.files.length > 0)
			{
				console.log("there are at least one file in it");
				try {
					var file = e.clipboardData.files[0]; // just the first one for now
					console.log("file : " + file);
					ManageFileReceived(file);
					e.preventDefault();
					e.stopPropagation();
				}
				catch (ex)
				{
					console.log("Paste file : warning 271");
				}
			}
			else
				if (e.clipboardData && e.clipboardData.items && e.clipboardData.items.length > 0)
				{
					console.log("there are at least one item in it");
					try {
						var file = e.clipboardData.items[0].getAsFile(); // just the first one for now
						ManageFileReceived(file);
						e.preventDefault();
						e.stopPropagation();
					}
					catch (ex)
					{
						console.log("Paste file : warning 272");
					}
				}
				else
					console.log("no items nor files in the clipboard, doing nothing...");
		}, false);
		}, htmlId);

#else
		// not available on other platforms
#endif
}

wTextInput * wTextInput::TextColor(cColor color)
{
#if defined(CUICUI_IOS)
	UIColor *col = [UIColor colorWithRed : (float)color.cr / 255.0f green : (float)color.cg / 255.0f blue : (float)color.cb / 255.0f alpha : color.alpha];
    if (uiTextField!=nullptr) [uiTextField setTextColor : col];
    if (uiTextViewMultiline!=nullptr) [uiTextViewMultiline setTextColor : col];
#endif
#if defined(WINAPI_FAMILY)
	auto uwpColor = ref new SolidColorBrush(Windows::UI::ColorHelper::FromArgb(color.ca, color.cr, color.cg, color.cb));
	if (isPasswordBox())
	{
		passwordBox->Foreground = uwpColor;
		// TODO: make a style too?
	}
	else
	{
		textBox->Foreground = uwpColor;
	}


#endif
#if defined(CUICUI_ANDROID)
	JNICall_Void_IntInt("cuiTextInput_SetTextColor", theId, color.GetARGB());
#endif
#if defined(__EMSCRIPTEN__) 	
	EM_ASM_(
		{
			var num = $0;
	var elem = getUnattachedElementsFromId(num.toString());
	if (elem == null)
	{
		Module.print("cText::TextColor() --> elem not found : " + num.toString());
		return;
	}

	var bgcol = "rgba(" + $1 + "," + $2 + "," + $3 + "," + $4 + ")";
	elem.style["color"] = bgcol;
		}, htmlId, color.cr, color.cg, color.cb, color.alpha
	);
#endif 
	return this;
}

} // namespace cui
