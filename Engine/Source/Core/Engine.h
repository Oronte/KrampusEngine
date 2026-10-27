#pragma once
#include "Graphics/Window.h"
#include "Managers/LayerStack.h"

namespace Krampus
{

	class LevelManager;
	class InputManager;
	class Mouse;

	/// <summary>
	/// Main engine class managing core runtime systems including the window, levels, inputs, and the game loop.
	/// </summary>
	class Engine
	{
		Bool						shouldClose = false;
		LevelManager*				levelManager = nullptr;
		InputManager*				inputManager = nullptr;
		Window						window;
		Mouse*						mouse = nullptr;
		LayerStack					layerStack;
		Event<>::ListenerHandle		handle;

	public:
		/// <summary>
		/// Event triggered when the engine stops.
		/// </summary>
		Event<>						onEngineStop;
		
	public:
		/// <summary>
		/// Gets the LevelManager responsible for loading and switching levels.
		/// </summary>
		/// <returns>Pointer to the LevelManager.</returns>
		inline LevelManager* GetLevelManager()
		{
			return levelManager;
		}

		/// <summary>
		/// Gets the InputManager responsible for handling input events.
		/// </summary>
		/// <returns>Pointer to the InputManager.</returns>
		inline InputManager* GetInputManager()
		{
			return inputManager;
		}

		/// <summary>
		/// Gets a reference to the main application window.
		/// </summary>
		/// <returns>Reference to the Window.</returns>
		inline Window& GetWindowRef()
		{
			return window;
		}

		/// <summary>
		/// Gets the stack of layers (level, UI, overlays...) updated, drawn and fed events each frame.
		/// </summary>
		/// <returns>Reference to the LayerStack.</returns>
		inline LayerStack& GetLayerStack()
		{
			return layerStack;
		}

		/// <summary>
		/// Gets the Mouse input handler.
		/// </summary>
		/// <returns>Pointer to the Mouse instance.</returns>
		Mouse* GetMouse()
		{
			return mouse;
		}

		/// <summary>
		/// Constructs the Engine and initializes sub-managers and window.
		/// </summary>
		Engine();

		/// <summary>
		/// Destructs the Engine and cleans up resources.
		/// </summary>
		~Engine();

		/// <summary>
		/// Starts the main engine loop.
		/// </summary>
		void Start();

		/// <summary>
		/// Signals the engine to quit the game loop.
		/// </summary>
		void QuitGame();

		/// <summary>
		/// Updates engine systems and polls window events for one iteration of the game loop.
		/// </summary>
		void Update();

	private:
		void Stop();
	};
}
