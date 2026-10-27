#pragma once
#include "Utilities/System/Timer.h"
#include "Core/KrampusObject.h"

namespace Krampus
{
	class Timer;

	/// <summary>
	/// Manages frame timing (delta time, elapsed time, FPS), time dilation scale, and active timers.
	/// </summary>
	class TimerManager : public KrampusObject
	{
	public:
		/// <summary>Broadcast when all timers are paused.</summary>
		Event<> onPauseTimer;
		/// <summary>Broadcast when all timers are resumed.</summary>
		Event<> onResumeTimer;
		/// <summary>Broadcast when all timers are stopped.</summary>
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
		/// <summary>
		/// Gets the scaled frame delta time in seconds (multiplied by timeScale).
		/// </summary>
		/// <returns>Delta time as Float.</returns>
		inline Float GetDeltaTime() const
		{
			return deltaTime;
		}

		/// <summary>
		/// Gets the raw unscaled frame duration in seconds.
		/// </summary>
		/// <returns>Elapsed frame time as Float.</returns>
		inline Float GetElapsedTime() const
		{
			return elapsedTime;
		}

		/// <summary>
		/// Gets the instantaneous frames-per-second measurement of the current frame.
		/// </summary>
		/// <returns>Current FPS.</returns>
		inline Float GetInstantFPS() const
		{
			return fps;
		}

		/// <summary>
		/// Gets the exponentially smoothed frames-per-second value suitable for UI display.
		/// </summary>
		/// <returns>Smoothed FPS.</returns>
		inline Float GetSmoothedFPS() const
		{
			return smoothedFPS;
		}

		/// <summary>
		/// Sets the time dilation scale multiplier (1.0 is normal speed, 0.5 is slow motion).
		/// </summary>
		/// <param name="_timeScale">Time multiplier.</param>
		inline void SetTimerScale(const Float& _timeScale)
		{
			timeScale = _timeScale;
		}

		/// <summary>
		/// Caps the engine framerate limit and configures the window limit.
		/// </summary>
		/// <param name="_fps">Target frame rate cap (0 = unlimited).</param>
		inline void SetMaxFrameRate(const UInt& _fps)
		{
			maxFrameRate = _fps;
			GetWorld()->GetWindowRef().SetFramerateLimit(maxFrameRate);
		}

		/// <summary>
		/// Gets the currently configured maximum framerate limit.
		/// </summary>
		/// <returns>Frame rate cap.</returns>
		inline UInt GetMaxFrameRate() const
		{
			return maxFrameRate;
		}
		

	public:
		/// <summary>
		/// Constructs a TimerManager associated with the engine world.
		/// </summary>
		/// <param name="_engine">Pointer to Engine world.</param>
		TimerManager(Engine* _engine);

		/// <summary>
		/// Destructs the TimerManager and clears all active timers.
		/// </summary>
		~TimerManager();

	private:
		void UpdateTimers();

	public:
		/// <summary>
		/// Updates clock timestamps, recalculates deltaTime and FPS, and updates active timers.
		/// </summary>
		/// <returns>Delta time in seconds.</returns>
		Float Update();

		/// <summary>
		/// Formats and returns the current system wall clock time as a string "HH:MM:SS".
		/// </summary>
		/// <returns>Time string.</returns>
		static String GetCurrentRealTime();

		/// <summary>
		/// Creates and tracks a timer triggering a function callback after duration.
		/// </summary>
		/// <param name="_callback">Function callback to trigger.</param>
		/// <param name="_duration">Timer duration in seconds.</param>
		/// <param name="_isLoop">Whether timer repeats.</param>
		/// <param name="_startRunning">Whether timer starts immediately.</param>
		/// <returns>Pointer to created Timer.</returns>
		Timer* CreateTimer(const std::function<void()> _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);

		/// <summary>
		/// Creates and tracks a timer triggering a member function callback after duration.
		/// </summary>
		/// <typeparam name="T">Class type owning the member function.</typeparam>
		/// <typeparam name="MemFn">Member function pointer type.</typeparam>
		/// <param name="_instance">Pointer to object instance.</param>
		/// <param name="_memFn">Member function pointer.</param>
		/// <param name="_duration">Timer duration in seconds.</param>
		/// <param name="_isLoop">Whether timer repeats.</param>
		/// <param name="_startRunning">Whether timer starts immediately.</param>
		/// <returns>Pointer to created Timer.</returns>
		template<typename T, typename MemFn>
		inline Timer* CreateTimer(T* _instance, MemFn _memFn, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true)
		{
			return timers.emplace_back(
				std::make_unique<Timer>(_instance, _memFn, _duration, _isLoop, _startRunning)
			).get();
		}

		/// <summary>
		/// Destroys and removes a Timer from management.
		/// </summary>
		/// <param name="_toDelete">Pointer to Timer to delete.</param>
		void DestroyTimer(Timer* _toDelete);

		/// <summary>
		/// Pauses all active timers.
		/// </summary>
		void Pause();

		/// <summary>
		/// Resumes all paused timers.
		/// </summary>
		void Resume();

		/// <summary>
		/// Stops and resets all active timers.
		/// </summary>
		void Stop();
	};
}
