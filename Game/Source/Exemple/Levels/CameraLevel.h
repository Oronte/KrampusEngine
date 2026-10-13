#pragma once
#include "GameFramework/Level.h"

namespace Krampus::Exemple
{
	class CameraLevel : public Level
	{
		Event<>::ListenerHandle handle;
	public:
		CameraLevel(Engine* _world);

		virtual void InitLevel() override;
	};
}