#include "Engine.h"
#include "Core/AssetPack.h"
#include "Managers/InputManager.h"
#include "Network/NetworkManager.h"
#include "Managers/LevelManager.h"
#include "Managers/AudioManager.h"
#include "Layers/LevelLayer.h"
#include "Layers/FadeLayer.h"
#include "Layers/PauseLayer.h"
#include "Layers/SplashLayer.h"
#include "Graphics/Mouse.h"
#include "Utilities/Debug/Profiler.h"

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

		// The icon of the window (title bar, taskbar): Settings > Ship > icon (the exe gets the same one, File > Package Game), else
		// the logo. Made 64 px: a big picture shows in the title bar but not in the taskbar (nor did the icon loaded from the exe).
		{
			const std::string _file = _ship.icon.empty() ? std::string(ShipSettings::DEFAULT_ICON) : _ship.icon;
			std::vector<std::uint8_t> _bytes;
			sf::Image _picture;
			if (AssetPack::ReadAll(CONTENT_PATH + "Textures/" + _file, _bytes) && _picture.loadFromMemory(_bytes.data(), _bytes.size()))
			{
				constexpr unsigned SIZE = 64;
				const sf::Vector2u _from = _picture.getSize();
				const unsigned _side = std::max(_from.x, _from.y);
				const int _left = static_cast<int>((_side - _from.x) / 2), _top = static_cast<int>((_side - _from.y) / 2);
				sf::Image _icon({ SIZE, SIZE }, sf::Color::Transparent);
				for (unsigned _y = 0; _y < SIZE; ++_y)	// the picture centered in a square, each pixel the average of those it covers
					for (unsigned _x = 0; _x < SIZE; ++_x)
					{
						const unsigned _x0 = _x * _side / SIZE, _y0 = _y * _side / SIZE;
						const unsigned _x1 = std::max(_x0 + 1, (_x + 1) * _side / SIZE), _y1 = std::max(_y0 + 1, (_y + 1) * _side / SIZE);
						double _r = 0.0, _g = 0.0, _b = 0.0, _a = 0.0;
						for (unsigned _sy = _y0; _sy < _y1; ++_sy)
							for (unsigned _sx = _x0; _sx < _x1; ++_sx)
							{
								const int _px = static_cast<int>(_sx) - _left, _py = static_cast<int>(_sy) - _top;
								if (_px < 0 || _py < 0 || _px >= static_cast<int>(_from.x) || _py >= static_cast<int>(_from.y)) continue;
								const sf::Color _c = _picture.getPixel({ static_cast<unsigned>(_px), static_cast<unsigned>(_py) });
								_r += _c.r * static_cast<double>(_c.a);
								_g += _c.g * static_cast<double>(_c.a);
								_b += _c.b * static_cast<double>(_c.a);
								_a += _c.a;
							}
						if (_a > 0.0)
							_icon.setPixel({ _x, _y }, sf::Color(static_cast<std::uint8_t>(_r / _a + 0.5), static_cast<std::uint8_t>(_g / _a + 0.5),
								static_cast<std::uint8_t>(_b / _a + 0.5), static_cast<std::uint8_t>(_a / ((_x1 - _x0) * (_y1 - _y0)) + 0.5)));
					}
				window.GetRenderWindow().setIcon(_icon);
			}
			else
				LOG_WARNING("Engine: the icon Textures/" + _file + " (Settings > Ship) cannot be read");
		}
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

	{
		PROFILE_SCOPE("Network");
		network->Update();	// receives what arrived, even while the level is paused (edit mode)
	}

	levelManager->ChangeLevel();
	Level* _currentLevel = levelManager->GetCurrentLevel();
	if (!_currentLevel) { Profiler::EndFrame(); return false; }
	if (!levelPaused) _currentLevel->StartPlay();	// the first frame of Play: the actors get their BeginPlay

	mouse->Update();
	Float _deltaTime = 0.f;
	{
		PROFILE_SCOPE("Timers + frame limit");	// the wait of maxFrameRate is in it
		_deltaTime = _currentLevel->UpdateTime();
	}
	{
		PROFILE_SCOPE("Update");
		layerStack.Update(_deltaTime);
	}
	{
		PROFILE_SCOPE("Render");
		layerStack.Render(window);
	}
	{
		PROFILE_SCOPE("Display");	// waits for the graphics card (and the screen with vsync)
		window.Display();
	}

	Profiler::Begin("Events");
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

	Profiler::End();
	Profiler::EndFrame();
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