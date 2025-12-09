#pragma once
#include "GameFramework/Level.h"

namespace Krampus
{

	class Engine
	{
		std::string windowSaveDir = "Content/Saves/";
		std::string windowSaveFileName = "Window.json";

	public:
		Event<> onEngineStart;
		Event<> onEngineStop;

		Event<> onFocusGained;
		Event<> onFocusLost;
		Event<UVector2> onWindowResized;
		Event<IVector2> onMouseMoved;
		Event<> onMouseEntered;
		Event<> onMouseExit;
		Event<> onWindowClosed;
		
	public:
		Engine();

		void Start();
		void Update();
		void Stop();

	private:
		void UpdateEvent();
		void SaveWindowInfo();
	};

}

