// headers needed by cuicui
// put there to be used with precompiled headers file (on UWP only for now)
// note: not yet tried to make it work

#ifdef CUICUI_DEBUG
#ifndef NOMINMAX
#define NOMINMAX // windows has macros "min" and "max" which are useless and annoying, this define removes them
#endif
#endif

#include <string>
#include <codecvt>
#include <locale>
#include <cmath>
#include <locale>
#include <vector>
#include <list>
#include <assert.h>
#include <string> 
#include <functional>
#include <future>
#include <thread>
#include <map>
#include <thread> // WARNING: ok on web? it depends, see "_notes.txt"
#include <chrono> // same question: I think so, check whether Timer works :)
#include <mutex>
#include <sstream>
#include <memory>
#include <float.h>

#ifdef CUICUI_IOS
#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#endif

#ifdef CUICUI_UWP
#include <collection.h>
#include <ppltasks.h>
#include <ppltasks.h>
#include <experimental\resumable>
#include <pplawait.h>
#endif




#if defined(CUICUI_ANDROID)
#include <jni.h>
#include <android/log.h>
#include <codecvt>
namespace cui {
extern JNIEnv *globalCuiJniEnv; // NOTE: this is the UI THREAD env. Set in cuiMain (NOT in JNI_OnLoad, which may run on a background thread, e.g. process started by FCM). A JNIEnv* is bound to its thread.
extern jobject globalCuiJniObj;
extern jclass globalCuiMainActivityJClass;
extern JavaVM *globalCuiJavaVm;      // stored by JNI_OnLoad: the only JNI handle valid on any thread.
JNIEnv *CurrentThreadJniEnv();       // current thread's env via the JavaVM (nullptr if thread not attached). Do NOT cache the result.
} // namespace cui
#endif

#if  defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <sys/time.h>
#include <codecvt>
#endif
