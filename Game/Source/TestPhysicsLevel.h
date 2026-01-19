#pragma once
#include "GameFramework/Level.h"

namespace Krampus
{

	class TestPhysicsLevel : public Level
	{
	public:
		TestPhysicsLevel() = default;
		virtual void InitLevel() override;
	};

}

