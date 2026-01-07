#pragma once
#include "Actors/Actor.h"
#include "Utilities/Math/Transform.h"

namespace Krampus
{

	class Level;

	class Widget : public Actor
	{
	protected:
		Widget(Level* _level);

	public:
		virtual void Destroy() override final;

		friend class HUD;
	};

}


