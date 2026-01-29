#pragma once
#include "Graphics/Window.h"

namespace Krampus
{

	class LevelManager;
	class InputManager;
	class Mouse;

	class Engine
	{
		bool						shouldClose = false;
		LevelManager*				levelManager = nullptr;
		InputManager*				inputManager = nullptr;
		Window						window;
		Mouse*						mouse = nullptr;
		Event<>::ListenerHandle		handle;

	public:
		Event<>						onEngineStart;
		Event<>						onEngineStop;
		
	public:
		LevelManager* GetLevelManager()
		{
			return levelManager;
		}
		InputManager* GetInputManagerRef()
		{
			return inputManager;
		}
		inline Window& GetWindowRef()
		{
			return window;
		}
		Mouse* GetMouse()
		{
			return mouse;
		}

		Engine();
		~Engine();

		void Start();
		void Update();
		void Stop();
	};
}

