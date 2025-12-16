#include "Timer.h"

using namespace Krampus;

Krampus::Timer::Timer(const std::function<void()>& _callback, const float& _duration, const bool& _startRunning, const bool& _isLoop)
{
	callback.AddListener(_callback);
	isRunning = _startRunning;
	isLoop = _isLoop;
	duration = _duration;
}

Timer::Timer(const float& _duration, const bool& _startRunning, const bool& _isLoop)
{
	isRunning = _startRunning;
	isLoop = _isLoop;
	duration = _duration;
}

void Timer::Start()
{
	Reset();
	Resume();
}

void Timer::Update(const float& _deltaTime)
{
	if (!isRunning) return;

	currentTime += _deltaTime;

	if (currentTime >= duration)
	{
		callback.Broadcast();

		isLoop ? Reset() : Stop();
	}
}

void Timer::Stop()
{
	Pause();
	isToDelete = true;
}

void Timer::Resume()
{
	isRunning = true;
}

void Timer::Reset()
{
	currentTime = 0.0f;
}

void Timer::Pause()
{
	isRunning = false;
}