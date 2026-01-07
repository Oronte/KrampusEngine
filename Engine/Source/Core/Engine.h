#pragma once
#include "GameFramework/Level.h"
#include "Utilities/System/Singleton.h"

#define ENGINE Krampus::Engine::GetInstance()

namespace Krampus
{

	class Engine : public Singleton<Engine>
	{
	public:
		Event<> onEngineStart;
		Event<> onEngineStop;
		
	public:
		Engine();

		void Start();
		void Update();
		void Stop();
	};

}

