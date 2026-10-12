#include "cuicui.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include "wText.h"
#include "central.h"

namespace cui {
int wText::gRiId = 0; 

int gDebugComputeMinOnWText = 0;
int gDebugComputeMinOnWText_cached = 0;

#ifdef CUICUI_IOS
} // namespace cui
@implementation CuiUILabel {
    wWidget *widgetPtr;
}
-(void)setPtr:(wWidget*)_ptr
{
    widgetPtr = _ptr;
}

- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    if (CGRectContainsPoint(self.bounds,point)==true)
        if (widgetPtr->opaqueToTouch==true || widgetPtr->touchableAdded==true)
            return self;

    return nil;
}

-(void)touchesBegan:(NSSet *)touches withEvent : (UIEvent *)event
{
    widgetPtr->ios_subclasswrap_touchesBegan(touches,event);
}
-(void)touchesMoved:(NSSet *)touches withEvent : (UIEvent *)event
{
    widgetPtr->ios_subclasswrap_touchesMoved(touches,event);
}
-(void)touchesEnded:(NSSet *)touches withEvent : (UIEvent *)event
{
    widgetPtr->ios_subclasswrap_touchesEnded(touches,event);
}

@end

namespace cui {
#endif


bool wText::HandleTouchSpan_PointerReleased(int characterPosition)
{
	Log("wText::HandleTouchSpan_PointerReleased : characterPos: " + std::to_string(characterPosition));
	wText *m = this;
	int i = 0;
	for (cSpan *ri : m->richItems)
	{
		if (ri->isTouchable)
			if (characterPosition >= ri->start && characterPosition < ri->end)
			{
				// UWP special case: if we have a large wText and its text doesn't fill the whole width,
				// (and the last ri is at the very end)
				// putting the cursor in the empty space on the right gives the position of the last character (+1)
				// if we are there, we don't want that to trigger the event
				// TODO: not perfect, the last char is not detected as a result
				// needed on platforms other than uwp? (TODO)
				//		web: works without this hack

				if (ri->onMouseClickOnRichTextSpan != nullptr)
				{
					ri->onMouseClickOnRichTextSpan(ri);
					return true; // Note: when receiving a 'span touch event', the 'onPointerMoved' of the wText is NOT called
				}
			}
		i++;
	}

	return false;
}

cSpan* cSpan::Italic()
{
	italic = true;
#if defined(WINAPI_FAMILY)
	run->FontStyle = Windows::UI::Text::FontStyle::Italic;
#else
	// not done here
#endif
	return this;
}

cSpan* cSpan::Bold()
{
	bold = true;
#if defined(WINAPI_FAMILY)
	run->FontWeight = Windows::UI::Text::FontWeights::Bold;
#else
	// not done here
#endif
	return this;
}

cSpan* cSpan::TextColor(const cColor& color)
{
	usingTextColor = true;
	textColor = color;

#if defined(CUICUI_UWP)
	run->Foreground = textColor.ToUWPSolidColorBrush();
#elif defined(CUICUI_WEB)
	CUICUI_NOP // done elsewhere
#elif defined(CUICUI_ANDROID)
	CUICUI_NOP // done elsewhere
#elif defined(CUICUI_IOS)
	CUICUI_NOP // done elsewhere
#else
	missing platform
#endif
		return this;
}

void wText::ClearSelection()
{
#if defined(WINAPI_FAMILY)
	assert(false);
#else
	assert(false); // 
#endif
}

wText* wText::Ellipsis()
{
	assert(isSingleLine); // not available on multiline wText

#if defined(CUICUI_ANDROID)
	JNICall_Void_Int("cuiTextBlock_SetEllipsis", theId);
#elif defined(CUICUI_UWP)
	tb->TextTrimming = TextTrimming::CharacterEllipsis;
#elif defined(CUICUI_WEB)
	EM_ASM_(
		{
			var num = $0;
			var thespan = gu(num.toString());
			if (thespan == null) return;
			thespan.style["text-overflow"] = "ellipsis";
		}, htmlId);

#elif defined(CUICUI_IOS)
	uiTextView.lineBreakMode = NSLineBreakByTruncatingTail;
#else
	missing
#endif
		return this;
}

wText* wText::SetText(const std::string &str, cColor textColor, bool bold)
{
	AssertOnUiThread();

	cache_valid = false;

	RichText_Clear();
	text = "";
	if (bold==false)
		RichText_Append(cSpan::Create(str)->TextColor(textColor.rgb));
	else
		RichText_Append(cSpan::Create(str)->TextColor(textColor.rgb)->Bold());
	return this;
}


cSpan* wText::GetRichItemFromId(int riId)
{
	for (auto r : richItems)
	{
		if (r->riId == riId)
			return r;
	}
	return nullptr;
}

wText* wText::RichText_Clear()
{
    // note: we dont remove "touchable" if it's there, because Touchable() may have been called by Touchable() or by adding a touchable span, and we don't know
    
	cache_valid = false;
	text = "";

	for (auto ri : richItems) delete ri;
	richItems.clear();

#if defined(CUICUI_IOS)
	mutableAttributedString = [[NSMutableAttributedString alloc] initWithString:@""];
	richTextAppendCurrentPos = 0; // we don't use it on ios though
#endif
#if defined(WINAPI_FAMILY)
	tb->Blocks->Clear();
	richParagraph = ref new Windows::UI::Xaml::Documents::Paragraph(); // I guess we can't do without it?
	tb->Blocks->Append(richParagraph);

	richTextAppendCurrentPos = tb->ContentEnd->Offset;
#endif
#if defined(CUICUI_WEB)
	richTextAppendCurrentPos = 0; // we don't use it on web though
	EM_ASM_({
		var num = $0;
		var thespan = gu(num.toString());
		if (thespan == null) return;
		thespan.innerHTML = "";
		}, htmlId);
#endif
#if defined(CUICUI_ANDROID)
	JNICall_Void("cuiTextBlock_RichTextClear");
	richTextAppendCurrentPos = 0;
#endif

	return this;
}

wText* wText::RichText_Append(cSpan *ri)
{
	// TODO/PERF: we re-send everything to the control on each RichText_Append, not great (all platforms)(would need a RichEdit_Done? meh, but should check if that gains a lot of perf)
	cache_valid = false;

	if (ri->text == "3")
		CUICUI_NOP

	text += ri->text;

	assert(richTextAppendCurrentPos != -1000); // call RichText_Clear() first

    if (ri->isTouchable)
        this->Touchable(); // if the wText contains a touchable span, it must be Touchable() itself
    
#if defined(CUICUI_IOS)
	NSString *nstext = StdStringToNSString(ri->text);

	NSMutableAttributedString *tmp = [[NSMutableAttributedString alloc] initWithString:nstext];

	// PERF: useful to cache the generated UIFonts on our side? or does ios do it?
	UIFont *sf = central.defaultFont->native;
	double s = central.defaultFont->sizeInPoints * emRatio();
	UIFontDescriptor *fd = [sf fontDescriptor];
	if (ri->bold)
		fd = [fd fontDescriptorWithSymbolicTraits : UIFontDescriptorTraitBold];
	if (ri->italic)
		fd = [fd fontDescriptorWithSymbolicTraits : UIFontDescriptorTraitItalic];

	UIFont *f = [UIFont fontWithDescriptor : fd size : s];
	[tmp addAttribute : NSFontAttributeName value : f range : NSMakeRange(0, nstext.length)];

	if (ri->usingTextColor)
	{
        UIColor *col =  ri->textColor.ToUIColor();
		[tmp addAttribute : NSForegroundColorAttributeName value : col range : NSMakeRange(0, nstext.length)];
	}

	if (ri->isTouchable)
	{
		// we don't use the native "link click" anymore
	}
	ri->start = (int)mutableAttributedString.length;

	[mutableAttributedString appendAttributedString : tmp];

	ri->end = (int)mutableAttributedString.length;

	uiTextView.attributedText = mutableAttributedString; // .text -> for a possible non rich text version
#endif
#if defined(WINAPI_FAMILY)

	ri->start = richTextAppendCurrentPos;
	richParagraph->Inlines->Append(ri->run);
	ri->end = tb->ContentEnd->Offset;
	richTextAppendCurrentPos = ri->end;
#endif
#if defined(CUICUI_WEB)

	std::string str = ri->GetAsHtml(this);

	StdStringReplaceAllOccurences(str, "\r\n", "<br/>");
	StdStringReplaceAllOccurences(str, "\r", "<br/>");
	StdStringReplaceAllOccurences(str, "\n", "<br/>");
	
	EM_ASM_({
	var msg = UTF8ToString($1);
	var num = $0;
	var thespan = gu(num.toString());
	if (thespan == null) return;
	thespan.innerHTML = thespan.innerHTML + msg;
		}, htmlId, str.c_str());
#endif
#if defined(CUICUI_ANDROID)

	int riId = ri->riId;
	if (ri->isTouchable == false)
		riId = -1;
	unsigned int tc = 0; // 0 = will use default color
	if (ri->usingTextColor)
		tc = ri->textColor.GetARGB();

	int flag = 0;
	if (ri->bold) flag |= 1;
	if (ri->italic) flag |= 2;
	int endPos = JNICall_Int_IntIntIntIntString("cuiTextBlock_RichTextAppend", this->theId, riId, flag, tc, ri->text);

	// note: this is the endPos of the ENTIRE text in the wText

	ri->start = richTextAppendCurrentPos;
	ri->end = endPos;
	richTextAppendCurrentPos = ri->end;

	JNICall_Void_Int("cuiTextBlock_RichTextCommit", this->theId); // TODO/PERF: we re-send everything on each RichText_Append, not great
#endif

	richItems.push_back(ri);

	return this;
}

wText* wText::SetText(const std::string &str)
{
	cache_valid = false;

	RichText_Clear(); 	
	RichText_Append(cSpan::Create(str)); // useless -> make a real settext? (optim?)
	return this;
}

void wText::UpdateRealFontSize() // DUPLICATE_5671
{
	cache_valid = false;

#if defined(WINAPI_FAMILY)
	double df = central.defaultFont->fontSizeInPixels;
	double v = df * emRatio();
	if (v < 0) v = 0.1; // <=0 -> crash
	tb->FontSize = v;
#endif
#if defined(CUICUI_IOS)
	UIFont *f = central.defaultFont->native;
	double s = central.defaultFont->sizeInPoints * emRatio();
	[uiTextView setFont : [UIFont fontWithDescriptor : [f fontDescriptor] size : s]];

#endif
#if defined(CUICUI_ANDROID)
	JNICall_Void_IntDouble("cuiTextBlock_SetFontSize", theId, emRatio()*central.defaultFont->fontSizeInPixels);
#endif
#if defined(CUICUI_WEB)
	assert(htmlId != -1);
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

void wText::AllowTextSelection() // WARNING! This have side effects: some touch/mouse events may not be called (and maybe other side effects) This may not be true anymore
{		
#if defined(CUICUI_ANDROID)
	// Note: we don't support mouse on ios so there is no text selection possible
#elif defined(CUICUI_UWP)
	tb->IsTextSelectionEnabled = true;
#elif defined(CUICUI_WEB)
	assert(htmlId != -1);
	EM_ASM_(
		{
		var num = $0;
	var elem = getUnattachedElementsFromId(num.toString());
	if (elem == null)
	{
		Module.print("ERROR wText::wText() -> elem not found : " + num.toString());
		return;
	}

	elem.style["user-select"] = "text";
	elem.style["pointer-events"] = "auto";

		}, htmlId);
#elif defined(CUICUI_IOS)
    // Note: we don't support mouse on ios so there is no text selection possible
#else
	missing
#endif
}

cSpan::cSpan(const std::string _text)
{
	text = _text;
#if defined(CUICUI_UWP)
	run = ref new Windows::UI::Xaml::Documents::Run();
	Platform::String^ ps = StdStringToUwpString(text);
	run->Text = ps;
#endif
	riId = wText::gRiId;
	wText::gRiId++;
}

#if defined(CUICUI_WEB)

std::string cSpan::GetAsHtml(wText *parent)
{
	if (isTouchable == false && bold == false && italic == false && usingTextColor == false)
		return text; // optim
 
	std::string s = "<span ";

	std::string style;
	if (usingTextColor)
	{
		std::string bgcol = textColor.ToHtmlColorCodeWithAlpha();
		style += "color:" + bgcol + ";";
	}
	if (bold) style += "font-weight: bold;";
	if (italic) style += "font-style: italic;";	

	if (style.empty() == false)
		s += "style=\"" + style + "\" ";

	// note_454541: spantouch events are handled here because we don't know how to implement GetCharacterPositionFromLocalPos() on WEB (workaround, CLEANUP possible?)
	if (isTouchable)
	{
		unsigned int widgetPtr = (unsigned int)parent;
		s += "onmouseup=\"CallSpanTouchEvent_click(event, " + std::to_string(widgetPtr) + ", " + std::to_string(this->riId) + ")\" ";
		s += "onmouseenter=\"CallSpanTouchEvent_hover(event, " + std::to_string(widgetPtr) + ", " + std::to_string(this->riId) + ")\" ";
		s += "onmouseleave=\"CallSpanTouchEvent_leave(event, " + std::to_string(widgetPtr) + ", " + std::to_string(this->riId) + ")\" ";
	}

	s += ">";
	s += text;
	s += "</span>";

	return s;
}
#endif


wText::wText(Length _xem, Length _yem, eType _wrapMode) : wText(_wrapMode)
{
	if (_wrapMode == wText::eType::Wrap && _xem.IsMin()) assert(false); // Xem(MIN) doesn't make sense on wText with Wrap (multilines)
	
	Xem(_xem);
	Yem(_yem);
}

wText::wText(eType wrap) : wWidget()
{
	assert(wrap == eType::NoWrap || wrap == eType::Wrap);

	coreType = eCoreType::Text;
	XemYem(min(), ems(1.0));

	if (wrap == eType::NoWrap)
	{
		isSingleLine = true;
	}
	else
	{
		isSingleLine = false;
	}


#ifdef CUICUI_IOS
    assert(uiView==nullptr);
	uiTextView = [[CuiUILabel alloc] init]; // CLEANUP: need to set a size? no, set cgzerothing
    [uiTextView setPtr:this];

	if (isSingleLine)
	{
		uiTextView.lineBreakMode = NSLineBreakByClipping; // cuicui default: clip
		uiTextView.numberOfLines = 1;
	}
	else
	{
		uiTextView.lineBreakMode = NSLineBreakByWordWrapping; // ios default
		uiTextView.numberOfLines = 0;
        uiTextView.layoutMargins = UIEdgeInsetsZero;
       
	}
	uiTextView.userInteractionEnabled = NO; // important, the parent (the real widget) must receive the event

	// we need to set a font, or uiTextView.font is nil (and we need the current font in richtext stuff)
	[uiTextView setFont : central.defaultFont->native];

	// default text color is black
	UIColor* col = [UIColor colorWithRed:0.0f green:0.0f blue:0.0f alpha:1.0];
	[uiTextView setTextColor : col];

    assert(uiView==nullptr);
    uiView = (CuiUIView*)uiTextView;

#endif
#if defined(CUICUI_UWP)
	tb = ref new RichTextBlock();
	if (isSingleLine)
		tb->TextWrapping = TextWrapping::NoWrap;
	else
		tb->TextWrapping = TextWrapping::Wrap;

	b = ref new Canvas();
	((Canvas^)b)->Children->Append(tb);

	tb->IsTextSelectionEnabled = false;
#endif
#if defined(CUICUI_ANDROID)

	theTextBlockId = JNICall_Int_IntInt("cuiTextBlock_CreateAndAddToElement", theId, isSingleLine ? 0 : 1);
#endif

#if defined(CUICUI_WEB)

	htmlId = htmlIdCount;
	htmlIdCount++;

	EM_ASM_(
		{
			var num = $0;
	
	
	var isSingleLine = $1;
	

	// we add a span (without it we can't measure the textblock) (PERF: still useful?)
	var theSpan = document.createElement('span');
	theSpan.style["overflow"] = "hidden";
	theSpan.id = num.toString();

	
	/* moved to .ts and .tm in cuicui.cpp, we keep this here commented out for the doc it contains
	theSpan.style["position"] = "absolute";
	if (isSingleLine == 0)
	{
		theSpan.style["overflow-wrap"] = "break-word";

		// DUP_565 this also allows wrapping correctly even if the text is a single block without any space
		theSpan.style["word-break"] = "break-word";
	}
	else
	{
		theSpan.style["overflow"] = "hidden";
		theSpan.style["white-space"] = "nowrap"; // this prevent text wrapping
	}
	theSpan.style["user-select"] = "none"; // note: QUESTION: I blocked text selection in the whole doc (in cuicui.html), it's better (especially on buttons and clickable stuff, and the mouse cursor is not the text one, which is much better),
											// here I enable selection on multiline because it's handy to copy paste text; maybe add an AllowWebSelection()?
											// I re-block selection because it causes problems in an app
											// -> the new default is NO text selection

	*/
	if (isSingleLine == 0)
		theSpan.className = "tm"; // multiline
	else
		theSpan.className = "ts"; // singleline

	
	addUnattachedElements(theSpan, num.toString());
	}, htmlId, isSingleLine ? 1 : 0);

#endif

	RichText_Clear();
}


bool wText::ComputeMinSize(double * w, double * h)
{
	UpdateNativeFontsIfNeeded(emRatio());

	gDebugComputeMinOnWText++;

	if (cache_valid == true) // TODO_optim_5457: note: only used on singlelined for now
	{
		if (isSingleLine == false)
		{
			assert(GetEm(X).IsPixels());
			if (cache_min_w != GetEm(X).v)
				goto dontUseCache;
		}

		*w = cache_min_w;
		*h = cache_min_h;
		gDebugComputeMinOnWText_cached++;
		return true;
	}
dontUseCache:;

	/////////////////////////
	if (isSingleLine)
	{
		bool b = ComputeMinSize_Private(w, h, 10000);
		if (b == true)
		{
			cache_min_w = *w;
			cache_min_h = *h;
			cache_valid = true;
			return true;
		}
		else
		{
			assert(false); // can't happen
			return false;
		}
	}
	else
	{
		bool b = ComputeMinSize_Private(w, h, this->GetPixels(X));
		if (b == false) return false;
		*w = -666; // note: a Multiline wText can't have MIN as X, we dont care about "w'
		
		cache_valid = true; 		
		assert(GetEm(X).IsPixels());
		cache_min_w = GetEm(X).v; // or ax[X].size
		cache_min_h = *h;
		

		return true;
	}
}



bool wText::ComputeMinSize_Private(double * w, double * h, double requiredWidthInPixels)
{
	UpdateNativeFontsIfNeeded(emRatio());

#if defined(CUICUI_UWP)
	//SetPos_HW(pzero, pzero, requiredWidthInPixels, 0); // definitely not, that breaks the cache
	Size s2((float)requiredWidthInPixels, (float)10000000);
	tb->Measure(s2);				 // TODO/PERF: would not doing measure and using actualheight work?
	double desiredWidthInPixels = tb->DesiredSize.Width;
	double desiredHeightInPixels = tb->DesiredSize.Height;
	*w = (desiredWidthInPixels);  
	*h = (desiredHeightInPixels);  
#elif defined(CUICUI_WEB)
	// We measure on a CLONE attached to cuiRootDiv (always in the live DOM), not on the real element.
	// Reason: elem.offsetWidth is 0 as long as the element is not placed in the rendered DOM (e.g. the very first
	// screen built before the attach), and that 0 ends up cached -> "empty" text/button. The clone copies the class
	// (ts/tm), the inline font-size and the content (spans), and inherits the global font-family under cuiRootDiv,
	// so it measures the same as the real element, without ever touching or moving the latter.
	unsigned int retVal = EM_ASM_INT(
		{

		var num = $0;
		var elem = gu(num.toString());

		if (elem == null) {
			console.log("error 5114");
			return;
		}

		var clone = elem.cloneNode(true); // copies class + inline font-size + subtree (rich spans)
		clone.removeAttribute("id");      // avoids a duplicate id (synchronous measurement -> harmless otherwise)

		if ($2 == 0)
			clone.style["width"] = $1.toString() + "px";
		else
			clone.style["width"] = null;

		clone.style["height"] = null; // needed for measuring to work

		clone.style["line-height"] = $3.toString() + "px"; // important, check NOTE_7614

		clone.style["position"] = "absolute";   // doesn't disturb the flow of cuiRootDiv
		clone.style["visibility"] = "hidden";   // measurable (unlike display:none) and invisible

		var root = document.getElementById("cuiRootDiv");
		root.appendChild(clone);

		var widthInPixels = clone.offsetWidth;
		var heightInPixels = clone.offsetHeight;

		root.removeChild(clone);


		return ((heightInPixels << 16) + widthInPixels);
	}, htmlId, requiredWidthInPixels, isSingleLine?1:0, central.defaultEmSize.asDouble()* emRatio());
	*w = (double)(retVal & 0xffff);
	*h = (double)(retVal >> 16);



	if (*h < 1) *h = 1; // note: on WEB if the text is empty the computation gives 0, which is not ok, it should be 1.0 at the minimum

#elif defined(CUICUI_ANDROID)
	pixels pzero(0);


	//SetPos_HW(pzero, pzero, requiredWidthInPixels, 0); // definitely not, that breaks the cache
	int heightAndWidthInPixels = JNICall_Int_IntInt("cuiTextViewComputeWidthHeight", this->theId, requiredWidthInPixels);
	*w = (double)(heightAndWidthInPixels & 0xffff) ;
	*h = (double)(heightAndWidthInPixels >> 16) ;
#elif defined(CUICUI_IOS)
	//SetPos_HW(pzero, pzero, requiredWidthInPixels, 0); // definitely not, that breaks the cache
	
	// source : https://stackoverflow.com/questions/50467/how-do-i-size-a-uitextview-to-its-content
	CGSize newSize = [uiTextView sizeThatFits : CGSizeMake(requiredWidthInPixels, CGFLOAT_MAX)];

	*w = newSize.width ;
	*h = newSize.height ;
#else
	missing
#endif

		return true;
}

bool wText::HandleTouchSpan_PointerMoved(int characterPosition)
{
	wText *m = this;
	int i = 0;
	for (cSpan *ri : m->richItems)
	{
		if (ri->isTouchable)
			if (characterPosition >= ri->start && characterPosition < ri->end)
			{
				// UWP special case: if we have a large wText and its text doesn't fill the whole width,
				// (and the last ri is at the very end)
				// putting the cursor in the empty space on the right gives the position of the last character (+1)
				// if we are there, we don't want that to trigger the event
				// TODO: not perfect, the last char is not detected as a result
				// needed on platforms other than uwp? (TODO)
				//		web: works without this hack
				// TODO: to move into the UWP code

				if (ri->onMouseMoveOverRichTextSpan != nullptr)
				{
					ri->onMouseMoveOverRichTextSpan(ri);
					m->previouslySentMouseOverRichTextSpan = ri;
					return true; // swallow the event: Note: when receiving a 'span touch event', the 'onPointerMoved' of the wText is NOT called
				}
			}
		i++;
	}

	// we didn't touch any touchspan

	if (m->previouslySentMouseOverRichTextSpan != nullptr)
	{
		if (m->previouslySentMouseOverRichTextSpan->onMouseExitedRichTextSpan != nullptr)
		{			
			m->previouslySentMouseOverRichTextSpan->onMouseExitedRichTextSpan(m->previouslySentMouseOverRichTextSpan);
			// note: we don't "return" here, the widget will receive the onPointerMoved
		}
		m->previouslySentMouseOverRichTextSpan = nullptr;
	}

	return false; // don't swallow the event
}


int wText::GetCharacterPositionFromLocalPos(pixels localX, pixels localY) 
{
#if defined(CUICUI_ANDROID)
	int characterPosition = JNICall_Int_IntIntInt("wText_GetCharacterPositionFromLocalPos", theId, localX.asDouble(), localY.asDouble());
	return characterPosition;
#elif defined(CUICUI_UWP)
	// WARNING: on uwp, when pos is in the wText but after the end of the text : it returns the position of the end of the text ! (TODO_P1)
	Windows::Foundation::Point localPos((float)localX.asDouble(), (float)localY.asDouble());
	Windows::UI::Xaml::Documents::TextPointer ^tp = tb->GetPositionFromPoint(localPos);
	int characterPosition = tp->Offset;
	return characterPosition;
#elif defined(CUICUI_WEB)
	assert(false); // You should not be here. See note_454541, and WEB_CLICK_ON_CHAR_METHOD_1 in _todo.txt
	return -1;
#elif defined(CUICUI_IOS)
    CGPoint point = CGPointMake(localX.asDouble(), localY.asDouble());

    UILabel *textLabel = this->uiTextView;
    CGPoint tapLocation = point;
    
    // init text storage
    NSTextStorage *textStorage = [[NSTextStorage alloc] initWithAttributedString:textLabel.attributedText];
    NSLayoutManager *layoutManager = [[NSLayoutManager alloc] init];
    [textStorage addLayoutManager:layoutManager];
    
    // init text container
    NSTextContainer *textContainer = [[NSTextContainer alloc] initWithSize:CGSizeMake(textLabel.frame.size.width, textLabel.frame.size.height+100) ];
    textContainer.lineFragmentPadding  = 0;
    textContainer.maximumNumberOfLines = textLabel.numberOfLines;
    textContainer.lineBreakMode        = textLabel.lineBreakMode;
    
    [layoutManager addTextContainer:textContainer];
    
    NSUInteger characterIndex = [layoutManager characterIndexForPoint:tapLocation
                                                      inTextContainer:textContainer
                             fractionOfDistanceBetweenInsertionPoints:NULL];
    
    return (int)characterIndex;
#elif defined(CUICUI_MACOS)
		missing
#else
		missing
#endif
}

} // namespace cui