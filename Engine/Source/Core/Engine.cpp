#include "Engine.h"
#include "Managers/InputManager.h"
#include "Network/NetworkManager.h"
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

	levelManager = std::make_unique<LevelManager>(this);
	inputManager = std::make_unique<InputManager>(this);
	network = std::make_unique<NetworkManager>(this);
	mouse = std::make_unique<Mouse>(this);

	settings.Attach(this);
	settings.LoadFile();	// before the window: the Ship section says how it looks

	if (_hostWindow)
	{
		window.Attach(_hostWindow);
	}
	else
	{
		const ShipSettings& _ship = settings.ship;
		const UVector2 _size(static_cast<unsigned int>(std::max(320, _ship.windowWidth)), static_cast<unsigned int>(std::max(240, _ship.windowHeight)));
		if (_ship.windowMode == 0)
			window.Create(String(_ship.gameName));
		else
			window.Create(String(_ship.gameName), _size, _ship.windowMode == 2);
		window.SetFramerateLimit(0);
	}
	settings.Apply();

	layerStack.PushLayer<LevelLayer>(this);
	layerStack.PushOverlay<Krampus::FadeLayer>(this);
	layerStack.PushOverlay<Krampus::PauseLayer>();
#ifdef SHIP
	if (settings.ship.showSplash)
		layerStack.PushOverlay<Krampus::SplashLayer>(CONTENT_PATH +"Textures/_internal/KrampusEngineBanner.png");
#endif

	handle = inputManager->WindowClose.AddListener(this, &Engine::QuitGame);

	LOG_MSG("Engine: ready (window " + std::to_string(window.GetSize().x) + "x" + std::to_string(window.GetSize().y) + (_hostWindow ? ", hosted by the editor" : ", own window") + ")");
}

Engine::~Engine()
{
	levelManager.reset();	// in this order: the level goes first, the input last
	network.reset();
	inputManager.reset();
	mouse.reset();

	// Stop() already did it on a normal exit; after an exception the engine is destroyed without Stop(), and a log thread
	// still running at the end of the program would terminate it.
	Logger::Shutdown();
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

	network->Update();	// receives what arrived, even while the level is paused (edit mode)

	levelManager->ChangeLevel();
	Level* _currentLevel = levelManager->GetCurrentLevel();
	if (!_currentLevel) return false;
	if (!levelPaused) _currentLevel->StartPlay();	// the first frame of Play: the actors get their BeginPlay

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

		const Bool _consumed = layerStack.DispatchEvent(*_event);
		if (!_consumed && !layerStack.BlocksInput())
			inputManager->Update(_event);
		// The system events always go through (focus, resize...), except a close a layer took: it asks the user first (the editor).
		if (!_consumed || !_event->is<sf::Event::Closed>())
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