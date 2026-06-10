#pragma once
#include "GameFramework/Level.h"

namespace Krampus::Exemple
{
	class MainMenu : public Level
	{
		Event<>::ListenerHandle quitHandle;

	public:
		MainMenu(Engine* _world);

	protected:
		virtual void InitLevel() override;
	};
}