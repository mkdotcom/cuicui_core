#pragma once

#include "platform.h"

#ifdef CUICUI_IOS
@interface CuiUIScrollView : UIScrollView <UIScrollViewDelegate> 
-(void)setPtr:(cui::wWidget*)_ptr;
@end
#endif

namespace cui {
class wScrollView : public wWidget
{
	///////////////////////////////////
	// Documentation
	//
	// Important: On some platforms (web for example) the size of the contentview of your scrollview may not be the same as the 
	//             scrollview itself. Therefore it's very important to use MIN/FILL instead of fixed size for the content of the scrollview.
	//             TODO/QUESTION: prevent the user from using FIXED on the scrollview content? wouldn't that solve the problem?
	//
	// DEPRECATED informations: (delete them)
	// Scrollview is not as straightforward as it could because on web we need to do some hack to prevent the scrollbar to disturb the layouter
	// We use a workaround using "scrollViewFirstChild" and "userContentContainer" to handle this
	// It's used on all platforms, though only on web it's really useful (but better to use it everywhere to ensure it's all working)
	//
	///////////////////////////////////

public: // protected:
#if defined(CUICUI_IOS)
	CuiUIScrollView *uiScrollView;
#endif 
#if defined(WINAPI_FAMILY)
	ScrollViewer ^uwpScrollViewer;
#endif
#if defined(CUICUI_ANDROID)
	int scrollViewContentTheId;
#endif

	wWidget *scrollViewContentWidget = nullptr;
	double scrollBarWidthInPixels;

	enum class Orientation : unsigned char {
		Vertical = 0, // default
		Horizontal = 1,		
	};

	Orientation orientation;

public: //protected: protected not ok on ios (todo: we can friend the class)
	std::function<void()> onUserScrolled;

public:				
	wScrollView* OnUserScrolled(std::function<void()> f)
	{
		onUserScrolled = f;
		return this;
	}
	
	wScrollView(Orientation orientation = Orientation::Vertical);
	void ScrollToEnd(); // bottom if vertical, right if horizontal
	virtual wScrollView* Content(std::function<void(Children &dc)> a_content);
	virtual ~wScrollView() { }

	pixels GetCurrentOffset();

	// currently a scrollview can't be MIN on its scrolling axis

public:
	static wScrollView *Create(Length xem, Length yem, Orientation orientation = Orientation::Vertical) { auto c = new wScrollView(orientation); c->Xem(xem)->Yem(yem); return c; }
};

class wVerticalListView : public wScrollView
{
public:
	wRows* r;
	wEmpty* temp;

	// TODO/PERF: this widget could be improved to optimize adding item at the end (by not calling RelayoutContent() but by setting position manually)

public:
	wVerticalListView() : wScrollView(Orientation::Vertical)
	{
		XemYem(fill());

		wScrollView::Content([this](Children& dc)
		{
			r = (wRows*)wRows::Create(fill(), min());
			dc += r;
		});		
	}

	void AppendWidget(wWidget* addMe) // NOTE: you don't need to call "RefreshUI" after adding a widget with this method
	{		
		r->AddChild_private(addMe);			
	}
	void RelayoutContent() // call this after you are done using AppendWidget()
	{
		RefreshUI_Fast(this);
	}

	wRows* GetRowsWidget() // only use this if you know what you are doing
	{
		return r;
	}

	// lock some methods to avoid user mistake
	virtual wScrollView* Content(std::function<void(Children & dc)> a_content)
	{
		assert(false); // don't use this method, use AppendWidget() to add content to the listview
		return this;
	}
	void AddChild_private(wWidget*) = delete;

public:
	static wVerticalListView* Create(Length xem, Length yem) { auto c = new wVerticalListView(); c->Xem(xem)->Yem(yem); return c; }
};

} // namespace cui