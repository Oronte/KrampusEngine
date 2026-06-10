#pragma once
#include "GameFramework/Level.h"

namespace Krampus::Exemple
{
	class CameraLevel : public Level
	{
	public:
		CameraLevel(Engine* _world);

		virtual void InitLevel() override;
	};
}