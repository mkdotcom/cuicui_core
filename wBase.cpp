#include "cuicui.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include "wBase.h"


namespace cui {
void wWidget::GetLocalPos(pixels xInMaster, pixels yInMaster, pixels *localX, pixels *localY) 
{
	double outX, outY;
	ConvertCoordinates(this, master, 0, 0, &outX, &outY); // position of "this" in master space

	pixels x = xInMaster - pixels::CreateFromDouble(outX);
	pixels y = yInMaster - pixels::CreateFromDouble(outY);
	*localX = x;
	*localY = y;
}

wWidget* wWidget::Border_Private(pixels thicknessInPixels, unsigned int color, double alpha, ems_or_pixels cornerRadiusInEm  /*warning: these are EMs, not pixels*/) // TODO: need an em "type" to avoid confusion
{
	BorderStyle args;
	args.thicknessInPixels = thicknessInPixels;
	args.color.Set(color, alpha);
    args.cornerRadiusInEm = cornerRadiusInEm;
	args.cornerTopLeft = true;
	args.cornerTopRight = true;
	args.cornerBottomLeft = true;
	args.cornerBottomRight = true;
	Border(args);
	Apply_Border_Private2();
	return this;
	return this;
}

wWidget* wWidget::Border(const BorderStyle& args)
{
	EnsureNative();

	border_use = true;
	border2_args = args;
	if (HasParentInNativeDom())
		Apply_Border_Private2();
	return this;
}



pixels wWidget::ConvertEmsRemsToPixels(ems_or_pixels v)
{
    if (v.IsPixels()) return pixels::CreateFromDouble(v.v);
	if (v.IsEms())  return pixels::CreateFromDouble(GetEmSize(v.v * this->emRatio()));
	if (v.IsRems()) return pixels::CreateFromDouble((int)(v.v * central.defaultEmSize.asDouble()));
	cui_assert(false);
	return pixels::CreateZero();
}

void wWidget::Apply_Border_Private2() 
{
	if (debugName == "test8784")
		CUICUI_NOP

	if (border_use == false) return;

	cui_assert(IsNative());	
	

	BorderStyle &args = border2_args;

#if defined(CUICUI_IOS)
    uiView.layer.borderColor = args.color.ToUIColor().CGColor; 
	uiView.layer.borderWidth = args.thicknessInPixels.asDouble();
    uiView.layer.cornerRadius = ConvertEmsRemsToPixels(args.cornerRadiusInEm).AsDouble();
    CACornerMask mask = 0;
    if (args.cornerTopLeft) mask |= kCALayerMinXMinYCorner;
    if (args.cornerTopRight) mask |= kCALayerMaxXMinYCorner;
    if (args.cornerBottomLeft) mask |= kCALayerMinXMaxYCorner;
    if (args.cornerBottomRight) mask |= kCALayerMaxXMaxYCorner;
    uiView.layer.maskedCorners = mask;
	uiView.layer.masksToBounds = true;
#endif
#if defined(CUICUI_UWP)
	
	if (coreType == eCoreType::Image) // TODO_uwp_56712
		return;

	Grid^ g = (Grid^)b;
	g->BorderBrush = border2_args.color.ToUWPSolidColorBrush();
	g->BorderThickness = Thickness(args.thicknessInPixels.AsDouble(), args.thicknessInPixels.AsDouble(), args.thicknessInPixels.AsDouble(), args.thicknessInPixels.AsDouble());

	double v = this->ConvertEmsRemsToPixels(args.cornerRadiusInEm).AsDouble();
	g->CornerRadius = CornerRadius(args.cornerTopLeft?v:0,
									args.cornerTopRight?v:0,
									args.cornerBottomRight?v:0,
									args.cornerBottomLeft?v:0);
	
#endif
#if defined(CUICUI_ANDROID)	
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiElement_Border", "(IIIII)I");	
	// TODO: alpha
	int corners = 0;
	if (args.cornerTopLeft) corners |= 1;
	if (args.cornerTopRight) corners |= 2;
	if (args.cornerBottomRight) corners |= 4;
	if (args.cornerBottomLeft) corners |= 8;
	int tip = (int)args.thicknessInPixels.AsDouble();
	int rgb = (int)args.color.rgb;
	globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, theId, tip, rgb, (int)this->ConvertEmsRemsToPixels(args.cornerRadiusInEm).asDouble(), corners);
#endif
#if defined(CUICUI_WEB)
			// TODO: alpha + per-corner corners
	EM_ASM_({
		var num = $0;
	var elem = getUnattachedElementsFromId(num.toString());
	if (elem == null)
	{
		Module.print("cCuiElement::Border() --> elem not found : " + num.toString());
		return;
	}

	var col = "rgba(" + $2 + "," + $3 + "," + $4 + "," + $5 + ")";

	elem.style["border-width"] = $1.toString() + "px";
	elem.style["border-color"] = col;
	elem.style["border-radius"] = $6 + "px";
	elem.style["border-style"] = "solid";
	elem.style["overflow"] = "hidden"; // makes the content clipped when there's a border radius


		}, htmlId, args.thicknessInPixels.AsDouble(), args.color.cr, args.color.cg, args.color.cb, args.color.ca, (int)this->ConvertEmsRemsToPixels(args.cornerRadiusInEm).asDouble());
#endif		
	return;
}

wWidget* wWidget::BlurBackground()
{
#ifdef CUICUI_IOS
    blurEffect = [UIBlurEffect effectWithStyle:UIBlurEffectStyleSystemThinMaterialDark];
    visualEffectView = [[UIVisualEffectView alloc] initWithEffect:blurEffect];
    visualEffectView.frame = uiView.bounds;
    [uiView addSubview:visualEffectView];
#else
    // not available yet
#endif
    
    return this;
}

wWidget* wWidget::Native()
{
	if (IsNative()) return this;
	cui_assert(parent == nullptr); // Native() MUST be called before the widget is added to the dom

#if defined(CUICUI_ANDROID)
	// TODO
#elif defined(CUICUI_UWP)
	hd = ref new cCuiElement_uwp(this);
	b = ref new Grid();
#ifdef CUICUI_DEBUG
	b->Name = ""; // improve readability while debugging, please look at note_248 for more info
#endif
//b->IsHitTestVisible = false; // NO because doing this on master for example makes ALL children non-clickable (even if they set "true" themselves)
#elif defined(CUICUI_WEB)
	htmlId = htmlIdCount;
	htmlIdCount++;
	EM_ASM_({
		var iDiv = document.createElement('div');
		var num = $0;
		iDiv.id = num.toString();
		//iDiv.className = 'block'; // TODO: not useful I think?

		// put in class "b" in cuicui.css
		//iDiv.style['position'] = "absolute";
		//iDiv.style["box-sizing"] = "border-box"; // not sure what this is for, may be useful or not // https://css-tricks.com/international-box-sizing-awareness-day/			
		//iDiv.style["overflow"] = "hidden";
		iDiv.className = "b";

		addUnattachedElements(iDiv, num.toString());
		}, htmlId);
#elif defined(CUICUI_IOS)
	uiView = [[CuiUIView alloc]init];
	[uiView setPtr : this] ;
	uiView.userInteractionEnabled = YES; // NO would be logical (since Touchable() sets YES) but setting NO on a parent makes@ all children NO too, not ok for cuicui

	uiView.clipsToBounds = YES; // important (don't know if it's bad for perf, possibly; but without it there's a problem in an app's toolbar as soon as some overlay item is present -> the others show superimposed; we could fix it on the app side, but it would happen again elsewhere; it seems mandatory for cuicui)

	[uiView setOpaque : NO] ;
	// uiView.backgroundColor = [UIColor clearColor]; // default

#elif defined(CUICUI_MACOS)
	missing
#else
	non_implemented_platform
#endif

	ApplyDebugNameNative();

	return this;
}

void wWidget::ApplyDebugNameNative()
{

#ifdef CUICUI_DEBUG
#ifdef CUICUI_UWP		
	b->Name = StdStringToUwpString(debugName); // note_248: this allows to see the DebugName in the "live tree" window of visual studio when debugging
#endif
#ifdef CUICUI_WEB
	EM_ASM_(
		{
			var elem = gu($0);
			if (elem == null) { console.log("error 57451:"); return; }
			var theDebugName = UTF8ToString($1);
			elem.setAttribute('debugName', theDebugName);
		}, htmlId, debugName.c_str());
#endif
#endif
}

wWidget* wWidget::Xem(Length x)
{ 
	cui_assert(!lockedXem); 
	ax[X].realEm = x; 	

	if (x.IsPixels())
		ax[X].size = x.v;

	

	return this; 
}



wWidget* wWidget::Yem(Length y)
{ 
	cui_assert(!lockedYem); 
	ax[Y].realEm = y; 

	if (y.IsPixels())
		ax[Y].size = y.v;

	

	return this; 
}

wWidget* wWidget::XemYem(Length x_and_y)
{
	Xem(x_and_y);
	Yem(x_and_y);
	return this;
}

wWidget* wWidget::XemYem(Length x, Length y)
{
	Xem(x);
	Yem(y);
	return this;
}

void wWidget::ZeroMaxSizeInMinAll_Recur(AXIS axis, wWidget *w)
{
	w->ax[axis].maxSizeInMin = 0;
	for (auto c : w->children)
		ZeroMaxSizeInMinAll_Recur(axis, c);
}

wWidget* wWidget::GetTouchableInMeOrParents()
{
	wWidget *p = this;
	while (p != nullptr)
	{
		if (p->touchableAdded == true)
			return p;
		p = p->parent;
	}
	return nullptr;
}

wWidget* wWidget::IsInsideAScrollView()
{
	wWidget *w = this;
	do
	{
		if (w->coreType == eCoreType::ScrollView)
		{
			return w;
		}
		w = w->parent;
	} while (w);
	return nullptr;
}

wWidget* wWidget::Visibility(bool show)
{
	cui_assert(IsNative()); //

	visibility = show;

    
#if defined(CUICUI_UWP)
	if (!show)
		this->b->Visibility = Visibility::Collapsed; // hidden
	else
		this->b->Visibility = Visibility::Visible; // visible
#elif defined(CUICUI_ANDROID)
	if (!show)
		JNICall_Void_IntInt("cui_ChangeVisibility", this->theId, 0); // hidden
	else
		JNICall_Void_IntInt("cui_ChangeVisibility", this->theId, 1); // visible
#elif defined(CUICUI_WEB) 	
	int s = show == true ? 1 : 0;
	EM_ASM_(
		{
			var num = $0;
			var elem = getUnattachedElementsFromId(num.toString());
			if (elem == null)
			{
				Module.print("Visibility() -> elem not found : " + num.toString());
				return;
			}
			if ($1 == 1)
				elem.style["display"] = "inline-block"; // visible
			else
				elem.style["display"] = "none"; // hidden
		}, this->htmlId, s
	);
#elif defined(CUICUI_IOS)
    [uiView setHidden:!show];
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif

	return this;
}

cColor wWidget::GetBgColor()
{
	if (bgColorTarget != this)
	{
		return bgColorTarget->GetBgColor();
	}
	return bgColor;
}

void wWidget::UnsetBgColor()
{
	if (bgColorTarget != this)
	{
		bgColorTarget->UnsetBgColor();
		return;
	}
	bgColor.Nullify();

#if defined(CUICUI_ANDROID)
	JNICall_Void_Int("cui_UnsetBgColor", this->theId);
#elif defined(CUICUI_UWP)
	// Note: on UWP we do "b->Background = ref new SolidColorBrush(Windows::UI::Colors::Transparent);" when calling Touchable()
	//          because without that we don't receive touch events on Touchable() that are transparents
		
	cui_assert(b != nullptr);
	Panel^ panel = (Panel^)b;
	if (touchableAdded)
		panel->Background = ref new SolidColorBrush(Windows::UI::Colors::Transparent);
	else
		panel->Background = nullptr;
	
#elif defined(CUICUI_WEB)
	EM_ASM_(
		{
			var elem = gu($0);
			if (elem == null)
			{
				Module.print("cText()::BackgroundColor() -> elem not found : " + $0.toString());
				return;
			}
			elem.style.backgroundColor = null; // ["background-color"] = ""; // "transparent" ?
		}, htmlId
	);
#elif defined(CUICUI_IOS)
	[uiView setOpaque : NO];
	uiView.backgroundColor = [UIColor clearColor];
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif						
}

void wWidget::EnsureNative() // DEPRECATE IT? -> make Native() the default and Virtual() up to the user? not decided
{
	if (IsNative() == false)
	{
		cui_assert(HasParentInNativeDom() == nullptr); // Native() can only be called during the creation phase, once the control is in the nativedom it's too late
		Native();		
	}
}


wWidget* wWidget::BgColor(cColor col)
{		
	EnsureNative();

	if (bgColorTarget != this)
	{
		bgColorTarget->BgColor(col);
		return this;
	}

	if (col.null == true)
	{
		UnsetBgColor();
		return this;
	}

	bgColor = col;

#if defined(CUICUI_IOS)
    cui_assert(uiView!=nullptr);
    [uiView setBackgroundColor : [UIColor colorWithRed : (float)bgColor.cr / 255.0f green : (float)bgColor.cg / 255.0f blue : (float)bgColor.cb / 255.0f alpha : bgColor.alpha]];
	[uiView setOpaque : YES];
#endif

#if defined(CUICUI_UWP)
	cui_assert(b != nullptr);
	Panel^ panel = (Panel^)b; 
	panel->Background = bgColor.ToUWPSolidColorBrush();	
	
#endif
#if defined(CUICUI_ANDROID)
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiElement_BackgroundColor", "(III)I");
	globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, theId, bgColor.GetARGB(), touchableAdded); // TODO: need to do something if "touchableddisabled==true"?
#endif
#if defined(__EMSCRIPTEN__) 			
	EM_ASM_(
	{
		var num = $0;
		var elem = getUnattachedElementsFromId(num.toString());
		if (elem == null)
		{
			Module.print("cText()::BackgroundColor() -> elem not found : " + num.toString());
			return;
		}
		var bgcol = "rgba(" + $1 + "," + $2 + "," + $3 + "," + $4 + ")";
		elem.style["background-color"] = bgcol;
	}, htmlId, bgColor.cr, bgColor.cg, bgColor.cb, bgColor.alpha);
#endif


	return this;
}


void wWidget::EnableDropFile() // dropped file will be received by the app in the onFileReceived event
						  // any widget can received dropped files
{
#ifdef CUICUI_UWP	
	b->AllowDrop = true;
	b->DragOver += ref new Windows::UI::Xaml::DragEventHandler(hd, &cCuiElement_uwp::OnDragOver);
	b->Drop += ref new Windows::UI::Xaml::DragEventHandler(hd, &cCuiElement_uwp::OnDrop);
#endif
#ifdef CUICUI_WEB
	EM_ASM_({
		console.log("EnableDropFile");
		var textinput = gu($0);

		textinput.addEventListener('drop', function(e){
			e.preventDefault();
			webOnDrop(e);
		});
		textinput.addEventListener('dragover', function(e){
			e.preventDefault();
		});

		}, htmlId);
#endif
	// other platforms: no mouse, so no drop
}

wWidget* wWidget::Opacity(double _opacity) // opacity of this widget and its children
{
	opacity = _opacity;
#if defined(CUICUI_ANDROID)
	JNICall_Void_IntDouble("cuiElement_Opacity", theId, opacity);
#elif defined(CUICUI_UWP)
	b->Opacity = opacity;
#elif defined(CUICUI_WEB)			  
	EM_ASM_(
		{
			var num = $0;
			var elem = getUnattachedElementsFromId(num.toString());
			if (elem == null)
			{
				Module.print("Opacity() -> elem not found : " + num.toString());
				return;
			}
			elem.style["opacity"] = $1;
		},
		htmlId, opacity);
#elif defined(CUICUI_IOS)
	uiView.alpha = (CGFloat)opacity;
#else
	missing
#endif
		return this;
}

double wWidget::emRatio() 
{
	double ratio = 1;
	wWidget *c = this;
	while (c)
	{
		cui_assert(c->_emRatio != 0);
		ratio *= c->_emRatio;
			c = c->parent;
	}
	return ratio;
}

int wWidget::GetPixels(int axis)
{
	cui_assert(GetEm((AXIS)axis).IsPixels());
	return (int)ax[axis].size;
}

void wWidget::SetPixels(int axis, int pixels)
{
	ax[axis].SetPixels(pixels);	
}

void wWidget::SetEm(int axis, Length _em)
{

	//if (GetEm((AXIS)axis) == _em) // already the right value  // PERF/TODO_87112: disabled because we don't handle "==" on Length; redo? not very useful but well
//		return; // no big deal?

		ax[axis].DontUseMePrivateSetEm(_em);


#ifdef CUICUI_DEBUG
#endif

}

Length wWidget::GetEm(AXIS axis)
{
	return ax[axis].DontUseMePrivateGetEm();
}

wWidget* wWidget::DebugName(const std::string & t)
{
#ifdef CUICUI_DEBUG
	debugName = t;
	if (IsNative())
		ApplyDebugNameNative();	
#endif
	return this;
}

void wWidget::Reset_setPosHw_prev_values()
{
	prevs.Clear();
	
}

void wWidget::Private_RemoveMeFromDom()
{
	if (parent == nullptr) return;
	cui_assert(children.size() == 0);  // this method doesn't remove children
	
	if (this->IsNative())
	{				
#if defined(CUICUI_ANDROID)
		JNICall_Void_Int("cuiElement_Delete", this->theId);		
#elif defined(WINAPI_FAMILY)			

		// note: there is no "this->b->DeleteMe()" in uwp, we must find the parent view and remove it there:
		int idx = 0;
		wWidget* parentInNativeDom = this->HasParentInNativeDom();
		cui_assert(parentInNativeDom);
		
		UIElement^ found = nullptr;
		Panel^ parentPanel = (Panel^)parentInNativeDom->b; // note: if it has children it's necessarily a panel
						
		if (parentInNativeDom->coreType == eCoreType::ScrollView)
		{
			found = parentPanel->Children->GetAt(0);
			idx = 0;
		}
		else
		{
			for each (UIElement ^ item in parentPanel->Children)
			{
				if (item == this->b)
				{
					found = item;
					break;
				}
				idx++;
			}
		}			
		if (found)
			parentPanel->Children->RemoveAt(idx);  // TODO: will removing "b" actually remove the native child if there is one (TextBox, ...)?		
		else
			cui_assert(false); // should not happen
	
		
#elif defined(__EMSCRIPTEN__)			
		EM_ASM_({
			removeFromUnattachedElementsAndFromDom($0);
		}, htmlId);
#elif defined(CUICUI_IOS)
		[uiView removeFromSuperview];
		// the "delegate" class instances: shouldn't we delete them?
#else
		missing
#endif
	}

	// remove it from cuicui ("virtual") DOM			
	parent->children.remove(this); // note: this DO NOT delete the wWidget in purpose, it's done later
	this->parent = nullptr; 
	this->removedFromDom = true;
}

void wWidget::DeleteMe(bool deleteNativeStuff)
{
	cui_assert(deleted == false);

	AssertOnUiThread();

	std::list<wWidget*> list;
	GetAllWidgetRecursively(list, this);
	for (auto w : list)
	{
		cui_assert(w->deleted == false);

		animationManager.DeleteTransitionsOwnedByThisWidget(w);

		// if it's touchable, remove it from the list
		if (w->touchableAdded)
			touchables.remove(w);

		auto f = std::find(central.hardwareBackButtonLinked.begin(), central.hardwareBackButtonLinked.end(), w);
		if (f != central.hardwareBackButtonLinked.end())
		{
			central.hardwareBackButtonLinked.erase(f);
		}

		

		w->deleted = true;
	}


		
	// now we remove the native widgets from native dom (and from the non native dom too actually)
	list.reverse();
	int deletedCount;
	std::vector<wWidget*> toDelete;
	do
	{
		deletedCount = 0;
		for (auto w : list)
		{
			if (w->removedFromDom==false)
			{
				if (w->children.size() == 0)
				{
					w->Private_RemoveMeFromDom(); // from native and non native dom
					toDelete.push_back(w);
					deletedCount++;
				}
			}
		}
	} while (deletedCount !=0);	
	
	// NOTE: we DO NOT delete the this and children's wWidget ptrs yet (as some events may arrives later)	
	// we use a "garbage collector" :
	timerManager.Create(10, [toDelete](Timer* timer)
	{
		for (wWidget* w : toDelete)
		{			
#ifdef CUICUI_DEBUG
			debugDeletedWidgets.push_back(w->debugCountId);
#endif
			delete w;
		}
	});
}


wWidget* wWidget::OnMouseWheel(std::function<void(int, double, double, int)> f)
{
    cui_assert(onMouseWheel==nullptr);
    
	onMouseWheel = f;
#ifdef CUICUI_UWP
	b->PointerWheelChanged += ref new PointerEventHandler([this](Platform::Object ^sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args)
	{
		args->Handled = true;
		if (!CheckDefWidgetAlive(this)) return;

		Windows::UI::Input::PointerPoint ^pp = args->GetCurrentPoint(this->b);
		auto prop = pp->Properties;
		int mwd = prop->MouseWheelDelta;
		float x = pp->Position.X;
		float y = pp->Position.Y;

		int keyModifier = 0; // todo

		if (this->onMouseWheel != nullptr) this->onMouseWheel(mwd, x, y, keyModifier);
	});
#elif CUICUI_WEB
	EM_ASM_({
		var elem = gu($0);
		if (elem == null) { Module.print("web: OnMouseWheel --> elem not found : " + $0); return; }



		elem.addEventListener("wheel", function(e)
		{
			// note: for some reason "e.clientX" in the event is not the local mouseX, but the window mouseX (same thing on Y)
			// (and offsetX/offsetY is not what we want it seems ?)
			var localMouseX = e.clientX;
			var localMouseY = e.clientY;

			var viewportOffset = this.getBoundingClientRect();
			localMouseX = localMouseX - viewportOffset.left;
			localMouseY = localMouseY - viewportOffset.top;

			Module.ccall('webOnMouseWheel', 'void',['number', 'number', 'number', 'number'],[$1, 0 - e.deltaY, localMouseX, localMouseY]);
			e.preventDefault();
		}, false);
		}, htmlId, (unsigned int)this);
#elif CUICUI_IOS
    [cvet EnableMouseWheelEvent];
#else
    // not handled
#endif

	return this;
};

#ifdef CUICUI_WEB
extern "C" {
	int EMSCRIPTEN_KEEPALIVE ontouchableevent(int elementId /*deprecated*/, unsigned int ptr, int eventType, float x, float y)
	{
		
		wWidget* touchable = (wWidget*)ptr;

		if (touchable == nullptr) return -1;
		if (!CheckDefWidgetAlive(touchable)) return -1;

		if (eventType == 1 && touchable->onPointerEntered)
		{
			touchable->onPointerEntered(touchable, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
			return 1;
		}
		if (eventType == 2 && touchable->onPointerExited)
		{
			touchable->onPointerExited(touchable, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
			return 1;
		}
		if (eventType == 3)
		{
			touchable->dragging = true;
			touchable->cumulativeDraggingDistance = 0;
			touchable->dragStartX = x;
			touchable->dragStartY = y;
			touchable->draggingPrevX = x;
			touchable->draggingPrevY = y;
			if (touchable->onPointerPressed != nullptr)
			{
				touchable->onPointerPressed(touchable, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
				return 1;
			}


			return -1;
		}
		if (eventType == 4) // pointer released
		{
			if (touchable->dragging)
			{
				double a = pow(touchable->dragStartX - x, 2);
				double b = pow(touchable->dragStartY - y, 2); 
				double distance = sqrt(a + b);
				//touchable->cumulativeDraggingDistance += fabs(distance); // note: we use cumulativeDraggingDistance because if user drag and then come back to the start position, it would not be identified as a tap (as the cumultaive distance won't be near to zero)
				if (distance/*cumulativeDraggingDistance*/ < 15) // TODO: how to decide this? on non touch device it could be <5 (or ==1?)
				{
					if (touchable->prv_onGesture_Tap != nullptr)
					{
						touchable->prv_onGesture_Tap(touchable, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
						return 1;
					}
				}
			}

			touchable->dragging = false;

			if (touchable->onPointerReleased != nullptr)
			{
				touchable->onPointerReleased(touchable, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
				return 1;
			}
		


			return -1;
		}

		if (eventType == 6) // ondblclick
		{
			if (touchable->prv_onGesture_DoubleTap != nullptr)
			{
				touchable->prv_onGesture_DoubleTap(touchable, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
				return 1;
			}

			return -1;
		}

		if (eventType == 5)
		{
			if (touchable->dragging)
				if (touchable->prv_onGesture_Pan != nullptr)
				{
                    cOnPanGestureArgs &args = touchable->currentOnPanGestureArgs;
					args.state = cOnPanGestureArgs::eState::Run;
					args.x.setFromDouble(x);
					args.y.setFromDouble(y);
					// deltaX/Y is deprecated in favor of offsetX/Y (cumulative offset since the start of the gesture, see wBase.h)
					args.offsetX.setFromDouble(x - touchable->dragStartX);
					args.offsetY.setFromDouble(y - touchable->dragStartY);
					touchable->prv_onGesture_Pan(touchable, args);

					touchable->draggingPrevX = x;
					touchable->draggingPrevY = y;

					return 1;
				}

			if (touchable->onPointerMoved != nullptr)
			{
				cOnPointerMovedArgs args;
				args.x = pixels::CreateFromDouble(x);
				args.y = pixels::CreateFromDouble(y);
				touchable->onPointerMoved(touchable, args);
				return 1;
			}


			return -1;
		}
		

		return -1;
	}
}
#endif

#ifdef CUICUI_IOS
} // namespace cui
#import <UIKit/UIGestureRecognizerSubclass.h>

@interface CustomPanGestureRecognizer : UIPanGestureRecognizer {
    
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;

@end

@protocol CustomPanGestureRecognizer <UIGestureRecognizerDelegate>

- (void) panGestureRecognizer:(UIPanGestureRecognizer *)gr movedWithTouches:(NSSet*)touches andEvent:(UIEvent *)event;


@end

@implementation CustomPanGestureRecognizer

-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [super touchesEnded:touches withEvent:event];
    
    if ([self.delegate respondsToSelector:@selector(panGestureRecognizer:movedWithTouches:andEvent:)])
    {
        [(id)self.delegate panGestureRecognizer:self movedWithTouches:touches andEvent:event];
    }
}
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    
    [super touchesMoved:touches withEvent:event];
    
    if ([self.delegate respondsToSelector:@selector(panGestureRecognizer:movedWithTouches:andEvent:)])
    {
        [(id)self.delegate panGestureRecognizer:self movedWithTouches:touches andEvent:event];
    }
    
}

@end

namespace cui {
#endif

wWidget* wWidget::Touchable(bool withGestures)
{
	if (touchableAdded) return this; // already Touchable

	cui_assert(IsNative());

	touchableAdded = true;
	touchables.push_back(this);


#if defined(CUICUI_IOS)
	// note: on ios all views listen to touches I think

	if (withGestures)
	{
        // note: this could go in each OnGesture_(), with "touchableadded", or it can stay here; CLEANUP
        cvet = [CuiVariousEventsTarget alloc];
        [cvet setPtr:this];
        
        // can't we use lambdas/blocks (and thus get rid of CuiVariousEventsTarget)?
	}
#endif
#if defined(CUICUI_UWP)        

	hd = ref new cCuiElement_uwp(this);

	if (withGestures)
	{
		b->IsHitTestVisible = true; // important

		recognizer = ref new Windows::UI::Input::GestureRecognizer();
		recognizer->GestureSettings = Windows::UI::Input::GestureSettings::DoubleTap | Windows::UI::Input::GestureSettings::ManipulationTranslateX | Windows::UI::Input::GestureSettings::ManipulationTranslateY;
	}

	b->IsHitTestVisible = true; // important

	b->PointerMoved += ref new PointerEventHandler(hd, &cCuiElement_uwp::touchRectangle_PointerMoved);
	b->PointerReleased += ref new PointerEventHandler(hd, &cCuiElement_uwp::touchRectangle_PointerReleased);
	b->PointerCaptureLost += ref new PointerEventHandler(hd, &cCuiElement_uwp::touchRectangle_PointerReleased); // lost sends a release
	b->PointerPressed += ref new PointerEventHandler(hd, &cCuiElement_uwp::touchRectangle_PointerPressed);
	b->PointerExited += ref new PointerEventHandler(hd, &cCuiElement_uwp::touchRectangle_PointerExited);
	b->PointerEntered += ref new PointerEventHandler(hd, &cCuiElement_uwp::touchRectangle_PointerEntered);

	// this allows transparent Touchable widgets to receive events (important_8716)	
	cui_assert(b != nullptr);
	Panel^ panel = (Panel^)b;
	if (panel->Background == nullptr)
		panel->Background = ref new SolidColorBrush(Windows::UI::Colors::Transparent);
	else
		CUICUI_NOP // it has a bg color, so we do nothing, it will respond to touches
	



		/* // potentially useful if multilinetext doesn't use "tb->IsTextSelectionEnabled = false;" (but it does by default for now)
		if (type == "multilinetext")
		{
			wText *w = (wText*)this;
			w->tb->SelectionChanged += ref new RoutedEventHandler(hd, &cCuiElement_uwp::multilinetext_SelectionChanged);
		}*/


#endif        
#if defined(CUICUI_WEB)
		
		EM_ASM_({
			var num = $0;
			var ptr = $1;
			var withGestures = $2;

			var elem = getUnattachedElementsFromId(num.toString());
			if (elem == null)
			{
				Module.print("err9845 --> elem not found : " + num.toString());
				return;
			}

			//elem.style["touch-action"] = "none"; // important: without this, browser may interpret touch gestures and prevent cuicui touch management to work correctly
												// problem: it blocks things: finger scrolling in scrollview for example

			elem.style["touch-action"] = "auto";
			elem.style["pointer-events"] = "auto";                                    

			// https://mobiforge.com/design-development/html5-pointer-events-api-combining-touch-mouse-and-pen

			elem.onpointerenter = function(e) 
			{ 				
				var viewportOffset = this.getBoundingClientRect();
				localMouseX = e.clientX - viewportOffset.left;
				localMouseY = e.clientY - viewportOffset.top;				
				var ret = Module.ccall('ontouchableevent', 'number', ['number', 'number', 'number', 'number', 'number'], [num, ptr, 1, localMouseX, localMouseY]);
			}; // 1= PointerEntered

			elem.onpointerleave = function(e)
			{ 
				var viewportOffset = this.getBoundingClientRect();
				localMouseX = e.clientX - viewportOffset.left;
				localMouseY = e.clientY - viewportOffset.top;				
				var ret = Module.ccall('ontouchableevent', 'number',['number', 'number', 'number', 'number', 'number'],[num, ptr, 2, localMouseX, localMouseY]);
			}; // 2= PointerExited

			elem.onpointerdown = function(e)
			{ 
				var viewportOffset = this.getBoundingClientRect();
				localMouseX = e.clientX - viewportOffset.left;
				localMouseY = e.clientY - viewportOffset.top;
				if (e.button == 0 /*left click*/) 
				{ 					
					var ret = Module.ccall('ontouchableevent', 'number',['number', 'number', 'number', 'number', 'number'],[num, ptr, 3, localMouseX, localMouseY]);
				}
			}; // 3= PointerPressed
			
			elem.onpointerup = function(e)
			{				
				var viewportOffset = this.getBoundingClientRect();
				localMouseX = e.clientX - viewportOffset.left;
				localMouseY = e.clientY - viewportOffset.top;
				if (e.button == 0 /*left click*/)
				{				
					var ret = Module.ccall('ontouchableevent', 'number',['number', 'number', 'number', 'number', 'number'],[num, ptr, 4, localMouseX, localMouseY]);
				}
			}; // 4= PointerReleased

			elem.onpointermove = function(e)
			{ 
				var viewportOffset = this.getBoundingClientRect();
				localMouseX = e.clientX - viewportOffset.left;
				localMouseY = e.clientY - viewportOffset.top;
				var ret = Module.ccall('ontouchableevent', 'number',['number', 'number', 'number', 'number', 'number'],[num, ptr, 5, localMouseX, localMouseY]);
			}; // 5= PointerMoved

			elem.ondblclick = function(e)
			{
				var viewportOffset = this.getBoundingClientRect();
				localMouseX = e.clientX - viewportOffset.left;
				localMouseY = e.clientY - viewportOffset.top;
				var ret = Module.ccall('ontouchableevent', 'number', ['number', 'number', 'number', 'number', 'number'], [num, ptr, 6, localMouseX, localMouseY]);
			}; // 1= PointerEntered

			}, htmlId, (unsigned int)this, withGestures);
#endif        
#if defined(CUICUI_ANDROID)
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiElement_Touchable", "(I)I");
	globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, theId);
#endif

	return this;
}


void wWidget::SetPos_HW2()
{
	SetPos_HW(ax[X].pos, ax[Y].pos, ax[X].size, ax[Y].size);
}

bool wWidget::IsNative()
{
#if defined(CUICUI_ANDROID)
	return (theId != -1);
#elif defined(CUICUI_UWP)
	return (b != nullptr);
#elif defined(CUICUI_WEB)
	return (htmlId != -1);
#elif defined(CUICUI_IOS)
	return (uiView != nullptr);
#elif defined(CUICUI_MACOS)
	missing
#else
	non_implemented_platform
#endif
}


wWidget * wWidget::GetChild(int idx)
{
	int i = 0;
	for (auto c : children)
	{
		if (i == idx) return c;
		i++;
	}
	return nullptr;
}

bool wWidget::IsFullyTransparent()
{
	if (opacity == 0) return true;
	if (bgColor.IsFullyTransparent()) return true;
	return false;
}

void wWidget::ResetAllSetPosHwPrevsRecursively()
{
	std::list<wWidget*> list;
	GetAllWidgetRecursively(list, this);
	for (auto w : list)
		w->Reset_setPosHw_prev_values();
} 

void wWidget::AddChild_private(wWidget * w, int atPos)
{
	if (coreType == eCoreType::Text)
	{
		auto te = (wText*)this;
		if (te->text.find("dynamically added te") == 0)
			CUICUI_NOP
	}
	
#ifdef CUICUI_DEBUG
	if (this->coreType == eCoreType::ScrollView)
		if (this->children.size() == 1) cui_assert(false); // a wScrollView can only have one child AND it MUST be set through Content()! please check your code
#endif
	if (atPos == INT_MAX)
		this->children.push_back(w);
	else
		if (atPos == 0)
			this->children.push_front(w);
		else
			cui_assert(false); // end or start only
	
	w->parent = this;
	
	this->Reset_setPosHw_prev_values(); // adding an element invalidates the setposhw cache

	if (w->IsNative())
	{
		if (this->HasParentInNativeDom())
		{
			// it's probably the user dynamically adding a widget (and its children), we can (and must) put in the dom right now
			std::list<wWidget*> list;
			GetAllWidgetRecursively(list, w);
			for (auto w : list)
				if (w->IsNative())
					w->DoAddInNativeDom();
		}
		else
		{
			// we probably are inside a "Content(Children &dc...)"
			// we cannot add the new widget in the dom, as its parents are not yet in the dom.
			// this will be done in the SetPosHWs later
		}
	}

	return;
	
}

wWidget * wWidget::HasParentInNativeDom()
{
	wWidget *p = this->parent;
	while (p != nullptr && p->IsNative() == false)
	{
		p = p->parent;
	}
	return p;
}

bool wWidget::IsThisWidgetOneOfMyParent(wWidget *w)
{
	if (w == this) return true;

	wWidget *curr = this;
	do
	{
			curr = curr->parent;
		if (curr == nullptr) return false;		
		if (curr == w) return true;	 			
	} while (1);
}

void wWidget::TransformToParentInDom(pixels &_x, pixels &_y) // use convertcoord? TODO
{
	// TODO: overload "+" in pixels and clean up this method
	double x = _x.asDouble();
	double y = _y.asDouble();

	wWidget* p = this->parent;
	while (p != nullptr && p->IsNative() == false)
	{
		x += p->ax[X].pos.asDouble();
		y += p->ax[Y].pos.asDouble();

		p = p->parent;
	}

	_x.setFromDouble(x);
	_y.setFromDouble(y);

}



#if defined(CUICUI_ANDROID)
void ANDROID_CuiAttachChildInNativeDom(int parentTheId, int childTheId)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiElement_AddToParentView", "(II)I");
	globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, parentTheId, childTheId);
}
#endif
#if defined(CUICUI_UWP)
void UWP_CuiAttachChildInNativeDom(Panel ^parent, UIElement ^child, int atPos)
{
	if (atPos == INT_MAX)
		parent->Children->Append(child);
	else
		if (atPos == 0)
			parent->Children->InsertAt(0, child);
		else
			cui_assert(false);
}
#endif
#if defined(CUICUI_WEB)
void WEB_CuiAttachChildInNativeDom(int parentHtmlId, int childHtmlId)
{
	EM_ASM_({
		var parentNum = $0;
		var childNum = $1;
		var parent = getUnattachedElementsFromId(parentNum.toString());
		if (parent == null)
		{
			Module.print("cHBox()::AddChild() --> parent not found : " + parentNum.toString());
			return;
		}
		var child = getUnattachedElementsFromId(childNum.toString());
		if (child == null)
		{
			Module.print("cHBox()::AddChild() --> child not found : " + childNum.toString());
			return;
		}
		// ok
		parent.appendChild(child);
	}, parentHtmlId, childHtmlId);
}
#endif


int gDebugUpdateNativeFontsIfNeeded = 0;

void wWidget::UpdateNativeFontsIfNeeded(double _currentEmRatio)
{

//#ifndef OPTIM_272_DISABLE 
	if (prevs.setPosHw_prev_emRatio != _currentEmRatio) // TODO: eventually we'll also need to check whether the font has changed (but the feature doesn't exist yet)
//#endif
	{
		gDebugUpdateNativeFontsIfNeeded++;

		prevs.setPosHw_prev_emRatio = _currentEmRatio;

		if (border_use)
			if (border2_args.cornerTopLeft ||
				border2_args.cornerTopRight ||
				border2_args.cornerBottomLeft ||
				border2_args.cornerBottomRight)
			{
				// this is important for proper emRatio handling
				Apply_Border_Private2();
			}

		if (coreType == eCoreType::Text)
		{
			wText* t = (wText*)this;
			t->UpdateRealFontSize();
		}
		if (coreType == eCoreType::TextInput)
		{
			wTextInput* t = (wTextInput*)this;
			t->UpdateRealFontSize();
		}

	}
}

void wWidget::DoAddInNativeDom()
{
	cui_assert(this->IsNative());

	if (addedToNativeDom == false && this != master)
	{
		wWidget* closestNativeParent = this->HasParentInNativeDom();
#ifdef CUICUI_DEBUG
		if (closestNativeParent==nullptr) {
			cui_assert(closestNativeParent != nullptr);
		}
#else
		if (closestNativeParent==nullptr) return; // avoid crash (but the ui won't be right)
#endif
			wWidget* w = this;

			cui_assert(w->IsInNativeDom() == false);
			cui_assert(closestNativeParent->IsNative());
			cui_assert(w->IsNative());

			w->addedToNativeDom = true;

#if defined(WINAPI_FAMILY)
			UWP_CuiAttachChildInNativeDom((Panel^)closestNativeParent->b, w->b, INT_MAX/*last position*/);
#endif
#if defined(CUICUI_WEB)
			WEB_CuiAttachChildInNativeDom(closestNativeParent->htmlId, w->htmlId);
#endif
#if defined(CUICUI_ANDROID)
			ANDROID_CuiAttachChildInNativeDom(closestNativeParent->theId, w->theId);
#endif
#if defined(CUICUI_IOS)
			[closestNativeParent->uiView addSubview : w->uiView];
#endif
#if defined(CUICUI_MACOS)
			missing
#endif

	}
}

int gDebugSetPosHwDone = 0;


void wWidget::RecreateContent()
{
	AssertOnUiThread();
	cui_assert(IsNative()); // must be Native() to use RecreateContent()  -> actually it's not sure, we should test it
	
	while (children.size())
	{
		children.front()->DeleteMe();
	}
	children.clear();

	CreateUI_Recur(this);
	RefreshUI_Fast(this);
}

void wWidget::SetPos_HW(pixels x, pixels y, double width, double height)
{
    if (debugName=="wActivityIndicator")
        CUICUI_NOP

	

	if (IsNative() == false)
	{
		if (onSizeChanged != nullptr)
			RefreshUI_Private_sces.push_back(this);
		return;
	}


	cui_assert(width != -666);
	cui_assert(height != -666);
	cui_assert(x.asDouble() != -666);
	cui_assert(y.asDouble() != -666);
	
	if (this->coreType == eCoreType::ScrollView)
		CUICUI_NOP

	if (IsInNativeDom() == false && this!=master)
	{
		this->DoAddInNativeDom();
	}

	TransformToParentInDom(x, y);

	x += hwPosOffsetX;
	y += hwPosOffsetY;

	// optim_6541 : not ok on android (see todo.txt)
	// useless now: another optim supersedes it (the "virtual/native" dom)
	/*if (0)
	{
		if (touchableAdded == false)
			if (isNativeControl == false)
				if (IsFullyTransparent())
					if (border_use==false)
						if (children.size() == 0)
							return;
	}*/

	double currentEmRatio = emRatio(); // TODO/PERF: emRatio() is recursive, whereas it could probably be computed during SetRefreshUI

	cPrevPosHwValues curr;
	curr.setPosHw_prev_x = x;
	curr.setPosHw_prev_y = y;
	curr.setPosHw_prev_width = width;
	curr.setPosHw_prev_height = height;
	curr.setPosHw_prev_emRatio = currentEmRatio;
	curr.setPosHw_prev_widthInEm_optim_5675 = this->GetEm(X);
	curr.setPosHw_prev_heightInEm_optim_5675 = this->GetEm(Y);


/*#ifdef OPTIM_272_DISABLE
	CUICUI_NOP
#else*/
#ifdef CUICUI_IOS
    if (coreType != eCoreType::ScrollView) // antibug ios only, it's because NOTE_2293
#endif
	if (prevs.IsTheSame(curr)) return;
//#endif

	gDebugSetPosHwDone++;

	UpdateNativeFontsIfNeeded(currentEmRatio);


	prevs = curr;



#if defined(CUICUI_IOS)        
    cui_assert(uiView);
    if (isnan(y.asDouble())) y.setFromDouble(0); // anticrash
    if (isnan(x.asDouble())) x.setFromDouble(0); // anticrash
	[uiView setFrame : CGRectMake(x.asDouble(), y.asDouble(), width, height)];
    [uiView setBounds: CGRectMake(0,0, width, height)];
    
    if (visualEffectView) // related to BlurBackground()
    {
        visualEffectView.frame = uiView.bounds;
    }
    
    if (coreType != eCoreType::Image) // special case for wImage, the same thing is done later
    {
        // TODO: IOS, we have to do this because we can't "autofit" wWidget's children; find the right config (or is it fine like this? enabling "autofit" could slow things down?
        // problem with this system is that an external module that needs to do something special won't be able to (worst case: make an overridable method, or a std::function)
       //[uiView setFrame : CGRectMake(0, 0, width, height)];
    }
    if (coreType == eCoreType::Gradient) // note: I added this for wGradient, need to check if it doesn't create issues elsewhere
    {
        [uiView.layer setFrame : CGRectMake(0, 0, width, height)];
        wGradient *g = (wGradient*)this;
        g->ApplyGradient(); // note: i had to do that here (for a "hue" slider in an app), it's not great for perfs (PERF)
    }
    
	if (coreType == eCoreType::Text) // TODO: IOS, we have to do this because we can't "autofit" wWidget's children; find the right config "autolayout/resizemask/whatever"
	{
		wText *t = (wText*)this;
        [t->uiTextView sizeToFit]; // this is very important (because UILabel centers text vertically, this make the text fit the frame, so the "vertical align" can't apply)
	}
    if (coreType == eCoreType::WebView) // TODO: IOS, we have to do this because we can't "autofit" wWidget's children; find the right config "autolayout/resizemask/whatever"
    {
        wWebView *t = (wWebView*)this;
        CUICUI_NOP
    }
	if (coreType == eCoreType::Image) // TODO: IOS, we have to do this because we can't "autofit" wWidget's children; find the right config "autolayout/resizemask/whatever"
	{
		CUICUI_NOP
	}
	if (coreType == eCoreType::TextInput) // TODO: IOS, we have to do this because we can't "autofit" wWidget's children; find the right config "autolayout/resizemask/whatever"
	{
		wTextInput *t = (wTextInput*)this;
        CUICUI_NOP
	}
	if (coreType == eCoreType::ScrollView)
	{
		wScrollView *sw = (wScrollView*)this;
        CUICUI_NOP
	}


	if (coreType == eCoreType::ScrollView)
	{
		// NOTE_2293: on ios we must tell the scrollview the size of its content
		wScrollView *sw = (wScrollView*)this;
		[sw->uiScrollView setContentSize : CGSizeMake(sw->scrollViewContentWidget->ax[X].size, sw->scrollViewContentWidget->ax[Y].size)];
	}
#endif

#if defined(CUICUI_UWP)

	

	// if we used Canvases we'd have to do this:
	//Canvas::SetLeft(b, x);
	//Canvas::SetTop(b, y);

	b->HorizontalAlignment = HorizontalAlignment::Left;
	b->VerticalAlignment = VerticalAlignment::Top;

	// but we do this because we use Grids:
	b->Margin = Thickness(x.asDouble(), y.asDouble(), 0, 0); // TODO: right/bottom not ok? I think it is since width/height works (should maybe double-check it anyway)
		
	// avoiding crash
	if (width < 0) width = 0;
	if (height < 0) height = 0;

	b->Width = width;
	b->Height = height;

#ifdef wListViewNative_ENABLED
	if (type == "listview") // for item virtualization in a listview to work we must give the exact ListView size in the wListView
	{
		wListView *lv = (wListView*)this;
		lv->listView->Width = width;
		lv->listView->Height = height;
	}
#endif

	// QUESTION/TODO_654:  is it good to clip everything? (perf problem?) do we also do it everywhere on the other platforms? need to check and standardize that
	// important for wImage
	auto rectangle = ref new Windows::UI::Xaml::Media::RectangleGeometry();
	rectangle->Rect = Rect(0, 0, (float)width, (float)height);
	b->Clip = rectangle;

#endif
#if defined(CUICUI_ANDROID)
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiElement_SetPos_HW", "(IIIII)I");
	globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, theId, (int)x.asDouble(), (int)y.asDouble(), (int)width, (int)height);
#endif
#if defined(CUICUI_WEB)
	EM_ASM_(
		{
			var num = $0;
			var elem = getUnattachedElementsFromId(num.toString());
			if (elem == null)
			{
				Module.print("--> elem not found : " + num.toString());
				return;
			}
			elem.style["left"] = $1.toString() + "px";
			elem.style["top"] = $2.toString() + "px";
			elem.style["width"] = $3.toString() + "px";
			elem.style["height"] = $4.toString() + "px";
		}, htmlId, x.asDouble(), y.asDouble(), width, height
	);


	if (this->coreType == eCoreType::Text)
	{
		// NOTE_7614: we need to set "line-height" style here
		// NOTE: this is very important ! without that, a wText with a emoticon will not be well aligned with other wText not containing emoticons
		EM_ASM_(
			{
				var elem = gu($0);
				if (elem == null) {
					console.log("error 5122");
					return;
				}
				elem.style["line-height"] = $1 + "px";  
			}, htmlId, central.defaultEmSize.asDouble()*emRatio()
		);			
	}
	

#endif

	if (coreType == eCoreType::Image)
	{
		wImage *im = (wImage*)this;

		if (im->resizeMode == wImage::AutoResize::Contain)
			im->Transform_FitContain();
		if (im->resizeMode == wImage::AutoResize::Cover)
			im->Transform_FitCover();

		// note: these events are called here because user may want to use the actual wImage size to do transforms
		if (im->loadState == wImage::eLoadState::Success)
		{
			if (im->onLoadSuccess != nullptr)
			{
				timerManager.Create(0.1, [this,im](Timer* timer) { // we do that because on android we saw issue when the user tries to set a new image in this callback
					if (CheckDefWidgetAlive(this))
						im->onLoadSuccess(im);
				});
			}
			im->loadState = wImage::eLoadState::None;
		}
		if (im->loadState == wImage::eLoadState::Fail)
		{
#ifdef CUICUI_DEBUG
			if (im->source.find("http") != 0)
			{
                Log("debug: missing asset file : " + im->source);
                CUICUI_NOP
			}
#endif

			if (im->onLoadFail != nullptr)
			{
				timerManager.Create(0.1, [this,im](Timer* timer) { // we do that because on android we saw issue when the user tries to set a new image in this callback
					if (CheckDefWidgetAlive(this))
						im->onLoadFail(im);
				});
			}
			im->loadState = wImage::eLoadState::None;
		}

		im->ApplyMatrix_HW();
	}

	OnNativeFrameChanged(width, height); // extension point for module widgets

	if (onSizeChanged != nullptr)
	{
		RefreshUI_Private_sces.push_back(this);
	}
}

#ifdef CUICUI_IOS



} // namespace cui
@implementation CuiVariousEventsTarget {
    wWidget *widgetPtr;
	double onGesture_Scale_centerX, onGesture_Scale_centerY;
    int count;
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer
shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    if (widgetPtr == nullptr) return NO;
    if (!CheckDefWidgetAlive(widgetPtr)) return NO;
    
    if (widgetPtr->allowSimultaneouslyGesture)
        return YES;
    
    return NO; // this allows two gestures simultaneously (useful for pinch and pan in an app)
}

-(void)EnableMouseWheelEvent
{
    if (widgetPtr == nullptr) return;
    if (!CheckDefWidgetAlive(widgetPtr)) return;
    
    UIPanGestureRecognizer *pan = [[UIPanGestureRecognizer alloc] initWithTarget:self action : @selector(mouseWheelGesture:)];
    pan.maximumNumberOfTouches = 0;
    pan.allowedScrollTypesMask = UIScrollTypeMaskDiscrete; //UIScrollTypeMaskDiscrete /*mouse for mac catalyst support*/ | UIScrollTypeContinuous /*trackpad and touch screen*/;
    // pan.allowedTouchTypes =  // note: all is the default
    pan.allowedTouchTypes = @[ @(UITouchTypeIndirectPointer) ];
    pan.cancelsTouchesInView=NO;
    pan.delaysTouchesBegan = NO;
    pan.delaysTouchesEnded = NO;
    [widgetPtr->uiView addGestureRecognizer : pan];
}


-(void)mouseWheelGesture:(UIPanGestureRecognizer *)gesture
{
    if (widgetPtr == nullptr) return;
    if (!CheckDefWidgetAlive(widgetPtr)) return;
    
    if (gesture.state == UIGestureRecognizerStateBegan ||
        gesture.state == UIGestureRecognizerStateChanged ||
        gesture.state == UIGestureRecognizerStateEnded)
    {
        if (widgetPtr->onMouseWheel != nullptr)
        {
            CGPoint delta = [gesture translationInView:widgetPtr->uiView];
            
            if (delta.y!=0)
            {
                CGPoint location = [gesture locationInView:widgetPtr->uiView];
                
                int km = cuicui_KeyModifier_None;
                if (gesture.modifierFlags == UIKeyModifierShift)
                {
                    km |= cuicui_KeyModifier_Shift;
                }
                if (gesture.modifierFlags == UIKeyModifierControl)
                {
                    km |= cuicui_KeyModifier_Ctrl;
                }
            
                // not really sure why but we must do this otherwise the mouse position in the uiView is wrong when moving the wheel quickly
                location.x = location.x - delta.x;
                location.y = location.y - delta.y;
            
                int way = delta.y<0?-1:1;
                widgetPtr->onMouseWheel(way, (float)location.x, (float)location.y, km);
            }
        }
    }
}

-(void)setPtr:(wWidget*)_ptr
{
    widgetPtr = _ptr;
}

-(void)handleSwipe:(UISwipeGestureRecognizer*)sender
{
    if (widgetPtr == nullptr) return; // the widget probably just got deleted
    if (!CheckDefWidgetAlive(widgetPtr)) return;
    
    if (widgetPtr->prv_onGesture_Swipe != nullptr)
    {
        cOnSwipeGestureArgs args;
        widgetPtr->prv_onGesture_Swipe(widgetPtr, args);
    }
}

-(void)handlePinch:(UIPinchGestureRecognizer*)sender
{
    if (widgetPtr == nullptr) return; // the widget probably just got deleted
    if (!CheckDefWidgetAlive(widgetPtr)) return;
    
    if (widgetPtr->prv_onGesture_Pinch != nullptr)
    {
        
        // the apple doc does this
        if (sender.state != UIGestureRecognizerStateBegan &&
            sender.state != UIGestureRecognizerStateChanged)
            return;
        
        cOnPinchGestureArgs::eState state;
        if (sender.state == UIGestureRecognizerStateBegan)
        {
            state = cOnPinchGestureArgs::eState::Start;
        }
        if (sender.state == UIGestureRecognizerStatePossible)
            return;
        if (sender.state == UIGestureRecognizerStateEnded)
            state = cOnPinchGestureArgs::eState::End;
        if (sender.state == UIGestureRecognizerStateCancelled)
            return;
        if (sender.state == UIGestureRecognizerStateFailed)
            return;
        if (sender.state == UIGestureRecognizerStateChanged)
            state = cOnPinchGestureArgs::eState::Run;

        CGPoint location = [sender locationInView:widgetPtr->uiView];
        
        onGesture_Scale_centerX = location.x;
        onGesture_Scale_centerY = location.y;
        
        cOnPinchGestureArgs args;
        args.scale = sender.scale;
        args.state = state;
        args.centerX.setFromDouble(onGesture_Scale_centerX);
        args.centerY.setFromDouble(onGesture_Scale_centerY);
        widgetPtr->prv_onGesture_Pinch(widgetPtr, args);
    }
}

- (void)panGestureRecognizer:(UIPanGestureRecognizer *)sender movedWithTouches:(NSSet*)touches andEvent:(UIEvent *)event{


    if (widgetPtr == nullptr) return; // the widget probably just got deleted
    if (!CheckDefWidgetAlive(widgetPtr)) return;
    
    auto tiv =  [sender translationInView:widgetPtr->uiView];


    UITouch *touch = [[event allTouches] anyObject];


    CGPoint location = [sender locationInView:widgetPtr->uiView];
    
    cOnPanGestureArgs::eState state;
    if (sender.state == UIGestureRecognizerStateBegan)
    {
        widgetPtr->panNumberOfTouches = (int)sender.numberOfTouches; // we get it here because when "End" arrives "sender.numberOfTouches" returns 0
        state = cOnPanGestureArgs::eState::Start;
        central.iosPanGestureStarted=widgetPtr;
    }
    if (sender.state == UIGestureRecognizerStatePossible)
        return;
    if (sender.state == UIGestureRecognizerStateEnded)
        state = cOnPanGestureArgs::eState::End;
    if (sender.state == UIGestureRecognizerStateCancelled)
        return;
    if (sender.state == UIGestureRecognizerStateFailed)
        return;
    if (sender.state == UIGestureRecognizerStateChanged)
        state = cOnPanGestureArgs::eState::Run;
   
    bool shiftKeyPressed=false;
    if (sender.modifierFlags == UIKeyModifierShift)
    {
        shiftKeyPressed=true;
    }
    
    bool ctrlKeyPressed = false;
    if (sender.modifierFlags == UIKeyModifierControl)
    {
        ctrlKeyPressed=true;
    }
    
    

    if (widgetPtr->panNumberOfTouches==1)
        if (widgetPtr->prv_onGesture_Pan!=nullptr)
        {
            cOnPanGestureArgs &args = widgetPtr->currentOnPanGestureArgs;
            args.state = state;
            args.x = pixels::CreateFromDouble(location.x);
            args.y = pixels::CreateFromDouble(location.y);
            args.offsetX = pixels::CreateFromDouble(tiv.x);
            args.offsetY = pixels::CreateFromDouble(tiv.y);
            args.radius = touch.majorRadius;
            args.radiusTolerance = touch.majorRadiusTolerance;
            args.forcePercentage = touch.force / touch.maximumPossibleForce;
            args.shiftKeyPressed = shiftKeyPressed;
            args.ctrlKeyPressed = ctrlKeyPressed;
            widgetPtr->prv_onGesture_Pan(widgetPtr, args);
        }
    
    if (widgetPtr->panNumberOfTouches==2)
        if (widgetPtr->prv_onGesture_PanTwoFingers!=nullptr)
        {
            cOnPanGestureArgs &args = widgetPtr->currentOnPanGestureArgs;
            args.state = state;
            args.x = pixels::CreateFromDouble(location.x);
            args.y = pixels::CreateFromDouble(location.y);
            args.offsetX = pixels::CreateFromDouble(tiv.x);
            args.offsetY = pixels::CreateFromDouble(tiv.y);
            args.radius = touch.majorRadius;
            args.radiusTolerance = touch.majorRadiusTolerance;
            args.forcePercentage = touch.force / touch.maximumPossibleForce;
            args.shiftKeyPressed = shiftKeyPressed;
            args.ctrlKeyPressed = ctrlKeyPressed;
            widgetPtr->prv_onGesture_PanTwoFingers(widgetPtr, args);
        }
    
}

/*  hover event exists for mac catalyst: should be implemented someday (need to add a new OnGesture_Hover())
UIHoverGestureRecognizer *grHover = nullptr;
{
    grHover = [[UIHoverGestureRecognizer alloc] initWithTarget:cvet action : @selector(handleHover : )];
    [grHover setDelegate : cvet];
    [uiView addGestureRecognizer : grHover];
}
-(void)handleHover:(UIHoverGestureRecognizer*)sender {
    if (widgetPtr == nullptr) return; // the widget probably just got deleted
    if (!CheckDefWidgetAlive(widgetPtr)) return;
    
    // not yet finished
    
    if (widgetPtr->xx!=nullptr)
    {
        CGPoint location = [sender locationInView:widgetPtr->uiView];
        
        
        
        double realLocationX, realLocationY;
        ConvertCoordinates(widgetPtr, widgetPtr, (double)location.x, (double)location.y, &realLocationX, &realLocationY);
        
        // todo
    }
}
 */


-(void)handleTap:(UITapGestureRecognizer*)sender {
    if (widgetPtr == nullptr) return; // the widget probably just got deleted
    if (!CheckDefWidgetAlive(widgetPtr)) return;
	
    
    if (widgetPtr->prv_onGesture_Tap!=nullptr)
    {
        CGPoint location = [sender locationInView:widgetPtr->uiView];
        
        double realLocationX, realLocationY;
        ConvertCoordinates(widgetPtr, widgetPtr, (double)location.x, (double)location.y, &realLocationX, &realLocationY);
        
        if (sender.state == UIGestureRecognizerStateCancelled) 
            CUICUI_NOP
        if (sender.state == UIGestureRecognizerStateBegan)
            CUICUI_NOP
        if (sender.state == UIGestureRecognizerStateEnded)
        {
            widgetPtr->prv_onGesture_Tap(widgetPtr, pixels(location.x), pixels(location.y));
        }
    }
}

-(void)handleDoubleTap:(UITapGestureRecognizer*)sender {
    if (widgetPtr == nullptr) return; // the widget probably just got deleted
    if (!CheckDefWidgetAlive(widgetPtr)) return;
    
    if (widgetPtr->prv_onGesture_DoubleTap!=nullptr)
    {
        CGPoint location = [sender locationInView:widgetPtr->uiView];
        
        double realLocationX, realLocationY;
        ConvertCoordinates(widgetPtr, widgetPtr, (double)location.x, (double)location.y, &realLocationX, &realLocationY);
        
        widgetPtr->prv_onGesture_DoubleTap(widgetPtr, pixels(location.x), pixels(location.y));
       }
}
@end


namespace cui {
void wWidget::ios_subclasswrap_touchesMoved(NSSet *touches, UIEvent *event)
{
     UITouch *touch = [[event allTouches] anyObject];
        
    wWidget *widgetPtr = this;

        if (widgetPtr == nullptr) return; // the widget probably just got deleted
        // TODO

        wWidget *touchableWidget = widgetPtr->GetTouchableInMeOrParents();
        if (touchableWidget == nullptr) return;
        if (!CheckDefWidgetAlive(touchableWidget)) return;

        CGPoint location = [touch locationInView:touch.view];

        double x, y;
        ConvertCoordinates(widgetPtr, touchableWidget, (double)location.x, (double)location.y, &x, &y); // note: locationInView above could take widgetPtr->view to do the same thing, no?

        if (touchableWidget->onPointerMoved)
        {
            cOnPointerMovedArgs args;
            args.x = pixels::CreateFromDouble(x);
            args.y = pixels::CreateFromDouble(y);
            args.radius = touch.majorRadius;
            args.radiusTolerance = touch.majorRadiusTolerance;
            args.forcePercentage = touch.force / touch.maximumPossibleForce;
            touchableWidget->onPointerMoved(touchableWidget, args); // pointer entered (useful with wButton in touch mode for example)(I did it on android too)(maybe wButton should check IsTouchDevice() and handle it itself? QUESTION)
        }
        else
        {
            CUICUI_NOP
        }
}

void wWidget::ios_subclasswrap_touchesBegan(NSSet *touches, UIEvent *event)
{
    UITouch *touch = [[event allTouches] anyObject];

        if (this == nullptr) return; // the widget probably just got deleted
        // TODO

        wWidget *touchableWidget = this->GetTouchableInMeOrParents();
        if (touchableWidget == nullptr)
        {
            // this is important, when user click on wEmpty with children; if the top most children is not touchable, it will go to the child underneath (and so on until it reach the bottom most)
            // NO! we don't want to do that
            // [super touchesBegan:touches withEvent:event];
            return;
        }
        if (!CheckDefWidgetAlive(touchableWidget)) return;
        
        touch = [[event allTouches] anyObject];
        CGPoint location = [touch locationInView:touch.view];
        
        double x, y;
        ConvertCoordinates(this, touchableWidget, (double)location.x, (double)location.y, &x, &y); // note: locationInView above could take widgetPtr->view to do the same thing, no?

        if (touchableWidget->onPointerEntered) touchableWidget->onPointerEntered(touchableWidget, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y)); // pointer entered (useful with wButton in touch mode for example)(I did it on android too)(maybe wButton should check IsTouchDevice() and handle it itself? QUESTION)
        
        if (this == nullptr) return;
        if (!CheckDefWidgetAlive(touchableWidget)) return;
        if (touchableWidget->onPointerPressed) touchableWidget->onPointerPressed(touchableWidget, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));

}

void wWidget::ios_subclasswrap_touchesEnded(NSSet *touches, UIEvent *event)
{
    wWidget *widgetPtr = this;
    UITouch *touch = [[event allTouches] anyObject];

        if (widgetPtr == nullptr) return; // the widget probably just got deleted
        // Triggered when touch is released

        wWidget *touchableWidget = widgetPtr->GetTouchableInMeOrParents();
        if (touchableWidget == nullptr)
            return;
        
        if (touchableWidget->coreType == eCoreType::Text)
        {
            wText *w = (wText*)touchableWidget;
            
            UITouch *aTouch = [touches anyObject];
            CGPoint point = [aTouch locationInView : w->uiTextView];
            
            // use TTTAttributedLabel instead of UILabel? it doesn't have that either I think

            // source : https://stackoverflow.com/questions/21349725/character-index-at-touch-point-for-uilabel/26806991
            // it seems ok but should be tested more
            UILabel *textLabel = w->uiTextView;
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
            
            NSUInteger lastClickedCharacterPosition = characterIndex;
            
            if (lastClickedCharacterPosition >= 0)
            {
                wText *m = w;
                
                int i = 0;
                int richItemsCount = (int)m->richItems.size();
                for (cSpan *ri : m->richItems)
                {
                    if (ri->isTouchable)
                        if (lastClickedCharacterPosition >= ri->start && lastClickedCharacterPosition < ri->end)
                        {
                            // UWP special case: if we have a large wText and its text doesn't fill the whole width,
                            // (and the last ri is at the very end)
                            // putting the cursor in the empty space on the right gives the position of the last character (+1)
                            // if we're there, we don't want this to trigger the event
                            // TODO: not perfect, the last char isn't detected as a result
                            // needed on platforms other than uwp? (TODO)
                            //        web: it works without this hack
                            // ios: too? if so remove all this
                            /*if (richItemsCount - 1 == i && lastClickedCharacterPosition + 2 >= ri->end) // +2: because of paragraph end?
                             break;*/
                            
                            if (ri->onMouseClickOnRichTextSpan != nullptr)
                            {
                                ri->onMouseClickOnRichTextSpan(ri);
                                return; // Note: when receiving a 'span touch event', the 'onPointerMoved' of the wText is NOT called
                            }
                        }
                    i++;
                }
            }
        }
        
        touch = [[event allTouches] anyObject];
        CGPoint location = [touch locationInView:touch.view];
        
        double x, y;
        ConvertCoordinates(widgetPtr, touchableWidget, (double)location.x, (double)location.y, &x, &y); // note: locationInView above could take widgetPtr->view to do the same thing, no?

        if (touchableWidget->onPointerReleased)
        {
            touchableWidget->onPointerReleased(touchableWidget, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
        }
        
        if (widgetPtr == nullptr) return;
        if (!CheckDefWidgetAlive(touchableWidget)) return;
        if (touchableWidget->onPointerExited) touchableWidget->onPointerExited(touchableWidget, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y)); // pointer exited (useful with wButton in touch mode for example)(I did it on android too)(maybe wButton should check IsTouchDevice() and handle it itself? QUESTION)
}

} // namespace cui
@implementation CuiUIView{
    wWidget *widgetPtr;
}
-(void)setPtr:(wWidget*)_ptr
{
    widgetPtr = _ptr;
}

- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    UIView *view = [super hitTest:point withEvent:event];
    if (view == self && (widgetPtr->opaqueToTouch==true || widgetPtr->touchableAdded==true))
        return view;
    return view == self ? nil : view;
}


-(void)touchesBegan:(NSSet *)touches withEvent : (UIEvent *)event
{
    if (widgetPtr->deleted) return;
    widgetPtr->ios_subclasswrap_touchesBegan(touches,event);
}
-(void)touchesMoved:(NSSet *)touches withEvent : (UIEvent *)event
{
    if (widgetPtr->deleted) return;
    widgetPtr->ios_subclasswrap_touchesMoved(touches,event);
}
-(void)touchesEnded:(NSSet *)touches withEvent : (UIEvent *)event
{
    if (widgetPtr->deleted) return;
    widgetPtr->ios_subclasswrap_touchesEnded(touches,event);
}
-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    if (widgetPtr->deleted) return;
    // it's important :
    // why? if a user press the mouse, then drag outside of the widget, and release mouse button : this is a "touches cancelled"
    // cuicui handles it as it is a "touch end"
    widgetPtr->ios_subclasswrap_touchesEnded(touches,event);
}
@end
namespace cui {
#endif

wWidget::wWidget()
{
	Reset_setPosHw_prev_values();
	hwPosOffsetX = pixels(0);
	hwPosOffsetY = pixels(0);
	centerChildrenInBadAxis = false;
	rowcolaxis = -1;
	gDebugCountId++;
	debugCountId = gDebugCountId;
	touchableAdded = false;
	parent = nullptr;
	_emRatio = 1.0;
	lockedXem = lockedYem = false;

	if (debugCountId == 35)
		CUICUI_NOP

	bgColorTarget = this;

	definitions_children = nullptr;

	border_use = false;

	onPointerReleased = nullptr;
	onPointerPressed = nullptr;
	onPointerMoved = nullptr;
	onPointerExited = nullptr;
	onPointerEntered = nullptr;



#if defined(CUICUI_UWP)	
	hd = nullptr;
	b = nullptr;			
#endif
#if defined(CUICUI_IOS)
    uiView=nullptr;
#endif
#if defined(CUICUI_ANDROID)        
	//fatal: must go in Native() + change all the java code so it doesn't look for "elem.firstchild()"
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiElement_Create", "(I)I");
	jint _v = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, -1);
	theId = (int)_v;
	JNICall_Void_IntLong("cuiSetWidgetPtr", theId, (long)this); // reminder: c++ "long" is 32 bits on 32 bits OS, and 64 bits on 64 bits (so it will work on both os/cpu)
#endif
#if defined(CUICUI_WEB)
	htmlId = -1;
#endif


}

wWidget* wWidget::LinkToHardwareBackButton()
{	
	// this is only valid if Clickable() was previoulsy called for this widget
	// you can't use this with a Touchable() widget
	cui_assert(touchableFeedback0 != nullptr || touchableFeedback1 != nullptr); 
																							

#ifdef CUICUI_DEBUG

#endif

	auto f = std::find(central.hardwareBackButtonLinked.begin(), central.hardwareBackButtonLinked.end(), this);
	if (f != central.hardwareBackButtonLinked.end())
	{
		// LinkToHardwareBackButton() already called for this widget
		return this;
	}

	central.hardwareBackButtonLinked.push_back(this);

	return this;
}

void wWidget::TriggerClickable()
{
	if (touchableFeedback0 != nullptr)  touchableFeedback0->onClick(this);
	if (touchableFeedback1 != nullptr)  touchableFeedback1->onClick(this);
}

wWidget* wWidget::Clickable(int type, std::function<void(wWidget *self)> onClick)
{
	if (touchableAdded) return this;
	EnsureNative();	

	if (type > 1)
	{
		cui_assert(false);
		return this;
	}

	cui_assert(opaqueToTouch == false);

	if (type == 0)
	{
		touchableFeedback0 = std::make_unique<cTouchableFeedback0>();

		touchableFeedback0->onClick = onClick;
		this->Native();
		this->Touchable();
		this->onPointerReleased = [this](wWidget* self, pixels x, pixels y)
		{
			if (touchableFeedback0->onClick)
			{
				bool pointerIsInside = false;
				if (x >= pixels::CreateZero() && y >= pixels::CreateZero() && x.asDouble() < this->ax[X].size && y.asDouble() < this->ax[Y].size)
					pointerIsInside = true;
				if (pointerIsInside)
					touchableFeedback0->onClick(this);
			}
		};
	}
	if (type == 1)
	{
		touchableFeedback1 = std::make_unique<cTouchableFeedback1>();

		touchableFeedback1->onClick = onClick;
		touchableFeedback1->cbg = this;		
		touchableFeedback1->pressedColor =  0xcccccc;
		touchableFeedback1->hoverColor =  0x888888;

		this->Touchable();

		this->onPointerPressed = [this](wWidget* self, pixels x, pixels y)
		{
			//touchableFeedback1->pointerIsInside = true; // useful for android (since exited/entered don't really exist); if we click on the button we're necessarily "inside" anyway
			// TODO: problem! the "prevcolor" handling is done in entered (and we can't do both a priori
			//check pointerIsInside and run the entered code or not?
			touchableFeedback1->cbg->BgColor(touchableFeedback1->pressedColor);
		};
		this->onPointerReleased = [this](wWidget* self, pixels x, pixels y)
		{
			bool pointerIsInside = false;

			// Not sure this is really useful? maybe on touch devices. Anyway it can't hurt
			if (x >= pixels::CreateZero() && y >= pixels::CreateZero() && x.asDouble() < this->ax[X].size && y.asDouble() < this->ax[Y].size)
				pointerIsInside = true;

			if (pointerIsInside)
			{
				touchableFeedback1->cbg->BgColor(touchableFeedback1->hoverColor);
				if (touchableFeedback1->onClick) touchableFeedback1->onClick(this);
			}
			else
			{
				touchableFeedback1->cbg->BgColor(touchableFeedback1->previousBgColor);
			}
		};
		this->onPointerExited = [this](wWidget* self, pixels x, pixels y)
		{
			touchableFeedback1->cbg->BgColor(touchableFeedback1->previousBgColor);
		};
		this->onPointerEntered = [this](wWidget* self, pixels x, pixels y)
		{
			touchableFeedback1->previousBgColor = touchableFeedback1->cbg->GetBgColor();
			touchableFeedback1->cbg->BgColor(touchableFeedback1->hoverColor);
		};
	}
	return this;
}

wWidget* wWidget::SetOpaqueToTouch()
{
	cui_assert(touchableAdded == false); // to avoid mistakes and hard-to-find issues, this is not allowed

	EnsureNative();

    opaqueToTouch=true;
    
#if defined(CUICUI_ANDROID)
	JNICall_Void_Int("cuiElement_SetOpaqueToTouch", this->theId); 
#elif defined(CUICUI_UWP)	
	Touchable(); // this will catch all touches (and no nothing); Note: didn't find a real native way to do it
#elif defined(CUICUI_WEB)
		
	EM_ASM_({	
		var elem = gu($0);
		if (elem == null) return;		
		//elem.style["pointer-events"] = "auto"; // we'd need "always" but it doesn't exist, auto isn't good (almost ok EXCEPT if the view has a transparent bg I think)

		

		// didn't find a better way
		elem.style["touch-action"] = "auto";
		elem.style["pointer-events"] = "auto";
		elem.onpointerenter = function(e) { e.stopPropagation(); };
		elem.onpointerleave = function(e) { e.stopPropagation(); };
		elem.onpointermove = function(e) { e.stopPropagation(); };
		elem.onpointerdown = function(e) { e.stopPropagation(); };
		elem.onpointerout = function(e) { e.stopPropagation(); };
		elem.onpointerhover = function(e) { e.stopPropagation(); };
		elem.onpointerup = function(e) { e.stopPropagation(); };
		elem.ondblclick = function(e) { e.stopPropagation(); };

		elem.cuicuiSetOpaqueToTouch = "true"; // debug info
	}, htmlId);		
#elif defined(CUICUI_IOS)
	CUICUI_NOP // opaqueToTouch=true is enough
    
    // actually it's not enough, we need to do that for gestures
    // isn't there a better solution?
    OnGesture_Tap([](wWidget *w, pixels x, pixels y)
    {
        // we catch the tap so it won't go through
    });
    OnGesture_Pan([](wWidget *w, cOnPanGestureArgs args)
    {
        // we catch the tap so it won't go through
    });
    OnGesture_Pinch([](wWidget *w, cOnPinchGestureArgs args)
    {
        // we catch the tap so it won't go through
    });
    OnGesture_LongPress([](wWidget *w, pixels x, pixels y)
    {
        // we catch the tap so it won't go through
    });
    
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif
	return this;
}

// rename to EmRatio()?
wWidget* wWidget::ChangeEmRatio(double _value) // DOES NOT TRIGGER refreshUI
{		
	
	_emRatio = _value;

	/*std::list<wWidget*> list;
	GetAllWidgetRecursively(list, this); // TODO/PERF: we could have a list of scrollviews/doms in "master" (and in each dom root)
	for (auto w : list)
	{
		//if (w->type == "verticalscrollview")
		if (w->coreType == eCoreType::ScrollView)
		{
			//Log("debug_7878");
			wScrollView *sv = (wScrollView*)w;
			sv->ForceRefreshContentNextTime();
		}
	}*/


	//if (parent == nullptr) return this; // if we do this, it doesn't work when used in definitions_children

	return this;
}

bool wWidget::IsReadyToSetPosHW()
{
	if (ax[X].pos.asDouble() == -666) return false;
	if (ax[Y].pos.asDouble() == -666) return false;
	if (ax[X].size == -666) return false;
	if (ax[Y].size == -666) return false;
	return true;
}


void wWidget::OnGesture_LongPress(std::function<void(wWidget*, pixels x, pixels y)> longPress)
{
	cui_assert(Touchable());
	cui_assert(IsNative());
	cui_assert(prv_onGesture_LongPress==nullptr); // can't call it twice

	prv_onGesture_LongPress = longPress;

#if defined(CUICUI_ANDROID)
	JNICall_Void_IntString("cuicuiEnableGesture", this->theId, "longPress");
#elif defined(CUICUI_UWP)
	if (recognizer == nullptr)
	{
		recognizer = ref new Windows::UI::Input::GestureRecognizer();
	}

	recognizer->GestureSettings = recognizer->GestureSettings | Windows::UI::Input::GestureSettings::Hold;	

	recognizer->Holding += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::HoldingEventArgs^>([this](Windows::UI::Input::GestureRecognizer^ gr, Windows::UI::Input::HoldingEventArgs^ args)
		{		
			if (!CheckDefWidgetAlive(this)) return;
			this->prv_onGesture_LongPress(this, pixels(args->Position.X), pixels(args->Position.Y));			
		});
#elif defined(CUICUI_WEB)
	// warning: longpress is not (yet) implemented on web.
	// IMPORTANT: we do NOT set "touch-action: none" here. Long-press provides no
	// functionality on web (never triggered), but "touch-action: none" disabled the
	// browser's native scroll as soon as a drag started on the element. Since an app's
	// chat lines set an OnGesture_LongPress and cover the whole scrollview,
	// this broke scrolling of the chat area. (The side menu uses Clickable -> touch-action:auto -> OK.)
	CUICUI_NOP;
#elif defined(CUICUI_IOS)
	//missing
#elif defined(CUICUI_MACOS)
	missing
#else
	non_implemented_platform
#endif
}

wWidget* wWidget::OnGesture_Tap(std::function<void(wWidget*, pixels x, pixels y)> simpleTap,
                       std::function<void(wWidget*, pixels x, pixels y)> doubleTap,
						bool simpleTapConfirmed)
{		
	cui_assert(Touchable());
	cui_assert(IsNative());
	cui_assert(prv_onGesture_Tap == nullptr && prv_onGesture_DoubleTap==nullptr); // can't call it twice

    prv_onGesture_Tap = simpleTap;
    prv_onGesture_DoubleTap = doubleTap;
        
#ifdef CUICUI_IOS
    
   
    
    UITapGestureRecognizer *grSingleTap = nullptr;
    if (simpleTap)
    {
        grSingleTap = [[UITapGestureRecognizer alloc] initWithTarget:cvet action : @selector(handleTap : )];
        [grSingleTap setDelegate : cvet];
        grSingleTap.numberOfTapsRequired=1;
        grSingleTap.numberOfTouchesRequired=1;
        grSingleTap.cancelsTouchesInView=NO;
        [uiView addGestureRecognizer : grSingleTap];
    }
        
    UITapGestureRecognizer *grDoubleTap = nullptr;
    if (doubleTap)
    {
        grDoubleTap = [[UITapGestureRecognizer alloc] initWithTarget:cvet action : @selector(handleDoubleTap : )];
        [grDoubleTap setDelegate : cvet];
        grDoubleTap.numberOfTapsRequired=2;
        grDoubleTap.numberOfTouchesRequired=1;
        grDoubleTap.cancelsTouchesInView=NO;
        [uiView addGestureRecognizer : grDoubleTap];
    }
// TODO:	there's simpleTapConfirmed now, do the same as android and uwp
    if (simpleTap && doubleTap)
    {
        // important: without that the single and double tap won't work together
        [grSingleTap requireGestureRecognizerToFail:grDoubleTap];
    }
#elif CUICUI_UWP
	if (recognizer == nullptr)
	{
		recognizer = ref new Windows::UI::Input::GestureRecognizer();
	}

	if (simpleTap != nullptr)
		recognizer->GestureSettings = recognizer->GestureSettings | Windows::UI::Input::GestureSettings::Tap;
	if (doubleTap != nullptr)
		recognizer->GestureSettings = recognizer->GestureSettings | Windows::UI::Input::GestureSettings::DoubleTap;
	
	// note: RightTapped useful? 
	// there is Hold and HoldWithMouse, useful?

	recognizer->Tapped += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::TappedEventArgs^>([this, simpleTapConfirmed](Windows::UI::Input::GestureRecognizer^ gr, Windows::UI::Input::TappedEventArgs^ args)
	{			
		if (args->TapCount == 1 && this->prv_onGesture_Tap != nullptr)
		{			
			if (!CheckDefWidgetAlive(this)) return;

			if (simpleTapConfirmed)
			{
				uwp_doubleTapOccurred = false;
				double waitTimeInSeconds = 0.3; // todo: a way to get the real value from uwp?
				pixels px(args->Position.X);
				pixels py(args->Position.Y);
				timerManager.Create(waitTimeInSeconds, [this, px ,py](Timer* timer) {
					if (uwp_doubleTapOccurred == false)
					{
						this->prv_onGesture_Tap(this, px, py);						
					}
					uwp_doubleTapOccurred = false;
				});
			}
			else
			{
				this->prv_onGesture_Tap(this, pixels(args->Position.X), pixels(args->Position.Y));
			}
		}
		if (args->TapCount == 2 && this->prv_onGesture_DoubleTap != nullptr)
		{
			if (!CheckDefWidgetAlive(this)) return; 
			uwp_doubleTapOccurred = true;
			this->prv_onGesture_DoubleTap(this, pixels(args->Position.X), pixels(args->Position.Y));
		}
	});
#elif CUICUI_ANDROID
	if (simpleTap && !simpleTapConfirmed)
        JNICall_Void_IntString("cuicuiEnableGesture", this->theId, "singleTap");
	if (simpleTap && simpleTapConfirmed)
		JNICall_Void_IntString("cuicuiEnableGesture", this->theId, "singleTapConfirmed");
    if (doubleTap)
        JNICall_Void_IntString("cuicuiEnableGesture", this->theId, "doubleTap");
#elif CUICUI_WEB
	// IMPORTANT: no "touch-action: none" for a Tap. A tap is discrete (pointerdown
	// then pointerup at the same place) and doesn't conflict with native scroll: if
	// the user does a small drag, the browser scrolls and emits pointercancel -> the tap
	// doesn't trigger, which is the intended behavior. Setting "none" was useless
	// and created dead zones for scrolling (e.g. tappable thumbnails/lines in a scrollview).
	CUICUI_NOP;
#elif
	missing
#endif
	return this;
}



void wWidget::OnGesture_Swipe(std::function<void(wWidget*, cOnSwipeGestureArgs)> swipe)
{
    cui_assert(Touchable());
    cui_assert(IsNative());
    cui_assert(prv_onGesture_Swipe == nullptr); // can't call it twice
    
    prv_onGesture_Swipe = swipe;
#ifdef CUICUI_IOS
     UISwipeGestureRecognizer *g = [[UISwipeGestureRecognizer alloc] initWithTarget:cvet action : @selector(handleSwipe : )];
     g.cancelsTouchesInView=NO;
    g.numberOfTouchesRequired=1;
    g.direction =   UISwipeGestureRecognizerDirectionRight |
                    UISwipeGestureRecognizerDirectionLeft |
                    UISwipeGestureRecognizerDirectionUp |
                    UISwipeGestureRecognizerDirectionDown;
     [g setDelegate : cvet];
     [uiView addGestureRecognizer : g];
#else
    // not done on this platform
#endif
}

void wWidget::OnGesture_Pinch(std::function<void(wWidget*, cOnPinchGestureArgs)> f)
{
	cui_assert(Touchable());
	cui_assert(IsNative());
	cui_assert(prv_onGesture_Pinch == nullptr); // can't call it twice

	prv_onGesture_Pinch = f;

#ifdef CUICUI_WEB
  // Really?
    EM_ASM_({
            var elem = gu($0);
            elem.style["touch-action"] = "none";
    }, htmlId);
#elif CUICUI_IOS
   UIPinchGestureRecognizer *pinch = [[UIPinchGestureRecognizer alloc] initWithTarget:cvet action : @selector(handlePinch : )];
    pinch.cancelsTouchesInView=YES;
    [pinch setDelegate : cvet];
    [uiView addGestureRecognizer : pinch];
#elif CUICUI_UWP
	//Windows::UI::Input::GestureSettings::ManipulationScale ?
	// NYI
#elif CUICUI_ANDROID
    JNICall_Void_IntString("cuicuiEnableGesture", this->theId, "pinch");
#elif CUICUI_WEB
	EM_ASM_({
			var elem = gu($0);
			elem.style["touch-action"] = "none";
		}, htmlId);
#elif 
	missing
#endif 
}


void wWidget::OnGesture_Pan(std::function<void(wWidget*, cOnPanGestureArgs)> panOneFinger,
                              std::function<void(wWidget*, cOnPanGestureArgs)> panTwoFingers)
{
	cui_assert(Touchable());
	cui_assert(IsNative());
	cui_assert(prv_onGesture_Pan == nullptr && prv_onGesture_PanTwoFingers==nullptr); // can't call it twice

	prv_onGesture_Pan = panOneFinger;
	prv_onGesture_PanTwoFingers = panTwoFingers;

#ifdef CUICUI_WEB
        EM_ASM_({
            var elem = gu($0);
            elem.style["touch-action"] = "none";
    }, htmlId);
#elif CUICUI_IOS
    CustomPanGestureRecognizer *pan = [[CustomPanGestureRecognizer alloc] initWithTarget:cvet action : nil];
    pan.maximumNumberOfTouches = 2;
    pan.allowedScrollTypesMask = UIScrollTypeMaskAll; // UIScrollTypeMaskAll = UIScrollTypeMaskDiscrete /*mouse for mac catalyst support*/ | UIScrollTypeContinuous /*trackpad and touch screen*/;
    pan.cancelsTouchesInView=YES;
    [pan setDelegate : cvet];
    [uiView addGestureRecognizer : pan];
#elif CUICUI_UWP
	if (recognizer == nullptr)
	{
		recognizer = ref new Windows::UI::Input::GestureRecognizer();		
	}

	if (panOneFinger != nullptr)
		recognizer->GestureSettings = recognizer->GestureSettings | Windows::UI::Input::GestureSettings::Drag;
	if (panTwoFingers != nullptr)
	{
		// TODO: NOT HANDLED on UWP
		//recognizer->GestureSettings = recognizer->GestureSettings | (Windows::UI::Input::GestureSettings::ManipulationTranslateX | Windows::UI::Input::GestureSettings::ManipulationTranslateY);
	}

	recognizer->Dragging += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::DraggingEventArgs^>([this](Windows::UI::Input::GestureRecognizer^ gr, Windows::UI::Input::DraggingEventArgs^ args)
	{
		if (!CheckDefWidgetAlive(this)) return;
		if (this->prv_onGesture_Pan != nullptr)
		{			
			using namespace Windows::UI::Input;

			cOnPanGestureArgs pga;
			if (args->DraggingState == DraggingState::Started)
			{
				pga.state = cOnPanGestureArgs::eState::Start;
				uwpDraggingStart = args->Position;
			}
			if (args->DraggingState == DraggingState::Completed)	pga.state = cOnPanGestureArgs::eState::End;
			if (args->DraggingState == DraggingState::Continuing)	pga.state = cOnPanGestureArgs::eState::Run;
			pga.x = pixels(args->Position.X);
			pga.y = pixels(args->Position.Y);
			// offset since the beginning of the gesture (same semantics as the other platforms)
			pga.offsetX = pixels(args->Position.X - uwpDraggingStart.X);
			pga.offsetY = pixels(args->Position.Y - uwpDraggingStart.Y);
			this->prv_onGesture_Pan(this, pga);
		}
	});
#elif CUICUI_ANDROID
	if (panOneFinger)
        JNICall_Void_IntString("cuicuiEnableGesture", this->theId, "panOneFinger");
    if (panTwoFingers)
        JNICall_Void_IntString("cuicuiEnableGesture", this->theId, "panTwoFingers");
#elif
	missing
#endif
}




} // namespace cui