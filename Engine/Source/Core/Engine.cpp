#include "Engine.h"
#include "Managers/TimerManager.h"
#include "Managers/InputManager.h"
#include "Managers/LevelManager.h"
#include "Graphics/Window/MainWindow.h"
#include "Graphics/Mouse.h"

using namespace Krampus;

Engine::Engine()
{
	MAIN_WINDOW.Create("EngineSFML", UVector2(1920, 1080));
}

void Engine::Start()
{
	onEngineStart.Broadcast();

	M_INPUT.WindowClose.AddListener([this](){
		MAIN_WINDOW.Close();
		M_LEVEL.SetLevel(nullptr);
		});

	Logger::Init();

	Update();
	Stop();
}

void Engine::Update()
{
	while (Level* _currentLevel = M_LEVEL.GetCurrentLevel())
	{
		Mouse::GetInstance().Update();
		_currentLevel->Update(M_TIMER.Update());
		const std::optional<sf::Event>& _event = MAIN_WINDOW.PollEvent();
		M_INPUT.Update(_event);
		M_INPUT.UpdateSystemEvent(_event);
	}
}

void Engine::Stop()
{
	onEngineStop.Broadcast();
	Logger::Shutdown();
}