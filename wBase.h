#pragma once

#ifdef CUICUI_WEB
namespace cui {
void WebOnTouchableEvent(wWidget *ptr, int eventType, float x, float y);
} // namespace cui
#endif



#ifdef CUICUI_IOS
// CLEANUP: rename to "CuicuiGestureRecognizerDelegate"?
@interface CuiVariousEventsTarget : NSObject <UIGestureRecognizerDelegate>
-(void)setPtr:(cui::wWidget*)_ptr;
-(BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer;
-(void)mouseWheelGesture:(UIPanGestureRecognizer *)gesture;
-(void)EnableMouseWheelEvent;
@end

@interface CuiUIView : UIView
-(void)setPtr:(cui::wWidget*)_ptr;
@end
#endif


namespace cui {
class cOnPanGestureArgs
{
public:
	enum class eState {
		Start,
		Run, 
		End,
	};
public:
	eState state; // TODO: android, web (done: uwp, ios)
    pixels x, y;
    pixels offsetX, offsetY ; // it's the full offset from the beggining of the gesture
    // note: before the was "deltaX/Y" which was the delta between previous event, it's deprecated as it causes laggy panning
    bool shiftKeyPressed=false;
    bool ctrlKeyPressed=false;
#ifdef CUICUI_IOS
    double radius = -1, forcePercentage = -1; // IOS ONLY !! // -1 = invalid
    double radiusTolerance=-1;
    // we could pass the UITouch instead? (if this stays ios only)
#endif
};

class cOnSwipeGestureArgs
{
public:
    int direction; // 4/6/8/2 (left/right/top/bottom)
};

class cOnPinchGestureArgs
{
public:
    enum class eState {
        Start,
        Run,
        End,
    };
public:
    eState state;
	pixels centerX, centerY; // center between the two fingers
	double scale; // note: it's not a delta anymore to prevent laggy pinching
};

class cOnPointerMovedArgs
{
public:
	pixels x, y;
#ifdef CUICUI_IOS
	double radius = -1, forcePercentage = -1; // IOS ONLY !! // -1 = invalid
    double radiusTolerance=-1;
    // we could pass the UITouch instead? (if this stays ios only)
#endif
};

#define cuicui_KeyModifier_None 0
#define cuicui_KeyModifier_Shift (1<<1) // ios only for now
#define cuicui_KeyModifier_Ctrl (1<<2)  // ios only for now


class wWidget
{
public: // protected

	std::string debugName; // protected/private	

	axis ax[2];
	std::list<wWidget*> children;
	wWidget* parent;
	bool deleted = false; // TODO: rename to "deletionInProgress"?
	bool removedFromDom = false;
    bool allowSimultaneouslyGesture = false; // IOS ONLY for now

    
    
#ifdef CUICUI_IOS
    CuiVariousEventsTarget *cvet = nullptr;
    UIVisualEffectView *visualEffectView = nullptr;
    UIVisualEffect *blurEffect = nullptr;
#endif
    
public: // protected
#if defined(CUICUI_UWP)
	FrameworkElement ^b; // note: I use Grids because it doesn't work with Canvas (changing the size of child Canvases is never applied, actualwidth/height always stays the same, namely the window size at app launch)
			// note2: I tried putting canvas back, it's roughly correct (except images, don't know why)
			//		  AND above all there's no bordrebrush/borderradius/... on Canvas; we could probably put a Border in the Canvas, but I guess the thickness will eat some space?
	// PERF: it would (maybe!) be faster with canvases rather than Grids? not sure but it would seem logical
	cCuiElement_uwp ^hd;
	Windows::UI::Input::GestureRecognizer ^recognizer = nullptr;
#endif
#if defined(CUICUI_IOS)
	CuiUIView *uiView;
    UIGestureRecognizer *uiGestureRecognizer = nullptr;
    void ios_subclasswrap_touchesBegan(NSSet *touches, UIEvent *event);
    void ios_subclasswrap_touchesMoved(NSSet *touches, UIEvent *event);
    void ios_subclasswrap_touchesEnded(NSSet *touches, UIEvent *event);
    
    
#endif
#if defined(CUICUI_ANDROID)
	int theId;
#endif
#if  defined(CUICUI_WEB)
	int htmlId;

	// note: js doesn't have "gestures" so we do manually here:
	// note: we should find a lib i think : hammer.js for example
	// BUT there's elem.style["touch-action"], isn't it meant to receive gestures? -> I don't think so, and it's NOT supported everywhere
	bool dragging = false;
	double draggingPrevX, draggingPrevY;
	double dragStartX, dragStartY;
	double cumulativeDraggingDistance = 0; 
#endif

    cOnPanGestureArgs currentOnPanGestureArgs; // only used by ios? normally shouldn't be needed but ios needs it for the end event

    int panNumberOfTouches=-1;
    
	bool addedToNativeDom = false;

	bool IsNative();

public:
	class cTouchableFeedback0 // rename to ClickableFeedback?
	{
	public:
		std::function<void(wWidget *self)> onClick = nullptr;
	};
	
	class cTouchableFeedback1 // rename to ClickableFeedback?
	{
	public:
		wWidget *cbg;	 // useless? is it always this? maybe not necessarily
		unsigned int hoverColor, pressedColor;
		std::function<void(wWidget *self)> onClick = nullptr;
		cColor previousBgColor;
	};
	
#ifdef CUICUI_IOS
    UIView *IosGetNativeWidgetFirstChild()
    {
        auto arr = [uiView subviews];
        if (arr.count==0) return nullptr;
        return arr[0];
    }
#endif
    
private:
	bool visibility = true;



public:
	std::unique_ptr<cTouchableFeedback0> touchableFeedback0 = nullptr;
	std::unique_ptr<cTouchableFeedback1> touchableFeedback1 = nullptr;
	int virtPos; // used by wScrollView_Virtualized
	eCoreType coreType = eCoreType::wWidget;	
	//wWidget *wScrollViewFakeOwner = nullptr; // note: this enables optim_5675, not great to have this in every defwidget, but no other solution for now (CLEANUP)
	bool opaqueToTouch = false;
	
	bool centerChildrenInBadAxis;
	//bool isNativeControl = false; // (deprecated? no longer used I think) true if text component, scrollview, textinput, etc...  // useful only for optim_6541
	double opacity = 1.0;
	bool alignRight = false;
	bool alignBottom = false;
	bool centerHorizontallyInParent = false;
	bool centerVerticallyInParent = false;
	
	int rowcolaxis;
	bool lockedXem, lockedYem;
	double _emRatio;
	
	
	bool border_use = false;
	bool private_pointerInIt = false; // cuicui internal only, dont use it
	cColor bgColor;
	wWidget *bgColorTarget; // this replace the needs of virtuals for BgColor/UnsetBgColor/GetColor (don't know if it's so much better but it works, to think about)
	bool touchableAdded;	
	int debugCountId;

#ifdef CUICUI_UWP
	Windows::Foundation::Point uwpDraggingStart; // position at the beginning of the current pan gesture
	bool uwp_doubleTapOccurred = false; // used by OnGesture_Tap when simpleTapConfirmed==true
#endif

	std::function<void(Children &dc)> definitions_children; // we should be able to do without it (CLEANUP); really?
	std::function<void(wWidget *self, pixels posX, pixels posY)> prv_onGesture_DoubleTap = nullptr;
	std::function<void(wWidget * self, pixels posX, pixels posY)> prv_onGesture_LongPress = nullptr;
    std::function<void(wWidget *self, cOnPanGestureArgs)> prv_onGesture_Pan = nullptr;	
	std::function<void(wWidget * self, cOnPanGestureArgs)> prv_onGesture_PanTwoFingers = nullptr;
	std::function<void(wWidget * self, pixels posX, pixels posY)> prv_onGesture_Tap = nullptr;
	std::function<void(wWidget * self, cOnPinchGestureArgs)> prv_onGesture_Pinch = nullptr;
    std::function<void(wWidget * self, cOnSwipeGestureArgs)> prv_onGesture_Swipe = nullptr;
    
	

    // 
	// OnGesture_Tap : When using both simpleTap and doubleTap you should set simpleTapConfirmed to true
	//				   This will prevent simpleTap to trigger to early (before the doubleTap happen)
	//                 (Single tap will trigger more slowly though, this is expected behavior)
	//				
	wWidget* OnGesture_Tap(std::function<void(wWidget*, pixels x, pixels y)> simpleTap=nullptr,
							 std::function<void(wWidget*, pixels x, pixels y)> doubleTap=nullptr,
							 bool simpleTapConfirmed = false); // WARNING: not handled on ios for now? web neither?
    
	void OnGesture_LongPress(std::function<void(wWidget*, pixels x, pixels y)> longPress); // [X] ANDROID, [X] UWP (touch only, doesn't work with mouse!), [ ] iOS, [ ] WEB

    void OnGesture_Pan(std::function<void(wWidget*, cOnPanGestureArgs)> panOneFinger=nullptr,
                       std::function<void(wWidget*, cOnPanGestureArgs)> panTwoFingers=nullptr);
    
    void OnGesture_Pinch(std::function<void(wWidget*, cOnPinchGestureArgs)> pinch=nullptr);
    void OnGesture_Swipe(std::function<void(wWidget*, cOnSwipeGestureArgs)> swipe=nullptr);
    
    
	// DEPRECATED
	//std::function<void(wWidget* self, std::string type, double a, double b, double c, double d)> onGesture=nullptr; // if Touchable(true) was called
		// DEPRECATED type == "translate" -> params: deltaX, deltaY, posX, posY (in pixels)
		// type == "scale" -> params: scale, ignored, scaleCenterX, scaleCenterY (in pixels)
		// type == "tap" -> params: none // TODO: not done? or only on one platform?

	// do we keep all this? or switch to "all gesture" (probably not a good idea but well)
	std::function<void(wWidget* self, pixels x, pixels y)> onPointerReleased=nullptr; // the widget must have Touchable() for this to work
	std::function<void(wWidget* self, pixels x, pixels y)> onPointerPressed=nullptr; // the widget must have Touchable() for this to work
	std::function<void(wWidget* self, cOnPointerMovedArgs args)> onPointerMoved=nullptr; // the widget must have Touchable() for this to work
        // note: on ios there is a "HoverGesture"
	std::function<void(wWidget* self, pixels x, pixels y)> onPointerExited=nullptr; // the widget must have Touchable() for this to work
	std::function<void(wWidget* self, pixels x, pixels y)> onPointerEntered=nullptr; // the widget must have Touchable() for this to work
    std::function<void(wWidget*, pixels width, pixels height)> onSizeChanged = nullptr;
	std::function<void(int, double, double, int)> onMouseWheel = nullptr;
	pixels hwPosOffsetX, hwPosOffsetY;

	class cPrevPosHwValues
	{
	public:
		pixels setPosHw_prev_x;
		pixels setPosHw_prev_y;
		double setPosHw_prev_width;
		double setPosHw_prev_height;
		double setPosHw_prev_emRatio;
		Length setPosHw_prev_widthInEm_optim_5675;
		Length setPosHw_prev_heightInEm_optim_5675;

		void Clear()
		{
			setPosHw_prev_x.Unset();
			setPosHw_prev_y.Unset();;
			setPosHw_prev_width = -10000;
			setPosHw_prev_height = -10000;
			setPosHw_prev_emRatio = -1;
			setPosHw_prev_widthInEm_optim_5675 = unset();
			setPosHw_prev_heightInEm_optim_5675 = unset();
		}
		bool IsTheSame(const cPrevPosHwValues &other)
		{
			return !IsNotTheSame(other);
		}
		bool IsNotTheSame(const cPrevPosHwValues &other)
		{
			bool b = memcmp(this, &other, sizeof(cPrevPosHwValues));
			return b;
		}
	};
	cPrevPosHwValues prevs;

//protected:
public:
	virtual wWidget* LinkToHardwareBackButton(); 

public:
	class BorderStyle
	{
	public:
		pixels thicknessInPixels = pixels(0); // TODO_6575: must be ems_or_pixels!
		cColor color;				
        ems_or_pixels cornerRadiusInEm=pixels(0);
		bool cornerTopLeft = false, cornerTopRight = false, cornerBottomLeft = false, cornerBottomRight = false; // TODO: if the user passes "0.3" by mistake the compiler gives no error, error prone
        
		BorderStyle()
		{
			color.Set(0x00000);
		}		
        void SetAllCornersRadius(ems_or_pixels _cornerRadiusInEm) // set all corners
        {
            cornerRadiusInEm = _cornerRadiusInEm;
            cornerTopLeft=cornerTopRight=cornerBottomLeft=cornerBottomRight=true;
        }

		BorderStyle(ems_or_pixels _cornerRadiusInEm, cColor _color, pixels _thickness)
		{
			SetAllCornersRadius(_cornerRadiusInEm);
			color = _color;
			thicknessInPixels = _thickness;
		}
	};
	BorderStyle border2_args;

public:
	void Reset_setPosHw_prev_values();
	static void ZeroMaxSizeInMinAll_Recur(AXIS axis, wWidget *w);		
	void ClipChildren() { Native(); } // why? if the parent of a widget is not in the native dom, the children are not clipped // forcing the native dom does clip the children		
	void ZeroMaxSizeInMinInChildren(AXIS axis) { for (auto c : children) c->ax[axis].maxSizeInMin = 0; }
	bool IsFixed(AXIS axis) { if (GetEm(axis).IsSameAsX()) return false; if (GetEm(axis).IsMin()) return false;	if (GetEm(axis).IsFill()) return false; return true; }
	bool IsMin(AXIS axis) {	return (GetEm(axis).IsMin()); }
	bool IsFill(AXIS axis) { return (GetEm(axis).IsFill()); }
	bool NoChild() { return (children.size() == 0); }	
	wWidget* Visibility(bool show);		// false = widget is not visible AND not touchable/clickable/gesturable
											// WARNING! on WEB if the widget is Visibility(false) when created, its layout may not be correct when doing Visibility(true)											
											// TODO_2315: how to fix this?
	bool Visibility() { return visibility;  }
	wWidget* DebugName(const std::string &t);		
	void ApplyDebugNameNative(); // private
	wWidget* GetChild(int idx);	
	wWidget* Center() { centerHorizontallyInParent = centerVerticallyInParent = true; return this; }	
	wWidget* CenterHorizontally() { centerHorizontallyInParent = true; return this; }
	wWidget* CenterVertically() { centerVerticallyInParent = true; return this; }	
	wWidget* AlignRight() { alignRight = true; return this; } // TODO: cui_assert(xem!=FILL) (otherwise it has no effect)
	wWidget* AlignBottom() { alignBottom = true; return this; } // TODO: cui_assert(yem!=FILL) (otherwise it has no effect)
	bool IsFullyTransparent();
	Length GetEm(AXIS axis);
	void UpdateNativeFontsIfNeeded(double _currentEmRatio); // should not be public
	bool IsThisWidgetOneOfMyParent(wWidget *w);
	void SetPixels(int axis, int pixels); // PRIVATE!
	int GetPixels(int axis); // PRIVATE!
	void SetEm(int axis, Length _em);		
	virtual bool ComputeMinSize(double *w, double *h) { return false; }
	virtual wWidget* Content(std::function<void(Children &dc)> ff) { definitions_children = ff; CreateUI_Recur(this); return this; }
	void RecreateContent();
	wWidget* IsInsideAScrollView();		
	wWidget* Native();
	void EnsureNative();
	void ResetAllSetPosHwPrevsRecursively(); // private
	void AddChild_private(wWidget *w, int atPos = INT_MAX);
	void Private_RemoveMeFromDom();
	wWidget* HasParentInNativeDom();
	void DoAddInNativeDom();
	bool IsInNativeDom() { return addedToNativeDom; }
	void TransformToParentInDom(pixels &x, pixels &y);
	bool IsReadyToSetPosHW();		
	wWidget* Touchable(bool withGestures = false);
	virtual	wWidget* Clickable(int type, std::function<void(wWidget *self)> onClick); // note: linkToHardwareBackButton -> available on android, on uwp (win+backspace) and on web ("back" arrow); NOT available on ios
	void TriggerClickable(); // is this useful? and does it work?
	wWidget* Border_Private( // deprecated, user Border()
		pixels thicknessInPixels, unsigned int color, double alpha, ems_or_pixels cornerRadiusInEm  /*warning: these are EMs, not pixels*/); // TODO: need an em "type" to avoid confusion		
	wWidget* Border(const BorderStyle& args); 
	void Apply_Border_Private2();
	wWidget *XemYem(Length x, Length y);
	wWidget* XemYem(Length x_and_y);	
	wWidget* Xem(Length x);
	wWidget* Yem(Length y);
	cColor GetBgColor();
	void UnsetBgColor();
	wWidget* BgColor(cColor col);
    wWidget* BlurBackground(); // ios only for now
	void SetPos_HW(pixels x, pixels y, double width, double height);

	// Extension point for widgets defined outside the core (modules):
	// called at the end of SetPos_HW(), once the native view has its new position and size.
	virtual void OnNativeFrameChanged(double width, double height) {}
	void SetPos_HW2();
	void LockXem(bool b=true) { lockedXem = b; } // controls that have a fixed size should call that to prevent user to change Xem/Yem
	void LockYem(bool b=true) { lockedYem = b; } // controls that have a fixed size should call that to prevent user to change Xem/Yem
	wWidget* ChangeEmRatio(double _value);
	wWidget* SetOpaqueToTouch();
	double emRatio(); // TODO: rename to GetEmRatio() or EmRatio()	
	void EnableDropFile(); // Dropped file will be received by the app in the onFileReceived event. Any widget can received dropped files						   		
	wWidget* GetTouchableInMeOrParents();  // rename: this tells if this or one of its parents is touchable (but it doesn't search in the children of "this", the name is confusing)
	wWidget();
	virtual ~wWidget() { }		
	void GetLocalPos(pixels xInMaster, pixels yInMaster, pixels *localX, pixels *localY); // cuicui internal usage only 
	void DeleteMe(bool deleteNativeStuff = true);
	void DeleteMe_cuiParts();	
	pixels ConvertEmsRemsToPixels(ems_or_pixels v);
	wWidget* Opacity(double _opacity); // opacity of this widget AND its children 
	wWidget* OnMouseWheel(std::function<void(int, double, double, int/*eKeyModifier*/)> f); // available on UWP and WEB; Note: the "int" is <0 or >0 (the sign is meaningful, the value ISN'T)
																											// not sure, need to check, on web there are several possibilities,
																											// on UWP I think it's always -120 or +120
					
};

} // namespace cui