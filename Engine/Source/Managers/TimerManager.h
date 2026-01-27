#pragma once
#include "Utilities/System/Singleton.h"
#include "Utilities/System/Timer.h"
#include "Core/Engine.h"

#define M_TIMER Krampus::TimerManager::GetInstance()

namespace Krampus
{
	class Timer;

	class TimerManager : public Singleton<TimerManager>
	{
	public:
		Event<> onPauseTimer;
		Event<> onResumeTimer;
		Event<> onStopTimer;

	private:
		sf::Clock clock;					// An object that contains all the time data.

		float lastTimeStamp = -1.0f;		// Timestamp of previous frame (seconds)
		float lastFrameDuration = 0.0f;			// Duration of last frame (raw, seconds)
		float deltaTime = 0.0f;					// Scaled frame duration (lastFrameDuration * timeScale)
		float elapsedTime = 0.0f;					// Raw frame duration
		float timeScale = 1.0f;				// Time speed multiplier

		long long framesCount = 0;				// Total number of frames since program start
		unsigned short maxFrameRate = 60;	// Frame rate cap (0 = unlimited)
		float fps = 0.0f;						
		float smoothedFPS = 60.0f;			// Smoothed FPS for stable display

		std::vector<std::unique_ptr<Timer>> timers;

	private:
		inlin float GetTime(const sf::Time& _time) const
		{
			return _time.asSeconds();
		}
		inlin std::string TwoDigitsTime(const int& _value) const
		{
			if (_value >= 10) return std::to_string(_value);
			return "0" + std::to_string(_value);
		}
	
	public:
		inlin float GetDeltaTime() const
		{
			return deltaTime;
		}
		inlin float GetElapsedTime() const
		{
			return elapsedTime;
		}
		inlin float GetInstantFPS() const
		{
			return fps;
		}
		// Use for display
		inlin float GetSmoothedFPS() const
		{
			return smoothedFPS;
		}
		inlin void SetTimerScale(const float& _timeScale)
		{
			timeScale = _timeScale;
		}
		inlin void SetMaxFrameRate(const unsigned short& _fps)
		{
			maxFrameRate = _fps;
		}
		inlin unsigned short GetMaxFrameRate() const
		{
			return maxFrameRate;
		}
		

	public:
		TimerManager() = default;
		~TimerManager();


	private:
		float Update();
		void UpdateTimers();

	public:
		std::string GetCurrentRealTime() const;

		Timer* CreateTimer(const std::function<void()> _callback, const float& _duration, const bool& _isLoop = false, const bool& _startRunning = true);
		template<typename T, typename MemFn>
		inlin Timer* CreateTimer(T* _instance, MemFn _memFn, const float& _duration, const bool& _isLoop = false, const bool& _startRunning = true)
		{
			return timers.emplace_back(
				std::make_unique<Timer>(_instance, _memFn, _duration, _isLoop, _startRunning)
			).get();
		}
		Timer* CreateTimer(const float& _duration, const bool& _isLoop = false, const bool& _startRunning = true);
		void DestroyTimer(Timer* _toDelete);

		void Pause();
		void Resume();
		void Stop();

		friend void Engine::Update();
	};
}