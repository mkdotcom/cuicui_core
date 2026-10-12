#include <jni.h>
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include <string>

#include "../cuicui.h"
#include "../web_request.h"


namespace cui {
extern std::thread::id cuiUiThreadId;
JNIEnv *globalCuiJniEnv=NULL;
jobject globalCuiJniObj;


jclass globalCuiMainActivityJClass;  // note: it's supposed to be ok, but i've read that it's not... BUT we need it, or there is weird crash after RunOnUiThread/looper (FindClass always returns null)
// note2: maybe the timerthread should be "attached to the JNI" (or something like that) ? maybe that would fix the FindClass returning null after being in RunOnUiThread/looper)
// of course this "globalCuiMainActivityJClass" way if better/faster if it always work




#ifdef listview_not_finished
extern "C"
JNIEXPORT int
JNICALL
Java_io_github_mkdotcom_cuicui_CuicuiActivity_cuiListViewCreateOneCell(
        JNIEnv *env,
        jobject obj/* this */,
        int cellId
        )
{
    //globalCuiJniEnv = env; // TODO: do this only once at startup?
    //globalCuiJniObj = obj; // TODO: do this only once at startup?

    AndroidListViewCreateOneCell(cellId);
    return -1; // useless
}


extern "C"
JNIEXPORT void
JNICALL
Java_io_github_mkdotcom_cuicui_CuicuiActivity_cuiListViewUpdateCell(
        JNIEnv *env,
        jobject obj/* this */,
        int wListViewAndroidId,
        int cellId,
        int viewId
)
{
    AndroidListViewUpdateCell(wListViewAndroidId, cellId, viewId);

}
#endif

extern "C"
void
JNICALL
cuijni_cuiAndroidOnFileInputSelection(
        JNIEnv *env,
        jobject obj/* this */,
        jstring fileName,
        jstring base64Data,
        jstring mimeType,
        int imageOrientation,
        jstring option)
{
    std::string sBase64Data = JstringToStdString(env, base64Data);
    std::string sOption = JstringToStdString(env, option);

    AndroidOnFileInputSelection(JstringToStdString(env, fileName), sBase64Data, sBase64Data.length(), JstringToStdString(env, mimeType), imageOrientation, sOption);
}

extern "C"
void
JNICALL
cuijni_cuiTextInputChangedEvent(
        JNIEnv *env,
        jobject obj/* this */,
        jlong widgetPtr,
        jstring newText)
{
            wTextInput *ti = (wTextInput*)widgetPtr;
            if (!CheckDefWidgetAlive(ti)) return;
            ti->currentText = JstringToStdString(env, newText); // very important
            if (ti->onTextChanged!=nullptr) ti->onTextChanged(ti->currentText);
            return;
}

// TODO option: name them all Java_io_github_mkdotcom_cuicui_CuicuiActivity_nativeBlabla ? that way in the java we call "nativeBlabla" ?

extern "C"
void
JNICALL
cuijni_scrollViewUserScrolled(
        JNIEnv *env,
        jobject obj/* this */,
        jlong widgetPtr)
{
    wScrollView *sw = (wScrollView*)widgetPtr;
    if (sw->coreType != eCoreType::ScrollView)
        return;
    if (!CheckDefWidgetAlive(sw)) return;
    if (sw->onUserScrolled != nullptr)
        sw->onUserScrolled();
}


extern "C"
int
JNICALL
cuijni_cuiHandleTouchSpanReleased(
        JNIEnv *env,
        jobject obj/* this */,
        jlong widgetPtr,
        int characterPos)
{
    wWidget *_w = (wWidget*)widgetPtr;
    wText *w = (wText*)_w;
    if (_w==nullptr) return 1; // swallow the event
    if (!CheckDefWidgetAlive(_w)) return 1; // swallow the event

    if (w->coreType != eCoreType::Text) return 0; // what is this for? CLEANUP: remove

    bool ret = w->HandleTouchSpan_PointerReleased(characterPos);
    if (ret == true) return 1; // swallow the event
    return 0;
}

extern "C"
int
JNICALL
cuijni_cuiGetBackButtonManagerPushedElementsCount(
        JNIEnv *env,
        jobject obj/* this */)
{
    return central.hardwareBackButtonLinked.size();
}


extern "C"
void
JNICALL
cuijni_cuiVolumeDownKeyEvent(
        JNIEnv *env,
        jobject obj/* this */)
{
    if (central.onVolumeDownKeyPressed!=nullptr) {
        AssertOnUiThread();
        central.onVolumeDownKeyPressed();
    }
}

extern "C"
void
JNICALL
cuijni_cuiVolumeUpKeyEvent(
        JNIEnv *env,
        jobject obj/* this */)
{
    if (central.onVolumeUpKeyPressed!=nullptr)
    {
        AssertOnUiThread();
        central.onVolumeUpKeyPressed();
    }

}

extern "C"
void
JNICALL
cuijni_cuiAnimationEvent(
        JNIEnv *env,
        jobject obj/* this */,
        int useless,
        int currentTimeInMilliseconds)
{
    animationManager.Run();
}


extern "C"
void
JNICALL
cuijni_cuiWebRequestEvent(
        JNIEnv *env,
        jobject obj/* this */,
        int webRequestId,
        jstring result,
        jboolean success,
        int httpStatusCode)
{
    webRequestMutex.lock();
    for (WebRequest *wr : webRequests_private)
    {
        if (wr->GetGuid() == webRequestId)
        {
            if (success) {
                std::string ss = JstringToStdString(env, result);

                webRequestMutex.unlock();
                wr->Success(ss, httpStatusCode);
                return;
            }
            else {
                webRequestMutex.unlock();
                wr->Failed();
                return;
            }
        }
    }
    webRequestMutex.unlock();
}


extern "C"
void
JNICALL
cuijni_cuiAppSizeChanged(
        JNIEnv *env,
        jobject obj/* this */,
        int width,
        int height)
{
    OnWindowSizeChanged(width,height);
}




extern "C"
void
JNICALL
cuijni_cuiTextInputDonePressed(
        JNIEnv *env,
        jobject obj/* this */,
        jlong widgetPtr)
{
    wWidget *_w = (wWidget*)widgetPtr;
    if (_w==nullptr) return;
    if (!CheckDefWidgetAlive(_w)) return;
    wTextInput *wi = (wTextInput*)_w;
    if (wi->onEnterKeyPressed != nullptr) wi->onEnterKeyPressed();
}



extern "C"
void
JNICALL
cuijni_nativecuiCppMessageBox( // Note : this is a "NativeCaller" package native function (not MainActivity)
        JNIEnv *env,
        jclass cls,
        jstring tag,
        jstring value
)
{
    if (tag==NULL) return;

    // TODO: make a static jstring->std::string conversion that handles NULL and use it everywhere ()

    std::string sTag = JstringToStdString(env, tag);
    std::string sValue = JstringToStdString(env, value);

    cppMessageBoxManager.Send(sTag, sValue);
}

extern "C"
void
JNICALL
cuijni_cuiBackButtonEvent(
        JNIEnv *env,
        jobject obj/* this */
        )
{
    // TODO: DUPLICATE_5668 (one in each platform code), cleaner
    if (central.hardwareBackButtonLinked.size() == 0) return;
    wWidget *w = central.hardwareBackButtonLinked.back();
    //central.hardwareBackButtonLinked.pop_back();  NO! it will be pop_back when the widget is destroyed TODO: check we don't make this mistake on the other platforms !
    w->TriggerClickable();
}



extern "C"
void
JNICALL
cuijni_cuiMain( // note: this one can't go in "NativeCaller" as it's the one which provide the globalCuiJniEnv/Obj/Jclass/...
        JNIEnv *env,
        jobject obj/* this */,
        int androidRootView)
{

    // cuiMain runs on the UI thread: this is WHERE we capture the env (and not in JNI_OnLoad,
    // which can run on a background thread during an FCM cold-start). A JNIEnv* is tied to its thread.
    globalCuiJniEnv = env;
    globalCuiJniObj = env->NewGlobalRef(obj);


    // https://stackoverflow.com/questions/2093112/why-i-should-not-reuse-a-jclass-and-or-jmethodid-in-jni
            // it says we can have globalCuiMainActivityJClass as a globalref
                    // PERF:  Method ID and field ID values are forever (we request them again every time)
                                // NOTE: apparently no need to even do NewGlobalRef? see the link (one comment says the opposite)

    jclass temp = env->FindClass("io/github/mkdotcom/cuicui/CuicuiActivity"); // env (current thread), definitely not a cache from another thread -> this was THE SIGSEGV line
    globalCuiMainActivityJClass = (jclass)env->NewGlobalRef(temp);
    env->DeleteLocalRef(temp);

    someJniFuncThatYouShouldCallOnceOnMainThread();

    cuiUiThreadId = std::this_thread::get_id();
#ifdef CUICUI_DEBUG
    //Log("cuiUiThreadId = " + ThreadIdAsString(cuiUiThreadId));
#endif


    central.Init();





}
} // namespace cui

// ---- native methods registration (RegisterNatives, called from JNI_OnLoad)
// The C++ functions are not looked up by their name (Java_<package>_<class>_<method>), so they don't
// depend on the Java package; and a missing / mismatching method fails at load time, not at the first call.
// When adding a native method: declare it here and add it to the table of its Java class.
extern "C" void JNICALL cuijni_cuiAndroidOnFileInputSelection(JNIEnv *env, jobject obj, jstring fileName, jstring base64Data, jstring mimeType, int imageOrientation, jstring option);
extern "C" void JNICALL cuijni_cuiTextInputChangedEvent(JNIEnv *env, jobject obj, jlong widgetPtr, jstring newText);
extern "C" void JNICALL cuijni_scrollViewUserScrolled(JNIEnv *env, jobject obj, jlong widgetPtr);
extern "C" int JNICALL cuijni_cuiHandleTouchSpanReleased(JNIEnv *env, jobject obj, jlong widgetPtr, int characterPos);
extern "C" int JNICALL cuijni_cuiGetBackButtonManagerPushedElementsCount(JNIEnv *env, jobject obj);
extern "C" void JNICALL cuijni_cuiVolumeDownKeyEvent(JNIEnv *env, jobject obj);
extern "C" void JNICALL cuijni_cuiVolumeUpKeyEvent(JNIEnv *env, jobject obj);
extern "C" void JNICALL cuijni_cuiAnimationEvent(JNIEnv *env, jobject obj, int useless, int currentTimeInMilliseconds);
extern "C" void JNICALL cuijni_cuiWebRequestEvent(JNIEnv *env, jobject obj, int webRequestId, jstring result, jboolean success, int httpStatusCode);
extern "C" void JNICALL cuijni_cuiAppSizeChanged(JNIEnv *env, jobject obj, int width, int height);
extern "C" void JNICALL cuijni_cuiTextInputDonePressed(JNIEnv *env, jobject obj, jlong widgetPtr);
extern "C" void JNICALL cuijni_nativecuiCppMessageBox(JNIEnv *env, jclass cls, jstring tag, jstring value);
extern "C" void JNICALL cuijni_cuiBackButtonEvent(JNIEnv *env, jobject obj);
extern "C" void JNICALL cuijni_cuiMain(JNIEnv *env, jobject obj, int androidRootView);
extern "C" void JNICALL cuijni_cuiSetCentralSafeAreaRect(JNIEnv * env, jobject obj, jint l, jint t, jint r, jint b);
extern "C" jint JNICALL cuijni_cuiIsNativeWithTouch(JNIEnv *env, jobject obj, jlong widgetPtr);
extern "C" void JNICALL cuijni_wImageIsAnimated(JNIEnv *env, jobject obj, jlong widgetPtr);
extern "C" void JNICALL cuijni_cuicuiShowSimpleMessageBoxYesNo(JNIEnv * env, jobject obj, int answer);
extern "C" void JNICALL cuijni_cuiTouchEvent(JNIEnv *env, jobject obj, jlong widgetPtr, int eventType, float _x, float _y, float c, float d);
extern "C" void JNICALL cuijni_cuiGestureDoubleTapEvent(JNIEnv *env, jobject obj, jlong widgetPtr, float _x, float _y);
extern "C" void JNICALL cuijni_cuiGestureSingleTapEvent(JNIEnv * env, jobject obj, jlong widgetPtr, float _x, float _y);
extern "C" void JNICALL cuijni_cuiGestureLongPressEvent(JNIEnv * env, jobject obj, jlong widgetPtr, float _x, float _y);
extern "C" void JNICALL cuijni_cuiGesturePanEvent(JNIEnv *env, jobject obj, jlong widgetPtr, float _x, float _y, float offsetX, float offsetY, int phase);
extern "C" void JNICALL cuijni_cuiGesture2FingersPanEvent(JNIEnv *env, jobject obj, jlong widgetPtr, float _x, float _y, float offsetX, float offsetY, int phase);
extern "C" void JNICALL cuijni_cuiGestureScaleEvent(JNIEnv *env, jobject obj, jlong widgetPtr, float scale, float _x, float _y, int phase);
extern "C" void JNICALL cuijni_cuicuiOnAppStateChanged(JNIEnv * env, jobject obj, int state);
extern "C" void JNICALL cuijni_wDatePickerOnDateChanged(JNIEnv *env, jobject obj, jlong widgetPtr, int y, int m, int d);
extern "C" void JNICALL cuijni_wTimePickerOnDateChanged(JNIEnv *env, jobject obj, jlong widgetPtr, int h, int m);
extern "C" void JNICALL cuijni_wImageOnLoaded(JNIEnv *env, jobject obj, jlong widgetPtr, int state, int imageWidth, int imageHeight);

namespace cui {
static bool RegisterJavaNatives(JNIEnv *env, const char *className, const JNINativeMethod *methods, int count)
{
	jclass c = env->FindClass(className);
	if (c == nullptr) { LogError(std::string("RegisterNatives: class not found: ") + className); return false; }
	bool ok = env->RegisterNatives(c, methods, count) == JNI_OK;
	if (!ok) LogError(std::string("RegisterNatives failed for ") + className);
	env->DeleteLocalRef(c);
	return ok;
}

bool RegisterCuicuiNatives(JNIEnv *env)
{
	static const JNINativeMethod methodsCuicuiActivity[] = {
		{"cuiAndroidOnFileInputSelection", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)V", reinterpret_cast<void*>(cuijni_cuiAndroidOnFileInputSelection)},
		{"cuiTextInputChangedEvent", "(JLjava/lang/String;)V", reinterpret_cast<void*>(cuijni_cuiTextInputChangedEvent)},
		{"scrollViewUserScrolled", "(J)V", reinterpret_cast<void*>(cuijni_scrollViewUserScrolled)},
		{"cuiHandleTouchSpanReleased", "(JI)I", reinterpret_cast<void*>(cuijni_cuiHandleTouchSpanReleased)},
		{"cuiGetBackButtonManagerPushedElementsCount", "()I", reinterpret_cast<void*>(cuijni_cuiGetBackButtonManagerPushedElementsCount)},
		{"cuiVolumeDownKeyEvent", "()V", reinterpret_cast<void*>(cuijni_cuiVolumeDownKeyEvent)},
		{"cuiVolumeUpKeyEvent", "()V", reinterpret_cast<void*>(cuijni_cuiVolumeUpKeyEvent)},
		{"cuiAnimationEvent", "(II)V", reinterpret_cast<void*>(cuijni_cuiAnimationEvent)},
		{"cuiWebRequestEvent", "(ILjava/lang/String;ZI)V", reinterpret_cast<void*>(cuijni_cuiWebRequestEvent)},
		{"cuiAppSizeChanged", "(II)V", reinterpret_cast<void*>(cuijni_cuiAppSizeChanged)},
		{"cuiTextInputDonePressed", "(J)V", reinterpret_cast<void*>(cuijni_cuiTextInputDonePressed)},
		{"cuiBackButtonEvent", "()V", reinterpret_cast<void*>(cuijni_cuiBackButtonEvent)},
		{"cuiMain", "(I)V", reinterpret_cast<void*>(cuijni_cuiMain)},
		{"cuiSetCentralSafeAreaRect", "(IIII)V", reinterpret_cast<void*>(cuijni_cuiSetCentralSafeAreaRect)},
		{"cuiIsNativeWithTouch", "(J)I", reinterpret_cast<void*>(cuijni_cuiIsNativeWithTouch)},
		{"wImageIsAnimated", "(J)V", reinterpret_cast<void*>(cuijni_wImageIsAnimated)},
		{"cuicuiShowSimpleMessageBoxYesNo", "(I)V", reinterpret_cast<void*>(cuijni_cuicuiShowSimpleMessageBoxYesNo)},
		{"cuiTouchEvent", "(JIFFFF)V", reinterpret_cast<void*>(cuijni_cuiTouchEvent)},
		{"cuiGestureDoubleTapEvent", "(JFF)V", reinterpret_cast<void*>(cuijni_cuiGestureDoubleTapEvent)},
		{"cuiGestureSingleTapEvent", "(JFF)V", reinterpret_cast<void*>(cuijni_cuiGestureSingleTapEvent)},
		{"cuiGestureLongPressEvent", "(JFF)V", reinterpret_cast<void*>(cuijni_cuiGestureLongPressEvent)},
		{"cuiGesturePanEvent", "(JFFFFI)V", reinterpret_cast<void*>(cuijni_cuiGesturePanEvent)},
		{"cuiGesture2FingersPanEvent", "(JFFFFI)V", reinterpret_cast<void*>(cuijni_cuiGesture2FingersPanEvent)},
		{"cuiGestureScaleEvent", "(JFFFI)V", reinterpret_cast<void*>(cuijni_cuiGestureScaleEvent)},
		{"cuicuiOnAppStateChanged", "(I)V", reinterpret_cast<void*>(cuijni_cuicuiOnAppStateChanged)},
		{"wDatePickerOnDateChanged", "(JIII)V", reinterpret_cast<void*>(cuijni_wDatePickerOnDateChanged)},
		{"wTimePickerOnDateChanged", "(JII)V", reinterpret_cast<void*>(cuijni_wTimePickerOnDateChanged)},
		{"wImageOnLoaded", "(JIII)V", reinterpret_cast<void*>(cuijni_wImageOnLoaded)},
	};
	static const JNINativeMethod methodsNativeCaller[] = {
		{"nativecuiCppMessageBox", "(Ljava/lang/String;Ljava/lang/String;)V", reinterpret_cast<void*>(cuijni_nativecuiCppMessageBox)},
	};
	return RegisterJavaNatives(env, "io/github/mkdotcom/cuicui/CuicuiActivity", methodsCuicuiActivity, sizeof(methodsCuicuiActivity) / sizeof(JNINativeMethod))
		&& RegisterJavaNatives(env, "io/github/mkdotcom/cuicui/NativeCaller", methodsNativeCaller, sizeof(methodsNativeCaller) / sizeof(JNINativeMethod));
}
} // namespace cui
