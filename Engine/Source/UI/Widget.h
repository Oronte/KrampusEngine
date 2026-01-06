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

		friend class HUD;
	};

}


