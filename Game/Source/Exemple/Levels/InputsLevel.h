#pragma once
#include "GameFramework/Level.h"

namespace Krampus::Exemple
{

	class InputsLevel : public Level
	{
	public:
		InputsLevel(Engine* _world);

		virtual void InitLevel() override;
	};

}