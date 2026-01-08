#pragma once
#include "Actors/Actor.h"
#include "Utilities/Math/Transform.h"

namespace Krampus
{

	class Level;

	class Widget : public Actor
	{
	public:
		FVector2 screenPosition;
		Angle localRotation;

	protected:
		Widget(Level* _level);

	public:
		virtual void Destroy() override final;

	private:
		void UpdateWorldPos(const sf::View& _view);

		friend class HUD;
	};

}


