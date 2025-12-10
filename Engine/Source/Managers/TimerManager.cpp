#include "TimerManager.h"
#include "Graphics/Window/MainWindow.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui-SFML.h"

using namespace Krampus;

Krampus::TimerManager::~TimerManager()
{
	allTimers.clear();
}

std::string Krampus::TimerManager::GetCurrentRealTime() const
{
	const time_t& _now = std::time(nullptr);

	std::tm _ltm;

#ifdef _MSC_VER
	localtime_s(&_ltm, &_now);
#else
	localtime_r(&_now, &_ltm);
#endif

	const std::string& _date = TwoDigitsTime(_ltm.tm_mday) + "/" + TwoDigitsTime(1 + _ltm.tm_mon) + "/" + TwoDigitsTime(1900 + _ltm.tm_year);
	const std::string& _time = TwoDigitsTime(_ltm.tm_hour) + ":" + TwoDigitsTime(_ltm.tm_min) + ":" + TwoDigitsTime(_ltm.tm_sec);

	return _date + " " + _time;
}

Timer* Krampus::TimerManager::CreateTimer(const std::function<void()>& _callback, const float& _duration, const bool& _startRunning, const bool& _isLoop)
{
    return allTimers.emplace_back(
        std::make_unique<Timer>(_callback, _duration, _startRunning, _isLoop)
    ).get();
}

float Krampus::TimerManager::Update()
{
	float _currentTime = GetTime(clock.getElapsedTime());

    // First frame initialization
    if (lastTimeStamp < 0.0f)
    {
        lastTimeStamp = _currentTime;
        lastFrameDuration = 0.0f;
    }
    else
    {
        lastFrameDuration = _currentTime - lastTimeStamp;
        if (lastFrameDuration < 0.0f)
            lastFrameDuration = 0.0f;
    }


    if (maxFrameRate > 0)
    {
        float _targetFrameTime = 1.0f / static_cast<float>(maxFrameRate);

        if (lastFrameDuration > 0.0f && lastFrameDuration < _targetFrameTime)
        {
            float _sleepTime = _targetFrameTime - lastFrameDuration;
            sf::sleep(sf::seconds(_sleepTime));

            // Recalculate correct timing after sleep
            float _newTime = GetTime(clock.getElapsedTime());
            lastFrameDuration = _newTime - lastTimeStamp;
            if (lastFrameDuration < 0.0f) lastFrameDuration = 0.0f;

            _currentTime = _newTime;
        }
    }


    lastTimeStamp = _currentTime;
    elapsedTime = lastFrameDuration;
    deltaTime = lastFrameDuration * timeScale;
    framesCount++;

    ImGui::SFML::Update(MAIN_WINDOW.GetRenderWindow(), sf::seconds(deltaTime));

    if (lastFrameDuration > 0.0f)
    {
        fps = 1.0f / lastFrameDuration;
        constexpr float _smoothingFactor = 0.1f;
        smoothedFPS = (1.0f - _smoothingFactor) * smoothedFPS + _smoothingFactor * fps;
    }

    UpdateTimers();

    return deltaTime;
}

void Krampus::TimerManager::UpdateTimers()
{
    for (std::vector<std::unique_ptr<Timer>>::iterator _iterator = allTimers.begin(); _iterator != allTimers.end(); )
    {
        Timer* _timer = _iterator->get();
        _timer->Update(deltaTime);

        if (_timer->IsToDelete())
        {
            _timer->Stop();
            _iterator = allTimers.erase(_iterator);
        }
        else ++_iterator;
    }
}

void Krampus::TimerManager::Pause()
{
	onPauseTimer.Broadcast();

	for (const std::unique_ptr<Timer>& _timer : allTimers) _timer->Pause();
}

void Krampus::TimerManager::Resume()
{
	onResumeTimer.Broadcast();

	for (const std::unique_ptr<Timer>& _timer : allTimers) _timer->Resume();
}

void Krampus::TimerManager::Stop()
{
	onStopTimer.Broadcast();

	for (const std::unique_ptr<Timer>& _timer : allTimers) _timer->Stop();
}