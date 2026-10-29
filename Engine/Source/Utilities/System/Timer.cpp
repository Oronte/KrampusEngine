#include "Timer.h"

using namespace Krampus;

Krampus::Timer::Timer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop, const Bool& _startRunning)
{
	callback = _callback;
	isRunning = _startRunning;
	isLoop = _isLoop;
	duration = _duration;
}

void Timer::Play()
{
	Reset();
	Resume();
}

void Timer::Update(const Float& _deltaTime)
{
	if (!isRunning) return;

	currentTime += _deltaTime;

	if (currentTime >= duration)
	{
		if (callback) callback();
		if (isToDelete) return;	// the callback stopped the timer

		if (isLoop)
			currentTime = duration > 0.0f ? currentTime - duration : Float(0.0f);	// keep the overshoot: a loop does not drift
		else
			Stop();
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