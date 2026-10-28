//Core engine includes
#include "Core/Engine.h"
#include "Utilities/KrampusInclude.h"

//Game and editor
#include "GameEntry.h"
#include "EditorLayer.h"

// Entry points called by the Editor host (Editor/Source/Main.cpp). The host owns the window and reloads this DLL when it is rebuilt.
// Everything else (engine statics, ImGui, the game) lives in this DLL, so each reload starts from a clean state.
#define MODULE_API extern "C" __declspec(dllexport)

namespace
{
	std::unique_ptr<Krampus::Engine> engine;
}

MODULE_API bool GameInit(void* _hostWindow)
{
	try
	{
		Krampus::Logger::canColorText = false;
		Krampus::Logger::canPlaySound = false;

		engine = std::make_unique<Krampus::Engine>(static_cast<sf::RenderWindow*>(_hostWindow));
		engine->GetLayerStack().PushOverlay<Krampus::EditorLayer>(engine.get());
		Krampus::GameStart(*engine);
		return true;
	}
	catch (const Krampus::Exception& _exception)
	{
		std::cerr << TXT_DARK_RED << _exception.What() << COLOR_RESET << std::endl;
		engine.reset();
		return false;
	}
}

/// Runs one frame. Returns false when the game wants to quit.
MODULE_API bool GameFrame()
{
	if (!engine) return false;

	try
	{
		return engine->Tick();
	}
	catch (const Krampus::Exception& _exception)
	{
		std::cerr << TXT_DARK_RED << _exception.What() << COLOR_RESET << std::endl;
		return false;
	}
}

MODULE_API void GameShutdown()
{
	if (!engine) return;

	engine->Stop();
	engine.reset();
}
