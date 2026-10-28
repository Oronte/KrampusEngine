#include "Engine.h"
#include "Managers/InputManager.h"
#include "Managers/LevelManager.h"
#include "Managers/AudioManager.h"
#include "Layers/LevelLayer.h"
#include "Layers/FadeLayer.h"
#include "Layers/PauseLayer.h"
#include "Layers/SplashLayer.h"
#include "Graphics/Mouse.h"

using namespace Krampus;
#undef new

Engine::Engine(sf::RenderWindow* _hostWindow)
{
	Logger::Init();	// first: everything below can log
	LOG_MSG("Engine: starting");

	levelManager = new LevelManager(this);
	inputManager = new InputManager(this);
	mouse = new Mouse(this);

	if (_hostWindow)
	{
		window.Attach(_hostWindow);
	}
	else
	{
		window.Create();
		window.SetFramerateLimit(0);
	}

	settings.Attach(this);
	settings.Load();

	layerStack.PushLayer<LevelLayer>(this);
	layerStack.PushOverlay<Krampus::FadeLayer>(this);
	layerStack.PushOverlay<Krampus::PauseLayer>();
#ifdef SHIP
	layerStack.PushOverlay<Krampus::SplashLayer>(CONTENT_PATH +"Textures/_internal/KrampusEngineBanner.png");
#endif

	handle = inputManager->WindowClose.AddListener(this, &Engine::QuitGame);

	LOG_MSG("Engine: ready (window " + std::to_string(window.GetSize().x) + "x" + std::to_string(window.GetSize().y) + (_hostWindow ? ", hosted by the editor" : ", own window") + ")");
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
	LOG_MSG("Engine: quit requested");
	shouldClose = true;
}

void Engine::Update()
{
	while (Tick()) { }
}

Bool Engine::Tick()
{
	if (shouldClose) return false;

	levelManager->ChangeLevel();
	Level* _currentLevel = levelManager->GetCurrentLevel();
	if (!_currentLevel) return false;

	mouse->Update();
	layerStack.Update(_currentLevel->UpdateTime());
	layerStack.Render(window);
	window.Display();

	while (true)
	{
		const std::optional<sf::Event> _event = GetWindowRef().PollEvent();
		if (!_event.has_value()) break;

		// Window events worth a trace (not the mouse or keyboard ones: too many).
		if (const sf::Event::Resized* _resized = _event->getIf<sf::Event::Resized>())
			LOG(VerbosityType::Log, "Window: resized to " + std::to_string(_resized->size.x) + "x" + std::to_string(_resized->size.y));
		else if (_event->is<sf::Event::FocusLost>())
			LOG(VerbosityType::Log, "Window: focus lost");
		else if (_event->is<sf::Event::FocusGained>())
			LOG(VerbosityType::Log, "Window: focus gained");
		else if (_event->is<sf::Event::Closed>())
			LOG_MSG("Window: close requested");

		if (!layerStack.DispatchEvent(*_event) && !layerStack.BlocksInput())
			inputManager->Update(_event);
		inputManager->UpdateSystemEvent(_event);
	}
	if (!layerStack.BlocksInput())
		inputManager->UpdateOnPerform();

	return !shouldClose;
}

void Engine::Stop()
{
	LOG_MSG("Engine: stopping");
	onEngineStop.Broadcast();
	layerStack.Clear();

	if (Level* _level = levelManager->GetCurrentLevel())
		_level->Unload();

	LOG(VerbosityType::Log, "Engine: audio cache cleared");
	M_AUDIO.ClearCache();
	window.Close();
	Logger::Shutdown();
}