#pragma once
#include "Core/CoreMinimal.h"

namespace Krampus
{
	class Timer
	{
		bool isToDelete = false;
		bool isRunning = false;
		bool isLoop = false;
		float currentTime = 0.0f;
		float duration = 0.0f;

	public:
		Event<> callback;

	public:
		INLINE void SetDuration(const float& _duration)
		{
			duration = _duration;
		}
		INLINE bool IsToDelete() const
		{
			return isToDelete;
		}
		INLINE bool IsRunning() const
		{
			return isRunning;
		}
		INLINE bool IsLoop() const
		{
			return isLoop;
		}
		INLINE float GetTime() const
		{
			return currentTime;
		}
		INLINE float GetRemainingTime() const
		{
			return duration - currentTime;
		}

	public:
		Timer(const std::function<void()>& _callback, const float& _duration, const bool& _startRunning = true, const bool& _isLoop = false);
		Timer(const float& _duration, const bool& _startRunning = true, const bool& _isLoop = false);

	public:
		void Start();
		void Stop();
		void Resume();
		void Reset();
		void Pause();
		void Update(const float& _deltaTime);
	};

}

/////////////////////////////////////////////////////////////
// 
// M_TIMER.CreateTimer([](){ LOG(engine::VerbosityType::Display, "Timer");}, 2.0f);
// 
// void TestTimer() 
// {
//	   LOG(engine::VerbosityType::Display, "Timer");
// }
// 
// Timer* _timer = M_TIMER.CreateTimer(TestTimer, 2.0f);
// _timer->callback.AddListener(TestTimer);
// _timer->Pause();
// _timer->Resume();
// _timer->Stop();
// 
/////////////////////////////////////////////////////////////