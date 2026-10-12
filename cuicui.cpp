
#include "platform.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include "cuicui.h"


namespace cui {
wEmpty *master = NULL; // it's the "root" wWidget that is placed on the native root View 

std::function<void()> onAppSizeChanged = nullptr;

#if  defined(__EMSCRIPTEN__)
int htmlIdCount = 0;
#endif



std::list<wWidget*> touchables;
#ifdef CUICUI_DEBUG
std::list<int> debugDeletedWidgets;
#endif

AnimationManager animationManager;

cCppMessageBoxManager cppMessageBoxManager;

std::function<void()> onFileReceivedSoon = nullptr;
std::function<void(std::shared_ptr<cFileReceived> pr)> onFileReceived = nullptr;
#ifdef CUICUI_WEB
std::function<void()> webOnPwaReadyToBeInstalled = nullptr;
#endif


 
int gDebugCountId = 0;

std::thread::id cuiUiThreadId;


#ifdef CUICUI_ANDROID
JniCall_manager jniCallManager;
#endif

/* deprecated
 - (BOOL)textView:(UITextView *)textView shouldInteractWithURL:(NSURL *)URL inRange:(NSRange)characterRange
 {
 wMultiLineText *m = (wMultiLineText*)widgetPtr;
 //indows::UI::Xaml::Documents::TextPointer ^tp = m->tb->GetPositionFromPoint(pp->Position);
 int lastClickedCharacterPosition = characterRange.location; // rename this variable
 // WARNING: m->lastClickedCharacterPosition is the native position, which is NOT the one of the real characters
 
 int i = 0;
 int richItemsCount = m->richItems.size();
 for (wMultiLineText::cSpan *ri : m->richItems)
 {
 if (ri->isTouchable)
 if (lastClickedCharacterPosition >= ri->start && lastClickedCharacterPosition < ri->end)
 {
 // UWP special case: if we have a big wMultiLineText and its text doesn't fill the whole width,
 // (and the last ri is at the very end)
 // putting the cursor in the empty space on the right gives the position of the last character (+1)
 // if we are here, we don't want this to trigger the event
 // TODO: not perfect, the last char is not detected as a result
 // needed on platforms other than uwp? (TODO)
 //        web: it works without this hack
 if (richItemsCount - 1 == i && lastClickedCharacterPosition + 2 >= ri->end) // +2: because of end of paragraph?
 break;
 
 if (ri->onMouseClickOnRichTextSpan != nullptr)
 {
 ri->onMouseClickOnRichTextSpan(ri);
 return NO; //  Note: when receiving a 'span touch event', the 'onPointerClick' of the wMultiLineText is NOT called
 }
 }
 i++;
 }
 
 // hack: we send "pointer released" to the multilinetext (because it doesn't receive it anymore since we had to set userinteractionenabled for the touchspans to work...)
 // not ok, we don't receive this event, it only works for "url" spans
 //if (m->onPointerReleased!=nullptr)
 //   m->onPointerReleased(m, -1, -1);
 
 return NO;
 }*/

#if defined(CUICUI_IOS)
} // namespace cui
@implementation CuiTransitionEventClass {
    //Transition *ti; // TODO: nullptr
    float startTimestamp;
}
- (void) timerFired:(CADisplayLink *)sender {
    animationManager.Run();
}
@end
namespace cui {
#endif



bool CheckDefWidgetAlive(wWidget *w)
{
	if (w == nullptr) return false;
#ifdef CUICUI_DEBUG
	if (Debug_IsDeleted(w))
	{
		Log("CRITICAL: CheckDefWidgetAlive: Debug_IsDeleted is true! : " + w->debugName + " / " + std::to_string(w->debugCountId));
		return false;
	}
#endif

	if (w->deleted)
	{
		return false;
	}

	return true;
}

#if defined(CUICUI_WEB)


void WebOnTransitionEvent(int transitionPtr, int currentTimeInMilliseconds)
{
	assert(false); // deprecated
	AssertOnUiThread();

	animationManager.Run();

}



// TODO: we could do without this function by putting the code directly in test.cpp in the "fileinput_event" function
void WebOnFileInputSelection(std::string fileName, char *base64Data /*NOT null terminated!!*/, int base64DataSize, std::string mimeType)  // WARNING: do NOT delete/free "base64Data", it will be done by javascript side
{
	Log("WebOnFileInputSelection : " + std::string(fileName) + "; base64Data size = " + std::to_string(base64DataSize) + " bytes.");
	if (onFileReceived != nullptr)
	{
		Log("calling onFileReceived");

		auto pr = std::make_shared<cFileReceived>();
		pr->base64Data.assign(base64Data, base64DataSize);
		pr->fileName = fileName;
		pr->mimeType = mimeType;

		onFileReceived(pr);
	}
	else
		Log("No onFileReceived, doing nothing");
}
#endif

void GetAllWidgetRecursively(std::list<wWidget*> &list, wWidget *w)
{
	list.push_back(w);
	for (wWidget *c : w->children) // 
		GetAllWidgetRecursively(list, c);
}

Children& Children::operator+=(wWidget* w) {

	list.push_back(w);

	assert(w->deleted == false); // you are trying to add a deleted widget !
	assert(CheckDefWidgetAlive(w)); // you are trying to add a deleted widget !

	return *this;
}

void CreateUI_Recur(wWidget *w)
{
	AssertOnUiThread();

	if (w->definitions_children != nullptr)
	{
		Children dc;
		dc.self = w;
		w->definitions_children(dc);

		for (unsigned int i = 0; i < dc.list.size(); i++)
		{
			w->AddChild_private(dc.list.at(i));
		}

	}
}

} // namespace cui
extern void InitMyApp(); // must be defined in user code

namespace cui {
bool firstAppSizeChanged = true;
#ifdef CUICUI_UWP
void OnWindowSizeChanged(int width, int height, Canvas ^rootGrid)
#elif CUICUI_IOS
void OnWindowSizeChanged(int width, int height, UIView *parentView)
#else
void OnWindowSizeChanged(int width, int height)
#endif
{
	Log("OnWindowSizeChanged : width/height: " + std::to_string(width) + " / " + std::to_string(height));
	if (firstAppSizeChanged == true)
	{
		// note: it's importnt it's there, because InitMyApp() may use these values
		central.currentScreenWidth.setFromDouble(width);
		central.currentScreenHeight.setFromDouble(height);

		firstAppSizeChanged = false;
		InitMyApp();

#ifdef CUICUI_ANDROID
		// adds master in the "rootAndroidView"
		JNICall_Int_IntInt("cuiElement_AddToParentView",
			-1000, //indicates that it must go in the androidRootView
			master->theId);
#elif CUICUI_UWP
		// InitMyApp() created a"master" that we add here in the root element of the uwp window					
		rootGrid->Children->Append(master->b);
#elif CUICUI_WEB
		Log("WEB: Will add 'master' to cuiRootDiv...");
		EM_ASM_({
			   Module.print("Adding 'master' to cuiRootDiv");
			   Module.print("'master' has this htmlId : " + $0);
			   var cuiRootDiv = document.getElementById('cuiRootDiv');

			   var childNum = 0; // master is always 0
			   var child = getUnattachedElementsFromId(childNum.toString());
			   if (child == null)
			   {
				   Module.print("--> master child not found : " + childNum.toString());
				   return;
			   }
			   cuiRootDiv.appendChild(child);


		}, master->htmlId
		);
#elif defined(CUICUI_IOS)
        UIView *subViewConstrainedToSafeArea = parentView;
        [subViewConstrainedToSafeArea addSubview:master->uiView];
#else
		missing
#endif

		assert(master->ax[X].realEm.IsFill());
		assert(master->ax[Y].realEm.IsFill());

		master->ax[X].pos.setFromDouble(0);
		master->ax[Y].pos.setFromDouble(0);
		master->ax[X].SetPixels(width);
		master->ax[Y].SetPixels(height);
		RefreshUI_Everything();

		return;
	}

/*#ifdef CUICUI_WEB
	// TODO: useful? can't we do "right=0 & bottom=0" once at startup ?
	EM_ASM_(
		{
			//alert("resize:" + $0 + "," + $1);

			var cuiRootDiv = document.getElementById('cuiRootDiv');
			//cuiRootDiv.setAttribute("style","position:absolute; top:0px; left:0px; width:" + $0 + "px; height:" + $1 + "px;background-color:black");
			cuiRootDiv.style["left"] = "0px";
			cuiRootDiv.style["top"] = "0px";
			cuiRootDiv.style["width"] = $0.toString() + "px";
			cuiRootDiv.style["height"] = $1.toString() + "px";
		},
		width, height
	);
#endif*/

#if defined(CUICUI_ANDROID)     	
	bool a = central.IsScreenPortrait();
	bool b = central.isCurrentlyScreenInPortrait;
	central.isCurrentlyScreenInPortrait = a;
	if (a!=b)
	{
		Log("Info223: the phone changed orientation."); 				
		central.currentScreenWidth.setFromDouble(width);
		central.currentScreenHeight.setFromDouble(height);
		if (onAppSizeChanged != nullptr) onAppSizeChanged();
		master->ax[X].pos.setFromDouble(0);
		master->ax[Y].pos.setFromDouble(0);
		master->ax[X].SetPixels(width);
		master->ax[Y].SetPixels(height);
		RefreshUI_Everything();
		return;
	}
#endif

#if defined(CUICUI_ANDROID)|| defined(CUICUI_IOS) // TODO: "isDeviceWithSoftwareKeyboard" ?
	if (central.GetVirtualKeyboardScreenMode() == cCentral::VirtualKeyboardScreenMode::Pan)
	{		
		double diff = central.currentScreenHeight.asDouble() - height;
		if (diff < 0)
			master->hwPosOffsetY = pixels::CreateFromDouble(0);
		else
			master->hwPosOffsetY = pixels::CreateFromDouble(-diff);

		master->SetPos_HW2();
		return;

	}
	if (central.GetVirtualKeyboardScreenMode() == cCentral::VirtualKeyboardScreenMode::Resize)
	{
		central.currentScreenWidth.setFromDouble(width);
		central.currentScreenHeight.setFromDouble(height);
		if (onAppSizeChanged != nullptr) onAppSizeChanged();
		master->ax[X].pos.setFromDouble(0);
		master->ax[Y].pos.setFromDouble(0);
        master->ax[X].SetPixels(width);
        master->ax[Y].SetPixels(height);
		RefreshUI_Everything();
		return;
	}
#else
	central.currentScreenWidth.setFromDouble(width);
	central.currentScreenHeight.setFromDouble(height);
	if (onAppSizeChanged != nullptr) onAppSizeChanged();
	master->ax[X].pos.setFromDouble(0);
	master->ax[Y].pos.setFromDouble(0);
	master->ax[X].SetPixels(width);
	master->ax[Y].SetPixels(height);
	RefreshUI_Everything();
#endif
}




#ifdef CUICUI_WEB
extern "C" {
void EMSCRIPTEN_KEEPALIVE webOnMouseWheel(unsigned int ptr, int deltaY, int mouseX, int mouseY)
{
	wWidget *w = (wWidget*)ptr;
	if (!CheckDefWidgetAlive(w)) return;

	if (w->onMouseWheel != nullptr) w->onMouseWheel(deltaY, mouseX, mouseY, cuicui_KeyModifier_None);
}
}
#endif


#ifdef CUICUI_UWP
void UwpCommandLineArguments(Platform::String ^args)
{
	if (args != "") // TODO P5: could be nice to trim before testing empty
	{
		auto pr = std::make_shared<cFileReceived>();
		pr->base64Data = UwpStringToStdString(args);
		pr->mimeType = "text/plain";
		pr->fileName = "";
		central.initialPayload = pr;
	}
}
#endif


void wGradient::ApplyGradient()
{
#if defined(CUICUI_ANDROID)
	// TODO/PROBLEM P1: we can't give "positions" of colors on android? we allow it on other platforms
	std::string colors;
	for (auto s : stops)
	{
		colors += s.color.ToHtmlColorCode();
	}
	JNICall_Void_IntString("wGradient_Apply", theId, colors);

#elif defined(CUICUI_UWP)
    LinearGradientBrush ^linear = ref new LinearGradientBrush();
    linear->StartPoint = Point(0, 0);
    linear->EndPoint = Point(0, 1);
    
    for each(auto s in stops)
    {
        auto stop = ref new GradientStop();
        stop->Color = Windows::UI::ColorHelper::FromArgb(s.color.ca, s.color.cr, s.color.cg, s.color.cb);
        stop->Offset = s.offset;
        linear->GradientStops->Append(stop);
    }

    this->canvas->Background = linear;

#elif defined(CUICUI_WEB)
	LogWarning("wGradient not supported on WEB yet !"); // TODO WEB P1
#elif defined(CUICUI_IOS)
  
    
    DebugName("debug283");
    
    if (gradient==nullptr) {
        gradient = [CAGradientLayer layer];
        [uiView.layer insertSublayer:gradient atIndex:0];
    }
    gradient.startPoint = CGPointMake(0,0);
    gradient.endPoint = CGPointMake(0,1); 
    gradient.frame = uiView.bounds; // TODO: cs_hue has no gradient because when we create it its bounds are not ok
   // must put this update in setposhw or somewhere, not sure where
    
    NSMutableArray *colors = [NSMutableArray array];
    NSMutableArray *locations = [NSMutableArray array];
    for (size_t i=0; i<stops.size(); i++)
    {
        wGradient::cStop s = stops.at(i);
        UIColor *c = [UIColor colorWithRed : (float)s.color.cr/255.0f green : (float)s.color.cg/255.0f blue: (float)s.color.cb/255.0f alpha: 1];
        [colors addObject:(id)c.CGColor];
        [locations addObject:[NSNumber numberWithFloat:(float)s.offset]];
    }
    gradient.colors = [colors copy];
    gradient.locations = [locations copy];
   
#elif defined(CUICUI_MACOS)
    missing
#else
    missing
#endif
}


bool TryComputedSize(AXIS axis, wWidget *p)
{

	if (p->GetEm(axis).IsMin())
	{
		double w, h;
		if (p->ComputeMinSize(&w, &h) == true)
		{
			//w *= GetEmSize(1); // TODO !! remove this, and remove the inverse in ComputeMinSize();
			//h *= GetEmSize(1); // TODO !! remove this, and remove the inverse in ComputeMinSize();
			if (axis == X && w != -666)
			{
				p->SetPixels(axis, w);
				return true;
			}
			else
				if (axis == Y && h != -666)
				{
					p->SetPixels(axis, (int)h);
					return true;
				}
				else
					assert(false);
		}
		// it's not a ComputeMinSize() capable		
	}
	else
		assert(false); // MIN only

	return false;
}


//double easeOutQuint(double t); // TODO: remove

void Transition::RunFrame(std::chrono::steady_clock::time_point currentGlobalTime)
{
	if (done)
	{
		return;
	}

	auto spentTimeInMilliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentGlobalTime - this->startedAtThisGlobalTime);
	auto ms = spentTimeInMilliSeconds.count();
	auto s = ((double)ms) / 1000.0;

	double normalizedTime = s / duration;



	double v = valueStart + ((valueEnd - valueStart) * easingFunc(normalizedTime));

	if (normalizedTime > 1.0) //currentTimeSeconds >= duration) // time exceeded?
	{
		if (loop)
		{
			auto endTime = startedAtThisGlobalTime + std::chrono::duration<int, std::milli>((int)(duration*1000.0));
			auto dt = currentGlobalTime - endTime;
			this->startedAtThisGlobalTime = currentGlobalTime - dt;
			RunFrame(currentGlobalTime);

			return;
		}
		else
		{
			done = true;
			// we don't care, to remove currentTimeSeconds = duration;
			v = valueEnd;
		}
	}
	AnimationStep step;
	step.self = widget;
	step.isLast = done;
	step.value = v;
	step.deltaValue = v - prevValue;
	callback(step);

	prevValue = v;

}


bool DoHaveMinParent(AXIS axis, wWidget *w)
{
	wWidget *p = w;
	do
	{
		p = p->parent;
		if (p == nullptr) return false;
		if (p->GetEm(axis).IsMin()) return true;
	} while (1);

	assert(false);
}

std::string EmToString(Length em)
{
	return "NYI6845";
}

void ComputeNativeWidgetMinSize(wWidget *dw, double *w, double *h, bool subView=true)
{
	// note: emRatio() doesn't apply on natives widgets

#if defined(CUICUI_ANDROID)
	//cuiComputeNativeWidgetSize
	int retVal = JNICall_Int_Int("cuiComputeNativeWidgetSize", dw->theId);
	*w = (double)(retVal & 0xffff) ;
	*h = (double)(retVal >> 16) ;
#elif defined(CUICUI_UWP)
	Size s2(10000000, 10000000);
	dw->b->Measure(s2);
	*w = ((dw->b->DesiredSize.Width));
	*h = ((dw->b->DesiredSize.Height));
#elif defined(CUICUI_WEB)	
	unsigned int retVal = EM_ASM_INT(
		{
			var num = $0;
			var elem = gu(num.toString());
			if (elem == null) return;
			var widthInPixels = elem.offsetWidth;
			var heightInPixels = elem.offsetHeight;
			return ((heightInPixels << 16) + widthInPixels);
	}, dw->htmlId);
	*w = (double)(retVal & 0xffff) ;
	*h = (double)(retVal >> 16) ;	
#elif defined(CUICUI_IOS)

    CGSize size;
	
	if (subView)
		size = [dw->uiView.subviews[0] sizeThatFits:CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX)];
	else
		size = [dw->uiView sizeThatFits:CGSizeMake(CGFLOAT_MAX, CGFLOAT_MAX)];
    *w = size.width ;
    *h = size.height ;
#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif
}

void DumpTree_recur(AXIS axis, wWidget *w, int deepness)
{
	std::string s;

	if (axis == 0) s += "X___";
	if (axis == 1) s += "Y___";

	for (int i = 0; i < deepness; i++)
		s += "---";

	std::string strtype;
	if ( w->rowcolaxis == -1)		strtype = "EMPTY  ";
	if ( w->rowcolaxis == Y)	strtype = "ROWS   ";
	if ( w->rowcolaxis == X)	strtype = "COLUMNS";
	if (w->coreType == eCoreType::Image) strtype = "wImage";
	s += strtype;

	s += " / [";
#ifdef CUICUI_DEBUG
	s += w->ax[axis].realEm.ToString();
#endif

	s += "]  /  " + w->debugName;

	assert(w->parent != nullptr); 

	s += " / parent: ";
	s += std::to_string(w->HasParentInNativeDom()->debugCountId);

	if (w->IsNative()) s += " / NATIVE "; else s += " / virtual ";
	

	s += "   (" + std::to_string(w->debugCountId) + ")";

	if (w->coreType == eCoreType::Text)
	{
		wText *t = (wText*)w;
	}

	Log(s);

	for (auto c : w->children)
		DumpTree_recur(axis, c, deepness + 1);
}

void DumpTree(AXIS axis, wWidget *w)
{	
#ifndef CUICUI_DEBUG
	return;
#endif
	int d = 0;
	Log("==============");
	DumpTree_recur(axis, w, d);
}

void ConvertToPixelsSize(int axis, std::list<wWidget*> &all)
{
	return; // deprecated
}

#ifdef CUICUI_ANDROID



JavaVM* globalCuiJavaVm = nullptr;

// env of the current thread via the JavaVM. Unlike a cached JNIEnv*, it's valid
// whatever the thread (the JavaVM itself is shared). Returns nullptr if the thread
// is not attached to the VM (we don't attach here so as not to leak an attachment).
JNIEnv* CurrentThreadJniEnv()
{
	if (globalCuiJavaVm == nullptr)
		return nullptr;
	JNIEnv* env = nullptr;
	if (globalCuiJavaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
		return nullptr;
	return env;
}

bool RegisterCuicuiNatives(JNIEnv *env); // android/native-lib.cpp

} // namespace cui

// outside the namespace: it must keep its C name to be called by the JVM
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	// IMPORTANT : do NOT cache the JNIEnv* in globalCuiJniEnv here.
	// JNI_OnLoad runs on the thread that loads the lib ; but when the process is
	// cold-started by an FCM notification, it's NativeCaller (static loadLibrary) that
	// loads the lib on a BACKGROUND THREAD. A JNIEnv* is bound to its thread : reusing it
	// later from the UI thread (cuiMain -> FindClass) = UB -> SIGSEGV (Crashlytics crash).
	// So we only keep the JavaVM (valid cross-thread) ; globalCuiJniEnv is set
	// in cuiMain, on the UI thread.
	cui::globalCuiJavaVm = vm;

	// the env is only used here, on the loading thread
	JNIEnv* env = nullptr;
	if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) return JNI_ERR;
	if (!cui::RegisterCuicuiNatives(env)) return JNI_ERR;

	return JNI_VERSION_1_6;
}
namespace cui {
#endif

void LogWrite(LogLevel level, const std::string& str)
{
#if defined(CUICUI_ANDROID)
	int prio = ANDROID_LOG_DEBUG;
	if (level == LogLevel::Info) prio = ANDROID_LOG_INFO;
	if (level == LogLevel::Warning) prio = ANDROID_LOG_WARN;
	if (level == LogLevel::Error) prio = ANDROID_LOG_ERROR;
	__android_log_print(prio, "cuicui", "%s", str.c_str());
#else
	const char* prefix = "";
	if (level == LogLevel::Warning) prefix = "[cuicui] WARNING: ";
	else if (level == LogLevel::Error) prefix = "[cuicui] ERROR: ";
	std::string line = prefix + str;
#if defined(CUICUI_IOS)
	NSLog(@"%@", StdStringToNSString(line));
#elif defined(CUICUI_UWP)
	OutputDebugStringA(line.c_str());
	if (line.size() > 0 && line[line.size() - 1] != '\n')
		OutputDebugStringA("\n");
#elif defined(CUICUI_WEB)
	EM_ASM_({
		var str = UTF8ToString($1);
		if ($0 == 2) console.warn(str);
		else if ($0 == 3) console.error(str);
		else console.log(str);
		}, (int)level, line.c_str());
#endif
#endif
}
void LogDebug(const std::string& str)
{
#ifdef CUICUI_DEBUG
	LogWrite(LogLevel::Debug, str);
#endif
}
void LogInfo(const std::string& str) { LogWrite(LogLevel::Info, str); }
void LogWarning(const std::string& str) { LogWrite(LogLevel::Warning, str); }
void LogError(const std::string& str) { LogWrite(LogLevel::Error, str); }
void Log(const std::string& str) { LogDebug(str); }
void LogD(const std::string& str) { LogDebug(str); }
void LogR(const std::string& str) { LogInfo(str); }


// TODO P5: make Log::d() / Log::r() / Log() would be like Log::d() ?

void RefreshUI_Special_Scrollview(wScrollView *sv)
{
	
	// the content of a wScrollView is a kind of "master", we handle it here:
	// TODO/PERF: don't recompute if Y changes, only if X changes (and if Xem(fill) ?)

	wWidget *content = sv->scrollViewContentWidget;
	if (content == nullptr) return;
	content->ax[X].pos = pixels(0);
	content->ax[Y].pos = pixels(0);
	if (sv->orientation == wScrollView::Orientation::Vertical)
	{
		assert(sv->GetEm(X).IsFill()==false);
		assert(sv->GetEm(X).IsSameAsX()==false);
		if (sv->GetEm(X).IsMin())
			content->SetEm(X, min());
		else
		{
			assert(sv->GetEm(X).IsPixels());
			content->SetPixels(X, sv->GetPixels(X) - sv->scrollBarWidthInPixels);
		}

		content->ax[Y].DontUseMePrivateSetEm(MIN);

	}
	else
	{
		content->ax[X].DontUseMePrivateSetEm(MIN);

		assert(sv->GetEm(Y).IsFill()==false);
		assert(sv->GetEm(Y).IsSameAsX()==false);
		if (sv->GetEm(Y).IsMin())
			content->SetEm(Y, min());
		else
		{
			assert(sv->GetEm(Y).IsPixels());
			content->SetPixels(Y, (int)(sv->GetPixels(Y) - sv->scrollBarWidthInPixels));
		}
	}

	RefreshUI_Private(content);

}


class cCI // calc info
{
public:
	double biggest;
	double fixedSizeOnAxis;
	double availableSpaceForFillers;
	std::vector<wWidget*> fillers;
	std::vector<wWidget*> fixed;

	cCI(int axis, wWidget *p)
	{

		fixedSizeOnAxis = 0;
		biggest = -666;
		availableSpaceForFillers = -666;
		for (auto c : p->children)
		{
			if (c->GetEm((AXIS)axis).IsMin())
				assert(false); // must never happen mins.push_back(c);
			else
				if (c->GetEm((AXIS)axis).IsFill())
					fillers.push_back(c);
				else
					if (c->GetEm((AXIS)axis).IsPixels())
					{
						fixed.push_back(c);
						fixedSizeOnAxis += c->GetPixels((AXIS)axis);
						if (biggest < c->GetPixels((AXIS)axis)) biggest = c->GetPixels((AXIS)axis);
					}
					else
						assert(0); // not normal I presume
		}

		if (p->IsFixed((AXIS)axis))
		{
			if (fillers.size() == 0)
				availableSpaceForFillers = 0;
			else
			{
				availableSpaceForFillers = p->GetPixels((AXIS)axis) - fixedSizeOnAxis;
			}
		}

		if (axis != p->rowcolaxis) fixedSizeOnAxis = -666; // fixedSizeOnAxis makes no sense if not good axis
	}
};

static bool RefreshUI_Private_inProgress = false;

std::vector<wWidget*> RefreshUI_Private_sces; // widgets that are listening on "onSizeChanged" 

extern int gDebugSetPosHwDone;
extern int gDebugComputeMinOnWText;
extern int gDebugComputeMinOnWText_cached;
extern int gDebugUpdateNativeFontsIfNeeded;

void RefreshUI_Private(wWidget *defw) // TODO : remove "_recur"
{	
	if (!defw->GetEm(X).IsPixels()  && !defw->GetEm(Y).IsPixels()) // "&&" because when we do it on a scrollviewcontent, one of the axes is MIN
		return; // app is not ready	

#ifdef CUICUI_DEBUG
	// TODO_5857 P5: stats NOT ok because they are globals, and with each scrollview they get overwritten
	/*gDebugSetPosHwDone = 0;
	gDebugComputeMinOnWText = 0;
	gDebugComputeMinOnWText_cached = 0;
	gDebugUpdateNativeFontsIfNeeded = 0;*/
#endif

	auto startTime = std::chrono::system_clock::now();

	std::list<wWidget*> all;
	GetAllWidgetRecursively(all, defw);

	int modified = 0;
	for (wWidget *w : all)
	{


#ifdef CUICUI_DEBUG
		if (1)
			if (w->coreType == eCoreType::ScrollView)
			{
				wScrollView *sv = (wScrollView*)w;
				if (sv->orientation == wScrollView::Orientation::Vertical)
				{
					assert(sv->ax[Y].realEm.IsMin()==false); // a vertical scrollview cannot be MIN on axis Y
				}
				else
				{
					assert(sv->ax[X].realEm.IsMin()==false); // a horizontal scrollview cannot be MIN on axis X
				}
			}
#endif

		

		if (w != defw)
		{
			assert(w->ax[X].realEm.IsUnset()==false);
			assert(w->ax[Y].realEm.IsUnset() == false);

			if (w != defw)
			{
				w->ax[X].ResetTempValues();
				w->ax[Y].ResetTempValues();
			}

			{
				w->ax[X].pos.setFromDouble(0);
				w->ax[Y].pos.setFromDouble(0);
			}

			w->SetEm(X, w->ax[X].realEm);
			if (w->GetEm(X).IsEms()) 
				w->SetPixels(X, w->GetEm(X).v * GetEmSize(w->emRatio()));
			if (w->GetEm(X).IsRems()) 
				w->SetPixels(X, w->GetEm(X).v * central.defaultEmSize.asDouble());

			w->SetEm(Y, w->ax[Y].realEm);
			if (w->GetEm(Y).IsEms()) 
				w->SetPixels(Y, w->GetEm(Y).v * GetEmSize(w->emRatio()));
			if (w->GetEm(Y).IsRems())
				w->SetPixels(Y, w->GetEm(Y).v * central.defaultEmSize.asDouble());
		}
	}
	CUICUI_NOP



		for (int __axis = 0; __axis < 2; __axis++)
		{
			AXIS axis = (AXIS)__axis;

			// stuff we can do without taking the parent into account
			for (auto p : all)
			{
				if (p->GetEm(axis).IsMin())
					if (TryComputedSize((AXIS)axis, p) == true) continue;

				// don't do this (I think scrollview doesn't like it)(and it's useless a priori)(very slight optim maybe?)
				/*if (p->coreType == eCoreType::RowColEmpty && p->ax[axis].em == MIN && p->children.size() == 0)
					p->ax[axis].em = 0;*/
			}

			

#define IS_EMPTY(_v) (_v->coretype==TYPE::EMPTY)
#define IS_RC(_v) (_v->coretype==TYPE::ROWSCOLS)
#define HAS_CHILDREN(_v) (_v->children.size()>0)
#define IS_FIXED(_v) (_v->ax[axis].em>=0)
#define IS_FILL(_v) (_v->ax[axis].em==FILL)
#define IS_MIN(_v) (_v->ax[axis].em==MIN)
#define GOOD_AXIS(_v) (_v->rowcolaxis == axis)
#define BAD_AXIS(_v) (_v->rowcolaxis != axis)
#define IS_READY(_v) (_v->ax[axis].em!=FILL && _v->ax[axis].em!=MIN)
#define ITER_CHILDREN(_v, _n) for (auto _n : _v->children)



#define LAYOUTER_V7 // done
#ifdef LAYOUTER_V7


// useless macro now (CLEANUP)


		// step 1: we compute the size of all the mins
			{
				std::list<wWidget*> mins;
				for (auto p : all)
				{
					if (p->IsMin(axis)) mins.push_back(p);
				}
				// WARNING: must go from the lowest to the highest in the tree

				mins.reverse();

				for (auto min : mins)
				{
					/*if (min->coreType == eCoreType::ScrollView)
					{
						// SCROLL VIEW with MIN
						wScrollView *sv = (wScrollView*)min;
						assert(sv->ax[X].realEm == MIN);
						assert(false); // using MIN on scrollview is NYI
						if (sv->orientation == wScrollView::Orientation::Vertical)
						{
							RefreshUI_Special_Scrollview(sv); // it can't compute its MIN, I don't know why
							CUICUI_NOP
						}
						else
						{
							// TODO
							assert(false);
						}
					}*/

					std::list<wWidget*> content;
					GetAllWidgetRecursively(content, min);

					content.reverse();

					for (auto c : content)
					{
						{
							if (c->IsFixed(axis))
							{
								c->ax[axis].maxSizeInMin = c->ax[axis].size;
								if (c->ax[axis].maxSizeInMin == -666.0)
								{
									assert(false);

								}
							}
						}
					}

					for (auto c : content)
					{
						if (c->children.size() > 0)
						{
							if (c->rowcolaxis == axis && c->rowcolaxis != -1)
							{
								double total = 0;
								for (auto child : c->children)
								{
									total += child->ax[axis].maxSizeInMin;
								}
								c->ZeroMaxSizeInMinInChildren(axis);
								if (c->IsFixed(axis) == false)
									c->ax[axis].maxSizeInMin = total;
							}
							if (c->rowcolaxis != axis || c->rowcolaxis == -1)
							{
								double biggestMaxSizeInMin = 0;
								for (auto child : c->children)
								{
									if (child->ax[axis].maxSizeInMin > biggestMaxSizeInMin) biggestMaxSizeInMin = child->ax[axis].maxSizeInMin;
								}

								c->ZeroMaxSizeInMinInChildren(axis);
								if (c->IsFixed(axis) == false)
									c->ax[axis].maxSizeInMin = biggestMaxSizeInMin;

							}
						}
						CUICUI_NOP
					}

					content.reverse();

					double size = 0;
					for (auto c : content)
					{

						assert(c->ax[axis].maxSizeInMin != -666);
						size += c->ax[axis].maxSizeInMin;
					}

					// does this depend on rowcolaxis of min? or of the parent of min?
					if (min->rowcolaxis != axis || min->rowcolaxis == -1)
						min->SetPixels(axis, min->ax[axis].maxSizeInMin);
					else
						min->SetPixels(axis, size);


					CUICUI_NOP
				}
			}

			// step 1.5: SAME_AS_X handling
			{
				if (axis == Y)
				for (auto p : all)
				{					
					if (p->ax[Y].realEm.IsSameAsX())
					{
						//p->SetEm(Y, p->GetEm(X));						 // I don't use this one because it says I rewrite the em, it's weird (CLEANUP?)
						assert(p->GetEm(X).IsPixels());
						p->SetPixels(Y, p->GetPixels(X));
					}
				}
			}
			// step 2: we apply the simple algo
			{
				for (auto p : all)
				{


					{
						assert(p->IsMin(axis) == false);
						cCI ci(axis, p);

						if (p->rowcolaxis == axis && p->rowcolaxis != -1)
						{
							if (ci.fillers.size() > 0)
							{								
								assert(ci.availableSpaceForFillers != -666);
								double fillerSize = ci.availableSpaceForFillers / ci.fillers.size();
								bool notRound = (fmod(ci.availableSpaceForFillers, ci.fillers.size()) != 0);
								int idx = 0;
								for (auto f : ci.fillers)
								{									
									if (idx == ci.fillers.size() - 1)
										if (notRound)
											fillerSize++;
									f->SetPixels(axis, fillerSize);
									idx++;
								}
							}
						}
						else
						{
							for (auto f : ci.fillers)
								f->SetPixels(axis, p->GetPixels(axis));
						}
					}

					

					if (p->coreType==eCoreType::ScrollView && axis==X && ((wScrollView*)p)->orientation == wScrollView::Orientation::Vertical)
					{
						wScrollView *sv = (wScrollView*)p;
						
#ifndef OPTIM_211_DISABLE
						if (sv->prevs.setPosHw_prev_widthInEm_optim_5675 == p->GetEm(X))
						{
							// optim_5675: when a vertical scrollview doesn't change width, we don't need to recompute it (nor its children) 
							// (even if its height changes)								
#ifdef CUICUI_DEBUG
#endif
						}
						else
#endif
						{
							if (sv->ax[X].realEm.IsMin()==false) 
								RefreshUI_Special_Scrollview(sv);
						}

					}

					if (p->coreType == eCoreType::ScrollView && axis == Y && ((wScrollView*)p)->orientation == wScrollView::Orientation::Horizontal)
					{
						wScrollView *sv = (wScrollView*)p;
						
#ifndef OPTIM_211_DISABLE
						if (sv->prevs.setPosHw_prev_heightInEm_optim_5675 == p->GetEm(Y))
						{
							// optim_5675: when a horizontal scrollview doesn't change height, we don't need to recompute it (nor its children) 
							// (even if its width changes)								
#ifdef CUICUI_DEBUG
#endif
						}
						else
#endif
						{
							if (sv->ax[Y].realEm.IsMin() == false)
								RefreshUI_Special_Scrollview(sv);
						}						
					}

				}
			}

			// check all is ok
			for (auto p : all)
			{
				if (p->IsFixed((AXIS)axis) == false)
				{
					DumpTree(axis, all.front());
					assert(false);
					break;
				}
			}

#endif


		}

	CUICUI_NOP

	for (int axis = 0; axis < 2; axis++)
		for (wWidget *p : all)
		{

			if (p->debugCountId == 1142)
				CUICUI_NOP

			// row/col in the main axis? (only case where we need to position, otherwise it's always 0, which was set for all controls earlier)
			if (p->rowcolaxis == axis)
			{
/*#ifdef USE_PIXEL_PERFECT_SIZES_AND_POSITIONS
				// TODO_5644 -> assert(p->ax[axis].size == std::round(p->ax[axis].size)); // the parent must have been "rounded" (note: master always is)
				// master is not rounded on web? it's possible since the browser offers a zoom, so master.width is not necessarily rounded? to check
#endif*/

				double posInPixels = 0;
				bool containsAtLeastOneFiller = false;
				for (wWidget *c : p->children)
				{
					c->ax[axis].pos.setFromDouble(posInPixels);

					if (c->ax[axis].realEm.IsFill())
						containsAtLeastOneFiller = true;

					assert(c->GetEm((AXIS)axis).IsPixels());
					double cSize = c->GetPixels(axis);

					posInPixels += cSize;

/*#ifdef USE_PIXEL_PERFECT_SIZES_AND_POSITIONS
					// TODO_5645: how are we sure it lands exactly at the far right (if the last child goes to the end, either via fixed or a fill)
#endif*/
				}

#ifdef CUICUI_DEBUG
				if (containsAtLeastOneFiller)
                {
					if(posInPixels != p->ax[axis].size)
                        LogWarning("precision problem? seen in an app on ios"); // TODO
                }
#endif
			}

			if (p->alignRight && axis == X)
			{				
				if (p->parent->rowcolaxis == -1 || p->parent->rowcolaxis == Y) {
					double pw = p->parent->ax[X].size;
					double tw = p->ax[X].size;
					double pos = pw - tw;
					p->ax[X].pos.setFromDouble(pos);
				}
			}

			if (p->alignBottom && axis == Y)
			{
				if (p->parent->rowcolaxis == -1 || p->parent->rowcolaxis == X) {
					double ph = p->parent->ax[Y].size;
					double th = p->ax[Y].size;
					double pos = ph - th;
					p->ax[Y].pos.setFromDouble(pos);
				} else
                {
					CUICUI_NOP // assert? does this mean the user probably made a mistake ? OR replace with "alignToEnd" (this way it's ok on both axes) ?? TODO
                }
			}

			if (p->centerHorizontallyInParent && axis==X)
			{
				if (p->parent->rowcolaxis == -1 || p->parent->rowcolaxis == Y)
				if (p->ax[X].size < p->parent->ax[X].size) // TODO: we should be able to center even if it's bigger than the parent
				{
					double pos = p->parent->ax[X].size;
					pos -= p->ax[X].size;
					pos /= 2;
					p->ax[X].pos.setFromDouble(pos);
				}
			}
			if (p->centerVerticallyInParent && axis == Y)
			{
				if (p->parent->rowcolaxis == -1 || p->parent->rowcolaxis == X)
				if (p->ax[Y].size < p->parent->ax[Y].size) // TODO: we should be able to center even if it's bigger than the parent
				{
					double pos = p->parent->ax[Y].size;
					pos -= p->ax[Y].size;
					pos /= 2;
					p->ax[Y].pos.setFromDouble(pos);
				}
			}

			if (p->centerChildrenInBadAxis)
			if (p->rowcolaxis != axis)
			{
				for (wWidget *c : p->children)
				{
					c->ax[axis].pos.setFromDouble((p->ax[axis].size - c->ax[axis].size) / 2.0);
				}
			}
		}


	auto endTime = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = endTime - startTime;


	auto startTime2 = std::chrono::system_clock::now();


	for (wWidget *w : all)
	{		
#ifdef CUICUI_DEBUG
		if (w->IsReadyToSetPosHW() == false)
			assert(false);
#endif

		w->SetPos_HW2();
	}

	auto endTime2 = std::chrono::system_clock::now();
	std::chrono::duration<double> diff2 = endTime2 - startTime2;

	

#ifdef CUICUI_DEBUG
	// see TODO_5857
	/*
	Log("gDebugSetPosHwDone : " + std::to_string(gDebugSetPosHwDone));
	Log("gDebugComputeMinOnWText : " + std::to_string(gDebugComputeMinOnWText-gDebugComputeMinOnWText_cached) + "/" + std::to_string(gDebugComputeMinOnWText));
	Log("gDebugUpdateNativeFontsIfNeeded : " + std::to_string(gDebugUpdateNativeFontsIfNeeded));*/
#endif
	CUICUI_NOP
}


void RefreshUI_Fast(wWidget *defw) // WARNING: use it ONLY when you are 100% sure that the size of "defw" WILL NOT change! Or the layout will be incorrect
{
	AssertOnUiThread();

	// on WEB platform if the widget is not in the dom : RefreshUI_Private will do incorrect computations
	// we MUST try to assert() this but I don't know how to : 
	// not ok : assert(defw->IsThisWidgetOneOfMyParent(master) || defw->IsInsideAScrollView()); 

#ifdef CUICUI_DEBUG
	// TODO: on WEB if defw is not in the dom the computations and the layout will be incorrect (but it's ok on uwp for example) -> add an assert ! (TODO P1)
	//         
#endif
	if (RefreshUI_Private_inProgress) return;
	RefreshUI_Private_inProgress = true;
	RefreshUI_Private_sces.clear();
	RefreshUI_Private(defw);
	RefreshUI_Private_inProgress = false;
	for (auto sce : RefreshUI_Private_sces)
	{
		sce->onSizeChanged(sce, pixels::CreateFromDouble(sce->ax[X].size), pixels::CreateFromDouble(sce->ax[Y].size));
	}
	RefreshUI_Private_sces.clear();
	

}

#ifdef CUICUI_DEBUG
void GetAllWidgetRecursively_IncludingInScrollViewsContent(std::list<wWidget*> &list, wWidget *w)
{
	if (w->coreType == eCoreType::ScrollView)
	{
		wScrollView *sv = (wScrollView*)w;
		GetAllWidgetRecursively_IncludingInScrollViewsContent(list, sv->scrollViewContentWidget);
	}
	list.push_back(w);
	for (wWidget *c : w->children) // 
	{
		GetAllWidgetRecursively_IncludingInScrollViewsContent(list, c);
	}
}
#endif


void RefreshUI_Everything()
{
	Log("RefreshUI_Everything");
	AssertOnUiThread();

	RefreshUI_Fast(master);


#ifdef CUICUI_DEBUG
#endif
}


#if defined(CUICUI_ANDROID)

extern "C"
void
JNICALL
cuijni_cuiSetCentralSafeAreaRect(
	JNIEnv * env,
	jobject obj/* this */,
	jint l, jint t, jint r, jint b)
{
	// note_safearea_relayout : the insets (system bars + virtual keyboard) can change WITHOUT
	// the window changing size. This is notably the case in edge-to-edge / Android 15+ where
	// SOFT_INPUT_ADJUST_RESIZE no longer resizes the window when the keyboard opens : we then
	// receive the new bottom inset (= keyboard height) here, but "cuiAppSizeChanged" is never
	// called again, so the relayout (and thus applying the wSafeZone gaps) was not triggered
	// -> the input bar stayed under the keyboard. So we trigger the relayout ourselves here.
	cRect &sa = central.safeAreaRect;
	if (sa.left==l && sa.top==t && sa.right==r && sa.bottom==b)
		return; // no change : do nothing (avoids relayouting at each frame of the keyboard animation)

	sa.Set(l,t,r,b);

	// master->onSizeChanged is set by wSafeZone : it re-applies the 4 gaps from
	// central.safeAreaRect then relayouts. The size of master doesn't change here, so no recursion
	// (see the "prevs.IsTheSame()" guard in wWidget::SetPos_HW).
	if (master != nullptr && master->onSizeChanged != nullptr)
		master->onSizeChanged(master,
			pixels::CreateFromDouble(master->ax[X].size),
			pixels::CreateFromDouble(master->ax[Y].size));

	return;
}


extern "C"
jint
JNICALL
cuijni_cuiIsNativeWithTouch(
	JNIEnv *env,
	jobject obj/* this */,
	jlong widgetPtr)
{
	wWidget *touchedWidget = (wWidget*)widgetPtr;
	if (touchedWidget == nullptr) return 0; // false
	if (!CheckDefWidgetAlive(touchedWidget)) return 0; // false

	return 0; // false
}

extern "C"
void
JNICALL
cuijni_wImageIsAnimated(
		JNIEnv *env,
		jobject obj/* this */,
		jlong widgetPtr)
{
	wWidget *widget = (wWidget*)widgetPtr;
	if (widget == nullptr) return;
	if (!CheckDefWidgetAlive(widget)) return;
	if (widget->coreType != eCoreType::Image) return;

	wImage *img = (wImage*)widget;
	img->isAnimatedImage=true;
}

extern "C"
void
JNICALL
cuijni_cuicuiShowSimpleMessageBoxYesNo(
	JNIEnv * env,
	jobject obj/* this */,
	int answer)
{	
	if (answer == 1)
	{
		if (central.androidMessageBoxOkCancel_onOk!=nullptr)
			central.androidMessageBoxOkCancel_onOk();
	}
	else
	{
		if (central.androidMessageBoxOkCancel_onCancel != nullptr)
			central.androidMessageBoxOkCancel_onCancel();
	}

	central.androidMessageBoxOkCancel_onOk = central.androidMessageBoxOkCancel_onCancel = nullptr;
}


extern "C"
void
JNICALL
cuijni_cuiTouchEvent(
	JNIEnv *env,
	jobject obj/* this */,
	jlong widgetPtr,
	int eventType,
	float _x,
	float _y,
	float c,
	float d)
{
	wWidget *touchedWidget = (wWidget*)widgetPtr;
	if (touchedWidget == nullptr) return; // this widget must have been destroyed at the same time the touch event happened...        
	if (!CheckDefWidgetAlive(touchedWidget)) return;

	wWidget *parentTouchable = touchedWidget->GetTouchableInMeOrParents();
	if (parentTouchable == nullptr)
	{
		return; // no touchable contains this widget
	}

	if (!CheckDefWidgetAlive(parentTouchable)) return;


	double x, y;
	ConvertCoordinates(touchedWidget, parentTouchable, (double)_x, (double)_y, &x, &y);

	double xEm = (double)x;
	double yEm = (double)y;
	pixels _xEm = pixels::CreateFromDouble(xEm); 
	pixels _yEm = pixels::CreateFromDouble(yEm);
	// TODO: rename _xEm, _yEm which are not ems but pixels

	if (eventType == 1 && parentTouchable->onPointerEntered) parentTouchable->onPointerEntered(parentTouchable, _xEm, _yEm);
	if (eventType == 2 && parentTouchable->onPointerExited) parentTouchable->onPointerExited(parentTouchable, _xEm, _yEm);
	if (eventType == 3 && parentTouchable->onPointerPressed) parentTouchable->onPointerPressed(parentTouchable, _xEm, _yEm);
	if (eventType == 4 && parentTouchable->onPointerReleased) parentTouchable->onPointerReleased(parentTouchable, _xEm, _yEm);
	if (eventType == 5 && parentTouchable->onPointerMoved) {
		cOnPointerMovedArgs args;
		args.x = _xEm;
		args.y = _yEm;
		parentTouchable->onPointerMoved(parentTouchable, args);
	}




	return;
}





extern "C"
void
JNICALL
cuijni_cuiGestureDoubleTapEvent(
		JNIEnv *env,
		jobject obj,
		jlong widgetPtr,
		float _x,
		float _y)
{
	wWidget *touchedWidget = (wWidget*)widgetPtr;
	if (touchedWidget == nullptr) return;
	if (!CheckDefWidgetAlive(touchedWidget)) return;
	wWidget *parentTouchable = touchedWidget->GetTouchableInMeOrParents();
	if (parentTouchable == nullptr) return;
	if (!CheckDefWidgetAlive(parentTouchable)) return;

	// we need to convert x,y
	double x, y;
	ConvertCoordinates(touchedWidget, parentTouchable, (double)_x, (double)_y, &x, &y);

	if (parentTouchable->prv_onGesture_DoubleTap != nullptr) parentTouchable->prv_onGesture_DoubleTap(parentTouchable, pixels(x), pixels(y));

	return;
}


extern "C"
void
JNICALL
cuijni_cuiGestureSingleTapEvent(
	JNIEnv * env,
	jobject obj,
	jlong widgetPtr,
	float _x,
	float _y)
{
	wWidget* touchedWidget = (wWidget*)widgetPtr;
	if (touchedWidget == nullptr) return;
	if (!CheckDefWidgetAlive(touchedWidget)) return;
	wWidget* parentTouchable = touchedWidget->GetTouchableInMeOrParents();
	if (parentTouchable == nullptr) return;
	if (!CheckDefWidgetAlive(parentTouchable)) return;

	// we need to convert x,y
	double x, y;
	ConvertCoordinates(touchedWidget, parentTouchable, (double)_x, (double)_y, &x, &y);

	if (parentTouchable->prv_onGesture_Tap != nullptr) parentTouchable->prv_onGesture_Tap(parentTouchable, pixels(x), pixels(y));

	return;
}


extern "C"
void
JNICALL
cuijni_cuiGestureLongPressEvent(
	JNIEnv * env,
	jobject obj,
	jlong widgetPtr,
	float _x,
	float _y)
{
	wWidget* touchedWidget = (wWidget*)widgetPtr;
	if (touchedWidget == nullptr) return;
	if (!CheckDefWidgetAlive(touchedWidget)) return;
	wWidget* parentTouchable = touchedWidget->GetTouchableInMeOrParents();
	if (parentTouchable == nullptr) return;
	if (!CheckDefWidgetAlive(parentTouchable)) return;

	// we need to convert x,y
	double x, y;
	ConvertCoordinates(touchedWidget, parentTouchable, (double)_x, (double)_y, &x, &y);

	if (parentTouchable->prv_onGesture_LongPress != nullptr) parentTouchable->prv_onGesture_LongPress(parentTouchable, pixels(x), pixels(y));

	return;
}

extern "C"
void
JNICALL
cuijni_cuiGesturePanEvent(
	JNIEnv *env,
	jobject obj,
	jlong widgetPtr,
	float _x,
	float _y,
	float offsetX,
	float offsetY,
	int phase)
{
	wWidget *touchedWidget = (wWidget*)widgetPtr;
	if (touchedWidget == nullptr) return;
	if (!CheckDefWidgetAlive(touchedWidget)) return;	
	wWidget *parentTouchable = touchedWidget->GetTouchableInMeOrParents();
	if (parentTouchable == nullptr) return; 	
	if (!CheckDefWidgetAlive(parentTouchable)) return;

	// we need to convert x,y 
	double x, y;
	ConvertCoordinates(touchedWidget, parentTouchable, (double)_x, (double)_y, &x, &y);

	if (parentTouchable->prv_onGesture_Pan)
	{
		cOnPanGestureArgs args;
		args.x = pixels::CreateFromDouble(x);
		args.y = pixels::CreateFromDouble(y);
		args.offsetX = pixels::CreateFromDouble(offsetX);
		args.offsetY = pixels::CreateFromDouble(offsetY);
		args.state = (phase == 0) ? cOnPanGestureArgs::eState::Start
		           : (phase == 2) ? cOnPanGestureArgs::eState::End
		           : cOnPanGestureArgs::eState::Run;
		parentTouchable->prv_onGesture_Pan(parentTouchable, args);
	}
	
	return;
}


extern "C"
void
JNICALL
cuijni_cuiGesture2FingersPanEvent(
		JNIEnv *env,
		jobject obj,
		jlong widgetPtr,
		float _x,
		float _y,
		float offsetX,
		float offsetY,
		int phase)
{
	wWidget *touchedWidget = (wWidget*)widgetPtr;
	if (touchedWidget == nullptr) return;
	if (!CheckDefWidgetAlive(touchedWidget)) return;
	wWidget *parentTouchable = touchedWidget->GetTouchableInMeOrParents();
	if (parentTouchable == nullptr) return;
	if (!CheckDefWidgetAlive(parentTouchable)) return;

	// we need to convert x,y
	double x, y;
	ConvertCoordinates(touchedWidget, parentTouchable, (double)_x, (double)_y, &x, &y);

	if (parentTouchable->prv_onGesture_PanTwoFingers)
	{
		cOnPanGestureArgs args;
		args.x = pixels::CreateFromDouble(x);
		args.y = pixels::CreateFromDouble(y);
		args.offsetX = pixels::CreateFromDouble(offsetX);
		args.offsetY = pixels::CreateFromDouble(offsetY);
		args.state = (phase == 0) ? cOnPanGestureArgs::eState::Start
		           : (phase == 2) ? cOnPanGestureArgs::eState::End
		           : cOnPanGestureArgs::eState::Run;
		parentTouchable->prv_onGesture_PanTwoFingers(parentTouchable, args);
	}

	return;
}

extern "C"
void
JNICALL
cuijni_cuiGestureScaleEvent(
	JNIEnv *env,
	jobject obj,
	jlong widgetPtr,
	float scale,
	float _x,
	float _y,
	int phase
	)
{
	wWidget *touchedWidget = (wWidget*)widgetPtr;
	if (touchedWidget == nullptr) return;
	if (!CheckDefWidgetAlive(touchedWidget)) return;
	wWidget *parentTouchable = touchedWidget->GetTouchableInMeOrParents();
	if (parentTouchable == nullptr) return;
	if (!CheckDefWidgetAlive(parentTouchable)) return;

	// we need to convert x,y 
	double x, y;
	ConvertCoordinates(touchedWidget, parentTouchable, (double)_x, (double)_y, &x, &y);

	if (parentTouchable->prv_onGesture_Pinch != nullptr)
	{
		cOnPinchGestureArgs args;
		args.scale = scale;
		args.state = (phase == 0) ? cOnPinchGestureArgs::eState::Start
		           : (phase == 2) ? cOnPinchGestureArgs::eState::End
		           : cOnPinchGestureArgs::eState::Run;
		args.centerX.setFromDouble(x);
		args.centerY.setFromDouble(y);
		parentTouchable->prv_onGesture_Pinch(parentTouchable, args);
	}
	return;
}


extern "C"
void
JNICALL
cuijni_cuicuiOnAppStateChanged(
	JNIEnv * env,
	jobject obj,	
	int state
)
{	
	if (central.onAppStateChanged != nullptr)
	{
		if (state == 0)
			central.onAppStateChanged(cCentral::eAppState::Foreground);
		else
			if (state == 1)
				central.onAppStateChanged(cCentral::eAppState::Background);
			else
				assert(false); // error 
	}

	return;
}

void AndroidOnFileInputSelection(std::string fileName, std::string base64Data, int dataSize, std::string mimeType, int imageOrientation, std::string option)
{

	Log("AndroidOnFileInputSelection received : " + fileName + " / dataSize = " + std::to_string(dataSize));
	{
		Log("calling onFileReceived");

		auto pr = std::make_shared<cFileReceived>();
		pr->base64Data = base64Data;


		pr->fileName = fileName;
		pr->option = option;
		pr->mimeType = mimeType;
		pr->imageOrientation = imageOrientation;

		if (central.Inited() == false)
		{
			central.initialPayload = pr;
		}
		else
		{
			if (onFileReceived != nullptr)
			{
				AssertOnUiThread();
				onFileReceived(pr);
			}
			else
				Log("No onFileReceived, doing nothing");
		}
	}

}
#endif



Transition* AnimationManager::create(wWidget *w, double _valueStart, double _valueEnd, double _duration, std::function<void(AnimationStep step)> _callback, bool _loop, easing_functions easing)
{
	if (transitions.size() == 0)
		StartEventLoop();

	auto pA = new Transition();

	pA->startedAtThisGlobalTime = std::chrono::steady_clock::now();

	pA->easingFunc = getEasingFunction(easing);

	pA->loop = _loop;
	pA->widget = w;
	pA->valueStart = _valueStart;
	pA->valueEnd = _valueEnd;
	pA->duration = _duration;
	pA->callback = _callback;
	pA->prevValue = _valueStart;

	pA->guid = (int)transitions.size();
	transitions.push_back(pA);



	return pA;
}

void AnimationManager::StartEventLoop()
{
	assert(transitions.size() == 0);

#ifdef CUICUI_UWP		
	dispatchTimer = ref new Windows::UI::Xaml::DispatcherTimer();

	TimeSpan ts;
	ts.Duration = (long long)(0.016 * 10000000); // 16ms = 60fps    // Duration is in "100-nanosecond" units!!
	dispatchTimer->Interval = ts;
	dispatchTimer->Start();

	// TODO: AnimationManager could have its own "hd" instead of using the one of master (CLEANUP)
	dispatchTimer_registrationtoken = dispatchTimer->Tick += ref new EventHandler<Object^>(master->hd, &cCuiElement_uwp::OnTransitionTick);
#endif
#ifdef CUICUI_WEB
	EM_ASM_({

		globalTransitionEventLoopShouldStop = false;

		window.requestAnimationFrame(function fadeIn(now) {


		var progress = -1; // deprecated
		var elemId = -1; // deprecated

		Module.ccall('onTransitionEvent', 'number',['number', 'number'],[elemId, progress]);


		if (globalTransitionEventLoopShouldStop == true)
		{
			globalTransitionEventLoopShouldStop = false;
		}
		else
		{
			// continue looping
			window.requestAnimationFrame(fadeIn);
		}

		});

		});
#endif
#ifdef CUICUI_ANDROID
	JNICall_Void_IntInt("cuiElement_StartTransitionEventLoop", -1, -1);
#endif
#ifdef CUICUI_IOS
	displayLink = [CADisplayLink displayLinkWithTarget : cuiTransitionEvent selector : @selector(timerFired : )];
	[displayLink addToRunLoop : [NSRunLoop currentRunLoop] forMode : NSDefaultRunLoopMode];
#endif
}

void AnimationManager::StopEventLoop()
{
	assert(transitions.size() == 0);

#ifdef CUICUI_IOS
	[this->displayLink invalidate];
#endif
#ifdef CUICUI_UWP
	if (dispatchTimer == nullptr) return; // it can happens as many "OnTick" can have been posted on the event queue		
	dispatchTimer->Tick -= this->dispatchTimer_registrationtoken;
	dispatchTimer->Stop();
	dispatchTimer = nullptr;
#endif
#ifdef CUICUI_WEB
	EM_ASM_({
		globalTransitionEventLoopShouldStop = true;
		});
#endif
#ifdef CUICUI_ANDROID
	JNICall_Void_IntInt("cuiElement_StopTransitionEventLoop", -1, -1);
#endif



}

wSafeZone::wSafeZone(cColor leftColor, cColor topColor, cColor rightColor, cColor bottomColor) : wColumns()
{
	XemYem(fill());
	Native();
	BgColor(0x000000);

	if (leftColor.IsNull())
		left = new wHGap(pixels(0)); // stays virtual
	else
		left = (wHGap*)(new wHGap(pixels(0)))->BgColor(leftColor);

	if (topColor.IsNull())
		top = new wVGap(pixels(0)); // stays virtual
	else
		top = (wVGap*)(new wVGap(pixels(0)))->BgColor(topColor);

	if (rightColor.IsNull())
		right = new wHGap(pixels(0)); // stays virtual
	else
		right = (wHGap*)(new wHGap(pixels(0)))->BgColor(rightColor);

	if (bottomColor.IsNull())
		bottom = new wVGap(pixels(0)); // stays virtual
	else
		bottom = (wVGap*)(new wVGap(pixels(0)))->BgColor(bottomColor);
		
	master->onSizeChanged = [this](wWidget* self, pixels x, pixels y)
	{
		top->Yem(pixels(central.safeAreaRect.top));
		bottom->Yem(pixels(central.safeAreaRect.bottom));
		left->Xem(pixels(central.safeAreaRect.left));
		right->Xem(pixels(central.safeAreaRect.right));

		RefreshUI_Everything();
	};
}

wSafeZone* wSafeZone::Content(std::function<void(Children & dc)> a_content)
{
	wColumns::Content([this, a_content](Children& dc)
		{
			dc += left;
			dc += (new wRows())->XemYem(fill())->Content([this, a_content](Children& dc)
				{
					dc += top;
					dc += (new wEmpty())->XemYem(fill())->Content(a_content);
					dc += bottom;
				});
			dc += right;
		});
	return this;
}

void AnimationManager::Run()
{
	AssertOnUiThread();

	// note: RunFrame() may destroy one of the transitions, so it's important to get a temporary list to iterate on
	std::list<Transition*> tmp;
	for (auto t : animationManager.transitions)
		tmp.push_back(t);

	auto now = std::chrono::steady_clock::now();

	std::vector<Transition*> toRemove;
	for (auto t : tmp)
	{
		t->RunFrame(now);
		if (t->done)
		{
			toRemove.push_back(t);
		}
	}

	for (auto t : toRemove)
	{
		transitions.remove(t);
		// note: remove() does call the destructor of "t"
	}

	if (transitions.size() == 0)
	{
		StopEventLoop();
	}
}





#ifdef wListViewNative_ENABLED
#if defined(CUICUI_UWP)
// https://docs.microsoft.com/en-us/windows/uwp/design/controls-and-patterns/nested-ui
void cCuiElement_uwp::cic(Windows::UI::Xaml::Controls::ListViewBase^ lvb, Windows::UI::Xaml::Controls::ChoosingItemContainerEventArgs^ args)
{

	Platform::Object ^i = args->Item;
	String ^cellId = i->ToString();
	int cellid = (int)i;

	{

		ListViewItem ^uwpItem = ref new ListViewItem();
		uwpItem->Margin = 0;
		uwpItem->Padding = 0;

		wListView *lv = (wListView*)e;

		wListView::cItem *item = nullptr;
		for (wListView::cItem *i : lv->data)
		{
			if (i->uid == cellid)
			{
				item = i;
				break;
			}
		}
		assert(item != nullptr);

		wWidget *www = lv->onCreateItem(item);
		lv->items.push_back(www);


		//todo: get matching wListView::cItem (or is it the create that gives it?)
		//	and call Create() then SetSTates()

		uwpItem->Content = www->b; // we put it in the ListViewItem

		uwpItem->ContentTemplate = nullptr; // useful?
		args->ItemContainer = uwpItem;
		args->IsContainerPrepared = true;		 // useful?


	}
	{
		CUICUI_NOP // ?
	}
}

void cCuiElement_uwp::ccc(Windows::UI::Xaml::Controls::ListViewBase ^lvb, Windows::UI::Xaml::Controls::ContainerContentChangingEventArgs ^args)
{

	Platform::Object ^i = args->Item;
	String ^cellId = i->ToString();
	int cellid = (int)i;

	if (args->ItemContainer != nullptr)
	{
		wListView *lv = (wListView*)e;

		wListView::cItem *item = nullptr;
		for (wListView::cItem *i : lv->data)
		{
			if (i->uid == cellid)
			{
				item = i;
				break;
			}
		}
		assert(item != nullptr);


		ListViewItem ^uwpItem = (ListViewItem^)args->ItemContainer;
		Grid ^theB = (Grid^)uwpItem->Content;
		if (theB == nullptr)
		{
			CUICUI_NOP
				assert(false); // not normal
		}

		wWidget *www = nullptr;
		for (wWidget *w : lv->items) {
			if (w->b == theB)
			{
				www = w;
				break;
			}
		}
		assert(www != nullptr);

		if (lv->ax[X].size != -666)
		{
			// TODO: update to layout v3
			/*lv->onUpdateItem(www, item);
			RefreshUI_Recur(www, lv->ax[X].size, 0);
			www->x = www->y = 0;
			www->SetPos_HW(www->x, www->y, www->w, www->h);*/
		}
		else
		{
			CUICUI_NOP // the listview has not been computed yet
		}
	}
	else
		assert(false); // not normal?

	args->Handled = true; // need to do it for "cic" to work	
}
#endif
#endif

#ifdef CUICUI_UWP

void cCuiElement_uwp::OnDragOver(Platform::Object ^sender, DragEventArgs ^e) // make it a lambda if possible
{
	e->AcceptedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Copy; // should be "upload" (or nothing at all)...
}


// reads a picked / pasted / dropped file and sends it to onFileReceived (base64)
static task<void> UwpReceiveFile(Windows::Storage::Streams::IRandomAccessStreamWithContentType ^inputStream, String ^mimeType, String ^fileName)
{
	// TODO: try/catch  + event onFileReceivedFailed ?
	Log("uploaded file mimetype : " + UwpStringToStdString(mimeType));

	auto buffer = ref new Windows::Storage::Streams::Buffer((unsigned int)inputStream->Size);
	co_await inputStream->ReadAsync(buffer, (unsigned int)inputStream->Size, Windows::Storage::Streams::InputStreamOptions::None);
	String ^strBase64New = Windows::Security::Cryptography::CryptographicBuffer::EncodeToBase64String(buffer);

	auto pr = std::make_shared<cFileReceived>();
	pr->base64Data = UwpStringToStdString(strBase64New);
	pr->fileName = UwpStringToStdString(fileName);
	pr->mimeType = UwpStringToStdString(mimeType);
	if (onFileReceived != nullptr) onFileReceived(pr);
}

task<void> UwpPickGnia(Windows::Storage::Streams::RandomAccessStreamReference ^stream, String ^_mimetype, String ^_fileName)
{
	auto inputStream = co_await stream->OpenReadAsync();
	co_await UwpReceiveFile(inputStream, _mimetype, _fileName);
}

task<void> UwpPickGnia(Windows::Storage::StorageFile ^file)
{
	auto inputStream = co_await file->OpenReadAsync();
	co_await UwpReceiveFile(inputStream, file->ContentType, file->Name);
}

task<void> OnDropStep2(Windows::ApplicationModel::DataTransfer::DataPackageView ^dataView, wWidget *w)
{
	// option: need to know the mime type to say if we accept or not, and to be able to send it to onFileEvent
	if (dataView->Contains(Windows::ApplicationModel::DataTransfer::StandardDataFormats::StorageItems))
	{
		auto items = co_await dataView->GetStorageItemsAsync();
		if (items->Size > 0)
		{
			Windows::Storage::StorageFile ^storageFile = (Windows::Storage::StorageFile^)items->GetAt(0); // note: just the first one for now
			UwpPickGnia(storageFile);
		}
	}
}

task<void> OnPasteOnTextInputStep2_Bitmap(Windows::ApplicationModel::DataTransfer::DataPackageView ^dataPackageView, wWidget *w)
{

	Windows::Storage::Streams::RandomAccessStreamReference ^stream = co_await dataPackageView->GetBitmapAsync();
	if (stream)
	{

		// NOTE: we know it's an image as it's a Bitmap (see cCuiElement_uwp::OnPasteOnTextInput) and it seems it's always BMP format
		// It's better to convert to jpg or png (as BMP file is large and mostly deprecated)
		// TODO: CONVERT TO JPG
		std::string fakeFileName = GenFakeBmpFileName(); // we don't have a filename so we generate a fake one
		std::string mimeType = "image/bmp"; 

		UwpPickGnia(stream, StdStringToUwpString(mimeType), StdStringToUwpString(fakeFileName));
		
	}
}

void cCuiElement_uwp::OnPasteOnTextInput(Platform::Object ^sender, TextControlPasteEventArgs ^args)
{
	auto dataPackageView = Windows::ApplicationModel::DataTransfer::Clipboard::GetContent();
	if (dataPackageView->Contains(Windows::ApplicationModel::DataTransfer::StandardDataFormats::Bitmap))
	{
		// this is when user did a "screen capture" to clipboard, the content is not a file, it's a Bitmap

		args->Handled = true; // we handle the paste
		OnPasteOnTextInputStep2_Bitmap(dataPackageView, this->e);
	}
	if (dataPackageView->Contains(Windows::ApplicationModel::DataTransfer::StandardDataFormats::StorageItems))
	{
		// this is when user did a "ctrl+c" (copy) on a file

		args->Handled = true; // we handle the paste
		OnDropStep2(dataPackageView, this->e);
	}	
}

void cCuiElement_uwp::OnDrop(Platform::Object ^sender, DragEventArgs ^e)
{
	OnDropStep2(e->DataView, this->e);
}

void cCuiElement_uwp::OnTransitionTick(Object^ sender, Object^ obj)
{
	animationManager.Run();
}

void cCuiElement_uwp::ButtonClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args)
{
}

void cCuiElement_uwp::touchRectangle_PointerEntered(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args)
{
	args->Handled = true;
	if (!CheckDefWidgetAlive(e)) return;

	Windows::UI::Input::PointerPoint ^pp = args->GetCurrentPoint(e->b);
	float x = pp->Position.X;
	float y = pp->Position.Y;

	if (e->onPointerEntered) e->onPointerEntered(e, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
}
void cCuiElement_uwp::touchRectangle_PointerExited(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args)
{
	args->Handled = true;

	if (!CheckDefWidgetAlive(e)) return;


	Windows::UI::Input::PointerPoint ^pp = args->GetCurrentPoint(e->b);
	float x = pp->Position.X;
	float y = pp->Position.Y;

	if (e->coreType == eCoreType::Text)
	{
		wText *m = (wText*)e;
		if (m->previouslySentMouseOverRichTextSpan != nullptr)
		{
			if (m->previouslySentMouseOverRichTextSpan->onMouseExitedRichTextSpan != nullptr)
			{
				m->previouslySentMouseOverRichTextSpan->onMouseExitedRichTextSpan(m->previouslySentMouseOverRichTextSpan);
				// note: we don't "return" here, the widget must receive the onPointerExited
			}
			m->previouslySentMouseOverRichTextSpan = nullptr;
		}
	}

	if (e->onPointerExited) e->onPointerExited(e, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y)); // TODO/QUESTION: call touchRectangle_PointerReleased?
}

static bool uwp_antiDoublePointerReleased = false;

void cCuiElement_uwp::touchRectangle_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args)
{
	args->Handled = true;
	
	/* commented for now, check if this still happens, yes*/
	if (uwp_antiDoublePointerReleased) return;
	uwp_antiDoublePointerReleased = true;
	e->b->ReleasePointerCapture(args->Pointer); // this call triggers a new PointerReleased event, but if we don't call it, we always get two PointerReleased; we use "uwp_antiDoublePointerReleased" as a fix; I don't know if it's a UWP bug or what
												// Note: if we dont user "CapturePointer()" this doesn't happen, but we need to use it so drag event can go outside of widget (for wSlider for example)
	uwp_antiDoublePointerReleased = false;
	

	if (!CheckDefWidgetAlive(e)) return;

	// click only mouse left -> TODO: doesn't work for PointerReleased (whereas it's ok for PointerPressed)
	// this also blocks left click, no?
	/*Windows::UI::Input::PointerPoint ^ptrPt = args->GetCurrentPoint(e->b);
	if (ptrPt->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
		if (ptrPt->Properties->IsLeftButtonPressed == false)
		{
			return;
		}*/

	Windows::UI::Input::PointerPoint ^pp = args->GetCurrentPoint(e->b);
	float x = pp->Position.X;
	float y = pp->Position.Y;

	if (e->coreType == eCoreType::Text)
	{
		wText *m = (wText*)e;
		Windows::UI::Xaml::Documents::TextPointer ^tp = m->tb->GetPositionFromPoint(pp->Position);
		int lastClickedCharacterPosition = tp->Offset; // rename this variable
		// WARNING: m->lastClickedCharacterPosition is the native position, which is NOT the one of the real characters

		bool ret = m->HandleTouchSpan_PointerReleased(lastClickedCharacterPosition);
		if (ret == true) return; // swallow the event
	}

	if (e->recognizer != nullptr)
		e->recognizer->ProcessUpEvent(pp);

	if (e->onPointerReleased && e->onPointerReleased) e->onPointerReleased(e, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
}


void cCuiElement_uwp::touchRectangle_PointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args)
{
	args->Handled = true;
	if (!CheckDefWidgetAlive(e)) return;

	Windows::UI::Input::PointerPoint ^pp = args->GetCurrentPoint(e->b);
	float x = pp->Position.X;
	float y = pp->Position.Y;

	if (e->coreType == eCoreType::Text)
	{
		wText *m = (wText*)e;
		Windows::UI::Xaml::Documents::TextPointer ^tp = m->tb->GetPositionFromPoint(pp->Position);
		int lastClickedCharacterPosition = tp->Offset; // rename this variable
		// WARNING: m->lastClickedCharacterPosition is the native position, which is NOT the one of the real characters

		bool ret = m->HandleTouchSpan_PointerMoved(lastClickedCharacterPosition);
		if (ret == true) return; // swallow the event
	}

	if (e->recognizer != nullptr)
	{
		try {
			auto p = args->GetIntermediatePoints(e->b);
			if (p->Size>0)
				e->recognizer->ProcessMoveEvents(p);
		}
		catch (...)
		{
			Log("touchRectangle_PointerMoved gesture exception");
		}
	}

	if (e->onPointerMoved) {
		cOnPointerMovedArgs args;
		args.x = pixels::CreateFromDouble(x);
		args.y = pixels::CreateFromDouble(y);
		e->onPointerMoved(e, args);
	}
}

void cCuiElement_uwp::touchRectangle_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^args)
{
	args->Handled = true;
	if (!CheckDefWidgetAlive(e)) return;

	// click only mouse left 
	Windows::UI::Input::PointerPoint ^ptrPt = args->GetCurrentPoint(e->b);
	if (ptrPt->PointerDevice->PointerDeviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
		if (ptrPt->Properties->IsLeftButtonPressed == false)
		{
			return;
		}

	Windows::UI::Input::PointerPoint ^pp = args->GetCurrentPoint(e->b);
	float x = pp->Position.X;
	float y = pp->Position.Y;

	try {
		if (e->recognizer != nullptr)
			e->recognizer->ProcessDownEvent(pp); 
	}
	catch (Exception ^e)
	{
		// NOP // happens sometimes but not important I think
	}
	
	bool b = e->b->CapturePointer(args->Pointer); // VERY useful (for wSlider especially)

	if (e->onPointerPressed) e->onPointerPressed(e, pixels::CreateFromDouble(x), pixels::CreateFromDouble(y));
}
/*
void cCuiElement_uwp::multilinetext_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	wText *m = (wText*)e;
	Windows::UI::Xaml::Documents::TextPointer ^tp = m->tb->SelectionStart;
	m->lastClickedCharacterPosition = tp->Offset - 2; // TODO: I don't know why "-2"

	//Windows::UI::Input::PointerPoint ^pp = args->GetCurrentPoint(e->b);
	//float x = pp->Position.X;
	//float y = pp->Position.Y;
	//double ems = GetEmSize(1.0);
	//double xEm = (double)x / ems;
	//double yEm = (double)y / ems;

	//Log("multilinetext_SelectionChanged");

	// probably not but possibly useful: if (e->onPointerPressed) e->onPointerPressed(e, -10666, -10666); // xEm, yEm); // TODO
}*/


void cCuiElement_uwp::textinput_OnKeyUp(Platform::Object ^sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^args)
{
	wTextInput *ti = (wTextInput*)e;
	if (args->Key == Windows::System::VirtualKey::Enter)
	{
		if (ti->onEnterKeyPressed != nullptr) ti->onEnterKeyPressed();
	}
}

void cCuiElement_uwp::textinput_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs ^args)
{
	// DUPLICATE_657
	wTextInput *ti = (wTextInput*)e;
	std::string txt = UwpStringToStdString(ti->textBox->Text);
	ti->currentText = txt;
	if (ti->onTextChanged) ti->onTextChanged(txt);
}
void cCuiElement_uwp::textinput_PasswordChanged(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs ^args)
{
	// DUPLICATE_657 (except textbox/passwordbox)
	wTextInput *ti = (wTextInput*)e;
	std::string txt = UwpStringToStdString(ti->passwordBox->Password);
	ti->currentText = txt;
	if (ti->onTextChanged) ti->onTextChanged(txt);
}


task<void> UwpPickAnImageAndUpload(std::string option) // "images" for images only, or "" for any files
{
	// TODO: try/catch + event onFileReceivedFailed ?

	// TODO: allow png, gif, ...

	auto picker = ref new Windows::Storage::Pickers::FileOpenPicker();
	if (option == "images")
	{
		picker->FileTypeFilter->Append(L".jpg");
		picker->FileTypeFilter->Append(L".jpeg");
		picker->FileTypeFilter->Append(L".png");
		picker->FileTypeFilter->Append(L".gif");
	}
	else
	{
		picker->FileTypeFilter->Append(L"*");
	}
	
	picker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::PicturesLibrary;

	Windows::Storage::StorageFile ^file = co_await picker->PickSingleFileAsync();
	if (nullptr == file)
		return;

	UwpPickGnia(file);
}

#endif


void AnimationManager::DeleteTransitionsOwnedByThisWidget(wWidget *w)
{
	if (transitions.size() == 0)
		return;

	std::vector<Transition*> toRemove;

	for (auto t : transitions)
	{
		if (t->widget == w)
			toRemove.push_back(t);
	}

	for (auto t : toRemove)
	{
		transitions.remove(t);
		// note: remove() does call the destructor of "t"
	}

	if (transitions.size() == 0)
	{
		StopEventLoop();
	}
}

double GetEmSize(double v) 
{
	return v * central.defaultEmSize.asDouble();
}

#ifdef CUICUI_ANDROID
} // namespace cui
#include <android/looper.h>
#include <unistd.h>
namespace cui {
std::mutex androidCuiRunOnUiThreadMutex;
std::list<std::function<void()>> androidCuiRunOnUiThreadList;

// source : https://stackoverflow.com/questions/44808206/android-jni-call-function-on-android-ui-thread-from-c
static ALooper* mainThreadLooper;
static int messagePipe[2];

static int looperCallback(int fd, int events, void* data);

void someJniFuncThatYouShouldCallOnceOnMainThread()
{

	mainThreadLooper = ALooper_forThread(); // get looper for this thread
	ALooper_acquire(mainThreadLooper); // add reference to keep object alive
	pipe(messagePipe); //create send-receive pipe
	// listen for pipe read end, if there is something to read
	// - notify via provided callback on main thread
	ALooper_addFd(mainThreadLooper, messagePipe[0],
		0, ALOOPER_EVENT_INPUT, looperCallback, nullptr);

	// send few messages from arbitrary thread
	/*std::thread worker([]() {
		for (char msg = 100; msg < 110; msg++) {
			LOGI("send message #%d", msg);
			write(messagePipe[1], &msg, 1);
			sleep(1);
		}
	});
	worker.detach();*/
}

static int looperCallback(int fd, int events, void* data) {

#ifdef CUICUI_DEBUG
#endif

	AssertOnUiThread();

	char msg;
	read(fd, &msg, 1); // read message from pipe (block until next message arrives)
	// note: we don't care about the msg BUT it's important to call read() or the looper throttle all the time and consume a lot of cpu/battery

	androidCuiRunOnUiThreadMutex.lock();
	for (auto f : androidCuiRunOnUiThreadList)
	{
		f();
	}
	androidCuiRunOnUiThreadList.clear();
	androidCuiRunOnUiThreadMutex.unlock();



	return 1; // continue listening for events
}

#endif

void RunOnUiThread(std::function<void()> func) // note: func will be executed asynchronously
{
#ifdef CUICUI_DEBUG
	std::thread::id nothreadid;
	assert(cuiUiThreadId != nothreadid); // this can happen on Android if you are calling c++ code from a BrocastReceiver (no CuicuiActivity would have been created, then cuiUiThreadId is not set, then you can't be on ui thread obviously...)										
#endif

	std::thread::id currentThreadId = std::this_thread::get_id();
	if (cuiUiThreadId == currentThreadId)
	{
		// already on UI Thread
		func();
		return;
	}


#if defined(CUICUI_ANDROID)

	AssertNotOnUiThread(); // Note: writing to looper when being on the UI thread make big crashes 

	// enqueue the func
	androidCuiRunOnUiThreadMutex.lock();
	androidCuiRunOnUiThreadList.push_back(func);
	androidCuiRunOnUiThreadMutex.unlock();

	// send message to the looper
	// note: we don't care what message we send
	char msg = '9';
	write(messagePipe[1], &msg, 1);


#elif defined(CUICUI_UWP)
	master->b->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler(func));
#elif defined(CUICUI_WEB)
	func(); // ok : everything is on the UI thread in js, right? :)
#elif defined(CUICUI_IOS)
	dispatch_async(dispatch_get_main_queue(), ^{
		func();
		});
#else
	missing
#endif


}

#ifdef CUICUI_DEBUG
std::string ThreadIdAsString(std::thread::id tid)
{

	std::ostringstream ss;
	ss << std::this_thread::get_id();
	std::string mystring = ss.str();
	return mystring;	
}
#endif


void DebugPrintOnUiThread()
{    
#ifdef CUICUI_DEBUG
	std::thread::id currentThreadId = std::this_thread::get_id();
	if (cuiUiThreadId != currentThreadId)
		Log("DebugPrintOnUiThread: NOT on ui thread : " + ThreadIdAsString(currentThreadId));
	else
		Log("DebugPrintOnUiThread: on ui thread : " + ThreadIdAsString(currentThreadId));
#endif
}


void AssertOnUiThread()
{
#ifdef CUICUI_DEBUG
	std::thread::id nothreadid;

	if (cuiUiThreadId==nothreadid)
	{
		LogError("error_7815"); // this can happen on Android if you are calling c++ code from a BrocastReceiver (no CuicuiActivity would have been created, then cuiUiThreadId is not set, then you can't be on ui thread obviously...)
		assert(false);
	}

	std::thread::id currentThreadId = std::this_thread::get_id();
	if (cuiUiThreadId != currentThreadId)
		assert(false);	
#endif
}
void AssertNotOnUiThread()
{
#ifdef CUICUI_DEBUG
	std::thread::id currentThreadId = std::this_thread::get_id();
	if (cuiUiThreadId == currentThreadId)
		assert(false);
#endif
}



	void wSlider::Apply(bool callOnChanged)
	{		
		if (currentValue < 0) currentValue = 0;
		if (currentValue > 1) currentValue = 1;
		dot->hwPosOffsetX = barWidth * currentValue;
		RefreshUI_Fast(this); // note: important to refresh "this" and not just the dot (as hwPosOffsetX is applied when the parent is relayouted)

		if (callOnChanged)
			if (onChanged) onChanged(this, currentValue);
	}

	void wSlider::SetValue(double f) // [0.0,1.0]
	{		
		currentValue = f;
		if (currentValue < 0.0) currentValue = 0.0;
		if (currentValue > 1.0) currentValue = 1.0;
		//if (onChanged) onChanged(this, currentValue); // No (or yes?) see comment 

		if (this->GetEm(X).IsUnset()==false) // wSlider already layouted?
			Apply(currentValue);	 // yes
	}

	wSlider::wSlider() : wEmpty()
	{		
		hgap = ems(0.5); // constant
		dotDiameter = ems(1); // constant

		pressed = false;
		currentValue = 0;		

		DebugName("wSlider");

		this->Xem(EM_NOT_SET);
		this->Yem(ems(1));
		this->LockYem();

		onSizeChanged = [this](wWidget *self, pixels width, pixels height)
		{			
			hgapPixels.setFromDouble(hgap.v * GetEmSize(1) * emRatio());

			barWidth = width - hgapPixels * 2;
			Apply(false);			
		};

		Content([this](Children &dc)
		{
			wColumns *wc = (wColumns*)wColumns::Create(FILL, FILL)->Content([this](Children &dc)
			{
				auto *leftGap = wEmpty::Create(hgap,FILL)->Touchable();
				leftGap->onPointerPressed = [this](wWidget *self, pixels x, pixels y)
				{					
					currentValue = 0; 
					Apply();				
				};
				dc += leftGap;

				// the black bar
				dc += theBar = (wEmpty*)wEmpty::Create(FILL, FILL)->Content([this](Children &dc)
				{
					dc += wEmpty::Create(FILL, ems(0.5))->CenterVertically()->BgColor(0x000000);
				});

				auto *rightGap = wEmpty::Create(hgap, FILL)->Touchable();
				rightGap->onPointerPressed = [this](wWidget *self, pixels x, pixels y)
				{
					currentValue = 1;
					Apply();
				};
				dc += rightGap;

				theBar->Touchable();
				theBar->onPointerPressed = [this](wWidget *self, pixels x, pixels y) {
					currentValue = (x.asDouble()) / barWidth.asDouble();
					Apply();
					pressed = true;
				};
				theBar->onPointerMoved = [this](wWidget *self, cOnPointerMovedArgs args) {
					if (pressed)
					{
						currentValue = (args.x.asDouble()) / barWidth.asDouble();
						Apply();
					}
				};
				theBar->onPointerReleased = [this](wWidget *self, pixels x, pixels y) {
					pressed = false;					
				};

			});
			dc += wc;
	
			wWidget::BorderStyle dotBorder;
			dotBorder.thicknessInPixels = pixels::CreateFromDouble(1);
			dotBorder.color.Set(0x000000);
			dotBorder.SetAllCornersRadius(ems(0.5));
			dot = (wBorder*)(new wBorder(dotDiameter, dotDiameter, dotBorder, wBorder::ePadding::None))->Content([](Children& dc) { dc += wEmpty::Create(FILL, FILL)->DebugName("dot"); })->BgColor(0x0000ff)->DebugName("wSlider dot border");
			//dot->SetTransparentToTouchEvents(); // TODO/QUESTION: why do we need this? isn't a "non Touchable()" control supposed to let events through in all cases? normally that's the normal behavior of Cuicui

			dc += dot;
		});

		
	}



void cCppMessageBoxManager::SetCallback(std::function<void(std::string, std::string)> _onCppMessageBox)
{
	assert(onCppMessageBox == nullptr); // already set
	onCppMessageBox = _onCppMessageBox;

	cuiCppMessageBox_mutex.lock();
	for (auto m : cuiCppMessageBox_list)
	{
		std::string a = m.first;
		std::string b = m.second;
		RunOnUiThread([this, a, b]() {
			this->onCppMessageBox(a, b);
		});
	}
	cuiCppMessageBox_list.clear();
	cuiCppMessageBox_mutex.unlock();
}

void cCppMessageBoxManager::Send(std::string _tag, std::string _value)
{
	if (onCppMessageBox != nullptr)
	{
		RunOnUiThread([this, _tag, _value]() {
			this->onCppMessageBox(_tag, _value);
		});
	}
	else
	{
		cuiCppMessageBox_mutex.lock();
		cuiCppMessageBox_list.push_back(std::pair<std::string, std::string>(_tag, _value));
		cuiCppMessageBox_mutex.unlock();
	}
}



//// touch manager

extern void GetAllWidgetRecursively_IncludingInScrollViewsContent(std::list<wWidget*> &list, wWidget *w);



// recursive iteration on "master", only on widgets that contains the dot with the coords "xInMaster,yInMaster"
// note: this is fast (not like other GellAllWidget code)
void HT_GetAllWidgetRecursivelyIncludingInScrollViewsContent(double xInMaster, double yInMaster, std::list<wWidget*> &list, wWidget *w)
{
	double outX, outY;
	ConvertCoordinates(w, master, master->ax[X].pos.asDouble(), master->ax[Y].pos.asDouble(), &outX, &outY);

	// TODO: hidden objects (and their children) must not be in list 
	// same for opacity 0 I presume
	// if SetTransparentToTouch() was called
	// other cases?

	if (xInMaster >= outX && xInMaster < outX + w->ax[X].size)
		if (yInMaster >= outY && yInMaster < outY + w->ax[Y].size)
		{
			if (w->coreType == eCoreType::ScrollView)
			{
                list.push_back(w);
				wScrollView *sv = (wScrollView*)w;
				HT_GetAllWidgetRecursivelyIncludingInScrollViewsContent(xInMaster, yInMaster, list, sv->scrollViewContentWidget);
                //list.push_back(w); // important it's AFTER its children -> NO :)
			}
			else
            {
                list.push_back(w);
                for (wWidget *c : w->children) //  // TODO: shouldn't we get the list by iterating each child in reverse order? to get the "top most" first? I don't know if it's useful or necessary
                {
                    HT_GetAllWidgetRecursivelyIncludingInScrollViewsContent(xInMaster, yInMaster, list, c);
                }
            }
		}

}








#ifdef CUICUI_WEB

extern "C" {
	int EMSCRIPTEN_KEEPALIVE visibility_event(int state)
	{
		if (central.onAppStateChanged != nullptr)
		{
			if (state == 1)
				central.onAppStateChanged(cCentral::eAppState::Foreground);
			else
				if (state == 0)
					central.onAppStateChanged(cCentral::eAppState::Background);
				else
					assert(false); // error 
		}
		
		return -1; // useless
	}
}

extern "C" {
	int EMSCRIPTEN_KEEPALIVE resize_event(int w, int h)
	{
		// TODO: should test if w/h has changed until last time we were here

		OnWindowSizeChanged(w, h);

		return -1; // useless
	}
}


extern "C" {
	int EMSCRIPTEN_KEEPALIVE back_button_event(int unused)
	{
		// TODO: DUPLICATE_5668 (one in each platform code), cleaner
		if (central.hardwareBackButtonLinked.size() == 0) return 0;
		wWidget* w = central.hardwareBackButtonLinked.back();
		//central.hardwareBackButtonLinked.pop_back();  NO! it will be pop_back when the widget is destroyed TODO: check we don't make this mistake on the other platforms !
		w->TriggerClickable();
		return 0; // useless? clean up 
	}
}




extern "C" {
	int EMSCRIPTEN_KEEPALIVE onspantouchableevent_hover(int widgetPtr, int riId, float x, float y)
	{
		wText* w = (wText*)widgetPtr;
		if (w->deleted) return -1;

		cSpan* ri = w->GetRichItemFromId(riId);
		if (ri == nullptr) return -1;
		if (ri->onMouseMoveOverRichTextSpan != nullptr)
		{
			ri->onMouseMoveOverRichTextSpan(ri);
		}
		return -1;
	}
}

extern "C" {
	int EMSCRIPTEN_KEEPALIVE onspantouchableevent_click(int widgetPtr, int riId, float x, float y)
	{
		wText* w = (wText*)widgetPtr;
		if (w->deleted) return -1;

		cSpan* ri = w->GetRichItemFromId(riId);
		if (ri == nullptr) return -1;
		if (ri->onMouseClickOnRichTextSpan != nullptr)
		{
			ri->onMouseClickOnRichTextSpan(ri);
		}
		return -1;
	}
}


extern "C" {
	void EMSCRIPTEN_KEEPALIVE cui_webOnPwaReadyToBeInstalled()
	{
		Log("PWA: in cui_webOnPwaReadyToBeInstalled.");
		if (webOnPwaReadyToBeInstalled != nullptr)
		{
			Log("PWA: calling the user callback webOnPwaReadyToBeInstalled");
			webOnPwaReadyToBeInstalled();
		}
		else
		{
			Log("PWA: no user callback webOnPwaReadyToBeInstalled, doing nothing.");
		}
	}
}

extern "C" {
	int EMSCRIPTEN_KEEPALIVE onspantouchableevent_leave(int widgetPtr, int riId, float x, float y)
	{
		wText* w = (wText*)widgetPtr;
		if (w->deleted) return -1;

		cSpan* ri = w->GetRichItemFromId(riId);
		if (ri == nullptr) return -1;
		if (ri->onMouseExitedRichTextSpan != nullptr)
		{
			ri->onMouseExitedRichTextSpan(ri);
		}
		return -1;
	}
}

extern "C" {
	int EMSCRIPTEN_KEEPALIVE wtextinput_keyup_event(int wtextinput_ptr)
	{
		wTextInput* ti = (wTextInput*)wtextinput_ptr;
		if (!CheckDefWidgetAlive(ti)) return -1;
		if (ti->onEnterKeyPressed != nullptr) ti->onEnterKeyPressed();
		return -1;
	}
}

extern "C" {
	int EMSCRIPTEN_KEEPALIVE fileinput_event(char* fileName, char* base64Data, int base64DataSize, char* mimeType)
	{
		std::string tmp1(fileName); // PERF: if a lot of data, a lot of copying (I think we can do something more direct with a memory buffer, see emscripten doc)
		std::string tmp2(mimeType);
		//std::string *tmp2 = new std::string(base64Data); // PERF: if a lot of data, a lot of copying (I think we can do something more direct with a memory buffer, see emscripten doc)
		// note: pointer to avoid stackoverflow if big file
		WebOnFileInputSelection(tmp1, base64Data, base64DataSize, tmp2);
		//delete tmp2;
		return -1; // useless
	}
}

extern "C" {
	int EMSCRIPTEN_KEEPALIVE ontextinput(int wTextInputPtr, char* newText)
	{
		wTextInput* ti = (wTextInput*)wTextInputPtr;
		if (ti->deleted) return -1;
#ifdef CUICUI_DEBUG
		if (Debug_IsDeleted(ti))
		{
			LogError("error 6871");
			assert(false); // should not happen		
		}
#endif
		ti->currentText.assign(newText);
		if (ti->onTextChanged) ti->onTextChanged(newText);

		return -1; // useless
	}
}

extern "C" {
	int EMSCRIPTEN_KEEPALIVE cui_timer_event(int timerUid)
	{
		timerManager.CompletedFromUid(timerUid);
		return -1;// useless
	}
}


extern "C" {
	int EMSCRIPTEN_KEEPALIVE onTransitionEvent(int transitionPtr, int currentTimeInMilliseconds)
	{
		AssertOnUiThread();
		animationManager.Run();
		return -1; // useless
	}
}

} // namespace cui
int main(int argc, char** argv) {


	cuiUiThreadId = std::this_thread::get_id();

	int w = EM_ASM_INT({
		console.log("document.documentElement.clientWidth=" + document.documentElement.clientWidth);
		return document.documentElement.clientWidth;
		});
	int h = EM_ASM_INT({
		console.log("document.documentElement.clientHeight=" + document.documentElement.clientHeight);
		return document.documentElement.clientHeight;
		});

	central.Init();

	OnWindowSizeChanged(w, h);
	EM_ASM(onLoaded()); // note: if it's not here at the end, the layout is not ok at opening 
}

namespace cui {
#endif

} // namespace cui