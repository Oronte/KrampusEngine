#pragma once
#include "GameFramework/Level.h"

namespace Krampus
{

	class Engine
	{
		bool shouldClose = false;

		Event<>::ListenerHandle onWindowCloseHandle;
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

