#pragma once
#include "GameFramework/Level.h"

namespace Krampus::Exemple
{

	class LogLevel : public Level
	{
		Event<>::ListenerHandle messageLogHandle;
		Event<>::ListenerHandle warningLogHandle;
		Event<>::ListenerHandle errorLogHandle;
		Event<>::ListenerHandle fatalLogHandle;

	public:
		LogLevel(Engine* _world);

		virtual void InitLevel() override;
	};

}