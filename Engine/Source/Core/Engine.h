#pragma once
#include "GameFramework/Level.h"
#include "Utilities/Singleton.h"

#define ENGINE Krampus::Engine::GetInstance()
#define KRAMPUS ENGINE

namespace Krampus
{

	class Engine : public Singleton<Engine>
	{
		//std::string windowSaveDir = "Content/Saves/";
		//std::string windowSaveFileName = "Window.json";

	public:
		Event<> onEngineStart;
		Event<> onEngineStop;
		
	public:
		Engine();

		void Start();
		void Update();
		void Stop();

	private:
		void UpdateEvent();
		//void SaveWindowInfo();
	};

}

