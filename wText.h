#pragma once


#ifdef CUICUI_IOS
@interface CuiUILabel : UILabel <UIGestureRecognizerDelegate>
-(void)setPtr:(cui::wWidget*)_ptr;
@end
#endif


namespace cui {
class wText;

class cSpan
{
	friend class wText;

public:
	std::string text;
	int start, end; // WARNING: this may NOT be real character position, it's native position which can be very different than characters positions
	bool isTouchable = false;
	bool bold = false;
	int riId;
	cColor textColor = 0x000000;	
	bool usingTextColor = false;
	bool italic = false;

#if defined(CUICUI_UWP)
	Windows::UI::Xaml::Documents::Run ^run;
#endif
#if defined(CUICUI_WEB)
	std::string GetAsHtml(wText* parent);
#endif

	cSpan(const std::string _text);

	cSpan* TextColor(const cColor& color);	
	cSpan* Italic();
	cSpan* Bold();

	cSpan* Touchable() // useful?
	{
		isTouchable = true;
		return this;
	}

	std::function<void(const cSpan *self)> onMouseMoveOverRichTextSpan = nullptr;
	std::function<void(const cSpan *self)> onMouseClickOnRichTextSpan = nullptr;
	std::function<void(const cSpan *self)> onMouseExitedRichTextSpan = nullptr;

	static cSpan* Create(const std::string &txt) { cSpan *r = new cSpan(txt); return r; }
};

class wText : public wWidget // note: don't use wEmpty as it would add a useless div
{
public:
#if defined(CUICUI_IOS)	
	CuiUILabel *uiTextView;
	NSMutableAttributedString *mutableAttributedString;
#endif
#if defined(CUICUI_UWP)
	Canvas^ canvas; // uwp RichTextBlock doesn't have a Background: we need to embed it in a canvas so we can set the bgcolor
	RichTextBlock ^ tb; // note: this is a RichTextBlock, maybe it could be nice to have a wTextSimple using simpler stuff and not handling rich text (only have global settextcolor, etc...)
	Windows::UI::Xaml::Documents::Paragraph ^richParagraph;
#endif
#if defined(CUICUI_ANDROID)
	int theTextBlockId;
#endif

	static int gRiId;

	bool cache_valid = false;
	double cache_min_w, cache_min_h;

	int richTextAppendCurrentPos = -1000; // we could get rid of it, no? even if it's not necessarily better (extra computation to determine the real pos of the end of the "paragraph"
	enum class eType {
		Wrap, 
		NoWrap
	};	
	bool isSingleLine;
	std::string text;
	std::vector<cSpan*> richItems;
	cSpan *previouslySentMouseOverRichTextSpan = nullptr; // not used on web

public:	 // protected:	
	bool HandleTouchSpan_PointerMoved(int characterPosition);  
	bool HandleTouchSpan_PointerReleased(int characterPosition);
	
	wText(Length xem, Length yem, eType wrap = eType::NoWrap);
	wText(eType wrap = eType::NoWrap);
	virtual ~wText()
	{
		for (auto ri : richItems)
			delete ri;
		richItems.clear();
	}

	virtual bool ComputeMinSize(double *w, double *h);	
	bool ComputeMinSize_Private(double *w, double *h, double requiredWidthInPixels);
		
public:
	void ClearSelection();	
	void AllowTextSelection(); // WARNING! This have side effects: some touch/mouse events may not be called (and maybe other side effects)	? This may not be true anymore
	wText* Ellipsis();		
	void UpdateRealFontSize(); // DUPLICATE_5671	
	cSpan* GetRichItemFromId(int riId);	
    wText* RichText_Clear();	
	wText* RichText_Append(cSpan *ri);
	std::string GetText() { return text; }	
	wText* SetText(const std::string &str, cColor textColor, bool bold=false);
	wText* SetText(const std::string &str);
	int GetCharacterPositionFromLocalPos(pixels localX, pixels localY);


public:
	static wText *Create(Length xem, Length yem, wText::eType wrapMode = wText::eType::NoWrap)
	{ 
		auto c = new wText(wrapMode);
        
        //static_assert(!(wrapMode == wText::eType::Wrap && xem.IsMin())); // should be better but doesn't compile, fixable?
		if (wrapMode == wText::eType::Wrap && xem.IsMin()) assert(false); // Xem(MIN) is not allowed on wText with Wrap (multilines)
        
		c->Xem(xem)->Yem(yem);
		return c; 
	}
};

} // namespace cui