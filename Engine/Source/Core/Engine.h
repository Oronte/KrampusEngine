#pragma once
#include "Graphics/Window.h"

namespace Krampus
{

	class LevelManager;
	class InputManager;
	class Mouse;

	class Engine
	{
		Bool						shouldClose = false;
		LevelManager*				levelManager = nullptr;
		InputManager*				inputManager = nullptr;
		Window						window;
		Mouse*						mouse = nullptr;
		Event<>::ListenerHandle		handle;

	public:
		Event<>						onEngineStop;
		
	public:
		inline LevelManager* GetLevelManager()
		{
			return levelManager;
		}
		inline InputManager* GetInputManager()
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
		void QuitGame();
		void Update();

	private:
		void Stop();
	};
}

