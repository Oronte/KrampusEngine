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
	timerManager = new TimerManager(this);
	mouse = new Mouse(this);
}

Engine::~Engine()
{
	delete levelManager;
	delete inputManager;
	delete mouse;
}

void Engine::Start()
{
	window.Create();
	window.SetFramerateLimit(0);

	handle = inputManager->WindowClose.AddListener([this]()
		{
			levelManager->GetCurrentLevel()->Unload();
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
		_currentLevel->Update(timerManager->Update());
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