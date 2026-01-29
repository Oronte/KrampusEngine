#include "Engine.h"
#include "Managers/TimerManager.h"
#include "Managers/InputManager.h"
#include "Managers/LevelManager.h"
#include "Graphics/Mouse.h"

using namespace Krampus;
#undef new

Engine::Engine()
{
	levelManager = new LevelManager(this);
	inputManager = new InputManager(this);
	mouse = new Mouse(this);
	window.Create("EngineSFML", UVector2(1920, 1080));
}

Engine::~Engine()
{
	delete levelManager;
	delete inputManager;
	delete mouse;
}

void Engine::Start()
{
	onEngineStart.Broadcast();

	handle = inputManager->WindowClose.AddListener([this]()
		{
			window.Close();
			shouldClose = true;
		});

	Logger::Init();

	Update();
	Stop();
}

void Engine::Update()
{
	while (!shouldClose)
	{
		Level* _currentLevel = levelManager->GetCurrentLevel();
		if (!_currentLevel) break;

		mouse->Update();
		_currentLevel->Update(M_TIMER.Update());
		const std::optional<sf::Event>& _event = window.PollEvent();
		inputManager->Update(_event);
		inputManager->UpdateSystemEvent(_event);
	}
}

void Engine::Stop()
{
	onEngineStop.Broadcast();
	Logger::Shutdown();
}