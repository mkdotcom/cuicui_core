#pragma once




namespace cui {
	class Timer
	{
	public:
		std::function<void(Timer * timer)> func;
		double duration; // seconds
		int guid;
#ifndef CUICUI_WEB
		std::thread *th;
#endif

	public:
		~Timer();
	};

	extern int cuiTimerGlobalGuidCount;

	class TimerManager
	{
	public:

	protected:
		std::list<Timer*> timers;


	public:
		TimerManager()
		{

		}
		void Remove(Timer *t);
		Timer *Create(double durationInSeconds, std::function<void(Timer * timer)> _func);

		// should be protected
		void Completed(Timer *t);
		void CompletedFromUid(int uid);

		void Start(Timer *t);
	};
	extern TimerManager timerManager;



} // namespace cui