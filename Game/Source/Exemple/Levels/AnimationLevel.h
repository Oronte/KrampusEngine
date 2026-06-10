#pragma once
#include "GameFramework/Level.h"

namespace Krampus::Exemple
{
	class AnimationLevel : public Level
	{
	public:
		AnimationLevel(Engine* _world);

	protected:
		virtual void InitLevel() override;
	};
}