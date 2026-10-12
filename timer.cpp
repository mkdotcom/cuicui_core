#include "platform.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include "timer.h"
#include "cuicui.h"

namespace cui {
	int cuiTimerGlobalGuidCount = 0;
	TimerManager timerManager;


	void TimerManager::Remove(Timer * t)
	{
		AssertOnUiThread(); // note: we don't need mutex as we are on UI thread
		for (auto i : timers)
		{
			if (i == t)
			{
				timers.remove(t);
				delete t;
				return;
			}
		}
	}

	Timer* TimerManager::Create(double durationInSeconds, std::function<void(Timer *timer)> _func)
	{
		AssertOnUiThread(); // note: we don't need mutex as we are on UI thread

		Timer *t = new Timer();
		t->func = _func;
		t->guid = cuiTimerGlobalGuidCount++;
		t->duration = durationInSeconds;
		timers.push_back(t);

		Start(t);

		return t;
	}

	void TimerManager::Completed(Timer *t)
	{
		AssertOnUiThread();

#ifndef CUICUI_WEB
		// experimental with threads (not working for now on web)
		t->func(t);
		timerManager.Remove(t);
        return;
#else

    #ifdef CUICUI_WEB
        CUICUI_NOP // setTimeout stops by itself
    #endif
        t->func(t);
		timerManager.Remove(t);
        return;
#endif
	}

	void TimerManager::CompletedFromUid(int uid)
	{
		AssertOnUiThread(); // note: we don't need mutex as we are on UI thread
		for (auto i : timers)
			if (i->guid == uid)
			{
				Completed(i);
				return;
			}
	}

	void TimerManager::Start(Timer * t)
	{
#ifndef CUICUI_WEB
		// experimental using thread (ok everywhere but web, until thread works fine; maybe already ok?)
		// note: the uwp native version can be removed if this is ok (it seems so)
		t->th = new std::thread([this, t]
		{
			std::this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::milliseconds((int)(t->duration*1000.0)));
			RunOnUiThread([this, t]() {
				Completed(t);
			});
		});
		return;
#endif

#if defined(CUICUI_ANDROID)
		// nop
#elif defined(CUICUI_WEB)
		EM_ASM_({
			var timerUid = $0;
		var durationInMilliseconds = $1;
		setTimeout(function() { cuiTimerEvent($0); }, durationInMilliseconds);
			}, t->guid, (int)(t->duration*1000.0));
#elif defined(CUICUI_IOS)
		// nop
#else
		// nop
#endif
	}

	Timer::~Timer()
	{
#ifndef CUICUI_WEB
		th->detach();
		delete th;
#endif
	}


} // namespace cui