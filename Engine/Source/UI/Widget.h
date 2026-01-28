#pragma once
#include "Actors/Actor.h"
#include "Utilities/Math/Transform.h"

namespace Krampus
{

	class Level;

	class Widget : public Actor
	{
		Event<sf::View>::ListenerHandle onViewChangeHandle;

	protected:
		FVector2 screenPosition;
		Angle localRotation;

	protected:
		inline FVector2 GetScreenPosition() const
		{
			return screenPosition;
		}
		inline Angle GetLocalRotation() const
		{
			return localRotation;
		}

		Widget(Level* _level);

	public:
		void SetScreenPosition(const FVector2& _screenPos);
		void SetLocalRotation(const Angle& _rotation);
		virtual void Destroy() override final;

	private:
		void UpdateWorldPos(const sf::View& _view);

		friend class HUD;
	};

}


