#include "cuicui.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include "wScrollView.h"

#ifdef CUICUI_IOS
@implementation CuiUIScrollView{
    wWidget *widgetPtr;
}
-(void)setPtr:(wWidget*)_ptr
{
    widgetPtr = _ptr;
}
-(void)scrollViewWillBeginDragging:(UIScrollView *)scrollView {

    wScrollView *sw = (wScrollView*)widgetPtr;
    if (sw->deleted) return;
    if (sw->onUserScrolled != nullptr)
        sw->onUserScrolled();
}

@end
#endif

namespace cui {
wScrollView::wScrollView(Orientation a_orientation) : wWidget()
{
	coreType = eCoreType::ScrollView;

	orientation = a_orientation;

	{
#if defined(CUICUI_WEB)			
		unsigned int retVal = EM_ASM_INT({
			var result = getScrollbarWidth();
		return result;
			});
		scrollBarWidthInPixels = retVal;
#else
		scrollBarWidthInPixels = 0;

		// because: 
		//	uwp: the scrollbar is autohidding: no issue
		//	android: no real scrollbar, no issue
		//	ios: no real scrollbar, no issue
#endif
	}

	// New system: now a scrollview content is NOT in the DOM (the scrollview content is like another DOM)(kinda)

#if defined(CUICUI_UWP)
	uwpScrollViewer = ref new ScrollViewer();
	if (orientation == wScrollView::Orientation::Vertical)
	{
		uwpScrollViewer->VerticalScrollBarVisibility = ScrollBarVisibility::Auto;
		uwpScrollViewer->HorizontalScrollBarVisibility = ScrollBarVisibility::Hidden;
	}
	else
	{
		uwpScrollViewer->VerticalScrollBarVisibility = ScrollBarVisibility::Hidden;
		uwpScrollViewer->HorizontalScrollBarVisibility = ScrollBarVisibility::Auto;
	}

	assert(b == nullptr);


	b = ref new Grid(); // note: this Grid is useful so user can use BgColor (and others) on scrollview (note: don't replace this with a Canvas or scroll doesn't work anymore)
	((Grid^)b)->Children->Append(uwpScrollViewer);


	uwpScrollViewer->ViewChanging += ref new EventHandler<ScrollViewerViewChangingEventArgs^>([this](Object ^sender, ScrollViewerViewChangingEventArgs ^args)
	{
		if (onUserScrolled != nullptr) onUserScrolled();
	});
#endif
#if defined(CUICUI_IOS)

	uiScrollView = [[CuiUIScrollView alloc] init]; // CLEANUP: need to set a size?
	[uiView addSubview : uiScrollView];
	[uiScrollView setScrollEnabled : YES];
    uiScrollView.userInteractionEnabled = YES;

    [uiScrollView setPtr:this];
	[uiScrollView setDelegate : this->uiScrollView]; // so we can receive "scrollViewWillBeginDragging" event
    uiView = (CuiUIView*)uiScrollView;

	// on ios we hide scrollbars by default (for now at least) (careful, it's important for an app that this stays NO)
	[uiScrollView setShowsHorizontalScrollIndicator:NO];
	[uiScrollView setShowsVerticalScrollIndicator:NO];

											   // potentially useful:
											   // https://stackoverflow.com/questions/5095713/disabling-vertical-scrolling-in-uiscrollview
											   // self.automaticallyAdjustsScrollViewInsets = NO;

#endif
#if defined(CUICUI_ANDROID)
	int type = 0;
	if (orientation == Orientation::Horizontal) type = 1;
	scrollViewContentTheId = JNICall_Int_IntInt("cuiScrollView_CreateAndAdd", theId, type);
#endif
#if defined(CUICUI_WEB)

	htmlId = htmlIdCount;
	htmlIdCount++;

	EM_ASM_({
		var contentView = gu($1);

		var sv = document.createElement('div');
		var num = $0;
		sv.id = num.toString();

		if ($2 == 1) // vertical
		{
			sv.style["overflow-x"] = "hidden"; // so that it's not scrollable
			sv.style["overflow-y"] = "scroll"; // so that it's scrollable
		}
		else // horizontal
		{
			sv.style["overflow-x"] = "scroll"; // so that it's scrollable
			sv.style["overflow-y"] = "hidden"; // so that it's not scrollable
		}
		sv.className = "b";
		sv.style["pointer-events"] = "auto";

		sv.addEventListener("scroll", function(e) { Module.ccall('WebOnUserScrolled', 'void', ['number'], [$3]);  });
		
		addUnattachedElements(sv, num.toString());

	}, this->htmlId, scrollViewContentWidget->htmlId, (orientation==Orientation::Vertical) ? 1 : 0, this);	
#endif		

	DebugName("wScrollView");
}

#ifdef CUICUI_WEB
extern "C" {
void EMSCRIPTEN_KEEPALIVE WebOnUserScrolled(int widgetPtr)
{	
	wScrollView *sv = (wScrollView*)widgetPtr;
	if (!CheckDefWidgetAlive(sv)) return;

	if (sv->onUserScrolled != nullptr) sv->onUserScrolled();
}
}
#endif

wScrollView* wScrollView::Content(std::function<void(Children&dc)> a_content)
{	
	assert(scrollViewContentWidget == nullptr); // this scrollview already has a content	

	scrollViewContentWidget = new wEmpty();
	scrollViewContentWidget->XemYem(MIN, MIN);
	scrollViewContentWidget->DebugName("scrollViewContentWidget");
#if defined(CUICUI_ANDROID)
	int type = 0;
	if (orientation == Orientation::Horizontal) type = 1;
	JNICall_Void_IntIntInt("cuiScrollView_SetContent", this->theId, scrollViewContentWidget->theId, type);
#elif defined(CUICUI_UWP)
	uwpScrollViewer->Content = scrollViewContentWidget->b;
#elif defined(CUICUI_WEB)
	WEB_CuiAttachChildInNativeDom(this->htmlId, scrollViewContentWidget->htmlId);
#elif defined(CUICUI_IOS)
	[uiScrollView addSubview : scrollViewContentWidget->uiView];
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif


    
	scrollViewContentWidget->definitions_children = a_content;
	CreateUI_Recur(scrollViewContentWidget);

	this->children.push_back(scrollViewContentWidget); // we DONT use AddChildren() in purpose
	scrollViewContentWidget->parent = this;
	scrollViewContentWidget->addedToNativeDom = true;

#ifndef CUICUI_UWP // note: uwp widgets may have a "canvasBgColor"
	assert(scrollViewContentWidget->children.size() == 1); // a wScrollView::Content() must only contain 1 child
#endif
	
	return this;
}

pixels wScrollView::GetCurrentOffset()
{
#if defined(CUICUI_ANDROID)		
	int offset = JNICall_Int_IntInt("cuiScrollView_GetOffset", theId, orientation == Orientation::Vertical ? 0 : 1);	
	return pixels::CreateFromDouble((double)offset);	
#elif defined(CUICUI_UWP)
	if (orientation == Orientation::Vertical)
		return pixels::CreateFromDouble(uwpScrollViewer->VerticalOffset);
	else
		return pixels::CreateFromDouble(uwpScrollViewer->HorizontalOffset);
#elif defined(CUICUI_WEB)	


	unsigned int retVal = EM_ASM_INT({
		var elem = gu($0);
		if (elem == null) 
		{
			console.log("elem not found, err 6572");
			return 0;
		}
		var result;
		if ($1==1)
			result = elem.scrollTop;
		else
			result = elem.scrollLeft;		
		return result;
		}, this->htmlId, orientation == Orientation::Vertical ?1 :0);
	return pixels::CreateFromDouble(retVal);

#elif defined(CUICUI_IOS)
    if (orientation == Orientation::Vertical)
        return pixels::CreateFromDouble(uiScrollView.contentOffset.y);
    else
        return pixels::CreateFromDouble(uiScrollView.contentOffset.x);
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif
}

void wScrollView::ScrollToEnd() // bottom if vertical, right if horizontal
{
	// why this timer? --> most of the time if you call ScrollToEnd on a scrollview on whom you just added widgets, the ScrollToEnd won't work because the OS didn't yet computed the content. 								   				
	timerManager.Create(0.100, [this](Timer* timer)
	{
		AssertOnUiThread();
#if defined(CUICUI_ANDROID)
		JNICall_Void_IntInt("cuiScrollView_ScrollToBottom", theId, orientation==Orientation::Vertical? 0 : 1);

#elif defined(WINAPI_FAMILY)
		
		// version with animation
		//if (vertical)
		//	sv->ChangeView(nullptr, sv->ExtentHeight /*double.MaxValue*/, nullptr);
		//else
		//	sv->ChangeView(sv->ExtentWidth /*double.MaxValue*/, nullptr, nullptr);
		

		// without animation
		if (orientation == wScrollView::Orientation::Vertical)
			uwpScrollViewer->ScrollToVerticalOffset(uwpScrollViewer->ExtentHeight);
		else
			uwpScrollViewer->ScrollToHorizontalOffset(uwpScrollViewer->ExtentWidth);

#elif defined(__EMSCRIPTEN__)

		EM_ASM_({
		var num = $0;
		var elem = getUnattachedElementsFromId(num.toString());
		if (elem == null)
		{
			Module.print("wScrollview::ScrollToEnd() --> elem not found : " + num.toString());
			return;
		}

		var numc = $1;

		if ($1 == 1) // vertical
		{
			elem.scrollTop = elem.scrollHeight;
		}
		else // horizontal
		{
			elem.scrollLeft = elem.scrollWidth;
		}

			}, this->htmlId, (orientation==Orientation::Vertical) ? 1 : 0);
#elif defined(CUICUI_IOS)
		if (orientation==Orientation::Vertical)
		{
			CGPoint bottomOffset = CGPointMake(0, uiScrollView.contentSize.height - uiScrollView.bounds.size.height);
			[uiScrollView setContentOffset : bottomOffset animated : NO];
		}
		else
		{
			CGPoint bottomOffset = CGPointMake(uiScrollView.contentSize.width - uiScrollView.bounds.size.width, 0);
			[uiScrollView setContentOffset : bottomOffset animated : NO];
		}
#else
		missing
#endif
	});
}


} // namespace cui