#pragma once
#include "Core/CoreMinimal.h"

namespace Krampus
{
	class Timer
	{
		bool isRunning = false;
		bool isToDelete = false;
		bool isLoop = false;
		float currentTime = 0.0f;
		float duration = 0.0f;

	public:
		Event<> callback;

	public:
		inlin bool IsToDelete() const
		{
			return isToDelete;
		}
		inlin void SetDuration(const float& _duration)
		{
			duration = _duration;
		}
		inlin float GetDuration() const
		{
			return duration;
		}
		inlin bool IsRunning() const
		{
			return isRunning;
		}
		inlin void SetIsLoop(const bool& _isLoop)
		{
			isLoop = _isLoop;
		}
		inlin bool IsLoop() const
		{
			return isLoop;
		}
		inlin float GetTime() const
		{
			return currentTime;
		}
		inlin float GetRemainingTime() const
		{
			return duration - currentTime;
		}

	public:
		Timer(const std::function<void()>& _callback, const float& _duration, const bool& _isLoop = false, const bool& _startRunning = true);
		template<typename T, typename MemFn>
		Timer(T* _instance, MemFn _memFn, const float& _duration, const bool& _isLoop = false, const bool& _startRunning = true)
		{
			callback.AddListener(_instance, _memFn);
			isRunning = _startRunning;
			isLoop = _isLoop;
			duration = _duration;
		}
		Timer(const float& _duration, const bool& _isLoop = false, const bool& _startRunning = true);

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
		void Update(const float& _deltaTime);
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