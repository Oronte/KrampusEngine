#include "Engine.h"
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

	window.Create();
	window.SetFramerateLimit(0);

	handle = inputManager->WindowClose.AddListener(this, &Engine::QuitGame);

	Logger::Init();
}

Engine::~Engine()
{
	delete levelManager;
	delete inputManager;
	delete mouse;
}

void Engine::Start()
{
	Update();
	Stop();
}

void Krampus::Engine::QuitGame()
{
	shouldClose = true;
}

void Engine::Update()
{
	while (!shouldClose)
	{
		levelManager->ChangeLevel();
		Level* _currentLevel = levelManager->GetCurrentLevel();
		if (!_currentLevel) break;

		mouse->Update();
		_currentLevel->Update(_currentLevel->UpdateTime());
		const std::optional<sf::Event>& _event = window.PollEvent();
		inputManager->Update(_event);
		inputManager->UpdateSystemEvent(_event);
	}
}

void Engine::Stop()
{
	onEngineStop.Broadcast();

	if (Level* _level = levelManager->GetCurrentLevel())
		_level->Unload();

	window.Close();
	Logger::Shutdown();
}