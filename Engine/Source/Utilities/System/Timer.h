#pragma once
#include "Core/CoreMinimal.h"

namespace Krampus
{
	class Timer : public IPrintable
	{
		Bool					isRunning	= false;
		Bool					isToDelete	= false;
		Bool					isLoop		= false;
		Float					currentTime = 0.0f;
		Float					duration	= 0.0f;
		std::function<void()>	callback;

	public:
		inline Bool IsToDelete() const
		{
			return isToDelete;
		}
		inline void SetDuration(const Float& _duration)
		{
			duration = _duration;
		}
		inline Float GetDuration() const
		{
			return duration;
		}
		inline Bool IsRunning() const
		{
			return isRunning;
		}
		inline void SetIsLoop(const Bool& _isLoop)
		{
			isLoop = _isLoop;
		}
		inline Bool IsLoop() const
		{
			return isLoop;
		}
		inline Float GetTime() const
		{
			return currentTime;
		}
		inline Float GetRemainingTime() const
		{
			return duration - currentTime;
		}

	public:
		Timer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);
		template<typename T, typename MemFn>
		Timer(T* _instance, MemFn _memFn, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true)
		{
			callback = [_instance, _memFn]()
				{
					std::invoke(_memFn, _instance);
				};
			isRunning = _startRunning;
			isLoop = _isLoop;
			duration = _duration;
		}

	public:
		// Start the timer from zero
		void Play();
		// Stop and destroy the timer
		void Stop();
		// Start the timer from the current time
		void Resume();
		// Reset the timer to zero
		void Reset();
		// Pause the timer at the current time
		void Pause();
		void Update(const Float& _deltaTime);

		virtual std::string ToString() const override
		{
			return std::format("Time : {} / {}", currentTime.ToString(), duration.ToString()) +
				", IsRunning = " + isRunning.ToString() +
				", IsLoop = " + isLoop.ToString();
		}
	};

}

/////////////////////////////////////////////////////////////
// 
//  TimerManager& _timerManager = M_TIMER;
// 
//	_timerManager.CreateTimer([]() { LOG_MSG("Timer finished!"); }, 2.0f);
// 
//	_timerManager.CreateTimer(&TestTimer, 2.0f);
// 
//	MyClass _myClass;
//	Timer* _timer = _timerManager.CreateTimer(&_myClass, &MyClass::TestTimer, 2.0f, true, false);
// 
//	_timer->Play();
//  _timer->Pause();
//  _timer->Resume();
//  _timer->Reset();
//  _timer->Stop();
// 
/////////////////////////////////////////////////////////////