#pragma once
#include "Utilities/System/Timer.h"
#include "Core/KrampusObject.h"

namespace Krampus
{
	class Timer;

	class TimerManager : public KrampusObject
	{
	public:
		Event<> onPauseTimer;
		Event<> onResumeTimer;
		Event<> onStopTimer;

	private:
		sf::Clock clock;					// An object that contains all the time data.

		Float lastTimeStamp = -1.0f;		// Timestamp of previous frame (seconds)
		Float lastFrameDuration = 0.0f;		// Duration of last frame (raw, seconds)
		Float deltaTime = 0.0f;				// Scaled frame duration (lastFrameDuration * timeScale)
		Float elapsedTime = 0.0f;			// Raw frame duration
		Float timeScale = 1.0f;				// Time speed multiplier

		ULongLong framesCount = 0;			// Total number of frames since program start
		UInt maxFrameRate = 60;				// Frame rate cap (0 = unlimited)
		Float fps = 0.0f;					// Frames Per Seconds
		Float smoothedFPS = 60.0f;			// Smoothed FPS for stable display

		std::vector<std::unique_ptr<Timer>> timers;

	private:
		static inline Float GetTime(const sf::Time& _time) 
		{
			return _time.asSeconds();
		}
		static inline String TwoDigitsTime(const Int& _value)
		{
			if (_value >= 10) return _value.ToString();
			return "0" + _value.ToString();
		}
	
	public:
		inline Float GetDeltaTime() const
		{
			return deltaTime;
		}
		inline Float GetElapsedTime() const
		{
			return elapsedTime;
		}
		inline Float GetInstantFPS() const
		{
			return fps;
		}
		// Use for display
		inline Float GetSmoothedFPS() const
		{
			return smoothedFPS;
		}
		inline void SetTimerScale(const Float& _timeScale)
		{
			timeScale = _timeScale;
		}
		inline void SetMaxFrameRate(const UInt& _fps)
		{
			maxFrameRate = _fps;
			GetWorld()->GetWindowRef().SetFramerateLimit(maxFrameRate);
		}
		inline UInt GetMaxFrameRate() const
		{
			return maxFrameRate;
		}
		

	public:
		TimerManager(Engine* _engine);
		~TimerManager();


	private:
		void UpdateTimers();

	public:
		Float Update();
		static String GetCurrentRealTime();

		Timer* CreateTimer(const std::function<void()> _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);
		template<typename T, typename MemFn>
		inline Timer* CreateTimer(T* _instance, MemFn _memFn, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true)
		{
			return timers.emplace_back(
				std::make_unique<Timer>(_instance, _memFn, _duration, _isLoop, _startRunning)
			).get();
		}
		void DestroyTimer(Timer* _toDelete);

		void Pause();
		void Resume();
		void Stop();
	};
}