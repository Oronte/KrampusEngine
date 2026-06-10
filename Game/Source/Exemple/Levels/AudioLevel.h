#pragma once
#include "GameFramework/Level.h"

namespace Krampus::Exemple
{
	class AudioLevel : public Level
	{
	public:
		AudioLevel(Engine* _world);

	protected:
		virtual void InitLevel() override;
	};
}