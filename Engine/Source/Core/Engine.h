#pragma once
#include "Graphics/Window.h"

namespace Krampus
{

	class LevelManager;
	class InputManager;
	class TimerManager;
	class Mouse;

	class Engine
	{
		Bool						shouldClose = false;
		LevelManager*				levelManager = nullptr;
		InputManager*				inputManager = nullptr;
		TimerManager*				timerManager = nullptr;
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
		inline TimerManager* GetTimerManager()
		{
			return timerManager;
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

